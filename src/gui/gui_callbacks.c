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
#include <ctype.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include <glib-object.h>
#include <gtk/gtk.h>

#include "../include/gui.h"
#include "../include/gui_globals.h" /* GtkBuilder *builder */
#include "../include/gui_types.h"

#include "../include/class_types.h" /* portfolio_packet, equity_folder, metal, meta, window_data */
#include "../include/globals.h" /* portfolio_packet packet */
#include "../include/mutex.h" /* pthread_mutex_t mutex_working[ MUTEX_NUMBER ] */
#include "../include/sqlite.h"
#include "../include/workfuncs.h" /* LowerCaseStr () */

void GUICallbackHandler(GtkWidget *widget, void *data)
/* The widget callback functions block the gui loop until they return,
   therefore we do not want a pthread_join statement in this function. */
{
  /* The Gtk3.0 callback function prototype includes a widget parameter,
     which we do not use, the following statement prevents a compiler
     warning/error. */
  if (!(widget))
    return;

  pthread_t thread_id;
  /* Create a thread, pass the func and widget signal to it. */
  pthread_create(&thread_id, NULL, GUIThreadHandler, data);
}

void GUICallbackHandler_add_rem_stack(GObject *gobject, GParamSpec *pspec,
                                      void *data) {
  GtkStack *stack = GTK_STACK(gobject);
  const gchar *name = gtk_stack_get_visible_child_name(stack);

  GtkWidget *ComboBox =
      GTK_WIDGET(gtk_builder_get_object(builder, "AddRemoveSecurityComboBox"));
  GtkWidget *EntryBoxSymbol = GTK_WIDGET(
      gtk_builder_get_object(builder, "AddRemoveSecuritySymbolEntryBox"));
  GtkWidget *EntryBoxShares = GTK_WIDGET(
      gtk_builder_get_object(builder, "AddRemoveSecuritySharesEntryBox"));
  GtkWidget *Button =
      GTK_WIDGET(gtk_builder_get_object(builder, "AddRemoveSecurityOkBTN"));

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

gboolean GUICallbackHandler_pref_clock_switch(GtkSwitch *Switch, bool state,
                                              void *data) {
  meta *D = packet->GetMetaClass();

  /* Visually, the underlying state is represented by the trough color of the
     switch, while the “active” property is represented by the position of the
     switch. */

  packet->SetClockDisplayed(state);

  if (state) {
    SqliteAddAPIData("Clocks_Displayed", "true", D);
    MainDisplayClocks(packet);
    MainDisplayTime();
    MainDisplayTimeRemaining(packet);
  } else {
    SqliteAddAPIData("Clocks_Displayed", "false", D);
    MainDisplayClocks(packet);
  }

  /* Return false to keep the state and active properties in sync. */
  return false;
}

gboolean GUICallbackHandler_pref_indices_switch(GtkSwitch *Switch, bool state,
                                                void *data) {
  meta *D = packet->GetMetaClass();

  /* Visually, the underlying state is represented by the trough color of the
     switch, while the “active” property is represented by the position of the
     switch. */
  if (state) {
    SqliteAddAPIData("Indices_Displayed", "true", D);
  } else {
    SqliteAddAPIData("Indices_Displayed", "false", D);
  }
  packet->SetIndicesDisplayed(state);

  if (!packet->IsDefaultView()) {
    /* If we are not displaying the default view, reveal/hide the indices bar */
    GtkWidget *revealer =
        GTK_WIDGET(gtk_builder_get_object(builder, "MainIndicesRevealer"));
    gtk_revealer_set_reveal_child(GTK_REVEALER(revealer),
                                  packet->IsIndicesDisplayed());
  } /* The default view always hides the indices bar, no need for an else
       statement here. */

  /* Return false to keep the state and active properties in sync. */
  return false;
}

void GUICallbackHandler_pref_dec_places_combobox(GtkComboBox *ComboBox) {
  meta *D = packet->GetMetaClass();
  metal *M = packet->GetMetalClass();
  equity_folder *F = packet->GetEquityFolderClass();

  gchar *new = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(ComboBox));
  unsigned short new_shrt = (unsigned short)strtol(new, NULL, 10);

  if (new_shrt != D->decimal_places_shrt) {
    SqliteAddAPIData("Decimal_Places", new, D);
    /* Obviously I shouldn't need three variables to hold the same data.
       The sqlite callback functions use this variable, but do not
       have access to all three classes.  I could fix this later, by passing
       the portfolio_packet class in sqlite rather than one of these nested
       classes. */
    D->decimal_places_shrt = new_shrt;
    M->decimal_places_shrt = new_shrt;
    F->decimal_places_shrt = new_shrt;

    packet->Calculate();
    packet->ToStrings();

    if (packet->IsDefaultView()) {
      MainDefaultTreeview(packet);
    } else {
      MainPrimaryTreeview(packet);
    }
  }
  free(new);
}

void GUICallbackHandler_pref_up_min_combobox(GtkComboBox *ComboBox) {
  meta *D = packet->GetMetaClass();

  gchar *new = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(ComboBox));
  double new_f = strtod(new, NULL);

  if (new_f != D->updates_per_min_f) {
    SqliteAddAPIData("Updates_Per_Min", new, D);
    D->updates_per_min_f = new_f;
  }
  free(new);
}

void GUICallbackHandler_pref_hours_spinbutton(GtkEditable *spin_button) {
  meta *D = packet->GetMetaClass();
  const gchar *new = gtk_entry_get_text(GTK_ENTRY(spin_button));

  gboolean check =
      (strtod(new, NULL) <= 7) & CheckIfStringDoublePositiveNumber(new);
  check = check & (strlen(new) != 0);
  if (!check)
    return;

  double new_f = strtod(new, NULL);

  if (new_f != D->updates_hours_f) {
    SqliteAddAPIData("Updates_Hours", new, D);
    D->updates_hours_f = new_f;
  }
}

gboolean GUICallbackHandler_hide_window_on_delete(GtkWidget *window,
                                                  GdkEvent *event, void *data) {
  if (!(event))
    return gtk_widget_hide_on_delete(window);
  uintptr_t index_signal = (uintptr_t)data;

  switch (index_signal) {
  case ABOUT_TOGGLE_BTN:
    AboutShowHide();
    break;
  case SHORTCUT_TOGGLE_BTN:
    ShortcutShowHide();
    break;
  case EQUITY_TOGGLE_BTN:
    AddRemShowHide(packet);
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
  case RSI_TOGGLE_BTN:
    RSITreeViewClear();
    RSIShowHide(packet);
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
  if (!(event))
    return false;
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
  case GUI_RSI_WINDOW:
    W->rsi_width = (int)width;
    W->rsi_height = (int)height;

    W->rsi_x_pos = (int)x;
    W->rsi_y_pos = (int)y;
    break;
  }

  /* TRUE to stop other handlers from being invoked for the event.
     FALSE to propagate the event further.
  */
  return false;
}

gboolean GUICallbackHandler_select_comp(GtkEntryCompletion *completion,
                                        GtkTreeModel *model, GtkTreeIter *iter)
/* activated when an item is selected from the completion list */
{
  if (!(completion))
    return true;
  GtkWidget *EntryBoxRSI =
      GTK_WIDGET(gtk_builder_get_object(builder, "ViewRSISymbolEntryBox"));
  GtkWidget *EntryBoxAddRem = GTK_WIDGET(
      gtk_builder_get_object(builder, "AddRemoveSecuritySymbolEntryBox"));
  GtkWidget *EntryBox = NULL;
  if (gtk_widget_has_focus(EntryBoxRSI)) {
    EntryBox = EntryBoxRSI;
  } else {
    EntryBox = EntryBoxAddRem;
  }
  gchar *item;
  /* when a match is selected insert column zero instead of column 2 */
  gtk_tree_model_get(model, iter, 0, &item, -1);
  /* This function is already blocking the gtk main loop, it's ok
     to change widgets here without using a "gdk_threads_add_idle" wrapper
     function. */
  gtk_entry_set_text(GTK_ENTRY(EntryBox), item);
  gint pos = strlen(item);
  g_free(item);

  /* move the cursor to the end of the string */
  gtk_editable_set_position(GTK_EDITABLE(EntryBox), pos);
  return true;
}

gboolean GUICallbackHandler_cursor_comp(GtkEntryCompletion *completion,
                                        GtkTreeModel *model, GtkTreeIter *iter)
/* activated when an item is highlighted from the completion list */
{
  if (!(completion))
    return true;
  GtkWidget *EntryBoxRSI =
      GTK_WIDGET(gtk_builder_get_object(builder, "ViewRSISymbolEntryBox"));
  GtkWidget *EntryBoxAddRem = GTK_WIDGET(
      gtk_builder_get_object(builder, "AddRemoveSecuritySymbolEntryBox"));
  GtkWidget *EntryBox = NULL;
  if (gtk_widget_has_focus(EntryBoxRSI)) {
    EntryBox = EntryBoxRSI;
  } else {
    EntryBox = EntryBoxAddRem;
  }
  gchar *item;
  /* when a match is highlighted insert column zero instead of column 2 */
  gtk_tree_model_get(model, iter, 0, &item, -1);
  /* This function is already blocking the gtk main loop, it's ok
     to change widgets here without using a "gdk_threads_add_idle" wrapper
     function. */
  gtk_entry_set_text(GTK_ENTRY(EntryBox), item);
  gint pos = strlen(item);
  g_free(item);

  /* move the cursor to the end of the string */
  gtk_editable_set_position(GTK_EDITABLE(EntryBox), pos);
  return true;
}

static void view_popup_menu_onViewRSIData(GtkWidget *menuitem, void *userdata) {
  if (menuitem == NULL)
    return;
  char *symbol = (char *)userdata;

  GtkWidget *Window =
      GTK_WIDGET(gtk_builder_get_object(builder, "ViewRSIWindow"));
  GtkWidget *EntryBox =
      GTK_WIDGET(gtk_builder_get_object(builder, "ViewRSISymbolEntryBox"));
  GtkWidget *Button =
      GTK_WIDGET(gtk_builder_get_object(builder, "ViewRSIFetchDataBTN"));
  gboolean visible = gtk_widget_is_visible(Window);

  if (!visible)
    RSIShowHide(packet);

  gtk_entry_set_text(GTK_ENTRY(EntryBox), symbol);
  /* move the cursor to the end of the string */
  gtk_editable_set_position(GTK_EDITABLE(EntryBox), strlen(symbol));

  gtk_button_clicked(GTK_BUTTON(Button));
}

static void view_popup_menu_onViewSummary(GtkWidget *menuitem) {
  if (menuitem == NULL)
    return;

  GtkWidget *Button =
      GTK_WIDGET(gtk_builder_get_object(builder, "FetchDataBTN"));
  if (packet->IsFetchingData())
    gtk_button_clicked(GTK_BUTTON(Button));

  MainDefaultTreeview(packet);
}

static void view_popup_menu_onDeleteBullion(GtkWidget *menuitem,
                                            void *userdata) {
  if (menuitem == NULL)
    return;
  const gchar *metal_name = (gchar *)userdata;

  portfolio_packet *pkg = packet;
  metal *M = pkg->GetMetalClass();
  meta *D = pkg->GetMetaClass();

  /* Prevents Program From Crashing During A Data Fetch Operation */
  pthread_mutex_lock(&mutex_working[FETCH_DATA_MUTEX]);

  SqliteAddBullion(metal_name, "0", "0", M, D);

  pthread_mutex_unlock(&mutex_working[FETCH_DATA_MUTEX]);

  if (pkg->IsDefaultView()) {
    MainDefaultTreeview(packet);
  } else {
    pkg->Calculate();
    pkg->ToStrings();
    /* Set Gtk treeview. */
    MainPrimaryTreeview(packet);
  }
}

static void view_popup_menu_onDeleteAllBullion(GtkWidget *menuitem) {
  if (menuitem == NULL)
    return;
  portfolio_packet *pkg = packet;
  metal *M = pkg->GetMetalClass();
  meta *D = pkg->GetMetaClass();

  /* Prevents Program From Crashing During A Data Fetch Operation */
  pthread_mutex_lock(&mutex_working[FETCH_DATA_MUTEX]);

  SqliteAddBullion("gold", "0", "0", M, D);
  SqliteAddBullion("silver", "0", "0", M, D);
  SqliteAddBullion("platinum", "0", "0", M, D);
  SqliteAddBullion("palladium", "0", "0", M, D);

  pthread_mutex_unlock(&mutex_working[FETCH_DATA_MUTEX]);

  if (pkg->IsDefaultView()) {
    MainDefaultTreeview(packet);
  } else {
    pkg->Calculate();
    pkg->ToStrings();
    /* Set Gtk treeview. */
    MainPrimaryTreeview(packet);
  }
}

static void view_popup_menu_onDeleteEquity(GtkWidget *menuitem,
                                           void *userdata) {
  if (menuitem == NULL)
    return;
  const gchar *symbol = (gchar *)userdata;

  portfolio_packet *pkg = packet;
  equity_folder *F = pkg->GetEquityFolderClass();
  meta *D = pkg->GetMetaClass();

  /* Prevents Program From Crashing During A Data Fetch Operation */
  pthread_mutex_lock(&mutex_working[FETCH_DATA_MUTEX]);

  SqliteRemoveEquity(symbol, D);
  F->RemoveStock(symbol);

  pthread_mutex_unlock(&mutex_working[FETCH_DATA_MUTEX]);

  if (pkg->IsDefaultView()) {
    MainDefaultTreeview(packet);
  } else {
    pkg->Calculate();
    pkg->ToStrings();
    /* Set Gtk treeview. */
    MainPrimaryTreeview(packet);
  }
}

static void view_popup_menu_onDeleteAllEquity(GtkWidget *menuitem) {
  if (menuitem == NULL)
    return;
  portfolio_packet *pkg = packet;
  equity_folder *F = pkg->GetEquityFolderClass();
  meta *D = pkg->GetMetaClass();

  /* Prevents Program From Crashing During A Data Fetch Operation */
  pthread_mutex_lock(&mutex_working[FETCH_DATA_MUTEX]);

  SqliteRemoveAllEquity(D);

  /* Reset Equity Folder */
  F->Reset();

  pthread_mutex_unlock(&mutex_working[FETCH_DATA_MUTEX]);

  if (pkg->IsDefaultView()) {
    MainDefaultTreeview(packet);
  } else {
    pkg->Calculate();
    pkg->ToStrings();
    /* Set Gtk treeview. */
    MainPrimaryTreeview(packet);
  }
}

static void view_popup_menu_onAddRow(GtkWidget *menuitem, void *userdata) {
  if (menuitem == NULL)
    return;
  char *type = (char *)userdata;

  if (strcmp(type, "equity") == 0) {
    AddRemShowHide(packet);
  } else if (strcmp(type, "equity_total") == 0) {
    AddRemShowHide(packet);
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

        /* Some of the menu signal connections need the type and symbol strings.
           We store the data in the meta class and free the members on
           subsequent clicks, which keeps the strings available to the signal
           connections. */
        /* The meta class is available globally, in this file, although I think
         * this method is more obvious.  The data is freed when the meta class
         * is destructed. */
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
          menuitem = gtk_menu_item_new_with_label("View RSI Data");
          g_signal_connect(menuitem, "activate",
                           G_CALLBACK(view_popup_menu_onViewRSIData),
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