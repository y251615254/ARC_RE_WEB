#include "httpd.h"
#include "cgi.h"
#include "httoken.h"


extern input_t inputs[MAX_CGI_VARS];
extern int input_num;

extern char* Get_MD5_Str(char* buffer, char* result);
extern int get_login_timeout(void);
extern int is_http_logined(void);
extern void grant_user(sock_t ip);

void login_cgi(int argc, char **argv, void *p);
void logout_cgi(int argc, char **argv, void *p);
void lte_settings_cgi(int argc, char **argv, void *p);
//void apply_sipp_cgi(int argc, char **argv, void *p);
void wl_cal_cgi(int argc, char **argv, void *p);

/*
 * CGI Table: translate script's filename to its task function, all tasks can
 *      be assigned a different stack size, respectively
 */
const CGI_ENTRY cgi_tbl[] =
{
	/* common, always need */
	{"apply.cgi",	TASK_t apply_cgi, 	TASK_STKSIZ},
	{"upload.cgi",	TASK_t upload_cgi, TASK_STKSIZ},
	{"login.cgi",	TASK_t login_cgi, 	TASK_STKSIZ},
	{"logout.cgi",	TASK_t logout_cgi, 	TASK_STKSIZ},

	/* cgi for Abstract layer set */
	{"apply_abstract.cgi",	TASK_t apply_abstract_cgi, 	TASK_STKSIZ},

	/* cgi for SIP Proxy */
//	{"apply_sipp.cgi",	TASK_t apply_sipp_cgi, 	TASK_STKSIZ},


	/* VOIP use apply.cgi */

	/* LTE */
	{"lte_settings.cgi",	TASK_t lte_settings_cgi, 	TASK_STKSIZ},

	/* Wireless calibration */
	{"wl_cal.cgi",	TASK_t wl_cal_cgi, 	TASK_STKSIZ},

	{NULL, 			(void (*)()) 0, 	0}
};




#define SKIP_PASSWORD_VAR	""
#define	FAKE_PASSWORD	"d6nw5v1x2pc7st9m"


int decrypt_conf_file(const char *in, const char *out)
{
	char cmd[1024];
	FILE *fp = NULL;

	system("mkdir -p /tmp/.config");

	snprintf(cmd, sizeof(cmd), "tar xzf %s -C /tmp/.config", in);
	fp = popen(cmd, "r");
	cprintf("%s:%d->fp[%d]\n", __FUNCTION__, __LINE__, fp?1:-1);
	if (fp != NULL) {
		pclose(fp);
		if (0 == access("/tmp/.config/config/.glbcfg",0))               //by seal to fix restore issue 150303
	{
		system("rm /tmp/.config/config/glbcfg.dft");
		system("cp -af /tmp/.config/config/. /etc/config; rm -rf /tmp/.config");

		snprintf(cmd, sizeof(cmd), "cp /etc/config/.glbcfg %s", out);
		system(cmd);
		system("arccfg commit");

		return 0;
		}
	}
	else
	{
		system("rm -rf /tmp/.config");
		return 1;
	}
}

int is_fake_passwd(const char *name, const char *value)
{
	if (strstr(SKIP_PASSWORD_VAR, name) && strcmp(value, FAKE_PASSWORD)==0 )
		return 1;

	return 0;
}

int skip_var(input_t *input_var)
{
	return is_fake_passwd(input_var->name, input_var->val);
}


static int translate_http_passwd(const char *old_value, char *new_value, int size)
{
	char key[8] = "wg7005d";
	int k_idx = 0, j, ri = 0;
	char *s;
	char tmp[64][4];
	char http_md5_passwd[33];
	char system_pwd[128];
	int tid;

	if((tid = get_tid()) == MID_FAIL)
		return -1;


	//printf("%s(%d)\n", __FUNCTION__, __LINE__);
	//

	if (old_value == NULL || new_value == NULL)
		return -1;

	strncpy(new_value, old_value, size);
	new_value[size] = '\0';
	s = strtok(new_value, " ");
	while(s)
	{
		strcpy(tmp[ri++], s);
		s = strtok(NULL, " ");
	}
	for(j = 0; j < ri; j++)
	{
		if (j > size)
			return -1;
		new_value[j] = atoi(tmp[j]) - key[k_idx];
		k_idx = (k_idx + 1) % strlen(key);
	}
	new_value[j] = 0;

	ht_dbg("old_value[%s], new_value[%s]\n",old_value, new_value);
	/*
	   if (!valid_name(wp, new_value, v)) return;
	   */

	/* User didn't modify the password */
	mapi_ccfg_get_str(tid, "ARC_SYS_Password", system_pwd, sizeof(system_pwd));
	Get_MD5_Str(system_pwd, http_md5_passwd);
	if( memcmp(http_md5_passwd, new_value, 33) == 0 )
		return -2;

	return 0;
}

int hack_var(int tid, input_t *input_var)
{
	/* value[] contains 128 characters and 1 NULL-character at most.
	 * For example, when "ARC_SYS_Password"=5566 in glbcfg,
	 * inputs[i].val = "217 151 154 103 145 154 150 170 152 109 147 103 154 156 169 152 107 150 148 107 153 176 204 107 146 147 109 197 167 203 156 145 ";      
	 * NOTE: there is a space in the end.
	 * strlen(inputs[i].val) = 128;  // 32 * 4 = 128;
	 */
	char value[32*4 + 1];  /* 32 is the length of md5sum . 4 is the length of the string "xxx ". */

	/* translate http passwd */
	if (!strcmp(input_var->name, "ARC_SYS_Password")) {
		if (translate_http_passwd(input_var->val, value, sizeof(value) - 1))
			return -1;
		return mapi_ccfg_set_str(tid, input_var->name, value);
	}
	return 1;
}
/*
 * paramters
 *   "pws": password
 *   "url": next jump URL if sucess
 *
 * return:
 *
 *
 *
 */

/* -1: session timeout to login again
 * -2: password error
 * -3: duplicate error
 */
#define HTTPD_LOGIN_OK        0
#define HTTPD_LOGIN_AGAIN     -1
#define HTTPD_LOGIN_ERROR     -2
#define HTTPD_LOGIN_DUPLICATE -3

void login_cgi(int argc, char **argv, void *p)
{
	CGI_info *ci = (CGI_info *)p;
	struct request_rec *r = ci->r;
	char *passwd = NULL;
	char *next_page = NULL;
	char buf[1024];
	char http_md5_passwd[33];
	char system_pwd[128];
	int tid;
	int do_rediret=HTTPD_LOGIN_AGAIN;

	if((tid = get_tid()) == MID_FAIL)
		return;

	/* Destory last cgi table */
	if ((input_num = __destroy_cgi(inputs, input_num)) != 0)
		ht_dbg("warning: %d entries have not been freed, memory leaking.. \n", input_num);

	input_num = __init_cgi(ci, inputs, sizeof(inputs)/sizeof(input_t));

	/*
 	pete_zhang 2013-12-20	ticket-1167

	description: GUI security enhancement
 	*/
#ifdef CONFIG_HTTPD_TOKEN_CHECK_SUPPORT
	if ( !cgi_token_pass(ci, inputs, 0) ) //failed to pass token checking
	{
		do_rediret=HTTPD_LOGIN_AGAIN;
		goto done;
	}
	else
	{
		httoken_entry_remove( HTTOKEN_REMOVE_ALL );//clean previous token when new user login
	}
#endif
	/*end pete_zhang	2013-12-20*/

	passwd = __get_cgi(inputs, "pws");
	next_page = __get_cgi(inputs, "url");

	ht_dbg("passwd = %s,  next_page=%s\n", passwd, next_page);
	mapi_ccfg_get_str(tid, "ARC_SYS_Password", system_pwd, sizeof(system_pwd));
	Get_MD5_Str(system_pwd, http_md5_passwd);

	if(passwd && !strcmp(http_md5_passwd, passwd)) /* Password is correct. */
	{
		/*
		 * Alpha Liu 2012.11.27 issue for PR711AAW
		 * Issue description: GUI did not allow another PC login
		 *										when one PC already logined.
		 * Solution: Allow any PC login when the login password is correct.
		*/
		//if (!is_http_logined()) /* nobody login yet, you can login */
		if (1) /* login anyway */
		/* end Alpha Liu 2012.11.27 issue for PR711AAW */
		{
			grant_user(r->remote_ip);

			/*
 			pete_zhang 2013-12-20	ticket-1167

			description: GUI security enhancement
 			*/
#ifdef CONFIG_HTTPD_TOKEN_CHECK_SUPPORT
			//To ensure token timeout after user timeout, so add half minute
			httoken_timeout_set( get_login_timeout() + 30 );
#endif
			/*end pete_zhang	2013-12-20*/
/* KILL ME
			sprintf(buf, "<html>\n"
					"<head>\n"
					"<META http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\" >\n"
					"</head>\n"
					"\n"
					"<script>\n"
					"function reload(){  \n"
					//"top.topFrame.location.href=top.topFrame.location.href; \n"
					//"document.location.href=\"%s\"; \n"
"top.location.href=\"%s\"; \n"
					"}\n"
					"setTimeout(\"reload()\", 900); \n"
					"function SetCookie (name, value) { \n"
					"var argv = SetCookie.arguments; \n"
					"var argc = SetCookie.arguments.length;\n"
					"var expires = (argc > 2) ? argv[2] : null;\n"
					"var path = (argc > 3) ? argv[3] : null;\n"
					"var domain = (argc > 4) ? argv[4] : null;\n"
					"var secure = (argc > 4) ? argv[5] : false;\n"
					"document.cookie = name + \"=\" + escape (value) + ((expires == null) ? \"\" : (\"; expires=\" + expires.toGMTString())) + ((path == null) ? \"\" : (\"; path=\" + path)) + ((domain == null) ? \"\" : (\"; domain=\" + domain));\n"
					"}\n"
					"</script>\n"
					//"</html> \n ", next_page? next_page : "status.htm");
"</html> \n ", next_page? next_page : "index.htm");
			r->status = DOCUMENT_FOLLOWS;
			r->content_type = default_type;
			r->content_length = strlen(buf);
			send_http_header(r);
			so_printf(r, buf);
*/
			do_rediret=HTTPD_LOGIN_OK;
		}
		else /* someone has logined in */
		{
			do_rediret=HTTPD_LOGIN_DUPLICATE;
/* KILL_ME
			#ifndef CONFIG_HTTPD_HOT_FIX_SW1 //KILL_ME hugh 2014/1/14
			snprintf(buf, sizeof(buf), "Location: /login.htm");
			#else
			snprintf(buf, sizeof(buf), "Location: /loginduperr.htm"); //FIXME: hugh 2013/12/12 hard code????
			#endif

			//answer(r, REDIRECT, buf);
			send_response(r, buf);
*/
		}
	}
	else
	{
		/* Fail to auth, redirect to loginpeer.stm */
		do_rediret=HTTPD_LOGIN_ERROR;
/* KILL_ME
		#ifndef CONFIG_HTTPD_HOT_FIX_SW1 //KILL_ME hugh 2014/1/14
		snprintf(buf, sizeof(buf), "Location: /login.htm");
		#else
		snprintf(buf, sizeof(buf), "Location: /loginpserr.htm"); //FIXME: hugh 2013/12/12, it should be login err??
		#endif
		//answer(r, REDIRECT, buf);
		send_response(r, buf);
*/
	}

done:
	switch(do_rediret)
	{
		case HTTPD_LOGIN_OK: //0
			snprintf(buf, sizeof(buf), "Location: /index.htm");
			break;
		default:
		case HTTPD_LOGIN_AGAIN: //-1
			snprintf(buf, sizeof(buf), "Location: /login.htm");
			break;
		case HTTPD_LOGIN_ERROR: //-2
			snprintf(buf, sizeof(buf), "Location: /loginpserr.htm");
			break;
		case HTTPD_LOGIN_DUPLICATE: //-3
			snprintf(buf, sizeof(buf), "Location: /loginduperr.htm");
			break;
	}
	send_response(r, buf);

	if (argv)
		free(argv);

	pthread_exit((void *)CGI_CODE);
}


void clear_granted_user(struct request_rec *r);

void logout_cgi(int argc, char **argv, void *p)
{
	CGI_info *ci = (CGI_info *)p;
	struct request_rec *r = ci->r;
	char content[SOPRINTFBUFSIZE*2];
	int i, len;
	char buf[1024];

	/* Destory last cgi table */
	if ((input_num = __destroy_cgi(inputs, input_num)) != 0)
		ht_dbg("warning: %d entries have not been freed, memory leaking.. \n", input_num);

	input_num = __init_cgi(ci, inputs, sizeof(inputs)/sizeof(input_t));

	/*
 	pete_zhang 2013-12-20	ticket-1167

	description: GUI security enhancement
 	*/
#ifdef CONFIG_HTTPD_TOKEN_CHECK_SUPPORT
	if ( !cgi_token_pass(ci, inputs, 0) ) //failed to pass token checking
	{
		goto done;
	}
#endif
	/*end pete_zhang	2013-12-20*/

	len = r->content_length;
	/* dummy read */
	while (len > 0)
	{
#ifdef CONFIG_HTTPD_HTTPS_SUPPORT
		if (ci->r->ishttps) {
			i = as_read(ci->r->ssrv, content, sizeof(content));
		}
		else
#endif
			i = recv(ci->socketid, content, sizeof(content), MSG_DONTWAIT);

		if (i <=0 )
			break;

		len -= i;
	}

	clear_granted_user(r);

	/*
 	pete_zhang 2013-12-20	ticket-1167

	description: GUI security enhancement
 	*/
#ifdef CONFIG_HTTPD_TOKEN_CHECK_SUPPORT
	httoken_entry_remove( HTTOKEN_REMOVE_ALL );
#endif
	/*end pete_zhang	2013-12-20*/

	snprintf(buf, sizeof(buf), "Location: /login.htm");
	//answer(r, REDIRECT, buf);
	send_response(r, buf);

done:
	if (argv)
		free(argv);

	pthread_exit((void *)CGI_CODE);
}

/*
 * typedef struct aunet_UI_settings_s {
 *		char            pin[MAX_PIN_LEN];               //  VAR_AUNET_PIN
 *		char            APN[MAX_APN_LEN + 4];           //  VAR_AUNET_PDPINFO_APN0 or VAR_AUNET_PDPINFO_APN1
 *		char            ISP_Phone[STR_LEN_68];          //  VAR_AUNET_ISP_PHONE
 *		char            Username[PPP_USERNAME_STR_LEN]; //  VAR_AUNET_USERNAME
 *		char            Password[PPP_PASSWORD_STR_LEN]; //  VAR_AUNET_PASSWORD

 *		short           idle_timeout;                   //  VAR_AUNET_IDLE_TIMEOUT
 *		unsigned short  mtu;                            //  VAR_AUNET_MTU
 *		unsigned int    auto_reconnect  :1;             //  VAR_AUNET_AUTO_RECONNECT
 *		unsigned int    on_demend       :1;             //  VAR_AUNET_ON_DEMAND
 *		unsigned int    startup_stat    :2;             //  VAR_AUNET_STARTUP_STATE

 *		unsigned int    ppp_or_ndis     :1;             //  VAR_AUNET_FUNC, 0: PPP, 1: NDIS
 * } aunet_UI_settings_t;
 *
 */
void lte_settings_cgi(int argc, char **argv, void *p)
{
	CGI_info *ci = (CGI_info *)p;
	struct request_rec *r = ci->r;
	char *pin = NULL;
#ifdef CONFIG_HTTPD_HOT_FIX_SW1
	char *next_page = NULL; //hugh 2013/12/12
#endif

#if 0
	char *apn2;
	char *apn0, *apn1, *username, *password, *idle_timeout, *isp_phone, *backup_mode;
	char *save_pin = NULL, *page_from = NULL;
#endif
	char *username, *password, *idle_timeout;
	char *page_from = NULL;
	char buf[1024];
	int tid;
	int ret = 0;
	//int ifno = T_COM1_INT;
	//aunet_UI_settings_t as;

	if((tid = get_tid()) == MID_FAIL)
		return;

	/* Destory last cgi table */
	if ((input_num = __destroy_cgi(inputs, input_num)) != 0)
		ht_dbg("warning: %d entries have not been freed, memory leaking.. \n", input_num);

	/* action=ui_lte&submit_button=lte_main.htm&VAR_AUNET_PIN=1234&save_pin=on \
	 * &VAR_AUNET_PDPINFO_APN0=3gnet&VAR_AUNET_PDPINFO_APN1=3gnet
	 */
	input_num = __init_cgi(ci, inputs, sizeof(inputs)/sizeof(input_t));

	/*
 	pete_zhang 2013-12-20	ticket-1167

	description: GUI security enhancement
 	*/
#ifdef CONFIG_HTTPD_TOKEN_CHECK_SUPPORT
	if ( !cgi_token_pass(ci, inputs, 0) ) //failed to pass token checking
	{
		goto done;
	}
#endif
	/*end pete_zhang	2013-12-20*/

	page_from = __get_cgi(inputs, "page_from");

	username = NULL;
	password = NULL;
	idle_timeout = NULL;

	idle_timeout = __get_cgi(inputs, "VAR_AUNET_0_IDLE_TIMEOUT");
	username = __get_cgi(inputs, "VAR_AUNET_0_USERNAME");
	password = __get_cgi(inputs, "VAR_AUNET_0_PASSWORD");

	#ifdef CONFIG_HTTPD_HOT_FIX_SW1
	next_page = __get_cgi(inputs, "next_page");  //hugh 2013/12/12
	#endif

#if 0
	pin = NULL;
	apn0 = NULL;
	apn1 = NULL;

	apn2 = NULL;
	username = NULL;
	password = NULL;
	idle_timeout = NULL;
	isp_phone = NULL;
	backup_mode = NULL;

	page_from = __get_cgi(inputs, "page_from");

	/* from LTE */
	pin = __get_cgi(inputs, "VAR_AUNET_0_PIN");
	apn0 = __get_cgi(inputs, "VAR_AUNET_PDPINFO_0_APN0");
	apn1 = __get_cgi(inputs, "VAR_AUNET_PDPINFO_0_APN1");
	apn2 = __get_cgi(inputs, "VAR_AUNET_PDPINFO_0_APN2");
	save_pin = __get_cgi(inputs, "save_pin");

	/* from UTMS */
	backup_mode = __get_cgi(inputs, "VAR_AUNET_PDPINFO_0_APN_BACKUP0");
	isp_phone = __get_cgi(inputs, "VAR_AUNET_0_ISP_PHONE");
	idle_timeout = __get_cgi(inputs, "VAR_AUNET_0_IDLE_TIMEOUT");
	username = __get_cgi(inputs, "VAR_AUNET_0_USERNAME");
	password = __get_cgi(inputs, "VAR_AUNET_0_PASSWORD");

	#ifdef CONFIG_HTTPD_HOT_FIX_SW1
	next_page = __get_cgi(inputs, "next_page");  //hugh 2013/12/12
	#endif

	memset(&as, 0, sizeof(aunet_UI_settings_t));

	/* ----  fill the structure ---- */
	if (pin)
		snprintf(as.pin, sizeof(as.pin), "%s", pin);
	else /* no change on PIN */
		mapi_ccfg_get_str(tid, "VAR_AUNET_0_PIN", as.pin, sizeof(as.pin));

	if (apn0)
		snprintf(as.APN, sizeof(as.APN), "%s", apn0);
	else /* no change on APN0 */
		mapi_ccfg_get_str(tid, "VAR_AUNET_PDPINFO_0_APN0", as.APN, sizeof(as.APN));

	/* apn1, backup_mode are not used, just set to glbcfg */
	if (apn1)
		mapi_ccfg_set_str(tid, "VAR_AUNET_PDPINFO_0_APN1", apn1);

	if (apn2)
		mapi_ccfg_set_str(tid, "VAR_AUNET_PDPINFO_0_APN2", apn2);

	if (backup_mode)
		mapi_ccfg_set_str(tid, "VAR_AUNET_PDPINFO_0_APN_BACKUP0", backup_mode);

	if (isp_phone)
		snprintf(as.ISP_Phone, sizeof(as.ISP_Phone), "%s", isp_phone);
	else /* no change on isp_phone */
		mapi_ccfg_get_str(tid, "VAR_AUNET_0_ISP_PHONE", as.ISP_Phone, sizeof(as.ISP_Phone));

	if (idle_timeout)
		as.idle_timeout = (short) atoi(idle_timeout);
	else /* no change on idle_timeout */
		as.idle_timeout = (short) atoi(mapi_ccfg_get_str(tid, "VAR_AUNET_IDLE_TIMEOUT", buf, sizeof(buf)));

	if (username)
		snprintf(as.Username, sizeof(as.Username), "%s", username);
	else /* no change on username */
		mapi_ccfg_get_str(tid, "VAR_AUNET_0_USERNAME", as.Username, sizeof(as.Username));

	if (password)
		snprintf(as.Password, sizeof(as.Password), "%s", password);
	else /* no change on password */
		mapi_ccfg_get_str(tid, "VAR_AUNET_0_PASSWORD", as.Password, sizeof(as.Password));

	/*
	 * linghong_tan 2013-09-10.
	 *
	 * FIXME: These data do not come from webpage,
	 * we just get them from glbcfg.
	 */
	as.mtu = (unsigned short)atoi(mapi_ccfg_get_str(tid, "VAR_AUNET_0_MTU", buf, sizeof(buf)));
	as.auto_reconnect = (unsigned int)atoi(mapi_ccfg_get_str(tid, "VAR_AUNET_0_AUTO_RECONNECT", buf, sizeof(buf)));
	as.on_demend = (unsigned int)atoi(mapi_ccfg_get_str(tid, "VAR_AUNET_0_ON_DEMAND", buf, sizeof(buf)));
	as.startup_stat = (unsigned int)atoi(mapi_ccfg_get_str(tid, "VAR_AUNET_0_STARTUP_STATE", buf, sizeof(buf)));
	//as.ppp_or_ndis = (unsigned int)atoi(mapi_ccfg_get_str(tid, "VAR_AUNET_0_FUNC", buf, sizeof(buf)));
	as.ppp_or_ndis = 1;

	if (page_from && !strcmp(page_from, "UMTS"))
		ifno = T_COM3_INT;	/* UMTS page */
	else
		ifno = T_COM1_INT;	/* LTE page */

	if (save_pin && !strcmp(save_pin, "1"))
		mapi_aunet_set_pin(tid, ifno, &as, 1);
	else
		mapi_aunet_set_pin(tid, ifno, &as, 0);

	cprintf("pin=[%s], APN=[%s], isp_phone=[%s], username=[%s], password=[%s], page_from=[%s]\n",
			as.pin, as.APN, as.ISP_Phone, as.Username, as.Password, page_from);

	ret = mapi_aunet_set_UI_settings(tid, ifno, &as);
#endif
	#ifdef CONFIG_HTTPD_HOT_FIX_SW1
	// NOTE: hugh
	//    2013/12/16: we no feedback OK,Fail state to browser, here just send Sucess or Fail
	//
	if (v_ops->page_suffix != NULL) {
		if (ret == 0) {
			// sucess
			snprintf(buf, sizeof(buf), "Location: /%s%s\n\n", (next_page==NULL) ? "Success" : next_page, v_ops->page_suffix);
		}else{
			snprintf(buf, sizeof(buf), "Location: /Fail%s\n\n", v_ops->page_suffix);
		}
		send_response(r, buf);
	}else
	#endif
	if (ret == 0) {
		answer(r, DOCUMENT_FOLLOWS, NOTHING); //only send 200 ok
	}
	else {
		/* error report */
	}

	mapi_ccfg_commit(tid);

done:
	if (argv)
		free(argv);

	pthread_exit((void *)CGI_CODE);
}

void wl_cal_cgi(int argc, char **argv, void *p)
{
	CGI_info *ci = (CGI_info *)p;
	struct request_rec *r = ci->r;
	int len;
	char *boundary = NULL;
	int tail_len = 0;
	char buf[1024];
	int sock;
	int ret_code = -1;
	FILE *stream = NULL;
	int x = 0;
	int got_file = 0;

	sock = ci->socketid;
	len = r->content_length;

	if ((input_num = __destroy_cgi(inputs, input_num)) != 0)
		ht_dbg("warning: %d entries have not been freed, memory leaking.. \n", input_num);

	/*
 	pete_zhang 2013-12-20	ticket-1167

	description: GUI security enhancement
 	*/
#ifdef CONFIG_HTTPD_TOKEN_CHECK_SUPPORT
	if ( !cgi_token_pass(ci, inputs, 0) ) //failed to pass token checking
	{
		goto done;
	}
#endif
	/*end pete_zhang        2013-12-20*/

	stream = fdopen(sock, "r");
	if (!stream) {
		goto done;
	}

	while (len > 0) {

		memset(buf, 0, sizeof(buf));

		if (!get_line(r, buf, MIN(len + 1, sizeof(buf)), stream)) {
			goto done;
		}

		len -= strlen(buf);

		if (!strncasecmp(buf, "Content-Disposition:", 20)) {
			if (strstr(buf, "name=\"wait_time\""))
			{
				char *ptr, *stop;
				int idx;

				for (idx=0; idx<2 && len>0; idx++) {

					memset(buf, 0, sizeof(buf));

					/* eat a line */
					if (!get_line(r, buf, MIN(len + 1, sizeof(buf)), stream))
						goto done;

					len -= strlen(buf);
				}

				ptr = malloc(100);
				if (!ptr)
					goto done;

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
			else if (strstr(buf, "name=\"calibration\"")) {
				got_file = 1;
				break;
			}
		}
	}

	input_num = x;

	memset(buf, 0, sizeof(buf));

	/* Skip boundary and headers */
	while (len > 0) {
		if (!get_line(r, buf, MIN(len + 1, sizeof(buf)), stream))
			goto done;

		len -= strlen(buf);
		if (!strcmp(buf, "\n") || !strcmp(buf, "\r\n"))
			break;
	}

	if ((boundary = strcasestr(r->content_type, "boundary=")) != NULL) {
		//Boundary was found.
		boundary += strlen("boundary=");

		//Please see RFC2616:
		//boundary=THIS_STRING_SEPARATES
		//The tail is: "\r\n--THIS_STRING_SEPARATES--\r\n"
		tail_len = 2 + 2 + strlen(boundary) + 2 + 2;

		//Remove the tail length first.
		len -= tail_len;

		//cprintf("tail length: [%d] => [%d]\n", tail_len, len);
	}

	if (got_file) {

		cprintf("===> Wireless calibration CGI <===\n");

		/* store file to /tmp/cal_wlan0.bin */
		ret_code = sys_restore(r, "/tmp/cal_wlan0.bin", stream, &len);
		if (ret_code < 0)
			goto done;

		/* package it to /tmp/eeprom_wl.tar.gz */
		ret_code = system("cd /tmp; tar czf /tmp/eeprom_wl.tar.gz cal_wlan0.bin; cd -");
		if (ret_code < 0)
			goto done;

		/* flash it to wlanconfig */
		ret_code = system("vol_mgmt write_vol wlanconfig /tmp/eeprom_wl.tar.gz");
		if (ret_code < 0)
			goto done;
	}

	//crazy_liang 2013.3.12: For calculating the correct file size.
	if (tail_len > 0) {
		//Recover the tail length.
		len += tail_len;
	}
	//end crazy_liang 2013.3.12

	/* Slurp anything remaining in the request */
	while (len--)
		get_char(r, stream);

done:
	if (argv)
		free(argv);

	answer(r, DOCUMENT_FOLLOWS, NOTHING); //only send 200 ok
	so_flush(r);

	/* Only when success, device will reboot. */
	if (ret_code == 0) {
		/* reboot the system */
		mng_action("sys_wl_reset", NOTHING);
	}

	pthread_exit((void *)CGI_CODE);
}

#if 0
void apply_sipp_cgi(int argc, char **argv, void *p)
{
	CGI_info *ci = (CGI_info *)p;
	struct request_rec *r = ci->r;
	int idx = -1, act_idx = -1, param_idx = -1, add_idx = 0, del_idx = 0;
	int i = 0, j = 0, k = 0, l = 0, tid = MID_FAIL;
	int cfgID = 0, idx1 = -1, idx2 = -1, tmpidx = 0, tmplen = 0, entrycount = 0;
	char next_page[128];
	char buf[256], tmp[32], tmp2[32];
	char *ptmp, *pEnd;
	int ret;

	memset(buf, 0, sizeof(buf));

	ht_dbg("r->url = %s\n", r->url);

	memset(next_page, 0, sizeof(next_page));

	/* Destory last cgi table */
	if ((input_num = __destroy_cgi(inputs, input_num)) != 0)
		ht_dbg("warning: %d entries have not been freed, memory leaking.. \n", input_num);

	input_num = __init_cgi(ci, inputs, sizeof(inputs)/sizeof(input_t));

	if ( input_num <= 0 ) {
		sprintf(next_page, "%s",  "Fail");
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
#endif
	/*end pete_zhang        2013-12-20*/

	tid = get_tid();
	if (tid == MID_FAIL) {
		ht_dbg( "Failed to start a transaction to arc middle\n" );
		sprintf(next_page, "%s",  "Fail");
		goto mapi_done;
	}


	for (i=0; i<input_num; i++) {

		//ht_dbg("input.name=[%s], input.val=[%s]\n", inputs[i].name, inputs[i].val);
		if (!inputs[i].name || !inputs[i].val) {
			sprintf(next_page, "%s",  "Fail");
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
		else if (!strcmp(inputs[i].name, "add"))
			add_idx = i;
		else if (!strcmp(inputs[i].name, "del"))
			del_idx = i;
		else if (inputs[i].name[0] != '\0') {
			printf("inputs[i].name=%s, cfgID=%d, idx1=%d, idx2=%d \n", inputs[i].name,cfgID, idx1, idx2 );

			/* Parse out parameter with indexes */
			parseCfgIdx(inputs[i].name, &cfgID, &idx1, &idx2);

			printf("inputs[i].name=%s, cfgID=%d, idx1=%d, idx2=%d \n", inputs[i].name,cfgID, idx1, idx2 );
			if (skip_var(&inputs[i]))
				continue;

			if (cfgID == ARC_VO_SIPP_INTACC_x_UserId) /* internal account*/
			{
				if (add_idx)
				{
					tmpidx = strtol(inputs[add_idx].val, &pEnd, 10);
					/* modify existing incoming mapping */
					for (j=0; ; j++)
					{
						arc_cfg_get_idx1(tid, ARC_VO_SIPP_EXTACC_x_InMapping, j, tmp, sizeof(tmp), "");

						if (strlen(tmp)>0)
						{
							sprintf(tmp, "%s%s", tmp, "0");
							arc_cfg_set_idx1(tid, ARC_VO_SIPP_EXTACC_x_InMapping, j, tmp);
							tmp[0]= '\0';

							sprintf(tmp2, "%s%s-1", tmp2, (j==0)?"":",");/* for outgoing mapping */
						}
						else
							break;
					}

					/* add outgoing mapping */
					arc_cfg_set_idx1(tid, ARC_VO_SIPP_INTACC_x_OutCallMapping, tmpidx, tmp2);

					add_idx = 0;
				}
				else if (del_idx)
				{
						tmpidx = strtol(inputs[del_idx].val, &pEnd,10);
					/* modify existing incoming mapping */
					for (j=0; ; j++)
					{
						arc_cfg_get_idx1(tid, ARC_VO_SIPP_EXTACC_x_InMapping, j, tmp, sizeof(tmp), "");
						if (strlen(tmp)>0)
						{
							ptmp = tmp + tmpidx + 1;
							tmp[tmpidx] = '\0';
							sprintf(tmp, "%s%s",tmp, ptmp);

							arc_cfg_set_idx1(tid, ARC_VO_SIPP_EXTACC_x_InMapping, j, tmp);
							tmp[0]= '\0';
						}
						else
							break;
					}

					/* re-sorting outgoing mapping */
					for (k = tmpidx; ; k++)
					{
						arc_cfg_get_idx1(tid, ARC_VO_SIPP_INTACC_x_OutCallMapping, k+1, tmp2, sizeof(tmp2), "");
						if (strlen(tmp2)>0)
						{
							arc_cfg_set_idx1(tid, ARC_VO_SIPP_INTACC_x_OutCallMapping, k, tmp2);
						}
						else
							break;
					}
					arc_cfg_set_idx1(tid, ARC_VO_SIPP_INTACC_x_OutCallMapping, k, "");
					del_idx = 0;
				}
			}
			else if (cfgID == ARC_VO_SIPP_EXTACC_x_UserId) /* external account */
			{
				if (add_idx)
				{
					tmpidx = strtol(inputs[add_idx].val, &pEnd, 10);

					/* modify existing outgoing mapping */
					for (j=0; ; j++)
					{
						arc_cfg_get_idx1(tid, ARC_VO_SIPP_INTACC_x_OutCallMapping, j, tmp, sizeof(tmp), ""); /* get the outmapping of previous last entry*/

						if (strlen(tmp)>0)
						{
							sprintf(tmp, "%s,-1",tmp);
							arc_cfg_set_idx1(tid, ARC_VO_SIPP_INTACC_x_OutCallMapping, j, tmp);
							tmp[0]= '\0';

							sprintf(tmp2, "%s0", tmp2 ); /* to used by incoming mapping below */
						}
						else
							break;
					}

					/* add incoming mapping */
					arc_cfg_set_idx1(tid, ARC_VO_SIPP_EXTACC_x_InMapping, tmpidx, tmp2);

					add_idx = 0;
				}
				else if (del_idx)
				{
					tmpidx = strtol(inputs[del_idx].val, &pEnd,10);

					/* re-sorting incoming mapping */
					for (k = tmpidx; ; k++)
					{
						arc_cfg_get_idx1(tid, ARC_VO_SIPP_EXTACC_x_InMapping, k+1, tmp2, sizeof(tmp2), "");
						if (strlen(tmp2)>0)
						{
							arc_cfg_set_idx1(tid, ARC_VO_SIPP_EXTACC_x_InMapping, k, tmp2);
						}
						else
							break;
					}

					arc_cfg_set_idx1(tid, ARC_VO_SIPP_EXTACC_x_InMapping, k, "");

					entrycount = k+1; /* total amount of External account */

					/* modify existing outgoing mapping */
					for (j=0; ; j++)
					{
						arc_cfg_get_idx1(tid, ARC_VO_SIPP_INTACC_x_OutCallMapping, j, tmp, sizeof(tmp), ""); /* get the outmapping of previous last entry*/

						if (strlen(tmp)>0)
						{
							/* parse each mapping seperated by ",".Delete the one same as index and minus one for those number larger than index */
							ptmp = strtok(tmp,",");
							k = 0;
						    while (ptmp != NULL)
							{
								tmplen = strtol(ptmp, &pEnd, 10);

								if (k == entrycount-1 && tmplen == -1)
								{ /* skip the last one if it is -1*/
									break;
								}

								if (tmplen != tmpidx)
								{
									sprintf(tmp2, "%s%s%d", tmp2, (k==0)?"":",", (tmplen>tmpidx)?(tmplen-1):tmplen );
									k++;
								}

								ptmp = strtok (NULL, ",");
							}

							arc_cfg_set_idx1(tid, ARC_VO_SIPP_INTACC_x_OutCallMapping, j, tmp2);

							tmp[0]= '\0';
							tmp2[0]= '\0';
						}
						else
							break;
					}

					del_idx = 0;
				}
			}

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
			if (ret != 0){
				sprintf(next_page, "%s", "Fail");
				goto done;	/* fail */
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

done:
	/*
	 * Alpha Liu 2012.11.27 issue for PR711AAW
	 * Issue description: httpd send an 302 redirect response
	 *										with wrong location url to browser.
	 * Solution: Only send 200 ok response when page_suffix do not set.
	*/
	if (v_ops->page_suffix != NULL) {
		snprintf(buf, sizeof(buf), "Location: /%s%s\n\n", next_page[0]=='\0' ? "Success" : next_page, v_ops->page_suffix);
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
#endif


