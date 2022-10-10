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
#include <stdbool.h>

#include <unistd.h>
#include <pwd.h>

#include <monetary.h>
#include <locale.h>

#include "../include/class_types.h"

#include "../include/multicurl_types.h"
#include "../include/workfuncs.h"
#include "../include/macros.h"
#include "../include/mutex.h"

/* Class Method (also called Function) Definitions */
static double EntireStake (const double *bullion, const double *equity, const double *cash) {
    return ( (*bullion) + (*equity) + (*cash) );
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

static void PortfolioToStrings (void *data){
    meta* Met = (meta*)data;

    /* The total portfolio value. */
    free( Met->portfolio_port_value_ch );
    Met->portfolio_port_value_ch = Met->DoubToStr( Met->portfolio_port_value_f );

    /* The change in total portfolio value. */
    free( Met->portfolio_port_value_chg_ch );
    Met->portfolio_port_value_chg_ch = Met->DoubToStr( Met->portfolio_port_value_chg_f );

    /* The change in total portfolio value as a percentage. */
    free( Met->portfolio_port_value_p_chg_ch );
    size_t len = strlen("###.###%%") + 1;
    Met->portfolio_port_value_p_chg_ch = (char*) malloc ( len );
    snprintf( Met->portfolio_port_value_p_chg_ch, len, "%.3lf%%", *Met->portfolio_port_value_p_chg_f );
}

static void PortfolioCalculate (void *met_data, void *metal_data, void *eq_folder_data){
    meta* Met = (meta*)met_data;
    metal* M = (metal*)metal_data;
    equity_folder* F = (equity_folder*)eq_folder_data;

    /* The total portfolio value. */
    *Met->portfolio_port_value_f = Met->EntireStake( M->bullion_port_value_f, F->stock_port_value_f, Met->cash_f);

    /* The change in total portfolio value. */
    /* Edit the next line as needed, if you want to 
       add a change value besides equity and bullion to the portfolio. */
    *Met->portfolio_port_value_chg_f = *F->stock_port_value_chg_f + *M->bullion_port_value_chg_f;
    
    /* The change in total portfolio value as a percentage. */
    double prev_total = *Met->portfolio_port_value_f - *Met->portfolio_port_value_chg_f;
    *Met->portfolio_port_value_p_chg_f = CalcGain ( *Met->portfolio_port_value_f, prev_total );
}

static void StopRSICurl (void *data) {
    meta *Met = (meta*)data;

    curl_multi_wakeup( Met->multicurl_rsi_hnd );
    pthread_mutex_lock( &mutex_working[ MULTICURL_NO_PROG_MUTEX ] );

    curl_multi_remove_handle( Met->multicurl_rsi_hnd, Met->rsi_hnd );

    pthread_mutex_unlock( &mutex_working[ MULTICURL_NO_PROG_MUTEX ] );
}

static void StopSNMapCurl (void *data) {
    meta *Met = (meta*)data;

    curl_multi_wakeup( Met->multicurl_cmpltn_hnd );
    pthread_mutex_lock( &mutex_working[ MULTICURL_NO_PROG_MUTEX ] );

    curl_multi_remove_handle( Met->multicurl_cmpltn_hnd, Met->NASDAQ_completion_hnd );
    curl_multi_remove_handle( Met->multicurl_cmpltn_hnd, Met->NYSE_completion_hnd );

    pthread_mutex_unlock( &mutex_working[ MULTICURL_NO_PROG_MUTEX ] );
}

/* Class Init Functions */
meta* class_init_meta_data ()
{ 
    /* Allocate Memory For A New Class Object */
    meta* new_class = (meta*) malloc( sizeof(*new_class) );

    /* Allocate Memory For Variables */
    new_class->cash_f = (double*) malloc( sizeof(double) );
    new_class->portfolio_port_value_f = (double*) malloc( sizeof(double) );
    new_class->portfolio_port_value_chg_f = (double*) malloc( sizeof(double) );
    new_class->portfolio_port_value_p_chg_f = (double*) malloc( sizeof(double) );
    new_class->updates_per_min_f = (double*) malloc( sizeof(double) );
    new_class->updates_hours_f = (double*) malloc( sizeof(double) );

    new_class->stock_url = (char*) malloc( strlen( FINNHUB_URL )+1 );
	new_class->curl_key = (char*) malloc( strlen( FINNHUB_URL_TOKEN )+1 );                 

    new_class->cash_ch = (char*) malloc( strlen("$0.00")+1 );                          
    new_class->portfolio_port_value_ch = (char*) malloc( strlen("$0.00")+1 ); 
    new_class->portfolio_port_value_chg_ch = (char*) malloc( strlen("$0.00")+1 ); 
    new_class->portfolio_port_value_p_chg_ch = (char*) malloc( strlen("000.000%%")+1 ); 

    new_class->fetching_data_bool = (bool*) malloc( sizeof(bool) );
    new_class->holiday_bool = (bool*) malloc( sizeof(bool) );
    new_class->multicurl_cancel_bool = (bool*) malloc( sizeof(bool) );

    /* Initialize Variables */
    strcpy( new_class->stock_url, FINNHUB_URL );
    strcpy( new_class->curl_key, FINNHUB_URL_TOKEN );
    strcpy( new_class->cash_ch,"$0.00" );
    strcpy( new_class->portfolio_port_value_ch,"$0.00" );
    strcpy( new_class->portfolio_port_value_chg_ch,"$0.00" );
    strcpy( new_class->portfolio_port_value_p_chg_ch,"000.000%%" );

    *new_class->cash_f = 0.0;
    *new_class->portfolio_port_value_f = 0.0;
    *new_class->portfolio_port_value_chg_f = 0.0;
    *new_class->portfolio_port_value_p_chg_f = 0.0;
    *new_class->updates_per_min_f = 6.0;
    *new_class->updates_hours_f = 1.0;

    *new_class->fetching_data_bool = false;
    *new_class->holiday_bool = false;
    *new_class->multicurl_cancel_bool = false;

    new_class->rsi_hnd = curl_easy_init ();
    new_class->NASDAQ_completion_hnd = curl_easy_init ();
    new_class->NYSE_completion_hnd = curl_easy_init ();

    new_class->multicurl_cmpltn_hnd = curl_multi_init ();
    new_class->multicurl_rsi_hnd = curl_multi_init ();

    /* Set The User's Home Directory */
    /* We need to get the path to the User's home directory: */
    /* Get User ID (need to #include<pwd.h> and #include<unistd.h> for this) */
    uid_t uid = getuid();

    /* Get User Account Information For This ID */
    struct passwd *pw = getpwuid(uid);

    if (pw == NULL) {
        printf("Gathering User Account Info Failed\n");
        exit( EXIT_FAILURE );
    }

    new_class->home_dir_ch = strdup( pw->pw_dir ); 

    /* Append the sqlite db file path to the end of the home directory path. */
    size_t len = strlen(pw->pw_dir) + strlen( DB_FILE ) + 1;
    new_class->sqlite_db_path_ch = (char*) malloc( len );
    snprintf(new_class->sqlite_db_path_ch, len, "%s%s", pw->pw_dir, DB_FILE);

    /* Connect Function Pointers To Function Definitions */
    new_class->EntireStake = EntireStake;
    new_class->DoubToStr = DoubToStr;
    new_class->StrToDoub = StrToDoub;
    new_class->PortfolioToStrings = PortfolioToStrings;
    new_class->PortfolioCalculate = PortfolioCalculate;
    new_class->StopRSICurl = StopRSICurl;
    new_class->StopSNMapCurl = StopSNMapCurl;

    /* Return Our Initialized Class */
    return new_class; 
}

/* Class Destruct Functions */
void class_destruct_meta_data (meta* meta_class)
{ 
    /* Free Memory From Variables */
    if ( meta_class->cash_f ) free( meta_class->cash_f );
    if ( meta_class->portfolio_port_value_f ) free( meta_class->portfolio_port_value_f );
    if ( meta_class->portfolio_port_value_chg_f ) free( meta_class->portfolio_port_value_chg_f );
    if ( meta_class->portfolio_port_value_p_chg_f ) free( meta_class->portfolio_port_value_p_chg_f );
    if ( meta_class->updates_per_min_f ) free( meta_class->updates_per_min_f );
    if ( meta_class->updates_hours_f ) free( meta_class->updates_hours_f );

    if ( meta_class->stock_url ) free( meta_class->stock_url );
	if ( meta_class->curl_key ) free( meta_class->curl_key );                 

    if ( meta_class->cash_ch ) free( meta_class->cash_ch );                      
    if ( meta_class->portfolio_port_value_ch ) free( meta_class->portfolio_port_value_ch );
    if ( meta_class->portfolio_port_value_chg_ch ) free( meta_class->portfolio_port_value_chg_ch );
    if ( meta_class->portfolio_port_value_p_chg_ch ) free( meta_class->portfolio_port_value_p_chg_ch );

    if ( meta_class->home_dir_ch ) free( meta_class->home_dir_ch );
    if ( meta_class->sqlite_db_path_ch ) free( meta_class->sqlite_db_path_ch );

    if ( meta_class->fetching_data_bool ) free( meta_class->fetching_data_bool );
    if ( meta_class->holiday_bool ) free( meta_class->holiday_bool );
    if ( meta_class->multicurl_cancel_bool ) free( meta_class->multicurl_cancel_bool );

    if ( meta_class->NASDAQ_completion_hnd ) curl_easy_cleanup( meta_class->NASDAQ_completion_hnd );
    if ( meta_class->NYSE_completion_hnd ) curl_easy_cleanup( meta_class->NYSE_completion_hnd );
    if ( meta_class->rsi_hnd ) curl_easy_cleanup( meta_class->rsi_hnd );

    if ( meta_class->multicurl_cmpltn_hnd ) curl_multi_cleanup ( meta_class->multicurl_cmpltn_hnd );
    if ( meta_class->multicurl_rsi_hnd ) curl_multi_cleanup ( meta_class->multicurl_rsi_hnd );


    /* Free Memory From Class Object */
    if ( meta_class ) {
         free( meta_class ); 
         meta_class = NULL;
    }
}