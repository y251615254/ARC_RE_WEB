#define RTF_UP 0x0001
#define SINGLE_PORT_FOWWARD_COUNT 20
#define TRIGGER_PORT_COUNT 10

int ssi_dump_route_table(struct request_rec *r, int argc, char **argv);
int ssi_http_pws(struct request_rec *r, int argc, char **argv);
int ssi_LoginStatus(struct request_rec *r, int argc, char **argv);
int ssi_get_mask_dns_gateway_status(struct request_rec *r, int argc, char **argv);
int ssi_get_backup_name(struct request_rec *r, int argc, char **argv);
int ssi_wl_asso_table(struct request_rec *r, int argc, char **argv);
int ssi_wl_inused_channel(struct request_rec *r, int argc, char **argv);
int ssi_get_lan_status(struct request_rec *r, int argc, char **argv);
int ssi_dump_wl_asso_table(struct request_rec *r, int argc, char **argv);
int ssi_get_system_uptime(struct request_rec *r, int argc, char **argv);
int ssi_get_lan_statistics(struct request_rec *r, int argc, char **argv);
int ssi_get_mem_cpu_info(struct request_rec *r, int argc, char **argv);
int ssi_dump_new_dhcp_leased(struct request_rec *r, int argc, char **argv);
int ssi_dump_ap_scan(struct request_rec *r, int argc, char **argv);
int ssi_dump_ap_scan5g(struct request_rec *r, int argc, char **argv);
int ssi_get_ssid(struct request_rec *r, int argc, char **argv);
int ssi_get_cur_WSC_Status(struct request_rec *r, int argc, char **argv);
int ssi_get_bridge_status(struct request_rec *r, int argc, char **argv);
int ssi_get_bridge_status_5g(struct request_rec *r, int argc, char **argv);
int ssi_dump_cloud_info(struct request_rec *r, int argc, char **argv);
int ssi_dump_toplogy_map_info(struct request_rec *r, int argc, char **argv);
int ssi_dump_toplogy_station_info(struct request_rec *r, int argc, char **argv);
int ssi_dump_toplogy_map_info_and_led(struct request_rec *r, int argc, char **argv);
int ssi_dump_toplogy_led_status_info(struct request_rec *r, int argc, char **argv);
int ssi_dump_FMWLoad_info(struct request_rec *r, int argc, char **argv);
int ssi_dump_autofw_info(struct request_rec *r, int argc, char **argv);
int ssi_dump_optimising_info(struct request_rec *r, int argc, char **argv);
int ssi_dump_fwstatus_info(struct request_rec *r, int argc, char **argv);
int ssi_dump_setpwd_result(struct request_rec *r, int argc, char **argv);
int ssi_syslog_save(struct request_rec *r, int argc, char **argv);
int ssi_get_tmp_value(struct request_rec *r, int argc, char **argv);
int ssi_get_upgrade_schedule(struct request_rec *r, int argc, char **argv);
int ssi_get_access_control_profile_info(struct request_rec *r, int argc, char **argv);
int ssi_get_access_control_block_mac_info(struct request_rec *r, int argc, char **argv);
int ssi_get_internet_control_status_info(struct request_rec *r, int argc, char **argv);
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