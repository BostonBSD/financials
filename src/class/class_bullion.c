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

#include <monetary.h>
#include <locale.h>

#include "../include/class_types.h"

#include "../include/workfuncs.h"
#include "../include/multicurl.h"
#include "../include/csv.h"
#include "../include/macros.h"
#include "../include/mutex.h"

/* Class Method (also called Function) Definitions */
static double Stake (const double *ounces, const double *prem, const double *price) {
    return ( ((*prem) + (*price)) * (*ounces) );
}

static double BullionStake (const double *gold_stake, const double *silver_stake) {
    return ( (*gold_stake) + (*silver_stake) );
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

static void convert_bullion_to_strings (bullion *B){
    /* Basic metal data */
    free( B->prev_closing_metal_ch );
    B->prev_closing_metal_ch = B->DoubToStr( B->prev_closing_metal_f );

    free( B->high_metal_ch );
    B->high_metal_ch = B->DoubToStr( B->high_metal_f );

    free( B->low_metal_ch );
    B->low_metal_ch = B->DoubToStr( B->low_metal_f );

    free( B->spot_price_ch );
    B->spot_price_ch = B->DoubToStr( B->spot_price_f );

    /* The total invested in this metal */
    free( B->port_value_ch );
    B->port_value_ch = B->DoubToStr( B->port_value_f );

    /* The change in spot price per ounce. */
    free( B->change_ounce_ch );
    B->change_ounce_ch = B->DoubToStr( B->change_ounce_f );

    /* The change in total investment in this metal. */
    free( B->change_value_ch );
    B->change_value_ch = B->DoubToStr( B->change_value_f );

    /* The change in total investment in this metal as a percentage. */
    free( B->change_percent_ch );
    size_t len = strlen("###.###%%") + 1;
    B->change_percent_ch = (char*) malloc ( len );
    snprintf( B->change_percent_ch, len, "%.3lf%%", *B->change_percent_f );
}

static void ToStrings (void *data) {
    metal *M = (metal*)data;
    convert_bullion_to_strings ( M->Gold );
    convert_bullion_to_strings ( M->Silver );

    /* The total investment in bullion. */
    free( M->bullion_port_value_ch );
    M->bullion_port_value_ch = M->DoubToStr( M->bullion_port_value_f );

    /* The change in total investment in bullion. */
    free( M->bullion_port_value_chg_ch );
    M->bullion_port_value_chg_ch = M->DoubToStr( M->bullion_port_value_chg_f );

    /* The change in total investment in bullion as a percentage. */
    free( M->bullion_port_value_p_chg_ch );
    size_t len = strlen("###.###%%") + 1;
    M->bullion_port_value_p_chg_ch = (char*) malloc ( len );
    snprintf( M->bullion_port_value_p_chg_ch, len, "%.3lf%%", *M->bullion_port_value_p_chg_f );
}

static void bullion_calculations (bullion *B){
    /* The total invested in this metal */
    *B->port_value_f = B->Stake( B->ounce_f, B->premium_f, B->spot_price_f );

    /* The change in spot price per ounce. */
    *B->change_ounce_f = *B->spot_price_f - *B->prev_closing_metal_f;

    /* The change in total investment in this metal. */
    *B->change_value_f = *B->change_ounce_f * *B->ounce_f;

    /* The change in total investment in this metal as a percentage. */
    double prev_total = *B->port_value_f - *B->change_value_f;
    /* Bullion gain is calculated based off of the total bullion holdings, if there is no bullion,
       the gain is calculated based off of the current and prev spot price. These gains are ordinarily
       different because of premiums. */
    if ( prev_total == 0 ){
        *B->change_percent_f = CalcGain ( *B->spot_price_f, *B->prev_closing_metal_f );
    } else {
        *B->change_percent_f = CalcGain ( *B->port_value_f, prev_total );
    }
}

static void Calculate (void *data){
    metal* M = (metal*)data;

    bullion_calculations ( M->Gold );
    bullion_calculations ( M->Silver );

    /* The total investment in bullion. */
    *M->bullion_port_value_f = M->BullionStake( M->Gold->port_value_f, M->Silver->port_value_f );

    /* The change in total investment in bullion. */
    *M->bullion_port_value_chg_f = *M->Gold->change_value_f + *M->Silver->change_value_f;

    /* The change in total investment in bullion as a percentage. */
    double prev_total = *M->bullion_port_value_f - *M->bullion_port_value_chg_f;
    if ( prev_total == 0.0f ){
        *M->bullion_port_value_p_chg_f = 0.0f;
    } else {
        *M->bullion_port_value_p_chg_f = CalcGain ( *M->bullion_port_value_f, prev_total );
    }
}

static void create_bullion_url ( bullion *B, const char *symbol_ch ){
    time_t end_time, start_time;
    size_t len;

    time( &end_time );
    /* The start time needs to be a week before the current time, so minus seven days 
       This compensates for weekends and holidays and ensures enough data. */
    start_time = end_time - ( 86400 * 7 );

    if( B->url_ch ) free( B->url_ch );
    len = strlen( symbol_ch ) + strlen( YAHOO_URL_START ) + strlen( YAHOO_URL_MIDDLE_ONE ) + strlen( YAHOO_URL_MIDDLE_TWO ) + strlen( YAHOO_URL_END ) + 25;
    B->url_ch = malloc ( len );
    snprintf( B->url_ch, len, YAHOO_URL_START"%s"YAHOO_URL_MIDDLE_ONE"%d"YAHOO_URL_MIDDLE_TWO"%d"YAHOO_URL_END, symbol_ch, (int)start_time, (int)end_time );
}

static int GetData (void *data){
    metal *M = (metal*)data;

    create_bullion_url ( M->Gold, "GC=F" );
    create_bullion_url ( M->Silver, "SI=F" );

    SetUpCurlHandle( M->Silver->YAHOO_hnd, M->multicurl_hnd, M->Silver->url_ch, &M->Silver->CURLDATA );
    SetUpCurlHandle( M->Gold->YAHOO_hnd, M->multicurl_hnd, M->Gold->url_ch, &M->Gold->CURLDATA );

    /* Perform the cURL requests simultaneously using multi-cURL. */
    int return_code = PerformMultiCurl_no_prog( M->multicurl_hnd );
    if( return_code ){
        free ( M->Gold->CURLDATA.memory );
        free ( M->Silver->CURLDATA.memory );
        M->Gold->CURLDATA.memory = NULL;
        M->Silver->CURLDATA.memory = NULL;
    }

    return return_code;
}

static void extract_bullion_data (bullion *B) {
    if ( B->CURLDATA.memory == NULL ){
        *B->prev_closing_metal_f = 0.0f;
        *B->high_metal_f = 0.0f;
        *B->low_metal_f = 0.0f;
        *B->spot_price_f = 0.0f;
        return;
    }

    /* Convert a String to a File Pointer Stream for Reading */
    FILE* fp = fmemopen( (void*)B->CURLDATA.memory, strlen( B->CURLDATA.memory ) + 1, "r" );

    if ( fp == NULL ){
        *B->prev_closing_metal_f = 0.0f;
        *B->high_metal_f = 0.0f;
        *B->low_metal_f = 0.0f;
        *B->spot_price_f = 0.0f;
        
        free( B->CURLDATA.memory );
        B->CURLDATA.memory = NULL; 
        return;
    }
    
    char line[1024];
    char **csv_array;

    /* Yahoo! sometimes updates bullion when the equities markets are closed. 
       The while loop iterates to the end of file to get the latest data. */
    double prev_closing = 0.0f, cur_price = 0.0f;
    while (fgets( line, 1024, fp) != NULL){ 
        prev_closing = cur_price;

        Chomp( line );
        csv_array = parse_csv( line );
        cur_price = strtod( csv_array[ 4 ] ? csv_array[ 4 ] : "0", NULL );
        free_csv_line( csv_array );
    };
    
    Chomp( line );
    csv_array = parse_csv( line );
    *B->prev_closing_metal_f = prev_closing;
    *B->high_metal_f = strtod( csv_array[ 2 ] ? csv_array[ 2 ] : "0", NULL );
    *B->low_metal_f = strtod( csv_array[ 3 ] ? csv_array[ 3 ] : "0", NULL );
    *B->spot_price_f = strtod( csv_array[ 4 ] ? csv_array[ 4 ] : "0", NULL );

    free_csv_line( csv_array );
    fclose( fp );
    free( B->CURLDATA.memory );
    B->CURLDATA.memory = NULL; 
}

static void ExtractData (void *data) {
    metal *M = (metal*)data;

    extract_bullion_data ( M->Gold );
    extract_bullion_data ( M->Silver );
}

static void StopCurl (void *data) {
    metal *M = (metal*)data;

    curl_multi_wakeup( M->multicurl_hnd );
    pthread_mutex_lock( &mutex_working[ MULTICURL_NO_PROG_MUTEX ] );

    curl_multi_remove_handle( M->multicurl_hnd, M->Gold->YAHOO_hnd );
    curl_multi_remove_handle( M->multicurl_hnd, M->Silver->YAHOO_hnd );

    pthread_mutex_unlock( &mutex_working[ MULTICURL_NO_PROG_MUTEX ] );
}

/* Class Init Functions */
bullion* class_init_bullion ()
{ 
    /* Allocate Memory For A New Class Object */
    bullion* new_class = (bullion*) malloc( sizeof(*new_class) ); 

    /* Allocate Memory For Variables */
    new_class->spot_price_f = (double*) malloc( sizeof(double) );
    new_class->premium_f = (double*) malloc( sizeof(double) );
    new_class->port_value_f = (double*) malloc( sizeof(double) );
    new_class->ounce_f = (double*) malloc( sizeof(double) );

    new_class->high_metal_f = (double*) malloc( sizeof(double) );
    new_class->low_metal_f = (double*) malloc( sizeof(double) );
    new_class->prev_closing_metal_f = (double*) malloc( sizeof(double) );
    new_class->change_ounce_f = (double*) malloc( sizeof(double) );
    new_class->change_value_f = (double*) malloc( sizeof(double) );
    new_class->change_percent_f = (double*) malloc( sizeof(double) );

    new_class->spot_price_ch = (char*) malloc( strlen("$0.00")+1 );            
    new_class->premium_ch = (char*) malloc( strlen("$0.00")+1 ); 
    new_class->port_value_ch = (char*) malloc( strlen("$0.00")+1 );            
    new_class->ounce_ch = (char*) malloc( strlen("0.000")+1 );  

    new_class->high_metal_ch = (char*) malloc( strlen("$0.00")+1 );
    new_class->low_metal_ch = (char*) malloc( strlen("$0.00")+1 );
    new_class->prev_closing_metal_ch = (char*) malloc( strlen("$0.00")+1 );
    new_class->change_ounce_ch = (char*) malloc( strlen("$0.00")+1 );
    new_class->change_value_ch = (char*) malloc( strlen("$0.00")+1 );
    new_class->change_percent_ch = (char*) malloc( strlen("000.000%%")+1 );                            
    
    /* Initialize Variables */
    *new_class->ounce_f = 0.0;
    *new_class->spot_price_f = 0.0;
    *new_class->premium_f = 0.0;
    *new_class->port_value_f = 0.0;

    *new_class->high_metal_f = 0.0;
    *new_class->low_metal_f = 0.0;
    *new_class->prev_closing_metal_f = 0.0;
    *new_class->change_ounce_f = 0.0;
    *new_class->change_value_f = 0.0;
    *new_class->change_percent_f = 0.0;

    new_class->url_ch = NULL;

    strcpy( new_class->spot_price_ch,"$0.00" );
    strcpy( new_class->premium_ch,"$0.00" );
    strcpy( new_class->port_value_ch,"$0.00" );
    strcpy( new_class->ounce_ch,"0.000" );

    strcpy( new_class->high_metal_ch,"$0.00" );
    strcpy( new_class->low_metal_ch,"$0.00" );
    strcpy( new_class->prev_closing_metal_ch,"$0.00" );
    strcpy( new_class->change_ounce_ch,"$0.00" );
    strcpy( new_class->change_value_ch,"$0.00" );
    strcpy( new_class->change_percent_ch,"000.000%%" );

    new_class->YAHOO_hnd = curl_easy_init ();
    new_class->CURLDATA.memory = NULL;

    /* Connect Function Pointers To Function Definitions */
    new_class->Stake = Stake;
    new_class->DoubToStr = DoubToStr;
    new_class->StrToDoub = StrToDoub;
    
    /* Return Our Initialized Class */
    return new_class; 
}

metal* class_init_metal ()
{
    /* Allocate Memory For A New Class */
    metal* new_class = (metal*) malloc( sizeof(*new_class) );

    /* Initialize Nested Class Objects */
    new_class->Gold = class_init_bullion ();
    new_class->Silver = class_init_bullion ();

    /* Allocate Memory For Variables */
    new_class->bullion_port_value_f = (double*) malloc( sizeof(double) );
    new_class->bullion_port_value_chg_f = (double*) malloc( sizeof(double) );
    new_class->bullion_port_value_p_chg_f = (double*) malloc( sizeof(double) );

    new_class->bullion_port_value_ch = (char*) malloc( strlen("$0.00")+1 );
    new_class->bullion_port_value_chg_ch = (char*) malloc( strlen("$0.00")+1 ); 
    new_class->bullion_port_value_p_chg_ch = (char*) malloc( strlen("000.000%%")+1 );

    /* Initialize Variables */
    *new_class->bullion_port_value_f = 0.0;
    *new_class->bullion_port_value_chg_f = 0.0;
    *new_class->bullion_port_value_p_chg_f = 0.0;

    strcpy( new_class->bullion_port_value_ch,"$0.00" );
    strcpy( new_class->bullion_port_value_chg_ch,"$0.00" );
    strcpy( new_class->bullion_port_value_p_chg_ch,"000.000%%" );

    /* Create a MultiCurl Handle */
    new_class->multicurl_hnd = curl_multi_init();

    /* Connect Function Pointers To Function Definitions */
    new_class->BullionStake = BullionStake;
    new_class->DoubToStr = DoubToStr;
    new_class->ToStrings = ToStrings;
    new_class->Calculate = Calculate;
    new_class->GetData = GetData;
    new_class->ExtractData = ExtractData;
    new_class->StopCurl = StopCurl;

    /* Return Our Initialized Class */
    return new_class; 
}

/* Class Destruct Functions */
void class_destruct_bullion (bullion* bullion_class)
{ 
    /* Free Memory From Variables */
    if ( bullion_class->spot_price_f ) free( bullion_class->spot_price_f );
    if ( bullion_class->premium_f ) free( bullion_class->premium_f );
    if ( bullion_class->port_value_f ) free( bullion_class->port_value_f );
    if ( bullion_class->ounce_f ) free( bullion_class->ounce_f );

    if ( bullion_class->high_metal_f ) free( bullion_class->high_metal_f );
    if ( bullion_class->low_metal_f ) free( bullion_class->low_metal_f );
    if ( bullion_class->prev_closing_metal_f ) free( bullion_class->prev_closing_metal_f );
    if ( bullion_class->change_ounce_f ) free( bullion_class->change_ounce_f );
    if ( bullion_class->change_value_f ) free( bullion_class->change_value_f );
    if ( bullion_class->change_percent_f ) free( bullion_class->change_percent_f );

    if ( bullion_class->url_ch ) free( bullion_class->url_ch );
    if ( bullion_class->spot_price_ch ) free( bullion_class->spot_price_ch );            
    if ( bullion_class->premium_ch ) free( bullion_class->premium_ch ); 
    if ( bullion_class->port_value_ch ) free( bullion_class->port_value_ch );            
    if ( bullion_class->ounce_ch ) free( bullion_class->ounce_ch );

    if ( bullion_class->high_metal_ch ) free( bullion_class->high_metal_ch );
    if ( bullion_class->low_metal_ch ) free( bullion_class->low_metal_ch );
    if ( bullion_class->prev_closing_metal_ch ) free( bullion_class->prev_closing_metal_ch );
    if ( bullion_class->change_ounce_ch ) free( bullion_class->change_ounce_ch );
    if ( bullion_class->change_value_ch ) free( bullion_class->change_value_ch );
    if ( bullion_class->change_percent_ch ) free( bullion_class->change_percent_ch );  

    if ( bullion_class->YAHOO_hnd ) curl_easy_cleanup( bullion_class->YAHOO_hnd );
    if ( bullion_class->CURLDATA.memory ) {
        free( bullion_class->CURLDATA.memory );
        bullion_class->CURLDATA.memory = NULL;
    }

    /* Free Memory From Class Object */
    if ( bullion_class ) {
        free( bullion_class ); 
        bullion_class = NULL;
    }
}

void class_destruct_metal (metal* metal_handle)
{
    /* Free Memory From Class Objects */
    if ( metal_handle->Gold ) class_destruct_bullion ( metal_handle->Gold );
    if ( metal_handle->Silver ) class_destruct_bullion ( metal_handle->Silver );

    /* Free Memory From Variables */
    if ( metal_handle->bullion_port_value_f ) free( metal_handle->bullion_port_value_f );
    if ( metal_handle->bullion_port_value_chg_f ) free( metal_handle->bullion_port_value_chg_f );
    if ( metal_handle->bullion_port_value_p_chg_f ) free( metal_handle->bullion_port_value_p_chg_f );

    if ( metal_handle->bullion_port_value_ch ) free( metal_handle->bullion_port_value_ch );
    if ( metal_handle->bullion_port_value_chg_ch ) free( metal_handle->bullion_port_value_chg_ch );
    if ( metal_handle->bullion_port_value_p_chg_ch ) free( metal_handle->bullion_port_value_p_chg_ch );

    if ( metal_handle->multicurl_hnd ) curl_multi_cleanup ( metal_handle->multicurl_hnd );

    if ( metal_handle ) free( metal_handle );
}