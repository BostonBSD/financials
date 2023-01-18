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

#include "../include/gui.h"

#include "../include/class_types.h" /* portfolio_packet, equity_folder, metal, meta, window_data */
#include "../include/globals.h" /* portfolio_packet packet */
#include "../include/macros.h"
#include "../include/mutex.h" /* pthread_mutex_t mutex_working */
#include "../include/sqlite.h"
#include "../include/workfuncs.h" /* LowerCaseStr () */

void GUICallbackHandler(GtkWidget *widget, void *data)
/* The widget callback functions block the gui loop until they return,
   therefore we do not want a pthread_join statement in this function. */
{
  UNUSED(widget)

  pthread_t thread_id;

  /* We're using data as a value rather than a pointer. */
  cb_signal index_signal = (cb_signal)((uintptr_t)data);

  switch (index_signal) {
  case MAIN_FETCH_BTN:
    pthread_create(&thread_id, NULL, GUIThreadHandler_main_fetch_data, packet);
    pthread_detach(thread_id);
    break;
  case MAIN_EXIT:
    pthread_create(&thread_id, NULL, GUIThreadHandler_main_exit, packet);
    pthread_detach(thread_id);
    break;
  case HISTORY_FETCH_BTN:
    pthread_create(&thread_id, NULL, GUIThreadHandler_history_fetch, packet);
    pthread_detach(thread_id);
    break;
  case ABOUT_TOGGLE_BTN:
    AboutShowHide();
    break;
  case SECURITY_TOGGLE_BTN:
    SecurityShowHide(packet);
    break;
  case SECURITY_OK_BTN:
    SecurityShowHide(packet);
    SecurityOk(packet);
    break;
  case SECURITY_COMBO_BOX:
    SecurityComBoxChange(packet);
    break;
  case SECURITY_CURSOR_MOVE:
    SecurityCursorMove();
    break;
  case BUL_TOGGLE_BTN:
    BullionShowHide(packet);
    break;
  case BUL_OK_BTN:
    pthread_create(&thread_id, NULL, GUIThreadHandler_bul_ok, packet);
    pthread_detach(thread_id);
    break;
  case BUL_COMBO_BOX:
    BullionComBoxChange();
    break;
  case BUL_CURSOR_MOVE:
    BullionCursorMove();
    break;
  case CASH_TOGGLE_BTN:
    CashShowHide(packet);
    break;
  case CASH_OK_BTN:
    pthread_create(&thread_id, NULL, GUIThreadHandler_cash_ok, packet);
    pthread_detach(thread_id);
    break;
  case CASH_CURSOR_MOVE:
    CashCursorMove();
    break;
  case API_TOGGLE_BTN:
    APIShowHide(packet);
    break;
  case API_OK_BTN:
    pthread_create(&thread_id, NULL, GUIThreadHandler_api_ok, packet);
    pthread_detach(thread_id);
    break;
  case API_CURSOR_MOVE:
    APICursorMove();
    break;
  case PREF_TOGGLE_BTN:
    PrefShowHide(packet);
    break;
  case PREF_SYMBOL_UPDATE_BTN:
    pthread_create(&thread_id, NULL, GUIThreadHandler_sym_name_update, packet);
    pthread_detach(thread_id);
    break;
  case HISTORY_TOGGLE_BTN:
    HistoryTreeViewClear();
    HistoryShowHide(packet);
    break;
  case HISTORY_CURSOR_MOVE:
    HistoryCursorMove();
    break;
  case HOTKEYS_TOGGLE_BTN:
    HotkeysShowHide();
    break;
  default:
    break;
  }
}

void GUICallbackHandler_security_stack(GObject *gobject) {
  GtkStack *stack = GTK_STACK(gobject);
  const gchar *name = gtk_stack_get_visible_child_name(stack);

  GtkWidget *ComboBox = GetWidget("SecurityComboBox");
  GtkWidget *EntryBoxSymbol = GetWidget("SecuritySymbolEntryBox");
  GtkWidget *EntryBoxShares = GetWidget("SecuritySharesEntryBox");
  GtkWidget *Button = GetWidget("SecurityOkBTN");

  if (strcasecmp(name, "add") == 0) {
    gtk_combo_box_set_button_sensitivity(GTK_COMBO_BOX(ComboBox),
                                         GTK_SENSITIVITY_OFF);
    gtk_combo_box_set_active(GTK_COMBO_BOX(ComboBox), 0);
    gtk_widget_set_sensitive(EntryBoxSymbol, true);
    gtk_widget_set_sensitive(EntryBoxShares, true);
    gtk_widget_set_sensitive(Button, false);

    /* Reset EntryBoxes */
    gtk_entry_set_text(GTK_ENTRY(EntryBoxSymbol), "");
    gtk_entry_set_text(GTK_ENTRY(EntryBoxShares), "");
  } else {
    gtk_combo_box_set_button_sensitivity(GTK_COMBO_BOX(ComboBox),
                                         GTK_SENSITIVITY_AUTO);
    gtk_widget_set_sensitive(EntryBoxSymbol, false);
    gtk_widget_set_sensitive(EntryBoxShares, false);
    gtk_widget_set_sensitive(Button, false);

    /* Reset EntryBoxes */
    gtk_entry_set_text(GTK_ENTRY(EntryBoxSymbol), "");
    gtk_entry_set_text(GTK_ENTRY(EntryBoxShares), "");
  }
}

typedef struct {
  const char *keyword;
  const char *value;
} pref_data;

static pref_data *pref_data_init(const char *keyword, const char *value) {
  pref_data *pref_d = malloc(sizeof *pref_d);
  pref_d->keyword = keyword;
  pref_d->value = value;
  return pref_d;
}

/* These thread funcs have mutexes that shouldn't be in the main loop, so we
 * make them threads instead. */
static void *add_pref_data_thd(void *data) {
  pref_data *pref_d = (pref_data *)data;
  meta *D = packet->GetMetaClass();

  SqlitePrefAdd(pref_d->keyword, pref_d->value, D);

  /* Don't free the member strings */
  free(pref_d);
  pthread_exit(NULL);
}

static void *add_pref_data_font_thd(void *data) {
  pref_data *pref_d = (pref_data *)data;
  meta *D = packet->GetMetaClass();

  SqlitePrefAdd(pref_d->keyword, pref_d->value, D);

  packet->ToStrings();
  packet->equity_folder_class->SetSecurityNames(packet);

  /* Make sure font is set on the main window
     labels and the treeview header strings. */
  gdk_threads_add_idle(MainSetFonts, packet);

  if (packet->IsDefaultView()) {
    gdk_threads_add_idle(MainDefaultTreeview, packet);
  } else {
    gdk_threads_add_idle(MainPrimaryTreeview, packet);
  }

  /* Don't free the member strings */
  free(pref_d);
  pthread_exit(NULL);
}

void GUICallbackHandler_pref_font_button(GtkFontButton *widget) {
  meta *D = packet->GetMetaClass();
  pthread_t thread_id;

  free(D->main_font_ch);
  D->main_font_ch = gtk_font_chooser_get_font(GTK_FONT_CHOOSER(widget));
  /* Set the pango_formatting.c font variable. */
  SetFont(D->main_font_ch);

  /* Add the sqlite data */
  pref_data *pref_d = pref_data_init("Main_Font", D->main_font_ch);
  pthread_create(&thread_id, NULL, add_pref_data_font_thd, pref_d);
  pthread_detach(thread_id);
}

gboolean GUICallbackHandler_pref_clock_switch(GtkSwitch *Switch, bool state) {
  UNUSED(Switch)

  meta *D = packet->GetMetaClass();
  pthread_t thread_id;

  /* Visually, the underlying state is represented by the trough color of the
     switch, while the “active” property is represented by the position of the
     switch. */

  packet->SetClockDisplayed(state);

  if (state) {
    /* Add the sqlite data */
    pref_data *pref_d = pref_data_init("Clocks_Displayed", "true");
    pthread_create(&thread_id, NULL, add_pref_data_thd, pref_d);
    pthread_detach(thread_id);

    /* Start clock threads */
    pthread_create(&D->thread_id_clock, NULL, GUIThreadHandler_main_clock,
                   packet);
    pthread_create(&D->thread_id_closing_time, NULL,
                   GUIThreadHandler_time_to_close, packet);
    pthread_detach(D->thread_id_clock);
    pthread_detach(D->thread_id_closing_time);

    /* Show revealer */
    MainDisplayClocks(packet);

  } else {
    /* Add the sqlite data */
    pref_data *pref_d = pref_data_init("Clocks_Displayed", "false");
    pthread_create(&thread_id, NULL, add_pref_data_thd, pref_d);
    pthread_detach(thread_id);

    /* Cancel clock threads */
    pthread_cancel(D->thread_id_clock);
    pthread_cancel(D->thread_id_closing_time);
    pthread_join(D->thread_id_clock, NULL);
    pthread_join(D->thread_id_closing_time, NULL);

    /* Hide revealer */
    MainDisplayClocks(packet);
  }

  /* Return false to keep the state
     and active properties in sync. */
  return false;
}

gboolean GUICallbackHandler_pref_indices_switch(GtkSwitch *Switch, bool state) {
  UNUSED(Switch)

  pthread_t thread_id;

  /* Visually, the underlying state is represented by the trough color of the
     switch, while the “active” property is represented by the position of the
     switch. */
  if (state) {
    /* Add the sqlite data */
    pref_data *pref_d = pref_data_init("Indices_Displayed", "true");
    pthread_create(&thread_id, NULL, add_pref_data_thd, pref_d);
    pthread_detach(thread_id);
  } else {
    pref_data *pref_d = pref_data_init("Indices_Displayed", "false");
    pthread_create(&thread_id, NULL, add_pref_data_thd, pref_d);
    pthread_detach(thread_id);
  }
  packet->SetIndicesDisplayed(state);

  if (!packet->IsDefaultView()) {
    /* If we are not displaying the default view, reveal/hide the indices bar */
    GtkWidget *revealer = GetWidget("MainIndicesRevealer");
    gtk_revealer_set_reveal_child(GTK_REVEALER(revealer),
                                  packet->IsIndicesDisplayed());
  } /* The default view always hides the indices bar, no need for an else
       statement here. */

  /* Return false to keep the state and active properties in sync. */
  return false;
}

static void *pref_dec_places_thd(void *data) {
  char *value = (char *)data;
  meta *D = packet->GetMetaClass();

  SqlitePrefAdd("Decimal_Places", value, D);

  packet->Calculate();
  packet->ToStrings();

  if (packet->IsDefaultView()) {
    gdk_threads_add_idle(MainDefaultTreeview, packet);
  } else {
    gdk_threads_add_idle(MainPrimaryTreeview, packet);
  }

  free(value);
  pthread_exit(NULL);
}

void GUICallbackHandler_pref_dec_places_combobox(GtkComboBox *ComboBox) {
  meta *D = packet->GetMetaClass();
  pthread_t thread_id;

  gchar *new = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(ComboBox));
  unsigned short new_shrt = (unsigned short)strtol(new, NULL, 10);

  if (new_shrt != D->decimal_places_shrt) {
    D->decimal_places_shrt = new_shrt;

    /* new is freed in the pref_dec_places_thd func */
    pthread_create(&thread_id, NULL, pref_dec_places_thd, new);
    pthread_detach(thread_id);
  }
}

static void *pref_up_min_thd(void *data) {
  char *value = (char *)data;
  meta *D = packet->GetMetaClass();

  SqlitePrefAdd("Updates_Per_Min", value, D);

  free(value);
  pthread_exit(NULL);
}

void GUICallbackHandler_pref_up_min_combobox(GtkComboBox *ComboBox) {
  meta *D = packet->GetMetaClass();
  pthread_t thread_id;

  gchar *new = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(ComboBox));
  double new_f = strtod(new, NULL);

  if (new_f != D->updates_per_min_f) {
    D->updates_per_min_f = new_f;

    /* new is freed in the pref_up_min_thd func */
    pthread_create(&thread_id, NULL, pref_up_min_thd, new);
    pthread_detach(thread_id);
  }
}

static void *pref_hours_thd(void *data) {
  pref_data *pref_d = (pref_data *)data;
  meta *D = packet->GetMetaClass();

  SqlitePrefAdd(pref_d->keyword, pref_d->value, D);

  /* Don't free the member strings */
  free(pref_d);

  pthread_exit(NULL);
}

void GUICallbackHandler_pref_hours_spinbutton(GtkEditable *spin_button) {
  meta *D = packet->GetMetaClass();
  const gchar *new = gtk_entry_get_text(GTK_ENTRY(spin_button));
  pthread_t thread_id;

  /* Right now this value will be an integral, however it may change to a
   * floating point in the future. */
  double new_f = strtod(new, NULL);
  gboolean check = (new_f <= 7) & CheckIfStringDoublePositiveNumber(new);
  check = check & (strlen(new) != 0);
  if (!check)
    return;

  if (new_f != D->updates_hours_f) {
    D->updates_hours_f = new_f;

    /* Add the data */
    pref_data *pref_d = pref_data_init("Updates_Hours", new);
    pthread_create(&thread_id, NULL, pref_hours_thd, pref_d);
    pthread_detach(thread_id);
  }
}

gboolean GUICallbackHandler_hide_window_on_delete(GtkWidget *window,
                                                  GdkEvent *event, void *data) {
  UNUSED(event)

  uintptr_t index_signal = (uintptr_t)data;

  switch (index_signal) {
  case ABOUT_TOGGLE_BTN:
    AboutShowHide();
    break;
  case HOTKEYS_TOGGLE_BTN:
    HotkeysShowHide();
    break;
  case SECURITY_TOGGLE_BTN:
    SecurityShowHide(packet);
    break;
  case CASH_TOGGLE_BTN:
    CashShowHide(packet);
    break;
  case BUL_TOGGLE_BTN:
    BullionShowHide(packet);
    break;
  case API_TOGGLE_BTN:
    APIShowHide(packet);
    break;
  case PREF_TOGGLE_BTN:
    PrefShowHide(packet);
    break;
  case HISTORY_TOGGLE_BTN:
    HistoryTreeViewClear();
    HistoryShowHide(packet);
    break;
  }
  return gtk_widget_hide_on_delete(window);
}

gboolean GUICallbackHandler_window_data(GtkWidget *window, GdkEvent *event,
                                        void *data) {
  /*
      The "event->configure.x" and "event->configure.y" data members are
     slightly less accurate than the gtk_window_get_position function, so we're
     not using the GdkEvent data type.
  */
  UNUSED(event)

  gint width, height, x, y;
  gtk_window_get_size(GTK_WINDOW(window), &width, &height);
  gtk_window_get_position(GTK_WINDOW(window), &x, &y);

  window_data *W = packet->GetWindowData();

  int s = (int)((uintptr_t)data);

  switch (s) {
  case GUI_MAIN_WINDOW:
    W->main_width = (int)width;
    W->main_height = (int)height;

    W->main_x_pos = (int)x;
    W->main_y_pos = (int)y;
    break;
  case GUI_HISTORY_WINDOW:
    W->history_width = (int)width;
    W->history_height = (int)height;

    W->history_x_pos = (int)x;
    W->history_y_pos = (int)y;
    break;
  }

  /* TRUE to stop other handlers from being invoked for the event.
     FALSE to propagate the event further.
  */
  return false;
}

gboolean GUICallbackHandler_select_comp(GtkEntryCompletion *completion,
                                        GtkTreeModel *model, GtkTreeIter *iter,
                                        void *data)
/* activated when an item is selected from the completion list */
{
  UNUSED(completion)

  /* We're using data as a value rather than a pointer. */
  uintptr_t index_signal = (uintptr_t)data;
  GtkWidget *EntryBox = NULL;

  if (index_signal == GUI_COMPLETION_HISTORY) {
    EntryBox = GetWidget("HistorySymbolEntryBox");
  } else {
    EntryBox = GetWidget("SecuritySymbolEntryBox");
  }

  gchar *item;
  /* when a match is selected insert column zero instead of column 2 */
  gtk_tree_model_get(model, iter, 0, &item, -1);
  /* This function is already blocking the gtk main loop, it's ok
     to change widgets here without using a "gdk_threads_add_idle" wrapper
     function. */
  gtk_entry_set_text(GTK_ENTRY(EntryBox), item);

  /* move the cursor to the end of the string */
  gtk_editable_set_position(GTK_EDITABLE(EntryBox), strlen(item));
  g_free(item);
  return true;
}

gboolean GUICallbackHandler_cursor_comp(GtkEntryCompletion *completion,
                                        GtkTreeModel *model, GtkTreeIter *iter,
                                        void *data)
/* activated when an item is highlighted from the completion list */
{
  UNUSED(completion)

  /* We're using data as a value rather than a pointer. */
  uintptr_t index_signal = (uintptr_t)data;
  GtkWidget *EntryBox = NULL;

  if (index_signal == GUI_COMPLETION_HISTORY) {
    EntryBox = GetWidget("HistorySymbolEntryBox");
  } else {
    EntryBox = GetWidget("SecuritySymbolEntryBox");
  }

  gchar *item;
  /* when a match is highlighted insert column zero instead of column 2 */
  gtk_tree_model_get(model, iter, 0, &item, -1);
  /* This function is already blocking the gtk main loop, it's ok
     to change widgets here without using a "gdk_threads_add_idle" wrapper
     function. */
  gtk_entry_set_text(GTK_ENTRY(EntryBox), item);

  /* move the cursor to the end of the string */
  gtk_editable_set_position(GTK_EDITABLE(EntryBox), strlen(item));
  g_free(item);
  return true;
}

static void view_popup_menu_onHistoryData(GtkWidget *menuitem, void *userdata) {
  UNUSED(menuitem)

  char *symbol = (char *)userdata;

  GtkWidget *Window = GetWidget("HistoryWindow");
  GtkWidget *EntryBox = GetWidget("HistorySymbolEntryBox");
  GtkWidget *Button = GetWidget("HistoryFetchDataBTN");
  gboolean visible = gtk_widget_is_visible(Window);

  if (!visible)
    HistoryShowHide(packet);

  gtk_entry_set_text(GTK_ENTRY(EntryBox), symbol);
  /* move the cursor to the end of the string */
  gtk_editable_set_position(GTK_EDITABLE(EntryBox), strlen(symbol));

  gtk_button_clicked(GTK_BUTTON(Button));
}

static void view_popup_menu_onViewSummary() {
  GtkWidget *Button = GetWidget("FetchDataBTN");
  if (packet->IsFetchingData())
    gtk_button_clicked(GTK_BUTTON(Button));

  MainDefaultTreeview(packet);
}

static void zeroize_bullion(bullion *B) {
  B->ounce_f = 0.0f;
  B->premium_f = 0.0f;
}

static void *delete_bullion_thd(void *data) {
  gchar *metal_name = (gchar *)data;
  metal *M = packet->GetMetalClass();
  meta *D = packet->GetMetaClass();
  bool gold = (strcasecmp(metal_name, "gold") == 0);
  bool silver = (strcasecmp(metal_name, "silver") == 0);
  bool platinum = (strcasecmp(metal_name, "platinum") == 0);
  bool palladium = (strcasecmp(metal_name, "palladium") == 0);

  if (gold) {
    zeroize_bullion(M->Gold);
  } else if (silver) {
    zeroize_bullion(M->Silver);
  } else if (platinum) {
    zeroize_bullion(M->Platinum);
  } else if (palladium) {
    zeroize_bullion(M->Palladium);
  }

  SqliteBullionAdd(metal_name, "0", "0", D);

  if (packet->IsDefaultView()) {
    packet->ToStrings();
    gdk_threads_add_idle(MainDefaultTreeview, packet);
  } else {
    packet->Calculate();
    packet->ToStrings();
    /* Set Gtk treeview. */
    gdk_threads_add_idle(MainPrimaryTreeview, packet);
  }

  g_free(metal_name);
  pthread_exit(NULL);
}

static void view_popup_menu_onDeleteBullion(GtkWidget *menuitem,
                                            void *userdata) {
  UNUSED(menuitem)

  const gchar *metal_name = (gchar *)userdata;

  pthread_t thread_id;
  pthread_create(&thread_id, NULL, delete_bullion_thd, strdup(metal_name));
  pthread_detach(thread_id);
}

static void *delete_all_bullion_thd() {
  metal *M = packet->GetMetalClass();
  meta *D = packet->GetMetaClass();

  SqliteBullionAdd("gold", "0", "0", D);
  SqliteBullionAdd("silver", "0", "0", D);
  SqliteBullionAdd("platinum", "0", "0", D);
  SqliteBullionAdd("palladium", "0", "0", D);

  zeroize_bullion(M->Gold);
  zeroize_bullion(M->Silver);
  zeroize_bullion(M->Platinum);
  zeroize_bullion(M->Palladium);

  if (packet->IsDefaultView()) {
    packet->ToStrings();
    gdk_threads_add_idle(MainDefaultTreeview, packet);
  } else {
    packet->Calculate();
    packet->ToStrings();
    /* Set Gtk treeview. */
    gdk_threads_add_idle(MainPrimaryTreeview, packet);
  }

  pthread_exit(NULL);
}

static void view_popup_menu_onDeleteAllBullion() {
  pthread_t thread_id;
  pthread_create(&thread_id, NULL, delete_all_bullion_thd, NULL);
  pthread_detach(thread_id);
}

static void *remove_stock_thd(void *data) {
  gchar *symbol = (gchar *)data;
  equity_folder *F = packet->GetEquityFolderClass();
  meta *D = packet->GetMetaClass();

  /* Prevents Program From Crashing During A Data Fetch Operation */
  pthread_mutex_lock(&mutex_working[FETCH_DATA_MUTEX]);

  SqliteEquityRemove(symbol, D);
  F->RemoveStock(symbol);

  pthread_mutex_unlock(&mutex_working[FETCH_DATA_MUTEX]);

  if (packet->IsDefaultView()) {
    gdk_threads_add_idle(MainDefaultTreeview, packet);
  } else {
    packet->Calculate();
    packet->ToStrings();
    /* Set Gtk treeview. */
    gdk_threads_add_idle(MainPrimaryTreeview, packet);
  }

  g_free(symbol);
  pthread_exit(NULL);
}

static void view_popup_menu_onDeleteEquity(GtkWidget *menuitem,
                                           void *userdata) {
  UNUSED(menuitem)

  const gchar *symbol = (gchar *)userdata;

  pthread_t thread_id;
  pthread_create(&thread_id, NULL, remove_stock_thd, strdup(symbol));
  pthread_detach(thread_id);
}

static void *remove_all_stocks_thd() {
  equity_folder *F = packet->GetEquityFolderClass();
  meta *D = packet->GetMetaClass();

  /* Prevents Program From Crashing During A Data Fetch Operation */
  pthread_mutex_lock(&mutex_working[FETCH_DATA_MUTEX]);

  SqliteEquityRemoveAll(D);
  F->Reset();

  pthread_mutex_unlock(&mutex_working[FETCH_DATA_MUTEX]);

  if (packet->IsDefaultView()) {
    gdk_threads_add_idle(MainDefaultTreeview, packet);
  } else {
    packet->Calculate();
    packet->ToStrings();
    /* Set Gtk treeview. */
    gdk_threads_add_idle(MainPrimaryTreeview, packet);
  }

  pthread_exit(NULL);
}

static void view_popup_menu_onDeleteAllEquity() {
  pthread_t thread_id;
  pthread_create(&thread_id, NULL, remove_all_stocks_thd, NULL);
  pthread_detach(thread_id);
}

static void view_popup_menu_onAddRow(GtkWidget *menuitem, void *userdata) {
  UNUSED(menuitem)

  char *type = (char *)userdata;

  if (strcmp(type, "equity") == 0) {
    SecurityShowHide(packet);
  } else if (strcmp(type, "equity_total") == 0) {
    SecurityShowHide(packet);
  } else if (strcmp(type, "bullion") == 0) {
    BullionShowHide(packet);
  } else if (strcmp(type, "bullion_total") == 0) {
    BullionShowHide(packet);
  } else if (strcmp(type, "cash") == 0) {
    CashShowHide(packet);
  }
}

gboolean view_onButtonPressed(GtkWidget *treeview, GdkEventButton *event) {
  GtkTreeModel *model;
  GtkTreeIter iter;
  GtkWidget *menu, *menuitem;
  gchar *type = NULL;
  gchar *symbol = NULL;
  meta *D = packet->GetMetaClass();

  /* single click with the right mouse button */
  if (event->type == GDK_BUTTON_PRESS && event->button == 3) {
    /* optional: select row if no row is selected or only
     *  one other row is selected (will only do something
     *  if you set a tree selection mode) */
    if (1) {
      GtkTreeSelection *selection;

      selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(treeview));
      gtk_tree_selection_set_mode(selection, GTK_SELECTION_SINGLE);
      if (gtk_tree_selection_get_selected(selection, &model, &iter)) {
        gtk_tree_model_get(model, &iter, GUI_TYPE, &type, GUI_SYMBOL, &symbol,
                           -1);
        if (!type || !symbol) {
          if (!symbol) {
            g_free(type);
            return GDK_EVENT_STOP;
          }

          if (!type) {
            g_free(symbol);
            return GDK_EVENT_STOP;
          }
        }

        gboolean eq_flag = (strcmp(type, "equity") == 0);
        gboolean eq_tot_flag = (strcmp(type, "equity_total") == 0);
        gboolean bu_flag = (strcmp(type, "bullion") == 0);
        gboolean bu_tot_flag = (strcmp(type, "bullion_total") == 0);
        gboolean ca_flag = (strcmp(type, "cash") == 0);
        gboolean bs_d_flag = (strcmp(type, "blank_space_default") == 0);
        gboolean bs_p_flag = (strcmp(type, "blank_space_primary") == 0);

        /* Some of the right-click menu signal connections need the type and
           symbol strings. We store the data in the meta class and free the
           members on subsequent clicks, which keeps the strings available to
           the signal connections. */
        /* The meta class is available globally in this file, although I think
         * this method is more obvious [using the variable similarly to a static
         * local variable rather than a global variable].  The data is freed
         * when the meta class is destructed. */
        if (D->rght_clk_data.type)
          free(D->rght_clk_data.type);
        if (D->rght_clk_data.symbol)
          free(D->rght_clk_data.symbol);

        D->rght_clk_data.type = type;
        D->rght_clk_data.symbol = symbol;

        if (eq_flag)
        /* If the type is an equity enable row deletion. */
        {
          menu = gtk_menu_new();
          menuitem = gtk_menu_item_new_with_label("View History");
          g_signal_connect(menuitem, "activate",
                           G_CALLBACK(view_popup_menu_onHistoryData),
                           D->rght_clk_data.symbol);
          gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
          g_object_ref_sink(G_OBJECT(menuitem));

          menuitem = gtk_menu_item_new_with_label("Edit Equity");
          g_signal_connect(menuitem, "activate",
                           G_CALLBACK(view_popup_menu_onAddRow),
                           D->rght_clk_data.type);
          gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
          g_object_ref_sink(G_OBJECT(menuitem));

          size_t len = strlen("Delete ") + strlen(D->rght_clk_data.symbol) + 1;
          char *menu_label = (char *)malloc(len);
          snprintf(menu_label, len, "Delete %s", D->rght_clk_data.symbol);
          menuitem = gtk_menu_item_new_with_label(menu_label);
          g_signal_connect(menuitem, "activate",
                           G_CALLBACK(view_popup_menu_onDeleteEquity),
                           D->rght_clk_data.symbol);
          gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
          free(menu_label);
          g_object_ref_sink(G_OBJECT(menuitem));

          menuitem = gtk_menu_item_new_with_label("Delete All Equity");
          g_signal_connect(menuitem, "activate",
                           G_CALLBACK(view_popup_menu_onDeleteAllEquity), NULL);
          gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
          g_object_ref_sink(G_OBJECT(menuitem));

          gtk_widget_show_all(menu);
          gtk_menu_popup_at_pointer(GTK_MENU(menu), (GdkEvent *)event);

          g_object_ref_sink(G_OBJECT(menu));

        } else if (eq_tot_flag) {
          menu = gtk_menu_new();
          menuitem = gtk_menu_item_new_with_label("Edit Equity");
          g_signal_connect(menuitem, "activate",
                           G_CALLBACK(view_popup_menu_onAddRow),
                           D->rght_clk_data.type);
          gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
          g_object_ref_sink(G_OBJECT(menuitem));

          menuitem = gtk_menu_item_new_with_label("Delete All Equity");
          g_signal_connect(menuitem, "activate",
                           G_CALLBACK(view_popup_menu_onDeleteAllEquity), NULL);
          gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
          g_object_ref_sink(G_OBJECT(menuitem));

          gtk_widget_show_all(menu);
          gtk_menu_popup_at_pointer(GTK_MENU(menu), (GdkEvent *)event);

          g_object_ref_sink(G_OBJECT(menu));

        } else if (bu_flag || bu_tot_flag || ca_flag) {
          if (bu_flag || bu_tot_flag) {
            menuitem = gtk_menu_item_new_with_label("Edit Bullion");
          } else {
            menuitem = gtk_menu_item_new_with_label("Edit Cash");
          }

          menu = gtk_menu_new();
          g_signal_connect(menuitem, "activate",
                           G_CALLBACK(view_popup_menu_onAddRow),
                           D->rght_clk_data.type);
          gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
          g_object_ref_sink(G_OBJECT(menuitem));

          if (bu_flag) {
            D->rght_clk_data.symbol[0] = toupper(D->rght_clk_data.symbol[0]);
            size_t len =
                strlen("Delete ") + strlen(D->rght_clk_data.symbol) + 1;
            char *menu_label = (char *)malloc(len);
            snprintf(menu_label, len, "Delete %s", D->rght_clk_data.symbol);
            D->rght_clk_data.symbol[0] = tolower(D->rght_clk_data.symbol[0]);
            menuitem = gtk_menu_item_new_with_label(menu_label);
            g_signal_connect(menuitem, "activate",
                             G_CALLBACK(view_popup_menu_onDeleteBullion),
                             D->rght_clk_data.symbol);
            gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
            free(menu_label);
            g_object_ref_sink(G_OBJECT(menuitem));
          }

          if (bu_flag || bu_tot_flag) {
            menuitem = gtk_menu_item_new_with_label("Delete All Bullion");
            g_signal_connect(menuitem, "activate",
                             G_CALLBACK(view_popup_menu_onDeleteAllBullion),
                             NULL);
            gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
            g_object_ref_sink(G_OBJECT(menuitem));
          }

          gtk_widget_show_all(menu);
          gtk_menu_popup_at_pointer(GTK_MENU(menu), (GdkEvent *)event);

          g_object_ref_sink(G_OBJECT(menu));

        } else if (bs_d_flag || bs_p_flag) {
          menu = gtk_menu_new();

          if (bs_p_flag) {
            menuitem = gtk_menu_item_new_with_label("View Summary");
            g_signal_connect(menuitem, "activate",
                             G_CALLBACK(view_popup_menu_onViewSummary), NULL);
            gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
            g_object_ref_sink(G_OBJECT(menuitem));
          }

          menuitem = gtk_menu_item_new_with_label("Edit Cash");
          g_signal_connect(menuitem, "activate",
                           G_CALLBACK(view_popup_menu_onAddRow), "cash");
          gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
          g_object_ref_sink(G_OBJECT(menuitem));

          menuitem = gtk_menu_item_new_with_label("Edit Bullion");
          g_signal_connect(menuitem, "activate",
                           G_CALLBACK(view_popup_menu_onAddRow), "bullion");
          gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
          g_object_ref_sink(G_OBJECT(menuitem));

          menuitem = gtk_menu_item_new_with_label("Edit Equity");
          g_signal_connect(menuitem, "activate",
                           G_CALLBACK(view_popup_menu_onAddRow), "equity");
          gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
          g_object_ref_sink(G_OBJECT(menuitem));

          gtk_widget_show_all(menu);
          gtk_menu_popup_at_pointer(GTK_MENU(menu), (GdkEvent *)event);

          g_object_ref_sink(G_OBJECT(menu));
        }
      }

      /* Note: gtk_tree_selection_count_selected_rows() does not
       *   exist in gtk+-2.0, only in gtk+ >= v2.2 ! */
      if (gtk_tree_selection_count_selected_rows(selection) <= 1) {
        GtkTreePath *path;

        /* Get tree path for row that was clicked */
        if (gtk_tree_view_get_path_at_pos(GTK_TREE_VIEW(treeview),
                                          (gint)event->x, (gint)event->y, &path,
                                          NULL, NULL, NULL)) {
          gtk_tree_selection_unselect_all(selection);
          gtk_tree_selection_select_path(selection, path);
          gtk_tree_path_free(path);
        }
      }
    } /* end of optional bit */
    return GDK_EVENT_STOP;
  }
  return GDK_EVENT_PROPAGATE;
}