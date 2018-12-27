/*
 * http_request.c: functions to get and process requests
 *
 * Rob McCool 3/21/93
 *
 */
#include "httpd.h"
#include <sys/sysinfo.h>

extern int FSread(char *buf, unsigned size, void *fp);
extern int FSgetc(void *fp);

#define MAX_CONTENT_LENGTH	64000

// added by chlin, to prevent attacks of huge content-length
const char form_type[] = "multipart/form-data";

/*
 * time outs handler.
 */

void timed_out_handler(COOKIE_t cookie)
{
	char tbuf[100]={0};
	
	snprintf(tbuf, sizeof(tbuf)-1, "WARNING: %lu time out", (unsigned long)pthread_self());
	tbuf[sizeof(tbuf)-1] = '\0';

	//log_error(tbuf);
	//pthread_exit(0);

	return ;
}



long send_fd(register struct request_rec *r)
{
	long total_bytes_sent = 0;
	int nbytes = 0, o, w, ret;
	int nbytes_read;
	int open_file_flag = 0;
	fd_set readfds, writefds; // added by chlin, for PS3
	struct timeval timeouts;
	char *out_buf = r->out_buf;

	//ht_dbg("[%d] Entering ..., content_length:%lld\n", r->index, r->content_length);

	so_flush(r);

	memset(out_buf, 0, IOBUFSIZE);
	
	while (r->content_length != 0) {
#ifdef CONFIG_HTTPD_HTTPS_SUPPORT
		if (r->ishttps == 0)
#endif //CONFIG_HTTPD_HTTPS_SUPPORT
    {
		FD_ZERO(&readfds);
		FD_ZERO(&writefds);
		/* wait for data */
		timeouts.tv_sec = ht_timeout;
		timeouts.tv_usec = 0;
		FD_SET(r->sd, &readfds);
		FD_SET(r->sd, &writefds);

		ret = select(r->sd+1, &readfds, &writefds, NULL, &timeouts);

		if (ret <= 0) {
			goto done;
		}

		if (FD_ISSET(r->sd, &readfds)) {
			ht_dbg("[%d] maybe new request... BREAK !!!\n", r->index);
			goto done;
		}
    }

		if(r->fd == (FILE_t)NULL)
			open_file_flag = 0;
		else
			open_file_flag = 1;

		nbytes_read = IOBUFSIZE;

		if (open_file_flag==1) {

			if (r->content_length > 0) {
				if (r->content_length < IOBUFSIZE) 
					nbytes_read = r->content_length;
			}

			if ((nbytes = FSread(out_buf, nbytes_read, r->fd)) < 1)
				break;	// end of file
		}
		else {
			log_error("No handle used");
			answer(r, SERVER_ERROR, "No handle used");
			goto done;
		}

		// moved here by chlin
		if (r->content_length >= nbytes) {
			r->content_length -= nbytes;
		}
		else if (r->content_length >= 0) {
			printf("Invalid content length:%lld, nbytes:%d\n", r->content_length, nbytes);
			r->content_length = 0L;
		}

		ht_dbg("[%d] read %d bytes...\n", r->index, nbytes);

		o = 0;
		if(r->bytes_sent != -1) /* if bytes_sent's initial value is */
			r->bytes_sent += nbytes; /* zero then accumulate the */
						/* size otherwise abandon it */

		while(nbytes) {
#ifdef CONFIG_HTTPD_HTTPS_SUPPORT
			if (r->ishttps) {
				w = nbytes;
				ret = as_write(r->ssrv, (unsigned char *)out_buf+o, w);

				if (ret<=0) {
					goto done;
				}
			} else
#endif //CONFIG_HTTPD_HTTPS_SUPPORT
			{
				w = send(r->sd, out_buf+o, nbytes, 0);
				if (w < 0) {
					perror("send");

					goto done;
				}
			}

			nbytes-=w;
			o+=w;
			total_bytes_sent += w;     /* Patch B18 */
		}
	}

done:

	//tmr_cancel(r->cookie);
	ht_dbg("[%d] %ld transferred...\n", r->index, total_bytes_sent);


	return total_bytes_sent;
}


int find_script(register struct request_rec *r)
{
	char *ct_backup;
	int clen, rlen;
	char dummy_body[8];

	//ht_dbg("[%d] content_type=[%s]\n", r->index, r->content_type);


	/* backup r->content_type */
	ct_backup = r->content_type;

	/* probe_content_type will make r->content_type point to an array, e.g, CGI_MAGIC_TYPE if POST a cgi */
	probe_content_type(r);

	if (!strcmp(r->content_type, CGI_MAGIC_TYPE)) {
		/* restore r->content_type */
		r->content_type = ct_backup;
		exec_cgi_script(r);
		return 1;
	}

	/*
	pete_zhang	ticket-1492	2014-01-16
	bug description: Backup glbcfg would be failed if some data
			 followed with HTTP header in a POST request.
	*/

	/*
	 bypass some useless body in the POST message, in case some unimplemented POST request

	 for exmaple:
	 "POST /tmp/xxxxx.cfg HTTP/1.1"
	 ......

	 httoken=764056105&page=tools_gateway&logout=
	*/
	clen = r->content_length;

	while (clen > 0) {
		if (clen > 8)
			rlen = 8;
		else
			rlen = clen;

		ht_dbg("read dummy...., len to be read %d\n", clen);
#ifdef CONFIG_HTTPD_HTTPS_SUPPORT
		if (r->ishttps)
			rlen = as_read(r->ssrv, dummy_body, rlen);
		else
#endif //CONFIG_HTTPD_HTTPS_SUPPORT
		rlen = recv(r->sd, dummy_body, rlen, MSG_DONTWAIT);
		//recv return -1 for error, 0 for the peer has performed an orderly shutdown
		if (rlen <= 0) {
			break;
		}
		clen -= rlen;
	}
	/*end pete_zhang	2013-12-20*/

	printf("find script failed\n");
	return 0;
}

#ifdef CONFIG_NO_INTERNET_REDIRECT_SUPPORT
int wanstatus()
{
	FILE *fp;
	if (fp=fopen("/tmp/wanup", "r")) {
		fclose(fp);
		return 1;
	}
	return 0;
}
#endif
#if defined(VR9517PAC22_A_PP) || defined(AR7516AOW22_A_PP)	//for PLDT project
void init_traffic_data_file()
{
	char filename[32] = "/tmp/realtime";
	FILE* fp = NULL;

	fp = fopen(filename, "w+");
	if(fp == NULL)
		return;
	
	fputs("0\n0", fp);
	fclose(fp);

	return;
}
#endif

void toLowerLetter(char *src, char *dst)
{
	int i = 0;
	for(i = 0; i < strlen(src); i ++)
	{
		if(src[i] >= 65 && src[i] <= 90)
			dst[i] = src[i] + 32;
		else
			dst[i] = src[i];
	}

	return;
}

void process_request(register struct request_rec *r) 
{
	char *line, *http_ver, *tmp_url;
#if defined(CONFIG_HTTPD_Security_Attack_Protect) || defined(WE410443_SA) || defined(WE5202243_SA) || defined(WE410223_SA) || defined(WE410443_TS) || defined(WE410443_TA) || defined(WE410443_A1) || defined(WE410443_6DX) || defined(WE410443_ZZ)
	int is_attack_flag = 0;
#endif

	r->in_buf_read = _getword(r->in_buf, CR);
	line = r->in_buf;

	r->status = -1;
	r->bytes_sent = -1;

	/*
	 * Format: Request-Line = Method SP Request-URI SP HTTP-Version CRLF
	 *
	 * Request-URI = abs_path?para_name1=value1&para_name2=value2 ...
	 * SP = space char
	 */

	/* GET /photos/000.jpg HTTP/1.1 */
	tmp_url = _getword(line, ' ');
	r->method = line;
	http_ver = _getword(tmp_url, ' ');
	r->args = _getword(tmp_url, '?');

	strncpy(r->url, tmp_url, MAX_STRING_LEN-1);
	r->url[MAX_STRING_LEN-1] = '\0';

	/* plustospace(url); */
	unescape_url(r->url);

	ht_dbg("url=[%s], args=[%s], method=[%s]\n", r->url, r->args, r->method);
#if defined(VR9517PAC22_A_PP) || defined(AR7516AOW22_A_PP)	//for PLDT project
	if(strncmp(r->url, "/traffic_monitor_real", 21) == 0)
		init_traffic_data_file();
#endif

	if (clear_vars(r) < 0) {
		return;
	}

	if (http_ver[0] != '\0') {              /* HTTP 1.0 or newer */
		r->assbackwards = 0;

		if (get_mime(r) < 0)
			return;

#if defined(WE410443_SA) || defined(WE5202243_SA) || defined(WE410223_SA) || defined(WE410443_TS) || defined(WE410443_TA) || defined(WE410443_A1) || defined(WE410443_6DX) || defined(WE410443_ZZ)
	{
		FILE *fp;
		char string[32];
		fp = fopen("/proc/br_externed_filter/ip", "r");
		int masterredirect = 0;
		int len;
		
		if(fp != NULL)
		{
			memset(string,0,32);
			if(fgets(string, sizeof(string), fp))
			{	
				len = strlen(string);
				if(string[len-1] == 0xA)
					string[len-1] = 0;
				if((strcmp(string,r->server_ipstr) != 0) && (strlen(string) >= 7) && (strcmp(string,"0.0.0.0") != 0))
				{
					masterredirect = 1;
				}
			}	
			fclose(fp);
//A1  redirect to https for GUI
/*#if defined(WE410443_A1)  
	if (r->ishttps){
			;
		}
		else	
			{
			int tid;
			char urlbuf[32] = {0};
			if ((tid = get_tid()) == MID_FAIL) {
				ht_dbg("[%s] Failed to connect to midcore\n", __FUNCTION__);
				return;
			}

			
			char dnshandle_url[32] = {0};
			mapi_ccfg_get_str(tid, "ARC_SYS_DNSHANDLE_URL", dnshandle_url, sizeof(dnshandle_url));			
			sprintf(r->url, "https://%s", dnshandle_url);
			answer(r, REDIRECT, r->url);
			return ;
			}
	
#endif
*/
			if(masterredirect)
			{
#ifdef CONFIG_HTTPD_HTTPS_SUPPORT
				if (r->ishttps)
					sprintf(r->url, "https://%s", string);
				else
#endif
					sprintf(r->url, "http://%s", string);

				answer(r, REDIRECT, r->url);
				return;			
			}
		}
	}
#endif

#if defined(WE410443_SA) || defined(WE5202243_SA) || defined(WE410223_SA) || defined(WE410443_TS) || defined(WE410443_TA) || defined(WE410443_6DX) || defined(WE410443_ZZ) || defined(WE410443_A1)
		int tid;
		char urlbuf[32] = {0};
		if ((tid = get_tid()) == MID_FAIL) {
			ht_dbg("[%s] Failed to connect to midcore\n", __FUNCTION__);
			return;
		}

#if defined(WE410443_TS) || defined(WE410443_TA) || defined(WE410443_A1)
		char lang[8] = {0};
		char dnshandle_url[32] = {0};
		char dnshandle_url2[32] = {0};

		mapi_ccfg_get_str(tid, "ARC_SYS_Language_Auto", lang, sizeof(lang));
		mapi_ccfg_get_str(tid, "ARC_SYS_DNSHANDLE_URL", dnshandle_url, sizeof(dnshandle_url));

		toLowerLetter(dnshandle_url, dnshandle_url2);
#endif
#if defined(WE410443_TS)
		if(atoi(lang) == 1)
		{
			if(r->accept_lang != NULL)
			{
				if(strstr(r->accept_lang, "en"))
					mapi_ccfg_set_str(tid, "ARC_SYS_Language", "0");
				else if(strstr(r->accept_lang, "fr"))
					mapi_ccfg_set_str(tid, "ARC_SYS_Language", "2");
				else
					mapi_ccfg_set_str(tid, "ARC_SYS_Language", "0");
			}
			else
				mapi_ccfg_set_str(tid, "ARC_SYS_Language", "0");
		}
		else
			mapi_ccfg_set_str(tid, "ARC_SYS_Language", lang);
#endif

#if defined(WE410443_TS) || defined(WE410443_TA)|| defined(WE410443_A1)
		if ((!mapi_ccfg_match_str(tid, "ARC_LAN_0_IP4_Addr", r->host)) 
			&& (!mapi_ccfg_match_str(tid, "ARC_SYS_DNSHANDLE_URL", r->host)) && strcmp(dnshandle_url2, r->host))
#else
		if ((!mapi_ccfg_match_str(tid, "ARC_LAN_0_IP4_Addr", r->host)) 
			&& (!mapi_ccfg_match_str(tid, "ARC_SYS_DNSHANDLE_URL", r->host)))
#endif
		{
			mapi_ccfg_get_str(tid, "ARC_SYS_DNSHANDLE_URL", urlbuf, sizeof(urlbuf));
#ifdef CONFIG_HTTPD_HTTPS_SUPPORT
			if (r->ishttps)
				sprintf(r->url, "https://%s/internet_paused.htm", urlbuf);
			else
#endif
				sprintf(r->url, "http://%s/internet_paused.htm", urlbuf);

			answer(r, REDIRECT, r->url);
			return;
		}
#endif
	
#ifdef CONFIG_NO_INTERNET_REDIRECT_SUPPORT
		int tid;
		if ((tid = get_tid()) == MID_FAIL) {
			ht_dbg("[%s] Failed to connect to midcore\n", __FUNCTION__);
			return;
		}

		if (!mapi_ccfg_match_str(tid, "ARC_LAN_0_IP4_Addr", r->host)  && !wanstatus())
		{
			strcpy(r->url, "http://192.168.1.1/no_internet_redirect.htm");
			answer(r, REDIRECT, r->url);
			return;
		}
#endif

		/* If the request is not for firmware upgrade, 
		 * and content-length is very large, just reject this requests */
		if (strncasecmp(r->content_type, form_type, strlen(form_type)) != 0 && 
				r->soapaction && !strcasestr(r->soapaction, "FirmwareUpload") &&
				r->content_length > MAX_CONTENT_LENGTH) {

			garbage_collect(r, r->sd, r->content_length);
			answer(r, FORBIDDEN, "The Content-length is extreme large!");
			return;
		}
	}
	else {
		r->assbackwards = 1;    /* HTTP 0.9 */
	}

#if defined(CONFIG_HTTPD_Security_Attack_Protect) || defined(WE410443_SA) || defined(WE5202243_SA) || defined(WE410223_SA) || defined(WE410443_TS) || defined(WE410443_TA) || defined(WE410443_A1) || defined(WE410443_6DX) || defined(WE410443_ZZ)
	is_attack_flag = attack_check(r->url);
#endif

	/* BYPASS CHECK */
	r->bypass_flag = bypass_check(r->url);

	/* vendor hook */
	if (v_ops->request_hook) {
		switch (v_ops->request_hook(r)) {

			case V_ACCEPT:
				break;
			case V_DROP:
				return;
			default:
				break;
		}
	}

#if defined(WE410443_SA) || defined(WE5202243_SA) || defined(WE410223_SA) || defined(WE410443_TS) || defined(WE410443_TA) || defined(WE410443_A1) || defined(WE410443_6DX) || defined(WE410443_ZZ)
	if (is_attack_flag != 1)
	{
		//invalid URL
		cprintf("[%s_%d]%s  is invalid!!!",__FUNCTION__,__LINE__, r->url);
		return;
	}	
#endif
#ifdef CONFIG_HTTPD_Security_Attack_Protect
	if (is_attack_flag != 1)
	{
#ifdef CONFIG_HTTPD_Security_Attack_Protect
		check_addr_is_attacker(r->remote_ip);
#endif		
		return;
	}
#endif
	/* bypass_flag probably changed by vendor */
	if (!r->bypass_flag) {
		if (evaluate_access(r->url, 0, r))
			return;
	}
	
	r->header_only = 0;

	if (!strcmp(r->method, "HEAD")) {
		r->header_only = 1;

		if (r->assbackwards) {
			r->header_only = 0;
			answer(r, BAD_REQUEST, "Invalid HTTP/0.9 method.");
			return;
		}
		process_get(r);
	}
	else if (!strcmp(r->method, "GET")) {
		process_get(r);
	}
	else if (!strcmp(r->method, "POST")) {
		process_post(r);
	}
	else {
		answer(r, BAD_REQUEST,"Invalid or unsupported method.");
		return;
	}

	return;
}

