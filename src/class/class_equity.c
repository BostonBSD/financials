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

#include "../include/class.h" /* The class init and destruct funcs are required 
                                 in the class methods, includes portfolio_packet 
                                 metal, meta, and equity_folder class types */

#include "../include/json.h"
#include "../include/macros.h"
#include "../include/multicurl.h"
#include "../include/mutex.h"
#include "../include/workfuncs.h"

/* The init and destruct function prototypes for the nested class objects. */
static stock *class_init_equity();
static void class_destruct_equity(stock *);

/* The static global variable 'FolderClassObject' is always accessed via
 * these functions. */
/* This is an ad-hoc way of self referencing a class.
   It prevents multiple instances of the equity_folder class. */

static equity_folder
    *FolderClassObject; /* A class handle to an array of stock class objects,
                           can change dynamically. */

/* Class Method (also called Function) Definitions */
static void convert_equity_to_strings(stock *S, guint8 digits_right) {
  /* Convert the double values into string values. */
  DoubleToFormattedStrPango(&S->current_price_stock_mrkd_ch,
                            S->current_price_stock_f, digits_right, MON_STR,
                            BLACK);

  DoubleToFormattedStrPango(&S->opening_stock_mrkd_ch, S->opening_stock_f,
                            digits_right, MON_STR, GREY);

  DoubleToFormattedStrPango(&S->cost_mrkd_ch, S->cost_basis_f, digits_right,
                            MON_STR, GREY);

  RangeStrPango(&S->range_mrkd_ch, S->low_stock_f, S->high_stock_f,
                digits_right);

  DoubleToFormattedStrPango(&S->prev_closing_stock_mrkd_ch,
                            S->prev_closing_stock_f, digits_right, MON_STR,
                            GREY);

  ChangeStrPango(&S->change_share_stock_mrkd_ch, S->change_share_f,
                 S->change_percent_f, digits_right);

  switch (S->quantity_int) {
  case 0:

    SymbolStrPango(&S->symbol_stock_mrkd_ch, S->symbol_stock_ch,
                   (gdouble)S->quantity_int, 0, BLACK_ITALIC);
    break;
  default:

    SymbolStrPango(&S->symbol_stock_mrkd_ch, S->symbol_stock_ch,
                   (gdouble)S->quantity_int, 0, BLUE);
    break;
  }

  /* The total current investment in this equity. */
  TotalStrPango(&S->current_investment_stock_mrkd_ch,
                S->current_investment_stock_f, S->change_value_f, digits_right);

  /* The total cost of this investment. */
  DoubleToFormattedStrPango(&S->total_cost_mrkd_ch, S->total_cost_f,
                            digits_right, MON_STR, BLACK);

  /* The total investment gain since purchase [value and percentage]. */
  ChangeStrPango(&S->total_gain_mrkd_ch, S->total_gain_value_f,
                 S->total_gain_percent_f, digits_right);
}

static void ToStrings(guint8 digits_right) {
  equity_folder *F = FolderClassObject;

  for (guint8 g = 0; g < F->size; g++)
    convert_equity_to_strings(F->Equity[g], digits_right);

  /* The total equity portfolio value. */
  DoubleToFormattedStrPango(&F->stock_port_value_mrkd_ch, F->stock_port_value_f,
                            digits_right, MON_STR, BLACK);

  /* The equity portfolio's change in value. */
  ChangeStrPango(&F->stock_port_day_gain_mrkd_ch, F->stock_port_day_gain_val_f,
                 F->stock_port_day_gain_per_f, digits_right);

  /* The total cost of the equity portfolio. */
  DoubleToFormattedStrPango(&F->stock_port_cost_mrkd_ch, F->stock_port_cost_f,
                            digits_right, MON_STR, BLACK);

  /* The total portfolio gain since purchase [value and percentage]. */
  ChangeStrPango(&F->stock_port_total_gain_mrkd_ch,
                 F->stock_port_total_gain_value_f,
                 F->stock_port_total_gain_percent_f, digits_right);
}

static void equity_calculations(stock *S) {
  /* Calculate the stock holdings change. */
  if (S->quantity_int > 0)
    S->change_value_f = S->change_share_f * (gdouble)S->quantity_int;
  else
    S->change_value_f = 0.0f;

  /* The total current investment in this equity. */
  S->current_investment_stock_f =
      (gdouble)S->quantity_int * S->current_price_stock_f;

  /* The total cost of this investment. */
  S->total_cost_f = S->cost_basis_f * (gdouble)S->quantity_int;

  /* The total investment gain since purchase. */
  S->total_gain_value_f = S->current_investment_stock_f - S->total_cost_f;

  /* The total investment gain, as a percentage, since purchase. */
  if (S->total_cost_f > 0)
    S->total_gain_percent_f =
        CalcGain(S->current_investment_stock_f, S->total_cost_f);
  else
    S->total_gain_percent_f = 0.0f;
}

static void Calculate() {
  equity_folder *F = FolderClassObject;

  /* Equity Calculations. */
  F->stock_port_value_f = 0.0f;
  F->stock_port_day_gain_val_f = 0.0f;
  F->stock_port_cost_f = 0.0f;

  for (guint8 g = 0; g < F->size; g++) {
    equity_calculations(F->Equity[g]);

    /* Add the equity investment to the total equity value. */
    F->stock_port_value_f += F->Equity[g]->current_investment_stock_f;

    /* Add the equity's change in value to the equity portfolio's change in
     * value. */
    F->stock_port_day_gain_val_f += F->Equity[g]->change_value_f;

    /* Add the equity cost to the portfolio cost. */
    F->stock_port_cost_f += F->Equity[g]->total_cost_f;
  }

  /* The change in total investment in equity as a percentage. */
  gdouble prev_total = F->stock_port_value_f - F->stock_port_day_gain_val_f;
  F->stock_port_day_gain_per_f = CalcGain(F->stock_port_value_f, prev_total);

  /* The gain as a value, since purchase. */
  F->stock_port_total_gain_value_f =
      F->stock_port_value_f - F->stock_port_cost_f;

  /* The gain as a percentage, since purchase. */
  if (F->stock_port_cost_f > 0)
    F->stock_port_total_gain_percent_f =
        CalcGain(F->stock_port_value_f, F->stock_port_cost_f);
  else
    F->stock_port_total_gain_percent_f = 0.0f;
}

static void GenerateURL(portfolio_packet *pkg) {
  g_mutex_lock(&mutexes[CLASS_MEMBER_MUTEX]);
  equity_folder *F = pkg->GetEquityFolderClass();
  meta *Met = pkg->GetMetaClass();

  /* Cycle through the list of equities. */
  for (guint8 c = 0; c < F->size; c++) {
    /* Generate the request URL for this equity. */
    g_free(F->Equity[c]->curl_url_stock_ch);
    F->Equity[c]->curl_url_stock_ch =
        g_strconcat(Met->stock_url_ch, F->Equity[c]->symbol_stock_ch,
                    Met->curl_key_ch, NULL);
  }

  g_mutex_unlock(&mutexes[CLASS_MEMBER_MUTEX]);
}

static gint SetUpCurl(portfolio_packet *pkg) {
  equity_folder *F = pkg->GetEquityFolderClass();

  /* Cycle through the list of equities. */
  for (guint8 c = 0; c < F->size; c++) {
    /* Add a cURL easy handle to the multi-cURL handle
    (passing JSON output struct by reference) */
    SetUpCurlHandle(F->Equity[c]->easy_hnd, pkg->multicurl_main_hnd,
                    F->Equity[c]->curl_url_stock_ch, &F->Equity[c]->JSON);
  }

  return 0;
}

static void Reset() {
  g_mutex_lock(&mutexes[CLASS_MEMBER_MUTEX]);

  equity_folder *F = FolderClassObject;

  if (F->size == 0) {
    /*
    We do not know if F->size = 0 means that the stock** array already
    exists or not.

    This is the only place where the stock** array is created.

    There are multiple points in the code where a zero size array
    could be reset, some where the array exists, some where it does not
    exist, so we need to keep freeing this to prevent a memory leak.

    */
    if (F->Equity)
      g_free(F->Equity);
    stock **temp = (stock **)g_malloc(sizeof(stock *));

    F->Equity = temp;
  } else {
    for (guint8 c = 0; c < F->size; c++) {
      class_destruct_equity(F->Equity[c]);
    }

    g_free(F->Equity);
    stock **temp = (stock **)g_malloc(sizeof(stock *));

    F->Equity = temp;
    F->size = 0;
  }

  g_mutex_unlock(&mutexes[CLASS_MEMBER_MUTEX]);
}

static void AddStock(const gchar *symbol, const gchar *shares,
                     const gchar *cost)
/* Adds a new stock object to our folder,
   increments size. */
{
  g_mutex_lock(&mutexes[CLASS_MEMBER_MUTEX]);
  equity_folder *F = FolderClassObject;
  /* Does nothing if maximum number of stocks has been reached. */
  if (F->size == 255)
    return;

  stock **tmp_array =
      (stock **)g_realloc(F->Equity, (F->size + 1) * sizeof(stock *));
  F->Equity = tmp_array;

  /* class_init_equity returns an object pointer. */
  F->Equity[F->size] = class_init_equity();

  /* Add the Shares to the stock object */
  F->Equity[F->size]->quantity_int =
      (guint)g_ascii_strtoll(shares ? shares : "0", NULL, 10);

  /* Add the Cost to the stock object */
  F->Equity[F->size]->cost_basis_f = g_ascii_strtod(cost ? cost : "0.0", NULL);

  /* Add The Stock Symbol To the stock object */
  /* This string is used to process the stock. */
  CopyString(&F->Equity[F->size]->symbol_stock_ch, symbol);

  switch (F->Equity[F->size]->quantity_int) {
    /* This string is used on TreeViews. */
  case 0:
    SymbolStrPango(&F->Equity[F->size]->symbol_stock_mrkd_ch, symbol,
                   (gdouble)F->Equity[F->size]->quantity_int, 0, BLACK_ITALIC);
    break;
  default:
    SymbolStrPango(&F->Equity[F->size]->symbol_stock_mrkd_ch, symbol,
                   (gdouble)F->Equity[F->size]->quantity_int, 0, BLUE);
    break;
  }
  F->size++;
  /* We don't sort here because we might want to alter the new stock object,
   * which will be at the end of the unsorted array. */
  g_mutex_unlock(&mutexes[CLASS_MEMBER_MUTEX]);
}

static void RemoveStock(const gchar *s)
/* Removes a stock object from our folder,
   decrements size. If the stock isn't found
   the size doesn't change.  Locate stock by the
   symbol string. */
{
  g_mutex_lock(&mutexes[CLASS_MEMBER_MUTEX]);

  equity_folder *F = FolderClassObject;
  stock **tmp;

  guint8 j, i = 0;
  while (i < F->size) {
    /* If we find a matching stock object. */
    if (g_strcmp0(s, F->Equity[i]->symbol_stock_ch) == 0) {
      /* Remove the object. */
      class_destruct_equity(F->Equity[i]);
      j = i;
      /* Shift the stock object array to the left by one. */
      while (j < F->size - 1) {
        F->Equity[j] = F->Equity[j + 1];
        j++;
      }
      /* Resize the array, g_realloc will free memory if assigned a smaller new
       * value. This will leave us with an extra element (if we removed the last
       * element, leaving a zero size array, g_realloc will destroy the array,
       * preventing us from adding a future element, instead we leave an extra
       * empty array element). */
      tmp = g_realloc(F->Equity, (F->size * sizeof(stock *)));
      if (tmp)
        F->Equity = tmp;
      F->size--;
      break; /* Each symbol has only one unique stock object */
    }
    i++;
  }

  g_mutex_unlock(&mutexes[CLASS_MEMBER_MUTEX]);
}

static void extract_data_reset(stock *S) {
  S->current_price_stock_f = 0.0f;
  S->high_stock_f = 0.0f;
  S->low_stock_f = 0.0f;
  S->opening_stock_f = 0.0f;
  S->prev_closing_stock_f = 0.0f;
  S->change_share_f = 0.0f;
  S->change_percent_f = 0.0f;
}

static void ExtractData() {
  equity_folder *F = FolderClassObject;
  gboolean transfer_complete = TRUE;

  for (guint8 c = 0; c < F->size; c++)
  /* Extract current price from JSON data for each Symbol. */
  {
    /* Extract double values from JSON data using JSON-glib */
    if (transfer_complete) {
      /* If one transfer is incomplete, reject all further transfers. */
      transfer_complete = JsonExtractEquity(
          F->Equity[c]->JSON.memory, &F->Equity[c]->current_price_stock_f,
          &F->Equity[c]->high_stock_f, &F->Equity[c]->low_stock_f,
          &F->Equity[c]->opening_stock_f, &F->Equity[c]->prev_closing_stock_f,
          &F->Equity[c]->change_share_f, &F->Equity[c]->change_percent_f);

      FreeMemtype(&F->Equity[c]->JSON);
    } else {
      extract_data_reset(F->Equity[c]);
      FreeMemtype(&F->Equity[c]->JSON);
    }
  }
}

static gint alpha_asc(gconstpointer a, gconstpointer b, gpointer data)
/* This is a callback function for Glib sorting functions.
   It compares stock-structs in alphabetically ascending order,
   by the stock symbol data member.  Swapping only the pointer.
*/
{
  UNUSED(data)

  /* Cast the gconstpointer as a stock double pointer. */
  stock **aa = (stock **)a;
  stock **bb = (stock **)b;

  /* Compare the stock symbols. */
  return g_strcmp0(aa[0]->symbol_stock_ch, bb[0]->symbol_stock_ch);
}

static void Sort() {
  g_mutex_lock(&mutexes[CLASS_MEMBER_MUTEX]);

  /* Sort the equity folder in alphabetically ascending order. */
  equity_folder *F = FolderClassObject;
  g_qsort_with_data((gconstpointer)&F->Equity[0], (gint)F->size,
                    (gsize)sizeof(F->Equity[0]), alpha_asc, NULL);

  g_mutex_unlock(&mutexes[CLASS_MEMBER_MUTEX]);
}

static void remove_dash(gchar *s)
/* Locate first dash character '-' in a string,
   replace prior space with NULL character.
   If the string pointer is NULL or the first
   character is a dash; do nothing */
{
  if (s == NULL || s[0] == '-')
    return;
  gchar *ch = g_utf8_strchr(s, -1, (gunichar)'-');
  if (ch)
    ch[-1] = 0;
}

static void SetSecurityNames(portfolio_packet *pkg) {
  g_mutex_lock(&mutexes[CLASS_MEMBER_MUTEX]);
  equity_folder *F = pkg->GetEquityFolderClass();
  symbol_name_map *sn_map = pkg->GetSymNameMap();
  gchar *security_name = NULL;

  guint8 g = 0;
  while (g < F->size) {
    if (pkg->IsExitingApp()) {
      g_mutex_unlock(&mutexes[CLASS_MEMBER_MUTEX]);
      return;
    }

    security_name = GetSecurityName(F->Equity[g]->symbol_stock_ch, sn_map,
                                    pkg->GetMetaClass());
    remove_dash(security_name);

    if (F->Equity[g]->quantity_int)
      StringToStrPango(&F->Equity[g]->security_name_mrkd_ch,
                       security_name ? security_name : "", BLUE);
    else
      StringToStrPango(&F->Equity[g]->security_name_mrkd_ch,
                       security_name ? security_name : "", BLACK_ITALIC);

    if (security_name) {
      g_free(security_name);
      security_name = NULL;
    }
    g++;
  }
  g_mutex_unlock(&mutexes[CLASS_MEMBER_MUTEX]);
}

/* Class Init Functions */
static stock *class_init_equity() {
  /* Allocate Memory For A New Class Object */
  stock *new_class = (stock *)g_malloc(sizeof(*new_class));

  /* Initialize Variables */
  new_class->quantity_int = 0;
  new_class->cost_basis_f = 0.0f;

  new_class->current_price_stock_f = 0.0f;
  new_class->high_stock_f = 0.0f;
  new_class->low_stock_f = 0.0f;
  new_class->opening_stock_f = 0.0f;
  new_class->prev_closing_stock_f = 0.0f;
  new_class->change_share_f = 0.0f;
  new_class->change_value_f = 0.0f;
  new_class->change_percent_f = 0.0f;
  new_class->current_investment_stock_f = 0.0f;
  new_class->total_cost_f = 0.0f;
  new_class->total_gain_value_f = 0.0f;
  new_class->total_gain_percent_f = 0.0f;

  new_class->current_price_stock_mrkd_ch = NULL;
  new_class->cost_mrkd_ch = NULL;
  new_class->range_mrkd_ch = NULL;
  new_class->opening_stock_mrkd_ch = NULL;
  new_class->prev_closing_stock_mrkd_ch = NULL;
  new_class->change_share_stock_mrkd_ch = NULL;
  new_class->current_investment_stock_mrkd_ch = NULL;
  new_class->total_cost_mrkd_ch = NULL;
  new_class->total_gain_mrkd_ch = NULL;

  new_class->symbol_stock_mrkd_ch = NULL;
  new_class->security_name_mrkd_ch = NULL;

  new_class->symbol_stock_ch = NULL;
  new_class->curl_url_stock_ch = NULL;

  new_class->easy_hnd = curl_easy_init();
  new_class->JSON.memory = NULL;
  new_class->JSON.size = 0;

  /* Return Our Initialized Class */
  return new_class;
}

equity_folder *ClassInitEquityFolder() {
  /* Allocate Memory For A New Class */
  equity_folder *new_class = (equity_folder *)g_malloc(sizeof(*new_class));

  /* A placeholder for our nested stock class array */
  new_class->Equity = NULL;
  new_class->size = 0;

  /* Initialize Variables */
  new_class->stock_port_value_mrkd_ch = NULL;
  new_class->stock_port_day_gain_mrkd_ch = NULL;
  new_class->stock_port_cost_mrkd_ch = NULL;
  new_class->stock_port_total_gain_mrkd_ch = NULL;

  new_class->stock_port_value_f = 0.0f;
  new_class->stock_port_day_gain_val_f = 0.0f;
  new_class->stock_port_day_gain_per_f = 0.0f;
  new_class->stock_port_cost_f = 0.0f;
  new_class->stock_port_total_gain_value_f = 0.0f;
  new_class->stock_port_total_gain_percent_f = 0.0f;

  /* Connect Function Pointers To Function Definitions */
  new_class->ToStrings = ToStrings;
  new_class->Calculate = Calculate;
  new_class->GenerateURL = GenerateURL;
  new_class->SetUpCurl = SetUpCurl;
  new_class->ExtractData = ExtractData;
  new_class->AddStock = AddStock;
  new_class->Reset = Reset;
  new_class->Sort = Sort;
  new_class->RemoveStock = RemoveStock;
  new_class->SetSecurityNames = SetSecurityNames;

  /* Set the static global variable so we can self-reference this class. */
  FolderClassObject = new_class;

  /* Return Our Initialized Class */
  return new_class;
}

/* Class Destruct Functions */
static void class_destruct_equity(stock *stock_class) {
  /* Free Memory */
  if (stock_class->current_price_stock_mrkd_ch) {
    g_free(stock_class->current_price_stock_mrkd_ch);
    stock_class->current_price_stock_mrkd_ch = NULL;
  }
  if (stock_class->cost_mrkd_ch) {
    g_free(stock_class->cost_mrkd_ch);
    stock_class->cost_mrkd_ch = NULL;
  }
  if (stock_class->range_mrkd_ch) {
    g_free(stock_class->range_mrkd_ch);
    stock_class->range_mrkd_ch = NULL;
  }
  if (stock_class->opening_stock_mrkd_ch) {
    g_free(stock_class->opening_stock_mrkd_ch);
    stock_class->opening_stock_mrkd_ch = NULL;
  }
  if (stock_class->prev_closing_stock_mrkd_ch) {
    g_free(stock_class->prev_closing_stock_mrkd_ch);
    stock_class->prev_closing_stock_mrkd_ch = NULL;
  }
  if (stock_class->change_share_stock_mrkd_ch) {
    g_free(stock_class->change_share_stock_mrkd_ch);
    stock_class->change_share_stock_mrkd_ch = NULL;
  }
  if (stock_class->current_investment_stock_mrkd_ch) {
    g_free(stock_class->current_investment_stock_mrkd_ch);
    stock_class->current_investment_stock_mrkd_ch = NULL;
  }
  if (stock_class->total_cost_mrkd_ch) {
    g_free(stock_class->total_cost_mrkd_ch);
    stock_class->total_cost_mrkd_ch = NULL;
  }
  if (stock_class->total_gain_mrkd_ch) {
    g_free(stock_class->total_gain_mrkd_ch);
    stock_class->total_gain_mrkd_ch = NULL;
  }

  if (stock_class->symbol_stock_mrkd_ch) {
    g_free(stock_class->symbol_stock_mrkd_ch);
    stock_class->symbol_stock_mrkd_ch = NULL;
  }

  if (stock_class->security_name_mrkd_ch) {
    g_free(stock_class->security_name_mrkd_ch);
    stock_class->security_name_mrkd_ch = NULL;
  }

  if (stock_class->symbol_stock_ch) {
    g_free(stock_class->symbol_stock_ch);
    stock_class->symbol_stock_ch = NULL;
  }
  if (stock_class->curl_url_stock_ch) {
    g_free(stock_class->curl_url_stock_ch);
    stock_class->curl_url_stock_ch = NULL;
  }

  if (stock_class->easy_hnd)
    curl_easy_cleanup(stock_class->easy_hnd);

  FreeMemtype(&stock_class->JSON);

  /* Free Memory From Class Object */
  if (stock_class) {
    g_free(stock_class);
    stock_class = NULL;
  }
}

void ClassDestructEquityFolder(equity_folder *F) {
  /* Free Memory From Class Objects */
  for (guint8 c = 0; c < F->size; c++) {
    if (F->Equity[c])
      class_destruct_equity(F->Equity[c]);
  }

  if (F->Equity) {
    g_free(F->Equity);
    F->Equity = NULL;
  }

  /* Free Pointer Memory */
  if (F->stock_port_value_mrkd_ch)
    g_free(F->stock_port_value_mrkd_ch);
  if (F->stock_port_day_gain_mrkd_ch)
    g_free(F->stock_port_day_gain_mrkd_ch);

  if (F->stock_port_cost_mrkd_ch)
    g_free(F->stock_port_cost_mrkd_ch);
  if (F->stock_port_total_gain_mrkd_ch)
    g_free(F->stock_port_total_gain_mrkd_ch);

  if (F)
    g_free(F);
}