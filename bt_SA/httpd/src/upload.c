#include <sys/ioctl.h>
#include "httpd.h"
#include "utils.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#ifdef TRX_VALIDATE_SUPPORT
#include "trxhdr.h"
#endif

#ifdef CONFIG_HTTPD_PLC_UPGRADE
#include <sys/ipc.h>
#include <sys/shm.h>
#define PLC_FW_SHARED_MEMORY_KEY   0x12345

// in cfe availableSizeOneImage=7279KB
#ifdef CONFIG_HTTPD_ZZ_UPGRADE
#if defined(VR9517PAC22_A_PP)
#define PLC_IMAGE_MAXLEN 0x1200000      //PLDT project has 256M RAM, current def 18M
#elif defined(AR7516AOW22_A_PP)
#define PLC_IMAGE_MAXLEN 0x680000      //PLDT ADSL project only has 32M RAM & 8M SPI flash
#elif defined(VR7516RW22)
#define PLC_IMAGE_MAXLEN 0xC00000      //Telmex project only has 64M RAM
#else
#define PLC_IMAGE_MAXLEN 0x3C00000		// Velmurugan 07022015 > image size is getting bigger and bigger, allow 60M,
#endif
#else
#define PLC_IMAGE_MAXLEN 0x71BC00
#endif
#endif

#define	MIN_BUF_SIZE	4096

/* Upgrade from remote server or socket stream */
int sys_upgrade(struct request_rec *r, FILE *stream, int *total, int b_rom)
{
	char upload_fifo[] = "/tmp/uploadXXXXXX";
	FILE *fifo = NULL;
	char *write_argv[] = { "arcupload", upload_fifo, ((b_rom == 1) ? "rom" : "linux"), NULL };
	pid_t pid;
	char *buf = NULL;
	int count, ret = 0;
	long flags = -1;
	int size = BUFSIZ;
	int fd;
#ifdef TRX_VALIDATE_SUPPORT
	int i = 0;
	struct trx_header trx;
#endif
	//crazy_liang 2013.3.15: Fix waitpid return -1 and cannot get status issue.
	struct sigaction new_sa;
	struct sigaction old_sa;
	int ret_sa = 0;

	memset(&new_sa, 0x00, sizeof(struct sigaction));
	memset(&old_sa, 0x00, sizeof(struct sigaction));

	/*
	 * After exit(), the exit status must be transmitted to 
	 * the parent process. There are three cases. If the 
	 * parent has set SA_NOCLDWAIT, or has set the SIGCHLD 
	 * handler to SIG_IGN, the status is discarded...
	 */
	//Here need use waitpid to get the status of child process, 
	//set SIGCHLD to SIG_DFL and restore it later.
	new_sa.sa_handler = SIG_DFL;
	sigemptyset(&new_sa.sa_mask);
	new_sa.sa_flags = 0;
	//end crazy_liang 2013.3.15

	/* system is under upgrading or configuration is updating, blink power led */
	//system("echo 1 > /proc/sys/blink_diag_led");
#if defined(VRV9518SWAC33)
	system("/sbin/arc_led_all_blink_on.sh");
#elif defined (VRV7518CW22) || defined (VRV7006AW22)
	system("arc_led upgrade blink");
#else
	system("arc_led power blink");
#endif
	
	//crazy_liang 2013.3.15: Try to backup signal handler of SIGCHLD and set it to SIG_DFL.
	if((ret_sa = sigaction(SIGCHLD, &new_sa, &old_sa)) != 0)
	{
		cprintf("Failed to backup signal handler for SIGCHLD[%d]\n", SIGCHLD);
	}
	//end crazy_liang 2013.3.15

	/* temporary filename */
	if ((fd = mkstemp(upload_fifo)) < 0) {
		goto err;
	}
	unlink(upload_fifo);
	close(fd);

	if (!(fifo = fopen(upload_fifo, "w"))) {
		goto err;
	}

	if (r->ishttps == 0) {
		if ((flags = fcntl(fileno(stream), F_GETFL)) < 0 ||
				fcntl(fileno(stream), F_SETFL, flags | O_NONBLOCK) < 0) {
			ret = errno;
			goto err;
		}
	}

	/*
	* The buffer must be at least as big as what the stream file is
	* using so that it can read all the data that has been buffered
	* in the stream file. Otherwise it would be out of sync with fn
	* select specially at the end of the data stream in which case
	* the select tells there is no more data available but there in
	* fact is data buffered in the stream file's buffer. Since no
	* one has changed the default stream file's buffer size, let's
	* use the constant BUFSIZ until someone changes it.
	*/
	if (size < MIN_BUF_SIZE)
		size = MIN_BUF_SIZE;
	if ((buf = malloc(size)) == NULL) {
		ret = ENOMEM;
		goto err;
	}

	/* Pipe the rest to the FIFO */
	printf("Upgrading.\n");
	while (total && *total) {
#ifdef CONFIG_HTTPD_HTTPS_SUPPORT
		if (r->ishttps) {

			if (size > *total) 
				size=*total;

			if ( (count=as_read(r->ssrv, buf, size)) <= 0) {
				ret = EINVAL;
				goto err;
			}
		}
		else
#endif
		{
			if (waitfor(fileno(stream), 5) <= 0)
				break;

			//crazy_liang 2013.3.12: Do not read the tail here. 
			if(size > *total)
			{
				size = *total;
			}
			//end crazy_liang 2013.3.12

			count = safe_fread(buf, 1, size, stream);
			if (!count && (ferror(stream) || feof(stream)))
				break;
		}

#ifdef TRX_VALIDATE_SUPPORT
		if(b_rom != 1) 
		{
		if (i++ == 0) { 

			memcpy(&trx, buf, sizeof(struct trx_header));

			/* TRX header */
			if (trx.magic != TRX_MAGIC) {
				cprintf("TRX header is not correct!\n");
				ret = EINVAL;
				goto err;
			}
			cprintf("TRX header is correct!\n");
		}
		}
#endif

		*total -= count;

		//crazy_liang 2013.3.14: Check the return value. If write failed then halt.
		if(safe_fwrite(buf, 1, count, fifo) != count)
		{
			cprintf("Write %s failed! There are %d was not written!\n", upload_fifo, *total);
			ret = -1;
			goto err;
		}
		//end crazy_liang 2013.3.14

		printf(".");
	}
	
	fclose(fifo);
	fifo = NULL;

	if ((ret = _eval(write_argv, NULL, 0, &pid)) != 0) {
		if (!ret)
			ret = errno;
		goto err;
	}

	/* Wait for write to terminate */
	waitpid(pid, &ret, 0);
	
	cprintf("upload return: %d\n", ret);
	/* check if mtd_write is ok */
	if (WIFEXITED(ret)) {
		if (WEXITSTATUS(ret) != 0) {
			ret = -1;
		}
	}

	printf("done\n");

	if (r->ishttps == 0) {
		/* Reset nonblock on the socket */
		if (fcntl(fileno(stream), F_SETFL, flags) < 0) {
			ret = errno;
			goto err;
		}
	}

	goto ok;

err:
#if defined(VRV9518SWAC33)
	system("/sbin/arc_led_all_blink_off.sh");
#elif defined (VRV7518CW22)
	system("arc_led upgrade off");
#endif

ok:
	//crazy_liang 2013.3.15: Fix waitpid return -1 and cannot get status issue.
	if(ret_sa == 0)
	{
		//Try to restore signal handler of SIGCHLD.
		if(sigaction(SIGCHLD, &old_sa, NULL) != 0)
		{
			cprintf("Failed to restore signal handler for SIGCHLD[%d]\n", SIGCHLD);
		}
	}
	//end crazy_liang 2013.3.15

 	if (buf)
		free(buf);
	if (fifo)
		fclose(fifo);

	unlink(upload_fifo);

	return ret;
}


int sys_restore(struct request_rec *r, char *path, FILE *stream, int *total)
{
	char up_file[256];
	int ret = 0;
	FILE *fp = NULL;
	char buf[1024];
	int flags = 0;
	int count = 0;
	
	if (!path)
		return -1;
	/* system is under upgrading or configuration is updating, blink power led */
#if defined(VRV9518SWAC33)
	system("/sbin/arc_led_all_blink_on.sh");
#else
	system("arc_led power blink");
#endif

	snprintf(up_file, sizeof(up_file)-1, "%s", path);
	up_file[sizeof(up_file)-1] = '\0';

	if ((fp=fopen(up_file, "w")) == NULL)
		return -1;

	if (r->ishttps == 0) {
		if ((flags = fcntl(fileno(stream), F_GETFL)) < 0 ||
				fcntl(fileno(stream), F_SETFL, flags | O_NONBLOCK) < 0) {
			return -1;
		}
	}

	printf("Restore..\n");
	while (total && *total) {

#ifdef CONFIG_HTTPD_HTTPS_SUPPORT
		if (r->ishttps) {
			int size = sizeof(buf);

			if (size > *total) 
				size = *total;

			if ( (count=as_read(r->ssrv, buf, size)) <= 0) {
				ret = EINVAL;
				break;
			}
		}
		else 
#endif
		{
			if (waitfor(fileno(stream), 5) <= 0) {
				/* NOTE: Even if select timeout,
				 * there are data cached on the stream, try to get them.
				 */
				while(!(ferror(stream) || feof(stream)))
				{
					//read the rest of bufs.
					count = safe_fread(buf, 1, sizeof(buf), stream);
					/* nothing to read, escape */
					if (count == 0) {
						ret = -1;
						break;
					}
					*total -= count;
					safe_fwrite(buf, 1, count, fp);
				}

				break;
			}

			count = safe_fread(buf, 1, sizeof(buf), stream);
			if (!count && (ferror(stream) || feof(stream))) {
				ret = -1;
				break;
			}
		}

		*total -= count;
		safe_fwrite(buf, 1, count, fp);

		printf(".");
	}

	fclose(fp);
	fp = NULL;

	printf("done");

	if (r->ishttps == 0) {
		/* Reset nonblock on the socket */
		if (fcntl(fileno(stream), F_SETFL, flags) < 0) {
			return -1;
		}
	}

	/*return 0;*/
	return ret;
}
#ifdef CONFIG_HTTPD_LTE_UPGRADE
// LTE firmware upload and upgrade from remote server or socket stream */
int sys_lte_upgrade(struct request_rec *r, FILE *stream, int *total)
{
	char upload_fifo[] = "/tmp/uploadXXXXXX";
	FILE *fifo = NULL;
	char *write_argv[] = { "umts_firmwareupt", "upgrade", upload_fifo,  NULL };
	pid_t pid;
	char *buf = NULL;
	int count, ret = 0;
	long flags = -1;
	int size = BUFSIZ;
	int fd;
	//crazy_liang 2013.3.15: Fix waitpid return -1 and cannot get status issue.
	struct sigaction new_sa;
	struct sigaction old_sa;
	int ret_sa = 0;

	memset(&new_sa, 0x00, sizeof(struct sigaction));
	memset(&old_sa, 0x00, sizeof(struct sigaction));

	/*
	 * After exit(), the exit status must be transmitted to
	 * the parent process. There are three cases. If the
	 * parent has set SA_NOCLDWAIT, or has set the SIGCHLD
	 * handler to SIG_IGN, the status is discarded...
	 */
	//Here need use waitpid to get the status of child process,
	//set SIGCHLD to SIG_DFL and restore it later.
	new_sa.sa_handler = SIG_DFL;
	sigemptyset(&new_sa.sa_mask);
	new_sa.sa_flags = 0;
	//end crazy_liang 2013.3.15

	/* system is under upgrading or configuration is updating, blink power led */
	//system("echo 1 > /proc/sys/blink_diag_led");
	system("arc_led power blink");

	//crazy_liang 2013.3.15: Try to backup signal handler of SIGCHLD and set it to SIG_DFL.
	if((ret_sa = sigaction(SIGCHLD, &new_sa, &old_sa)) != 0)
	{
		cprintf("Failed to backup signal handler for SIGCHLD[%d]\n", SIGCHLD);
	}
	//end crazy_liang 2013.3.15

	/* temporary filename */
	if ((fd = mkstemp(upload_fifo)) < 0) {
		goto err;
	}
	unlink(upload_fifo);
	close(fd);

	if (!(fifo = fopen(upload_fifo, "w"))) {
		goto err;
	}

	if (r->ishttps == 0) {
		if ((flags = fcntl(fileno(stream), F_GETFL)) < 0 ||
				fcntl(fileno(stream), F_SETFL, flags | O_NONBLOCK) < 0) {
			ret = errno;
			goto err;
		}
	}

	/*
	* The buffer must be at least as big as what the stream file is
	* using so that it can read all the data that has been buffered
	* in the stream file. Otherwise it would be out of sync with fn
	* select specially at the end of the data stream in which case
	* the select tells there is no more data available but there in
	* fact is data buffered in the stream file's buffer. Since no
	* one has changed the default stream file's buffer size, let's
	* use the constant BUFSIZ until someone changes it.
	*/
	if (size < MIN_BUF_SIZE)
		size = MIN_BUF_SIZE;
	if ((buf = malloc(size)) == NULL) {
		ret = ENOMEM;
		goto err;
	}

	/* Pipe the rest to the FIFO */
	printf("Upgrading.\n");
	while (total && *total) {
#ifdef CONFIG_HTTPD_HTTPS_SUPPORT
		if (r->ishttps) {

			if (size > *total)
				size=*total;

			if ( (count=as_read(r->ssrv, buf, size)) <= 0) {
				ret = EINVAL;
				goto err;
			}
		}
		else
#endif
		{
			if (waitfor(fileno(stream), 5) <= 0)
				break;

			//crazy_liang 2013.3.12: Do not read the tail here.
			if(size > *total)
			{
				size = *total;
			}
			//end crazy_liang 2013.3.12

			count = safe_fread(buf, 1, size, stream);
			if (!count && (ferror(stream) || feof(stream)))
				break;
		}

		*total -= count;

		//crazy_liang 2013.3.14: Check the return value. If write failed then halt.
		if(safe_fwrite(buf, 1, count, fifo) != count)
		{
			cprintf("Write %s failed! There are %d was not written!\n", upload_fifo, *total);
			ret = -1;
			goto err;
		}
		//end crazy_liang 2013.3.14

		printf(".");
	}

	fclose(fifo);
	fifo = NULL;

	if ((ret = _eval(write_argv, NULL, 0, &pid)) != 0) {
		if (!ret)
			ret = errno;
		goto err;
	}

	/* Wait for write to terminate */
	waitpid(pid, &ret, 0);

	cprintf("lte upload return: %d\n", ret);
	/* check if mtd_write is ok */
	if (WIFEXITED(ret)) {
		if (WEXITSTATUS(ret) != 0) {
			ret = -1;
		}
	}

	printf("lte upgrade done\n");

	if (r->ishttps == 0) {
		/* Reset nonblock on the socket */
		if (fcntl(fileno(stream), F_SETFL, flags) < 0) {
			ret = errno;
			goto err;
		}
	}

 err:
	//crazy_liang 2013.3.15: Fix waitpid return -1 and cannot get status issue.
	if(ret_sa == 0)
	{
		//Try to restore signal handler of SIGCHLD.
		if(sigaction(SIGCHLD, &old_sa, NULL) != 0)
		{
			cprintf("Failed to restore signal handler for SIGCHLD[%d]\n", SIGCHLD);
		}
	}
	//end crazy_liang 2013.3.15

 	if (buf)
		free(buf);
	if (fifo)
		fclose(fifo);

	unlink(upload_fifo);

	return ret;
}
#endif // CONFIG_HTTPD_LTE_UPGRADE

#ifdef CONFIG_HTTPD_PLC_UPGRADE
// PLC firmware upload and upgrade from remote server or socket stream */
int sys_plc_upgrade(struct request_rec *r, FILE *stream, int *total)
{
	char imglen[10];
	char *write_argv[] = { "fw_upgrade",  imglen,  NULL };

	pid_t pid;
	key_t key = PLC_FW_SHARED_MEMORY_KEY;
	char *shmbuf = NULL;
	char *buf = NULL;
	int mid;
	int count, ret = 0;
	long flags = -1;
	int size = BUFSIZ;
	int offset = 0;
	int fd;
	//crazy_liang 2013.3.15: Fix waitpid return -1 and cannot get status issue.
	struct sigaction new_sa;
	struct sigaction old_sa;
	int ret_sa = 0;
	char cmd[64]={0};
	
	memset(&new_sa, 0x00, sizeof(struct sigaction));
	memset(&old_sa, 0x00, sizeof(struct sigaction));

	/*
	 * After exit(), the exit status must be transmitted to
	 * the parent process. There are three cases. If the
	 * parent has set SA_NOCLDWAIT, or has set the SIGCHLD
	 * handler to SIG_IGN, the status is discarded...
	 */
	//Here need use waitpid to get the status of child process,
	//set SIGCHLD to SIG_DFL and restore it later.
	new_sa.sa_handler = SIG_DFL;
	sigemptyset(&new_sa.sa_mask);
	new_sa.sa_flags = 0;
	//end crazy_liang 2013.3.15

	/* system is under upgrading or configuration is updating, blink power led */
	//system("echo 1 > /proc/sys/blink_diag_led");

	//crazy_liang 2013.3.15: Try to backup signal handler of SIGCHLD and set it to SIG_DFL.
	if((ret_sa = sigaction(SIGCHLD, &new_sa, &old_sa)) != 0)
	{
		cprintf("Failed to backup signal handler for SIGCHLD[%d]\n", SIGCHLD);
	}
	//end crazy_liang 2013.3.15
	
	// free the cache
	system("sync");
	system("echo 3 > /proc/sys/vm/drop_caches");

	/* Velmurugan 05142015> Fixed shmget failed when firmware size is more than 32M */
	sprintf(cmd, "echo %lu > /proc/sys/kernel/shmmax", PLC_IMAGE_MAXLEN);
	system(cmd);

	
	mid = shmget(key, PLC_IMAGE_MAXLEN, IPC_CREAT|0666);

	if ( mid < 0 )
	{
		printf("\r\nshare memory creating failed!, retval=%d\r\n", mid);
		exit(1);
	}
	else
		printf("\r\nshare memory key=%d successful on creating share memory id=%d\r\n", key, mid);


	shmbuf = shmat(mid, 0, 0);

	if (shmbuf == (char *) -1)
	{
		printf("\r\ncan not get share memory buffer address, retval=0x%08d\r\n", shmbuf);
		exit(1);
	}


	memset(shmbuf, 0x00, PLC_IMAGE_MAXLEN);

	if (r->ishttps == 0) {
		if ((flags = fcntl(fileno(stream), F_GETFL)) < 0 ||
				fcntl(fileno(stream), F_SETFL, flags | O_NONBLOCK) < 0) {
			ret = errno;
			goto err;
		}
	}

	/*
	* The buffer must be at least as big as what the stream file is
	* using so that it can read all the data that has been buffered
	* in the stream file. Otherwise it would be out of sync with fn
	* select specially at the end of the data stream in which case
	* the select tells there is no more data available but there in
	* fact is data buffered in the stream file's buffer. Since no
	* one has changed the default stream file's buffer size, let's
	* use the constant BUFSIZ until someone changes it.
	*/
	if (size < MIN_BUF_SIZE)
		size = MIN_BUF_SIZE;
	if ((buf = malloc(size)) == NULL) {
		ret = ENOMEM;
		goto err;
	}

	/* check the image's len, if ok,set the param imglen, the upgrade tool need it */
	if(total &&  (*total) > 0 && (*total < PLC_IMAGE_MAXLEN))
	{
		memset(imglen, 0x00, sizeof(imglen));
		sprintf(imglen, "%d", *total);
	}
	else
	{
		cprintf("Illegal image size %d.  Image size must not be greater than %d.\n", *total, PLC_IMAGE_MAXLEN);
		ret = -1;
		goto err;
	}

	/* read the image from sock, and write to share memory */
	printf("Upgrading.\n");
	while (total && *total) {
#ifdef CONFIG_HTTPD_HTTPS_SUPPORT
		if (r->ishttps) {

			if (size > *total)
				size=*total;

			if ( (count=as_read(r->ssrv, buf, size)) <= 0) {
				ret = EINVAL;
				goto err;
			}
		}
		else
#endif
		{
			if (waitfor(fileno(stream), 5) <= 0)
				break;

			//crazy_liang 2013.3.12: Do not read the tail here.
			if(size > *total)
			{
				size = *total;
			}
			//end crazy_liang 2013.3.12

			count = safe_fread(buf, 1, size, stream);
			if (!count && (ferror(stream) || feof(stream)))
				break;
		}

		*total -= count;

/*
		//crazy_liang 2013.3.14: Check the return value. If write failed then halt.
		if(safe_fwrite(buf, 1, count, fifo) != count)
		{
			cprintf("Write %s failed! There are %d was not written!\n", upload_fifo, *total);
			ret = -1;
			goto err;
		}
		//end crazy_liang 2013.3.14
*/
		memcpy(shmbuf + offset, buf, count);
		offset += count;

		printf(".");
	}

	shmdt(shmbuf);



	if ((ret = _eval(write_argv, NULL, 0, &pid)) != 0) {
		if (!ret)
			ret = errno;
		goto err;
	}

	/* Wait for write to terminate */
	waitpid(pid, &ret, 0);

	cprintf("upload return: %d\n", ret);
	/* check if mtd_write is ok */
	if (WIFEXITED(ret)) {
		if (WEXITSTATUS(ret) != 0) {
			ret = -1;
		}
	}

	printf("upgrade done\n");

	if (r->ishttps == 0) {
		/* Reset nonblock on the socket */
		if (fcntl(fileno(stream), F_SETFL, flags) < 0) {
			ret = errno;
			goto err;
		}
	}

 err:
	//crazy_liang 2013.3.15: Fix waitpid return -1 and cannot get status issue.
	if(ret_sa == 0)
	{
		//Try to restore signal handler of SIGCHLD.
		if(sigaction(SIGCHLD, &old_sa, NULL) != 0)
		{
			cprintf("Failed to restore signal handler for SIGCHLD[%d]\n", SIGCHLD);
		}
	}
	//end crazy_liang 2013.3.15

 	if (buf)
		free(buf);
	if (mid)
		shmctl(mid, IPC_RMID, 0);

	return ret;
}
#endif // CONFIG_HTTPD_PLC_UPGRADE

#ifdef CONFIG_HTTPD_MTK_UPGRADE
int sys_mtk_upgrade(struct request_rec *r, FILE *stream, int *total, int b_rom)
{
	char upload_fifo[] = "/tmp/uploadXXXXXX";
	FILE *fifo = NULL;
	char *write_argv[] = { "mtk_fw_upgrade", upload_fifo, ((b_rom == 1) ? "Bootloader" : "Kernel"), NULL };
	pid_t pid;
	char *buf = NULL;
	int count, ret = 0;
	long flags = -1;
	int size = BUFSIZ;
	int fd;
	char cmd_buf[128];

	//crazy_liang 2013.3.15: Fix waitpid return -1 and cannot get status issue.
	struct sigaction new_sa;
	struct sigaction old_sa;
	int ret_sa = 0;

	memset(&new_sa, 0x00, sizeof(struct sigaction));
	memset(&old_sa, 0x00, sizeof(struct sigaction));

	/*
	 * After exit(), the exit status must be transmitted to 
	 * the parent process. There are three cases. If the 
	 * parent has set SA_NOCLDWAIT, or has set the SIGCHLD 
	 * handler to SIG_IGN, the status is discarded...
	 */
	//Here need use waitpid to get the status of child process, 
	//set SIGCHLD to SIG_DFL and restore it later.
	new_sa.sa_handler = SIG_DFL;
	sigemptyset(&new_sa.sa_mask);
	new_sa.sa_flags = 0;
	//end crazy_liang 2013.3.15

	system("arc_led power blink");

	//crazy_liang 2013.3.15: Try to backup signal handler of SIGCHLD and set it to SIG_DFL.
	if((ret_sa = sigaction(SIGCHLD, &new_sa, &old_sa)) != 0)
	{
		cprintf("Failed to backup signal handler for SIGCHLD[%d]\n", SIGCHLD);
	}
	//end crazy_liang 2013.3.15

	/* temporary filename */
	if ((fd = mkstemp(upload_fifo)) < 0) {
		goto err;
	}
	unlink(upload_fifo);
	close(fd);

	if (!(fifo = fopen(upload_fifo, "w"))) {
		goto err;
	}

	if (r->ishttps == 0) {
		if ((flags = fcntl(fileno(stream), F_GETFL)) < 0 ||
				fcntl(fileno(stream), F_SETFL, flags | O_NONBLOCK) < 0) {
			ret = errno;
			goto err;
		}
	}

	/*
	* The buffer must be at least as big as what the stream file is
	* using so that it can read all the data that has been buffered
	* in the stream file. Otherwise it would be out of sync with fn
	* select specially at the end of the data stream in which case
	* the select tells there is no more data available but there in
	* fact is data buffered in the stream file's buffer. Since no
	* one has changed the default stream file's buffer size, let's
	* use the constant BUFSIZ until someone changes it.
	*/
	if (size < MIN_BUF_SIZE)
		size = MIN_BUF_SIZE;
	if ((buf = malloc(size)) == NULL) {
		ret = ENOMEM;
		goto err;
	}

	/* Pipe the rest to the FIFO */
	printf("Upgrading.\n");
	while (total && *total) {
#ifdef CONFIG_HTTPD_HTTPS_SUPPORT
		if (r->ishttps) {

			if (size > *total) 
				size=*total;

			if ( (count=as_read(r->ssrv, buf, size)) <= 0) {
				ret = EINVAL;
				goto err;
			}
		}
		else
#endif
		{
			if (waitfor(fileno(stream), 5) <= 0)
				break;

			//crazy_liang 2013.3.12: Do not read the tail here. 
			if(size > *total)
			{
				size = *total;
			}
			//end crazy_liang 2013.3.12

			count = safe_fread(buf, 1, size, stream);
			if (!count && (ferror(stream) || feof(stream)))
				break;
		}

		*total -= count;

		//crazy_liang 2013.3.14: Check the return value. If write failed then halt.
		if(safe_fwrite(buf, 1, count, fifo) != count)
		{
			cprintf("Write %s failed! There are %d was not written!\n", upload_fifo, *total);
			ret = -1;
			goto err;
		}
		//end crazy_liang 2013.3.14

		printf(".");
	}
	fclose(fifo);
	fifo = NULL;

	if ((ret = _eval(write_argv, NULL, 0, &pid)) != 0) {
		if (!ret)
			ret = errno;
		goto err;
	}		
	
	/* Wait for write to terminate */
	waitpid(pid, &ret, 0);
	
	cprintf("upload return: %d\n", ret);
	/* check if mtd_write is ok */
	if (WIFEXITED(ret)) {
		if (WEXITSTATUS(ret) != 0) {
			ret = -1;
		}
	}

	printf("done\n");

	if (r->ishttps == 0) {
		/* Reset nonblock on the socket */
		if (fcntl(fileno(stream), F_SETFL, flags) < 0) {
			ret = errno;
			goto err;
		}
	}

	goto ok;

err:
	system("arc_led power on");

ok:
	//crazy_liang 2013.3.15: Fix waitpid return -1 and cannot get status issue.
	if(ret_sa == 0)
	{
		//Try to restore signal handler of SIGCHLD.
		if(sigaction(SIGCHLD, &old_sa, NULL) != 0)
		{
			cprintf("Failed to restore signal handler for SIGCHLD[%d]\n", SIGCHLD);
		}
	}
	//end crazy_liang 2013.3.15

 	if (buf)
		free(buf);
	if (fifo)
		fclose(fifo);

	unlink(upload_fifo);

	return ret;
}
#endif // CONFIG_HTTPD_PLC_UPGRADE

#define IH_MAGIC	0x27051956	/* Image Magic Number		*/
#define IH_NMLEN	(32-4)		/* Image Name Length		*/

struct ap_list_info{
	char version[64];
	char ip[32];
	char mac[32];
	int need_fwupdate;
	int need_tftp;
	int tftp_per;
};

typedef struct image_header {
	unsigned int	ih_magic;	/* Image Header Magic Number	*/
	unsigned int	ih_hcrc;	/* Image Header CRC Checksum	*/
	unsigned int	ih_time;	/* Image Creation Timestamp	*/
	unsigned int	ih_size;	/* Image Data Size		*/
	unsigned int	ih_load;	/* Data	 Load  Address		*/
	unsigned int	ih_ep;		/* Entry Point Address		*/
	unsigned int	ih_dcrc;	/* Image Data CRC Checksum	*/
	unsigned char		ih_os;		/* Operating System		*/
	unsigned char		ih_arch;	/* CPU architecture		*/
	unsigned char		ih_type;	/* Image Type			*/
	unsigned char		ih_comp;	/* Compression Type		*/
	unsigned char		ih_name[IH_NMLEN];	/* Image Name		*/
	unsigned int	ih_ksz;		/* Kernel Part Size		*/
} image_header_t;

struct ap_list_info g_aplist_info[8];
int temp_upgrade=0;
int start_upload_fw=2;

int get_tftp_per(int id)
{
	if(g_aplist_info[id].need_tftp) // //strlen(g_aplist_info[id].mac
	{			
		return g_aplist_info[id].tftp_per;	//g_aplist_info[id].tftp_per
	}
	else 
		return 100;
}

int get_need_fwupdate(int id)
{
	return g_aplist_info[id].need_fwupdate;
}

int get_start_upload_fw(void)
{
	return start_upload_fw;
}

void fw_head_encry(char *buf, char *key, int len)
{
	int i,j;
	if(buf == NULL || key == NULL)
		return;
	for(i = 0, j = 0; i < len; i++, j++)
	{
		if(j >= strlen(key))
			j = 0;
		cprintf("%02X-", buf[i]);
		if((buf[i] != 0) && (buf[i] != key[j]))
			buf[i] = buf[i] ^ key[j];
		cprintf("%02X-%02X\n", key[j], buf[i]);
	}
	cprintf("\n");
}	

int sys_owl_upgrade_autofw(struct request_rec *r, int b_rom, int update_time)
{
	char jasonbuf[512];
	char buf_cmd[1024];
	char recvline[10240]={0};  //same size  as owl
	char autofw_url[256] = {0};
	int tid;

	cprintf("%s[%d]update_time=%d\n", __func__, __LINE__, update_time);
	start_upload_fw = 1;

	if(update_time != -1)
		update_time *= 3600;
	
	unlink("/tmp/fwupgrade_err.txt");
	if((tid = get_tid()) != MID_FAIL)
	{
		mapi_ccfg_get_str(tid, "ARC_SYS_AUTOFW_URL", autofw_url, sizeof(autofw_url));
	}
	FW_OPT_IN FwAutoOptIn = OPT_IN_NO;
	FW_CHECK_DEVICE FwCheckDevice = CHECK_DEVICE_WEB;
	FW_CHECK_TYPE FwCheckType = CHECK_TYPE_MANUAL;
	sprintf(jasonbuf,"{\"url\":\"%s\",\"flag\":\"%d\",\"time\":\"%d\"}", autofw_url, URL_FLAG(FwAutoOptIn, FwCheckDevice, FwCheckType), update_time);
//	cprintf("jasonbuf:%s\n", jasonbuf);
	sprintf(buf_cmd,"POST /tools/FW_upgrade HTTP/1.1\r\nContent-Type:application/json\r\nContent-Length:%d\r\nConnection:Keep-Alive\r\n\r\n%s",strlen(jasonbuf),jasonbuf);
	start_upload_fw = 0;
	get_message_from_owl(buf_cmd, recvline , 10240);
	answer(r, DOCUMENT_FOLLOWS, "");
	return 1;
}


int sys_owl_upgrade(struct request_rec *r, FILE *stream, int *total, int b_rom, int resetFlag)
{
	char upload_fifo[] = "/tmp/uploadXXXXXX";
	char upload_fifo_tmp[] = "/tmp/uploadXXXXXX";
	char upload_fifo_backup[32];
	FILE *fifo = NULL, *fifo_w = NULL;
	char *write_argv[] = { "mtk_fw_upgrade", upload_fifo, ((b_rom == 1) ? "Bootloader" : "Kernel"), NULL };
	pid_t pid;
	char *buf = NULL;
	int count, ret = 0;
	long flags = -1;
	int size = BUFSIZ;
	int fd;
	char cmd_buf[128];
	int tid;
	char ver[128] = {0}, rev[32] = {0}, mode[32] = {0}, lan_ip[32] = {0}, fw_ver[128]={0};
	char cur_ver[128] = {0};
	int read_times=0, read_pad=0, read_count=0,need_check_ver=0, mode_len=0;
	int i=0, file_len=0;
	char recvline[10240]={0};  //same size  as owl
	char buf_cmd[1024];
	int num=0, update_num=0, normal_update= 0;
	char jasonbuf[512];
	
	//crazy_liang 2013.3.15: Fix waitpid return -1 and cannot get status issue.
	struct sigaction new_sa;
	struct sigaction old_sa;
	int ret_sa = 0;

	unlink("/tmp/fw_dl_state.txt");
	unlink("/tmp/fwupgrade_err.txt");
	start_upload_fw = 1;
	memset(&new_sa, 0x00, sizeof(struct sigaction));
	memset(&old_sa, 0x00, sizeof(struct sigaction));
	memset(upload_fifo_backup, 0, 32);
	memset(fw_ver, 0, 128);
	memset(&g_aplist_info, 0, sizeof(struct ap_list_info)*8);
	/*
	 * After exit(), the exit status must be transmitted to 
	 * the parent process. There are three cases. If the 
	 * parent has set SA_NOCLDWAIT, or has set the SIGCHLD 
	 * handler to SIG_IGN, the status is discarded...
	 */
	//Here need use waitpid to get the status of child process, 
	//set SIGCHLD to SIG_DFL and restore it later.
	new_sa.sa_handler = SIG_DFL;
	sigemptyset(&new_sa.sa_mask);
	new_sa.sa_flags = 0;
	//end crazy_liang 2013.3.15

	//system("arc_led power blink");    // need move to owl
	temp_upgrade = 1;
	//crazy_liang 2013.3.15: Try to backup signal handler of SIGCHLD and set it to SIG_DFL.
	if(b_rom == 1)
		if((ret_sa = sigaction(SIGCHLD, &new_sa, &old_sa)) != 0)
		{
			cprintf("Failed to backup signal handler for SIGCHLD[%d]\n", SIGCHLD);
		}
	//end crazy_liang 2013.3.15

	/* temporary filename */
	if ((fd = mkstemp(upload_fifo)) < 0) {
		goto err;
	}
	unlink(upload_fifo);
	close(fd);

	if (!(fifo = fopen(upload_fifo, "w"))) {
		goto err;
	}

	if (r->ishttps == 0) {
		if ((flags = fcntl(fileno(stream), F_GETFL)) < 0 ||
				fcntl(fileno(stream), F_SETFL, flags | O_NONBLOCK) < 0) {
			ret = errno;
			goto err;
		}
	}

	/*
	* The buffer must be at least as big as what the stream file is
	* using so that it can read all the data that has been buffered
	* in the stream file. Otherwise it would be out of sync with fn
	* select specially at the end of the data stream in which case
	* the select tells there is no more data available but there in
	* fact is data buffered in the stream file's buffer. Since no
	* one has changed the default stream file's buffer size, let's
	* use the constant BUFSIZ until someone changes it.
	*/
	if (size < MIN_BUF_SIZE)
		size = MIN_BUF_SIZE;
	if ((buf = malloc(size)) == NULL) {
		ret = ENOMEM;
		goto err;
	}
	if(b_rom != 1)
	{		
		if((tid = get_tid()) != MID_FAIL)
		{
			mapi_ccfg_get_str(tid, "ARC_SYS_ModelName", mode, sizeof(mode));
			mapi_ccfg_get_str(tid, "ARC_SYS_FWVersion", ver, sizeof(ver));
			mapi_ccfg_get_str(tid, "ARC_SYS_FWSubVersion", rev, sizeof(rev));
#ifdef WE410443_6DX
			sprintf(cur_ver, "%s_%s.%s", mode, ver, rev);
#else
			sprintf(cur_ver, "%s_%s_%s", mode, ver, rev);
#endif
			mode_len = strlen(mode);
			need_check_ver = 1;
		}
		if(total && *total)
		{
			if(size > *total)
			{
				read_times = 1;
				read_pad = *total;
			}
			else
			{
				read_times = *total / size;
				read_pad = *total % size;
			}
		}
	}
	/* Pipe the rest to the FIFO */
	printf("Upgrading.\n");
	while (total && *total) {
#ifdef CONFIG_HTTPD_HTTPS_SUPPORT
		if (r->ishttps) {

			if (size > *total) 
				size=*total;

			if ( (count=as_read(r->ssrv, buf, size)) <= 0) {
				ret = EINVAL;
				goto err;
			}
		}
		else
#endif
		{
			if (waitfor(fileno(stream), 5) <= 0)
				break;

			//crazy_liang 2013.3.12: Do not read the tail here. 
			if(size > *total)
			{
				size = *total;
			}
			//end crazy_liang 2013.3.12

			count = safe_fread(buf, 1, size, stream);
			if (!count && (ferror(stream) || feof(stream)))
				break;
		}

		*total -= count;

		//crazy_liang 2013.3.14: Check the return value. If write failed then halt.
		if(safe_fwrite(buf, 1, count, fifo) != count)
		{
			cprintf("Write %s failed! There are %d was not written!\n", upload_fifo, *total);
#ifdef SYSLOG_ENHANCED
			SetSystemLogFmt(LOG_TYPE_APP, LOG_LEVEL_TYPE_ERROR, LOG_APP_MESSAGE_TYPE_http, "HTTP-Transfer file failed! There are %d was not written!", *total);
#endif

			ret = -1;
			goto err;
		}
		//end crazy_liang 2013.3.14

		printf(".");
	}
	fclose(fifo);
	fifo = NULL;
	if(need_check_ver == 1)
	{
		image_header_t header;
		int header_encry=1;
		
		answer(r, DOCUMENT_FOLLOWS, "");
		
		FW_OPT_IN FwAutoOptIn = OPT_IN_NO;
		FW_CHECK_DEVICE FwCheckDevice = CHECK_DEVICE_WEB;
		FW_CHECK_TYPE FwCheckType = CHECK_TYPE_MANUAL;		
		sprintf(jasonbuf,"{\"url\":\"%s\",\"flag\":\"%d\",\"time\":\"-1\"}", upload_fifo, URL_FLAG(FwAutoOptIn, FwCheckDevice, FwCheckType));
		cprintf("jasonbuf:%s\n", jasonbuf);
		sprintf(buf_cmd,"POST /tools/FW_upgrade HTTP/1.1\r\nContent-Type:application/json\r\nContent-Length:%d\r\nConnection:Keep-Alive\r\n\r\n%s",strlen(jasonbuf),jasonbuf);
		start_upload_fw = 0;
		if(get_message_from_owl(buf_cmd, recvline , 10240) >= 0) 
		{	
			SetSystemLog(LOG_TYPE_APP, LOG_LEVEL_TYPE_INFO, LOG_APP_MESSAGE_TYPE_http, "Firmware Upgrade - httpd POST FW_upgrade to OWL SUCCESS.");
			goto ok;
		}
		else
		{
			SetSystemLog(LOG_TYPE_APP, LOG_LEVEL_TYPE_INFO, LOG_APP_MESSAGE_TYPE_http, "Firmware Upgrade - httpd POST FW_upgrade to OWL FAIL");
			if ((fd = mkstemp(upload_fifo_tmp)) >= 0) {
				unlink(upload_fifo_tmp);
				close(fd);
				if (fifo = fopen(upload_fifo, "r")) 
				{
					count = safe_fread(buf, 1, sizeof(image_header_t), fifo);
					memcpy (&header, buf, sizeof(image_header_t));
					if (ntohl(header.ih_magic) == IH_MAGIC) 
						header_encry = 0;
					fclose(fifo);
				}
				if (fifo = fopen(upload_fifo, "r")) 
				{	
					fseek(fifo, -64L, SEEK_END);
					file_len = ftell(fifo); 
					safe_fread(buf, 1, 64, fifo);
					if(header_encry)
						fw_head_encry(buf, FW_HEAD_ENCRPTION_KEY, 64);
					if(memcmp(buf, mode, mode_len) == 0)
					{
						if(buf[mode_len] == '_')
						{
							while(buf[i] != 0)
							{
								fw_ver[i] = buf[ i];
								i++;
								if(i > 62)
									break;
							}
							fw_ver[strlen(cur_ver)] = 0;
							mapi_ccfg_set_str(tid, "ARC_UPGRADE_FW_VER", fw_ver);
							//cprintf("fw_ver:%s\n", fw_ver);
							fseek(fifo, 0, SEEK_SET);
							if (fifo_w = fopen(upload_fifo_tmp, "w")) 
							{
								int head=1;
								while (file_len) {
									if(file_len < size)
										size = file_len;
									count = safe_fread(buf, 1, size, fifo);
									if (!count && (ferror(fifo) || feof(fifo)))
										break;
									if(head == 1 && header_encry)
										fw_head_encry(buf, FW_HEAD_ENCRPTION_KEY, 64);
									head = 0;
									file_len -= count;
									safe_fwrite(buf, 1, count, fifo_w);
								}
								fclose(fifo_w);
								fifo_w = NULL;
								strcpy(upload_fifo_backup, upload_fifo);
								strcpy(upload_fifo, upload_fifo_tmp);
								//cprintf("upload_fifo_tmp:%s, upload_fifo:%s\n", upload_fifo_tmp, upload_fifo);
							}
						}
					}
					fclose(fifo);
					fifo = NULL;
				}
		}

		if(strlen(fw_ver) == 0)
		{	
			ret = -1;
			cprintf("Firmware tail error.\n");
			goto err;
		}
		normal_update = 1;
		}		
	}	
	else
		normal_update = 1;
	if(normal_update == 1)
	{
		SetSystemLog(LOG_TYPE_APP, LOG_LEVEL_TYPE_INFO, LOG_APP_MESSAGE_TYPE_http, "Firmware Upgrade - Local upgrade.");
		start_upload_fw = 0;
		if ((ret = _eval(write_argv, NULL, 0, &pid)) != 0) {
			if (!ret)
				ret = errno;
			goto err;
		}		
		
		/* Wait for write to terminate */
		waitpid(pid, &ret, 0);
		
		cprintf("upload return: %d\n", ret);
		/* check if mtd_write is ok */
		if (WIFEXITED(ret)) {
			if (WEXITSTATUS(ret) != 0) {
				ret = -1;
			}
		}
#ifdef SYSLOG_ENHANCED
	if(ret == 0)
	{
		char *curver=NULL;
		char *newver=NULL;
		curver = strchr(ver, '_');
		newver = strchr(fw_ver, '_');
		SetSystemLogFmt(LOG_TYPE_APP, LOG_LEVEL_TYPE_INFO, LOG_APP_MESSAGE_TYPE_http, "HTTP-Firmware Upgrade Success from %s to %s by IP %s.", ver+1, newver+1, r->remote_ipstr);
	}
#endif
	}
	printf("done\n");

	if (r->ishttps == 0) {
		/* Reset nonblock on the socket */
		if (fcntl(fileno(stream), F_SETFL, flags) < 0) {
			ret = errno;
			goto err;
		}
	}

	goto ok;

err:
	system("arc_led power on");
	start_upload_fw = 0;
ok:
	//crazy_liang 2013.3.15: Fix waitpid return -1 and cannot get status issue.
	if(b_rom == 1)
		if(ret_sa == 0)
		{
			//Try to restore signal handler of SIGCHLD.
			if(sigaction(SIGCHLD, &old_sa, NULL) != 0)
			{
				cprintf("Failed to restore signal handler for SIGCHLD[%d]\n", SIGCHLD);
			}
		}
	//end crazy_liang 2013.3.15

 	if (buf)
		free(buf);
	if (fifo)
		fclose(fifo);
	
	if(normal_update == 1)
	{
#ifdef SYSLOG_ENHANCED
		system("arc_syslogc save");
#endif
		mng_action("reboot", "");
	}
	
	unlink(upload_fifo_backup);
	return ret;
}
