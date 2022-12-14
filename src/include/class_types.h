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
#include <time.h>                         /* struct tm  */

/* We need the MemType, CURL, and CURLM types from this header file. */
#include "multicurl_types.h"

#include "gui_types.h" /*  */

/* Method prototypes used by these classes
   Define the function pointer type here. */
typedef double sub_func_a_t (const unsigned int,const double);
typedef double sub_func_b_t (const double,const double,const double);
typedef double sub_func_c_t (const double,const double,const double,const double);
typedef void sub_func_d_t (char**,const double);
typedef double sub_func_e_t (const char*);
typedef void* sub_func_f_t ();
typedef void sub_func_g_t ();
typedef int sub_func_h_t ();
typedef bool sub_func_i_t ();
typedef void sub_func_j_t (bool);
typedef double sub_func_k_t ();
typedef struct tm sub_func_l_t ();
typedef unsigned int sub_func_m_t ();
typedef int sub_func_n_t (void*);
typedef void sub_func_o_t (void*);
typedef void sub_func_p_t (const char*,const char*);
typedef void sub_func_q_t (char**,const double,const short);
typedef void sub_func_r_t (const char*);

/* class type definitions (emulated class definitions; C doesn't really have class types) */
typedef struct {
    /* Data Variables */
	double spot_price_f;
    double premium_f;
    double port_value_f;
    double ounce_f;

    double high_metal_f; 
	double low_metal_f;
    double prev_closing_metal_f;
    double change_ounce_f;
    double change_value_f;
    double change_percent_f;
    double change_percent_raw_f;          

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
    char* change_percent_raw_ch;           /* The gain not including premium values. */

    char* url_ch;

    CURL* YAHOO_hnd;                        /* Bullion cURL Easy Handle. */
    MemType CURLDATA;

    /* 
       Methods or Functions
       Create a function pointer from the type here.
    */
    sub_func_b_t* Stake;
    sub_func_d_t* DoubToStr;
    sub_func_q_t* DoubToPerStr;
    sub_func_q_t* DoubToNumStr;
    sub_func_e_t* StrToDoub;
    
} bullion;

typedef struct {
    /* Data Variables */
	unsigned int num_shares_stock_int;     /* Cannot hold more than 4294967295 shares of stock on most 64-bit machines */        

    double current_price_stock_f;
    double high_stock_f; 
	double low_stock_f;
    double opening_stock_f; 
    double prev_closing_stock_f;
    double change_share_f;
    double change_value_f;
    double change_percent_f;

	double current_investment_stock_f;    

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

    CURL* easy_hnd;                         /* cURL Easy Handle. */
    MemType JSON;

    /* 
       Methods or Functions
       Create a function pointer from the type here.
    */
    sub_func_a_t* Stake;
    sub_func_d_t* DoubToStr;
    sub_func_d_t* DoubToPerStr;
    sub_func_e_t* StrToDoub;

} stock;

typedef struct {
    /* Data Variables */
    double cash_f;
    
    double portfolio_port_value_f;
    double portfolio_port_value_chg_f;
    double portfolio_port_value_p_chg_f;

    double index_dow_value_f;
    double index_dow_value_chg_f;
    double index_dow_value_p_chg_f;

    double index_nasdaq_value_f;
    double index_nasdaq_value_chg_f;
    double index_nasdaq_value_p_chg_f;

    double index_sp_value_f;
    double index_sp_value_chg_f;
    double index_sp_value_p_chg_f;

    double crypto_bitcoin_value_f;
    double crypto_bitcoin_value_chg_f;
    double crypto_bitcoin_value_p_chg_f;
    
    double updates_per_min_f;
    double updates_hours_f;

    char* stock_url_ch;                        /* First component of the stock request URL */
	char* curl_key_ch;                         /* Last component of the stock request URL */
    char* Nasdaq_Symbol_url_ch;                /* Nasdaq Symbol List URL */
    char* NYSE_Symbol_url_ch;                  /* NYSE Symbol List URL */
  
    char* cash_ch;                          /* Total value of cash */
    char* portfolio_port_value_ch;          /* Total value of the entire portfolio */
    char* portfolio_port_value_chg_ch;      /* Total value of the entire portfolio change */
    char* portfolio_port_value_p_chg_ch;    /* Total value of the entire portfolio percent change */

    char* index_dow_value_ch;
    char* index_dow_value_chg_ch;
    char* index_dow_value_p_chg_ch;

    char* index_nasdaq_value_ch;
    char* index_nasdaq_value_chg_ch;
    char* index_nasdaq_value_p_chg_ch;

    char* index_sp_value_ch;
    char* index_sp_value_chg_ch;
    char* index_sp_value_p_chg_ch;

    char* crypto_bitcoin_value_ch;
    char* crypto_bitcoin_value_chg_ch;
    char* crypto_bitcoin_value_p_chg_ch;

    char* home_dir_ch;                      /* Path to the user's home directory */
    char* sqlite_db_path_ch;                /* Path to the sqlite db file */
    char* sqlite_symbol_name_db_path_ch;    /* Path to the sqlite symbol-name db file */

    bool fetching_data_bool;               /* Indicates a fetch operation in progress. */
    bool holiday_bool;                     /* Indicates if today is a holiday. */
    bool multicurl_cancel_bool;            /* Indicates if we should cancel the multicurl request. */
    bool index_bar_revealed_bool;          /* Indicates if the indices bar is revealed or not. */
    bool clocks_displayed_bool;            /* Indicates if the clocks are displayed or not. */
    bool main_win_default_view_bool;       /* Indicates if the main window treeview is displaying 
                                              the default or the primary view. Default is true. */

    CURL* rsi_hnd;                          /* RSI Data cURL Easy Handle. */
    CURL* NASDAQ_completion_hnd;            /* RSI NASDAQ Symbol list cURL Easy Handle. */
    CURL* NYSE_completion_hnd;              /* RSI NYSE Symbol list cURL Easy Handle. */

    CURL* index_dow_hnd;                    /* DJIA Index cURL Easy Handle. */
    CURL* index_nasdaq_hnd;                 /* Nasdaq Index cURL Easy Handle. */
    CURL* index_sp_hnd;                     /* S&P Index cURL Easy Handle. */
    CURL* crypto_bitcoin_hnd;               /* Bitcoin cURL Easy Handle. */

    CURLM* multicurl_cmpltn_hnd;            /* Multicurl Handle for the No Prog Multicurl function. */
    CURLM* multicurl_rsi_hnd;               /* Multicurl Handle for the No Prog Multicurl function. */

    MemType INDEX_DOW_CURLDATA;
    MemType INDEX_NASDAQ_CURLDATA;
    MemType INDEX_SP_CURLDATA;
    MemType CRYPTO_BITCOIN_CURLDATA;

    /* 
       Methods or Functions
       Create a function pointer from the type here.
    */
    sub_func_b_t* EntireStake;
    sub_func_q_t* DoubToStr;
    sub_func_q_t* DoubToPerStr;
    sub_func_e_t* StrToDoub;
    sub_func_g_t* ToStringsPortfolio;
    sub_func_o_t* CalculatePortfolio;
    sub_func_g_t* StopRSICurl;
    sub_func_g_t* StopSNMapCurl;
    sub_func_n_t* SetUpCurlIndicesData;
    sub_func_g_t* ExtractIndicesData;
    sub_func_g_t* ToStringsIndices;
} meta;

typedef struct {
    /* Bullion Handles */
	bullion* Gold;
    bullion* Silver;
    bullion* Platinum;
    bullion* Palladium;

    /* Data Variables */
    double bullion_port_value_f;
    double bullion_port_value_chg_f;
    double bullion_port_value_p_chg_f;
    double gold_silver_ratio_f;

    char* bullion_port_value_ch;            /* Total value of bullion holdings */
    char* bullion_port_value_chg_ch;        /* Total value of bullion holdings change */
    char* bullion_port_value_p_chg_ch;      /* Total value of bullion holdings percent change */
    char* gold_silver_ratio_ch;

    /* Methods or Functions */
    sub_func_c_t* BullionStake;
    sub_func_d_t* DoubToStr;
    sub_func_q_t* DoubToPerStr;
    sub_func_q_t* DoubToNumStr;
    sub_func_g_t* ToStrings;
    sub_func_g_t* Calculate;
    sub_func_n_t* SetUpCurl;
    sub_func_g_t* ExtractData;
} metal;

typedef struct {
    /* Handle to stock array */
	stock** Equity;                          /* Stock Double Pointer Array */

    /* Data Variables */
    double stock_port_value_f;
    double stock_port_value_chg_f;
    double stock_port_value_p_chg_f;

    char* stock_port_value_ch;              /* Total value of equity holdings */
    char* stock_port_value_chg_ch;          /* Total value of equity holdings change */
    char* stock_port_value_p_chg_ch;        /* Total value of equity holdings percent change */

    unsigned short size;                     /* The number of stocks */

    /* Methods or Functions */
    sub_func_d_t* DoubToStr;
    sub_func_d_t* DoubToPerStr;
    sub_func_g_t* ToStrings;
    sub_func_g_t* Calculate;
    sub_func_o_t* GenerateURL;
    sub_func_n_t* SetUpCurl;
    sub_func_g_t* ExtractData;
    sub_func_p_t* AddStock;
    sub_func_g_t* Sort;
    sub_func_g_t* Reset;
    sub_func_r_t* RemoveStock;
} equity_folder;

typedef struct {
  int main_height;
  int main_width;
  int main_x_pos;
  int main_y_pos;
  int rsi_height;
  int rsi_width;
  int rsi_x_pos;
  int rsi_y_pos;
} window_data;

/* A handle to our three primary classes and the window_data struct for passing through threads */
typedef struct {
  /* handles to each of our three classes and the window_data struct */
  metal* metal_class;
  equity_folder* equity_folder_class;
  meta* meta_class;
  window_data* window_struct;                          /* A struct that holds the size and position of 
                                                          the main and rsi windows. */
  symbol_name_map* sym_map;                            /* The symbol to name mapping struct */

  /* Main Multicurl Handle for use with data fetch operation */
  CURLM* multicurl_main_hnd;

  /* Methods or Functions */
  sub_func_g_t* Calculate;
  sub_func_g_t* ToStrings;
  sub_func_h_t* GetData;
  sub_func_g_t* ExtractData;
  sub_func_i_t* IsFetchingData;
  sub_func_j_t* SetFetchingData;
  sub_func_i_t* IsDefaultView;
  sub_func_j_t* SetDefaultView;
  sub_func_g_t* StopMultiCurl;
  sub_func_i_t* IsCurlCanceled;
  sub_func_j_t* SetCurlCanceled;
  sub_func_k_t* GetHoursOfUpdates;
  sub_func_k_t* GetUpdatesPerMinute;
  sub_func_i_t* IsHoliday;
  sub_func_l_t* SetHoliday;
  sub_func_g_t* SetWindowDataSql;
  sub_func_f_t* GetWindowData;
  sub_func_f_t* GetMetaClass;
  sub_func_f_t* GetMetalClass;
  sub_func_f_t* GetSymNameMap;
  sub_func_o_t* SetSymNameMap;
  sub_func_f_t* GetEquityFolderClass;
  sub_func_m_t* SecondsToOpen;
  sub_func_i_t* IsClockDisplayed;
  sub_func_j_t* SetClockDisplayed;
  sub_func_i_t* IsIndicesDisplayed;
  sub_func_j_t* SetIndicesDisplayed;

} portfolio_packet;

#endif /* CLASS_TYPES_HEADER_H */