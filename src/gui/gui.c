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

#include "../include/class_types.h" /* portfolio_packet, window_data */
#include "../include/gui.h"
#include "../include/gui_types.h"
#include "../include/workfuncs.h"

static GtkBuilder *builder;

/* Three Convenience Functions */
GtkWidget *GetWidget(const gchar *widget_name_ch) {
  return GTK_WIDGET(gtk_builder_get_object(builder, widget_name_ch));
}

GObject *GetGObject(const gchar *gobject_name_ch) {
  return gtk_builder_get_object(builder, gobject_name_ch);
}

const gchar *GetEntryText(const char *name_ch) {
  GtkWidget *EntryBox = GetWidget(name_ch);
  return gtk_entry_get_text(GTK_ENTRY(EntryBox));
}

static struct {
  const char *name;
  const char *shortcut;
} commands[] = {{"Application Window", ""},
                {"      File", "Ctrl - F"},
                {"      RSI", "Ctrl - R"},
                {"      Quit", "Ctrl - Q"},
                {"", ""},
                {"      Edit", "Ctrl - E"},
                {"      Securities", "Ctrl - S"},
                {"      Bullion", "Ctrl - B"},
                {"      Cash", "Ctrl - C"},
                {"      API", "Ctrl - I"},
                {"      Preferences", "Ctrl - P"},
                {"", ""},
                {"      Help", "Ctrl - H"},
                {"      Hotkeys", "Ctrl - K"},
                {"      About", "Ctrl - A"},
                {"", ""},
                {"      Get Data", "Ctrl - D"},
                {"", ""},
                {"RSI Window", ""},
                {"      Get Data", "Ctrl - D"},
                {"      Close", "Ctrl - C"},
                {"", ""},
                {"Preferences Window", ""},
                {"      Get Symbols", "Ctrl - S"},
                {"      Close", "Ctrl - C"},
                {"", ""},
                {"Other Windows", ""},
                {"      File", "Ctrl - F"},
                {"      OK", "Ctrl - O"},
                {"      Close", "Ctrl - C"}};

static void hotkeys_set_treeview() {
  GtkWidget *TreeView = GetWidget("ShortcutWindowTreeView");

  GtkCellRenderer *renderer;
  GtkTreeViewColumn *column;
  char *cmd_name_markup = NULL, *cmd_shortcut_markup = NULL;

  /* In order to display a model/store we need to set the TreeView Columns. */
  renderer = gtk_cell_renderer_text_new();
  column = gtk_tree_view_column_new_with_attributes("column_one", renderer,
                                                    "markup", 0, NULL);
  gtk_tree_view_column_set_resizable(column, true);
  gtk_tree_view_column_set_visible(column, true);
  gtk_tree_view_column_set_min_width(column, 0);
  gtk_tree_view_append_column(GTK_TREE_VIEW(TreeView), column);

  renderer = gtk_cell_renderer_text_new();
  column = gtk_tree_view_column_new_with_attributes("column_two", renderer,
                                                    "markup", 1, NULL);
  gtk_tree_view_column_set_resizable(column, true);
  gtk_tree_view_column_set_visible(column, true);
  gtk_tree_view_column_set_min_width(column, 0);
  gtk_tree_view_append_column(GTK_TREE_VIEW(TreeView), column);

  /* Here we set the rows for the 2 column store */
  GtkListStore *store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_STRING);
  GtkTreeIter iter;

  for (int i = 0; i < G_N_ELEMENTS(commands); i++) {
    /* We mark them up slightly first. */
    StringToStrPangoColor(&cmd_name_markup, commands[i].name, BLACK);
    StringToStrPangoColor(&cmd_shortcut_markup, commands[i].shortcut, BLACK);

    /* We add them to a new row. */
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, cmd_name_markup, 1, cmd_shortcut_markup,
                       -1);
  }

  free(cmd_name_markup);
  free(cmd_shortcut_markup);

  /* Add the store of data to the TreeView. */
  gtk_tree_view_set_model(GTK_TREE_VIEW(TreeView), GTK_TREE_MODEL(store));
  g_object_unref(store);

  /* Set the TreeView header as invisible. */
  gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(TreeView), false);

  /* Make the TreeView unselectable. */
  GtkTreeSelection *select =
      gtk_tree_view_get_selection(GTK_TREE_VIEW(TreeView));
  gtk_tree_selection_set_mode(select, GTK_SELECTION_NONE);

  /* Remove TreeView Grid Lines. */
  gtk_tree_view_set_grid_lines(GTK_TREE_VIEW(TreeView),
                               GTK_TREE_VIEW_GRID_LINES_NONE);
}

static void about_set_label() {
  /* Set the About window labels. */
  const gchar *text = "<a "
                      "href=\"https://github.com/BostonBSD/"
                      "finnhub.io-stock-ticker\">Website</a>";
  GtkWidget *label = GetWidget("AboutWebsiteLabel");
  gtk_label_set_markup(GTK_LABEL(label), text);

  text =
      "<a href=\"https://www.flaticon.com/free-icons/trends\">Trends icon</a> "
      "designed by Freepik from <a "
      "href=\"https://media.flaticon.com/license/license.pdf\">Flaticon</a>";
  label = GetWidget("AboutTrendsIconLabel");
  gtk_label_set_markup(GTK_LABEL(label), text);
}

static void clock_display(void *data)
/* Set the initial display of the clocks.
   We don't want the revealer animation on startup. */
{
  portfolio_packet *pkg = (portfolio_packet *)data;
  meta *D = pkg->GetMetaClass();

  GtkWidget *revealer = GetWidget("MainClockRevealer");

  if (pkg->IsClockDisplayed()) {
    /* Start the clock threads. */
    /* gdk_threads_add_idle (in these threads) creates a pending event for the
       gtk_main loop. When the gtk_main loop starts the event will be processed.
    */
    pthread_create(&D->thread_id_clock, NULL, GUIThreadHandler_main_clock,
                   NULL);
    pthread_create(&D->thread_id_closing_time, NULL,
                   GUIThreadHandler_time_to_close, pkg);
    pthread_detach(D->thread_id_clock);
    pthread_detach(D->thread_id_closing_time);
  }

  /* Revealer animation set to 0 milliseconds */
  gtk_revealer_set_transition_duration(GTK_REVEALER(revealer), 0);
  /* Show/Hide the clocks */
  gtk_revealer_set_reveal_child(GTK_REVEALER(revealer),
                                pkg->IsClockDisplayed());
  /* Revealer animation set to 300 milliseconds */
  gtk_revealer_set_transition_duration(GTK_REVEALER(revealer), 300);
}

static void completion_set(void *data) {
  pthread_t thread_id;

  /* Set up the EntryBox Completion Widgets
     This will populate the symbol to name mapping,
     from an sqlite Db when the application loads.

     The lists of symbol to name mappings need to be
     downloaded by the user [in the preferences window]
     if the db isn't already populated. */
  pthread_create(&thread_id, NULL, GUIThreadHandler_completion_set, data);
  pthread_detach(thread_id);
}

static void main_window_sig_connect(void *data) {
  portfolio_packet *pkg = (portfolio_packet *)data;
  window_data *W = pkg->GetWindowData();
  GObject *window, *object;

  /* Connect signal handlers to the constructed widgets/objects [widgets are a
   * subclass of objects, they can be casted back and forth]. */
  /* The last argument, the widget index signal, is an enum/int casted to a
     void*, small to larger datatype, in the GUICallbackHandler we cast this
     back to an enum/int, because it started as an enum/int we should not worry
     about data loss through casting truncation. */

  window = GetGObject("MainWindow");
  g_signal_connect(window, "destroy", G_CALLBACK(GUICallbackHandler),
                   (void *)MAIN_EXIT);
  gtk_window_resize(GTK_WINDOW(window), W->main_width, W->main_height);
  gtk_window_move(GTK_WINDOW(window), W->main_x_pos, W->main_y_pos);

  g_signal_connect(window, "configure-event",
                   G_CALLBACK(GUICallbackHandler_window_data),
                   (void *)GUI_MAIN_WINDOW);

  object = GetGObject("FetchDataBTN");
  g_signal_connect(object, "clicked", G_CALLBACK(GUICallbackHandler),
                   (void *)MAIN_FETCH_BTN);
  gtk_widget_grab_focus(GTK_WIDGET(object));

  object = GetGObject("MainFileMenuRSI");
  g_signal_connect(object, "activate", G_CALLBACK(GUICallbackHandler),
                   (void *)RSI_TOGGLE_BTN);

  object = GetGObject("MainFileMenuQuit");
  g_signal_connect(object, "activate", G_CALLBACK(GUICallbackHandler),
                   (void *)MAIN_EXIT);

  object = GetGObject("MainEditMenuAddRemoveSecurity");
  g_signal_connect(object, "activate", G_CALLBACK(GUICallbackHandler),
                   (void *)EQUITY_TOGGLE_BTN);

  object = GetGObject("MainEditMenuBullion");
  g_signal_connect(object, "activate", G_CALLBACK(GUICallbackHandler),
                   (void *)BUL_TOGGLE_BTN);

  object = GetGObject("MainEditMenuCash");
  g_signal_connect(object, "activate", G_CALLBACK(GUICallbackHandler),
                   (void *)CASH_TOGGLE_BTN);

  object = GetGObject("MainEditMenuAPI");
  g_signal_connect(object, "activate", G_CALLBACK(GUICallbackHandler),
                   (void *)API_TOGGLE_BTN);

  object = GetGObject("MainEditMenuPreferences");
  g_signal_connect(object, "activate", G_CALLBACK(GUICallbackHandler),
                   (void *)PREF_TOGGLE_BTN);

  object = GetGObject("MainHelpMenuKeyboardShortcuts");
  g_signal_connect(object, "activate", G_CALLBACK(GUICallbackHandler),
                   (void *)HOTKEYS_TOGGLE_BTN);

  object = GetGObject("MainHelpMenuAbout");
  g_signal_connect(object, "activate", G_CALLBACK(GUICallbackHandler),
                   (void *)ABOUT_TOGGLE_BTN);

  /* This is the main window's TreeView ( There was only one treeview when I
   * started ). */
  object = GetGObject("TreeView");
  g_signal_connect(object, "button-press-event",
                   G_CALLBACK(view_onButtonPressed), NULL);
}

static void securities_window_sig_connect() {
  GObject *window, *object;

  object = GetGObject("SecuritiesMenuClose");
  g_signal_connect(object, "activate", G_CALLBACK(GUICallbackHandler),
                   (void *)EQUITY_TOGGLE_BTN);

  window = GetGObject("AddRemoveSecurity");
  g_signal_connect(window, "delete_event",
                   G_CALLBACK(GUICallbackHandler_hide_window_on_delete),
                   (void *)EQUITY_TOGGLE_BTN);

  object = GetGObject("AddRemoveSecurityOkBTN");
  g_signal_connect(object, "clicked", G_CALLBACK(GUICallbackHandler),
                   (void *)EQUITY_OK_BTN);

  object = GetGObject("AddRemoveSecurityStack");
  g_signal_connect(object, "notify::visible-child",
                   G_CALLBACK(GUICallbackHandler_add_rem_stack), NULL);

  object = GetGObject("AddRemoveSecurityComboBox");
  g_signal_connect(object, "changed", G_CALLBACK(GUICallbackHandler),
                   (void *)EQUITY_COMBO_BOX);

  object = GetGObject("AddRemoveSecuritySymbolEntryBox");
  g_signal_connect(object, "changed", G_CALLBACK(GUICallbackHandler),
                   (void *)EQUITY_CURSOR_MOVE);

  object = GetGObject("AddRemoveSecuritySharesEntryBox");
  g_signal_connect(object, "changed", G_CALLBACK(GUICallbackHandler),
                   (void *)EQUITY_CURSOR_MOVE);
}

static void bullion_window_sig_connect() {
  GObject *window, *object;

  object = GetGObject("BullionMenuClose");
  g_signal_connect(object, "activate", G_CALLBACK(GUICallbackHandler),
                   (void *)BUL_TOGGLE_BTN);

  window = GetGObject("AddRemoveBullionWindow");
  g_signal_connect(window, "delete_event",
                   G_CALLBACK(GUICallbackHandler_hide_window_on_delete),
                   (void *)BUL_TOGGLE_BTN);

  object = GetGObject("AddRemoveBullionOKBTN");
  g_signal_connect(object, "clicked", G_CALLBACK(GUICallbackHandler),
                   (void *)BUL_OK_BTN);
  gtk_widget_set_sensitive(GTK_WIDGET(object), false);

  object = GetGObject("AddRemoveBullionGoldOuncesEntryBox");
  g_signal_connect(object, "changed", G_CALLBACK(GUICallbackHandler),
                   (void *)BUL_CURSOR_MOVE);

  object = GetGObject("AddRemoveBullionGoldPremiumEntryBox");
  g_signal_connect(object, "changed", G_CALLBACK(GUICallbackHandler),
                   (void *)BUL_CURSOR_MOVE);

  object = GetGObject("AddRemoveBullionSilverOuncesEntryBox");
  g_signal_connect(object, "changed", G_CALLBACK(GUICallbackHandler),
                   (void *)BUL_CURSOR_MOVE);

  object = GetGObject("AddRemoveBullionSilverPremiumEntryBox");
  g_signal_connect(object, "changed", G_CALLBACK(GUICallbackHandler),
                   (void *)BUL_CURSOR_MOVE);

  object = GetGObject("AddRemoveBullionPlatinumOuncesEntryBox");
  g_signal_connect(object, "changed", G_CALLBACK(GUICallbackHandler),
                   (void *)BUL_CURSOR_MOVE);

  object = GetGObject("AddRemoveBullionPlatinumPremiumEntryBox");
  g_signal_connect(object, "changed", G_CALLBACK(GUICallbackHandler),
                   (void *)BUL_CURSOR_MOVE);

  object = GetGObject("AddRemoveBullionPalladiumOuncesEntryBox");
  g_signal_connect(object, "changed", G_CALLBACK(GUICallbackHandler),
                   (void *)BUL_CURSOR_MOVE);

  object = GetGObject("AddRemoveBullionPalladiumPremiumEntryBox");
  g_signal_connect(object, "changed", G_CALLBACK(GUICallbackHandler),
                   (void *)BUL_CURSOR_MOVE);

  object = GetGObject("AddRemoveBullionComboBox");
  g_signal_connect(object, "changed", G_CALLBACK(GUICallbackHandler),
                   (void *)BUL_COMBO_BOX);
}

static void preferences_window_sig_connect(void *data) {
  portfolio_packet *pkg = (portfolio_packet *)data;
  meta *D = pkg->GetMetaClass();
  GObject *window, *object;

  object = GetGObject("PreferencesMenuClose");
  g_signal_connect(object, "activate", G_CALLBACK(GUICallbackHandler),
                   (void *)PREF_TOGGLE_BTN);

  window = GetGObject("PreferencesWindow");
  g_signal_connect(window, "delete_event",
                   G_CALLBACK(GUICallbackHandler_hide_window_on_delete),
                   (void *)PREF_TOGGLE_BTN);

  object = GetGObject("PrefStockSymbolUpdateBTN");
  g_signal_connect(object, "clicked", G_CALLBACK(GUICallbackHandler),
                   (void *)PREF_SYMBOL_UPDATE_BTN);

  object = GetGObject("PrefHoursSpinBox");
  g_signal_connect(object, "changed",
                   G_CALLBACK(GUICallbackHandler_pref_hours_spinbutton), NULL);

  object = GetGObject("PrefShowClocksSwitch");
  /* Make sure the switch is set before connecting a signal to it. */
  gtk_switch_set_active(GTK_SWITCH(object), D->clocks_displayed_bool);
  g_signal_connect(object, "state-set",
                   G_CALLBACK(GUICallbackHandler_pref_clock_switch), NULL);

  object = GetGObject("PrefShowIndicesSwitch");
  /* Make sure the switch is set before connecting a signal to it. */
  gtk_switch_set_active(GTK_SWITCH(object), D->index_bar_revealed_bool);
  g_signal_connect(object, "state-set",
                   G_CALLBACK(GUICallbackHandler_pref_indices_switch), NULL);

  object = GetGObject("PrefUpPerMinComboBox");
  gtk_combo_box_set_active(GTK_COMBO_BOX(object), (int)D->updates_per_min_f);
  g_signal_connect(object, "changed",
                   G_CALLBACK(GUICallbackHandler_pref_up_min_combobox), NULL);

  object = GetGObject("PrefDecPlacesComboBox");
  /* The combobox index 0, is 2 dec places, index 1 is 3 dec places. So
   * minus 2. */
  gtk_combo_box_set_active(GTK_COMBO_BOX(object),
                           (int)D->decimal_places_shrt - 2);
  g_signal_connect(object, "changed",
                   G_CALLBACK(GUICallbackHandler_pref_dec_places_combobox),
                   NULL);
}

static void api_window_sig_connect() {
  GObject *window, *object;

  object = GetGObject("APIMenuClose");
  g_signal_connect(object, "activate", G_CALLBACK(GUICallbackHandler),
                   (void *)API_TOGGLE_BTN);

  window = GetGObject("ChangeApiInfoWindow");
  g_signal_connect(window, "delete_event",
                   G_CALLBACK(GUICallbackHandler_hide_window_on_delete),
                   (void *)API_TOGGLE_BTN);

  object = GetGObject("ChangeApiInfoOKBTN");
  g_signal_connect(object, "clicked", G_CALLBACK(GUICallbackHandler),
                   (void *)API_OK_BTN);
  gtk_widget_set_sensitive(GTK_WIDGET(object), false);

  object = GetGObject("ChangeApiInfoEquityUrlEntryBox");
  g_signal_connect(object, "changed", G_CALLBACK(GUICallbackHandler),
                   (void *)API_CURSOR_MOVE);

  object = GetGObject("ChangeApiInfoUrlKeyEntryBox");
  g_signal_connect(object, "changed", G_CALLBACK(GUICallbackHandler),
                   (void *)API_CURSOR_MOVE);

  object = GetGObject("ChangeApiInfoNasdaqSymbolsUrlEntryBox");
  g_signal_connect(object, "changed", G_CALLBACK(GUICallbackHandler),
                   (void *)API_CURSOR_MOVE);

  object = GetGObject("ChangeApiInfoNYSESymbolsUrlEntryBox");
  g_signal_connect(object, "changed", G_CALLBACK(GUICallbackHandler),
                   (void *)API_CURSOR_MOVE);
}

static void rsi_window_sig_connect(void *data) {
  portfolio_packet *pkg = (portfolio_packet *)data;
  window_data *W = pkg->GetWindowData();
  GObject *window, *object;

  window = GetGObject("ViewRSIWindow");
  g_signal_connect(window, "delete_event",
                   G_CALLBACK(GUICallbackHandler_hide_window_on_delete),
                   (void *)RSI_TOGGLE_BTN);
  gtk_window_resize(GTK_WINDOW(window), W->rsi_width, W->rsi_height);
  gtk_window_move(GTK_WINDOW(window), W->rsi_x_pos, W->rsi_y_pos);

  g_signal_connect(window, "configure-event",
                   G_CALLBACK(GUICallbackHandler_window_data),
                   (void *)GUI_RSI_WINDOW);

  object = GetGObject("RSIMenuCloseBTN");
  g_signal_connect(object, "activate", G_CALLBACK(GUICallbackHandler),
                   (void *)RSI_TOGGLE_BTN);

  object = GetGObject("ViewRSIFetchDataBTN");
  g_signal_connect(object, "clicked", G_CALLBACK(GUICallbackHandler),
                   (void *)RSI_FETCH_BTN);

  object = GetGObject("ViewRSISymbolEntryBox");
  g_signal_connect(object, "changed", G_CALLBACK(GUICallbackHandler),
                   (void *)RSI_CURSOR_MOVE);
}

static void other_window_sig_connect() {
  GObject *window, *object;

  object = GetGObject("AboutMenuClose");
  g_signal_connect(object, "activate", G_CALLBACK(GUICallbackHandler),
                   (void *)ABOUT_TOGGLE_BTN);

  window = GetGObject("AboutWindow");
  g_signal_connect(window, "delete_event",
                   G_CALLBACK(GUICallbackHandler_hide_window_on_delete),
                   (void *)ABOUT_TOGGLE_BTN);

  object = GetGObject("ShortcutsMenuClose");
  g_signal_connect(object, "activate", G_CALLBACK(GUICallbackHandler),
                   (void *)HOTKEYS_TOGGLE_BTN);

  window = GetGObject("ShortcutWindow");
  g_signal_connect(window, "delete_event",
                   G_CALLBACK(GUICallbackHandler_hide_window_on_delete),
                   (void *)HOTKEYS_TOGGLE_BTN);

  object = GetGObject("CashMenuClose");
  g_signal_connect(object, "activate", G_CALLBACK(GUICallbackHandler),
                   (void *)CASH_TOGGLE_BTN);

  window = GetGObject("AddRemoveCashWindow");
  g_signal_connect(window, "delete_event",
                   G_CALLBACK(GUICallbackHandler_hide_window_on_delete),
                   (void *)CASH_TOGGLE_BTN);

  object = GetGObject("AddRemoveCashOKBTN");
  g_signal_connect(object, "clicked", G_CALLBACK(GUICallbackHandler),
                   (void *)CASH_OK_BTN);
  gtk_widget_set_sensitive(GTK_WIDGET(object), false);

  object = GetGObject("AddRemoveCashValueSpinBTN");
  g_signal_connect(object, "changed", G_CALLBACK(GUICallbackHandler),
                   (void *)CASH_CURSOR_MOVE);
}

static void gui_signal_connect(void *data)
/* Connect widget signals to signal handlers. */
{
  main_window_sig_connect(data);
  securities_window_sig_connect();
  bullion_window_sig_connect();
  preferences_window_sig_connect(data);
  api_window_sig_connect();
  rsi_window_sig_connect(data);
  other_window_sig_connect();
}

/* Engineering Note */

/* After the gtk_main() loop starts nearly every widget signal is connected
   to the GUICallbackHandler function in gui_callbacks.c.

   Some signals require separate threads to perform computational tasks, these
   are dispatched from GUICallbackHandler functions [in gui_threads.c].  The
   clock threads are dispatched by separate functions [clock_display(),
   GUICallbackHandler_pref_clock_switch()].

   There are ancillary widget signals to perform other tasks depending upon
   widget states, all of these callback functions are also in gui_callbacks.c.

   In order to make window delete [closing a window by pressing the windowbar X]
   operate the same as a close button, we have our own "delete-event" signal
   handler called GUICallbackHandler_hide_window_on_delete().

   Widget index signals are defined in gui_types.h [these are application
   defined and not the same as GtkWidget signals].  These index signals are
   convenient for adding new widgets with a generic callback function.
*/

void GuiStart(void *data)
/* Setup GUI widgets and display the GUI. */
{
  GError *error = NULL;

  /* Compiling the resources.c file ensures the resources are in the
     GResources namespace (basically ensures the UI description file
     and the icon file are in the namespace, our only resources).
     The icon file is referenced in the UI description file. */

  /* Construct a GtkBuilder instance and load our UI description. */
  builder = gtk_builder_new();
  if (gtk_builder_add_from_resource(builder, "/financials.glade", &error) ==
      0) {
    g_printerr("Error loading user interface: %s\n", error->message);
    g_clear_error(&error);
    exit(EXIT_FAILURE);
  }

  /* Set whether the clocks are displayed or not.
     Start clock threads if the clocks are displayed. */
  clock_display(data);

  /* Add the list of stock symbols from sqlite to a struct.
     Set two entrybox completion widgets.*/
  completion_set(data);

  /* Add the keyboard shortcuts to the Hotkeys window. */
  hotkeys_set_treeview();

  /* Add hyperlink markup to the About window labels. */
  about_set_label();

  /* Connect callback functions to corresponding GUI signals. */
  gui_signal_connect(data);

  /* Set the default treeview.
     This treeview is already set in the completion_set thread [created in
     completion_set()], however, there may be a db read delay before it is
     displayed. So we set it here showing any available data. */
  MainDefaultTreeview(data);

  /* Start the gtk_main loop. */
  gtk_main();
}