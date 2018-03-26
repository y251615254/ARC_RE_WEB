/*
 * httpd.c: simple http daemon for answering WWW file requests
 *
 *
 * Rob McCool 3/21/93
 *
 */

/*
 * Ported to Multitask! real-time O.S by
 *
 * C.H. Lin     3/11/1998
 */

#include "httpd.h"      /* http daemon header file */
#include "typedefs.h" 

/* Terry 20141027, for o2 redirect URL */
/* To access lib-arc-cfg */
#ifdef VRV9518SWAC33
#include "arc_cfg_api.h"
#include "arc_lan_table.h"
#include "arc_proj_table.h"
#endif

//#ifdef SYSLOG_ENHANCED
//#include "arc_syslog.h"
//#endif

//tlhhh
#ifndef _80X86
  #define HT_MAIN_STKSIZE	18000
  #define HT_MAIN_SSL_STKSIZE 20000
  #if _UPNPVER == 2
    #define HT_CHILD_STKSIZE 28000 //20000
  #else
    #define HT_CHILD_STKSIZE 18000
  #endif
  #define HT_TIMER_STKSIZE 10000
  #define fAR
#else
  #include <dos.h>
  #define HT_MAIN_STKSIZE	1600
  #define HT_CHILD_STKSIZE 3000
  #define HT_TIMER_STKSIZE 1000
#endif

#include "pthread.h" // added by chlin 2006, Oct, 24th

#define THREAD_COUNT	MAX_REQ
static workthread_t **thread_pool = NULL;
static sem_t thr_sem;
int http_enabled;
int ssl_enabled; 
int http_userremoteenable;
#ifdef VRV9518SWAC33
int http_serviceremoteenable;
#endif
int httpd_reload = 0;
int httpd_main_exit = 0;
unsigned long sys_clock_res=10000000;
unsigned long sys_clock_hz=CLOCKHZ;
pthread_mutex_t rand_mutex_c;


extern void get_local_sock(struct request_rec *r);
extern void get_remote_sock(struct request_rec *r);

/*
 * Global variables used in httpd.c
 */
int     ht_prior;
char	NULL_STR[] = "";


/*======= support for multiport listening =======*/
#define MAX_LISTEN_PORTS	10
static struct listen_port {
	u_long slot;                   /* task's slot No.(=ID) */
	int	sd;
	union{
		struct in_addr addr4;
		struct in6_addr addr6;
	} if_addr;
	u_short	port;
	struct listen_port *next;
} listen_pool[MAX_LISTEN_PORTS];


/* Read any data kept in socket buffer. Maybe block */
int garbage_collect(struct request_rec *r, int sock, int max_size)
{
	fd_set readfds;
	char buf[1024];
	int len = 0;
	int have_pending = 0;
	struct timeval timeouts;
	int ret;

	if (r == NULL)
		return -1;

	while (1) {

		if (max_size > 0 && len >= max_size)
			break;

		have_pending = 0;

#ifdef CONFIG_HTTPD_HTTPS_SUPPORT
		if (r->ishttps) {
			have_pending = as_pending(r->ssrv);
		}
#endif
		if (!have_pending) {

			FD_ZERO(&readfds);

			/* wait for data */
			timeouts.tv_sec = 1; //timeout;
			timeouts.tv_usec = 0;
			FD_SET(sock, &readfds);

			ret = select(sock+1, &readfds, NULL, NULL, &timeouts);

			if (ret < 0)
				return -1; /* socket error */

			if (ret == 0)
				break;	/* timeout means no other data */
		}

#ifdef CONFIG_HTTPD_HTTPS_SUPPORT
		if (r->ishttps) {

			ret = as_read(r->ssrv, buf, sizeof(buf));
			if (ret < 0)
				return -1;

			if (ret == 0)
				break;
		}
		else
#endif
		{
			/* read that many bytes */
			ret = recv(sock, buf, sizeof(buf), MSG_DONTWAIT);
			if (ret < 0)
				return -1;

			if (ret == 0)
				break; // get a FIN from peer side
		}

		len += ret;	
	}

	return len;
}

/*
 * function: get http header from socket
 * parameters:
 *  fd          socket id of current request
 *  buff        a buffer allocated for header
 *  buff_size   the size of this buffer
 *  timeout     time to wait for further incoming data
 * return value:
 *  = -2        buffer under run
 *  = -1        any socket error
 *  = 0         time out
 *  > 0         the length of data received
 */
int ht_get_header(struct request_rec *r, int fd, char *buff, int buff_size, int timeout)
{
	fd_set readfds;
	struct timeval timeouts;
	int size, n, ret;
	char *ptr, *endp = NULL;
	char ending[5] = {0xd, 0xa, 0xd, 0xa, 0};
	int have_pending = 0;

	ptr = buff;
	size = buff_size-1; // modified by chlin, 20070903

	while (endp == NULL && size > 0) { // still do not find "\015\012\015\012"

		have_pending = 0;
#ifdef CONFIG_HTTPD_HTTPS_SUPPORT
		if (r->ishttps) {
			have_pending = as_pending(r->ssrv);
		}
#endif
		if (!have_pending) {
			FD_ZERO(&readfds);
			/* wait for data */
			timeouts.tv_sec = timeout/1000;
			timeouts.tv_usec = (timeout%1000) * 1000;
			FD_SET(fd, &readfds);

			ret = select(fd+1, &readfds, NULL, NULL, &timeouts);
			if (ret == -1) return -1;

			if (ret == 0) { /* timeout */
				ht_dbg("select TIMEOUT\n");
				return 0;
			}
		}

#ifdef CONFIG_HTTPD_HTTPS_SUPPORT
		if (r->ishttps) {
__ssl_read:
#if 0	//fix bug that this code will read too many bytes include some of the content
			ret = as_read(r->ssrv, ptr, size);

			if (ret <= 0) {
				return -1;
			}

			ptr[ret] = '0';  // add ending zero for strstr()

			endp = strstr(buff, ending);
			size -= ret;
			ptr += ret;

			if (endp==NULL && size>0)
				goto __ssl_read;
#endif
			ret = as_read_line(r->ssrv, (unsigned char *)ptr, size);
			if((memcmp(ptr, "\r\n", 2) != 0) && (ret > 0))
			{
				ptr += ret;
				goto __ssl_read;
			}
			else
			{
				r->in_header_lens = ptr - buff + 2;
				return ptr - buff + 2;
			}
		} else
#endif
		{
			/* have a look at the data */
			ret = recv(fd, ptr, size, MSG_PEEK | MSG_DONTWAIT); // modified by chlin, 20070903
			if (ret <= 0) return -1;

			ptr[ret] = '\0'; // add ending zero for strstr()
			endp = strstr(buff, ending);
			if (endp) {
				n = endp - ptr + 4;
			} else
				n = ret;

			/* read that many bytes */
			ret = recv(fd, ptr, n, MSG_DONTWAIT);
			if (ret <= 0) return -1;

			size -= n;
			ptr += n;
		}
	}

	if (endp != NULL) {
		//endp[4] = '\0';
		r->in_header_lens = endp - buff + 4;
		return (buff_size - size -1);
	}

	garbage_collect(r, fd, -1);

	return -2; // data overrun, buffer size not enough
}


int clear_request(struct request_rec *r, int sd) 
{

	if (r == NULL)
		return -1;

	/* initialize request_rec */
	memset(r, 0, sizeof(struct request_rec));

	r->sd = sd;
	r->cgi_slot = 0;
	
	return 0;
}


/*
 * httpd_child():
 *	Httpd child task. If there are multi-request coming simultaneously,
 *	The main task will run lots of child tasks for processing the incoming
 *	request.
 */
void *httpd_child(void *param)
{
	int thread_id = *((int *)param);
	int errcode = 0;
#ifdef CONFIG_HTTPD_Security_Attack_Protect
	int is_attack = 0;
	int attack_index = -1;
#endif	
	workthread_t *cur_thread = thread_pool[thread_id];
	struct	request_rec *r = (struct request_rec *)&cur_thread->r;
	struct th_info *th;
#ifdef CONFIG_HTTPD_HTTPS_SUPPORT
	void *aux_para1;
#endif //	CONFIG_HTTPD_HTTPS_SUPPORT
	int sd;
	struct fd_entry *pFd_entry;  // add for httpd v1.1 new feature.
	//int flags; //no used

	ht_dbg("thread #%d start ...\n", thread_id);
	
	sig_ignore_reload_httpd();
	
	while (1)
	{
		/* wait for a semephore */
		errcode = sem_wait(&cur_thread->sem);
		if (errcode == -1)
		{
			if (errno == EINTR)
				continue;
			ht_dbg("sem_wait err: %d", errno);
			break;
		}

//		if (do_exit)
//			break;

		ht_dbg("thread %d wake up\n", cur_thread->number_id);

		pthread_mutex_lock(&cur_thread->f_lock);
		th = (struct th_info *)cur_thread->data;
		pthread_mutex_unlock(&cur_thread->f_lock);  //release data pointer until we already copy that. tlhhh 2010-8-4.

		sd = (int)th->args_list[0];
		if (sd < 0)
			goto done;

		clear_request(r, sd);

		r->cookie = (unsigned long)cur_thread->pthread_id;
		r->slot = (unsigned long)cur_thread->pthread_id;
		r->index = cur_thread->number_id;

		get_local_sock(r);
		get_remote_sock(r);     /* set the fileds: remote_name, remote_ip, */

#ifdef CONFIG_HTTPD_Security_Attack_Protect
		attack_index = find_entry_in_attack_array(r->remote_ip);
		if ((-1 != attack_index) && (1 == attack_entry.attack_addr[attack_index].is_attacker))
		{
			is_attack = is_block_attack_addr(r->remote_ip);
			if (1 == is_attack)
			{
				cprintf("pid[%d], function[%s], line[%d], -----> is attacker!!!\n", getpid(), __FUNCTION__, __LINE__);
				goto done;
			}
		}
#endif		

#ifdef CONFIG_HTTPD_HTTPS_SUPPORT
		aux_para1 = (void *)th->args_list[1];

		if (aux_para1)
		{
			int err_code;

			r->ssrv = &(cur_thread->ssl_srv);

			r->ishttps = 1;
			r->ssrv->conn_sock = r->sd;

			/* setup recv/send, and hand shake */
			err_code = as_server_open(r->ssrv, DEFAULT_TIMEOUT);
			if (err_code != 0) {
				ht_dbg("as_server_open failed[err_code: %d]\n", err_code);

				goto done;
			}
		}
#endif


		r->in_buf_used = ht_get_header(r, sd, r->in_buf, INBUFSIZE, ht_timeout * 1000);

		
		if (r->in_buf_used <= 0) {

			if (r->in_buf_used == -2)
			{
				log_error("Unable to process request headers");
				answer(r, SERVER_ERROR, "Unable to process request headers");
			}
			else
			{
				/* -1, socket error, 0 means time out */
			}

			goto done;
		}

		/* tlhhh. Set timer for every connection. kill self when time is up */
		tmr_set(r->cookie, ht_timeout, timed_out_handler);

		process_request(r);

		/* clean up timer */
		tmr_cancel(r->cookie);
		so_flush(r);

done:
#ifdef CONFIG_HTTPD_HTTPS_SUPPORT
		if (r->ishttps && r->ssrv) {
			as_server_close(r->ssrv);
		}
		else
#endif
		{
			close(r->sd);
		}

#if 0
		pthread_mutex_lock(&cur_thread->f_lock);
		ht_dbg("cur_thread %d is %s\n", cur_thread->number_id, cur_thread->is_busy ? "BUSY" : "IDLE");
		pthread_mutex_unlock(&cur_thread->f_lock);
#endif

		if (r->fd != (FILE_t)NULL)	/* and close reqular file */
			FSclose(r->fd);

		while (r->fd_backup != NULL) {
			FSclose(r->fd_backup->fd);
			pFd_entry = r->fd_backup->next_fd;
			free(r->fd_backup);
			r->fd_backup = pFd_entry;
		}

		ht_dbg("thread %d work finished\n\n", cur_thread->number_id);

		pthread_mutex_lock(&cur_thread->f_lock);
		cur_thread->is_busy = 0;
		pthread_mutex_unlock(&cur_thread->f_lock);

		errcode = sem_post(&(thr_sem));
		if (errcode == -1) {
			ht_dbg("sem_post %d err: %d\n", cur_thread->number_id, errno);
			break;
		}
	}
	
#ifdef CONFIG_HTTPD_HTTPS_SUPPORT
	if (r->ishttps) {
		as_unload_key_cert();
	}
#endif

	pthread_exit(NULL);
}

int ht_open_socket(struct in_addr if_addr, u_short port) 
{
	int sd, one;
	struct	sockaddr_in sa_server;
	struct in_addr iaddr;
	int tid;

	if ((sd = socket(AF_INET,SOCK_STREAM,0)) == -1) {
		ht_dbg("httpd: could not get a socket: %d\n", errno);
		return -1;
	}

	one = 1;
	if ((setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, (char *)&one, sizeof(one))) == -1) {
		ht_dbg("httpd: could not set socket option: %d\n", errno);
		close(sd);
		return -1;
	}

	if((tid = get_tid()) == MID_FAIL)
	{
		ht_dbg("[%s] Failed to connect to midcore\n", __FUNCTION__);
		close(sd);
		return -1;
	}
	memset((char *) &sa_server, 0, sizeof(sa_server));
	sa_server.sin_family = AF_INET;
	sa_server.sin_port = htons(port);
	if (mapi_ccfg_match_str(tid, "ARC_WAN_0_IP4_Proto", "bridge")) {
		sa_server.sin_addr.s_addr = htonl(INADDR_ANY);
		iaddr.s_addr = htonl(INADDR_ANY);
	}
	else {
		sa_server.sin_addr = if_addr;
		iaddr = if_addr;
	}

	if (bind(sd,(struct sockaddr *) &sa_server,sizeof(sa_server)) == -1) {
		ht_dbg("httpd: could not bind to [%s] port %d, error=[%d][%s]\n", inet_ntoa(iaddr), ntohs(sa_server.sin_port), errno, strerror(errno));
		close(sd);
		return -1;
	}

	ht_dbg("httpd: listen at %s:%d\n", inet_ntoa(if_addr), port);
	return sd;
}

int ht_open_socket6(struct in6_addr if_addr, u_short port, char *dev) 
{
	int sd, one=1;
	char addr6_str[64];
	struct sockaddr_in6 sin6;
	struct ifreq intf;

	if ((sd = socket(AF_INET6,SOCK_STREAM,0)) == -1) {
		ht_dbg("httpd: could not get a socket: %d\n", errno);
		return -1;
	}

	#define IPV6_V6ONLY	26
	if ((setsockopt( sd, IPPROTO_IPV6, IPV6_V6ONLY, (void*) &one, sizeof(one))) == -1) {
		perror( "setsockopt IPV6_V6ONLY" );
		close(sd);
		return -1;
	}

	if ((setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, (char *)&one, sizeof(one))) == -1) {
		ht_dbg("httpd: could not set socket option: %d\n", errno);
		close(sd);
		return -1;
	}

	if (dev){
		ht_dbg("%s will bind to interface %s.", __func__, dev);

		strcpy(intf.ifr_name, dev);
		if (setsockopt(sd, SOL_SOCKET, SO_BINDTODEVICE, (char *)&intf, sizeof(intf)) == -1){
			perror("setsockopt SO_BINDTODEVICE");
			close(sd);
			return -1;
		}
	}

	memset((char *) &sin6, 0, sizeof(sin6));
	sin6.sin6_family = AF_INET6;
	sin6.sin6_flowinfo = 0;
	sin6.sin6_port = htons(port);
	sin6.sin6_addr = if_addr;

	if (bind(sd,(struct sockaddr *) &sin6,sizeof(sin6)) == -1) {
		ht_dbg("httpd: could not bind to %s port %d\n", inet_ntop(AF_INET6, (void*)&if_addr, addr6_str, sizeof(addr6_str)), port);
		close(sd);
		return -1;
	}

	ht_dbg("httpd: listen at %s:%d\n", inet_ntop(AF_INET6, (void*)&if_addr, addr6_str, sizeof(addr6_str)), port);
	return sd;
}

void httpd_main(void *l_port, void *ssl) 
{
	int	csd, sd;
	int	clen;
	struct	sockaddr sa_client;
	int	err_code;
	int	last_errno = 0;
	u_short port_num;
	struct listen_port *lport = (struct listen_port *)l_port;
	int ishttps;
	int i, flag=0;

	if (!ssl)
		return;

	memcpy(&ishttps, ssl, sizeof(int));
	free(ssl);

	port_num = lport->port;
	sd = lport->sd;

	if (listen(sd, THREAD_COUNT) < 0) {
		perror("listen");
		return;
	}

	while(!httpd_main_exit) {

		struct th_info th;

		clen = sizeof(sa_client);
		memset(&sa_client, 0, clen);

		if ((csd = accept(sd, &sa_client, (socklen_t *)&clen)) == -1) {

			if (lport != NULL && lport->sd == -1) {
				ht_dbg("httpd unbind port %d\n", port_num);
				memset(&lport->if_addr, 0, sizeof(lport->if_addr));	// release ListenPort
				return;
			}

			if (last_errno != errno) {
				perror("accept");
			}

			if (errno == EINTR) {
				log_error("Interrupted by others");

				if (lport != NULL) {
					ht_dbg("Terminated at %s:%d\n",	inet_ntoa(lport->if_addr.addr4), lport->port);
					memset(&lport->if_addr, 0, sizeof(lport->if_addr)); // release ListenPort
				}

				close(sd);
				continue;
			}

			last_errno = errno;
			DELAY(CLOCKHZ/10);
			continue;
		}


		if (lport != NULL && lport->sd == -1) {
			ht_dbg("PORT[%d] has been closed\n", port_num);
			memset(&lport->if_addr, 0, sizeof(lport->if_addr)); // release ListenPort
			return;
		}

		last_errno = 0;

		do 
		{
			/* wait for semephore - free thread */
			err_code = sem_wait(&thr_sem);
			if (err_code == -1)
			{
				if (errno == EINTR)
					continue;
				ht_dbg("sem_wait err: %d", errno);
				return;
			}
			flag=0;

			for (i=0; i<THREAD_COUNT; i++)
			{
				pthread_mutex_lock(&(thread_pool[i]->f_lock));

				//if (thread_pool[i]->is_busy == 0 && thread_pool[i]->data == NULL)
				if (thread_pool[i]->is_busy == 0)
				{
					thread_pool[i]->is_busy = 1;

					th.args_num = 2;
					th.args_list[0] = (long)csd;
					if (ishttps)
						th.args_list[1] = (long)(ishttps);
					else
						th.args_list[1] = (long)NULL;
					memset(thread_pool[i]->data, 0, sizeof(thread_pool[i]->data));
					memcpy(thread_pool[i]->data, &th, sizeof(th));

					ht_dbg("Assigned thread %d to process\n", i);

					err_code = sem_post(&(thread_pool[i]->sem));
					if (err_code == -1) {
						ht_dbg("sem_post %d err: %d\n", i, errno);
						pthread_mutex_unlock(&(thread_pool[i]->f_lock));
						return;
					}

					flag=1;
				}

				pthread_mutex_unlock(&(thread_pool[i]->f_lock));

				if (flag)
				{
					break;
				}
			}
		}
		while (flag == 0);

		if (ishttps)
			DELAYMS(20);
	}
}

void init_vendor(void)
{
	if (v_ops->init_hook)
		return v_ops->init_hook(hc);

	return ;
}

int init_thread(void)
{
	int i;
	int ret;

	thread_pool = (workthread_t**) malloc(sizeof(workthread_t*)*THREAD_COUNT);

	ret = sem_init(&(thr_sem), 0, THREAD_COUNT);
	
	for (i=0;i<THREAD_COUNT;i++)
	{
		thread_pool[i] = (workthread_t*) malloc(sizeof(workthread_t));
		thread_pool[i]->number_id = i;
		thread_pool[i]->is_busy = 0;
		memset(thread_pool[i]->data, 0, sizeof(thread_pool[i]->data));

		ret = pthread_mutex_init(&(thread_pool[i]->f_lock),NULL);
		if (ret != 0) {
			ht_dbg("f_lock %d error\n", i);
			return -1;
		}

		ret=sem_init(&(thread_pool[i]->sem), 0, 0);
		if (ret == -1) {
			ht_dbg("sem %d error\n", i);
			return -1;
		}
	}

	return 0;
}

#ifdef CONFIG_HTTPD_HTTPS_SUPPORT
int init_ssl(void)
{
	int i, k, ret;

	if (!ssl_enabled)
		return 0;

	/* guang_zhao 20130529
	 * To fix HNAP MultipleSSL fail, don`t reuse global server's key/cert settings: 
	 * for OPENSSL: means CTX 
	 * for POLARSSL: means static key/cert in lib-arcssl
	 * But lib-arcssl need a protect mechaism(e.g: thread mutex) for encrypt.
	 */
	for (i=0; i<THREAD_COUNT; i++) {
		ret = as_server_init(&thread_pool[i]->ssl_srv);
		if (ret != 0) {
			for(k=0; k<i; k++)
				as_server_deinit(&thread_pool[k]->ssl_srv);		
			as_unload_key_cert();
			ht_dbg("init server failed\n");
			return -1;
		}
		/* initialize and setup rand generater */
		ret = as_load_key_cert(&thread_pool[i]->ssl_srv, KEY_FILE, CERT_FILE);
		if (ret != 0) {
			ht_dbg("init cert failed\n");
			return -1;
		}
	}

	return 0;
}
#endif

/* workaround initialize
*/
static void specific_init()
{
	//make sure previous
	system("killall -9 udpsvd");
	login_info.error = 0;
	login_info.error_count = 0;
	login_info.error_uptime = 0;
	login_info.error_wait_time = 0;
	pthread_mutex_init(&(login_info.lock),NULL);
}

int httpd_init(void)
{
	int ret;
	
	specific_init();
		
	init_global();

	init_vendor();

	init_listener();

	ret = init_thread();
	if (ret < 0) {
		return -1;
	}
#ifdef CONFIG_HTTPD_HTTPS_SUPPORT
	ret = init_ssl();
	if (ret < 0) {
		return -1;
	}
#endif
	
	return 0;
}


/*======= support for multiport listening =======*/
void init_listener(void)
{
	int i;

	for (i=0; i<MAX_LISTEN_PORTS; i++) {
		listen_pool[i].slot = 0;
		listen_pool[i].sd = -1;
		//listen_pool[i].if_addr = 0;
		memset(&listen_pool[i].if_addr, 0, sizeof(listen_pool[i].if_addr));
		listen_pool[i].port = 0;
	}

	return ;
}

/*
 * if_addr: Network order IP address of the specified I/F
 */
int add_listener(struct in_addr if_addr, u_short port, int ishttps) 
{
	int i, sd;
	struct th_info th;
	int *ssl = malloc(sizeof(int));
	
	if (!ssl)
		return 0;

	memcpy(ssl, &ishttps, sizeof(int));

	// check if duplicated I/F & port# setting
	for (i=0; i<MAX_LISTEN_PORTS; i++) {
		if (listen_pool[i].if_addr.addr4.s_addr == if_addr.s_addr &&
			listen_pool[i].port == port) {
			ht_dbg("found duplicated I/F & port# setting.\n");
			return 0;
		}
	}

	for (i=0; i<MAX_LISTEN_PORTS; i++) {
		if (listen_pool[i].sd == -1/* && listen_pool[i].if_addr == 0*/) {
			break;
		}
	}

	if (i == MAX_LISTEN_PORTS) 
		return -1;

	sd = ht_open_socket(if_addr, port);
	if (sd == -1) {
		return -1;
	}

	ENTER_CRITICAL();
	listen_pool[i].sd = sd;
	listen_pool[i].if_addr.addr4 = if_addr;
	listen_pool[i].port = port;
	EXIT_CRITICAL();

	th.prior = ht_prior;
	th.stacksize = HT_MAIN_STKSIZE;
	th.type = TH_STD;
	th.args_num = 2;
	th.args_list[0] = (long)httpd_main;
	th.args_list[1] = (long)(&listen_pool[i]);
	th.args_list[2] = (long)(ssl);

	if (ht_create_thread(&th) != 0) {
		close(listen_pool[i].sd);
		ENTER_CRITICAL();
		listen_pool[i].sd = -1;
		memset(&listen_pool[i].if_addr, 0, sizeof(listen_pool[i].if_addr));
		listen_pool[i].port = 0;
		EXIT_CRITICAL();
		return -1;
	}

	listen_pool[i].slot = th.pthread_id;

	ht_dbg("httpd listen at %s:%d\n", inet_ntoa(if_addr), port);
	//SLEEP(1);
	//add_listener6(if_addr6, port, ishttps);
	return 1;
}

/*
 * if_addr: Network order IP address of the specified I/F
 */
int add_listener6(struct in6_addr if_addr6, u_short port, char*dev, int ishttps) 
{
	int i, sd;
	struct th_info th;
	int *ssl = malloc(sizeof(int));
	char addr6_str[64];

	if (!ssl)
		return 0;

	memcpy(ssl, &ishttps, sizeof(int));

	// check if duplicated I/F & port# setting
	for (i=0; i<MAX_LISTEN_PORTS; i++) {
		if (memcmp(&listen_pool[i].if_addr.addr6, &if_addr6, sizeof(struct in6_addr)) == 0 &&
				listen_pool[i].port == port) {
			ht_dbg("found duplicated I/F & port# setting.\n");
			return 0;
		}
	}

	for (i=0; i<MAX_LISTEN_PORTS; i++) {
		if (listen_pool[i].sd == -1) {
			break;
		}
	}

	if (i == MAX_LISTEN_PORTS) 
		return -1;

	sd = ht_open_socket6(if_addr6, port, dev);
	if (sd == -1) {
		return -1;
	}

	ENTER_CRITICAL();
	listen_pool[i].sd = sd;
	listen_pool[i].if_addr.addr6 = if_addr6;
	listen_pool[i].port = port;
	EXIT_CRITICAL();

	th.prior = ht_prior;
	th.stacksize = HT_MAIN_STKSIZE;
	th.type = TH_STD;
	th.args_num = 2;
	th.args_list[0] = (long)httpd_main;
	th.args_list[1] = (long)(&listen_pool[i]);
	th.args_list[2] = (long)(ssl);

	if (ht_create_thread(&th) != 0) {
		close(listen_pool[i].sd);
		ENTER_CRITICAL();
		listen_pool[i].sd = -1;
		memset(&listen_pool[i].if_addr, 0, sizeof(listen_pool[i].if_addr));
		listen_pool[i].port = 0;
		EXIT_CRITICAL();
		return -1;
	}

	listen_pool[i].slot = th.pthread_id;

	ht_dbg("httpd listen at %s:%d dev:%s\n", inet_ntop(AF_INET6, (void*)&if_addr6, addr6_str, sizeof(addr6_str)), port, dev?dev:"NULL");
	return 1;
}

/*
 * if_addr: Network order IP address of the specified I/F
 */
int remove_listener(struct in_addr if_addr, u_short port) 
{
	int i;

	for (i=0; i<MAX_LISTEN_PORTS; i++) {
		if (listen_pool[i].if_addr.addr4.s_addr == if_addr.s_addr &&
		    listen_pool[i].port == port )
			break;
	}

	if (i == MAX_LISTEN_PORTS) // not found
		return 0;


	ht_dbg("ListenPort: try send signal to %s:%d idx=%d.\n",
			inet_ntoa(listen_pool[i].if_addr.addr4), listen_pool[i].port, i);

	close(listen_pool[i].sd);
	/*
	 * we close the socket fd to let the thread exit automatically,
	 * ie. release resources,not to kill it
	 */
	pthread_kill(listen_pool[i].slot, SIGUSR2);
	pthread_join(listen_pool[i].slot, NULL);

	ENTER_CRITICAL();
	listen_pool[i].sd = -1;
	listen_pool[i].slot = 0;
	memset(&listen_pool[i].if_addr, 0, sizeof(listen_pool[i].if_addr));
	listen_pool[i].port = 0;
	EXIT_CRITICAL();

	return 1;
}

int remove_listener6(struct in6_addr if_addr, u_short port) 
{
	int i;
	char addr6_str[64];

	for (i=0; i<MAX_LISTEN_PORTS; i++) {
		if (memcmp(&listen_pool[i].if_addr.addr6, &if_addr, sizeof(struct in6_addr)) == 0 &&
				listen_pool[i].port == port )
			break;
	}

	if (i == MAX_LISTEN_PORTS) // not found
		return 0;


	ht_dbg("ListenPort: try send signal to %s:%d idx=%d.\n",
			inet_ntop(AF_INET, &listen_pool[i].if_addr.addr6, addr6_str, sizeof(addr6_str)), listen_pool[i].port, i);

	close(listen_pool[i].sd);
	/*
	 * we close the socket fd to let the thread exit automatically,
	 * ie. release resources,not to kill it
	 */
	pthread_kill(listen_pool[i].slot, SIGUSR2);
	pthread_join(listen_pool[i].slot, NULL);

	ENTER_CRITICAL();
	listen_pool[i].sd = -1;
	listen_pool[i].slot = 0;
	memset(&listen_pool[i].if_addr, 0, sizeof(listen_pool[i].if_addr));
	listen_pool[i].port = 0;
	EXIT_CRITICAL();

	return 1;
}

// Unix or Linux related code

void usage(char *bin) 
{
    fprintf(stderr,"Usage: %s [-d directory] [-v]\n", bin);
    fprintf(stderr,"-d directory : specify an alternate web page directory\n");
    exit(1);
}


unsigned long get_sys_time() 
{
	struct timespec tm_buf;
	unsigned long sys_time;

	clock_gettime(CLOCK_MONOTONIC, &tm_buf);
	sys_time = tm_buf.tv_sec * sys_clock_hz + tm_buf.tv_nsec / sys_clock_res;
	return sys_time;
}

int get_wan_address()
{
	int tid;
	char var[64], wan_name[64], wan_proto[64], wan6_proto[64];

	if ((tid = get_tid()) == MID_FAIL) {
		ht_dbg("[%s] Failed to connect to midcore\n", __FUNCTION__);
		return 0;
	}

	mapi_ccfg_get_str(tid, "ARC_WAN_DefaultRouteIdx", var, 64);
	snprintf(wan_name, sizeof(wan_name), "ARC_WAN_%s_IP4_Proto", var);
	mapi_ccfg_get_str(tid, wan_name, wan_proto, sizeof(wan_proto));

	if (strncmp(wan_proto, "pppoe", sizeof(wan_proto)) == 0
		|| strncmp(wan_proto, "pppoa", sizeof(wan_proto)) == 0
		|| strncmp(wan_proto, "pptp", sizeof(wan_proto)) == 0){
		snprintf(wan_name, sizeof(wan_name), "ARC_WAN_%s_PPP_GET_IP4_Addr", var);
		mapi_ccfg_get_str(tid, wan_name, wanip_str, sizeof(wanip_str));
	}
	else{
		snprintf(wan_name, sizeof(wan_name), "ARC_WAN_%s_IP4_Addr", var);
		mapi_ccfg_get_str(tid, wan_name, wanip_str, sizeof(wanip_str));
	}

	ht_dbg("wan ipv4 addr:%s\n", wanip_str);

	snprintf(wan_name, sizeof(wan_name), "ARC_WAN_%s_IP6_Proto", var);
	mapi_ccfg_get_str(tid, wan_name, wan6_proto, sizeof(wan6_proto));
	
	snprintf(wan_name, sizeof(wan_name), "ARC_WAN_%s_IP6_DHCP", var);
	mapi_ccfg_get_str(tid, wan_name, wanip6_en, sizeof(wanip6_en));

	if (strncmp(wan6_proto, "dhcp", 4) == 0 && strncmp(wanip6_en, "on", 2) == 0){
		snprintf(wan_name, sizeof(wan_name), "ARC_WAN_%s_IP6_Addr", var);
		mapi_ccfg_get_str(tid, wan_name, wanip6_str, sizeof(wanip6_str));

		snprintf(wan_name, sizeof(wan_name), "ARC_WAN_%s_Iface", var);
		mapi_ccfg_get_str(tid, wan_name, wanip_dev, sizeof(wanip_dev));
	}

	ht_dbg("wan ipv6 enable:%s, addr:%s, dev:%s", wanip6_en, wanip6_str, wanip_dev);

	return 1;
}

int reload_httpd()
{
	int tid, i, sd;
	int http_port = 0;
	int https_port = 0;
	struct in6_addr addr6;
	struct in_addr addr4;
	int https_userremoteport=0;
#ifdef VRV9518SWAC33
	int https_serviceremoteport=0;
#endif
#ifdef VR7516RW22
	unsigned long lanip = 0;
#endif
	char tmp_buf[128] = {0};

	if((tid = get_tid()) == MID_FAIL)
	{
		ht_dbg("[%s] Failed to connect to midcore\n", __FUNCTION__);
		return -1;
	}
	
#ifndef VR7516RW22	//Include the Telmex & PLDT, for only relead REMOTE listen process
	/* if http/https all disabled, just quit */
	if (mapi_ccfg_match_str(tid, "ARC_SYS_HTTP_Enable", "1"))
		http_enabled = 1; 

#ifdef CONFIG_HTTPD_HTTPS_SUPPORT
	if (mapi_ccfg_match_str(tid, "ARC_SYS_HTTPS_Enable", "1"))
		ssl_enabled = 1; 
	else
#endif
		ssl_enabled = 0; 

	if (!http_enabled && !ssl_enabled) {
		ht_dbg("HTTP/HTTPS all disabled, quit\n");
		exit(0);
	}
#endif	//endof ndef VR7516RW22

	/**
	 ** actually in new O2 user remote management, the httpd will support two services:
	 ** user remote GUI management and service remote GUI management.
	 ** so, the new design shows that httpd MUST support two types of remote GUI management
	 ** 1)user remote GUI management, which port is defined by user himself, and https session
	 ** 2)service remote GUI management, which port is defined as port 443 by the specification of 
	 **   " HomeBox 2 - 2nd. Source Device Requirements Template", and https session.
	 **/
	if (mapi_ccfg_match_str(tid, "ARC_UI_WEB_REMOTEMN_Enable", "1")){
		http_userremoteenable = 1;
	}
	else{
		http_userremoteenable = 0;
	}

#ifdef VRV9518SWAC33
	if (mapi_ccfg_match_str(tid, "ARC_PROJ_O2_UI_WEB_REMOTEMN_ServiceEnable", "1")){
		http_serviceremoteenable = 1;
	}
	else{
		http_serviceremoteenable = 0;
	}
#endif

	if (v_ops->clear_session_hook)
		v_ops->clear_session_hook();

	//get lanip
	strcpy(lanip_str, mapi_ccfg_get_str(tid, "ARC_LAN_0_IP4_Addr", tmp_buf, sizeof(tmp_buf)));
	strcpy(lanmask_str, mapi_ccfg_get_str(tid, "ARC_LAN_0_IP4_Netmask", tmp_buf, sizeof(tmp_buf)));
	strcpy(lanip6_str, mapi_ccfg_get_str(tid, "ARC_LAN_0_IP6_LLAAddr", tmp_buf, sizeof(tmp_buf)));
	strcpy(lanip_dev, mapi_ccfg_get_str(tid, "ARC_LAN_0_Ifname", tmp_buf, sizeof(tmp_buf)));

#ifndef VR7516RW22	//Include the Telmex & PLDT, for only relead REMOTE listen process
	//open the listening port according config var above
	if (http_port == 0)
	{
		if(mapi_ccfg_match_str(tid, "ARC_SYS_HTTP_UseSpecialPort", "1")) {
			mapi_ccfg_get_str(tid, "ARC_SYS_HTTP_PORT", tmp_buf, sizeof(tmp_buf));
			http_port = atoi(tmp_buf);
			if(http_port <= 0 || http_port > 65535) {
				http_port = DEFAULT_HTTP_PORT;
			}

			mapi_ccfg_get_str(tid, "ARC_SYS_HTTPS_PORT", tmp_buf, sizeof(tmp_buf));
			https_port = atoi(tmp_buf);
			if(https_port <= 0 || https_port > 65535) {
				https_port = DEFAULT_HTTPS_PORT;
			}
		} else {
			//Use default.
			http_port = DEFAULT_HTTP_PORT;
			https_port = DEFAULT_HTTPS_PORT;
		}
	}
	//end crazy_liang 2013.9.9
#else
	inet_aton(lanip_str, &lanip);
	//ht_dbg("lanip %x\n",lanip);
#endif	//endof ndef VR7516RW22

	/*first close all the http listening port*/
	httpd_main_exit = 1;

	/*
	 * we close the socket fd to let the thread exit automatically,
	 * ie. release resources,not to kill it
	 */
	ENTER_CRITICAL();

	for (i=0; i<MAX_LISTEN_PORTS; i++) {
		if(listen_pool[i].sd != -1)
		{
			ht_dbg("close the opened socket:%d\n", listen_pool[i].sd);
#ifdef VR7516RW22	//Include the Telmex & PLDT, for only relead REMOTE listen process
			if(listen_pool[i].if_addr.addr4.s_addr != lanip)
			{
#endif			
				cprintf("close pool %d listen is not lanip\n",i);
				sd = listen_pool[i].sd;
				listen_pool[i].sd = -1;
				close(sd);
				memset(&listen_pool[i].if_addr, 0, sizeof(listen_pool[i].if_addr));
				listen_pool[i].port = 0;
#ifdef VR7516RW22
			}
			else
				cprintf("close pool %d listen is lanip,slot %d, no action!!\n",i,listen_pool[i].slot);
#endif
		}
	}

	EXIT_CRITICAL();
	
	//wait for pthread to close
	for (i=0; i<MAX_LISTEN_PORTS; i++) {
#ifdef VR7516RW22
		if ((listen_pool[i].slot) && (listen_pool[i].sd == -1))
		{
			//we send the SIGUSR2 signal to interrupt the listener's accept block status
			cprintf("pool %d clear slot %d\n",i,listen_pool[i].slot);
			pthread_kill(listen_pool[i].slot, SIGUSR2);
			pthread_join(listen_pool[i].slot, NULL);
			listen_pool[i].slot = 0;
		}
#else
		if (listen_pool[i].slot)
		{
			//we send the SIGUSR2 signal to interrupt the listener's accept block status
			pthread_kill(listen_pool[i].slot, SIGUSR2);
			pthread_join(listen_pool[i].slot, NULL);
		}
		listen_pool[i].slot = 0;
#endif		
	}


	httpd_main_exit = 0;
#ifndef VR7516RW22	//Include the Telmex & PLDT, for only relead REMOTE listen process
	/* HTTP only */
	if (http_enabled){
		//add_listener(inet_addr(lanip_str), DEFAULT_HTTP_PORT, 0);
		if (inet_pton(AF_INET, lanip_str, &addr4) == 1)
			add_listener(addr4, http_port, 0); //crazy_liang 2013.9.9

		DELAYMS(200);

		if (inet_pton(AF_INET6, lanip6_str, &addr6) == 1)
			add_listener6(addr6, http_port, lanip_dev, 0);
	}
#endif	//end of ndef VR7516RW22

	if (http_userremoteenable
#ifdef VRV9518SWAC33
		|| http_serviceremoteenable
#endif
		){
		get_wan_address();
	}

	if (http_userremoteenable){
		https_userremoteport = atoi(mapi_ccfg_get_str(tid, "ARC_UI_WEB_REMOTEMN_Port",
					tmp_buf, sizeof(tmp_buf)));
		if (https_userremoteport == 0){
			https_userremoteport = DEFAULT_HTTPS_PORT;
		}

		//we add a new listener to support it, with https session enabled.
		if(strlen(wanip_str) && strcmp(wanip_str, "0.0.0.0"))
		{
			if (inet_pton(AF_INET, wanip_str, &addr4) == 1)
				add_listener(addr4, https_userremoteport, 1);

		}
		DELAYMS(200);
		if(strlen(wanip6_str) && strcmp(wanip6_str, "::"))
		{
			if (strncmp(wanip6_en, "on", 2) == 0 &&
				inet_pton(AF_INET6, wanip6_str, &addr6) == 1){
				add_listener6(addr6, https_userremoteport, wanip_dev, 1);
			}
		}
	}

#ifdef VRV9518SWAC33
	if (http_serviceremoteenable){
		https_serviceremoteport = atoi(mapi_ccfg_get_str(tid, "ARC_PROJ_O2_UI_WEB_REMOTEMN_ServicePort",
					tmp_buf, sizeof(tmp_buf)));
		if (https_serviceremoteport == 0){
			https_serviceremoteport = DEFAULT_HTTPS_PORT;
		}

		//we add a new listener to support it, with https session enabled.
		if(strlen(wanip_str) && strcmp(wanip_str, "0.0.0.0"))
		{
			if (inet_pton(AF_INET, wanip_str, &addr4) == 1)
				add_listener(addr4, https_serviceremoteport, 1);
		}

		DELAYMS(200);
		if(strlen(wanip6_str) && strcmp(wanip6_str, "::"))
		{
			if (strncmp(wanip6_en, "on", 2) == 0 &&
					inet_pton(AF_INET6, wanip6_str, &addr6) == 1){
				add_listener6(addr6, https_serviceremoteport, wanip_dev, 1);
			}
		}
	}
#endif

#ifndef VR7516RW22   //Telmex don't need https on LAN. include PP??
#ifdef CONFIG_HTTPD_HTTPS_SUPPORT
	if (ssl_enabled){
		//add_listener(inet_addr(lanip_str), DEFAULT_HTTPS_PORT, 1);
		if (inet_pton(AF_INET, lanip_str, &addr4) == 1)
			add_listener(addr4, https_port, 1); //crazy_liang 2013.9.9

		DELAYMS(200);

		if (inet_pton(AF_INET6, lanip6_str, &addr6) == 1)
			add_listener6(addr6, https_port, lanip_dev, 1);
	}
#endif
#endif	//endof ndef VR7516RW22

	/* Terry 20141027, for o2 redirect URL */
#ifdef VRV9518SWAC33
	o2_get_local_addresses();
#endif

	return 0;
}

void sig_handler(int signo)
{
	/* re-catch */
	signal(signo, sig_handler);

	switch(signo)
	{
		case SIGSEGV:
			log_error("httpd: caught SIGSEGV, dumping core");
			pthread_exit(0);
			break;
		case SIGBUS:
			log_error("httpd: caught SIGBUS, dumping core");
			pthread_exit(0);
			break;
		case SIGUSR1:
			log_error("httpd: caught SIGUSR1");
			httpd_reload = 1;
			break;
		case SIGUSR2:
			log_error("httpd: caught SIGUSR2");
			break;
		case SIGTERM:
			//log_error("httpd: caught SIGTERM, shutting down");
			pthread_exit(0);
			break;
		default:
			break;
	}

	return;
}

/*
 * ignore the SIGUSR1 signal which cause the main process to
 * call the reload_httpd() 
 * */
void sig_ignore_reload_httpd()
{
	sigset_t set;
	int s;

	sigemptyset(&set);
	sigaddset(&set, SIGUSR1);

	s = pthread_sigmask(SIG_BLOCK, &set, NULL);
	if (s != 0)
	{
		perror("pthread_sigmask");
	}

	return;
}

void set_signals(void) 
{
    signal(SIGSEGV, sig_handler);
    signal(SIGBUS,  sig_handler);
    signal(SIGTERM, sig_handler);
    signal(SIGUSR1, sig_handler);
    signal(SIGUSR2, sig_handler);

    signal(SIGCHLD, SIG_IGN);
    signal(SIGABRT, SIG_IGN);
    signal(SIGPIPE, SIG_IGN);
}

//violet_20150806 add for directly debug web ++
#ifdef CONFIG_HTTPD_SUPPORT_TOKEN_DISABLE
extern int G_token_off_flag;
#endif //CONFIG_HTTPD_SUPPORT_TOKEN_DISABLE
//violet_20150806  --

//for the thread_pool are all used. roger-20170915
pthread_t   pthread_monitor_id;
void *httpd_monitor(void)
{
	int i, in_use = 0, full_count = 0, is_free = 0;
	
	while(1)
	{	
		in_use = 0;
		is_free = 0;
		for (i=0; i<THREAD_COUNT; i++)
		{
			pthread_mutex_lock(&(thread_pool[i]->f_lock));
			if (thread_pool[i]->is_busy == 1)
				in_use++;
			else
				is_free++;
			pthread_mutex_unlock(&(thread_pool[i]->f_lock));
			if(is_free)
				break;
		}
		
		if(in_use == THREAD_COUNT)
			full_count++;
		else
			full_count = 0;

		if(full_count > 2)
		{
			ht_dbg("httpd_child threads blocked, need restart httpd.\n");
			system("mngcli action httpd_restart");
		}
		sleep(5);
	}
}

int main(int argc, char *argv[])
{
    int i, c;
    int ret;
    int tid;
	//crazy_liang 2013.9.9: add two options (p/s) for the specified http(s) ports.
	int http_port = 0;
	int https_port = 0;
	struct in6_addr addr6;
	struct in_addr addr4;
    int https_userremoteport=0;
	/*initialize pthread mutex*/
	if (pthread_mutex_init(&rand_mutex_c, NULL) != 0)
		return -1;
	
#ifdef VRV9518SWAC33
    int https_serviceremoteport=0;
#endif
	char tmp_buf[8] = {0};
	//end crazy_liang 2013.9.9

	int http_monitor = 1;
	while ((c = getopt(argc, argv, "p:d:v:t:m")) != -1) {
		switch(c) {
			case 'd':
				document_root = optarg;
				break;
			case 'v':
				ht_dbg("Version: %s.\n", SERVER_VERSION);
				exit(1);
			case 'p':
				ht_dbg("Port option: [%s]\n", optarg);
				http_port = atoi(optarg);
				break;
			case 't':
//violet_20150806 add for directly debug web ++
#ifdef CONFIG_HTTPD_SUPPORT_TOKEN_DISABLE
				G_token_off_flag = 1;
#endif
//violet_20150806 --
				break;
			case 'm':
				http_monitor = 0;
				break;
			case '?':
				usage(argv[0]);
				exit(0);
		}
	}

	set_signals();

	if((tid = get_tid()) == MID_FAIL)
	{
		ht_dbg("[%s] Failed to connect to midcore\n", __FUNCTION__);
		/*destroy pthread mutex*/
		pthread_mutex_destroy(&rand_mutex_c);
		return -1;
	}
	/* if http/https all disabled, just quit */
	if (mapi_ccfg_match_str(tid, "ARC_SYS_HTTP_Enable", "1"))
		http_enabled = 1; 

#ifdef CONFIG_HTTPD_HTTPS_SUPPORT
	if (mapi_ccfg_match_str(tid, "ARC_SYS_HTTPS_Enable", "1"))
		ssl_enabled = 1; 
	else
#endif
		ssl_enabled = 0; 

	if (!http_enabled && !ssl_enabled) {
		ht_dbg("HTTP/HTTPS all disabled, quit\n");
		exit(0);
	}

    /**
    ** actually in new O2 user remote management, the httpd will support two services:
    ** user remote GUI management and service remote GUI management.
    ** so, the new design shows that httpd MUST support two types of remote GUI management
    ** 1)user remote GUI management, which port is defined by user himself, and https session
    ** 2)service remote GUI management, which port is defined as port 443 by the specification of 
    **   " HomeBox 2 - 2nd. Source Device Requirements Template", and https session.
    **/
    if (mapi_ccfg_match_str(tid, "ARC_UI_WEB_REMOTEMN_Enable", "1")){
        http_userremoteenable = 1;
    }
    else{
        http_userremoteenable = 0;
    }

#ifdef VRV9518SWAC33
    if (mapi_ccfg_match_str(tid, "ARC_PROJ_O2_UI_WEB_REMOTEMN_ServiceEnable", "1")){
        http_serviceremoteenable = 1;
    }
    else{
        http_serviceremoteenable = 0;
    }
#endif

	/* init */
	if (httpd_init() < 0){
		/*destroy pthread mutex*/
		pthread_mutex_destroy(&rand_mutex_c);
		return -1;
	}
	/* main */
	ht_prior = 0; /* set up the task's priority in httpd group */

	

	/* Firstly, bring up the worker threads */
	for (i=0; i<THREAD_COUNT; i++) {

		size_t stacksize;
		pthread_attr_t attr;

		pthread_attr_init(&attr);
		stacksize = (size_t)HT_MAIN_STKSIZE;

		pthread_attr_setstacksize (&attr, stacksize);
		pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

		ret = pthread_create(&(thread_pool[i]->pthread_id), NULL, (void  *)httpd_child, (void *)(&(thread_pool[i]->number_id)));
		if (ret != 0) {
			ht_dbg("Create pthread %d failed \n", i);
			/*destroy pthread mutex*/
			pthread_mutex_destroy(&rand_mutex_c);
			return -1;
		}
	}

	if(http_monitor)
	{
		ht_dbg("Start create pthread httpd_monitor.\n");
		ret = pthread_create(&pthread_monitor_id, NULL, (void  *)httpd_monitor, NULL);
		if (ret != 0) {
			ht_dbg("Create pthread httpd_monitor failed.\n");
			/*destroy pthread mutex*/
			pthread_mutex_destroy(&rand_mutex_c);
			return -1;
		}
	}

	//crazy_liang 2013.9.9: Read HTTP(S) port from glbcfg.
if (http_port == 0)
{
	if(mapi_ccfg_match_str(tid, "ARC_SYS_HTTP_UseSpecialPort", "1")) {
		mapi_ccfg_get_str(tid, "ARC_SYS_HTTP_PORT", tmp_buf, sizeof(tmp_buf));
		http_port = atoi(tmp_buf);
		if(http_port <= 0 || http_port > 65535) {
			http_port = DEFAULT_HTTP_PORT;
		}

		mapi_ccfg_get_str(tid, "ARC_SYS_HTTPS_PORT", tmp_buf, sizeof(tmp_buf));
		https_port = atoi(tmp_buf);
		if(https_port <= 0 || https_port > 65535) {
			https_port = DEFAULT_HTTPS_PORT;
		}
	} else {
		//Use default.
		http_port = DEFAULT_HTTP_PORT;
		https_port = DEFAULT_HTTPS_PORT;
	}
}
	//end crazy_liang 2013.9.9

	/* HTTP only */
	if (http_enabled){
		//add_listener(inet_addr(lanip_str), DEFAULT_HTTP_PORT, 0);
		if (inet_pton(AF_INET, lanip_str, &addr4) == 1)
			add_listener(addr4, http_port, 0); //crazy_liang 2013.9.9

		DELAYMS(200);
		if (inet_pton(AF_INET6, lanip6_str, &addr6) == 1)
			add_listener6(addr6, http_port, lanip_dev, 0);
	}

	if (http_userremoteenable
#ifdef VRV9518SWAC33
		|| http_serviceremoteenable
#endif
		){
		get_wan_address();
	}

    if (http_userremoteenable){
        https_userremoteport = atoi(mapi_ccfg_get_str(tid, "ARC_UI_WEB_REMOTEMN_Port",
                                    tmp_buf, sizeof(tmp_buf)));
        if (https_userremoteport == 0){
            https_userremoteport = DEFAULT_HTTPS_PORT;
        }

        //we add a new listener to support it, with https session enabled.
	if(strlen(wanip_str) && strcmp(wanip_str, "0.0.0.0"))
	{
		if (inet_pton(AF_INET, wanip_str, &addr4) == 1)
        	add_listener(addr4, https_userremoteport, 1);

	}
		DELAYMS(200);
	if(strlen(wanip6_str) && strcmp(wanip6_str, "::"))
	{
		if (strncmp(wanip6_en, "on", 2) == 0 &&
				inet_pton(AF_INET6, wanip6_str, &addr6) == 1){
			add_listener6(addr6, https_userremoteport, wanip_dev, 1);
		}
	}
    }

#ifdef VRV9518SWAC33
    if (http_serviceremoteenable){
        https_serviceremoteport = atoi(mapi_ccfg_get_str(tid, "ARC_PROJ_O2_UI_WEB_REMOTEMN_ServicePort",
                                    tmp_buf, sizeof(tmp_buf)));
        if (https_serviceremoteport == 0){
            https_serviceremoteport = DEFAULT_HTTPS_PORT;
        }

        //we add a new listener to support it, with https session enabled.
      	if(strlen(wanip_str) && strcmp(wanip_str, "0.0.0.0"))
      	{
		if (inet_pton(AF_INET, wanip_str, &addr4) == 1)
        	add_listener(addr4, https_serviceremoteport, 1);
      	}
		DELAYMS(200);
	if(strlen(wanip6_str) && strcmp(wanip6_str, "::"))
	{
		if (strncmp(wanip6_en, "on", 2) == 0
			&& inet_pton(AF_INET6, wanip6_str, &addr6) == 1){
			add_listener6(addr6, https_serviceremoteport, wanip_dev, 1);
		}
		}
    }
#endif

#ifndef VR7516RW22   //Telmex don't need https on LAN.
#ifdef CONFIG_HTTPD_HTTPS_SUPPORT
	if (ssl_enabled){
		//add_listener(inet_addr(lanip_str), DEFAULT_HTTPS_PORT, 1);
		if (inet_pton(AF_INET, lanip_str, &addr4) == 1)
			add_listener(addr4, https_port, 1); //crazy_liang 2013.9.9

		DELAYMS(200);

		if (inet_pton(AF_INET6, lanip6_str, &addr6) == 1)
			add_listener6(addr6, https_port, lanip_dev, 1);
	}
#endif
#endif

	if (v_ops->open_hook)
		v_ops->open_hook();

	ht_dbg("%s: starting ...\n", argv[0]);
#ifdef SYSLOG_ENHANCED
	SetSystemLog(LOG_TYPE_APP, LOG_LEVEL_TYPE_INFO, LOG_APP_MESSAGE_TYPE_http, "HTTP-start ...");
	char fw_version[32] = {0};
	char fw_subversion[32] = {0};
	char boot[8] = {0};
	char log_buf[128] = {0};
	mapi_ccfg_get_str(tid, "ARC_SYS_FWVersion", fw_version, sizeof(fw_version));
	mapi_ccfg_get_str(tid, "ARC_SYS_FWSubVersion", fw_subversion, sizeof(fw_subversion));
	mapi_ccfg_get_str(tid, "ARC_SYS_Boot", boot, sizeof(boot));
#ifdef WE410443_6DX
	sprintf(log_buf, "Current firmware version is %s.%s and boot flag is %d.", fw_version, fw_subversion, atoi(boot));
#else
	sprintf(log_buf, "Current firmware version is %s %s and boot flag is %d.", fw_version, fw_subversion, atoi(boot));
#endif
	ht_dbg("%s\n", log_buf);
	SetSystemLog(LOG_TYPE_APP, LOG_LEVEL_TYPE_INFO, LOG_APP_MESSAGE_TYPE_http, log_buf);
#endif
	/* Terry 20141027, for o2 redirect URL */
#ifdef VRV9518SWAC33
	o2_get_local_addresses();
#endif

	if (!is_tmr_init()) tmr_init();

	/* infinitely loop */
	//tmr_task();

	while (1) {
		if (httpd_reload == 1){
			httpd_reload = 0;
			reload_httpd();
		}

		if (v_ops->check_session_hook)
			v_ops->check_session_hook();

		tmr_check();
		DELAYMS(500);
	}

	ht_dbg("%s: exiting...\n", argv[0]);
#ifdef SYSLOG_ENHANCED
		SetSystemLog(LOG_TYPE_APP, LOG_LEVEL_TYPE_INFO, LOG_APP_MESSAGE_TYPE_http, "HTTP-exiting ...");
#endif
	/*destroy pthread mutex*/
	pthread_mutex_destroy(&rand_mutex_c);
	
    return 0;
}

/* Terry 20141027, for o2 redirect URL */
#ifdef VRV9518SWAC33
#include <arpa/inet.h>

unsigned char g_local_v4_addr[sizeof(struct in_addr)];
unsigned char g_local_v6_addr[sizeof(struct in6_addr)];

void o2_get_local_addresses()
{
	int tid;
	char addr_buf[INET6_ADDRSTRLEN];

	if ((tid = get_tid()) == MID_FAIL) {
		ht_dbg("[%s] Failed to connect to midcore\n", __FUNCTION__);
		return;
	}
	
	arc_cfg_get_idx1(tid, ARC_LAN_x_IP4_Addr, 0, addr_buf, sizeof(addr_buf), "");
	if (inet_pton(AF_INET, addr_buf, g_local_v4_addr) <= 0) {
		ht_dbg("[%s] Failed to parse local v4 address %s\n", __FUNCTION__, addr_buf);
	}

	arc_cfg_get_idx1(tid, ARC_LAN_x_IP6_LLAAddr, 0, addr_buf, sizeof(addr_buf), "");
	if (inet_pton(AF_INET6, addr_buf, g_local_v6_addr) <= 0) {
		ht_dbg("[%s] Failed to parse local v6 address %s\n", __FUNCTION__, addr_buf);
	}
}

int o2_redirect(int s, struct request_rec *r)
{
	int tid;
	char *host_addr = r->host;
	unsigned char local_v4_addr[sizeof(struct in_addr)];
	unsigned char local_v6_addr[sizeof(struct in6_addr)];
	
	if ((tid = get_tid()) == MID_FAIL) {
		ht_dbg("[%s] Failed to connect to midcore\n", __FUNCTION__);
		return s;
	}

	if (!arc_cfg_match(tid, ARC_PROJ_O2_AccessNetworkDiscoveryConfirm, "3")) {
		/* WAN not provisioned */
		if (!strcmp("o2.box", host_addr)) {
			return s;
		}

		/* 2015-0128, Vergil Wei. */
		/* Fix the issue of the Windows 8 Active Probing. */
		/* Bypass the URL http://www.msftncsi.com/ncsi.txt. */
		if (strstr (r->url, "/ncsi.txt")) {
			return s;
		}
		
		if (inet_pton(AF_INET, host_addr, local_v4_addr) == 1) {
			if (!memcmp(local_v4_addr, g_local_v4_addr, sizeof(struct in_addr)))
				return s;
		} else if (inet_pton(AF_INET6, host_addr, local_v6_addr) == 1) {
			if (!memcmp(local_v6_addr, g_local_v6_addr, sizeof(struct in6_addr)))
				return s;
		}
		/* Redirect URL */
#ifdef CONFIG_HTTPD_HTTPS_SUPPORT
		if (r->ishttps)
			strncpy(r->url, "https://o2.box", sizeof(r->url));
		else
#endif
			strncpy(r->url, "http://o2.box", sizeof(r->url));
		
		return REDIRECT_URL;
	}
	
	return s;
}
#endif

#ifdef CONFIG_HTTPD_Security_Attack_Protect
int is_block_attack_addr(sock_t ip)
{
	int is_attack = 0;
	int is_ever_attack = -1;
	time_t rawtime;
	struct sysinfo now_time;
	sysinfo(&now_time);
	time ( &rawtime );
	int time_diff = 0;
	int time = now_time.uptime;

	is_ever_attack = find_entry_in_attack_array(ip);
	if (-1 == is_ever_attack) 
	{
		insert_entry_info_attack_array(ip, time);
	}
	else
	{
		time_diff = time - attack_entry.attack_addr[is_ever_attack].timeinfo;
		if (time_diff > TIMEOUT_BLOCK_ATTACK_ADDR) // block timeout
		{
			is_attack = 0;
			attack_entry.attack_addr[is_ever_attack].is_attacker = 0;
			attack_entry.attack_addr[is_ever_attack].timeinfo = time;
			attack_entry.attack_addr[is_ever_attack].attck_cnt = 0;
		}
		else
		{
			is_attack = 1;
		}
	}

	return is_attack;
}

void check_addr_is_attacker(sock_t ip)
{
	int is_attack = 0;
	int is_ever_attack = -1;
	time_t rawtime;
	struct sysinfo now_time;
	sysinfo(&now_time);
	time ( &rawtime );
	int time_diff = 0;
	int time = now_time.uptime;

	is_ever_attack = find_entry_in_attack_array(ip);
	if (-1 == is_ever_attack) 
	{
		insert_entry_info_attack_array(ip, time);
	}
	else
	{
		if (0 == attack_entry.attack_addr[is_ever_attack].is_attacker)
		{
			attack_entry.attack_addr[is_ever_attack].attck_cnt++;
			if (0 == attack_entry.attack_addr[is_ever_attack].attck_cnt % MAX_ATTACK_CNT)
			{
				time_diff = time - attack_entry.attack_addr[is_ever_attack].timeinfo;
				attack_entry.attack_addr[is_ever_attack].timeinfo = time;
				attack_entry.attack_addr[is_ever_attack].attck_cnt = 0;
				if (time_diff > DETECT_ATTACK_INTERVAL) // not attack
				{
					attack_entry.attack_addr[is_ever_attack].is_attacker = 0;
					is_attack = 0;
				}
				else // not attack
				{
					attack_entry.attack_addr[is_ever_attack].is_attacker = 1;
					is_attack = 1;
				}
			}
		}
	}
}

int find_entry_in_attack_array(sock_t ip)
{
	int index = 0;

	for (index = 0;index < attack_entry.num; index++)
	{
		if (ip.sa_in.sin_addr.s_addr == attack_entry.attack_addr[index].attack_ip.sa_in.sin_addr.s_addr)
		{
			return index;
		}
	}

	return -1;
}

void insert_entry_info_attack_array(sock_t ip, int timeinfo)
{
	int min_index = 0;
	
	if (attack_entry.num >= MAX_ATTACK_ADDR_NUM) // full
	{
		min_index = search_oldest_entry_attack_array();
	}
	else // not full
	{
		min_index = attack_entry.num;
	}
	memcpy(&attack_entry.attack_addr[min_index].attack_ip, &ip, sizeof(sock_t));
	attack_entry.attack_addr[min_index].timeinfo = timeinfo;
	attack_entry.attack_addr[min_index].attck_cnt = 0;
	attack_entry.num = ((attack_entry.num + 1) >= MAX_ATTACK_ADDR_NUM) ? MAX_ATTACK_ADDR_NUM : (attack_entry.num + 1);
}

int search_oldest_entry_attack_array()
{
	int min_index = 0;
	int min_time = attack_entry.attack_addr[0].timeinfo;
	int index = 0;

	for (index = 0;index < attack_entry.num; index++)
	{
		if (attack_entry.attack_addr[index].timeinfo < min_time)
		{
			min_index = index;
			min_time = attack_entry.attack_addr[index].timeinfo;
		}
	}

	return min_index;
}
#endif
