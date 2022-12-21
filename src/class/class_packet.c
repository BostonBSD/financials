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

#include <stdlib.h>
#include <time.h> /* time_t, struct tm, time ()  */

#include "../include/class.h" /* The class init and destruct funcs are required 
                                       in the class methods, includes portfolio_packet 
                                       metal, meta, and equity_folder class types */
#include "../include/multicurl.h"
#include "../include/mutex.h"
#include "../include/sqlite.h"
#include "../include/workfuncs.h"

/* The global variable 'packet' from globals.h is always accessed via these
 * functions. */
/* This is an ad-hoc way of self referencing a class.
   It prevents multiple instances of the portfolio_packet class. */

portfolio_packet *packet;

/* Class Method (also called Function) Definitions */
static void Calculate() {
  pthread_mutex_lock(&mutex_working[CLASS_MEMBER_MUTEX]);

  packet->equity_folder_class->Calculate();
  packet->metal_class->Calculate();
  packet->meta_class->CalculatePortfolio(packet);
  /* No need to calculate the index data [the gain calculation is performed
   * during extraction] */

  pthread_mutex_unlock(&mutex_working[CLASS_MEMBER_MUTEX]);
}

static void ToStrings() {
  pthread_mutex_lock(&mutex_working[CLASS_MEMBER_MUTEX]);

  packet->meta_class->ToStringsPortfolio();
  packet->meta_class->ToStringsIndices();
  packet->metal_class->ToStrings();
  packet->equity_folder_class->ToStrings();

  pthread_mutex_unlock(&mutex_working[CLASS_MEMBER_MUTEX]);
}

static int perform_multicurl_request(portfolio_packet *pkg) {
  equity_folder *F = pkg->GetEquityFolderClass();
  metal *M = pkg->GetMetalClass();
  meta *Met = pkg->GetMetaClass();
  int return_code = 0;
  short num_metals = 2;
  if (M->Platinum->ounce_f > 0)
    num_metals++;
  if (M->Palladium->ounce_f > 0)
    num_metals++;

  /* Perform the cURL requests simultaneously using multi-cURL. */
  /* Four Indices plus Two-to-Four Metals plus Number of Equities */
  return_code = PerformMultiCurl(pkg->multicurl_main_hnd,
                                 4.0f + (double)num_metals + (double)F->size);
  if (return_code) {
    for (unsigned short c = 0; c < F->size; c++) {
      if (F->Equity[c]->JSON.memory)
        free(F->Equity[c]->JSON.memory);
      F->Equity[c]->JSON.memory = NULL;
    }
    if (Met->INDEX_DOW_CURLDATA.memory)
      free(Met->INDEX_DOW_CURLDATA.memory);
    if (Met->INDEX_NASDAQ_CURLDATA.memory)
      free(Met->INDEX_NASDAQ_CURLDATA.memory);
    if (Met->INDEX_SP_CURLDATA.memory)
      free(Met->INDEX_SP_CURLDATA.memory);
    if (Met->CRYPTO_BITCOIN_CURLDATA.memory)
      free(Met->CRYPTO_BITCOIN_CURLDATA.memory);
    Met->INDEX_DOW_CURLDATA.memory = NULL;
    Met->INDEX_NASDAQ_CURLDATA.memory = NULL;
    Met->INDEX_SP_CURLDATA.memory = NULL;
    Met->CRYPTO_BITCOIN_CURLDATA.memory = NULL;
    if (M->Gold->CURLDATA.memory)
      free(M->Gold->CURLDATA.memory);
    if (M->Silver->CURLDATA.memory)
      free(M->Silver->CURLDATA.memory);
    M->Gold->CURLDATA.memory = NULL;
    M->Silver->CURLDATA.memory = NULL;
    if (M->Platinum->CURLDATA.memory)
      free(M->Platinum->CURLDATA.memory);
    if (M->Palladium->CURLDATA.memory)
      free(M->Palladium->CURLDATA.memory);
    M->Platinum->CURLDATA.memory = NULL;
    M->Palladium->CURLDATA.memory = NULL;
  }

  return return_code;
}

static int GetData() {
  pthread_mutex_lock(&mutex_working[CLASS_MEMBER_MUTEX]);

  portfolio_packet *pkg = packet;
  equity_folder *F = pkg->GetEquityFolderClass();
  metal *M = pkg->GetMetalClass();
  meta *Met = pkg->GetMetaClass();
  int return_code = 0;

  Met->SetUpCurlIndicesData(pkg); /* Four Indices */
  M->SetUpCurl(pkg);              /* Two to Four Metals */
  F->SetUpCurl(pkg);
  return_code = perform_multicurl_request(pkg);

  pthread_mutex_unlock(&mutex_working[CLASS_MEMBER_MUTEX]);
  return return_code;
}

static void ExtractData() {
  pthread_mutex_lock(&mutex_working[CLASS_MEMBER_MUTEX]);

  packet->meta_class->ExtractIndicesData();
  packet->metal_class->ExtractData();
  packet->equity_folder_class->ExtractData();

  pthread_mutex_unlock(&mutex_working[CLASS_MEMBER_MUTEX]);
}

static bool IsFetchingData() { return packet->meta_class->fetching_data_bool; }

static void SetFetchingData(bool data) {
  pthread_mutex_lock(&mutex_working[CLASS_MEMBER_MUTEX]);

  packet->meta_class->fetching_data_bool = data;

  pthread_mutex_unlock(&mutex_working[CLASS_MEMBER_MUTEX]);
}

static bool IsDefaultView() {
  return packet->meta_class->main_win_default_view_bool;
}

static void SetDefaultView(bool data) {
  pthread_mutex_lock(&mutex_working[CLASS_MEMBER_MUTEX]);

  packet->meta_class->main_win_default_view_bool = data;

  pthread_mutex_unlock(&mutex_working[CLASS_MEMBER_MUTEX]);
}

static bool IsCurlCanceled() {
  return packet->meta_class->multicurl_cancel_bool;
}

static void SetCurlCanceled(bool data) {
  packet->meta_class->multicurl_cancel_bool = data;
}

static bool IsHoliday() { return packet->meta_class->holiday_bool; }

static void SetHoliday() {
  pthread_mutex_lock(&mutex_working[CLASS_MEMBER_MUTEX]);

  meta *D = packet->GetMetaClass();
  struct tm NY_Time = NYTimeComponents();
  D->holiday_bool = CheckHoliday(NY_Time);

  pthread_mutex_unlock(&mutex_working[CLASS_MEMBER_MUTEX]);
}

static double GetHoursOfUpdates() {
  pthread_mutex_lock(&mutex_working[CLASS_MEMBER_MUTEX]);

  meta *D = packet->GetMetaClass();
  double return_value = D->updates_hours_f;

  pthread_mutex_unlock(&mutex_working[CLASS_MEMBER_MUTEX]);
  return return_value;
}

static double GetUpdatesPerMinute() {
  pthread_mutex_lock(&mutex_working[CLASS_MEMBER_MUTEX]);

  meta *D = packet->GetMetaClass();
  double return_value = D->updates_per_min_f;

  pthread_mutex_unlock(&mutex_working[CLASS_MEMBER_MUTEX]);
  return return_value;
}

static void remove_main_curl_handles(portfolio_packet *pkg)
/* Removing the easy handle from the multihandle will stop the cURL data
   transfer immediately. curl_multi_remove_handle does nothing if the easy
   handle is not currently set in the multihandle. */
{
  metal *M = pkg->GetMetalClass();
  equity_folder *F = pkg->GetEquityFolderClass();
  meta *Met = pkg->GetMetaClass();

  curl_multi_wakeup(pkg->multicurl_main_hnd);
  pthread_mutex_lock(&mutex_working[MULTICURL_PROG_MUTEX]);

  /* Equity Multicurl Operation */
  for (unsigned short i = 0; i < F->size; i++) {
    curl_multi_remove_handle(pkg->multicurl_main_hnd, F->Equity[i]->easy_hnd);
  }

  /* Bullion Multicurl Operation */
  curl_multi_remove_handle(pkg->multicurl_main_hnd, M->Gold->YAHOO_hnd);
  curl_multi_remove_handle(pkg->multicurl_main_hnd, M->Silver->YAHOO_hnd);
  if (M->Platinum->ounce_f > 0)
    curl_multi_remove_handle(pkg->multicurl_main_hnd, M->Platinum->YAHOO_hnd);
  if (M->Palladium->ounce_f > 0)
    curl_multi_remove_handle(pkg->multicurl_main_hnd, M->Palladium->YAHOO_hnd);

  /* Indices Multicurl Operation */
  curl_multi_remove_handle(pkg->multicurl_main_hnd, Met->index_dow_hnd);
  curl_multi_remove_handle(pkg->multicurl_main_hnd, Met->index_nasdaq_hnd);
  curl_multi_remove_handle(pkg->multicurl_main_hnd, Met->index_sp_hnd);
  curl_multi_remove_handle(pkg->multicurl_main_hnd, Met->crypto_bitcoin_hnd);

  pthread_mutex_unlock(&mutex_working[MULTICURL_PROG_MUTEX]);
}

static void StopMultiCurl() {
  meta *D = packet->GetMetaClass();

  /* Symbol Name Fetch Multicurl Operation */
  D->StopSNMapCurl();

  /* RSI Data Multicurl Operation */
  D->StopRSICurl();

  /* Main Window Data Fetch Multicurl Operation */
  remove_main_curl_handles(packet);
}

static void *GetPrimaryHeadings() { return packet->meta_class->pri_h_mkd; }

static void *GetDefaultHeadings() { return packet->meta_class->def_h_mkd; }

static void SetWindowDataSql() {
  pthread_mutex_lock(&mutex_working[CLASS_MEMBER_MUTEX]);

  meta *D = packet->GetMetaClass();
  window_data *W = packet->GetWindowData();

  /* Save the Window Size and Location. */
  SqliteAddMainWindowSize(W->main_width, W->main_height, D);
  SqliteAddMainWindowPos(W->main_x_pos, W->main_y_pos, D);
  SqliteAddRSIWindowSize(W->rsi_width, W->rsi_height, D);
  SqliteAddRSIWindowPos(W->rsi_x_pos, W->rsi_y_pos, D);

  pthread_mutex_unlock(&mutex_working[CLASS_MEMBER_MUTEX]);
}

static void *GetWindowData() { return packet->meta_class->window_struct; }

static void *GetMetaClass() { return packet->meta_class; }

static void *GetMetalClass() { return packet->metal_class; }

static void *GetEquityFolderClass() { return packet->equity_folder_class; }

static unsigned int Seconds2Open() { return SecondsToOpen(); }

static void *GetSymNameMap() { return packet->meta_class->sym_map; }

static void SetSymNameMap(void *data) {
  packet->meta_class->sym_map = (symbol_name_map *)data;
}

static bool IsClockDisplayed() {
  return packet->meta_class->clocks_displayed_bool;
}

static void SetClockDisplayed(bool data) {
  packet->meta_class->clocks_displayed_bool = data;
}

static bool IsIndicesDisplayed() {
  return packet->meta_class->index_bar_revealed_bool;
}

static void SetIndicesDisplayed(bool data) {
  packet->meta_class->index_bar_revealed_bool = data;
}

static void SetSecurityNames() {
  packet->equity_folder_class->SetSecurityNames(packet);
}

/* Class Init Functions */
void ClassInitPortfolioPacket() {
  /* Allocate Memory For A New Class Object */
  portfolio_packet *new_class = (portfolio_packet *)malloc(sizeof(*new_class));

  /* Initialize Variables */
  new_class->metal_class = ClassInitMetal();
  new_class->equity_folder_class = ClassInitEquityFolder();
  new_class->meta_class = ClassInitMeta();

  /* Connect Function Pointers To Function Definitions */
  new_class->Calculate = Calculate;
  new_class->ToStrings = ToStrings;
  new_class->GetData = GetData;
  new_class->ExtractData = ExtractData;
  new_class->IsFetchingData = IsFetchingData;
  new_class->SetFetchingData = SetFetchingData;
  new_class->IsDefaultView = IsDefaultView;
  new_class->SetDefaultView = SetDefaultView;
  new_class->StopMultiCurl = StopMultiCurl;
  new_class->IsCurlCanceled = IsCurlCanceled;
  new_class->SetCurlCanceled = SetCurlCanceled;
  new_class->GetHoursOfUpdates = GetHoursOfUpdates;
  new_class->GetUpdatesPerMinute = GetUpdatesPerMinute;
  new_class->IsHoliday = IsHoliday;
  new_class->SetHoliday = SetHoliday;
  new_class->GetPrimaryHeadings = GetPrimaryHeadings;
  new_class->GetDefaultHeadings = GetDefaultHeadings;
  new_class->SetWindowDataSql = SetWindowDataSql;
  new_class->GetWindowData = GetWindowData;
  new_class->GetMetaClass = GetMetaClass;
  new_class->GetMetalClass = GetMetalClass;
  new_class->GetEquityFolderClass = GetEquityFolderClass;
  new_class->SecondsToOpen = Seconds2Open;
  new_class->GetSymNameMap = GetSymNameMap;
  new_class->SetSymNameMap = SetSymNameMap;
  new_class->IsClockDisplayed = IsClockDisplayed;
  new_class->SetClockDisplayed = SetClockDisplayed;
  new_class->IsIndicesDisplayed = IsIndicesDisplayed;
  new_class->SetIndicesDisplayed = SetIndicesDisplayed;
  new_class->SetSecurityNames = SetSecurityNames;

  /* General Multicurl Handle for the Main Fetch Operation */
  new_class->multicurl_main_hnd = curl_multi_init();

  /* Set the global variable. */
  packet = new_class;
}

/* Class Destruct Functions */
void ClassDestructPortfolioPacket(portfolio_packet *pkg) {
  /* Free Memory From Class Member Objects */
  if (pkg->equity_folder_class)
    ClassDestructEquityFolder(pkg->equity_folder_class);
  if (pkg->metal_class)
    ClassDestructMetal(pkg->metal_class);
  if (pkg->meta_class)
    ClassDestructMeta(pkg->meta_class);

  if (pkg->multicurl_main_hnd)
    curl_multi_cleanup(pkg->multicurl_main_hnd);

  /* Free Memory From Class Object */
  free(pkg);
}