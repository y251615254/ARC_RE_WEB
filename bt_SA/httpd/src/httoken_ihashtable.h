#ifndef __HTTOKEN_IHASHTABLE_
#define __HTTOKEN_IHASHTABLE_


typedef struct httoken_ihashitem_s {
	unsigned long key1 ;
	unsigned long key2 ;
	void *data ;
    struct httoken_ihashitem_s *next ;
} httoken_ihashitem ;

typedef void (*httoken_ihashtable_callback)( void *data ) ;

typedef struct httoken_ihashtable_s {
	httoken_ihashitem 	**items ;
	int size ;
    int item_count;

    httoken_ihashtable_callback free_func ;
    httoken_ihashtable_callback print_func ;
} httoken_ihashtable ;



#if __cplusplus
extern "C" {
#endif

void httoken_ihashtable_create( httoken_ihashtable *table, int size,
			void (*free_func)( void *data ), void (*print_func)( void *data ) ) ;

void httoken_ihashtable_clean( httoken_ihashtable *table ) ;

void httoken_ihashtable_free( httoken_ihashtable *table );

httoken_ihashitem *httoken_ihashtable_add( httoken_ihashtable *table,
            unsigned long key1, unsigned long key2, void *data ) ;

void *httoken_ihashtable_remove( httoken_ihashtable *table,
                unsigned long key1, unsigned long key2 ) ;

void httoken_ihashtable_delete( httoken_ihashtable *table,
            unsigned long key1, unsigned long key2 ) ;

httoken_ihashitem *httoken_ihashtable_lookup( httoken_ihashtable *table, unsigned long key1, unsigned long key2 ) ;

// for visit table
httoken_ihashitem *httoken_ihashtable_visit_next( httoken_ihashtable *table,
            const unsigned long *key1, const unsigned long *key2 ) ;

void httoken_ihashtable_print( httoken_ihashtable *table ) ;

#if __cplusplus
}
#endif

#endif // _H_HTTOKEN_IHASHTABLE_
