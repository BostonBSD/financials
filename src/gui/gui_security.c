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

#include "../include/class_types.h" /* portfolio_packet, equity_folder, metal, meta, window_data */
#include "../include/gui.h"
#include "../include/json.h"
#include "../include/multicurl.h"
#include "../include/mutex.h"
#include "../include/sqlite.h"
#include "../include/workfuncs.h"

gint SecurityCompletionSet(gpointer sn_map_data) {
  CompletionSet((symbol_name_map *)sn_map_data, GUI_COMPLETION_SECURITY);
  return 0;
}

gint SecurityCursorMove() {
  GtkWidget *Button = GetWidget("SecurityOkBTN");
  const gchar *symbol = GetEntryText("SecuritySymbolEntryBox");
  const gchar *shares = GetEntryText("SecuritySharesEntryBox");

  if (CheckValidString(symbol) && CheckValidString(shares) &&
      CheckIfStringLongPositiveNumber(shares)) {
    gtk_widget_set_sensitive(Button, TRUE);
  } else {
    gtk_widget_set_sensitive(Button, FALSE);
  }

  return 0;
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
  if (ch != NULL)
    ch[-1] = 0;
}

gint SecurityComBoxChange(portfolio_packet *pkg) {
  symbol_name_map *sn_map = pkg->GetSymNameMap();

  GtkWidget *ComboBox = GetWidget("SecurityComboBox");
  gint index = gtk_combo_box_get_active(GTK_COMBO_BOX(ComboBox));

  GtkWidget *button = GetWidget("SecurityOkBTN");
  GtkWidget *label = GetWidget("SecurityWindowLabel");

  if (index != 0) {
    gchar *symbol =
        gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(ComboBox));
    gchar *name = GetSecurityName(symbol, sn_map, pkg->GetMetaClass());
    g_free(symbol);

    remove_dash(name);
    const gchar *fmt =
        "<span foreground='MidnightBlue' size='large'>%s</span>";
    SetFormattedLabel(label, fmt, pkg->meta_class->font_ch, name ? name : "");
    if (name)
      g_free(name);

    gtk_widget_set_sensitive(button, TRUE);
  } else {
    gtk_label_set_label(GTK_LABEL(label), "");
    gtk_widget_set_sensitive(button, FALSE);
  }
  return 0;
}

static gpointer fetch_data_for_new_stock(gpointer data) {
  portfolio_packet *pkg = (portfolio_packet *)data;
  equity_folder *F = pkg->GetEquityFolderClass();

  g_mutex_lock(&mutexes[CLASS_MEMBER_MUTEX]);

  /* The new stock is currently at the end of the Equity array. */
  SetUpCurlHandle(F->Equity[F->size - 1]->easy_hnd, pkg->multicurl_main_hnd,
                  F->Equity[F->size - 1]->curl_url_stock_ch,
                  &F->Equity[F->size - 1]->JSON);
  if (PerformMultiCurl(pkg->multicurl_main_hnd, 1.0f) == 0) {
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
    FreeMemtype(&F->Equity[F->size - 1]->JSON);
  } else {
    FreeMemtype(&F->Equity[F->size - 1]->JSON);
  }

  g_mutex_unlock(&mutexes[CLASS_MEMBER_MUTEX]);

  /* Sort the equity folder, the following three statements lock the
   * MUTEXes */
  F->Sort(); /* The new stock is in alphabetical order within the array. */

  pkg->Calculate();
  pkg->ToStrings();

  return NULL;
}

static void add_equity_to_folder(gchar *symbol, const gchar *shares,
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
    /* Fetch the data for the new stock */
    fetch_data_for_new_stock(pkg);
    gdk_threads_add_idle(MainProgBarReset, NULL);
    gdk_threads_add_idle(MainPrimaryTreeview, pkg);
  }
}

static gpointer add_security_ok_thd(gpointer data) {
  /* This mutex prevents the program from crashing if a
      MAIN_FETCH_BTN signal is run in parallel with this thread. */
  g_mutex_lock(&mutexes[FETCH_DATA_MUTEX]);

  /* Unpack the package */
  portfolio_packet *package = (portfolio_packet *)data;
  meta *D = package->GetMetaClass();

  const gchar *tmp = GetEntryText("SecuritySymbolEntryBox");
  /* Convert to uppercase letters, must free return value. */
  gchar *symbol = g_ascii_strup(tmp, -1);
  const gchar *shares = GetEntryText("SecuritySharesEntryBox");

  SqliteEquityAdd(symbol, shares, D);
  add_equity_to_folder(symbol, shares, package);

  g_free(symbol);

  g_mutex_unlock(&mutexes[FETCH_DATA_MUTEX]);
  g_thread_exit(NULL);
  return NULL;
}

static gpointer remove_security_ok_thd(gpointer data) {
  /* This mutex prevents the program from crashing if a
     MAIN_FETCH_BTN signal is run in parallel with this thread. */
  g_mutex_lock(&mutexes[FETCH_DATA_MUTEX]);

  /* Unpack the package */
  portfolio_packet *pkg = (portfolio_packet *)data;
  equity_folder *F = pkg->GetEquityFolderClass();
  meta *D = pkg->GetMetaClass();

  GtkWidget *ComboBox = GetWidget("SecurityComboBox");
  gint index = gtk_combo_box_get_active(GTK_COMBO_BOX(ComboBox));
  if (index == 0) {
    g_mutex_unlock(&mutexes[FETCH_DATA_MUTEX]);
    g_thread_exit(NULL);
  }
  if (index == 1) {
    SqliteEquityRemoveAll(D);
    /* Reset Equity Folder */
    F->Reset();

    pkg->Calculate();
    pkg->ToStrings();
  } else {
    gchar *symbol =
        gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(ComboBox));
    SqliteEquityRemove(symbol, D);
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

  g_mutex_unlock(&mutexes[FETCH_DATA_MUTEX]);
  g_thread_exit(NULL);
  return NULL;
}

gint SecurityOk(portfolio_packet *pkg) {
  GtkWidget *stack = GetWidget("SecurityStack");
  const gchar *name = gtk_stack_get_visible_child_name(GTK_STACK(stack));

  GThread *g_thread_id;
  if (g_strcmp0(name, "add") == 0) {
    /* Add the data in a separate thread */
    g_thread_id = g_thread_new(NULL, add_security_ok_thd, pkg);
    g_thread_unref(g_thread_id);
  } else {
    /* Remove the data in a separate thread */
    g_thread_id = g_thread_new(NULL, remove_security_ok_thd, pkg);
    g_thread_unref(g_thread_id);
  }
  return 0;
}

gint SecurityShowHide(portfolio_packet *pkg) {
  equity_folder *F = pkg->GetEquityFolderClass();

  GtkWidget *window = GetWidget("SecurityWindow");
  gboolean visible = gtk_widget_is_visible(window);

  GtkWidget *stack = GetWidget("SecurityStack");
  GtkWidget *ComboBox = GetWidget("SecurityComboBox");
  GtkWidget *Button = GetWidget("SecurityOkBTN");

  if (visible == FALSE) {
    gtk_stack_set_visible_child_name(GTK_STACK(stack), "add");

    /* Set the OK button sensitivity to FALSE. */
    gtk_widget_set_sensitive(Button, FALSE);

    /* Reset EntryBoxes */
    GtkWidget *EntryBox = GetWidget("SecuritySymbolEntryBox");
    gtk_entry_set_text(GTK_ENTRY(EntryBox), "");
    gtk_widget_grab_focus(EntryBox);

    EntryBox = GetWidget("SecuritySharesEntryBox");
    gtk_entry_set_text(GTK_ENTRY(EntryBox), "");
    g_object_set(G_OBJECT(EntryBox), "activates-default", TRUE, NULL);

    /* Reset ComboBox */
    /* Temp. Disconnect combobox signal handler. */
    g_signal_handlers_disconnect_by_func(G_OBJECT(ComboBox),
                                         G_CALLBACK(GUICallbackHandler),
                                         (gpointer)SECURITY_COMBO_BOX);

    gtk_combo_box_text_remove_all(GTK_COMBO_BOX_TEXT(ComboBox));
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(ComboBox), NULL,
                              "Select a security");

    if (F->size > 0) {
      gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(ComboBox), NULL,
                                "Remove all");
    }

    for (guint8 i = 0; i < F->size; i++) {
      gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(ComboBox), NULL,
                                F->Equity[i]->symbol_stock_ch);
    }

    GtkWidget *label = GetWidget("SecurityWindowLabel");

    gtk_label_set_label(GTK_LABEL(label), "");

    gtk_combo_box_set_active(GTK_COMBO_BOX(ComboBox), 0);

    /* Reconnect combobox signal handler. */
    g_signal_connect(G_OBJECT(ComboBox), "changed",
                     G_CALLBACK(GUICallbackHandler),
                     (gpointer)SECURITY_COMBO_BOX);

    gtk_widget_set_visible(window, TRUE);

  } else {
    gtk_widget_set_visible(window, FALSE);
  }

  return 0;
}