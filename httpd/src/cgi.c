#include "cgi.h"
#include "httoken.h"
#include "util.h" //parseCfgIdx,arccfgFormat

#include <stdio.h> //system

#ifdef SYSLOG_ENHANCED
#include "arc_syslog.h"
#endif
/*
 * CGI Table: translate script's filename to its task function, all tasks can
 *      be assigned a different stack size, respectively
 */
extern const CGI_ENTRY cgi_tbl[];

input_t inputs[MAX_CGI_VARS];
int input_num = 0;

int __init_cgi(CGI_info *p, input_t *inputs, int max_num) 
{
	int i,x;
	int len,idx, isMSIE=0, ret;
	char *getstr;
	char *judgestr;
	char *stop;
	char *buf;
	fd_set readfds;
	struct timeval timeouts;
	struct request_rec *r = p->r;

	if (max_num==0) {
		ht_dbg("Inputs data structure number not available\n");
	}

	//judgestr = _getenv("REQUEST_METHOD", env);
	judgestr = r->method;
	if (!judgestr)
		return 0;

	if (strcmp(judgestr,"GET") == 0)
	{
		getstr = r->args;

		if (!getstr) return 0;

		for (x=0; *getstr != '\0'; x++)
		{
			if (max_num!=0 && max_num-1<=x) {
				ht_dbg("GET Out of Inputs array \n");
				return 0 ;
				//break;
			}
			inputs[x].name = makeword(getstr,'&');
			plustospace(inputs[x].name);
			unescape_url(inputs[x].name);
			if (!(stop = strchr(inputs[x].name,'=')))
				inputs[x].val = NULL;
			else {
				*stop++ = '\0';
				inputs[x].val = stop;
			}
		}
	}
	else if (strcmp(judgestr,"POST") == 0)
	{
		len = r->content_length;
		if (p->r->user_agent!=NULL && strstr(p->r->user_agent, "MSIE ")!=NULL) {
			isMSIE = 1;
		}

		if (!(buf = malloc(len+1)))
			return 0;

		/* getline(p->r, buf, len+1); */
		idx=0;

#ifdef CONFIG_HTTPD_HTTPS_SUPPORT
		if (p->r->ishttps) {
			int in_header_lens, in_buf_used;
			int cnt;

			in_header_lens = p->r->in_header_lens;
			in_buf_used = p->r->in_buf_used;
			cnt = in_buf_used-in_header_lens;
			memcpy(buf, p->r->in_buf+in_header_lens, cnt);

			buf[cnt] = '\0';
			idx = cnt;
			len -= cnt;
		}
#endif

		/* wait for data */
		timeouts.tv_sec = 0;
		timeouts.tv_usec = 5000;
		while (len > 0)
		{
#ifdef CONFIG_HTTPD_HTTPS_SUPPORT
			if (p->r->ishttps) {
				i = as_read(p->r->ssrv, buf+idx, len);
			}
			else
#endif
			{
				FD_ZERO(&readfds);
				FD_SET(p->socketid, &readfds);
				ret = select(p->socketid+1, &readfds, NULL, NULL, &timeouts);
				if (ret <= 0) {
					// time out or select error
					return 0;
				}
				// data come in
				i = recv(p->socketid, buf+idx, len, MSG_DONTWAIT);
			}

			if (i<=0) {
				return 0;
			}
			if (i < len)
				len=len-i;
			else
				len = 0;
			idx+=i;
		}

		buf[idx] = '\0';

	 	for (x=0; buf[0] != '\0'; x++)
		{
			if (max_num!=0 && max_num-1<=x) {
				ht_dbg("POST Out of Inputs array. max_num=%d, x=%d\n", max_num, x );
				return 0;
				//break;
			}
			inputs[x].name = makeword(buf, '&');
			if(inputs[x].name == NULL)
			{
				printf("GetCGI() -> makeword() fail\n");
				return 0;
			}
			plustospace(inputs[x].name);
			unescape_url(inputs[x].name);
			if (!(stop = strchr(inputs[x].name,'=')))
				inputs[x].val = NULL;
			else {
				*stop++ = '\0';
				inputs[x].val = stop;
			}
		}

		FD_ZERO(&readfds);
		/* wait for data */
		timeouts.tv_sec = 0;
		timeouts.tv_usec = 5000;
		FD_SET(p->socketid, &readfds);
		ret = select(p->socketid+1, &readfds, NULL, NULL, &timeouts);
		if (ret > 0) {
			ret = recv(p->socketid, &idx, 2, MSG_DONTWAIT);
			printf("there are two extra bytes from client\n");
		}
	 }
	 else
	     return 0;

	 inputs[x].name=NULL;
	 return x;
}


char *__get_cgi(input_t *inputs, char *field)
{
	int i;
	char *pos = NULL;

	for (i=0; inputs[i].name!=NULL; i++)
	{
		if (strcmp(field, inputs[i].name) == 0)
		{
			if (!inputs[i].val || strlen(inputs[i].val)==0)
				pos = NULL;
			else 
				pos = inputs[i].val;

			break;
		}
	}

	return pos;
}

char *__get_cgi_df(char *field, char *sdf)
{
	char *ptr;

	if ( (ptr = __get_cgi(inputs, field)) == NULL)
		return sdf;

	return ptr;
}

int __destroy_cgi(input_t *inputs, int input_num)
{
	int i;
	int count = input_num;
	int left = input_num;

	if (count  > MAX_CGI_VARS)
		count = MAX_CGI_VARS;

	for (i=0; i<count ; i++) {
		//ht_dbg("input.name=[%s], input.val=[%s], left=[%d]\n", inputs[i].name, inputs[i].val, left);
		if (inputs[i].name) {
			left--;
			free(inputs[i].name);
			inputs[i].name = NULL;
		}

#if 0	/* tlhhh 2011-09-23. _val_ use part of memory which _name_ pointed to */
		if (inputs[i].val) {
			free(inputs[i].val);
			inputs[i].val = NULL;
		}
#endif
	}

	return left;
}


char *get_cgi(char *field)
{
	char *ptr = NULL;

	LOCK_CGI();
	ptr = __get_cgi(inputs, field);
	UNLOCK_CGI();

	return ptr;
}

char *get_cgi_df(char *field, char *sdf)
{
	char *ptr;

	if ( (ptr = get_cgi(field)) == NULL)
		return sdf;

	return ptr;
}

int is_std_var(const char *name)
{
	if (!name)
		return 0;

	if (strncmp(name, STD_PREFIX, strlen(STD_PREFIX))) 
		return 0;

	/* try read through glb.cfg? */

	return 1;
}


/* CGI handler */
void apply_cgi(int argc, char **argv, void *p)
{
	CGI_info *ci = (CGI_info *)p;
	struct request_rec *r = ci->r;
	int idx = -1, act_idx = -1, param_idx = -1,enc=0;
	int i, tid = MID_FAIL;
	char next_page[128];
	char buf[256];
	int ret=0,sz=0;

	memset(buf, 0, sizeof(buf));
	 
	ht_dbg("r->url = %s\n", r->url);

	//memset(next_page, 0, sizeof(next_page)); // no used
	sprintf(next_page, HTTP_APPLY_SUCCESS);

	/* Destory last cgi table */
	if ((input_num = __destroy_cgi(inputs, input_num)) != 0)
		ht_dbg("warning: %d entries have not been freed, memory leaking.. \n", input_num);

	input_num = __init_cgi(ci, inputs, sizeof(inputs)/sizeof(input_t));

	if ( input_num <= 0 ) {
		sprintf(next_page, "%s",  HTTP_APPLY_FAIL); //"Fail");
		goto mapi_done;
	}

	/*
 	pete_zhang 2013-12-20	ticket-1167

	description: GUI security enhancement
 	*/
#ifdef CONFIG_HTTPD_TOKEN_CHECK_SUPPORT
	if ( !cgi_token_pass(ci, inputs, 0) ) //failed to pass token checking
	{
		goto mapi_done;
	}
#endif //CONFIG_HTTPD_TOKEN_CHECK_SUPPORT
	/*end pete_zhang	2013-12-20*/

	tid = get_tid();
	if (tid == MID_FAIL) {
		ht_dbg( "Failed to start a transaction to arc middle\n" );
		sprintf(next_page, "%s",  HTTP_APPLY_FAIL); //""Fail");
		goto mapi_done;
	}


	for (i=0; i<input_num; i++) {
		
		//ht_dbg("input.name=[%s], input.val=[%s]\n", inputs[i].name, inputs[i].val);
		if (!inputs[i].name || !inputs[i].val) {
			sprintf(next_page, "%s",  HTTP_APPLY_FAIL); //""Fail");
			goto apply_done;
		}

		if (!strcmp(inputs[i].name, "submit_button"))
			idx = i;
		else if (!strcmp(inputs[i].name, "next_page"))
			snprintf(next_page, sizeof(next_page), "%s", inputs[i].val);
		else if (!strcmp(inputs[i].name, "action"))
			act_idx = i;
		else if (!strcmp(inputs[i].name, "action_params"))
			param_idx = i;
		else if (inputs[i].name[0] != '\0') {
			sz=strlen(inputs[i].name);
#ifdef CONFIG_HTTPD_SUPPORT_AES256
			if(inputs[i].name[sz-1] == '*')
			{
				enc=1;
				inputs[i].name[sz-1]='\0';
			}
#endif //CONFIG_HTTPD_SUPPORT_AES256
			/* if _not_ standard var, always skip */
			if (is_std_var(inputs[i].name) == 0)
				continue;

			if (skip_var(&inputs[i]))
				continue;

			// check if need decode, special NOTE: here browser send a %xx format if encrypted, 
			// Arc_decode() need do unescape_url() first
			if( enc) {
				Arc_decode(r,(unsigned char*)inputs[i].val);
			}

			ret = hack_var(tid, &inputs[i]);
			if(ret < 0){
				ht_dbg("Try to hack var [%s], but failed, continue\n", inputs[i].name);
				continue;
			}
			else if(ret > 0)
			{
				ret = mapi_ccfg_set_str(tid, inputs[i].name, inputs[i].val);
			}

			if (ret != 0){
				sprintf(next_page, "%s", HTTP_APPLY_FAIL); //""Fail");
				goto apply_done;	/* fail */
			}
		}
	}

	/* done, notify MNG the action */
	if (act_idx < 0) {
		//mng_action(NOTHING, NOTHING);
	}
	else if (param_idx < 0) {
		mng_action(inputs[act_idx].val, NOTHING);
	}
	else {
		mng_action(inputs[act_idx].val, inputs[param_idx].val);
	}

	mapi_ccfg_commit(tid);
	//mng_action("commit", NOTHING);

apply_done:
	/*
	 * Alpha Liu 2012.11.27 issue for PR711AAW
	 * Issue description: httpd send an 302 redirect response
	 *										with wrong location url to browser.
	 * Solution: Only send 200 ok response when page_suffix do not set.
	*/
	if (v_ops->page_suffix != NULL) {
		//snprintf(buf, sizeof(buf), "Location: /%s%s\n\n", next_page[0]=='\0' ? "Success" : next_page, v_ops->page_suffix);
		snprintf(buf, sizeof(buf), "Location: /%s\n\n", next_page);
		send_response(r, buf);
	}
	else
		answer(r, DOCUMENT_FOLLOWS, NOTHING); //only send 200 ok
	/* end Alpha Liu 2012.11.27 issue for PR711AAW */

mapi_done:
	if (argv)
		free(argv);
	
	pthread_exit((void *)CGI_CODE);

}
extern void send_file(register struct request_rec *r);

void upload_cgi(int argc, char **argv, void *p)
{
	CGI_info *ci = (CGI_info *)p;
	struct request_rec *r = ci->r;
	int len;
	//crazy_liang 2013.3.12: For calculating the correct file size.
	char *boundary = NULL;
	int tail_len = 0;
	//end crazy_liang 2013.3.12
	char buf[1024];
	int sock;
	int type = TP_DEFAULT, ret_code = -1;
	FILE *stream = NULL;
	int x = 0;
	int tid = MID_FAIL;
#if defined(WE410443_SA) || defined(WE5202243_SA) || defined(WE410223_SA) || defined(WE410443_TS) || defined(WE410443_TA) || defined(WE410443_A1) || defined(WE410443_6DX) || defined(WE410443_ZZ)
	int resetFlag = 0;
	int update_time = -1;
#endif

#if 0 //KILL ME, 2014/4/8 GZ henry confirmed no body used it
	char type_buf[4]="";
#endif //KILL_ME

	sock = ci->socketid;	
	len = r->content_length;

	if ((input_num = __destroy_cgi(inputs, input_num)) != 0)
		ht_dbg("warning: %d entries have not been freed, memory leaking.. \n", input_num);

	stream = fdopen(sock, "r");
	if (!stream) {
		goto upload_cgi_done;
	}

	//moved from below by Alpha 2013-10-23
	tid = get_tid();
	if (tid == MID_FAIL) {
		ht_dbg( "Failed to start a transaction to arc middle\n" );
		goto upload_cgi_done;
	}
#if 0 //KILL_ME 2014/4/8 GZ henry confirmed no body used it
	/* 
	 * if TR069 set the upgrade only managed by ACS server,
	 * httpd could not do the upgrade firmware action, just return
	 * 2013-10-23 by Alpha
	 */
	if(mapi_ccfg_match_str(tid, "ARC_SYS_HTTP_UpgradeDisabled", "1"))
	{
		goto done;	
	}
#endif //KILL_ME

	/* get enough memory as possible as we can */
#if defined(ARC_CODE_SHRINK)
	system("/sbin/arc_web_freemem");
#else
#if defined(AR7516AOW22_A_PP)
	mng_action("sys_release_memory", NOTHING);
#endif
#endif
	sleep(2);

	while (len > 0) {
		
		memset(buf, 0, sizeof(buf));

		if (!get_line(r, buf, MIN(len + 1, sizeof(buf)), stream)) {
			goto upload_cgi_done;
		}

		len -= strlen(buf);

		if (!strncasecmp(buf, "Content-Disposition:", 20)) {
			char *ptr, *stop;
			int idx;

			if (strstr(buf, "name=\"httoken\""))
			{
				for (idx=0; idx<2 && len>0; idx++) {

					memset(buf, 0, sizeof(buf));

					/* eat a line */
					if (!get_line(r, buf, MIN(len + 1, sizeof(buf)), stream))
						goto upload_cgi_done;

					len -= strlen(buf);
				}
				
				ptr = malloc(100);
				if (!ptr)
					goto upload_cgi_done;

				snprintf(ptr, 100, "httoken=%lu", atol(buf));

				inputs[x].name = ptr;
				plustospace(inputs[x].name);
				unescape_url(inputs[x].name);

				if (!(stop = strchr(inputs[x].name, '=')))
					inputs[x].val = NULL;
				else {
					*stop++ = '\0';
					inputs[input_num].val = stop;
				}

				ht_dbg("name=%s, val=%s\n", inputs[x].name, inputs[x].val);
				x++;
			}
#if defined(WE410443_SA) || defined(WE5202243_SA) || defined(WE410223_SA) || defined(WE410443_TS) || defined(WE410443_TA) || defined(WE410443_A1) || defined(WE410443_6DX) || defined(WE410443_ZZ)
			else if (strstr(buf, "name=\"reset\""))
			{
				for (idx=0; idx<2 && len>0; idx++) {

					memset(buf, 0, sizeof(buf));

					/* eat a line */
					if (!get_line(r, buf, MIN(len + 1, sizeof(buf)), stream))
						goto upload_cgi_done;

					len -= strlen(buf);
				}
				if(strlen(buf) != 0)
					resetFlag = atoi(buf);
			}
			else if (strstr(buf, "name=\"update_time\""))
			{
				for (idx=0; idx<2 && len>0; idx++) {

					memset(buf, 0, sizeof(buf));

					/* eat a line */
					if (!get_line(r, buf, MIN(len + 1, sizeof(buf)), stream))
						goto upload_cgi_done;

					len -= strlen(buf);
				}
				if(strlen(buf) != 0)
					update_time = atoi(buf);
			}
#endif
			else if (strstr(buf, "name=\"wait_time\""))
			{
				for (idx=0; idx<2 && len>0; idx++) {

					memset(buf, 0, sizeof(buf));

					/* eat a line */
					if (!get_line(r, buf, MIN(len + 1, sizeof(buf)), stream))
						goto upload_cgi_done;

					len -= strlen(buf);
				}
				
				ptr = malloc(100);
				if (!ptr)
					goto upload_cgi_done;

				snprintf(ptr, 100, "wait_time=%d", atoi(buf));

				ht_dbg("ptr=%s\n", ptr);
				inputs[x].name = ptr;
				plustospace(inputs[x].name);
				unescape_url(inputs[x].name);

				if (!(stop = strchr(inputs[x].name, '=')))
					inputs[x].val = NULL;
				else {
					*stop++ = '\0';
					inputs[input_num].val = stop;
				}

				ht_dbg("name=%s, val=%s\n", inputs[x].name, inputs[x].val);
				x++;
			}
			else if (strstr(buf, "name=\"file\"")) {
#ifdef WEB_GUI_UPGRADE_ROM_SUPPORT
				if(strstr(buf, ".rom"))
				{
					cprintf("******** %s-%d: Special FW allows upgrade ROM via Web GUI\n", 
							__func__, __LINE__);
					type = TP_UPGRADE_ROM; 
				}
				else
#endif
				type = TP_UPGRADE;
				break;
			}
			else if(strstr(buf, "name=\"restore\"")) {
				type = TP_RESTORE;
				break;
			}
#ifdef CONFIG_HTTPD_LTE_UPGRADE
			// hugh 2014/1/24: extend for LTE firmware upgrade
			else if(strstr(buf, "name=\"ltefile\"")) {
				type = TP_UPGRADE_LTE;
				break;
			}
#endif  //CONFIG_HTTPD_LTE_UPGRADE
#ifdef CONFIG_HTTPD_PLC_UPGRADE 
			// runsen_lao 2014/12/18: extend for PLC firmware upgrade
			 else if(strstr(buf, "name=\"plcfile\"")) {
				type = TP_UPGRADE_PLC;
				break;
			}
#endif  //CONFIG_HTTPD_PLC_UPGRADE
#ifdef CONFIG_HTTPD_MTK_UPGRADE 
			// roger 2016/01/04: extend for mtk uboot  firmware upgrade
			 else if(strstr(buf, "name=\"mtkfile\"")) {
				type = TP_UPGRADE_MTK;
				break;
			}
#endif  //CONFIG_HTTPD_MTK_UPGRADE
			else if(strstr(buf, "name=\"owlfile\"")) {
				type = TP_UPGRADE_OWL;
				break;
			}
			else if(strstr(buf, "name=\"owlfile_auto\"")) {
				type = TP_UPGRADE_OWL_AUTO;
				break;
			}
		}
	}

	/*
 	pete_zhang 2013-12-20	ticket-1167

	description: GUI security enhancement
 	*/
#ifdef CONFIG_HTTPD_TOKEN_CHECK_SUPPORT
	if ( !cgi_token_pass(ci, inputs, 0) ) //failed to pass token checking
	{
		goto upload_cgi_done;
	}
#endif //CONFIG_HTTPD_TOKEN_CHECK_SUPPORT
	/*end pete_zhang	2013-12-20*/

	/*
 	  libra_zheng 2013.04.08: fix bug about upload failed for MRL7016AW-22-AC
	  Bug description: the description on the webpage should be different when upgrade firmware failed and upload previous setting failed
	  Solution: get the upload type when upload file, instead of get it only at the time of upload right file,setting value of "VAR_SYSTEM_UPLOAD_TYPE" here will avoid "VAR_SYSTEM_UPLOAD_TYPE" cann't get a right value when upload incorrect file.
	*/
	/* set action to notify MNG */
	/* move to upper by Alpha 2013-10-23
	tid = get_tid();
	if (tid == MID_FAIL) {
		ht_dbg( "Failed to start a transaction to arc middle\n" );
		goto upload_cgi_done;
	}
	*/

#if 0 //KILL ME, 2014/4/8 GZ henry confirmed no body used it
	/* store this variable to distinguish upload type */
	snprintf(type_buf, sizeof(type_buf), "%d", type);
	mapi_tmp_set(tid, "VAR_SYSTEM_UPLOAD_TYPE", type_buf);
	/* end libra_zheng 2013.04.08*/
#endif

	input_num = x;

	memset(buf, 0, sizeof(buf));

	/* Skip boundary and headers */
	while (len > 0) {
		if (!get_line(r, buf, MIN(len + 1, sizeof(buf)), stream))
			goto upload_cgi_done;

		len -= strlen(buf);
		if (!strcmp(buf, "\n") || !strcmp(buf, "\r\n"))
			break;
	}

	//crazy_liang 2013.3.12: For calculating the correct file size.
	if((boundary = strcasestr(r->content_type, "boundary=")) != NULL)
	{
		//Boundary was found.
		boundary += strlen("boundary=");

		//Please see RFC2616: 
		//boundary=THIS_STRING_SEPARATES
		//The tail is: "\r\n--THIS_STRING_SEPARATES--\r\n"
		tail_len = 2 + 2 + strlen(boundary) + 2 + 2;

#if defined(WE410443_SA) || defined(WE5202243_SA) || defined(WE410223_SA) || defined(WE410443_TS) || defined(WE410443_TA) || defined(WE410443_A1) || defined(WE410443_6DX) || defined(WE410443_ZZ)
		//if the resetFlag checkbox is checked, need remove this length too.
		if(resetFlag == 1)
		{
			tail_len += 53+ 2 + strlen(boundary) + 2 + 2;
			cprintf("resetlen=%d\n", 53+2+strlen(boundary) + 2 + 2);
		}
#endif

		//Remove the tail length first. 
		len -= tail_len;

		cprintf("The tail length is %d! Update len to %d!\n", tail_len, len);
	}
	//end crazy_liang 2013.3.12

	if (type == TP_UPGRADE) {
		ret_code = sys_upgrade(r, stream, &len, 0);
		if (ret_code < 0)
			goto upload_cgi_done;
	}
#ifdef WEB_GUI_UPGRADE_ROM_SUPPORT
	else if (type == TP_UPGRADE_ROM) {
		ret_code = sys_upgrade(r, stream, &len, 1);
		if (ret_code < 0)
			goto upload_cgi_done;
	}
#endif
	else if (type == TP_RESTORE) {
		ret_code = sys_restore(r, USER_ROUTER_CONF, stream, &len);
		if (ret_code < 0)
		{
#ifdef SYSLOG_ENHANCED
//	SetSystemLog(LOG_TYPE_OTHER, LOG_LEVEL_TYPE_INFO, LOG_MESSAGE_TYPE_DEFAULT, "HTTP start ...");
	SetSystemLog(LOG_TYPE_APP, LOG_LEVEL_TYPE_ERROR, LOG_APP_MESSAGE_TYPE_http, "UD103");
#endif
			goto upload_cgi_done;
		}
	}
#ifdef CONFIG_HTTPD_LTE_UPGRADE
	// hugh extend for LTE upgrade
	else if (type == TP_UPGRADE_LTE) {
		ret_code = sys_lte_upgrade(r, stream, &len);
		if (ret_code < 0)
			goto upload_cgi_done;
	}
#endif //CONFIG_HTTPD_LTE_UPGRADE
#ifdef CONFIG_HTTPD_PLC_UPGRADE
	// runsen_lao extend for PLC upgrade
	else if (type == TP_UPGRADE_PLC) {
		ret_code = sys_plc_upgrade(r, stream, &len);
		if (ret_code < 0)
			goto upload_cgi_done;
	}
#endif //CONFIG_HTTPD_PLC_UPGRADE
#ifdef CONFIG_HTTPD_MTK_UPGRADE
	// runsen_lao extend for PLC upgrade
	else if (type == TP_UPGRADE_MTK) {
		ret_code = sys_mtk_upgrade(r, stream, &len, 0);
		if (ret_code < 0)
			goto upload_cgi_done;
	}
#endif //CONFIG_HTTPD_MTK_UPGRADE
#if defined(WE410443_SA) || defined(WE5202243_SA) || defined(WE410223_SA) || defined(WE410443_TS) || defined(WE410443_TA) || defined(WE410443_A1) || defined(WE410443_6DX) || defined(WE410443_ZZ)  //BT: load defalt when set resetFlag
	else if (type == TP_UPGRADE_OWL) {
		ret_code = sys_owl_upgrade(r, stream, &len, 0, resetFlag);
#ifdef SYSLOG_ENHANCED
		if(ret_code != 0)
			SetSystemLogFmt(LOG_TYPE_APP, LOG_LEVEL_TYPE_ERROR, LOG_APP_MESSAGE_TYPE_http, "HTTP-Firmware Upgrade Failed from IP %s.", r->remote_ipstr);
#endif
		goto cgi_done;
	}
	else if (type == TP_UPGRADE_OWL_AUTO) {
		ret_code = sys_owl_upgrade_autofw(r, 0, update_time);
#ifdef SYSLOG_ENHANCED
		//if(ret_code != 0)
			//SetSystemLogFmt(LOG_TYPE_APP, LOG_LEVEL_TYPE_ERROR, LOG_APP_MESSAGE_TYPE_http, "HTTP-Firmware Upgrade Failed from IP %s.", r->remote_ipstr);
#endif
		goto cgi_done;
	}
#endif
	//crazy_liang 2013.3.12: For calculating the correct file size.
	if(tail_len > 0)
	{
		//Recover the tail length. 
		len += tail_len;
	}
	//end crazy_liang 2013.3.12

	if (type == TP_RESTORE || type == TP_DEFAULT)
	{
		if (type == TP_RESTORE)
		{
			ret_code = decrypt_conf_file(USER_ROUTER_CONF, "/tmp/glbcfg");

			if ((ret_code == 0) && (0 == access("/tmp/glbcfg",0))) {

				ht_dbg( "start mapi_ccfg_restore\n" );
				ret_code = mapi_ccfg_restore(tid, "/tmp/glbcfg");
				ret_code = mapi_tmp_set(tid, "VAR_RESTORE_BACKUP_FLAG", "0");
#ifdef SYSLOG_ENHANCED
	//Properties are successfully restored from a file.
	SetSystemLog(LOG_TYPE_APP, LOG_LEVEL_TYPE_INFO, LOG_APP_MESSAGE_TYPE_http, "S102");
#endif
			}
			else {
				ht_dbg( "fail to restore\n" );
				ret_code = -1;
#ifdef SYSLOG_ENHANCED
	//Properties are successfully restored from a file.
	SetSystemLog(LOG_TYPE_APP, LOG_LEVEL_TYPE_ERROR, LOG_APP_MESSAGE_TYPE_http, "S007");
#endif
				goto upload_cgi_done;
			}
		}
		else
			ret_code = mapi_ccfg_restore(tid, DEFAULT_ROUTER_CONF);

		ret_code = mapi_ccfg_commit(tid);
	}
	
	/* Slurp anything remaining in the request */
	while (len--)
		get_char(r, stream);

upload_cgi_done:
	if (argv)
		free(argv);

	// FIXME & NOTE:
	//    2013/12/19: hugh
	//      we no need those dirty code to hard set and no rule to identify which problem we are.....
	//   ITEM      OK               FAIL
	//  ========== ================ ================
	//   Restore   Success_restore  Fail_restore
	//   Upgrade   Success_upgrade  Fail_upgrade
	//   Apply     Success          Fail
	//
	// 2014/1/24: hugh extend old DUT's upgrade+ LTE upgrade  fail message 
#if 0
	if ((type == TP_UPGRADE) || (type == TP_UPGRADE_LTE) || (type == TP_UPGRADE_PLC))
	{
		snprintf(buf, sizeof(buf), "Location: /%s%s\n", ret_code==0 ? "Success_upgrade" : "Fail_upgrade", v_ops->page_suffix);
	}
	else
	{
		snprintf(buf, sizeof(buf), "Location: /%s%s\n", ret_code==0 ? "Success_restore" : "Fail_restore", v_ops->page_suffix);
	}

	/*
	 * Alpha Liu 2012.11.27 issue for PR711AAW
	 * Issue description: httpd send an 302 redirect response
	 *										with wrong location url to browser.
	 * Solution: Only send 200 ok response when page_suffix do not set.
	*/
	if(v_ops->page_suffix != NULL) //only send the redirec response when suffix is set.
		send_response(r, buf);
	else
		answer(r, DOCUMENT_FOLLOWS, NOTHING); //only send 200 ok
	/* end Alpha Liu 2012.11.27 issue for PR711AAW */
	so_flush(r);
	//sleep(2);
#else
	if ((type == TP_UPGRADE) || (type == TP_UPGRADE_LTE) || (type == TP_UPGRADE_PLC))
	{
		snprintf(r->url, MAX_STRING_LEN, "/%s", (ret_code==0) ? HTTP_UPGRADE_SUCCESS : HTTP_UPGRADE_FAIL);
	}
	else
	{
		snprintf(r->url, MAX_STRING_LEN, "/%s", (ret_code==0) ? HTTP_RESTORE_SUCCESS : HTTP_RESTORE_FAIL);
	}
	// NOTE:
	//     send_node() about r->url MUST a absolute path e.g.: /www/xxx.htm, original url is xxx.htm only
	//     use translate_name() to add prefix "/www" before r->url content.
	//
	translate_name(r->url);

	send_file(r);

#endif

#ifdef VENDOR_BELKIN_SUPPORT
	/* Only when success, belkin will reboot. */
	if (ret_code == 0)
#endif

#if defined(VENDOR_ZZ_SUPPORT) || defined(VENDOR_VERIZON_SUPPORT)
	/* Only when success, device will reboot. */
	if (ret_code == 0)
#endif
	{
		/* reboot the system */

#ifdef PKG_SIP_CONFIG
		/* trigger brnsip to unregister all sip acc.	Misora, 2014-06-11	*/
		system("sip_config SYS_CMD_UNREG");
#endif

		mng_action("reboot", NOTHING);

		/* linghong_tan 2013.05.10
		 * wait 2 seconds, if we still here, 
		 * must be rootfs error, kill 1
		 */
		 
		/*	Misora, 2014-06-12	
			Enlarge sleep duration to 8s(2+6) for waiting brnsip unregistering sip account.		*/
		sleep(8);
		
		kill(1, SIGTERM); //Kill init to reboot. 
	}
cgi_done:
	if (argv)
		free(argv);
	pthread_exit((void *)CGI_CODE);
}

#ifdef EXTERN_CGI_SUPPORT
void free_env(char **env)
{
    int x;

    for(x=0;env[x];x++)
        free(env[x]);

    free(env);
}

char *add_env(char *name, char *value) 
{
    char *t, *tp;

    if ((t = (char *) malloc( strlen(name)+strlen(value)+2)) == NULL)
        return NULL;

    for (tp=t; (*tp = *name)!='\0'; tp++,name++);
    for (*tp++ = '='; (*tp = *value)!='\0'; tp++,value++);
    return t;
}

char *_getenv(char *name, char **env) 
{
    char *env_name;
    int i;

    while (*env != NULL) {
        env_name = *env++;
        i = 0;
        while(env_name[i]!='=') i++;
        env_name[i] = '\0';
        if (!strcmp(name, env_name)) {
            env_name[i++] = '=';
            return (env_name+i);
        }

        env_name[i] = '=';
    }
    return NULL;
}

void replace_env(char **env, char *name, char *value)
{
    register int i, len;

	for (i = 0, len = strlen(name); env[i]; i++) {
		if (strncmp(env[i], name, len) == 0) {
			free(env[i]);
			env[i] = add_env(name, value);
			break;
		}
	}

	return ;
}

char **new_env_tbl(int size)
{
	char **env;
    
	env = (char **) malloc((size+1)*sizeof(char *));
	memset(env, 0, sizeof(char *)*(size+1));

	return env;
}

char **fill_envs(struct request_rec *r, input_t *cgi_list, int cgi_size)
{
	int x = 0;
	int i;
	struct sockaddr_in  *s4;
#ifdef IPV6_SUPPORT
	struct sockaddr_in6 *s6;
#endif
	char buf[128];
	char **env;

	if ((env = new_env_tbl(MAX_ENV_SIZE)) == NULL) {
		perror("malloc");
		return NULL;
	}

	/* step 1: common vars */
	env[x++] = add_env("HOST", r->host);

	s4 = (struct sockaddr_in *)&r->server_ip.sa_in;
	inet_ntop(AF_INET, &s4->sin_addr, buf, sizeof(buf));
	env[x++] = add_env("SERVER_ADDR4", buf);

	s4 = (struct sockaddr_in *)&r->remote_ip.sa_in;
	inet_ntop(AF_INET, &s4->sin_addr, buf, sizeof(buf));
	env[x++] = add_env("REMOTE_ADDR4", buf);

#ifdef IPV6_SUPPORT
	s6 = (struct sockaddr_in6 *)&r->server_ip.sa_in6;
	inet_ntop(AF_INET6, &s6->sin6_addr, buf, sizeof(buf));
	env[x++] = add_env("SERVER_ADDR6", buf);

	s6 = (struct sockaddr_in6 *)&r->remote_ip.sa_in6;
	inet_ntop(AF_INET6, &s6->sin6_addr, buf, sizeof(buf));
	env[x++] = add_env("REMOTE_ADDR6", buf);
#endif

	sprintf(buf, "%d", r->server_port);
	env[x++] = add_env("SERVER_PORT", buf);
	env[x++] = add_env("DOCUMENT_ROOT", document_root);


	/* step 2: CGI vars */
	env[x++] = add_env("REQUEST_METHOD", r->method);
	if (!strcmp(r->method, "GET")) {
		env[x++] = add_env("QUERY_STRING", r->args);
	}

	for (i=0; i<cgi_size; i++) {
		if (x >= MAX_ENV_SIZE)
			break; 
		env[x++] = add_env(cgi_list[i].name, cgi_list[i].val);
	}

	env[x] = NULL;

	//r->envs = env;
	//r->env_size = x;

	return env;
}

int call_exec(int argc, char **argv, char **env, struct request_rec *r, BIN_ENTRY *bin)
{
	pid_t pid;
	int status;

	if (bin == NULL || r == NULL)
		return 0;

	pid = fork();
	if (pid < 0) {
		perror("fork");
		return -1;
	}
	else if (pid == 0) {
		/* child */
		int sig;
		char bin_path[512];

		/* Reset signal handlers set for parent process */
		for (sig = 0; sig < (_NSIG-1); sig++)
			signal(sig, SIG_DFL);

		if (bin->type == TYPE_SHELL) {
			execle(SHELL_PATH, SHELL_PATH, "-c", bin->name, NULL, env);
		}
		else if ((!r->args) || (!r->args[0]) || strchr(r->args, '=')) {
			snprintf(bin_path, sizeof(bin_path), "%s%s", BINARY_PATH, bin->name);
			execle(bin_path, bin->name, NULL, env);
		}
		else {
			snprintf(bin_path, sizeof(bin_path), "%s%s", BINARY_PATH, bin->name);
			execve(bin_path, argv, env);
		}

		/* still alive */
		exit(0);
	}
	else {
		/* parent */
		if (waitpid(pid, &status, 0) == -1) {
			perror("waitpid");
			return errno;
		}

		if (WEXITSTATUS(status))
			return WEXITSTATUS(status);
		else if (WIFSIGNALED(status))
			return WTERMSIG(status);
		else
			return status;
	}

	return 0;
}

void extern_cgi(int argc, char **argv, void *p)
{
	CGI_info *ci = (CGI_info *)p;
	struct request_rec *r = ci->r;
	BIN_ENTRY *bin = ci->bin;
	//int i = 0;
	char **envs;
	 
	ht_dbg("r->url=[%s], r->args=[%s]\n", r->url, r->args);

	/* Destory last cgi table */
	if ((input_num = __destroy_cgi(inputs, input_num)) != 0)
		ht_dbg("warning: %d entries have not been freed, memory leaking.. \n", input_num);

	input_num = __init_cgi(ci, inputs, sizeof(inputs)/sizeof(input_t));

	if (input_num <= 0) {
		goto done;
	}

	/*
 	pete_zhang 2013-12-20	ticket-1167

	description: GUI security enhancement
 	*/
#ifdef CONFIG_HTTPD_TOKEN_CHECK_SUPPORT
	if ( !cgi_token_pass(ci, inputs, 0) ) //failed to pass token checking
	{
		goto done;
	}
#endif //CONFIG_HTTPD_TOKEN_CHECK_SUPPORT
	/*end pete_zhang	2013-12-20*/
	
	/* add vars to envs from the request and CGI info */ 
	envs = fill_envs(r, inputs, input_num);

#if 0
	if (envs == NULL)
		ht_dbg("envs is NULL\n");
	else {
		while (envs[i]) {
			ht_dbg("envs[%d]: %s\n", i, envs[i]);
			i++;
		}
	}
#endif

	/* fork, then do it */
	call_exec(argc, argv, envs, r, bin);

done:
	if (argv) {
		free(argv);
	}

	pthread_exit((void *)CGI_CODE);
}
#endif

void apply_abstract_cgi(int argc, char **argv, void *p)
{
	CGI_info *ci = (CGI_info *)p;
	struct request_rec *r = ci->r;
	int idx = -1, act_idx = -1, param_idx = -1;
	int i, tid = MID_FAIL;
	int cfgID = 0, idx1 = -1, idx2 = -1, enc=0;
	char next_page[128];
	char buf[256];
	int ret=0,sz=0;

	memset(buf, 0, sizeof(buf));
	 
	ht_dbg("r->url = %s\n", r->url);

	//memset(next_page, 0, sizeof(next_page)); // no need

	sprintf(next_page, HTTP_APPLY_SUCCESS);

	/* Destory last cgi table */
	if ((input_num = __destroy_cgi(inputs, input_num)) != 0)
		ht_dbg("warning: %d entries have not been freed, memory leaking.. \n", input_num);

	input_num = __init_cgi(ci, inputs, sizeof(inputs)/sizeof(input_t));

	if ( input_num <= 0 ) {
		sprintf(next_page, "%s", HTTP_APPLY_FAIL); // "Fail");
		goto mapi_done;
	}

	/*
 	pete_zhang 2013-12-20	ticket-1167

	description: GUI security enhancement
 	*/
#ifdef CONFIG_HTTPD_TOKEN_CHECK_SUPPORT
	if ( !cgi_token_pass(ci, inputs, 0) ) //failed to pass token checking
	{
		goto mapi_done;
	}
#endif //CONFIG_HTTPD_TOKEN_CHECK_SUPPORT
	/*end pete_zhang	2013-12-20*/

	tid = get_tid();
	if (tid == MID_FAIL) {
		ht_dbg( "Failed to start a transaction to arc middle\n" );
		sprintf(next_page, "%s",  HTTP_APPLY_FAIL); //"Fail");
		goto mapi_done;
	}


	for (i=0; i<input_num; i++) {
		
		//ht_dbg("input.name=[%s], input.val=[%s]\n", inputs[i].name, inputs[i].val);
		if (!inputs[i].name || !inputs[i].val) {
			sprintf(next_page, "%s",  HTTP_APPLY_FAIL); //"Fail");
			goto done;
		}

		if (!strcmp(inputs[i].name, "submit_button"))
			idx = i;
		else if (!strcmp(inputs[i].name, "next_page"))
			snprintf(next_page, sizeof(next_page), "%s", inputs[i].val);
		else if (!strcmp(inputs[i].name, "action"))
			act_idx = i;
		else if (!strcmp(inputs[i].name, "action_params"))
			param_idx = i;
		else if (inputs[i].name[0] != '\0') {
			//printf("[%s:%d]: inputs[i].name=%s, cfgID=%d, idx1=%d, idx2=%d \n", __FUNCTION__, __LINE__, inputs[i].name,cfgID, idx1, idx2 );
			// at util.c
#if defined(VR9517PAC22_A_PP) || defined(AR7516AOW22_A_PP)	//for PLDT project
			if ((!arccfgFormat(inputs[i].name)) 
				&& (strcmp(inputs[i].name,"ARC_SYS_Password") != 0)
				&& (strcmp(inputs[i].name,"ARC_SYS_Password2") != 0)
				&& (strcmp(inputs[i].name,"ARC_SYS_Telnetd_Password") != 0)
			    )
#elif defined(VR7516RW22) || defined(WE410443)		
			if ((!arccfgFormat(inputs[i].name)) && (strcmp(inputs[i].name,"ARC_SYS_Password") != 0))
#else
			if (!arccfgFormat(inputs[i].name))
#endif
			{
				sz=strlen(inputs[i].name);
				if(inputs[i].name[sz-1] == '*'){
						enc=1;
						inputs[i].name[sz-1]='\0';
				}
				// check if need decode, special NOTE: here browser send a %xx format if encrypted, 
				// Arc_decode() need do unescape_url() first
				if( enc) {
					Arc_decode(r,(unsigned char*)inputs[i].val);
				}

				continue;
			}

#if defined(VR9517PAC22_A_PP) || defined(AR7516AOW22_A_PP)	//for PLDT project
			if((strcmp(inputs[i].name,"ARC_SYS_Password") != 0)
				&& (strcmp(inputs[i].name,"ARC_SYS_Password2") != 0)
				&& (strcmp(inputs[i].name,"ARC_SYS_Telnetd_Password") != 0)
			  )
#elif defined(VR7516RW22) || defined(WE410443)	
			if(strcmp(inputs[i].name,"ARC_SYS_Password") != 0)
#endif
			{
				/* Parse out parameter with indexes */
				parseCfgIdx(inputs[i].name, &cfgID, &idx1, &idx2, &enc);
			}

			// check if need decode, special NOTE: here browser send a %xx format if encrypted,
			// Arc_decode() need do unescape_url() first
			if( enc) {
				Arc_decode(r,(unsigned char*)inputs[i].val);
			}

			//printf("[%s:%d]: inputs[i].name=%s, cfgID=%d, idx1=%d, idx2=%d \n", __FUNCTION__, __LINE__,  inputs[i].name,cfgID, idx1, idx2 );
			if (skip_var(&inputs[i]))
				continue;
#if defined(VR7516RW22) || defined(WE410443)	
			ret = hack_var(tid, &inputs[i]);
			if(ret < 0){
				ht_dbg("Try to hack var [%s], but failed, continue\n", inputs[i].name);
				continue;
			}
			else if(ret > 0)
#endif				
			{
				if (idx1 < 0 )/* set for parameter without index */
				{
					ret = arc_cfg_set(tid, cfgID, inputs[i].val);
				}
				else 
				{
					if (idx2 < 0) /* set for parameter with one index */
					{
						ret = arc_cfg_set_idx1(tid, cfgID, idx1, inputs[i].val);
					} 
					else /* set for parameter with two indexes */
					{
						ret = arc_cfg_set_idx2(tid, cfgID, idx1, idx2, inputs[i].val);
					}
				} 
			}
			if (ret != 0){
				sprintf(next_page, "%s", HTTP_APPLY_FAIL); //"Fail");
				goto done;	/* fail */
			}
		}
	}

	//printf("apply abstract, act_idx=%d, param_idx=%d \n",  act_idx, param_idx);

	/* done, notify MNG the action */
	if (act_idx < 0) {
		//mng_action(NOTHING, NOTHING);
	}
	else if (param_idx < 0) {
		mng_action(inputs[act_idx].val, NOTHING);
	}
	else {
		//printf("apply abstract, act_idx=%d, param_idx=%d, inputs[param_idx].val=%s \n",  act_idx, param_idx, inputs[param_idx].val);
		mng_action(inputs[act_idx].val, inputs[param_idx].val);
	}

	arc_cfg_commit(tid);

done:
#if 0 /* TEY: in current GUI platform the next_page redirection will be done by GUI, so here should only reply 200 OK */	
	/*
	 * Alpha Liu 2012.11.27 issue for PR711AAW
	 * Issue description: httpd send an 302 redirect response
	 *										with wrong location url to browser.
	 * Solution: Only send 200 ok response when page_suffix do not set.
	*/

	if (v_ops->page_suffix != NULL) {
		//snprintf(buf, sizeof(buf), "Location: /%s%s\n\n", next_page[0]=='\0' ? "Success" : next_page, v_ops->page_suffix);
		snprintf(buf, sizeof(buf), "Location: /%s\n\n", next_page);
		send_response(r, buf);
	}
	else
#endif		
		answer(r, DOCUMENT_FOLLOWS, NOTHING); //only send 200 ok
	/* end Alpha Liu 2012.11.27 issue for PR711AAW */

mapi_done:
	if (argv)
		free(argv);
	
	pthread_exit((void *)CGI_CODE);

}

