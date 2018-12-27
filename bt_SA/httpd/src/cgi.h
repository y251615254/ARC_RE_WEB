#ifndef __CGI_H__
#define __CGI_H__

#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <mid_mapi_midcore.h>
#include <arc_cfg_api.h>

#include "httpd.h"
#include "utils.h"

#define TASK_t (void (*)(void))
#undef  TASK_STKSIZ
#define TASK_STKSIZ		14000	//tlhh
#define NOTHING		""
#define	STD_PREFIX		""

#define TP_UPGRADE	1
#define TP_RESTORE	2
#define TP_DEFAULT	3
#define TP_UPGRADE_ROM	4
// hugh 2014/1/24 extend for LTE upgrade
#define TP_UPGRADE_LTE	5
// runsen_lao 2014/12/18 extend for PLC upgrade
#define TP_UPGRADE_PLC	6
// roger 2016/01/04: extend for mtk uboot  firmware upgrade
#define TP_UPGRADE_MTK	7
#define TP_UPGRADE_OWL	8
#define TP_UPGRADE_OWL_AUTO	9

#define DEFAULT_ROUTER_CONF		"/etc/config/glbcfg.dft"
#define USER_ROUTER_CONF		"/tmp/glbcfg.user"

#define MIN(a,b)	((a)>(b)?(b):(a))

typedef struct {
	char *name;
	char *val;
} input_t;


int __init_cgi(CGI_info *p, input_t *inputs, int max_num);
char *__get_cgi(input_t *inputs, char *field);
char *__get_cgi_df(char *field, char *sdf);
int __destroy_cgi(input_t *inputs, int input_num);
char *get_cgi(char *field);
char *get_cgi_df(char *field, char *sdf);

void apply_cgi(int argc, char **argv, void *p);
void upload_cgi(int argc, char **argv, void *p);
void extern_cgi(int argc, char **argv, void *p);

void apply_abstract_cgi(int argc, char **argv, void *p);


extern int sys_upgrade(struct request_rec *r, FILE *stream, int *total, int b_rom); 
extern int sys_restore(struct request_rec *r, char *path, FILE *stream, int *total);
// LTE upgrade
#ifdef CONFIG_HTTPD_LTE_UPGRADE
extern int sys_lte_upgrade(struct request_rec *r, FILE *stream, int *total);
#endif //CONFIG_HTTPD_LTE_UPGRADE

// PLC upgrade
#ifdef CONFIG_HTTPD_PLC_UPGRADE
extern int sys_plc_upgrade(struct request_rec *r, FILE *stream, int *total);
#endif //CONFIG_HTTPD_PLC_UPGRADE

/* vendor specified: every vendor has different conf file */
extern int decrypt_conf_file(const char *in, const char *out);
extern int skip_var(input_t *input_var);
extern int hack_var(int tid, input_t *input_var);

#endif /*__CGI_H__*/
