/*
 * util.c: misc utility things
 *
 * 3/21/93 Rob McCool
 *
 * 3/17/98 C. H. Lin
 */

/**
 * @defgroup UtilAPI Httpd Utility API
 * @ingroup HTTPD
 * @brief A common Utility  library of Httpd
 *
 */
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <sys/wait.h>   //for WIFEXITED in pclose()

#include "httpd.h"


static pthread_key_t key;
static pthread_once_t init_done = PTHREAD_ONCE_INIT;

int atoi(const char *);

int send_loop(struct request_rec *r, char *buf, int size, char *func)
{
	int	nbytes, len = size,  ret=0;

	while (len > 0) {
#ifdef CONFIG_HTTPD_HTTPS_SUPPORT
		if (r->ishttps) {
			nbytes = len;
			ret = as_write(r->ssrv, (unsigned char *)buf, nbytes);
			if (ret <= 0)
			{
				return -1;
			}
		} else
#endif

		{
			nbytes = send(r->sd, buf, len, 0);
			if (nbytes < 0) {
				char tbuf[512]={0};

				if (errno == EAGAIN || errno == EWOULDBLOCK)
					continue;
				else if (errno == ECONNRESET)
				{
					snprintf(tbuf, sizeof(tbuf)-1, "%s: failed, sock=%d, code=%d[Reset by peer, ECONNRESET], len=%d\n",
							func, r->sd, errno, len);
				}
				else if (errno == EPIPE)
				{
					snprintf(tbuf, sizeof(tbuf)-1, "%s: failed, sock=%d, code=%d[Pipe broken, EPIPE], len=%d\n",
							func, r->sd, errno, len);
				}

				tbuf[sizeof(tbuf)-1] = '\0';
				//log_error(tbuf);

				return -1;
			}
		}

		len -= nbytes;
		buf += nbytes;
	}

	return size;
}

/*
 * so_flush(): send out whole data in outgoing buffer.
 * Return Value
 *      On success, return 0
 *      On error, return -1
 */
int so_flush(register struct request_rec *r)
{
	if (send_loop(r, r->out_buf, r->out_pos, "so_flush()") < 0)
		return -1;

	r->out_pos = 0;
	return 0;
}

/*
 * so_printf(): append a formatted string to the outgoing buffer. if free space
 *              of the buffer is to small to storing this string, flush out the
 *              old data to socket first.
 *
 * History:
 *      Add support for processing SSI documents locally.
 *
 * Return Value
 *      On success, return the number of bytes sent
 *      On error, return -1
 */

int so_printf(register struct request_rec *r, const char *fmt, ...)
{
	char buffer[SOPRINTFBUFSIZE*2];
	va_list argptr;
	int nbytes;


	va_start(argptr, fmt);
	nbytes = vsnprintf(buffer, sizeof(buffer), fmt, argptr);
	va_end(argptr);

	if (r->out_pos + nbytes > IOBUFSIZE)
		if (so_flush(r) == -1)
			goto __failed;

	memcpy((void *)(r->out_buf + r->out_pos), (void *)buffer, nbytes);
	r->out_pos += nbytes;
	if (r->out_pos == IOBUFSIZE)
		if (so_flush(r) == -1)
			goto __failed;

	return nbytes;

__failed:
//	log_error("so_printf");
	r->out_pos = 0;

	return -1;
}

int so_puts(struct request_rec *r, char *buffer)
{
	int nbytes;

	nbytes = strlen(buffer);

	if (r->out_pos + nbytes > IOBUFSIZE)
		if (so_flush(r) == -1)
			goto __failed;

	if (nbytes >= IOBUFSIZE) {
		if (send_loop(r, buffer, nbytes, "so_puts()") < 0)
			goto __failed;
	} else {
		memcpy((void *)(r->out_buf + r->out_pos), (void *)buffer, nbytes);
		r->out_pos += nbytes;
		if (r->out_pos == IOBUFSIZE)
			if (so_flush(r) == -1)
				goto __failed;
	}
	return nbytes;

__failed:
//	log_error("so_puts");
	// the followings will not be executed
	r->out_pos = 0;

	return -1;
}
/*
 * so_putc(): append a single byte to the outgoing buffer. if the buffer is
 *              full, flush it to socket.
 * history:
 *      add support for processing parsed XML file locally
 *
 * Return Value
 *      On success, return the byte be sent
 *      On error, return -1
 */
int so_putc(int c, struct request_rec *r)
{
	r->out_buf[r->out_pos++] = c;
	if (r->out_pos == IOBUFSIZE)
		if (so_flush(r) != 0) {
//			log_error("so_putc");
			r->out_pos = 0;

			return -1;
		}

	return c;       /* success. return the character be sent */
}

int get_char(struct request_rec *r, FILE *stream)
{
#ifdef CONFIG_HTTPD_HTTPS_SUPPORT
	if (r->ishttps) {
		int c;

		as_read(r->ssrv, &c, 1);

		return c;

	} else
#endif
	{
		return fgetc(stream);
	}
}

char *get_line(struct request_rec *r, char *buffer, int size, FILE *stream)
{
	if (!buffer)
		return 0;

#ifdef CONFIG_HTTPD_HTTPS_SUPPORT
	if (r->ishttps) {
		as_read_line(r->ssrv, (unsigned char *)buffer, size);
		return buffer;

	} else
#endif
	{
		return fgets(buffer, size, stream);
	}

}

char *months[] = {
    "Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"
};

int find_month(char *mon)
{
	register int x;

	for(x=0; x<12; x++)
		if(!strcmp(months[x],mon))
			return(x+1);
	return -1;
}

/* Roy owes Rob beer. */
/* This would be considerably easier if strptime or timegm were portable */

int later_than(TIME_t *lms, char *ims)
{
	char *ip;
	char mname[4];  /* month name */
	int year = 0, month = 0, day = 0, hour = 0, min = 0, sec = 0, x;

	/* Whatever format we're looking at, it will start with weekday. */
	/* Skip to first space. */
	if((ip = strchr(ims,' ')) == NULL)
		return 0;
	else
		while(isspace(*ip))
			++ip;

	if(isalpha(*ip)) {
		/* ctime */
		sscanf(ip, "%s %d %d:%d:%d %*s %d",
			mname, &day, &hour, &min, &sec, &year);
	}
	else if(ip[2] == '-') {
		/* RFC 850 (normal HTTP) */
		char t[10];     /* date string, format: dd-mon-yy */
		sscanf(ip,"%s %d:%d:%d",t,&hour,&min,&sec);
		t[2] = '\0';
		day = atoi(t);
		t[6] = '\0';
		strcpy(mname,&t[3]);
		x = atoi(&t[7]);

		/* Prevent wraparound from ambiguity */
		if(x < 70) x += 100;

		year = 1900 + x;
	}
	else {
		/* RFC 822 */
		sscanf(ip, "%d %s %d %d:%d:%d",
			&day, mname, &year, &hour, &min, &sec);
	}

	month = find_month(mname);

	if((x = (int)(lms->year) - year)!=0)
		return x < 0;

	if((x = (int)lms->month - month)!=0)
		return x < 0;

	if((x = (int)lms->day - day)!=0)
		return x < 0;

	if((x = (int)lms->hour - hour)!=0)
		return x < 0;

	if((x = (int)lms->minute - min)!=0)
		return x < 0;

	if((x = (int)lms->second - sec)!=0)
		return x < 0;

	return 1;
}

time_t ht_cvt_time(char *tmStr)
{
	char *ip;
	char mname[4];  /* month name */
	struct tm cur;

	/* Whatever format we're looking at, it will start with weekday. */
	/* Skip to first space. */
	if((ip = strchr(tmStr,' ')) == NULL)
		return 0;
	else
		while(isspace(*ip))
			++ip;

	if(isalpha(*ip)) {
		/* ctime */
		sscanf(ip, "%s %d %d:%d:%d %*s %d",
			mname, &cur.tm_mday, &cur.tm_hour, &cur.tm_min, &cur.tm_sec, &cur.tm_year);
	}
	else if(ip[2] == '-') {
		/* RFC 850 (normal HTTP) */
		int x;
		char t[10];     /* date string, format: dd-mon-yy */
		sscanf(ip,"%s %d:%d:%d",t,&cur.tm_hour,&cur.tm_min,&cur.tm_sec);
		t[2] = '\0';
		cur.tm_mday = atoi(t);
		t[6] = '\0';
		strcpy(mname,&t[3]);
		x = atoi(&t[7]);

		/* Prevent wraparound from ambiguity */
		if(x < 70) x += 100;

		cur.tm_year = 1900 + x;
	}
	else {
		/* RFC 822 */
		sscanf(ip, "%d %s %d %d:%d:%d",
			&cur.tm_mday, mname, &cur.tm_year, &cur.tm_hour, &cur.tm_min, &cur.tm_sec);
	}

	cur.tm_mon = find_month(mname);
	cur.tm_year -= 1900;
	return mktime(&cur);
}

/* Match = 0, NoMatch = 1, Abort = -1 */
/* Based loosely on sections of wildmat.c by Rich Salz */
int strcmp_match(char *str, char *exp)
{
	int x,y;

	for(x=0, y=0; exp[y]; ++y, ++x) {
		if((!str[x]) && (exp[y] != '*'))
			return -1;

		if(exp[y] == '*') {
			while(exp[++y] == '*');

			if(!exp[y])
				return 0;

			while(str[x]) {
				int ret;
				if ((ret = strcmp_match(&str[x++],&exp[y]))!= 1)
                                        return ret;

			}

			return -1;
		} else if((exp[y] != '?') && (str[x] != exp[y]))
			return 1;
	}

	return (str[x] != '\0');
}

int is_matchexp(char *str)
{
	register int x;

	for(x=0;str[x];x++)
		if((str[x] == '*') || (str[x] == '?'))
			return 1;

	return 0;
}

void strsubfirst(int start, char *dest, char *src)
{
	int src_len, dest_len, i;

	if ((src_len = strlen(src)) < start){  /** src "fits" in dest **/
		for (i=0; (dest[i] = src[i]) != '\0'; i++);

		for (i=src_len; (dest[i] = dest[i-src_len+start]) != '\0'; i++);
	}
	else {  /** src doesn't fit in dest **/
		for (dest_len = strlen(dest), i = dest_len+src_len-start;
						i >= src_len; i--)
			dest[i] = dest[i-src_len+start];

		for (i=0; i<src_len; i++) dest[i] = src[i];
	}
}

/*
 * Parse .. so we don't compromise security
 */
void getparents(char *name)
{
	int l=0,w=0;
	const char *lookfor="..";

	while(name[l]!='\0') {
		if (name[l]!=lookfor[w]) {
			(w>0 ? (l-=(w-1),w=0) : l++);
			continue;
		}

		if (lookfor[++w]=='\0') {
			if( ( (name[l+1]=='\0') || (name[l+1]=='/')) &&
			   (((l > 3) && (name[l-2] == '/')) || (l<=3))) {
				register int m=l+1, n;
				l=l-3;

				if(l>=0) {
				 while((l!=0) && (name[l]!='/')) --l;
				}
				else l=0;

				n=l;
				while((name[n]=name[m])!='\0') (++n,++m);
				w=0;
			}
			else w=0;
		}
		else ++l;
	}
}
/*
void no2slash(char *name) {
	register int x, y;

	for(x=0; name[x]; x++)
		if(x && (name[x-1] == '/') && (name[x] == '/'))
			for (y=x; name[y]; y++);
				name[y] = name[y+1];
}
*/
void make_dirstr(char *s, int n, char *d)
{
	register int x,f;

	for(x=0,f=0;s[x];x++) {
		if((d[x] = s[x]) != '/') continue;

		if((++f) == n) {
			if (n==1) d[x+1] = '\0';
			else d[x] = '\0';

			return;
		}
	}

	d[x] = '\0';
}

void strcpy_dir(char *d, const char *s)
{
	register int x;

	for(x=0;s[x];x++)
		d[x] = s[x];

	if(s[x-1] != '/') d[x++] = '/';

	d[x] = '\0';
}

void http2cgi(char *w)
{
	register int x;

	for(x=strlen(w);x != -1; --x)
		w[x+5]= (w[x] == '-' ? '_' : toupper(w[x]));

	strncpy(w, "HTTP_", 5);
}

char *makeword(char *line, char stop)
{
	int x=0, y;
	char *word;

	if ((y = ind(line, stop)) == -1)
		y = strlen(line);

	word = (char *) malloc( sizeof(char)*(y + 1));
	// check whether the word is valid.
	if(word == NULL)
		return NULL;
	for(x=0;((line[x]) && (line[x] != stop));x++)
		word[x] = line[x];

	word[x] = '\0';
	if(line[x]) ++x;
		y=0;

	while((line[y++]=line[x++])!='\0');

	return word;
}

void getword(char *word, char *line, char stop)
{
	int x = 0, y;

	for(x=0; ((line[x]) && (line[x] != stop)); x++)
		word[x] = line[x];

	word[x] = '\0';

	if (line[x])
		++x;

	y=0;

	while((line[y++] = line[x++])!='\0')
		;
}


/*
 * The high efficiency edition of string process routines.
 * These two function as the same as the above but prevent
 * no necessary memory movement, just use pointer to locate
 * the data we need.
 *
 * USE CAREFULLY, PLEASE.
 */

/*
 * _getword():
 *      line - data buffer, no prefixed blank.
 *      stop - delimiter
 *      return: the beginning of next token in buffer.
 *      side effect: change the postfixed delimiter to '\0'
 * hint:
 *      before calling _getword(), set up a pointer points to
 *      the buffer, we'll get a token after _getword() returns.
 */
char *_getword(char *line, char stop)
{
	while (*line && *line != stop && *line != '\n') line++;

	if (*line == stop) {
		*line++ = '\0';
		if (stop == CR && *line == LF)
			*line++ = '\0';
	}

	return line;
}


void plustospace(char *str)
{
	register int x;

	for(x=0; str[x]; x++) if(str[x] == '+') str[x] = ' ';
}

void spacetoplus(char *str)
{
	register int x;

	for(x=0; str[x]; x++) if(str[x] == ' ') str[x] = '+';
}

//trim non-printing characters at either end
char *str_trim( char *str, int str_len, int *del_len )
{
	int I=1 , L, Del=0 ;

	if(del_len) *del_len = 0;
	if( !str ) return 0 ;

	L=str_len;
	while ( (I<=L) && (str[I-1]<=' '&&str[I-1]>0) ){
		I++ ;    // left
		Del++;
	}

	if ( I > L){
		str[0]='\0' ;
	}else{
		while ( str[L-1]<=' '&&str[L-1]>0 ){
			L-- ;  // right
			Del++;
		}
		str[L]='\0';

		if ( I-1>0 )
			strcpy( str, str+I-1 );
	}

	if(del_len) *del_len = Del;

	return str ;
}

int get_param_value( const char *str, char deli_token, const char *para_name, char *value, int value_size )
{
	// str: para_name1=value1&para_name2=value2 ...
	//MAX_STRING_LEN maybe need enlarge in future
	char line[MAX_STRING_LEN], name[MAX_STRING_LEN/2], val[MAX_STRING_LEN/2] ;
	int x ;

	if (value_size <= 0)
	{
		return 0;
	}

	value[0] = '\0';
	strncpy( line, str, sizeof(line) );
	line[sizeof(line)-1] = '\0';

	for(x=0; line[0] != '\0'; x++ ) {
		getword( val, line, deli_token );
		plustospace( val );
		unescape_url( val );
		getword( name, val, '=' );
		str_trim( name, strlen(name), 0 );

		if ( strcmp( para_name, name ) == 0 ){ // found
			strncpy( value, val, value_size ) ;
			value[value_size-1]='\0';
			ht_dbg("value=%s\n", value);

			return 1 ;
		}
	}

	return 0; // not found
}

/**************************************************************************************************
FUNCTION:  getQueryValue
PURPOSE:   get the specified parameter's value of URL's QUERY_STRING

PARAMETERS:
	[In]struct request_rec 	*r
	[In]char *para_name: parameter name
	[Out]char *value: parameter value
RETURN:
	1 if succeeded or 0 if parameter name not exists.
***************************************************************************************************/
int get_query_value(struct request_rec *r, const char *para_name, char *value, int value_size )
{
	// Request-URI = abs_path?para_name1=value1&para_name2=value2 ...
	// struct request_rec *r: r->args == "para_name1=value1&para_name2=value2"
	return get_param_value( r->args, '&', para_name, value, value_size );
}

char *strcpy_guard(char *dst, unsigned int dstSize, const char *src)
{
	char *ret;
	unsigned int srcSize = strlen(src);

	if (dstSize > srcSize)
	{
		ret = strcpy(dst, src == NULL ? "" : src);
	}
	else
	{
		ret = strncpy(dst, src == NULL ? "" : src, dstSize - 1);
		dst[dstSize - 1] = '\0';
		ht_dbg("[strcpy_guard] B.O.\n");
	}

	return ret;
}

char *strcat_guard(char *dst, unsigned int dstSize, const char *src)
{
      char *ret;
      unsigned int srcSize = strlen(src);
      unsigned int dstStrSize = strlen(dst);
      unsigned int newDstSize;

      if (dstStrSize >= dstSize)
      {
            return NULL;
      }

      newDstSize = dstSize - dstStrSize;
      if (newDstSize > srcSize)
      {
            ret = strcat(dst, src);
      }
      else
      {
            ret = strncat(dst, src, newDstSize - 1);
            dst[dstSize - 1] = '\0';
            //cprintf("[strcat_s] B.O.\n");
      }

      return ret;
}

int sprintf_guard(char *buf, unsigned int bufSize, char *fmt, ...)
{
    int count;
    va_list args;

    va_start(args,fmt);         /* get variable arg list address */
    //count = mt_vsprintf(buf,fmt,args);  /* process fmt & args into buf */
    count = vsnprintf(buf,bufSize,fmt,args);  /* process fmt & args into buf */

      if (count >= bufSize)
	{
		buf[bufSize-1] = '\0';
		count = bufSize-1; // ps: should not change the return value
		//cprintf("[sprintf_s] B.O.\n");
	}

    return count;
}


char x2c(char *what)
{
	register char digit;

	digit = ((what[0] >= 'A') ? ((what[0] & 0xdf) - 'A')+10 :
				(what[0] - '0'));
	digit *= 16;
	digit += (what[1] >= 'A' ? ((what[1] & 0xdf) - 'A')+10 :
				(what[1] - '0'));
	return(digit);
}

void unescape_url(char *url)
{
	register int x,y;

	for (x=0,y=0; url[y]; ++x,++y) {
		if ((url[x] = url[y]) == '%') {
			url[x] = x2c(&url[y+1]);
			y+=2;
		}
	}

	url[x] = '\0';
}

#define c2x(what,where)		sprintf(where,"%%%2x", what)

void escape_url(char *url)
{
	register int x,y;
	char copy[MAX_STRING_LEN];

	strcpy(copy, url);

	for(x=0,y=0;copy[x];x++,y++) {
		if(ind("% ?+&",url[y] = copy[x]) != -1) {
			c2x(copy[x],&url[y]);
			y+=2;
		}
	}

	url[y] = '\0';
}

void make_full_path(const char *src1,const char *src2,char *dst)
{
	register int x = 0,y;

	if (src1==NULL || src1[0]=='\0')
		dst[x++]='/';
	else
		while ((dst[x]=src1[x]) != '\0') x++;

	if (dst[x-1] != '/')
		dst[x++] = '/';

	for(y=0; (dst[x]=src2[y])!='\0'; x++,y++);
}

int is_url(char *u)
{
	register int x;

	for(x=0;u[x] != ':';x++)
		if((!u[x]) || (!isalpha(u[x])))
			return 0;

	if((u[x+1] == '/') && (u[x+2] == '/'))
		return 1;
	else return 0;
}

int ind(const char *s, char c)
{
	register int x;

	for(x=0;s[x];x++)
		if(s[x] == c) return x;

	return -1;
}

int rind(char *s, char c)
{
	register int x;

	for(x=strlen(s)-1;x != -1;x--)
		if(s[x] == c) return x;

	return -1;
}

void str_tolower(char *str)
{
	while(*str) {
		*str = tolower(*str);
		++str;
	}
}


void construct_url(struct request_rec *r, char *d, char *s, char *local_host, unsigned short port, unsigned char ishttps)
{
	if (port == DEFAULT_HTTP_PORT) {
		snprintf(d, MAX_STRING_LEN-1, "http://%s%s", local_host,s);
	}
#ifdef CONFIG_HTTPD_HTTPS_SUPPORT
	else if (ishttps) {
		//crazy_liang 2013.9.9: maybe the port for HTTPS is not default (443).
		if (port == DEFAULT_HTTPS_PORT) {
		snprintf(d, MAX_STRING_LEN-1, "https://%s%s", local_host, s);
		} else {
			snprintf(d, MAX_STRING_LEN - 1, "https://%s:%d%s", local_host, port, s);
		}
		//end crazy_liang 2013.9.9
	}
#endif
	else {
		snprintf(d, MAX_STRING_LEN-1, "http://%s:%d%s", local_host, port, s);
	}
	/* escape_url(d); */

	ht_dbg("construct_url: %s\n", d);
}

/* aaaack but it's fast and const should make it shared text page. */
const unsigned char pr2six[256]={
	64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,
	64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,62,64,64,64,63,
	52,53,54,55,56,57,58,59,60,61,64,64,64,64,64,64,64,0,1,2,3,4,5,6,7,8,9,
	10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,64,64,64,64,64,64,26,27,
	28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,
	64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,
	64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,
	64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,
	64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,
	64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,
	64,64,64,64,64,64,64,64,64,64,64,64,64
};

void uudecode(char *bufcoded, unsigned char *bufplain, int outbufsize) {
	int nbytesdecoded;
	register unsigned char *bufin;
	register unsigned char *bufout = bufplain;
	register int nprbytes;

	/* Strip leading whitespace. */

	while(*bufcoded==' ' || *bufcoded == '\t') bufcoded++;

	/* Figure out how many characters are in the input buffer.
	 * If this would decode into more bytes than would fit into
	 * the output buffer, adjust the number of input bytes downwards.
	 */
	bufin = (unsigned char *)bufcoded;
	while(pr2six[*(bufin++)] <= 63);
	nprbytes = (char *)bufin - bufcoded - 1;
	nbytesdecoded = ((nprbytes+3)/4) * 3;
	if(nbytesdecoded > outbufsize) {
		nprbytes = (outbufsize*4)/3;
	}

	bufin = (unsigned char *)bufcoded;

	while (nprbytes > 0) {
		*(bufout++) = (unsigned char) ((pr2six[*bufin] << 2)|
							(pr2six[bufin[1]] >> 4));
		*(bufout++) = (unsigned char) ((pr2six[bufin[1]] << 4)|
							(pr2six[bufin[2]] >> 2));
		*(bufout++) = (unsigned char) ((pr2six[bufin[2]] << 6) |
							pr2six[bufin[3]]);
		bufin += 4;
		nprbytes -= 4;
	}

	if(nprbytes & 03) {
		if(pr2six[bufin[-2]] > 63)
			nbytesdecoded -= 2;
		else
			nbytesdecoded -= 1;
	}
	bufplain[nbytesdecoded] = '\0';
}

int is_null_string(char *ptr)
{
	if (ptr == NULL) return 1;
	if (*ptr == '\0') return 1;

	return 0;
}
int is_ipv6(const char *ip)
{
#ifdef IPV6_SUPPORT
	unsigned char buf[sizeof(struct in6_addr)];
	int s;

	s = inet_pton(AF_INET6, ip, buf);
	if (s <= 0)
		return 0;
	else
		return 1;
#endif

	return 0;
}
int is_localhost(const char *ip)
{
	unsigned long my_ip, my_netmask, peer;

	if (is_ipv6(ip))
		return 1;	/* IPv6 alway from LAN, no nat */

	peer = inet_addr(ip);
	if (peer == INADDR_NONE)
		return 0;

	my_ip = inet_addr(lanip_str);
	my_netmask = inet_addr(lanmask_str);
	if ((my_ip & my_netmask) == (peer & my_netmask))
		return 1;

	return 0;
}

int strrncmp(const char *s1, const char *s2, size_t max_len)
{
    size_t idx1, idx2;

    if( (s1 == 0) && (s2 == 0) )
    	return 0;
    if(s1 == 0)
		return (-*s2);
    if(s2 == 0)
		return (*s1);

	idx1 = strlen(s1) - 1;
	idx2 = strlen(s2) - 1;

    while (idx1 >= 0 && idx2 >= 0 &&  max_len > 0)
		if (s1[idx1] == s2[idx2]) {
		    idx1--; idx2--; max_len--;
		} else
		    return (s1[idx1]-s2[idx1]);

    if (max_len > 0) {
		if (idx1 == 0) return -s2[idx2];

		return s1[idx1];
    } else
		return 0;
}


//change to capital letters
void to_upper(char s[])
{
	int i;

	for(i=0;s[i]!='\0';i++)
	{
		if(s[i]>='a' && s[i]<='z')
			s[i] -= 32;
	}
}

char *get_mac_by_ip(char *ip, char mac[])
{
	FILE *fp;
	char line[80], *p;
    char ipaddr[MAX_IP_STRING];
	char hwaddr[20];
    int type;
	int flags;
	int i = 0;

	if (!ip)
		return mac;

#ifdef IPV6_SUPPORT
	if (is_ipv6(ip)) {
		sprintf(line, "ip -6 neigh | grep %s > /tmp/neigh.txt", ip);
		system(line);

		usleep(100);
		if ((fp = fopen("/tmp/neigh.txt", "r")) == NULL)
		{
			to_upper(mac);
			return mac;
		}

		if (fgets(line, sizeof(line), fp))
		{
			if(strlen(line) > 0)
			{
				p = strstr(line, "lladdr");
				strncpy(mac, p+7, 17);
			}
		}
		fclose(fp);
		unlink("/tmp/neigh.txt");

		to_upper(mac);
		return mac;
	}
#endif

	if ((fp = fopen("/proc/net/arp", "r")) == NULL)
		return mac;

	for (i=0; fgets(line, sizeof(line), fp); i++) {
		if (i == 0)
			continue;

		if (sscanf(line, "%s 0x%x 0x%x %100s\n", ipaddr, &type, &flags, hwaddr) != 4)
			continue;

		if(strcmp(ip, ipaddr))
			continue;

		sprintf(mac, "%s", hwaddr);
		fclose(fp);

		return mac;
	}

	fclose(fp);

	return mac;
}


// strcasestr() added by chlin, 2007, May 10th
typedef unsigned int chartype;
#undef a

char *strcasestr(const char *phaystack, const char *pneedle)
{
  register const unsigned char *haystack, *needle;
  register chartype bl, bu, cl, cu;

  haystack = (const unsigned char *) phaystack;
  needle = (const unsigned char *) pneedle;

  bl = tolower (*needle);
  if (bl != '\0')
    {
      bu = toupper (bl);
      haystack--;				/* possible ANSI violation */
      do
	{
	  cl = *++haystack;
	  if (cl == '\0')
	    goto ret0;
	}
      while ((cl != bl) && (cl != bu));

      cl = tolower (*++needle);
      if (cl == '\0')
	goto foundneedle;
      cu = toupper (cl);
      ++needle;
      goto jin;

      for (;;)
	{
	  register chartype a;
	  register const unsigned char *rhaystack, *rneedle;

	  do
	    {
	      a = *++haystack;
	      if (a == '\0')
		goto ret0;
	      if ((a == bl) || (a == bu))
		break;
	      a = *++haystack;
	      if (a == '\0')
		goto ret0;
shloop:
	      ;
	    }
	  while ((a != bl) && (a != bu));

jin:	  a = *++haystack;
	  if (a == '\0')
	    goto ret0;

	  if ((a != cl) && (a != cu))
	    goto shloop;

	  rhaystack = haystack-- + 1;
	  rneedle = needle;
	  a = tolower (*rneedle);

	  if (tolower (*rhaystack) == (int) a)
	    do
	      {
		if (a == '\0')
		  goto foundneedle;
		++rhaystack;
		a = tolower (*++needle);
		if (tolower (*rhaystack) != (int) a)
		  break;
		if (a == '\0')
		  goto foundneedle;
		++rhaystack;
		a = tolower (*++needle);
	      }
	    while (tolower (*rhaystack) == (int) a);

	  needle = rneedle;		/* took the register-poor approach */

	  if (a == '\0')
	    break;
	}
    }
foundneedle:
  return (char*) haystack;
ret0:
  return 0;
}

/* Parse the cname, idx_x, idx_y, in form of cname+idx_x+idx_y, where idx_x and idx_y might be absent*/
int parse_cname_idx(char *cname, int *idx_x, int *idx_y)
{
	//int i =0, pcname=0; // no used
	int i =0;
	char *ptmp, *pEnd;

	ptmp = strtok(cname,"+");

	while (ptmp != NULL)
	{
	        switch(i)
	        {
	           case 1: *idx_x = strtol(ptmp, &pEnd, 10);
	                   break;
	           case 2: *idx_y = strtol(ptmp, &pEnd, 10);
	                   break;
	        }

	        ptmp = strtok (NULL, "+");
	        i++;
	}
	return 0; //hugh bug fix
}


/* Find the configID of Arcadyan abstract layer using the config name passed from GUI */
int find_cfgID_by_cname(char *cname)
{
	int cfgID = 0;
	char *pEnd;

	cfgID = strtol (cname,&pEnd,10); /* change this line if the project use other config mapping approach */

	return cfgID;
}

/* check if the format is for arccfg */
int arccfgFormat(char *cfgname)
{
	int i =0;

	for (i = 0; cfgname[i]!='\0'; i++)
	{
#ifdef CONFIG_HTTPD_SUPPORT_AES256
		if(cfgname[i] == '*') continue; //SKIP "*" it means a encryptioed data
#endif //CONFIG_HTTPD_SUPPORT_AES256
		if (cfgname[i]>'9' || cfgname[i]<'0')
			return 0;
	}

	return 1;
}

/* parse the config name in format "cfgIDxxxyyy" where xxx and yyy are 3 bytes represent idx_x and idx_y accordingly
 * 2014/5/7: hugh extend to support encryption mode which last digit is "*"
 *
 */
int parseCfgIdx(char *name, int *cfgID, int *idx1, int *idx2, int *enc)
{
	//char *p, *pidx1, *pidx2, *pEnd, tmp[32];
	char *pEnd, tmp[32];
	int len =0;
	*cfgID = *idx1 = *idx2 = -1;
	*enc=0;

	len = strlen(name);
#ifdef CONFIG_HTTPD_SUPPORT_AES256
	if(name[len-1]=='*')
	{
		name[len-1]='\0';
		*enc=1; // a encrypted mode
		len--;
	}
#endif //CONFIG_HTTPD_SUPPORT_AES256
	tmp[3] = '\0';
	tmp[2] = name[len -1];
	tmp[1] = name[len -2];
	tmp[0] = name[len -3];
	*idx2 = strtol (tmp,&pEnd,10) - 1;

	tmp[3] = '\0';
	tmp[2] = name[len -4];
	tmp[1] = name[len -5];
	tmp[0] = name[len -6];
	*idx1 = strtol (tmp,&pEnd,10) - 1;

	name[len - 6] = '\0';
	*cfgID = strtol (name,&pEnd,10);

	return 0;
}


/* Find first process pid with same name from ps command */
int find_pid_by_ps(char* name)
{
	FILE * fp;
	int pid= -1 ;
	char line[256];

	if ((fp = popen("ps", "r"))) {
		while ( fgets(line, sizeof(line), fp) != NULL ) {
			if(strstr(line, name)){
				sscanf(line, "%d", &pid);
				printf("%s pid is %d\n", name, pid);
				break;
			}
		}
		pclose(fp);
	}

	return pid;
}

static void* thread_stub(void *th_info)
{
	struct th_info th;
	long *argv;
	void (* script)();
	void *arg1, *arg2, *arg3;


	memcpy(&th, th_info, sizeof(struct th_info));

	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, 0);

	argv = (long *) th.args_list;

	/* install signal handlers */
	set_signals();
	sig_ignore_reload_httpd();

	switch (th.type)
	{
		case TH_STD:	/* STD thread */
			script = (void (*)(void *, void *))argv[0];
			arg1 = (void *)argv[1];
			arg2 = (void *)argv[2];

			/* call routine */
			if (th.args_num == 3) {
				arg3 = (void *)argv[3];
				script(arg1, arg2, arg3);
			}
			else
				script(arg1, arg2);

			break;

		case TH_CGI:	/* CGI thread */
			script = (void (*)(int , char **, void *))argv[0];
			arg1 = (void *)argv[1];
			arg2 = (void *)argv[2];
			arg3 = (void *)argv[3];
			//arg4 = (void *)argv[4];

			//if (arg2)
				//ht_dbg("argc=%d, argv=%s\n", *(int *)arg1, *(char **)arg2);

			/* call routine */
			script(*(int *)arg1, (char **)arg2, arg3);
			break;

#ifdef HNAP_SUPPORT
		/* TODO: hnap implement */
		case TH_HNAP:	/* HNAP thread */
			script = (void (*)(void *))argv[0];
			arg1 = (void *)argv[1];

			/* call routine */
			script(arg1);
			break;
#endif
		default:
			break;

	}
	free(th_info);

	pthread_exit(NULL);
}

int ht_create_thread(struct th_info *th_info)
{
	struct th_info *th;
	size_t stacksize;
	pthread_attr_t attr;

	if (!th_info)
		return -1;

	if ((th = malloc(sizeof(struct th_info))) == NULL)
		return -1;

	memcpy(th, th_info, sizeof(struct th_info));

	pthread_attr_init(&attr);
	stacksize = (size_t)th->stacksize;

	pthread_attr_setstacksize (&attr, stacksize);
#if 0
	/* need join CGI task */
	if (th->type == TH_CGI
#ifdef HNAP_SUPPORT
		|| th->type == TH_HNAP
#endif
		) {
		pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
	}
	else
		pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
#else
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
#endif

	if (pthread_create((pthread_t *)&th->pthread_id, &attr, (void*)thread_stub, (void *)th) != 0) {
		free(th);
		return -1;
	}

	th_info->pthread_id = th->pthread_id;

	pthread_attr_destroy(&attr);


	return 0;	/* success */
}

void destructor(void *p)
{
	//printf("destructor\n");
	if(p != NULL)
	{
		int tid = *(int*)p;
		if(tid > 0)
		{
			//printf("close tid %d\n", tid);
			mapi_end_transc(tid);
		}
		free(p);
	}
}

static void
thread_init(void)
{
	//printf("thread init\n");
	pthread_key_create(&key, destructor);
}

int get_tid(void)
{
	int *p;
	pthread_once(&init_done, thread_init);

	p = (int *) pthread_getspecific(key);

	if(p == NULL)
	{
		p = (int *) malloc(sizeof(int));
		if(p == NULL)
		{
			// out of mem
			return MID_FAIL;
		}
		*p = -1;
		//printf("new specific\n");
		pthread_setspecific(key, p);
	}

	if(*p < 0)
	{
		*p = mapi_start_transc();

		if(*p == MID_FAIL)
			return MID_FAIL;
	}
	return *p;
}

// ================================================
/*
* base64 encoder
*
* encode 3 8-bit binary bytes as 4 '6-bit' characters
*/
char *b64_encode( unsigned char *src ,int src_len, unsigned char* space, int space_len)
{
    static const char cb64[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

    unsigned char *out = space;
    unsigned char *in=src;
    int sub_len,len;
    int out_len;

   out_len=0;

   if (src_len < 1 ) return NULL;
   if (!src) return NULL;
   if (!space) return NULL;
   if (space_len < 1) return NULL;


   /* Required space is 4/3 source length  plus one for NULL terminator*/
   if ( space_len < ((1 +src_len/3) * 4 + 1) )return NULL;

   memset(space,0,space_len);

   for (len=0;len < src_len;in=in+3, len=len+3)
   {

    sub_len = ( ( len + 3  < src_len ) ? 3: src_len - len);

    /* This is a little inefficient on space but covers ALL the
       corner cases as far as length goes */
    switch(sub_len)
    	{
    	case 3:
        	out[out_len++] = cb64[ in[0] >> 2 ];
		out[out_len++] = cb64[ ((in[0] & 0x03) << 4) | ((in[1] & 0xf0) >> 4) ] ;
		out[out_len++] = cb64[ ((in[1] & 0x0f) << 2) | ((in[2] & 0xc0) >> 6) ] ;
		out[out_len++] = cb64[ in[2] & 0x3f ];
		break;
    	case 2:
        	out[out_len++] = cb64[ in[0] >> 2 ];
		out[out_len++] = cb64[ ((in[0] & 0x03) << 4) | ((in[1] & 0xf0) >> 4) ] ;
		out[out_len++] = cb64[ ((in[1] & 0x0f) << 2) ];
		out[out_len++] = (unsigned char) '=';
    		break;
    	case 1:
        	out[out_len++] = cb64[ in[0] >> 2 ];
		out[out_len++] = cb64[ ((in[0] & 0x03) << 4)  ] ;
		out[out_len++] = (unsigned char) '=';
		out[out_len++] = (unsigned char) '=';
		break;
    	default:
		break;
    		/* do nothing*/
    	}
   }
   out[out_len]='\0';
   return (char *)out;
}

/* Base-64 decoding.  This represents binary data as printable ASCII
** characters.  Three 8-bit binary bytes are turned into four 6-bit
** values, like so:
**
**   [11111111]  [22222222]  [33333333]
**
**   [111111] [112222] [222233] [333333]
**
** Then the 6-bit values are represented using the characters "A-Za-z0-9+/".
*/

static int b64_decode_table[256] = {
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* 00-0F */
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* 10-1F */
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,62,-1,-1,-1,63,  /* 20-2F */
    52,53,54,55,56,57,58,59,60,61,-1,-1,-1,-1,-1,-1,  /* 30-3F */
    -1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,  /* 40-4F */
    15,16,17,18,19,20,21,22,23,24,25,-1,-1,-1,-1,-1,  /* 50-5F */
    -1,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,  /* 60-6F */
    41,42,43,44,45,46,47,48,49,50,51,-1,-1,-1,-1,-1,  /* 70-7F */
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* 80-8F */
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* 90-9F */
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* A0-AF */
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* B0-BF */
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* C0-CF */
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* D0-DF */
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* E0-EF */
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1   /* F0-FF */
    };

/* Do base-64 decoding on a string.  Ignore any non-base64 bytes.
** Return the actual number of bytes generated.  The decoded size will
** be at most 3/4 the size of the encoded, and may be smaller if there
** are padding characters (blanks, newlines).
*/
int
b64_decode( const char* str, unsigned char* space, int size )
    {
    const char* cp;
    int space_idx, phase;
    int d, prev_d=0;
    unsigned char c;

    space_idx = 0;
    phase = 0;
    for ( cp = str; *cp != '\0'; ++cp )
	{
	d = b64_decode_table[(int)*cp];
	if ( d != -1 )
	    {
	    switch ( phase )
		{
		case 0:
		++phase;
		break;
		case 1:
		c = ( ( prev_d << 2 ) | ( ( d & 0x30 ) >> 4 ) );
		if ( space_idx < size )
		    space[space_idx++] = c;
		++phase;
		break;
		case 2:
		c = ( ( ( prev_d & 0xf ) << 4 ) | ( ( d & 0x3c ) >> 2 ) );
		if ( space_idx < size )
		    space[space_idx++] = c;
		++phase;
		break;
		case 3:
		c = ( ( ( prev_d & 0x03 ) << 6 ) | d );
		if ( space_idx < size )
		    space[space_idx++] = c;
		phase = 0;
		break;
		}
	    prev_d = d;
	    }
	}
    return space_idx;
}
// BASE64 dump
// 2014/3/25 hugh add
/*
 *
 *  parsing buffer content (entries delimited by "\n" or " "<space> and store as entry list array
 */
 struct words_s *wordslist_init(char *buf, struct words_s *wordlist, int max_size)
{
	char *c=(char *) buf;
	int count=0;

	count=0; //reuse variable to store count

	wordlist=(words *) malloc(sizeof(words)*(count+1));

	// protect first entry
	while(*c){
		if(*c !=0x09 && *c!=0x20 && *c!=0x0a && *c!=0x0d) break;
		c++;
	}
	wordlist[0].key=c;

	while(*c){
		if(*c ==0x9 || *c==0x20 || *c== 0x0a)
		{
			*c='\0';
			// check till non \t \r \n \s code
			while(*(++c)){
				if(*c !=0x09 && *c!=0x20 && *c!=0x0a && *c!=0x0d) break;
			}
			// realloc for next if available
			if(strlen(c) > 1)
			{
				//cprintf("buf[%s]\n", wordlist[count].key);
				count++;
				wordlist=(words *) realloc(wordlist, sizeof(words)*(count+1));
				wordlist[count].key=c;
			}
		}
		if(*c==0x0d){*c='\0';}
		c++;
		if(count >= max_size){
			cprintf("Error  %d\n",max_size);
			break; //force STOP
		}
	}
	// protection, if next entry is empty...
	if(strlen(wordlist[count].key) !=0)
	{
		//cprintf("buf[%s]\n", wordlist[count].key);
		count++;
		// alloc last NULL entry
		wordlist=(words *) realloc(wordlist, sizeof(words)*(count+1));
	}
	wordlist[count].key=NULL; //last use NULL protect

	return wordlist;
}
#if 0 //def CONFIG_HTTPD_SUPPORT_AES256
void hexdump(char *prefix, char *buf, int sz)
{
	int i=0;
	unsigned char *c=(unsigned char *)buf;
	cprintf("%s\n",prefix);
	while(sz--)
	{
		cprintf("%02x ",*c++);
		i++;
		if(i%16 == 0) cprintf("\n");
	}
	cprintf("\n");
}
#endif //CONFIG_HTTPD_SUPPORT_AES256

/**
 * @ingroup UTilAPI
 * @brief Arcadyan security data encode function
 *
 * @param token the token key for decode
 * @param *buf  the input/output bufer.
 *
 * @retval 0  empty encode result.
 * @retval !0 the encode result size
 *
 * @note:
 *     - caller MUST allocate enought buffer size, and the size of input bufer MUST a multiple of 16.
 *     - out putformat is a binary interger array, caller must do a hex printing handle.
 *
 */
int Arc_encode(void * _r, unsigned char* buf)
{
	struct request_rec *r=(struct request_rec *) _r;
    int sz=strlen((char *)buf);
#ifdef CONFIG_HTTPD_SUPPORT_AES256
	uint8_t *abKey=NULL;
	uint8_t *abIV=NULL;


    int i;
    char tmp[1024]={0};
	if(!r->ctx.init){

		// *********************
		// (1) Token value
		// *********************
		// NOTE: 2014/5/6
		//       if no turn on CONFIG_HTTPD_TOKEN_CHECK_SUPPORT, here r->token will be zero
		//
		// encryption
		//cprintf(" encryption : token=%lu from [%s] , Referer: [%s]\n", r->token, r->url, r->referer);
		sprintf(tmp,"%lu",r->token);
//cprintf("token: [%s]\n",tmp);
	}
// *********************
// (2) plain text
// *********************
	if(!buf || buf[0]=='\0') return 0;
//cprintf("text : [%s]\n",buf);
	if(!r->ctx.init){

		// *********************
		// (3) encrypted key from token(Binary)
		// *********************
		abKey=r->ctx.enckey;
		//memset((void*)&abKey,0,AES256_KEYBYTES);
		memset((void*) abKey,0,AES256_KEYBYTES);
		for(i=0; i < strlen(tmp);i++) abKey[i]=(int)tmp[i];
//hexdump("key  :",(char *)abKey, AES256_KEYBYTES);

		// *********************
		// (4) init vector
		// *********************
		//memset((void*) &abIV ,0,AES256_BLOCKBYTES);
		abIV=r->ctx.iv;
		memset((void*) abIV ,0,AES256_BLOCKBYTES);

		// TODO: vector need enhance ,should not all zero
		// for(i=0; i < strlen(tmp);i++) abIV[i]=(int)tmp[i];
//hexdump("iv   :",(char *)abIV , AES256_BLOCKBYTES);
	}

// *********************
// (5) encode string
// *********************
	// NOTE:
	//   we no copy the value , but direct do AES encryption, the size of buf MUST a multiple of 16.
	//
	sz=strlen((char *)buf);
	sz=aes256_encrypt_cbc((aes256_context *)&(r->ctx), (uint8_t *)abKey, (uint8_t *)abIV, (uint8_t *)buf, sz);

//hexdump("encode:",(char *)buf, sz);

#endif //CONFIG_HTTPD_SUPPORT_AES256
	return sz;
}

/**
 * @ingroup UTilAPI
 * @brief Arcadyan security data decode function
 *
 * @param token the token key for decode
 * @param *buf  the input bufer want to decode and the result of output.
 *
 * @retval 0  empty encode result.
 * @retval !0 the encode result size
 *
 * @note:
 *     - caller MUST allocate enought buffer size, and the size of input bufer MUST a multiple of 16.
 *     - output format is may a binary interger array or plain text, caller must known it.
 *
 */
int Arc_decode(void *_r, unsigned char* buf)
{
	struct request_rec *r=(struct request_rec *) _r;
    int sz=0;
#ifdef CONFIG_HTTPD_SUPPORT_AES256
	uint8_t *abKey=NULL;
	uint8_t *abIV=NULL;

    int i;
    char tmp[1024]={0};


	if(!r->ctx.init){

		// *********************
		// (1) Token value
		// *********************

		// NOTE: 2014/5/6
		//       if no turn on CONFIG_HTTPD_TOKEN_CHECK_SUPPORT, here r->token will be zero
		//
		// encryption
		//cprintf(" encryption : token=%lu from [%s] , Referer: [%s]\n", r->token, r->url, r->referer);

		sprintf(tmp,"%lu",r->token);
	}
//cprintf("token: [%s]\n",tmp);

	// *********************
	// (2) encoded string (binaray format by strip out %)
	// *********************
		// Why?
		//   due to UI submit the content is <cfgid>=<Hex value>
		//   <Hex Value> will be  a escape format by %xx%yy,
		//   we need convert to integer format , then do AES encryption
		//
		if(!buf || buf[0]=='\0') return 0;

		unescape_url((char *)buf);
		sz=strlen((char *)buf);
//hexdump("encode:",(char *)buf, sz);

	if(!r->ctx.init){
			// *********************
			// (3) encrypted key from token(Binary)
			// *********************
			abKey=r->ctx.enckey;
			//memset((void*) &abKey,0,AES256_KEYBYTES);
			memset((void*) abKey,0,AES256_KEYBYTES);
			for(i=0; i < strlen(tmp);i++) abKey[i]=(int)tmp[i];
//hexdump("key  :",(char *)abKey, AES256_KEYBYTES);

			// *********************
			// (4) init vector
			// *********************

			//memset((void*) &abIV ,0,AES256_BLOCKBYTES);
			abIV=r->ctx.iv;
			memset((void*) abIV ,0,AES256_BLOCKBYTES);
			// TODO: vector need enhance ,should not all zero
			// for(i=0; i < strlen(tmp);i++) abIV[i]=(int)tmp[i];
//hexdump("iv   :",(char *)abIV , AES256_BLOCKBYTES);
	}

// *********************
// (5) decode string
// *********************
	sz=aes256_decrypt_cbc((aes256_context *)&(r->ctx), (uint8_t *)abKey, (uint8_t *)abIV, (uint8_t *)buf, sz);
//cprintf("txt  : [%s]\n",buf);

#endif //CONFIG_HTTPD_SUPPORT_AES256
	return sz;
}

#include <stdlib.h>
#include <sys/sysinfo.h>

int random_str(char*out_buf, int size)
{
	struct sysinfo info;
	int i;

	if (!out_buf)
		return 0;

	sysinfo(&info);
	srand((unsigned int)info.uptime);

	for(i=0; i<size; i++){
		out_buf[i] = '0' + rand()%10;
	}
	out_buf[i] = '\0';

	return 1;
}


//examine the range of data between two boundary string
int find_term(char **begin, char **end, char *boundary)
{
	char *ptr = NULL;

	if((ptr = memstr(*begin, *end, boundary)) == NULL)
	{
		return 0;
	}
	//\r\n--boundary\r\n
        *begin = ptr + strlen(boundary) + 2;

	if((ptr = memstr(*begin, *end, boundary)) == NULL )
	{
		return 0;
	}
	//\r\n--boundary\r\n
	*end = ptr - 5;

	return 1;
}

/*******************some common function for analyzing MIME data*********************/
char* memstr(char *begin, const char *end, const char *key)
{
	char *ptr;
	const size_t keylen = strlen(key);

	for(ptr = begin; &ptr[keylen-1] <= end; ptr++)
	{
		if (!memcmp(ptr, key, keylen))
		{
			return ptr;
		}
	}

	return NULL;
}

//Find the name and filename of iput item
int is_file_term(const char *begin, char *name, char *fname)
{
	int ret = 0;

	if (sscanf(begin, "Content-Disposition: form-data; name=\"%[^\"]\"; filename=\"%[^\"]\"", name, fname ) == 2)
	{
		ret = 1;
	}

	return ret;
}

//Find the name of input item
int is_plain_term(const char *begin, char *name)
{
	int ret = 0;

	ret = sscanf(begin, "Content-Disposition: form-data; name=\"%[^\"]\"", name);

	return ret;
}

//jump to the data after blank line
int skip_blank_line(char **begin, char **end)
{
	int ret = 1;
	const char *blank = "\r\n\r\n";
	char *ptr;

	if((ptr = memstr(*begin, *end, blank)) == NULL)
	{
		ht_dbg("Warning: no blank line found");

		ret = 0;
	}
	*begin = ptr + strlen(blank);

	return ret;
}

int get_message_from_owl(char *buf, char *recvline, int len)
{
	struct sockaddr_in RemoteAddr;
	int sock_fd, nLen, select_r, totalLen = 0;
	struct timeval t;
	fd_set readfds;
	char peek;
	
	sock_fd = socket(AF_INET, SOCK_STREAM, 0);
	memset((char *)&RemoteAddr, 0, sizeof(RemoteAddr));
	RemoteAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	RemoteAddr.sin_port = htons (OWL_LISTEN_PORT);
	RemoteAddr.sin_family = AF_INET;
	t.tv_sec = 5;
	t.tv_usec = 0;
	if(connect(sock_fd, (struct sockaddr *)&RemoteAddr, sizeof(RemoteAddr)) < 0)
	{
		cprintf("connect with owl error!\n");
		close(sock_fd);
		return -1;
	}
	write(sock_fd, buf, strlen(buf));
        FD_ZERO( &readfds );
        FD_SET( sock_fd, &readfds );
	select_r = select(sock_fd + 1, &readfds, NULL, NULL, &t);
	 if (select_r > 0 && FD_ISSET(sock_fd, &readfds))
	 {
		do
		{
			 if((nLen = recv(sock_fd, recvline+totalLen, len -totalLen -1, 0)) > 0)
			 {
				totalLen += nLen;
				usleep(100);
			 }
			else
				break;
		}while(recv(sock_fd, &peek, 1, MSG_PEEK|MSG_DONTWAIT) > 0);
	 }
	close(sock_fd);
	return totalLen;
}
/*******************some common function for analyzing MIME data*********************/ 
