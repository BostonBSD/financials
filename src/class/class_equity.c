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

#include <stdlib.h>
#include <string.h>

#include <monetary.h>
#include <locale.h>

#include "../include/class.h" /* The class init and destruct funcs are required 
                                 in the class methods, includes portfolio_packet 
                                 metal, meta, and equity_folder class types */

#include "../include/multicurl.h"
#include "../include/workfuncs.h"
#include "../include/json.h"
#include "../include/macros.h"
#include "../include/mutex.h"

/* The static local-global variable 'Folder' is always accessed via these functions. */
/* This is an ad-hoc way of self referencing a class. 
   It prevents multiple instances of the equity_folder class. */
   
static equity_folder *Folder;  /* A class handle to an array of stock class objects, can change dynamically. */    

/* Class Method (also called Function) Definitions */
static double Stake (const unsigned int *shares, const double *price) {
    return ( (double)(*shares) * (*price) );
}

static char *DoubToStr (const double *num) 
/* Take in a double pointer [the datatype is double] and convert to a monetary format string. */
{    
    size_t len = strlen("###,###,###,###,###,###.###") + 1;
    char* str = (char*) malloc( len ); 

    /* The C.UTF-8 locale does not have a monetary 
       format and is the default in C. 
    */
    setlocale(LC_ALL, LOCALE);  
    strfmon(str, len, "%(.3n", *num);

    /* Trying not to waste memory. */
    char* tmp = realloc( str, strlen ( str ) + 1 );
    str = tmp;

    return str;
}

static double StrToDoub (const char *str) {
    char *newstr = (char*) malloc( strlen( str )+1 );
    strcpy( newstr, str );

    FormatStr( newstr );
    double num = strtod( newstr, NULL );

    free( newstr );

    return num;
}

static void convert_equity_to_strings (stock *S){
    /* Convert the double values into string values. */
    free( S->current_price_stock_ch );
    S->current_price_stock_ch = S->DoubToStr( S->current_price_stock_f );
    
    free( S->high_stock_ch );
    S->high_stock_ch = S->DoubToStr( S->high_stock_f ); 
    
    free( S->low_stock_ch );
    S->low_stock_ch = S->DoubToStr( S->low_stock_f ); 
    
    free( S->opening_stock_ch );
    S->opening_stock_ch = S->DoubToStr( S->opening_stock_f ); 
    
    free( S->prev_closing_stock_ch );
    S->prev_closing_stock_ch = S->DoubToStr( S->prev_closing_stock_f ); 
    
    free( S->change_share_ch );
    S->change_share_ch = S->DoubToStr( S->change_share_f );

    free( S->change_value_ch );
    S->change_value_ch = S->DoubToStr( S->change_value_f );
    
    free( S->change_percent_ch );
    size_t len = strlen("###.###%%") + 1;
    S->change_percent_ch = (char*) malloc ( len );
    snprintf( S->change_percent_ch, len, "%.3lf%%", *S->change_percent_f );

    /* The total current investment in this equity. */
    free( S->current_investment_stock_ch );
    S->current_investment_stock_ch = S->DoubToStr( S->current_investment_stock_f );
}

static void ToStrings (){
    equity_folder *F = Folder;

    for(unsigned short g = 0; g < F->size; g++){
        convert_equity_to_strings ( F->Equity[ g ] );
    }

    /* The total equity portfolio value. */
    free( F->stock_port_value_ch );
    F->stock_port_value_ch = F->DoubToStr( F->stock_port_value_f );

    /* The equity portfolio's change in value. */
    free( F->stock_port_value_chg_ch );
    F->stock_port_value_chg_ch = F->DoubToStr( F->stock_port_value_chg_f );

    /* The change in total investment in equity as a percentage. */
    free( F->stock_port_value_p_chg_ch );
    size_t len = strlen("###.###%%") + 1;
    F->stock_port_value_p_chg_ch = (char*) malloc ( len );
    snprintf( F->stock_port_value_p_chg_ch, len, "%.3lf%%", *F->stock_port_value_p_chg_f );
}

static void equity_calculations (stock *S){
    /* Calculate the stock holdings change. */ 
    if( *S->num_shares_stock_int > 0 ){
        *S->change_value_f = *S->change_share_f * (double)*S->num_shares_stock_int;
    } else {
        *S->change_value_f = 0.0f;
    }

    /* The total current investment in this equity. */
    *S->current_investment_stock_f = S->Stake( S->num_shares_stock_int, S->current_price_stock_f );
}

static void Calculate (){
    equity_folder* F = Folder;

    /* Equity Calculations. */
    *F->stock_port_value_f = 0.0f;
    *F->stock_port_value_chg_f = 0.0f;

    for (unsigned short g = 0; g < F->size; g++){
        equity_calculations ( F->Equity[ g ] );

        /* Add the equity investment to the total equity value. */
        *F->stock_port_value_f += *F->Equity[ g ]->current_investment_stock_f;

        /* Add the equity's change in value to the equity portfolio's change in value. */
        *F->stock_port_value_chg_f += *F->Equity[ g ]->change_value_f;
    }

    /* The change in total investment in equity as a percentage. */
    double prev_total = *F->stock_port_value_f - *F->stock_port_value_chg_f;
    *F->stock_port_value_p_chg_f = CalcGain ( *F->stock_port_value_f, prev_total );
}

static void GenerateURL ( void *data ){
    pthread_mutex_lock( &mutex_working [ CLASS_MEMBER_MUTEX ] );

    portfolio_packet *pkg = (portfolio_packet*)data;
    equity_folder *F = Folder;
    meta *Met = pkg->GetMetaClass ();

    size_t len;
 
    /* Cycle through the list of equities. */
    for( unsigned short c = 0; c < F->size; c++ ) { 
        /* Generate the request URL for this equity. */
        free( F->Equity[ c ]->curl_url_stock_ch );

        len = strlen(Met->stock_url_ch) + strlen(F->Equity[ c ]->symbol_stock_ch) + strlen(Met->curl_key_ch)+1;
        F->Equity[ c ]->curl_url_stock_ch = (char*) malloc( len );
        snprintf( F->Equity[ c ]->curl_url_stock_ch, len, "%s%s%s", Met->stock_url_ch, F->Equity[ c ]->symbol_stock_ch, Met->curl_key_ch );
    }

    pthread_mutex_unlock( &mutex_working [ CLASS_MEMBER_MUTEX ] );
}

static int SetUpCurl ( void *data ){
    portfolio_packet *pkg = (portfolio_packet*)data;
    equity_folder *F = Folder;

    /* Cycle through the list of equities. */
    for( unsigned short c = 0; c < F->size; c++ ) {       
        /* Add a cURL easy handle to the multi-cURL handle 
        (passing JSON output struct by reference) */
        SetUpCurlHandle( F->Equity[ c ]->easy_hnd, pkg->multicurl_main_hnd, F->Equity[ c ]->curl_url_stock_ch, &F->Equity[ c ]->JSON );
    }
    
    return 0;
}

static void Reset () {
    pthread_mutex_lock( &mutex_working [ CLASS_MEMBER_MUTEX ] );

    equity_folder *F = Folder;

    if (F->size == 0){
        /*
        We do not know if F->size = 0 means that the stock** array already 
        exists or not.

        This is the only place where the stock** array is created.

        There are multiple points in the code where a zero size array
        could be reset, some where the array exists, some where it does not 
        exist, so we need to keep freeing this to prevent a memory leak.

        */
        if ( F->Equity ) free( F->Equity ); 
        stock** temp = (stock**) malloc( sizeof(stock*) );
                
        if (temp == NULL){
            printf("Not Enough Memory, malloc returned NULL.\n");
            exit(EXIT_FAILURE);
        }
                
        F->Equity = temp;
    } else {
        for(short c = (F->size - 1); c >= 0; c--){
            class_destruct_equity( F->Equity[ c ] );
        }

        free( F->Equity ); 
        stock** temp = (stock**) malloc( sizeof(stock*) );
                
        if (temp == NULL){
            printf("Not Enough Memory, malloc returned NULL.\n");
            exit(EXIT_FAILURE);
        }
                
        F->Equity = temp;
        F->size = 0;
    }

    pthread_mutex_unlock( &mutex_working [ CLASS_MEMBER_MUTEX ] );
}

static void AddStock ( char *symbol, char *shares )
/* Adds a new stock object to our folder, 
   increments size. */
{
    pthread_mutex_lock( &mutex_working [ CLASS_MEMBER_MUTEX ] );

    equity_folder *F = Folder;

    /* realloc will free memory if assigned a smaller new value. */
    stock** tmp = (stock**) realloc( F->Equity, (F->size + 1) * sizeof(stock*) );              

    if (tmp == NULL){
        printf("Not Enough Memory, realloc returned NULL.\n");
        exit(EXIT_FAILURE);
    }
                
    F->Equity = tmp;

    /* class_init_equity returns an object pointer. */
    F->Equity[ F->size ] = class_init_equity ();

    /* Add The Stock Symbol To the Folder */
    free( F->Equity[ F->size ]->symbol_stock_ch ); 
    F->Equity[ F->size ]->symbol_stock_ch = strdup ( symbol ? symbol : "NULL" );

    /* Add The Shares To the Folder */
    *F->Equity[ F->size ]->num_shares_stock_int = (unsigned int)strtol( shares ? shares : "0", NULL, 10 );
    F->size++;

    pthread_mutex_unlock( &mutex_working [ CLASS_MEMBER_MUTEX ] );
}

static void RemoveStock ( void *data ) 
/* Removes a stock object from our folder, 
   decrements size. If the stock isn't found 
   the size doesn't change. */
{
    pthread_mutex_lock( &mutex_working [ CLASS_MEMBER_MUTEX ] );

    char* s = (char*)data;
    equity_folder *F = Folder;
    stock** tmp;

    unsigned short j, i = 0;
    while ( i < F->size ){
        if( strcasecmp ( s, F->Equity[i]->symbol_stock_ch ) == 0 ){
            class_destruct_equity( F->Equity[i] );
            j = i;
            while ( j < F->size - 1 ) {
                F->Equity[j] = F->Equity[j + 1];
                j++;
            }
            tmp = realloc( F->Equity, (F->size - 1) * sizeof(stock*) );
            if( tmp ) F->Equity = tmp;
            F->size--;
            break; /* Each symbol has only one unique stock object */
        }
        i++;
    }

    pthread_mutex_unlock( &mutex_working [ CLASS_MEMBER_MUTEX ] );
}

static void ExtractData () {
    equity_folder *F = Folder;

    for( unsigned short c = 0; c < F->size; c++ ) 
    /* Extract current price from JSON data for each Symbol. */
    { 
        if ( F->Equity[ c ]->JSON.memory ){
            /* Extract double values from JSON data using JSON-glib */
            JsonExtractEquity( F->Equity[ c ]->JSON.memory, F->Equity[ c ]->current_price_stock_f, F->Equity[ c ]->high_stock_f, F->Equity[ c ]->low_stock_f, F->Equity[ c ]->opening_stock_f, F->Equity[ c ]->prev_closing_stock_f, F->Equity[ c ]->change_share_f, F->Equity[ c ]->change_percent_f);

            /* Free memory. */
            free( F->Equity[ c ]->JSON.memory );
            F->Equity[ c ]->JSON.memory = NULL;  
        }else{
            *F->Equity[ c ]->current_price_stock_f = 0.0;
            *F->Equity[ c ]->high_stock_f = 0.0;
            *F->Equity[ c ]->low_stock_f = 0.0;
            *F->Equity[ c ]->opening_stock_f = 0.0;
            *F->Equity[ c ]->prev_closing_stock_f = 0.0;
            *F->Equity[ c ]->change_share_f = 0.0;
            *F->Equity[ c ]->change_percent_f = 0.0;
        }
    }
}

static int alpha_asc (const void *a, const void *b)
/* This is a callback function for stdlib sorting functions. 
   It compares stock-structs in alphabetically ascending order,
   by the stock symbol data member.  Swapping only the pointer.
*/ 
{
    /* Cast the void pointer as a stock double pointer. */
    stock **aa = (stock **)a;
    stock **bb = (stock **)b;
    
    /* Compare the stock symbols ignoring for case. */
    return strcasecmp( aa[0]->symbol_stock_ch, bb[0]->symbol_stock_ch );
}

static void Sort () {
    pthread_mutex_lock( &mutex_working [ CLASS_MEMBER_MUTEX ] );

    /* Sort the equity folder in alphabetically ascending order. */
    equity_folder *F = Folder;
    qsort(&F->Equity[0], (size_t)F->size, sizeof(stock*), alpha_asc);

    pthread_mutex_unlock( &mutex_working [ CLASS_MEMBER_MUTEX ] );
}

/* Class Init Functions */
stock *class_init_equity ()
{ 
    /* Allocate Memory For A New Class Object */
    stock* new_class = (stock*) malloc( sizeof(*new_class) );
	
    /* Allocate Memory For Variables */
    new_class->num_shares_stock_int = (unsigned int*) malloc( sizeof(unsigned int) );

    new_class->current_price_stock_f = (double*) malloc( sizeof(double) );
    new_class->high_stock_f = (double*) malloc( sizeof(double) );
    new_class->low_stock_f = (double*) malloc( sizeof(double) );
    new_class->opening_stock_f = (double*) malloc( sizeof(double) );
    new_class->prev_closing_stock_f = (double*) malloc( sizeof(double) );
    new_class->change_share_f = (double*) malloc( sizeof(double) );
    new_class->change_value_f = (double*) malloc( sizeof(double) );
    new_class->change_percent_f = (double*) malloc( sizeof(double) );

	new_class->current_investment_stock_f = (double*) malloc( sizeof(double) );

    /* Initialize Variables */
    *new_class->num_shares_stock_int = 0;
    
    *new_class->current_price_stock_f = 0.0f;
    *new_class->high_stock_f = 0.0f;
    *new_class->low_stock_f = 0.0f;
    *new_class->opening_stock_f = 0.0f;
    *new_class->prev_closing_stock_f = 0.0f;
    *new_class->change_share_f = 0.0f;
    *new_class->change_value_f = 0.0f;
    *new_class->change_percent_f = 0.0f;

	*new_class->current_investment_stock_f = 0.0f;  
    
    new_class->current_price_stock_ch = strdup( "$0.00" );
    new_class->high_stock_ch = strdup( "$0.00" );
    new_class->low_stock_ch = strdup( "$0.00" );
    new_class->opening_stock_ch = strdup( "$0.00" );
    new_class->prev_closing_stock_ch = strdup( "$0.00" );
    new_class->change_share_ch = strdup( "$0.00" );
    new_class->change_value_ch = strdup( "$0.00" );
    new_class->change_percent_ch = strdup( "000.000%" );

	new_class->current_investment_stock_ch = strdup( "$0.00" ); 
	
	new_class->symbol_stock_ch = strdup( "NO_SYMBOL" );         
	new_class->curl_url_stock_ch = strdup( "http://" ); 

    new_class->easy_hnd = curl_easy_init ();
    new_class->JSON.memory = NULL;

    /* Connect Function Pointers To Function Definitions */
    new_class->Stake = Stake;
    new_class->DoubToStr = DoubToStr;
    new_class->StrToDoub = StrToDoub;

    /* Return Our Initialized Class */
    return new_class; 
}

equity_folder *class_init_equity_folder ()
{
    /* Allocate Memory For A New Class */
    equity_folder* new_class = (equity_folder*) malloc( sizeof(*new_class) );

    /* A placeholder for our nested stock class array */
    new_class->Equity = NULL;
    new_class->size = 0;

    /* Allocate Memory For Variables */
    new_class->stock_port_value_f = (double*) malloc( sizeof(double) );
    new_class->stock_port_value_chg_f = (double*) malloc( sizeof(double) );
    new_class->stock_port_value_p_chg_f = (double*) malloc( sizeof(double) );

    /* Initialize Variables */
    new_class->stock_port_value_ch = strdup( "$0.00" );   
    new_class->stock_port_value_chg_ch = strdup( "$0.00" ); 
    new_class->stock_port_value_p_chg_ch = strdup( "000.000%" );

    *new_class->stock_port_value_f = 0.0f;
    *new_class->stock_port_value_chg_f = 0.0f;
    *new_class->stock_port_value_p_chg_f = 0.0f;

    /* Connect Function Pointers To Function Definitions */
    /* The functions do not need to have the same name as the pointer,
       but it is easier to follow this way. */
    new_class->DoubToStr = DoubToStr;
    new_class->ToStrings = ToStrings;
    new_class->Calculate = Calculate;
    new_class->GenerateURL = GenerateURL;
    new_class->SetUpCurl = SetUpCurl;
    new_class->ExtractData = ExtractData;
    new_class->AddStock = AddStock;
    new_class->Reset = Reset;
    new_class->Sort = Sort;
    new_class->RemoveStock = RemoveStock;

    /* Set the local global variable so we can self-reference this class. */
    Folder = new_class;

    /* Return Our Initialized Class */
    return new_class; 
}

/* Class Destruct Functions */
void class_destruct_equity (stock *stock_class)
{ 
    /* Free Memory From Variables */
    if ( stock_class->num_shares_stock_int ) {
        free( stock_class->num_shares_stock_int );
        stock_class->num_shares_stock_int = NULL;
    }
    
    if ( stock_class->current_price_stock_f ) { 
        free( stock_class->current_price_stock_f ); 
        stock_class->current_price_stock_f = NULL;
    }
    if ( stock_class->high_stock_f ) { 
        free( stock_class->high_stock_f ); 
        stock_class->high_stock_f = NULL;
    }
    if ( stock_class->low_stock_f ) { 
        free( stock_class->low_stock_f ); 
        stock_class->low_stock_f = NULL;
    }
    if ( stock_class->opening_stock_f ) { 
        free( stock_class->opening_stock_f ); 
        stock_class->opening_stock_f = NULL;
    }
    if ( stock_class->prev_closing_stock_f ) { 
        free( stock_class->prev_closing_stock_f ); 
        stock_class->prev_closing_stock_f = NULL;
    }
    if ( stock_class->change_share_f ) { 
        free( stock_class->change_share_f ); 
        stock_class->change_share_f = NULL;
    }
    if ( stock_class->change_value_f ) { 
        free( stock_class->change_value_f ); 
        stock_class->change_value_f = NULL;
    }
    if ( stock_class->change_percent_f ) { 
        free( stock_class->change_percent_f ); 
        stock_class->change_percent_f = NULL;
    }

	if ( stock_class->current_investment_stock_f ) {
        free( stock_class->current_investment_stock_f ); 
        stock_class->current_investment_stock_f = NULL;
    }

	if ( stock_class->current_price_stock_ch ) {
        free( stock_class->current_price_stock_ch );
        stock_class->current_price_stock_ch = NULL;
    }
    if ( stock_class->high_stock_ch ) {
        free( stock_class->high_stock_ch );
        stock_class->high_stock_ch = NULL;
    }
    if ( stock_class->low_stock_ch ) {
        free( stock_class->low_stock_ch );
        stock_class->low_stock_ch = NULL;
    }
    if ( stock_class->opening_stock_ch ) {
        free( stock_class->opening_stock_ch );
        stock_class->opening_stock_ch = NULL;
    }
    if ( stock_class->prev_closing_stock_ch ) {
        free( stock_class->prev_closing_stock_ch );
        stock_class->prev_closing_stock_ch = NULL;
    }
    if ( stock_class->change_share_ch ) {
        free( stock_class->change_share_ch );
        stock_class->change_share_ch = NULL;
    }
    if ( stock_class->change_value_ch ) {
        free( stock_class->change_value_ch );
        stock_class->change_value_ch = NULL;
    }
    if ( stock_class->change_percent_ch ) {
        free( stock_class->change_percent_ch );
        stock_class->change_percent_ch = NULL;
    }

	if ( stock_class->current_investment_stock_ch ) {
        free( stock_class->current_investment_stock_ch );
        stock_class->current_investment_stock_ch = NULL; 
    }
	
	if ( stock_class->symbol_stock_ch ) {
        free( stock_class->symbol_stock_ch ); 
        stock_class->symbol_stock_ch = NULL;
        
    }        
	if ( stock_class->curl_url_stock_ch ) {
        free( stock_class->curl_url_stock_ch );
        stock_class->curl_url_stock_ch = NULL;
    }

    if ( stock_class->easy_hnd ) curl_easy_cleanup( stock_class->easy_hnd );

    if ( stock_class->JSON.memory ) {
        free( stock_class->JSON.memory );
        stock_class->JSON.memory = NULL;
    }

    /* Free Memory From Class Object */
    if ( stock_class ) {
        free( stock_class ); 
        stock_class = NULL;
    }
}

void class_destruct_equity_folder (equity_folder *F)
{
    /* Free Memory From Class Objects */
    for(short c = (F->size - 1); c >= 0; c--){
        if ( F->Equity[ c ] ) class_destruct_equity( F->Equity[ c ] );
    }

    if ( F->Equity ){
        free( F->Equity );
        F->Equity = NULL;
    }

    /* Free Memory From Variables */
    if ( F->stock_port_value_f ) free( F->stock_port_value_f );
    if ( F->stock_port_value_chg_f ) free( F->stock_port_value_chg_f );
    if ( F->stock_port_value_p_chg_f ) free( F->stock_port_value_p_chg_f );

    if ( F->stock_port_value_ch ) free( F->stock_port_value_ch );      
    if ( F->stock_port_value_chg_ch ) free( F->stock_port_value_chg_ch );
    if ( F->stock_port_value_p_chg_ch ) free( F->stock_port_value_p_chg_ch ); 

    if ( F ) free( F );

}