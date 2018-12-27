/*
 * http_mime.c: Sends/gets MIME headers for requests
 *
 * Rob McCool
 *
 */
// malone, 27/Feb/2003
#ifndef PARADIGM
#include "string.h"
#endif // PARADIGM
#include "httpd.h"

extern struct mime_ext types[];
extern struct mime_ext encoding_types[];

#define MSG_CODE(x)	MSG_##x

#define MSG_200		"200 OK"
#define MSG_302		"302 Found"
#define MSG_304		"304 Not modified"
#define	MSG_400		"400 Bad Request"
#define	MSG_401		"401 Unauthorized"
#define MSG_403		"403 Forbidden"
#define MSG_404		"404 Not Found"
#define MSG_500		"500 Server Error"
#define MSG_501		"501 Not Implemented"

#define MSG_206		"206 Partial Content"



char *get_status_line(int status)
{
	switch(status)
	{
		case DOCUMENT_FOLLOWS:
			return MSG_200;
		case REDIRECT:
			return MSG_302;
		case USE_LOCAL_COPY:
			return MSG_304;
		case BAD_REQUEST:
			return MSG_400;
		case AUTH_REQUIRED :
			return MSG_401;
		case FORBIDDEN:
			return MSG_403;
		case NOT_FOUND:
			return MSG_404;
		case SERVER_ERROR:
			return MSG_500;
		case NOT_IMPLEMENTED:
			return MSG_501;
		case 206:
			return MSG_206;
		default:
			break;
	}

	return "";
}


//char *strcasestr (const char *phaystack, const char *pneedle);
void find_ct(register struct request_rec *r, int store_encoding)
{
	int p;
	char *ext;


	if ((ext=strrchr(r->url, '.')) == NULL) {
//FIXME: vendor specified macro should be removed
#ifdef VENDOR_BELKIN_SUPPORT
#ifdef NN3_0
		/*
		 * For FW ver API function:
		 * /www/BelkinAPI/APIVersion has ssi function CGI_QUERY() in it.
		 * so we need to handle "APIVersion" with send_parsed_file() to execute CGI_QUERY();
		 * So we need to set its type as INCLUDES_MAGIC_TYPE which is the same type as "*.htm", "*.js", etc.
		 */
		if(strcmp(r->url, "/www/BelkinAPI/APIVersion") == 0)
			r->content_type = INCLUDES_MAGIC_TYPE;
		else
#endif
#endif
		r->content_type = default_type;
		r->content_encoding[0] = '\0';
		return;
	}

	ext++;
	p = 0;

	if (store_encoding) {
		while (encoding_types[p].ext != NULL) {
			if (!strcmp(encoding_types[p].ext, ext)) {
				strcpy(r->content_encoding,
						encoding_types[p].ct);
				break;
			}

			p++;
		}

		p = 0;
	} 
	else 
		r->content_encoding[0] = '\0';

	while (types[p].ext != NULL) {
		if(strcasecmp(types[p].ext, ext)==0) {
			r->content_type = types[p].ct;
			return;
		}

		p++;
	}

	r->content_type = default_type;
}

void probe_content_type(struct request_rec *r) 
{
	find_ct(r, 0);
}

void set_content_type(struct request_rec *r) 
{
	find_ct(r, 1);
}

extern char *months[];
char *wkday(int, int, int);

/* linghong_tan 2013.01.03 
 *
 * Output laste modified time of the requested _file_.
 * (e.g: styles.css)
 */
int set_last_modified(struct request_rec *r) 
{
	/* linghong_tan. 2011.09.28 
	 * make sure the time is legal
	 */
	r->finfo.ftime.day = r->finfo.ftime.day % 30; 
	r->finfo.ftime.month = r->finfo.ftime.month % 12;

	r->finfo.ftime.hour = r->finfo.ftime.hour % 12;
	r->finfo.ftime.minute = r->finfo.ftime.minute % 60;
	r->finfo.ftime.second = r->finfo.ftime.second % 60;

	/* linghong_tan 2012.1.3
	 * localtime(): month could be 0~11
	 */
	snprintf(r->last_modified, sizeof(r->last_modified)-1, "%s, %02d %s %d %02d:%02d:%02d GMT",
		wkday(r->finfo.ftime.year, r->finfo.ftime.month,r->finfo.ftime.day),
		r->finfo.ftime.day, 
		months[r->finfo.ftime.month],
		r->finfo.ftime.year, 
		r->finfo.ftime.hour, 
		r->finfo.ftime.minute,
		r->finfo.ftime.second);

	r->last_modified[sizeof(r->last_modified)-1] = '\0';

	if(!r->ims[0])
		return 0;

	if (later_than(&r->finfo.ftime, r->ims)) {
		answer(r, USE_LOCAL_COPY, NULL_STR);
		return -1;
	}

	return 0;
}


void set_expires(struct request_rec *r, TIME_t *t)
{
	r->expires[0] = '0';
	r->expires[1] = '\0';
	return;
}

int clear_vars(struct request_rec *r) 
{
	r->content_type     = NULL_STR;
	r->last_modified[0] = '\0';
	r->auth_line        = NULL_STR;
	r->ims              = NULL_STR;
	r->referer          = NULL_STR;
	r->content_encoding[0] = '\0';
	r->location[0] = '\0';

	return 0;
}

int get_mime(struct request_rec *r) 
{
	char *header, *value;
	int byte_at_header_end;
	char *tail;

	tail = r->in_buf_read;

	// temporarily modify the byte at end of header to '\0'
	byte_at_header_end = r->in_buf[r->in_header_lens];
	r->in_buf[r->in_header_lens] = '\0';

	while (!is_null_string(tail)) {

		header = tail;
		tail = _getword(header, CR);

		if (*tail == '\0' || *header == '\0') {
			r->in_buf_read = tail;
			r->in_buf_read[0] = byte_at_header_end;

			if (r->in_buf_read != &r->in_buf[r->in_header_lens])
				ht_dbg("warning: in_header_lens is incorrect for in_buf_read\n");

			return 0;
		}

		if ((value = strchr(header, ':')) == NULL)
			continue;

		*value++ = '\0';
		while(isspace(*value)) ++value;

		if(!strcasecmp(header, "Content-type")) {
			if ( strcmp(r->method,"POST")!=0 ) // if it is not post method , skip it
				continue;

			r->content_type = value;
			continue;
		}

		if(!strcasecmp(header, "Authorization")) {
			r->auth_line = value;
			continue;
		}

		if(!strcasecmp(header, "Content-length")) {
			sscanf(value, "%lld", &r->content_length);
			continue;
		}

		if(!strcasecmp(header, "If-modified-since")) {
			r->ims = value;
			continue;
		}

		if (!strcasecmp(header, "Referer")) {
			r->referer = value;
			continue;
		}

		if (!strcasecmp(header, "Host")) {
			char *ptr;
			r->host = value;

			ptr = strchr(value, ':');
			if (ptr) *ptr = '\0';

			continue;
		}

		if (!strcasecmp(header, "Connection")) {
			if (strncasecmp(value, "Keep-Alive", 10) == 0 ) {
				/* TODO: Keep-Alive implement */
				r->keep_alive = 1;
			}
			continue;
		}

		if (!strcasecmp(header, "Cookie")) {
			r->http_cookie_string = value;
			continue;
		}

		if (!strcasecmp(header, "User-Agent")) {
			r->user_agent = value;
			continue;
		}

		if (!strcasecmp(header, "SOAPAction")) {
			r->soapaction = value;
			continue;
		}

		if (!strcasecmp(header, "Accept-Language")) {
			r->accept_lang = value;
			continue;
		}

		if (!strcasecmp(header, "Accept")) {
			r->http_accept = value;
			continue;
		}
		if (!strcasecmp(header, "x-wap-profile")) {
			r->x_wap_profile = 1;
			continue;
		}
		/* Skip the DNT header. 
		 * Otherwise, http2cgi() will modify the remaining data of r->in_buf and destroy the next HTTP header.
		 * As a result, get_mime_headers() will return in advance. 
		 */
		if (!strcasecmp(header, "DNT")) {
			continue;
		}
		/* Skip the Upgrade-Insecure-Requests header & Accept-Encoding header -used  chrome browser. 
		 * Otherwise, http2cgi() will modify the remaining data of r->in_buf and destroy the next HTTP header.
		 */
		if (!strcasecmp(header, "Upgrade-Insecure-Requests")) {
			continue;
		}

		if (!strcasecmp(header, "Accept-Encoding")) {
			continue;
		}	
	
		/*
		 * Ticket-3149,3886    Pete 2015-07-29
		 * http2cgi() will modify the remaining data of r->in_buf and destroy the next HTTP header.
		 * It is useless and harmfu to current httpd.
		 */
		//http2cgi(header);
	}

	return 0;
}


void send_http_header(register struct request_rec *r) 
{
	time_t now;
	char timebuf[MAX_TIMESTAMP_LEN];
	char *status_line = NULL_STR;

	if (r->status)
		status_line = get_status_line(r->status);
	
	so_printf(r, "%s %s\r\n", SERVER_PROTOCOL, status_line);
	now = time( (time_t*) 0 );
	strftime( timebuf, sizeof(timebuf), HTTP_TIME_FORMAT, gmtime( &now ) );
	so_printf(r, "Date: %s\r\n", timebuf);

#if defined(WE410443_SA) || defined(WE5202243_SA) || defined(WE410223_SA) || defined(WE410443_TS) || defined(WE410443_TA) || defined(WE410443_A1) || defined(WE410443_6DX) || defined(WE410443_ZZ)
	so_printf(r, "X-Frame-Options:SAMEORIGIN\r\n");
	so_printf(r, "X-Content-Type-Options:nosniff\r\n");
	so_printf(r, "X-XSS-Protection: 1;mode=block\r\n");
	//so_printf(r, "Content-Security-Policy:default-src 'self'\r\n");
	so_printf(r, "Set-Cookie:key=value;HttpOnly\r\n");
#else
	so_printf(r, "Server: %s\r\n", SERVER_VERSION);
#endif

	if( r->ishttps ||
		(strcasestr(r->url, ".txt") != NULL) || 
		(strcasestr(r->url, ".js") != NULL)  || 
		(strcasestr(r->url, ".jpg") != NULL) || 
		(strcasestr(r->url, ".css") != NULL) || 
		(strcasestr(r->url, ".gif") != NULL) || 
		(strcasestr(r->url, ".cfg") != NULL))
	{
		/* cache */
	}
	else
	{
		so_printf(r, "Pragma: no-cache\r\n");
		so_printf(r, "Cache-Control: no-cache\r\n");
	}

	if (r->content_type && r->content_type[0] != '\0' ) {
		#if defined (VRV9518SWAC33)
		/* 2015-0311, Vergil Wei. Fix the issue the Mac safari changed the name of the  konfiguration.conf to konfiguration.conf.txt. */
		if (strstr (r->url, "konfiguration.conf")) {
			extern char BINARY_TYPE[];

			r->content_type = BINARY_TYPE;
		}
		#endif

		so_printf(r,"Content-type: %s\r\n", r->content_type);

		/* support txt download */
		if (strcmp(r->content_type, "text/plain") == 0 && 
			(strstr(r->url, ".txt") 
			 || strstr(r->url, ".cfg")
			 || strstr(r->url, ".conf") /* Belkin support .conf download */
			 || strstr(r->url, ".log") /* Belkin support .log download */
			 )) 
		{
			if(strstr(r->url, "/tmp/"))
				so_printf(r,"Content-Disposition: attachment; filename=\"%s\"\r\n", (char *)(r->url) + 5);
			else
				so_printf(r,"Content-Disposition: attachment; filename=%s\r\n", r->url);
		}
		#if defined (VRV9518SWAC33)
		/* 2015-0311, Vergil Wei. Fix the issue the Mac safari changed the name of the  konfiguration.conf to konfiguration.conf.txt. */
		else if (strcmp(r->content_type, "application/octet-stream") == 0 &&  (strstr(r->url, ".conf"))) 
		{
			if(strstr(r->url, "/tmp/"))
				so_printf(r,"Content-Disposition: attachment; filename=%s\r\n", (char *)(r->url) + 5);
			else
				so_printf(r,"Content-Disposition: attachment; filename=%s\r\n", r->url);
		}
		#endif
	}


	if (r->content_length >= 0)
		so_printf(r,"Content-length: %lld\r\n", r->content_length);

	if (r->location[0])
		so_printf(r,"Location: %s\r\n", r->location);

	if (r->content_encoding[0])
		so_printf(r,"Content-encoding: %s\r\n", r->content_encoding);

	if (r->session && r->session->cookie_status == 1){
		r->session->cookie_status = 2;
		so_printf(r, "Set-Cookie: SID=%s\r\n", r->session->cookie_val);
	}

	/* TODO: Keep-Alive implement */

	so_printf(r, "Connection: close\r\n");

	if (r->oheader)
		so_printf(r, "%s\r\n", r->oheader);
	else
		so_printf(r, "\r\n");

}


#define MOBILE_COUNTS      6
const char mobile_devices[MOBILE_COUNTS][16]={
	"iPhone",
	"Android",
	"Opera Mini",
	"Palm",
	"BlackBerry",
	"Windows Mobile",
};

#define MobileType 8
typedef struct MobileType_s{
	int Index;
	char Type[12];

}MobileType_t;

struct MobileType_s  MobileType_list[] = {
	{0,"iPhone"}, 
	{1,"phone"}, //windows phone
	{2,"Android"},
	{3,"Opera Mini"},
	{4,"Palm"},
	{5,"BLACKBERRY"},
	{6,"IEMobile"},
	{7,"iPod"},
};


//detect the client which connected to the httpd is whether mobile or not
//0|1|2	illegal value|mobile device|non-mobile device
int check_device_type(char *http_accept, int x_wap_profile, char *cookie,  char *user_agent)
{	
	int  ret = 0;
	char *cp, *view_type;

	/*check cookie first*/
	if (cookie)
	{
		/* get cookie mobile_view_type value*/
		if (NULL != (cp = strstr(cookie, "mobile_view_type=")))
		{
			view_type = &cp[17];
			for( cp = cp + 17; *cp && *cp != '\r' && *cp != '\n' && *cp != ';'; cp++ );
			*cp = '\0';

			if (view_type)
			{
				ret = atoi(view_type);
				//if mobile_view_type is not 1 or 2, it should be illegal.
				if ((ret !=1) && (ret != 2)) ret =0;
			}
		}

		return ret;
	}
	else
	{
		if (x_wap_profile || (http_accept && strstr(http_accept, "application/vnd.wap.xhtml+xml")))
		{
			return 1;
		}
		else
		{
			int i;
			for (i=0; i < MobileType; i++)
			{
				if(user_agent && (strcasestr(user_agent,MobileType_list[i].Type) != NULL ))
					return 1;
			}
			return 0;
		}
	}
}

