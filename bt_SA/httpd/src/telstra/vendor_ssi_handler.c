#include "httpd.h"
#include "utils.h"
#include "md5.h"
#include "ssi_handler.h"
#include "vendor_ssi_handler.h"
#include <arca-global.h>
#include "arc_proj_table.h"
#include <pthread.h>

extern int ssi_voip_get_register_status(struct request_rec *r, int argc, char **argv);

#define SYSLOG_TMP "/tmp/sys_log.log"
#define SYSLOG_BUF_SIZE 1023

#define STORAGE_MOUNT_ROOT_PATH_SLASH "/tmp/media/"

static int ssi_syslog(struct request_rec *r, int argc, char **argv);

static int ssi_image_token(struct request_rec *r, int argc, char **argv);


extern int fmt_args(int argc, char **argv, char *fmt, ...);


/* Vendor specified handlers.
 */
extern pthread_mutex_t rand_mutex_c;

struct ssi_handler vendor_handlers[] = {

	/* The following are only for arcadyan. */
	{ "dump_route_table", ssi_dump_route_table },
	{ "http_pws", ssi_http_pws },
	{ "syslog", ssi_syslog }, //Sync from belkin project.
	{ "LoginStatus", ssi_LoginStatus },
	{ "get_backup_name", ssi_get_backup_name },
	{ "wl_asso_table", ssi_wl_asso_table },
	{ "wl_inused_channel", ssi_wl_inused_channel },
	{ "get_disk_info_files", ssi_get_disk_info_files },
	{ "get_file_info", ssi_get_file_info },
	{ "get_lan_status", ssi_get_lan_status },
	{ "get_dsl_status", ssi_get_dsl_status },
	{ "get_disk_info", ssi_get_disk_info },
	{ "dump_wl_asso_table", ssi_dump_wl_asso_table },

/* common parts*/
	{ "IMG_TOKEN", ssi_image_token },

	{ NULL, NULL }
};

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


	mapi_ccfg_get_str(tid, "ARC_SYS_Password", system_pwd, sizeof(system_pwd));

	Get_MD5_Str(system_pwd, opws);
	checkStringToJS(opws, npws);

	return so_printf(r, "%s", npws);
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

	if((tid = get_tid()) == MID_FAIL)
		return 0;


	while(atoi(mapi_ccfg_get_str(tid, "ARC_SYS_LogDumping", buf_status, sizeof(buf_status))) != 0)
	{
		usleep(100000);
	}

	mng_action("ui_dump_log", "");

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

			nbytes += so_printf(r, "'%s', \n", output_buf);
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

int ssi_LoginStatus(struct request_rec *r, int argc, char **argv)
{
	sock_t *gr_ip;

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

int ssi_get_backup_name(struct request_rec *r, int argc, char **argv)
{
	char buf1[128] = {0}, buf2[128] = {0};
	char cmd[256] = {0};

	/*Adam 20150424 Add mutex lock to prevent backup request thread from being preempted.  */
	pthread_mutex_lock(&rand_mutex_c);
	
	/*pete  2013-04-16      ticket-462*/
	int tid;
	char tmpfile[] = "/tmp/glbcfg_XXXXXX";
	/*end   pete 2013-04-16*/

	//snprintf(buf, sizeof(buf), "%s%s%s_%s.cfg", path, MODEL_NAME, MODEL_VERSION, ARCADYAN_VERSION);
	/*mapi_ccfg_get_str(tid, "VAR_SYSTEM_FWVERSION", buf1, sizeof(buf1));*/
	snprintf(buf1, sizeof(buf1), "backup");
	snprintf(buf2, sizeof(buf2), "%s%s_%s.cfg", "arcadyan", "V1", buf1);
	r->bytes_sent += so_printf(r, "%s", buf2);

	/*pete  2013-04-16      ticket-462
	 Description:
	 In au-0.5, the file /etc/config/.glbcfg is NOT updated while do the ccfg_set(), or mng_cli set.
	 It is only be updated while ccfg_commit()/ccfg_backup(), mng_cli commit/mng_cli backup.
	 To update .glbcfg file, and avoid manipulating .glbcfg file directly,
	 execute ccfg_backup(), then handle the temporary file.
	*/
	if ((tid = get_tid()) == MID_FAIL){
		pthread_mutex_unlock(&rand_mutex_c);

		return r->bytes_sent;
	}
	mktemp(tmpfile);

	if(mapi_ccfg_backup(tid, tmpfile) == MID_FAIL)
	{
		printf("Fail to backup .glbcfg file\n");

		// unlink the temporary file
		unlink(tmpfile);

		pthread_mutex_unlock(&rand_mutex_c);

		return r->bytes_sent;
	}

	system("mkdir -p /tmp/.config; cp -a /etc/config/ /tmp/.config");

	/* Replace the .glbcfg with the tmpfile. After that, tar the whole dir. */
	snprintf(cmd, sizeof(cmd), "mv %s /tmp/.config/config/.glbcfg", tmpfile);
	system(cmd);
	system("tar czf /tmp/.config.tgz -C /tmp/.config/ .");
	system("rm -rf /tmp/.config");

	/*Adam 20150326 change SSL encryption method for GUI back/restore configuration  */
	char enc_method[17] = "";
	arc_cfg_get(tid, ARC_HTTPD_CONFIG_SSL_EncrptType, enc_method, sizeof(enc_method), "0");
	
	/* Velmurugan 12232014> Encrypt config file while backup */
	#if defined (VRV9510RWAC)
		snprintf(cmd, sizeof(cmd), "openssl enc -%s -salt -a -in /tmp/.config.tgz -out /tmp/.config.enc -k %s", enc_method, CONFIG_BACKUP_KEY);
		system(cmd);
		system("rm -rf /tmp/.config.tgz");
		snprintf(cmd, sizeof(cmd), "mv /tmp/.config.enc /tmp/%s", buf2);
		system(cmd);
	#else
	// backup success, using temporary file
	snprintf(cmd, sizeof(cmd), "mv /tmp/.config.tgz /tmp/%s", buf2);

	system(cmd);
	#endif

	unlink(tmpfile);
	/*end	pete 2013-04-16*/

	pthread_mutex_unlock(&rand_mutex_c);
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

#define DF_INFO_FILE "/tmp/disk_info_file"
#define GET_DF_INFO_FILE_CMD	"df > "DF_INFO_FILE
#define DF_PARSE_FILE "/tmp/disk_info"
#define FS_TMP_FILE "/tmp/disk_fs_tmp"
#define FS_PARSE_FILE "/tmp/disk_fs"

int ssi_get_disk_info_files(struct request_rec *r, int argc, char **argv)
{
  FILE *fp = NULL;
  FILE *fp1 = NULL;
  char buffer[255]={0};
	char device_tmp[64]={0};
	char device[64]={0};
	char total_size[64]={0};
	char used_size[64]={0};
	char free_size[64]={0};
	char percent[64]={0};
	char mountpoint[64]={0};
	char *cp = NULL;
	/*char fs_type[64]={0};*/
	char cmd[128] = {0x00};

	unlink(DF_PARSE_FILE);
	unlink(FS_TMP_FILE);
	unlink(FS_PARSE_FILE);
	system(GET_DF_INFO_FILE_CMD);

	if((fp = fopen(DF_INFO_FILE,"r"))!=NULL)
	{
		if((fp1 = fopen(DF_PARSE_FILE, "w")))
		{
			while(fgets(buffer,255,fp)!=NULL)
			{
					sscanf(buffer, "%s %s %s %s %s %s", device_tmp,total_size,used_size,free_size, percent, mountpoint);
					memcpy(device,device_tmp+5,10);
					//here only for show usb-disk, so skip others.
					if(strstr(mountpoint, STORAGE_MOUNT_ROOT_PATH_SLASH) == NULL)
						continue;
					if(strstr(mountpoint, USER_ROOT_DIR))
						continue;

					memset(buffer,'\0',255);
					strcpy(buffer,"'");
					strcat(buffer,device);
					strcat(buffer,"'");
					strcat(buffer,":");
					strcat(buffer,"'");
					strcat(buffer,"");
					strcat(buffer,total_size);
					strcat(buffer,"|");
					strcat(buffer,used_size);
					strcat(buffer,"|");
					strcat(buffer,free_size);
					strcat(buffer,"|");
					if((cp = strstr(mountpoint, STORAGE_MOUNT_ROOT_PATH_SLASH)) != NULL)
					{
						cp += strlen(STORAGE_MOUNT_ROOT_PATH_SLASH);
						strcat(buffer, cp);
					}
					else
					{
						strcat(buffer, mountpoint);
					}
					strcat(buffer,"',\n");
					fprintf(fp1, buffer);

					//get the fs type from mount. format='device':'fs_type',
					snprintf(cmd, sizeof(cmd), "mount|awk '$1 ~ /%s/{printf(\"%%c%s%%c:%%c%%s%%c,\\n\",39,39,39,$5,39)}' >> %s", device, device, FS_TMP_FILE);
					system(cmd);
			}
			fclose(fp1);
		}
		fclose(fp);
	}
	snprintf(cmd, sizeof(cmd), "cat %s |uniq > %s", FS_TMP_FILE, FS_PARSE_FILE);
	system(cmd);
	unlink(DF_INFO_FILE);
	unlink(FS_TMP_FILE);
	return 0;
}
int ssi_get_file_info(struct request_rec *r, int argc, char **argv)
{
	FILE *fp = NULL;
	char line[80];
	char *name;
	char filename[80] = {0x00};

	if (fmt_args(argc, argv, "%s", &name) < 1) {
			so_printf(r, "%s", "Insufficient args\n");
			return -1;
	}
	if(!strcmp(name, "disk_info"))
	{
		snprintf(filename, sizeof(filename), "%s", DF_PARSE_FILE);
	}
	else if(!strcmp(name, "fs_info"))
	{
		snprintf(filename, sizeof(filename), "%s", FS_PARSE_FILE);
	}
	else
	{
		return -1;
	}
	if((fp = fopen(filename, "r")))
	{
		while(fgets(line, sizeof(line), fp) != NULL) {
			so_printf(r, "%s", line);
		}
		fclose(fp);
	}
	return 0;
}

static int separate_string(char *orig_string, char **tar_ptrs, char *sep_symbol, int sep_count)
{
	int i = 0;
	char *pos = NULL;
	char *found = NULL;

	if(orig_string == NULL || tar_ptrs == NULL || sep_symbol == NULL || sep_count < 0)
	{
		printf("NULL pointer exception!\n");
		return -1;
	}

	if(strlen(orig_string) == 0 || (sep_count > 0 && strlen(orig_string) < sep_count))
	{
		printf("orig_string[%s] is too short!\n", orig_string);
		return -1;
	}

	tar_ptrs[0] = orig_string;
	pos = orig_string;
	i = 1;

	while((found = strstr(pos, sep_symbol)) != NULL)
	{
		tar_ptrs[i] = found + 1;
		pos = found + 1;
		*found = '\0';
		i++;
	}

	if(sep_count > 0 && i != sep_count)
	{
		printf("Missing some parameters! i == %d != %d\n", i, sep_count);
		return -1;
	}
	else if(sep_count == 0)
	{
		return i;
	}
	else
	{
		return 0;
	}
}

enum SECTION
{
	SECTION_ID			= 0,
	SECTION_LINK,
	SECTION_DUPLEX,
	SECTION_SPEED,
	SECTION_UNIT,
	SECTION_NUM,
};

int ssi_get_lan_status(struct request_rec *r, int argc, char **argv)
{
	FILE *fp = NULL;
	int fd;
	char filename[16] = "/tmp/lan_status";
	char cmd[256] = {0};

	char duplex[16] = {0};
	char link[16] = {0};
	int speed =0;

	char buf[256] = {0};
	char *cp = NULL;
	int lanid = 0;

	/* the lan_status file maintain by async_event */
	/* but if the  file not exit, create one and get the status */
	if (access(filename, F_OK) == -1) 
	{ 
		sprintf(cmd, "dmesg |grep link_speed|grep %s|tail -1 > %s", ARC_ETHWAN_INF, filename);
		system(cmd);
	}

	if ((fp = fopen(filename, "r")) != NULL)
	{
		while(fgets(buf, sizeof(buf), fp) != NULL)
		{
			if ((cp = strstr(buf, "link_speed:")) != NULL)
			{
				cp += strlen("link_speed:");
				while (isblank(*cp) && *cp)
					cp++;

				if (cp != NULL)
				{
					speed = atoi(cp);
				}

			}
			if ((cp = strstr(buf, "duplex:")) != NULL)
			{
				/* Duplex: 1 for half, 0 for full. */
				cp += strlen("1uplex:");
				while (isblank(*cp) && *cp)
					cp++;

				if (cp != NULL)
				{
					if (atoi(cp) == 1)
						strncpy(duplex, "Full", sizeof(duplex));
					else if (atoi(cp) == 0)
						strncpy(duplex, "Half", sizeof(duplex));
				}

				continue;
			}
		}

		fclose(fp);
	}

	/* lan_status file not convert the speed nor show the link status
	 * Linkspeed: 10/100/1000 Mbis/s. 
         */
	if (speed > 0)
	{
		speed /= 1000000;

		strncpy(link, "Up", sizeof(link));
	}
	else
	{
		strncpy(link, "Down", sizeof(link));
	}

	so_printf(r, "['%d','%s','%d Mbits','%s'],", lanid, link, speed, duplex);

	return 0;
}

int ssi_get_dsl_status(struct request_rec *r, int argc, char **argv)
{
	FILE *fp = NULL;
	int fd;
	char filename[16] = "/tmp/dsl_status";
	char cmd[256] = {0};
	char buf[256] = {0};
	char strupspeed[20] = {0};
	char strdownspeed[20] = {0};
	char *pus = NULL;
	char *pds = NULL;
	int linkstatus = 0;
	int upspeed = 0;
	int downspeed = 0;
	int size = 0;

	sprintf(cmd, "bcm_xdslctl info|grep Bearer|awk  '{print \"us=\" $6 \" ds=\" $11}' > %s", filename);
	system(cmd);

	if ((fp = fopen(filename, "r")) != NULL)
	{
		while(fgets(buf, sizeof(buf), fp) != NULL)
		{
			if ((pus = strstr(buf, "us=")) != NULL && (pds = strstr(buf, "ds=")) != NULL)
			{
				size = pds - (pus + 3);
				if(size > sizeof(strupspeed))
				{
					size = sizeof(strupspeed);
				}
				
				strncpy(strupspeed, pus+3, size);
				upspeed = atoi(strupspeed);

				strncpy(strdownspeed, pds+3, sizeof(strdownspeed));
				downspeed = atoi(strdownspeed);

				linkstatus = 1;
			}
			continue;
		}

		fclose(fp);
	}

	so_printf(r, "['dsl','%d', '%d','%d'],", linkstatus, upspeed, downspeed);

	return 0;
}

#define DISK_DEV_FILE "/tmp/disk_dev"
#define DISK_SIZE_FILE "/tmp/disk_size"
/*#define GET_DISK_DEV_CMD "ls -1 /dev/|sed -n '/^sd[a-z]*$/p'|awk '{printf(\"%%s,\", $1)}' > "DISK_DEV_FILE*/
int ssi_get_disk_info(struct request_rec *r, int argc, char **argv)
{
	char cmd[128] = {0x00};
	FILE *fp = NULL;
	FILE *fp1 = NULL;
	char disk_dev[8] = {0x00};
	char disk_info[64] = {0x00};
	char line[256] = {0};
	char *ch = NULL;
	snprintf(cmd, sizeof(cmd), "ls -1 /dev/|sed -n '/^sd[a-z]\\{1,\\}$/p' > %s", DISK_DEV_FILE);
	system(cmd);

	//get vendor & model info
	if(fp1 = fopen(DISK_DEV_FILE, "r"))
	{
		while(fgets(disk_dev, sizeof(disk_dev), fp1) != NULL)
		{
			unlink(DISK_SIZE_FILE);

			disk_dev[strlen( disk_dev ) - 1] = '\0';
			so_printf(r, "[\"%s\", ", disk_dev);

			memset(disk_info,'\0',sizeof(disk_info));
			memset(cmd,'\0',sizeof(cmd));
			memset(line,'\0',sizeof(line));

			/* william_chen 2014.3.4 
			 * #1641 Disk information can not display on webpage usb.htm 
			 * In 388, usb device information is recorded in /proc/arcusb/status.storage
			 */
			snprintf(cmd, sizeof(cmd), "/proc/arcusb/status.storage");
			if((fp = fopen(cmd, "r")))
			{
				while(fgets(line, sizeof(line), fp) != NULL)
				{
					if((strstr(line, "vendor") != NULL) \
						&& (strstr(line, disk_dev) != NULL))
					{
						if((ch = strchr(line, ':')) != NULL)
						{
							strcat(disk_info, ch+2);
							if((ch = strchr(disk_info, '\n')) != NULL)
							{
								*ch = '\0';
							}

							so_printf(r, "\"%s\", ", disk_info);
						}
					}

					memset(disk_info,'\0',sizeof(disk_info));	
					if((strstr(line, "model") != NULL) \
						&& (strstr(line, disk_dev) != NULL))
					{
						if((ch = strchr(line, ':')) != NULL)
						{
							strcat(disk_info, ch+2);
							if((ch = strchr(disk_info, '\n')) != NULL)
							{
								*ch = '\0';
							}

							so_printf(r, "\"%s\", ", disk_info);
						}
					}
				}

				fclose(fp);
			}
			/* end william_chen 2014.3.4 */

			memset(disk_info,'\0',sizeof(disk_info));
			memset(cmd,'\0',sizeof(cmd));
			memset(line,'\0',sizeof(line));
			snprintf(cmd, sizeof(cmd), "fdisk -l |sed -n '/\\/dev\\/%s:/p'|awk -F \",\" '{print $2}' > %s", disk_dev, DISK_SIZE_FILE);
			system(cmd);
			if((fp = fopen(DISK_SIZE_FILE, "r")))
			{
				if(fgets(line, sizeof(line), fp) != NULL)
				{
					strcat(disk_info, line);
				}
				if((ch = strchr(disk_info, '\n')) != NULL)
				{
					*ch = '\0';
				}
				so_printf(r, "\"%s\"],", disk_info);
				fclose(fp);
			}
		}
		fclose(fp1);
	}

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
