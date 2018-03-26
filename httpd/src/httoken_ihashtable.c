#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "httpd.h"
#include "httoken_ihashtable.h"

#define HTTOKEN_IHASHTABLE_HASH_FUNC(key1, key2, size)	(((key1)+(key2)) % size)
#define BLOCK_PREEMPTION
#define UNBLOCK_PREEMPTION

//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------

void httoken_ihashtable_create( httoken_ihashtable *table, int size,
				void (*free_func)( void *data ), void (*print_func)( void *data ) )
{
	int i ;

	memset( table, 0, sizeof(httoken_ihashtable) );
    if( size==0 )  return ;

	table->items = (httoken_ihashitem **) malloc( sizeof(httoken_ihashitem *) * size ) ;
	for( i=0 ; i < size ; i++ )
		table->items[i] = NULL ;

	table->size = size ;
    	table->item_count = 0;
	table->free_func = free_func ;
	table->print_func = print_func ;

}

void httoken_ihashtable_clean( httoken_ihashtable *table )
{
	int i ;
	httoken_ihashitem *item, *tmp ;

	ht_dbg("all tokens were cleaned\n");

	for( i=0 ; i < table->size ; i++ ){
		item = table->items[i] ;
		while( item ){
			tmp = item->next ;
			if( table->free_func )
				table->free_func( item->data ) ;
			free( item ) ;
				table->item_count--;

			if( tmp==NULL ) break ;
			item = tmp ;
		}
		table->items[i] = NULL ;
	}
}

void httoken_ihashtable_free( httoken_ihashtable *table )
{
	httoken_ihashtable_clean( table ) ;
	free( table->items );

	memset( table, 0, sizeof(httoken_ihashtable) );
}

httoken_ihashitem *httoken_ihashtable_add( httoken_ihashtable *table,
                unsigned long key1, unsigned long key2, void *data )
{
	int index ;
	httoken_ihashitem *item, *tmp;
	//const char *fn = __func__ ; // no used

    if( table->size<=0||table->items==NULL ){
		ht_dbg("size=%d, items=%08x\n", table->size, (unsigned int)table->items );

    		return 0;
	}
	index = HTTOKEN_IHASHTABLE_HASH_FUNC( key1, key2, table->size );

	item = (httoken_ihashitem *)malloc( sizeof(httoken_ihashitem) ) ;
	if( item==NULL ){
		ht_dbg("allocate memory fail, key1=%08x, key2=%08x\n", (unsigned int)key1, (unsigned int)key2 );

		return 0; // fail
	}
	item->key1 = key1 ;
	item->key2 = key2 ;
    item->data = data ;
	item->next = NULL ;

BLOCK_PREEMPTION;
	if(table->items[index]==NULL){
        table->items[index] = item ;
	}else{
		tmp = table->items[index] ;
		table->items[index] = item ;
		item->next = tmp ;
	}
    table->item_count++ ;
UNBLOCK_PREEMPTION;

	return item ;
}

void *httoken_ihashtable_remove( httoken_ihashtable *table,
                unsigned long key1, unsigned long key2 )
{   // return the data object
    // just remove it from hash table, and don't free it.
	int index ;
	httoken_ihashitem *tmp, *prior ;
    void *data = NULL;

    if( table->size<=0 || table->items==NULL )  return 0;
	index = HTTOKEN_IHASHTABLE_HASH_FUNC( key1, key2, table->size );

	prior = NULL ;
BLOCK_PREEMPTION;
	tmp = table->items[index] ;
UNBLOCK_PREEMPTION;
	while( tmp ){
		if( tmp->key1 == key1 && tmp->key2 == key2 ){
BLOCK_PREEMPTION;
			if( prior ){
				prior->next = tmp->next ;
			}else{ // first node
				table->items[index] = tmp->next ;
			}
			data = tmp->data ; // return its data, don't free it
			tmp->key1 = 0;
			tmp->key2 = 0;
			tmp->data = NULL;
			tmp->next = NULL;
			table->item_count-- ;
UNBLOCK_PREEMPTION;
			free( tmp ) ;  // delete this item
			break ;
		}
		prior = tmp ;
		tmp = tmp->next ;
	}

    return data ;
}

void httoken_ihashtable_delete( httoken_ihashtable *table,
                unsigned long key1, unsigned long key2 )
{
    void *data ;

    data = httoken_ihashtable_remove( table, key1, key2 ) ;
	//if( data==0 )
	//	printf("%s> not found key1=%d, key2=%d\n", __func__, key1, key2 );

	if( data && table->free_func )
		table->free_func( data ) ;
}

httoken_ihashitem *httoken_ihashtable_lookup( httoken_ihashtable *table,
                unsigned long key1, unsigned long key2 )
{
	int index ;
	httoken_ihashitem *tmp ;

    if( table->size<=0||table->items==NULL )  return 0;
	index = HTTOKEN_IHASHTABLE_HASH_FUNC( key1, key2, table->size );

	tmp = table->items[index] ;
	while( tmp ){
		if( tmp->key1 == key1 && tmp->key2 == key2 )
		    return tmp ;

		tmp = tmp->next ;
	}

	return 0 ;
}

httoken_ihashitem *httoken_ihashtable_visit_next( httoken_ihashtable *table,
                    const unsigned long *key1, const unsigned long *key2 )
{
// for visit table
// if item is NULL, return the first item
    httoken_ihashitem *tmp ;
    int index ;

    if( table->size<=0||table->items==NULL )  return 0;
    if( key1 && key2 ) index = HTTOKEN_IHASHTABLE_HASH_FUNC( *key1, *key2, table->size );
    else      index = 0;

    while( index < table->size ){
        tmp = table->items[index] ;
        if( !(key1 && key2) && tmp )  return tmp ;

        while( tmp ){
            if( tmp->key1 == *key1 && tmp->key2 == *key2 ){
                if( tmp->next )
                    return tmp->next ;
                else{
                    key1 = 0 ;
                    key2 = 0 ;
                    break ;
                }
            }
            tmp = tmp->next ;
        }
        index++ ;
    }

    return 0; // there is no next item
}

void httoken_ihashtable_print( httoken_ihashtable *table )
{
	int i ;
	httoken_ihashitem *item;

	for( i=0 ; i < table->size ; i++ ){
		item = table->items[i] ;

		if( item==NULL ) continue;

        	ht_dbg("entry %d: ", i );

		while( item ){
            		ht_dbg("%lu+%lu,", item->key1, item->key2 );

            		if( table->print_func )
                		table->print_func( item->data ) ;

            		ht_dbg("->");

			item = item->next ;
		}

		ht_dbg( "nil\n" );
	}
}

