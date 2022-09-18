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

#include <monetary.h>
#include <locale.h>

#include "../financials.h"


/* Class Method (also called Function) Definitions */
static double EquityStake (const unsigned int *shares, const double *price) {
    return ( (double)(*shares) * (*price) );
}

static double BullionPremStake (const double *ounces, const double *prem, const double *price) {
    return ( ((*prem) + (*price)) * (*ounces) );
}

static double BullionPortfolio (const double *gold_stake, const double *silver_stake) {
    return ( (*gold_stake) + (*silver_stake) );
}

static double EntirePortfolio (const double *bullion, const double *equity, const double *cash) {
    return ( (*bullion) + (*equity) + (*cash) );
}

static char* DoubleToString (const double *num) 
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

static double StringToDouble (const char *str) {
    char *newstr = (char*) malloc( strlen( str )+1 );
    strcpy( newstr, str );

    FormatStr( newstr );
    double num = strtod( newstr, NULL );

    free( newstr );

    return num;
}

/* Class Init Functions */
equity_folder* class_init_equity_folder ()
{
    /* Allocate Memory For A New Class Handle */
    equity_folder* new_class_handle = (equity_folder*) malloc( sizeof(*new_class_handle) );
    assert( new_class_handle != NULL );

    new_class_handle->Equity = NULL;
    new_class_handle->size = 0;

    /* Return Our Initialized Class Handle */
    return new_class_handle; 
}

metal* class_init_metal ()
{
    /* Allocate Memory For A New Class Handle */
    metal* new_class_handle = (metal*) malloc( sizeof(*new_class_handle) );
    assert( new_class_handle != NULL );

    /* Initialize Class Objects */
    new_class_handle->Gold = class_init_bullion ();
    new_class_handle->Silver = class_init_bullion ();

    /* Return Our Initialized Class Handle */
    return new_class_handle; 
}

bullion* class_init_bullion ()
{ 
    /* Allocate Memory For A New Class Object */
    bullion* new_class = (bullion*) malloc( sizeof(*new_class) ); 
    assert( new_class != NULL );

    /* Allocate Memory For Variables */
    new_class->spot_price_f = (double*) malloc( sizeof(double) );
    new_class->premium_f = (double*) malloc( sizeof(double) );
    new_class->port_value_f = (double*) malloc( sizeof(double) );
    new_class->ounce_f = (double*) malloc( sizeof(double) );

    new_class->spot_price_ch = (char*) malloc( strlen("$0.00")+1 );            
    new_class->premium_ch = (char*) malloc( strlen("$0.00")+1 ); 
    new_class->port_value_ch = (char*) malloc( strlen("$0.00")+1 );            

    new_class->ounce_ch = (char*) malloc( strlen("0.000")+1 );                              
    
    /* Initialize Variables */
    strcpy( new_class->spot_price_ch,"$0.00" );
    strcpy( new_class->premium_ch,"$0.00" );
    strcpy( new_class->port_value_ch,"$0.00" );

    strcpy( new_class->ounce_ch,"0.000" );

    *new_class->ounce_f = 0.0;
    *new_class->spot_price_f = 0.0;
    *new_class->premium_f = 0.0;
    *new_class->port_value_f = 0.0;

    /* Connect Function Pointers To Function Definitions */
    new_class->Stake = BullionPremStake;
    new_class->DoubToStr = DoubleToString;
    new_class->StrToDoub = StringToDouble;
    
    /* Return Our Initialized Class */
    return new_class; 
}

stock* class_init_equity ()
{ 
    /* Allocate Memory For A New Class Object */
    stock* new_class = (stock*) malloc( sizeof(*new_class) );
    assert( new_class != NULL ); 
	
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

    /* Connect Function Pointers To Function Definitions */
    new_class->Stake = EquityStake;
    new_class->DoubToStr = DoubleToString;
    new_class->StrToDoub = StringToDouble;

    /* Return Our Initialized Class */
    return new_class; 
}

meta* class_init_meta_data ()
{ 
    /* Allocate Memory For A New Class Object */
    meta* new_class = (meta*) malloc( sizeof(*new_class) );
    assert( new_class != NULL );

    /* Allocate Memory For Variables */
    new_class->cash_f = (double*) malloc( sizeof(double) );
    new_class->bullion_port_value_f = (double*) malloc( sizeof(double) );
    new_class->stock_port_value_f = (double*) malloc( sizeof(double) );
    new_class->stock_port_value_chg_f = (double*) malloc( sizeof(double) );
    new_class->stock_port_value_p_chg_f = (double*) malloc( sizeof(double) );
    new_class->portfolio_port_value_f = (double*) malloc( sizeof(double) );
    new_class->portfolio_port_value_chg_f = (double*) malloc( sizeof(double) );
    new_class->portfolio_port_value_p_chg_f = (double*) malloc( sizeof(double) );
    new_class->updates_per_min_f = (double*) malloc( sizeof(double) );
    new_class->updates_hours_f = (double*) malloc( sizeof(double) );

    new_class->stock_url = (char*) malloc( strlen( FINNHUB_URL )+1 );
	new_class->curl_key = (char*) malloc( strlen( FINNHUB_URL_TOKEN )+1 );                 

    new_class->cash_ch = (char*) malloc( strlen("$0.00")+1 );                  
    new_class->bullion_port_value_ch = (char*) malloc( strlen("$0.00")+1 );    
    new_class->stock_port_value_ch = (char*) malloc( strlen("$0.00")+1 );   
    new_class->stock_port_value_chg_ch = (char*) malloc( strlen("$0.00")+1 ); 
    new_class->stock_port_value_p_chg_ch = (char*) malloc( strlen("000.000%%")+1 );    
    new_class->portfolio_port_value_ch = (char*) malloc( strlen("$0.00")+1 ); 
    new_class->portfolio_port_value_chg_ch = (char*) malloc( strlen("$0.00")+1 ); 
    new_class->portfolio_port_value_p_chg_ch = (char*) malloc( strlen("000.000%%")+1 ); 

    new_class->fetching_data_bool = (bool*) malloc( sizeof(bool) );
    new_class->holiday_bool = (bool*) malloc( sizeof(bool) );

    /* Initialize Variables */
    strcpy( new_class->stock_url, FINNHUB_URL );
    strcpy( new_class->curl_key, FINNHUB_URL_TOKEN );
    strcpy( new_class->cash_ch,"$0.00" );
    strcpy( new_class->bullion_port_value_ch,"$0.00" );
    strcpy( new_class->stock_port_value_ch,"$0.00" );
    strcpy( new_class->stock_port_value_chg_ch,"$0.00" );
    strcpy( new_class->stock_port_value_p_chg_ch,"000.000%%" );
    strcpy( new_class->portfolio_port_value_ch,"$0.00" );
    strcpy( new_class->portfolio_port_value_chg_ch,"$0.00" );
    strcpy( new_class->portfolio_port_value_p_chg_ch,"000.000%%" );

    *new_class->cash_f = 0.0;
    *new_class->bullion_port_value_f = 0.0;
    *new_class->stock_port_value_f = 0.0;
    *new_class->stock_port_value_chg_f = 0.0;
    *new_class->stock_port_value_p_chg_f = 0.0;
    *new_class->portfolio_port_value_f = 0.0;
    *new_class->portfolio_port_value_chg_f = 0.0;
    *new_class->portfolio_port_value_p_chg_f = 0.0;
    *new_class->updates_per_min_f = 6.0;
    *new_class->updates_hours_f = 1.0;

    *new_class->fetching_data_bool = false;
    *new_class->holiday_bool = false;

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
    new_class->EntireStake = EntirePortfolio;
    new_class->BullionStake = BullionPortfolio;
    new_class->DoubToStr = DoubleToString;
    new_class->StrToDoub = StringToDouble;

    /* Return Our Initialized Class */
    return new_class; 
}

/* Class Destruct Functions */
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
    if ( F ) free( F );

}

void class_destruct_metal (metal* metal_handle)
{
    /* Free Memory From Class Objects */
    if ( metal_handle->Gold ) class_destruct_bullion ( metal_handle->Gold );
    if ( metal_handle->Silver ) class_destruct_bullion ( metal_handle->Silver );
    if ( metal_handle ) free( metal_handle );
}

void class_destruct_bullion (bullion* bullion_class)
{ 
    /* Free Memory From Variables */
    if ( bullion_class->spot_price_f ) free( bullion_class->spot_price_f );
    if ( bullion_class->premium_f ) free( bullion_class->premium_f );
    if ( bullion_class->port_value_f ) free( bullion_class->port_value_f );
    if ( bullion_class->ounce_f ) free( bullion_class->ounce_f );

    if ( bullion_class->spot_price_ch ) free( bullion_class->spot_price_ch );            
    if ( bullion_class->premium_ch ) free( bullion_class->premium_ch ); 
    if ( bullion_class->port_value_ch ) free( bullion_class->port_value_ch );            

    if ( bullion_class->ounce_ch ) free( bullion_class->ounce_ch );                 

    /* Free Memory From Class Object */
    if ( bullion_class ) {
        free( bullion_class ); 
        bullion_class = NULL;
    }
}

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

    /* Free Memory From Class Object */
    if ( stock_class ) {
        free( stock_class ); 
        stock_class = NULL;
    }
}

void class_destruct_meta_data (meta* meta_class)
{ 
    /* Free Memory From Variables */
    if ( meta_class->cash_f ) free( meta_class->cash_f );
    if ( meta_class->bullion_port_value_f ) free( meta_class->bullion_port_value_f );
    if ( meta_class->stock_port_value_f ) free( meta_class->stock_port_value_f );
    if ( meta_class->stock_port_value_chg_f ) free( meta_class->stock_port_value_chg_f );
    if ( meta_class->stock_port_value_p_chg_f ) free( meta_class->stock_port_value_p_chg_f );
    if ( meta_class->portfolio_port_value_f ) free( meta_class->portfolio_port_value_f );
    if ( meta_class->portfolio_port_value_chg_f ) free( meta_class->portfolio_port_value_chg_f );
    if ( meta_class->portfolio_port_value_p_chg_f ) free( meta_class->portfolio_port_value_p_chg_f );
    if ( meta_class->updates_per_min_f ) free( meta_class->updates_per_min_f );
    if ( meta_class->updates_hours_f ) free( meta_class->updates_hours_f );

    if ( meta_class->stock_url ) free( meta_class->stock_url );
	if ( meta_class->curl_key ) free( meta_class->curl_key );                 

    if ( meta_class->cash_ch ) free( meta_class->cash_ch );                  
    if ( meta_class->bullion_port_value_ch ) free( meta_class->bullion_port_value_ch );    
    if ( meta_class->stock_port_value_ch ) free( meta_class->stock_port_value_ch );      
    if ( meta_class->stock_port_value_chg_ch ) free( meta_class->stock_port_value_chg_ch );
    if ( meta_class->stock_port_value_p_chg_ch ) free( meta_class->stock_port_value_p_chg_ch );
    if ( meta_class->portfolio_port_value_ch ) free( meta_class->portfolio_port_value_ch );
    if ( meta_class->portfolio_port_value_chg_ch ) free( meta_class->portfolio_port_value_chg_ch );
    if ( meta_class->portfolio_port_value_p_chg_ch ) free( meta_class->portfolio_port_value_p_chg_ch );

    if ( meta_class->home_dir_ch ) free( meta_class->home_dir_ch );
    if ( meta_class->sqlite_db_path_ch ) free( meta_class->sqlite_db_path_ch );

    if ( meta_class->fetching_data_bool ) free( meta_class->fetching_data_bool );
    
    /* Free Memory From Class Object */
    if ( meta_class ) {
         free( meta_class ); 
         meta_class = NULL;
    }
}