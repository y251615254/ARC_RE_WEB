#ifndef __HTTOKEN_H
#define __HTTOKEN_H

#include "httpd.h"
#include "cgi.h"

#define HTTOKEN_REMOVE_ALL				0x0001
#define HTTOKEN_REMOVE_GRMC				0x0002	// /html/xxx
#define HTTOKEN_REMOVE_HIDDEN			0x0004  // not /html/xxx
#define HTTOKEN_REMOVE_HIDDEN_TIMEOUT	0x0008	// not /html/xxx, only remove age-out item

#define HTTOKEN_GRMC_LOGOUT		(HTTOKEN_REMOVE_GRMC|HTTOKEN_REMOVE_HIDDEN_TIMEOUT)
//#define HTTOKEN_HIDDEN_LOGOUT	(HTTOKEN_REMOVE_HIDDEN)
#define HTTOKEN_HIDDEN_LOGOUT	(HTTOKEN_REMOVE_HIDDEN_TIMEOUT)

typedef int (*httoken_print_callback)( void *r1, const char *text );

#if __cplusplus
extern "C" {
#endif

void httoken_init();
void httoken_free();

void httoken_entry_remove( int method );

unsigned long httoken_get( const char *url );
int httoken_lookup( unsigned long token, const char *url );

void httoken_timeout_set( unsigned long seconds );

void httoken_print();
int httoken_dump( void *p, httoken_print_callback func );



char *httoken_strrstr( const char *s1, const char *s2 );
char *httoken_strrchr( const char *s1, char ch );
unsigned long httoken_calc( const char *url );
char *httoken_path_split( char *url, int url_size );
int cgi_httoken_print_callback( void *p, const char *text );
int url_token_pass(struct request_rec *r, int flag);
int cgi_token_pass(CGI_info *p, input_t *inputs, int flag);


#if __cplusplus
}
#endif


#endif
