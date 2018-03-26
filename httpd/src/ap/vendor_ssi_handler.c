#include "httpd.h"
#include "utils.h"
#include "md5.h"
#include "ssi_handler.h"
#include "vendor_ssi_handler.h"
#include "arc_proj_table.h"
#include "arc_def_maxnum.h"
#include <arca-global.h>

extern int ssi_voip_get_register_status(struct request_rec *r, int argc, char **argv);

#define SYSLOG_TMP "/tmp/sys_log.log"
#define SYSLOG_BUF_SIZE 1023

#define STORAGE_MOUNT_ROOT_PATH_SLASH "/tmp/media/"

static int ssi_syslog(struct request_rec *r, int argc, char **argv);
static int ssi_sysautolog(struct request_rec *r, int argc, char **argv);

#if defined(SYSLOG_ENHANCED)
static int ssi_owl_syslog(struct request_rec *r, int argc, char **argv);
#endif
static int ssi_image_token(struct request_rec *r, int argc, char **argv);


extern int fmt_args(int argc, char **argv, char *fmt, ...);

/* Vendor specified handlers.
 */
struct ssi_handler vendor_handlers[] = {

	/* The following are only for arcadyan. */
	{ "dump_route_table", ssi_dump_route_table },
	{ "http_pws", ssi_http_pws },
	{ "syslog", ssi_syslog }, //Sync from belkin project.
	{ "sysautolog", ssi_sysautolog }, //Sync from belkin project.
	{ "syslog_save", ssi_syslog_save },
#if defined(SYSLOG_ENHANCED)
	{"dump_owlsyslog_info", ssi_owl_syslog },
#endif
	{ "LoginStatus", ssi_LoginStatus },
	{ "get_mask_dns_gateway_status", ssi_get_mask_dns_gateway_status },
	{ "get_backup_name", ssi_get_backup_name },
	{ "wl_asso_table", ssi_wl_asso_table },
	{ "wl_inused_channel", ssi_wl_inused_channel },
	{ "get_lan_status", ssi_get_lan_status },
	{ "dump_wl_asso_table", ssi_dump_wl_asso_table },
	{ "get_system_uptime", ssi_get_system_uptime },
	{ "get_lan_statistics", ssi_get_lan_statistics },
	{ "get_mem_cpu_info", ssi_get_mem_cpu_info },
	{ "dump_new_dhcp_leased", ssi_dump_new_dhcp_leased },
	{ "dump_ap_scan", ssi_dump_ap_scan },
	{ "dump_ap_scan5g", ssi_dump_ap_scan5g },
	{ "get_cur_WSC_Status", ssi_get_cur_WSC_Status },
	{ "get_ssid", ssi_get_ssid },
	{ "get_bridge_status", ssi_get_bridge_status},
	{ "get_bridge_status_5g", ssi_get_bridge_status_5g},
	{ "dump_cloud_info", ssi_dump_cloud_info },
	{ "dump_toplogy_map_info", ssi_dump_toplogy_map_info },
	{ "dump_toplogy_station_info", ssi_dump_toplogy_station_info },
	{ "dump_toplogy_map_info_and_led", ssi_dump_toplogy_map_info_and_led},
	{ "dump_toplogy_led_status_info", ssi_dump_toplogy_led_status_info},
	{ "dump_FMWLoad_info", ssi_dump_FMWLoad_info},
	{ "dump_autofw_info", ssi_dump_autofw_info},
	{ "dump_optimising_info", ssi_dump_optimising_info},
	{ "dump_fwstatus_info", ssi_dump_fwstatus_info},
	{ "dump_setpwd_result", ssi_dump_setpwd_result},	
	{ "get_tmp_value", ssi_get_tmp_value},		
	{ "get_upgrade_schedule", ssi_get_upgrade_schedule},	
	{ "get_access_control_profile_info", ssi_get_access_control_profile_info},	
	{ "get_access_control_block_mac_info", ssi_get_access_control_block_mac_info},
	{ "get_internet_control_status_info", ssi_get_internet_control_status_info},


/* common parts*/
	{ "IMG_TOKEN", ssi_image_token },

	{ NULL, NULL }
};

extern int get_tftp_per(int id);
extern int get_need_fwupdate(int id);
extern int temp_upgrade;
static int time1=0;
extern int get_start_upload_fw(void);
int ssi_dump_FMWLoad_info(struct request_rec *r, int argc, char **argv)
{	
	int id;
//	cprintf("ssi_dump_FMWLoad_in~~~\n");
	temp_upgrade = 0;
	so_printf(r,"{ \"Status\": '");
	for(id=0; id < 8; id++)
	{	
		if(id != 0)
			so_printf(r,";");
		//cprintf("g_aplist_info[id].need_tftp:%d\n", g_aplist_info[id].need_tftp);
		so_printf(r,"%d", get_tftp_per(id));
	}
	so_printf(r,"',\r\n");
	so_printf(r,"\"FWupdate\": '");
	for(id=0; id < 8; id++)
	{	
		if(id != 0)
			so_printf(r,";");
		so_printf(r,"%d",get_need_fwupdate(id));	//g_aplist_info[id].tftp_per
	}
	so_printf(r,"',\r\n");
	so_printf(r,"\"LoadingStatus\": '%d'", get_start_upload_fw());
	so_printf(r,"}");	
	return 0;
}

struct ap_info{
	char version[64];
	char ip[32];
	char mac[32];
};

int get_tply_ap_info(struct ap_info ap_info[])
{
	char recvline[10240]={0};  //same size  as owl
	char buf_cmd[1024];
	int num = 0;
	int i = 0;
	strcpy(buf_cmd, "GET /status/tplg HTTP/1.1\r\nContent-Type:application/json\r\nContent-Length:0\r\nConnection:Keep-Alive\r\nAccept-Encoding:gzip\r\n\r\n");
	//cprintf("buf_cmd:%s\n", buf_cmd);
	if(get_message_from_owl(buf_cmd, recvline , 10240) >= 0) 
	{
		char *p;
		char jasonbuf[512];
		char tm_buf[512];
		char tm_buf1[64];
		memset(jasonbuf, 0, 512);
		memset(tm_buf, 0, 512);
		//cprintf("recvline:%s\n", recvline);
		p = recvline;
		num = 0;
		while((p = strstr(p, "fw_ver")) != NULL)
		{
			memset(&ap_info[num], 0, sizeof(struct ap_info));
			i = 0;
			p = p + strlen("fw_ver\": \"");
			cprintf("*p:%c\n", *p);
			while(*p != 34)
			{
				ap_info[num].version[i] = *p;
				p++;
				i ++;
				cprintf("%c", *p);
			}
			ap_info[num].version[i] = 0;
			p = strstr(p, "device_mac");
			p = p + strlen("device_mac\": \"");
			cprintf("\n*p:%c", *p);
			i = 0;
			while(*p != 34)
			{
				ap_info[num].mac[i] = *p;
				p++;
				i ++;
				cprintf("%c", *p);
			}
			ap_info[num].mac[i] = 0;
			p = strstr(p, "device_ip");
			p = p + strlen("device_ip\": \"");
			cprintf("\n*p:%c", *p);
			i = 0;
			while(*p != 34)
			{
				ap_info[num].ip[i] = *p;
				p++;
				i ++;
				cprintf("%c", *p);
			}
			ap_info[num].ip[i] = 0;
			//cprintf("ap_info[%d]:%s,%s,%s\n",num, ap_info[num].version, ap_info[num].mac, ap_info[num].ip);
			num ++;
			if(num >= 8)
				break;
		}	
	}
	return num;
}

int cmp_fw_version(char *local_version, char *remote_version)
{
	int local_v1 = 0;
	int local_v2 = 0;
	int local_v3 = 0;
	int remote_v1 = 0;
	int remote_v2 = 0;
	int remote_v3 = 0;	
	int ret = 0;
	
	if((local_version == NULL) || (remote_version == NULL))
		return 0;
	
	cprintf("local_version:%s,remote_version:%s\n", local_version, remote_version);
#ifdef WE410443_TA	
	if((*(local_version+1) == '.') && (*(local_version+4) == '.') && (*(remote_version+1) == '.') && (*(remote_version+4) == '.'))
	{
		if(strlen(remote_version) == 11)
		{
			if((*(remote_version+10) == 'd') || (*(remote_version+10) == 't'))
				ret = 1;
			else
				ret = 0;
		}
		else if((strlen(local_version) >= 10) && (strlen(remote_version) == 10))
		{
			//format "1.01.11-03"
			local_v1 = atoi(local_version+2);
			local_v2 = atoi(local_version+5);
			local_v3 = atoi(local_version+8);
			remote_v1 = atoi(remote_version+2);
			remote_v2 = atoi(remote_version+5);
			remote_v3 = atoi(remote_version+8);
			if((local_v1*10000+local_v2*100+local_v3) < (remote_v1*10000+remote_v2*100+remote_v3))
				ret = 1;
			else if((local_v1*10000+local_v2*100+local_v3) == (remote_v1*10000+remote_v2*100+remote_v3))
			{
				if((strlen(local_version) == 10))
					ret = 0;
				else
					ret = 1;
			}
			else
				ret = 1;		
		}
		else
			ret = 0;			
	}
	else
		ret = 0;
#else
	if((*local_version == 'v') && (*(local_version+2) == '.') && (*(local_version+5) == '.') && (*(local_version+9) == 'b') 
		&& (*(local_version+10) == 'u') && (*(local_version+11) == 'i') && (*(local_version+12) == 'l') && (*(local_version+13) == 'd')
		&& (*remote_version == 'v') && (*(remote_version+2) == '.') && (*(remote_version+5) == '.') && (*(remote_version+9) == 'b') 
		&& (*(remote_version+10) == 'u') && (*(remote_version+11) == 'i') && (*(remote_version+12) == 'l') && (*(remote_version+13) == 'd'))
	{
		if(strlen(remote_version) == 17)
		{
			if((*(remote_version+16) == 's') || (*(remote_version+16) == 'd') || (*(remote_version+16) == 't') || (*(remote_version+16) == 'u') || (*(remote_version+16) == 'v'))
				ret = 1;
			else
				ret = 0;
		}
		else if((strlen(local_version) >= 16) && (strlen(remote_version) == 16))
		{
			//format "v1.01.11 build03"
			local_v1 = atoi(local_version+3);
			local_v2 = atoi(local_version+6);
			local_v3 = atoi(local_version+14);
			remote_v1 = atoi(remote_version+3);
			remote_v2 = atoi(remote_version+6);
			remote_v3 = atoi(remote_version+14);
			if((local_v1*10000+local_v2*100+local_v3) < (remote_v1*10000+remote_v2*100+remote_v3))
				ret = 1;
			else if((local_v1*10000+local_v2*100+local_v3) == (remote_v1*10000+remote_v2*100+remote_v3))
			{
				if((strlen(local_version) == 16))
					ret = 0;
				else
					ret = 1;
			}
			else
				ret = 0;		
		}
		else
			ret = 0;
	}
	else
		ret = 0;
#endif		
	return ret;
}

int cmp_tply_fw_version(char *remote_version)
{
	int num = 0;
	int i = 0;
	struct ap_info g_ap_info[8];
	num = get_tply_ap_info(g_ap_info);
	int ret = 0;
	for(i=0; i < num; i++)
	{
		cprintf("g_ap_info[%d]:%s,%s,%s\n",i, g_ap_info[i].version, g_ap_info[i].mac, g_ap_info[i].ip);
		if(cmp_fw_version(&g_ap_info[i].version, remote_version) == 1)
		{
			ret = 1;
			break;
		}
	}	
	cprintf("ret = %d\n", ret);
	return ret;
}

int ssi_dump_autofw_info(struct request_rec *r, int argc, char **argv)
{	
	int id;
	char buf[256];
	int tid;
	char fwmainversion[16] = {0}, fwsubversion[16] = {0}, fwversion[32] = {0};
//	cprintf("ssi_dump_FMWLoad_in~~~\n");
	if((tid = get_tid()) == MID_FAIL)
		return 0;
	mapi_tmp_get(tid, "ARC_SYSTEM_AUTOFW_SOFTVERSION", buf, sizeof(buf));
	mapi_ccfg_get_str(tid, "ARC_SYS_FWVersion", fwmainversion, sizeof(fwmainversion));
	mapi_ccfg_get_str(tid, "ARC_SYS_FWSubVersion", fwsubversion, sizeof(fwsubversion));
#ifdef WE410443_TA
	sprintf(fwversion, "%s-%s", fwmainversion, fwsubversion);
#else
	sprintf(fwversion, "%s %s", fwmainversion, fwsubversion);
#endif	
	if(strlen(buf) != 0)
	{
		so_printf(r,"{\"nowait\": '%d',\r\n", 0);	
		if(strcmp(buf, "err") != 0)
		{
			so_printf(r,"\"version\": '%s',\r\n", buf);
			if(cmp_tply_fw_version(buf) > 0)
				so_printf(r,"\"available\": '%d'", 1);
			else
				so_printf(r,"\"available\": '%d'", 0);
#ifdef SYSLOG_ENHANCED
			SetSystemLogFmt(LOG_TYPE_APP, LOG_LEVEL_TYPE_INFO, LOG_APP_MESSAGE_TYPE_http, "HTTP-Find new Firmware %s from auto upgrade server.", buf);
#endif
		}
		else
		{
			so_printf(r,"\"version\": '%s',\r\n", "");
			so_printf(r,"\"available\": '%d'", 0);
#ifdef SYSLOG_ENHANCED
			SetSystemLogFmt(LOG_TYPE_APP, LOG_LEVEL_TYPE_ERROR, LOG_APP_MESSAGE_TYPE_http, "HTTP-Can't connect to auto upgrade server.");
#endif
		}
		so_printf(r,"}");
		
	}
	else
	{
		so_printf(r,"{\"nowait\": '%d',\r\n", 1);	
		so_printf(r,"\"version\": '%s',\r\n", "");
		so_printf(r,"\"available\": '%d'", 0);
		so_printf(r,"}");
	}
	return 0;
}

int optimising_flag=0;
int ssi_dump_optimising_info(struct request_rec *r, int argc, char **argv ){
	FILE *fp;	
	char buffer[2];
	fp = fopen("/tmp/disc_optimising", "r");

	if( fp == NULL ){
		cprintf("disc_optimising File no conneted or open error \n");
		so_printf(r,"0");
		return 0;
	}

	if(fgets(buffer, sizeof(buffer), fp)==NULL )
		{
		cprintf("disc_optimising File no conneted or read error \n");
		so_printf(r,"0");
		fclose(fp);
		return 0;
		}

	cprintf("get ssi_dump_optimising_info buffer !!!!!!!!!!!%s\n",buffer);
	so_printf(r,buffer);
	fclose(fp);
	system("rm \/tmp\/disc_optimising");
	system("killall -9 udpsvd");

	return 1;
}
int ssi_dump_setpwd_result(struct request_rec *r, int argc, char **argv)
{	
	int tid;
	char nowait[32] = {0};
	char result[32] = {0};

	if((tid = get_tid()) == MID_FAIL)
		return 0;
	mapi_tmp_get(tid, "ARC_SYSTEM_SETPWD_NOWAIT", nowait, sizeof(nowait));
	mapi_tmp_get(tid, "ARC_SYSTEM_SETPWD_RESULT", result, sizeof(result));

	if((strlen(nowait) != 0) && (strlen(result) != 0))
	{
		if(atoi(nowait) == 1)
			so_printf(r,"{\"nowait\": '%d',\r\n", 1);
		else
			so_printf(r,"{\"nowait\": '%d',\r\n", 0);
		
		so_printf(r,"\"result\": '%d'", atoi(result));
		
		so_printf(r,"}");
		
	}
	else
	{
		so_printf(r,"{\"nowait\": '%d',\r\n", 1);	
		so_printf(r,"\"result\": '%d'", 0);
		so_printf(r,"}");
	}
	return 0;
}
int ssi_dump_fwstatus_info(struct request_rec *r, int argc, char **argv)
{	
	int id;
	FILE *fifo = NULL;
	char buf[256];
	char *p, *p1;
//	cprintf("ssi_dump_FMWLoad_in~~~\n");
	fifo = fopen("/tmp/fwupgrade_err.txt", "r");
	if(fifo)
	{
		safe_fread(buf, 1, 256, fifo);	
		p = buf;
		so_printf(r,"{");
		if(p)
		{
			p = p;
			p1 = strstr(p, "\n");
			*p1 = 0;
#ifdef SYSLOG_ENHANCED
			int fwsta = atoi(p);
			switch(fwsta)
			{
				case 1:
					SetSystemLogFmt(LOG_TYPE_APP, LOG_LEVEL_TYPE_ERROR, LOG_APP_MESSAGE_TYPE_http, "HTTP-The FW version is error from %s.", r->remote_ipstr);
					break;
				case 2:
					SetSystemLogFmt(LOG_TYPE_APP, LOG_LEVEL_TYPE_ERROR, LOG_APP_MESSAGE_TYPE_http, "HTTP-The FW CRC is error from %s.", r->remote_ipstr);
					break;
				case 4:
					SetSystemLogFmt(LOG_TYPE_APP, LOG_LEVEL_TYPE_ERROR, LOG_APP_MESSAGE_TYPE_http, "HTTP-The FW upgrade error from %s.", r->remote_ipstr);
					break;
				case 5:
					SetSystemLogFmt(LOG_TYPE_APP, LOG_LEVEL_TYPE_ERROR, LOG_APP_MESSAGE_TYPE_http, "HTTP-Can't connect to auto upgrade server.");
					break;
				default:
					break;
			}
#endif
			so_printf(r,"\"result\": '%s'\r\n", p);	
		}
		so_printf(r,"}");
		fclose(fifo);
	}
	else
	{
		so_printf(r,"{}");
	}
	return 0;
}
int ssi_dump_cloud_info(struct request_rec *r, int argc, char **argv)
{	
	int i, totalLen = 0;
	char *p=NULL;
	char recvline[102400]={0};  //same size  as owl
	char soprintfbuff[SOPRINTFBUFSIZE+1]={0};
	char *buf ="GET /tools/Cloud_Server_URL HTTP/1.1\r\nContent-Type:application/json\r\nContent-Length:0\r\nConnection:Keep-Alive\r\nAccept-Encoding:gzip\r\n\r\n";
	totalLen = get_message_from_owl(buf, recvline, sizeof(recvline));

	if(totalLen > 0)
	{
		p = strchr(recvline,'{');
		p[strlen(p)]='\0';
		for(i=strlen(p);i>0;i-=SOPRINTFBUFSIZE)  //data length >  SOPRINTFBUFSIZE
		{	
			memset(soprintfbuff,0,sizeof(char)*(SOPRINTFBUFSIZE+1));
			strncpy(soprintfbuff,p,SOPRINTFBUFSIZE);
			soprintfbuff[strlen(soprintfbuff)]='\0';
			p = p + SOPRINTFBUFSIZE;
			so_printf(r,"%s",soprintfbuff);
		}
	}
	else
	{
		so_printf(r,"{}");
	}
	return 0;
}


int ssi_dump_toplogy_station_info(struct request_rec *r, int argc, char **argv)
{	
	int i, totalLen = 0;
	char *p=NULL;
	char recvline[102400]={0};  //same size  as owl
	char soprintfbuff[SOPRINTFBUFSIZE+1]={0};
	char *buf ="GET /status/station HTTP/1.1\r\nContent-Type:application/json\r\nContent-Length:0\r\nConnection:Keep-Alive\r\nAccept-Encoding:gzip\r\n\r\n";
	totalLen = get_message_from_owl(buf, recvline, sizeof(recvline));
	if(totalLen > 0)
	{
		p = strchr(recvline,'{');
		if(p == NULL)
		{
			so_printf(r,"{ \"stations\": [ ] }");
			goto out;
		}			
		p[strlen(p)]='\0';		
		for(i=strlen(p);i>0;i-=SOPRINTFBUFSIZE)  //data length >  SOPRINTFBUFSIZE
		{	
			memset(soprintfbuff,0,sizeof(char)*(SOPRINTFBUFSIZE+1));
			strncpy(soprintfbuff,p,SOPRINTFBUFSIZE);
			soprintfbuff[strlen(soprintfbuff)]='\0';
			p = p + SOPRINTFBUFSIZE;
			so_printf(r,"%s",soprintfbuff);
		}
	}
	else
	{
		so_printf(r,"{ \"stations\": [ ] }");
	}
out:	
	return 0;
}


int ssi_dump_toplogy_led_status_info(struct request_rec *r, int argc, char **argv)
{	
	struct sockaddr_in RemoteAddr;
	int sock_fd,n,i;
	char *p=NULL;
	char recvline[10240]={0};  //same size  as owl	
	char led_send_buf[512]={0};	
	char soprintfbuff[SOPRINTFBUFSIZE+1]={0};
	char *buf ="GET /status/tplg HTTP/1.1\r\nContent-Type:application/json\r\nContent-Length:0\r\nConnection:Keep-Alive\r\nAccept-Encoding:gzip\r\n\r\n";
	sock_fd = socket(AF_INET, SOCK_STREAM, 0);
	memset((char *)&RemoteAddr, 0, sizeof(RemoteAddr));
	RemoteAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	RemoteAddr.sin_port = htons (OWL_LISTEN_PORT);
	RemoteAddr.sin_family = AF_INET;
	if(connect(sock_fd, (struct sockaddr *)&RemoteAddr, sizeof(RemoteAddr)) < 0)
	{
		cprintf("connect error!\n");
		close(sock_fd);
		so_printf(r,"{ \"nodes\": [ ] }");
		return -1;
	}
	write(sock_fd, buf, strlen(buf));
	if(read(sock_fd, recvline,sizeof(recvline)) > 0)
	{
		char buf_cmd[512], jasonbuf[256], buf_recv[512];
		char mac[32], color[16], led_tmp[32], brightness_value[16];
		char *pt, *pc, *pled;
		
		p = recvline;		
		strcat(led_send_buf,"\"");
		while((p = strstr(p, "device_mac")) != NULL)
		{
			memset(buf_recv , 0, 512);
			p = p + strlen("device_mac\": \"");
			pt = strstr(p, "\"");
			memset(mac , 0, 32);			
			memset(led_tmp , 0, 32);

			strncpy(mac, p, pt - p);
			//cprintf("pt:%s\n", pt);
			//cprintf("p:%s\n", p);
			sprintf(jasonbuf,"{\"device_mac\":\"%s\"}", mac);
			sprintf(buf_cmd,"POST /tools/diag/getLED HTTP/1.1\r\nContent-Type:application/json\r\nContent-Length:%d\r\nConnection:Keep-Alive\r\n\r\n%s",strlen(jasonbuf),jasonbuf);
			//cprintf("[###DEBUG_LED###]==%s_%d==> buf_cmd:%s\n",  __FUNCTION__, __LINE__,buf_cmd);
			write(sock_fd, buf_cmd, strlen(buf_cmd));
			read(sock_fd, buf_recv,sizeof(buf_recv));
			//cprintf("[###DEBUG_LED###]==%s_%d==> buf_recv:%s\n",  __FUNCTION__, __LINE__,buf_recv);


			pled = buf_recv;
			pled = strstr(pled, "brightness");
			//cprintf("brightness_value:%c\n", pled[14]);
			strcpy(led_tmp, mac);
			//cprintf("led_tmp:%s\n", led_tmp);
			sprintf(brightness_value, "@%c,", pled[14]);
			strcat(led_tmp,brightness_value);
			//cprintf("led_tmp:%s\n", led_tmp);
			strcat(led_send_buf,led_tmp);
		}

		//cprintf("len:%d	buf_recv:%c\n", strlen(led_send_buf),led_send_buf[strlen(led_send_buf)-1]);
		led_send_buf[strlen(led_send_buf)-1] = '\0';
		strcat(led_send_buf,"\"");
		//cprintf("led_send_buf:%s\n", led_send_buf);
		so_printf(r,"%s",led_send_buf);
	}
	else
	{
		so_printf(r,"{ \"nodes\": [ ] }");
	}
	close(sock_fd);
	return 0;
}


int ssi_dump_toplogy_map_info(struct request_rec *r, int argc, char **argv)
{	
	struct sockaddr_in RemoteAddr;
	int sock_fd,n,i;
	char *p=NULL;
	char recvline[10240]={0};  //same size  as owl
	char soprintfbuff[SOPRINTFBUFSIZE+1]={0};
	char *buf ="GET /status/tplg HTTP/1.1\r\nContent-Type:application/json\r\nContent-Length:0\r\nConnection:Keep-Alive\r\nAccept-Encoding:gzip\r\n\r\n";
	sock_fd = socket(AF_INET, SOCK_STREAM, 0);
	memset((char *)&RemoteAddr, 0, sizeof(RemoteAddr));
	RemoteAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	RemoteAddr.sin_port = htons (OWL_LISTEN_PORT);
	RemoteAddr.sin_family = AF_INET;
	if(connect(sock_fd, (struct sockaddr *)&RemoteAddr, sizeof(RemoteAddr)) < 0)
	{
		cprintf("connect error!\n");
		close(sock_fd);
		so_printf(r,"{ \"nodes\": [ ] }");
		return -1;
	}
	write(sock_fd, buf, strlen(buf));
	if(read(sock_fd, recvline,sizeof(recvline)) > 0)
	{
		p = strchr(recvline,'{');
		if(p == NULL)
		{
			so_printf(r,"{ \"nodes\": [ ] }");
			goto out;
		}	
		p[strlen(p)]='\0';
		//cprintf("%s",p);
		for(i=strlen(p);i>0;i-=SOPRINTFBUFSIZE)  //data length >  SOPRINTFBUFSIZE
		{	
			memset(soprintfbuff,0,sizeof(char)*(SOPRINTFBUFSIZE+1));
			strncpy(soprintfbuff,p,SOPRINTFBUFSIZE);
			soprintfbuff[strlen(soprintfbuff)]='\0';
			p = p + SOPRINTFBUFSIZE;
			so_printf(r,"%s",soprintfbuff);
		}
	}
	else
	{
		so_printf(r,"{ \"nodes\": [ ] }");
	}
out:	
	close(sock_fd);
	return 0;
}

int ssi_dump_toplogy_map_info_and_led(struct request_rec *r, int argc, char **argv)
{	
	struct sockaddr_in RemoteAddr;
	int sock_fd,n,i;
	char *p=NULL;
	char recvline[10240]={0};  //same size  as owl
	char soprintfbuff[SOPRINTFBUFSIZE+1]={0};
	char *buf ="GET /status/tplg HTTP/1.1\r\nContent-Type:application/json\r\nContent-Length:0\r\nConnection:Keep-Alive\r\nAccept-Encoding:gzip\r\n\r\n";
	sock_fd = socket(AF_INET, SOCK_STREAM, 0);
	memset((char *)&RemoteAddr, 0, sizeof(RemoteAddr));
	RemoteAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	RemoteAddr.sin_port = htons (OWL_LISTEN_PORT);
	RemoteAddr.sin_family = AF_INET;
	if(connect(sock_fd, (struct sockaddr *)&RemoteAddr, sizeof(RemoteAddr)) < 0)
	{
		cprintf("connect error!\n");
		close(sock_fd);
		so_printf(r,"{ \"nodes\": [ ] }");
		return -1;
	}
	write(sock_fd, buf, strlen(buf));
	if(read(sock_fd, recvline,sizeof(recvline)) > 0)
	{
		char buf_cmd[512], jasonbuf[256], buf_recv[512];
		char mac[32], color[16];
		char *pt, *pc;
		
		p = recvline;
		while((p = strstr(p, "device_mac")) != NULL)
		{
			memset(buf_recv , 0, 512);
			p = p + strlen("device_mac\": \"");
			pt = strstr(p, "\"");
			memset(mac , 0, 32);
			strncpy(mac, p, pt - p);
			//cprintf("pt:%s\n", pt);
			//cprintf("p:%s\n", p);
			sprintf(jasonbuf,"{\"device_mac\":\"%s\"}", mac);
			sprintf(buf_cmd,"POST /tools/diag/getLED HTTP/1.1\r\nContent-Type:application/json\r\nContent-Length:%d\r\nConnection:Keep-Alive\r\n\r\n%s",strlen(jasonbuf),jasonbuf);
			//cprintf("buf_cmd:%s\n", buf_cmd);
			write(sock_fd, buf_cmd, strlen(buf_cmd));
			read(sock_fd, buf_recv,sizeof(buf_recv));
			//cprintf("buf_recv:%s\n", buf_recv);
			pc = buf_recv;
			pc = strstr(pc, "color");
			pc = pc + strlen("color\": \"");
			pt = strstr(pc, "\"");
			memset(color , 0, 16);
			strncpy(color, pc, pt - pc);
			//cprintf("color:%s\n", color);
			p = strstr(p, "connect_rssi");
			p = p + strlen("connect_rssi\": \"");
			i = 0;
		//	while(*p != 34)
		//	{
				if(color[i] != 0)
					*p = color[i];
		//		else
		//			*p = '*';
		//		p ++;
		//	}
		}
		//cprintf("recvline:%s\n", recvline);
		p = strchr(recvline,'{');
		if(p == NULL)
		{
			so_printf(r,"{ \"nodes\": [ ] }");
			goto out;
		}			
		p[strlen(p)]='\0';
		//cprintf("%s",p);
		for(i=strlen(p);i>0;i-=SOPRINTFBUFSIZE)  //data length >  SOPRINTFBUFSIZE
		{	
			memset(soprintfbuff,0,sizeof(char)*(SOPRINTFBUFSIZE+1));
			strncpy(soprintfbuff,p,SOPRINTFBUFSIZE);
			soprintfbuff[strlen(soprintfbuff)]='\0';
			p = p + SOPRINTFBUFSIZE;
			so_printf(r,"%s",soprintfbuff);
		}
	}
	else
	{
		so_printf(r,"{ \"nodes\": [ ] }");
	}
out:	
	close(sock_fd);
	return 0;
}

int ssi_get_bridge_status(struct request_rec *r, int argc, char **argv)
{
	int nbytes = 0, tid;
	char ssid[50] = "", bssid[17+1] = "", secmode[20] = "", status[10] = "", tmp_ssid[50]="", final_ssid[250]="";
	char *c;
	if((tid = get_tid()) == MID_FAIL)
		return 0;

	mapi_tmp_get(tid, "ARC_WLAN_24G_APCLI_Status", status, sizeof(status));
	if(strcmp(status, "1"))
	{
		//"0", "extender ssid","bssid","open"
		nbytes = so_printf(r, "\"%s\", \"\", \"\"", status);

		return nbytes;
	}

	c = mapi_ccfg_get_str(tid, "ARC_WLAN_24G_APCLI_SSID_Customer", ssid, sizeof(ssid));
	mapi_ccfg_get_str(tid, "ARC_WLAN_24G_APCLI_BSSID_Customer", bssid, sizeof(bssid));
	mapi_ccfg_get_str(tid, "ARC_WLAN_24G_APCLI_SecurityType_Customer", secmode, sizeof(secmode));

	memset(final_ssid, 0, sizeof(final_ssid));
	for(;*c; c++)
	{
		memset(tmp_ssid, 0, sizeof(tmp_ssid));
		if(isprint((int) *c) && *c != '"' && *c != '&' && *c != '<' && *c != '>' && *c != '\'' && *c != '\\')
		{
			snprintf(tmp_ssid,sizeof(tmp_ssid),"%c", *c);
		}
		else
		{
			snprintf(tmp_ssid,sizeof(tmp_ssid),"&#%d;", *c);
		}
		strcat(final_ssid,tmp_ssid);
	}

	//if(!strcmp(secmode, "wpawpa2"))
	//{
	//	strcpy(secmode, "wpa2");
	//}

	//"0", "extender ssid", "open"
	nbytes = so_printf(r, "\"%s\", \"%s\", \"%s\", \"%s\"", status, final_ssid, bssid, secmode);
	//ht_dbg("\"%s\", \"%s\", \"%s\"", status, final_ssid, secmode);

	return nbytes;
}

int ssi_get_bridge_status_5g(struct request_rec *r, int argc, char **argv)
{
	int nbytes = 0, tid;
	char ssid[50] = "", bssid[17+1] = "", secmode[20] = "", status[10] = "", tmp_ssid[50]="", final_ssid[250]="";
	char *c;
	if((tid = get_tid()) == MID_FAIL)
		return 0;

	mapi_tmp_get(tid, "ARC_WLAN_5G_APCLI_Status", status, sizeof(status));
	if(strcmp(status, "1"))
	{
		//"0", "extender ssid","bssid","open"
		nbytes = so_printf(r, "\"%s\", \"\", \"\"", status);

		return nbytes;
	}

	c = mapi_ccfg_get_str(tid, "ARC_WLAN_5G_APCLI_SSID_Customer", ssid, sizeof(ssid));
	mapi_ccfg_get_str(tid, "ARC_WLAN_5G_APCLI_BSSID_Customer", bssid, sizeof(bssid));
	mapi_ccfg_get_str(tid, "ARC_WLAN_5G_APCLI_SecurityType_Customer", secmode, sizeof(secmode));

	memset(final_ssid, 0, sizeof(final_ssid));
	for(;*c; c++)
	{
		memset(tmp_ssid, 0, sizeof(tmp_ssid));
		if(isprint((int) *c) && *c != '"' && *c != '&' && *c != '<' && *c != '>' && *c != '\'' && *c != '\\')
		{
			snprintf(tmp_ssid,sizeof(tmp_ssid),"%c", *c);
		}
		else
		{
			snprintf(tmp_ssid,sizeof(tmp_ssid),"&#%d;", *c);
		}
		strcat(final_ssid,tmp_ssid);
	}
			
	//if(!strcmp(secmode, "wpawpa2"))
	//{
	//	strcpy(secmode, "wpa2");
	//}

	//"0", "extender ssid", "open"
	nbytes = so_printf(r, "\"%s\", \"%s\", \"%s\", \"%s\"", status, final_ssid, bssid, secmode);
	//ht_dbg("\"%s\", \"%s\", \"%s\"", status, final_ssid, secmode);

	return nbytes;
}



int ssi_get_access_control_block_mac_info(struct request_rec *r, int argc, char **argv)
{

		//char *filename = "/proc/br_externed_filter/ip";
		char *filename = "/proc/br_externed_filter/access_control_block_mac";
		char string[16];
		FILE *fp =NULL;
		int i;
		int id;
		FILE *fifo = NULL;
		char buf[5120];
		char *p, *p1;
		//cprintf("ssi_get_access_control_block_mac_info--------in--------------\n");
		//fifo = fopen("proc/br_externed_filter/access_control_block_mac", "r");
		//fifo = fopen("proc/br_externed_filter/ip", "r");
		fifo = fopen(filename, "r");

		if(fifo==NULL)
		{
			cprintf("open file access_control_block_mac failed!\n");
			exit(1);
		}
		else
		{
			//cprintf("open file access_control_block_mac succeed!\n");
		}

		
		if(fifo)
		{
			safe_fread(buf, 1, 5120, fifo);	
			p = buf;
			so_printf(r,"{");
			if(p)
			{
				p = p;
				p1 = strstr(p, "\n");
				*p1 = 0;
				so_printf(r,"\"result\": \"%s\"", p); 
			}
			so_printf(r,"}");
			//cprintf("ssi_get_access_control_block_mac_info------>%s\n", r);
			fclose(fifo);
		}
		else
		{
			so_printf(r,"{}");
		}
		return 0;

}


int ssi_get_internet_control_status_info(struct request_rec *r, int argc, char **argv)
{	
	struct sockaddr_in RemoteAddr;
	int sock_fd,n,i;
	char *p=NULL;
	char recvline[10240]={0};  //same size	as owl
	char soprintfbuff[SOPRINTFBUFSIZE+1]={0};
	char *buf ="GET /Tools/Control_Internet HTTP/1.1\r\nContent-Type:application/json\r\nContent-Length:0\r\nConnection:Keep-Alive\r\nAccept-Encoding:gzip\r\n\r\n";
	sock_fd = socket(AF_INET, SOCK_STREAM, 0);
	memset((char *)&RemoteAddr, 0, sizeof(RemoteAddr));
	RemoteAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	RemoteAddr.sin_port = htons (OWL_LISTEN_PORT);
	RemoteAddr.sin_family = AF_INET;
	if(connect(sock_fd, (struct sockaddr *)&RemoteAddr, sizeof(RemoteAddr)) < 0)
	{
		cprintf("connect error!\n");
		close(sock_fd);
		so_printf(r,"{ \"nodes\": [ ] }");
		return -1;
	}
	write(sock_fd, buf, strlen(buf));
	
	//cprintf("[%s_%d]--->buf[%s]\n\n\n", __FUNCTION__, __LINE__, buf);
	
	if(read(sock_fd, recvline,sizeof(recvline)) > 0)
	{
		//cprintf("[%s_%d]--->recvline:\n%s\n\n\n", __FUNCTION__, __LINE__, recvline);
		
		p = strchr(recvline,'{');
		if(p == NULL)
		{
			so_printf(r,"{ \"nodes\": [ ] }");
			goto out;
		}					
		p[strlen(p)]='\0';
		//cprintf("%s",p);
		for(i=strlen(p);i>0;i-=SOPRINTFBUFSIZE)  //data length >  SOPRINTFBUFSIZE
		{	
			memset(soprintfbuff,0,sizeof(char)*(SOPRINTFBUFSIZE+1));
			strncpy(soprintfbuff,p,SOPRINTFBUFSIZE);
			soprintfbuff[strlen(soprintfbuff)]='\0';
			p = p + SOPRINTFBUFSIZE;
			so_printf(r,"%s",soprintfbuff);
		}
	}
	else
	{
		so_printf(r,"{ \"nodes\": [ ] }");
	}
out:	
	close(sock_fd);
	return 0;
}



unsigned long int getCurWSCStatus(char *inf){
	char filename[20] = "/tmp/wlan_stats";
	char curwscstatus[8] = {0};
	char cmd[128];
	char string[128];
	FILE *fp;

	sprintf(cmd,"iwpriv %s stat > %s", inf, filename);
	system(cmd);

	fp = fopen(filename, "r");
	if( fp == NULL ){
		cprintf("File read error?\n");
		return 0;
	}

			
	while(fgets(string, sizeof(string), fp))
	{
		if(strstr(string, "WSC_Status"))
		{
			sscanf(string, "WSC_Status     = %s", curwscstatus);
			break;
		}
	}
	fclose(fp);

	sprintf(cmd,"rm -f %s",filename);
	system(cmd);

	return strtoul(curwscstatus,NULL, 10);
}

int ssi_get_cur_WSC_Status(struct request_rec *r, int argc, char **argv)
{

	unsigned long int wsc_status_24 = 0;
	unsigned long int wsc_status_5 = 0;
	int count=0;
	int tid;

	if((tid = get_tid()) == MID_FAIL)
		return 0;
	
	wsc_status_5 = getCurWSCStatus(WIFI_5G_AP0_NAME);
	so_printf(r, "'5G','%d'\n", wsc_status_5);


	wsc_status_24 = getCurWSCStatus(WIFI_24G_AP0_NAME);
	so_printf(r, ",'2.4G','%d'\n", wsc_status_24);

	
}




int ssi_dump_ap_scan(struct request_rec *r, int argc, char **argv)
{	
	FILE *fp = NULL;

	char filename5g[16] = "/tmp/ap_scan"; 
	char cmd[256] = {0};

	char buffer[256] = {0};
	int blank = 1;
	int count = 0;
	char *ptr;
	char Num[4], channel[4], ssid[186], bssid[20], security[23];
	char signal[9], mode[7], ext_ch[7], net_type[3], wps[4];
	int i, space_start;

	sprintf(cmd, "iwpriv apcli0 set SiteSurvey= ");
	system(cmd);

	sleep(1);	

	sprintf(cmd, "iwpriv apcli0 get_site_survey > %s", filename5g);
	system(cmd);

	sleep(1);	

	if ((fp = fopen(filename5g, "r")) == NULL)
	{
		so_printf(r, "%s", "No Ap scan table\n");
		return -1;

	}
	
	while( fgets(buffer, sizeof(buffer), fp) != NULL ) {			
			if (strlen(buffer) == 1)
			{
				fclose(fp);
				return 0;
			}
			if(count > 2)
			{				
			    ptr = buffer;
				/*4bytes No.+4bytes Channel+33bytes SSID+BSSID+...*/
				sscanf(ptr, "%s ", Num);
				ptr += 4;
				sscanf(ptr, "%s ", channel);
				ptr += 4;
				i = 0;
				while (i < 33) {
					if ((ptr[i] == 0x20) && (i == 0 || ptr[i-1] != 0x20))
						space_start = i;
					i++;
				}
				ptr[space_start] = '\0';
				strcpy(ssid, buffer+8);
				//TODO convert string to display in web
				ptr += 33;
				sscanf(ptr, "%s %s %s %s %s %s %s", bssid, security, signal, mode, ext_ch, net_type, wps);
				so_printf(r, "%c'%s %s %s %s %s %s %s %s %s#%s'\n", 
					blank ? ' ' : ',', Num,channel,wps,bssid,security,signal,mode,ext_ch,net_type,ssid);
				//cprintf("%c'%s %s %s %s %s %s %s %s %s %s'\n", 
				//	blank ? ' ' : ',', Num,channel,ssid,bssid,security,signal,mode,ext_ch,net_type,wps);
				blank = 0;
			}
			count++;
		}
	
	fclose(fp);
	
	return 0;		
}

int ssi_dump_ap_scan5g(struct request_rec *r, int argc, char **argv)
{	
	FILE *fp = NULL;

	char filename5g[16] = "/tmp/ap_scan5g"; 
	char cmd[256] = {0};

	char buffer[256] = {0};
	int blank = 1;
	int count = 0;
	char *ptr;
	char Num[4], channel[4], ssid[186], bssid[20], security[23];
	char signal[9], mode[7], ext_ch[7], net_type[3], wps[4];
	int i, space_start;

#ifdef DBDC_MODE
	sprintf(cmd, "iwpriv apcli1 set SiteSurvey= ");
	system(cmd);

	sleep(1);	

	sprintf(cmd, "iwpriv apcli1 get_site_survey > %s", filename5g);
	system(cmd);

	sleep(1);	
#else
	sprintf(cmd, "iwpriv apclii0 set SiteSurvey= ");
	system(cmd);

	sleep(1);	

	sprintf(cmd, "iwpriv apclii0 get_site_survey > %s", filename5g);
	system(cmd);

	sleep(1);
#endif

	if ((fp = fopen(filename5g, "r")) == NULL)
	{
		so_printf(r, "%s", "No Ap scan table\n");
		return -1;

	}
	
	while( fgets(buffer, sizeof(buffer), fp) != NULL ) {			
			if (strlen(buffer) == 1)
			{
				fclose(fp);
				return 0;
			}
			if(count > 2)
			{				
			    ptr = buffer;
				/*4bytes No.+4bytes Channel+33bytes SSID+BSSID+...*/
				sscanf(ptr, "%s ", Num);
				ptr += 4;
				sscanf(ptr, "%s ", channel);
				ptr += 4;
				i = 0;
				while (i < 33) {
					if ((ptr[i] == 0x20) && (i == 0 || ptr[i-1] != 0x20))
						space_start = i;
					i++;
				}
				ptr[space_start] = '\0';
				strcpy(ssid, buffer+8);
				//TODO convert string to display in web
				ptr += 33;
				sscanf(ptr, "%s %s %s %s %s %s %s", bssid, security, signal, mode, ext_ch, net_type, wps);
				so_printf(r, "%c'%s %s %s %s %s %s %s %s %s#%s'\n", 
					blank ? ' ' : ',', Num,channel,wps,bssid,security,signal,mode,ext_ch,net_type,ssid);
				//cprintf("%c'%s %s %s %s %s %s %s %s %s %s'\n", 
				//	blank ? ' ' : ',', Num,channel,ssid,bssid,security,signal,mode,ext_ch,net_type,wps);
				blank = 0;
			}
			count++;
		}
	
	fclose(fp);
	
	return 0;	
}

int ssi_dump_route_table(struct request_rec *r, int argc, char **argv)
{
	int ret = 0;
	int count = 0;
	char *format;
	FILE *fp;
	int flgs, ref, use, metric;
	unsigned long dest, gw, netmask;
	char line[256];
	struct in_addr dest_ip;
	struct in_addr gw_ip;
	struct in_addr netmask_ip;
	char sdest[16], sgw[16];
	int debug = 0, blank = 1;
	char iface[16];  //John add

	char wan_proto[24]={0}, wan_ifname[24] = {0};
	int tid;

	if((tid = get_tid()) == MID_FAIL)
		return -1;

	if (fmt_args(argc, argv, "%s", &format) < 1)
	{
		so_printf(r, "%s", "Insufficient args\n");
		return -1;
	}
	if ((fp = fopen("/proc/net/route", "r")) == NULL)
	{
		so_printf(r, "%s", "No route table\n");
		return -1;
	}

	while(fgets(line, sizeof(line), fp) != NULL )
	{
		if(count)
		{
			int ifl = 0;
			while(line[ifl]!=' ' && line[ifl]!='\t' && line[ifl]!='\0')
				ifl++;
			line[ifl]=0;	/* interface */
			if(sscanf(line+ifl+1, "%lx%lx%X%d%d%d%lx", &dest, &gw, &flgs, &ref, &use, &metric, &netmask)!=7)
			{
				break;
			}
			debug = 0;
			dest_ip.s_addr = dest;
			gw_ip.s_addr   = gw;
			netmask_ip.s_addr = netmask;

			strcpy(sdest,  (dest_ip.s_addr==0 ? "0.0.0.0" : inet_ntoa(dest_ip)));	 //default
			strcpy(sgw,    (gw_ip.s_addr==0   ? "0.0.0.0" : inet_ntoa(gw_ip))); 	 //*


			/* not 0x0001 route usable */
			if(!(flgs&RTF_UP))	continue;

			/* filter ppp pseudo interface for DOD */
			//if(!strcmp(sdest,PPP_PSEUDO_IP) && !strcmp(sgw,PPP_PSEUDO_GW))	debug = 1;

			/* Don't show loopback device */
			if(!strcmp(line,"lo"))	debug = 1;

			/* Don't show eth1 information for pppoe mode */

			mapi_ccfg_get_str(tid, "ARC_WAN_0_IP4_Proto", wan_proto, sizeof(wan_proto));
			mapi_ccfg_get_str(tid, "ARC_WAN_0_Ifname", wan_ifname, sizeof(wan_ifname));
			if(!strcmp(line, wan_ifname) && !strcmp(wan_proto,"pppoe")) debug = 1;

			if(!strcmp(line, "br0"))
			{
				snprintf(iface, sizeof(iface), "LAN");
			}
			else if(!strcmp(line, "lo"))
			{
				snprintf(iface, sizeof(iface), "Loopback");
			}
			else
			{
				snprintf(iface, sizeof(iface), "WAN");
			}

			//John zhu@2008.04.13 add the metric column while displaying  route
			ret += so_printf(r,"%s%c'%s','%s','%s','%d','%s'\n",
					debug ? "//" : "",
					blank ? ' ' : ',',
					sdest,
					inet_ntoa(netmask_ip),
					sgw,
					metric,
					iface
					/*(strcmp(line,"br0") ? "WAN" : "LAN")*/);

			if(debug && blank)	blank = 1;
			else		blank = 0;


		}

		count++;
	}

	return ret;
}

static int checkStringToJS(char *pstrS, char *pstrD)
{
	short i, j, str_length;
	char one_byte;

	if((!pstrS) || (!pstrD))
		return 0;

	str_length = strlen(pstrS);
	pstrD[0] = 0;

	j = 0;
	for (i=0; i<str_length; i++)
	{
		one_byte = pstrS[i];

		switch (one_byte) {
			case 0x22: // ", double quotes
				pstrD[j+0] = 0x5C;
				pstrD[j+1] = one_byte;
				j += 2;
				break;
			case 0x27: // ', single quotes
				pstrD[j+0] = 0x5C;
				pstrD[j+1] = one_byte;
				j += 2;
				break;
			case 0x5C: // \, backslash
				pstrD[j+0] = 0x5C;
				pstrD[j+1] = one_byte;
				j += 2;
				break;
			default:
				pstrD[j] = one_byte;
				j++;
				break;
		}
	}

	pstrD[j] = 0;

	return 1;
}
int ssi_http_pws(struct request_rec *r, int argc, char **argv)
{
	char opws[33], npws[24], system_pwd[128];
	int tid;

	if((tid = get_tid()) == MID_FAIL)
		return 0;

	if(atoi(argv[0]) == 1)
		mapi_ccfg_get_str(tid, "ARC_SYS_Password", system_pwd, sizeof(system_pwd));
	else if(atoi(argv[0]) == 2)
		mapi_ccfg_get_str(tid, "ARC_SYS_Password2", system_pwd, sizeof(system_pwd));

	Get_MD5_Str(system_pwd, opws);
	checkStringToJS(opws, npws);

	return so_printf(r, "%s", npws);
}

int ssi_get_ssid(struct request_rec *r, int argc, char **argv)
{
	char ossid[64], nssid[64];
	char cname[64] = {0};
	int tid;

	if((tid = get_tid()) == MID_FAIL)
		return 0;

	sprintf(cname, "ARC_WLAN_%sG_SSID_%s_ESSID", argv[0], argv[1]);
	
	mapi_ccfg_get_str(tid, cname, ossid, sizeof(ossid));
	checkStringToHTML(ossid, nssid);
	return so_printf(r, "%s", nssid);
}

static int ssi_syslog(struct request_rec *r, int argc, char **argv)
{
	int i;
	int type; // runsen_lao, log type: 0--system_log 1--security_log
	int nbytes = 0;
	char line[SYSLOG_BUF_SIZE + 1] = {[0 ... SYSLOG_BUF_SIZE] = 0x00};
	char output_buf[(SYSLOG_BUF_SIZE + 1) * 2] = {[0 ... (SYSLOG_BUF_SIZE * 2 + 1)] = 0x00};
	FILE *fp;
	char buf_status[4] = {[0 ... 3] = 0x00};
	int tid;
	char deviceName[64] = {0};
#if defined(SYSLOG_ENHANCED)
	char datearea[12] = {0};
	char timearea[10] = {0};
	int logtype, level, msgtype;
	char msg[512];
	int index = 1;
#endif

	if((tid = get_tid()) == MID_FAIL)
		return 0;

	char log_level[16] = {0};
	mapi_ccfg_get_str(tid, "ARC_SYS_LOG_Level", log_level, sizeof(log_level));
	if(atoi(log_level) == LOG_LEVEL_TYPE_DEBUG1)
	{
		cprintf("cmd_getlog save disc!!!\n");
		system("owl -save syslog");
		//usleep(500000);
	}
	mng_action("ui_dump_log", log_level);

	usleep(500000);

	while(atoi(mapi_ccfg_get_str(tid, "ARC_SYS_LogDumping", buf_status, sizeof(buf_status))) != 0)
	{
		usleep(100000);
	}

	fp = fopen(SYSLOG_TMP, "r");
	if(fp == NULL)
	{
		ht_dbg("Failed to open %s\n", SYSLOG_TMP);
		return 0;
	}

	type = atoi(argv[0]);

	if(type == 0)
	{
		while(fgets(line, SYSLOG_BUF_SIZE, fp) != NULL)
		{
			/*
			   type = 0 is system_log, security_log use ulogd so it have the keyword ulogd
			   so here we skip the line have keyword ulogd
			*/
			if(strstr(line, "ulogd"))
			{
				continue;
			}

			i = strlen(line);
			line[i] = '\0';

			while(line[i - 1] == '\r' || line[i - 1] == '\n')
			{
				line[i - 1] = '\0';
				i = strlen(line);
			}

			checkStringToJS(line, output_buf);
#if defined(SYSLOG_ENHANCED)
			sscanf(output_buf, "%s %s [%d %d %d] %[^\n]", 
				datearea, timearea, &logtype, &level, &msgtype, msg);
			
			if(strlen(msg) > 0)
			{
				nbytes += so_printf(r, "id[%d]='%d';\n", index, index);
				nbytes += so_printf(r, "timestamp[%d]='%s %s';\n", index, datearea, timearea);
				nbytes += so_printf(r, "moduletype[%d]='%d';\n", index , logtype);
				nbytes += so_printf(r, "level[%d]='%d';\n", index , level);
				nbytes += so_printf(r, "msgtype[%d]='%d';\n", index , msgtype);
				nbytes += so_printf(r, "message[%d]='%s';\n", index , msg);
				index ++;
			}
#else
			nbytes += so_printf(r, "'%s', \n", output_buf);
#endif
		}
	}
	else
	{
		while(fgets(line, SYSLOG_BUF_SIZE, fp) != NULL)
		{
			/* security log only handle the line have keywork ulogd */
			if(strstr(line, "ulogd"))
			{
				i = strlen(line);
				line[i] = '\0';

				while(line[i - 1] == '\r' || line[i - 1] == '\n')
				{
					line[i - 1] = '\0';
					i = strlen(line);
				}

				checkStringToJS(line, output_buf);

				nbytes += so_printf(r, "'%s', \n", output_buf);
			}
		}
	}

	fclose(fp);
	
	return nbytes;
}

static int ssi_sysautolog(struct request_rec *r, int argc, char **argv)
{
	int i;
	int type; // runsen_lao, log type: 0--system_log 1--security_log
	int nbytes = 0;
	char line[SYSLOG_BUF_SIZE + 1] = {[0 ... SYSLOG_BUF_SIZE] = 0x00};
	char output_buf[(SYSLOG_BUF_SIZE + 1) * 2] = {[0 ... (SYSLOG_BUF_SIZE * 2 + 1)] = 0x00};
	FILE *fp;
	char buf_status[4] = {[0 ... 3] = 0x00};
	int tid;
	char deviceName[64] = {0};
#if defined(SYSLOG_ENHANCED)
	char datearea[12] = {0};
	char timearea[10] = {0};
	int logtype, level, msgtype;
	char msg[512];
	int index = 1;
#endif

	if((tid = get_tid()) == MID_FAIL)
		return 0;

#if defined(SYSLOG_ENHANCED)
	char log_level[16] = {0};
	mapi_ccfg_get_str(tid, "ARC_SYS_LOG_Level", log_level, sizeof(log_level));
	mng_action("ui_dump_log", log_level);
#else	
	mng_action("ui_dump_log", "");
#endif

	usleep(500000);

	while(atoi(mapi_ccfg_get_str(tid, "ARC_SYS_LogDumping", buf_status, sizeof(buf_status))) != 0)
	{
		usleep(100000);
	}

	fp = fopen(SYSLOG_TMP, "r");
	if(fp == NULL)
	{
		ht_dbg("Failed to open %s\n", SYSLOG_TMP);
		return 0;
	}

	type = atoi(argv[0]);

	if(type == 0)
	{
		while(fgets(line, SYSLOG_BUF_SIZE, fp) != NULL)
		{
			/*
			   type = 0 is system_log, security_log use ulogd so it have the keyword ulogd
			   so here we skip the line have keyword ulogd
			*/
			if(strstr(line, "ulogd"))
			{
				continue;
			}

			i = strlen(line);
			line[i] = '\0';

			while(line[i - 1] == '\r' || line[i - 1] == '\n')
			{
				line[i - 1] = '\0';
				i = strlen(line);
			}

			checkStringToJS(line, output_buf);
#if defined(SYSLOG_ENHANCED)
			sscanf(output_buf, "%s %s [%d %d %d] %[^\n]", 
				datearea, timearea, &logtype, &level, &msgtype, msg);
			
			if(strlen(msg) > 0)
			{
				nbytes += so_printf(r, "id[%d]='%d';\n", index, index);
				nbytes += so_printf(r, "timestamp[%d]='%s %s';\n", index, datearea, timearea);
				nbytes += so_printf(r, "moduletype[%d]='%d';\n", index , logtype);
				nbytes += so_printf(r, "level[%d]='%d';\n", index , level);
				nbytes += so_printf(r, "msgtype[%d]='%d';\n", index , msgtype);
				nbytes += so_printf(r, "message[%d]='%s';\n", index , msg);
				index ++;
			}
#else
			nbytes += so_printf(r, "'%s', \n", output_buf);
#endif
		}
	}
	else
	{
		while(fgets(line, SYSLOG_BUF_SIZE, fp) != NULL)
		{
			/* security log only handle the line have keywork ulogd */
			if(strstr(line, "ulogd"))
			{
				i = strlen(line);
				line[i] = '\0';

				while(line[i - 1] == '\r' || line[i - 1] == '\n')
				{
					line[i - 1] = '\0';
					i = strlen(line);
				}

				checkStringToJS(line, output_buf);

				nbytes += so_printf(r, "'%s', \n", output_buf);
			}
		}
	}

	fclose(fp);

	return nbytes;
}

#define ALL_DISC_LOG_FILE  "/tmp/All_Discs_log.log"
#define ALL_DISC_LOG_TMP_FILE  "/tmp/tmp_log.log"
int ssi_syslog_save(struct request_rec *r, int argc, char **argv)
{	
	struct sockaddr_in RemoteAddr;
	int sock_fd,n,i;
	char *p=NULL;
	char buf_recv[512];
	char name[32];
	char cmdbuf[128] = {0};
	char filename[64] = {0};
	char tmpbuf[128] = {0};
	char *pt;
	char recvline[10240]={0};  //same size  as owl	
	char *buf ="GET /status/tplg HTTP/1.1\r\nContent-Type:application/json\r\nContent-Length:0\r\nConnection:Keep-Alive\r\nAccept-Encoding:gzip\r\n\r\n";
	sock_fd = socket(AF_INET, SOCK_STREAM, 0);
	memset((char *)&RemoteAddr, 0, sizeof(RemoteAddr));
	RemoteAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	RemoteAddr.sin_port = htons (OWL_LISTEN_PORT);
	RemoteAddr.sin_family = AF_INET;
	
	if(connect(sock_fd, (struct sockaddr *)&RemoteAddr, sizeof(RemoteAddr)) < 0)
	{
		cprintf("connect error!\n");
		close(sock_fd);
		so_printf(r,"{ \"nodes\": [ ] }");
		return -1;
	}
	write(sock_fd, buf, strlen(buf));
	if(read(sock_fd, recvline,sizeof(recvline)) > 0)
	{	
		p = recvline;		

		sprintf(cmdbuf, "rm -rf %s %s", ALL_DISC_LOG_FILE, ALL_DISC_LOG_TMP_FILE);
		system(cmdbuf);
		
		while((p = strstr(p, "device_name")) != NULL)
		{
			memset(buf_recv , 0, 512);
			p = p + strlen("device_name\": \"");
			pt = strstr(p, "\"");
			memset(name , 0, 32);			

			strncpy(name, p, pt - p);
				
			sprintf(filename, "%s_log.log", name);
			sprintf(tmpbuf, "\r\n==============================%s==============================\r\n\r\n", filename);
			sprintf(cmdbuf, "echo -ne \"%s\" > %s", tmpbuf, ALL_DISC_LOG_TMP_FILE);
			system(cmdbuf);

			sprintf(cmdbuf, "cat %s \"/tmp/%s\" >> %s", ALL_DISC_LOG_TMP_FILE, filename, ALL_DISC_LOG_FILE);
			system(cmdbuf);

			sprintf(cmdbuf, "cp %s %s", ALL_DISC_LOG_FILE, ALL_DISC_LOG_TMP_FILE);
			system(cmdbuf);
		}
	}
	
	close(sock_fd);
	return 0;
}

#if defined(SYSLOG_ENHANCED)

static int check_logfile_send_complete(char *filename)
{
	char log_filename[64]  = {0};
	char line[64]  = {0};
	FILE * fp = NULL;
	
	sprintf(log_filename, "/tmp/tftp_syslog");
	fp = fopen(log_filename, "r");
	if(fp != NULL)
	{
		while(fgets(line, 64, fp) != NULL)
		{
			line[strlen(line)-1] = '\0';
			if(strcmp(filename, line) == 0)
			{
				fclose(fp);
				return 0;
			}
		}
		fclose(fp);
	}	

	return 1;
}

static int ssi_owl_syslog(struct request_rec *r, int argc, char **argv)
{
	int i;
	int type; // runsen_lao, log type: 0--system_log 1--security_log
	int nbytes = 0;
	char line[SYSLOG_BUF_SIZE + 1] = {[0 ... SYSLOG_BUF_SIZE] = 0x00};
	char output_buf[(SYSLOG_BUF_SIZE + 1) * 2] = {[0 ... (SYSLOG_BUF_SIZE * 2 + 1)] = 0x00};
	FILE *fp;
	int tid;
	char datearea[12] = {0};
	char timearea[10] = {0};
	int logtype, level, msgtype;
	char msg[512];
	int index = 1;
	int sleep_times = 0;
	char file_name[64] = {0};
	char dev_name[64] = {0};
	char master_devname[64] = {0};

	if ((tid = get_tid()) == MID_FAIL) {
		ht_dbg("[%s] Failed to get_tid \n", __FUNCTION__);
		return 0;
	}

	sleep(3);	//wait for owl post and syslog dump
	
	mapi_ccfg_get_str(tid, "ARC_SYS_DeviceName", master_devname, sizeof(master_devname));	
	mapi_tmp_get(tid, "ARC_SYS_LogFile_Name", dev_name, sizeof(dev_name));
	if(strlen(dev_name) != 0)
	{
		sprintf(file_name, "/tmp/%s_log.log", dev_name);
	}
	else
	{
		sprintf(file_name, "%s", SYSLOG_TMP);
	}

	if(strcmp(master_devname, dev_name) != 0)	//wait for tftp transfer
	{
		while(check_logfile_send_complete(file_name))
		{
			usleep(500000);
			sleep_times += 1;
			if(sleep_times >= 20)
				break;
		}
	}
	
	fp = fopen(file_name, "r");
	if(fp == NULL)
	{
		ht_dbg("Failed to open %s\n", SYSLOG_TMP);
		cprintf("%s_%d>>failed open\n", __FUNCTION__, __LINE__);
		return 0;
	}
	
//	type = atoi(argv[0]);
cprintf("%s_%d>>\n", __FUNCTION__, __LINE__);

//	if(type == 0)
	{
		while(fgets(line, SYSLOG_BUF_SIZE, fp) != NULL)
		{
			/*
			   type = 0 is system_log, security_log use ulogd so it have the keyword ulogd
			   so here we skip the line have keyword ulogd
			*/
			if(strstr(line, "ulogd"))
			{
				continue;
			}

			i = strlen(line);
			line[i] = '\0';

			while(line[i - 1] == '\r' || line[i - 1] == '\n')
			{
				line[i - 1] = '\0';
				i = strlen(line);
			}

			checkStringToJS(line, output_buf);

			sscanf(output_buf, "%s %s [%d %d %d] %[^\n]", 
				datearea, timearea, &logtype, &level, &msgtype, msg);
			
			if(strlen(msg) > 0)
			{
				nbytes += so_printf(r, "id[%d]='%d';\n", index, index);
				nbytes += so_printf(r, "timestamp[%d]='%s %s';\n", index, datearea, timearea);
				nbytes += so_printf(r, "moduletype[%d]='%d';\n", index , logtype);
				nbytes += so_printf(r, "level[%d]='%d';\n", index , level);
				nbytes += so_printf(r, "msgtype[%d]='%d';\n", index , msgtype);
				nbytes += so_printf(r, "message[%d]='%s';\n", index , msg);
				index ++;
			}
		}
	}

	fclose(fp);
//cprintf("%s_%d>>end rtn %d\n", __FUNCTION__, __LINE__, nbytes);

	//Kill tftp server in Master RE
	system("killall -9 udpsvd");

	return nbytes;
}
#endif

int ssi_get_system_uptime(struct request_rec *r, int argc, char **argv)
{
#if 0 //"uptime" gets the current time.
	char buf[128];
	char *p = NULL;
	FILE* fp = NULL;
		
	system("uptime > /tmp/sys_uptime");
	fp = fopen("/tmp/sys_uptime", "r");
	
	if(fp == NULL)
		return so_printf(r, "%s", " ");
	
	fgets(buf, sizeof(buf), fp);

	p = strtok(buf, "up");
	if(p == NULL)
	{
		fclose(fp);
		return so_printf(r, "%s", " ");
	}
	fclose(fp);
	system("rm -f /tmp/sys_uptime");
	
	return so_printf(r, "%s", p);
#endif

	time_t uptime;
	int secs = 0;
	int mins = 0;
	int hours = 0;
	
	uptime = get_time();
	secs = uptime % 60;
	mins = (uptime / 60) % 60;
	hours = uptime / 60 / 60;
	
	return so_printf(r, "%02d:%02d:%02d",  hours, mins, secs);
}

int ssi_get_lan_statistics(struct request_rec *r, int argc, char **argv)
{
	int i = 0;
	char buf[128];
	char *p = NULL;
	char cmd[64];
	FILE* fp = NULL;
	char retbuf[256];
	int tmp[5] = {0};

	memset(retbuf, 0, sizeof(retbuf));
	sprintf(cmd, "ifconfig eth3 > /tmp/lan_statistics");
	system(cmd);
	
	fp = fopen("/tmp/lan_statistics", "r");
	if(fp == NULL)
		return -1;

	while(fgets(buf, sizeof(buf), fp)){
		if(strstr(buf, "RX packets"))
		{
		sscanf(buf, " RX packets:%d errors:%d dropped:%d overruns:%d frame:%d",&tmp[0],&tmp[1],&tmp[2],&tmp[3],&tmp[4] );
		sprintf(retbuf, "%s%d,%d,%d",retbuf,tmp[0],tmp[1],tmp[2]);
		}
		else if(strstr(buf, "TX packets"))
		{
		sscanf(buf, " TX packets:%d errors:%d dropped:%d overruns:%d carrier:%d",&tmp[0],&tmp[1],&tmp[2],&tmp[3],&tmp[4]);
		sprintf(retbuf, "%s,%d,%d,%d",retbuf,tmp[0],tmp[1],tmp[2]);
		}
		else if(strstr(buf, "collisions"))
		{
		sscanf(buf, " collisions:%d txqueuelen:%d",&tmp[0],&tmp[1]);
		sprintf(retbuf, "%s,%d",retbuf,tmp[0]);
		}
		else if(strstr(buf, "RX bytes"))
		{
		sscanf(buf, " RX bytes:%d",&tmp[0]);
		p = strstr(buf, "TX bytes");
		sscanf(p, "TX bytes:%d",&tmp[1]);
		sprintf(retbuf, "%s,%d,%d",retbuf,tmp[0],tmp[1]);
		}
	}
	sprintf(retbuf, "%s|", retbuf);
	fclose(fp);
	system("rm -f /tmp/lan_statistics");
	so_printf(r, "%s", retbuf);

	memset(retbuf, 0, sizeof(retbuf));
	for(i = 0; i < (ARC_DEF_WLAN_SSID_MAXNUM+ARC_DEF_WLAN_5G_MAXNUM); i ++)
	{
		if(i < ARC_DEF_WLAN_SSID_MAXNUM)
			sprintf(cmd, "ifconfig ra%d > /tmp/lan_statistics", i);	
		else
			sprintf(cmd, "ifconfig rai%d > /tmp/lan_statistics", i-ARC_DEF_WLAN_SSID_MAXNUM);	
		
		system(cmd);
		fp = fopen("/tmp/lan_statistics", "r");
		
		if(fp == NULL)
			continue;

		while(fgets(buf, sizeof(buf), fp)){
			if(strstr(buf, "RX packets"))
			{
				sscanf(buf, " RX packets:%d errors:%d dropped:%d overruns:%d frame:%d",&tmp[0],&tmp[1],&tmp[2],&tmp[3],&tmp[4] );
				sprintf(retbuf, "%s%d,%d,%d",retbuf,tmp[0],tmp[1],tmp[2]);
			}
			else if(strstr(buf, "TX packets"))
			{
				sscanf(buf, " TX packets:%d errors:%d dropped:%d overruns:%d frame:%d",&tmp[0],&tmp[1],&tmp[2],&tmp[3],&tmp[4]);
				sprintf(retbuf, "%s,%d,%d,%d",retbuf,tmp[0],tmp[1],tmp[2]);
			}
			else if(strstr(buf, "collisions"))
			{
				sscanf(buf, " collisions:%d txqueuelen:%d",&tmp[0],&tmp[1]);
				sprintf(retbuf, "%s,%d",retbuf,tmp[0]);
			}
			else if(strstr(buf, "RX bytes"))
			{
				sscanf(buf, " RX bytes:%d",&tmp[0]);
				p = strstr(buf, "TX bytes");
				sscanf(p, "TX bytes:%d",&tmp[1]);
				sprintf(retbuf, "%s,%d,%d",retbuf,tmp[0],tmp[1]);
			}
		}
		sprintf(retbuf, "%s|", retbuf);
		fclose(fp);
		system("rm -f /tmp/lan_statistics");
	}
	return so_printf(r, "%s", retbuf);
}

int ssi_get_mem_cpu_info(struct request_rec *r, int argc, char **argv)
{
	int usedmem = 0;
	int freemem = 0;
	int totolmem = 0;
	char usrcpu[16];
	char syscpu[16];
	char buf[128];
	char retbuf[256] = " ";
	char *p = NULL;
	FILE* fp = NULL;

	system("top -n 1 > /tmp/meminfo");
	fp = fopen("/tmp/meminfo", "r");
	if(fp == NULL)
		return so_printf(r, "%s", " ");
	
	while(fgets(buf, sizeof(buf), fp))
	{
		if(p = strstr(buf, "Mem:"))
		{
			usedmem = atoi(p+5);
			p = strstr(buf, "used,");
			freemem = atoi(p+6);
			totolmem = usedmem+freemem;
		}
		if(strstr(buf, "CPU:"))
		{
			sscanf(buf, "CPU:   %s usr   %s sys", usrcpu, syscpu);	
			break;
		}
	}
	fclose(fp);
	system("rm -rf > /tmp/meminfo");

	return so_printf(r, "%d,%d,%s,%s", totolmem, freemem, usrcpu,syscpu);
}

#define PC_LIST_LOG "/tmp/pc_list_tmp"
int ssi_dump_new_dhcp_leased(struct request_rec *r, int argc, char **argv)
{
	char buf[256] = {0};
	FILE *fp = NULL;
	char dev_info[11][64] = {0};
	int ret = 0;
	char *ptr = NULL, *q = NULL;
	char *p = NULL;
	int c = 0;
	struct tm timep;
	time_t timet;

	snprintf(buf, sizeof(buf), "pc_list_cli print > %s", PC_LIST_LOG);
	system(buf);

	fp = fopen(PC_LIST_LOG, "r");
	if(fp)
	{
		while(fgets(buf, sizeof(buf), fp) != NULL)
		{
			if(strchr(buf, '[') == NULL)
				continue;

			memset(dev_info, 0, sizeof(dev_info));

			for(ptr = strtok(buf, "["), c = 0; ptr != NULL && c < 11; ptr = strtok(NULL, "["), c++)
			{
				if(c == 0)
				{
					strncpy(dev_info[c], ptr, 17);
					continue;
				}

				q = ptr;
				while(*q != ']')
					q++;
				*q = '\0';

				strcpy(dev_info[c], ptr);

			}
			if(strcmp(dev_info[6], "DHCP") == 0 )
			{
				p = strtok(dev_info[4], " ");
				timep.tm_wday = atoi(p);
				p = strtok(NULL, "/");
				timep.tm_year = atoi(p) - 1900;
				p = strtok(NULL, "/");
				timep.tm_mon = atoi(p) - 1;
				p = strtok(NULL, " ");
				timep.tm_mday = atoi(p);
				p = strtok(NULL, ":");
				timep.tm_hour = atoi(p);
				p = strtok(NULL, ":");
				timep.tm_min = atoi(p);
				p = strtok(NULL, " ");
				timep.tm_sec = atoi(p);

				timet = mktime(&timep);
				sprintf(dev_info[4], "%d", timet - get_time());
			}
			else
				sprintf(dev_info[4], " ");
			
			if(ret != 0)
				ret += so_printf(r, ",");
			ret += so_printf(r, "[\'%s\'", dev_info[0]);

			for(c = 1; c < 11; c++)
				ret += so_printf(r, ", \'%s\'", dev_info[c]);

			ret += so_printf(r, "]");
		}
		fclose(fp);
		fp = NULL;
	}
	snprintf(buf, sizeof(buf), "rm -rf %s", PC_LIST_LOG);
	system(buf);

	return ret;
}

int ssi_LoginStatus(struct request_rec *r, int argc, char **argv)
{
	sock_t *gr_ip;
#ifdef SYSLOG_ENHANCED
	SetSystemLogFmt(LOG_TYPE_APP, LOG_LEVEL_TYPE_INFO, LOG_APP_MESSAGE_TYPE_http, "HTTP-Auto Logout from IP %s.", r->remote_ipstr);
#endif

	gr_ip = get_granted_ip();
	if (!gr_ip)
		return so_printf(r, "0");

	if ( match_ip(*gr_ip, r->remote_ip) &&
			/*(time(NULL) - get_granted_time() < get_login_timeout()))*/
			//get the system uptime instead of system time.
			(get_time() - get_granted_time() < get_login_timeout()))
		return so_printf(r, "1");
	else
		return so_printf(r, "0");
}


int ssi_get_mask_dns_gateway_status(struct request_rec *r, int argc, char **argv)
{
	int tid;
	char mask_val[64] = {0},  dns_server_val[64] = {0}, gate_way_val[64] = {0};
	char mask_dns_gate[128] = {0};
	int tmp;

	if ((tid = get_tid()) == MID_FAIL) {
		ht_dbg("[%s] Failed to get_tid \n", __FUNCTION__);
		return 0;
	}

	mapi_ccfg_get_str(tid, "ARC_LAN_0_IP4_Netmask", mask_val, sizeof(mask_val));
	mapi_ccfg_get_str(tid, "ARC_LAN_0_IP4_DNS", dns_server_val, sizeof(dns_server_val));
	mapi_ccfg_get_str(tid, "ARC_LAN_0_IP4_Gateway", gate_way_val, sizeof(gate_way_val));

	for(tmp=0;tmp<64;tmp++)
	{
		if(dns_server_val[tmp]==' ')
			{
			dns_server_val[tmp]='\0';
			break;
			}
	}


	strcat(mask_dns_gate, "\"");
	strcat(mask_dns_gate, mask_val);
	strcat(mask_dns_gate, "@");
	strcat(mask_dns_gate, dns_server_val);
	strcat(mask_dns_gate, "@");
	strcat(mask_dns_gate, gate_way_val);
	strcat(mask_dns_gate, "\"");

//	cprintf("[********Debug********]ssi_get_mask_dns_gateway_status->:%s\n\n", mask_dns_gate);	

	return so_printf(r, mask_dns_gate);
}


int add_backup_config_MD5(char *filename){

	FILE *fp, *fp_out;
	int ret = -1;
	int i, ch, a;
	char md5_sum[MD5_length+1];
	char MD5_random32[MD5_length+1];
	char line[MD5_length*2+1];
	char buffer[256];

	// generate random 32 
	srand(time(NULL));
	for( i = 0 ; i < MD5_length ; i++){
		a = (rand()%64);
		if( a >=0 && a <=25) MD5_random32[i] = a+65;		// Low case
		else if( a >=26 && a <=51) MD5_random32[i] = a+71;	// High case
		else if( a >=52 && a <=61) MD5_random32[i] = a-4;	// Number
		else if( a == 62 ) MD5_random32[i] = '+';
		else MD5_random32[i] = '/';
	}
	MD5_random32[MD5_length] = '\0';
	
	// generate MD5 checksum
	snprintf(buffer, sizeof(buffer), "md5sum %s > /tmp/config.md5", filename);
	system(buffer);
	
	fp = fopen("/tmp/config.md5", "r");
	if( fp == NULL) return ret;
	for (i = 0 ; i < MD5_length && isxdigit(ch = fgetc(fp)) ; i++) 	md5_sum[i] = ch;
	fclose(fp);
	md5_sum[MD5_length] = '\0';
		
	// add MD5 checkum
	fp = fopen(filename, "r");
	fp_out = fopen("/tmp/.config.tgz.tmp_", "w");
	if( fp == NULL || fp_out == NULL ) return ret;
	i = 0;
	memset(buffer, 0, 256);
	snprintf(buffer, MD5_length*2+1, "%s%s", MD5_random32, md5_sum);
	
	while( !feof(fp) ){
	 	if( fgets(line, MD5_length*2+1, fp) != NULL){
	 		if( !strcmp(line, "\n") ) continue;
	 		fprintf(fp_out, "%s\n", line);
	 		i++;
		}
		if( i == MD5_location ){
			fprintf(fp_out, "%s\n", buffer);
		}
	}
	fclose(fp);
	fclose(fp_out);
	
	// Delete the last line and add EOF tag
	snprintf(buffer, sizeof(buffer), "sed -i '$d' %s", "/tmp/.config.tgz.tmp_");
	system(buffer);
	snprintf(buffer, sizeof(buffer), "echo ================================================================ >> %s", "/tmp/.config.tgz.tmp_");
	system(buffer);

	snprintf(buffer, sizeof(buffer), "mv /tmp/.config.tgz.tmp_ %s", filename);
	system(buffer);
	
	ret = 0;
	return ret;
}

int ssi_get_backup_name(struct request_rec *r, int argc, char **argv)
{
	char buf1[128] = {0}, buf2[128] = {0};
	char cmd[256] = {0};
	char fwver[64] = {0};
	char enc_method[17] = "";
	struct tm *tm, tm_tmp;
	time_t t;

	/*pete  2013-04-16      ticket-462*/
	int tid;
	char tmpfile[] = "/tmp/glbcfg_XXXXXX";
	/*end   pete 2013-04-16*/

	//snprintf(buf, sizeof(buf), "%s%s%s_%s.cfg", path, MODEL_NAME, MODEL_VERSION, ARCADYAN_VERSION);
	/*mapi_ccfg_get_str(tid, "VAR_SYSTEM_FWVERSION", buf1, sizeof(buf1));*/
	snprintf(buf1, sizeof(buf1), "backup");

	if ((tid = get_tid()) == MID_FAIL)
		return r->bytes_sent;
	
	time(&t);
	tm = localtime_r(&t, &tm_tmp);
	mapi_ccfg_get_str(tid, "ARC_SYS_FWVersion", fwver, sizeof(fwver));
	snprintf(buf2, sizeof(buf2), "RGA_config_%s_%02lu.%02lu.%02lu_%02lu%02lu.bin", fwver,tm -> tm_mday,tm -> tm_mon + 1,
									(tm -> tm_year + 1900) - ((tm -> tm_year + 1900) / 100) * 100,  // get value under 100
									tm -> tm_hour,tm -> tm_min);
	
	r->bytes_sent += so_printf(r, "%s", buf2);

	/*pete  2013-04-16      ticket-462
	 Description:
	 In au-0.5, the file /etc/config/.glbcfg is NOT updated while do the ccfg_set(), or mng_cli set.
	 It is only be updated while ccfg_commit()/ccfg_backup(), mng_cli commit/mng_cli backup.
	 To update .glbcfg file, and avoid manipulating .glbcfg file directly,
	 execute ccfg_backup(), then handle the temporary file.
	*/

	mktemp(tmpfile);

	if(mapi_ccfg_backup(tid, tmpfile) == MID_FAIL)
	{
		printf("Fail to backup .glbcfg file\n");

		// unlink the temporary file
		unlink(tmpfile);

		return r->bytes_sent;
	}

	system("mkdir -p /tmp/.config; cp -a /etc/config/ /tmp/.config");

	/* Replace the .glbcfg with the tmpfile. After that, tar the whole dir. */
	snprintf(cmd, sizeof(cmd), "mv %s /tmp/.config/config/.glbcfg", tmpfile);
	system(cmd);
	system("tar czf /tmp/.config.tgz -C /tmp/.config/ .");
	system("rm -rf /tmp/.config");

	arc_cfg_get(tid, ARC_PROJ_ASUS_HTTPD_CONFIG_SSL_EncrptType, enc_method, sizeof(enc_method), "0");

	snprintf(cmd, sizeof(cmd), "openssl enc -%s -salt -a -in /tmp/.config.tgz -out /tmp/.config.enc -k %s", enc_method, CONFIG_BACKUP_KEY);
	system(cmd);
	system("rm -rf /tmp/.config.tgz");
		
	/* Adam 20151106, Add MD5 into backup configuration file.*/
	add_backup_config_MD5("/tmp/.config.enc");
	
	snprintf(cmd, sizeof(cmd), "mv /tmp/.config.enc /tmp/%s", buf2);
	system(cmd);

	unlink(tmpfile);
	/*end	pete 2013-04-16*/

	return r->bytes_sent;
}


int ssi_dump_wl_asso_table(struct request_rec *r, int argc, char **argv)
{
	mng_action("dump_wl_assoclist", "");
	sleep(1);
	return 1;
}

int ssi_wl_inused_channel(struct request_rec *r, int argc, char **argv)
{
	char cmd[64] = {0x00};
	//get_inused_ch is a shell script
	snprintf(cmd, sizeof(cmd), "source /usr/scripts/get_inused_ch");
	system(cmd);
	return 1;
}

int ssi_wl_asso_table(struct request_rec *r, int argc, char **argv)
{
	FILE *fp;
	char line[80];
	int count=0;

	if ((fp = fopen("/tmp/wl_assoclist", "r"))) {
		//now /tmp/wl_assoclist only contains assocated mac.
		//please refer /sbin/arc_wlan.
		//Alpha modified 2013-03-19
		while( fgets(line, sizeof(line), fp) != NULL ) {
			if((strlen(line) > 0) && (line[strlen(line) - 1] == '\n'))
			{
				line[strlen(line) - 1] = '\0';
			}
			so_printf(r, "%c'%s'\n", count++?',':' ', line);
		}
		fclose(fp);
	}
	return 1;
}

typedef  unsigned int    __u32;
#define RAETH_LINK_STATUS  0x89FF

typedef struct ralink_mii_ioctl_data
{
	__u32   phy_id;
	__u32   reg_num;
	__u32   val_in;
	__u32   val_out;
} ra_mii_ioctl_data;

int getCurBss2040NeedFallBack(char *inf){
	char filename[20] = "/tmp/wlan_stats";
	char cmd[128];
	char string[128];
	int Bss2040NeedFallBack;
	FILE *fp;

	sprintf(cmd,"iwpriv %s stat > %s", inf, filename);
	system(cmd);

	fp = fopen(filename, "r");
	if( fp == NULL ){
		return;
	}

			
	while(fgets(string, sizeof(string), fp))
	{
		if(strstr(string, "Bss2040NeedFallBack"))
		{
			sscanf(string, "Bss2040NeedFallBack = %ld", &Bss2040NeedFallBack);
			break;
		}
	}
			
	fclose(fp);
	sprintf(cmd,"rm -f %s",filename);
	system(cmd);
	
	return Bss2040NeedFallBack;
	
}

int ssi_get_lan_status(struct request_rec *r, int argc, char **argv)
{
	int val;
	int sk = 0;
	int ret = 0;
	struct ifreq ifr;
	ra_mii_ioctl_data mii;
	int wlanspeed24;
	int wlanspeed5;
	int bindwidth = 0;
	int tid;
	char radiomode[16];
	char wllink[8][16] = {"Down","Down","Down","Down","Down","Down","Down","Down"};
	char tmp[16];
	char tmp2[16];
	char name[64];
	int i =0;
	char bandwidth[2];
	char coexistence[2];
	int Bandwidth;
	int Coexistence;
	int Bss2040NeedFallBack;
	
	/*LAN*/
	sk = socket(AF_INET, SOCK_DGRAM, 0);
	if (sk < 0)
	{
		cprintf("Open socket failed\n");
		return -1;
	}

	strncpy(ifr.ifr_name, "eth2", 5);
	ifr.ifr_data = &mii;
	mii.phy_id = 0;
	mii.reg_num = 0;

	ret = ioctl(sk, RAETH_LINK_STATUS, &ifr);
	if (ret < 0)
	{
		cprintf("mii_mgr: ioctl error\n");
		close(sk);
		return ret;
	}
	else
	{
		val = mii.val_out;
		close(sk);
		if(val == 0)
		{
			so_printf(r, "['0','Down','-','-'],");
		}
		else if((val == 10) || (val == 100)|| (val == 1000))
		{
			so_printf(r, "['0','up','%d Mbps','-'],", val);
		}
	}

	/*WLAN 2.4G*/
	if((tid = get_tid()) == MID_FAIL)
		return 0;

	mapi_ccfg_get_str(tid, "ARC_WLAN_24G_RadioMode", radiomode, sizeof(radiomode));
	if(strcmp(radiomode, "b") == 0)
		wlanspeed24 = 11;
	else if((strcmp(radiomode, "g") == 0) || (strcmp(radiomode, "bg") == 0))
		wlanspeed24 = 54;
	else
	{
		mapi_ccfg_get_str(tid, "ARC_WLAN_24G_ChannelBandwidth", bandwidth, sizeof(bandwidth));
		Bandwidth = atoi(bandwidth);			
		mapi_ccfg_get_str(tid, "ARC_WLAN_24G_HT_BSSCoexistence", coexistence, sizeof(coexistence));
		Coexistence = atoi(coexistence);
		if((Bandwidth == 1) && (Coexistence == 0))
			wlanspeed24 = 300;
		else if((Bandwidth == 1) && (Coexistence == 1))
		{
			Bss2040NeedFallBack = getCurBss2040NeedFallBack("ra0");
			
			if(Bss2040NeedFallBack == 0)
				wlanspeed24 = 300;
			else
				wlanspeed24 = 150;
		}
		else
			wlanspeed24 = 150;
	}
	
	mapi_ccfg_get_str(tid, "ARC_WLAN_24G_Enable", tmp, sizeof(tmp));
	if(atoi(tmp) == 1)
	{
		for(i = 0; i < ARC_DEF_WLAN_SSID_MAXNUM; i ++)
		{
			sprintf(name, "ARC_WLAN_24G_SSID_%d_Enable", i);
			mapi_ccfg_get_str(tid, name, tmp, sizeof(tmp));
			if(atoi(tmp) == 1)
			{
				strcpy(wllink[i], "up");
			}
			so_printf(r, "['%d','%s','%d Mbps','-'],", i+1, wllink[i],wlanspeed24);
		}
	}

	/*WLAN 5G*/
	mapi_ccfg_get_str(tid, "ARC_WLAN_5G_ChannelBandwidth", tmp, sizeof(tmp));
	mapi_ccfg_get_str(tid, "ARC_WLAN_5G_VHT_BW", tmp2, sizeof(tmp2));
	if(atoi(tmp) == 0)
		bindwidth = 20;
	else if(atoi(tmp2) == 0)
		bindwidth = 40;
	else
		bindwidth = 80;

	mapi_ccfg_get_str(tid, "ARC_WLAN_5G_RadioMode", radiomode, sizeof(radiomode));
	if(strcmp(radiomode, "a") == 0)
		wlanspeed5 = 54;
	else if(strcmp(radiomode, "an") == 0)
	{
		if(bindwidth == 20)
			wlanspeed5 = 144;
		else
			wlanspeed5 = 300;
	}
	else
	{
		if(bindwidth == 20)
			wlanspeed5 = 173;
		else if(bindwidth == 40)
			wlanspeed5 = 400;
		else
			wlanspeed5 = 867;
	}
	
	mapi_ccfg_get_str(tid, "ARC_WLAN_5G_Enable", tmp, sizeof(tmp));
	if(atoi(tmp) == 1)
	{
		for(i = 0; i < ARC_DEF_WLAN_5G_MAXNUM; i ++)
		{
			sprintf(name, "ARC_WLAN_5G_SSID_%d_Enable", i);
			mapi_ccfg_get_str(tid, name, tmp, sizeof(tmp));
			if(atoi(tmp) == 1)
			{
				strcpy(wllink[4+i], "up");
			}
			so_printf(r, "['%d','%s','%d Mbps','-'],", i+1+ARC_DEF_WLAN_SSID_MAXNUM, wllink[4+i],wlanspeed5);
		}
	}
	
	return 0;
}

int ssi_get_tmp_value(struct request_rec *r, int argc, char **argv)
{
	int tid;
	char buf[8] = {0};
	unsigned char randBuf[5] = {0};

	if((tid = get_tid()) == MID_FAIL)
		return 0;

	RAND_bytes(randBuf, 4);

	sprintf(buf, "%u",  *((unsigned int*)(randBuf)));	
	mapi_tmp_set(tid, "ARC_HTTP_TMP_VAL", buf);
	so_printf(r, "%s", buf);

	return 0;
}

int ssi_get_upgrade_schedule(struct request_rec *r, int argc, char **argv)
{
	int tid;
	char buf[32] = {0};
	int hour = 0;
	int min = 0;
	int sec = 0;

	if((tid = get_tid()) == MID_FAIL)
		return 0;

	mapi_tmp_get(tid, "ARC_FW_Upgrade_Delay_Enable", buf, sizeof(buf));
	if(atoi(buf) == 1)
	{
		mapi_tmp_get(tid, "ARC_FW_Upgrade_Delay_TIME", buf, sizeof(buf));
		if(strlen(buf) != 0)
		{
			sec = atoi(buf);
			hour = sec/60/60;
			min = sec/60%60;
			sprintf(buf, "%02d:%02d", hour, min);
		}
	}
	
	so_printf(r, "%s", buf);

	return 0;
}


int ssi_get_access_control_profile_info(struct request_rec *r, int argc, char **argv)
{
	int i, totalLen = 0;
	char *p=NULL;
	char recvline[102400]={0};  //same size	as owl
	char soprintfbuff[SOPRINTFBUFSIZE+1]={0};
	char *buf ="GET /status/access_control_profile HTTP/1.1\r\nContent-Type:application/json\r\nContent-Length:0\r\nConnection:Keep-Alive\r\nAccept-Encoding:gzip\r\n\r\n";
	totalLen = get_message_from_owl(buf, recvline, sizeof(recvline));
	if(totalLen > 0)
	{
		p = strchr(recvline,'{');
		if(p == NULL)
		{
			so_printf(r,"{ \"profile\": [ ] }");
			goto out;
		}		
		p[strlen(p)]='\0';			
		for(i=strlen(p);i>0;i-=SOPRINTFBUFSIZE)  //data length >  SOPRINTFBUFSIZE
		{	
			memset(soprintfbuff,0,sizeof(char)*(SOPRINTFBUFSIZE+1));
			strncpy(soprintfbuff,p,SOPRINTFBUFSIZE);
			soprintfbuff[strlen(soprintfbuff)]='\0';
			p = p + SOPRINTFBUFSIZE;
			so_printf(r,"%s",soprintfbuff);
		}
	}
	else
	{
		so_printf(r,"{ \"profile\": [ ] }");
	}
out:	
	return 0;
}




#define OUT_BASE64_SZ 100
extern char *b64_encode( unsigned char *src ,int src_len, unsigned char* space, int space_len);

static int ssi_image_token(struct request_rec *r, int argc, char **argv)
{
	int i;
	int type; // runsen_lao, log type: 0--system_log 1--security_log
	int nbytes = 0;
	unsigned long token = 0;
	// NzgyMjIwMTU2Cg==
	char output_buf[OUT_BASE64_SZ+1] ={0}; // {[0 ... (OUT_BASE64_SZ + 1)] = 0x00};
	char *p;
	int len=0;
	int tid;

	if((tid = get_tid()) == MID_FAIL)
		return 0;

#ifdef CONFIG_HTTPD_TOKEN_CHECK_SUPPORT
	token = httoken_get( r->url?r->url:"/" );

	//ht_dbg("token=%lu from %s\n", token, r->url?r->url:"NULL" );

    sprintf(output_buf, "%lu", token );
#else
	sprintf(output_buf, "123456789"); //hard code
#endif
	len=strlen(output_buf);

	p=(char *)( output_buf + len+1);
	//cprintf("get [%s]\n",output_buf);
	b64_encode((unsigned char *)output_buf ,len, (unsigned char *) p, OUT_BASE64_SZ-len);

	// NOTE: hugh 2014/2/20
	//  A Data URI scheme(RFC2397) to present a space GIF icon.
	//
	//  this return a empty gif for present purpuse! but we dirty append our session ID after
	//  UI need take time to reteieve the sesison ID from "dutrefer" Image Object
	//
   nbytes += so_printf(r, "<img title=spacer src=\"data:image/gif;base64,R0lGODlhAQABAIAAAAAAAP///yH5BAEAAAAALAAAAAABAAEAAAIBRAA7%s\" border=0>", p);

	return nbytes;
}
