/*
Copyright (c) 2022 BostonBSD. All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:

    (1) Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.

    (2) Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in
    the documentation and/or other materials provided with the
    distribution.

    (3)The name of the author may not be used to
    endorse or promote products derived from this software without
    specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT,
INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "../include/gui_types.h"
#include "../include/multicurl.h"
#include "../include/class_types.h"
#include "../include/macros.h"

static int sym_search_func (const void *a, const void *b){
    /* Cast the void pointer to a char pointer. */
   char* aa = (char *)a;
   /* Cast the void pointer to a double struct pointer. */
   symbol_to_security_name_container** bb = (symbol_to_security_name_container **)b;

   return strcasecmp( aa, bb[0]->symbol ); 
}

char *GetSecurityName (const char *s, symbol_name_map *sn_map)
/* Look for a Security Name using the Symbol as a key value.
   Must free return value. */
{
    symbol_to_security_name_container **item = NULL;
    if ( sn_map == NULL ) return NULL;


    /* The second parameter is the same type as the return value. A double pointer to a struct. */
    /* It's basically searching through an array of pointer addresses, the compare function dereferences
       the pointer address to get the string we are comparing against. */
    /* The array must already be sorted for bsearch to work. */
    item = (symbol_to_security_name_container**) bsearch (s, &sn_map->sn_container_arr[0], sn_map->size, sizeof (symbol_to_security_name_container*), sym_search_func);

    if ( item ){
        /* The item pointer is not freed. It points to an item in the 
           sn_container_arr array and not a duplicate. */
        return strdup( item[ 0 ]->security_name );
    } else {
        return NULL;
    }
}

static int alpha_asc_sec_name (const void *a, const void *b)
/* This is a callback function for stdlib sorting functions. 
   It compares symbol_to_security_name_container in alphabetically 
   ascending order, by the stock symbol data member.  Swapping only 
   the pointer.
*/ 
{
    /* Cast the void pointer as a symbol_to_security_name_container double pointer. */
    symbol_to_security_name_container **aa = (symbol_to_security_name_container **)a;
    symbol_to_security_name_container **bb = (symbol_to_security_name_container **)b;
    
    /* Compare the symbol data members ignoring for case. */
    return strcasecmp( aa[0]->symbol, bb[0]->symbol );
}

static bool check_symbol ( const char *s )
/* Depository share symbols include dollars signs, which we don't want. 
   The first line of the file includes the "Symbol" string, which we don't want.
   The last line of the file includes the "File Creation Time" string, which we don't want.
*/
{ 
	if ( strpbrk( s, "$" ) ) return false;
    if ( strstr( s, "Symbol" ) ) return false;
    if ( strstr( s, "File Creation Time" ) ) return false;

    return true;
}

symbol_name_map *SymNameFetch (portfolio_packet *pkg)
/* This function is only meant to be run once, at application startup. */
{
    meta *Met = pkg->portfolio_meta_info;

    char *line = malloc ( 1024 ), *line_start;
	char *token;
	line_start = line;

    symbol_name_map *sn_map = (symbol_name_map*) malloc ( sizeof(*sn_map) );
    sn_map->sn_container_arr = malloc ( 1 );
    sn_map->size = 0;
    symbol_to_security_name_container **sec_sym_tmp = NULL;

    char* Nasdaq_Url = NASDAQ_SYMBOL_URL; 
    char* NYSE_Url = NYSE_SYMBOL_URL; 
   	MemType Nasdaq_Struct, NYSE_Struct;
    Nasdaq_Struct.memory = NULL;
    NYSE_Struct.memory = NULL;
    
    SetUpCurlHandle( Met->NASDAQ_completion_hnd, Met->multicurl_cmpltn_hnd, Nasdaq_Url, &Nasdaq_Struct );
    SetUpCurlHandle( Met->NYSE_completion_hnd, Met->multicurl_cmpltn_hnd, NYSE_Url, &NYSE_Struct );
    if ( PerformMultiCurl_no_prog( Met->multicurl_cmpltn_hnd ) != 0 || pkg->IsCurlCanceled () ) { 
        if ( Nasdaq_Struct.memory ) free( Nasdaq_Struct.memory );
        if ( NYSE_Struct.memory ) free( NYSE_Struct.memory ); 
        if ( sn_map->sn_container_arr ) free( sn_map->sn_container_arr );
        if ( sn_map ) free( sn_map );      
        if ( line ) free( line );
        return NULL;
    }

    /* Convert a String to a File Pointer Stream for Reading */
    FILE* fp[2];
    fp[0] = fmemopen( (void*)Nasdaq_Struct.memory, strlen( Nasdaq_Struct.memory ) + 1, "r" );
    fp[1] = fmemopen( (void*)NYSE_Struct.memory, strlen( NYSE_Struct.memory ) + 1, "r" );
    
    short k = 0;
	while( k < 2 ){
        /* Read the file stream one line at a time */
        /* Populate the Symbol-Name Array. The second list is concatenated after the first list. */
        while ( fgets( line, 1024, fp[k] ) != NULL ) {
            
            /* Extract the symbol from the line. */
            token = strsep( &line, "|" );
            if ( check_symbol( token ) == false ){ line = line_start; continue; }

            /* Increase the array to hold another pointer address */
            sec_sym_tmp = realloc( sn_map->sn_container_arr, sizeof( symbol_to_security_name_container* ) * ( sn_map->size + 1 ) );
            sn_map->sn_container_arr = sec_sym_tmp;
            /* Allocate memory for a pointer address and add it to the array */
            sn_map->sn_container_arr[ sn_map->size ] = malloc( sizeof( symbol_to_security_name_container ) );
            /* Populate a data member with the symbol string */
            sn_map->sn_container_arr[ sn_map->size ]->symbol = strdup( token ? token : "EOF Incomplete Symbol List" );

            /* Extract the security name from the line. */
            token = strsep( &line, "|" );

            /* Populate a data member with the security_name string and increment the size */
            sn_map->sn_container_arr[ sn_map->size++ ]->security_name = strdup( token ? token : "EOF Incomplete Symbol List" );

            /* strsep moves the line pointer, we need to reset it so the pointer memory can be reused */
            line = line_start;

            /* If we are exiting the application, return immediately. */
            if ( pkg->IsCurlCanceled () ) {
                if ( fp[0] ) fclose( fp[0] ); 
                if ( fp[1] ) fclose( fp[1] );  
                if ( Nasdaq_Struct.memory ) free( Nasdaq_Struct.memory );
                if ( NYSE_Struct.memory ) free( NYSE_Struct.memory );
                if ( line ) free( line );
                return sn_map;
            }
        }
        k++;

    } /* end while loop */
    free( line );

    /* Sort the security symbol array, this merges both lists into one sorted list. */
    qsort( &sn_map->sn_container_arr[ 0 ], sn_map->size, sizeof(symbol_to_security_name_container*), alpha_asc_sec_name );    

    fclose( fp[0] ); 
    fclose( fp[1] );
    free( Nasdaq_Struct.memory );
    free( NYSE_Struct.memory );
    return sn_map;
}

void SNMapDestruct ( symbol_name_map *sn_map ) {
    if ( sn_map == NULL ) return;
    
    if ( sn_map->sn_container_arr ){
        for(int i=0; i<sn_map->size; i++ ){
            /* Free the string members */
            free( sn_map->sn_container_arr[ i ]->symbol );
            free( sn_map->sn_container_arr[ i ]->security_name );
            /* Free the memory that the address is pointing to */
            free( sn_map->sn_container_arr[ i ] );
        }
        /* Free the array of pointer addresses */
        free( sn_map->sn_container_arr );
        sn_map->sn_container_arr = NULL;
        sn_map->size = 0;
    }
}