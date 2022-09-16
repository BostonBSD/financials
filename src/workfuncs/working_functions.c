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
        /* If there are no stocks in the config file, 
        we need to keep freeing this to prevent a memory 
        leak on subsequent file reads. */
        free( F->Equity ); 
        stock** temp = (stock**) malloc( sizeof(stock*) );
                
        if (temp == NULL){
            printf("Not Enough Memory, realloc returned NULL.\n");
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
            printf("Not Enough Memory, realloc returned NULL.\n");
            exit(EXIT_FAILURE);
        }
                
        F->Equity = temp;
        F->size = 0;
    }
}

void PerformCalculations () 
/* calc portfolio values and populate character strings. */
{
    pthread_mutex_lock( &mutex_working[0] );

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

    /* Edit the next line as needed. */
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

    pthread_mutex_unlock( &mutex_working[0] );
}

int MultiCurlProcessing () {
    size_t len;

    /* Create a multi-cURL handle. */
    CURLM *mult_hnd = SetUpMultiCurlHandle ();
 
    /* Cycle through the list of equities. */
    for( unsigned short c = 0; c < Folder->size; c++ ) { 
        /* Generate the request URL for this equity. */
        free( Folder->Equity[ c ]->curl_url_stock_ch );

        len = strlen(MetaData->stock_url) + strlen(Folder->Equity[ c ]->symbol_stock_ch) + strlen(MetaData->curl_key)+1;
        Folder->Equity[ c ]->curl_url_stock_ch = (char*) malloc( len );
        snprintf( Folder->Equity[ c ]->curl_url_stock_ch, len, "%s%s%s", MetaData->stock_url, Folder->Equity[ c ]->symbol_stock_ch, MetaData->curl_key );
        
        /* Create and add a cURL easy handle to the multi-cURL handle 
        (passing JSON output struct by reference) */
        SetUpCurlHandle( mult_hnd, Folder->Equity[ c ]->curl_url_stock_ch, &Folder->Equity[ c ]->JSON );
    }

    /* Perform the cURL requests simultaneously using multi-cURL. */
    return PerformMultiCurl( mult_hnd, (double)Folder->size );
}

void GetBullionUrl_Yahoo ( char** SilverUrl, char** GoldUrl ){
    time_t currenttime;
    size_t len;

    time( &currenttime );
    char *silver_symbol_ch = "SI=F";
    char *gold_symbol_ch = "GC=F";

    len = strlen( silver_symbol_ch ) + strlen( YAHOO_URL_START ) + strlen( YAHOO_URL_MIDDLE_ONE ) + strlen( YAHOO_URL_MIDDLE_TWO ) + strlen( YAHOO_URL_END ) + 25;
    SilverUrl[0] = malloc ( len );
    snprintf(SilverUrl[0], len, YAHOO_URL_START"%s"YAHOO_URL_MIDDLE_ONE"%d"YAHOO_URL_MIDDLE_TWO"%d"YAHOO_URL_END, silver_symbol_ch, (int)currenttime, (int)currenttime);

    len = strlen( gold_symbol_ch ) + strlen( YAHOO_URL_START ) + strlen( YAHOO_URL_MIDDLE_ONE ) + strlen( YAHOO_URL_MIDDLE_TWO ) + strlen( YAHOO_URL_END ) + 25;
    GoldUrl[0] = malloc ( len );
    snprintf(GoldUrl[0], len, YAHOO_URL_START"%s"YAHOO_URL_MIDDLE_ONE"%d"YAHOO_URL_MIDDLE_TWO"%d"YAHOO_URL_END, gold_symbol_ch, (int)currenttime, (int)currenttime);
}

void FetchBullionData_Yahoo ( MemType *SilverOutput, MemType *GoldOutput ){
    char *SilverUrl, *GoldUrl;
    GetBullionUrl_Yahoo( &SilverUrl, &GoldUrl );

    CURLM *mult_hnd = SetUpMultiCurlHandle();
    SetUpCurlHandle( mult_hnd, SilverUrl, SilverOutput );
    SetUpCurlHandle( mult_hnd, GoldUrl, GoldOutput );
    if ( PerformMultiCurl_no_prog( mult_hnd ) != 0 ) { free ( SilverUrl ); free ( GoldUrl ); return; }
    free ( SilverUrl );
    free ( GoldUrl );
}

void ParseBullionData_Yahoo (double *silver_f, double *gold_f){
    MemType SilverOutputStruct, GoldOutputStruct;

    FetchBullionData_Yahoo ( &SilverOutputStruct, &GoldOutputStruct );

    /* Convert a String to a File Pointer Stream for Reading */
    FILE* fp = fmemopen( (void*)SilverOutputStruct.memory, strlen( SilverOutputStruct.memory ) + 1, "r" );
    
    char line[1024];
    char **csv_array;
    
    /* Get rid of the header line from the file stream */
    if (fgets( line, 1024, fp) == NULL) { fclose( fp ); return; }
    
    /* Get the current spot price */
    if (fgets( line, 1024, fp) == NULL) { fclose( fp ); return; }
    
    chomp( line );
    csv_array = parse_csv( line );
    *silver_f = strtod( csv_array[ 4 ], NULL );
    free_csv_line( csv_array );
    fclose( fp );
    free( SilverOutputStruct.memory ); 

    /* Convert a String to a File Pointer Stream for Reading */
    fp = fmemopen( (void*)GoldOutputStruct.memory, strlen( GoldOutputStruct.memory ) + 1, "r" );
    
    /* Get rid of the header line from the file stream */
    if (fgets( line, 1024, fp) == NULL) { fclose( fp ); return; }
    
    /* Get the current spot price */
    if (fgets( line, 1024, fp) == NULL) { fclose( fp ); return; }
    
    chomp( line );
    csv_array = parse_csv( line );
    *gold_f = strtod( csv_array[ 4 ], NULL );
    free_csv_line( csv_array );
    fclose( fp );
    free( GoldOutputStruct.memory ); 
}

void PopulateBullionPrice_Yahoo (){
    pthread_mutex_lock( &mutex_working[0] );

    ParseBullionData_Yahoo ( Precious->Silver->spot_price_f, Precious->Gold->spot_price_f );    

    /* Convert the double values into string values. */
    free( Precious->Gold->spot_price_ch );
    free( Precious->Silver->spot_price_ch );
    Precious->Gold->spot_price_ch = Precious->Gold->DoubToStr( Precious->Gold->spot_price_f );
    Precious->Silver->spot_price_ch = Precious->Silver->DoubToStr( Precious->Silver->spot_price_f );

    pthread_mutex_unlock( &mutex_working[0] );
}

void JSONProcessing () {
    pthread_mutex_lock( &mutex_working[0] );

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

        pthread_mutex_unlock( &mutex_working[0] );
    }
}

struct tm NYTimeComponents(){
    time_t currenttime;
    struct tm NY_tz;

    time( &currenttime );

    /* Get the localtime in NY from Epoch seconds */
    /* From the getenv() man page: The application should not modify the string pointed to
       by the getenv() function.  So we do not free this string. */
    char *tz_var = getenv( "TZ" );
    setenv("TZ", "America/New_York", 1);
    tzset();
    localtime_r( &currenttime, &NY_tz );

    /* Reset the TZ env variable. */
    tz_var ? setenv("TZ", tz_var, 1) : unsetenv( "TZ" );
    tzset();

    return NY_tz;
}

unsigned int ClockSleepSeconds (){
    time_t currenttime;
    struct tm Local_tz;

    time( &currenttime );
    localtime_r( &currenttime, &Local_tz );
    return ( 59 - Local_tz.tm_sec );
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
        case 11:
            return "December";
            break;
        default:
            return "Smarch";
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
        case 6:
            return "Saturday";
            break;
        default:
            return "The eighth day of the week";
            break;
    }
}

void LocalAndNYTime (int *h, int *m, int *month, int *day_month, int *day_week, int *year, int *ny_h, int *ny_m, int *ny_month, int *ny_day_month, int *ny_day_week, int *ny_year) {
    time_t currenttime;
    struct tm Local_NY_tz;

    time( &currenttime );

    /* Get the localtime and NY time from Epoch seconds */
    localtime_r( &currenttime, &Local_NY_tz );

    *h = (Local_NY_tz.tm_hour)%24;
    *m = Local_NY_tz.tm_min;
    *month = Local_NY_tz.tm_mon;
    *day_month = Local_NY_tz.tm_mday;
    *day_week = Local_NY_tz.tm_wday;
    *year = Local_NY_tz.tm_year + 1900;
   
    Local_NY_tz = NYTimeComponents ();

    *ny_h = (Local_NY_tz.tm_hour)%24;
    *ny_m = Local_NY_tz.tm_min;
    *ny_month = Local_NY_tz.tm_mon;
    *ny_day_month = Local_NY_tz.tm_mday;
    *ny_day_week = Local_NY_tz.tm_wday;
    *ny_year = Local_NY_tz.tm_year + 1900;  
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
    if(NY_tz.tm_mon == 5 && NY_tz.tm_wday == 1 && NY_tz.tm_mday >= 19 && NY_tz.tm_mday <= 21) return true;
    if(NY_tz.tm_mon == 5 && NY_tz.tm_mday == 19 ) return true;
    /* US Independence Day */
    if(NY_tz.tm_mon == 6 && NY_tz.tm_wday == 1 && NY_tz.tm_mday >= 4 && NY_tz.tm_mday <= 6) return true;
    if(NY_tz.tm_mon == 6 && NY_tz.tm_mday == 4 ) return true;
    /* Labor Day */
    if(NY_tz.tm_mon == 8 && NY_tz.tm_wday == 1 && NY_tz.tm_mday >= 1 && NY_tz.tm_mday <= 7) return true;
    /* Thanksgiving Day */
    if(NY_tz.tm_mon == 10 && NY_tz.tm_wday == 4 && NY_tz.tm_mday >= 22 && NY_tz.tm_mday <= 28) return true;
    /* Christmas Day */
    if(NY_tz.tm_mon == 11 && NY_tz.tm_wday == 1 && NY_tz.tm_mday >= 25 && NY_tz.tm_mday <= 27) return true;
    if(NY_tz.tm_mon == 11 && NY_tz.tm_mday == 25 ) return true;
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
    
    if( hour < hour_open_NY || ( hour == hour_open_NY && min < 30 ) || hour >= hour_closed_NY || weekday == 0 || weekday == 6 ){
        *h_r = 0;
        *m_r = 0;
        *s_r = 0;
        closed = true;
    } else {
        *h_r = hour_closed_NY - 1 - hour;
        *m_r = 59 - min;
        *s_r = 59 - sec;
        closed = false;

        /* Check if it is a holiday in NY. */
        if( IsHoliday( NY_tz ) ) {
            *h_r = 0;
            *m_r = 0;
            *s_r = 0;
            closed = true;
        }
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
       determining the future Epoch time. */
    
    char *tz_var = getenv( "TZ" );
    setenv("TZ", "America/New_York", 1);
    tzset();
    localtime_r( &currenttime, &NY_tz );
    

    int hour = (NY_tz.tm_hour)%24;
    int min = NY_tz.tm_min;
    int weekday = NY_tz.tm_wday;
    int hour_open_NY, hour_closed_NY;

    hour_open_NY = 9;
    hour_closed_NY = 16;

    if( ( hour > hour_open_NY || ( hour == hour_open_NY && min >= 30 ) ) && hour < hour_closed_NY && weekday != 0 && weekday != 6 ){
        /* Reset the TZ env variable. */
        tz_var ? setenv("TZ", tz_var, 1) : unsetenv( "TZ" );
        tzset();

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

    /* Reset the TZ env variable. */
    tz_var ? setenv("TZ", tz_var, 1) : unsetenv( "TZ" );
    tzset();
    
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
    char *poss_sell = "Possible Sell";
    char *sell_watch = "Sell Watch";
    char *neutral = "Neutral";
    char *purch_watch = "Purchase Watch";
    char *poss_purch = "Possible Purchase";

    if ( rsi >= 70 ) {
        return poss_sell;
	} else if ( rsi >= 60 && rsi < 70 ) {
        return sell_watch;
	} else if ( rsi > 40 && rsi < 60 ) {
        return neutral;
	} else if ( rsi > 30 && rsi <= 40 ) {
        return purch_watch;
	} else {
        return poss_purch;
	}
}