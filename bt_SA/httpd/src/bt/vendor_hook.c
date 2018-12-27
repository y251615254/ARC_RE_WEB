/*
 * Vendor specified 
 *
 * First verison:		linghong.tan	2012-06-10	
 */
#include "httpd.h"
#include "utils.h"
#include "httoken.h"

#define	HT_GRANTED	0
#define	HT_TIMEUP	1
#define	HT_NEWUSER	2

#define SECURITY_LOG_TMP	"/tmp/security_log.txt"

int login_timeout = 300;
time_t granted_time;
sock_t granted_ip;
int has_logined = 0;
#ifdef CHECK_LOGIN_BROWSER_SUPPORT
char granted_agent[256];
#endif

void arcadyan_init_hook(http_conf_t *hc);
void arcadyan_open_hook(void);
int arcadyan_request_hook(struct request_rec *r);

time_t get_time()
{
	struct sysinfo info;
	sysinfo(&info);

	return (time_t)info.uptime;
}

struct vendor_ops vendor_ops = {
	.vendor = "zz",
	.name = "mgl7016aw",
	/*.page_suffix = ".asp",*/
	.page_suffix = ".htm",
	.init_hook = arcadyan_init_hook,
	.open_hook = arcadyan_open_hook,
	.request_hook = arcadyan_request_hook,
	//.cgi_hook = arcadyan_cgi_hook,
}, *v_ops = &vendor_ops;

#define WE410443_6DX_ONLYFW
char *bypass_list[] = {
	"/images/",
	"/lang/",
#ifdef WE410443_6DX_ONLYFW
	"/cgi/", //hugh hide it due to security issue
#endif
	"/js/",
	"/css/",
	"/setup_top_login.htm",
	"/setup_top_help.htm",
	"/login.htm",
	"/loginpserr.htm",
	"/login.cgi",
	"/3g_conn.xml",
	"/top_conn.xml",
	"/wireless_calibration.htm",
	"/cgi/cgi_login.js",
	"/cgi/cgi_autologout.js",
#ifdef WE410443_6DX_ONLYFW
	"/firmware_upgrade.htm",
	"/system_info.htm",
	"/cgi/cgi_st.js",
	"/cgi/cgi_lan.js",
	"/fw_upgrade.htm",
	"/cgi/fwupgrade_status.js",
	"/cgi/fw_download_prg.js",
	"/cgi/cgi_init.js",
	"/apply.cgi",
	"/upload.cgi",
#else
	"/internet_paused.htm",
#endif
	NULL
#if 0	//KILL_ME
	"/images/",
	"/lang/",
	"/cgi/",
	"/top.htm",
	"/login.htm",
	"/index.htm",
	"/login.cgi",
	"/global.js",
	"/data.js",
	"/md5.js",
	"/menulist.js",
	"/menu.js",
	"/nat.js",
	"/routine.js",
	"/subformvar.js",
	"/jquery-1.6.2.min.js",
	"/jquery.jqtransform.js",
	"/login_frame.htm",
	"/main_brief.htm",
	"/setupw_top.htm",
	"/left_pane.htm",
	"/right_pane.htm",
	"/jquery.jqtransform.js",
	"/jquery-1.6.2.min.js",
	"/3g_conn.xml",
	"/top_conn.xml",
	"/styles.css",
	"/c_voda_login.css",
	"/c_voda_main.css",
	"/c_voda_top.css",
	"/c_voda_login.css",
	"/jqtransform.css",
	"/init_data.js",
	"/routine_data.js",
	"/loginzz.htm",
	"/wireless_calibration.htm",
	"/bottom_pane.htm"
#endif
};

char *valid_url_list[] = {
	"/Fail.htm",
	"/Fail_restore.htm",
	"/Fail_upgrade.htm",
	"/Fail_upload.htm ",
	"/Success.htm",
	"/Success_restore.htm",
	"/Success_upgrate.htm",
	"/Success_upload.htm",
	"/WPS_stautsShow.htm",
	"/debug_arc.htm",
	"/debug_bt.htm",
	"/help.htm",
	"/index.htm",
	"/internet.htm",
	"/internet_paused.htm",
	"/lan1.htm",
	"/led.htm",
	"/ledbck.htm",
#ifndef WE410443_6DX_ONLYFW
	"/login.htm",
#endif
	"/loginduperr.htm",
	"/loginpserr.htm",
	"/menu.htm",
	"/reboot.htm",
	"/security_log.htm",
	"/setup_top.htm",
	"/ssid24g.htm",
	"/ssid5g.htm",
	"/ssid5ghb.htm",
	"/ssid_bt.htm",
	"/ssid_guest.htm",
	"/static_wireless.htm",
	"/status.htm",
	"/status_lan_device.htm",
	"/status_lan_device_offline.htm",
	"/sysok.htm",
	"/system_f.htm",
	"/firmware_upgrade.htm",
	"/system_main.htm",
	"/system_p.htm",
	"/system_r.htm",
	"/token_info.htm",
	"/wireless_WPS.htm",
	"/wireless_advanced.htm",
	"/wireless_apcli_main_24g.htm",
	"/wireless_apcli_main_5g.htm",
	"/wireless_apcli_status.htm",
	"/wireless_main.htm",
	"/wireless_wmm.htm",
	"/wireless_wmm_5G.htm",
	"/wps_showInit.htm",
	"/wps_showProcess.htm",
	"/steering.htm",
	"/logging.htm",
	"/discs.htm",
	"/system_fw_polling.htm",	
	"/firewall_mac.htm",
	"/firewall_mac_ed.htm",
	"/access_control.htm",
	"/warning.htm",
	"/setup_top_help.htm",
	"/setup_top_login.htm",
	"/firmware_upgrade.htm",
	"/fw_upgrade.htm",
	"/system_info.htm",
	"/images/",
	"/lang/",
	"/js/",
	"/css/",
	"/tmp/sys_log.log",
	/* cgi */
	"/cgi/",
	"/apply.cgi",
	"/upload.cgi",
	"/login.cgi",
	"/logout.cgi",
	"/setpwd.cgi",
	"/wz_setpwd.cgi",
	"/apply_abstract.cgi",
	"/lte_settings.cgi",
	"/wl_cal.cgi",
	"/cgi-bin/toplogy.cgi",
	"/cgi-bin/get_logfile.cgi",
	"/cgi-bin/set_led.cgi",
	"/cgi-bin/set_status_station.cgi",
	"/cgi-bin/set_redevicename.cgi",
	"/cgi-bin/set_control_internet.cgi",
	"/cgi-bin/rescan.cgi",
	"/cgi-bin/frescan.cgi",
	"/fwcheck.cgi",
	
	"/favicon.ico",
	NULL
};

extern security_t *sec;

extern int same_subnet(uint32 ip1, uint32 ip2, uint32 mask);

int match_ip(sock_t ip1, sock_t ip2)
{
	unsigned short sa_family;

	if (ip1.sa.sa_family != ip2.sa.sa_family)
		return 0;

	sa_family = ip1.sa.sa_family;

	if (sa_family == AF_INET) {
		return (ip1.sa_in.sin_addr.s_addr == ip2.sa_in.sin_addr.s_addr);
	}
#ifdef IPV6_SUPPORT
	else if (sa_family == AF_INET6) {
		/*ten_zheng, 20130605, begin, modified the 3rd parameter for http login with IPv6 address.*/
		return !memcmp(&ip1.sa_in6.sin6_addr.s6_addr, &ip2.sa_in6.sin6_addr.s6_addr, sizeof(ip1.sa_in6.sin6_addr));
		/*ten_zheng, 20130605, end*/
	}
#endif

	return 0;
}

void set_login_timeout(int mins)
{
	login_timeout = mins * 60;
}

#ifdef CHECK_LOGIN_BROWSER_SUPPORT
void grant_user(sock_t ip, char *agent)
{
	/* write down the ip and time */
	memcpy(&granted_ip, &ip, sizeof(sock_t));
	strcpy(&granted_agent, agent);
	granted_time = get_time();
	has_logined = 1;

	return;
}
#else
void grant_user(sock_t ip)
{
	/* write down the ip and time */
	memcpy(&granted_ip, &ip, sizeof(sock_t));
	/*granted_time = time(NULL);*/
	//get the system uptime instead of system time.
	granted_time = get_time();
	has_logined = 1;

	return;
}
#endif

void clear_granted_user(struct request_rec *r)
{
	/* We just allow the granted user logout. */

	if (match_ip(r->remote_ip, granted_ip))
	{
		memset(&granted_ip, 0, sizeof(sock_t));
		granted_time = 0;
		has_logined = 0;
#ifdef CHECK_LOGIN_BROWSER_SUPPORT
		memset(&granted_agent, 0, sizeof(granted_agent));
#endif
		ht_dbg("%s has logouted.\n", r->remote_ipstr);
	}
	else
		ht_dbg("%s has not logined yet.\n", r->remote_ipstr);

	return;
}

sock_t *get_granted_ip(void)
{
	return &granted_ip;
}

time_t get_granted_time(void)
{
	return granted_time;
}

int get_login_timeout(void)
{
	return login_timeout;
}

int is_http_logined(void)
{
	return has_logined;
}

int get_pw(char *name, char **pw)
{
	char pwd[64];
	int tid;

	if((tid = get_tid()) == MID_FAIL)
		return 0;

	if (mapi_ccfg_match_str(tid, "ARC_SYS_LoginName", name)) {
		*pw = mapi_ccfg_get_str(tid, "ARC_SYS_Password", pwd, sizeof(pwd));
		return 1;
	}

	return 0;
}

/* For support only one Administrator */
#ifdef CHECK_LOGIN_BROWSER_SUPPORT
int check_grant(sock_t ip, char* user_agent) 
#else
int check_grant(sock_t ip) 
#endif
{
	//ht_dbg("login_timeout=[%d], now=[%d]\n", login_timeout, (time(NULL)-granted_time));
	/*if (time(NULL) - granted_time < login_timeout)*/
	//get the system uptime instead of system time.
	if (get_time() - granted_time < login_timeout)
	{
#ifdef CHECK_LOGIN_BROWSER_SUPPORT	
		if (match_ip(ip, granted_ip) && user_agent && !strcmp(user_agent, granted_agent)) 
#else
		if (match_ip(ip, granted_ip)) 
#endif
		{

			/*granted_time = time(NULL);*/
			//get the system uptime instead of system time.
			granted_time = get_time();
			return HT_GRANTED;

		} else {
			/* this IP is not granted_ip */
			printf("%s[%d]\n", __func__, __LINE__);
			return HT_NEWUSER;
		}
	} else {
		/* timeout */
		granted_time = 0;
		has_logined = 0;

		if (match_ip(ip, granted_ip))
		{
			memset(&granted_ip, 0, sizeof(sock_t));

			return HT_TIMEUP;
		} else {
			memset(&granted_ip, 0, sizeof(sock_t));

			/* this IP is not granted_ip */
			return HT_NEWUSER;
		}
	}
}

void auth_bong(char *s, struct request_rec *r) 
{
	char tbuf[162];
	/* debugging */

	if (s) {
		snprintf(tbuf, sizeof(tbuf)-1, "authorization: %s", s);
		tbuf[sizeof(tbuf)-1] = '\0';
//		log_error(tbuf);
	}


	snprintf(tbuf, sizeof(tbuf)-1, "Basic realm=\"%s\"", r->auth_name);
	tbuf[sizeof(tbuf)-1] = '\0';

	answer(r, AUTH_REQUIRED, tbuf);
	return;
}


int check_auth(register struct request_rec *r)
{
	struct in_addr iip;
	char tmpbuf[256] = {0};
	char buf[1024] = {0};
	char mac[20] = {0};
	int log_enable;
	int tid;

	if((tid = get_tid()) == MID_FAIL)
		return 1;

	log_enable = mapi_ccfg_match_str(tid, "ARC_SYS_LogEnable", "1") ? 1 : 0;

	if (log_enable) {
		get_mac_by_ip(r->remote_ipstr, mac);
	}

	/* check if already login-ed */
#ifdef CHECK_LOGIN_BROWSER_SUPPORT
	switch (check_grant(r->remote_ip, r->user_agent))
#else
	switch (check_grant(r->remote_ip))
#endif
	{
		case HT_GRANTED:
			ht_dbg("%s has already granted, pass\n", r->remote_ipstr);

			return 0;

		case HT_TIMEUP:
			ht_dbg("%s login time out, reauth\n", r->remote_ipstr);

			/*
 			pete_zhang 2013-12-20	ticket-1167

			description: GUI security enhancement
 			*/
#ifdef CONFIG_HTTPD_TOKEN_CHECK_SUPPORT
			httoken_entry_remove( HTTOKEN_REMOVE_ALL );
#endif
			/*end pete_zhang        2013-12-20*/

			/* Redirect to login.stm */
			snprintf(buf, sizeof(buf), "Location: /login.htm\n\n");
			break;

		case HT_NEWUSER:
			ht_dbg("new user %s(%s) comes to login, check user and auth\n", r->remote_ipstr, r->user);
			/*snprintf(buf, sizeof(buf), "Location: /login.htm?URL=%s\n\n", r->url + 1); [> + 1 for skip '/' <]*/
			snprintf(buf, sizeof(buf), "Location: /login.htm\n\n");
			break;
	}


	if (log_enable) {
		snprintf(tmpbuf, sizeof(tmpbuf), "User from %s(%s) authentication fail.", mac, r->remote_ipstr);
		append_to_file(SECURITY_LOG_TMP, tmpbuf);
	}

	//answer(r, REDIRECT, buf);
	send_response(r, buf);

	return 1;
}

int evaluate_access(const char *url, int is_dir, struct request_rec *r)
{
	if (!r)
		return 0; 

	if (r->bypass_flag)
		return 0; 
	
	return check_auth(r);
}

/* init variables */
void arcadyan_init_hook(http_conf_t *hc)
{
	int	val;
	int idx;
	char buf[128];
	int tid;

	if((tid = get_tid()) == MID_FAIL)
		return;

	if (hc == NULL)
		return;


	idx = get_free_sec();
	if (idx != -1) {
		idx = idx%MAX_SECURITY;
		strcpy(hc->sec[idx].d, "*.asp");
		strcpy(hc->sec[idx].auth_name, "Arcadyan");
	}


	/* login timeout */
	val = atol(mapi_ccfg_get_str(tid, "ARC_SYS_LoginTimeout", buf, sizeof(buf)));
	set_login_timeout(val);

	return ;
}


void arcadyan_open_hook(void)
{
	return;
}



/* vendor hook to handle the request.
 *
 * return value:
 *
 * V_ACCEPT: request has been accepted by this hook.
 * V_DROP: request has been eaten by this hook.
 */
int arcadyan_request_hook(struct request_rec *r)
{
	
	/* Home page, skip auth */
	if (!strcmp(r->url, "/"))
		r->bypass_flag = 1;

	return V_ACCEPT;
}


