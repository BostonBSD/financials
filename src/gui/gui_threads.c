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
#include "../include/gui.h" /* Gtk header, types: symbol_name_map, cb_signal, etc */
#include "../include/multicurl.h" /* FreeMemtype() */
#include "../include/mutex.h"     /* GMutex mutexes */
#include "../include/workfuncs.h" /* includes class_types.h [portfolio_packet, meta, etc] */

static gboolean cond_sleep(GCond *cond_var, GMutex *mutex, gint64 wait_time) {
  g_mutex_lock(mutex);
  wait_time += g_get_monotonic_time();
  /* Sleep */
  if (g_cond_wait_until(cond_var, mutex, wait_time)) {
    /* if cond signaled */
    g_mutex_unlock(mutex);
    return TRUE;
  }

  g_mutex_unlock(mutex);
  return FALSE;
}

static void cancel_thread(GCond *cond_var, GMutex *mutex) {
  /* Signal thread to exit. */
  g_mutex_lock(mutex);
  g_cond_signal(cond_var);
  g_mutex_unlock(mutex);
}

static gint64 main_fetch_iter(portfolio_packet *pkg) {
  /* The number of microseconds between data fetch operations. */
  if (pkg->GetUpdatesPerMinute() <= 0.0f) {
    return 0.0f;
  } else {
    return (gint64)(60.0f / pkg->GetUpdatesPerMinute()) * G_TIME_SPAN_SECOND;
  }
}

static gboolean main_fetch_sleep_init(gint64 start_curl,
                                      portfolio_packet *pkg) {
  meta *D = pkg->GetMetaClass();
  gint64 micro_sec_per_iter = main_fetch_iter(pkg);
  gint64 wait_time;

  /* Find out how long cURL processing took. */
  gint64 curl_proc_time = g_get_monotonic_time() - start_curl;

  if (curl_proc_time < micro_sec_per_iter) {
    /* Wait this many microseconds, accounts for cURL processing time. */
    wait_time = micro_sec_per_iter - curl_proc_time;
    return cond_sleep(&D->gthread_main_fetch_cond,
                      &mutexes[FETCH_DATA_COND_MUTEX], wait_time);
  } else if (micro_sec_per_iter == 0) {
    /* Continuous updating for an unlimited number of API calls
       per minute [subscription accounts]. */
    wait_time = G_TIME_SPAN_SECOND;
    return cond_sleep(&D->gthread_main_fetch_cond,
                      &mutexes[FETCH_DATA_COND_MUTEX], wait_time);
  }

  /* Continue loop without sleeping */
  return FALSE;
}

static gboolean main_fetch_awakens(gint64 start_curl, portfolio_packet *pkg) {
  /* If hours to update is zero, the market is closed, or fetching was canceled;
   * exit thread before sleep. */
  if (!pkg->GetHoursOfUpdates() || pkg->IsClosed() || !pkg->IsFetchingData())
    return TRUE;

  /* sleep, returns TRUE if cond signalled */
  return main_fetch_sleep_init(start_curl, pkg);
}

static gint64 main_fetch_length(portfolio_packet *pkg) {
  /* microseconds to keep looping. */
  return (gint64)(3600 * pkg->GetHoursOfUpdates()) * G_TIME_SPAN_SECOND;
}

static gboolean main_fetch_check(gint64 start_fetch, portfolio_packet *pkg) {
  gint64 length_fetch = main_fetch_length(pkg);
  gint64 end_fetch = start_fetch + length_fetch;
  if (end_fetch > g_get_monotonic_time())
    return TRUE;

  return FALSE;
}

static void main_fetch_exit(portfolio_packet *pkg) {
  pkg->FreeMainCurlData();

  /* Reset FetchingData flag. */
  pkg->SetFetchingData(FALSE);

  /* Reset the progressbar */
  gdk_threads_add_idle(MainProgBarReset, NULL);

  /* Reset Fetch Button label. */
  gdk_threads_add_idle(MainFetchBTNLabel, pkg);
  g_thread_exit(NULL);
}

static gpointer main_fetch_thd(gpointer pkg_data) {
  portfolio_packet *pkg = (portfolio_packet *)pkg_data;

  gdk_threads_add_idle(MainFetchBTNLabel, pkg);

  gint64 start_fetch = g_get_monotonic_time();
  gint64 start_curl;

  do {
    start_curl = g_get_monotonic_time();

    /* This mutex prevents the program from crashing if an
       MAIN_EXIT, SECURITY_OK_BTN, or API_OK_BTN thread is run
       concurrently with this thread. */
    g_mutex_lock(&mutexes[FETCH_DATA_MUTEX]);
    if (pkg->GetData()) {
      g_mutex_unlock(&mutexes[FETCH_DATA_MUTEX]);
      break;
    }
    g_mutex_unlock(&mutexes[FETCH_DATA_MUTEX]);

    /* Reset the progressbar */
    gdk_threads_add_idle(MainProgBarReset, NULL);

    pkg->ExtractData();
    pkg->Calculate();
    pkg->ToStrings();

    /* Set Gtk treeview. */
    gdk_threads_add_idle(MainPrimaryTreeview, pkg);

    if (main_fetch_awakens(start_curl, pkg))
      break;
  } while (main_fetch_check(start_fetch, pkg));

  main_fetch_exit(pkg);
  return NULL;
}

gpointer GUIThreadHandler_main_fetch(gpointer pkg_data)
/* A thread that handles the main_fetch_thd thread. */
{
  if (!g_mutex_trylock(&mutexes[FETCH_DATA_HANDLER_MUTEX]))
    g_thread_exit(NULL);

  portfolio_packet *pkg = (portfolio_packet *)pkg_data;
  meta *D = pkg->GetMetaClass();

  /* If the main_fetch_thd thread is currently running. */
  if (pkg->IsFetchingData()) {
    /* Reset FetchingData flag. */
    pkg->SetFetchingData(FALSE);

    /* Exit fetching data thread. */
    cancel_thread(&D->gthread_main_fetch_cond, &mutexes[FETCH_DATA_COND_MUTEX]);
    pkg->StopMultiCurlMain();
    g_thread_join(D->gthread_main_fetch_id);

    /* If the main_fetch_thd is not running. */
  } else {
    /* This flag needs to be set TRUE before g_thread_new. */
    pkg->SetFetchingData(TRUE);

    /* Create fetching data thread. */
    g_cond_clear(&D->gthread_main_fetch_cond);
    g_cond_init(&D->gthread_main_fetch_cond);
    D->gthread_main_fetch_id = g_thread_new(NULL, main_fetch_thd, pkg_data);
  }

  g_mutex_unlock(&mutexes[FETCH_DATA_HANDLER_MUTEX]);
  g_thread_exit(NULL);
  return NULL;
}

gpointer GUIThread_clock(gpointer pkg_data) {
  /*
     Set the wallclock to the NY time.
     Display whether the market is open or closed,
     time remaining until closed, or if it is a holiday.
  */
  portfolio_packet *pkg = (portfolio_packet *)pkg_data;
  meta *D = pkg->GetMetaClass();
  gint64 wait_time;

  /* If there were a blocking variation of gdk_threads_add_idle, we could
     structure this more rigidly. For the time being we will assume the
     market_closed_bool flag is always set within the current second. */
  while (D->clocks_displayed_bool) {
    gdk_threads_add_idle(MainSetClocks, pkg_data);

    /* Set Sleep until the end of the current second. */
    wait_time = ClockSleepSecond();

    if (cond_sleep(&D->gthread_clocks_cond, &mutexes[CLOCKS_COND_MUTEX],
                   wait_time))
      break;

    /* Will take into account holidays,
       including the black friday early close.
       Updated in MainSetClocks() */
    if (D->market_closed_bool) {

      /* Set Sleep until the end of the current minute. */
      wait_time = ClockSleepMinute();

      if (cond_sleep(&D->gthread_clocks_cond, &mutexes[CLOCKS_COND_MUTEX],
                     wait_time))
        break;
    }
  }

  g_thread_exit(NULL);
  return NULL;
}

gpointer GUIThreadHandler_clock(gpointer pkg_data)
/* A thread that handles the clock thread. */
{
  if (!g_mutex_trylock(&mutexes[CLOCKS_HANDLER_MUTEX]))
    g_thread_exit(NULL);

  portfolio_packet *pkg = (portfolio_packet *)pkg_data;
  meta *D = pkg->GetMetaClass();

  /* If the clock thread is currently running. */
  if (pkg->IsClockDisplayed()) {
    pkg->SetClockDisplayed(FALSE);

    /* Exit clock thread. */
    cancel_thread(&D->gthread_clocks_cond, &mutexes[CLOCKS_COND_MUTEX]);
    g_thread_join(D->gthread_clocks_id);
    g_cond_clear(&D->gthread_clocks_cond);

    /* Hide revealer */
    gdk_threads_add_idle(MainHideClocks, NULL);

    /* Make sure the pref window clock switch is set correctly. */
    gdk_threads_add_idle(PrefSetClockSwitch, pkg_data);

    /* If the clock thread is not running. */
  } else {
    /* This flag needs to be set TRUE before g_thread_new. */
    pkg->SetClockDisplayed(TRUE);

    /* Create clock thread. */
    g_cond_init(&D->gthread_clocks_cond);
    D->gthread_clocks_id = g_thread_new(NULL, GUIThread_clock, pkg_data);

    /* Show revealer */
    gdk_threads_add_idle(MainDisplayClocks, NULL);

    /* Make sure the pref window clock switch is set correctly. */
    gdk_threads_add_idle(PrefSetClockSwitch, pkg_data);
  }

  g_mutex_unlock(&mutexes[CLOCKS_HANDLER_MUTEX]);
  g_thread_exit(NULL);
  return NULL;
}

gpointer GUIThread_recalculate(gpointer pkg_data) {
  portfolio_packet *pkg = (portfolio_packet *)pkg_data;

  /* Set Gtk treeview. */
  if (pkg->IsDefaultView()) {
    pkg->ToStrings();
    gdk_threads_add_idle(MainDefaultTreeview, pkg);
  } else {
    pkg->Calculate();
    pkg->ToStrings();
    gdk_threads_add_idle(MainPrimaryTreeview, pkg);
  }

  g_thread_exit(NULL);
  return NULL;
}

gpointer GUIThread_api_ok(gpointer pkg_data) {
  portfolio_packet *pkg = (portfolio_packet *)pkg_data;

  g_mutex_lock(&mutexes[FETCH_DATA_MUTEX]);

  APIOk(pkg);

  g_mutex_unlock(&mutexes[FETCH_DATA_MUTEX]);

  g_thread_exit(NULL);
  return NULL;
}

gpointer GUIThread_bul_fetch(gpointer pkg_data) {
  portfolio_packet *pkg = (portfolio_packet *)pkg_data;
  metal *M = pkg->GetMetalClass();
  guint8 num_metals = 2;
  if (M->Platinum->ounce_f > 0)
    num_metals++;
  if (M->Palladium->ounce_f > 0)
    num_metals++;

  /* Ensures that pkg->multicurl_main_hnd is free to use. */
  g_mutex_lock(&mutexes[CLASS_MEMBER_MUTEX]);

  /* This func doesn't have a mutex. */
  M->SetUpCurl(pkg);

  /* Perform the cURL requests. */
  int return_code =
      PerformMultiCurl(pkg->multicurl_main_hnd, (double)num_metals);
  if (return_code) {
    FreeMemtype(&M->Gold->CURLDATA);
    FreeMemtype(&M->Silver->CURLDATA);
    FreeMemtype(&M->Platinum->CURLDATA);
    FreeMemtype(&M->Palladium->CURLDATA);
  }

  /* This func doesn't have a mutex. */
  M->ExtractData();

  g_mutex_unlock(&mutexes[CLASS_MEMBER_MUTEX]);

  /* These funcs have mutexes */
  pkg->Calculate();
  pkg->ToStrings();

  /* Reset the progressbar */
  gdk_threads_add_idle(MainProgBarReset, NULL);

  /* Update the main window treeview. */
  gdk_threads_add_idle(MainPrimaryTreeview, pkg_data);

  g_thread_exit(NULL);
  return NULL;
}

typedef struct {
  MemType *HistoryOutput;
  gchar *symbol;
} history_cleanup;

static void history_fetch_exit(history_cleanup *hist_data) {
  if (hist_data->HistoryOutput) {
    FreeMemtype(hist_data->HistoryOutput);
    g_free(hist_data->HistoryOutput);
  }

  if (hist_data->symbol)
    g_free(hist_data->symbol);

  g_thread_exit(NULL);
}

void dstry_notify_func_string_font(gpointer string_font_data) {
  string_font *str_fnt_container = (string_font *)string_font_data;
  if (str_fnt_container->string)
    g_free(str_fnt_container->string);

  if (str_fnt_container->font)
    g_free(str_fnt_container->font);

  if (str_fnt_container)
    g_free(str_fnt_container);
}

void dstry_notify_func_store(gpointer store_data) {
  GtkListStore *store = (GtkListStore *)store_data;
  if (store)
    g_object_unref(store);
}

gpointer GUIThread_history_fetch(gpointer pkg_data) {
  portfolio_packet *pkg = (portfolio_packet *)pkg_data;

  history_cleanup hstry_data = (history_cleanup){NULL};
  string_font *str_font_container = g_malloc(sizeof *str_font_container);
  *str_font_container = (string_font){NULL};
  GtkListStore *store = NULL;

  /* Prevents concurrent history fetch requests. */
  g_mutex_lock(&mutexes[HISTORY_FETCH_MUTEX]);

  /* Get the symbol string */
  HistoryGetSymbol(&hstry_data.symbol);

  /* Get the security name from the symbol map. */
  str_font_container->string = GetSecurityName(
      hstry_data.symbol, pkg->GetSymNameMap(), pkg->GetMetaClass());
  str_font_container->font = g_strdup(pkg->meta_class->font_ch);

  /* Perform multicurl. */
  hstry_data.HistoryOutput = HistoryFetchData(hstry_data.symbol, pkg);
  if (pkg->IsExitingApp() || hstry_data.HistoryOutput == NULL) {
    gdk_threads_add_idle(HistoryTreeViewClear, NULL);
    /* The str_font_container is freed in dstry_notify_func_string_font */
    gdk_threads_add_idle_full(G_PRIORITY_DEFAULT_IDLE, HistorySetSNLabel,
                              str_font_container,
                              dstry_notify_func_string_font);
    g_mutex_unlock(&mutexes[HISTORY_FETCH_MUTEX]);
    history_fetch_exit(&hstry_data);
  }

  /* Clear the current TreeView model */
  gdk_threads_add_idle(HistoryTreeViewClear, NULL);

  /* Perform calculations and set the liststore. */
  store = HistoryMakeStore(hstry_data.HistoryOutput->memory,
                           hstry_data.HistoryOutput->size);

  /* Set and display the history treeview model. */
  /* dstry_notify_func_store will unref the store */
  gdk_threads_add_idle_full(G_PRIORITY_DEFAULT_IDLE, HistoryMakeTreeview, store,
                            dstry_notify_func_store);

  /* Set the security name label.
     The str_font_container is freed in dstry_notify_func_string_font */
  gdk_threads_add_idle_full(G_PRIORITY_DEFAULT_IDLE, HistorySetSNLabel,
                            str_font_container, dstry_notify_func_string_font);

  g_mutex_unlock(&mutexes[HISTORY_FETCH_MUTEX]);

  history_fetch_exit(&hstry_data);
  return NULL;
}

static void dstry_notify_func_snmap(gpointer snmap_data) {
  symbol_name_map *sym_map = (symbol_name_map *)snmap_data;
  SNMapDestruct(sym_map);
  g_free(sym_map);
}

gpointer GUIThread_pref_sym_update(gpointer pkg_data) {
  portfolio_packet *pkg = (portfolio_packet *)pkg_data;

  symbol_name_map *sym_map = NULL;

  gdk_threads_add_idle(PrefSymBtnStart, NULL);

  /* Get the current Symbol-Name map pointer, if any. */
  sym_map = pkg->GetSymNameMap();
  /* Download the new sym-name map, if downloaded successfully; free the
     current map if any exists [most likely there isn't a current map because I
     changed the code slightly], set the new map to the current map. Otherwise
     do nothing.*/
  sym_map = SymNameFetchUpdate(pkg, sym_map);

  gdk_threads_add_idle(PrefSymBtnStop, NULL);

  if (pkg->IsExitingApp()) {
    if (sym_map) {
      SNMapDestruct(sym_map);
      g_free(sym_map);
    }
    pkg->SetSymNameMap(NULL);
    g_thread_exit(NULL);
  }

  if (sym_map) {
    /* This flag will force map lookups to memory rather than from disk,
     * temporarily [it is reset in add_mapping_to_database_thd]. */
    pkg->SetSnmapDbBusy(TRUE);

    /* Make sure the security names are set with pango style markups [in the
     * equity folder]. */
    pkg->SetSecurityNames();

    /* Set the completion widget on two entry boxes. */
    /* Destroy the sn_map after setting the two widgets. */
    gdk_threads_add_idle(HistoryCompletionSet, sym_map);
    gdk_threads_add_idle_full(G_PRIORITY_DEFAULT_IDLE, SecurityCompletionSet,
                              sym_map, dstry_notify_func_snmap);
    pkg->SetSymNameMap(NULL);

    if (pkg->IsDefaultView())
      gdk_threads_add_idle(MainDefaultTreeview, pkg);
  }

  g_thread_exit(NULL);
  return NULL;
}

gpointer GUIThread_completion_set(gpointer pkg_data) {
  portfolio_packet *pkg = (portfolio_packet *)pkg_data;

  symbol_name_map *sym_map = NULL;
  /* Fetch the stock symbols and names [from a local Db] outside the Gtk
     main loop, then create a GtkListStore and set it into
     a GtkEntryCompletion widget. */
  sym_map = SymNameFetch(pkg);
  pkg->SetSymNameMap(sym_map);

  if (pkg->IsExitingApp())
    g_thread_exit(NULL);

  /* Destroy the sn_map after setting the two widgets
   * [dstry_notify_func_snmap()]. */
  if (sym_map) {
    gdk_threads_add_idle(HistoryCompletionSet, sym_map);
    gdk_threads_add_idle_full(G_PRIORITY_DEFAULT_IDLE, SecurityCompletionSet,
                              sym_map, dstry_notify_func_snmap);
    pkg->SetSymNameMap(NULL);
  }

  g_thread_exit(NULL);
  return NULL;
}

static void main_exit_set_flags(portfolio_packet *pkg) {
  if (pkg->IsFetchingData()) {
    pkg->SetFetchingData(FALSE);
    /* Main Curl */
    pkg->SetMainCurlCanceled(TRUE);
  }

  /* Exiting App */
  pkg->SetExitingApp(TRUE);

  pkg->StopMultiCurlAll();
}

gpointer GUIThread_main_exit(gpointer pkg_data) {
  portfolio_packet *pkg = (portfolio_packet *)pkg_data;

  /* Hide the windows. Prevents them from hanging open if a db write is in
   * progress. */
  gdk_threads_add_idle(MainHideWindows, NULL);

  /* Flag the other threads. */
  main_exit_set_flags(pkg);

  /* This mutex prevents the program from crashing if a
     main_fetch_thd thread is run concurrently with this thread. */
  g_mutex_lock(&mutexes[FETCH_DATA_MUTEX]);

  /* Save application data in Sqlite. */
  pkg->SaveSqlData();

  /* Hold the application until the Sqlite sym-name map thread is finished
   * [prevents db write errors]. */
  g_mutex_lock(&mutexes[SYMBOL_NAME_MAP_SQLITE_MUTEX]);

  /* Exit the GTK main loop. */
  gtk_main_quit();

  g_mutex_unlock(&mutexes[SYMBOL_NAME_MAP_SQLITE_MUTEX]);
  g_mutex_unlock(&mutexes[FETCH_DATA_MUTEX]);

  g_thread_exit(NULL);
  return NULL;
}