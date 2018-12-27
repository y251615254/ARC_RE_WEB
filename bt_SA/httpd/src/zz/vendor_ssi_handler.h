#define RTF_UP 0x0001
#define SINGLE_PORT_FOWWARD_COUNT 20
#define TRIGGER_PORT_COUNT 10

int ssi_dump_route_table(struct request_rec *r, int argc, char **argv);
int ssi_http_pws(struct request_rec *r, int argc, char **argv);
int ssi_LoginStatus(struct request_rec *r, int argc, char **argv);
int ssi_get_backup_name(struct request_rec *r, int argc, char **argv);
int ssi_wl_asso_table(struct request_rec *r, int argc, char **argv);
int ssi_wl_inused_channel(struct request_rec *r, int argc, char **argv);
int ssi_get_disk_info_files(struct request_rec *r, int argc, char **argv);
int ssi_get_file_info(struct request_rec *r, int argc, char **argv);
int ssi_get_lan_status(struct request_rec *r, int argc, char **argv);
int ssi_get_dsl_status(struct request_rec *r, int argc, char **argv);
int ssi_get_disk_info(struct request_rec *r, int argc, char **argv);
int ssi_dump_wl_asso_table(struct request_rec *r, int argc, char **argv);
int ssi_get_system_uptime(struct request_rec *r, int argc, char **argv);
int ssi_get_lan_statistics(struct request_rec *r, int argc, char **argv);
int ssi_get_mem_cpu_info(struct request_rec *r, int argc, char **argv);
int ssi_dump_new_dhcp_leased(struct request_rec *r, int argc, char **argv);
int ssi_dump_ap_scan(struct request_rec *r, int argc, char **argv);
int ssi_dump_ap_scan5g(struct request_rec *r, int argc, char **argv);
int ssi_get_ssid(struct request_rec *r, int argc, char **argv);
/* VOIP */
extern int ssi_get_sip_register_status(struct request_rec *r, int argc, char **argv);
extern int ssi_get_sip_account_info(struct request_rec *r, int argc, char **argv);
int ssi_get_VoIP_call_logs(struct request_rec *r, int argc, char **argv);
/* LTE */
extern int ssi_get_lte_status(struct request_rec *r, int argc, char **argv);
extern int ssi_get_lte_strength(struct request_rec *r, int argc, char **argv);
extern int ssi_get_lte_operator(struct request_rec *r, int argc, char **argv);
extern int ssi_get_sim_imsi(struct request_rec *r, int argc, char **argv);
extern int ssi_get_sim_pin_status(struct request_rec *r, int argc, char **argv);
extern int ssi_get_umts_info(struct request_rec *r, int argc, char **argv);
