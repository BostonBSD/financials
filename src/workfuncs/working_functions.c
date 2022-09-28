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

#include "../financials.h"

int AlphaAsc (const void *a, const void *b)
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

int AlphaAscSecName (const void *a, const void *b)
/* This is a callback function for stdlib sorting functions. 
   It compares symbol_to_security_name_container in alphabetically 
   ascending order, by the stock symbol data member.  Swapping only 
   the pointer.
*/ 
{
    /* Cast the void pointer as a stock double pointer. */
    symbol_to_security_name_container **aa = (symbol_to_security_name_container **)a;
    symbol_to_security_name_container **bb = (symbol_to_security_name_container **)b;
    
    /* Compare the stock symbols ignoring for case. */
    return strcasecmp( aa[0]->symbol, bb[0]->symbol );
}

void AddStock(equity_folder* F)
/* Adds a new stock object to our array. */
{
    /* realloc will free memory if assigned a smaller new value. */
    stock** tmp = (stock**) realloc( F->Equity, (F->size + 1) * sizeof(stock*) );              

    if (tmp == NULL){
        printf("Not Enough Memory, realloc returned NULL.\n");
        exit(EXIT_FAILURE);
    }
                
    F->Equity = tmp;

    /* Returns an object pointer. */
    F->Equity[ F->size ] = class_init_equity (); 
}

void ResetEquity(equity_folder *F) {
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

void PerformCalculations () 
/* calc portfolio values and populate character strings. */
{
    pthread_mutex_lock( &mutex_working [CLASS_MEMBER_MUTEX ] );

    *Precious->Silver->port_value_f = Precious->Silver->Stake( Precious->Silver->ounce_f, Precious->Silver->premium_f, Precious->Silver->spot_price_f );
    *Precious->Gold->port_value_f = Precious->Gold->Stake( Precious->Gold->ounce_f, Precious->Gold->premium_f, Precious->Gold->spot_price_f );

    free( Precious->Silver->port_value_ch );
    free( Precious->Gold->port_value_ch );
    Precious->Silver->port_value_ch = Precious->Silver->DoubToStr( Precious->Silver->port_value_f );
    Precious->Gold->port_value_ch = Precious->Gold->DoubToStr( Precious->Gold->port_value_f );
    
    *MetaData->bullion_port_value_f = MetaData->BullionStake( Precious->Gold->port_value_f, Precious->Silver->port_value_f );

    free( MetaData->bullion_port_value_ch );
    MetaData->bullion_port_value_ch = MetaData->DoubToStr( MetaData->bullion_port_value_f );

    *MetaData->stock_port_value_f = 0;
    *MetaData->stock_port_value_chg_f = 0;
    unsigned short c = 0;
    while( c < Folder->size ) {
        *Folder->Equity[ c ]->current_investment_stock_f = Folder->Equity[ c ]->Stake( Folder->Equity[ c ]->num_shares_stock_int, Folder->Equity[ c ]->current_price_stock_f );
        *MetaData->stock_port_value_f += *Folder->Equity[ c ]->current_investment_stock_f;
        *MetaData->stock_port_value_chg_f += *Folder->Equity[ c ]->change_value_f;

        free( Folder->Equity[ c ]->current_investment_stock_ch );
        Folder->Equity[ c ]->current_investment_stock_ch = Folder->Equity[ c ]->DoubToStr( Folder->Equity[ c ]->current_investment_stock_f );
        
        c++;
    }
    free( MetaData->stock_port_value_ch );
    MetaData->stock_port_value_ch = MetaData->DoubToStr( MetaData->stock_port_value_f );
    free( MetaData->stock_port_value_chg_ch );
    MetaData->stock_port_value_chg_ch = MetaData->DoubToStr( MetaData->stock_port_value_chg_f );

    double prev_value = *MetaData->stock_port_value_f - *MetaData->stock_port_value_chg_f;
    *MetaData->stock_port_value_p_chg_f = 100 * ( *MetaData->stock_port_value_chg_f / prev_value );
    
    free( MetaData->stock_port_value_p_chg_ch );
    size_t len = strlen("###.###%%") + 1;
    MetaData->stock_port_value_p_chg_ch = (char*) malloc ( len );
    snprintf( MetaData->stock_port_value_p_chg_ch, len, "%.3lf%%", *MetaData->stock_port_value_p_chg_f );

    *MetaData->portfolio_port_value_f = MetaData->EntireStake( MetaData->bullion_port_value_f, MetaData->stock_port_value_f, MetaData->cash_f);
    
    free( MetaData->portfolio_port_value_ch );
    MetaData->portfolio_port_value_ch = MetaData->DoubToStr( MetaData->portfolio_port_value_f );

    /* Edit the next line as needed, if you want to 
       add a change value besides equity to the portfolio. */
    *MetaData->portfolio_port_value_chg_f = *MetaData->stock_port_value_chg_f;
    prev_value = *MetaData->portfolio_port_value_f - *MetaData->portfolio_port_value_chg_f;
    *MetaData->portfolio_port_value_p_chg_f = 100 * ( *MetaData->portfolio_port_value_chg_f / prev_value );
    
    free( MetaData->portfolio_port_value_ch );
    MetaData->portfolio_port_value_ch = MetaData->DoubToStr( MetaData->portfolio_port_value_f );
    free( MetaData->portfolio_port_value_chg_ch );
    MetaData->portfolio_port_value_chg_ch = MetaData->DoubToStr( MetaData->portfolio_port_value_chg_f );

    free( MetaData->portfolio_port_value_p_chg_ch );
    len = strlen("###.###%%") + 1;
    MetaData->portfolio_port_value_p_chg_ch = (char*) malloc ( len );
    snprintf( MetaData->portfolio_port_value_p_chg_ch, len, "%.3lf%%", *MetaData->portfolio_port_value_p_chg_f );

    pthread_mutex_unlock( &mutex_working [CLASS_MEMBER_MUTEX ] );
}

int MultiCurlProcessing () {
    size_t len;
 
    /* Cycle through the list of equities. */
    for( unsigned short c = 0; c < Folder->size; c++ ) { 
        /* Generate the request URL for this equity. */
        free( Folder->Equity[ c ]->curl_url_stock_ch );

        len = strlen(MetaData->stock_url) + strlen(Folder->Equity[ c ]->symbol_stock_ch) + strlen(MetaData->curl_key)+1;
        Folder->Equity[ c ]->curl_url_stock_ch = (char*) malloc( len );
        snprintf( Folder->Equity[ c ]->curl_url_stock_ch, len, "%s%s%s", MetaData->stock_url, Folder->Equity[ c ]->symbol_stock_ch, MetaData->curl_key );
        
        /* Add a cURL easy handle to the multi-cURL handle 
        (passing JSON output struct by reference) */
        SetUpCurlHandle( Folder->Equity[ c ]->easy_hnd, Folder->multicurl_hnd, Folder->Equity[ c ]->curl_url_stock_ch, &Folder->Equity[ c ]->JSON );
    }

    /* Perform the cURL requests simultaneously using multi-cURL. */
    return PerformMultiCurl( Folder->multicurl_hnd, (double)Folder->size );
}

void GetBullionUrl_Yahoo ( char** SilverUrl, char** GoldUrl ){
    time_t end_time, start_time;
    struct tm NY_tz = NYTimeComponents ();
    size_t len;

    time( &end_time );
    /* if today is Sunday in NY */
    if ( NY_tz.tm_wday == 0 ){
        /* the start time needs to be Friday, so minus two days */
        start_time = end_time - (86400 * 2);

    /* if today is Saturday in NY */
    } else  if ( NY_tz.tm_wday == 6 ){   
        /* the start time needs to be Friday, so minus one day */
        start_time = end_time - 86400;

    /* if today is a Monday holiday in NY */
    } else if (*MetaData->holiday_bool && NY_tz.tm_wday == 1){
        /* the start time needs to be Friday, so minus three days */
        start_time = end_time - (86400 * 3);

    /* if today is a non-Monday holiday in NY */
    } else if (*MetaData->holiday_bool && NY_tz.tm_wday != 1){
        /* the start time needs to be yesterday, so minus one day */
        start_time = end_time - 86400;

    } else {
        start_time = end_time;
    }

    char *silver_symbol_ch = "SI=F";
    char *gold_symbol_ch = "GC=F";

    len = strlen( silver_symbol_ch ) + strlen( YAHOO_URL_START ) + strlen( YAHOO_URL_MIDDLE_ONE ) + strlen( YAHOO_URL_MIDDLE_TWO ) + strlen( YAHOO_URL_END ) + 25;
    SilverUrl[0] = malloc ( len );
    snprintf(SilverUrl[0], len, YAHOO_URL_START"%s"YAHOO_URL_MIDDLE_ONE"%d"YAHOO_URL_MIDDLE_TWO"%d"YAHOO_URL_END, silver_symbol_ch, (int)start_time, (int)end_time);
    
    len = strlen( gold_symbol_ch ) + strlen( YAHOO_URL_START ) + strlen( YAHOO_URL_MIDDLE_ONE ) + strlen( YAHOO_URL_MIDDLE_TWO ) + strlen( YAHOO_URL_END ) + 25;
    GoldUrl[0] = malloc ( len );
    snprintf(GoldUrl[0], len, YAHOO_URL_START"%s"YAHOO_URL_MIDDLE_ONE"%d"YAHOO_URL_MIDDLE_TWO"%d"YAHOO_URL_END, gold_symbol_ch, (int)start_time, (int)end_time);

}

void FetchBullionData_Yahoo ( MemType *SilverOutput, MemType *GoldOutput ){
    char *SilverUrl, *GoldUrl;
    GetBullionUrl_Yahoo( &SilverUrl, &GoldUrl );

    //CURLM *mult_hnd = curl_multi_init();
    SetUpCurlHandle( Precious->Silver->YAHOO_hnd, Precious->multicurl_hnd, SilverUrl, SilverOutput );
    SetUpCurlHandle( Precious->Gold->YAHOO_hnd, Precious->multicurl_hnd, GoldUrl, GoldOutput );
    if ( PerformMultiCurl_no_prog( Precious->multicurl_hnd ) != 0 ) { free ( SilverUrl ); free ( GoldUrl ); return; }
    free ( SilverUrl );
    free ( GoldUrl );
}

void ParseBullionData_Yahoo (double *silver_f, double *gold_f){
    MemType SilverOutputStruct, GoldOutputStruct;

    FetchBullionData_Yahoo ( &SilverOutputStruct, &GoldOutputStruct );
    if ( *MetaData->multicurl_cancel_bool == true ){
        free( SilverOutputStruct.memory );
        free( GoldOutputStruct.memory );
        *silver_f = 0.0f;
        *gold_f = 0.0f;
        return;
    }

    /* Convert a String to a File Pointer Stream for Reading */
    FILE* fp = fmemopen( (void*)SilverOutputStruct.memory, strlen( SilverOutputStruct.memory ) + 1, "r" );
    
    char line[1024];
    char **csv_array;

    /* Yahoo! sometimes updates bullion when the equities markets are closed. 
       The while loop iterates to the end of file to get the latest data. */
    while (fgets( line, 1024, fp) != NULL);
    
    chomp( line );
    csv_array = parse_csv( line );
    *silver_f = strtod( csv_array[ 4 ] ? csv_array[ 4 ] : "0", NULL );
    free_csv_line( csv_array );
    fclose( fp );
    free( SilverOutputStruct.memory ); 

    /* Convert a String to a File Pointer Stream for Reading */
    fp = fmemopen( (void*)GoldOutputStruct.memory, strlen( GoldOutputStruct.memory ) + 1, "r" );
    
    while (fgets( line, 1024, fp) != NULL);
    
    chomp( line );
    csv_array = parse_csv( line );
    *gold_f = strtod( csv_array[ 4 ] ? csv_array[ 4 ] : "0", NULL );
    free_csv_line( csv_array );
    fclose( fp );
    free( GoldOutputStruct.memory ); 
}

void PopulateBullionPrice_Yahoo (){
    pthread_mutex_lock( &mutex_working [CLASS_MEMBER_MUTEX ] );

    ParseBullionData_Yahoo ( Precious->Silver->spot_price_f, Precious->Gold->spot_price_f );    

    /* Convert the double values into string values. */
    free( Precious->Gold->spot_price_ch );
    free( Precious->Silver->spot_price_ch );
    Precious->Gold->spot_price_ch = Precious->Gold->DoubToStr( Precious->Gold->spot_price_f );
    Precious->Silver->spot_price_ch = Precious->Silver->DoubToStr( Precious->Silver->spot_price_f );

    pthread_mutex_unlock( &mutex_working [CLASS_MEMBER_MUTEX ] );
}

void JSONProcessing () {
    pthread_mutex_lock( &mutex_working [CLASS_MEMBER_MUTEX ] );

    size_t len;

    for( unsigned short c = 0; c < Folder->size; c++ ) 
    /* Extract current price from JSON data for each Symbol. */
    { 
        if ( Folder->Equity[ c ]->JSON.memory ){
            /* Extract double values from JSON data using JSON-glib */
            JsonExtractEquity( Folder->Equity[ c ]->JSON.memory, Folder->Equity[ c ]->current_price_stock_f, Folder->Equity[ c ]->high_stock_f, Folder->Equity[ c ]->low_stock_f, Folder->Equity[ c ]->opening_stock_f, Folder->Equity[ c ]->prev_closing_stock_f, Folder->Equity[ c ]->change_share_f, Folder->Equity[ c ]->change_percent_f);

            /* Free memory. */
            free( Folder->Equity[ c ]->JSON.memory ); 
        }else{
            *Folder->Equity[ c ]->current_price_stock_f = 0.0;
            *Folder->Equity[ c ]->high_stock_f = 0.0;
            *Folder->Equity[ c ]->low_stock_f = 0.0;
            *Folder->Equity[ c ]->opening_stock_f = 0.0;
            *Folder->Equity[ c ]->prev_closing_stock_f = 0.0;
            *Folder->Equity[ c ]->change_share_f = 0.0;
            *Folder->Equity[ c ]->change_percent_f = 0.0;
        }

        /* Convert the double values into string values. */
        free( Folder->Equity[ c ]->current_price_stock_ch );
        Folder->Equity[ c ]->current_price_stock_ch = Folder->Equity[ c ]->DoubToStr( Folder->Equity[ c ]->current_price_stock_f );

        free( Folder->Equity[ c ]->high_stock_ch );
        Folder->Equity[ c ]->high_stock_ch = Folder->Equity[ c ]->DoubToStr( Folder->Equity[ c ]->high_stock_f ); 

        free( Folder->Equity[ c ]->low_stock_ch );
        Folder->Equity[ c ]->low_stock_ch = Folder->Equity[ c ]->DoubToStr( Folder->Equity[ c ]->low_stock_f ); 

        free( Folder->Equity[ c ]->opening_stock_ch );
        Folder->Equity[ c ]->opening_stock_ch = Folder->Equity[ c ]->DoubToStr( Folder->Equity[ c ]->opening_stock_f ); 

        free( Folder->Equity[ c ]->prev_closing_stock_ch );
        Folder->Equity[ c ]->prev_closing_stock_ch = Folder->Equity[ c ]->DoubToStr( Folder->Equity[ c ]->prev_closing_stock_f ); 

        free( Folder->Equity[ c ]->change_share_ch );
        Folder->Equity[ c ]->change_share_ch = Folder->Equity[ c ]->DoubToStr( Folder->Equity[ c ]->change_share_f );

        free( Folder->Equity[ c ]->change_percent_ch );
        len = strlen("###.###%%") + 1;
        Folder->Equity[ c ]->change_percent_ch = (char*) malloc ( len );
        snprintf( Folder->Equity[ c ]->change_percent_ch, len, "%.3lf%%", *Folder->Equity[ c ]->change_percent_f );
        
        /* Calculate the stock holdings change. */ 
        if( *Folder->Equity[ c ]->num_shares_stock_int > 0){
            *Folder->Equity[ c ]->change_value_f = *Folder->Equity[ c ]->change_share_f * (double)*Folder->Equity[ c ]->num_shares_stock_int;
        } else {
            *Folder->Equity[ c ]->change_value_f = 0.0;
        }

        free( Folder->Equity[ c ]->change_value_ch );
        Folder->Equity[ c ]->change_value_ch = Folder->Equity[ c ]->DoubToStr( Folder->Equity[ c ]->change_value_f );
    }
    pthread_mutex_unlock( &mutex_working [CLASS_MEMBER_MUTEX ] );
}

struct tm NYTimeComponents (){
    time_t currenttime;
    struct tm NY_tz;

    time( &currenttime );

    /* Get the localtime in NY from Epoch seconds */
    localtime_r( &currenttime, &NY_tz );
    return NY_tz;
}

unsigned int ClockSleepSeconds (){
    time_t currenttime;
    struct tm tm_struct;

    time( &currenttime );

    /* The timezone here isn't relevant. */
    localtime_r( &currenttime, &tm_struct );
    return ( 60 - tm_struct.tm_sec );
}

char* MonthNameStr ( int month ){
    switch ( month ){
        case 0:
            return "January";
            break;
        case 1:
            return "February";
            break;
        case 2:
            return "March";
            break;
        case 3:
            return "April";
            break;
        case 4:
            return "May";
            break;
        case 5:
            return "June";
            break;
        case 6:
            return "July";
            break;
        case 7:
            return "August";
            break;
        case 8:
            return "September";
            break;
        case 9:
            return "October";
            break;
        case 10:
            return "November";
            break;
        default:
            return "December";
            break;
    }
}

char* WeekDayStr ( int weekday ){
    switch ( weekday ){
        case 0:
            return "Sunday";
            break;
        case 1:
            return "Monday";
            break;
        case 2:
            return "Tuesday";
            break;
        case 3:
            return "Wednesday";
            break;
        case 4:
            return "Thursday";
            break;
        case 5:
            return "Friday";
            break;
        default:
            return "Saturday";
            break;
    }
}

void NYTime (int *ny_h, int *ny_m, int *ny_month, int *ny_day_month, int *ny_day_week, int *ny_year) {
    struct tm NY_tz;

    /* Get the NY time from Epoch seconds */
    NY_tz = NYTimeComponents ();

    *ny_h = (NY_tz.tm_hour)%24;
    *ny_m = NY_tz.tm_min;
    *ny_month = NY_tz.tm_mon;
    *ny_day_month = NY_tz.tm_mday;
    *ny_day_week = NY_tz.tm_wday;
    *ny_year = NY_tz.tm_year + 1900;  
}

void easter(int year, int *month, int *day)
/* Copied From: https://c-for-dummies.com/blog/?p=2446 by dgookin */
/* For any given year, will determine the month and day of Easter Sunday. 
   Month numbering starts at 1; March is 3, April is 4, etc. */
{
    int Y,a,c,e,h,k,L;
    double b,d,f,g,i,m;

    Y = year;
    a = Y%19;
    b = floor(Y/100);
    c = Y%100;
    d = floor(b/4);
    e = (int)b%4;
    f = floor((b+8)/25);
    g = floor((b-f+1)/3);
    h = (19*a+(int)b-(int)d-(int)g+15)%30;
    i = floor(c/4);
    k = c%4;
    L = (32+2*e+2*(int)i-h-k)%7;
    m = floor((a+11*h+22*L)/451);
    *month = (int)floor((h+L-7*m+114)/31);
    *day = (int)((h+L-7*(int)m+114)%31)+1;
}

char* WhichHoliday (struct tm NY_tz){
    /* New Years Day */
    if(NY_tz.tm_mon == 0 && NY_tz.tm_mday == 1 ) return "Market Closed - New Years Day";
    /* Presidents Day */
    if(NY_tz.tm_mon == 0 && NY_tz.tm_wday == 1 && NY_tz.tm_mday >= 15 && NY_tz.tm_mday <= 21) return "Market Closed - Presidents Day";
    /* Martin Luther King Jr. Day */
    if(NY_tz.tm_mon == 1 && NY_tz.tm_wday == 1 && NY_tz.tm_mday >= 15 && NY_tz.tm_mday <= 21) return "Market Closed - Martin Luther King Jr. Day";
    /* Memorial Day */
    if(NY_tz.tm_mon == 4 && NY_tz.tm_wday == 1 && NY_tz.tm_mday >= 25 && NY_tz.tm_mday <= 31) return "Market Closed - Memorial Day";
    /* Juneteenth Day */
    if(NY_tz.tm_mon == 5 && NY_tz.tm_mday == 19 ) return "Market Closed - Juneteenth Day";
    if(NY_tz.tm_mon == 5 && NY_tz.tm_wday == 1 && NY_tz.tm_mday >= 19 && NY_tz.tm_mday <= 21) return "Market Closed - Juneteenth Holiday";
    /* US Independence Day */
    if(NY_tz.tm_mon == 6 && NY_tz.tm_mday == 4 ) return "Market Closed - US Independence Day";
    if(NY_tz.tm_mon == 6 && NY_tz.tm_wday == 1 && NY_tz.tm_mday >= 4 && NY_tz.tm_mday <= 6) return "Market Closed - US Independence Holiday";
    /* Labor Day */
    if(NY_tz.tm_mon == 8 && NY_tz.tm_wday == 1 && NY_tz.tm_mday >= 1 && NY_tz.tm_mday <= 7) return "Market Closed - Labor Day";
    /* Thanksgiving Day */
    if(NY_tz.tm_mon == 10 && NY_tz.tm_wday == 4 && NY_tz.tm_mday >= 22 && NY_tz.tm_mday <= 28) return "Market Closed - Thanksgiving Day";
    /* Christmas Day */
    if(NY_tz.tm_mon == 11 && NY_tz.tm_mday == 25 ) return "Market Closed - Christmas Day";
    if(NY_tz.tm_mon == 11 && NY_tz.tm_wday == 1 && NY_tz.tm_mday >= 25 && NY_tz.tm_mday <= 27) return "Market Closed - Christmas Holiday";
    /* Good Friday */
    int month, day;
    easter( (int)NY_tz.tm_year, &month, &day ); /* Finds the date of easter for a given year. */
    if( ( (int)NY_tz.tm_mon + 1 ) == month && (int)NY_tz.tm_mday == ( day - 2 ) ) return "Market Closed - Good Friday";

    return "Not A Holiday - Error";
}

bool IsHoliday (struct tm NY_tz){
    /* New Years Day */
    if(NY_tz.tm_mon == 0 && NY_tz.tm_mday == 1 ) return true;
    /* Presidents Day */
    if(NY_tz.tm_mon == 0 && NY_tz.tm_wday == 1 && NY_tz.tm_mday >= 15 && NY_tz.tm_mday <= 21) return true;
    /* Martin Luther King Junior Day */
    if(NY_tz.tm_mon == 1 && NY_tz.tm_wday == 1 && NY_tz.tm_mday >= 15 && NY_tz.tm_mday <= 21) return true;
    /* Memorial Day */
    if(NY_tz.tm_mon == 4 && NY_tz.tm_wday == 1 && NY_tz.tm_mday >= 25 && NY_tz.tm_mday <= 31) return true;
    /* Juneteenth Day */
    if(NY_tz.tm_mon == 5 && NY_tz.tm_mday == 19 ) return true;
    if(NY_tz.tm_mon == 5 && NY_tz.tm_wday == 1 && NY_tz.tm_mday >= 19 && NY_tz.tm_mday <= 21) return true;
    /* US Independence Day */
    if(NY_tz.tm_mon == 6 && NY_tz.tm_mday == 4 ) return true;
    if(NY_tz.tm_mon == 6 && NY_tz.tm_wday == 1 && NY_tz.tm_mday >= 4 && NY_tz.tm_mday <= 6) return true;
    /* Labor Day */
    if(NY_tz.tm_mon == 8 && NY_tz.tm_wday == 1 && NY_tz.tm_mday >= 1 && NY_tz.tm_mday <= 7) return true;
    /* Thanksgiving Day */
    if(NY_tz.tm_mon == 10 && NY_tz.tm_wday == 4 && NY_tz.tm_mday >= 22 && NY_tz.tm_mday <= 28) return true;
    /* Christmas Day */
    if(NY_tz.tm_mon == 11 && NY_tz.tm_mday == 25 ) return true;
    if(NY_tz.tm_mon == 11 && NY_tz.tm_wday == 1 && NY_tz.tm_mday >= 25 && NY_tz.tm_mday <= 27) return true;
    /* Good Friday */
    int month, day;
    easter( (int)NY_tz.tm_year, &month, &day ); /* Finds the date of easter for a given year. */
    if( ( (int)NY_tz.tm_mon + 1 ) == month && (int)NY_tz.tm_mday == ( day - 2 ) ) return true;

    return false;
}

bool TimeToClose (int *h_r, int *m_r, int *s_r) {
    /* The NYSE/NASDAQ Markets are open from 09:30 to 16:00 EST. */
    struct tm NY_tz;
    bool closed;
    
    /* Get the localtime in NY from Epoch seconds */
    /* Accounts if it is DST or STD time in NY. */
    NY_tz = NYTimeComponents ();

    int hour = (NY_tz.tm_hour)%24;
    int min = NY_tz.tm_min;
    int sec = NY_tz.tm_sec;
    int weekday = NY_tz.tm_wday;

    int hour_open_NY, hour_closed_NY;    

    hour_open_NY = 9;
    hour_closed_NY = 16;
    
    if( *MetaData->holiday_bool || hour < hour_open_NY || ( hour == hour_open_NY && min < 30 ) || hour >= hour_closed_NY || weekday == 0 || weekday == 6 ){
        *h_r = 0;
        *m_r = 0;
        *s_r = 0;
        closed = true;
    } else {
        *h_r = hour_closed_NY - 1 - hour;
        *m_r = 59 - min;
        *s_r = 59 - sec;
        closed = false;
    }
    return closed;
}

unsigned int SecondsToOpen () {
    /* The seconds until the NYSE/NASDAQ markets open, does not account
    for holidays. */
    struct tm NY_tz;
    time_t currenttime, futuretime;
    unsigned int diff;

    /* The current time in Epoch seconds */
    time( &currenttime );

    /* Get the localtime in NY from Epoch seconds 
       We need to do this to account for DST changes when
       determining the future Epoch time. 
    */

    NY_tz = NYTimeComponents ();
    

    int hour = (NY_tz.tm_hour)%24;
    int min = NY_tz.tm_min;
    int weekday = NY_tz.tm_wday;
    int hour_open_NY, hour_closed_NY;

    hour_open_NY = 9;
    hour_closed_NY = 16;

    if( ( hour > hour_open_NY || ( hour == hour_open_NY && min >= 30 ) ) && hour < hour_closed_NY && weekday != 0 && weekday != 6 ){

        /* The market is open. */
        return 0;
    }
    
    /* Today is not Fri and today is not Sat and today is not Sun and it is past 4 PM EST/EDST */
    if( weekday != 5 && weekday != 6 && weekday != 0 && hour >= hour_closed_NY ){
    	NY_tz.tm_mday++;
    }
    
    /* Today is Sun */
    if( weekday == 0 ) {
    	NY_tz.tm_mday++;
    }
    
    /* Today is Sat */
    if( weekday == 6 ) {
    	NY_tz.tm_mday += 2;
    }

    /* Today is Fri and it is past 4 PM EST/EDST */
    if( weekday == 5 && hour >= hour_closed_NY ) {
    	NY_tz.tm_mday += 3;
    }
     
    /* The market always opens at 09:30:00 EST/EDST */
    NY_tz.tm_hour = hour_open_NY;
    NY_tz.tm_min = 30;
    NY_tz.tm_sec = 0;
    
    /* The future NY EST/EDST date to time in Epoch seconds. */
    futuretime = mktime( &NY_tz );
    
    /* The market is closed, reopens in this many seconds. */
    diff = (unsigned int) difftime( futuretime, currenttime );

    return ( diff );
}

double calc_gain ( double cur_price, double prev_price ){
	return ( 100 * ( ( cur_price - prev_price ) / prev_price ) );
}

void summation (double current_gain, double *avg_gain, double *avg_loss){
	if( current_gain >= 0 ){
		*avg_gain += current_gain;
	} else {
		*avg_loss += -1 * current_gain;
	}
}

void calc_avg (double current_gain, double *avg_gain, double *avg_loss){
	if( current_gain >= 0 ){
		*avg_gain = ( ( *avg_gain * 13 ) + current_gain ) / 14;
		*avg_loss = ( ( *avg_loss * 13 ) + 0 ) / 14;
	} else {
		*avg_gain = ( ( *avg_gain * 13 ) + 0 ) / 14;
		*avg_loss = ( ( *avg_loss * 13 ) + ( -1 * current_gain ) ) / 14;
	}
}

double calc_rsi ( double avg_gain, double avg_loss ){
	double rs = avg_gain / avg_loss;
	return ( 100 - ( 100 / ( 1 + rs ) ) );
}

char *rsi_indicator( double rsi )
/* Return an indicator string. Do not free return value (faster using stack memory). */
{
    if ( rsi >= 70 ) {
        return "Overbought";
	} else if ( rsi >= 60 && rsi < 70 ) {
        return "Overbought Watch";
	} else if ( rsi > 40 && rsi < 60 ) {
        return "Neutral";
	} else if ( rsi > 30 && rsi <= 40 ) {
        return "Oversold Watch";
	} else {
        return "Oversold";
	}
}

void symbol_security_name_map_destruct ( symbol_name_map* sn_map ) {
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

void RSIPeriod(time_t *currenttime, time_t *starttime){
    
    /* Number of Seconds in a Year Plus Three Weeks */
    int period = 31557600 + ( 604800 * 3 ); 

    time( currenttime );
    *starttime = *currenttime - (time_t)period;

}

char *RSIGetURL ( const char *symbol ){
    time_t start, end;
    size_t len;
    
    RSIPeriod( &end, &start );

    len = strlen( symbol ) + strlen( YAHOO_URL_START ) + strlen( YAHOO_URL_MIDDLE_ONE ) + strlen( YAHOO_URL_MIDDLE_TWO ) + strlen( YAHOO_URL_END ) + 25;
    char *url = malloc ( len );
    snprintf(url, len, YAHOO_URL_START"%s"YAHOO_URL_MIDDLE_ONE"%d"YAHOO_URL_MIDDLE_TWO"%d"YAHOO_URL_END, symbol, (int)start, (int)end);

    return url;
}

MemType *RSIMulticurlProcessing (const char *symbol){
    char *MyUrl=NULL;
    MyUrl = RSIGetURL ( symbol );

    MemType *MyOutputStruct = (MemType*)malloc( sizeof(*MyOutputStruct) );

    SetUpCurlHandle( MetaData->rsi_hnd, MetaData->multicurl_rsi_hnd, MyUrl, MyOutputStruct );
    if ( PerformMultiCurl_no_prog( MetaData->multicurl_rsi_hnd ) != 0 ) { free ( MyUrl ); return NULL; }
    free ( MyUrl );

    return MyOutputStruct;
}

int symsearchfunc(const void * a, const void * b){
    /* Cast the void pointer to a char pointer. */
   char* aa = (char *)a;
   /* Cast the void pointer to a double struct pointer. */
   symbol_to_security_name_container** bb = (symbol_to_security_name_container **)b;

   return strcasecmp( aa, bb[0]->symbol ); 
}

char *GetSecurityNameFromMapping(const char *s, symbol_name_map* sn_map)
/* Look for a Security Name using the Symbol as a key value.
   Must free return value. */
{
    symbol_to_security_name_container **item = NULL;

    /* The second parameter is the same type as the return value. A double pointer to a struct. */
    /* It's basically searching through an array of pointer addresses, the compare function dereferences
       the pointer address to get the string we are comparing against. */
    /* The array must already be sorted for bsearch to work. */
    item = (symbol_to_security_name_container**) bsearch (s, &sn_map->sn_container_arr[0], sn_map->size, sizeof (symbol_to_security_name_container*), symsearchfunc);

    if ( item != NULL ){
        /* The item pointer is not freed. It points to an item in the 
           sn_container_arr array and not a duplicate. */
        return strdup( item[ 0 ]->security_name );
    } else {
        return NULL;
    }
}

symbol_name_map *CompletionSymbolFetch ()
/* This function is only meant to be run once, at application startup. */
{
    
    char *tofree;
    char *line = malloc ( 1024 ), *line_start;
	char *token;
	char *output, *tmp, *original;
	short firstline = 1;
	line_start = line;

    symbol_name_map *sn_map = (symbol_name_map*) malloc ( sizeof(*sn_map) );
    sn_map->sn_container_arr = malloc ( 1 );
    sn_map->size = 0;
    symbol_to_security_name_container **sec_sym_tmp = NULL;

    char* Nasdaq_Url = NASDAQ_SYMBOL_URL; 
    char* NYSE_Url = NYSE_SYMBOL_URL; 
   	MemType Nasdaq_Struct, NYSE_Struct;
    SetUpCurlHandle( MetaData->NASDAQ_completion_hnd, MetaData->multicurl_cmpltn_hnd, Nasdaq_Url, &Nasdaq_Struct );
    SetUpCurlHandle( MetaData->NYSE_completion_hnd, MetaData->multicurl_cmpltn_hnd, NYSE_Url, &NYSE_Struct );
    if ( PerformMultiCurl_no_prog( MetaData->multicurl_cmpltn_hnd ) != 0 || *MetaData->multicurl_cancel_bool == true) { 
        if ( Nasdaq_Struct.memory ) free( Nasdaq_Struct.memory );
        if ( NYSE_Struct.memory ) free( NYSE_Struct.memory );       
        if ( line ) free( line );
        return sn_map; 
    }

    /* Convert a String to a File Pointer Stream for Reading */
    FILE* fp[2];
    fp[0] = fmemopen( (void*)Nasdaq_Struct.memory, strlen( Nasdaq_Struct.memory ) + 1, "r" );
    fp[1] = fmemopen( (void*)NYSE_Struct.memory, strlen( NYSE_Struct.memory ) + 1, "r" );
    
    short k = 0;
	while( k < 2 ){
        /* Initialize the output string handle. */
        output = malloc ( 1 );

	    /* Get rid of the header line from the file stream */
	    if ( fgets( line, 1024, fp[k] ) == NULL ) {
                if ( line ) free( line );
            	if ( output ) free( output );
            	if ( fp[0] ) fclose( fp[0] ); 
                if ( fp[1] ) fclose( fp[1] );
                if ( Nasdaq_Struct.memory ) free( Nasdaq_Struct.memory );
                if ( NYSE_Struct.memory ) free( NYSE_Struct.memory );
            	return sn_map;
        }

        /* Read the file stream one line at a time */
        while ( fgets( line, 1024, fp[k] ) != NULL ) {
            
            /* Extract the symbol from the line. */
           	if( ( token = strsep( &line, "|" ) ) != NULL ){
                if ( check_symbol( token ) == false ){ line = line_start; continue; }

           		original = strdup ( output );
           		tmp = realloc( output, strlen ( token ) + strlen ( output ) + 2 );
           		output = tmp;

           		if ( firstline ) {
           			snprintf( output, strlen ( token ) + 2, "%s", token );
           			firstline = 0;
           		} else {
           			snprintf( output, strlen ( token ) + strlen ( output ) + 2, "%s|%s", original, token );
           		}

           		free ( original );
           	}

            /* Extract the security name from the line. */
           	if( ( token = strsep( &line, "|" ) ) != NULL ){
           		original = strdup ( output );
           		tmp = realloc( output, strlen ( token ) + strlen ( output ) + 2 );
           		output = tmp;

           		snprintf( output, strlen ( token ) + strlen ( output ) + 2, "%s|%s", original, token );

           		free ( original );
                /* strsep moves the line pointer, we need to reset it so the pointer memory can be reused */
                line = line_start;
           	}

            /* If we are exiting the application, return immediately. */
            if ( *MetaData->multicurl_cancel_bool == true ) {
                if ( fp[0] ) fclose( fp[0] ); 
                if ( fp[1] ) fclose( fp[1] );  
                if ( Nasdaq_Struct.memory ) free( Nasdaq_Struct.memory );
                if ( NYSE_Struct.memory ) free( NYSE_Struct.memory );
                if ( output ) free( output );
                if ( line_start ) free( line_start );
                return sn_map;
            }
        }
        /* We aren't sure of the current location of the line pointer 
           [the server's data places arbitrary separators at the end].
           so we reset the line pointer before reading a second file. */
        line = line_start;
        firstline = 1;  

        /* Truncate the last three entries from the output string */
        tmp = strrchr( output, '|' );
        *tmp = 0;
        tmp = strrchr( output, '|' );
        *tmp = 0;
        tmp = strrchr( output, '|' );
        *tmp = 0;

        /* Populate the Security Symbol Array. The second list is concatenated after the first list. */
        tofree = output;
        bool symbol = true;
        while ( ( token = strsep( &output, "|" ) ) != NULL ) {
            if( symbol ){
                /* Add another pointer address to the array */
                sec_sym_tmp = realloc( sn_map->sn_container_arr, sizeof(symbol_to_security_name_container*) * (sn_map->size + 1) );
                sn_map->sn_container_arr = sec_sym_tmp;
                /* Allocate memory for that pointer address */
                sn_map->sn_container_arr[ sn_map->size ] = malloc( sizeof(symbol_to_security_name_container) );
                /* Populate the memory with the character string */
                sn_map->sn_container_arr[ sn_map->size ]->symbol = strdup( token );
                symbol = false;

            } else {
                /* Populate the memory with the character string and increment the symbolcount */
                sn_map->sn_container_arr[ sn_map->size++ ]->security_name = strdup( token );
                symbol = true;

            }

            /* If we are exiting the application, return immediately. */
            if ( *MetaData->multicurl_cancel_bool == true ) { 
                if ( fp[0] ) fclose( fp[0] ); 
                if ( fp[1] ) fclose( fp[1] );
                if ( Nasdaq_Struct.memory ) free( Nasdaq_Struct.memory );
                if ( NYSE_Struct.memory ) free( NYSE_Struct.memory );
                if ( tofree ) free( tofree );
                if ( line ) free( line ); 
                return sn_map;
            }
        }

        /* Free the output string, so the handle can be reused on the second list. */
        free( tofree );
        k++;

    } /* end while loop */
    free( line );

    /* Sort the security symbol array, this merges both lists into one sorted list. */
    qsort( &sn_map->sn_container_arr[ 0 ], sn_map->size, sizeof(symbol_to_security_name_container*), AlphaAscSecName );    

    fclose( fp[0] ); 
    fclose( fp[1] );
    free( Nasdaq_Struct.memory );
    free( NYSE_Struct.memory );
    return sn_map;
}

void stop_multicurl ()
/* Removing the easy handle from the multihandle will stop the cURL data transfer
   immediately. curl_multi_remove_handle does nothing if the easy handle is not
   currently set in the multihandle. */
{
    /* Equity Multicurl Operation */
    curl_multi_wakeup( Folder->multicurl_hnd );
    pthread_mutex_lock( &mutex_working[ MULTICURL_PROG_MUTEX ] );

    for(unsigned short i=0; i<Folder->size; i++){
        curl_multi_remove_handle( Folder->multicurl_hnd, Folder->Equity[ i ]->easy_hnd );
        
    }

    pthread_mutex_unlock( &mutex_working[ MULTICURL_PROG_MUTEX ] );

    /* Bullion Multicurl Operation */
    curl_multi_wakeup( Precious->multicurl_hnd );
    pthread_mutex_lock( &mutex_working[ MULTICURL_NO_PROG_MUTEX ] );

    curl_multi_remove_handle( Precious->multicurl_hnd, Precious->Gold->YAHOO_hnd );
    curl_multi_remove_handle( Precious->multicurl_hnd, Precious->Silver->YAHOO_hnd );

    pthread_mutex_unlock( &mutex_working[ MULTICURL_NO_PROG_MUTEX ] );

    /* RSI Completion Fetch Multicurl Operation */
    curl_multi_wakeup( MetaData->multicurl_cmpltn_hnd );
    pthread_mutex_lock( &mutex_working[ MULTICURL_NO_PROG_MUTEX ] );

    curl_multi_remove_handle( MetaData->multicurl_cmpltn_hnd, MetaData->NASDAQ_completion_hnd );
    curl_multi_remove_handle( MetaData->multicurl_cmpltn_hnd, MetaData->NYSE_completion_hnd );

    pthread_mutex_unlock( &mutex_working[ MULTICURL_NO_PROG_MUTEX ] );

    /* RSI Data Multicurl Operation */
    curl_multi_wakeup( MetaData->multicurl_rsi_hnd );
    pthread_mutex_lock( &mutex_working[ MULTICURL_NO_PROG_MUTEX ] );

    curl_multi_remove_handle( MetaData->multicurl_rsi_hnd, MetaData->rsi_hnd );

    pthread_mutex_unlock( &mutex_working[ MULTICURL_NO_PROG_MUTEX ] );
}