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

#include "../include/class.h" /* The class init and destruct funcs are required 
                                 in the class methods, includes portfolio_packet 
                                 metal, meta, and equity_folder class types */

#include "../include/json.h"
#include "../include/multicurl.h"
#include "../include/mutex.h"
#include "../include/workfuncs.h"

/* The init and destruct function prototypes for the nested class objects. */
static stock *class_init_equity();
static void class_destruct_equity(stock *);

/* The static file-global variable 'FolderClassObject' is always accessed via
 * these functions. */
/* This is an ad-hoc way of self referencing a class.
   It prevents multiple instances of the equity_folder class. */

static equity_folder
    *FolderClassObject; /* A class handle to an array of stock class objects,
                           can change dynamically. */

/* Class Method (also called Function) Definitions */
static void convert_equity_to_strings(stock *S, unsigned short digits_right) {
  /* Convert the double values into string values. */
  DoubleToMonStrPango(&S->current_price_stock_mrkd_ch, S->current_price_stock_f,
                      digits_right);

  DoubleToMonStrPango(&S->high_stock_mrkd_ch, S->high_stock_f, digits_right);

  DoubleToMonStrPango(&S->low_stock_mrkd_ch, S->low_stock_f, digits_right);

  DoubleToMonStrPango(&S->opening_stock_mrkd_ch, S->opening_stock_f,
                      digits_right);

  DoubleToMonStrPango(&S->prev_closing_stock_mrkd_ch, S->prev_closing_stock_f,
                      digits_right);

  DoubleToMonStrPangoColor(&S->change_value_mrkd_ch, S->change_value_f,
                           digits_right, NOT_ITALIC);

  DoubleToNumStrPango(&S->num_shares_stock_mrkd_ch,
                      (double)S->num_shares_stock_int, 0);
  switch (S->num_shares_stock_int) {
  case 0:

    DoubleToMonStrPangoColor(&S->change_share_stock_mrkd_ch, S->change_share_f,
                             digits_right, ITALIC);

    DoubleToPerStrPangoColor(&S->change_percent_mrkd_ch, S->change_percent_f,
                             digits_right, ITALIC);

    StringToStrPangoColor(&S->symbol_stock_mrkd_ch, S->symbol_stock_ch, BLACK_ITALIC);
    break;
  default:

    DoubleToMonStrPangoColor(&S->change_share_stock_mrkd_ch, S->change_share_f,
                             digits_right, NOT_ITALIC);

    DoubleToPerStrPangoColor(&S->change_percent_mrkd_ch, S->change_percent_f,
                             digits_right, NOT_ITALIC);

    StringToStrPangoColor(&S->symbol_stock_mrkd_ch, S->symbol_stock_ch, BLUE);
    break;
  }

  /* The total current investment in this equity. */
  DoubleToMonStrPango(&S->current_investment_stock_mrkd_ch,
                      S->current_investment_stock_f, digits_right);
}

static void ToStrings(unsigned short digits_right) {
  equity_folder *F = FolderClassObject;

  for (unsigned short g = 0; g < F->size; g++) {
    convert_equity_to_strings(F->Equity[g], digits_right);
  }

  /* The total equity portfolio value. */
  DoubleToMonStrPango(&F->stock_port_value_ch, F->stock_port_value_f,
                      digits_right);

  /* The equity portfolio's change in value. */
  DoubleToMonStrPangoColor(&F->stock_port_value_chg_ch,
                           F->stock_port_value_chg_f, digits_right, NOT_ITALIC);

  /* The change in total investment in equity as a percentage. */
  DoubleToPerStrPangoColor(&F->stock_port_value_p_chg_ch,
                           F->stock_port_value_p_chg_f, digits_right,
                           NOT_ITALIC);
}

static void equity_calculations(stock *S) {
  /* Calculate the stock holdings change. */
  if (S->num_shares_stock_int > 0) {
    S->change_value_f = S->change_share_f * (double)S->num_shares_stock_int;
  } else {
    S->change_value_f = 0.0f;
  }

  /* The total current investment in this equity. */
  S->current_investment_stock_f =
      (double)S->num_shares_stock_int * S->current_price_stock_f;
}

static void Calculate() {
  equity_folder *F = FolderClassObject;

  /* Equity Calculations. */
  F->stock_port_value_f = 0.0f;
  F->stock_port_value_chg_f = 0.0f;

  for (unsigned short g = 0; g < F->size; g++) {
    equity_calculations(F->Equity[g]);

    /* Add the equity investment to the total equity value. */
    F->stock_port_value_f += F->Equity[g]->current_investment_stock_f;

    /* Add the equity's change in value to the equity portfolio's change in
     * value. */
    F->stock_port_value_chg_f += F->Equity[g]->change_value_f;
  }

  /* The change in total investment in equity as a percentage. */
  double prev_total = F->stock_port_value_f - F->stock_port_value_chg_f;
  F->stock_port_value_p_chg_f = CalcGain(F->stock_port_value_f, prev_total);
}

static void GenerateURL(void *data) {
  pthread_mutex_lock(&mutex_working[CLASS_MEMBER_MUTEX]);

  portfolio_packet *pkg = (portfolio_packet *)data;
  equity_folder *F = FolderClassObject;
  meta *Met = pkg->GetMetaClass();

  size_t len;

  /* Cycle through the list of equities. */
  for (unsigned short c = 0; c < F->size; c++) {
    /* Generate the request URL for this equity. */
    len = strlen(Met->stock_url_ch) + strlen(F->Equity[c]->symbol_stock_ch) +
          strlen(Met->curl_key_ch) + 1;
    char *tmp = realloc(F->Equity[c]->curl_url_stock_ch, len);

    F->Equity[c]->curl_url_stock_ch = tmp;
    snprintf(F->Equity[c]->curl_url_stock_ch, len, "%s%s%s", Met->stock_url_ch,
             F->Equity[c]->symbol_stock_ch, Met->curl_key_ch);
  }

  pthread_mutex_unlock(&mutex_working[CLASS_MEMBER_MUTEX]);
}

static int SetUpCurl(void *data) {
  portfolio_packet *pkg = (portfolio_packet *)data;
  equity_folder *F = FolderClassObject;

  /* Cycle through the list of equities. */
  for (unsigned short c = 0; c < F->size; c++) {
    /* Add a cURL easy handle to the multi-cURL handle
    (passing JSON output struct by reference) */
    SetUpCurlHandle(F->Equity[c]->easy_hnd, pkg->multicurl_main_hnd,
                    F->Equity[c]->curl_url_stock_ch, &F->Equity[c]->JSON);
  }

  return 0;
}

static void Reset() {
  pthread_mutex_lock(&mutex_working[CLASS_MEMBER_MUTEX]);

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
      free(F->Equity);
    stock **temp = (stock **)malloc(sizeof(stock *));

    if (temp == NULL) {
      printf("Not Enough Memory, malloc returned NULL.\n");
      exit(EXIT_FAILURE);
    }

    F->Equity = temp;
  } else {
    for (short c = (F->size - 1); c >= 0; c--) {
      class_destruct_equity(F->Equity[c]);
    }

    free(F->Equity);
    stock **temp = (stock **)malloc(sizeof(stock *));

    if (temp == NULL) {
      printf("Not Enough Memory, malloc returned NULL.\n");
      exit(EXIT_FAILURE);
    }

    F->Equity = temp;
    F->size = 0;
  }

  pthread_mutex_unlock(&mutex_working[CLASS_MEMBER_MUTEX]);
}

static void AddStock(const char *symbol, const char *shares)
/* Adds a new stock object to our folder,
   increments size. */
{
  pthread_mutex_lock(&mutex_working[CLASS_MEMBER_MUTEX]);
  equity_folder *F = FolderClassObject;
  /* Does nothing if maximum number of stocks has been reached. */
  if (F->size == 255)
    return;

  stock **tmp_array =
      (stock **)realloc(F->Equity, (F->size + 1) * sizeof(stock *));

  if (tmp_array == NULL) {
    printf("Not Enough Memory, realloc returned NULL.\n");
    exit(EXIT_FAILURE);
  }

  F->Equity = tmp_array;

  /* class_init_equity returns an object pointer. */
  F->Equity[F->size] = class_init_equity();

  /* Add The Shares To the stock object */
  F->Equity[F->size]->num_shares_stock_int =
      (unsigned int)strtol(shares ? shares : "0", NULL, 10);
  DoubleToNumStrPango(&F->Equity[F->size]->num_shares_stock_mrkd_ch,
                      (double)F->Equity[F->size]->num_shares_stock_int, 0);

  /* Add The Stock Symbol To the stock object */
  /* This string is used to process the stock. */
  CopyString(&F->Equity[F->size]->symbol_stock_ch, symbol);

  switch (F->Equity[F->size]->num_shares_stock_int) {
    /* This string is used on TreeViews. */
  case 0:
    StringToStrPangoColor(&F->Equity[F->size]->symbol_stock_mrkd_ch, symbol,
                          BLACK_ITALIC);
    break;
  default:
    StringToStrPangoColor(&F->Equity[F->size]->symbol_stock_mrkd_ch, symbol,
                          BLUE);
    break;
  }
  F->size++;

  pthread_mutex_unlock(&mutex_working[CLASS_MEMBER_MUTEX]);
}

static void RemoveStock(const char *s)
/* Removes a stock object from our folder,
   decrements size. If the stock isn't found
   the size doesn't change.  Locate stock by the
   symbol string. */
{
  pthread_mutex_lock(&mutex_working[CLASS_MEMBER_MUTEX]);

  equity_folder *F = FolderClassObject;
  stock **tmp;

  unsigned short j, i = 0;
  while (i < F->size) {
    if (strcasecmp(s, F->Equity[i]->symbol_stock_ch) == 0) {
      class_destruct_equity(F->Equity[i]);
      j = i;
      while (j < F->size - 1) {
        F->Equity[j] = F->Equity[j + 1];
        j++;
      }
      /* realloc will free memory if assigned a smaller new value. */
      tmp = realloc(F->Equity, (F->size - 1) * sizeof(stock *));
      if (tmp)
        F->Equity = tmp;
      F->size--;
      break; /* Each symbol has only one unique stock object */
    }
    i++;
  }

  pthread_mutex_unlock(&mutex_working[CLASS_MEMBER_MUTEX]);
}

static void ExtractData() {
  equity_folder *F = FolderClassObject;

  for (unsigned short c = 0; c < F->size; c++)
  /* Extract current price from JSON data for each Symbol. */
  {
    if (F->Equity[c]->JSON.memory) {
      /* Extract double values from JSON data using JSON-glib */
      JsonExtractEquity(
          F->Equity[c]->JSON.memory, &F->Equity[c]->current_price_stock_f,
          &F->Equity[c]->high_stock_f, &F->Equity[c]->low_stock_f,
          &F->Equity[c]->opening_stock_f, &F->Equity[c]->prev_closing_stock_f,
          &F->Equity[c]->change_share_f, &F->Equity[c]->change_percent_f);

      /* Free memory. */
      FreeMemtype(&F->Equity[c]->JSON);
    } else {
      F->Equity[c]->current_price_stock_f = 0.0;
      F->Equity[c]->high_stock_f = 0.0;
      F->Equity[c]->low_stock_f = 0.0;
      F->Equity[c]->opening_stock_f = 0.0;
      F->Equity[c]->prev_closing_stock_f = 0.0;
      F->Equity[c]->change_share_f = 0.0;
      F->Equity[c]->change_percent_f = 0.0;
    }
  }
}

static int alpha_asc(const void *a, const void *b)
/* This is a callback function for stdlib sorting functions.
   It compares stock-structs in alphabetically ascending order,
   by the stock symbol data member.  Swapping only the pointer.
*/
{
  /* Cast the void pointer as a stock double pointer. */
  stock **aa = (stock **)a;
  stock **bb = (stock **)b;

  /* Compare the stock symbols ignoring for case. */
  return strcasecmp(aa[0]->symbol_stock_ch, bb[0]->symbol_stock_ch);
}

static void Sort() {
  pthread_mutex_lock(&mutex_working[CLASS_MEMBER_MUTEX]);

  /* Sort the equity folder in alphabetically ascending order. */
  equity_folder *F = FolderClassObject;
  qsort(&F->Equity[0], (size_t)F->size, sizeof(stock *), alpha_asc);

  pthread_mutex_unlock(&mutex_working[CLASS_MEMBER_MUTEX]);
}

static void remove_dash(char *s)
/* Locate first dash character '-' in a string,
   replace prior space with NULL character.
   If the string pointer is NULL or the first
   character is a dash; do nothing */
{
  if (s == NULL || s[0] == '-')
    return;
  char *ch = strchr(s, (int)'-');
  if (ch != NULL)
    ch[-1] = 0;
}

static void SetSecurityNames(void *data) {
  pthread_mutex_lock(&mutex_working[CLASS_MEMBER_MUTEX]);
  portfolio_packet *pkg = (portfolio_packet *)data;
  equity_folder *F = pkg->GetEquityFolderClass();
  symbol_name_map *sn_map = pkg->GetSymNameMap();
  char *security_name = NULL;

  short g = 0;
  while (g < F->size) {
    if (sn_map) {
      security_name = GetSecurityName(F->Equity[g]->symbol_stock_ch, sn_map);
      remove_dash(security_name);
    }

    if (F->Equity[g]->num_shares_stock_int) {
      StringToStrPangoColor(&F->Equity[g]->security_name_mrkd_ch,
                            security_name ? security_name : "", BLUE);
    } else {
      StringToStrPangoColor(&F->Equity[g]->security_name_mrkd_ch,
                            security_name ? security_name : "", BLACK_ITALIC);
    }

    if (security_name) {
      free(security_name);
      security_name = NULL;
    }
    g++;
  }
  pthread_mutex_unlock(&mutex_working[CLASS_MEMBER_MUTEX]);
}

/* Class Init Functions */
static stock *class_init_equity() {
  /* Allocate Memory For A New Class Object */
  stock *new_class = (stock *)malloc(sizeof(*new_class));

  /* Initialize Variables */
  new_class->num_shares_stock_int = 0;

  new_class->current_price_stock_f = 0.0f;
  new_class->high_stock_f = 0.0f;
  new_class->low_stock_f = 0.0f;
  new_class->opening_stock_f = 0.0f;
  new_class->prev_closing_stock_f = 0.0f;
  new_class->change_share_f = 0.0f;
  new_class->change_value_f = 0.0f;
  new_class->change_percent_f = 0.0f;
  new_class->current_investment_stock_f = 0.0f;

  new_class->current_price_stock_mrkd_ch = malloc(1);
  new_class->high_stock_mrkd_ch = malloc(1);
  new_class->low_stock_mrkd_ch = malloc(1);
  new_class->opening_stock_mrkd_ch = malloc(1);
  new_class->prev_closing_stock_mrkd_ch = malloc(1);
  new_class->change_share_stock_mrkd_ch = malloc(1);
  new_class->change_value_mrkd_ch = malloc(1);
  new_class->change_percent_mrkd_ch = malloc(1);
  new_class->current_investment_stock_mrkd_ch = malloc(1);
  new_class->num_shares_stock_mrkd_ch = malloc(1);
  new_class->symbol_stock_mrkd_ch = malloc(1);
  new_class->security_name_mrkd_ch = NULL;

  new_class->symbol_stock_ch = strdup("NO_SYMBOL");
  new_class->curl_url_stock_ch = strdup("http://");

  new_class->easy_hnd = curl_easy_init();
  new_class->JSON.memory = NULL;
  new_class->JSON.size = 0;

  /* Return Our Initialized Class */
  return new_class;
}

equity_folder *ClassInitEquityFolder() {
  /* Allocate Memory For A New Class */
  equity_folder *new_class = (equity_folder *)malloc(sizeof(*new_class));

  /* A placeholder for our nested stock class array */
  new_class->Equity = NULL;
  new_class->size = 0;

  /* Initialize Variables */
  new_class->stock_port_value_ch = malloc(1);
  new_class->stock_port_value_chg_ch = malloc(1);
  new_class->stock_port_value_p_chg_ch = malloc(1);

  new_class->stock_port_value_f = 0.0f;
  new_class->stock_port_value_chg_f = 0.0f;
  new_class->stock_port_value_p_chg_f = 0.0f;

  /* Connect Function Pointers To Function Definitions */
  /* The functions do not need to have the same name as the pointer,
     but it is easier to follow this way. */
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

  /* Set the file global variable so we can self-reference this class. */
  FolderClassObject = new_class;

  /* Return Our Initialized Class */
  return new_class;
}

/* Class Destruct Functions */
static void class_destruct_equity(stock *stock_class) {
  /* Free Memory */

  if (stock_class->current_price_stock_mrkd_ch) {
    free(stock_class->current_price_stock_mrkd_ch);
    stock_class->current_price_stock_mrkd_ch = NULL;
  }
  if (stock_class->high_stock_mrkd_ch) {
    free(stock_class->high_stock_mrkd_ch);
    stock_class->high_stock_mrkd_ch = NULL;
  }
  if (stock_class->low_stock_mrkd_ch) {
    free(stock_class->low_stock_mrkd_ch);
    stock_class->low_stock_mrkd_ch = NULL;
  }
  if (stock_class->opening_stock_mrkd_ch) {
    free(stock_class->opening_stock_mrkd_ch);
    stock_class->opening_stock_mrkd_ch = NULL;
  }
  if (stock_class->prev_closing_stock_mrkd_ch) {
    free(stock_class->prev_closing_stock_mrkd_ch);
    stock_class->prev_closing_stock_mrkd_ch = NULL;
  }
  if (stock_class->change_share_stock_mrkd_ch) {
    free(stock_class->change_share_stock_mrkd_ch);
    stock_class->change_share_stock_mrkd_ch = NULL;
  }
  if (stock_class->change_value_mrkd_ch) {
    free(stock_class->change_value_mrkd_ch);
    stock_class->change_value_mrkd_ch = NULL;
  }
  if (stock_class->change_percent_mrkd_ch) {
    free(stock_class->change_percent_mrkd_ch);
    stock_class->change_percent_mrkd_ch = NULL;
  }

  if (stock_class->current_investment_stock_mrkd_ch) {
    free(stock_class->current_investment_stock_mrkd_ch);
    stock_class->current_investment_stock_mrkd_ch = NULL;
  }

  if (stock_class->num_shares_stock_mrkd_ch) {
    free(stock_class->num_shares_stock_mrkd_ch);
    stock_class->num_shares_stock_mrkd_ch = NULL;
  }

  if (stock_class->symbol_stock_mrkd_ch) {
    free(stock_class->symbol_stock_mrkd_ch);
    stock_class->symbol_stock_mrkd_ch = NULL;
  }

  if (stock_class->security_name_mrkd_ch) {
    free(stock_class->security_name_mrkd_ch);
    stock_class->security_name_mrkd_ch = NULL;
  }

  if (stock_class->symbol_stock_ch) {
    free(stock_class->symbol_stock_ch);
    stock_class->symbol_stock_ch = NULL;
  }
  if (stock_class->curl_url_stock_ch) {
    free(stock_class->curl_url_stock_ch);
    stock_class->curl_url_stock_ch = NULL;
  }

  if (stock_class->easy_hnd)
    curl_easy_cleanup(stock_class->easy_hnd);

  FreeMemtype(&stock_class->JSON);

  /* Free Memory From Class Object */
  if (stock_class) {
    free(stock_class);
    stock_class = NULL;
  }
}

void ClassDestructEquityFolder(equity_folder *F) {
  /* Free Memory From Class Objects */
  for (short c = (F->size - 1); c >= 0; c--) {
    if (F->Equity[c])
      class_destruct_equity(F->Equity[c]);
  }

  if (F->Equity) {
    free(F->Equity);
    F->Equity = NULL;
  }

  /* Free Pointer Memory */
  if (F->stock_port_value_ch)
    free(F->stock_port_value_ch);
  if (F->stock_port_value_chg_ch)
    free(F->stock_port_value_chg_ch);
  if (F->stock_port_value_p_chg_ch)
    free(F->stock_port_value_p_chg_ch);

  if (F)
    free(F);
}