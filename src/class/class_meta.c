/*
Copyright (c) 2022-2024 BostonBSD. All rights reserved.

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

#include "../include/class_types.h" /* Includes portfolio_packet, metal, meta, 
                                             and equity_folder class types */
#include "../include/macros.h"
#include "../include/multicurl.h"
#include "../include/mutex.h"
#include "../include/workfuncs.h"

/* The static global variable 'MetaClassObject' is always accessed via
 * these functions. */
/* This is an ad-hoc way of self referencing a class.
   It prevents multiple instances of the meta class.

   A more elegant approach would be to send the class object to the class method
   and mask it with macro substitution [such that the syntax appears the same
   class->method()], although, this application does not require that level of
   sophistication. */

static meta
    *MetaClassObject; /* A class object pointer called MetaClassObject. */

/* Class Method (also called Function) Definitions */
static void ToStringsPortfolio() {
  meta *Met = MetaClassObject;

  /* The cash value. */
  DoubleToFormattedStrPango(&Met->cash_mrkd_ch, Met->cash_f,
                            Met->decimal_places_guint8, MON_STR, BLACK);
  DoubleToFormattedStrPango(&Met->cash_cost_mrkd_ch, Met->cash_f,
                            Met->decimal_places_guint8, MON_STR, GREY);

  /* The total portfolio value. */
  DoubleToFormattedStrPango(&Met->portfolio_value_mrkd_ch,
                            Met->portfolio_value_f, Met->decimal_places_guint8,
                            MON_STR, BLACK);

  /* The change in total portfolio value. */
  ChangeStrPango(&Met->portfolio_day_gain_mrkd_ch,
                 Met->portfolio_day_gain_value_f,
                 Met->portfolio_day_gain_percent_f, Met->decimal_places_guint8);

  /* The total portfolio cost. */
  DoubleToFormattedStrPango(&Met->portfolio_cost_mrkd_ch, Met->portfolio_cost_f,
                            Met->decimal_places_guint8, MON_STR, GREY);

  /* The total portfolio gain since purchase. */
  ChangeStrPango(
      &Met->portfolio_total_gain_mrkd_ch, Met->portfolio_total_gain_value_f,
      Met->portfolio_total_gain_percent_f, Met->decimal_places_guint8);
}

static void CalculatePortfolio(portfolio_packet *pkg) {
  meta *Met = pkg->GetMetaClass();
  metal *M = pkg->GetMetalClass();
  equity_folder *F = pkg->GetEquityFolderClass();

  /* The total portfolio value. */
  Met->portfolio_value_f =
      M->bullion_port_value_f + F->stock_port_value_f + Met->cash_f;

  /* The change in total portfolio value. */
  /* Edit the next line as needed, if you want to
     add a change value besides equity and bullion to the portfolio. */
  Met->portfolio_day_gain_value_f =
      F->stock_port_day_gain_val_f + M->bullion_port_day_gain_val_f;

  /* The change in total portfolio value as a percentage. */
  gdouble prev_total = Met->portfolio_value_f - Met->portfolio_day_gain_value_f;
  Met->portfolio_day_gain_percent_f =
      CalcGain(Met->portfolio_value_f, prev_total);

  /* The cost of the portfolio */
  Met->portfolio_cost_f =
      M->bullion_port_cost_f + F->stock_port_cost_f + Met->cash_f;

  /* The gain of the portfolio, as a value */
  Met->portfolio_total_gain_value_f =
      Met->portfolio_value_f - Met->portfolio_cost_f;

  /* The gain of the portfolio, as a percentage */
  Met->portfolio_total_gain_percent_f =
      CalcGain(Met->portfolio_value_f, Met->portfolio_cost_f);
}

static void StopHistoryCurl() {
  meta *Met = MetaClassObject;

  curl_multi_wakeup(Met->multicurl_history_hnd);
  g_mutex_lock(&mutexes[MULTICURL_NO_PROG_MUTEX]);

  curl_multi_remove_handle(Met->multicurl_history_hnd, Met->history_hnd);

  g_mutex_unlock(&mutexes[MULTICURL_NO_PROG_MUTEX]);
}

static void StopSNMapCurl() {
  meta *Met = MetaClassObject;

  curl_multi_wakeup(Met->multicurl_cmpltn_hnd);
  g_mutex_lock(&mutexes[MULTICURL_NO_PROG_MUTEX]);

  curl_multi_remove_handle(Met->multicurl_cmpltn_hnd,
                           Met->NASDAQ_completion_hnd);
  curl_multi_remove_handle(Met->multicurl_cmpltn_hnd, Met->NYSE_completion_hnd);

  g_mutex_unlock(&mutexes[MULTICURL_NO_PROG_MUTEX]);
}

static gint SetUpCurlIndicesData(portfolio_packet *pkg) {
  meta *Met = pkg->GetMetaClass();
  gchar *dow_url_ch = NULL, *nasdaq_url_ch = NULL, *sp_url_ch = NULL,
        *bitcoin_url_ch = NULL;

  /* The start time needs to be a few days before the current time, so minus
     seven days This compensates for weekends and holidays and ensures enough
     data. */
  guint period = (86400 * 7);
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

  g_free(dow_url_ch);
  g_free(nasdaq_url_ch);
  g_free(sp_url_ch);
  g_free(bitcoin_url_ch);

  return 0;
}

static void extract_index_data_reset(const gchar *index, MemType *Data) {
  meta *Met = MetaClassObject;
  if (g_strcmp0(index, "dow") == 0) {
    Met->index_dow_value_f = 0.0f;
    Met->index_dow_value_chg_f = 0.0f;
    Met->index_dow_value_p_chg_f = 0.0f;
  } else if (g_strcmp0(index, "nasdaq") == 0) {
    Met->index_nasdaq_value_f = 0.0f;
    Met->index_nasdaq_value_chg_f = 0.0f;
    Met->index_nasdaq_value_p_chg_f = 0.0f;
  } else if (g_strcmp0(index, "sp") == 0) {
    Met->index_sp_value_f = 0.0f;
    Met->index_sp_value_chg_f = 0.0f;
    Met->index_sp_value_p_chg_f = 0.0f;
  } else if (g_strcmp0(index, "bitcoin") == 0) {
    Met->crypto_bitcoin_value_f = 0.0f;
    Met->crypto_bitcoin_value_chg_f = 0.0f;
    Met->crypto_bitcoin_value_p_chg_f = 0.0f;
  }
  FreeMemtype(Data);
}

static void extract_index_data(const gchar *index, MemType *Data) {
  meta *Met = MetaClassObject;

  if (Data->memory == NULL) {
    extract_index_data_reset(index, Data);
    return;
  }

  FILE *fp = fmemopen((gpointer)Data->memory, Data->size + 1, "r");

  if (fp == NULL) {
    extract_index_data_reset(index, Data);
    return;
  }

  double prev_closing = 0.0f, cur_price = 0.0f;
  char *line = ExtractYahooData(fp, &prev_closing, &cur_price);
  if (line)
    g_free(line);

  if (g_strcmp0(index, "dow") == 0) {
    Met->index_dow_value_f = cur_price;
    Met->index_dow_value_chg_f = cur_price - prev_closing;
    Met->index_dow_value_p_chg_f = CalcGain(cur_price, prev_closing);
  } else if (g_strcmp0(index, "nasdaq") == 0) {
    Met->index_nasdaq_value_f = cur_price;
    Met->index_nasdaq_value_chg_f = cur_price - prev_closing;
    Met->index_nasdaq_value_p_chg_f = CalcGain(cur_price, prev_closing);
  } else if (g_strcmp0(index, "sp") == 0) {
    Met->index_sp_value_f = cur_price;
    Met->index_sp_value_chg_f = cur_price - prev_closing;
    Met->index_sp_value_p_chg_f = CalcGain(cur_price, prev_closing);
  } else if (g_strcmp0(index, "bitcoin") == 0) {
    Met->crypto_bitcoin_value_f = cur_price;
    Met->crypto_bitcoin_value_chg_f = cur_price - prev_closing;
    Met->crypto_bitcoin_value_p_chg_f = CalcGain(cur_price, prev_closing);
  }

  fclose(fp);
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

/* The order of the strings, in this struct init, is important,
   they match similar names within the struct definition
   (in class_types.h). */
static const heading_str_t heading_str = {
    "Bullion",    "Metal",  "Premium", "Cost",      "Range",
    "Pr. Close",  "Chg",    "Gain",    "Total",     "Total Cost",
    "Total Gain", "Equity", "Symbol",  "Price",     "Open",
    "Asset",      "Value",  "Cash",    "Portfolio", "Portfolio has no assets."};

static void format_heading_str_pango(heading_str_t *headings_mkd) {

  StringToStrPango(&headings_mkd->metal, heading_str.metal,
                   HEADING_UNLN_FORMAT);
  StringToStrPango(&headings_mkd->price, heading_str.price,
                   HEADING_UNLN_FORMAT);
  StringToStrPango(&headings_mkd->premium, heading_str.premium,
                   HEADING_UNLN_FORMAT);

  StringToStrPango(&headings_mkd->cost, heading_str.cost, HEADING_UNLN_FORMAT);
  StringToStrPango(&headings_mkd->range, heading_str.range,
                   HEADING_UNLN_FORMAT);
  StringToStrPango(&headings_mkd->prev_closing, heading_str.prev_closing,
                   HEADING_UNLN_FORMAT);

  StringToStrPango(&headings_mkd->chg, heading_str.chg, HEADING_UNLN_FORMAT);
  StringToStrPango(&headings_mkd->day_gain, heading_str.day_gain,
                   HEADING_UNLN_FORMAT);
  StringToStrPango(&headings_mkd->total, heading_str.total,
                   HEADING_UNLN_FORMAT);

  StringToStrPango(&headings_mkd->total_cost, heading_str.total_cost,
                   HEADING_UNLN_FORMAT);
  StringToStrPango(&headings_mkd->total_gain, heading_str.total_gain,
                   HEADING_UNLN_FORMAT);
  StringToStrPango(&headings_mkd->symbol, heading_str.symbol,
                   HEADING_UNLN_FORMAT);

  StringToStrPango(&headings_mkd->opening, heading_str.opening,
                   HEADING_UNLN_FORMAT);
  StringToStrPango(&headings_mkd->asset, heading_str.asset,
                   HEADING_UNLN_FORMAT);
  StringToStrPango(&headings_mkd->value, heading_str.value,
                   HEADING_UNLN_FORMAT);

  StringToStrPango(&headings_mkd->cash, heading_str.cash,
                   HEADING_ASST_TYPE_FORMAT);
  StringToStrPango(&headings_mkd->bullion, heading_str.bullion,
                   HEADING_ASST_TYPE_FORMAT);
  StringToStrPango(&headings_mkd->equity, heading_str.equity,
                   HEADING_ASST_TYPE_FORMAT);

  StringToStrPango(&headings_mkd->portfolio, heading_str.portfolio,
                   HEADING_ASST_TYPE_FORMAT);
  StringToStrPango(&headings_mkd->no_assets, heading_str.no_assets, BLUE);
}

static void free_heading_str(heading_str_t *headings_mkd) {
  g_free(headings_mkd->bullion);
  g_free(headings_mkd->metal);
  g_free(headings_mkd->premium);

  g_free(headings_mkd->cost);
  g_free(headings_mkd->range);
  g_free(headings_mkd->prev_closing);

  g_free(headings_mkd->chg);
  g_free(headings_mkd->day_gain);
  g_free(headings_mkd->total);

  g_free(headings_mkd->total_cost);
  g_free(headings_mkd->total_gain);
  g_free(headings_mkd->equity);

  g_free(headings_mkd->symbol);
  g_free(headings_mkd->price);
  g_free(headings_mkd->opening);

  g_free(headings_mkd->asset);
  g_free(headings_mkd->value);
  g_free(headings_mkd->cash);

  g_free(headings_mkd->portfolio);
  g_free(headings_mkd->no_assets);
}

static void ToStringsHeadings() {
  meta *D = MetaClassObject;
  format_heading_str_pango(&D->headings_mkd);
}

/* Class Init Functions */
meta *ClassInitMeta() {
  /* Allocate Memory For A New Class Object */
  meta *new_class = (meta *)g_malloc(sizeof(*new_class));

  /* Initialize Variables */
  new_class->sym_map = NULL;
  new_class->rght_clk_data = (right_click_container){NULL};

  /* Initialize the main and rsi window size and locations to zero */
  new_class->window_struct = (window_data){0};

  new_class->stock_url_ch = g_strdup(FINNHUB_URL);
  new_class->curl_key_ch = g_strdup(FINNHUB_URL_TOKEN);
  new_class->Nasdaq_Symbol_url_ch = g_strdup(NASDAQ_SYMBOL_URL);
  new_class->NYSE_Symbol_url_ch = g_strdup(NYSE_SYMBOL_URL);

  new_class->cash_mrkd_ch = NULL;
  new_class->cash_cost_mrkd_ch = NULL;
  new_class->portfolio_value_mrkd_ch = NULL;
  new_class->portfolio_day_gain_mrkd_ch = NULL;

  new_class->portfolio_cost_mrkd_ch = NULL;
  new_class->portfolio_total_gain_mrkd_ch = NULL;

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
  new_class->font_ch = g_strdup(MAIN_FONT);
  SetFont(new_class->font_ch);

  /* Set The User's Config Directory: ~/.config/financials */
  new_class->config_dir_ch =
      g_strconcat(g_get_user_config_dir(), CONFIG_DIR, NULL);

  /* Append the sqlite db files to the end of the config directory path. */
  new_class->sqlite_db_path_ch =
      g_strconcat(new_class->config_dir_ch, DB_FILE, NULL);
  new_class->sqlite_symbol_name_db_path_ch =
      g_strconcat(new_class->config_dir_ch, SN_DB_FILE, NULL);

  /* The pango funcs require each dest string to point to allocated space
                or NULL, they use realloc. */
  new_class->headings_mkd = (heading_str_t){NULL};

  new_class->cash_f = 0.0f;
  new_class->portfolio_value_f = 0.0f;
  new_class->portfolio_day_gain_value_f = 0.0f;
  new_class->portfolio_day_gain_percent_f = 0.0f;
  new_class->portfolio_cost_f = 0.0f;
  new_class->portfolio_total_gain_value_f = 0.0f;
  new_class->portfolio_total_gain_percent_f = 0.0f;

  new_class->updates_per_min_f = 6.0f;
  new_class->updates_hours_f = 1.0f;

  new_class->decimal_places_guint8 = 2;

  new_class->fetching_data_bool = FALSE;
  new_class->market_closed_bool =
      GetTimeData(NULL, NULL, NULL, NULL, NULL, NULL, NULL);
  new_class->exit_app_bool = FALSE;
  new_class->multicurl_cancel_main_bool = FALSE;
  new_class->index_bar_revealed_bool = TRUE;
  new_class->clocks_displayed_bool = TRUE;
  new_class->main_win_default_view_bool = TRUE;
  new_class->snmap_db_busy_bool = FALSE;

  new_class->history_hnd = curl_easy_init();
  new_class->NASDAQ_completion_hnd = curl_easy_init();
  new_class->NYSE_completion_hnd = curl_easy_init();

  new_class->index_dow_hnd = curl_easy_init();
  new_class->index_nasdaq_hnd = curl_easy_init();
  new_class->index_sp_hnd = curl_easy_init();
  new_class->crypto_bitcoin_hnd = curl_easy_init();

  new_class->multicurl_cmpltn_hnd = curl_multi_init();
  new_class->multicurl_history_hnd = curl_multi_init();

  g_cond_init(&new_class->gthread_main_fetch_cond);

  new_class->INDEX_DOW_CURLDATA.memory = NULL;
  new_class->INDEX_DOW_CURLDATA.size = 0;
  new_class->INDEX_NASDAQ_CURLDATA.memory = NULL;
  new_class->INDEX_NASDAQ_CURLDATA.size = 0;
  new_class->INDEX_SP_CURLDATA.memory = NULL;
  new_class->INDEX_SP_CURLDATA.size = 0;
  new_class->CRYPTO_BITCOIN_CURLDATA.memory = NULL;
  new_class->CRYPTO_BITCOIN_CURLDATA.size = 0;

  /* Connect Function Pointers To Function Definitions */
  new_class->ToStringsPortfolio = ToStringsPortfolio;
  new_class->CalculatePortfolio = CalculatePortfolio;
  new_class->StopHistoryCurl = StopHistoryCurl;
  new_class->StopSNMapCurl = StopSNMapCurl;
  new_class->SetUpCurlIndicesData = SetUpCurlIndicesData;
  new_class->ExtractIndicesData = ExtractIndicesData;
  new_class->ToStringsIndices = ToStringsIndices;
  new_class->ToStringsHeadings = ToStringsHeadings;

  /* Set the static global variable so we can self-reference this class. */
  MetaClassObject = new_class;

  /* Return Our Initialized Class */
  return new_class;
}

/* Class Destruct Functions */
void ClassDestructMeta(meta *meta_class) {
  /* Free Memory */
  free_heading_str(&meta_class->headings_mkd);

  if (meta_class->rght_clk_data.type)
    g_free(meta_class->rght_clk_data.type);
  if (meta_class->rght_clk_data.symbol)
    g_free(meta_class->rght_clk_data.symbol);
  /* rght_clk_data is a variable and not a pointer, do not free.
     fyi; both variables and pointers are in heap memory, the variables are
     allocated with the struct pointer and freed with the struct pointer. */

  /* Free the symbol to security name mapping array. */
  if (meta_class->sym_map) {
    SNMapDestruct(meta_class->sym_map);
    g_free(meta_class->sym_map);
    meta_class->sym_map = NULL;
  }

  if (meta_class->stock_url_ch)
    g_free(meta_class->stock_url_ch);
  if (meta_class->curl_key_ch)
    g_free(meta_class->curl_key_ch);
  if (meta_class->Nasdaq_Symbol_url_ch)
    g_free(meta_class->Nasdaq_Symbol_url_ch);
  if (meta_class->NYSE_Symbol_url_ch)
    g_free(meta_class->NYSE_Symbol_url_ch);

  if (meta_class->cash_mrkd_ch)
    g_free(meta_class->cash_mrkd_ch);
  if (meta_class->cash_cost_mrkd_ch)
    g_free(meta_class->cash_cost_mrkd_ch);
  if (meta_class->portfolio_value_mrkd_ch)
    g_free(meta_class->portfolio_value_mrkd_ch);
  if (meta_class->portfolio_day_gain_mrkd_ch)
    g_free(meta_class->portfolio_day_gain_mrkd_ch);

  if (meta_class->portfolio_cost_mrkd_ch)
    g_free(meta_class->portfolio_cost_mrkd_ch);
  if (meta_class->portfolio_total_gain_mrkd_ch)
    g_free(meta_class->portfolio_total_gain_mrkd_ch);

  if (meta_class->index_dow_value_ch)
    g_free(meta_class->index_dow_value_ch);
  if (meta_class->index_dow_value_chg_ch)
    g_free(meta_class->index_dow_value_chg_ch);
  if (meta_class->index_dow_value_p_chg_ch)
    g_free(meta_class->index_dow_value_p_chg_ch);

  if (meta_class->index_nasdaq_value_ch)
    g_free(meta_class->index_nasdaq_value_ch);
  if (meta_class->index_nasdaq_value_chg_ch)
    g_free(meta_class->index_nasdaq_value_chg_ch);
  if (meta_class->index_nasdaq_value_p_chg_ch)
    g_free(meta_class->index_nasdaq_value_p_chg_ch);

  if (meta_class->index_sp_value_ch)
    g_free(meta_class->index_sp_value_ch);
  if (meta_class->index_sp_value_chg_ch)
    g_free(meta_class->index_sp_value_chg_ch);
  if (meta_class->index_sp_value_p_chg_ch)
    g_free(meta_class->index_sp_value_p_chg_ch);

  if (meta_class->crypto_bitcoin_value_ch)
    g_free(meta_class->crypto_bitcoin_value_ch);
  if (meta_class->crypto_bitcoin_value_chg_ch)
    g_free(meta_class->crypto_bitcoin_value_chg_ch);
  if (meta_class->crypto_bitcoin_value_p_chg_ch)
    g_free(meta_class->crypto_bitcoin_value_p_chg_ch);

  if (meta_class->font_ch)
    g_free(meta_class->font_ch);

  if (meta_class->config_dir_ch)
    g_free(meta_class->config_dir_ch);
  if (meta_class->sqlite_db_path_ch)
    g_free(meta_class->sqlite_db_path_ch);
  if (meta_class->sqlite_symbol_name_db_path_ch)
    g_free(meta_class->sqlite_symbol_name_db_path_ch);

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

  g_cond_clear(&meta_class->gthread_main_fetch_cond);

  /* Free Memory From Class Object */
  if (meta_class) {
    g_free(meta_class);
    meta_class = NULL;
  }
}