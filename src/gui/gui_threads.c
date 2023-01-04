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
#include <gtk/gtk.h>

#include "../include/gui.h"
#include "../include/gui_types.h" /* symbol_name_map, cb_signal, etc */
#include "../include/multicurl.h" /* FreeMemtype() */
#include "../include/mutex.h"     /* pthread_mutex_t mutex_working */
#include "../include/workfuncs.h" /* includes class_types.h [portfolio_packet, meta, etc] */

static void api_ok_thd_cleanup(void *data) {
  portfolio_packet *pkg = (portfolio_packet *)data;

  pthread_mutex_unlock(&mutex_working[SQLITE_MUTEX]);
  pthread_mutex_unlock(&mutex_working[FETCH_DATA_MUTEX]);
  pthread_mutex_unlock(&mutex_working[CLASS_MEMBER_MUTEX]);

  gdk_threads_add_idle(APIShowHide, pkg);
}

void *GUIThreadHandler_api_ok(void *data) {
  pthread_cleanup_push(api_ok_thd_cleanup, data);

  portfolio_packet *pkg = (portfolio_packet *)data;

  pthread_mutex_lock(&mutex_working[FETCH_DATA_MUTEX]);

  APIOk(pkg);

  pthread_cleanup_pop(1);
  pthread_exit(NULL);
}

static void cash_ok_thd_cleanup(void *data) {
  portfolio_packet *pkg = (portfolio_packet *)data;

  pthread_mutex_unlock(&mutex_working[SQLITE_MUTEX]);
  pthread_mutex_unlock(&mutex_working[CLASS_CALCULATE_MUTEX]);
  pthread_mutex_unlock(&mutex_working[CLASS_TOSTRINGS_MUTEX]);

  gdk_threads_add_idle(CashShowHide, pkg);
}

void *GUIThreadHandler_cash_ok(void *data) {
  pthread_cleanup_push(cash_ok_thd_cleanup, data);

  portfolio_packet *pkg = (portfolio_packet *)data;

  CashOk(pkg);

  /* Set Gtk treeview. */
  if (pkg->IsDefaultView()) {
    pkg->ToStrings();
    gdk_threads_add_idle(MainDefaultTreeview, pkg);
  } else {
    pkg->Calculate();
    pkg->ToStrings();
    gdk_threads_add_idle(MainPrimaryTreeview, pkg);
  }

  pthread_cleanup_pop(1);
  pthread_exit(NULL);
}

static void bul_ok_thd_cleanup(void *data) {
  portfolio_packet *pkg = (portfolio_packet *)data;

  pthread_mutex_unlock(&mutex_working[SQLITE_MUTEX]);
  pthread_mutex_unlock(&mutex_working[CLASS_CALCULATE_MUTEX]);
  pthread_mutex_unlock(&mutex_working[CLASS_TOSTRINGS_MUTEX]);

  gdk_threads_add_idle(BullionShowHide, pkg);
}

void *GUIThreadHandler_bul_ok(void *data) {
  pthread_cleanup_push(bul_ok_thd_cleanup, data);

  portfolio_packet *pkg = (portfolio_packet *)data;

  BullionOk(pkg);

  pthread_cleanup_pop(1);
  pthread_exit(NULL);
}

static void looping_time(time_t *end_time, time_t *current_time,
                         double *seconds_per_iteration, portfolio_packet *pkg) {
  double loop_val;

  /* The number of seconds between data fetch operations. */
  if (pkg->GetUpdatesPerMinute() <= 0.0f) {
    *seconds_per_iteration = 0.0f;
  } else {
    *seconds_per_iteration = 60.0f / pkg->GetUpdatesPerMinute();
  }

  /* Because loop_val is not always evenly divisible by the
     seconds_per_iteration value, our loop will finish in an
     approximate length of time plus the slack seconds. */
  /* The number of seconds to keep looping. */
  loop_val = 3600 * pkg->GetHoursOfUpdates();
  /* If the hours to run is zero, run one loop iteration. */
  if (loop_val == 0.0f)
    loop_val = 1.0f;
  time(current_time);
  *end_time = *current_time + (time_t)loop_val;
}

static void loop_sleep(time_t end_curl, time_t start_curl,
                       double seconds_per_iteration) {
  double diff = difftime(end_curl, start_curl);
  /* Wait this many seconds, accounts for cURL processing time.
     We have double to int casting truncation here. */
  if (diff < seconds_per_iteration) {
    sleep((unsigned int)(seconds_per_iteration - diff));
  } else if (seconds_per_iteration == 0) {
    /* Continuous updating for an unlimited number of API calls
       per minute [subscription accounts]. */
    sleep(1);
  }
}

static void main_fetch_data_thd_cleanup(void *data)
/* Called when main_fetch_data_thd is canceled or exited. */
{
  portfolio_packet *pkg = (portfolio_packet *)data;

  /* Make sure the thread mutexes are unlocked. */
  pthread_mutex_unlock(&mutex_working[FETCH_DATA_MUTEX]);
  pthread_mutex_unlock(&mutex_working[CLASS_MEMBER_MUTEX]);
  pthread_mutex_unlock(&mutex_working[CLASS_TOSTRINGS_MUTEX]);
  pthread_mutex_unlock(&mutex_working[CLASS_CALCULATE_MUTEX]);
  pthread_mutex_unlock(&mutex_working[MULTICURL_PROG_MUTEX]);

  /* Reset the progressbar */
  gdk_threads_add_idle(MainProgBarReset, NULL);

  /* Make sure the curl data is reset for each
     item [equity, bullion, indice]. */
  pkg->FreeMainCurlData();

  /* Reset FetchingData flag. */
  pkg->SetFetchingData(false);
  /* Reset Fetch Button label. */
  gdk_threads_add_idle(MainFetchBTNLabel, pkg);
}

static void *main_fetch_data_thd(void *data) {
  portfolio_packet *pkg = (portfolio_packet *)data;

  pthread_cleanup_push(main_fetch_data_thd_cleanup, data);

  pkg->SetFetchingData(true);
  gdk_threads_add_idle(MainFetchBTNLabel, pkg);

  double seconds_per_iteration;
  time_t current_time, end_time;
  time_t start_curl, end_curl;

  pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
  pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);

  looping_time(&end_time, &current_time, &seconds_per_iteration, pkg);

  while (end_time > current_time) {

    time(&start_curl);

    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);

    /* This mutex prevents the program from crashing if an
       MAIN_EXIT, EQUITY_OK_BTN, or API_OK_BTN thread is run
       concurrently with this thread. */
    pthread_mutex_lock(&mutex_working[FETCH_DATA_MUTEX]);

    pkg->GetData();

    pthread_mutex_unlock(&mutex_working[FETCH_DATA_MUTEX]);

    if (pkg->IsMainCurlCanceled())
      pthread_exit(NULL);

    pkg->ExtractData();
    pkg->Calculate();
    pkg->ToStrings();

    /* Allow the thread to be canceled. */
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);

    /* Reset the progressbar */
    gdk_threads_add_idle(MainProgBarReset, NULL);

    /* Set Gtk treeview. */
    gdk_threads_add_idle(MainPrimaryTreeview, pkg);

    /* If hours to update is zero or the market is closed;
     * only loop once. */
    if (!pkg->GetHoursOfUpdates() || !MarketOpen()) {
      pthread_exit(NULL);
    }

    usleep(ClockSleepMicroSeconds());
    /* Find out how long cURL processing took. */
    time(&end_curl);

    /* sleep */
    loop_sleep(end_curl, start_curl, seconds_per_iteration);

    /* Find the current epoch time. */
    time(&current_time);
  }

  pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
  pthread_cleanup_pop(1);
  pthread_exit(NULL);
}

void *GUIThreadHandler_main_fetch_data(void *data)
/* A thread that handles the main_fetch_data_thd thread. */
{
  portfolio_packet *pkg = (portfolio_packet *)data;
  meta *D = pkg->GetMetaClass();

  /* If the main_fetch_data_thd thread is currently running. */
  if (pkg->IsFetchingData()) {
    /* Cancel fetching data thread. */
    pthread_cancel(D->thread_id_main_fetch_data);
    pthread_join(D->thread_id_main_fetch_data, NULL);

    /* If the fetching thread is not running. */
  } else {
    pthread_create(&D->thread_id_main_fetch_data, NULL, main_fetch_data_thd,
                   data);
    pthread_detach(D->thread_id_main_fetch_data);
  }
  pthread_exit(NULL);
}

typedef struct {
  MemType *RSIOutput;
  char *symbol;
}rsi_cleanup;

static void rsi_fetch_thd_cleanup(void *data) {
  rsi_cleanup *thd_data = (rsi_cleanup*)data;
  pthread_mutex_unlock(&mutex_working[SYMBOL_NAME_MAP_MUTEX]);
  pthread_mutex_unlock(&mutex_working[RSI_FETCH_MUTEX]);
  pthread_mutex_unlock(&mutex_working[MULTICURL_NO_PROG_MUTEX]);

  if(thd_data->RSIOutput){
    FreeMemtype(thd_data->RSIOutput);
    free(thd_data->RSIOutput);
  }

  if(thd_data->symbol){
    free(thd_data->symbol);
  }
}

void *GUIThreadHandler_rsi_fetch(void *data) {
  portfolio_packet *pkg = (portfolio_packet *)data;

  rsi_cleanup rsi_thd_data = (rsi_cleanup){NULL};
  char *sec_name = NULL;
  GtkListStore *store = NULL;

  pthread_cleanup_push(rsi_fetch_thd_cleanup, &rsi_thd_data);
  /* Prevent's multiple concurrent RSI fetch requests. */
  pthread_mutex_lock(&mutex_working[RSI_FETCH_MUTEX]);


  /* Get the symbol string */
  RSIGetSymbol(&rsi_thd_data.symbol);

  /* Get the security name from the symbol map. */
  pthread_mutex_lock(&mutex_working[SYMBOL_NAME_MAP_MUTEX]);
  sec_name = GetSecurityName(rsi_thd_data.symbol, pkg->meta_class->sym_map);
  pthread_mutex_unlock(&mutex_working[SYMBOL_NAME_MAP_MUTEX]);

  /* Perform multicurl, doesn't block the gtk main loop. */
  rsi_thd_data.RSIOutput = FetchRSIData(rsi_thd_data.symbol, pkg);
  if (pkg->IsCurlCanceled() || rsi_thd_data.RSIOutput == NULL) {
    /* The sec_name string is freed in RSISetSNLabel */
    gdk_threads_add_idle(RSISetSNLabel, sec_name);
    gdk_threads_add_idle(RSITreeViewClear, NULL);
    pthread_exit(NULL);
  }

  /* Clear the current TreeView model */
  gdk_threads_add_idle(RSITreeViewClear, NULL);

  /* Perform RSI calculations and set the liststore. */
  store = RSIMakeStore(rsi_thd_data.RSIOutput->memory);

  /* Set the security name label, this function runs inside the Gtk Loop.
     The sec_name string is freed in RSISetSNLabel */
  gdk_threads_add_idle(RSISetSNLabel, sec_name);

  /* Set and display the RSI treeview model. */
  /* This will unref store */
  gdk_threads_add_idle(RSIMakeTreeview, store);

  pthread_cleanup_pop(1);
  pthread_exit(NULL);
}

static void sym_name_update_thd_cleanup() {
  pthread_mutex_unlock(&mutex_working[MULTICURL_NO_PROG_MUTEX]);
  pthread_mutex_unlock(&mutex_working[SYMBOL_NAME_MAP_MUTEX]);
}

void *GUIThreadHandler_sym_name_update(void *data) {
  portfolio_packet *pkg = (portfolio_packet *)data;

  pthread_cleanup_push(sym_name_update_thd_cleanup, NULL);

  symbol_name_map *sym_map = NULL;

  pthread_mutex_lock(&mutex_working[SYMBOL_NAME_MAP_MUTEX]);

  gdk_threads_add_idle(PrefSymBtnStart, NULL);

  /* Get the current Symbol-Name map pointer, if any. */
  sym_map = pkg->GetSymNameMap();
  /* Download the new sym-name map, if downloaded successfully; free the
     current map, if any exists, set the current map to the new map.
     Otherwise do nothing.*/
  sym_map = SymNameFetchUpdate(pkg, sym_map);

  gdk_threads_add_idle(PrefSymBtnStop, NULL);

  /* Make sure the security names are set with pango style markups [in the
   * equity folder]. */
  pkg->SetSecurityNames();

  if (pkg->IsCurlCanceled())
    pthread_exit(NULL);

  /* gdk_threads_add_idle is non-blocking, we need the mutex
     in the RSICompletionSet and AddRemCompletionSet functions. */
  if (sym_map) {
    gdk_threads_add_idle(RSICompletionSet, sym_map);
    gdk_threads_add_idle(AddRemCompletionSet, sym_map);

    if (pkg->IsDefaultView())
      gdk_threads_add_idle(MainDefaultTreeview, pkg);
  }

  pthread_cleanup_pop(1);
  pthread_exit(NULL);
}

static void completion_set_thd_cleanup() {
  pthread_mutex_unlock(&mutex_working[SYMBOL_NAME_MAP_MUTEX]);
}

void *GUIThreadHandler_completion_set(void *data) {
  portfolio_packet *pkg = (portfolio_packet *)data;

  pthread_cleanup_push(completion_set_thd_cleanup, NULL);

  symbol_name_map *sym_map = NULL;
  /* Fetch the stock symbols and names [from a local Db] outside the Gtk
     main loop, then create a GtkListStore and set it into
     a GtkEntryCompletion widget. */

  /* This mutex prevents the program from crashing if an
     MAIN_EXIT thread is run concurrently with this thread.
     This thread is only run once at application start.
  */
  pthread_mutex_lock(&mutex_working[SYMBOL_NAME_MAP_MUTEX]);

  sym_map = SymNameFetch(pkg);
  pkg->SetSymNameMap(sym_map);
  /* Make sure the security names are set with pango style markups. */
  pkg->SetSecurityNames();

  if (pkg->IsCurlCanceled())
    pthread_exit(NULL);

  /* gdk_threads_add_idle is non-blocking, we need the mutex
     in the RSICompletionSet and AddRemCompletionSet functions. */
  if (sym_map) {
    gdk_threads_add_idle(RSICompletionSet, sym_map);
    gdk_threads_add_idle(AddRemCompletionSet, sym_map);
  }

  pthread_mutex_unlock(&mutex_working[SYMBOL_NAME_MAP_MUTEX]);

  /* Set the default treeview. */
  if (pkg->IsDefaultView())
    gdk_threads_add_idle(MainDefaultTreeview, pkg);

  pthread_cleanup_pop(1);
  pthread_exit(NULL);
}

void *GUIThreadHandler_main_clock() {
  /*
     This is a single process multithreaded application.
     The process is always using New York time.
  */
  pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);
  pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);

  while (1) {
    gdk_threads_add_idle(MainDisplayTime, NULL);

    /* Allow the thread to be canceled while sleeping. */
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);

    /* Sleep until the end of the current second. */
    usleep((useconds_t)ClockSleepMicroSeconds());
    /* Sleep until the end of the current minute. */
    sleep(ClockSleepSeconds());

    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
  }
  pthread_exit(NULL);
}

void *GUIThreadHandler_time_to_close(void *data) {
  pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);
  pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);

  portfolio_packet *pkg = (portfolio_packet *)data;

  pkg->SetHoliday();
  while (1) {
    /* MarketOpen () will take into account holidays,
       including the black friday early close. */
    if (MarketOpen()) {

      gdk_threads_add_idle(MainDisplayTimeRemaining, pkg);

      /* Allow the thread to be canceled while sleeping. */
      pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
      usleep((useconds_t)ClockSleepMicroSeconds());
      pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);

    } else {
      pkg->SetHoliday();
      gdk_threads_add_idle(MainDisplayTimeRemaining, pkg);

      /* Allow the thread to be canceled while sleeping. */
      pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
      sleep(pkg->SecondsToOpen());
      pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);

      pkg->SetHoliday();
    }
  }
  pthread_exit(NULL);
}

static void main_exit_curl_cleanup(portfolio_packet *pkg) {
  if(pkg->IsFetchingData()){
    /* Non-Main Curl */
    pkg->SetCurlCanceled(true);
    /* Main Curl */
    pkg->SetMainCurlCanceled(true);
    pkg->StopMultiCurlAll();
    /* Cancel data fetching thread if any. */
    pthread_cancel(pkg->meta_class->thread_id_main_fetch_data);
    pthread_join(pkg->meta_class->thread_id_main_fetch_data, NULL);
  }
}

static void main_exit_thd_cleanup() {
  pthread_mutex_unlock(&mutex_working[SYMBOL_NAME_MAP_SQLITE_MUTEX]);
  pthread_mutex_unlock(&mutex_working[SYMBOL_NAME_MAP_MUTEX]);
  pthread_mutex_unlock(&mutex_working[SQLITE_MUTEX]);
  pthread_mutex_unlock(&mutex_working[FETCH_DATA_MUTEX]);
}

void *GUIThreadHandler_main_exit(void *data) {
  portfolio_packet *pkg = (portfolio_packet *)data;

  pthread_cleanup_push(main_exit_thd_cleanup, NULL);

  /* Stop cURL transfers.
       Cancel data fetching thread. */
  main_exit_curl_cleanup(pkg);

  /* This mutex prevents the program from crashing if a
     MAIN_FETCH_BTN thread is run concurrently with this thread. */
  pthread_mutex_lock(&mutex_working[FETCH_DATA_MUTEX]);

  /* Save the Window Size and Location. */
  pkg->SetWindowDataSql();

  /* Hide the windows. Prevents them from hanging open if a db write is in
   * progress. */
  gdk_threads_add_idle(MainHideWindow, NULL);

  /* This mutex prevents the program from crashing if a
     COMPLETION thread is run concurrently with this thread. */
  pthread_mutex_lock(&mutex_working[SYMBOL_NAME_MAP_MUTEX]);

  /* Hold the application until the Sqlite thread is finished [prevents db
   * write errors]. */
  pthread_mutex_lock(&mutex_working[SYMBOL_NAME_MAP_SQLITE_MUTEX]);

  /* Exit the GTK main loop. */
  gtk_main_quit();

  pthread_cleanup_pop(1);
  pthread_exit(NULL);
}