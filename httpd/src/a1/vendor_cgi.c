#include "httpd.h"
#include "cgi.h"
#include "httoken.h"
#include "arc_proj_table.h"

// 128 may enough
#define SYSTEM_PASSWORD_SIZE 256
#include "arc_tr64_table.h"
#include "arc_sys_table.h"
#include "arc_ui_table.h"

#ifdef CONFIG_HTTPD_SUPPORT_LOGIN_SHA512
#include "sha512.h"
#endif //CONFIG_HTTPD_SUPPORT_LOGIN_SHA512
#include "md5.h"

#include "user_session.h"
#ifdef SYSLOG_ENHANCED
int login_error_times = 0;
#endif
extern input_t inputs[MAX_CGI_VARS];
extern int input_num;

extern char* Get_MD5_Str(char* buffer, char* result);
extern int get_login_timeout(void);
extern int is_http_logined(void);

#ifdef CHECK_LOGIN_BROWSER_SUPPORT
extern void grant_user(sock_t ip, char *agent);
#else
extern void grant_user(sock_t ip);
#endif
void sys_setpwd_cgi(int argc, char **argv, void *p);
void wz_setpwd_cgi(int argc, char **argv, void *p);


void rescan_cgi(int argc, char **argv, void *p);
void frescan_cgi(int argc, char **argv, void *p);
void login_cgi(int argc, char **argv, void *p);
void logout_cgi(int argc, char **argv, void *p);
void lte_settings_cgi(int argc, char **argv, void *p);
//void apply_sipp_cgi(int argc, char **argv, void *p);
void wl_cal_cgi(int argc, char **argv, void *p);
void toplogy_cgi(int argc, char **argv, void *p);
void get_logfile_cgi(int argc, char **argv, void *p);
void set_led_cgi(int argc, char **argv, void *p);
void set_access_control_cgi(int argc, char **argv, void *p);
void set_status_station_cgi(int argc, char **argv, void *p);
void set_control_internet_cgi(int argc, char **argv, void *p);
void fwcheck_cgi(int argc, char **argv, void *p);
void change_lang_cgi(int argc, char **argv, void *p);
void set_wifi_config_for_sche(int argc, char **argv, void *p);
void set_wifi_config_pid(int argc, char **argv, void *p);


/*
 * CGI Table: translate script's filename to its task function, all tasks can
 *      be assigned a different stack size, respectively
 */
const CGI_ENTRY cgi_tbl[] =
{
	/* common, always need */
	{"apply.cgi",	TASK_t apply_cgi, 	TASK_STKSIZ},
	{"upload.cgi",	TASK_t upload_cgi, TASK_STKSIZ},
	{"login.cgi",	TASK_t login_cgi, 	TASK_STKSIZ},
	{"logout.cgi",	TASK_t logout_cgi, 	TASK_STKSIZ},

	{"setpwd.cgi",	TASK_t sys_setpwd_cgi,	TASK_STKSIZ},
	{"wz_setpwd.cgi",	TASK_t wz_setpwd_cgi,	TASK_STKSIZ},
	/* cgi for Abstract layer set */
	{"apply_abstract.cgi",	TASK_t apply_abstract_cgi, 	TASK_STKSIZ},

	/* cgi for SIP Proxy */
//	{"apply_sipp.cgi",	TASK_t apply_sipp_cgi, 	TASK_STKSIZ},


	/* VOIP use apply.cgi */

	/* LTE */
	{"lte_settings.cgi",	TASK_t lte_settings_cgi, 	TASK_STKSIZ},

	/*change language */
	{"chglang.cgi",	TASK_t change_lang_cgi, 	TASK_STKSIZ},

	/* Wireless calibration */
	{"wl_cal.cgi",	TASK_t wl_cal_cgi, 	TASK_STKSIZ},
	{"toplogy.cgi",	TASK_t toplogy_cgi, 	TASK_STKSIZ},	
	{"get_logfile.cgi",	TASK_t get_logfile_cgi, 	TASK_STKSIZ},
	{"set_led.cgi",	TASK_t set_led_cgi, 	TASK_STKSIZ},
	{"set_access_control.cgi",	TASK_t set_access_control_cgi, 	TASK_STKSIZ},
	{"set_wifi_config_for_sche.cgi",	TASK_t set_wifi_config_for_sche, 	TASK_STKSIZ},
	{"set_wifi_config_pid.cgi",	TASK_t set_wifi_config_pid,	TASK_STKSIZ},
	{"fwcheck.cgi",	TASK_t fwcheck_cgi, TASK_STKSIZ},
	{"set_status_station.cgi", TASK_t set_status_station_cgi, 	TASK_STKSIZ},
	{"set_control_internet.cgi", TASK_t set_control_internet_cgi,	TASK_STKSIZ},
	{"rescan.cgi",	TASK_t rescan_cgi, TASK_STKSIZ},
	{"frescan.cgi",	TASK_t frescan_cgi, TASK_STKSIZ},
	{NULL, 			(void (*)()) 0, 	0}
};




#define SKIP_PASSWORD_VAR	""
#define	FAKE_PASSWORD	"d6nw5v1x2pc7st9m"

int separate_string_to_array( char *pszInput, char *pszDelimiters , unsigned int Ary_num, unsigned int Ary_size, char *pszAry_out)
{
    //char *pszData = strdup(pszInput);
    char pszData[2048]={0};
    strcpy(pszData, pszInput);
    char *pszToken = NULL, *pszToken_saveptr;
    unsigned int Ary_cnt = 0;

    pszToken = strtok_r(pszData, pszDelimiters, &pszToken_saveptr);
    while( pszToken!=NULL)
    {
        //printf("pszToken=%s\n", pszToken);
        memcpy( pszAry_out + Ary_cnt*Ary_size, pszToken, Ary_size);
        if( ++Ary_cnt >= Ary_num)
            break;
        pszToken = strtok_r( NULL, pszDelimiters, &pszToken_saveptr);
    }
    //free(pszData);

    return Ary_cnt;
}



int check_backup_config_MD5(const char *filename){

	FILE *fp, *fp_out;
	int ret = -1;
	int i, j, ch;
	char line[MD5_length*2+1];
	char md5_sum[MD5_length];
	char md5_checksum[MD5_length];
	char buffer[256];
	char *buf = (char *)malloc(MD5_length);

	// delete MD5 checkum in Settings_DSL-AC87U.CFG and read the MD5 checkum
	fp = fopen(filename, "r");
	fp_out = fopen("/tmp/.config.tmp", "w");
	if( fp == NULL || fp_out == NULL ) return ret;
	i = 1;
	while( !feof(fp) ){
	 	if( fgets(line, MD5_length*2+1, fp) != NULL){
			if( !strcmp(line, "================================================================") ){ 
				//cprintf("check_backup_config_MD5: end of file.\n");
				break;
			}
	 		if( !strcmp(line, "\n") ) {
				continue;
	 		}
	 		if( i == MD5_location+1 ){
				for( j = 0 ; j < MD5_length ; j++) buf[j] = line[j+MD5_length];
				//cprintf("line=%s\nbuf=%s\n", line, buf);
				i++;
	 			continue;
			} 
			fprintf(fp_out, "%s\n", line);
			i++;
		}
	}
	fclose(fp);
	fclose(fp_out);
	snprintf(md5_sum, sizeof(md5_sum), buf);
	//cprintf("check_backup_config_MD5 md5_sum: %s.\n", md5_sum);
	memset(buf, 0, MD5_length);

	snprintf(buffer, sizeof(buffer), "sed -i '$d' %s", "/tmp/.config.tmp");
	system(buffer);

	// check the validity of the upload file
	snprintf(buffer, sizeof(buffer), "md5sum %s > /tmp/config.md5", "/tmp/.config.tmp");
	system(buffer);	
	
	fp = fopen("/tmp/config.md5", "r");
	if( fp == NULL) return ret;
	for (i = 0 ; i < MD5_length && isxdigit(ch = fgetc(fp)) ; i++) 	buf[i] = (char)ch;
	fclose(fp);

	snprintf(md5_checksum, sizeof(md5_checksum), buf);
	//cprintf("check_backup_config_MD5 md5_checksum: %s.\n", md5_checksum);
	
	if( strcmp(md5_sum, md5_checksum) == 0 ){
		//cprintf("check_backup_config_MD5 The file has NOT been modified.\n");
		snprintf(buffer, sizeof(buffer), "mv %s %s", "/tmp/.config.tmp", filename);
		system(buffer);
		ret = 1;
	}
	else{ 
		//cprintf("check_backup_config_MD5 The file has been modified.\n");
		ret = 0;
	}
	
	return ret;
}

int decrypt_conf_file(const char *in, const char *out)
{
	char cmd[1024];
	FILE *fp = NULL;
	char enc_method[17] = "";
	int tid;
	
	if((tid = get_tid()) == MID_FAIL) {
		return -1;
	}
	
	/* Adam 20151106, Add MD5 into backup configuration file.*/
	if(check_backup_config_MD5(in))
		cprintf("check_md5 checksum success.\n");
	else{
		cprintf("check_md5 checksum fail.\n");
		return -1;
	}

	arc_cfg_get(tid, ARC_PROJ_ASUS_HTTPD_CONFIG_SSL_EncrptType, enc_method, sizeof(enc_method), "0");
 
	snprintf(cmd, sizeof(cmd), "openssl enc -%s -salt -d -a -in %s -out /tmp/.config.tgz -k %s", enc_method, in, CONFIG_BACKUP_KEY);
	system (cmd);
	system("mkdir -p /tmp/.config");
	snprintf(cmd, sizeof(cmd), "tar xzf /tmp/.config.tgz -C /tmp/.config");
	
	fp = popen(cmd, "r");
	if (fp != NULL) {
		pclose(fp);
		if (0 == access("/tmp/.config/config/.glbcfg",0))               //by seal to fix restore issue 150303
	{
		system("rm /tmp/.config/config/glbcfg.dft");
		system("cp -af /tmp/.config/config/. /etc/config; rm -rf /tmp/.config");

		snprintf(cmd, sizeof(cmd), "cp /etc/config/.glbcfg %s", out);
		system(cmd);
		system("arccfg commit");

		return 0;
		}
	}
	else
	{
		system("rm -rf /tmp/.config");
		return 1;
	}
}

int is_fake_passwd(const char *name, const char *value)
{
	if (strstr(SKIP_PASSWORD_VAR, name) && strcmp(value, FAKE_PASSWORD)==0 )
		return 1;

	return 0;
}

int skip_var(input_t *input_var)
{
	return is_fake_passwd(input_var->name, input_var->val);
}


static int translate_http_passwd(const char *old_value, char *new_value, int size, int index)
{
	char key[8] = "wg7005d";
	int k_idx = 0, j, ri = 0;
	char *s;
	char tmp[64][4];
	char http_md5_passwd[33];
	char system_pwd[128];
	int tid;

	if((tid = get_tid()) == MID_FAIL)
		return -1;


	//printf("%s(%d)\n", __FUNCTION__, __LINE__);
	//

	if (old_value == NULL || new_value == NULL)
		return -1;

	strncpy(new_value, old_value, size);
	new_value[size] = '\0';
	s = strtok(new_value, " ");
	while(s)
	{
		strcpy(tmp[ri++], s);
		s = strtok(NULL, " ");
	}
	for(j = 0; j < ri; j++)
	{
		if (j > size)
			return -1;
		new_value[j] = atoi(tmp[j]) - key[k_idx];
		k_idx = (k_idx + 1) % strlen(key);
	}
	new_value[j] = 0;

	ht_dbg("old_value[%s], new_value[%s]\n",old_value, new_value);
	/*
	   if (!valid_name(wp, new_value, v)) return;
	   */

	/* User didn't modify the password */
	if(index == 1)
		mapi_ccfg_get_str(tid, "ARC_SYS_Password", system_pwd, sizeof(system_pwd));
	else if(index == 2)
		mapi_ccfg_get_str(tid, "ARC_SYS_Password2", system_pwd, sizeof(system_pwd));
	
#if defined(WE410443_SA) || defined(WE410443_A1) || defined(WE410443_6DX)
	Get_MD5_Str(system_pwd, http_md5_passwd);
	if( memcmp(http_md5_passwd, new_value, 33) == 0 )
		return -2;
#endif
	return 0;
}

int hack_var(int tid, input_t *input_var)
{
	/* value[] contains 128 characters and 1 NULL-character at most.
	 * For example, when "ARC_SYS_Password"=5566 in glbcfg,
	 * inputs[i].val = "217 151 154 103 145 154 150 170 152 109 147 103 154 156 169 152 107 150 148 107 153 176 204 107 146 147 109 197 167 203 156 145 ";      
	 * NOTE: there is a space in the end.
	 * strlen(inputs[i].val) = 128;  // 32 * 4 = 128;
	 */
	char value[32*4 + 1];  /* 32 is the length of md5sum . 4 is the length of the string "xxx ". */

	/* translate http passwd */
	if (!strcmp(input_var->name, "ARC_SYS_Password")) {
		if (translate_http_passwd(input_var->val, value, sizeof(value) - 1, 1))
			return -1;
		return mapi_ccfg_set_str(tid, input_var->name, value);
	}
	else if (!strcmp(input_var->name, "ARC_SYS_Password2")) {
		if (translate_http_passwd(input_var->val, value, sizeof(value) - 1, 2))
			return -1;
		return mapi_ccfg_set_str(tid, input_var->name, value);
	}
	return 1;
}

void fwcheck_cgi(int argc, char **argv, void *p)
{
	CGI_info *ci = (CGI_info *)p;
	struct request_rec *r = ci->r;
	char httprequest[2048] = {0};
	char recvline[2048] = {0};
	char jasonbuf[1024] = {0};
	char cmdbuf[128] = {0};
	int tid;
	char system_pwd[256] = {0};
	if((tid = get_tid()) == MID_FAIL)
		return;
	mapi_tmp_set(tid, "ARC_SYSTEM_AUTOFW_SOFTVERSION", "");
	mapi_tmp_set(tid, "ARC_SYSTEM_AUTOFW_VERSION", "");
	mapi_tmp_set(tid, "ARC_SYSTEM_AUTOFW_PATH", "");
	mapi_tmp_set(tid, "ARC_SYSTEM_AUTOFW_MD5", "");
	mapi_tmp_set(tid, "ARC_SYSTEM_AUTOFW_FILENAME", "");
	mapi_ccfg_get_str(tid, "ARC_SYS_Password", system_pwd, sizeof(system_pwd));

	FW_OPT_IN FwAutoOptIn = OPT_IN_NO;
	FW_CHECK_DEVICE FwCheckDevice = CHECK_DEVICE_WEB;
	FW_CHECK_TYPE FwCheckType = CHECK_TYPE_MANUAL;
	sprintf(jasonbuf,"{\"password_check\":\"%s\",\"action\":\"%d\"}", system_pwd, URL_FLAG(FwAutoOptIn, FwCheckDevice, FwCheckType));

//	cprintf("jasonbuf:%s\n", jasonbuf);
	sprintf(httprequest,"POST /tools/trigger_FW_upgrade_check HTTP/1.1\r\nContent-Type:application/json\r\nContent-Length:%d\r\nConnection:Keep-Alive\r\n\r\n%s",strlen(jasonbuf),jasonbuf);
//	cprintf("httprequest:%s\n", httprequest);	
	connection_with_owl(httprequest, recvline , 10240); 
//	cprintf("recvline:%s\n", recvline);	
	
	answer(r, DOCUMENT_FOLLOWS, NOTHING);
	if (argv)
		free(argv);
	
	pthread_exit((void *)CGI_CODE);
}

/*
 * paramters
 *   "pws": password
 *   "url": next jump URL if sucess
 *
 * return:
 *
 *
 *
 */

/* -1: session timeout to login again
 * -2: password error
 * -3: duplicate error
 */
// get current system password value
//    NOTE:
//       1. no enable CONFIG_HTTPD_SUPPORT_LOGIN_SHA512, system stor a visible value (like: admin)
//          API will return MD5 format (32 characters)
//       2. enable CONFIG_HTTPD_SUPPORT_LOGIN_SHA512, API return MD5+SHA512 encode. a 128 characters return
//          and also currect CFG's value to 128 characters format...
//
//
//
// tid: current session
// userid: uyser index (N/A)
// pwd: return a MUST 129 buffer at least allocated by caller
//
#define DEFAULT_SYSTEM_PWD	"1"
char *get_user_passwd(int tid, int userid, char *system_pwd)
{
	char http_md5_passwd[32+1];
	int pwd_sz=0;
	char szTr64H1Input[34+SYSTEM_PASSWORD_SIZE]="", szTr64H1[32+1]="";

#ifdef CONFIG_HTTPD_SUPPORT_LOGIN_SHA512
  SHA512Context foo;
  uint8_t hash[SHA512_HASH_SIZE];
  int i;
#endif //CONFIG_HTTPD_SUPPORT_LOGIN_SHA512

	//mapi_ccfg_get_str(tid, "ARC_SYS_Password", system_pwd, SYSTEM_PASSWORD_SIZE);
	arc_cfg_get(tid, userid, system_pwd, SYSTEM_PASSWORD_SIZE, DEFAULT_SYSTEM_PWD);
#ifdef HTTPD_SUPERUSER_SUPPORT	
	//Fix that the previous f/w does not have  ARC_SYS_GUI_SUPERUSER_NAME and ARC_SYS_GUI_SUPERUSER_PASSWORD.
	if(userid == ARC_SYS_GUI_SUPERUSER_NAME || userid == ARC_SYS_GUI_SUPERUSER_PASSWORD)
	{
		if (strcmp(system_pwd, DEFAULT_SYSTEM_PWD) == 0)
		{
			strcpy(system_pwd, "test");	//default value of superuser is "test/test"
		}
	}
#endif	
	//
	// backword compare here
	// if old is not 32 or 128 convert it
	pwd_sz = strlen(system_pwd);

	if (pwd_sz == 0)
		return (char *)system_pwd;

#ifndef CONFIG_HTTPD_SUPPORT_LOGIN_SHA512
	Get_MD5_Str(system_pwd, http_md5_passwd);
	// copy result into a buffer to compare
	// Why? if we turn on CONFIG_HTTPD_SUPPORT_LOGIN_SHA512, also compre at same buffer
	//
	strcpy_guard(system_pwd, 33, http_md5_passwd);

#else //! CONFIG_HTTPD_SUPPORT_LOGIN_SHA512

	// backword value to store MD5+SHA512 her
	if(pwd_sz != 128)
	{
		//for TR64 user authentication HA1+++
		if(userid == ARC_SYS_Password)
		{
		sprintf(szTr64H1Input, "dslf-config:DSLFORUM DSLF-CONFIG:%s", system_pwd);
		Get_MD5_Str(szTr64H1Input, szTr64H1);
		arc_cfg_set(tid, ARC_TR64_ConfigPasswd, szTr64H1);
		}
		//for TR64 user authentication HA1---
#if defined(WE410443_SA) || defined(WE410443_A1) || defined(WE410443_6DX)
		Get_MD5_Str(system_pwd, http_md5_passwd);
#endif
		SHA512Init (&foo);
#if defined(WE410443_SA) || defined(WE410443_A1) || defined(WE410443_6DX)
		SHA512Update (&foo, http_md5_passwd, strlen(http_md5_passwd));
#else
		SHA512Update (&foo, system_pwd, strlen(system_pwd));
#endif
		SHA512Final (&foo, hash);
		system_pwd[0]='\0';

		for (i = 0; i < SHA512_HASH_SIZE;)
		{
			sprintf(system_pwd, "%s%02x",system_pwd, hash[i++]);
		}

		arc_cfg_set(tid, userid, system_pwd);
		arc_cfg_commit(tid);
	}
#endif
	return (char *)system_pwd;
}

#define SET_PASSWD_EMPTY 0
#define SET_PASSWD_FAIL -1
#define SET_PASSWD_SUCCESS 1

int check_and_setpwd(int tid , char *name, char *newpasswd, char *oldpasswd, char *tr64h1)
{
	char http_md5_passwd[SYSTEM_PASSWORD_SIZE], http_md5_name[SYSTEM_PASSWORD_SIZE];
	char http_md5_remote_passwd[SYSTEM_PASSWORD_SIZE], http_md5_remote_name[SYSTEM_PASSWORD_SIZE]; 

	get_user_passwd(tid, ARC_SYS_LoginName, http_md5_name); // system_password will return MD5 format or SHA512 format
	get_user_passwd(tid, ARC_SYS_Password, http_md5_passwd); // system_password will return MD5 format or SHA512 format
	get_user_passwd(tid, ARC_UI_WEB_REMOTEMN_Name, http_md5_remote_passwd); // system_password will return MD5 format or SHA512 format
	get_user_passwd(tid, ARC_UI_WEB_REMOTEMN_Password, http_md5_remote_name); // system_password will return MD5 format or SHA512 format

	//ht_dbg("##########login_cgi: ARC_SYS_Password =%s \n", http_md5_passwd );

	if((name && !strcmp(http_md5_name, name))
		&& (oldpasswd && !strcmp(http_md5_passwd, oldpasswd))){ /* Password is correct. */

		arc_cfg_set (tid, ARC_SYS_Password, newpasswd);
		arc_cfg_set (tid, ARC_TR64_ConfigPasswd, tr64h1);
		arc_cfg_commit(tid);

		return SET_PASSWD_SUCCESS;
	}else if((name && !strcmp(http_md5_remote_name, name))
		&& (oldpasswd && !strcmp(http_md5_remote_passwd, oldpasswd))){ /* remote Password is correct. */

		arc_cfg_set (tid, ARC_UI_WEB_REMOTEMN_Password, newpasswd);
		arc_cfg_set (tid, ARC_TR64_ConfigPasswd, tr64h1);
		arc_cfg_commit(tid);

		return SET_PASSWD_SUCCESS;
	}

	return SET_PASSWD_FAIL;
}

int check_pwd(int tid ,char *oldpasswd)
{
	char http_md5_passwd[SYSTEM_PASSWORD_SIZE];

	get_user_passwd(tid, ARC_SYS_Password, http_md5_passwd); // system_password will return MD5 format or SHA512 formatt

	//ht_dbg("##########login_cgi: ARC_SYS_Password =%s \n", http_md5_passwd );

	if(oldpasswd && !strcmp(http_md5_passwd, oldpasswd)){ /* Password is correct. */
		return SET_PASSWD_SUCCESS;
	}

	return SET_PASSWD_FAIL;
}

void sys_setpwd_cgi(int argc, char **argv, void *p)
{
	CGI_info *ci = (CGI_info *)p;
	struct request_rec *r = ci->r;
	char *newpasswd = NULL;
	char *oldpasswd = NULL;
	char *name = NULL;
	char *tr64h1 = NULL;
	char *next_page=HTTP_APPLY_FAIL;
	int cfgID = 0, idx1 = -1, idx2 = -1, enc=0;

	char system_pwd[128];//SYSTEM_PASSWORD_SIZE]; // we at least 128 to compare old and newer
	int tid;
	int rtn_code = SET_PASSWD_EMPTY, i = 0, ntp = 0, remote = 0;

	char buf[1024];

	if((tid = get_tid()) == MID_FAIL){
	        goto setpwd_done;
	}
	mapi_tmp_set(tid, "ARC_SYSTEM_SETPWD_NOWAIT", "0");
	mapi_tmp_set(tid, "ARC_SYSTEM_SETPWD_RESULT", "0");
	/* Destory last cgi table */
	if ((input_num = __destroy_cgi(inputs, input_num)) != 0)
	        ht_dbg("warning: %d entries have not been freed, memory leaking.. \n", input_num);

	input_num = __init_cgi(ci, inputs, sizeof(inputs)/sizeof(input_t));
#ifdef CONFIG_HTTPD_TOKEN_CHECK_SUPPORT
	if ( !cgi_token_pass(ci, inputs, 0) ) //failed to pass token checking
	{
	        goto setpwd_done;
	}
#endif

	for (i=0; i<input_num; i++)
	{
		//ht_dbg("input.name=[%s], input.val=[%s]\n", inputs[i].name, inputs[i].val);
		if (!inputs[i].name || !inputs[i].val) {
			sprintf(next_page, "%s",  HTTP_APPLY_FAIL); //"Fail");
			goto setpwd_done;
		}

		if (!strcmp(inputs[i].name, "chk"))
		{
			oldpasswd= inputs[i].val;
		}
		else if (!strcmp(inputs[i].name, "pws"))
		{
			newpasswd = inputs[i].val;
		}
		else if (!strcmp(inputs[i].name, "usr"))
		{
			name = inputs[i].val;
		}
		else if (!strcmp(inputs[i].name, "tr64h1"))
		{
			//tr64h1
			tr64h1 = inputs[i].val;

		}
	}

	cprintf("newpasswd = %s,  oldpasswd=%s\n", newpasswd, oldpasswd);
	if (oldpasswd != NULL)
		rtn_code = check_pwd(tid, oldpasswd);
	

	// show CGI result:  success or Fail
	switch (rtn_code)
	{
			case SET_PASSWD_SUCCESS:
				mapi_tmp_set(tid, "ARC_SYSTEM_SETPWD_NOWAIT", "1");
				mapi_tmp_set(tid, "ARC_SYSTEM_SETPWD_RESULT", "1");
				break;
			default:
				mapi_tmp_set(tid, "ARC_SYSTEM_SETPWD_NOWAIT", "1");
				mapi_tmp_set(tid, "ARC_SYSTEM_SETPWD_RESULT", "0");
				break;
	}

	answer(r, DOCUMENT_FOLLOWS, NOTHING);

setpwd_done:

	if (argv)
		free(argv);
	pthread_exit((void *)CGI_CODE);

}


void wz_setpwd_cgi(int argc, char **argv, void *p)
	{
		CGI_info *ci = (CGI_info *)p;
		struct request_rec *r = ci->r;
		char *newpasswd = NULL;
		char *oldpasswd = NULL;
		char *tr64h1 = NULL;
		char *name = NULL;
		char *next_page=HTTP_APPLY_FAIL;

		char system_pwd[128];//SYSTEM_PASSWORD_SIZE]; // we at least 128 to compare old and newer
		int tid;
		int rtn_code = SET_PASSWD_FAIL, i = 0;

		char buf[1024];

		if((tid = get_tid()) == MID_FAIL){
				goto setpwd_done;
		}

		/* Destory last cgi table */
		if ((input_num = __destroy_cgi(inputs, input_num)) != 0)
				ht_dbg("warning: %d entries have not been freed, memory leaking.. \n", input_num);

		input_num = __init_cgi(ci, inputs, sizeof(inputs)/sizeof(input_t));
#ifdef CONFIG_HTTPD_TOKEN_CHECK_SUPPORT
		if ( !cgi_token_pass(ci, inputs, 0) ) //failed to pass token checking
		{
				goto setpwd_done;
		}
#endif

		for (i=0; i<input_num; i++)
		{
			//ht_dbg("input.name=[%s], input.val=[%s]\n", inputs[i].name, inputs[i].val);
			if (!inputs[i].name || !inputs[i].val) {
				sprintf(next_page, "%s",  HTTP_APPLY_FAIL); //"Fail");
				goto setpwd_done;
			}

			if (!strcmp(inputs[i].name, "chk"))
			{
				oldpasswd= inputs[i].val;
			}
			else if (!strcmp(inputs[i].name, "pws"))
			{
				newpasswd = inputs[i].val;
			}
			else if (!strcmp(inputs[i].name, "usr"))
			{
				name = inputs[i].val;
			}
			else if (!strcmp(inputs[i].name, "tr64h1"))
			{
				tr64h1 = inputs[i].val;
			}

		}

		ht_dbg("newpasswd = %s,  oldpasswd=%s\n", newpasswd, oldpasswd);

		rtn_code = check_and_setpwd(tid, name, newpasswd, oldpasswd, tr64h1);

		// show CGI result:  success or Fail
		if (rtn_code == SET_PASSWD_SUCCESS)
			snprintf(buf, sizeof(buf), "Location: /%s\n\n", "QIS_detect.htm" );
		else
			snprintf(buf, sizeof(buf), "Location: /%s\n\n", "QIS_admin_pass.htm?err=1" );

		send_response(r, buf);


	setpwd_done:

		if (argv)
				free(argv);

		pthread_exit((void *)CGI_CODE);

	}



/*
 * paramters
 *   "pws": password
 *   "url": next jump URL if sucess
 *
 * return:
 *
 *
 *
 */

/* -1: session timeout to login again
 * -2: password error
 * -3: duplicate error
 */
void set_login_info(LOGIN_INFO *info, int fail)
{
       struct sysinfo now_time;
       sysinfo(&now_time);
       pthread_mutex_lock(&info->lock);
       if(fail)
       {
               info->error = 1;
               info->error_count++;
               info->error_uptime = now_time.uptime;
               if(info->error_count > 4)
                       info->error_wait_time = (info->error_count - 4)*60;
               else
                       info->error_wait_time = 0;              
       }
       else
       {
               info->error = 0;
               info->error_count = 0;
               info->error_uptime = 0;
               info->error_wait_time = 0;
       }
       pthread_mutex_unlock(&info->lock);
}

int check_login_info(LOGIN_INFO *info)
{
       struct sysinfo now_time;
       sysinfo(&now_time);
       int ret = 0;
       pthread_mutex_lock(&info->lock);
       if((info->error == 1) && (info->error_wait_time > 0)
               && (now_time.uptime - info->error_uptime < info->error_wait_time))
               ret = -1;
       else
               ret = 0;
       pthread_mutex_unlock(&info->lock);
 
       return ret;
}
 
 

#define HTTPD_LOGIN_OK        0
#define HTTPD_LOGIN_AGAIN     -1
#define HTTPD_LOGIN_ERROR     -2
#define HTTPD_LOGIN_DUPLICATE -3
#define HTTPD_LOGIN_LIMIT -4

void login_cgi(int argc, char **argv, void *p)
{
	CGI_info *ci = (CGI_info *)p;
	struct request_rec *r = ci->r;
	char *passwd = NULL, *name = NULL;
	char *next_page = NULL;
	char buf[1024];
	char http_md5_passwd[33];
	char http_md5_name[33];
	char http_md5_passwd2[33];
	char system_user[128];
	char system_pwd[128];
	char system_user2[128];
	char system_pwd2[128];
	int tid;
	int do_rediret=HTTPD_LOGIN_AGAIN;
#if  defined(WE410443_SA) || defined(WE410443_A1) || defined(WE5202243_SA)
#ifdef CONFIG_HTTPD_SUPPORT_LOGIN_SHA512	
	SHA512Context foo;
	uint8_t hash[SHA512_HASH_SIZE];
	int i;
	char tmp_val[32];
	char system_pwd_tmp[SYSTEM_PASSWORD_SIZE];
	char system_pwd_tmp2[SYSTEM_PASSWORD_SIZE];
#endif //CONFIG_HTTPD_SUPPORT_LOGIN_SHA512
#endif
	if((tid = get_tid()) == MID_FAIL)
		return;

	/* Destory last cgi table */
	if ((input_num = __destroy_cgi(inputs, input_num)) != 0)
		ht_dbg("warning: %d entries have not been freed, memory leaking.. \n", input_num);

	input_num = __init_cgi(ci, inputs, sizeof(inputs)/sizeof(input_t));

	/*
 	pete_zhang 2013-12-20	ticket-1167

	description: GUI security enhancement
 	*/
#ifdef CONFIG_HTTPD_TOKEN_CHECK_SUPPORT
	if ( !cgi_token_pass(ci, inputs, 0) ) //failed to pass token checking
	{
		do_rediret=HTTPD_LOGIN_AGAIN;
		goto done;
	}
	else
	{
		httoken_entry_remove( HTTOKEN_REMOVE_ALL );//clean previous token when new user login
	}
#endif
	/*end pete_zhang	2013-12-20*/

	name = __get_cgi(inputs, "name");
	passwd = __get_cgi(inputs, "pws");
	next_page = __get_cgi(inputs, "url");

      if(check_login_info(&login_info) < 0)
      {
              do_rediret = HTTPD_LOGIN_LIMIT;
              goto done;
      }
	
	
	get_user_passwd(tid, ARC_SYS_Password, system_pwd);

#if  defined(WE410443_SA)  || defined(WE5202243_SA)	
#ifdef CONFIG_HTTPD_SUPPORT_LOGIN_SHA512
	{
		mapi_tmp_get(tid, "ARC_HTTP_TMP_VAL", tmp_val, sizeof(tmp_val));
#ifdef WE410443_SA
		memset(system_pwd_tmp2, 0, sizeof(system_pwd_tmp2));
		sprintf(system_pwd_tmp2, "%s%s", system_pwd, tmp_val);
		Get_MD5_Str(system_pwd_tmp2, system_pwd_tmp);
#else
		memset(system_pwd_tmp, 0, sizeof(system_pwd_tmp));
		sprintf(system_pwd_tmp, "%s%s", system_pwd, tmp_val);
		//Get_MD5_Str(system_pwd_tmp2, system_pwd_tmp);
#endif
		SHA512Init (&foo);
		SHA512Update (&foo, system_pwd_tmp, strlen(system_pwd_tmp));
		SHA512Final (&foo, hash);
		
		memset(system_pwd, 0, sizeof(system_pwd));
		sprintf(system_pwd, "%02x", hash[0]);
		for (i = 1; i < SHA512_HASH_SIZE;)
		{
			sprintf(system_pwd, "%s%02x",system_pwd, hash[i++]);
		}
	}
#endif
#endif
	
	//if((name && !strcmp(name, system_user)) && (passwd && !strcmp(http_md5_passwd, passwd))) /* Password is correct. */
	if(  passwd && (!strcmp(system_pwd, passwd))) /* Password is correct. */
	{
		/*
		 * Alpha Liu 2012.11.27 issue for PR711AAW
		 * Issue description: GUI did not allow another PC login
		 *										when one PC already logined.
		 * Solution: Allow any PC login when the login password is correct.
		*/
		//if (!is_http_logined()) /* nobody login yet, you can login */
		if (1) /* login anyway */
		/* end Alpha Liu 2012.11.27 issue for PR711AAW */
		{
#ifdef CHECK_LOGIN_BROWSER_SUPPORT		
			grant_user(r->remote_ip, r->user_agent);
#else
			grant_user(r->remote_ip);
#endif
			/*
 			pete_zhang 2013-12-20	ticket-1167

			description: GUI security enhancement
 			*/
#ifdef CONFIG_HTTPD_TOKEN_CHECK_SUPPORT
			//To ensure token timeout after user timeout, so add half minute
			httoken_timeout_set( get_login_timeout() + 30 );
#endif
			/*end pete_zhang	2013-12-20*/
/* KILL ME
			sprintf(buf, "<html>\n"
					"<head>\n"
					"<META http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\" >\n"
					"</head>\n"
					"\n"
					"<script>\n"
					"function reload(){  \n"
					//"top.topFrame.location.href=top.topFrame.location.href; \n"
					//"document.location.href=\"%s\"; \n"
"top.location.href=\"%s\"; \n"
					"}\n"
					"setTimeout(\"reload()\", 900); \n"
					"function SetCookie (name, value) { \n"
					"var argv = SetCookie.arguments; \n"
					"var argc = SetCookie.arguments.length;\n"
					"var expires = (argc > 2) ? argv[2] : null;\n"
					"var path = (argc > 3) ? argv[3] : null;\n"
					"var domain = (argc > 4) ? argv[4] : null;\n"
					"var secure = (argc > 4) ? argv[5] : false;\n"
					"document.cookie = name + \"=\" + escape (value) + ((expires == null) ? \"\" : (\"; expires=\" + expires.toGMTString())) + ((path == null) ? \"\" : (\"; path=\" + path)) + ((domain == null) ? \"\" : (\"; domain=\" + domain));\n"
					"}\n"
					"</script>\n"
					//"</html> \n ", next_page? next_page : "status.htm");
"</html> \n ", next_page? next_page : "index.htm");
			r->status = DOCUMENT_FOLLOWS;
			r->content_type = default_type;
			r->content_length = strlen(buf);
			send_http_header(r);
			so_printf(r, buf);
*/
			do_rediret=HTTPD_LOGIN_OK;
		}
		else /* someone has logined in */
		{
			do_rediret=HTTPD_LOGIN_DUPLICATE;
/* KILL_ME
			#ifndef CONFIG_HTTPD_HOT_FIX_SW1 //KILL_ME hugh 2014/1/14
			snprintf(buf, sizeof(buf), "Location: /login.htm");
			#else
			snprintf(buf, sizeof(buf), "Location: /loginduperr.htm"); //FIXME: hugh 2013/12/12 hard code????
			#endif

			//answer(r, REDIRECT, buf);
			send_response(r, buf);
*/
		}
	}
	else
	{
		/* Fail to auth, redirect to loginpeer.stm */
		do_rediret=HTTPD_LOGIN_ERROR;
/* KILL_ME
		#ifndef CONFIG_HTTPD_HOT_FIX_SW1 //KILL_ME hugh 2014/1/14
		snprintf(buf, sizeof(buf), "Location: /login.htm");
		#else
		snprintf(buf, sizeof(buf), "Location: /loginpserr.htm"); //FIXME: hugh 2013/12/12, it should be login err??
		#endif
		//answer(r, REDIRECT, buf);
		send_response(r, buf);
*/
	}

done:
	switch(do_rediret)
	{
		case HTTPD_LOGIN_OK: //0
			snprintf(buf, sizeof(buf), "Location: /index.htm");
#ifdef SYSLOG_ENHANCED
			login_error_times = 0;
			SetSystemLogFmt(LOG_TYPE_APP, LOG_LEVEL_TYPE_INFO, LOG_APP_MESSAGE_TYPE_http, "HTTP-User login from %s.", r->remote_ipstr);
#endif
			set_login_info(&login_info, HTTPD_LOGIN_OK);
			break;
		default:
		case HTTPD_LOGIN_AGAIN: //-1
			snprintf(buf, sizeof(buf), "Location: /login.htm");
#ifdef SYSLOG_ENHANCED
			SetSystemLogFmt(LOG_TYPE_APP, LOG_LEVEL_TYPE_INFO, LOG_APP_MESSAGE_TYPE_http, "HTTP-User login  again from %s.", r->remote_ipstr);
#endif
			break;
		case HTTPD_LOGIN_ERROR: //-2
			snprintf(buf, sizeof(buf), "Location: /loginpserr.htm");
#ifdef SYSLOG_ENHANCED
			login_error_times++;
			SetSystemLogFmt(LOG_TYPE_APP, LOG_LEVEL_TYPE_ERROR, LOG_APP_MESSAGE_TYPE_http, "HTTP-User Login Error %d times from %s.", login_error_times, r->remote_ipstr);
#endif
			set_login_info(&login_info, HTTPD_LOGIN_ERROR);
			break;
		case HTTPD_LOGIN_DUPLICATE: //-3
			snprintf(buf, sizeof(buf), "Location: /loginduperr.htm");
#ifdef SYSLOG_ENHANCED
			SetSystemLogFmt(LOG_TYPE_APP, LOG_LEVEL_TYPE_INFO, LOG_APP_MESSAGE_TYPE_http, "HTTP-There is other user on accessing , the login %s login duplicate", r->remote_ipstr);
#endif
			break;
			
		case HTTPD_LOGIN_LIMIT: //-4
			snprintf(buf, sizeof(buf), "Location: /login.htm");
			break;
	}
	send_response(r, buf);

	if (argv)
		free(argv);

	pthread_exit((void *)CGI_CODE);
}


void clear_granted_user(struct request_rec *r);

void logout_cgi(int argc, char **argv, void *p)
{
	CGI_info *ci = (CGI_info *)p;
	struct request_rec *r = ci->r;
	char content[SOPRINTFBUFSIZE*2];
	int i, len;
	char buf[1024];

	/* Destory last cgi table */
	if ((input_num = __destroy_cgi(inputs, input_num)) != 0)
		ht_dbg("warning: %d entries have not been freed, memory leaking.. \n", input_num);

	input_num = __init_cgi(ci, inputs, sizeof(inputs)/sizeof(input_t));

	/*
 	pete_zhang 2013-12-20	ticket-1167

	description: GUI security enhancement
 	*/
#ifdef CONFIG_HTTPD_TOKEN_CHECK_SUPPORT
	if ( !cgi_token_pass(ci, inputs, 0) ) //failed to pass token checking
	{
		goto done;
	}
#endif
	/*end pete_zhang	2013-12-20*/

	len = r->content_length;
#if 0	//the content has been read in __init_cgi(), so needn't recv() agian here.
	/* dummy read */
	while (len > 0)
	{
#ifdef CONFIG_HTTPD_HTTPS_SUPPORT
		if (ci->r->ishttps) {
			i = as_read(ci->r->ssrv, content, sizeof(content));
		}
		else
#endif
			i = recv(ci->socketid, content, sizeof(content), MSG_DONTWAIT);

		if (i <=0 )
			break;

		len -= i;
	}
#endif

	clear_granted_user(r);

	/*
 	pete_zhang 2013-12-20	ticket-1167

	description: GUI security enhancement
 	*/
#ifdef CONFIG_HTTPD_TOKEN_CHECK_SUPPORT
	httoken_entry_remove( HTTOKEN_REMOVE_ALL );
#endif
	/*end pete_zhang	2013-12-20*/

	snprintf(buf, sizeof(buf), "Location: /login.htm");
	//answer(r, REDIRECT, buf);
	send_response(r, buf);
#ifdef SYSLOG_ENHANCED
	SetSystemLogFmt(LOG_TYPE_APP, LOG_LEVEL_TYPE_INFO, LOG_APP_MESSAGE_TYPE_http, "HTTP-User Logout from IP at %s.", r->remote_ipstr);
#endif

done:
	if (argv)
		free(argv);

	pthread_exit((void *)CGI_CODE);
}

/*
 * typedef struct aunet_UI_settings_s {
 *		char            pin[MAX_PIN_LEN];               //  VAR_AUNET_PIN
 *		char            APN[MAX_APN_LEN + 4];           //  VAR_AUNET_PDPINFO_APN0 or VAR_AUNET_PDPINFO_APN1
 *		char            ISP_Phone[STR_LEN_68];          //  VAR_AUNET_ISP_PHONE
 *		char            Username[PPP_USERNAME_STR_LEN]; //  VAR_AUNET_USERNAME
 *		char            Password[PPP_PASSWORD_STR_LEN]; //  VAR_AUNET_PASSWORD

 *		short           idle_timeout;                   //  VAR_AUNET_IDLE_TIMEOUT
 *		unsigned short  mtu;                            //  VAR_AUNET_MTU
 *		unsigned int    auto_reconnect  :1;             //  VAR_AUNET_AUTO_RECONNECT
 *		unsigned int    on_demend       :1;             //  VAR_AUNET_ON_DEMAND
 *		unsigned int    startup_stat    :2;             //  VAR_AUNET_STARTUP_STATE

 *		unsigned int    ppp_or_ndis     :1;             //  VAR_AUNET_FUNC, 0: PPP, 1: NDIS
 * } aunet_UI_settings_t;
 *
 */
void lte_settings_cgi(int argc, char **argv, void *p)
{
	CGI_info *ci = (CGI_info *)p;
	struct request_rec *r = ci->r;
	char *pin = NULL;
#ifdef CONFIG_HTTPD_HOT_FIX_SW1
	char *next_page = NULL; //hugh 2013/12/12
#endif

#if 0
	char *apn2;
	char *apn0, *apn1, *username, *password, *idle_timeout, *isp_phone, *backup_mode;
	char *save_pin = NULL, *page_from = NULL;
#endif
	char *username, *password, *idle_timeout;
	char *page_from = NULL;
	char buf[1024];
	int tid;
	int ret = 0;
	//int ifno = T_COM1_INT;
	//aunet_UI_settings_t as;

	if((tid = get_tid()) == MID_FAIL)
		return;

	/* Destory last cgi table */
	if ((input_num = __destroy_cgi(inputs, input_num)) != 0)
		ht_dbg("warning: %d entries have not been freed, memory leaking.. \n", input_num);

	/* action=ui_lte&submit_button=lte_main.htm&VAR_AUNET_PIN=1234&save_pin=on \
	 * &VAR_AUNET_PDPINFO_APN0=3gnet&VAR_AUNET_PDPINFO_APN1=3gnet
	 */
	input_num = __init_cgi(ci, inputs, sizeof(inputs)/sizeof(input_t));

	/*
 	pete_zhang 2013-12-20	ticket-1167

	description: GUI security enhancement
 	*/
#ifdef CONFIG_HTTPD_TOKEN_CHECK_SUPPORT
	if ( !cgi_token_pass(ci, inputs, 0) ) //failed to pass token checking
	{
		goto done;
	}
#endif
	/*end pete_zhang	2013-12-20*/

	page_from = __get_cgi(inputs, "page_from");

	username = NULL;
	password = NULL;
	idle_timeout = NULL;

	idle_timeout = __get_cgi(inputs, "VAR_AUNET_0_IDLE_TIMEOUT");
	username = __get_cgi(inputs, "VAR_AUNET_0_USERNAME");
	password = __get_cgi(inputs, "VAR_AUNET_0_PASSWORD");

	#ifdef CONFIG_HTTPD_HOT_FIX_SW1
	next_page = __get_cgi(inputs, "next_page");  //hugh 2013/12/12
	#endif

#if 0
	pin = NULL;
	apn0 = NULL;
	apn1 = NULL;

	apn2 = NULL;
	username = NULL;
	password = NULL;
	idle_timeout = NULL;
	isp_phone = NULL;
	backup_mode = NULL;

	page_from = __get_cgi(inputs, "page_from");

	/* from LTE */
	pin = __get_cgi(inputs, "VAR_AUNET_0_PIN");
	apn0 = __get_cgi(inputs, "VAR_AUNET_PDPINFO_0_APN0");
	apn1 = __get_cgi(inputs, "VAR_AUNET_PDPINFO_0_APN1");
	apn2 = __get_cgi(inputs, "VAR_AUNET_PDPINFO_0_APN2");
	save_pin = __get_cgi(inputs, "save_pin");

	/* from UTMS */
	backup_mode = __get_cgi(inputs, "VAR_AUNET_PDPINFO_0_APN_BACKUP0");
	isp_phone = __get_cgi(inputs, "VAR_AUNET_0_ISP_PHONE");
	idle_timeout = __get_cgi(inputs, "VAR_AUNET_0_IDLE_TIMEOUT");
	username = __get_cgi(inputs, "VAR_AUNET_0_USERNAME");
	password = __get_cgi(inputs, "VAR_AUNET_0_PASSWORD");

	#ifdef CONFIG_HTTPD_HOT_FIX_SW1
	next_page = __get_cgi(inputs, "next_page");  //hugh 2013/12/12
	#endif

	memset(&as, 0, sizeof(aunet_UI_settings_t));

	/* ----  fill the structure ---- */
	if (pin)
		snprintf(as.pin, sizeof(as.pin), "%s", pin);
	else /* no change on PIN */
		mapi_ccfg_get_str(tid, "VAR_AUNET_0_PIN", as.pin, sizeof(as.pin));

	if (apn0)
		snprintf(as.APN, sizeof(as.APN), "%s", apn0);
	else /* no change on APN0 */
		mapi_ccfg_get_str(tid, "VAR_AUNET_PDPINFO_0_APN0", as.APN, sizeof(as.APN));

	/* apn1, backup_mode are not used, just set to glbcfg */
	if (apn1)
		mapi_ccfg_set_str(tid, "VAR_AUNET_PDPINFO_0_APN1", apn1);

	if (apn2)
		mapi_ccfg_set_str(tid, "VAR_AUNET_PDPINFO_0_APN2", apn2);

	if (backup_mode)
		mapi_ccfg_set_str(tid, "VAR_AUNET_PDPINFO_0_APN_BACKUP0", backup_mode);

	if (isp_phone)
		snprintf(as.ISP_Phone, sizeof(as.ISP_Phone), "%s", isp_phone);
	else /* no change on isp_phone */
		mapi_ccfg_get_str(tid, "VAR_AUNET_0_ISP_PHONE", as.ISP_Phone, sizeof(as.ISP_Phone));

	if (idle_timeout)
		as.idle_timeout = (short) atoi(idle_timeout);
	else /* no change on idle_timeout */
		as.idle_timeout = (short) atoi(mapi_ccfg_get_str(tid, "VAR_AUNET_IDLE_TIMEOUT", buf, sizeof(buf)));

	if (username)
		snprintf(as.Username, sizeof(as.Username), "%s", username);
	else /* no change on username */
		mapi_ccfg_get_str(tid, "VAR_AUNET_0_USERNAME", as.Username, sizeof(as.Username));

	if (password)
		snprintf(as.Password, sizeof(as.Password), "%s", password);
	else /* no change on password */
		mapi_ccfg_get_str(tid, "VAR_AUNET_0_PASSWORD", as.Password, sizeof(as.Password));

	/*
	 * linghong_tan 2013-09-10.
	 *
	 * FIXME: These data do not come from webpage,
	 * we just get them from glbcfg.
	 */
	as.mtu = (unsigned short)atoi(mapi_ccfg_get_str(tid, "VAR_AUNET_0_MTU", buf, sizeof(buf)));
	as.auto_reconnect = (unsigned int)atoi(mapi_ccfg_get_str(tid, "VAR_AUNET_0_AUTO_RECONNECT", buf, sizeof(buf)));
	as.on_demend = (unsigned int)atoi(mapi_ccfg_get_str(tid, "VAR_AUNET_0_ON_DEMAND", buf, sizeof(buf)));
	as.startup_stat = (unsigned int)atoi(mapi_ccfg_get_str(tid, "VAR_AUNET_0_STARTUP_STATE", buf, sizeof(buf)));
	//as.ppp_or_ndis = (unsigned int)atoi(mapi_ccfg_get_str(tid, "VAR_AUNET_0_FUNC", buf, sizeof(buf)));
	as.ppp_or_ndis = 1;

	if (page_from && !strcmp(page_from, "UMTS"))
		ifno = T_COM3_INT;	/* UMTS page */
	else
		ifno = T_COM1_INT;	/* LTE page */

	if (save_pin && !strcmp(save_pin, "1"))
		mapi_aunet_set_pin(tid, ifno, &as, 1);
	else
		mapi_aunet_set_pin(tid, ifno, &as, 0);

	cprintf("pin=[%s], APN=[%s], isp_phone=[%s], username=[%s], password=[%s], page_from=[%s]\n",
			as.pin, as.APN, as.ISP_Phone, as.Username, as.Password, page_from);

	ret = mapi_aunet_set_UI_settings(tid, ifno, &as);
#endif
	#ifdef CONFIG_HTTPD_HOT_FIX_SW1
	// NOTE: hugh
	//    2013/12/16: we no feedback OK,Fail state to browser, here just send Sucess or Fail
	//
	if (v_ops->page_suffix != NULL) {
		if (ret == 0) {
			// sucess
			snprintf(buf, sizeof(buf), "Location: /%s%s\n\n", (next_page==NULL) ? "Success" : next_page, v_ops->page_suffix);
		}else{
			snprintf(buf, sizeof(buf), "Location: /Fail%s\n\n", v_ops->page_suffix);
		}
		send_response(r, buf);
	}else
	#endif
	if (ret == 0) {
		answer(r, DOCUMENT_FOLLOWS, NOTHING); //only send 200 ok
	}
	else {
		/* error report */
	}

	mapi_ccfg_commit(tid);

done:
	if (argv)
		free(argv);

	pthread_exit((void *)CGI_CODE);
}

void wl_cal_cgi(int argc, char **argv, void *p)
{
	CGI_info *ci = (CGI_info *)p;
	struct request_rec *r = ci->r;
	int len;
	char *boundary = NULL;
	int tail_len = 0;
	char buf[1024];
	int sock;
	int ret_code = -1;
	FILE *stream = NULL;
	int x = 0;
	int got_file = 0;

	sock = ci->socketid;
	len = r->content_length;

	if ((input_num = __destroy_cgi(inputs, input_num)) != 0)
		ht_dbg("warning: %d entries have not been freed, memory leaking.. \n", input_num);

	/*
 	pete_zhang 2013-12-20	ticket-1167

	description: GUI security enhancement
 	*/
#ifdef CONFIG_HTTPD_TOKEN_CHECK_SUPPORT
	if ( !cgi_token_pass(ci, inputs, 0) ) //failed to pass token checking
	{
		goto done;
	}
#endif
	/*end pete_zhang        2013-12-20*/

	stream = fdopen(sock, "r");
	if (!stream) {
		goto done;
	}

	while (len > 0) {

		memset(buf, 0, sizeof(buf));

		if (!get_line(r, buf, MIN(len + 1, sizeof(buf)), stream)) {
			goto done;
		}

		len -= strlen(buf);

		if (!strncasecmp(buf, "Content-Disposition:", 20)) {
			if (strstr(buf, "name=\"wait_time\""))
			{
				char *ptr, *stop;
				int idx;

				for (idx=0; idx<2 && len>0; idx++) {

					memset(buf, 0, sizeof(buf));

					/* eat a line */
					if (!get_line(r, buf, MIN(len + 1, sizeof(buf)), stream))
						goto done;

					len -= strlen(buf);
				}

				ptr = malloc(100);
				if (!ptr)
					goto done;

				snprintf(ptr, 100, "wait_time=%d", atoi(buf));

				ht_dbg("ptr=%s\n", ptr);
				inputs[x].name = ptr;
				plustospace(inputs[x].name);
				unescape_url(inputs[x].name);

				if (!(stop = strchr(inputs[x].name, '=')))
					inputs[x].val = NULL;
				else {
					*stop++ = '\0';
					inputs[input_num].val = stop;
				}

				ht_dbg("name=%s, val=%s\n", inputs[x].name, inputs[x].val);
				x++;
			}
			else if (strstr(buf, "name=\"calibration\"")) {
				got_file = 1;
				break;
			}
		}
	}

	input_num = x;

	memset(buf, 0, sizeof(buf));

	/* Skip boundary and headers */
	while (len > 0) {
		if (!get_line(r, buf, MIN(len + 1, sizeof(buf)), stream))
			goto done;

		len -= strlen(buf);
		if (!strcmp(buf, "\n") || !strcmp(buf, "\r\n"))
			break;
	}

	if ((boundary = strcasestr(r->content_type, "boundary=")) != NULL) {
		//Boundary was found.
		boundary += strlen("boundary=");

		//Please see RFC2616:
		//boundary=THIS_STRING_SEPARATES
		//The tail is: "\r\n--THIS_STRING_SEPARATES--\r\n"
		tail_len = 2 + 2 + strlen(boundary) + 2 + 2;

		//Remove the tail length first.
		len -= tail_len;

		//cprintf("tail length: [%d] => [%d]\n", tail_len, len);
	}

	if (got_file) {

		cprintf("===> Wireless calibration CGI <===\n");

		/* store file to /tmp/cal_wlan0.bin */
		ret_code = sys_restore(r, "/tmp/cal_wlan0.bin", stream, &len);
		if (ret_code < 0)
			goto done;

		/* package it to /tmp/eeprom_wl.tar.gz */
		ret_code = system("cd /tmp; tar czf /tmp/eeprom_wl.tar.gz cal_wlan0.bin; cd -");
		if (ret_code < 0)
			goto done;

		/* flash it to wlanconfig */
		ret_code = system("vol_mgmt write_vol wlanconfig /tmp/eeprom_wl.tar.gz");
		if (ret_code < 0)
			goto done;
	}

	//crazy_liang 2013.3.12: For calculating the correct file size.
	if (tail_len > 0) {
		//Recover the tail length.
		len += tail_len;
	}
	//end crazy_liang 2013.3.12

	/* Slurp anything remaining in the request */
	while (len--)
		get_char(r, stream);

done:
	if (argv)
		free(argv);

	answer(r, DOCUMENT_FOLLOWS, NOTHING); //only send 200 ok
	so_flush(r);

	/* Only when success, device will reboot. */
	if (ret_code == 0) {
		/* reboot the system */
		mng_action("sys_wl_reset", NOTHING);
	}

	pthread_exit((void *)CGI_CODE);
}

int connection_with_owl(char *request, char *answer, int answerlen)
{
	struct sockaddr_in RemoteAddr;
	int sock_fd;
	int ret = 0;
	sock_fd = socket(AF_INET, SOCK_STREAM, 0);
	memset((char *)&RemoteAddr, 0, sizeof(RemoteAddr));
	RemoteAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	RemoteAddr.sin_port = htons (OWL_LISTEN_PORT);
	RemoteAddr.sin_family = AF_INET;

	if(connect(sock_fd, (struct sockaddr *)&RemoteAddr, sizeof(RemoteAddr)) < 0)
	{
		cprintf("connect error!\n");
		close(sock_fd);
		return -1;
	}	
	write(sock_fd, request, strlen(request));
	
/*	
	if(read(sock_fd, answer,answerlen) > 0)
	{
		cprintf("answer=%s\n",answer);
		ret = 0;
	}
	else
	{
		cprintf("read error\n");
		ret = -1;
	}
*/	
	
	close(sock_fd);
	return ret;
}

void bt_answer(struct request_rec *r, char *buf)
{
	r->status = DOCUMENT_FOLLOWS;
	r->content_type = "application/json";
	r->content_length = strlen(buf); 
	send_http_header(r);
	r->header_only = 1;
	so_printf(r, "%s", buf);
}

int check_valid_IP_str(char *buf)
{
	int ip[4];

	if(sscanf(buf, "%d.%d.%d.%d", &ip[0], &ip[1], &ip[2], &ip[3]) != 4)
		return 0;

	return 1;
}

int check_valid_MAC_str(char *buf)
{
	unsigned int m[6];

	if(sscanf(buf,"%x:%x:%x:%x:%x:%x",&m[0],&m[1],&m[2],&m[3],&m[4],&m[5]) != 6)
		return 0;

	return 1;
}


//date: 1@all,2
void set_led_cgi(int argc, char **argv, void *p)
{
	CGI_info *ci = (CGI_info *)p;
	struct request_rec *r = ci->r;	
	char *data=NULL;
	char *cmd=NULL;
	int i = 0;
#if defined(SYSLOG_ENHANCED)
	char log_level[16] = {0};
	char *mac=NULL;
	char *master_ip_p = NULL;
	char str_tmp[256] = {0};
#endif
	char httprequest[2048] = {0};
	char recvline[2048] = {0};
	char jasonbuf[1024] = {0};
	char cmdbuf[128] = {0};
	int tid;
	char input_ary[32][32];
	char arry_tmp;
	int arry_number=0;	
	int arry_number_double=0; 
	char *set_mac=NULL;
	char *set_brightness=NULL;
	
	if((tid = get_tid()) == MID_FAIL)
		return;
	/* Destory last cgi table */
	if ((input_num = __destroy_cgi(inputs, input_num)) != 0)
		ht_dbg("warning: %d entries have not been freed, memory leaking.. \n", input_num);
		
	input_num = __init_cgi(ci, inputs, sizeof(inputs)/sizeof(input_t));
	/*
	pete_zhang 2013-12-20	ticket-1167
	
	description: GUI security enhancement
	*/
#ifdef CONFIG_HTTPD_TOKEN_CHECK_SUPPORT
	if ( !cgi_token_pass(ci, inputs, 0) ) //failed to pass token checking
	{
		so_printf(r, "%s", "error");
		goto done;
	}
#endif
	/*end pete_zhang	2013-12-20*/

	cmd = __get_cgi(inputs, "cmd");
	data = __get_cgi(inputs, "data");
	for (i = 0;i < strlen(data); i++)
	{
		if (('\"' == data[i]) || ('\\' == data[i]))
		{
			data[i] += 127;
		}
	}
	//cprintf("---> set_led_cgi, data[%s]\n", data);
	
	//cprintf("---> set_led_cgi, cmd[%s]\n", cmd);


	arry_tmp = *(data);
	arry_number = arry_tmp - '0';
	arry_number_double = arry_number*2;
	
	//cprintf("[Debug]arry_tmp:%c\n", arry_tmp);
	//cprintf("[Debug]arry_number:%d\n", arry_number);
	//cprintf("[Debug]data+2:%s\n", data+2);
	
	if(!strncmp(cmd, "ledbrightness", 13))
	{
		
		if(separate_string_to_array(data+2, ",", arry_number_double, 32, (char *)&input_ary) != arry_number_double)
		{
			cprintf("[%s]%d data format error!", __func__,__LINE__);
			return -1;		
		}
		for(i=0; i < arry_number_double; i++)
		{
			//cprintf("[Debug][%s]%d  array[%d]:%s array[%d]:%s\n\n\n", __func__,__LINE__, i, input_ary[i],i+1, input_ary[i+1]);
			if(i%2 == 0)//Even numbers
			{
				set_mac = input_ary[i];
				set_brightness= input_ary[i+1];
				
				//cprintf("set_mac = %s\n",set_mac);
				//cprintf("set_brightness = %s\n",set_brightness);
				
				snprintf(jasonbuf, sizeof(jasonbuf),"{\"device_mac\":\"%s\",\"index\":\"0\",\"color\":\"0\", \"brightness\":\"%s\"}",
						set_mac,set_brightness);
				
				//cprintf("jasonbuf = %s\n\n\n",jasonbuf);
				snprintf(httprequest, sizeof(httprequest),"POST /tools/diag/setLED HTTP/1.1\r\nContent-Type:application/json\r\nContent-Length:%d\r\nConnection:Keep-Alive\r\n\r\n%s",strlen(jasonbuf),jasonbuf);
				
				if(connection_with_owl(httprequest, recvline, sizeof(recvline)) < 0)	
					bt_answer(r, "error");
				else
				{
					if(!strncmp(cmd, "ledbrightness", sizeof("ledbrightness")))
					{
						bt_answer(r, "ok_wifi");
					}
					else
						bt_answer(r, "ok");
				}
			}	
		}
	}
			
done:	
	if (argv)
		free(argv);
	
	pthread_exit((void *)CGI_CODE);	

}

void set_access_control_cgi(int argc, char **argv, void *p)
{
	CGI_info *ci = (CGI_info *)p;
	struct request_rec *r = ci->r;	
	char *data=NULL;
	char *cmd=NULL;
	int i = 0;
#if defined(SYSLOG_ENHANCED)
	char log_level[16] = {0};
	char *mac=NULL;
	char *master_ip_p = NULL;
	char str_tmp[256] = {0};
#endif
	char httprequest[2048] = {0};
	char recvline[2048] = {0};
	char jasonbuf[2024] = {0};
	char cmdbuf[128] = {0};
	int tid;
	char input_ary[32][32];
	
	if((tid = get_tid()) == MID_FAIL)
		return;
	/* Destory last cgi table */
	if ((input_num = __destroy_cgi(inputs, input_num)) != 0)
		ht_dbg("warning: %d entries have not been freed, memory leaking.. \n", input_num);
		
	input_num = __init_cgi(ci, inputs, sizeof(inputs)/sizeof(input_t));
	/*
	pete_zhang 2013-12-20	ticket-1167
	
	description: GUI security enhancement
	*/
#ifdef CONFIG_HTTPD_TOKEN_CHECK_SUPPORT
	if ( !cgi_token_pass(ci, inputs, 0) ) //failed to pass token checking
	{
		so_printf(r, "%s", "error");
		goto done;
	}
#endif
	/*end pete_zhang	2013-12-20*/

	cmd = __get_cgi(inputs, "cmd");
	data = __get_cgi(inputs, "data");
	for (i = 0;i < strlen(data); i++)
	{
		if (('\"' == data[i]) || ('\\' == data[i]))
		{
			data[i] += 127;
		}
	}
	//cprintf("[%s_%d]--->data[%s]\n", __FUNCTION__, __LINE__, data);
	//cprintf("[%s_%d]--->cmd[%s]\n", __FUNCTION__, __LINE__, cmd);

	
	if(separate_string_to_array(data, "?$[+", 11, 32, (char *)&input_ary) != 11)
	{
		cprintf("[%s]%d data format error!", __func__,__LINE__);
		return -1;		
	}
/*
	for(i=0; i <= 11 ; i++)
	{
		cprintf("[%s_%d]--->input_ary[%d]:%s\n", __FUNCTION__, __LINE__, i, input_ary[i]);

	}
	cprintf("[%s_%d]--->\"jasonbuf:%s\n\n\n", __FUNCTION__, __LINE__, jasonbuf);
*/
	if(!strncmp(cmd, "syncaccesscontrol", 17))
	{
		
		snprintf(jasonbuf, sizeof(jasonbuf),"{\"profile\":[{\"act\":\"%s\",\"pid\":\"%s\",\"n\":\"%s\", \"configs\":[{\"act\":\"%s\",\"id\":\"%s\",\"enable\":\"%s\",\"type\":\"%s\",\"day\":\"%s\",\"st\":\"%s\",\"et\":\"%s\",\"note\":\"%s\"}]}]}",input_ary[0],input_ary[1],input_ary[2],input_ary[3],input_ary[4],input_ary[5],input_ary[6],input_ary[7],input_ary[8],input_ary[9],input_ary[10],input_ary[11]);
		snprintf(httprequest, sizeof(httprequest),"POST /Status/access_control_profile HTTP/1.1\r\nContent-Type:application/json\r\nContent-Length:%d\r\nConnection:Keep-Alive\r\n\r\n%s",strlen(jasonbuf),jasonbuf);
		cprintf("[%s_%d]--->httprequest:\n%s\n\n\n", __FUNCTION__, __LINE__, httprequest);	

		if(connection_with_owl(httprequest, recvline, sizeof(recvline)) < 0)
		{
			bt_answer(r, "error");
		}
		else
		{
			bt_answer(r, "ok");
			system("tr69_trigger reinitnodes accessprofile");
		}
	}
			
done:	
	if (argv)
		free(argv);
	
	pthread_exit((void *)CGI_CODE);	

}



/*
Description:
Set WIFI config.Bind the pid  of accsee_control_profile with pid  of the wifi/config

Input: 
guest@2@116@216
pid      interface_number    2.4Ginterface  5Ginterface

Output: 


In Web:
user wifi default pid is user
guest wifi default pid is guest

leo_lin  2017-11-29
*/
void set_wifi_config_for_sche(int argc, char **argv, void *p)
{
	CGI_info *ci = (CGI_info *)p;
	struct request_rec *r = ci->r;	
	char *data=NULL;
	char *cmd=NULL;
	int i = 0;
	int tid = MID_FAIL;
#if defined(SYSLOG_ENHANCED)
	char log_level[16] = {0};
	char *mac=NULL;
	char *master_ip_p = NULL;
	char str_tmp[256] = {0};
#endif
	char httprequest[2048] = {0};
	char recvline[2048] = {0};
	char jasonbuf[2024] = {0};
	char cmdbuf[128] = {0};
	int tmp;
	int number;
	char input_ary[32][32];
	char password_check[256];
	char *point;
	
	if((tid = get_tid()) == MID_FAIL)
		return;
	/* Destory last cgi table */
	if ((input_num = __destroy_cgi(inputs, input_num)) != 0)
		ht_dbg("warning: %d entries have not been freed, memory leaking.. \n", input_num);
		
	input_num = __init_cgi(ci, inputs, sizeof(inputs)/sizeof(input_t));
	/*
	pete_zhang 2013-12-20	ticket-1167
	
	description: GUI security enhancement
	*/
#ifdef CONFIG_HTTPD_TOKEN_CHECK_SUPPORT
	if ( !cgi_token_pass(ci, inputs, 0) ) //failed to pass token checking
	{
		so_printf(r, "%s", "error");
		goto done;
	}
#endif
	/*end pete_zhang	2013-12-20*/

	cmd = __get_cgi(inputs, "cmd");
	data = __get_cgi(inputs, "data");
	for (i = 0;i < strlen(data); i++)
	{
		if (('\"' == data[i]) || ('\\' == data[i]))
		{
			data[i] += 127;
		}
	}
	//cprintf("[%s_%d]--->data[%s]\n", __FUNCTION__, __LINE__, data);
	//cprintf("[%s_%d]--->cmd[%s]\n", __FUNCTION__, __LINE__, cmd);



	point = strstr(data,"@");
      //cprintf("point:%s\n",point+1);
	number =atoi(point+1);
	number = number +2;//number:the value of the number of parameters after the partition
	//cprintf("[%s_%d]--->number:%d\n\n\n", __FUNCTION__, __LINE__,number);

	if(separate_string_to_array(data, "@", number, 32, (char *)&input_ary) != number)
	{
		cprintf("[%s]%d data format error!", __func__,__LINE__);
		return -1;		
	}
/*
	for(i=0; i < number ; i++)
	{
		cprintf("[%s_%d]--->input_ary[%d]:%s\n", __FUNCTION__, __LINE__, i, input_ary[i]);
	}
*/
/*
[set_wifi_config_for_sche_1802]--->input_ary[0]:guest
[set_wifi_config_for_sche_1802]--->input_ary[1]:2
[set_wifi_config_for_sche_1802]--->input_ary[2]:116
[set_wifi_config_for_sche_1802]--->input_ary[3]:216
[set_wifi_config_for_sche_1805]--->"num:4  number:4
*/

	if ((tid = mapi_start_transc()) == MID_FAIL )
	{
		cprintf("[%s_%d]--->get password check error\n\n\n", __FUNCTION__, __LINE__);
		return -1;
	}

	mapi_ccfg_get_str(tid, "ARC_SYS_Password", password_check, sizeof(password_check));
	//cprintf("[%s_%d]--->password_check:%s\n\n\n", __FUNCTION__, __LINE__,password_check);
	if(!strncmp(cmd, "wifi_sche_pid", 13))
	{
		for(i=0; i< number-2; i++)
		{
			snprintf(jasonbuf, sizeof(jasonbuf),"{\"wifi_config\":[{\"password_check\":\"%s\",\"interface\":\"%s\",\"pid\":\"%s\"}]}]}",password_check,input_ary[i+2],input_ary[0]);
			snprintf(httprequest, sizeof(httprequest),"POST /WIFI/config HTTP/1.1\r\nContent-Type:application/json\r\nContent-Length:%d\r\nConnection:Keep-Alive\r\n\r\n%s",strlen(jasonbuf),jasonbuf);
			cprintf("[%s_%d]-WIFI-->httprequest:\n%s\n\n\n", __FUNCTION__, __LINE__, httprequest);	
			if(connection_with_owl(httprequest, recvline, sizeof(recvline)) < 0)
			{
				bt_answer(r, "error");
			}
			else
			{
				bt_answer(r, "ok");
			}
		}	

	}
			
done:	
	if (argv)
		free(argv);
	
	pthread_exit((void *)CGI_CODE);	

}


//each one
/*
date:
11@guest
21@user
*/
void set_wifi_config_pid(int argc, char **argv, void *p)
{
	CGI_info *ci = (CGI_info *)p;
	struct request_rec *r = ci->r;	
	char *data=NULL;
	char *cmd=NULL;
	int i = 0;
#if defined(SYSLOG_ENHANCED)
	char log_level[16] = {0};
	char *mac=NULL;
	char *master_ip_p = NULL;
	char str_tmp[256] = {0};
#endif
	char httprequest[2048] = {0};
	char recvline[2048] = {0};
	char jasonbuf[2024] = {0};
	char cmdbuf[128] = {0};
	int tid=-1;
	int tmp;
	int number;
	char input_ary[32][32];
	char password_check[256];
	
	if((tid = get_tid()) == MID_FAIL)
		return;
	/* Destory last cgi table */
	if ((input_num = __destroy_cgi(inputs, input_num)) != 0)
		ht_dbg("warning: %d entries have not been freed, memory leaking.. \n", input_num);
		
	input_num = __init_cgi(ci, inputs, sizeof(inputs)/sizeof(input_t));
	/*
	pete_zhang 2013-12-20	ticket-1167
	
	description: GUI security enhancement
	*/
#ifdef CONFIG_HTTPD_TOKEN_CHECK_SUPPORT
	if ( !cgi_token_pass(ci, inputs, 0) ) //failed to pass token checking
	{
		so_printf(r, "%s", "error");
		goto done;
	}
#endif
	/*end pete_zhang	2013-12-20*/

	cmd = __get_cgi(inputs, "cmd");
	data = __get_cgi(inputs, "data");
	for (i = 0;i < strlen(data); i++)
	{
		if (('\"' == data[i]) || ('\\' == data[i]))
		{
			data[i] += 127;
		}
	}
	//cprintf("[%s_%d]--->data[%s]\n", __FUNCTION__, __LINE__, data);
	//cprintf("[%s_%d]--->cmd[%s]\n", __FUNCTION__, __LINE__, cmd);

	if(separate_string_to_array(data, "@", 2, 32, (char *)&input_ary) != 2)
	{
		cprintf("[%s]%d data format error!", __func__,__LINE__);
		return -1;		
	}
/*
	for(i=0; i < 2 ; i++)
	{
		cprintf("[%s_%d]--->input_ary[%d]:%s\n", __FUNCTION__, __LINE__, i, input_ary[i]);
	}
*/

	if ((tid = mapi_start_transc()) == MID_FAIL )
	{
		cprintf("[%s_%d]--->get password check error\n\n\n", __FUNCTION__, __LINE__);
		return -1;
	}

	mapi_ccfg_get_str(tid, "ARC_SYS_Password", password_check, sizeof(password_check));


	if(!strncmp(cmd, "set_wifi_config_pid", 19))
	{
		//cprintf("[%s_%d]--->cmd[%s]\n", __FUNCTION__, __LINE__, cmd);
		//strcpy(password_check,"admin");
		snprintf(jasonbuf, sizeof(jasonbuf),"{\"wifi_config\":[{\"password_check\":\"%s\",\"interface\":\"%s\",\"pid\":\"%s\"}]}]}",password_check,input_ary[0],input_ary[1]);
		snprintf(httprequest, sizeof(httprequest),"POST /WIFI/config HTTP/1.1\r\nContent-Type:application/json\r\nContent-Length:%d\r\nConnection:Keep-Alive\r\n\r\n%s",strlen(jasonbuf),jasonbuf);
		//cprintf("[%s_%d]-WIFI-->httprequest:\n%s\n\n\n", __FUNCTION__, __LINE__, httprequest);	
		
		if(connection_with_owl(httprequest, recvline, sizeof(recvline)) < 0)
		{
			bt_answer(r, "error");
		}
		else
		{
			bt_answer(r, "ok");
		}
	}
			
done:	
	if (argv)
		free(argv);
	
	pthread_exit((void *)CGI_CODE);	

}




void set_status_station_cgi(int argc, char **argv, void *p)
{
	CGI_info *ci = (CGI_info *)p;
	struct request_rec *r = ci->r;	
	char *data=NULL;
	char *cmd=NULL;
	int i = 0;
#if defined(SYSLOG_ENHANCED)
	char log_level[16] = {0};
	char *mac=NULL;
	char *master_ip_p = NULL;
	char str_tmp[256] = {0};
#endif
	char httprequest[2048] = {0};
	char recvline[2048] = {0};
	char jasonbuf[2024] = {0};
	char cmdbuf[128] = {0};
	int tid;
	char input_ary[32][32];
	
	if((tid = get_tid()) == MID_FAIL)
		return;
	/* Destory last cgi table */
	if ((input_num = __destroy_cgi(inputs, input_num)) != 0)
		ht_dbg("warning: %d entries have not been freed, memory leaking.. \n", input_num);
		
	input_num = __init_cgi(ci, inputs, sizeof(inputs)/sizeof(input_t));
	/*
	pete_zhang 2013-12-20	ticket-1167
	
	description: GUI security enhancement
	*/
#ifdef CONFIG_HTTPD_TOKEN_CHECK_SUPPORT
	if ( !cgi_token_pass(ci, inputs, 0) ) //failed to pass token checking
	{
		so_printf(r, "%s", "error");
		goto done;
	}
#endif
	/*end pete_zhang	2013-12-20*/

	cmd = __get_cgi(inputs, "cmd");
	data = __get_cgi(inputs, "data");
	for (i = 0;i < strlen(data); i++)
	{
		if (('\"' == data[i]) || ('\\' == data[i]))
		{
			data[i] += 127;
		}
	}
	//cprintf("[%s_%d]--->data[%s]\n", __FUNCTION__, __LINE__, data);
	//cprintf("[%s_%d]--->cmd[%s]\n", __FUNCTION__, __LINE__, cmd);

	//parent_id   station_mac    alias_name		 pid
	if(separate_string_to_array(data, "?$[+", 4, 32, (char *)&input_ary) != 4)
	{
		cprintf("[%s]%d data format error!", __func__,__LINE__);
		return -1;		
	}
/*
	for(i=0; i <= 4 ; i++)
	{
		cprintf("[%s_%d]--->input_ary[%d]:%s\n", __FUNCTION__, __LINE__, i, input_ary[i]);

	}
*/
	//cprintf("[%s_%d]--->\"jasonbuf:%s\n\n\n", __FUNCTION__, __LINE__, jasonbuf);

	if(!strncmp(cmd, "syncclientdevicename", 20))
	{
		
		snprintf(jasonbuf, sizeof(jasonbuf),"{\"stations\":[{\"parent_id\":\"%s\",\"station_mac\":\"%s\",\"alias_name\":\"%s\", \"pid\":\"%s\"}]}",input_ary[0],input_ary[1],input_ary[2],input_ary[3]);
		snprintf(httprequest, sizeof(httprequest),"POST /Status/STATION HTTP/1.1\r\nContent-Type:application/json\r\nContent-Length:%d\r\nConnection:Keep-Alive\r\n\r\n%s",strlen(jasonbuf),jasonbuf);
		//cprintf("[%s_%d]--->httprequest:\n%s\n\n\n", __FUNCTION__, __LINE__, httprequest);	

		if(connection_with_owl(httprequest, recvline, sizeof(recvline)) < 0)
		{
			bt_answer(r, "error");
		}
		else
		{
			bt_answer(r, "ok");
		}
	}
			
done:	
	if (argv)
		free(argv);
	
	pthread_exit((void *)CGI_CODE);	

}



void set_control_internet_cgi(int argc, char **argv, void *p)
{
	CGI_info *ci = (CGI_info *)p;
	struct request_rec *r = ci->r;	
	char *data=NULL;
	char *cmd=NULL;
	int i = 0;
#if defined(SYSLOG_ENHANCED)
	char log_level[16] = {0};
	char *mac=NULL;
	char *master_ip_p = NULL;
	char str_tmp[256] = {0};
#endif
	char httprequest[2048] = {0};
	char recvline[2048] = {0};
	char jasonbuf[2024] = {0};
	char cmdbuf[128] = {0};
	int tid;
	char input_ary[32][32];
	
	if((tid = get_tid()) == MID_FAIL)
		return;
	/* Destory last cgi table */
	if ((input_num = __destroy_cgi(inputs, input_num)) != 0)
		ht_dbg("warning: %d entries have not been freed, memory leaking.. \n", input_num);
		
	input_num = __init_cgi(ci, inputs, sizeof(inputs)/sizeof(input_t));
	/*
	pete_zhang 2013-12-20	ticket-1167
	
	description: GUI security enhancement
	*/
#ifdef CONFIG_HTTPD_TOKEN_CHECK_SUPPORT
	if ( !cgi_token_pass(ci, inputs, 0) ) //failed to pass token checking
	{
		so_printf(r, "%s", "error");
		goto done;
	}
#endif
	/*end pete_zhang	2013-12-20*/

	cmd = __get_cgi(inputs, "cmd");
	data = __get_cgi(inputs, "data");
	for (i = 0;i < strlen(data); i++)
	{
		if (('\"' == data[i]) || ('\\' == data[i]))
		{
			data[i] += 127;
		}
	}
	//cprintf("[%s_%d]--->data[%s]\n", __FUNCTION__, __LINE__, data);
	//cprintf("[%s_%d]--->cmd[%s]\n", __FUNCTION__, __LINE__, cmd);




	if(!strncmp(cmd, "sync_control_internet", 21))
	{
		//cprintf("[%s_%d]----------sync_control_internet------------>\n\n\n", __FUNCTION__, __LINE__);
		
		snprintf(jasonbuf, sizeof(jasonbuf),"{\"internet_control_status\":\"%s\"}",data);
		//cprintf("[%s_%d]--->jasonbuf:\n%s\n\n\n", __FUNCTION__, __LINE__, jasonbuf);
		snprintf(httprequest, sizeof(httprequest),"POST /Tools/Control_Internet HTTP/1.1\r\nContent-Type:application/json\r\nContent-Length:%d\r\nConnection:Keep-Alive\r\n\r\n%s",strlen(jasonbuf),jasonbuf);
		//cprintf("[%s_%d]--->httprequest:\n%s\n\n\n", __FUNCTION__, __LINE__, httprequest);	

		if(connection_with_owl(httprequest, recvline, sizeof(recvline)) < 0)
		{
			bt_answer(r, "error");
		}
		else
		{
			bt_answer(r, "ok");
		}
	}
			
done:	
	if (argv)
		free(argv);
	
	pthread_exit((void *)CGI_CODE);	

}





void rescan_cgi(int argc, char **argv, void *p){
	char cmd[100] = "";	
	int tid;
	
	if((tid = get_tid()) == MID_FAIL)
		return;
	snprintf(cmd, sizeof(cmd), "iwpriv ra0  set AutoChannelSel=1");
	system(cmd);
	
	mapi_ccfg_set_str(tid, "ARC_WLAN_24G_Channel","0");
	mapi_ccfg_commit(tid);
}
void frescan_cgi(int argc, char **argv, void *p){
	char cmd[100] = "";	
	int tid;
	
	if((tid = get_tid()) == MID_FAIL)
		return;
	snprintf(cmd, sizeof(cmd), "iwpriv rai0  set AutoChannelSel=1");
	system(cmd);

	mapi_ccfg_set_str(tid, "ARC_WLAN_5G_Channel","0");
	mapi_ccfg_commit(tid);
}


void get_logfile_cgi(int argc, char **argv, void *p)
{
	CGI_info *ci = (CGI_info *)p;
	struct request_rec *r = ci->r;	
	char *data=NULL;
	char *cmd=NULL;
	int i = 0;
	char log_level[16] = {0};
	char *dev_name=NULL;
	char *master_ip_p = NULL;
	char str_tmp[256] = {0};
	char httprequest[2048] = {0};
	char recvline[2048] = {0};
	char jasonbuf[1024] = {0};
	char cmdbuf[128] = {0};
	int tid;
	
	if((tid = get_tid()) == MID_FAIL)
		return;
	/* Destory last cgi table */
	if ((input_num = __destroy_cgi(inputs, input_num)) != 0)
		ht_dbg("warning: %d entries have not been freed, memory leaking.. \n", input_num);
		
	input_num = __init_cgi(ci, inputs, sizeof(inputs)/sizeof(input_t));
	/*
	pete_zhang 2013-12-20	ticket-1167
	
	description: GUI security enhancement
	*/
#ifdef CONFIG_HTTPD_TOKEN_CHECK_SUPPORT
	if ( !cgi_token_pass(ci, inputs, 0) ) //failed to pass token checking
	{
		so_printf(r, "%s", "error");
		goto done;
	}
#endif
	/*end pete_zhang	2013-12-20*/

	cmd = __get_cgi(inputs, "cmd");
	data = __get_cgi(inputs, "data");
	dev_name = __get_cgi(inputs, "name");
	
	for (i = 0;i < strlen(data); i++)
	{
		if (('\"' == data[i]) || ('\\' == data[i]))
		{
			data[i] += 127;
		}
	}
	cprintf("---> get_logfile_cgi, data[%s]\n", data);

	mapi_tmp_set(tid, "ARC_SYS_LogFile_Name", dev_name);
	
	//slave log file from owl
	if(strlen(data) > 1)	//the minus ipv4...
	{
		/*security check*/
		strcpy(str_tmp, data);
		master_ip_p = strtok(str_tmp, ",");
		if(check_valid_IP_str(master_ip_p) == 0)
		{
			cprintf("---> get_logfile_cgi invalid ip!\n");
			so_printf(r, "%s", "error");
			goto done;
		}
		snprintf(cmdbuf, sizeof(cmdbuf),"udpsvd \-vE %s 69 tftpd \-c \-u root \/tmp &", master_ip_p);
	}
	else
	{
		snprintf(cmdbuf, sizeof(cmdbuf),"udpsvd \-vE 0.0.0.0 69 tftpd \-c \-u root \/tmp &");
	}
	system(cmdbuf);

	arc_cfg_get(tid, ARC_SYS_LOG_Level, log_level, sizeof(log_level), "4");
	
	snprintf(jasonbuf, sizeof(jasonbuf),"{\"cmd\":\"%s\",\"mac\":\"ALL\",\"data\":\"%s,%s\"}",cmd, data,log_level);

	snprintf(httprequest, sizeof(httprequest),"POST /tools/diag/extcmd HTTP/1.1\r\nContent-Type:application/json\r\nContent-Length:%d\r\nConnection:Keep-Alive\r\n\r\n%s",strlen(jasonbuf),jasonbuf);
	cprintf("httprequest = %s\n",httprequest);
	
	if(connection_with_owl(httprequest, recvline, sizeof(recvline)) < 0)	
		bt_answer(r, "error");
	else
		bt_answer(r, "ok");

done:	
	if (argv)
		free(argv);
	
	pthread_exit((void *)CGI_CODE);	
}


void toplogy_cgi(int argc, char **argv, void *p)
{
	CGI_info *ci = (CGI_info *)p;
	struct request_rec *r = ci->r;	
	char *data=NULL;
	char *cmd=NULL;
	int i = 0;
	char httprequest[2048] = {0};
	char recvline[2048] = {0};
	char jasonbuf[1024] = {0};
	char cmdbuf[128] = {0};
	int tid;
	
	if((tid = get_tid()) == MID_FAIL)
		return;
	/* Destory last cgi table */
	if ((input_num = __destroy_cgi(inputs, input_num)) != 0)
		ht_dbg("warning: %d entries have not been freed, memory leaking.. \n", input_num);
		
	input_num = __init_cgi(ci, inputs, sizeof(inputs)/sizeof(input_t));
	/*
	pete_zhang 2013-12-20	ticket-1167
	
	description: GUI security enhancement
	*/
#ifdef CONFIG_HTTPD_TOKEN_CHECK_SUPPORT
	if ( !cgi_token_pass(ci, inputs, 0) ) //failed to pass token checking
	{
		so_printf(r, "%s", "error");
		goto done;
	}
#endif
	/*end pete_zhang	2013-12-20*/

	cmd = __get_cgi(inputs, "cmd");
	data = __get_cgi(inputs, "data");
	for (i = 0;i < strlen(data); i++)
	{
		if (('\"' == data[i]) || ('\\' == data[i]))
		{
			data[i] += 127;
		}
	}
	cprintf("---> toplogy_cgi, data[%s]\n", data);
	if(!strncmp(cmd, "syncwificonfig", 14))
	{
		SetSystemLogFmt(LOG_TYPE_APP, LOG_LEVEL_TYPE_INFO, LOG_APP_MESSAGE_TYPE_http, "HTTP-Key parameters are modified from %s on Settings\-\>Wireless", r->remote_ipstr);
	}
	else if(!strncmp(cmd, "syncpasswdconfig", 16))
	{
		SetSystemLogFmt(LOG_TYPE_APP, LOG_LEVEL_TYPE_INFO, LOG_APP_MESSAGE_TYPE_http, "HTTP-Key parameters are modified from %s on System\-\>Admin Password", r->remote_ipstr);
	}
	else if(!strncmp(cmd, "synclanconfig", 13))
	{
		SetSystemLogFmt(LOG_TYPE_APP, LOG_LEVEL_TYPE_INFO, LOG_APP_MESSAGE_TYPE_http, "HTTP-Key parameters are modified from %s on Settings\-\>Network", r->remote_ipstr);
	}
/* Discs Optimising*/

	 if(!strncmp(cmd, "getdiscoptimising", 17))
	{
		char *mac=NULL;
		char *master_ip_p = NULL;
	
		mac = __get_cgi(inputs, "mac");
		if(strlen(data) > 1)	//the minus ipv4...
		{
		//	strcpy(master_ip_p, data);
		/*	if(check_valid_IP_str(master_ip_p) == 0)
			{
				cprintf("---> toplogy_cgi invalid ip!\n");
				so_printf(r, "%s", "error");
				goto done;
			}*/
			snprintf(cmdbuf, sizeof(cmdbuf),"udpsvd \-vE %s 69 tftpd \-c \-u root \/tmp &", data);
		}
		else
		{
			snprintf(cmdbuf, sizeof(cmdbuf),"udpsvd \-vE 0.0.0.0 69 tftpd \-c \-u root \/tmp &");
		}
		system(cmdbuf);

		/*security check*/
		if((mac == NULL) ||(!strcmp(mac, "00:00:00:00:00:00")) || (check_valid_MAC_str(mac) == 0))
		{
			cprintf("---> toplogy_cgi invalid mac!\n");
			so_printf(r, "%s", "error");
			goto done;
		}
		else
		{
			snprintf(jasonbuf, sizeof(jasonbuf),"{\"cmd\":\"%s\",\"mac\":\"%s\",\"data\":\"%s\"}",cmd, mac, data);
		}
		cprintf("jasonbuf:%s\n",jasonbuf);
	}
	else 
/*END Discs  Optimising*/	
		snprintf(jasonbuf, sizeof(jasonbuf),"{\"cmd\":\"%s\",\"mac\":\"ALL\",\"data\":\"%s\"}",cmd,data);

	snprintf(httprequest, sizeof(httprequest),"POST /tools/diag/extcmd HTTP/1.1\r\nContent-Type:application/json\r\nContent-Length:%d\r\nConnection:Keep-Alive\r\n\r\n%s",strlen(jasonbuf),jasonbuf);
	cprintf("httprequest = %s\n",httprequest);
	
	if(connection_with_owl(httprequest, recvline, sizeof(recvline)) < 0)	
		bt_answer(r, "error");
	else
	{
		if(!strncmp(cmd, "syncwificonfig", sizeof("syncwificonfig")))
		{
			bt_answer(r, "ok_wifi");
		}
		else
			bt_answer(r, "ok");
	}
done:	
	if (argv)
		free(argv);
	
	pthread_exit((void *)CGI_CODE);	
}
void change_lang_cgi(int argc, char **argv, void *p)
{
	CGI_info *ci = (CGI_info *)p;
	struct request_rec *r = ci->r;
	char *param, *pos;
	char *lang, *nexturl;
	char buf[1024];
	int tid;
	int status;
	int idx = 0;
	FILE* fp = NULL;

	if((tid = get_tid()) == MID_FAIL)
		return;
	
	/* Destory last cgi table */
	if ((input_num = __destroy_cgi(inputs, input_num)) != 0)
		ht_dbg("warning: %d entries have not been freed, memory leaking.. \n", input_num);
	
	input_num = __init_cgi(ci, inputs, sizeof(inputs)/sizeof(input_t));

	/*
 	pete_zhang 2013-12-20	ticket-1167

	description: GUI security enhancement
 	*/
#ifdef CONFIG_HTTPD_TOKEN_CHECK_SUPPORT
	if ( !cgi_token_pass(ci, inputs, 0) ) //failed to pass token checking
	{
ht_dbg("the current token is NULL\n");
		goto done;
	}
#endif
	/*end pete_zhang	2013-12-20*/

	lang = __get_cgi(inputs, "lang");
	nexturl = __get_cgi(inputs, "nexturl");

ht_dbg("the current lang is [%s]\n", lang);

	if(lang)
	{
		mapi_ccfg_set_str(tid, "ARC_SYS_Language", lang);
		mapi_ccfg_commit(tid);
	}
	if(nexturl)
	{
		snprintf(buf, sizeof(buf), "Location: /%s", nexturl);
		send_response(r, buf);
	}
done:	
	if (argv)
		free(argv);
	
	pthread_exit((void *)CGI_CODE);
}
#if 0
void apply_sipp_cgi(int argc, char **argv, void *p)
{
	CGI_info *ci = (CGI_info *)p;
	struct request_rec *r = ci->r;
	int idx = -1, act_idx = -1, param_idx = -1, add_idx = 0, del_idx = 0;
	int i = 0, j = 0, k = 0, l = 0, tid = MID_FAIL;
	int cfgID = 0, idx1 = -1, idx2 = -1, tmpidx = 0, tmplen = 0, entrycount = 0;
	char next_page[128];
	char buf[256], tmp[32], tmp2[32];
	char *ptmp, *pEnd;
	int ret;

	memset(buf, 0, sizeof(buf));

	ht_dbg("r->url = %s\n", r->url);

	memset(next_page, 0, sizeof(next_page));

	/* Destory last cgi table */
	if ((input_num = __destroy_cgi(inputs, input_num)) != 0)
		ht_dbg("warning: %d entries have not been freed, memory leaking.. \n", input_num);

	input_num = __init_cgi(ci, inputs, sizeof(inputs)/sizeof(input_t));

	if ( input_num <= 0 ) {
		sprintf(next_page, "%s",  "Fail");
		goto mapi_done;
	}

	/*
 	pete_zhang 2013-12-20	ticket-1167

	description: GUI security enhancement
 	*/
#ifdef CONFIG_HTTPD_TOKEN_CHECK_SUPPORT
	if ( !cgi_token_pass(ci, inputs, 0) ) //failed to pass token checking
	{
		goto mapi_done;
	}
#endif
	/*end pete_zhang        2013-12-20*/

	tid = get_tid();
	if (tid == MID_FAIL) {
		ht_dbg( "Failed to start a transaction to arc middle\n" );
		sprintf(next_page, "%s",  "Fail");
		goto mapi_done;
	}


	for (i=0; i<input_num; i++) {

		//ht_dbg("input.name=[%s], input.val=[%s]\n", inputs[i].name, inputs[i].val);
		if (!inputs[i].name || !inputs[i].val) {
			sprintf(next_page, "%s",  "Fail");
			goto done;
		}
		if (!strcmp(inputs[i].name, "submit_button"))
			idx = i;
		else if (!strcmp(inputs[i].name, "next_page"))
			snprintf(next_page, sizeof(next_page), "%s", inputs[i].val);
		else if (!strcmp(inputs[i].name, "action"))
			act_idx = i;
		else if (!strcmp(inputs[i].name, "action_params"))
			param_idx = i;
		else if (!strcmp(inputs[i].name, "add"))
			add_idx = i;
		else if (!strcmp(inputs[i].name, "del"))
			del_idx = i;
		else if (inputs[i].name[0] != '\0') {
			printf("inputs[i].name=%s, cfgID=%d, idx1=%d, idx2=%d \n", inputs[i].name,cfgID, idx1, idx2 );

			/* Parse out parameter with indexes */
			parseCfgIdx(inputs[i].name, &cfgID, &idx1, &idx2);

			printf("inputs[i].name=%s, cfgID=%d, idx1=%d, idx2=%d \n", inputs[i].name,cfgID, idx1, idx2 );
			if (skip_var(&inputs[i]))
				continue;

			if (cfgID == ARC_VO_SIPP_INTACC_x_UserId) /* internal account*/
			{
				if (add_idx)
				{
					tmpidx = strtol(inputs[add_idx].val, &pEnd, 10);
					/* modify existing incoming mapping */
					for (j=0; ; j++)
					{
						arc_cfg_get_idx1(tid, ARC_VO_SIPP_EXTACC_x_InMapping, j, tmp, sizeof(tmp), "");

						if (strlen(tmp)>0)
						{
							sprintf(tmp, "%s%s", tmp, "0");
							arc_cfg_set_idx1(tid, ARC_VO_SIPP_EXTACC_x_InMapping, j, tmp);
							tmp[0]= '\0';

							sprintf(tmp2, "%s%s-1", tmp2, (j==0)?"":",");/* for outgoing mapping */
						}
						else
							break;
					}

					/* add outgoing mapping */
					arc_cfg_set_idx1(tid, ARC_VO_SIPP_INTACC_x_OutCallMapping, tmpidx, tmp2);

					add_idx = 0;
				}
				else if (del_idx)
				{
						tmpidx = strtol(inputs[del_idx].val, &pEnd,10);
					/* modify existing incoming mapping */
					for (j=0; ; j++)
					{
						arc_cfg_get_idx1(tid, ARC_VO_SIPP_EXTACC_x_InMapping, j, tmp, sizeof(tmp), "");
						if (strlen(tmp)>0)
						{
							ptmp = tmp + tmpidx + 1;
							tmp[tmpidx] = '\0';
							sprintf(tmp, "%s%s",tmp, ptmp);

							arc_cfg_set_idx1(tid, ARC_VO_SIPP_EXTACC_x_InMapping, j, tmp);
							tmp[0]= '\0';
						}
						else
							break;
					}

					/* re-sorting outgoing mapping */
					for (k = tmpidx; ; k++)
					{
						arc_cfg_get_idx1(tid, ARC_VO_SIPP_INTACC_x_OutCallMapping, k+1, tmp2, sizeof(tmp2), "");
						if (strlen(tmp2)>0)
						{
							arc_cfg_set_idx1(tid, ARC_VO_SIPP_INTACC_x_OutCallMapping, k, tmp2);
						}
						else
							break;
					}
					arc_cfg_set_idx1(tid, ARC_VO_SIPP_INTACC_x_OutCallMapping, k, "");
					del_idx = 0;
				}
			}
			else if (cfgID == ARC_VO_SIPP_EXTACC_x_UserId) /* external account */
			{
				if (add_idx)
				{
					tmpidx = strtol(inputs[add_idx].val, &pEnd, 10);

					/* modify existing outgoing mapping */
					for (j=0; ; j++)
					{
						arc_cfg_get_idx1(tid, ARC_VO_SIPP_INTACC_x_OutCallMapping, j, tmp, sizeof(tmp), ""); /* get the outmapping of previous last entry*/

						if (strlen(tmp)>0)
						{
							sprintf(tmp, "%s,-1",tmp);
							arc_cfg_set_idx1(tid, ARC_VO_SIPP_INTACC_x_OutCallMapping, j, tmp);
							tmp[0]= '\0';

							sprintf(tmp2, "%s0", tmp2 ); /* to used by incoming mapping below */
						}
						else
							break;
					}

					/* add incoming mapping */
					arc_cfg_set_idx1(tid, ARC_VO_SIPP_EXTACC_x_InMapping, tmpidx, tmp2);

					add_idx = 0;
				}
				else if (del_idx)
				{
					tmpidx = strtol(inputs[del_idx].val, &pEnd,10);

					/* re-sorting incoming mapping */
					for (k = tmpidx; ; k++)
					{
						arc_cfg_get_idx1(tid, ARC_VO_SIPP_EXTACC_x_InMapping, k+1, tmp2, sizeof(tmp2), "");
						if (strlen(tmp2)>0)
						{
							arc_cfg_set_idx1(tid, ARC_VO_SIPP_EXTACC_x_InMapping, k, tmp2);
						}
						else
							break;
					}

					arc_cfg_set_idx1(tid, ARC_VO_SIPP_EXTACC_x_InMapping, k, "");

					entrycount = k+1; /* total amount of External account */

					/* modify existing outgoing mapping */
					for (j=0; ; j++)
					{
						arc_cfg_get_idx1(tid, ARC_VO_SIPP_INTACC_x_OutCallMapping, j, tmp, sizeof(tmp), ""); /* get the outmapping of previous last entry*/

						if (strlen(tmp)>0)
						{
							/* parse each mapping seperated by ",".Delete the one same as index and minus one for those number larger than index */
							ptmp = strtok(tmp,",");
							k = 0;
						    while (ptmp != NULL)
							{
								tmplen = strtol(ptmp, &pEnd, 10);

								if (k == entrycount-1 && tmplen == -1)
								{ /* skip the last one if it is -1*/
									break;
								}

								if (tmplen != tmpidx)
								{
									sprintf(tmp2, "%s%s%d", tmp2, (k==0)?"":",", (tmplen>tmpidx)?(tmplen-1):tmplen );
									k++;
								}

								ptmp = strtok (NULL, ",");
							}

							arc_cfg_set_idx1(tid, ARC_VO_SIPP_INTACC_x_OutCallMapping, j, tmp2);

							tmp[0]= '\0';
							tmp2[0]= '\0';
						}
						else
							break;
					}

					del_idx = 0;
				}
			}

			if (idx1 < 0 )/* set for parameter without index */
			{
				ret = arc_cfg_set(tid, cfgID, inputs[i].val);
			}
			else
			{
				if (idx2 < 0) /* set for parameter with one index */
				{
					ret = arc_cfg_set_idx1(tid, cfgID, idx1, inputs[i].val);
				}
				else /* set for parameter with two indexes */
				{
					ret = arc_cfg_set_idx2(tid, cfgID, idx1, idx2, inputs[i].val);
				}
			}
			if (ret != 0){
				sprintf(next_page, "%s", "Fail");
				goto done;	/* fail */
			}
		}
	}

	/* done, notify MNG the action */
	if (act_idx < 0) {
		//mng_action(NOTHING, NOTHING);
	}
	else if (param_idx < 0) {
		mng_action(inputs[act_idx].val, NOTHING);
	}
	else {
		mng_action(inputs[act_idx].val, inputs[param_idx].val);
	}

	mapi_ccfg_commit(tid);
	//mng_action("commit", NOTHING);

done:
	/*
	 * Alpha Liu 2012.11.27 issue for PR711AAW
	 * Issue description: httpd send an 302 redirect response
	 *										with wrong location url to browser.
	 * Solution: Only send 200 ok response when page_suffix do not set.
	*/
	if (v_ops->page_suffix != NULL) {
		snprintf(buf, sizeof(buf), "Location: /%s%s\n\n", next_page[0]=='\0' ? "Success" : next_page, v_ops->page_suffix);
		send_response(r, buf);
	}
	else
		answer(r, DOCUMENT_FOLLOWS, NOTHING); //only send 200 ok
	/* end Alpha Liu 2012.11.27 issue for PR711AAW */

mapi_done:
	if (argv)
		free(argv);

	pthread_exit((void *)CGI_CODE);

}
#endif


