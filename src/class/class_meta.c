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

#include <stdlib.h>
#include <string.h>

#include <pwd.h>
#include <unistd.h>

#include "../include/class_types.h" /* Includes portfolio_packet, metal, meta, 
                                             and equity_folder class types */

#include "../include/csv.h"
#include "../include/macros.h"
#include "../include/multicurl.h"
#include "../include/mutex.h"
#include "../include/workfuncs.h"

/* The static file-global variable 'MetaClassObject' is always accessed via
 * these functions. */
/* This is an ad-hoc way of self referencing a class.
   It prevents multiple instances of the meta class. */

static meta
    *MetaClassObject; /* A class object pointer called MetaClassObject. */

/* Class Method (also called Function) Definitions */
static void ToStringsPortfolio() {
  meta *Met = MetaClassObject;

  /* The cash value. */
  DoubleToFormattedStrPango(&Met->cash_mrkd_ch, Met->cash_f,
                            Met->decimal_places_shrt, MON_STR, BLACK);

  /* The total portfolio value. */
  DoubleToFormattedStrPango(&Met->portfolio_port_value_mrkd_ch,
                            Met->portfolio_port_value_f,
                            Met->decimal_places_shrt, MON_STR, BLACK);

  if (Met->portfolio_port_value_chg_f > 0) {

    /* The change in total portfolio value. */
    DoubleToFormattedStrPango(&Met->portfolio_port_value_chg_mrkd_ch,
                              Met->portfolio_port_value_chg_f,
                              Met->decimal_places_shrt, MON_STR, GREEN);

    /* The change in total portfolio value as a percentage. */
    DoubleToFormattedStrPango(&Met->portfolio_port_value_p_chg_mrkd_ch,
                              Met->portfolio_port_value_p_chg_f,
                              Met->decimal_places_shrt, PER_STR, GREEN);

  } else if (Met->portfolio_port_value_chg_f < 0) {

    DoubleToFormattedStrPango(&Met->portfolio_port_value_chg_mrkd_ch,
                              Met->portfolio_port_value_chg_f,
                              Met->decimal_places_shrt, MON_STR, RED);

    DoubleToFormattedStrPango(&Met->portfolio_port_value_p_chg_mrkd_ch,
                              Met->portfolio_port_value_p_chg_f,
                              Met->decimal_places_shrt, PER_STR, RED);

  } else {

    DoubleToFormattedStrPango(&Met->portfolio_port_value_chg_mrkd_ch,
                              Met->portfolio_port_value_chg_f,
                              Met->decimal_places_shrt, MON_STR, BLACK);

    DoubleToFormattedStrPango(&Met->portfolio_port_value_p_chg_mrkd_ch,
                              Met->portfolio_port_value_p_chg_f,
                              Met->decimal_places_shrt, PER_STR, BLACK);
  }
}

static void CalculatePortfolio(void *data) {
  portfolio_packet *pkg = (portfolio_packet *)data;
  meta *Met = MetaClassObject;
  metal *M = pkg->GetMetalClass();
  equity_folder *F = pkg->GetEquityFolderClass();

  /* The total portfolio value. */
  Met->portfolio_port_value_f =
      M->bullion_port_value_f + F->stock_port_value_f + Met->cash_f;

  /* The change in total portfolio value. */
  /* Edit the next line as needed, if you want to
     add a change value besides equity and bullion to the portfolio. */
  Met->portfolio_port_value_chg_f =
      F->stock_port_value_chg_f + M->bullion_port_value_chg_f;

  /* The change in total portfolio value as a percentage. */
  double prev_total =
      Met->portfolio_port_value_f - Met->portfolio_port_value_chg_f;
  Met->portfolio_port_value_p_chg_f =
      CalcGain(Met->portfolio_port_value_f, prev_total);
}

static void StopHistoryCurl() {
  meta *Met = MetaClassObject;

  curl_multi_wakeup(Met->multicurl_history_hnd);
  pthread_mutex_lock(&mutex_working[MULTICURL_NO_PROG_MUTEX]);

  curl_multi_remove_handle(Met->multicurl_history_hnd, Met->history_hnd);

  pthread_mutex_unlock(&mutex_working[MULTICURL_NO_PROG_MUTEX]);
}

static void StopSNMapCurl() {
  meta *Met = MetaClassObject;

  curl_multi_wakeup(Met->multicurl_cmpltn_hnd);
  pthread_mutex_lock(&mutex_working[MULTICURL_NO_PROG_MUTEX]);

  curl_multi_remove_handle(Met->multicurl_cmpltn_hnd,
                           Met->NASDAQ_completion_hnd);
  curl_multi_remove_handle(Met->multicurl_cmpltn_hnd, Met->NYSE_completion_hnd);

  pthread_mutex_unlock(&mutex_working[MULTICURL_NO_PROG_MUTEX]);
}

static int SetUpCurlIndicesData(void *data) {
  portfolio_packet *pkg = (portfolio_packet *)data;
  meta *Met = MetaClassObject;
  char *dow_url_ch = NULL, *nasdaq_url_ch = NULL, *sp_url_ch = NULL,
       *bitcoin_url_ch = NULL;

  /* The start time needs to be a few days before the current time, so minus
     seven days This compensates for weekends and holidays and ensures enough
     data. */
  unsigned int period = (86400 * 7);
  GetYahooUrl(&dow_url_ch, "^dji", period);
  GetYahooUrl(&nasdaq_url_ch, "^ixic", period);
  GetYahooUrl(&sp_url_ch, "^gspc", period);
  GetYahooUrl(&bitcoin_url_ch, "btc-usd", period);

  SetUpCurlHandle(Met->index_dow_hnd, pkg->multicurl_main_hnd, dow_url_ch,
                  &Met->INDEX_DOW_CURLDATA);
  SetUpCurlHandle(Met->index_nasdaq_hnd, pkg->multicurl_main_hnd, nasdaq_url_ch,
                  &Met->INDEX_NASDAQ_CURLDATA);
  SetUpCurlHandle(Met->index_sp_hnd, pkg->multicurl_main_hnd, sp_url_ch,
                  &Met->INDEX_SP_CURLDATA);
  SetUpCurlHandle(Met->crypto_bitcoin_hnd, pkg->multicurl_main_hnd,
                  bitcoin_url_ch, &Met->CRYPTO_BITCOIN_CURLDATA);

  free(dow_url_ch);
  free(nasdaq_url_ch);
  free(sp_url_ch);
  free(bitcoin_url_ch);

  return 0;
}

static void extract_index_data(char *index, MemType *Data) {
  meta *Met = MetaClassObject;

  if (Data->memory == NULL) {
    if (strcmp(index, "dow") == 0) {
      Met->index_dow_value_f = 0.0f;
      Met->index_dow_value_chg_f = 0.0f;
      Met->index_dow_value_p_chg_f = 0.0f;
    }

    if (strcmp(index, "nasdaq") == 0) {
      Met->index_nasdaq_value_f = 0.0f;
      Met->index_nasdaq_value_chg_f = 0.0f;
      Met->index_nasdaq_value_p_chg_f = 0.0f;
    }

    if (strcmp(index, "sp") == 0) {
      Met->index_sp_value_f = 0.0f;
      Met->index_sp_value_chg_f = 0.0f;
      Met->index_sp_value_p_chg_f = 0.0f;
    }

    if (strcmp(index, "bitcoin") == 0) {
      Met->crypto_bitcoin_value_f = 0.0f;
      Met->crypto_bitcoin_value_chg_f = 0.0f;
      Met->crypto_bitcoin_value_p_chg_f = 0.0f;
    }
    return;
  }

  /* Convert a String to a File Pointer Stream for Reading */
  FILE *fp = fmemopen((void *)Data->memory, strlen(Data->memory) + 1, "r");

  if (fp == NULL) {
    if (strcmp(index, "dow") == 0) {
      Met->index_dow_value_f = 0.0f;
      Met->index_dow_value_chg_f = 0.0f;
      Met->index_dow_value_p_chg_f = 0.0f;
    }

    if (strcmp(index, "nasdaq") == 0) {
      Met->index_nasdaq_value_f = 0.0f;
      Met->index_nasdaq_value_chg_f = 0.0f;
      Met->index_nasdaq_value_p_chg_f = 0.0f;
    }

    if (strcmp(index, "sp") == 0) {
      Met->index_sp_value_f = 0.0f;
      Met->index_sp_value_chg_f = 0.0f;
      Met->index_sp_value_p_chg_f = 0.0f;
    }

    if (strcmp(index, "bitcoin") == 0) {
      Met->crypto_bitcoin_value_f = 0.0f;
      Met->crypto_bitcoin_value_chg_f = 0.0f;
      Met->crypto_bitcoin_value_p_chg_f = 0.0f;
    }

    FreeMemtype(Data);
    return;
  }

  double prev_closing = 0.0f, cur_price = 0.0f;
  char *line = ExtractYahooData(fp, &prev_closing, &cur_price);

  if (strcmp(index, "dow") == 0) {
    Met->index_dow_value_f = cur_price;
    Met->index_dow_value_chg_f = cur_price - prev_closing;
    Met->index_dow_value_p_chg_f = CalcGain(cur_price, prev_closing);
  }
  if (strcmp(index, "nasdaq") == 0) {
    Met->index_nasdaq_value_f = cur_price;
    Met->index_nasdaq_value_chg_f = cur_price - prev_closing;
    Met->index_nasdaq_value_p_chg_f = CalcGain(cur_price, prev_closing);
  }
  if (strcmp(index, "sp") == 0) {
    Met->index_sp_value_f = cur_price;
    Met->index_sp_value_chg_f = cur_price - prev_closing;
    Met->index_sp_value_p_chg_f = CalcGain(cur_price, prev_closing);
  }

  if (strcmp(index, "bitcoin") == 0) {
    Met->crypto_bitcoin_value_f = cur_price;
    Met->crypto_bitcoin_value_chg_f = cur_price - prev_closing;
    Met->crypto_bitcoin_value_p_chg_f = CalcGain(cur_price, prev_closing);
  }

  fclose(fp);
  free(line);
  FreeMemtype(Data);
}

static void ExtractIndicesData() {
  meta *Met = MetaClassObject;

  extract_index_data("dow", &Met->INDEX_DOW_CURLDATA);
  extract_index_data("nasdaq", &Met->INDEX_NASDAQ_CURLDATA);
  extract_index_data("sp", &Met->INDEX_SP_CURLDATA);
  extract_index_data("bitcoin", &Met->CRYPTO_BITCOIN_CURLDATA);
}

static void ToStringsIndices() {
  meta *Met = MetaClassObject;
  /* Note that these aren't pango formatted.
     The pango tags are added in set_indices_labels()
     in gui_main.c.

     Formatting labels has a slightly different process than treeviews. */
  DoubleToFormattedStr(&Met->index_dow_value_ch, Met->index_dow_value_f, 2,
                       NUM_STR);
  DoubleToFormattedStr(&Met->index_dow_value_chg_ch, Met->index_dow_value_chg_f,
                       2, NUM_STR);
  DoubleToFormattedStr(&Met->index_dow_value_p_chg_ch,
                       Met->index_dow_value_p_chg_f, 2, PER_STR);

  DoubleToFormattedStr(&Met->index_nasdaq_value_ch, Met->index_nasdaq_value_f,
                       2, NUM_STR);
  DoubleToFormattedStr(&Met->index_nasdaq_value_chg_ch,
                       Met->index_nasdaq_value_chg_f, 2, NUM_STR);
  DoubleToFormattedStr(&Met->index_nasdaq_value_p_chg_ch,
                       Met->index_nasdaq_value_p_chg_f, 2, PER_STR);

  DoubleToFormattedStr(&Met->index_sp_value_ch, Met->index_sp_value_f, 2,
                       NUM_STR);
  DoubleToFormattedStr(&Met->index_sp_value_chg_ch, Met->index_sp_value_chg_f,
                       2, NUM_STR);
  DoubleToFormattedStr(&Met->index_sp_value_p_chg_ch,
                       Met->index_sp_value_p_chg_f, 2, PER_STR);

  DoubleToFormattedStr(&Met->crypto_bitcoin_value_ch,
                       Met->crypto_bitcoin_value_f, 2, MON_STR);
  DoubleToFormattedStr(&Met->crypto_bitcoin_value_chg_ch,
                       Met->crypto_bitcoin_value_chg_f, 2, MON_STR);
  DoubleToFormattedStr(&Met->crypto_bitcoin_value_p_chg_ch,
                       Met->crypto_bitcoin_value_p_chg_f, 2, PER_STR);
}

/* The order of the strings, in these struct inits, is important,
   they match similar names within the struct definitions
   (in class_types.h). */
static primary_heading primary_headings = {
    "Bullion",  "Metal",     "Ounces", "Premium",   "High",
    "Low",      "Pr. Close", "Chg",    "Gain ($)",  "Total",
    "Gain (%)", "Gold",      "Silver", "Platinum",  "Palladium",
    "Equity",   "Symbol",    "Shares", "Price",     "Open",
    "Asset",    "Value",     "Cash",   "Portfolio", "Portfolio has no assets."};

static default_heading default_headings = {"Bullion",
                                           "Metal",
                                           "Ounces",
                                           "Premium",
                                           "Gold",
                                           "Silver",
                                           "Platinum",
                                           "Palladium",
                                           "Equity",
                                           "Symbol",
                                           "Shares",
                                           "Cash",
                                           "Portfolio has no assets."};

static void format_primary_headings_pango(primary_heading *pri_h_mkd) {

  StringToStrPango(&pri_h_mkd->metal, primary_headings.metal,
                   HEADING_UNLN_FORMAT);
  StringToStrPango(&pri_h_mkd->ounces, primary_headings.ounces,
                   HEADING_UNLN_FORMAT);
  StringToStrPango(&pri_h_mkd->price, primary_headings.price,
                   HEADING_UNLN_FORMAT);
  StringToStrPango(&pri_h_mkd->premium, primary_headings.premium,
                   HEADING_UNLN_FORMAT);
  StringToStrPango(&pri_h_mkd->high, primary_headings.high,
                   HEADING_UNLN_FORMAT);
  StringToStrPango(&pri_h_mkd->low, primary_headings.low, HEADING_UNLN_FORMAT);
  StringToStrPango(&pri_h_mkd->prev_closing, primary_headings.prev_closing,
                   HEADING_UNLN_FORMAT);
  StringToStrPango(&pri_h_mkd->chg, primary_headings.chg, HEADING_UNLN_FORMAT);
  StringToStrPango(&pri_h_mkd->gain_sym, primary_headings.gain_sym,
                   HEADING_UNLN_FORMAT);
  StringToStrPango(&pri_h_mkd->total, primary_headings.total,
                   HEADING_UNLN_FORMAT);
  StringToStrPango(&pri_h_mkd->gain_per, primary_headings.gain_per,
                   HEADING_UNLN_FORMAT);

  StringToStrPango(&pri_h_mkd->symbol, primary_headings.symbol,
                   HEADING_UNLN_FORMAT);
  StringToStrPango(&pri_h_mkd->shares, primary_headings.shares,
                   HEADING_UNLN_FORMAT);
  StringToStrPango(&pri_h_mkd->opening, primary_headings.opening,
                   HEADING_UNLN_FORMAT);

  StringToStrPango(&pri_h_mkd->asset, primary_headings.asset,
                   HEADING_UNLN_FORMAT);
  StringToStrPango(&pri_h_mkd->value, primary_headings.value,
                   HEADING_UNLN_FORMAT);

  StringToStrPango(&pri_h_mkd->cash, primary_headings.cash,
                   HEADING_ASST_TYPE_FORMAT);
  StringToStrPango(&pri_h_mkd->bullion, primary_headings.bullion,
                   HEADING_ASST_TYPE_FORMAT);
  StringToStrPango(&pri_h_mkd->equity, primary_headings.equity,
                   HEADING_ASST_TYPE_FORMAT);
  StringToStrPango(&pri_h_mkd->portfolio, primary_headings.portfolio,
                   HEADING_ASST_TYPE_FORMAT);

  StringToStrPango(&pri_h_mkd->gold, primary_headings.gold, BLUE);
  StringToStrPango(&pri_h_mkd->silver, primary_headings.silver, BLUE);
  StringToStrPango(&pri_h_mkd->platinum, primary_headings.platinum, BLUE);
  StringToStrPango(&pri_h_mkd->palladium, primary_headings.palladium, BLUE);
  StringToStrPango(&pri_h_mkd->no_assets, primary_headings.no_assets, BLUE);
}

static void format_default_headings_pango(default_heading *def_h_mkd) {

  StringToStrPango(&def_h_mkd->metal, default_headings.metal,
                   HEADING_UNLN_FORMAT);
  StringToStrPango(&def_h_mkd->ounces, default_headings.ounces,
                   HEADING_UNLN_FORMAT);
  StringToStrPango(&def_h_mkd->premium, default_headings.premium,
                   HEADING_UNLN_FORMAT);
  StringToStrPango(&def_h_mkd->symbol, default_headings.symbol,
                   HEADING_UNLN_FORMAT);
  StringToStrPango(&def_h_mkd->shares, default_headings.shares,
                   HEADING_UNLN_FORMAT);

  StringToStrPango(&def_h_mkd->bullion, default_headings.bullion,
                   HEADING_ASST_TYPE_FORMAT);
  StringToStrPango(&def_h_mkd->equity, default_headings.equity,
                   HEADING_ASST_TYPE_FORMAT);
  StringToStrPango(&def_h_mkd->cash, default_headings.cash,
                   HEADING_ASST_TYPE_FORMAT);

  StringToStrPango(&def_h_mkd->gold, default_headings.gold, BLUE);
  StringToStrPango(&def_h_mkd->silver, default_headings.silver, BLUE);
  StringToStrPango(&def_h_mkd->platinum, default_headings.platinum, BLUE);
  StringToStrPango(&def_h_mkd->palladium, default_headings.palladium, BLUE);
  StringToStrPango(&def_h_mkd->no_assets, default_headings.no_assets, BLUE);
}

static void free_primary_headings(primary_heading *pri_h_mkd) {
  free(pri_h_mkd->bullion);
  free(pri_h_mkd->metal);
  free(pri_h_mkd->ounces);

  free(pri_h_mkd->premium);
  free(pri_h_mkd->high);
  free(pri_h_mkd->low);

  free(pri_h_mkd->prev_closing);
  free(pri_h_mkd->chg);
  free(pri_h_mkd->gain_sym);

  free(pri_h_mkd->total);
  free(pri_h_mkd->gain_per);
  free(pri_h_mkd->gold);

  free(pri_h_mkd->silver);
  free(pri_h_mkd->platinum);
  free(pri_h_mkd->palladium);

  free(pri_h_mkd->equity);
  free(pri_h_mkd->symbol);
  free(pri_h_mkd->shares);

  free(pri_h_mkd->price);
  free(pri_h_mkd->opening);
  free(pri_h_mkd->asset);

  free(pri_h_mkd->value);
  free(pri_h_mkd->cash);
  free(pri_h_mkd->portfolio);

  free(pri_h_mkd->no_assets);
}

static void free_default_headings(default_heading *def_h_mkd) {
  free(def_h_mkd->bullion);
  free(def_h_mkd->metal);
  free(def_h_mkd->ounces);

  free(def_h_mkd->premium);
  free(def_h_mkd->gold);
  free(def_h_mkd->silver);

  free(def_h_mkd->platinum);
  free(def_h_mkd->palladium);
  free(def_h_mkd->equity);

  free(def_h_mkd->symbol);
  free(def_h_mkd->shares);
}

static void ToStringsHeadings() {
  meta *D = MetaClassObject;
  format_default_headings_pango(&D->def_h_mkd);
  format_primary_headings_pango(&D->pri_h_mkd);
}

/* Class Init Functions */
meta *ClassInitMeta() {
  /* Allocate Memory For A New Class Object */
  meta *new_class = (meta *)malloc(sizeof(*new_class));

  /* Initialize Variables */
  new_class->sym_map = NULL;
  new_class->rght_clk_data = (right_click_container){NULL};

  /* Initialize the main and rsi window size and locations to zero */
  new_class->window_struct = (window_data){0};

  new_class->stock_url_ch = strdup(FINNHUB_URL);
  new_class->curl_key_ch = strdup(FINNHUB_URL_TOKEN);
  new_class->Nasdaq_Symbol_url_ch = strdup(NASDAQ_SYMBOL_URL);
  new_class->NYSE_Symbol_url_ch = strdup(NYSE_SYMBOL_URL);

  new_class->cash_mrkd_ch = NULL;
  new_class->portfolio_port_value_mrkd_ch = NULL;
  new_class->portfolio_port_value_chg_mrkd_ch = NULL;
  new_class->portfolio_port_value_p_chg_mrkd_ch = NULL;

  new_class->index_dow_value_ch = NULL;
  new_class->index_dow_value_chg_ch = NULL;
  new_class->index_dow_value_p_chg_ch = NULL;

  new_class->index_nasdaq_value_ch = NULL;
  new_class->index_nasdaq_value_chg_ch = NULL;
  new_class->index_nasdaq_value_p_chg_ch = NULL;

  new_class->index_sp_value_ch = NULL;
  new_class->index_sp_value_chg_ch = NULL;
  new_class->index_sp_value_p_chg_ch = NULL;

  new_class->crypto_bitcoin_value_ch = NULL;
  new_class->crypto_bitcoin_value_chg_ch = NULL;
  new_class->crypto_bitcoin_value_p_chg_ch = NULL;

  /* Set up the main treeview font */
  new_class->main_font_ch = strdup(MAIN_FONT);
  SetFont(new_class->main_font_ch);

  /* The pango funcs require each dest string to point to allocated space
                or NULL, they use realloc ( and possibly malloc if NULL ). */
  new_class->pri_h_mkd = (primary_heading){NULL};
  new_class->def_h_mkd = (default_heading){NULL};

  new_class->cash_f = 0.0f;
  new_class->portfolio_port_value_f = 0.0f;
  new_class->portfolio_port_value_chg_f = 0.0f;
  new_class->portfolio_port_value_p_chg_f = 0.0f;
  new_class->updates_per_min_f = 6.0f;
  new_class->updates_hours_f = 1.0f;

  new_class->decimal_places_shrt = 2;

  new_class->fetching_data_bool = false;
  new_class->holiday_bool = false;
  new_class->multicurl_cancel_bool = false;
  new_class->multicurl_cancel_main_bool = false;
  new_class->index_bar_revealed_bool = true;
  new_class->clocks_displayed_bool = true;
  new_class->main_win_default_view_bool = true;

  new_class->history_hnd = curl_easy_init();
  new_class->NASDAQ_completion_hnd = curl_easy_init();
  new_class->NYSE_completion_hnd = curl_easy_init();

  new_class->index_dow_hnd = curl_easy_init();
  new_class->index_nasdaq_hnd = curl_easy_init();
  new_class->index_sp_hnd = curl_easy_init();
  new_class->crypto_bitcoin_hnd = curl_easy_init();

  new_class->multicurl_cmpltn_hnd = curl_multi_init();
  new_class->multicurl_history_hnd = curl_multi_init();

  new_class->INDEX_DOW_CURLDATA.memory = NULL;
  new_class->INDEX_DOW_CURLDATA.size = 0;
  new_class->INDEX_NASDAQ_CURLDATA.memory = NULL;
  new_class->INDEX_NASDAQ_CURLDATA.size = 0;
  new_class->INDEX_SP_CURLDATA.memory = NULL;
  new_class->INDEX_SP_CURLDATA.size = 0;
  new_class->CRYPTO_BITCOIN_CURLDATA.memory = NULL;
  new_class->CRYPTO_BITCOIN_CURLDATA.size = 0;

  new_class->thread_id_clock = 0;
  new_class->thread_id_closing_time = 0;
  new_class->thread_id_main_fetch_data = 0;

  /* Set The User's Home Directory */
  /* We need to get the path to the User's home directory: */
  /* Get User ID (need to #include<pwd.h> and #include<unistd.h> for this) */
  uid_t uid = getuid();

  /* Get User Account Information For This ID */
  struct passwd *pw = getpwuid(uid);

  if (pw == NULL) {
    printf("Gathering User Account Info Failed\n");
    exit(EXIT_FAILURE);
  }

  new_class->home_dir_ch = strdup(pw->pw_dir);

  /* Append the sqlite db file paths to the end of the home directory path. */
  size_t len = snprintf(NULL, 0, "%s%s", pw->pw_dir, DB_FILE) + 1;
  new_class->sqlite_db_path_ch = (char *)malloc(len);
  snprintf(new_class->sqlite_db_path_ch, len, "%s%s", pw->pw_dir, DB_FILE);

  len = snprintf(NULL, 0, "%s%s", pw->pw_dir, SN_DB_FILE) + 1;
  new_class->sqlite_symbol_name_db_path_ch = (char *)malloc(len);
  snprintf(new_class->sqlite_symbol_name_db_path_ch, len, "%s%s", pw->pw_dir,
           SN_DB_FILE);

  /* Connect Function Pointers To Function Definitions */
  new_class->ToStringsPortfolio = ToStringsPortfolio;
  new_class->CalculatePortfolio = CalculatePortfolio;
  new_class->StopHistoryCurl = StopHistoryCurl;
  new_class->StopSNMapCurl = StopSNMapCurl;
  new_class->SetUpCurlIndicesData = SetUpCurlIndicesData;
  new_class->ExtractIndicesData = ExtractIndicesData;
  new_class->ToStringsIndices = ToStringsIndices;
  new_class->ToStringsHeadings = ToStringsHeadings;

  /* Set the file global variable so we can self-reference this class. */
  MetaClassObject = new_class;

  /* Return Our Initialized Class */
  return new_class;
}

/* Class Destruct Functions */
void ClassDestructMeta(meta *meta_class) {
  /* Free Memory */
  free_primary_headings(&meta_class->pri_h_mkd);
  free_default_headings(&meta_class->def_h_mkd);

  if (meta_class->rght_clk_data.type)
    free(meta_class->rght_clk_data.type);
  if (meta_class->rght_clk_data.symbol)
    free(meta_class->rght_clk_data.symbol);
  /* rght_clk_data is a variable and not a pointer, do not free.
     fyi; both variables and pointers are in heap memory, the variables are
     allocated with the struct pointer and freed with the struct pointer. */

  /* Free the symbol to security name mapping array. */
  pthread_mutex_lock(&mutex_working[SYMBOL_NAME_MAP_MUTEX]);

  if (meta_class->sym_map) {
    SNMapDestruct(meta_class->sym_map);
    free(meta_class->sym_map);
    meta_class->sym_map = NULL;
  }

  pthread_mutex_unlock(&mutex_working[SYMBOL_NAME_MAP_MUTEX]);

  if (meta_class->stock_url_ch)
    free(meta_class->stock_url_ch);
  if (meta_class->curl_key_ch)
    free(meta_class->curl_key_ch);
  if (meta_class->Nasdaq_Symbol_url_ch)
    free(meta_class->Nasdaq_Symbol_url_ch);
  if (meta_class->NYSE_Symbol_url_ch)
    free(meta_class->NYSE_Symbol_url_ch);

  if (meta_class->cash_mrkd_ch)
    free(meta_class->cash_mrkd_ch);
  if (meta_class->portfolio_port_value_mrkd_ch)
    free(meta_class->portfolio_port_value_mrkd_ch);
  if (meta_class->portfolio_port_value_chg_mrkd_ch)
    free(meta_class->portfolio_port_value_chg_mrkd_ch);
  if (meta_class->portfolio_port_value_p_chg_mrkd_ch)
    free(meta_class->portfolio_port_value_p_chg_mrkd_ch);

  if (meta_class->index_dow_value_ch)
    free(meta_class->index_dow_value_ch);
  if (meta_class->index_dow_value_chg_ch)
    free(meta_class->index_dow_value_chg_ch);
  if (meta_class->index_dow_value_p_chg_ch)
    free(meta_class->index_dow_value_p_chg_ch);

  if (meta_class->index_nasdaq_value_ch)
    free(meta_class->index_nasdaq_value_ch);
  if (meta_class->index_nasdaq_value_chg_ch)
    free(meta_class->index_nasdaq_value_chg_ch);
  if (meta_class->index_nasdaq_value_p_chg_ch)
    free(meta_class->index_nasdaq_value_p_chg_ch);

  if (meta_class->index_sp_value_ch)
    free(meta_class->index_sp_value_ch);
  if (meta_class->index_sp_value_chg_ch)
    free(meta_class->index_sp_value_chg_ch);
  if (meta_class->index_sp_value_p_chg_ch)
    free(meta_class->index_sp_value_p_chg_ch);

  if (meta_class->crypto_bitcoin_value_ch)
    free(meta_class->crypto_bitcoin_value_ch);
  if (meta_class->crypto_bitcoin_value_chg_ch)
    free(meta_class->crypto_bitcoin_value_chg_ch);
  if (meta_class->crypto_bitcoin_value_p_chg_ch)
    free(meta_class->crypto_bitcoin_value_p_chg_ch);

  if (meta_class->main_font_ch)
    free(meta_class->main_font_ch);

  if (meta_class->home_dir_ch)
    free(meta_class->home_dir_ch);
  if (meta_class->sqlite_db_path_ch)
    free(meta_class->sqlite_db_path_ch);
  if (meta_class->sqlite_symbol_name_db_path_ch)
    free(meta_class->sqlite_symbol_name_db_path_ch);

  if (meta_class->NASDAQ_completion_hnd)
    curl_easy_cleanup(meta_class->NASDAQ_completion_hnd);
  if (meta_class->NYSE_completion_hnd)
    curl_easy_cleanup(meta_class->NYSE_completion_hnd);
  if (meta_class->history_hnd)
    curl_easy_cleanup(meta_class->history_hnd);

  if (meta_class->index_dow_hnd)
    curl_easy_cleanup(meta_class->index_dow_hnd);
  if (meta_class->index_nasdaq_hnd)
    curl_easy_cleanup(meta_class->index_nasdaq_hnd);
  if (meta_class->index_sp_hnd)
    curl_easy_cleanup(meta_class->index_sp_hnd);
  if (meta_class->crypto_bitcoin_hnd)
    curl_easy_cleanup(meta_class->crypto_bitcoin_hnd);

  if (meta_class->multicurl_cmpltn_hnd)
    curl_multi_cleanup(meta_class->multicurl_cmpltn_hnd);
  if (meta_class->multicurl_history_hnd)
    curl_multi_cleanup(meta_class->multicurl_history_hnd);

  FreeMemtype(&meta_class->INDEX_DOW_CURLDATA);
  FreeMemtype(&meta_class->INDEX_NASDAQ_CURLDATA);
  FreeMemtype(&meta_class->INDEX_SP_CURLDATA);
  FreeMemtype(&meta_class->CRYPTO_BITCOIN_CURLDATA);

  /* Free Memory From Class Object */
  if (meta_class) {
    free(meta_class);
    meta_class = NULL;
  }
}