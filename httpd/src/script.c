/*
 * http_script: keeps all script-related ramblings together.
 *
 * Compliant to CGI/1.0 spec
 *
 * Rob McCool
 *
 */

/*
 * http_script.c be renamed to script.c, a ported edition of Multitask!
 *
 * Copyright (c) 1998 Institute for Information Industry. All right reserved.
 *
 * by C.H. Lin of Technical Service Dept.
 *
 */

#include <sys/types.h>
#include <dirent.h>

#include "httpd.h"
#include "cgi.h"

#ifdef HNAP_SUPPORT
#define MAX_HNAP_TIME	20
#endif

/* Global vars for CGI task. Need lock protection */
static CGI_info cgi_info;
int cgi_argc = 1;
char **cgi_argv;
pthread_mutex_t CGI_RESOURCE = PTHREAD_MUTEX_INITIALIZER;

/*
 * CGI Table: Translate script's filename to its task function, all tasks can
 *			 be assigned a different stack size, respectively
 */
CGI_ENTRY null_CGI_tbl[1] = {
	{NULL, NULL, 0},
};

CGI_ENTRY *_CGI_tbl = null_CGI_tbl; 
#ifdef EXTERN_CGI_SUPPORT
BIN_ENTRY bin_tbl[MAX_BIN_SIZE];
#endif

void CGI_init(CGI_ENTRY tbl[])
{
#ifdef EXTERN_CGI_SUPPORT
	int i = 0;
	struct dirent *x;
	DIR *d;
	char buf[512];
#endif

	_CGI_tbl = tbl;

#ifdef EXTERN_CGI_SUPPORT

	d = opendir(BINARY_PATH);

	if (d == NULL) {
		perror("opendir");
		return;
	}

	while (1) {
		BIN_ENTRY *bin;
		char *ext;

		x = readdir(d);
		if (x == NULL)
			break;

		if (x->d_name[0] == '\0')
			continue;

		if (!strcmp(x->d_name, ".") ||
			!strcmp(x->d_name, ".."))
			continue;

		snprintf(buf, sizeof(buf), "%s%s", BINARY_PATH, x->d_name);
		/* make sure the bin can be executed */
		if (access(buf, F_OK | X_OK) != 0) {
			ht_dbg("access [%s] failed\n", buf);
			continue;
		}

		/* table entry  */
		bin = (BIN_ENTRY *)&bin_tbl[i++];

		bin->name = (char *)malloc(MAXNAMLEN+1);
		if (bin->name == NULL) {
			perror("malloc");
			break;	/* just break out, keep already read enties */
		}

		memset(bin->name, '\0', MAXNAMLEN+1);

		strncpy(bin->name, x->d_name, strlen(x->d_name));
		if ((ext=strrchr(bin->name, '.')) == NULL) {
			bin->type = TYPE_BINARY;
			continue;
		} else {
			if (!strcasecmp(ext, ".sh"))
				bin->type = TYPE_SHELL;
			else
				bin->type = TYPE_BINARY;
		}
		ht_dbg("bin=[%s], type=[%s]\n", bin->name, bin->type==TYPE_SHELL? "shell" : "binary");
	}

	closedir(d);
#endif
	

	return;
}

#ifdef HNAP_SUPPORT
void hnap_cgi_stub(struct request_rec *r)
{
	ht_dbg("\n");


	tmr_set(r->cookie, ht_timeout + MAX_HNAP_TIME, timed_out_handler);

	/* wait until cgi task to be finished */
	r->bytes_sent = 0;

	LOCK_CGI();

	r->cgi_slot = r->slot;
#ifdef CONFIG_HTTPD_HTTPS_SUPPORT
	if (r->ishttps)
		hnap_handle(r->sd, r->method, r->auth_line, r->soapaction, 
			(long)r->content_length, r->ishttps, (unsigned long)r->remote_ip.sa_in.sin_addr.s_addr, r->ssrv);
	else
#endif //CONFIG_HTTPD_HTTPS_SUPPORT
		hnap_handle(r->sd, r->method, r->auth_line, r->soapaction, 
			(long)r->content_length, r->ishttps, (unsigned long)r->remote_ip.sa_in.sin_addr.s_addr, NULL);

	UNLOCK_CGI();

	//sleep(1);

#if 0
	/* block-wait CGI task finished */
	pthread_join(r->cgi_slot, &status);
	if (status != CGI_CODE) {
		/* kill cgi Z thread */
		pthread_kill(r->cgi_slot, SIGKILL);
	}

	ht_dbg("CGI thread %d was joined with status [%d]\n", r->index, status);
#endif

	return;
}
#endif


void send_response(struct request_rec *r, char *resp)
{
	char buf[256];
	int len;
	char *l = buf;

	if (!r)
		return;

	snprintf(buf, sizeof(buf), "%s", resp);

	//ht_dbg("buf=[%s]\n", buf);
	
	len = strlen(resp);
	if (len > 0 && buf[len-1] == '\n')  {
		/* strip out ending "LFCR" or 'LF' */
		if (len > 1 && buf[len-2] == '\015')        /* 'CR' */
			buf[len-2] = '\0';
		else
			buf[len-1] = '\0';
	}

	if (buf[0] == '\0') {	/* end of file or pipe */
		if (r->location[0]) {
			if (!is_url(r->location)) {
				strcpy(buf, r->location);
				construct_url(r, r->location, buf, lanip_str, r->server_port, r->ishttps);
			}
		}

		return;
	}

	l = _getword(buf, ':'); /* the delimiter ':' been replaced with '\0' */
	if (*l == '\0')
		return;

	if (!strcasecmp(buf, "Location")) {
		r->status = REDIRECT;

		/* ... but if we haven't - make one */
		sscanf(l, "%s", r->location);
	}

	r->content_length = 0;

	if (!r->assbackwards) {
		if (!strcasecmp(r->method, "get")) {
			set_expires(r, &r->finfo.ftime);
		}
		else
			r->expires[0] = '\0';

		send_http_header(r);
	}

	return;
}

char **create_argv(char *av0, char *args, int *size)
{               
	int x,n;
	char **av, *w, *t;

	for(x=0,n=2;args[x];x++)
		if(args[x] == '+') ++n;

	if ((av = (char **) malloc((n+1)*sizeof(char *)) ) == NULL)
		return NULL;

	t = args;
	av[0] = av0;

	for(x=1;x<n;x++) {
		w = t;
		t = _getword(t,'+');

		av[x] = w;
	}

	av[n] = NULL;
	*size = n;
	return av;
}

/*
 * Translates script's pathname to the address of it's task function. The whole
 * path other than filename part is to be omitted.
 */
void (* script2func(char *path, int *stksiz, int *bin_idx))()
{
	int i = 0;
	char *fname;

	if ((fname = strrchr(path, '/')) == NULL)
		fname = path;
	else
		fname++;

	while (_CGI_tbl[i].path) {
		if (!strcasecmp(_CGI_tbl[i].path, fname)) {
			*stksiz = _CGI_tbl[i].stack_size;
			*bin_idx = -1;
			return _CGI_tbl[i].script;
		} 

		i++;
	}

#ifdef EXTERN_CGI_SUPPORT
	i = 0;

	/* no found static entry, search binary for CGI */
	while (bin_tbl[i].name) {
		BIN_ENTRY *bin = (BIN_ENTRY *)&bin_tbl[i];
		int len;
		char *pf;

		if ((pf = strrchr(bin->name, '.')) == NULL) {
			len = strlen(bin->name);
		}
		else {
			len = bin->name + strlen(bin->name) - pf;
		}

		ht_dbg("bin=[%s], len=[%d], fname=[%s]\n", bin->name, len, fname);
		if (!strncasecmp(bin->name, fname, len)) {
			*bin_idx = i;
			return extern_cgi;
		}

		i++;
	}
#endif

	return((void (*)()) 0);
}

int cgi_stub(struct request_rec *req)
{
	char *param;
	int stksiz;
	int bin_idx = -1;
	void    (*cmd)();
	struct th_info th;
	struct request_rec *r = req;

	memset(&cgi_info, 0 , sizeof(cgi_info));

	cgi_info.r = r;
	cgi_info.socketid = r->sd;

	if ((param = strrchr(r->url, '/')) != NULL)
		param++;
	else 
		param = r->url;

	ht_dbg("[%d] method=[%s], url=[%s], param=[%s], args=[%s]\n", r->index, r->method, r->url, param, r->args);


	if ((cmd = script2func(r->url, &stksiz, &bin_idx)) == NULL) {

		ht_dbg(" Found no CGI func.\n");

		garbage_collect(r, r->sd, r->content_length);
		unmunge_name(r->url);
		answer(r, NOT_FOUND, r->url);

		return -1;
	}

#ifdef EXTERN_CGI_SUPPORT
	/* extern CGI */
	if (cmd == extern_cgi && bin_idx >= 0 ) {

		cgi_info.bin = &bin_tbl[bin_idx%MAX_BIN_SIZE];
	}
#endif

	tmr_set(r->cookie, ht_timeout+20, timed_out_handler);

	if (strstr(r->url, "upload.cgi") !=0) {
		/* if upgrade f/w cgi, extend the timeout */
		ht_dbg(" CGI:%s timeout:%d\n", r->url, ht_timeout+120);
		tmr_set(r->cookie, ht_timeout+120, timed_out_handler);
	}

	cgi_argv = create_argv(param, r->args, &cgi_argc);

	if (cgi_argv)
		ht_dbg("argc=[%d], argv=[%s]\n", cgi_argc, *cgi_argv);

	//snprintf(cgi_argv, sizeof(cgi_argv), "%s",  param);

	th.prior = 100;
	th.stacksize = stksiz;
	th.type = TH_CGI;
	th.args_num = 3;
	th.args_list[0] = (long)cmd;
	th.args_list[1] = (long)(&cgi_argc);
	th.args_list[2] = (long)(cgi_argv);
	th.args_list[3] = (long)(&cgi_info);
	
	if (ht_create_thread(&th) == -1) {
		log_error("CGI execution error!");
		answer(r, SERVER_ERROR, "CGI execution error!");
		return -1;
	}

	r->cgi_slot = th.pthread_id;

	return 0;
}

/* Called for ScriptAliased directories */
void exec_cgi_script(struct request_rec *r)
{
	//char **env;
	int status = 0;

	ht_dbg("[%d]: CGI task ...\n", r->index);

	//evaluate_access(r->url, r->finfo.is_dir, r);
	r->bytes_sent = 0;

	LOCK_CGI();
	if (cgi_stub(r) < 0 || r->cgi_slot <= 0) {
		UNLOCK_CGI();
		return ;
	}

	//sleep(1);

	/* block-wait CGI task finished */
	pthread_join(r->cgi_slot, (void **)&status);
	if (status != CGI_CODE) {
		/* kill cgi Z thread */
		pthread_kill(r->cgi_slot, SIGKILL);
	}
	UNLOCK_CGI();

	ht_dbg("CGI thread %d was joined with status [%d]\n", r->index, status);
}



