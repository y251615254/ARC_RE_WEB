#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


#include "httpd.h"
#include "utils.h"
#include "ssi_handler.h"
#include "vendor_ssi_handler.h"


//#include "mid_mapi_ccfgsal_voip.h"
//#include "mid_mapi_voip.h"
#include "mid_voip.h" 

#include "brnsip_ipc.h"
#include "guiVoip.h"

#define sprintf_s		snprintf//sipp.porting.h
#define strcpy_s(A,B,C) strcpy((A),(C))//baselnx.h


/* VOIP */


SIP_CORE_REGISTER_STATUS_T regStatus_List[_NUM_SIP_ACCOUNT];

int create_brnsip_socket(void)
{
	struct sockaddr_un sockaddrname;
	int socket_fd;

	socket_fd = socket(PF_LOCAL, SOCK_STREAM, 0);
	sockaddrname.sun_family = AF_LOCAL;
	strcpy(sockaddrname.sun_path, socket_path);

	if (connect(socket_fd, (struct sockaddr *)&sockaddrname, SUN_LEN(&sockaddrname)) < 0)
	{
		printf("%s: connect socket_fd fail! errno=%d(%s)\n", __func__, errno, strerror(errno));
		if (socket_fd > 0)
		{
			close(socket_fd);
			socket_fd = -1;
		}
	}

	return(socket_fd);
}

static int get_voip_register_status(void)
{
	int socket_fd;
	char buf[100];

	socket_fd = create_brnsip_socket();
	if (socket_fd > 0) {

		memset(buf, 0, sizeof(buf));
		strcpy(buf, "SIP_CORE_GetRegisterStatus 0 ");
		write(socket_fd, buf, sizeof(buf));

		read(socket_fd, &regStatus_List, sizeof(regStatus_List));
		close(socket_fd);
	}

	return 0;
}

int ssi_get_sip_register_status(struct request_rec *r, int argc, char **argv)
{
	int i;
	char reg_status[20];

	memset(reg_status, 0, sizeof(reg_status));

	get_voip_register_status();

	for (i=0; i<_NUM_SIP_ACCOUNT; i++) {
		switch (regStatus_List[i])
		{
			case SIP_CORE_REGISTER_STATUS_FAIL:
				strcpy(reg_status, "Fail");
				break;
			case SIP_CORE_REGISTER_STATUS_INPROGRESS:
				strcpy(reg_status, "Inprogress");
				break;
			case SIP_CORE_REGISTER_STATUS_SUCCESS:
				strcpy(reg_status, "Success");
				break;
			default:
				strcpy(reg_status, "Fail");
				break;
		}

		if (i == 0)
			r->bytes_sent += so_printf(r, "\"%s\"", reg_status);
		else
			r->bytes_sent += so_printf(r, ",\"%s\"", reg_status);
	}

	return r->bytes_sent;
}

/* get all SIP accounts info */
int ssi_get_sip_account_info(struct request_rec *r, int argc, char **argv)
{
	char *type;
	int index = 0;
	char entry_name[64] = {0};
	char entry_value[512] = {0};
	int tid;
	int j = 0;

	if((tid = get_tid()) == MID_FAIL)
		return 0;

	for (index = 0; index < _NUM_SIP_ACCOUNT; index++)//for (index = 1; index <= _NUM_SIP_ACCOUNT; index++)//jeremy
	{
		r->bytes_sent += so_printf(r, "{\n"); //start of a list

		snprintf(entry_name, sizeof(entry_name), "VAR_SIP_ACC_%d_LINEENABLE", index);
		mapi_ccfg_get_str(tid, entry_name, entry_value, sizeof(entry_value));
		r->bytes_sent += so_printf(r, "\"lineable\":\"%s\",\n", entry_value);

		snprintf(entry_name, sizeof(entry_name), "VAR_SIP_ACC_%d_USERID", index);
		mapi_ccfg_get_str(tid, entry_name, entry_value, sizeof(entry_value));
		r->bytes_sent += so_printf(r, "\"userid\":\"%s\",\n", entry_value);

		snprintf(entry_name, sizeof(entry_name), "VAR_SIP_ACC_%d_DISPLAYNAME", index);
		mapi_ccfg_get_str(tid, entry_name, entry_value, sizeof(entry_value));
		r->bytes_sent += so_printf(r, "\"displayname\":\"%s\",\n", entry_value);

		snprintf(entry_name, sizeof(entry_name), "VAR_SIP_ACC_%d_USEAUTHID", index);
		mapi_ccfg_get_str(tid, entry_name, entry_value, sizeof(entry_value));
		r->bytes_sent += so_printf(r, "\"useauthid\":\"%s\",\n", entry_value);

		snprintf(entry_name, sizeof(entry_name), "VAR_SIP_ACC_%d_AUTHID", index);
		mapi_ccfg_get_str(tid, entry_name, entry_value, sizeof(entry_value));
		r->bytes_sent += so_printf(r, "\"authid\":\"%s\",\n", entry_value);

		snprintf(entry_name, sizeof(entry_name), "VAR_SIP_ACC_%d_REALM", index);
		mapi_ccfg_get_str(tid, entry_name, entry_value, sizeof(entry_value));
		r->bytes_sent += so_printf(r, "\"realm\":\"%s\",\n", entry_value);

		snprintf(entry_name, sizeof(entry_name), "VAR_SIP_ACC_%d_SIPDOMAIN", index);
		mapi_ccfg_get_str(tid, entry_name, entry_value, sizeof(entry_value));
		r->bytes_sent += so_printf(r, "\"sipdomain\":\"%s\",\n", entry_value);

		snprintf(entry_name, sizeof(entry_name), "VAR_SIP_ACC_%d_SIPPORT", index);
		mapi_ccfg_get_str(tid, entry_name, entry_value, sizeof(entry_value));
		r->bytes_sent += so_printf(r, "\"sipport\":\"%s\",\n", entry_value);

		snprintf(entry_name, sizeof(entry_name), "VAR_SIP_ACC_%d_PROXY", index);
		mapi_ccfg_get_str(tid, entry_name, entry_value, sizeof(entry_value));
		r->bytes_sent += so_printf(r, "\"proxy\":\"%s\",\n", entry_value);

		snprintf(entry_name, sizeof(entry_name), "VAR_SIP_ACC_%d_PROXYPORT", index);
		mapi_ccfg_get_str(tid, entry_name, entry_value, sizeof(entry_value));
		r->bytes_sent += so_printf(r, "\"proxyport\":\"%s\",\n", entry_value);

		snprintf(entry_name, sizeof(entry_name), "VAR_SIP_ACC_%d_USEOUTBOUNDPROXY", index);
		mapi_ccfg_get_str(tid, entry_name, entry_value, sizeof(entry_value));
		r->bytes_sent += so_printf(r, "\"useoutboundproxy\":\"%s\",\n", entry_value);

		snprintf(entry_name, sizeof(entry_name), "VAR_SIP_ACC_%d_OUTBOUNDPROXY", index);
		mapi_ccfg_get_str(tid, entry_name, entry_value, sizeof(entry_value));
		r->bytes_sent += so_printf(r, "\"outboundproxy\":\"%s\",\n", entry_value);

		snprintf(entry_name, sizeof(entry_name), "VAR_SIP_ACC_%d_OBPROXYPORT", index);
		mapi_ccfg_get_str(tid, entry_name, entry_value, sizeof(entry_value));
		r->bytes_sent += so_printf(r, "\"obproxyport\":\"%s\",\n", entry_value);

		snprintf(entry_name, sizeof(entry_name), "VAR_SIP_ACC_%d_REGISTRAR", index);
		mapi_ccfg_get_str(tid, entry_name, entry_value, sizeof(entry_value));
		r->bytes_sent += so_printf(r, "\"registar\":\"%s\",\n", entry_value);

		snprintf(entry_name, sizeof(entry_name), "VAR_SIP_ACC_%d_REGISTRARPORT", index);
		mapi_ccfg_get_str(tid, entry_name, entry_value, sizeof(entry_value));
		r->bytes_sent += so_printf(r, "\"registarport\":\"%s\",\n", entry_value);

		snprintf(entry_name, sizeof(entry_name), "VAR_SIP_ACC_%d_REGISTEREXPIRE", index);
		mapi_ccfg_get_str(tid, entry_name, entry_value, sizeof(entry_value));
		r->bytes_sent += so_printf(r, "\"registerexpire\":\"%s\",\n", entry_value);

		snprintf(entry_name, sizeof(entry_name), "VAR_SIP_ACC_%d_DTMFTXMETHOD", index);
		mapi_ccfg_get_str(tid, entry_name, entry_value, sizeof(entry_value));
		r->bytes_sent += so_printf(r, "\"dtmftxmethod\":\"%s\",\n", entry_value);

		snprintf(entry_name, sizeof(entry_name), "VAR_SIP_ACC_%d_USEDNSSRV", index);
		mapi_ccfg_get_str(tid, entry_name, entry_value, sizeof(entry_value));
		r->bytes_sent += so_printf(r, "\"usednssrv\":\"%s\",\n", entry_value);

		snprintf(entry_name, sizeof(entry_name), "VAR_SIP_ACC_%d_TRANSPORTMODE", index);
		mapi_ccfg_get_str(tid, entry_name, entry_value, sizeof(entry_value));
		r->bytes_sent += so_printf(r, "\"transportmode\":\"%s\",\n", entry_value);

		snprintf(entry_name, sizeof(entry_name), "VAR_SIP_ACC_%d_HISTORYMODE", index);
		mapi_ccfg_get_str(tid, entry_name, entry_value, sizeof(entry_value));
		r->bytes_sent += so_printf(r, "\"historymode\":\"%s\",\n", entry_value);

		snprintf(entry_name, sizeof(entry_name), "VAR_SIP_ACC_%d_REJECTFORWARD", index);
		mapi_ccfg_get_str(tid, entry_name, entry_value, sizeof(entry_value));
		r->bytes_sent += so_printf(r, "\"rejectforward\":\"%s\",\n", entry_value);

		snprintf(entry_name, sizeof(entry_name), "VAR_SIP_ACC_%d_REJECTANONYMOUS", index);
		mapi_ccfg_get_str(tid, entry_name, entry_value, sizeof(entry_value));
		r->bytes_sent += so_printf(r, "\"rejectanonymous\":\"%s\",\n", entry_value);

		snprintf(entry_name, sizeof(entry_name), "VAR_SIP_ACC_%d_USENAPTR", index);
		mapi_ccfg_get_str(tid, entry_name, entry_value, sizeof(entry_value));
		r->bytes_sent += so_printf(r, "\"usenaptr\":\"%s\",\n", entry_value);

		snprintf(entry_name, sizeof(entry_name), "VAR_SIP_ACC_%d_BUSYONBUSY", index);
		mapi_ccfg_get_str(tid, entry_name, entry_value, sizeof(entry_value));
		r->bytes_sent += so_printf(r, "\"busyonbusy\":\"%s\",\n", entry_value);

		snprintf(entry_name, sizeof(entry_name), "VAR_SIP_ACC_%d_USESESSTIMER", index);
		mapi_ccfg_get_str(tid, entry_name, entry_value, sizeof(entry_value));
		r->bytes_sent += so_printf(r, "\"usesesstimer\":\"%s\",\n", entry_value);

		snprintf(entry_name, sizeof(entry_name), "VAR_SIP_ACC_%d_REFRESHMETHOD", index);
		mapi_ccfg_get_str(tid, entry_name, entry_value, sizeof(entry_value));
		r->bytes_sent += so_printf(r, "\"refreshmethod\":\"%s\",\n", entry_value);

		snprintf(entry_name, sizeof(entry_name), "VAR_SIP_ACC_%d_OUTCALLREFRESHER", index);
		mapi_ccfg_get_str(tid, entry_name, entry_value, sizeof(entry_value));
		r->bytes_sent += so_printf(r, "\"outcallrefresher\":\"%s\",\n", entry_value);

		snprintf(entry_name, sizeof(entry_name), "VAR_SIP_ACC_%d_INCALLREFRESHER", index);
		mapi_ccfg_get_str(tid, entry_name, entry_value, sizeof(entry_value));
		r->bytes_sent += so_printf(r, "\"incallrefresher\":\"%s\",\n", entry_value);

		snprintf(entry_name, sizeof(entry_name), "VAR_SIP_ACC_%d_SESSEXPIRE", index);
		mapi_ccfg_get_str(tid, entry_name, entry_value, sizeof(entry_value));
		r->bytes_sent += so_printf(r, "\"sessexpire\":\"%s\",\n", entry_value);

		snprintf(entry_name, sizeof(entry_name), "VAR_SIP_ACC_%d_MINEXPIRE", index);
		mapi_ccfg_get_str(tid, entry_name, entry_value, sizeof(entry_value));
		r->bytes_sent += so_printf(r, "\"minexpire\":\"%s\",\n", entry_value);

		for(j = 0; j < SYS_VOIP_CFG_MAX_CODEC_LIST_NUM; j++)//for(j = 1; j <= SYS_VOIP_CFG_MAX_CODEC_LIST_NUM; j++)//jeremy
		{
			snprintf(entry_name, sizeof(entry_name), "VAR_SIP_ACC_%d_CODEC_A_%d", index, j);
			mapi_ccfg_get_str(tid, entry_name, entry_value, sizeof(entry_value));
			r->bytes_sent += so_printf(r, "\"codec_a%d\":\"%s\",\n", j, entry_value);

			snprintf(entry_name, sizeof(entry_name), "VAR_SIP_ACC_%d_CODEC_S_%d", index, j);
			mapi_ccfg_get_str(tid, entry_name, entry_value, sizeof(entry_value));
			r->bytes_sent += so_printf(r, "\"codec_s%d\":\"%s\",\n", j, entry_value);
		}

		snprintf(entry_name, sizeof(entry_name), "VAR_SIP_ACC_%d_USECODEC", index);
		mapi_ccfg_get_str(tid, entry_name, entry_value, sizeof(entry_value));
		r->bytes_sent += so_printf(r, "\"usecodec\":\"%s\"\n", entry_value);

		r->bytes_sent += so_printf(r, "}"); //end of a list
/*//jeremy		
		if (index < _NUM_SIP_ACCOUNT) {
			r->bytes_sent += so_printf(r, ",\n");
		}
*/		
		if (index < _NUM_SIP_ACCOUNT-1) {
			r->bytes_sent += so_printf(r, ",\n");
		}
	}

	return r->bytes_sent;
}

//Get VoIP incoming Log
int cfd = 0;
char console_buf[256];

typedef struct cgi_call_log_t {
	char *mem; // used for free memory
	mid_voip_call_record_t *log;//mid_voip_call_log_t is defined in mid_voip.h, ref. mid_voip_call_record_t in W724
	int log_cnt;
} cgi_call_log;


int ssi_get_VoIP_call_logs(struct request_rec *r, int argc, char **argv)
{
	int ret, mem_len = 0, log_len = 0, cfd = 0, i, out_counter = 0, in_counter = 0;
	char id[16], who[64], date[64], time[64], duration[128];
	cgi_call_log loglist;
	// mid_voip_call_record_t *in_log, *out_log;
	mid_voip_call_record_t *records;
	int size;
	char console_buf[256];
	
#if 0 //for missing call
	int miss_counter = 0;
	mid_voip_call_record_t *miss_log;
#endif
	
	
	cfd = open( "/dev/console", O_RDWR );

	sprintf(console_buf, "[ssi_get_VoIP_call_logs] cgi_call_log_read!!\n");
    write( cfd, console_buf, strlen(console_buf) );
	
	ret = cgi_call_log_read( &loglist );//Get VoIP call logs from middle layer
	
	if( ret==0 )
	{
		//for missing call
		// miss_log = loglist.log;
	
		// in_log = loglist.log;
		// out_log = loglist.log;
		records = loglist.log;
		size = loglist.log_cnt;

		
		sprintf(console_buf, "[ssi_get_VoIP_call_logs] CallLogProcess enter, size %d!!\n", size);
        write( cfd, console_buf, strlen(console_buf) );
		
		
		InCallLogProcess(r, records, size);
		OutCallLogProcess(r, records, size);
		// MissCallLogProcess(r, records, size);
	}
	else
	{
		sprintf(console_buf, "[ssi_get_VoIP_call_logs] Get CallLog fail!!, ret %d!!\n", ret);
        write( cfd, console_buf, strlen(console_buf) );
		r->bytes_sent += so_printf(r, "[\"Read outgoing call log fail!!\"],\n");
		r->bytes_sent += so_printf(r, "[\"Read incoming call log fail!!\"],\n");
	}


	cgi_call_log_free( &loglist );
	close(cfd);

	return 0;
}



//Read VoIP call log from sip module by using socket
int cgi_call_log_read( cgi_call_log *loglist)
{
	// return 0: ok, otherwise error
	int k, socket_fd=-1, size, ret;
	mid_voip_call_record_t *records = 0;
	char buf[100];
	int log_len =0, log_len_max;
	struct timeval      tv;//timeval
	int retval = 0;
	
	if( loglist==0 ) return 1;

	memset( loglist, 0, sizeof(cgi_call_log) );
	socket_fd = create_brnsip_socket();
	if( socket_fd == -1 ) return 2;
	
	
	memset(buf,0,sizeof(buf));
	strcpy(buf, "TEL_LOG_GetCallRecord");
	write(socket_fd, buf, sizeof(buf));
	
	//set socket operation parameter
	tv.tv_sec = 2;
   	tv.tv_usec = 0;
    if(setsockopt(socket_fd, SOL_SOCKET, SO_RCVTIMEO, (char *)&tv, sizeof(struct timeval))<0){
		// printf("[cgi_call_log_read] setsockopt fail!!\n");
    }
	
	//get the number of VoIP call logs
	if ((ret = read(socket_fd, &size, sizeof(size))) <= 0)
	{
		retval = 3;//get VoIP call size fail
		goto END;
    }
	
	loglist->log_cnt = size;
	if (size <= 0)
	{
		loglist->log_cnt = 0;
		retval = 0; // there is no log
		goto END;
	}
	
	loglist->mem = records = (mid_voip_call_record_t *)malloc(sizeof(mid_voip_call_record_t) * size);
	if (records == 0)
	{
		retval = 4;//malloc records fail!!
		goto END;
	}

	log_len_max = sizeof(mid_voip_call_record_t)*size;
	k = 0;
	
	//get the VoIP call logs, including incoming and outgoing call
	if((ret = read(socket_fd, records, log_len_max)) <= 0)
	{
		retval = 5;//read call logs fail
		goto END;
	}


	log_len = ret;
	k++;
	while( log_len<log_len_max && k<6 )
	{
	    ret = read(socket_fd, (char *)records+log_len, log_len_max-log_len);
		if (ret > 0)
		{
			log_len = log_len + ret;
			// printf("[cgi_call_log_read] ret=%d , k = %d, records:%s\n", ret, k, records);
		}

		k++;
	}

	if(log_len != log_len_max)
	{
		retval = 6;//log+len fail!!
		goto END;
	}

	for (k=0; k < size; k++)
	{
		if (k == 0)
		{
			records[k].next = NULL;
		}
		else
		{
			records[k].next = (mid_voip_call_record_t *)&records[k-1];
		}
		if (k == (size-1))
			loglist->log = (mid_voip_call_record_t *)&records[k];
	}

	
END:
	if( socket_fd != -1 )
	{
		close(socket_fd);
		socket_fd = -1;
	}
	if( retval )
	{
		if( loglist ) cgi_call_log_free( loglist );
	}

	return retval;
}


char *utf8_str_fix(char *str, unsigned int str_len )
{
    int i, offset;

	if( str==0 ) return 0;
	if( str_len == 0 )  str_len = strlen(str);

	for( i=0; i < str_len ; i++ ) 
	{
		if ( (str[i] & 0x80) == 0 )  continue;

		if ((str[i] & 0xE0) == 0xC0)//2 bytes
			offset = 1;

		else if ((str[i] & 0xF0) == 0xE0)// 3 bytes
			offset = 2;
	  
		else if ((str[i] & 0xF8) == 0xF0)// 4 bytes
			offset = 3;
		else
			break; // invalid

		//printf("offset, %d+%d %d\n", i, offset, str_len);
		if( i + offset >= str_len )
		{
			str[i] = '\0';
			// printf("[utf8_str_fix] remove truncation\n");
			break;
		}
		i += offset;
	}
	
    return str;
}


char *utf8_strcpy_s( char *dst, unsigned int dstSize, const char *src )
{
    unsigned int len;

    len = strlen(src);
    strcpy_s( dst, dstSize, src );

    if( dstSize <= len )
        len = dstSize - 1;

    utf8_str_fix(dst, len);
    return dst;
}

//static 
void GetCallLog(int counter, mid_voip_call_record_t *records,
			char *id, int id_size, char *who, int who_size,
			char *date, int date_size, char *time, int time_size,
			char *duration, int duration_size, int show_flag)
{
// show_flag: 0x01  duration format: hh:mm:ss, otherwise in seconds
//            0x02  date format: yyyy:MM:dd (US), otherwise dd:MM:yyyy (Germany)

    sprintf_s(id, id_size, "%d", counter);
	if ((strncasecmp(records->peerNumber, "Unavailable", 11) == 0)||(strncasecmp(records->peerNumber, "Private", 7) == 0) ||
		(strncasecmp(records->peerNumber, "anonymous", 9) == 0))
	{
		strcpy_s(who, who_size, "Unbekannt");
	}
	else 
	{
		utf8_strcpy_s(who, who_size, records->peerNumber);
		// strcpy_s(who, who_size, records->peerNumber);
	}

	if (show_flag & 0x02) // language is en
		sprintf_s(date, date_size, "%04d-%02d-%02d", records->startTime.year, records->startTime.month, records->startTime.day);
	else
		sprintf_s(date, date_size, "%02d.%02d.%04d", records->startTime.day, records->startTime.month, records->startTime.year);

	sprintf_s(time, time_size, "%02d:%02d:%02d", records->startTime.hour, records->startTime.min, records->startTime.sec);
	if ((records->type == MID_VOIP_LOG_OUTGOING_SUCCESS) || (records->type == MID_VOIP_LOG_INCOMING_SUCCESS))
	{
		if( duration )
		{
			if (show_flag & 0x01)
				sprintf_s(duration, duration_size, "%02d:%02d:%02d", records->duration/3600, (records->duration%3600)/60, ((records->duration%3600)%60));
			else
				sprintf_s(duration, duration_size, "%ld", records->duration);
		}
	}
	else if (records->type == MID_VOIP_LOG_OUTGOING_NOANSWER)
	{
		if( duration )
		{
			if (show_flag & 0x01 )
				strcpy_s(duration, duration_size, "00:00:00");
			else
				strcpy_s(duration, duration_size, "0");
		}
	}
}


int OutCallLogProcess(struct request_rec *r, mid_voip_call_record_t *records, int size)
{
	char id[16], who[64], date[64], time[64], duration[128];
	int i, out_counter = 0;
	
	
	//for outgoing call log
	r->bytes_sent += so_printf(r, "[\n");
		
	for (i=0; i < size; i++)
	{	
		if ((records->type == MID_VOIP_LOG_OUTGOING_SUCCESS) || (records->type == MID_VOIP_LOG_OUTGOING_NOANSWER))
		{
			GetCallLog(out_counter, records, id, sizeof(id), who, sizeof(who), date, sizeof(date), time, sizeof(time), duration, sizeof(duration), 0x01);
					
			/*outgoing call show format*/		
			r->bytes_sent += so_printf(r, "\"%s\t", date);//dialedcalls_date
			r->bytes_sent += so_printf(r, "%s\t", time);//dialedcalls_time
			r->bytes_sent += so_printf(r, "-----to\t");//dialedcalls_time
			r->bytes_sent += so_printf(r, "%s\t", who);//dialedcalls_who
			r->bytes_sent += so_printf(r, "\t \t \t");
			r->bytes_sent += so_printf(r, "duration:%s\",\n", duration);//dialedcalls_duration
				
		
			out_counter++;
		}
		
		if (records->next == NULL)
			break;
		else
			records = records->next;
	}
	
	r->bytes_sent += so_printf(r, "],\n");
	
	return 0;
}

int InCallLogProcess(struct request_rec *r, mid_voip_call_record_t *records, int size)
{
	char id[16], who[64], date[64], time[64], duration[128];
	int i, in_counter = 0;
	
	//for incoming call log
	r->bytes_sent += so_printf(r, "[\n");		
		
	for (i=0; i < size; i++)
	{	
		if(records->type == MID_VOIP_LOG_INCOMING_SUCCESS)
		{
			GetCallLog(in_counter, records, id, sizeof(id), who, sizeof(who), date, sizeof(date), time, sizeof(time), duration, sizeof(duration), 0x01);
					
			/*incoming call show format*/		
			r->bytes_sent += so_printf(r, "\"%s\t", date);//dialedcalls_date
			r->bytes_sent += so_printf(r, "%s\t", time);//dialedcalls_time
			r->bytes_sent += so_printf(r, "-----from\t");//dialedcalls_time
			r->bytes_sent += so_printf(r, "%s\t", who);//dialedcalls_who
			r->bytes_sent += so_printf(r, "\t \t \t");
			r->bytes_sent += so_printf(r, "duration:%s\",\n", duration);//dialedcalls_duration
				
		
			in_counter++;
		}
		if (records->next == NULL)
			break;
		else
			records = records->next;
	}
	
	r->bytes_sent += so_printf(r, "],\n");
	
	return 0;
}

int MissCallLogProcess(struct request_rec *r, mid_voip_call_record_t *records, int size)
{
	char id[16], who[64], date[64], time[64], duration[128];
	int i, miss_counter = 0;
	
	
	//for missing call log
	r->bytes_sent += so_printf(r, "[\n");
		
	for (i=0; i < size; i++)
	{	
		if(records->type == MID_VOIP_LOG_INCOMING_SUCCESS)
		{
			GetCallLog(miss_counter, records, id, sizeof(id), who, sizeof(who), date, sizeof(date), time, sizeof(time), duration, sizeof(duration), 0x01);
					
			/*misscalllog show format*/		
			r->bytes_sent += so_printf(r, "\"%s\t", id);//dialedcalls_id
			r->bytes_sent += so_printf(r, "%s\t", date);//dialedcalls_date
			r->bytes_sent += so_printf(r, "%s\t", time);//dialedcalls_time
			r->bytes_sent += so_printf(r, "%s\t", who);//dialedcalls_who
			r->bytes_sent += so_printf(r, "%s\",\n", duration);//dialedcalls_duration
				
		
			miss_counter++;
		}
		if (records->next == NULL)
			break;
		else
			records = records->next;
	}		
	
	r->bytes_sent += so_printf(r, "],\n");
	
	return 0;
}

//static 
void cgi_call_log_free( cgi_call_log *loglist )
{
	if( loglist==0 ) return;
	if( loglist->mem ) free(loglist->mem);

	memset( loglist, 0, sizeof(cgi_call_log) );
}



