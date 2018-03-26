/*
 * h_config.c: Hard-coded HTTP daemon's parameters in code. It can prevent
 *      any reading operation in a diskless system
 *
 * C. H. Lin
 *
 */


#include "httpd.h"

/* Server config globals */

int ht_timeout;
int ht_port = 80;                /* port in host byte order */
char *index_names;
char *document_root = NULL;
alias_t *aliases;
security_t *sec;

// default pages
#ifdef CONFIG_WWW_DEFAULT_PAGE
	#define HTTPD_INDEX_PAGE CONFIG_WWW_DEFAULT_PAGE
#else
	#define HTTPD_INDEX_PAGE "index.htm"
#endif
// login pages
#ifdef CONFIG_WWW_LOGIN_PAGE
	#define HTTPD_LOGIN_PAGE CONFIG_WWW_LOGIN_PAGE
#else
	#define HTTPD_LOGIN_PAGE "login.htm"
#endif
// login error pages
#ifdef CONFIG_WWW_LOGIN_FAIL_PAGE
	#define HTTPD_LOGIN_FAIL_PAGE CONFIG_WWW_LOGIN_FAIL_PAGE
#else
	#define HTTPD_LOGIN_FAIL_PAGE "loginpserr.htm"
#endif
// login duplicate pages
#ifdef CONFIG_WWW_LOGIN_DUP_PAGE
	#define HTTPD_LOGIN_DUP_PAGE CONFIG_WWW_LOGIN_DUP_PAGE
#else
	#define HTTPD_LOGIN_DUP_PAGE "loginduperr.htm"
#endif

// Apply result
#ifdef CONFIG_WWW_APPLY_SUCCESS_PAGE
	#define HTTP_APPLY_SUCCESS_PAGE CONFIG_WWW_APPLY_SUCCESS_PAGE
#else
	#define HTTP_APPLY_SUCCESS_PAGE "Success.htm"
#endif
#ifdef CONFIG_WWW_APPLY_FAIL_PAGE
	#define HTTP_APPLY_FAIL_PAGE CONFIG_WWW_APPLY_FAIL_PAGE
#else
	#define HTTP_APPLY_FAIL_PAGE "Fail.htm"
#endif

// Upgrade result
#ifdef CONFIG_WWW_UPGRADE_SUCCESS_PAGE
	#define HTTP_UPGRADE_SUCCESS_PAGE CONFIG_WWW_UPGRADE_SUCCESS_PAGE
#else
	#define HTTP_UPGRADE_SUCCESS_PAGE "Success_upgrade.htm"
#endif
#ifdef CONFIG_WWW_UPGRADE_FAIL_PAGE
	#define HTTP_UPGRADE_FAIL_PAGE CONFIG_WWW_UPGRADE_FAIL_PAGE
#else
	#define HTTP_UPGRADE_FAIL_PAGE "Fail_upgrade.htm"
#endif

// Restore result
#ifdef CONFIG_WWW_RESTORE_SUCCESS_PAGE
	#define HTTP_RESTORE_SUCCESS_PAGE CONFIG_WWW_RESTORE_SUCCESS_PAGE
#else
	#define HTTP_RESTORE_SUCCESS_PAGE "Success_restore.htm"
#endif
#ifdef CONFIG_WWW_RESTORE_FAIL_PAGE
	#define HTTP_RESTORE_FAIL_PAGE CONFIG_WWW_RESTORE_FAIL_PAGE
#else
	#define HTTP_RESTORE_FAIL_PAGE "Fail_restore.htm"
#endif

char *HTTP_INDEX=HTTPD_INDEX_PAGE;
// login
char *HTTP_LOGIN=HTTPD_LOGIN_PAGE;
char *HTTP_LOGIN_FAIL=HTTPD_LOGIN_FAIL_PAGE;
char *HTTP_LOGIN_SUCCESS=HTTPD_INDEX_PAGE;
char *HTTP_LOGIN_DUP=HTTPD_LOGIN_DUP_PAGE;
// apply
char *HTTP_APPLY_SUCCESS=HTTP_APPLY_SUCCESS_PAGE;
char *HTTP_APPLY_FAIL=HTTP_APPLY_FAIL_PAGE;
// upgrade
char *HTTP_UPGRADE_SUCCESS=HTTP_UPGRADE_SUCCESS_PAGE;
char *HTTP_UPGRADE_FAIL=HTTP_UPGRADE_FAIL_PAGE;
// restore
char *HTTP_RESTORE_SUCCESS=HTTP_RESTORE_SUCCESS_PAGE;
char *HTTP_RESTORE_FAIL=HTTP_RESTORE_FAIL_PAGE;


// MIME-type constants
char CGI_MAGIC_TYPE[] = "application/x-httpd-cgi";
char INCLUDES_MAGIC_TYPE[] = "text/x-server-parsed-html";
char HTML_TYPE[] = "text/html";
char BINARY_TYPE[] = "application/octet-stream";
char ZIP_TYPE[] = "application/x-zip-compressed";
char JPEG_TYPE[] = "image/jpeg";
char V_MPEG_TYPE[] = "video/mpeg";
char A_MPEG_TYPE[] = "audio/mpeg";
char MIDI_TYPE[] = "audio/midi";
char QT_TYPE[] = "video/quicktime";
char XML_TYPE[] = "text/xml";
char JAVASCRIPT_TYPE[] = "application/x-javascript"; // or "application/javascript"

char *default_type = HTML_TYPE;


pthread_mutex_t TIMER_RESOURCE = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t OS_RESOURCE = PTHREAD_MUTEX_INITIALIZER;

char lanip_str[16];
char lanip_dev[16];
char lanip6_str[64];
char lanmask_str[16];

char wanip_str[16];
char wanip_dev[16];
char wanip6_en[4];
char wanip6_str[64];

extern CGI_ENTRY cgi_tbl[];

// token ON/OFF swicth
#ifdef CONFIG_HTTPD_SUPPORT_TOKEN_DISABLE
int G_token_off_flag=0;
#endif //CONFIG_HTTPD_SUPPORT_TOKEN_DISABLE

/* -----------------------------------------------------------------------
 * System databases. Hard coding for reducing the memory space requirement.
 * ----------------------------------------------------------------------*/
http_conf_t hconf, *hc = &hconf;

/* Mime-type definitions. */
struct mime_ext types[] = {
	{"gif", "image/gif"},
//	{"htm", HTML_TYPE},
	{"htm", INCLUDES_MAGIC_TYPE},
	{"sht", INCLUDES_MAGIC_TYPE},
	{"stm", INCLUDES_MAGIC_TYPE},
	{"asp", INCLUDES_MAGIC_TYPE},
	{"js",  JAVASCRIPT_TYPE}, //INCLUDES_MAGIC_TYPE,
	{"txt", "text/plain"},
#if defined (VRV9510RWAC) || defined(VR7516QW22)//Velmurugan 06152015> Avoid browser open the config file while backup */
	{"cfg", "application/force-download"},
#else
	{"cfg", "text/plain"},
#endif
	{"conf", "text/plain"},  // Belkin supports .conf file
	{"log", "text/plain"},  // Belkin supports .log file
	{"jpg", JPEG_TYPE},
	{"cgi", CGI_MAGIC_TYPE},
	{"exe", CGI_MAGIC_TYPE},
	{"nph", CGI_MAGIC_TYPE},
	{"xml", XML_TYPE},
	{"css", "text/css"},
	{"html", HTML_TYPE},
	{"jpeg", JPEG_TYPE},
	{"jpe", JPEG_TYPE},
	{"shtm", INCLUDES_MAGIC_TYPE},
	{"shtml", INCLUDES_MAGIC_TYPE},
	{"png", "image/png"},
	{"pnm", "image/x-portable-anymap"},
	{"pbm", "image/x-portable-bitmap"},
	{"pgm", "image/x-portable-graymap"},
	{"ppm", "image/x-portable-pixmap"},
	{"var", "application/x-type-map"},
	{"xbm", "image/x-xbitmap"},
	{"xpm", "image/x-xpixmap"},
	{"xwd", "image/x-xwindowdump"},
	{"tif", "image/tiff"},

	{"bin", BINARY_TYPE},
	{"pcap", BINARY_TYPE},		// alex_chen added for tcpdump packets	
	{"zip", ZIP_TYPE},
	{"rar", ZIP_TYPE},
	{"arc", ZIP_TYPE},
	{"arj", ZIP_TYPE},

	{"asf", "video/x-ms-asf"},
	{"asx", "video/x-ms-asx"},
	{"avi", "video/avi"},
	{"bmp", "image/bmp"},
	{"dat", "video/x-ms-dat"},		// ??
	{"ico", "image/x-icon"},
	{"m3u", "audio/m3u"},
	{"mid", MIDI_TYPE},
	{"midi", MIDI_TYPE},
	{"mov", QT_TYPE},
	{"mp3", A_MPEG_TYPE},
	{"mpeg", V_MPEG_TYPE},
	{"mpg", V_MPEG_TYPE},
	{"qt", QT_TYPE},
	{"ra", "audio/x-realaudio"},
	{"rv", "video/vnd.rn-realvideo"},
	{"wav", "audio/wav"},
	{"wma", "audio/x-ms-wma"},
	{"wmv", "video/x-ms-wmv"},		// ??
	{"swf", "application/futuresplash"},

	{"doc", "application/msword"},
	{"ppt", "application/vnd.ms-powerpoint"},
	{"xls", "application/vnd.ms-excel"},

	{"ovpn", "application/force-download"},

	{NULL, NULL}
};

struct mime_ext encoding_types[] = {
	//"gz", "application/x-gzip",
	{"Z", "x-compress"},
	{NULL, NULL},
};


/*------------------------------------------------------------------------*/
char *server_admin()
{
	return hc->server_admin;
}


void get_local_sock(struct request_rec *r)
{
	socklen_t name_len;
	int ret;

	name_len = sizeof(sock_t);
	ret = getsockname(r->sd, (struct sockaddr *)&r->server_ip.sa, &name_len);
	if (ret != 0)
		return;

	if (r->server_ip.sa.sa_family == AF_INET) {
		struct sockaddr_in *s = (struct sockaddr_in *)&r->server_ip.sa_in;

		r->server_port = htons(s->sin_port);
		inet_ntop(AF_INET, &s->sin_addr, r->server_ipstr, sizeof(r->server_ipstr));

	}
#ifdef IPV6_SUPPORT
	else if (r->server_ip.sa.sa_family == AF_INET6) {
		struct sockaddr_in6 *s = (struct sockaddr_in6 *)&r->server_ip.sa_in6;

		r->server_port = htons(s->sin6_port);
		inet_ntop(AF_INET6, &s->sin6_addr, r->server_ipstr, sizeof(r->server_ipstr));
	}
#endif

	ht_dbg("Local: %s:%d\n", r->server_ipstr, r->server_port);
}

void get_remote_sock(struct request_rec *r)
{
	socklen_t name_len;
	int ret;

	name_len = sizeof(sock_t);

	ret = getpeername(r->sd, (struct sockaddr *)&r->remote_ip.sa, &name_len);
	if (ret != 0)
		return;

	if (r->remote_ip.sa.sa_family == AF_INET) {
		struct sockaddr_in *s = (struct sockaddr_in *)&r->remote_ip.sa_in;

		r->remote_port = htons(s->sin_port);
		inet_ntop(AF_INET, &s->sin_addr, r->remote_ipstr, sizeof(r->remote_ipstr));
	}
#ifdef IPV6_SUPPORT
	else if (r->remote_ip.sa.sa_family == AF_INET6) {
		struct sockaddr_in6 *s = (struct sockaddr_in6 *)&r->remote_ip.sa_in6;

		r->remote_port = htons(s->sin6_port);
		inet_ntop(AF_INET6, &s->sin6_addr, r->remote_ipstr, sizeof(r->remote_ipstr));
	}
#endif

	ht_dbg("Remote: %s:%d\n", r->remote_ipstr, r->remote_port);
}


void init_global(void)
{
	char buf[128];
	int tid;

	if((tid = get_tid()) == MID_FAIL)
		return;

	memset(hc, 0, sizeof(struct http_conf));

	hc->port = DEFAULT_HTTP_PORT;
	ht_timeout = hc->timeout = DEFAULT_TIMEOUT;
	memcpy(hc->server_admin, DEFAULT_SERVER, strlen(DEFAULT_SERVER));

	/* default: /www */
	snprintf(hc->document_root, sizeof(hc->document_root), "%s", document_root ? document_root : DEFAULT_DOC_ROOT);
	document_root = hc->document_root;
	chdir(document_root);

	/* Vendor can override the index_names */
	memcpy(hc->index_names, DEFAULT_INDEX, strlen(DEFAULT_INDEX));
	index_names = hc->index_names;

	/* Vendor can override the aliase */
	aliases = hc->aliases;

	/* for download file, redirect to /tmp */
	sprintf(aliases[0].fake, "/tmp/");
	sprintf(aliases[0].real, "/tmp/");
	aliases[0].script = STD_DOCUMENT;

	/* Vendor can override the sec */
	sec = hc->sec;



	/* initialize CGI script translation table */
	CGI_init(cgi_tbl);

	strcpy(lanip_str, mapi_ccfg_get_str(tid, "ARC_LAN_0_IP4_Addr", buf, sizeof(buf)));
	strcpy(lanmask_str, mapi_ccfg_get_str(tid, "ARC_LAN_0_IP4_Netmask", buf, sizeof(buf)));
	strcpy(lanip6_str, mapi_ccfg_get_str(tid, "ARC_LAN_0_IP6_LLAAddr", buf, sizeof(buf)));
	strcpy(lanip_dev, mapi_ccfg_get_str(tid, "ARC_LAN_0_Ifname", buf, sizeof(buf)));

#ifdef CONFIG_HTTPD_SUPPORT_TOKEN_DISABLE
	/* special case for token on/off
	 *  0: means enabled
	 *  1: turn off token sesison verify
	 */
    if(mapi_ccfg_get_str(tid, "ARC_UI_WEB_TOKEN_Disable", buf, sizeof(buf)) != NULL)
	{
		G_token_off_flag=atoi(buf);
    }
//    cprintf("G_token_off_flag=%d\n",G_token_off_flag);

#endif //CONFIG_HTTPD_SUPPORT_TOKEN_DISABLE
	return;
}

