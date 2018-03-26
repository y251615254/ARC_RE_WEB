/*
 * httpd.h: header for simple (ha! not anymore) http daemon
 */
#ifndef __HTTPD_H__
#define __HTTPD_H__

#include <sys/types.h>  //for accept()
#include <sys/socket.h> //for accept()
#include <sys/wait.h>		//for WIFEXITED in pclose()
#include <netdb.h>		//for gethostbyname()
#include <unistd.h>		//for close()
#include <pthread.h>	//for pthread related functions
#include <semaphore.h>
#include <stdlib.h>		//for rand() and others
#include <signal.h>
#include <stdio.h>		//for fgets() and other file I/O
#include <stdarg.h>		//for va_list in vsprintf()
#include <string.h>		//for str*()
#include <ctype.h>		//for isspace,.. etc
#include <net/if.h>		// for umngif.h, IFNAMSIZ

#include <time.h>
#include <sys/sysinfo.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>

#include "ARC_GLOBAL.h"
#include <mid_mapi_midcore.h>
#include <action.h>
#include <limits.h>

#include "util.h"
#include "timer.h"
#include "fs.h"
#include "arc_features.h"

#ifdef CONFIG_HTTPD_SUPPORT_AES256
#include "aes256cbc.h"
#endif

#ifdef SYSLOG_ENHANCED
#include "arc_syslog.h"
#endif

/*
 * HTTPD's special features
 */
#if defined(ARC_CODE_SHRINK)
#define MAX_REQ	5
#else
#define MAX_REQ	10
#endif
#define DEBUG


#ifdef CONFIG_HTTPD_HTTPS_SUPPORT
#include "ssl_api.h"
#if defined(WE410443)
#define	KEY_FILE	"/usr/sbin/httpd.key"
#define	CERT_FILE	"/usr/sbin/httpd.crt"
#else
#define	KEY_FILE	"/usr/sbin/key.pem"
#define	CERT_FILE	"/usr/sbin/cert.pem"
#endif
#endif

#define V_ACCEPT	1
#define V_DROP		2


#ifdef EXTERN_CGI_SUPPORT
#define	MAX_BIN_SIZE	128
#define	MAX_ENV_SIZE	256
#define	BINARY_PATH		"/usr/sbin/cgi-bin/"
#define SHELL_PATH		"/bin/sh"
#define	TYPE_SHELL	1
#define	TYPE_BINARY	2
#endif

/* Velmurugan 12232014> Encryption key for backup configuration*/
#if defined (VRV9510RWAC) || defined(VR7516QW22) || defined(VR7516RW22) || defined(WE410443)
//#define CONFIG_BACKUP_KEY "b05U42ywoNWfok&3"

#ifdef WE410443_TS
#define CONFIG_BACKUP_KEY "arc256TS"   //Modifying key is for consistency with decryption tools under Windows systems..by leo
#else
#define CONFIG_BACKUP_KEY "b05U42$1"
#endif

#define MD5_length             32
#define MD5_location   10
#endif


#define MAX_CGI_VARS	256 //128
#define	CGI_CODE	2

#define	DELAY(nticks)	usleep((nticks)*1000L)
#define DELAYMS(msecs)	usleep((msecs)*1000L)
#define SLEEP(nsecs)	sleep(nsecs)
#define LOCK_TIMER()		pthread_mutex_lock(&TIMER_RESOURCE)
#define UNLOCK_TIMER()		pthread_mutex_unlock(&TIMER_RESOURCE)

/* Locking fs for reading-only web page is not neccessary. */
#define LOCK_FS()           do {} while (0)
#define UNLOCK_FS()         do {} while (0)

#define ENTER_CRITICAL()	pthread_mutex_lock(&OS_RESOURCE)
#define EXIT_CRITICAL()		pthread_mutex_unlock(&OS_RESOURCE)

#define LOCK_CGI()          pthread_mutex_lock(&CGI_RESOURCE)
#define UNLOCK_CGI()        pthread_mutex_unlock(&CGI_RESOURCE)


//#define SYS_CLOCK()	        clock()
//#define CLOCKHZ             CLOCKS_PER_SEC
#define CONFIG_HZ			100
#define SYS_CLOCK()			get_sys_time()
#define CLOCKHZ				CONFIG_HZ

#ifndef __STR
#define __STR(x) #x
#endif
#ifndef STR
#define STR(x) __STR(x)
#endif

typedef struct words_s{
	char *key;
} words;


/* -- Port number for server running standalone -- */
#define DEFAULT_HTTP_PORT	80
#define DEFAULT_HTTPS_PORT	443
#define DEFAULT_DOC_ROOT	"/www"
/* The timeout for waiting for messages */
#define DEFAULT_TIMEOUT		10
/* -- Define this to be what your HTML directory content files are called -- */
#define DEFAULT_INDEX	"index.htm index.asp index.stm"
#define DEFAULT_SERVER	"httpd@arcadyan.com.cn"



/* The default string lengths */
#define HUGE_STRING_LEN		480
#define MAX_STRING_LEN		256
#define MAX_TIMESTAMP_LEN 	32

/* The size of the server's internal read-write buffers */
#define SOPRINTFBUFSIZE 512
#define IOBUFSIZE       4096
#define INBUFSIZE       2000
/* RFC 1123 format for date - this is what HTTP/1.0 wants */
#define HTTP_TIME_FORMAT	"%a, %d %b %Y %H:%M:%S GMT"


/* ------------------------------ error types ------------------------------ */
#define SERVER_VERSION		"Arcadyan httpd 1.0"
#define SERVER_PROTOCOL		"HTTP/1.1"
#define SERVER_SUPPORT		"http://www.apache.org/"

#define DOCUMENT_FOLLOWS    200
#define REDIRECT            302
#define USE_LOCAL_COPY      304
#define BAD_REQUEST         400
#define AUTH_REQUIRED       401
#define FORBIDDEN           403
#define NOT_FOUND           404
#define SERVER_ERROR        500
#define NOT_IMPLEMENTED     501
#define SERVICE_UNAVAILABLE 503
#define NO_MEMORY           6992

/* Types of echo parameter */
#define STR_FMT		-1
#define INT_FMT		-2
#define LONG_FMT	-3
#define BYTE_FMT	-4
#define IP_FMT		-5
#define USHORT_FMT	-6
#define ULONG_FMT	-7
#define LIP_FMT 	-8
#define CHAR_FMT	-9
#define SHORT_FMT	-10
#define SPC_STR_FMT	-11
#define PASSWD_FMT  -12


/* Object types */
#define REDIRECT_URL	-1
#define STD_DOCUMENT	0
#define SCRIPT_NCSA		1
#define SCRIPT_CGI		2
#define BAD_URL		3
#define HNAP_CGI		4

/* Just in case your linefeed isn't the one the other end is expecting. */
#define LF 10
#ifndef CR
#define CR 13
#endif

#ifdef IPV6_SUPPORT
#define MAX_IP_STRING	INET6_ADDRSTRLEN
#else
#define MAX_IP_STRING	INET_ADDRSTRLEN
#endif

#define OWL_LISTEN_PORT 55761

#define	ht_dbg(fmt, arg...)		printf("[%s] "fmt, __FUNCTION__ , ## arg)

#define foreach(word, wordlist, next) \
	for (next = &wordlist[strspn(wordlist, " ")], \
			strncpy(word, next, sizeof(word)), \
			word[strcspn(word, " ")] = '\0', \
			word[sizeof(word) - 1] = '\0', \
			next = strchr(next, ' '); \
			strlen(word); \
			next = next ? &next[strspn(next, " ")] : "", \
			strncpy(word, next, sizeof(word)), \
			word[strcspn(word, " ")] = '\0', \
			word[sizeof(word) - 1] = '\0', \
			next = strchr(next, ' '))



// #error : Please Make Sure : Not _ETCPIP
/* The maximum security data per directory */
#define INDEX_NAMES_SIZE	40
#define MAX_SECURITY		4
#define MAX_ALIAS           8
#define MAX_PASSWD          2
#define PATH_NAME_SIZE		32
#define USER_NAME_SIZE		16
#define AUTH_NAME_SIZE		32
// fix config_macros.h define STR_LEN , and use local define
//#define STR_LEN             128
#define ROOT_STR_LEN    128
#define DOM_LEN             40

#ifndef FALSE
#define FALSE 0
#endif
#ifndef TRUE
#define TRUE  1
#endif

typedef struct security_data {
	char d[PATH_NAME_SIZE];	//  d[0] == '\0' means null entry
	char auth_name[AUTH_NAME_SIZE];
}security_t;

typedef struct alias_data {
	char fake[PATH_NAME_SIZE];	// null entry if fake[0] == '\0'
	char real[PATH_NAME_SIZE];
	short script;
}alias_t;

typedef struct http_conf {
	unsigned short	port;
    unsigned short  remote_port;
	unsigned short	timeout;

	alias_t	aliases[MAX_ALIAS];
	security_t	sec[MAX_SECURITY];
	char	index_names[INDEX_NAMES_SIZE];
	char	document_root[ROOT_STR_LEN];
	char	server_admin[DOM_LEN];

}http_conf_t;


#define MIME_TYPE_LEN	64
struct mime_ext {
	char *ext;      /* file name's extension field */
	char *ct;       /* content type */
};

struct fd_entry {
	void *fd;
	struct fd_entry *next_fd;
};

#define MAX_ARGS_NUM	16
#define TH_INFO_SIZE	sizeof(struct th_info)
typedef enum {
	TH_STD,
	TH_CGI,
#ifdef HNAP_SUPPORT
	TH_HNAP,
#endif
}TH_TYPE;

typedef union
{
	struct sockaddr sa;
	struct sockaddr_in sa_in;
#ifdef IPV6_SUPPORT
	struct sockaddr_in6 sa_in6;
#endif
} sock_t;

/* tlhhh. structure for package thread params */
struct th_info {
	u_long pthread_id;	/* pthread id */
	int prior;			/* pthread priority */
	int stacksize;		/* running stack size */
	TH_TYPE type;		/* pthread type */
	//(void *)(void *, void *)do_task;	/* task routine */
	//(void *)(void *, void *)clean_up;	/* clean up after mission */
	int args_num;		/* args number */
	long args_list[MAX_ARGS_NUM];	/* args list */
};

/* bruce_ma. 20150206 */
struct user_session {
	int type;//local user, remote user, service user
	int request_type;//current request user type
	int has_logined;//if session have logined.
	time_t granted_time;//session grant time
	sock_t granted_ip;//session grant ip
	int login_timeout;//session timeout 
	int cookie_status;//0,disable 1,set 2,send
	char cookie_val[256];//the string store the value of string "Set-Cookie:SID=value;"
};

struct request_rec {
	/* task's resources */
	int	index;                   /* task's slot No.(=ID) */
	u_long	slot;                   /* task's slot No.(=ID) */
	u_long	cgi_slot;               /* CGI's task id if exist */
	int     sd;                     /* socket id */
	void	*fd;                    /* opened HTML file */
	struct fd_entry *fd_backup;	/* added in httpd v1.1 source code, */
					/* use link-list instead of single instance */
	FILE_INFO_t	finfo;
	COOKIE_t	cookie;		/* task's time stamp at initiating */

	/* auth */
	char    *auth_type;             /* authentication type: default is Basic */
	char    *auth_name;
	char    user[USER_NAME_SIZE];   /* user name in auth data */
	char    *auth_line;             /* copy of auth line in request */
	unsigned long token;			/* token session control*/
#ifdef CONFIG_HTTPD_SUPPORT_AES256
    aes256_context ctx;
#endif
	/* buffer and offset */
	char    in_buf[INBUFSIZE];  /* buffer for headers */
	int     in_buf_used;        /* how many bytes now is used */
	int     in_header_lens;     /* added to indicate the ending of header */
	char    *in_buf_read;		/* used for _getword() */
	char	out_buf[IOBUFSIZE];	/* socket outgoing buffer, size is multiple of 512 */
	int     out_pos;			/* length of data in outgoing buffer */
	char    *oheader;			/* buffer for headers in outgoing */
	int		oheader_len;		/* total length of outgoing headers */

	/* MIME information */
	char    *content_type;          /* mime type */
	char    content_encoding[MIME_TYPE_LEN];        /* encoding type */

	/* for log */
	int     status;
	int		bypass_flag;
	long    bytes_sent;

	sock_t	remote_ip;
	sock_t	server_ip;
	int		remote_port;
	int		server_port;
	char	remote_ipstr[MAX_IP_STRING];
	char	server_ipstr[MAX_IP_STRING];

	/* 20150206, bruce_ma: add user session struct here*/
	struct user_session * session;

	int	ishttps;
#ifdef CONFIG_HTTPD_HTTPS_SUPPORT
	ssl_task_t *ssrv;
#endif

	/* request info */
	char    location[HUGE_STRING_LEN];
	char    *referer;
	char    *ims;                   /* If-modified-since */
	char    last_modified[MAX_TIMESTAMP_LEN];
	char    expires[MAX_TIMESTAMP_LEN];
	loff_t  content_length;
	int     assbackwards;           /* is HTTP/0.9? */
	int     header_only;            /* is a HEAD method ? */
	int		keep_alive;
	char	url[MAX_STRING_LEN];
	char	pwd[FNAME_SIZE];
	char	*args;				/* CGI args: like name=value&name=value.. */
	char	*method;
	char	*host;
	char    *user_agent;
	char	*soapaction;
	char 	*accept_lang;
	char 	*http_accept;
	int 	x_wap_profile;
	char	*http_cookie_string;
};

typedef struct
{
	pthread_mutex_t f_lock;
	volatile int is_busy;
	int number_id;
	pthread_t   pthread_id;
	char data[TH_INFO_SIZE];
	struct request_rec	r;
#ifdef CONFIG_HTTPD_HTTPS_SUPPORT
	ssl_task_t ssl_srv;
#endif
	sem_t sem;
} workthread_t;

/* Structure defines CGI script lookup table */
typedef struct cgi_entry {
	char *path;
	void (* script)(void);
	int stack_size;
	int timeout;
} CGI_ENTRY;

#ifdef EXTERN_CGI_SUPPORT
typedef struct bin_entry {
	char *name;
	int type;
}BIN_ENTRY;
#endif

typedef struct CGI_info {
	struct request_rec *r;
	int	socketid;
	//FILE  *pipe[2];
#ifdef EXTERN_CGI_SUPPORT
	BIN_ENTRY *bin;
#endif
} CGI_info;

#ifdef CONFIG_HTTPD_Security_Attack_Protect

#define THREAD_ATTACK_COUNT 		1
#define MAX_ATTACK_ADDR_NUM 		10
#define MAX_ATTACK_CNT 				30
#define DETECT_ATTACK_INTERVAL 		30
#define TIMEOUT_BLOCK_ATTACK_ADDR	120
struct attack
{
	sock_t attack_ip;
	int timeinfo;
	int attck_cnt;
	int is_attacker;
};

typedef struct attack_array
{
	struct attack attack_addr[MAX_ATTACK_ADDR_NUM];
	int num;
}ATTACK_ARRAY;

ATTACK_ARRAY attack_entry;
#endif

struct vendor_ops {
	char *vendor;
	char *name;
	char *page_suffix;

	void (*init_hook)(http_conf_t *hc);
	void (*open_hook)(void);
	int (*request_hook)(struct request_rec *r);
	//int (*cgi_hook)();
	void (*check_session_hook)(void);
	void (*clear_session_hook)(void);
};

/* Global, global, who's got the globals? */
extern struct vendor_ops vendor_ops, *v_ops;
extern http_conf_t htconf, *hc;
extern pthread_mutex_t TIMER_RESOURCE;
extern pthread_mutex_t OS_RESOURCE;
extern pthread_mutex_t CGI_RESOURCE;
extern char lanip_str[16];
extern char lanip_dev[16];
extern char lanip6_str[64];
extern char lanmask_str[16];

extern char wanip_str[16];
extern char wanip_dev[16];
extern char wanip6_en[4];
extern char wanip6_str[64];

extern unsigned long sys_clock_res, sys_clock_hz;

/* Server config */
extern int ht_port;
extern int ht_prior;
extern int ht_timeout;

/* Document config */
extern char *index_names;  /* might have lots of names now */
extern char *document_root;
extern char *default_type;
extern char CGI_MAGIC_TYPE[];
extern char INCLUDES_MAGIC_TYPE[];
extern char MAP_FILE_MAGIC_TYPE[];
extern char ASIS_MAGIC_TYPE[];
extern char NULL_STR[];
extern char XML_TYPE[];
extern char JAVASCRIPT_TYPE[];

extern char *HTTP_INDEX;
// login
extern char *HTTP_LOGIN;
extern char *HTTP_LOGIN_FAIL;
extern char *HTTP_LOGIN_SUCCESS;
extern char *HTTP_LOGIN_DUP;
// apply
extern char *HTTP_APPLY_SUCCESS;
extern char *HTTP_APPLY_FAIL;
// upgrade
extern char *HTTP_UPGRADE_SUCCESS;
extern char *HTTP_UPGRADE_FAIL;
// restore
extern char *HTTP_RESTORE_SUCCESS;
extern char *HTTP_RESTORE_FAIL;

unsigned long get_sys_time(void);

/* htconf.c */
char 	*server_admin(void);
void	CGI_init(CGI_ENTRY cgi_tbl[]);

/* request.c */
void process_request(struct request_rec *r);
long send_fd(struct request_rec *r);
void timed_out_handler(COOKIE_t);
int find_script(struct request_rec *r);

/* http_get */
void process_get(struct request_rec *r);
void send_file(struct request_rec *r);
void download_file(struct request_rec *r);

/* post.c */
void process_post(struct request_rec *r);

/* script.c */
void exec_cgi_script(struct request_rec *r);
int cgi_stub(struct request_rec *r);

#ifdef HNAP_SUPPORT
extern void hnap_handle(int sock, const char *method, const char *auth_line, 
		const char *soapaction, long content_length, int ishttps, unsigned long ulRemoteIP, void *pssrv);
extern int checkFromLAN(unsigned int clientAddr);
void hnap_cgi_stub(struct request_rec *r);
#endif

/* log.c */
void log_error(char *err);
void title_html(struct request_rec *r, char *msg);
void answer(struct request_rec *req, int type, char *err_string);

/* mime.c */
int get_mime(struct request_rec *r);
int clear_vars(struct request_rec *r);
void send_http_header(struct request_rec *r);
void set_content_type(struct request_rec *r);
void set_expires(struct request_rec *r, TIME_t *t);
int set_last_modified(struct request_rec *r);
void probe_content_type(struct request_rec *r);
int check_device_type(char *http_accept, int x_wap_profile, char *cookie,  char *user_agent);

/* access.c */
extern int evaluate_access(const char *url, int is_dir, struct request_rec *r);
int translate_name(char *name);
void unmunge_name(char *name);

/* include.c */
void send_parsed_file(struct request_rec *r);


/* util.c */
void    http2cgi(char *w);
int     later_than(TIME_t *tms, char *i);
int     strcmp_match(char *str, char *exp);
int     is_matchexp(char *str);
void    strsubfirst(int start,char *dest, char *src);
void    make_full_path(const char *src1, const char *src2, char *dst);
int     is_url(char *u);
void    getparents(char *name);
void	no2slash(char *name);
char    *_getword(char *line, char stop);
void	getword(char *word, char *line, char stop);
void    get_remote_host(struct request_rec *r);
void    make_dirstr(char *s, int n, char *d);
char    *makeword(char *line, char stop);
void    strcpy_dir(char *d, const char *s);
void    unescape_url(char *url);
char	*str_trim( char *str, int str_len, int *del_len );
void    escape_url(char *url);
void    plustospace(char *str);
void    spacetoplus(char *str);
void    str_tolower(char *str);
int     so_flush(struct request_rec *r);
int     so_putc(int c, struct request_rec *r);
int		so_puts(struct request_rec *r, char *buffer);
int     so_printf(struct request_rec *r, const char *fmt, ...);
void    uudecode(char *s,unsigned char *d,int dl);
int     ind(const char *s, char c);
int     rind(char *s, char c);
char	*strcpy_guard(char *dst, unsigned int dstSize, const char *src);
char	*strcat_guard(char *dst, unsigned int dstSize, const char *src);
int	sprintf_guard(char *buf, unsigned int bufSize, char *fmt, ...);
int	get_param_value( const char *str, char deli_token, const char *para_name, char *value, int value_size );
int	get_query_value(struct request_rec *r, const char *para_name, char *value, int value_size );
void    construct_url(struct request_rec *r, char *d, char *s, char *local_host, unsigned short port, unsigned char ishttps);
int     is_null_string(char *ptr);

// added for http client and other APs
int strrncmp(const char *s1, const char *s2, size_t max_len);
time_t ht_cvt_time(char *tmStr);
char *get_mac_by_ip(char *ip, char mac[]);

// export function
int remove_listener(struct in_addr if_addr, u_short port);
int remove_listener6(struct in6_addr if_addr, u_short port); 
int add_listener(struct in_addr if_addr, u_short port, int ishttps);
int add_listener6(struct in6_addr if_addr, u_short port, char*dev, int ishttps);

int is_localhost(const char *ip);

extern int ht_create_thread(struct th_info *th_info);
extern char *get_line(struct request_rec *r, char *buffer, int size, FILE *stream);
extern int get_char(struct request_rec *r, FILE *stream);

/*
 * BYPASS CHECK API definition
 */
int bypass_check(const char *url);


char *strcasestr(const char *phaystack, const char *pneedle); // in util.c
void set_signals(void);
/*
 * ignore the SIGUSR1 signal which cause the main process to
 * call the reload_httpd() 
 * */
void sig_ignore_reload_httpd();

extern void init_global(void);
extern void init_listener(void);

extern int FSclose(void *fp);
extern char *FSgetcwd(struct request_rec *r);
extern void FSchdir(struct request_rec *r, char *file);
extern void FSchdir_file(struct request_rec *r, char *dir);

extern int garbage_collect(struct request_rec *r, int sock, int max_size);
extern void send_response(struct request_rec *r, char *resp);

int get_free_sec(void);
int get_free_alias(void);

/* Terry 20141027, for o2 redirect URL */
#ifdef VRV9518SWAC33
extern int o2_redirect(int s, struct request_rec *r);
extern void o2_get_local_addresses();
#endif

#ifdef CONFIG_HTTPD_Security_Attack_Protect
void check_addr_is_attacker(sock_t ip);
int find_entry_in_attack_array(sock_t ip);
void insert_entry_info_attack_array(sock_t ip, int timeinfo);
int is_block_attack_addr(sock_t ip);
#endif

#if defined(WE410443_SA) || defined(WE5202243_SA) || defined(WE410223_SA) || defined(WE410443_TS) || defined(WE410443_TA) || defined(WE410443_A1) || defined(WE410443_6DX) || defined(WE410443_ZZ)
#define URL_FLAG(FwAutoOptIn, FwCheckDevice, FwCheckType) ((FwAutoOptIn<<8) | (FwCheckDevice<<4) | FwCheckType)
typedef enum _FW_OPT_IN
{
	OPT_IN_NO = 0,
	OPT_IN_YES,
}FW_OPT_IN;
typedef enum _FW_CHECK_DEVICE
{
	CHECK_DEVICE_IOS = 0,
	CHECK_DEVICE_AND,
	CHECK_DEVICE_WEB,
	CHECK_DEVICE_SYS,
}FW_CHECK_DEVICE;
typedef enum _FW_CHECK_TYPE
{
	CHECK_TYPE_AUTO = 0,
	CHECK_TYPE_STARTUP,
	CHECK_TYPE_MANUAL,
}FW_CHECK_TYPE;
#endif

typedef struct _login_info
{
	unsigned char error;
	unsigned int error_count;
	long error_uptime;
	unsigned int error_wait_time;
	pthread_mutex_t lock;
}LOGIN_INFO;
LOGIN_INFO login_info;

#endif	/* __HTTPD_H__ */

