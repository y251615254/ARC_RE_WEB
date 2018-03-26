#ifdef _NEWLIB
#include "string.h"
#endif

#include "httpd.h"
#include "ssi_handler.h"
#include "ssi_files_list.h"



#define STARTING_SEQUENCE		"<%"
#define ENDING_SEQUENCE			"%>"
#define MAX_DIRECTIVE_LEN		128

/*
  jerry_deng 2012.12.12 Belkin Bug ARCADYAN-2245
  Bug description: Webpage load time need improvements
  Solution:  For the web pages which do NOT contain ssi functions, we can directly print out.
	  For the web pages which contain ssi functions, we use fread() to read many bytes from /www a time.
*/
#define READ_BUF_SIZE				4096

struct read_info
{
	int index;
	int read_bytes;
	int eof;
	char buf[READ_BUF_SIZE];
};

static int file_contain_ssi(register struct request_rec *r);
static int find_string(register struct request_rec *r, char *str, int out, struct read_info *pRead);
static int get_directive(register struct request_rec *r, char *d, struct read_info *pRead);
/* end jerry_deng 2012.12.12 Belkin Bug ARCADYAN-2245 */
int fmt_args(int argc, char **argv, char *fmt, ...);
static char *unqstrstr(char *haystack, char *needle);
static char *get_args(char *args, char **next);
static void	handle_directive(char *directive, register struct request_rec *r); 
int send_included_file(register struct request_rec *r, char *file);
void send_parsed_content(register struct request_rec *r);
void send_parsed_file(struct request_rec *r);

extern void *FSopen(struct request_rec *r, char *fname);
extern char *FSgetcwd(struct request_rec *r);
extern void FSchdir_file(struct request_rec *r, char *file);
extern int FSstat(struct request_rec *r, char *path, FILE_INFO_t *finfo);
extern void FSchdir(struct request_rec *r, char *file);
extern int FSgetc(void *fp);
extern int FSclose(void *fp);

/*
  jerry_deng 2012.12.12 Belkin Bug ARCADYAN-2245
*/
#define GET_CHAR(fd, c, code, pRead) \
 { \
	\
	 /* If pRead is just initialized or all the bytes in the buf is read, then continue to read. */ \
	 if(((pRead)->index == 0) || ((pRead)->index >= READ_BUF_SIZE)) { \
		 (pRead)->read_bytes = fread((pRead)->buf, 1, READ_BUF_SIZE, (fd)); \
		 (pRead)->index = 0; \
		 if((pRead)->read_bytes < READ_BUF_SIZE) { \
			 if(feof(fd)) { \
				 (pRead)->eof = 1; \
				 FSclose(fd); \
				 fd = (FILE_t) NULL; \
			 } \
			 else if(ferror(fd)) { \
				 ht_dbg("[%s] read error\n", __FUNCTION__); \
				 FSclose(fd); \
				 fd = (FILE_t) NULL; \
				 return (code); \
			 } \
		 } \
	 } \
	\
	 /* If all the bytes in the buf is read and the fd stream reaches the end of file, then return the "code" */ \
	 if(((pRead)->index >= (pRead)->read_bytes) && (pRead)->eof) { \
		 return (code); \
	 } \
	\
	 (c) = (pRead)->buf[(pRead)->index++]; \
 }
/* end jerry_deng 2012.12.12 Belkin Bug ARCADYAN-2245 */


/* Common handlers. 
 * You can create a handler in vendor_dir,
 * based on common handler for specified vendor.
 */
static struct ssi_handler ssi_handlers[] = {

	/* webpages */
	{ "web_include_file", ssi_web_include_file },
	{ "no_cache", ssi_no_cache },
#if 0 //KILL_ME
    { "rtl", ssi_rtl},
#endif //KILL_ME
	{ "ejGet", ssi_Get},
	{ "onload", ssi_onload },

	/* nvram */
    { "nvram_get", ssi_nvram_get }, 
	{ "support_invmatch", ssi_support_invmatch},
	{ "support_match", ssi_support_match},
    { "nvram_else_match", ssi_nvram_else_match},
	{ "nvram_match", ssi_nvram_match },
	{ "nvram_n_else_match", ssi_nvram_n_else_match },
	{ "nvram_invmatch", ssi_nvram_invmatch },
	{ "nvram_selget", ssi_nvram_selget },
	{ "nvram_selmatch", ssi_nvram_selmatch },

	/* arcadyan abstract */
	{ "ABS_MAP", ssi_ABS_MAP },
	{ "ABS_ARR", ssi_ABS_ARR },
	{ "ABS_GET", ssi_ABS_GET },
	{ "ABS_DFT", ssi_ABS_DFT },

	/* httpd */
	{ "webs_get", ssi_webs_get },
	{ "get_http_method", ssi_get_http_method},
	{ "get_http_from", ssi_get_http_from },

	/* system */
	{ "get_firmware_short_version", ssi_get_firmware_short_version },
	{ "get_model_name", ssi_get_model_name },
	{ "get_firmware_title", ssi_get_firmware_title },
	{ "localtime", ssi_localtime },
	{ "date_timezone", ssi_date_timezone },
	{ "get_firmware_version", ssi_get_firmware_version },
	{ "compile_date", ssi_compile_date },
	{ "check_ipv6_support", ssi_check_ipv6_support},

    { "get_lang", ssi_get_lang },
	{ "prefix_ip_get", ssi_prefix_ip_get },
	{ "get_connected_device", ssi_get_connected_device },
	{ "get_pclist", ssi_get_pclist },
	{ "get_single_ip", ssi_get_single_ip },
	{ "get_dns_ip", ssi_get_dns_ip },
	/* ipv6 */
	{ "get_ipv6_info", ssi_get_ipv6_info },
	{ "get_ipv6_localaddr", ssi_get_ipv6_localaddr },
	{ "get_ipv6_prefixaddr", ssi_get_ipv6_prefixaddr },
	/* wan */
	{ "get_wan_dns", ssi_get_wan_dns },
	{ "get_wan_mac", ssi_get_wan_mac },
	{ "get_wanif_index", ssi_get_wanif_index },
	/* dhcp */
	{ "dump_clients", ssi_dump_clients },
	{ "dump_dhcp_leased", ssi_dump_dhcp_leased },
	/* SIP proxy */
	{ "get_SIPP_status", ssi_get_sipp_status },
	{ "get_SIPP_call_log", ssi_get_sipp_call_log },	
	{ "get_SIPP_call_cnt", ssi_get_sipp_call_cnt },	
	/*MTK*/
#if defined(VR7516RW22) || defined(WE410443) || (WE5202243_SA)
	{ "get_cur_channel", ssi_get_cur_channel },
#endif
#ifdef CONFIG_HTTPD_TOKEN_CHECK_SUPPORT	
	/* Httoken */
	{ "httoken_list_output", ssi_httoken_list_output },
	{ "httoken_output", ssi_httoken_output },
#endif
	{ NULL, NULL }
};

extern struct ssi_handler vendor_handlers[];


/*
  jerry_deng 2012.12.12 Belkin Bug ARCADYAN-2245
*/
/**
 *      file_contain_ssiï¼?jerry_deng 2012.12.12
 *      @r[in]ï¼šthe pointer of the requested struct "request_rec".
 *      return: 1 if the requested file contain ssi functio(<% xxxx %>), and 0 if not. 
 *      check the requested file name whether is the array ssi_files_list[] in ssi_files_list.h
 *      which is dynamically generated by create_ssi_file_list.sh
 */
static int file_contain_ssi(register struct request_rec *r)
{
	int www_dir_len; 
	char *requested_file_name;
	char *ssi_file_name;
	int i = 0;
	char url_full_path[FNAME_SIZE];

	memset(url_full_path, 0, sizeof(url_full_path));
	proc_dir(r, url_full_path, r->url); /* url_full_path="/www/xxx.htm" */
	www_dir_len = strlen(document_root); /* document_root = "/www" */
	requested_file_name = url_full_path + www_dir_len + 1;

	while(ssi_files_list[i])
	{
		ssi_file_name = ssi_files_list[i];
		if(strcmp(ssi_file_name, requested_file_name) == 0)
			return 1;
		i++;
	}

	return 0;
}
/* end jerry_deng 2012.12.12 Belkin Bug ARCADYAN-2245 */

/* --------------------------- Parser functions --------------------------- */
/*
  jerry_deng 2012.12.12 Belkin Bug ARCADYAN-2245
*/
static int find_string(register struct request_rec *r, char *str, int out, struct read_info *pRead) 
/* end jerry_deng 2012.12.12 Belkin Bug ARCADYAN-2245 */
{
	int x, l=strlen(str), p;
	char c = 0;

	p = 0;

	while(1) {
/*
  jerry_deng 2012.12.12 Belkin Bug ARCADYAN-2245
*/
	 /* GET_CHAR will RETURN "1" if the stream reach the end of file. Please see the MACRO of GET_CHAR */
		GET_CHAR(r->fd, c, 1, pRead);
/* end jerry_deng 2012.12.12 Belkin Bug ARCADYAN-2245 */

		if(c == str[p]) {
			if((++p) == l)
				return 0;
		}
		else {
			if (out) {
				if (p) {
					for(x=0;x<p;x++) {
						so_putc(str[x], r);
						++r->bytes_sent;
					}
				}

				so_putc(c, r);
				++r->bytes_sent;
			}
			p=0;
		}
	}
}

/*
  jerry_deng 2012.12.12 Belkin Bug ARCADYAN-2245
*/
static int get_directive(register struct request_rec *r, char *d, struct read_info *pRead) 
/* end jerry_deng 2012.12.12 Belkin Bug ARCADYAN-2245 */
{
	char c;
	int i = 0, buf_end = MAX_DIRECTIVE_LEN - 1;
	int match_one = 0;

	/* skip initial whitespace */
	while(1) {
/*
  jerry_deng 2012.12.12 Belkin Bug ARCADYAN-2245
*/
	 /* GET_CHAR will RETURN "-1" if the stream reach the end of file. Please see the MACRO of GET_CHAR */
		GET_CHAR(r->fd, c, -1, pRead);
/* end jerry_deng 2012.12.12 Belkin Bug ARCADYAN-2245 */
		if(!isspace(c))
			break;
	}

	/* now get directive */
	while(1) {
		//d[i++] = tolower(c);
		d[i++] = c;
		if (i == buf_end) {
			d[i] = '\0';
			return -2;
		}
/*
  jerry_deng 2012.12.12 Belkin Bug ARCADYAN-2245
*/
	 /* GET_CHAR will RETURN "-1" if the stream reach the end of file. Please see the MACRO of GET_CHAR */
		GET_CHAR(r->fd, c, -1, pRead);
/* end jerry_deng 2012.12.12 Belkin Bug ARCADYAN-2245 */

		if (c == '%')
			match_one = 1;
		else if (match_one) {
			if (c == '>') {
				i--;	
				d[i] = '\0';
				return 1;
			}
			else
				match_one = 0;
		}

		//if(isspace(c) || (c == '(') || c == '-')
		if (c == ';')
			break;
	}

	d[i] = '\0';
	
	return 0;
}

int fmt_args(int argc, char **argv, char *fmt, ...)
{
    va_list ap;
    int arg;
    char *c;

    if (!argv)
        return 0;

    va_start(ap, fmt);
    for (arg = 0, c = fmt; c && *c && arg < argc;) {
        if (*c++ != '%')
            continue;
        switch (*c) {
        case 'd':
            *(va_arg(ap, int *)) = atoi(argv[arg]);
            break;
        case 's':
            *(va_arg(ap, char **)) = argv[arg];
            break;
        }
        arg++;
    }
    va_end(ap);

    return arg;
}

/* Look for unquoted character within a string */
static char *unqstrstr(char *haystack, char *needle)
{
    char *cur = NULL;
    int q = 0;
	int needlelen=	strlen(needle);

    for (cur = haystack, q = 0;
         *cur!=0 && !(!q && !strncmp(needle, cur, needlelen));
         cur++) {
        if (*cur == '"')
            q =!q;
    }
		 
    return cur;
}

static char *get_args(char *args, char **next)
{
    char *arg, *end;

    /* Parse out arg, ... */
    if (!(end = unqstrstr(args, ","))) {
        end = args + strlen(args);
        *next = NULL;
    } else
        *next = end + 1;

    /* Skip whitespace and quotation marks on either end of arg */
    for (arg = args; isspace((int)*arg) || *arg == '"'; arg++);
    for (*end-- = '\0'; isspace((int)*end) || *end == '"'; end--)
        *end = '\0';

    return arg;
}

static void	handle_directive(char *directive, register struct request_rec *r) 
{
    char *args, *end, *next;
    int argc;
    char * argv[16];
    struct ssi_handler *h;
	int found = 0;

    /* Parse out ( args ) */
    if (!(args = unqstrstr(directive, "(")))
        return;
    if (!(end = unqstrstr(directive, ")")))
        return;
    *args++ = *end = '\0';

    /* Set up argv list */
    for (argc = 0; argc < 16 && args; argc++, args = next) {
        if (!(argv[argc] = get_args(args, &next)))
            break;
    }

    /* Call handler */
    for (h = &ssi_handlers[0]; h->pattern; h++) {
        if (strcasecmp(h->pattern, directive) == 0) {
			found = 1;
            h->output(r, argc, argv);
		}
    }

	if (found)
		return ;

	/* vendor handlers */
	for (h = &vendor_handlers[0]; h->pattern; h++) {
        if (strcasecmp(h->pattern, directive) == 0) {
            h->output(r, argc, argv);
		}
    }

	return ;
}


/* --------------------------- Action handlers ---------------------------- */

int send_included_file(register struct request_rec *r, char *file)
{
	FILE_INFO_t finfo;
	//long clen = 0;
	//FILE_t fd = r->fd;
	char *tmp_url;
	unsigned long nbytes;
	struct fd_entry *pOld_fd_entry;

	char to_send[MAX_STRING_LEN];
	char cwd[FNAME_SIZE];

	//getparents(file); /* get rid of any nasties */
	strcpy(cwd, FSgetcwd(r));
	make_full_path(cwd, file, to_send);

	if (FSstat(r, to_send, &finfo) == -1)
	{
		FSchdir(r, cwd);
		return -1;
	}

	ht_dbg("[%d]: PWD=%s, file=%s, to_send=%s\n", __LINE__, cwd, file, to_send);

#if 0
	r->fd_backup = r->fd;
#else
	/*
	 * httpd v1.1 new feature. Support multiple-layer include command.
	 */
	pOld_fd_entry = r->fd_backup;
	r->fd_backup = (struct fd_entry *)malloc(sizeof(struct fd_entry));
	r->fd_backup->fd = r->fd;
	r->fd_backup->next_fd = pOld_fd_entry;
#endif

	//evaluate_access(file, finfo.is_dir, r);

	/* patched for processing included SSI docs */
	// Save the original status
	tmp_url = strdup(r->url);
	strcpy(r->url, file);
	nbytes = r->bytes_sent;

	ht_dbg("[%d]: url=%s\n", __LINE__, r->url);
	send_file(r);

	//r->content_length = clen;
	/*
	 * httpd v1.1 new feature, Support multiple-layer include command.
	 * restore to previous layer's status.
	 */
	r->fd = r->fd_backup->fd;
	pOld_fd_entry = r->fd_backup->next_fd;
	free(r->fd_backup);
	r->fd_backup = pOld_fd_entry;

	/* patched for  processing included SSI docs */
	strcpy(r->url, tmp_url);
	ht_dbg("[%d]: url=%s\n", __LINE__, r->url);
	free(tmp_url);
	r->bytes_sent += nbytes;

	FSchdir(r, cwd);
	ht_dbg("[%d]: PWD=%s, file=%s, to_send=%s\n", __LINE__, cwd, file, to_send);

	return 0;
}


/*
  jerry_deng 2012.12.12 Belkin Bug ARCADYAN-2245
*/
/**
 *      direct_send_contentï¼?jerry_deng 2012.12.12
 *      @r[in/out]ï¼šthe pointer to the struct "request_rec".
 *      @pRead[in/out]ï¼šthe pointer to the struct "read_info".
 *      return: -1 after completing reading the whole files.
 *      Read the file which does NOT contain ssi function and then print out.
 */
int direct_send_content(register struct request_rec *r, struct read_info *pRead)
{
	char c;

	/* Note: This while loop is NOT a dead loop.
	 * It will RETURN once the stream reach the end of file. Please see the MACRO of GET_CHAR 
	 */ 
	while(1)
	{
		GET_CHAR(r->fd, c, -1, pRead);
		so_putc(c, r);
		++r->bytes_sent;
	}
}
/* end jerry_deng 2012.12.12 Belkin Bug ARCADYAN-2245 */

/* This is a stub which parses a file descriptor. */

void send_parsed_content(register struct request_rec *r)
{
	char directive[MAX_DIRECTIVE_LEN]; 
	int ret = 0;

/*
  jerry_deng 2012.12.12 Belkin Bug ARCADYAN-2245
*/
	struct read_info read_info;

	/* init read_info */
	memset(&read_info, 0, sizeof(read_info));
/* end jerry_deng 2012.12.12 Belkin Bug ARCADYAN-2245 */
 FSchdir_file(r, r->url);
/*
  jerry_deng 2012.12.12 Belkin Bug ARCADYAN-2245
*/
	if(!file_contain_ssi(r))
	{
		direct_send_content(r, &read_info);
		return;
	}
/* end jerry_deng 2012.12.12 Belkin Bug ARCADYAN-2245 */


	while (1) {
/*
  jerry_deng 2012.12.12 Belkin Bug ARCADYAN-2245
*/
		if(find_string(r, STARTING_SEQUENCE, 1, &read_info)) {
/* end jerry_deng 2012.12.12 Belkin Bug ARCADYAN-2245 */
			return;
		}

/*
  jerry_deng 2012.12.12 Belkin Bug ARCADYAN-2245
*/
		switch(get_directive(r, directive, &read_info))
/* end jerry_deng 2012.12.12 Belkin Bug ARCADYAN-2245 */
		{
			case -1:	/* file I/O error */
				//ht_dbg("file I/O error...\n");
				return;
			case -2:
				//ht_dbg("Directive is too long, skip ending ...\n");
/*
  jerry_deng 2012.12.12 Belkin Bug ARCADYAN-2245
*/
				ret = find_string(r, ENDING_SEQUENCE, 0, &read_info);
/* end jerry_deng 2012.12.12 Belkin Bug ARCADYAN-2245 */
				break;
			case 0:		/* success */
				//ht_dbg("Success, directive=%s\n", directive);
				handle_directive(directive, r);
/*
  jerry_deng 2012.12.12 Belkin Bug ARCADYAN-2245
*/
				ret = find_string(r, ENDING_SEQUENCE, 0, &read_info);
/* end jerry_deng 2012.12.12 Belkin Bug ARCADYAN-2245 */
				break;
			case 1:
				//ht_dbg("Success, directive=%s, skipped ENDING\n", directive);
				handle_directive(directive, r);
				break;
			default:
				break;
		}

		if (ret) {
			log_error("send_parsed_content: reach EOF");
			return;
		}
	}

	return ;
}

/* Called by send_file */
void send_parsed_file(struct request_rec *r)
{

	r->fd = FSopen(r, r->url);

	if (r->fd == NULL) {
		unmunge_name(r->url);
		return;
	}
	
	if (strcmp(r->content_type, INCLUDES_MAGIC_TYPE) == 0)
	{
//FIXME: vendor specified macro should be removed.
#ifdef VENDOR_BELKIN_SUPPORT
#ifdef NN3_0
		/* For FW ver API function:
		 * Follow the N750 19J code,  the "content_type" field in the http response header of /www/BelkinAPI/APIVersion 
		 * is "text/plain", instead of "text/html";
		 */
		if(strcmp(r->url, "/www/BelkinAPI/APIVersion") == 0)
			r->content_type = "text/plain";
		else
#endif
#endif
 	r->content_type = "text/html";
	}

	if (!r->assbackwards) {
		set_expires(r, &r->finfo.ftime);

		send_http_header(r);
	}

	if (r->header_only)
		return;

	r->assbackwards = 1;	/* make sure no headers get inserted anymore */
	tmr_set(r->cookie, ht_timeout+60, timed_out_handler); // change the timeout to a larger one in case
	                                                      // the connection is very busy
	send_parsed_content(r);
	//printf("bytes_sent=[%ld]\n", r->bytes_sent);

}

