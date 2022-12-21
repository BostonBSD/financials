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

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <time.h> /* time_t, struct tm, time ()  */

#include <pwd.h>
#include <unistd.h>

#include <locale.h>
#include <monetary.h>

#include "../include/class_types.h" /* Includes portfolio_packet, metal, meta, 
                                             and equity_folder class types */

#include "../include/csv.h"
#include "../include/macros.h"
#include "../include/multicurl.h"
#include "../include/mutex.h"
#include "../include/workfuncs.h"

/* The static local-global variable 'MetaClassObject' is always accessed via
 * these functions. */
/* This is an ad-hoc way of self referencing a class.
   It prevents multiple instances of the meta class. */

static meta
    *MetaClassObject; /* A class object pointer called MetaClassObject. */

/* Class Method (also called Function) Definitions */
static void ToStringsPortfolio() {
  meta *Met = MetaClassObject;

  /* The cash value. */
  DoubToMonStrPango(&Met->cash_mrkd_ch, Met->cash_f, Met->decimal_places_shrt);

  /* The total portfolio value. */
  DoubToMonStrPango(&Met->portfolio_port_value_mrkd_ch,
                    Met->portfolio_port_value_f, Met->decimal_places_shrt);

  /* The change in total portfolio value. */
  DoubToMonStrPangoColor(&Met->portfolio_port_value_chg_mrkd_ch,
                         Met->portfolio_port_value_chg_f,
                         Met->decimal_places_shrt, NOT_ITALIC);

  /* The change in total portfolio value as a percentage. */
  DoubToPerStrPangoColor(&Met->portfolio_port_value_p_chg_mrkd_ch,
                         Met->portfolio_port_value_p_chg_f,
                         Met->decimal_places_shrt, NOT_ITALIC);
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

static void StopRSICurl() {
  meta *Met = MetaClassObject;

  curl_multi_wakeup(Met->multicurl_rsi_hnd);
  pthread_mutex_lock(&mutex_working[MULTICURL_NO_PROG_MUTEX]);

  curl_multi_remove_handle(Met->multicurl_rsi_hnd, Met->rsi_hnd);

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

static void create_index_url(char **url_ch, const char *symbol_ch) {
  time_t end_time, start_time;
  size_t len;

  time(&end_time);
  /* The start time needs to be a few days before the current time, so minus
     four days This compensates for weekends and holidays and ensures enough
     data. */
  start_time = end_time - (86400 * 4);

  len = strlen(symbol_ch) + strlen(YAHOO_URL_START) +
        strlen(YAHOO_URL_MIDDLE_ONE) + strlen(YAHOO_URL_MIDDLE_TWO) +
        strlen(YAHOO_URL_END) + 25;
  char *tmp = realloc(url_ch[0], len);
  url_ch[0] = tmp;
  snprintf(url_ch[0], len,
           YAHOO_URL_START "%s" YAHOO_URL_MIDDLE_ONE "%d" YAHOO_URL_MIDDLE_TWO
                           "%d" YAHOO_URL_END,
           symbol_ch, (int)start_time, (int)end_time);
}

static int SetUpCurlIndicesData(void *data) {
  portfolio_packet *pkg = (portfolio_packet *)data;
  meta *Met = MetaClassObject;
  char *dow_url_ch = NULL, *nasdaq_url_ch = NULL, *sp_url_ch = NULL,
       *bitcoin_url_ch = NULL;

  create_index_url(&dow_url_ch, "^dji");
  create_index_url(&nasdaq_url_ch, "^ixic");
  create_index_url(&sp_url_ch, "^gspc");
  create_index_url(&bitcoin_url_ch, "btc-usd");

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

    free(Data->memory);
    Data->memory = NULL;
    return;
  }

  char line[1024];
  char **csv_array;

  /* Yahoo! updates bitcoin when the equities markets are closed.
     The while loop iterates to the end of file to get the latest data. */
  double prev_closing = 0.0f, cur_price = 0.0f;
  while (fgets(line, 1024, fp) != NULL) {
    prev_closing = cur_price;
    /* Sometimes the API gives us a null value for certain days.
       using the closing price from the day prior gives us a more accurate
       gain value. */
    if (strstr(line, "null") || strstr(line, "Date"))
      continue;
    Chomp(line);
    csv_array = parse_csv(line);
    cur_price = strtod(csv_array[4] ? csv_array[4] : "0", NULL);
    free_csv_line(csv_array);
  };

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
  free(Data->memory);
  Data->memory = NULL;
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

  DoubToNumStr(&Met->index_dow_value_ch, Met->index_dow_value_f, 2);
  DoubToNumStr(&Met->index_dow_value_chg_ch, Met->index_dow_value_chg_f, 2);
  DoubToPerStr(&Met->index_dow_value_p_chg_ch, Met->index_dow_value_p_chg_f, 2);

  DoubToNumStr(&Met->index_nasdaq_value_ch, Met->index_nasdaq_value_f, 2);
  DoubToNumStr(&Met->index_nasdaq_value_chg_ch, Met->index_nasdaq_value_chg_f,
               2);
  DoubToPerStr(&Met->index_nasdaq_value_p_chg_ch,
               Met->index_nasdaq_value_p_chg_f, 2);

  DoubToNumStr(&Met->index_sp_value_ch, Met->index_sp_value_f, 2);
  DoubToNumStr(&Met->index_sp_value_chg_ch, Met->index_sp_value_chg_f, 2);
  DoubToPerStr(&Met->index_sp_value_p_chg_ch, Met->index_sp_value_p_chg_f, 2);

  DoubleToMonStr(&Met->crypto_bitcoin_value_ch, Met->crypto_bitcoin_value_f, 2);
  DoubleToMonStr(&Met->crypto_bitcoin_value_chg_ch,
                 Met->crypto_bitcoin_value_chg_f, 2);
  DoubToPerStr(&Met->crypto_bitcoin_value_p_chg_ch,
               Met->crypto_bitcoin_value_p_chg_f, 2);
}

static primary_heading primary_headings = {
    "Bullion",    "Metal",        "Ounces",
    "Spot Price", "Premium",      "High",
    "Low",        "Prev Closing", "Chg/Ounce",
    "Gain ($)",   "Total",        "Gain (%)",
    "Gold",       "Silver",       "Platinum",
    "Palladium",  "Equity",       "Symbol",
    "Shares",     "Price",        "Opening",
    "Chg/Share",  "Asset",        "Value",
    "Cash",       "Portfolio",    "Portfolio has no assets."};

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

  StrToStrPangoColor(&pri_h_mkd->bullion, primary_headings.bullion, BOLD);
  StrToStrPangoColor(&pri_h_mkd->metal, primary_headings.metal, BOLD_UNDERLINE);
  StrToStrPangoColor(&pri_h_mkd->ounces, primary_headings.ounces,
                     BOLD_UNDERLINE);

  StrToStrPangoColor(&pri_h_mkd->spot_price, primary_headings.spot_price,
                     BOLD_UNDERLINE);
  StrToStrPangoColor(&pri_h_mkd->premium, primary_headings.premium,
                     BOLD_UNDERLINE);
  StrToStrPangoColor(&pri_h_mkd->high, primary_headings.high, BOLD_UNDERLINE);

  StrToStrPangoColor(&pri_h_mkd->low, primary_headings.low, BOLD_UNDERLINE);
  StrToStrPangoColor(&pri_h_mkd->prev_closing, primary_headings.prev_closing,
                     BOLD_UNDERLINE);
  StrToStrPangoColor(&pri_h_mkd->chg_ounce, primary_headings.chg_ounce,
                     BOLD_UNDERLINE);

  StrToStrPangoColor(&pri_h_mkd->gain_sym, primary_headings.gain_sym,
                     BOLD_UNDERLINE);
  StrToStrPangoColor(&pri_h_mkd->total, primary_headings.total, BOLD_UNDERLINE);
  StrToStrPangoColor(&pri_h_mkd->gain_per, primary_headings.gain_per,
                     BOLD_UNDERLINE);

  StrToStrPangoColor(&pri_h_mkd->gold, primary_headings.gold, BOLD);
  StrToStrPangoColor(&pri_h_mkd->silver, primary_headings.silver, BOLD);
  StrToStrPangoColor(&pri_h_mkd->platinum, primary_headings.platinum, BOLD);

  StrToStrPangoColor(&pri_h_mkd->palladium, primary_headings.palladium, BOLD);
  StrToStrPangoColor(&pri_h_mkd->equity, primary_headings.equity, BOLD);
  StrToStrPangoColor(&pri_h_mkd->symbol, primary_headings.symbol,
                     BOLD_UNDERLINE);

  StrToStrPangoColor(&pri_h_mkd->shares, primary_headings.shares,
                     BOLD_UNDERLINE);
  StrToStrPangoColor(&pri_h_mkd->price, primary_headings.price, BOLD_UNDERLINE);
  StrToStrPangoColor(&pri_h_mkd->opening, primary_headings.opening,
                     BOLD_UNDERLINE);

  StrToStrPangoColor(&pri_h_mkd->chg_share, primary_headings.chg_share,
                     BOLD_UNDERLINE);
  StrToStrPangoColor(&pri_h_mkd->asset, primary_headings.asset, BOLD_UNDERLINE);
  StrToStrPangoColor(&pri_h_mkd->value, primary_headings.value, BOLD_UNDERLINE);

  StrToStrPangoColor(&pri_h_mkd->cash, primary_headings.cash, BOLD);
  StrToStrPangoColor(&pri_h_mkd->portfolio, primary_headings.portfolio, BOLD);
  StrToStrPangoColor(&pri_h_mkd->no_assets, primary_headings.no_assets, BLUE);
}

static void format_default_headings_pango(default_heading *def_h_mkd) {

  StrToStrPangoColor(&def_h_mkd->bullion, default_headings.bullion, BOLD);
  StrToStrPangoColor(&def_h_mkd->metal, default_headings.metal, BOLD_UNDERLINE);
  StrToStrPangoColor(&def_h_mkd->ounces, default_headings.ounces,
                     BOLD_UNDERLINE);
  StrToStrPangoColor(&def_h_mkd->premium, default_headings.premium,
                     BOLD_UNDERLINE);
  StrToStrPangoColor(&def_h_mkd->gold, default_headings.gold, BOLD);
  StrToStrPangoColor(&def_h_mkd->silver, default_headings.silver, BOLD);
  StrToStrPangoColor(&def_h_mkd->platinum, default_headings.platinum, BOLD);
  StrToStrPangoColor(&def_h_mkd->palladium, default_headings.palladium, BOLD);

  StrToStrPangoColor(&def_h_mkd->equity, default_headings.equity, BOLD);
  StrToStrPangoColor(&def_h_mkd->symbol, default_headings.symbol,
                     BOLD_UNDERLINE);
  StrToStrPangoColor(&def_h_mkd->shares, default_headings.shares,
                     BOLD_UNDERLINE);

  StrToStrPangoColor(&def_h_mkd->cash, default_headings.cash, BOLD);
  StrToStrPangoColor(&def_h_mkd->no_assets, default_headings.no_assets, BLUE);
}

static void free_primary_headings(primary_heading *pri_h_mkd) {
  free(pri_h_mkd->bullion);
  free(pri_h_mkd->metal);
  free(pri_h_mkd->ounces);

  free(pri_h_mkd->spot_price);
  free(pri_h_mkd->premium);
  free(pri_h_mkd->high);

  free(pri_h_mkd->low);
  free(pri_h_mkd->prev_closing);
  free(pri_h_mkd->chg_ounce);

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

  free(pri_h_mkd->chg_share);
  free(pri_h_mkd->asset);
  free(pri_h_mkd->value);

  free(pri_h_mkd->cash);
  free(pri_h_mkd->portfolio);
  free(pri_h_mkd->no_assets);

  free(pri_h_mkd);
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

  free(def_h_mkd);
}

/* Class Init Functions */
meta *ClassInitMeta() {
  /* Allocate Memory For A New Class Object */
  meta *new_class = (meta *)malloc(sizeof(*new_class));

  /* Initialize Variables */
  new_class->rght_clk_data = (right_click_container){NULL};

  new_class->window_struct = (window_data *)malloc(sizeof(*new_class->window_struct));
  new_class->sym_map = NULL;

  new_class->pri_h_mkd = malloc(sizeof *new_class->pri_h_mkd);
  new_class->def_h_mkd = malloc(sizeof *new_class->def_h_mkd);

  /* The pango funcs require each dest string to point to allocated space
                or NULL, they use realloc ( and possibly malloc if NULL ). */
  *new_class->pri_h_mkd = (primary_heading){NULL};
  *new_class->def_h_mkd = (default_heading){NULL};

  format_primary_headings_pango(new_class->pri_h_mkd);
  format_default_headings_pango(new_class->def_h_mkd);

  new_class->stock_url_ch = strdup(FINNHUB_URL);
  new_class->curl_key_ch = strdup(FINNHUB_URL_TOKEN);
  new_class->Nasdaq_Symbol_url_ch = strdup(NASDAQ_SYMBOL_URL);
  new_class->NYSE_Symbol_url_ch = strdup(NYSE_SYMBOL_URL);

  new_class->cash_mrkd_ch = strdup("$0.00");
  new_class->portfolio_port_value_mrkd_ch = strdup("$0.00");
  new_class->portfolio_port_value_chg_mrkd_ch = strdup("$0.00");
  new_class->portfolio_port_value_p_chg_mrkd_ch = strdup("000.000%");

  new_class->index_dow_value_ch = strdup("0.00");
  new_class->index_dow_value_chg_ch = strdup("0.00");
  new_class->index_dow_value_p_chg_ch = strdup("000.000%");

  new_class->index_nasdaq_value_ch = strdup("0.00");
  new_class->index_nasdaq_value_chg_ch = strdup("0.00");
  new_class->index_nasdaq_value_p_chg_ch = strdup("000.000%");

  new_class->index_sp_value_ch = strdup("0.00");
  new_class->index_sp_value_chg_ch = strdup("0.00");
  new_class->index_sp_value_p_chg_ch = strdup("000.000%");

  new_class->crypto_bitcoin_value_ch = strdup("$0.00");
  new_class->crypto_bitcoin_value_chg_ch = strdup("$0.00");
  new_class->crypto_bitcoin_value_p_chg_ch = strdup("000.000%");

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
  new_class->index_bar_revealed_bool = true;
  new_class->clocks_displayed_bool = true;
  new_class->main_win_default_view_bool = true;

  new_class->rsi_hnd = curl_easy_init();
  new_class->NASDAQ_completion_hnd = curl_easy_init();
  new_class->NYSE_completion_hnd = curl_easy_init();

  new_class->index_dow_hnd = curl_easy_init();
  new_class->index_nasdaq_hnd = curl_easy_init();
  new_class->index_sp_hnd = curl_easy_init();
  new_class->crypto_bitcoin_hnd = curl_easy_init();

  new_class->multicurl_cmpltn_hnd = curl_multi_init();
  new_class->multicurl_rsi_hnd = curl_multi_init();

  new_class->INDEX_DOW_CURLDATA.memory = NULL;
  new_class->INDEX_NASDAQ_CURLDATA.memory = NULL;
  new_class->INDEX_SP_CURLDATA.memory = NULL;
  new_class->CRYPTO_BITCOIN_CURLDATA.memory = NULL;

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
  size_t len = strlen(pw->pw_dir) + strlen(DB_FILE) + 1;
  new_class->sqlite_db_path_ch = (char *)malloc(len);
  snprintf(new_class->sqlite_db_path_ch, len, "%s%s", pw->pw_dir, DB_FILE);

  len = strlen(pw->pw_dir) + strlen(SN_DB_FILE) + 1;
  new_class->sqlite_symbol_name_db_path_ch = (char *)malloc(len);
  snprintf(new_class->sqlite_symbol_name_db_path_ch, len, "%s%s", pw->pw_dir,
           SN_DB_FILE);

  /* Initialize the main and rsi window size and locations */
  new_class->window_struct->main_height = 0;
  new_class->window_struct->main_width = 0;
  new_class->window_struct->main_x_pos = 0;
  new_class->window_struct->main_y_pos = 0;
  new_class->window_struct->rsi_height = 0;
  new_class->window_struct->rsi_width = 0;
  new_class->window_struct->rsi_x_pos = 0;
  new_class->window_struct->rsi_y_pos = 0;

  /* Connect Function Pointers To Function Definitions */
  new_class->ToStringsPortfolio = ToStringsPortfolio;
  new_class->CalculatePortfolio = CalculatePortfolio;
  new_class->StopRSICurl = StopRSICurl;
  new_class->StopSNMapCurl = StopSNMapCurl;
  new_class->SetUpCurlIndicesData = SetUpCurlIndicesData;
  new_class->ExtractIndicesData = ExtractIndicesData;
  new_class->ToStringsIndices = ToStringsIndices;

  /* Set the file global variable so we can self-reference this class. */
  MetaClassObject = new_class;

  /* Return Our Initialized Class */
  return new_class;
}

/* Class Destruct Functions */
void ClassDestructMeta(meta *meta_class) {
  /* Free Memory */
  if (meta_class->rght_clk_data.type)
    free(meta_class->rght_clk_data.type);
  if (meta_class->rght_clk_data.symbol)
    free(meta_class->rght_clk_data.symbol);
  /* rght_clk_data is a variable and not a pointer, do not free */

  if (meta_class->window_struct)
    free(meta_class->window_struct);

  /* Free the symbol to security name mapping array. */
  pthread_mutex_lock(&mutex_working[SYMBOL_NAME_MAP_MUTEX]);

  if (meta_class->sym_map) {
    SNMapDestruct(meta_class->sym_map);
    free(meta_class->sym_map);
    meta_class->sym_map = NULL;
  }

  pthread_mutex_unlock(&mutex_working[SYMBOL_NAME_MAP_MUTEX]);

  if (meta_class->pri_h_mkd)
    free_primary_headings(meta_class->pri_h_mkd);
  if (meta_class->def_h_mkd)
    free_default_headings(meta_class->def_h_mkd);

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
  if (meta_class->rsi_hnd)
    curl_easy_cleanup(meta_class->rsi_hnd);

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
  if (meta_class->multicurl_rsi_hnd)
    curl_multi_cleanup(meta_class->multicurl_rsi_hnd);

  if (meta_class->INDEX_DOW_CURLDATA.memory) {
    free(meta_class->INDEX_DOW_CURLDATA.memory);
    meta_class->INDEX_DOW_CURLDATA.memory = NULL;
  }

  if (meta_class->INDEX_NASDAQ_CURLDATA.memory) {
    free(meta_class->INDEX_NASDAQ_CURLDATA.memory);
    meta_class->INDEX_NASDAQ_CURLDATA.memory = NULL;
  }

  if (meta_class->INDEX_SP_CURLDATA.memory) {
    free(meta_class->INDEX_SP_CURLDATA.memory);
    meta_class->INDEX_SP_CURLDATA.memory = NULL;
  }

  if (meta_class->CRYPTO_BITCOIN_CURLDATA.memory) {
    free(meta_class->CRYPTO_BITCOIN_CURLDATA.memory);
    meta_class->CRYPTO_BITCOIN_CURLDATA.memory = NULL;
  }

  /* Free Memory From Class Object */
  if (meta_class) {
    free(meta_class);
    meta_class = NULL;
  }
}