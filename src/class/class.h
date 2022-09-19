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
#ifndef CLASS_HEADER_H
#define CLASS_HEADER_H

/* We need the MemType struct from this header file. */
#include "../multicurl/multicurl.h"

/* These two macros define initial URL values before the user sets up the 
   application. */
/* The finnhub.io stock quote URL */
#ifndef FINNHUB_URL
#define FINNHUB_URL "https://finnhub.io/api/v1/quote?symbol="
#endif

/* The finnhub.io URL account token */
#ifndef FINNHUB_URL_TOKEN
#define FINNHUB_URL_TOKEN "&token=<YOUR ACCOUNT KEY>"
#endif

/* Method prototypes used by these classes
   Define the function pointer type here. */
typedef double sub_func_b_t (const unsigned int*,const double*);
typedef double sub_func_c_t (const double*,const double*,const double*);
typedef double sub_func_d_t (const double*,const double*);
typedef char* sub_func_e_t (const double*);
typedef double sub_func_f_t (const char*);

/* class type definitions (emulated class definitions; C doesn't really have class types) */
typedef struct {
    /* Data Variables */
	double* spot_price_f;
    double* premium_f;
    double* port_value_f;
    double* ounce_f;

    char* spot_price_ch;            
    char* premium_ch; 
    char* port_value_ch;                    /* Total current investment in this metal. */
    char* ounce_ch;                         /* Number of ounces. */

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
    MemType JSON;

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
    double* bullion_port_value_f;
    double* stock_port_value_f;
    double* stock_port_value_chg_f;
    double* stock_port_value_p_chg_f;
    double* portfolio_port_value_f;
    double* portfolio_port_value_chg_f;
    double* portfolio_port_value_p_chg_f;
    
    double* updates_per_min_f;
    double* updates_hours_f;

    char* stock_url;                        /* First component of the stock request URL */
	char* curl_key;                         /* Last component of the stock request URL */
  
    char* cash_ch;                          /* The total value of cash */
    char* bullion_port_value_ch;            /* Total value of bullion holdings */
    char* stock_port_value_ch;              /* Total value of equity holdings */
    char* stock_port_value_chg_ch;          /* Total value of equity holdings change */
    char* stock_port_value_p_chg_ch;        /* Total value of equity holdings percent change */
    char* portfolio_port_value_ch;          /* Total value of the entire portfolio */
    char* portfolio_port_value_chg_ch;      /* Total value of the entire portfolio change */
    char* portfolio_port_value_p_chg_ch;    /* Total value of the entire portfolio percent change */

    char* home_dir_ch;                      /* Path to the user's home directory */
    char* sqlite_db_path_ch;                /* Path to the sqlite db file */
    char* local_tz_ch;                      /* The local timezone variable */

    bool* fetching_data_bool;               /* Indicates a fetch operation in progress. */
    bool* holiday_bool;                     /* Indicates if today is a holiday. */

    /* 
       Methods or Functions
       Create a function pointer from the type here.
    */
    sub_func_c_t* EntireStake;
    sub_func_d_t* BullionStake;
    sub_func_e_t* DoubToStr;
    sub_func_f_t* StrToDoub;

} meta;

/* This is a handle to other class objects and not an emulation of a class object. */
typedef struct {
    /* Metal Handles */
	bullion* Gold;
    bullion* Silver;    
} metal;

/* This is a handle to other class objects and not an emulation of a class object. */
typedef struct {
	stock** Equity;                          /* Equity Double Pointer Array */
    unsigned short size;                    /* The number of stocks */
} equity_folder;

/* class init prototypes [these do not have to return pointers, but this program
   uses pointers to classes rather than regular class/struct objects for better memory 
   management procedures]. 

   These inits connect the function pointers to specific functions
   and initialize the variables. 
*/
equity_folder* class_init_equity_folder ();
stock *class_init_equity ();
metal* class_init_metal ();
bullion* class_init_bullion ();
meta* class_init_meta_data ();

/* Class destruct prototypes. */
void class_destruct_equity_folder (equity_folder*);
void class_destruct_equity (stock*);
void class_destruct_metal (metal*);
void class_destruct_bullion (bullion*);
void class_destruct_meta_data (meta*);

#endif /* CLASS_HEADER_H */