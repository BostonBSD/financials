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

#include "../include/gui.h"

#include "../include/class_types.h" /* portfolio_packet, equity_folder, metal, meta, window_data */
#include "../include/globals.h" /* portfolio_packet packet */
#include "../include/macros.h"
#include "../include/mutex.h" /* GMutex mutexes */
#include "../include/sqlite.h"
#include "../include/workfuncs.h" /* CheckIfStringDoublePositiveNumber (), SetFont () */

void GUICallbackHandler(GtkWidget *widget, gpointer sig_data) {
  UNUSED(widget)
  GThread *g_thread_id;

  /* We're using data as a value rather than a pointer. */
  cb_signal index_signal = (cb_signal)((guintptr)sig_data);

  switch (index_signal) {
  case MAIN_FETCH_BTN:
    g_thread_id = g_thread_new(NULL, GUIThreadHandler_main_fetch, packet);
    g_thread_unref(g_thread_id);
    break;
  case MAIN_EXIT:
    g_thread_id = g_thread_new(NULL, GUIThread_main_exit, packet);
    g_thread_unref(g_thread_id);
    break;
  case HISTORY_FETCH_BTN:
    g_thread_id = g_thread_new(NULL, GUIThread_history_fetch, packet);
    g_thread_unref(g_thread_id);
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
    BullionShowHide(packet);
    /* If we need to update the main treeview with new bullion data. */
    if (BullionOk(packet) && !packet->IsDefaultView()) {
      /* Fetch the data in a separate thread */
      g_thread_id = g_thread_new(NULL, GUIThread_bul_fetch, packet);
      g_thread_unref(g_thread_id);

      /* Recalculate and update the treeview. */
    } else {
      g_thread_id = g_thread_new(NULL, GUIThread_recalculate, packet);
      g_thread_unref(g_thread_id);
    }
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
    CashShowHide(packet);
    CashOk(packet);
    g_thread_id = g_thread_new(NULL, GUIThread_recalculate, packet);
    g_thread_unref(g_thread_id);
    break;
  case CASH_CURSOR_MOVE:
    CashCursorMove();
    break;
  case API_TOGGLE_BTN:
    APIShowHide(packet);
    break;
  case API_OK_BTN:
    APIShowHide(packet);
    g_thread_id = g_thread_new(NULL, GUIThread_api_ok, packet);
    g_thread_unref(g_thread_id);
    break;
  case API_CURSOR_MOVE:
    APICursorMove();
    break;
  case PREF_TOGGLE_BTN:
    PrefShowHide(packet);
    break;
  case PREF_SYMBOL_UPDATE_BTN:
    g_thread_id = g_thread_new(NULL, GUIThread_pref_sym_update, packet);
    g_thread_unref(g_thread_id);
    break;
  case HISTORY_TOGGLE_BTN:
    HistoryShowHide(packet);
    HistoryTreeViewClear();
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

void GUICallback_security_stack(GObject *gobject) {
  GtkStack *stack = GTK_STACK(gobject);
  const gchar *name = gtk_stack_get_visible_child_name(stack);

  GtkWidget *ComboBox = GetWidget("SecurityComboBox");
  GtkWidget *EntryBoxSymbol = GetWidget("SecuritySymbolEntryBox");
  GtkWidget *EntryBoxShares = GetWidget("SecuritySharesEntryBox");
  GtkWidget *Button = GetWidget("SecurityOkBTN");

  if (g_strcmp0(name, "add") == 0) {
    gtk_combo_box_set_button_sensitivity(GTK_COMBO_BOX(ComboBox),
                                         GTK_SENSITIVITY_OFF);
    gtk_combo_box_set_active(GTK_COMBO_BOX(ComboBox), 0);
    gtk_widget_set_sensitive(EntryBoxSymbol, TRUE);
    gtk_widget_set_sensitive(EntryBoxShares, TRUE);
    gtk_widget_set_sensitive(Button, FALSE);

    /* Reset EntryBoxes */
    gtk_entry_set_text(GTK_ENTRY(EntryBoxSymbol), "");
    gtk_entry_set_text(GTK_ENTRY(EntryBoxShares), "");
  } else {
    gtk_combo_box_set_button_sensitivity(GTK_COMBO_BOX(ComboBox),
                                         GTK_SENSITIVITY_AUTO);
    gtk_widget_set_sensitive(EntryBoxSymbol, FALSE);
    gtk_widget_set_sensitive(EntryBoxShares, FALSE);
    gtk_widget_set_sensitive(Button, FALSE);

    /* Reset EntryBoxes */
    gtk_entry_set_text(GTK_ENTRY(EntryBoxSymbol), "");
    gtk_entry_set_text(GTK_ENTRY(EntryBoxShares), "");
  }
}

void GUICallback_pref_font_button(GtkFontButton *widget) {
  meta *D = packet->GetMetaClass();
  GThread *g_thread_id;

  g_free(D->font_ch);
  D->font_ch = gtk_font_chooser_get_font(GTK_FONT_CHOOSER(widget));
  /* Set the pango_formatting.c font variable. */
  SetFont(D->font_ch);

  packet->equity_folder_class->SetSecurityNames(packet);
  /* Make sure font is set on the main window
     labels and the treeview header strings. */
  MainSetFonts(packet);

  /* Update the application with the new font */
  g_thread_id = g_thread_new(NULL, GUIThread_recalculate, packet);
  g_thread_unref(g_thread_id);
}

gboolean GUICallback_pref_clock_switch(GtkSwitch *Switch, gboolean state) {
  UNUSED(Switch)
  UNUSED(state)
  GThread *g_thread_id;

  /* Visually, the underlying state is represented by the trough color of the
     switch, while the “active” property is represented by the position of the
     switch. */

  /* Start/Stop clock threads, will toggle the clocks_displayed_bool flag */
  g_thread_id = g_thread_new(NULL, GUIThreadHandler_clock, packet);
  g_thread_unref(g_thread_id);

  /* Return FALSE to keep the state
     and active properties in sync. */
  return FALSE;
}

gboolean GUICallback_pref_indices_switch(GtkSwitch *Switch, gboolean state) {
  UNUSED(Switch)

  /* Visually, the underlying state is represented by the trough color of the
     switch, while the “active” property is represented by the position of the
     switch. */
  packet->SetIndicesDisplayed(state);

  if (!packet->IsDefaultView()) {
    /* If we are not displaying the default view, reveal/hide the indices bar */
    GtkWidget *revealer = GetWidget("MainIndicesRevealer");
    gtk_revealer_set_reveal_child(GTK_REVEALER(revealer),
                                  packet->IsIndicesDisplayed());
  } /* The default view always hides the indices bar, no need for an else
       statement here. */

  /* Return FALSE to keep the state and active properties in sync. */
  return FALSE;
}

void GUICallback_pref_dec_places_combobox(GtkComboBox *ComboBox) {
  meta *D = packet->GetMetaClass();
  GThread *g_thread_id;

  gchar *new = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(ComboBox));
  guint8 new_guint8 = (guint8)g_ascii_strtoll(new, NULL, 10);
  g_free(new);

  if (new_guint8 != D->decimal_places_guint8) {
    D->decimal_places_guint8 = new_guint8;

    /* Recalculate values and update the treeview */
    g_thread_id = g_thread_new(NULL, GUIThread_recalculate, packet);
    g_thread_unref(g_thread_id);
  }
}

void GUICallback_pref_up_min_combobox(GtkComboBox *ComboBox) {
  meta *D = packet->GetMetaClass();

  gchar *new = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(ComboBox));
  gdouble new_f = g_strtod(new, NULL);
  g_free(new);

  if (new_f != D->updates_per_min_f)
    D->updates_per_min_f = new_f;
}

void GUICallback_pref_hours_spinbutton(GtkEditable *spin_button) {
  meta *D = packet->GetMetaClass();
  const gchar *new = gtk_entry_get_text(GTK_ENTRY(spin_button));

  /* Right now this value will be an integral, however it may change to a
   * floating point in the future. */
  gdouble new_f = g_strtod(new, NULL);
  gboolean check = (new_f <= 7) & CheckIfStringDoublePositiveNumber(new);
  check = check & (g_utf8_strlen(new, -1) != 0);
  if (!check)
    return;

  if (new_f != D->updates_hours_f)
    D->updates_hours_f = new_f;
}

gboolean GUICallback_hide_window_on_delete(GtkWidget *window, GdkEvent *event,
                                           gpointer sig_data) {
  UNUSED(event)

  guintptr index_signal = (guintptr)sig_data;

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

gboolean GUICallback_window_data(GtkWidget *window, GdkEvent *event,
                                 gpointer sig_data) {
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

  guint s = (guint)((guintptr)sig_data);

  switch (s) {
  case GUI_MAIN_WINDOW:
    W->main_width = (gushort)width;
    W->main_height = (gushort)height;

    W->main_x_pos = (gushort)x;
    W->main_y_pos = (gushort)y;
    break;
  case GUI_HISTORY_WINDOW:
    W->history_width = (gushort)width;
    W->history_height = (gushort)height;

    W->history_x_pos = (gushort)x;
    W->history_y_pos = (gushort)y;
    break;
  }

  /* TRUE to stop other handlers from being invoked for the event.
     FALSE to propagate the event further.
  */
  return FALSE;
}

gboolean GUICallback_select_comp(GtkEntryCompletion *completion,
                                 GtkTreeModel *model, GtkTreeIter *iter,
                                 gpointer sig_data)
/* activated when an item is selected from the completion list */
{
  UNUSED(completion)

  /* We're using data as a value rather than a pointer. */
  guintptr index_signal = (guintptr)sig_data;
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
  gtk_editable_set_position(GTK_EDITABLE(EntryBox), g_utf8_strlen(item, -1));
  g_free(item);
  return TRUE;
}

gboolean GUICallback_cursor_comp(GtkEntryCompletion *completion,
                                 GtkTreeModel *model, GtkTreeIter *iter,
                                 gpointer sig_data)
/* activated when an item is highlighted from the completion list */
{
  UNUSED(completion)

  /* We're using data as a value rather than a pointer. */
  guintptr index_signal = (guintptr)sig_data;
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
  gtk_editable_set_position(GTK_EDITABLE(EntryBox), g_utf8_strlen(item, -1));
  g_free(item);
  return TRUE;
}

static void popup_menu_history_data(GtkWidget *menuitem, gpointer userdata) {
  UNUSED(menuitem)

  gchar *symbol = (gchar *)userdata;

  GtkWidget *Window = GetWidget("HistoryWindow");
  GtkWidget *EntryBox = GetWidget("HistorySymbolEntryBox");
  GtkWidget *Button = GetWidget("HistoryFetchDataBTN");
  gboolean visible = gtk_widget_is_visible(Window);

  if (!visible)
    HistoryShowHide(packet);

  gtk_entry_set_text(GTK_ENTRY(EntryBox), symbol);
  /* move the cursor to the end of the string */
  gtk_editable_set_position(GTK_EDITABLE(EntryBox), g_utf8_strlen(symbol, -1));

  gtk_button_clicked(GTK_BUTTON(Button));
}

static void popup_menu_view_summary() {
  GtkWidget *Button = GetWidget("FetchDataBTN");
  if (packet->IsFetchingData())
    gtk_button_clicked(GTK_BUTTON(Button));

  MainDefaultTreeview(packet);
}

static void zeroize_bullion(bullion *B) {
  B->ounce_f = 0.0f;
  B->premium_f = 0.0f;
}

static void popup_menu_delete_bullion(GtkWidget *menuitem, gpointer userdata) {
  UNUSED(menuitem)
  const gchar *metal_name = (gchar *)userdata;

  metal *M = packet->GetMetalClass();

  gboolean gold = (g_strcmp0(metal_name, "gold") == 0);
  gboolean silver = (g_strcmp0(metal_name, "silver") == 0);
  gboolean platinum = (g_strcmp0(metal_name, "platinum") == 0);
  gboolean palladium = (g_strcmp0(metal_name, "palladium") == 0);

  if (gold) {
    zeroize_bullion(M->Gold);
  } else if (silver) {
    zeroize_bullion(M->Silver);
  } else if (platinum) {
    zeroize_bullion(M->Platinum);
  } else if (palladium) {
    zeroize_bullion(M->Palladium);
  }

  GThread *g_thread_id;
  g_thread_id = g_thread_new(NULL, GUIThread_recalculate, packet);
  g_thread_unref(g_thread_id);
}

static void popup_menu_delete_all_bullion() {
  metal *M = packet->GetMetalClass();

  zeroize_bullion(M->Gold);
  zeroize_bullion(M->Silver);
  zeroize_bullion(M->Platinum);
  zeroize_bullion(M->Palladium);

  GThread *g_thread_id;
  g_thread_id = g_thread_new(NULL, GUIThread_recalculate, packet);
  g_thread_unref(g_thread_id);
}

/* These thread funcs have mutexes that shouldn't be in the main loop, so we
 * make them threads instead. */
static gpointer remove_stock_thd(gpointer data) {
  gchar *symbol = (gchar *)data;
  equity_folder *F = packet->GetEquityFolderClass();
  meta *D = packet->GetMetaClass();

  /* Prevents Program From Crashing During A Data Fetch Operation */
  g_mutex_lock(&mutexes[FETCH_DATA_MUTEX]);

  SqliteEquityRemove(symbol, D);
  F->RemoveStock(symbol);

  g_mutex_unlock(&mutexes[FETCH_DATA_MUTEX]);

  if (packet->IsDefaultView()) {
    gdk_threads_add_idle(MainDefaultTreeview, packet);
  } else {
    packet->Calculate();
    packet->ToStrings();
    /* Set Gtk treeview. */
    gdk_threads_add_idle(MainPrimaryTreeview, packet);
  }

  g_free(symbol);
  g_thread_exit(NULL);
  return NULL;
}

static void popup_menu_delete_equity(GtkWidget *menuitem, gpointer userdata) {
  UNUSED(menuitem)

  const gchar *symbol = (gchar *)userdata;

  GThread *g_thread_id;
  g_thread_id = g_thread_new(NULL, remove_stock_thd, g_strdup(symbol));
  g_thread_unref(g_thread_id);
}

static gpointer remove_all_stocks_thd() {
  equity_folder *F = packet->GetEquityFolderClass();
  meta *D = packet->GetMetaClass();

  /* Prevents Program From Crashing During A Data Fetch Operation */
  g_mutex_lock(&mutexes[FETCH_DATA_MUTEX]);

  SqliteEquityRemoveAll(D);
  F->Reset();

  g_mutex_unlock(&mutexes[FETCH_DATA_MUTEX]);

  if (packet->IsDefaultView()) {
    gdk_threads_add_idle(MainDefaultTreeview, packet);
  } else {
    packet->Calculate();
    packet->ToStrings();
    /* Set Gtk treeview. */
    gdk_threads_add_idle(MainPrimaryTreeview, packet);
  }

  g_thread_exit(NULL);
  return NULL;
}

static void popup_menu_delete_all_equity() {
  GThread *g_thread_id;
  g_thread_id = g_thread_new(NULL, remove_all_stocks_thd, NULL);
  g_thread_unref(g_thread_id);
}

static void popup_menu_add_row(GtkWidget *menuitem, gpointer userdata) {
  UNUSED(menuitem)

  gchar *type = (gchar *)userdata;

  if (g_strcmp0(type, "equity") == 0) {
    SecurityShowHide(packet);
  } else if (g_strcmp0(type, "equity_total") == 0) {
    SecurityShowHide(packet);
  } else if (g_strcmp0(type, "bullion") == 0) {
    BullionShowHide(packet);
  } else if (g_strcmp0(type, "bullion_total") == 0) {
    BullionShowHide(packet);
  } else if (g_strcmp0(type, "cash") == 0) {
    CashShowHide(packet);
  }
}

static void create_menu_item(GtkWidget *menu, const gchar *menu_label_ch,
                             void (*func)(), gpointer data) {
  GtkWidget *menuitem = gtk_menu_item_new_with_label(menu_label_ch);
  g_signal_connect(menuitem, "activate", G_CALLBACK(func), data);
  gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
  g_object_ref_sink(G_OBJECT(menuitem));
}

gboolean GUICallback_main_treeview_click(GtkWidget *treeview,
                                         GdkEventButton *event) {
  GtkTreeModel *model;
  GtkTreeIter iter;
  GtkWidget *menu;
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
        gtk_tree_model_get(model, &iter, MAIN_COLUMN_TYPE, &type,
                           MAIN_COLUMN_SYMBOL, &symbol, -1);
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

        gboolean eq_flag = (g_strcmp0(type, "equity") == 0);
        gboolean eq_tot_flag = (g_strcmp0(type, "equity_total") == 0);
        gboolean bu_flag = (g_strcmp0(type, "bullion") == 0);
        gboolean bu_tot_flag = (g_strcmp0(type, "bullion_total") == 0);
        gboolean ca_flag = (g_strcmp0(type, "cash") == 0);
        gboolean bs_d_flag = (g_strcmp0(type, "blank_space_default") == 0);
        gboolean bs_p_flag = (g_strcmp0(type, "blank_space_primary") == 0);

        /* Some of the right-click menu signal connections need the type and
           symbol strings. We store the data in the meta class and free the
           members on subsequent clicks, which keeps the strings available to
           the signal connections. */
        /* The meta class is available globally in this file, although I think
         * this method is more obvious [using the variable similarly to a static
         * local variable rather than a global variable].  The data is freed
         * when the meta class is destructed. */
        if (D->rght_clk_data.type)
          g_free(D->rght_clk_data.type);
        if (D->rght_clk_data.symbol)
          g_free(D->rght_clk_data.symbol);

        D->rght_clk_data.type = type;
        D->rght_clk_data.symbol = symbol;

        if (eq_flag)
        /* If the type is an equity enable row deletion. */
        {
          menu = gtk_menu_new();
          create_menu_item(menu, "View History", popup_menu_history_data,
                           D->rght_clk_data.symbol);
          create_menu_item(menu, "Edit Equity", popup_menu_add_row,
                           D->rght_clk_data.type);

          gchar *menu_label =
              g_strconcat("Delete ", D->rght_clk_data.symbol, NULL);
          create_menu_item(menu, menu_label, popup_menu_delete_equity,
                           D->rght_clk_data.symbol);
          g_free(menu_label);

          create_menu_item(menu, "Delete All Equity",
                           popup_menu_delete_all_equity, NULL);

          gtk_widget_show_all(menu);
          gtk_menu_popup_at_pointer(GTK_MENU(menu), (GdkEvent *)event);

          g_object_ref_sink(G_OBJECT(menu));

        } else if (eq_tot_flag) {
          menu = gtk_menu_new();

          create_menu_item(menu, "Edit Equity", popup_menu_add_row,
                           D->rght_clk_data.type);
          create_menu_item(menu, "Delete All Equity",
                           popup_menu_delete_all_equity, NULL);

          gtk_widget_show_all(menu);
          gtk_menu_popup_at_pointer(GTK_MENU(menu), (GdkEvent *)event);

          g_object_ref_sink(G_OBJECT(menu));

        } else if (bu_flag || bu_tot_flag || ca_flag) {
          menu = gtk_menu_new();
          if (bu_flag || bu_tot_flag) {
            create_menu_item(menu, "Edit Bullion", popup_menu_add_row,
                             D->rght_clk_data.type);
          } else {
            create_menu_item(menu, "Edit Cash", popup_menu_add_row,
                             D->rght_clk_data.type);
          }

          if (bu_flag) {
            D->rght_clk_data.symbol[0] =
                g_ascii_toupper(D->rght_clk_data.symbol[0]);
            gchar *menu_label =
                g_strconcat("Delete ", D->rght_clk_data.symbol, NULL);
            D->rght_clk_data.symbol[0] =
                g_ascii_tolower(D->rght_clk_data.symbol[0]);
            create_menu_item(menu, menu_label, popup_menu_delete_bullion,
                             D->rght_clk_data.symbol);
            g_free(menu_label);
          }

          if (bu_flag || bu_tot_flag) {
            create_menu_item(menu, "Delete All Bullion",
                             popup_menu_delete_all_bullion, NULL);
          }

          gtk_widget_show_all(menu);
          gtk_menu_popup_at_pointer(GTK_MENU(menu), (GdkEvent *)event);

          g_object_ref_sink(G_OBJECT(menu));

        } else if (bs_d_flag || bs_p_flag) {
          menu = gtk_menu_new();

          if (bs_p_flag) {
            create_menu_item(menu, "View Summary", popup_menu_view_summary,
                             NULL);
          }

          create_menu_item(menu, "Edit Cash", popup_menu_add_row, "cash");
          create_menu_item(menu, "Edit Bullion", popup_menu_add_row, "bullion");
          create_menu_item(menu, "Edit Equity", popup_menu_add_row, "equity");

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