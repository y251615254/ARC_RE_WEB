/**
 * @defgroup SSIlib Httpd CGI tags handl API
 * @ingroup HTTPD
 * @brief A CGI tags handl library support by Httpd
 *
 */
#include <time.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdio.h>

#include "httpd.h"
#include "ssi_handler.h"
#include "msg_def.h"
#include "utils.h" //cprintf
#include "util.h"
#include "httoken.h"
#include <arc_cfg_api.h>

extern int fmt_args(int argc, char **argv, char *fmt, ...);
extern int send_included_file(register struct request_rec *r, char *file);
extern int find_pid_by_ps(char* name);
extern int find_cfgID_by_cname(char *cname);

extern char *get_cgi(char *field);
extern char *get_cgi_df(char *field, char *sdf);

#define CFG_VAL_NORMAL     0x00
#define CFG_VAL_HEX        0x01
#define CFG_VAL_ENCRYPTED  0x02
// here define the MAX value size retriev from arc_cfg_get()
//
#define MAX_CFG_VAL_SIZE 1024

//#define CFG_CRLF "\n"
#define CFG_CRLF ""


/**
 * @ingroup SSIlib
 * @brief ABS_xx data formating function
 *
 * @param *r HTTPD handler request variable
 * @param *in  the input string bufer want to decode re-format.
 * @param *out the output result.
 * @param enc the request format type (a bit mask)
 *            - bit 0 : means a standard plain text format or  a escape HTML entries format by %xx
 *            - bit 1 : AES256-CBC encode value with escape HTML entries format by %xx.
 *            @note
 *               if ienc is NULL, see as no any bit set, means 0 value
 *            @endnote
 *
 * @retval char* the content of result
 *
 * @warning:
 *     - caller MUST allocate enought input buffer size, and the size of input bufer MUST a multiple of 16.
 *
 */
static char *get_ABS_VAL(struct request_rec *r,char *in, char *out, int enc)
{
	char prefix[2]={"%"};
	unsigned char *c=(unsigned char *)in;
	int sz=0;

	// token is save under r->token
	//cprintf("token=%lu from [%s] , Referer: [%s]\n", r->token, r->url, r->referer);

	out[0]='\0';
	if(!c) return out;

	sz=strlen((char *)c);
	// NOTE: 2014/5/6
	//       if no turn on CONFIG_HTTPD_TOKEN_CHECK_SUPPORT, here r->token will be zero
	//
	if(sz &&  ((enc & CFG_VAL_ENCRYPTED) ==CFG_VAL_ENCRYPTED) )
	{
		sz=Arc_encode(r,c);
		prefix[0]='\0'; // for AES encryption way, we no nedd ass "%" to reduce output sizes
	}
	if( sz > 0)
	{
		while (sz--)
		{
			//sprintf(out, "%s%02X", out, (unsigned char)(*c++));
			if (!enc && isprint((int) *c) && *c != '"' && *c != '&' && *c != '<' && *c != '>' && *c != '\'' && *c != '\\'&& *c != '%'){
				sprintf(out, "%s%c", out, *c);
			} else {
				sprintf(out, "%s%s%02X", out, prefix, (unsigned char)(*c));
			}
			c++;
		}
	}

	return out;
}
/* --------------------------- Handlers --------------------------- */

/**
 * @ingroup SSIlib
 * @brief ABS_MAP CGI tags handler library to support single item retrieve by addCfg() syntax
 *
 * @param *r HTTPD handler request variable
 * @param *argc  the input argument size.
 * @param **argv the input argument array variable.
 *
 * @retval 0  a empty content
 * @retval !0 the size of output content
 *
 * @b Format
 *    CGI tags format:
 *    @code
 *       <%ABS_MAP(<jn>,<cn><+idx_x><+idx_y>,<enc>)%>
 *    @endcode
 *    where
 *       - [jn]        : a UI variable name
 *       - [cn]        : a CFG cid value(Integer mode)
 *       - [+idx_x]    : a CFG items x index entries(0~max of x entries)
 *                       - if empty means first x index (e.g.: 0)
 *       - [+idx_y]    : a CFG items y index entries(0~max of y entries)
 *                       - if empty means first entry of y index (e.g.: 0) for x index.
 *       - [enc]       : a special security UI data formating control
 *                  + empty: means the conent is,
 *                       a native content(non string mode) or
 *                       a escaped fomrat for string mode
 *                  + 1: means the conent is a escape HTML entries format by %xx
 *                  + 2: means the conent is encrypted with escape HTML entries format by %xx
 *                      @code
 *                         UI need support decode and encode job
 *                      @endcode
 *
 * @b Output format:
 *    @code
 *       addCfg(jn, cn<x><y>,<val>, <enc>)
 *    @endcode
 *    where
 *       - [jn]        : a UI variable name
 *       - [cn]        : a CFG cid integer value (Integer mode)
 *       - [x]         : 3 digits of the value of x entries (0 ~ max of x entries)
 *                        - 0: if no x entries
 *                        - 1~max x entries : if support x index.
 *       - [y]         : 3 digits of the value of y entries (0 ~ max of y entries)
 *                        - 0: if no y entries
 *                        - 1~max y entries : if support y index.
 *       - [value]     : the value of CFG item
 *       - [enc]       : a special security UI data control
 *
 *
 * @b Example:
 *   UI CGI tags,
 *       @code
 *          <% ABS_MAP("lan_ip","ARC_LAN_x_IP4_Addr"+0) %>
 *      @endcode
 *   UI run time code,
 *       @code
 *          <% ABS_MAP("lan_ip","131076000000") %>
 *       @endcode
 *   httpd output
 *       @code
 *          addCfg("lan_ip","131076001000","192.168.2.1");
 *       @endcode
 *
 *
*/

int ssi_ABS_MAP(struct request_rec *r, int argc, char **argv)
{
	char buf[MAX_CFG_VAL_SIZE] = {0},tmp[MAX_CFG_VAL_SIZE*3] = {0}, enc_s[5]={0}; //",x" two character is enough
	char *jname, *cname, *enc=NULL, *c;
	int tid, cfgID, idx_x = -1, idx_y = -1; // default x, y is -1

	int mode=0; //default 0, but any exist enc, check it

	if((tid = get_tid()) == MID_FAIL)
		return 0;

	if(fmt_args(argc, argv, "%s %s %s", &jname, &cname, &enc) < 2) 
	{
		so_printf(r, "%s", "Insufficient args\n");
		return -1;
	}

	/* Todo: encrytion supports */

	parse_cname_idx(cname, &idx_x, &idx_y);

	cfgID = find_cfgID_by_cname(cname);
	
	if (cfgID <= 0) 
	{
		printf("[%s] :cannot get cfgID [%s]!\n", __FUNCTION__,cname );
		return -1;
	}
	
	c = arc_cfg_get_idx2(tid, cfgID, (idx_x==-1)?INT_MIN:idx_x ,(idx_y==-1)?INT_MIN:idx_y, buf, sizeof(buf), NULL);

	mode=(enc)? atoi(enc):0;

#ifdef CONFIG_HTTPD_SUPPORT_CFG_ENCRYPTED
	mode |=CFG_VAL_ENCRYPTED; // 0x02 maked as encrypted
#endif //CONFIG_HTTPD_SUPPORT_CFG_ENCRYPTED
	if(mode)
		sprintf(enc_s,",%d",mode);

	get_ABS_VAL(r, c, (char *)tmp, mode);
	//printf("ABS MAP[%s]=[%s]\n",jname, tmp);
	// NOTE:
	//    idx_x  idx_y       format
	// ========= =========== ============
	// -1        -1          000000
	// x         -1          xxx000     xxx=x+1
	// x          y          xxxyyy     xxx=x+1, yyy=y+1
//	if (idx_y == -1) idx_x=0;
//	if (idx_x == -1) idx_x=0;

    r->bytes_sent += so_printf(r, "addCfg(\"%s\", \"%d%03d%03d\", \"%s\"%s);" CFG_CRLF, jname, cfgID, (idx_x+1), (idx_y+1) ,tmp, enc_s);

    return r->bytes_sent;
}

/**
 * @ingroup SSIlib
 * @brief ABS_ARR CGI tags handler library to support multi-entries retrieve by addCfg() syntax
 *
 * @param *r HTTPD handler request variable
 * @param *argc  the input argument size.
 * @param **argv the input argument array variable.
 *
 * @retval 0  a empty content
 * @retval !0 the size of output content
 *
 * @b Format
 *    CGI tags format:
 *    @code
 *       <%ABS_ARR(<jn>,<cn><+idx_x><+idx_y>,<enc>)%>
 *    @endcode
 *    where
 *       - [jn]        : a UI variable name
 *       - [cn]        : a CFG cid value(Integer mode)
 *       - [+idx_x]    : a CFG items x index entries(0~max of x entries)
 *                       - if empty means may a single entry if no x index or
 *                         multi entries if CFG has y index by x index.
 *       - [+idx_y]    : a CFG items y index entries(0~max of y entries)
 *                       - if empty means may a single entry if no x index or
 *                         multi entris if CFG has y index by x index.
 *       - [enc]       : a special security UI data formating control
 *                  + empty: means the conent is,
 *                       a native content(non string mode) or
 *                       a escaped fomrat for string mode
 *                  + 1: means the conent is a escape HTML entries format by %xx
 *                  + 2: means the conent is encrypted with escape HTML entries format by %xx
 *                      @code
 *                         UI need support decode and encode job
 *                      @endcode
 *
 * @b Output format:
 *    @code
 *       addCfg(jn, cn<x><y>,<val>, <enc>)
 *    @endcode
 *    where
 *       - [jn]        : a UI variable name
 *       - [cn]        : a CFG cid integer value (Integer mode)
 *       - [x]         : 3 digits of the value of x entries (0 ~ max of x entries)
 *                        - 0: if no x entries
 *                        - 1~max x entries : if support x index.
 *       - [y]         : 3 digits of the value of y entries (0 ~ max of y entries)
 *                        - 0: if no y entries
 *                        - 1~max y entries : if support y index.
 *       - [value]     : the value of CFG item
 *       - [enc]       : a special security UI data control
 *
 *
 * @b Example:
 *   UI CGI tags,
 *       @code
 *         <% ABS_ARR("url","ARC_FIREWALL_URL_RULE_x_Keyword") %>
 *      @endcode
 *   UI run time code,
 *       @code
 *          <% ABS_ARR("url","589842000000") %>
 *       @endcode
 *   httpd output
 *       @code
 *          addCfg("url0","589842001000","test1");
 *          addCfg("url1","589842002000","test2");
 *          addCfg("url1","589842002000","test2");
 *       @endcode
 *
 *
*/
int ssi_ABS_ARR(struct request_rec *r, int argc, char **argv)
{
	char buf[MAX_CFG_VAL_SIZE] = {0}, tmp[MAX_CFG_VAL_SIZE*3] = {0}, enc_s[5]={0}; //",x" two character is enough
	//char *jname, *cname, *enc, *c, *pEnd;
	char *jname, *cname, *enc=NULL, *c;
	int tid = 0, cfgID = 0, i = 0, j = 0, idx_x = 0, idx_y = 0;

	int mode=0; //default 0, but any exist enc, check it

	if((tid = get_tid()) == MID_FAIL)
		return 0;

	if(fmt_args(argc, argv, "%s %s %s", &jname, &cname, &enc) < 2) 
	{
		so_printf(r, "%s", "Insufficient args\n");
		return -1;
	}
	
	mode=(enc)? atoi(enc):0;

#ifdef CONFIG_HTTPD_SUPPORT_CFG_ENCRYPTED
	mode |=CFG_VAL_ENCRYPTED; // 0x02 maked as encrypted
#endif //CONFIG_HTTPD_SUPPORT_CFG_ENCRYPTED
	if(mode)
		sprintf(enc_s,",%d",mode);

	/* Todo: encrytion supports */
	//parse_cname_idx(&cname, &idx_x, &idx_y);
	parse_cname_idx(cname, &idx_x, &idx_y);

	cfgID = find_cfgID_by_cname(cname);
	
	if (cfgID <= 0)
	{
		printf("[%s] :cannot get cfgID [%s]!\n", __FUNCTION__,cname );
		return -1;
	}
	/* we'll get the maximum possible entries if not set from GUI*/
	if (idx_x == 0)//if (len1 == NULL)
	{
//		arc_cfg_get_idx_offset(cfgID, &idx_x, &idx_y);
		idx_x = arc_cfg_get_idx1_limit(cfgID);
		idx_y = arc_cfg_get_idx2_limit(cfgID);
		
		//cprintf("[%s] :get the maximum possible entries [%d:%d]\n", __FUNCTION__,idx_x, idx_y );
	}
		
	if (idx_y == 0) /* parameter with one index */
	{
		for (i = 0; i < idx_x; i++)
		{
			c = arc_cfg_get_idx1(tid, cfgID,  i, buf, sizeof(buf), NULL);
			//cprintf("jname[%s]:[%d]:[%s]\n",jname, i,c);
			get_ABS_VAL(r, c, (char *)tmp, mode);

			r->bytes_sent += so_printf(r, "addCfg(\"%s%d\", \"%d%03d000\", \"%s\"%s);\n" CFG_CRLF, jname, i, cfgID, i+1, tmp, enc_s);
			//tmp[0] = '\0'; /* reset tmp[]*/
		}
	}
	else /* parameter with two indexes */
	{
		//cprintf("[%s] : [%d:%d]\n", jname,idx_x, idx_y );
		for (i = 0; i < idx_x; i++)
		{		
			for (j = 0; j < idx_y; j++)
			{
				c = arc_cfg_get_idx2(tid, cfgID,  i, j, buf, sizeof(buf), NULL);
				//cprintf("jname[%s]:[%d-%d]:[%s]\n",jname, i,j,c);
				get_ABS_VAL(r, c, (char *)tmp, mode);
				r->bytes_sent += so_printf(r, "addCfg(\"%s_%d_%d\", \"%d%03d%03d\", \"%s\"%s);\n" CFG_CRLF, jname, i, j, cfgID, i+1, j+1, tmp, enc_s);
				//tmp[0] = '\0'; /* reset tmp[]*/
			}
		}
	}
	
    return r->bytes_sent;
}


/* Format : ABS_GET(cn+idx_x+idx_y,enc) , [minumum value for idx_x/y = 0]
 * Purpose: do safe nvram get in .js
*/

 /**
 * @ingroup SSIlib
 * @brief ABS_GET CGI tags handler library to support single item value retrieve
 *
 * @param *r HTTPD handler request variable
 * @param *argc  the input argument size.
 * @param **argv the input argument array variable.
 *
 * @retval 0  a empty content
 * @retval !0 the size of output content
 *
 * @b Format
 *    CGI tags format:
 *    @code
 *       <%ABS_GET(<cn><+idx_x><+idx_y>,<enc>)%>
 *    @endcode
 *    where
 *       - [cn]        : a CFG cid value(Integer mode)
 *       - [+idx_x]    : a CFG items x index entries(0~max of x entries)
 *                       - if empty means first x index (e.g.: 0)
 *       - [+idx_y]    : a CFG items y index entries(0~max of y entries)
 *                       - if empty means first entry of y index (e.g.: 0) for x index.
 *       - [enc]       : a special security UI data formating control
 *                  + empty: means the conent is,
 *                       a native content(non string mode) or
 *                       a escaped fomrat for string mode
 *                  + 1: means the conent is a escape HTML entries format by %xx
 *                  + 2: means the conent is encrypted with escape HTML entries format by %xx
 *                      @code
 *                         UI need support decode and encode job
 *                      @endcode
 *
 * @b Output format:
 *    @code
 *       <val>
 *    @endcode
 *    where
 *       - [value]     : the value of CFG item
 *
 *
 * @b Example:
 *   UI CGI tags,
 *       @code
 *          <% ABS_GET("ARC_LAN_x_IP4_Addr") %>
 *      @endcode
 *   UI run time code,
 *       @code
 *         <% ABS_GET("131076000000") %>
 *       @endcode
 *   httpd output
 *       @code
 *          192.168.2.1
 *       @endcode
 *
 *
 */
int ssi_ABS_GET(struct request_rec *r, int argc, char **argv)
{
	char buf[MAX_CFG_VAL_SIZE] = {0},tmp[MAX_CFG_VAL_SIZE*3] = {0}; //, enc_s[5]={0}; //",x" two character is enough
	char *cname, *c, *enc=NULL;
	int tid = 0, cfgID = 0, idx_x = -1, idx_y = -1;

	int mode=0; //default 0, but any exist enc, check it

	if((tid = get_tid()) == MID_FAIL)
		return 0;

	if (fmt_args(argc, argv, "%s %s", &cname, &enc) < 1) {
		so_printf(r, "%s", "Insufficient args\n");
		return -1;
	}

	mode=(enc)? atoi(enc):0;

// FIXME: TODO
//#ifdef CONFIG_HTTPD_SUPPORT_CFG_ENCRYPTED
//	mode |=CFG_VAL_ENCRYPTED; // 0x02 maked as encrypted
//#endif //CONFIG_HTTPD_SUPPORT_CFG_ENCRYPTED

	parse_cname_idx(cname, &idx_x, &idx_y);

	cfgID = find_cfgID_by_cname(cname);
	
	if (cfgID <= 0) 
	{
		printf("[%s] :cannot get cfgID [%s]!\n", __FUNCTION__,cname );
		return -1;
	}
	
	c = arc_cfg_get_idx2(tid, cfgID, (idx_x==-1)?INT_MIN:idx_x ,(idx_y==-1)?INT_MIN:idx_y, buf, sizeof(buf), NULL);

	/* Todo: encrytion supports */
	get_ABS_VAL(r, c, (char *)tmp, mode);
	r->bytes_sent += so_printf(r, "%s", tmp);

	return r->bytes_sent;
}


 /**
 * @ingroup SSIlib
 * @brief ABS_DFT CGI tags handler library to support single item value retrieve special from default CFG
 *
 * @param *r HTTPD handler request variable
 * @param *argc  the input argument size.
 * @param **argv the input argument array variable.
 *
 * @retval 0  a empty content
 * @retval !0 the size of output content
 *
 * @b Format
 *    CGI tags format:
 *    @code
 *       <%ABS_DFT(<cn><+idx_x><+idx_y>,<enc>)%>
 *    @endcode
 *    where
 *       - [cn]        : a CFG cid value(Integer mode)
 *       - [+idx_x]    : a CFG items x index entries(0~max of x entries)
 *                       - if empty means first x index (e.g.: 0)
 *       - [+idx_y]    : a CFG items y index entries(0~max of y entries)
 *                       - if empty means first entry of y index (e.g.: 0) for x index.
 *       - [enc]       : a special security UI data formating control
 *                  + empty: means the conent is,
 *                       a native content(non string mode) or
 *                       a escaped fomrat for string mode
 *                  + 1: means the conent is a escape HTML entries format by %xx
 *                  + 2: means the conent is encrypted with escape HTML entries format by %xx
 *                      @code
 *                         UI need support decode and encode job
 *                      @endcode
 *
 * @b Output format:
 *    @code
 *       <val>
 *    @endcode
 *    where
 *       - [value]     : the value of CFG item
 *
 *
 * @b Example:
 *   UI CGI tags,
 *       @code
 *          <% ABS_DFT("ARC_LAN_x_IP4_Addr") %>
 *      @endcode
 *   UI run time code,
 *       @code
 *         <% ABS_DFT("131076000000") %>
 *       @endcode
 *   httpd output
 *       @code
 *          192.168.2.1
 *       @endcode
 *
 * @warning:
 *       UI MUST known HOW to decode if set enc flag, it cause UI present incorrect
 *
*/
int ssi_ABS_DFT(struct request_rec *r, int argc, char **argv)
{
	char buf[MAX_CFG_VAL_SIZE] = {0},tmp[MAX_CFG_VAL_SIZE *3] = {0};
	char *cname, *c, *enc=NULL;
	int tid = 0, cfgID = 0, idx_x = -1, idx_y = -1;

	int mode=0; //default 0, but any exist enc, check it

	if((tid = get_tid()) == MID_FAIL)
		return 0;

	if (fmt_args(argc, argv, "%s %s", &cname, &enc) < 1) {
		so_printf(r, "%s", "Insufficient args\n");
		return -1;
	}

	mode=(enc)? atoi(enc):0;

// FIXME: TODO
//#ifdef CONFIG_HTTPD_SUPPORT_CFG_ENCRYPTED
//	mode |=CFG_VAL_ENCRYPTED; // 0x02 maked as encrypted
//#endif //CONFIG_HTTPD_SUPPORT_CFG_ENCRYPTED

	parse_cname_idx(cname, &idx_x, &idx_y);

	cfgID = find_cfgID_by_cname(cname);
	
	if (cfgID <= 0) 
	{
		printf("[%s] :cannot get cfgID [%s]!\n", __FUNCTION__,cname );
		return -1;
	}
	
	c = arc_cfg_get_dft_idx2(tid, cfgID, (idx_x==-1)?INT_MIN:idx_x ,(idx_y==-1)?INT_MIN:idx_y, buf, sizeof(buf));

	/* Todo: encrytion supports */
	get_ABS_VAL(r, c, (char *)tmp, mode);
	r->bytes_sent += so_printf(r, "%s", tmp);

	return r->bytes_sent;
}


int ssi_nvram_get(struct request_rec *r, int argc, char **argv)
{
	char buf[MAX_CFG_VAL_SIZE] = {0},tmp[MAX_CFG_VAL_SIZE * 3] = {0};
	char *name, *c;
	int tid;

	if((tid = get_tid()) == MID_FAIL)
		return 0;


    if (fmt_args(argc, argv, "%s", &name) < 1) {
        so_printf(r, "%s", "Insufficient args\n");
        return -1;
    }

	c = mapi_ccfg_get_str(tid, name, buf, sizeof(buf));

	get_ABS_VAL(r, c, (char *)tmp, 0);
	r->bytes_sent += so_printf(r, "%s", tmp);

    return r->bytes_sent;
}
int ssi_nvram_js_get(struct request_rec *r, int argc, char **argv)
{
	char buf[MAX_CFG_VAL_SIZE*3] = {0};
	char *name, *c;
	int tid;

	if((tid = get_tid()) == MID_FAIL)
		return 0;


    if (fmt_args(argc, argv, "%s", &name) < 1) {
        so_printf(r, "%s", "Insufficient args\n");
        return -1;
    }

	c = mapi_ccfg_get_str(tid, name, buf, sizeof(buf));

	for (;*c; c++) 
	{
		if (isprint((int) *c) && *c != '\'' && *c != '\\')
			r->bytes_sent += so_printf(r, "%c", *c);
		else
			r->bytes_sent += so_printf(r, "\\%c", *c);
	}

    return r->bytes_sent;
}

int ssi_nvram_js_get_with_double_quotes(struct request_rec *r, int argc, char **argv)
{
	char buf[MAX_CFG_VAL_SIZE*3] = {0};
	char *name, *c;
	int tid;

	if((tid = get_tid()) == MID_FAIL)
		return 0;


    if (fmt_args(argc, argv, "%s", &name) < 1) {
        so_printf(r, "%s", "Insufficient args\n");
        return -1;
    }

	c = mapi_ccfg_get_str(tid, name, buf, sizeof(buf));

	for (;*c; c++) 
	{
		if (isprint((int) *c) && *c != '\'' && *c != '\\' && *c != '"')//single quotes, backslash, double quotes
			r->bytes_sent += so_printf(r, "%c", *c);
		else
			r->bytes_sent += so_printf(r, "\\%c", *c);
	}
	
	return r->bytes_sent;
}

int ssi_Get(struct request_rec *r, int argc, char **argv) 
{
	char buf[MAX_CFG_VAL_SIZE] = {0},tmp[MAX_CFG_VAL_SIZE*3] = {0};
	char *name, *c;
	int tid;

	if((tid = get_tid()) == MID_FAIL)
		return 0;


    if (fmt_args(argc, argv, "%s", &name) < 1) {
        so_printf(r, "%s", "Insufficient args\n");
        return -1;
    }

	c = mapi_ccfg_get_str(tid, name, buf, sizeof(buf));
/*
	for (;*c; c++) 
	{
		if (isprint((int) *c) && *c != '"' && *c != '&' && *c != '<' && *c != '>' && *c != '\'' && *c != '\\')
			r->bytes_sent += so_printf(r, "%c", *c);
		else
//#ifdef CONFIG_HTTPD_HOT_FIX_SW1  KILL_ME
			r->bytes_sent += so_printf(r, "%%%02X", (unsigned char)(*c));
//#else
//			r->bytes_sent += so_printf(r, "&#%d;", *c);
//#endif
	}
*/
	get_ABS_VAL(r, c,(char *)tmp, 0);
	r->bytes_sent += so_printf(r, "%s", tmp);

    return r->bytes_sent;


}

int ssi_Get_SSID(struct request_rec *r, int argc, char **argv) 
{
	char buf[MAX_CFG_VAL_SIZE] = {0};
	char *name, *c;
	int tid;

	if((tid = get_tid()) == MID_FAIL)
		return 0;


    if (fmt_args(argc, argv, "%s", &name) < 1) {
        so_printf(r, "%s", "Insufficient args\n");
        return -1;
    }

	c = mapi_ccfg_get_str(tid, name, buf, sizeof(buf));

	for (;*c; c++) 
	{
		if (isprint((int) *c) && *c != '\'' && *c != '\\')
			r->bytes_sent += so_printf(r, "%c", *c);
		else
			r->bytes_sent += so_printf(r, "\\%c", *c);
	}

    return r->bytes_sent;


}


void do_send_file(char *name, struct request_rec *r)
{
	
	char cwd[FNAME_SIZE] = {0};

	//ht_dbg("name=[%s]\n", name);

	strcpy(cwd, FSgetcwd(r));
	send_included_file(r, name);
	FSchdir(r, cwd);
}

int ssi_web_include_file(struct request_rec *r, int argc, char **argv) 
{
	char *name ;

	if(fmt_args(argc, argv, "%s", &name)<1)
	{
		so_printf(r, "%s", "Insufficient args\n");
		return -1;
	}
	
	//ht_dbg("name=[%s]\n", name);

	do_send_file(name, r);

	return 0;

}
	
int ssi_no_cache(struct request_rec *r, int argc, char **argv) 
{
//	char *name=NULL;

	so_printf(r, "%s", "<meta http-equiv=\"expires\" content=\"0\">\n");
	so_printf(r, "%s", "<meta http-equiv=\"cache-control\" content=\"no-cache\">\n");
	so_printf(r, "%s", "<meta http-equiv=\"pragma\" content=\"no-cache\">\n");

	return 0;
}


int ssi_rtl(struct request_rec *r, int argc, char **argv) 
{
	int tid;

	if((tid = get_tid()) == MID_FAIL)
		return 0;

	if(mapi_ccfg_match_str(tid, "ARC_UI_Language", "ar"))//arabic, should use rtl
	{
		so_printf(r, "%s", "dir=rtl\n");
	}

	return 0;
}


int ssi_get_lang(struct request_rec *r, int argc, char **argv) 
{

	/**tempory define**/
	char buf[128] = {0};
	int tid;

	if((tid = get_tid()) == MID_FAIL)
		return 0;

	so_printf(r, "%s", mapi_ccfg_get_str(tid, "ARC_UI_Language", buf, sizeof(buf)));

	return 0;

}
 
int ssi_prefix_ip_get(struct request_rec *r, int argc, char **argv) 
{
	char *name=NULL;
	char ipaddr[20] = {0};
	int i=0;
	int tid;

	if((tid = get_tid()) == MID_FAIL)
		return 0;


	if (fmt_args(argc, argv, "%s %d", &name, &i) < 2) {
		so_printf(r, "%s", "Insufficient args\n");
		return -1;
	}

	memset(ipaddr, 0, sizeof(ipaddr));
	
	mapi_ccfg_get_str(tid, name, ipaddr, sizeof(ipaddr));

	if(i == 2)
		r->bytes_sent += so_printf(r,"%d . %d . %d .",get_single_ip(ipaddr,0),
						   							get_single_ip(ipaddr,1),
						   							get_single_ip(ipaddr,2));
	else
		r->bytes_sent += so_printf(r,"%d.%d.%d.",get_single_ip(ipaddr,0),
						   							get_single_ip(ipaddr,1),
													get_single_ip(ipaddr,2));

	return r->bytes_sent;
}

int ssi_get_single_ip(struct request_rec *r, int argc, char **argv) 
{
	char *name, *c;
	int ret = 0;
	int which;
	char ipaddr[20] = {0};
	int tid;

	if((tid = get_tid()) == MID_FAIL)
		return 0;


	if (fmt_args(argc, argv, "%s %d", &name, &which) < 1) {
		so_printf(r, "%s", "Insufficient args\n");
		return -1;
	}
#if 0
	if(gozila_action){
		sprintf(cname,"%s_%d",name,which);
		char *buf = websGetVar(wp, cname, NULL);
		if(buf)
			return so_printf(r, "%s", buf);
	}
#endif  
	c = mapi_ccfg_get_str(tid, name, ipaddr, sizeof(ipaddr));

	if (c == ipaddr) {
		ret += so_printf(r, "%d", get_single_ip(c,which));
	}
	else
		ret += so_printf(r, "0");

	return ret;
}




/*
 * Example: 
 * HEARTBEAT_SUPPORT = 1
 * <% support_invmatch("HEARTBEAT_SUPPORT", "1", "<!--"); %> does not produce
 * HEARTBEAT_SUPPORT = 0
 * <% support_invmatch("HEARTBEAT_SUPPORT", "1", "-->"); %> produces "-->"
 */
int ssi_support_invmatch(struct request_rec *r, int argc, char **argv) 
{
	char *name, *value, *output;
	//struct support_list *v;
	
	if(fmt_args(argc, argv, "%s %s %s", &name, &value, &output) < 3) 
	{
		so_printf(r, "%s", "Insufficient args\n");
		return -1;
	}

#if 0	
	if(!strcasecmp(name, "WL_STA_SUPPORT") ||
	   !strcasecmp(name, "SYSLOG_SUPPORT"))
		return so_printf(wp, output);

	for(v = supports ; v < &supports[SUPPORT_COUNT] ; v++) 
	{
   		if(!strcmp(v->supp_name, name)){
			if (strcmp(v->supp_value, value)){
				return websWrite(wp, output);
			}
			else
				return 1;
		}
   	}
#endif
	return so_printf(r, "%s", output);
}

int ssi_support_match(struct request_rec *r, int argc, char **argv) 
{
	char *name, *value, *output;
	//struct support_list *v;
	
	if(fmt_args(argc, argv, "%s %s %s", &name, &value, &output) < 3) 
	{
		so_printf(r, "%s", "Insufficient args\n");
		return -1;
	}

#if 0	
	if(!strcasecmp(name, "WL_STA_SUPPORT") ||
	   !strcasecmp(name, "SYSLOG_SUPPORT"))
		return so_printf(wp, output);

	for(v = supports ; v < &supports[SUPPORT_COUNT] ; v++) 
	{
		if(!strcmp(v->supp_name, name)){
			if (strcmp(v->supp_value, value)){
				return websWrite(wp, output);
			}
			else
				return 1;
		}
	}
#endif
	return so_printf(r, "%s", output);
}

/*
 * Example: 
 * wan_proto=dhcp
 * <% nvram_else_match("wan_proto", "dhcp", "0","1"); %> produces "0"
 * <% nvram_else_match("wan_proto", "static", "0","1"); %> produces "1"
 */
int ssi_nvram_else_match(struct request_rec *r, int argc, char **argv) 
{
	char *name, *match, *output1, *output2;
	int tid;

	if((tid = get_tid()) == MID_FAIL)
		return 0;

	if (fmt_args(argc, argv, "%s %s %s %s", &name, &match, &output1, &output2) < 4) {
		so_printf(r, "%s", "Insufficient args\n");
		return -1;
	}

	if (mapi_ccfg_match_str(tid, name, match))
		return so_printf(r, "%s", output1);
	else
		return so_printf(r, "%s", output2);

	return 0;
}	

/*
 * Example: 
 * wan_proto=dhcp
 * <% nvram_match("wan_proto", "dhcp", "selected"); %> produces "selected"
 * <% nvram_match("wan_proto", "static", "selected"); %> does not produce
 */
int ssi_nvram_match(struct request_rec *r, int argc, char **argv) 

{
	char *name, *match, *output;
	int tid;

	if((tid = get_tid()) == MID_FAIL)
		return 0;


	if (fmt_args(argc, argv, "%s %s %s", &name, &match, &output) < 3) {
		so_printf(r, "%s", "Insufficient args\n");
		return -1;
	}

	if (mapi_ccfg_match_str(tid, name, match))
		return so_printf(r, "%s", output);

	return 0;
}	
/*
 * Example: 
 * wan_proto=dhcp
 * <% nvram_n_else_match("wan_proto", "dhcp", "len","0","1"); %>
 * <% nvram_n_else_match("wan_proto", "static", "len","0","1"); %>
 */
int ssi_nvram_n_else_match(struct request_rec *r, int argc, char **argv)
{
	char *name, *match, *output1, *output2;
	int len;
	char buf[MID_MNG_MAX_VALUE_LEN];
	int tid;

	if((tid = get_tid()) == MID_FAIL)
		return 0;


	if (fmt_args(argc, argv, "%s %s %d %s %s", &name, &match, &len,&output1,&output2) < 5) {
		so_printf(r, "%s", "Insufficient args\n");
		return -1;
	}
	
	if (!strncmp(mapi_ccfg_get_str(tid, name, buf, sizeof(buf)), match, len))
		return so_printf(r, "%s", output1);
	else
		return so_printf(r, "%s", output2);
}	
/*
 * Example: 
 * wan_proto=dhcp
 * <% nvram_invmatch("wan_proto", "dhcp", "disabled"); %> does not produce
 * <% nvram_invmatch("wan_proto", "static", "disabled"); %> produces "disabled"
 */
int ssi_nvram_invmatch(struct request_rec *r, int argc, char **argv) 

{
	char *name, *invmatch, *output;
	int tid;

	if((tid = get_tid()) == MID_FAIL)
		return 0;


	if (fmt_args(argc, argv, "%s %s %s", &name, &invmatch, &output) < 3) {
		so_printf(r, "%s", "Insufficient args\n");
		return -1;
	}

	if (!mapi_ccfg_match_str(tid, name, invmatch))	//temporary
		return so_printf(r, "%s", output);

	return 0;
}	

int ssi_nvram_selget(struct request_rec *r, int argc, char **argv) 
{
	char *name, *c;
	int ret = 0;
	char buf[128] = {0};
	int tid;

	if((tid = get_tid()) == MID_FAIL)
		return 0;


	if (fmt_args(argc, argv, "%s", &name) < 1) {
		so_printf(r, "%s", "Insufficient args\n");
		return -1;
	}
#if 0
	if(gozila_action){
		char *buf = websGetVar(wp, name, NULL);
		if(buf)
			return so_printf(r, "%s", buf);
	}
#endif

	c = mapi_ccfg_get_str(tid, name, buf, sizeof(buf));
	for (; *c; c++) {
		if (isprint((int) *c) &&
		    *c != '"' && *c != '&' && *c != '<' && *c != '>')
			ret += so_printf(r, "%c", *c);
		else
			ret += so_printf(r, "&#%d", *c);
	}

	return ret;
}

int ssi_nvram_selmatch(struct request_rec *r, int argc, char **argv) 
{
	char *name, *match, *output;
	char *type;
	char buf[128] = {0};
	int tid;

	if((tid = get_tid()) == MID_FAIL)
		return 0;


	if (fmt_args(argc, argv, "%s %s %s", &name, &match, &output) < 3) {
		so_printf(r, "%s", "Insufficient args\n");
		return -1;
	}

	type = mapi_ccfg_get_str(tid, name, buf, sizeof(buf));

	if (!strcmp(type, match)) {
		r->bytes_sent += so_printf(r, "%s", output);
	}

	return 0;

}


int ssi_get_dns_ip(struct request_rec *r, int argc, char **argv) 
{
	char *name;
	int count, which;
	char word[256] = {0}, *next;
	char buf[256] = {0};
	int tid;

	if((tid = get_tid()) == MID_FAIL)
		return 0;


	if (fmt_args(argc, argv, "%s %d %d", &name, &which, &count) < 3) {
		so_printf(r, "%s", "Insufficient args\n");
		return -1;
	}

	mapi_ccfg_get_str(tid, name, buf, sizeof(buf));
	foreach(word, buf, next) {
		if (which-- == 0)
			return so_printf(r, "%d", get_single_ip(word,count));
	}

	return so_printf(r, "%s", "0");	// not find
}


int ssi_check_ipv6_support(struct request_rec *r, int argc, char **argv)
{
	return so_printf(r, "%s", "1");
}

int ssi_get_http_method(struct request_rec *r, int argc, char **argv)
{
	return so_printf(r, "%s", r->method);
}


int checkStringToHTML(char *pstrS, char *pstrD)
{
	short i, str_length, temp_len;
	char one_byte;

	if((!pstrS) || (!pstrD))
		return 0;

	str_length = strlen(pstrS);
	pstrD[0] = 0;

	for (i=0; i<str_length; i++)
	{
		one_byte = pstrS[i];

		switch (one_byte) {
			case 0x22: // ", double quotes
				strcat(pstrD, "&#34;");
				break;
			case 0x26: // &, ampersand
				strcat(pstrD, "&#38;");
				break;
			case 0x27: // ', single quotes
				strcat(pstrD, "&#39;");
				break;
			case 0x3C: // <, less than sign
				strcat(pstrD, "&#60;");
				break;
			case 0x3E: // >, greater than sign
				strcat(pstrD, "&#62;");
				break;
			case 0x5C: // \, backslash
				strcat(pstrD, "&#92;");
				break;
			case 0x2E: // ., period
				strcat(pstrD, "&#46;");
				break;
			default:
				temp_len = strlen(pstrD);
				pstrD[temp_len+0] = one_byte;
				pstrD[temp_len+1] = 0x00;
				break;
		}
	}

	return 1;
}

int ssi_get_wan_mac(struct request_rec *r, int argc, char **argv)
{
	char wan_mac[24] = {0};
	int tid;

	if((tid = get_tid()) == MID_FAIL)
		return 0;


	mapi_ccfg_get_str(tid, "ARC_WAN_0_MACaddr", wan_mac, sizeof(wan_mac));

		r->bytes_sent += so_printf(r, "%s", wan_mac);
	return r->bytes_sent;
}

int ssi_get_http_from(struct request_rec *r, int argc, char **argv)
{
	char *type;
	char mac[20] = {0};
	int access_from = 0;
/*	 no used
#ifndef CONFIG_HTTPD_HOT_FIX_SW1 //KILL_ME hugh 2014/1/14 this code
	struct client_info *wlmac;
	int count_wl = 0;
	int i;
#endif
*/
//#ifdef CONFIG_HTTPD_HOT_FIX_SW1 //hugh KILL_ME
	FILE *fp;
	char line[80];
//#endif //CONFIG_HTTPD_HOT_FIX_SW1
	
	if (fmt_args(argc, argv, "%s", &type) < 1) {
		so_printf(r, "%s", "Insufficient args\n");
		return -1;
	}

	/* v4/v6 */
	ht_dbg("request from: IP=[%s], MAC=[%s], TYPE=[%s]\n", 
			r->remote_ipstr, get_mac_by_ip(r->remote_ipstr, mac), 
			(is_localhost(r->remote_ipstr)==0) ? "wan" : "lan");

	if (!strcmp(type, "ip"))
	{
		r->bytes_sent += so_printf(r, "%s", r->remote_ipstr);
	}
	else if (!strcmp(type, "mac"))
	{
		memset(mac, 0, sizeof(mac));

		r->bytes_sent += so_printf(r, "%s", get_mac_by_ip(r->remote_ipstr, mac));
	}
	else if (!strcmp(type, "port"))
	{
		memset(mac, 0, sizeof(mac));
		get_mac_by_ip(r->remote_ipstr, mac);
		if(mac[0] != '\0')
		{
				// NOTE: 2014/1/2,hugh
				//      old code no support API get_wl_assoc_list, we need manually get the wifi list and check if inside
				//
				// call vendor-based check wifi MAC list
				mng_action("dump_wl_assoclist", "");
				//sleep(1); //FIXME: 1s is enough?

				if ((fp = fopen("/tmp/wl_assoclist", "r"))) {
					//now /tmp/wl_assoclist only contains assocated mac.
					//please refer /sbin/arc_wlan.
					//Alpha modified 2013-03-19
					while( fgets(line, sizeof(line), fp) != NULL ) {
						if((strlen(line) > 0) && (line[strlen(line) - 1] == '\n'))
						{
							line[strlen(line) - 1] = '\0';
							// why use strcasecmp(), because we unknow module if support uppercase!
							if(!strcasecmp(line, mac)){
								access_from = 3;  // access from WIFI
// we no capability to know if from geust
//#ifdef GUEST_NETWORK_SUPPORT
//						if(wlmac->from & FROM_GUEST == FROM_GUEST)
//							access_from = 1; // access from Guest network
//#endif
								break;
							}
						}
					}
					fclose(fp);
				}
//#endif //CONFIG_HTTPD_HOT_FIX_SW1
		}

		if(access_from == 0) // Not WIFI, NOT Guest network
			access_from = (is_localhost(r->remote_ipstr)==0) ? 2 : 0; // 2 For WAN, 0 for wired LAN.
		if(access_from == 0)
			r->bytes_sent += so_printf(r, "%s", "lan");
		else if(access_from == 1)
			r->bytes_sent += so_printf(r, "%s", "gn");
		else if(access_from == 2)
			r->bytes_sent += so_printf(r, "%s", "wan");
		else if(access_from == 3)
			r->bytes_sent += so_printf(r, "%s", "wlan");
	}
	else 
		return 0;

	return r->bytes_sent;

}

/* piaddr() turns an iaddr structure into a printable address. */
/* XXX: should use a const pointer rather than passing the structure */
const char *piaddr(const struct iaddr addr)
{
    static char
        pbuf[sizeof("ffff:ffff:ffff:ffff:ffff:ffff:255.255.255.255")];
             /* "255.255.255.255" */

    /* INSIST((addr.len == 0) || (addr.len == 4) || (addr.len == 16)); */

    if (addr.len == 0) {
        return "<null address>";
    }
    if (addr.len == 4) {
        return inet_ntop(AF_INET, addr.iabuf, pbuf, sizeof(pbuf));
    }
    if (addr.len == 16) {
        return inet_ntop(AF_INET6, addr.iabuf, pbuf, sizeof(pbuf));
    }

    printf("%s:%d: Invalid address length %d.\n", __func__, __LINE__, addr.len);
    /* quell compiler warnings */
    return NULL;
}

int ssi_dump_clients(struct request_rec *r, int argc, char **argv)
{
	//FIXME: not merge belkin chang
#if 0	
	mng_action("dump_wl_assoclist", "");
	usleep(50000);
	mng_action("ui_dump_dhcpd_leases", "");
	sleep(1);
#endif	
	return 1;
}

static int dhcp_count;
static int dump_dhcp_leased(struct request_rec *r, char *lease_file, int with_leasetime
#ifdef NETBIOS_SUPPORT
		, struct client_info *netbios, int count_netbios
#endif
#ifdef MDNS_SUPPORT
		, struct client_info *mdns, int count_mdns
#endif
		)
{
	int ret = 0;
	FILE *fp;
	int i = 0, count = 0;
	struct lease_t lease;
	char mac[20]="", expires_time[50]="", hostname[128]="";
	const char *ipaddr; 
	char interface[20];
	struct client_info *dhcpclient = NULL;
	/*
 	  pete_zhang    2012.11.30    Belkin bug ARCADYAN-2232 for AC1200/AC1800

	  Bug description: Clients on the Guest access network are shown in the 
	  DHCP client list twice after they move to the normal SSID

	  Solution: Add start_time information
	*/
	#ifdef DHCP_LEASE_START_TIME_SUPPORT
	unsigned long start_time;
	#endif
	/*end    pete_zhang    2012.11.30*/
	
	unsigned long expires;

	if (with_leasetime)
	{
		mng_action("ui_dump_dhcpd_leases", "");		
		sleep(1);


		/* Parse leases file */
		if((fp = fopen(lease_file, "r"))) 
		{
			while (fread(&lease, sizeof(lease), 1, fp)) 
			{
				strcpy(mac,"");
	
				for (i = 0; i < 6; i++) {
					sprintf(mac+strlen(mac),"%02X", lease.chaddr[i]);
					if (i != 5) sprintf(mac+strlen(mac),":");
				}
				
				mac[17] = '\0';

				if(!strcmp(mac,"00:00:00:00:00:00"))
				continue;
			
				ipaddr = piaddr(lease.yiaddr);
			
				/*
				pete_zhang	  2012.11.30	Belkin bug ARCADYAN-2232 for AC1200/AC1800
				*/	
				#ifdef DHCP_LEASE_START_TIME_SUPPORT
				start_time = lease.start_time;
				#endif
				/*end	 pete_zhang    2012.11.30*/ 	
	
				expires = ntohl(lease.expires);
	
				strcpy(expires_time,"");
				if (!expires){
					continue;
					strcpy(expires_time,"expired");
				}
				else {
					if (expires > 60*60*24) {
						sprintf(expires_time+strlen(expires_time),"%ld days, ",expires / (60*60*24));
						expires %= 60*60*24;
					}
					if (expires > 60*60) {
						sprintf(expires_time+strlen(expires_time),"%02ld:",expires / (60*60));	// hours
						expires %= 60*60;
					}
					else{
						sprintf(expires_time+strlen(expires_time),"00:");	// no hours
					}
					if (expires > 60) {
						sprintf(expires_time+strlen(expires_time),"%02ld:",expires / 60);	// minutes
						expires %= 60;
					}
					else{
						sprintf(expires_time+strlen(expires_time),"00:");	// no minutes
					}
	
					sprintf(expires_time+strlen(expires_time),"%02ld:",expires);	// seconds
		
					expires_time[strlen(expires_time)-1]='\0';
	
				}
	
				if(0)//is_wireless_mac(mac)) 
				{
					strcpy(interface, "WL");
				}
				else
					strcpy(interface, "LAN");
	
				strcpy(hostname, lease.hostname);

				#ifdef NETBIOS_SUPPORT
				/* Check Netbios Table */
				for (i=0; i<count_netbios; i++) {
					/*cprintf("netbios: ip[%s] mac[%s] hostname[%s]\n", netbios[i].ip, netbios[i].mac, netbios[i].name);*/
					if (!strcasecmp(netbios[i].mac, mac)) {
					/* MAC existed in DHCP table or ARP table or Wireless table */
						if (hostname[0] == '\0')
							strncpy(hostname, netbios[i].name, strlen(netbios[i].name));
						break;
					}
				}
				#endif
	
				#ifdef MDNS_SUPPORT
				/* Check mDNS Table */
				for (i=0; i<count_mdns; i++) {
					/*cprintf("mdns: ip[%s] mac[%s] hostname[%s]\n", mdns[i].ip, mdns[i].mac, mdns[i].name);*/
					if (!strcasecmp(mdns[i].mac, mac)) {
					/* MAC existed in DHCP table or ARP table or Wireless table */
						if (hostname[0] == '\0')
							strncpy(hostname, mdns[i].name, strlen(mdns[i].name));
						break;
					}
				}
				#endif
				/*
				pete_zhang	  2012.11.30	Belkin bug ARCADYAN-2232 for AC1200/AC1800
				*/	
				#ifdef DHCP_LEASE_START_TIME_SUPPORT
				ret += so_printf(r, "%c'%s','%s','%s','%ld','%s','%s'\n",dhcp_count ? ',' : ' ', 
						hostname,
						ipaddr, 
						mac, 
						start_time,
						expires_time, 
						interface);
				#else
				ret += so_printf(r, "%c'%s','%s','%s','%s','%s'\n",dhcp_count ? ',' : ' ', 
						hostname,
						ipaddr, 
						mac, 
						expires_time, 
						interface);
				#endif
				/*end	 pete_zhang    2012.11.30*/
				dhcp_count ++;
		   }
		}
	
		if(fp) fclose(fp);
		
	}
	else
	{
		dhcpclient = get_fast_dhcp_list(&count);
		
		strcpy(interface, "LAN");

		for (i = 0; i< count; i++)
		{
			ret += so_printf(r, "%c'%s','%s','%s','%s','%s'\n", i?',' : ' ', dhcpclient[i].name, dhcpclient[i].ip, dhcpclient[i].mac, "", interface);
		}

		if (dhcpclient)
			free(dhcpclient);
		
	}


	return ret;
}

int ssi_dump_dhcp_leased(struct request_rec *r, int argc, char **argv)
{
	int ret = 0;
	int with_leasetime = 0;

	dhcp_count = 0;

#ifdef MDNS_SUPPORT
	struct client_info *mdns = NULL;
	int count_mdns = 0;
#endif
#ifdef NETBIOS_SUPPORT
	struct client_info *netbios = NULL;
	int count_netbios = 0;
#endif

	if (argc)
	{
		if (argv[0][0] == '1') 
			with_leasetime = 1;
	}


	/**Real actions for dump dhcpd clients**/

#ifdef NETBIOS_SUPPORT
	netbios = get_netbios_list(&count_netbios);
#endif
#ifdef MDNS_SUPPORT
	mdns = get_mdns_list(&count_mdns);
#endif

	/* Parse leases file */
	ret += dump_dhcp_leased(r, DHCPD_CLIENTS_FILE, with_leasetime
#ifdef NETBIOS_SUPPORT
			, netbios, count_netbios
#endif
#ifdef MDNS_SUPPORT
			, mdns, count_mdns
#endif
			); 

#ifdef GUEST_NETWORK_SUPPORT
#if defined(VENDOR_ZZ_SUPPORT) || defined(VENDOR_BELKIN_SUPPORT)
	ret += dump_dhcp_leased(r, DHCPD_GN_CLIENTS_FILE
#ifdef NETBIOS_SUPPORT
			, netbios, count_netbios
#endif
#ifdef MDNS_SUPPORT
			, mdns, count_mdns
#endif
			); 
#endif /* VENDOR_ZZ_SUPPORT || VENDOR_BELKIN_SUPPORT*/
#endif /* GUEST_NETWORK_SUPPORT */

#ifdef NETBIOS_SUPPORT
	if (netbios)
		free(netbios);
#endif

#ifdef MDNS_SUPPORT
	if (mdns)
		free(mdns);
#endif
	return ret;
}

int ssi_wireless_active_table(struct request_rec *r, int argc, char **argv)
{
	FILE *fp;
	char line[80];
	char list[3][20];
	int count=0;

	if ((fp = fopen("/tmp/wl_assoclist", "r"))) {
            while( fgets(line, sizeof(line), fp) != NULL ) {
		memset(list, 0, sizeof(list));
                sscanf(line,"%s %s %s",list[0], list[1], list[2]);

                if(strcmp(list[0],"assoclist"))
		{
                    continue;
		}
                    so_printf(r, "%c'%s', '%s', '%s', '%s', '%s'\n", 
			count++?',':' ', "ssid", list[2], list[1], "1", "1");
            }    
            fclose(fp);
        }
	return 1;
}


int ssi_onload(struct request_rec *r, int argc, char **argv)
{
    char *name, *arg;
	int pid;

	if (fmt_args(argc, argv, "%s %s", &name, &arg) < 2) {
		so_printf(r, "%s", "Insufficient args\n");
		return -1;
	}
	
	if (!name || !arg)
		return -1;
	
	pid = find_pid_by_ps(name);

	if (pid>0) {
		r->bytes_sent += so_printf(r, "%s", arg);
	}

	return r->bytes_sent;
}


char * rfctime(const time_t *timep)
{
    static char s[48] = {0};
    struct tm tm;
    char timezone[32] = {0};
	int tid;

	if((tid = get_tid()) == MID_FAIL)
		return 0;


   	mapi_ccfg_get_str(tid, "ARC_SYS_TimeZone", timezone, sizeof(timezone));

    setenv("TZ",timezone, 1);
    memcpy(&tm, localtime(timep), sizeof(struct tm));
    strftime(s, 47, "%a, %d %b %Y %H:%M:%S", &tm);

    return s;
}

/* Report time in RFC-822 format */
int ssi_localtime(struct request_rec *r, int argc, char **argv)
{
    time_t tm;

    time(&tm);

    if(time(0) > (unsigned long)60*60*24*365)
        return so_printf(r, "%s", rfctime(&tm));
    else
        return so_printf(r, "Not Available");
}

int ssi_date_timezone(struct request_rec *r, int argc, char **argv)
{
 int ret = 0,  nbytes = 0;
 FILE *fp = NULL;
 char line[64] = "";

 system("date -R > /tmp/tmp_date");
 if ((fp = fopen("/tmp/tmp_date", "r")) == NULL)
 {
     printf("%s", "Cannot open tmp_date file!\n");
     return -1;
 }

 if(fgets(line, sizeof(line)-1 , fp) != NULL )
 {
 		line[strlen(line)-1] = 0;
     nbytes += so_printf(r, "%s", line);
 }

 fclose(fp);
 unlink("/tmp/tmp_date");

 return nbytes;
}

int ssi_get_firmware_version(struct request_rec *r, int argc, char **argv)
{
	char ver[32] = {0}, rev[32] = {0};
	int tid;

	if((tid = get_tid()) == MID_FAIL)
		return 0;

	mapi_ccfg_get_str(tid, "ARC_SYS_FWVersion", ver, sizeof(ver));
	mapi_ccfg_get_str(tid, "ARC_SYS_FWSubVersion", rev, sizeof(rev));
#if defined(WE410443_TA)
	return so_printf(r, "%s-%s", ver, rev);
#elif defined(WE410443_6DX)
	return so_printf(r, "%s.%s", ver, rev);
#else
	return so_printf(r, "%s %s", ver, rev);
#endif	
}

int ssi_compile_date(struct request_rec *r, int argc, char **argv)
{
	//return so_printf(r, "%s", "Oct. 31, 2011");
	 char year[5], mon[4], day[3];
	 char string[20];

	 sscanf(__DATE__, "%s %s %s", mon, day, year);
	 snprintf(string, sizeof(string), "%s. %s, %s", mon, day, year);

	 r->bytes_sent += so_printf(r, "%s", string);
	 return r->bytes_sent;
}

int ssi_get_wan_dns(struct request_rec *r, int argc, char **argv)
{
	int which;
	
	char list[256] = {0}, word[128] = {0}, *next;
	int tid;

	if((tid = get_tid()) == MID_FAIL)
		return 0;

	
	if (fmt_args(argc, argv, "%d", &which) < 1) {
		so_printf(r, "%s", "Insufficient args\n");
		return -1;
	}

	mapi_ccfg_get_str(tid, "ARC_SYS_DNS", list, sizeof(list));

	foreach(word, list, next) 
	{
		if (which-- == 0)
		{
			return so_printf(r, "%s", word);
		}
	}

	return r->bytes_sent;
}

int ssi_disable_test_channel(struct request_rec *r, int argc, char **argv)
{
	char value[64];
	FILE *fp;

	if ((fp = fopen("/tmp/test_channel", "r")) != NULL)
	{
		if(fgets(value, sizeof(value), fp) != NULL)
			if ( value[0] == '1')
			{
				fclose(fp);
				return so_printf(r, "0");
			}
		fclose(fp);
	}
	return so_printf(r, "1");
}

int ssi_get_wanif_index(struct request_rec *r, int argc, char **argv)
{
	//following is example.
	char wan_port[32];
	int tid;

	if((tid = get_tid()) == MID_FAIL)
		return 0;


	memset(wan_port, 0, sizeof(wan_port));
	mapi_ccfg_get_str(tid, "ARC_WAN_Port", wan_port, sizeof(wan_port));
	if(strlen(wan_port) == 0)
		strcpy(wan_port, "0");

	return so_printf(r, "%s", wan_port);
}

int ssi_get_firmware_short_version(struct request_rec *r, int argc, char **argv)
{
	char buf[32] = {0};
	int tid;

	if((tid = get_tid()) == MID_FAIL)
		return 0;


	mapi_ccfg_get_str(tid, "ARC_SYS_FWVersion", buf, sizeof(buf));
	return so_printf(r, "%s", buf);
}

int ssi_get_model_name(struct request_rec *r, int argc, char **argv)
{
	char buf[64] = {0};
	int tid;

	if((tid = get_tid()) == MID_FAIL)
		return 0;


	mapi_ccfg_get_str(tid, "ARC_SYS_ModelName", buf, sizeof(buf));
	return so_printf(r, "%s", buf);
}

int ssi_get_firmware_title(struct request_rec *r, int argc, char **argv)
{
	char *type;

	if (fmt_args(argc, argv, "%s", &type) < 1) {
		so_printf(r, "%s", "Insufficient args\n");
		return -1;
	}

	//following is example.
	return so_printf(r, "%d", 2);
}

int ssi_webs_get(struct request_rec *r, int argc, char **argv)
{
	char *name, *dft;

	if (fmt_args(argc, argv, "%s %s", &name, &dft) < 2) {
		so_printf(r, "%s", "Insufficient args\n");
		return -1;
	}

	r->bytes_sent += so_printf(r, "%s", get_cgi_df(name, dft) ? : "");

	return r->bytes_sent;
}


int ssi_get_connected_device(struct request_rec *r, int argc, char **argv)
{
	struct client_info *cli_tbl = NULL;
	int i, count = 0;

	/* also show guest network clients */
	cli_tbl = get_client_list(&count, 1);

	if (cli_tbl == NULL)
		return 0;

	if (count > MAX_CLIENT_NUM)
		count = MAX_CLIENT_NUM;

	for (i=0; i<count; i++) {
		printf("cli_tbl[%d]=[%s], from=[%d]\n", i, cli_tbl[i].mac, cli_tbl[i].from);

		so_printf(r, "\"%s\t%s\"\n%s", cli_tbl[i].name, cli_tbl[i].mac, (i==count-1)? "" : ",");
	}
	
	if (cli_tbl)
		free(cli_tbl);

	return 0;
}

int ssi_get_pclist(struct request_rec *r, int argc, char **argv)
{
	struct client_info *cli_tbl = NULL;
	int i, count = 0;

	/* also show guest network clients */
	cli_tbl = get_client_list(&count, 1);

	if (cli_tbl == NULL)
		return 0;

	if (count > MAX_CLIENT_NUM)
		count = MAX_CLIENT_NUM;

	for (i=0; i<count; i++) {
		printf("cli_tbl[%d]=[%s], from=[%d], ip=[%s], name=[%s]\n", i, cli_tbl[i].mac, cli_tbl[i].from, cli_tbl[i].ip, cli_tbl[i].name);

		so_printf(r, "\"%s;%s;%s\"\n%s", cli_tbl[i].mac, cli_tbl[i].ip, cli_tbl[i].name ,(i==count-1)? "" : ",");
	}
	
	if (cli_tbl)
		free(cli_tbl);

	return 0;
}

int ssi_get_ipv6_info(struct request_rec *r, int argc, char **argv)
{
	char devname[20];
	int plen, scope, dad_status, if_idx, if_flags6, if_probes;
	char addr6p[8][5], wan_gw[8][5];
	char ip[INET6_ADDRSTRLEN];
	struct in6_addr inet6_addr;
	char print_addr[INET6_ADDRSTRLEN],print_gateway[INET6_ADDRSTRLEN], wan_ipv6_proto[10];
	FILE *fp = NULL;
	int tid;

	if((tid = get_tid()) == MID_FAIL)
		return 0;

   
	memset(wan_gw, 0, sizeof(wan_gw));
	memset(print_addr, 0, sizeof(print_addr));
	memset(print_gateway, 0, sizeof(print_gateway));
		
	fp = fopen(PATH_PROC_NET_IFFLAGS6, "r");
	if (fp != NULL) {
		while (fscanf(fp, "%4s%4s%4s%4s%4s%4s%4s%4s %02x %02x %02x %02x %02x %08x %20s %4s%4s%4s%4s%4s%4s%4s%4s\n",
			addr6p[0],addr6p[1],addr6p[2],addr6p[3],
			addr6p[4],addr6p[5],addr6p[6],addr6p[7],
			&if_idx,
			&plen,
			&scope,
			&dad_status,
			&if_probes,
			&if_flags6,
			devname,
			wan_gw[0],wan_gw[1],wan_gw[2],wan_gw[3],
			wan_gw[4],wan_gw[5],wan_gw[6],wan_gw[7]) != EOF) {
			if (mapi_ccfg_match_str(tid, "ARC_WAN_0_Iface", devname)&&(scope & IPV6_ADDR_SCOPE_MASK) == SCOPE_GLOBAL) {
				sprintf(ip, "%4s:%4s:%4s:%4s:%4s:%4s:%4s:%4s",
					addr6p[0], addr6p[1], addr6p[2], addr6p[3],
					addr6p[4], addr6p[5], addr6p[6], addr6p[7]);
				inet_pton(AF_INET6, ip, &inet6_addr);
				inet_ntop(AF_INET6, &inet6_addr, print_addr, sizeof(print_addr));
				if (strcmp(wan_gw[0], "")) {
					sprintf(ip, "%4s:%4s:%4s:%4s:%4s:%4s:%4s:%4s",
						wan_gw[0], wan_gw[1],wan_gw[2],wan_gw[3],
						wan_gw[4],wan_gw[5],wan_gw[6],wan_gw[7]);
					inet_pton(AF_INET6, ip, &inet6_addr);
					inet_ntop(AF_INET6, &inet6_addr, print_gateway, sizeof(print_gateway));
				}
			}
		}
		fclose( fp );

		// Static-IPv6 can't get gateway from if_flags6
		mapi_ccfg_get_str(tid, "ARC_WAN_0_IP6_Proto", wan_ipv6_proto, sizeof(wan_ipv6_proto));
		if (strcmp(wan_ipv6_proto, "static") == 0) { 
			mapi_ccfg_get_str(tid, "ARC_WAN_0_IP6_StaticGateway", print_gateway, sizeof(print_gateway));
		}

		printf("ssi_get_ipv6_info: %s/%s\n", print_addr,print_gateway);
		return so_printf(r, "%s/%s", print_addr,print_gateway);
	}
       	return -1;
}
 
int get_linklocal_ipv6addr(struct in6_addr *addr6, char *iface)
{
	char str[128], address[INET6_ADDRSTRLEN];
	char *addr, *index, *prefix, *scope, *flags, *name;
	char *delim = " \t\n", *p, *q;
	FILE *fp;
	int i = 0, count = 0;

	if (!addr6 || !iface) {
		printf("addr6 and iface can't be NULL!\n");
		return -1;
	}

	if (NULL == (fp = fopen(PATH_PROC_NET_IFINET6, "r"))) {
		perror("fopen error");
		return -1;
	}

	while (fgets(str, sizeof(str), fp)) {
		printf("str:%s", str);
		addr = strtok(str, delim);
		index = strtok(NULL, delim);
		prefix = strtok(NULL, delim);
		scope = strtok(NULL, delim);
		flags = strtok(NULL, delim);
		name = strtok(NULL, delim);
		printf("addr:%s, index:0x%s, prefix:0x%s, scope:0x%s, flags:0x%s, name:%s\n",
		addr, index, prefix, scope, flags, name);

		if (strcmp(name, iface))
			continue;

		/* Just get IPv6 linklocal address */
		if (IPV6_ADDR_LINKLOCAL != (unsigned int)strtoul(scope, NULL, 16))
			continue;

		memset(address, 0x00, sizeof(address));
		p = addr;
		q = address;
		while (*p != ' ' && i++ < 32) {
			if (count == 4) {
				*q++ = ':';
				count = 0;
			}
			*q++ = *p++;
			count++;
		}
		printf("find out %s's linklocal IPv6 address: %s\n", iface, address);

		inet_pton(AF_INET6, address, addr6);
		break;
	}

	fclose(fp);
	return 0;
}

int ssi_get_ipv6_localaddr(struct request_rec *r, int argc, char **argv)
{
	struct in6_addr addr6;
	char ifname[24] = {0}, address[64] = {0};
	int tid;

	if((tid = get_tid()) == MID_FAIL)
		return 0;


	mapi_ccfg_get_str(tid, "ARC_LAN_0_Ifname", ifname, sizeof(ifname));

	if (get_linklocal_ipv6addr(&addr6, ifname)) {
		printf("\nGet IPv6 linklocal address of %s failed :(\n", ifname);
		return -1;
	}
	if (inet_ntop(AF_INET6, &addr6, address, sizeof(address)))
		printf("\nIPv6 linklocal address of %s is %s :)\n", ifname, address);

	return so_printf(r, "%s", address);
}

int ssi_get_ipv6_prefixaddr(struct request_rec *r, int argc, char **argv)
{
	char addr[64] = {0}, len[24] = {0};
	int tid;

	if((tid = get_tid()) == MID_FAIL)
		return 0;


	mapi_ccfg_get_str(tid, "ARC_IP6_DelegatePrefix", addr, sizeof(addr));
	mapi_ccfg_get_str(tid, "ARC_IP6_DelegatePrefixLen", len, sizeof(len));

	if(strcmp(addr, ""))
		return so_printf(r, "%s/%s", addr, len);
	else
		return -1;
}

int ssi_get_sipp_status(struct request_rec *r, int argc, char **argv)
{
	int nbytes = 0;
	FILE *fp = NULL;
	char buffer[128] = "";
	char internal_file[] = "/tmp/arc-middle/sipproxy/internal_status";
	char external_file[] = "/tmp/arc-middle/sipproxy/external_status";
	int a = 0;

	/* alert DBUS agent */
	mng_action("ui_sipp_req_status", "");
    /* wait for SIP Proxy to generate status file*/	
    sleep(1);

    if (access(internal_file, 0) != 0 && access(external_file, 0) != 0)
    {
	/* Something wrong.....None of the status files is generated*/
		printf("[%s], Something wrong.....None of the status files is generated!\n", __FUNCTION__);
//		nbytes = -1;
		nbytes += so_printf(r, "[],[]");
    }	
	else
	{
		nbytes += so_printf(r, "[ ");	

		/* check Internal status first*/
		fp = fopen (internal_file, "r");
		if (fp == NULL)
		{
				printf("File Internal_Status not found!\n");
		
		}
		else
		{
				while (fgets (buffer,128, fp) != NULL)
				{
						if (a)
							nbytes += so_printf(r, " , ");
						
						buffer[strlen(buffer)-1] = '\0';
						nbytes += so_printf(r, "\"%s\"", buffer);
						a++;
				}
		
				fclose(fp);
		
				unlink(internal_file);
		}

		nbytes += so_printf(r, "] , [");
		
		a = 0;
		

		/* check External status*/
		fp = fopen (external_file, "r");
		
		if (fp == NULL)
		{
				printf("File External_Status not found!\n");
		}
		else
		{
				while (fgets (buffer,128, fp) != NULL)
				{
					if (a)
						nbytes += so_printf(r, " , ");
		
					buffer[strlen(buffer)-1] = '\0';
					nbytes += so_printf(r, "\"%s\"", buffer);

					a++;
				}
				fclose(fp);
				unlink(external_file);
		}

		nbytes += so_printf(r, "]");

	}


	return nbytes;
}


int ssi_get_sipp_call_log(struct request_rec *r, int argc, char **argv)
{
	int nbytes = 0;
	FILE *fp = NULL;
	char buffer[256] = "";
	char call_log_file[] = "/tmp/arc-middle/sipproxy/call_log";
	int a = 0;

	
	/* alert DBUS agent */
	mng_action("ui_sipp_req_call_log", "");
    /* wait for SIP Proxy to generate status file*/	
    sleep(1);	
	
    if (access(call_log_file, 0) != 0)
    {
        /* Something wrong.....call log is not generated*/
                printf("[%s], Something wrong.....call log is not generated!\n", __FUNCTION__);
                nbytes = -1;
    }
	else
	{
			fp = fopen (call_log_file, "r");
			if (fp == NULL)
			{
					printf("File %s not found!\n", call_log_file);
			}
			else
			{
					/*read line by line from file*/
					while (fgets (buffer,256, fp) != NULL)
					{
						if (a)
							nbytes += so_printf(r, " , ");
					
							/* print each line from file to GUI*/
						buffer[strlen(buffer)-1] = '\0';
						nbytes += so_printf(r, "\"%s\"", buffer);

						a++;
					}
	
					fclose(fp);
					/*delete call log file after read*/
					unlink(call_log_file);
			}	
	}

	return nbytes;
}

int ssi_get_sipp_call_cnt(struct request_rec *r, int argc, char **argv)
{
	int nbytes = 0, len;
	FILE *fp = NULL;
	char buffer[256] = "";
	char call_cnt_file[] = "/tmp/arc-middle/sipproxy/call_cnt";
	
    if (access(call_cnt_file, 0) != 0)
    {
        /* Something wrong.....call cnt is not generated*/
		printf("[%s], Something wrong.....call cnt is not generated!\n", __FUNCTION__);
		nbytes = -1;
    }
	else
	{
		fp = fopen (call_cnt_file, "r");
		if (fp == NULL)
		{
			printf("File %s not found!\n", call_cnt_file);
		}
		else
		{
			if ((len = fread(buffer, 1, 256, fp)) <= 0)
			{
				fclose(fp);
				return nbytes;
			}
			buffer[len] = 0;
			nbytes += so_printf(r, "%s", buffer);	
			fclose(fp);
		}	
	}

	return nbytes;
}

#if defined(VR7516RW22) || defined(WE410443) || (WE5202243_SA)
#define WLAN_STATS_FILE     "/tmp/wlan_stats"

unsigned long int getCurChannel(char *inf){
	char curchannel[8] = {0};
	char cmd[128];
	char string[128];
	FILE *fp;

	sprintf(cmd,"iwpriv %s stat > %s", inf, WLAN_STATS_FILE);
	system(cmd);

	fp = fopen(WLAN_STATS_FILE, "r");
	if( fp == NULL ){
		cprintf("File read error?\n");
		return 0;
	}

			
	while(fgets(string, sizeof(string), fp))
	{
		if(strstr(string, "Current Channel"))
		{
			sscanf(string, "Current Channel = %s", curchannel);
			break;
		}
	}
	fclose(fp);

//	if(outchannel)
//		*outchannel = (unsigned char)curchannel;

	unlink(WLAN_STATS_FILE);
	return strtoul(curchannel,NULL, 10);
}

int ssi_get_cur_channel(struct request_rec *r, int argc, char **argv)
{
	char *band;
	char tmp[8];
	unsigned long int channel = 0;;
	int tid;

	if (fmt_args(argc, argv, "%s", &band) < 1) {
		so_printf(r, "%s", "Insufficient args\n");
		printf("Insufficient args\n");
		return -1;
	}

	if((tid = get_tid()) == MID_FAIL)
		return 0;	

	if(strcmp(band,"5g") == 0)
	{
		mapi_ccfg_get_str(tid, "ARC_WLAN_5G_Channel", tmp, sizeof(tmp));
		channel = atoi(tmp);
		if(channel == 0)
		{
			channel = getCurChannel(WIFI_5G_AP0_NAME);
		}
		return so_printf(r, "%d", channel);
	}
#if defined (WE5202243_SA)	
	else if(strcmp(band,"5ghb") == 0)
	{
		mapi_ccfg_get_str(tid, "ARC_WLAN_5GHB_Channel", tmp, sizeof(tmp));
		channel = atoi(tmp);
		if(channel == 0)
		{
			channel = getCurChannel(WIFI_5GHB_AP0_NAME);
		}
		return so_printf(r, "%d", channel);
	}
#endif
	else
	{
		mapi_ccfg_get_str(tid, "ARC_WLAN_24G_Channel", tmp, sizeof(tmp));
		channel = atoi(tmp);
		if(channel == 0)
		{
			channel = getCurChannel(WIFI_24G_AP0_NAME);
		}
		return so_printf(r, "%d", channel);
	}
}
#endif


/*
 	pete_zhang 2013-12-20	ticket-1167

	description: GUI security enhancement
*/
#ifdef CONFIG_HTTPD_TOKEN_CHECK_SUPPORT
int ssi_httoken_list_output(struct request_rec *r, int argc, char **argv)
{
	int nbytes = 0;


	httoken_print();
	nbytes += httoken_dump( r, cgi_httoken_print_callback );


	return nbytes ;
}

int ssi_httoken_output(struct request_rec *r, int argc, char **argv)
{
	int nbytes = 0;
	unsigned long token = 0;

	token = httoken_get( r->url?r->url:"/" );

	ht_dbg("token=%lu from %s\n", token, r->url?r->url:"NULL" );	

	nbytes += so_printf(r, "%lu", token );

	return nbytes;
}
#endif
/*end pete_zhang        2013-12-20*/

