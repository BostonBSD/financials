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

  portfolio_packet *pkg = packet;
  equity_folder *F = pkg->GetEquityFolderClass();
  metal *M = pkg->GetMetalClass();
  meta *D = pkg->GetMetaClass();

  F->Calculate();
  M->Calculate();
  D->CalculatePortfolio(pkg);
  /* No need to calculate the index data [the gain calculation is performed
   * during extraction] */

  pthread_mutex_unlock(&mutex_working[CLASS_MEMBER_MUTEX]);
}

static void ToStrings() {
  pthread_mutex_lock(&mutex_working[CLASS_MEMBER_MUTEX]);

  portfolio_packet *pkg = packet;
  equity_folder *F = pkg->GetEquityFolderClass();
  metal *M = pkg->GetMetalClass();
  meta *D = pkg->GetMetaClass();

  D->ToStringsPortfolio();
  D->ToStringsIndices();
  M->ToStrings();
  F->ToStrings();

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
  /* Four Indices plus Two Metals plus Number of Equities */
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

  portfolio_packet *pkg = packet;
  equity_folder *F = pkg->GetEquityFolderClass();
  metal *M = pkg->GetMetalClass();
  meta *D = pkg->GetMetaClass();

  D->ExtractIndicesData();
  M->ExtractData();
  F->ExtractData();

  pthread_mutex_unlock(&mutex_working[CLASS_MEMBER_MUTEX]);
}

static bool IsFetchingData() {
  portfolio_packet *pkg = packet;
  meta *D = pkg->GetMetaClass();

  return D->fetching_data_bool;
}

static void SetFetchingData(bool data) {
  pthread_mutex_lock(&mutex_working[CLASS_MEMBER_MUTEX]);

  portfolio_packet *pkg = packet;
  meta *D = pkg->GetMetaClass();
  D->fetching_data_bool = data;

  pthread_mutex_unlock(&mutex_working[CLASS_MEMBER_MUTEX]);
}

static bool IsDefaultView() {
  portfolio_packet *pkg = packet;
  meta *D = pkg->GetMetaClass();

  return D->main_win_default_view_bool;
}

static void SetDefaultView(bool data) {
  pthread_mutex_lock(&mutex_working[CLASS_MEMBER_MUTEX]);

  portfolio_packet *pkg = packet;
  meta *D = pkg->GetMetaClass();
  D->main_win_default_view_bool = data;

  pthread_mutex_unlock(&mutex_working[CLASS_MEMBER_MUTEX]);
}

static bool IsCurlCanceled() {

  portfolio_packet *pkg = packet;
  meta *D = pkg->GetMetaClass();

  return D->multicurl_cancel_bool;
}

static void SetCurlCanceled(bool data) {
  portfolio_packet *pkg = packet;
  meta *D = pkg->GetMetaClass();
  D->multicurl_cancel_bool = data;
}

static bool IsHoliday() {
  portfolio_packet *pkg = packet;
  meta *D = pkg->GetMetaClass();
  return D->holiday_bool;
}

static struct tm SetHoliday() {
  pthread_mutex_lock(&mutex_working[CLASS_MEMBER_MUTEX]);

  portfolio_packet *pkg = packet;
  meta *D = pkg->GetMetaClass();
  struct tm NY_Time = NYTimeComponents();
  D->holiday_bool = CheckHoliday(NY_Time);

  pthread_mutex_unlock(&mutex_working[CLASS_MEMBER_MUTEX]);
  return NY_Time;
}

static double GetHoursOfUpdates() {
  pthread_mutex_lock(&mutex_working[CLASS_MEMBER_MUTEX]);

  portfolio_packet *pkg = packet;
  meta *D = pkg->GetMetaClass();
  double return_value = D->updates_hours_f;

  pthread_mutex_unlock(&mutex_working[CLASS_MEMBER_MUTEX]);
  return return_value;
}

static double GetUpdatesPerMinute() {
  pthread_mutex_lock(&mutex_working[CLASS_MEMBER_MUTEX]);

  portfolio_packet *pkg = packet;
  meta *D = pkg->GetMetaClass();
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
  portfolio_packet *pkg = packet;
  meta *Met = pkg->GetMetaClass();

  /* Symbol Name Fetch Multicurl Operation */
  Met->StopSNMapCurl();

  /* RSI Data Multicurl Operation */
  Met->StopRSICurl();

  /* Main Window Data Fetch Multicurl Operation */
  remove_main_curl_handles(pkg);
}

static void *GetPrimaryHeadings() { return packet->meta_class->pri_h_mkd; }

static void *GetDefaultHeadings() { return packet->meta_class->def_h_mkd; }

static void SetWindowDataSql() {
  pthread_mutex_lock(&mutex_working[CLASS_MEMBER_MUTEX]);

  portfolio_packet *pkg = packet;
  meta *D = pkg->GetMetaClass();
  window_data *W = pkg->GetWindowData();

  /* Save the Window Size and Location. */
  SqliteAddMainWindowSize(W->main_width, W->main_height, D);
  SqliteAddMainWindowPos(W->main_x_pos, W->main_y_pos, D);
  SqliteAddRSIWindowSize(W->rsi_width, W->rsi_height, D);
  SqliteAddRSIWindowPos(W->rsi_x_pos, W->rsi_y_pos, D);

  pthread_mutex_unlock(&mutex_working[CLASS_MEMBER_MUTEX]);
}

static void *GetWindowData() { return packet->window_struct; }

static void *GetMetaClass() { return packet->meta_class; }

static void *GetMetalClass() { return packet->metal_class; }

static void *GetEquityFolderClass() { return packet->equity_folder_class; }

static unsigned int Seconds2Open() { return SecondsToOpen(); }

static void *GetSymNameMap() { return packet->sym_map; }

static void SetSymNameMap(void *data) {
  symbol_name_map *sn_map = (symbol_name_map *)data;
  packet->sym_map = sn_map;
}

static bool IsClockDisplayed() {
  portfolio_packet *pkg = packet;
  meta *D = pkg->GetMetaClass();
  return D->clocks_displayed_bool;
}

static void SetClockDisplayed(bool data) {
  portfolio_packet *pkg = packet;
  meta *D = pkg->GetMetaClass();
  D->clocks_displayed_bool = data;
}

static bool IsIndicesDisplayed() {
  portfolio_packet *pkg = packet;
  meta *D = pkg->GetMetaClass();
  return D->index_bar_revealed_bool;
}

static void SetIndicesDisplayed(bool data) {
  portfolio_packet *pkg = packet;
  meta *D = pkg->GetMetaClass();
  D->index_bar_revealed_bool = data;
}

static void SetSecurityNames() {
  equity_folder *F = packet->GetEquityFolderClass();
  F->SetSecurityNames(packet);
}

/* Class Init Functions */
portfolio_packet *ClassInitPortfolioPacket() {
  /* Allocate Memory For A New Class Object */
  portfolio_packet *new_class = (portfolio_packet *)malloc(sizeof(*new_class));

  /* Initialize Variables */
  new_class->metal_class = ClassInitMetal();
  new_class->equity_folder_class = ClassInitEquityFolder();
  new_class->meta_class = ClassInitMeta();
  new_class->window_struct =
      (window_data *)malloc(sizeof(*new_class->window_struct));
  new_class->sym_map = NULL;

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

  /* Initialize the main and rsi window size and locations */
  new_class->window_struct->main_height = 0;
  new_class->window_struct->main_width = 0;
  new_class->window_struct->main_x_pos = 0;
  new_class->window_struct->main_y_pos = 0;
  new_class->window_struct->rsi_height = 0;
  new_class->window_struct->rsi_width = 0;
  new_class->window_struct->rsi_x_pos = 0;
  new_class->window_struct->rsi_y_pos = 0;

  /* General Multicurl Handle for the Main Fetch Operation */
  new_class->multicurl_main_hnd = curl_multi_init();

  /* Return Our Initialized Class */
  return new_class;
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
  if (pkg->window_struct)
    free(pkg->window_struct);

  /* Free the symbol to security name mapping array. */
  pthread_mutex_lock(&mutex_working[SYMBOL_NAME_MAP_MUTEX]);

  if (pkg->sym_map) {
    SNMapDestruct(pkg->sym_map);
    free(pkg->sym_map);
    pkg->sym_map = NULL;
  }

  pthread_mutex_unlock(&mutex_working[SYMBOL_NAME_MAP_MUTEX]);

  if (pkg->multicurl_main_hnd)
    curl_multi_cleanup(pkg->multicurl_main_hnd);

  /* Free Memory From Class Object */
  free(pkg);
}