#ifndef __SSI_HANDLER_H__
#define __SSI_HANDLER_H__

struct ssi_handler {
    char *pattern;
    int (*output)(struct request_rec *r, int argc, char **argv);
};


extern int fmt_args(int argc, char **argv, char *fmt, ...);

#define MAX_CLIENT_NUM	32
#define DHCPD_CLIENTS_FILE "/etc/dhcpd.clients" 
#define DHCPD_GN_CLIENTS_FILE "/etc/dhcpd-gn.clients"

int ssi_wsc_get_count(struct request_rec *r, int argc, char **argv);
int ssi_wsc_get_status(struct request_rec *r, int argc, char **argv);
int ssi_get_max_guest_num(struct request_rec *r, int argc, char **argv);
int ssi_get_wl_maclist(struct request_rec *r, int argc, char **argv);
int ssi_get_wl_max_channel(struct request_rec *r, int argc, char **argv);
int ssi_get_wl_5g_channels(struct request_rec *r, int argc, char **argv);
int ssi_nvram_get(struct request_rec *r, int argc, char **argv);
int ssi_nvram_js_get(struct request_rec *r, int argc, char **argv);
int ssi_nvram_js_get_with_double_quotes(struct request_rec *r, int argc, char **argv);
int ssi_web_include_file(struct request_rec *r, int argc, char **argv);
int ssi_get_lang(struct request_rec *r, int argc, char **argv);
int ssi_prefix_ip_get(struct request_rec *r, int argc, char **argv);
int ssi_no_cache(struct request_rec *r, int argc, char **argv);
int ssi_charset(struct request_rec *r, int argc, char **argv);
int ssi_rtl(struct request_rec *r, int argc, char **argv);
int ssi_Get(struct request_rec *r, int argc, char **argv);
int ssi_Get_SSID(struct request_rec *r, int argc, char **argv);
int ssi_support_invmatch(struct request_rec *r, int argc, char **argv);
int ssi_support_match(struct request_rec *r, int argc, char **argv);
int ssi_nvram_else_match(struct request_rec *r, int argc, char **argv);
int ssi_get_single_ip(struct request_rec *r, int argc, char **argv);
int ssi_nvram_invmatch(struct request_rec *r, int argc, char **argv);
int ssi_nvram_n_else_match(struct request_rec *r, int argc, char **argv);
int ssi_nvram_match(struct request_rec *r, int argc, char **argv);
int ssi_nvram_selget(struct request_rec *r, int argc, char **argv);
int ssi_nvram_selmatch(struct request_rec *r, int argc, char **argv);
int ssi_get_dns_ip(struct request_rec *r, int argc, char **argv);
int ssi_check_ipv6_support(struct request_rec *r, int argc, char **argv);
int ssi_get_http_method(struct request_rec *r, int argc, char **argv);
int ssi_get_wan_mac(struct request_rec *r, int argc, char **argv);
int ssi_get_http_from(struct request_rec *r, int argc, char **argv);
int ssi_dump_clients(struct request_rec *r, int argc, char **argv);
int ssi_dump_dhcp_leased(struct request_rec *r, int argc, char **argv);
int ssi_wireless_active_table(struct request_rec *r, int argc, char **argv);
int ssi_onload(struct request_rec *r, int argc, char **argv);
int ssi_localtime(struct request_rec *r, int argc, char **argv);
int ssi_date_timezone(struct request_rec *r, int argc, char **argv);
int ssi_get_firmware_version(struct request_rec *r, int argc, char **argv);
int ssi_compile_date(struct request_rec *r, int argc, char **argv);
int ssi_get_wan_dns(struct request_rec *r, int argc, char **argv);
int ssi_disable_test_channel(struct request_rec *r, int argc, char **argv);
int ssi_get_wanif_index(struct request_rec *r, int argc, char **argv);
int ssi_get_firmware_short_version(struct request_rec *r, int argc, char **argv);
int ssi_get_model_name(struct request_rec *r, int argc, char **argv);
int ssi_get_firmware_title(struct request_rec *r, int argc, char **argv);
int ssi_get_http_prefix(struct request_rec *r, int argc, char **argv);
int ssi_webs_get(struct request_rec *r, int argc, char **argv);
int ssi_get_connected_device(struct request_rec *r, int argc, char **argv);
int ssi_get_pclist(struct request_rec *r, int argc, char **argv);
int ssi_get_ipv6_info(struct request_rec *r, int argc, char **argv);
int ssi_get_ipv6_localaddr(struct request_rec *r, int argc, char **argv);
int ssi_get_ipv6_prefixaddr(struct request_rec *r, int argc, char **argv);
int ssi_get_sipp_status(struct request_rec *r, int argc, char **argv);
int ssi_get_sipp_call_log(struct request_rec *r, int argc, char **argv);
int ssi_get_sipp_call_cnt(struct request_rec *r, int argc, char **argv);
int ssi_ABS_MAP(struct request_rec *r, int argc, char **argv);
int ssi_ABS_ARR(struct request_rec *r, int argc, char **argv);
int ssi_ABS_GET(struct request_rec *r, int argc, char **argv);
int ssi_ABS_DFT(struct request_rec *r, int argc, char **argv);
int ssi_ABS_TMP(struct request_rec *r, int argc, char **argv);
#if defined(VR7516RW22) || defined(WE410443)
int ssi_get_cur_channel(struct request_rec *r, int argc, char **argv);
#endif


void do_send_file(char *name, struct request_rec *r);
int checkStringToHTML(char *pstrS, char *pstrD);
/* Httoken */
extern int ssi_httoken_list_output(struct request_rec *r, int argc, char **argv);
extern int ssi_httoken_output(struct request_rec *r, int argc, char **argv);
#endif	/* __SSI_HANDLER_H__ */
