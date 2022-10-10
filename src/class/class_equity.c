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

#include "../include/class.h" /* The class init and destruct funcs are required in the class methods */

#include "../include/multicurl.h"
#include "../include/workfuncs.h"
#include "../include/json.h"
#include "../include/macros.h"
#include "../include/mutex.h"

/* Class Method (also called Function) Definitions */
static double Stake (const unsigned int *shares, const double *price) {
    return ( (double)(*shares) * (*price) );
}

static char* DoubToStr (const double *num) 
/* Take in a double pointer [the datatype is double] and convert to a monetary format string. */
{    
    /* Char variables are 1 byte long, no need to scale strlen() by sizeof(char). */
    char* str = (char*) malloc( strlen("############.##")+1 ); 

    /* "" is the default system locale, the C.UTF-8 locale does not have a monetary 
       format and is the default on FreeBSD unless changed by the admin. 
    */
    setlocale(LC_ALL, LOCALE);  
    strfmon(str, strlen("############.##"), "%(.3n", *num);

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

static void ToStrings (void *data){
    equity_folder *F = (equity_folder*)data;

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
    if( *S->num_shares_stock_int > 0){
        *S->change_value_f = *S->change_share_f * (double)*S->num_shares_stock_int;
    } else {
        *S->change_value_f = 0.0f;
    }

    /* The total current investment in this equity. */
    *S->current_investment_stock_f = S->Stake( S->num_shares_stock_int, S->current_price_stock_f );
}

static void Calculate (void *data){
    equity_folder* F = (equity_folder*)data;

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

static void GenerateURL (void *eq_folder_data, void *met_data){
    equity_folder *F = (equity_folder*)eq_folder_data;
    meta *Met = (meta*)met_data;

    size_t len;
 
    /* Cycle through the list of equities. */
    for( unsigned short c = 0; c < F->size; c++ ) { 
        /* Generate the request URL for this equity. */
        free( F->Equity[ c ]->curl_url_stock_ch );

        len = strlen(Met->stock_url) + strlen(F->Equity[ c ]->symbol_stock_ch) + strlen(Met->curl_key)+1;
        F->Equity[ c ]->curl_url_stock_ch = (char*) malloc( len );
        snprintf( F->Equity[ c ]->curl_url_stock_ch, len, "%s%s%s", Met->stock_url, F->Equity[ c ]->symbol_stock_ch, Met->curl_key );
    }
}

static int GetData (void *data){
    equity_folder *F = (equity_folder*)data;

    /* Cycle through the list of equities. */
    for( unsigned short c = 0; c < F->size; c++ ) {       
        /* Add a cURL easy handle to the multi-cURL handle 
        (passing JSON output struct by reference) */
        SetUpCurlHandle( F->Equity[ c ]->easy_hnd, F->multicurl_hnd, F->Equity[ c ]->curl_url_stock_ch, &F->Equity[ c ]->JSON );
    }

    /* Perform the cURL requests simultaneously using multi-cURL. */
    int return_code = PerformMultiCurl( F->multicurl_hnd, (double)F->size );
    if( return_code ){
        for( unsigned short c = 0; c < F->size; c++ ) {       
            free( F->Equity[ c ]->JSON.memory );
            F->Equity[ c ]->JSON.memory = NULL;
        }
    }
    
    return return_code;
}

static void Reset(void *data) {
    equity_folder *F = (equity_folder*)data;

    if (F->size == 0){
        /*
        We do not know if F->size = 0 means that the stock** array already 
        exists or not.

        This is the only place where the stock** array is created.

        There are multiple points in the code where a zero size array
        could be reset, some where the array exists, some where it does not 
        exist, so we need to keep freeing this to prevent a memory leak.

        This function is only called from sqlite.c
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
}

static void AddStock(void *data)
/* Adds a new stock object to our folder, 
   does not increment size. */
{
    equity_folder *F = (equity_folder*)data;

    /* realloc will free memory if assigned a smaller new value. */
    stock** tmp = (stock**) realloc( F->Equity, (F->size + 1) * sizeof(stock*) );              

    if (tmp == NULL){
        printf("Not Enough Memory, realloc returned NULL.\n");
        exit(EXIT_FAILURE);
    }
                
    F->Equity = tmp;

    /* class_init_equity returns an object pointer. */
    F->Equity[ F->size ] = class_init_equity (); 
}

static void ExtractData (void *data) {
    equity_folder *F = (equity_folder*)data;

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

static void Sort(void *data) {
    /* Sort the equity folder in alphabetically ascending order. */
    equity_folder *F = (equity_folder*)data;
    qsort(&F->Equity[0], (size_t)F->size, sizeof(stock*), alpha_asc);
}

static void StopCurl (void *data)
/* Removing the easy handle from the multihandle will stop the cURL data transfer
   immediately. curl_multi_remove_handle does nothing if the easy handle is not
   currently set in the multihandle. */
{
    equity_folder *F = (equity_folder*)data;

    curl_multi_wakeup( F->multicurl_hnd );
    pthread_mutex_lock( &mutex_working[ MULTICURL_PROG_MUTEX ] );

    for(unsigned short i=0; i<F->size; i++){
        curl_multi_remove_handle( F->multicurl_hnd, F->Equity[ i ]->easy_hnd );
        
    }

    pthread_mutex_unlock( &mutex_working[ MULTICURL_PROG_MUTEX ] );
}

/* Class Init Functions */
stock* class_init_equity ()
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

	new_class->current_price_stock_ch = (char*) malloc( strlen("$0.00")+1 );
    new_class->high_stock_ch = (char*) malloc( strlen("$0.00")+1 );
    new_class->low_stock_ch = (char*) malloc( strlen("$0.00")+1 );
    new_class->opening_stock_ch = (char*) malloc( strlen("$0.00")+1 );
    new_class->prev_closing_stock_ch = (char*) malloc( strlen("$0.00")+1 );
    new_class->change_share_ch = (char*) malloc( strlen("$0.00")+1 );
    new_class->change_value_ch = (char*) malloc( strlen("$0.00")+1 );
    new_class->change_percent_ch = (char*) malloc( strlen("000.000%%")+1 );

	new_class->current_investment_stock_ch = (char*) malloc( strlen("$0.00")+1 ); 
	
	new_class->symbol_stock_ch = (char*) malloc( strlen("NO_SYMBOL")+1 );         
	new_class->curl_url_stock_ch = (char*) malloc( strlen("http://")+1 ); 

    /* Initialize Variables */
    *new_class->num_shares_stock_int = 0;
    
    *new_class->current_price_stock_f = 0.0;
    *new_class->high_stock_f = 0.0;
    *new_class->low_stock_f = 0.0;
    *new_class->opening_stock_f = 0.0;
    *new_class->prev_closing_stock_f = 0.0;
    *new_class->change_share_f = 0.0;
    *new_class->change_value_f = 0.0;
    *new_class->change_percent_f = 0.0;

	*new_class->current_investment_stock_f = 0.0;  
    
    strcpy( new_class->current_price_stock_ch,"$0.00" );
    strcpy( new_class->high_stock_ch,"$0.00" );
    strcpy( new_class->low_stock_ch,"$0.00" );
    strcpy( new_class->opening_stock_ch,"$0.00" );
    strcpy( new_class->prev_closing_stock_ch,"$0.00" );
    strcpy( new_class->change_share_ch,"$0.00" );
    strcpy( new_class->change_value_ch,"$0.00" );
    strcpy( new_class->change_percent_ch,"000.000%%" );

    strcpy( new_class->current_investment_stock_ch,"$0.00" );

    strcpy( new_class->symbol_stock_ch,"NO_SYMBOL" );
    strcpy( new_class->curl_url_stock_ch,"http://" );  

    new_class->easy_hnd = curl_easy_init ();
    new_class->JSON.memory = NULL;

    /* Connect Function Pointers To Function Definitions */
    new_class->Stake = Stake;
    new_class->DoubToStr = DoubToStr;
    new_class->StrToDoub = StrToDoub;

    /* Return Our Initialized Class */
    return new_class; 
}

equity_folder* class_init_equity_folder ()
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

    new_class->stock_port_value_ch = (char*) malloc( strlen("$0.00")+1 );   
    new_class->stock_port_value_chg_ch = (char*) malloc( strlen("$0.00")+1 ); 
    new_class->stock_port_value_p_chg_ch = (char*) malloc( strlen("000.000%%")+1 );

    /* Initialize Variables */
    strcpy( new_class->stock_port_value_ch,"$0.00" );
    strcpy( new_class->stock_port_value_chg_ch,"$0.00" );
    strcpy( new_class->stock_port_value_p_chg_ch,"000.000%%" );

    *new_class->stock_port_value_f = 0.0;
    *new_class->stock_port_value_chg_f = 0.0;
    *new_class->stock_port_value_p_chg_f = 0.0;

    /* Create a MultiCurl Handle */
    new_class->multicurl_hnd = curl_multi_init();

    /* Connect Function Pointers To Function Definitions */
    /* The functions do not need to have the same name as the pointer,
       but it is easier to follow this way. */
    new_class->DoubToStr = DoubToStr;
    new_class->ToStrings = ToStrings;
    new_class->Calculate = Calculate;
    new_class->GenerateURL = GenerateURL;
    new_class->GetData = GetData;
    new_class->ExtractData = ExtractData;
    new_class->AddStock = AddStock;
    new_class->Reset = Reset;
    new_class->Sort = Sort;
    new_class->StopCurl = StopCurl;

    /* Return Our Initialized Class */
    return new_class; 
}

/* Class Destruct Functions */
void class_destruct_equity (stock* stock_class)
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

void class_destruct_equity_folder (equity_folder* F)
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

    curl_multi_cleanup ( F->multicurl_hnd );
    if ( F ) free( F );

}