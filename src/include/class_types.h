/*
Copyright (c) 2022-2023 BostonBSD. All rights reserved.

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

#include "gui_types.h"       /* symbol_name_map */
#include "multicurl_types.h" /* CURL, CURLM */

typedef struct bullion bullion;
typedef struct metal metal;
typedef struct stock stock;
typedef struct equity_folder equity_folder;
typedef struct meta meta;
typedef struct portfolio_packet portfolio_packet;

typedef struct { /* A container to hold the type of row and symbol, on a right
                    click */
  gchar *type;
  gchar *symbol;
} right_click_container;

typedef struct {
  gint main_height;
  gint main_width;
  gint main_x_pos;
  gint main_y_pos;
  gint history_height;
  gint history_width;
  gint history_x_pos;
  gint history_y_pos;
} window_data;

typedef struct {
  gchar *bullion;
  gchar *metal;
  gchar *ounces;
  gchar *premium;
  gchar *high;
  gchar *low;
  gchar *prev_closing;
  gchar *chg;
  gchar *gain_sym;
  gchar *total;
  gchar *gain_per;
  gchar *gold;
  gchar *silver;
  gchar *platinum;
  gchar *palladium;
  gchar *equity;
  gchar *symbol;
  gchar *shares;
  gchar *price;
  gchar *opening;
  gchar *asset;
  gchar *value;
  gchar *cash;
  gchar *portfolio;
  gchar *no_assets;
} primary_heading;

typedef struct {
  gchar *bullion;
  gchar *metal;
  gchar *ounces;
  gchar *premium;
  gchar *gold;
  gchar *silver;
  gchar *platinum;
  gchar *palladium;
  gchar *equity;
  gchar *symbol;
  gchar *shares;
  gchar *cash;
  gchar *no_assets;
} default_heading;

/* class type definitions */
struct bullion {
  /* Data Variables */
  gdouble spot_price_f;
  gdouble premium_f;
  gdouble port_value_f; /* Total current investment in this metal. */
  gdouble ounce_f;      /* Number of ounces. */

  gdouble high_metal_f;
  gdouble low_metal_f;
  gdouble prev_closing_metal_f;
  gdouble change_ounce_f;
  gdouble change_value_f;
  gdouble change_percent_f;
  gdouble change_percent_raw_f; /* The gain not including premium values. */

  /* Pango Markup language strings */
  gchar *spot_price_mrkd_ch;
  gchar *premium_mrkd_ch;
  gchar *port_value_mrkd_ch;
  gchar *ounce_mrkd_ch;

  gchar *high_metal_mrkd_ch;
  gchar *low_metal_mrkd_ch;
  gchar *prev_closing_metal_mrkd_ch;
  gchar *change_ounce_mrkd_ch;
  gchar *change_value_mrkd_ch;
  gchar *change_percent_mrkd_ch;

  /* Unmarked strings */
  gchar *change_percent_raw_ch;
  gchar *url_ch;

  CURL *YAHOO_hnd; /* Bullion cURL Easy Handle. */
  MemType CURLDATA;
};

struct metal {
  /* Bullion Handles */
  bullion *Gold;
  bullion *Silver;
  bullion *Platinum;
  bullion *Palladium;

  /* Data Variables */
  gdouble bullion_port_value_f;
  gdouble bullion_port_value_chg_f;
  gdouble bullion_port_value_p_chg_f;
  gdouble gold_silver_ratio_f;

  /* Pango Markup language strings */
  gchar *bullion_port_value_mrkd_ch;       /* Total value of bullion holdings */
  gchar *bullion_port_value_chg_mrkd_ch;   /* Total value of bullion holdings
                                             change */
  gchar *bullion_port_value_p_chg_mrkd_ch; /* Total value of bullion holdings
                                             percent change */

  /* Unmarked strings */
  gchar *gold_silver_ratio_ch;

  /* Method/Function pointers */
  void (*ToStrings)(guint8 digits_right);
  void (*Calculate)();
  gint (*SetUpCurl)(portfolio_packet *pkg);
  void (*ExtractData)();
};

struct stock {
  /* Data Variables */
  guint num_shares_stock_int; /* Cannot hold more than 4294967295 shares
                                        of stock on most 64-bit machines */

  gdouble current_price_stock_f;
  gdouble high_stock_f;
  gdouble low_stock_f;
  gdouble opening_stock_f;
  gdouble prev_closing_stock_f;
  gdouble change_share_f;
  gdouble change_value_f;
  gdouble change_percent_f;

  gdouble current_investment_stock_f; /* The total current investment in
                                              the stock. */

  /* Pango Markup language strings */
  gchar *security_name_mrkd_ch;
  gchar *current_price_stock_mrkd_ch;
  gchar *high_stock_mrkd_ch;
  gchar *low_stock_mrkd_ch;
  gchar *opening_stock_mrkd_ch;
  gchar *prev_closing_stock_mrkd_ch;
  gchar *change_share_stock_mrkd_ch;
  gchar *change_value_mrkd_ch;
  gchar *change_percent_mrkd_ch;
  gchar *current_investment_stock_mrkd_ch;
  gchar *symbol_stock_mrkd_ch;
  gchar *num_shares_stock_mrkd_ch;

  /* Unmarked strings */
  gchar *symbol_stock_ch;   /* The stock symbol in all caps, it's used to create
                              the stock request URL */
  gchar *curl_url_stock_ch; /* The assembled request URL, Each stock has it's
                              own URL request */

  CURL *easy_hnd; /* cURL Easy Handle. */
  MemType JSON;
};

struct equity_folder {
  /* Handle to stock array */
  stock **Equity; /* Stock Double Pointer Array */

  /* Data Variables */
  gdouble stock_port_value_f;
  gdouble stock_port_value_chg_f;
  gdouble stock_port_value_p_chg_f;

  /* Can have up to 255 stocks [0-254]: 8 bits. */
  guint8 size;

  /* Pango Markup language strings */
  gchar *stock_port_value_mrkd_ch; /* Total value of equity holdings */
  gchar
      *stock_port_value_chg_mrkd_ch; /* Total value of equity holdings change */
  gchar *stock_port_value_p_chg_mrkd_ch; /* Total value of equity holdings
                                      percent change */

  /* Method/Function pointers */
  void (*ToStrings)(guint8 digits_right);
  void (*Calculate)();
  void (*GenerateURL)(portfolio_packet *pkg);
  gint (*SetUpCurl)(portfolio_packet *pkg);
  void (*ExtractData)();
  void (*AddStock)(const gchar *symbol, const gchar *shares);
  void (*Sort)();
  void (*Reset)();
  void (*RemoveStock)(const gchar *s);
  void (*SetSecurityNames)(portfolio_packet *pkg);
};

struct meta {
  /* Data Variables */
  right_click_container rght_clk_data;
  window_data window_struct; /* A struct that holds the size and position of
                                 the main and rsi windows. */

  symbol_name_map *sym_map; /* The symbol to name mapping struct */

  gdouble cash_f; /* Total value of cash */

  gdouble portfolio_port_value_f;     /* Total value of the entire portfolio */
  gdouble portfolio_port_value_chg_f; /* Total value of the entire
                                               portfolio change */
  gdouble portfolio_port_value_p_chg_f; /* Total value of the entire
                                                portfolio percent change */

  gdouble index_dow_value_f;
  gdouble index_dow_value_chg_f;
  gdouble index_dow_value_p_chg_f;

  gdouble index_nasdaq_value_f;
  gdouble index_nasdaq_value_chg_f;
  gdouble index_nasdaq_value_p_chg_f;

  gdouble index_sp_value_f;
  gdouble index_sp_value_chg_f;
  gdouble index_sp_value_p_chg_f;

  gdouble crypto_bitcoin_value_f;
  gdouble crypto_bitcoin_value_chg_f;
  gdouble crypto_bitcoin_value_p_chg_f;

  gdouble updates_per_min_f;
  gdouble updates_hours_f;

  guint8 decimal_places_guint8;

  /* Pango Markup language strings */
  primary_heading pri_h_mkd;
  default_heading def_h_mkd;

  gchar *cash_mrkd_ch;
  gchar *portfolio_port_value_mrkd_ch;
  gchar *portfolio_port_value_chg_mrkd_ch;
  gchar *portfolio_port_value_p_chg_mrkd_ch;

  /* Unmarked strings */
  gchar *stock_url_ch;         /* First component of the stock request URL */
  gchar *curl_key_ch;          /* Last component of the stock request URL */
  gchar *Nasdaq_Symbol_url_ch; /* Nasdaq Symbol List URL */
  gchar *NYSE_Symbol_url_ch;   /* NYSE Symbol List URL */

  gchar *index_dow_value_ch;
  gchar *index_dow_value_chg_ch;
  gchar *index_dow_value_p_chg_ch;

  gchar *index_nasdaq_value_ch;
  gchar *index_nasdaq_value_chg_ch;
  gchar *index_nasdaq_value_p_chg_ch;

  gchar *index_sp_value_ch;
  gchar *index_sp_value_chg_ch;
  gchar *index_sp_value_p_chg_ch;

  gchar *crypto_bitcoin_value_ch;
  gchar *crypto_bitcoin_value_chg_ch;
  gchar *crypto_bitcoin_value_p_chg_ch;

  gchar *config_dir_ch;     /* Path to the application config directory */
  gchar *sqlite_db_path_ch; /* Path to the sqlite db file */
  gchar *sqlite_symbol_name_db_path_ch; /* Path to the sqlite symbol-name db
                                           file */
  gchar *font_ch;                       /* The application font */

  gboolean
      fetching_data_bool : 1; /* Indicates a fetch operation in progress. */
  gboolean
      market_closed_bool : 1; /* Indicates if the market is open or closed. */
  gboolean exit_app_bool : 1; /* Indicates if we are exiting the application. */
  gboolean multicurl_cancel_main_bool : 1; /* Indicates if we should cancel the
                                            main multicurl request. */
  gboolean index_bar_revealed_bool : 1;    /* Indicates if the indices bar is
                                            revealed or not. */
  gboolean clocks_displayed_bool : 1;      /* Indicates if the clocks are
                                                     displayed or not. */
  gboolean
      main_win_default_view_bool : 1; /* Indicates if the main window
                                       treeview is displaying the default or
                                       the primary view. Default is TRUE. */
  gboolean
      snmap_db_busy_bool : 1; /* Indicates if the snmap db is busy or not. */

  CURL *history_hnd;           /* History Data cURL Easy Handle. */
  CURL *NASDAQ_completion_hnd; /* NASDAQ Symbol list cURL Easy Handle. */
  CURL *NYSE_completion_hnd;   /* NYSE Symbol list cURL Easy Handle. */

  CURL *index_dow_hnd;      /* DJIA Index cURL Easy Handle. */
  CURL *index_nasdaq_hnd;   /* Nasdaq Index cURL Easy Handle. */
  CURL *index_sp_hnd;       /* S&P Index cURL Easy Handle. */
  CURL *crypto_bitcoin_hnd; /* Bitcoin cURL Easy Handle. */

  /* Two multicurl handle used with the No Prog Multicurl function. */
  CURLM *multicurl_cmpltn_hnd;
  CURLM *multicurl_history_hnd;

  MemType INDEX_DOW_CURLDATA;
  MemType INDEX_NASDAQ_CURLDATA;
  MemType INDEX_SP_CURLDATA;
  MemType CRYPTO_BITCOIN_CURLDATA;

  /* Thread Ids and cond variables */
  GCond gthread_clocks_cond;
  GCond gthread_main_fetch_cond;
  GThread *gthread_clocks_id;
  GThread *gthread_main_fetch_id;

  /* Methods/Function pointers. */
  void (*ToStringsPortfolio)();
  void (*CalculatePortfolio)(portfolio_packet *pkg);
  void (*StopHistoryCurl)();
  void (*StopSNMapCurl)();
  gint (*SetUpCurlIndicesData)(portfolio_packet *pkg);
  void (*ExtractIndicesData)();
  void (*ToStringsIndices)();
  void (*ToStringsHeadings)();
};

/* A handle to our three primary classes and some useful functions */
struct portfolio_packet {
  /* handles to each of our three classes */
  metal *metal_class;
  equity_folder *equity_folder_class;
  meta *meta_class;

  /* Main Multicurl Handle for use with data fetch operation */
  CURLM *multicurl_main_hnd;

  /* Method/Function pointers */
  void (*Calculate)();
  void (*ToStrings)();
  gint (*GetData)();
  void (*ExtractData)();
  gboolean (*IsFetchingData)();
  void (*SetFetchingData)(gboolean fetching_bool);
  gboolean (*IsDefaultView)();
  void (*SetDefaultView)(gboolean default_vw_bool);
  void (*FreeMainCurlData)();
  void (*StopMultiCurlMain)();
  void (*StopMultiCurlAll)();
  gboolean (*IsClosed)();
  void (*SetClosed)(gboolean closed_bool);
  gboolean (*IsExitingApp)();
  void (*SetExitingApp)(gboolean exiting_bool);
  gboolean (*IsMainCurlCanceled)();
  void (*SetMainCurlCanceled)(gboolean canceled_bool);
  gdouble (*GetHoursOfUpdates)();
  gdouble (*GetUpdatesPerMinute)();
  gpointer (*GetPrimaryHeadings)();
  gpointer (*GetDefaultHeadings)();
  void (*SaveSqlData)();
  gpointer (*GetWindowData)();
  gpointer (*GetMetaClass)();
  gpointer (*GetMetalClass)();
  gpointer (*GetSymNameMap)();
  void (*SetSymNameMap)(symbol_name_map *sn_map);
  gpointer (*GetEquityFolderClass)();
  gboolean (*IsClockDisplayed)();
  void (*SetClockDisplayed)(gboolean displayed_bool);
  gboolean (*IsIndicesDisplayed)();
  void (*SetIndicesDisplayed)(gboolean displayed_bool);
  gboolean (*IsSnmapDbBusy)();
  void (*SetSnmapDbBusy)(gboolean busy_bool);
  void (*SetSecurityNames)();
};

#endif /* CLASS_TYPES_HEADER_H */