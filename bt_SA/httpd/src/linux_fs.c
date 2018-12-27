//============================================================================
// Unix-like File System specific routines
//============================================================================
#include <stdio.h>
#include "httpd.h"
#include <sys/types.h>
#include <sys/stat.h>

/*
 * Name: void FSchdir_file()	- change working directory according to the
 *				  file's path
 * Arguments: char *file	- file name including the path to be switched
 * Return: None
 */
void FSchdir_file(struct request_rec *r, char *file) 
{
	int i;
	int len;

	if (r == NULL || file == NULL) 
	{
		ht_dbg("Parameter is NULL\n");
		return;
	}

	/* Get the last '/' position. */
	i = rind(file, '/');
	if (i == -1) /* '/' not found */
		return;

	file[i] = '\0';
	len = snprintf(r->pwd, sizeof(r->pwd)-1, "%s", file);
	r->pwd[len] = '/';
	file[i] = '/';
}

/*
 * Name: void FSchdir()		- change working directory, if dir == NULL or
 *				  null string (""), release the allocated
 *				  resource.
 * Arguments: char *dir;
 * Return: None
 */
void FSchdir(struct request_rec *r, char *dir) 
{
	if (r == NULL || dir == NULL) 
	{
		ht_dbg("Parameter is NULL\n");
		return;
	}

	snprintf(r->pwd, sizeof(r->pwd), "%s", dir);
	return;
}

/*
 * Name: char *FSgetcwd() - get current working directory of current task.
 * Arguments: none
 *
 * Return: current working directory's path or
 *	   null string if no working directory be set before.
 */
char *FSgetcwd(struct request_rec *r) 
{
	if (r==NULL) 
		return NULL_STR;

	return r->pwd;
}

/*
 * Name: void proc_dir() - make up an absolute path (aka. staring with '/') 
 *                         if the path is a relative path.
 * Arguments: char *filespec - It will point to the absolute path of the requested file.
 *            char *path - It is the file path which may point to an ablsoute path
 *                         (which the browser requests directly) or a relative path
 *                         (asp nesting).
 * Return: None
 */
/*
  jerry_deng 2012.12.12 Belkin Bug ARCADYAN-2245
  Bug description: Webpage load time need improvements
  Solution:  As file_contain_ssi() will call this function, make proc_dir() be not-static function().
*/
void proc_dir(struct request_rec *r, char *filespec, char *path)
/* end  jerry_deng 2012.12.12 Belkin Bug ARCADYAN-2245 */
{
	int fn_lens;

	if (r == NULL || filespec == NULL || path == NULL)
	{
		ht_dbg("Parameter is NULL\n");
		return;
	}

#ifdef FS_DBG
	ht_dbg("proc_dir(): path=%s\n", path);
#endif

	*filespec = '\0';
	if (*path != '/')  /* "path" is a relative path */ 
	{ 
		char *cwd;

		cwd = FSgetcwd(r);
		if (cwd[0] != '\0')
			strcpy_dir(filespec, cwd); 
		else {
			log_error("No working directory specified");
			answer(r, SERVER_ERROR, "No working directory specified");
			return;
		}
	} 

	fn_lens = strlen(filespec);	// fix the over-length pathname's issue.
	strncat(filespec, path, (FNAME_SIZE - fn_lens - 1));
}

void *Linux_FSopen(struct request_rec *r, char *fname)
{
	FILE  *fp;
	char filespec[FNAME_SIZE];

	if (r == NULL || fname == NULL)
	{
		ht_dbg(" Parameter is NULL\n");
		return NULL;
	}

	memset(filespec, 0, sizeof(filespec));
	proc_dir(r, filespec, fname);

	LOCK_FS();
	fp = fopen(filespec, "rb");
	if (fp == NULL)
		ht_dbg("Failed to open %s. Reason: %s\n", filespec, strerror(errno));
	UNLOCK_FS();

	return (void *)fp;
}

int Linux_FSclose(void *fp)
{
	int ret_code;

	if(fp == NULL)
	{
		ht_dbg(" Parameter is NULL\n");
		return -1;
	}

	LOCK_FS();
	ret_code = fclose((FILE *)fp);
	if (ret_code != 0)
		ht_dbg("Failed to close. Reason: %s\n", strerror(errno));
	UNLOCK_FS();

	return ret_code;
}

int Linux_FSread(char *buf, unsigned size, void *fp)
{
	int nbytes;

	if (buf == NULL || fp == NULL) 
	{
		ht_dbg("Parameter is NULL\n");
		return 0;
	}

	LOCK_FS();
	nbytes = fread(buf, 1, size, (FILE *)fp);
	if (ferror(fp))
		ht_dbg("Read error.\n");
	else if (feof(fp))
		ht_dbg("stream met EOF\n");
	UNLOCK_FS();

	return nbytes;
}

int Linux_FSgetc(void *fp) {
	int err_code;

	if (fp == NULL)
	{
		ht_dbg("Parameter is NULL\n");
		return -1;
	}

	LOCK_FS();
	err_code = fgetc((FILE *)fp);
	if (ferror(fp))
		ht_dbg("Read error.\n");
	else if (feof(fp))
		ht_dbg("stream met EOF\n");
	UNLOCK_FS();

	return err_code;
}

int Linux_FSstat(struct request_rec *r, char *path, FILE_INFO_t *finfo)
{
	char filespec[FNAME_SIZE];
	int err_code;
	struct stat info;
	struct tm *pTM;

	if (r == NULL || path == NULL || finfo == NULL)
	{
		ht_dbg("Parameter is NULL\n");
		return -1;
	}

	memset(filespec, 0, sizeof(filespec));
	proc_dir(r, filespec, path);
	LOCK_FS();
	err_code = stat(filespec, &info);
	if (err_code != 0)
		ht_dbg("stat() error. Reason: %s\n", strerror(errno));
	UNLOCK_FS();
	if (err_code != 0) return err_code;

	LOCK_FS();
	pTM = localtime(&info.st_mtime);
	if (pTM == NULL)
	{
		ht_dbg("transform time error.\n");
		UNLOCK_FS();
		return -1;
	}

	finfo->ftime.second = pTM->tm_sec;
	finfo->ftime.minute = pTM->tm_min;
	finfo->ftime.hour   = pTM->tm_hour;
	finfo->ftime.day    = pTM->tm_mday;
	finfo->ftime.month  = pTM->tm_mon;
	finfo->ftime.year   = pTM->tm_year;
	finfo->size 	      = info.st_size;
	finfo->is_dir	      = (info.st_mode & S_IFDIR)>0 ? 1:0;
	UNLOCK_FS();

	return 0;
}
