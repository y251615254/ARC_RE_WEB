/*
 * http_access: Security options etc.
 *
 * Rob McCool
 *
 */

#include "httpd.h"

/* arca-shared */
#include "utils.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


extern security_t *sec;
extern char *bypass_list[];
#if defined(WE410443_SA) || defined(WE5202243_SA) || defined(WE410223_SA) || defined(WE410443_TS) || defined(WE410443_TA) || defined(WE410443_A1) || defined(WE410443_6DX) || defined(WE410443_ZZ)
extern char *valid_url_list[];
extern char *root_valid_url_list[];
#endif

/* Try to find an unused sec */
int get_free_sec(void)
{
	int i;

	for (i=0; i<MAX_SECURITY; i++) {
		if (sec[i].d[0] == '\0')
			return i;
	}

	return -1;
}

int bypass_check(const char *url) 
{
	int i;

	for (i=0; bypass_list[i] != NULL; i++) {
		if (strncasecmp(url, bypass_list[i], strlen(bypass_list[i])) == 0) {
			return 1;
		}
	}

	return 0;
}
#if defined(WE410443_SA) || defined(WE5202243_SA) || defined(WE410223_SA) || defined(WE410443_TS) || defined(WE410443_TA) || defined(WE410443_A1) || defined(WE410443_6DX) || defined(WE410443_ZZ)
int attack_check(const char *url) 
{
	int i;
	char path_name[128] = {0};

	strcpy(path_name, "/tmp/");
	if ((strncasecmp(url, path_name, strlen(path_name)) == 0) && (strstr(url, "_log.log")) )
	{
		return 1;
	}

	
#if 0//defined(WE410443_TS)
	int tid;
	if ((tid = get_tid()) == MID_FAIL) {
		ht_dbg("[%s] Failed to connect to midcore\n", __FUNCTION__);
		return;
	}
		
	if (mapi_ccfg_match_str(tid, "ARC_SYS_PrevilegeLevel", "2"))		//root user
	{

		for (i=0; root_valid_url_list[i] != NULL; i++) {
		if ((strcmp(url, "/") == 0) || (strcmp(url, "") == 0))
		{
			return 1;
		}
		if (strncasecmp(url, root_valid_url_list[i], strlen(root_valid_url_list[i])) == 0) 
		{
			return 1;
		}
		}

		
		strcpy(path_name, "/tmp/RGA_config_");
		if ((strncasecmp(url, path_name, strlen(path_name)) == 0) && (strstr(url, ".bin")) )
		{
			return 1;
		}
	}
#endif
	
	for (i=0; valid_url_list[i] != NULL; i++) {
		if ((strcmp(url, "/") == 0) || (strcmp(url, "") == 0))
		{
			return 1;
		}
		if (strncasecmp(url, valid_url_list[i], strlen(valid_url_list[i])) == 0) 
		{
			return 1;
		}
	}

	return 0;
}
#endif

