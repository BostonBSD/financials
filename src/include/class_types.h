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
#ifndef CLASS_TYPES_HEADER_H
#define CLASS_TYPES_HEADER_H

#include <stdbool.h>

/* We need the MemType, CURL, and CURLM types from this header file. */
#include "multicurl_types.h"

/* Method prototypes used by these classes
   Define the function pointer type here. */
typedef double sub_func_b_t (const unsigned int*,const double*);
typedef double sub_func_c_t (const double*,const double*,const double*);
typedef double sub_func_d_t (const double*,const double*);
typedef char* sub_func_e_t (const double*);
typedef double sub_func_f_t (const char*);
/* These mostly take in class types, but the class type isn't defined yet,
   so we cast them to their appropriate type within the member function. */
typedef void sub_func_g_t (void*);
typedef void sub_func_h_t (void*,void*,void*);
typedef void sub_func_i_t (void*,void*);
typedef int sub_func_j_t (void*);

/* class type definitions (emulated class definitions; C doesn't really have class types) */
typedef struct {
    /* Data Variables */
    char* url_ch;

	double* spot_price_f;
    double* premium_f;
    double* port_value_f;
    double* ounce_f;

    double* high_metal_f; 
	double* low_metal_f;
    double* prev_closing_metal_f;
    double* change_ounce_f;
    double* change_value_f;
    double* change_percent_f;

    char* spot_price_ch;            
    char* premium_ch; 
    char* port_value_ch;                    /* Total current investment in this metal. */
    char* ounce_ch;                         /* Number of ounces. */

    char* high_metal_ch; 
	char* low_metal_ch;
    char* prev_closing_metal_ch;
    char* change_ounce_ch;
    char* change_value_ch;
    char* change_percent_ch;

    CURL *YAHOO_hnd;                        /* Bullion cURL Easy Handle. */
    MemType CURLDATA;

    /* 
       Methods or Functions
       Create a function pointer from the type here.
    */
    sub_func_c_t* Stake;
    sub_func_e_t* DoubToStr;
    sub_func_f_t* StrToDoub;
    
} bullion;

typedef struct {
    /* Data Variables */
	unsigned int* num_shares_stock_int;           

    double* current_price_stock_f;
    double* high_stock_f; 
	double* low_stock_f;
    double* opening_stock_f; 
    double* prev_closing_stock_f;
    double* change_share_f;
    double* change_value_f;
    double* change_percent_f;

	double* current_investment_stock_f;    

    char* current_price_stock_ch;
    char* high_stock_ch; 
	char* low_stock_ch;
    char* opening_stock_ch; 
    char* prev_closing_stock_ch;
    char* change_share_ch;
    char* change_value_ch;
    char* change_percent_ch;

	char* current_investment_stock_ch;      /* The total current investment in the stock. */

	char* symbol_stock_ch;                  /* The stock symbol in all caps, it's used to create the stock request URL */
	char* curl_url_stock_ch;                /* The assembled request URL, Each stock has it's own URL request */

    CURL *easy_hnd;                         /* cURL Easy Handle. */
    MemType JSON;

    /* 
       Methods or Functions
       Create a function pointer from the type here.
    */
    sub_func_b_t* Stake;
    sub_func_e_t* DoubToStr;
    sub_func_f_t* StrToDoub;

} stock;

typedef struct {
    /* Data Variables */
    double* cash_f;
    
    double* portfolio_port_value_f;
    double* portfolio_port_value_chg_f;
    double* portfolio_port_value_p_chg_f;
    
    double* updates_per_min_f;
    double* updates_hours_f;

    char* stock_url;                        /* First component of the stock request URL */
	char* curl_key;                         /* Last component of the stock request URL */
  
    char* cash_ch;                          /* Total value of cash */
    char* portfolio_port_value_ch;          /* Total value of the entire portfolio */
    char* portfolio_port_value_chg_ch;      /* Total value of the entire portfolio change */
    char* portfolio_port_value_p_chg_ch;    /* Total value of the entire portfolio percent change */

    char* home_dir_ch;                      /* Path to the user's home directory */
    char* sqlite_db_path_ch;                /* Path to the sqlite db file */

    bool* fetching_data_bool;               /* Indicates a fetch operation in progress. */
    bool* holiday_bool;                     /* Indicates if today is a holiday. */
    bool* multicurl_cancel_bool;            /* Indicates if we should cancel the multicurl request. */

    CURL *rsi_hnd;                          /* RSI Data cURL Easy Handle. */
    CURL *NASDAQ_completion_hnd;            /* RSI NASDAQ Symbol list cURL Easy Handle. */
    CURL *NYSE_completion_hnd;              /* RSI NYSE Symbol list cURL Easy Handle. */
    CURLM *multicurl_cmpltn_hnd;            /* Multicurl Handle for the No Prog Multicurl function. */
    CURLM *multicurl_rsi_hnd;               /* Multicurl Handle for the No Prog Multicurl function. */

    /* 
       Methods or Functions
       Create a function pointer from the type here.
    */
    sub_func_c_t* EntireStake;
    sub_func_e_t* DoubToStr;
    sub_func_f_t* StrToDoub;
    sub_func_g_t* PortfolioToStrings;
    sub_func_h_t* PortfolioCalculate;
    sub_func_g_t* StopRSICurl;
    sub_func_g_t* StopSNMapCurl;

} meta;

typedef struct {
    /* Bullion Handles */
	bullion* Gold;
    bullion* Silver;

    /* Data Variables */
    double* bullion_port_value_f;
    double* bullion_port_value_chg_f;
    double* bullion_port_value_p_chg_f;

    char* bullion_port_value_ch;            /* Total value of bullion holdings */
    char* bullion_port_value_chg_ch;        /* Total value of bullion holdings change */
    char* bullion_port_value_p_chg_ch;      /* Total value of bullion holdings percent change */

    CURLM *multicurl_hnd;                    /* Multicurl Handle. */

    /* Methods or Functions */
    sub_func_d_t* BullionStake;
    sub_func_e_t* DoubToStr;
    sub_func_g_t* ToStrings;
    sub_func_g_t* Calculate;
    sub_func_j_t* GetData;
    sub_func_g_t* ExtractData;
    sub_func_g_t* StopCurl;
} metal;

typedef struct {
    /* Handle to stock array */
	stock** Equity;                          /* Stock Double Pointer Array */

    /* Data Variables */
    double* stock_port_value_f;
    double* stock_port_value_chg_f;
    double* stock_port_value_p_chg_f;

    char* stock_port_value_ch;              /* Total value of equity holdings */
    char* stock_port_value_chg_ch;          /* Total value of equity holdings change */
    char* stock_port_value_p_chg_ch;        /* Total value of equity holdings percent change */

    unsigned short size;                     /* The number of stocks */
    CURLM *multicurl_hnd;                    /* Multicurl Handle. */

    /* Methods or Functions */
    sub_func_e_t* DoubToStr;
    sub_func_g_t* ToStrings;
    sub_func_g_t* Calculate;
    sub_func_i_t* GenerateURL;
    sub_func_j_t* GetData;
    sub_func_g_t* ExtractData;
    sub_func_g_t* AddStock;
    sub_func_g_t* Sort;
    sub_func_g_t* Reset;
    sub_func_g_t* StopCurl;
} equity_folder;

/* A handle to our three primary classes for passing through threads */
typedef struct {
  metal *metal_chest;
  equity_folder *securities_folder;
  meta *portfolio_meta_info;
} portfolio_packet;

#endif /* CLASS_TYPES_HEADER_H */