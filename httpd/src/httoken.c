#include "httpd.h"
#include "cgi.h"
#include "httoken_ihashtable.h"
#include "httoken.h"
#include <pthread.h>

#define HTTOKEN_DBG    0

#define HTTOKEN_INCLUDE_PATH

#define HTTOKEN_PATH_SIZE       512
#define HTTOKEN_MAX_SIZE        512
#define HTTOKEN_GUARD_NUM	450 //clean timeout when exceeding this number

#if 1
#define HTTOKEN_LOCK()         pthread_mutex_lock(&CGI_TOKEN_RESOURCE)
#define HTTOKEN_UNLOCK()       pthread_mutex_unlock(&CGI_TOKEN_RESOURCE);

static pthread_mutex_t CGI_TOKEN_RESOURCE = PTHREAD_MUTEX_INITIALIZER;
#else
#define HTTOKEN_LOCK()
#define HTTOKEN_UNLOCK()
#endif

#define HTTOKEN_ATTR_GRMC_PAGE    0x00000001


typedef struct {
    unsigned long access_time;
    unsigned long attr;
#ifdef HTTOKEN_INCLUDE_PATH
    char          path[72];   // for debug
#endif
} httoken_item_t;

static unsigned char        g_token_inited = 0;
static httoken_ihashtable   g_token_tbl;
static unsigned long        g_timeout_sec = 600; // 10 minutes

static void httoken_item_free( httoken_item_t *buf )
{
	if( buf == NULL ) return;

	free(buf);
}

static void httoken_item_print( httoken_item_t *buf )
{
	if( buf == NULL ) return;

	ht_dbg("%lu,0x%08x", buf->access_time, (unsigned int)buf->attr);

#ifdef HTTOKEN_INCLUDE_PATH
	ht_dbg("%s", buf->path);
#endif
}

void httoken_init()
{
HTTOKEN_LOCK();
    if( g_token_inited==0 ){
    	httoken_ihashtable_create( &g_token_tbl, 17, (httoken_ihashtable_callback)httoken_item_free, (httoken_ihashtable_callback)httoken_item_print );

    } else {
    	httoken_ihashtable_clean( &g_token_tbl );
    }

    g_token_inited = 1;
HTTOKEN_UNLOCK();
}

void httoken_free()
{
HTTOKEN_LOCK();
    httoken_ihashtable_free( &g_token_tbl );
    g_token_inited = 0;
HTTOKEN_UNLOCK();
}

void httoken_entry_remove( int method )
{
//#define HTTOKEN_REMOVE_ALL				0x0001
//#define HTTOKEN_REMOVE_GRMC				0x0002	// /html/xxx
//#define HTTOKEN_REMOVE_HIDDEN			0x0004  // not /html/xxx
//#define HTTOKEN_REMOVE_HIDDEN_TIMEOUT	0x0008	// not /html/xxx, only remove leased time is timeout

    httoken_item_t *item ;
    httoken_ihashitem *hashitem, *tmp;
    unsigned char removed;
    unsigned long curr;

    if( method & HTTOKEN_REMOVE_ALL ){
        httoken_init() ;
        return;
    }

    if( g_token_inited==0 ) httoken_init();

	//ht_dbg("method=0x%08x\n", method );
    //curr = time(NULL);
    //get the system uptime instead of system time.
    curr = get_time();
    HTTOKEN_LOCK();
	hashitem = httoken_ihashtable_visit_next( &g_token_tbl, NULL, NULL);
	while( hashitem ){
		removed = 0;
		item = (httoken_item_t *)hashitem->data;
		if( item == NULL ) goto NEXT;

		//ht_dbg("attr=0x%08x\n", item->attr );
		if( (item->attr & HTTOKEN_ATTR_GRMC_PAGE) == 0 ){
			// not in /html/
			if( method & HTTOKEN_REMOVE_HIDDEN )
				removed = 1;

	       		if( method & HTTOKEN_REMOVE_HIDDEN_TIMEOUT ){ // check access time
				//ht_dbg("%lu+%lu, curr=%lu\n", item->access_time, g_timeout_sec, curr );
				if( item->access_time + g_timeout_sec < curr ){ // timeout
		            		ht_dbg("timeout, %lu+%lu<%lu\n", item->access_time, g_timeout_sec, curr );
					removed = 1;
				}
			}
   		} else {
   			// in /html/
	       		if( method & HTTOKEN_REMOVE_GRMC )
				removed = 1;
   		}

NEXT:
        	tmp = hashitem;
		hashitem = httoken_ihashtable_visit_next( &g_token_tbl, &hashitem->key1, &hashitem->key2 );

		if( removed ){
            		ht_dbg("remove (%lu, %lu)\n", tmp->key1, tmp->key2 );
            		httoken_ihashtable_delete( &g_token_tbl, tmp->key1, tmp->key2 );
		}
	}
    HTTOKEN_UNLOCK();
}

void httoken_timeout_set( unsigned long seconds )
{
    g_timeout_sec = seconds;
}

char *httoken_path_split( char *url, int url_size )
{
/*
http://speedport.ip
--> /

http://speedport.ip/?lang=en
--> /

http://speedport.ip/html/content/network/wlan_basic.html?lang=en
--> /html/content/network/wlan_basic.html

www/html/content/overview/index.html
--> /html/content/overview/

/www/api/RouterStatus.js
--> /api/RouterStatus.js
*/
    char str[1024]={0};
    char *start = (char *)str, *url_end = NULL, *pos;

    if( url == NULL ) return 0;

    strcpy_guard( str, sizeof(str), url );
    //if( start[0] == NULL ) strcpy( start, "/" );
    if( start[0] == '\0' ) strcpy( start, "/" );
    url_end = start + strlen(start);

    // remove local root patch /www
//violet_20150806 add for directly debug web ++
#if 1
   if( strncmp( start, document_root, strlen(document_root))==0 ) start += strlen(document_root);
#else
    if( strncmp( start, "/www", 4 )==0 ) start += 4;
#endif
	//violet_20150806 --
    pos = strstr( start, "//" );
    if( pos ){
        start = pos + 2;
        // remove server address
        pos = strstr( start, "/" );
        if( pos ) start = pos;
        else {
            start = "/";
        }
    }

    // remove parameter ?xxxx
    pos = strstr( start, "?" );
    if( pos ) *pos = '\0';

    // remove filename index.xxx
    pos = httoken_strrchr( start, '/');
    if( pos )
	pos++;
    else
	pos = start;
    if( strncmp( pos, "index.", 6 )==0 )
        *pos = '\0';

    strcpy_guard( url, url_size, start );

    //if( url[0] == NULL ) strcpy_guard( url, url_size, "/" );
    if( url[0] == '\0' ) strcpy_guard( url, url_size, "/" );

    return url;
}

unsigned long httoken_calc( const char *url )
{
    unsigned long ret = 0;
    unsigned char *p = (unsigned char *)&ret;
    char *s;
    int i;

    if( url == NULL ) return 0;

    for( s = (char *)url, i=0 ; *s ; s++, i++ ){
        if( i==4 ) i = 0;
        *(p+i) += (unsigned char)*s ;
        //ht_dbg("%c, ret=%lu (%08x)\n", *s, ret, ret);
        ret += (unsigned char)*s;
    }

    //ret &= 0x7fffffff;

    return ret;
}

static int httoken_lookup_ex( unsigned long token, unsigned long src_checksum )
{
// method: lookup and update access time, 1: just lookup
// return  1: found, 0 mean not found
    httoken_item_t *item ;
    httoken_ihashitem *hashitem;
    unsigned long curr;

    if( g_token_inited==0 ) httoken_init();

    ht_dbg("search token=%lu, checksum=%lu\n", token, src_checksum );

    //not clean timeout token until count is beyond guard_num
    if( g_token_tbl.item_count >= HTTOKEN_GUARD_NUM ) {
	ht_dbg("beyond guard number, remove all timeout token\n");
	httoken_entry_remove(HTTOKEN_REMOVE_HIDDEN_TIMEOUT);
    }

    HTTOKEN_LOCK();
    hashitem = httoken_ihashtable_lookup( &g_token_tbl, token, src_checksum ) ;
    if( hashitem == NULL ){
        HTTOKEN_UNLOCK();

        return 0; // not found
    }

    //curr = time(NULL);
    //get the system uptime instead of system time.
    curr = get_time();
    item = (httoken_item_t *)hashitem->data;
    if (item == NULL || (item->access_time + g_timeout_sec < curr)) // non-existent or timeout
    {
        ht_dbg("remove (%lu, %lu)\n", hashitem->key1, hashitem->key2 );
        httoken_ihashtable_delete( &g_token_tbl, hashitem->key1, hashitem->key2 );

        HTTOKEN_UNLOCK();

	return 0;
    }

    //item->access_time = time(NULL);
    //get the system uptime instead of system time.
    item->access_time = get_time();
    HTTOKEN_UNLOCK();

    return 1; // found
}

int httoken_lookup( unsigned long token, const char *url )
{
    unsigned long src_checksum;
    char path[HTTOKEN_PATH_SIZE];

    if( url == NULL ) url = "/";

    strcpy_guard( path, sizeof(path), url );
    httoken_path_split( path, sizeof(path) );
    src_checksum = httoken_calc( path );

    return httoken_lookup_ex( token, src_checksum );
}

static unsigned long httoken_checksum_lookup( unsigned long src_checksum )
{
    // return token
    httoken_item_t *item ;
    httoken_ihashitem *hashitem;
    unsigned long token = 0;

    HTTOKEN_LOCK();
	hashitem = httoken_ihashtable_visit_next( &g_token_tbl, NULL, NULL);
	while( hashitem ){
		item = (httoken_item_t *)hashitem->data;
        	if( hashitem->key2 == src_checksum ){
            		if( item ){
				//item->access_time = time(NULL);
    				//get the system uptime instead of system time.
    				item->access_time = get_time();
            		}
            		token = hashitem->key1;
            		break;
        	}
		hashitem = httoken_ihashtable_visit_next( &g_token_tbl, &hashitem->key1, &hashitem->key2 );
	}
    HTTOKEN_UNLOCK();

    return token;
}

static int httoken_oldest_entry_remove()
{
    httoken_item_t *item, *oldest_item=NULL ;
    httoken_ihashitem *hashitem, *oldest=NULL;
    unsigned char found_old;
    int ret = 0;

    HTTOKEN_LOCK();
	hashitem = httoken_ihashtable_visit_next( &g_token_tbl, NULL, NULL);
	oldest = hashitem;

	if( oldest ) oldest_item = (httoken_item_t *)oldest->data;

	while( hashitem ){
		item = (httoken_item_t *)hashitem->data;
		found_old = 0;

        	if( item && oldest_item ){
            		if( oldest_item->access_time > item->access_time )
                		found_old = 1;
        	} else
			found_old = 1;

        	if( found_old ){
            		oldest = hashitem;
            		oldest_item = (httoken_item_t *)oldest->data;
		}

		hashitem = httoken_ihashtable_visit_next( &g_token_tbl, &hashitem->key1, &hashitem->key2 );
	}

    if( oldest ){
        ht_dbg("remove %lu\n", oldest->key1 );
        httoken_ihashtable_delete( &g_token_tbl, oldest->key1, oldest->key2 );
        oldest = NULL;
        oldest_item = NULL;

        ret = 1;
    }
    HTTOKEN_UNLOCK();

    return ret;
}

static int httoken_attr_set( httoken_item_t *item, const char *url )
{
	if( item==NULL || url==NULL ) return 0;

	if( strncmp( url, "/html/", 6 ) == 0 ) // in /html/xxx
		item->attr |= HTTOKEN_ATTR_GRMC_PAGE ;

	return 0;
}

unsigned long httoken_get( const char *url )
{
    unsigned long curr, val ;
    long diff;
    unsigned char got_rand;
    int times;
    httoken_item_t *item ;
    static unsigned long s_pre_rand = 0;
    unsigned long src_checksum = 0;
    char path[HTTOKEN_PATH_SIZE];
#if defined(WE410443_SA) || defined(WE5202243_SA) || defined(WE410223_SA) || defined(WE410443_TS) || defined(WE410443_TA) || defined(WE410443_A1) || defined(WE410443_6DX) || defined(WE410443_ZZ)
    int tid;
    int len = 0;
    static int seedflag = 0;
    char sn[128] = {0};
    char nonce[128] = {0};
    char rnd_seed[16] = {0};
    unsigned char randBuf[5] = {0};
    struct timeval tv;
    struct timezone tz;
#endif

    if( g_token_inited==0 ) httoken_init();

    if( url==NULL ) url = "/";

	curr = get_time();
	
#if defined(WE410443_SA) || defined(WE5202243_SA) || defined(WE410223_SA) || defined(WE410443_TS) || defined(WE410443_TA) || defined(WE410443_A1) || defined(WE410443_6DX) || defined(WE410443_ZZ)
	if(seedflag == 0)
	{
		if((tid = get_tid()) == MID_FAIL)
			return 0;

		mapi_ccfg_get_str(tid, "ARC_SYS_NONCE", nonce, sizeof(nonce));
		len = strlen(nonce);

		if(len)
		{
			if(len > 16)
				memcpy(rnd_seed, nonce, 16);
			else
				memcpy(rnd_seed, nonce, len);
		}
		else
		{
			mapi_ccfg_get_str(tid, "ARC_SYS_SerialNum", sn, sizeof(sn));
			len = strlen(sn);
		
			gettimeofday(&tv, &tz);

			memcpy(rnd_seed, &tv.tv_sec, 4);
			memcpy(rnd_seed+4, &tv.tv_usec, 4);

			if(len >= 8)
				memcpy(rnd_seed+8, sn+len-8, 8);
			else
				memcpy(rnd_seed+8, sn, len);
		}

    //curr = time(NULL);
		RAND_seed((void*)rnd_seed, sizeof(rnd_seed));
		seedflag = 1;	
	}
#else
    diff = curr - s_pre_rand;
    if( diff < 0 || diff > 120 ){ // set seed every 2 minutes
		srand(curr);
		s_pre_rand = curr ;
	}
#endif

    strcpy_guard( path, sizeof(path), url );
    httoken_path_split( path, sizeof(path) );
    src_checksum = httoken_calc( path );

	//cprintf("src_checksum: [%s], [%lu]\n",path, src_checksum);
    times = 0;
    got_rand = 0;
    do{
        if( times < 1000 ){
#if defined(WE410443_SA) || defined(WE5202243_SA) || defined(WE410223_SA) || defined(WE410443_TS) || defined(WE410443_TA) || defined(WE410443_A1) || defined(WE410443_6DX) || defined(WE410443_ZZ)
            RAND_bytes(randBuf, 4);
            val = *((unsigned int*)(randBuf));
#else
            val = rand();
#endif			
            val <<= 16 ;
#if defined(WE410443_SA) || defined(WE5202243_SA) || defined(WE410223_SA) || defined(WE410443_TS) || defined(WE410443_TA) || defined(WE410443_A1) || defined(WE410443_6DX) || defined(WE410443_ZZ)
            memset(randBuf, 0, sizeof(randBuf));
            RAND_bytes(randBuf, 4);
            val += *((unsigned int*)(randBuf));
#else
            val += rand();
#endif	
            val &= 0x7fffffff;
        	if( val==0 ) val = curr;
        } else {
            // worst case
            val = curr + times;
        }

        // make sure token and checksum are unique in table
        HTTOKEN_LOCK();
        if( httoken_ihashtable_lookup( &g_token_tbl, val, src_checksum ) == NULL )
            got_rand = 1;
        HTTOKEN_UNLOCK();

        times++;
    } while( got_rand==0 );

    if( g_token_tbl.item_count >= HTTOKEN_MAX_SIZE ){
        ht_dbg("overflow, cnt=%d\n", g_token_tbl.item_count );
        httoken_oldest_entry_remove();
        ht_dbg("remove oldest one, cnt=%d\n", g_token_tbl.item_count );
    }

    item = (httoken_item_t *)malloc( sizeof(httoken_item_t) );
    if( item ){
    	memset( item, 0, sizeof(httoken_item_t) );
        item->access_time = curr;
		httoken_attr_set( item, path );

#ifdef HTTOKEN_INCLUDE_PATH
        strcpy_guard( item->path, sizeof(item->path), path );
#endif
    } else {
        ht_dbg("failed to alloc item\n");
        val = httoken_checksum_lookup( src_checksum );
        ht_dbg("try to get token %lu from the same checksum\n", val );
        return val;
    }

    HTTOKEN_LOCK();
    if( httoken_ihashtable_add( &g_token_tbl, val, src_checksum, item )==NULL ){
        HTTOKEN_UNLOCK();
        if( item ) free( item );
        item = NULL;
        ht_dbg("failed to add new item\n");

        val = httoken_checksum_lookup( src_checksum );
        ht_dbg("try to get token %lu from the same checksum\n", val );
    } else {
        HTTOKEN_UNLOCK();

    }

    return val;
}

char *httoken_strrchr( const char *s1, char ch )
{
    char *last = NULL, *p;

    if( ch==0 ) return 0;

    for( p = (char *)s1 ; *p ; p++ ){
        if( *p==ch ) last = p;
    }

    return last;
}

char *httoken_strrstr( const char *s1, const char *s2 )
{
    int len2;
    char *last = NULL, *tmp, *p = (char *)s1;

    if( s1==NULL || s2==NULL ) return 0;

    len2 = strlen(s2);
    while( (tmp=strstr(p, s2)) != NULL ){
        last = tmp;
        p = tmp + len2;
    }

    return last;
}

void httoken_print()
{
    ht_dbg("token table:\n");

    HTTOKEN_LOCK();
    httoken_ihashtable_print( &g_token_tbl );
    HTTOKEN_UNLOCK();

    ht_dbg("end of token table\n");
}

int httoken_dump( void *p, httoken_print_callback func )
{
	int nbytes = 0;
    httoken_item_t *item ;
    httoken_ihashitem *hashitem;
//    unsigned long curr, token = 0;
    unsigned long curr = 0;
	char buf[128]={0};
	int count = 1;

	if( func==NULL ) return 0;

	//curr = time(NULL);
    	//get the system uptime instead of system time.
    	curr = get_time();
	sprintf_guard( buf, sizeof(buf)-1, "time=%lu<br>", curr );
	nbytes += func( p, buf );
	sprintf_guard( buf, sizeof(buf)-1, "timeout_sec=%d<br>", g_timeout_sec );
	nbytes += func( p, buf );
	sprintf_guard( buf, sizeof(buf)-1, "token table(count=%d):<br>", g_token_tbl.item_count);
	nbytes += func( p, buf );

    HTTOKEN_LOCK();
	hashitem = httoken_ihashtable_visit_next( &g_token_tbl, NULL, NULL);
	while( hashitem ){
		item = (httoken_item_t *)hashitem->data;
		if( item ){
			sprintf_guard( buf, sizeof(buf), "%d,%lu,%lu,%lu%s,0x%08x,",
					count, hashitem->key1, hashitem->key2, item->access_time,
					((item->access_time+g_timeout_sec)<curr)?"(timeout)":"",
					item->attr );
#ifdef HTTOKEN_INCLUDE_PATH
			strcat_guard( buf, sizeof(buf), item->path );
#endif
			strcat_guard( buf, sizeof(buf), "<br>" );
		} else {
			sprintf_guard( buf, sizeof(buf), "%d,%lu,%lu<br>", count, hashitem->key1, hashitem->key2 );
		}
		count++;

		nbytes += func( p, buf );

		hashitem = httoken_ihashtable_visit_next( &g_token_tbl, &hashitem->key1, &hashitem->key2 );
	}
    HTTOKEN_UNLOCK();

    nbytes += func( p, "end of token table<br>");

	return nbytes;
}

int cgi_httoken_print_callback( void *p, const char *text )
{
	struct request_rec *r = p;
	int nbytes = 0;

	nbytes += so_printf(r, text );

	return nbytes;
}
#ifdef CONFIG_HTTPD_SUPPORT_TOKEN_DISABLE
extern int G_token_off_flag;
#endif //CONFIG_HTTPD_SUPPORT_TOKEN_DISABLE

//check token in url when get method
int url_token_pass(struct request_rec *r, int flag)
{
	char tn_val[16];
	char *param = NULL;
	unsigned long token=0;

	if( flag == 1 )
	{
		return 1; // ignore
	}

	//cprintf("G_token_off_flag=%d\n",G_token_off_flag);

	if ( (strcmp(r->method, "GET") && strcmp(r->method, "HEAD")) )
	{
		return 0; //failed
	}

	//anti-CSRF token
	//get token in url
	param = (get_query_value(r, "_tn", tn_val, sizeof(tn_val)) == 0)?NULL:tn_val;

	if (!param)
	{
		ht_dbg("GET, no _tn found from %s\n", r->url?r->url:"NULL");
#ifdef CONFIG_HTTPD_SUPPORT_TOKEN_DISABLE
		if(G_token_off_flag==1) //skip
		{
			param="0";
		}else
#endif //CONFIG_HTTPD_SUPPORT_TOKEN_DISABLE

		return 0;
	}

	token = atol(param);
	if( httoken_lookup( token, r->referer?r->referer:"/" ) != 1 )
	{
		ht_dbg("GET, %lu invalid from %s\n", token, r->referer?r->referer:"/");
#ifdef CONFIG_HTTPD_SUPPORT_TOKEN_DISABLE
		if(G_token_off_flag!=1) //skip
#endif //CONFIG_HTTPD_SUPPORT_TOKEN_DISABLE

		return 0;
	}
	r->token=token; //keep token for GET method

	return 1; //pass
}

//check token in cgi data when post method
int cgi_token_pass(CGI_info *p, input_t *inputs, int flag)
{
	char *param = NULL;
	unsigned long token=0;

	if( flag == 1 )
	{
		return 1; // ignore
	}
	//cprintf("G_token_off_flag=%d\n",G_token_off_flag);

	if( !p || strcmp(p->r->method,"POST") )
	{
		return 0; //failed
	}

	//anti-CSRF token
	if( inputs==NULL )
	{
		param = NULL;
	}
	else
	{
		param = __get_cgi(inputs, "httoken");
	}

	if( param==NULL )
	{
		ht_dbg("POST, no httoken found from %s\n", p->r->referer?p->r->referer:"/");
#ifdef CONFIG_HTTPD_SUPPORT_TOKEN_DISABLE
		if(G_token_off_flag==1) //skip
		{
			param="0";
		}else
#endif //CONFIG_HTTPD_SUPPORT_TOKEN_DISABLE

		return 0; // failed
	}

		token = atol(param);
		if( httoken_lookup( token, p->r->referer?p->r->referer:"/" )==1 )
		{
			ht_dbg("POST, valid %lu from %s\n", token, p->r->referer?p->r->referer:"/");
		}
		else
		{
			ht_dbg("POST, invalid %lu from %s\n", token, p->r->referer?p->r->referer:"/");
#ifdef CONFIG_HTTPD_SUPPORT_TOKEN_DISABLE
			if(G_token_off_flag!=1) //skip
#endif //CONFIG_HTTPD_SUPPORT_TOKEN_DISABLE

			return 0; // failed
		}
	p->r->token=token; //keep token for POST method
	return 1; // pass
}

