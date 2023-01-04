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

#include <ctype.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include <gtk/gtk.h>

#include "../include/class_types.h" /* portfolio_packet, equity_folder, metal, meta, window_data */
#include "../include/gui.h"
#include "../include/json.h"
#include "../include/multicurl.h"
#include "../include/mutex.h"
#include "../include/sqlite.h"
#include "../include/workfuncs.h"

int AddRemCompletionSet(void *data) {
  pthread_mutex_lock(&mutex_working[SYMBOL_NAME_MAP_MUTEX]);
  if (data == NULL) {
    pthread_mutex_unlock(&mutex_working[SYMBOL_NAME_MAP_MUTEX]);
    return 0;
  }

  GtkWidget *EntryBox = GetWidget("AddRemoveSecuritySymbolEntryBox");
  GtkEntryCompletion *completion = gtk_entry_completion_new();
  /* CompletionSetStore in gui_rsi.c */
  GtkListStore *store = CompletionSetStore((symbol_name_map *)data);

  gtk_entry_completion_set_model(completion, GTK_TREE_MODEL(store));
  g_object_unref(G_OBJECT(store));
  /* CompletionMatch in gui_rsi.c */
  gtk_entry_completion_set_match_func(
      completion, (GtkEntryCompletionMatchFunc)CompletionMatch, NULL,
      NULL);
  /* Set AddRemoveSecuritySymbol entrybox completion widget. */
  gtk_entry_set_completion(GTK_ENTRY(EntryBox), completion);

  /* The text column to display is column 2 */
  gtk_entry_completion_set_text_column(completion, 2);
  gtk_entry_completion_set_inline_completion(completion, FALSE);
  gtk_entry_completion_set_inline_selection(completion, TRUE);
  gtk_entry_completion_set_popup_completion(completion, TRUE);
  /* Must type at least two characters for completion to make suggestions,
     reduces the number of results for single character keys. */
  gtk_entry_completion_set_minimum_key_length(completion, 2);
  /* The text column to insert is column 0
     We use a callback on the match-selected signal and insert the text from
     column 0 instead of column 2 We use a callback on the cursor-on-match
     signal and insert the text from column 0 instead of column 2
  */
  g_signal_connect(G_OBJECT(completion), "match-selected",
                   G_CALLBACK(GUICallbackHandler_select_comp), NULL);
  g_signal_connect(G_OBJECT(completion), "cursor-on-match",
                   G_CALLBACK(GUICallbackHandler_cursor_comp), NULL);

  g_object_unref(G_OBJECT(completion));

  pthread_mutex_unlock(&mutex_working[SYMBOL_NAME_MAP_MUTEX]);
  return 0;
}

int AddRemCursorMove() {
  GtkWidget *Button = GetWidget("AddRemoveSecurityOkBTN");
  const gchar *symbol = GetEntryText("AddRemoveSecuritySymbolEntryBox");
  const gchar *shares = GetEntryText("AddRemoveSecuritySharesEntryBox");

  if (CheckValidString(symbol) && CheckValidString(shares) &&
      CheckIfStringLongPositiveNumber(shares)) {
    gtk_widget_set_sensitive(Button, true);
  } else {
    gtk_widget_set_sensitive(Button, false);
  }

  return 0;
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

int AddRemComBoxChange(void *data) {
  portfolio_packet *pkg = (portfolio_packet *)data;
  symbol_name_map *sn_map = pkg->GetSymNameMap();

  GtkWidget *ComboBox = GetWidget("AddRemoveSecurityComboBox");
  gint index = gtk_combo_box_get_active(GTK_COMBO_BOX(ComboBox));

  GtkWidget *button = GetWidget("AddRemoveSecurityOkBTN");
  GtkWidget *label = GetWidget("AddRemoveSecurityLabel");

  if (index != 0) {
    gchar *symbol =
        gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(ComboBox));
    char *name = GetSecurityName(symbol, sn_map);

    remove_dash(name);
    gtk_label_set_label(GTK_LABEL(label), name ? name : "");
    g_free(symbol);
    if (name)
      free(name);

    gtk_widget_set_sensitive(button, true);
  } else {
    gtk_label_set_label(GTK_LABEL(label), "");
    gtk_widget_set_sensitive(button, false);
  }
  return 0;
}

static void *fetch_data_for_new_stock(void *data) {
  portfolio_packet *pkg = (portfolio_packet *)data;
  equity_folder *F = pkg->GetEquityFolderClass();

  pthread_mutex_lock(&mutex_working[CLASS_MEMBER_MUTEX]);

  /* The new stock is currently at the end of the Equity array. */
  SetUpCurlHandle(F->Equity[F->size - 1]->easy_hnd, pkg->multicurl_main_hnd,
                  F->Equity[F->size - 1]->curl_url_stock_ch,
                  &F->Equity[F->size - 1]->JSON);
  PerformMultiCurl(pkg->multicurl_main_hnd, 1.0f);
  /* Extract double values from JSON data using JSON-glib */
  JsonExtractEquity(F->Equity[F->size - 1]->JSON.memory,
                    &F->Equity[F->size - 1]->current_price_stock_f,
                    &F->Equity[F->size - 1]->high_stock_f,
                    &F->Equity[F->size - 1]->low_stock_f,
                    &F->Equity[F->size - 1]->opening_stock_f,
                    &F->Equity[F->size - 1]->prev_closing_stock_f,
                    &F->Equity[F->size - 1]->change_share_f,
                    &F->Equity[F->size - 1]->change_percent_f);

  /* Free memory. */
  free(F->Equity[F->size - 1]->JSON.memory);
  F->Equity[F->size - 1]->JSON.memory = NULL;

  pthread_mutex_unlock(&mutex_working[CLASS_MEMBER_MUTEX]);

  /* Sort the equity folder, the following three statements lock the
   * MUTEXes */
  F->Sort(); /* The new stock is in alphabetical order within the array. */

  pkg->Calculate();
  pkg->ToStrings();

  pthread_exit(NULL);
}

static void add_equity_to_folder(char *symbol, const char *shares,
                                 portfolio_packet *pkg) {
  equity_folder *F = pkg->GetEquityFolderClass();

  /* Remove the stock if it already exists. */
  F->RemoveStock(symbol);

  /* Add a new stock object to the end of the folder's Equity array. */
  F->AddStock(symbol, shares);

  /* Generate the Equity Request URLs. */
  F->GenerateURL(pkg);

  /* Make sure the security names are set with pango style markups. */
  pkg->SetSecurityNames();

  if (pkg->IsDefaultView()) {
    /* Sort the equity folder. */
    F->Sort();
    gdk_threads_add_idle(MainDefaultTreeview, pkg);
  } else {
    /* Fetch the data in a separate thread */
    pthread_t thread_id;
    pthread_create(&thread_id, NULL, fetch_data_for_new_stock, pkg);
    pthread_join(thread_id, NULL);
    gdk_threads_add_idle(MainProgBarReset, NULL);
    gdk_threads_add_idle(MainPrimaryTreeview, pkg);
  }
}

static void *add_security_ok(void *data) {
  /* This mutex prevents the program from crashing if a
      MAIN_FETCH_BTN signal is run in parallel with this thread. */
  pthread_mutex_lock(&mutex_working[FETCH_DATA_MUTEX]);

  /* Unpack the package */
  portfolio_packet *package = (portfolio_packet *)data;
  meta *D = package->GetMetaClass();

  gchar *symbol = strdup(GetEntryText("AddRemoveSecuritySymbolEntryBox"));
  const gchar *shares = GetEntryText("AddRemoveSecuritySharesEntryBox");

  UpperCaseStr(symbol);

  SqliteAddEquity(symbol, shares, D);
  add_equity_to_folder(symbol, shares, package);

  g_free(symbol);

  pthread_mutex_unlock(&mutex_working[FETCH_DATA_MUTEX]);
  pthread_exit(NULL);
}

static void *remove_security_ok(void *data) {
  /* This mutex prevents the program from crashing if a
     MAIN_FETCH_BTN signal is run in parallel with this thread. */
  pthread_mutex_lock(&mutex_working[FETCH_DATA_MUTEX]);

  /* Unpack the package */
  portfolio_packet *pkg = (portfolio_packet *)data;
  equity_folder *F = pkg->GetEquityFolderClass();
  meta *D = pkg->GetMetaClass();

  GtkWidget *ComboBox = GetWidget("AddRemoveSecurityComboBox");
  gint index = gtk_combo_box_get_active(GTK_COMBO_BOX(ComboBox));
  if (index == 0) {
    pthread_mutex_unlock(&mutex_working[FETCH_DATA_MUTEX]);
    pthread_exit(NULL);
  }
  if (index == 1) {
    SqliteRemoveAllEquity(D);
    /* Reset Equity Folder */
    F->Reset();

    pkg->Calculate();
    pkg->ToStrings();
  } else {
    gchar *symbol =
        gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(ComboBox));
    SqliteRemoveEquity(symbol, D);
    F->RemoveStock(symbol);
    g_free(symbol);

    pkg->Calculate();
    pkg->ToStrings();
  }

  /* Update the treeview. */
  if (pkg->IsDefaultView()) {
    gdk_threads_add_idle(MainDefaultTreeview, data);
  } else {
    gdk_threads_add_idle(MainPrimaryTreeview, data);
  }

  pthread_mutex_unlock(&mutex_working[FETCH_DATA_MUTEX]);
  pthread_exit(NULL);
}

int AddRemOk(void *data) {
  GtkWidget *stack = GetWidget("AddRemoveSecurityStack");
  const gchar *name = gtk_stack_get_visible_child_name(GTK_STACK(stack));

  pthread_t thread_id;
  if (strcasecmp(name, "add") == 0) {
    /* Add the data in a separate thread */
    pthread_create(&thread_id, NULL, add_security_ok, data);
    pthread_detach(thread_id);
  } else {
    /* Remove the data in a separate thread */
    pthread_create(&thread_id, NULL, remove_security_ok, data);
    pthread_detach(thread_id);
  }
  return 0;
}

int AddRemShowHide(void *data) {
  /* Unpack the package */
  portfolio_packet *package = (portfolio_packet *)data;
  equity_folder *F = package->GetEquityFolderClass();

  GtkWidget *window = GetWidget("AddRemoveSecurity");
  gboolean visible = gtk_widget_is_visible(window);

  GtkWidget *stack = GetWidget("AddRemoveSecurityStack");
  GtkWidget *ComboBox = GetWidget("AddRemoveSecurityComboBox");
  GtkWidget *Button = GetWidget("AddRemoveSecurityOkBTN");

  if (visible == false) {
    gtk_stack_set_visible_child_name(GTK_STACK(stack), "add");

    /* Set the OK button sensitivity to false. */
    gtk_widget_set_sensitive(Button, false);

    /* Reset EntryBoxes */
    GtkWidget *EntryBox = GetWidget("AddRemoveSecuritySymbolEntryBox");
    gtk_entry_set_text(GTK_ENTRY(EntryBox), "");
    gtk_widget_grab_focus(EntryBox);

    EntryBox = GetWidget("AddRemoveSecuritySharesEntryBox");
    gtk_entry_set_text(GTK_ENTRY(EntryBox), "");
    g_object_set(G_OBJECT(EntryBox), "activates-default", TRUE, NULL);

    /* Reset ComboBox */
    /* Temp. Disconnect combobox signal handler. */
    g_signal_handlers_disconnect_by_func(G_OBJECT(ComboBox),
                                         G_CALLBACK(GUICallbackHandler),
                                         (void *)EQUITY_COMBO_BOX);

    gtk_combo_box_text_remove_all(GTK_COMBO_BOX_TEXT(ComboBox));
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(ComboBox), NULL,
                              "Select a security");

    if (F->size > 0) {
      gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(ComboBox), NULL,
                                "Remove all");
    }

    for (unsigned short i = 0; i < F->size; i++) {
      gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(ComboBox), NULL,
                                F->Equity[i]->symbol_stock_ch);
    }

    GtkWidget *label = GetWidget("AddRemoveSecurityLabel");

    gtk_label_set_label(GTK_LABEL(label), "");

    gtk_combo_box_set_active(GTK_COMBO_BOX(ComboBox), 0);

    /* Reconnect combobox signal handler. */
    g_signal_connect(G_OBJECT(ComboBox), "changed",
                     G_CALLBACK(GUICallbackHandler), (void *)EQUITY_COMBO_BOX);

    gtk_widget_set_visible(window, true);

  } else {
    gtk_widget_set_visible(window, false);
  }

  return 0;
}