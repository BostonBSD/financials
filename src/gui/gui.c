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

#include <glib-object.h>
#include <gtk/gtk.h>

#include "../include/class_types.h" /* portfolio_packet, window_data */
#include "../include/gui.h"
#include "../include/gui_types.h"
#include "../include/workfuncs.h"

GtkBuilder *builder;

typedef struct {
  const char *name;
  const char *shortcut;
} Command;

static Command commands[] = {{"Application Window", ""},
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

static void shortcuts_set_treeview() {
  GtkWidget *TreeView =
      GTK_WIDGET(gtk_builder_get_object(builder, "ShortcutWindowTreeView"));

  GtkCellRenderer *renderer;
  GtkTreeViewColumn *column;
  char *cmd_name_markup = NULL, *cmd_shortcut_markup = NULL;

  /* In order to display a model/store we need to set the TreeView Columns. */
  renderer = gtk_cell_renderer_text_new();
  column = gtk_tree_view_column_new_with_attributes("Column Header 1", renderer,
                                                    "markup", 0, NULL);
  gtk_tree_view_column_set_resizable(column, true);
  gtk_tree_view_column_set_visible(column, true);
  gtk_tree_view_column_set_min_width(column, 0);
  gtk_tree_view_append_column(GTK_TREE_VIEW(TreeView), column);

  renderer = gtk_cell_renderer_text_new();
  column = gtk_tree_view_column_new_with_attributes("Column Header 2", renderer,
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
    StrToStrPangoColor(&cmd_name_markup, commands[i].name, BLACK);
    StrToStrPangoColor(&cmd_shortcut_markup, commands[i].shortcut, BLACK);

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
  GtkWidget *label =
      GTK_WIDGET(gtk_builder_get_object(builder, "AboutWebsiteLabel"));
  gtk_label_set_markup(GTK_LABEL(label), text);

  text =
      "<a href=\"https://www.flaticon.com/free-icons/trends\">Trends icon</a> "
      "designed by Freepik from <a "
      "href=\"https://media.flaticon.com/license/license.pdf\">Flaticon</a>";
  label = GTK_WIDGET(gtk_builder_get_object(builder, "AboutTrendsIconLabel"));
  gtk_label_set_markup(GTK_LABEL(label), text);
}

static void clock_display(void *data)
/* Set the initial display of the clocks.
   We don't want the revealer animation on startup
   if the clocks aren't displayed. */
{
  portfolio_packet *pkg = (portfolio_packet *)data;

  GtkWidget *revealer =
      GTK_WIDGET(gtk_builder_get_object(builder, "MainClockRevealer"));

  if (pkg->IsClockDisplayed() == false) {
    /* Revealer animation set to 0 milliseconds */
    gtk_revealer_set_transition_duration(GTK_REVEALER(revealer), 0);
    /* Hide the clocks */
    gtk_revealer_set_reveal_child(GTK_REVEALER(revealer), false);
    /* Revealer animation set to 300 milliseconds */
    gtk_revealer_set_transition_duration(GTK_REVEALER(revealer), 300);

    /* Make sure there's data in the clock label. */
    MainDisplayTime();
  }
}

static void gui_signal_connect(void *data)
/* Connect GUI index signals to the signal handlers. */
{
  portfolio_packet *pkg = (portfolio_packet *)data;
  window_data *W = pkg->GetWindowData();
  GObject *window, *widget;

  /* Connect signal handlers to the constructed widgets. */
  /* The last argument, the widget index signal, is an enum/int casted to a
     void*, small to larger datatype, in the GUIThreadHandler we cast this back
     to an enum/int, because it started as an enum/int we should not worry about
     data loss through casting truncation. */

  window = gtk_builder_get_object(builder, "MainWindow");
  g_signal_connect(window, "destroy", G_CALLBACK(GUICallbackHandler),
                   (void *)MAIN_EXIT);
  g_signal_connect(window, "configure-event",
                   G_CALLBACK(GUICallbackHandler_window_data),
                   (void *)GUI_MAIN_WINDOW);
  gtk_window_resize(GTK_WINDOW(window), W->main_width, W->main_height);
  gtk_window_move(GTK_WINDOW(window), W->main_x_pos, W->main_y_pos);

  widget = gtk_builder_get_object(builder, "FetchDataBTN");
  g_signal_connect(widget, "clicked", G_CALLBACK(GUICallbackHandler),
                   (void *)MAIN_FETCH_BTN);
  gtk_widget_grab_focus(GTK_WIDGET(widget));

  widget = gtk_builder_get_object(builder, "MainFileMenuQuit");
  g_signal_connect(widget, "activate", G_CALLBACK(GUICallbackHandler),
                   (void *)MAIN_EXIT);

  widget = gtk_builder_get_object(builder, "MainEditMenuAddRemoveSecurity");
  g_signal_connect(widget, "activate", G_CALLBACK(GUICallbackHandler),
                   (void *)EQUITY_TOGGLE_BTN);

  widget = gtk_builder_get_object(builder, "SecuritiesMenuClose");
  g_signal_connect(widget, "activate", G_CALLBACK(GUICallbackHandler),
                   (void *)EQUITY_TOGGLE_BTN);

  window = gtk_builder_get_object(builder, "AddRemoveSecurity");
  g_signal_connect(window, "delete_event",
                   G_CALLBACK(GUICallbackHandler_hide_window_on_delete),
                   (void *)EQUITY_TOGGLE_BTN);

  widget = gtk_builder_get_object(builder, "AddRemoveSecurityOkBTN");
  g_signal_connect(widget, "clicked", G_CALLBACK(GUICallbackHandler),
                   (void *)EQUITY_OK_BTN);

  widget = gtk_builder_get_object(builder, "AddRemoveSecurityStack");
  g_signal_connect(widget, "notify::visible-child",
                   G_CALLBACK(GUICallbackHandler_add_rem_stack), NULL);

  widget = gtk_builder_get_object(builder, "AddRemoveSecurityComboBox");
  g_signal_connect(widget, "changed", G_CALLBACK(GUICallbackHandler),
                   (void *)EQUITY_COMBO_BOX);

  widget = gtk_builder_get_object(builder, "AddRemoveSecuritySymbolEntryBox");
  g_signal_connect(widget, "changed", G_CALLBACK(GUICallbackHandler),
                   (void *)EQUITY_CURSOR_MOVE);

  widget = gtk_builder_get_object(builder, "AddRemoveSecuritySharesEntryBox");
  g_signal_connect(widget, "changed", G_CALLBACK(GUICallbackHandler),
                   (void *)EQUITY_CURSOR_MOVE);

  widget = gtk_builder_get_object(builder, "MainHelpMenuAbout");
  g_signal_connect(widget, "activate", G_CALLBACK(GUICallbackHandler),
                   (void *)ABOUT_TOGGLE_BTN);

  widget = gtk_builder_get_object(builder, "AboutMenuClose");
  g_signal_connect(widget, "activate", G_CALLBACK(GUICallbackHandler),
                   (void *)ABOUT_TOGGLE_BTN);

  window = gtk_builder_get_object(builder, "AboutWindow");
  g_signal_connect(window, "delete_event",
                   G_CALLBACK(GUICallbackHandler_hide_window_on_delete),
                   (void *)ABOUT_TOGGLE_BTN);

  widget = gtk_builder_get_object(builder, "MainHelpMenuKeyboardShortcuts");
  g_signal_connect(widget, "activate", G_CALLBACK(GUICallbackHandler),
                   (void *)SHORTCUT_TOGGLE_BTN);

  widget = gtk_builder_get_object(builder, "ShortcutsMenuClose");
  g_signal_connect(widget, "activate", G_CALLBACK(GUICallbackHandler),
                   (void *)SHORTCUT_TOGGLE_BTN);

  window = gtk_builder_get_object(builder, "ShortcutWindow");
  g_signal_connect(window, "delete_event",
                   G_CALLBACK(GUICallbackHandler_hide_window_on_delete),
                   (void *)SHORTCUT_TOGGLE_BTN);

  widget = gtk_builder_get_object(builder, "MainEditMenuBullion");
  g_signal_connect(widget, "activate", G_CALLBACK(GUICallbackHandler),
                   (void *)BUL_TOGGLE_BTN);

  widget = gtk_builder_get_object(builder, "BullionMenuClose");
  g_signal_connect(widget, "activate", G_CALLBACK(GUICallbackHandler),
                   (void *)BUL_TOGGLE_BTN);

  window = gtk_builder_get_object(builder, "AddRemoveBullionWindow");
  g_signal_connect(window, "delete_event",
                   G_CALLBACK(GUICallbackHandler_hide_window_on_delete),
                   (void *)BUL_TOGGLE_BTN);

  widget = gtk_builder_get_object(builder, "AddRemoveBullionOKBTN");
  g_signal_connect(widget, "clicked", G_CALLBACK(GUICallbackHandler),
                   (void *)BUL_OK_BTN);
  gtk_widget_set_sensitive(GTK_WIDGET(widget), false);

  widget =
      gtk_builder_get_object(builder, "AddRemoveBullionGoldOuncesEntryBox");
  g_signal_connect(widget, "changed", G_CALLBACK(GUICallbackHandler),
                   (void *)BUL_CURSOR_MOVE);

  widget =
      gtk_builder_get_object(builder, "AddRemoveBullionGoldPremiumEntryBox");
  g_signal_connect(widget, "changed", G_CALLBACK(GUICallbackHandler),
                   (void *)BUL_CURSOR_MOVE);

  widget =
      gtk_builder_get_object(builder, "AddRemoveBullionSilverOuncesEntryBox");
  g_signal_connect(widget, "changed", G_CALLBACK(GUICallbackHandler),
                   (void *)BUL_CURSOR_MOVE);

  widget =
      gtk_builder_get_object(builder, "AddRemoveBullionSilverPremiumEntryBox");
  g_signal_connect(widget, "changed", G_CALLBACK(GUICallbackHandler),
                   (void *)BUL_CURSOR_MOVE);

  widget =
      gtk_builder_get_object(builder, "AddRemoveBullionPlatinumOuncesEntryBox");
  g_signal_connect(widget, "changed", G_CALLBACK(GUICallbackHandler),
                   (void *)BUL_CURSOR_MOVE);

  widget = gtk_builder_get_object(builder,
                                  "AddRemoveBullionPlatinumPremiumEntryBox");
  g_signal_connect(widget, "changed", G_CALLBACK(GUICallbackHandler),
                   (void *)BUL_CURSOR_MOVE);

  widget = gtk_builder_get_object(builder,
                                  "AddRemoveBullionPalladiumOuncesEntryBox");
  g_signal_connect(widget, "changed", G_CALLBACK(GUICallbackHandler),
                   (void *)BUL_CURSOR_MOVE);

  widget = gtk_builder_get_object(builder,
                                  "AddRemoveBullionPalladiumPremiumEntryBox");
  g_signal_connect(widget, "changed", G_CALLBACK(GUICallbackHandler),
                   (void *)BUL_CURSOR_MOVE);

  widget = gtk_builder_get_object(builder, "AddRemoveBullionComboBox");
  g_signal_connect(widget, "changed", G_CALLBACK(GUICallbackHandler),
                   (void *)BUL_COMBO_BOX);

  widget = gtk_builder_get_object(builder, "MainEditMenuCash");
  g_signal_connect(widget, "activate", G_CALLBACK(GUICallbackHandler),
                   (void *)CASH_TOGGLE_BTN);

  widget = gtk_builder_get_object(builder, "CashMenuClose");
  g_signal_connect(widget, "activate", G_CALLBACK(GUICallbackHandler),
                   (void *)CASH_TOGGLE_BTN);

  window = gtk_builder_get_object(builder, "AddRemoveCashWindow");
  g_signal_connect(window, "delete_event",
                   G_CALLBACK(GUICallbackHandler_hide_window_on_delete),
                   (void *)CASH_TOGGLE_BTN);

  widget = gtk_builder_get_object(builder, "AddRemoveCashOKBTN");
  g_signal_connect(widget, "clicked", G_CALLBACK(GUICallbackHandler),
                   (void *)CASH_OK_BTN);
  gtk_widget_set_sensitive(GTK_WIDGET(widget), false);

  widget = gtk_builder_get_object(builder, "AddRemoveCashValueSpinBTN");
  g_signal_connect(widget, "changed", G_CALLBACK(GUICallbackHandler),
                   (void *)CASH_CURSOR_MOVE);

  widget = gtk_builder_get_object(builder, "MainEditMenuPreferences");
  g_signal_connect(widget, "activate", G_CALLBACK(GUICallbackHandler),
                   (void *)PREF_TOGGLE_BTN);

  widget = gtk_builder_get_object(builder, "PreferencesMenuClose");
  g_signal_connect(widget, "activate", G_CALLBACK(GUICallbackHandler),
                   (void *)PREF_TOGGLE_BTN);

  window = gtk_builder_get_object(builder, "PreferencesWindow");
  g_signal_connect(window, "delete_event",
                   G_CALLBACK(GUICallbackHandler_hide_window_on_delete),
                   (void *)PREF_TOGGLE_BTN);

  widget = gtk_builder_get_object(builder, "PrefStockSymbolUpdateBTN");
  g_signal_connect(widget, "clicked", G_CALLBACK(GUICallbackHandler),
                   (void *)PREF_SYMBOL_UPDATE_BTN);

  widget = gtk_builder_get_object(builder, "PrefHoursSpinBox");
  g_signal_connect(widget, "changed",
                   G_CALLBACK(GUICallbackHandler_pref_hours_spinbutton), NULL);

  widget = gtk_builder_get_object(builder, "PrefShowClocksSwitch");
  g_signal_connect(widget, "state-set",
                   G_CALLBACK(GUICallbackHandler_pref_clock_switch), NULL);

  widget = gtk_builder_get_object(builder, "PrefShowIndicesSwitch");
  g_signal_connect(widget, "state-set",
                   G_CALLBACK(GUICallbackHandler_pref_indices_switch), NULL);

  widget = gtk_builder_get_object(builder, "PrefUpPerMinComboBox");
  g_signal_connect(widget, "changed",
                   G_CALLBACK(GUICallbackHandler_pref_up_min_combobox), NULL);

  widget = gtk_builder_get_object(builder, "PrefDecPlacesComboBox");
  g_signal_connect(widget, "changed",
                   G_CALLBACK(GUICallbackHandler_pref_dec_places_combobox),
                   NULL);

  widget = gtk_builder_get_object(builder, "MainEditMenuAPI");
  g_signal_connect(widget, "activate", G_CALLBACK(GUICallbackHandler),
                   (void *)API_TOGGLE_BTN);

  widget = gtk_builder_get_object(builder, "APIMenuClose");
  g_signal_connect(widget, "activate", G_CALLBACK(GUICallbackHandler),
                   (void *)API_TOGGLE_BTN);

  window = gtk_builder_get_object(builder, "ChangeApiInfoWindow");
  g_signal_connect(window, "delete_event",
                   G_CALLBACK(GUICallbackHandler_hide_window_on_delete),
                   (void *)API_TOGGLE_BTN);

  widget = gtk_builder_get_object(builder, "ChangeApiInfoOKBTN");
  g_signal_connect(widget, "clicked", G_CALLBACK(GUICallbackHandler),
                   (void *)API_OK_BTN);
  gtk_widget_set_sensitive(GTK_WIDGET(widget), false);

  widget = gtk_builder_get_object(builder, "ChangeApiInfoEquityUrlEntryBox");
  g_signal_connect(widget, "changed", G_CALLBACK(GUICallbackHandler),
                   (void *)API_CURSOR_MOVE);

  widget = gtk_builder_get_object(builder, "ChangeApiInfoUrlKeyEntryBox");
  g_signal_connect(widget, "changed", G_CALLBACK(GUICallbackHandler),
                   (void *)API_CURSOR_MOVE);

  widget =
      gtk_builder_get_object(builder, "ChangeApiInfoNasdaqSymbolsUrlEntryBox");
  g_signal_connect(widget, "changed", G_CALLBACK(GUICallbackHandler),
                   (void *)API_CURSOR_MOVE);

  widget =
      gtk_builder_get_object(builder, "ChangeApiInfoNYSESymbolsUrlEntryBox");
  g_signal_connect(widget, "changed", G_CALLBACK(GUICallbackHandler),
                   (void *)API_CURSOR_MOVE);

  widget = gtk_builder_get_object(builder, "MainFileMenuRSI");
  g_signal_connect(widget, "activate", G_CALLBACK(GUICallbackHandler),
                   (void *)RSI_TOGGLE_BTN);

  window = gtk_builder_get_object(builder, "ViewRSIWindow");
  g_signal_connect(window, "delete_event",
                   G_CALLBACK(GUICallbackHandler_hide_window_on_delete),
                   (void *)RSI_TOGGLE_BTN);
  g_signal_connect(window, "configure-event",
                   G_CALLBACK(GUICallbackHandler_window_data),
                   (void *)GUI_RSI_WINDOW);
  gtk_window_resize(GTK_WINDOW(window), W->rsi_width, W->rsi_height);
  gtk_window_move(GTK_WINDOW(window), W->rsi_x_pos, W->rsi_y_pos);

  widget = gtk_builder_get_object(builder, "RSIMenuCloseBTN");
  g_signal_connect(widget, "activate", G_CALLBACK(GUICallbackHandler),
                   (void *)RSI_TOGGLE_BTN);

  widget = gtk_builder_get_object(builder, "ViewRSIFetchDataBTN");
  g_signal_connect(widget, "clicked", G_CALLBACK(GUICallbackHandler),
                   (void *)RSI_FETCH_BTN);

  widget = gtk_builder_get_object(builder, "ViewRSISymbolEntryBox");
  g_signal_connect(widget, "changed", G_CALLBACK(GUICallbackHandler),
                   (void *)RSI_CURSOR_MOVE);

  /* This is the main window's TreeView ( There was only one treeview when I
   * started ). */
  widget = gtk_builder_get_object(builder, "TreeView");
  g_signal_connect(widget, "button-press-event",
                   G_CALLBACK(view_onButtonPressed), NULL);
}

static void start_threads() {
  /* Start the clock threads. */
  /* gdk_threads_add_idle (in these threads) creates a pending event for the
     gtk_main loop. When the gtk_main loop starts the event will be processed.
   */
  pthread_t thread_id;
  pthread_create(&thread_id, NULL, GUIThreadHandler, (void *)MAIN_CLOCK);
  pthread_create(&thread_id, NULL, GUIThreadHandler,
                 (void *)MAIN_TIME_CLOSE_INDICATOR);

  /* Set up the RSIView and AddRemSecurity Window's EntryBox Completion Widgets
     This will populate the NYSE and NASDAQ symbol to name mapping, from an
     sqlite Db, when the application loads.  It will not download anything at
     application startup. The Db of symbol to name mappings needs to be
     downloaded manually by the user [in the API window]. */
  pthread_create(&thread_id, NULL, GUIThreadHandler, (void *)COMPLETION);
}

/* Engineering Note */

/* After the gtk_main loop starts nearly all computational tasks
   are running in a separate thread concurrently with the main thread.

   The GUIThreadHandler function in gui_threads.c is essentially the
   heart of the application after the main loop starts. Nearly every
   gui signal is connected to a callback function which then creates
   a new thread and passes a signal to the GUIThreadHandler function.

   When the GUI needs to be updated a gdk_threads_add_idle statement
   sends a function into the main loop. */

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

  /* Start the clock threads and download the list of stock symbols */
  start_threads();

  /* Add the keyboard shortcuts to the Keyboard Shortcut window. */
  shortcuts_set_treeview();

  /* Add hyperlink markup to the About window labels. */
  about_set_label();

  /* Set whether the clocks are displayed or not. */
  clock_display(data);

  /* Connect callback functions to corresponding GUI signals. */
  gui_signal_connect(data);

  /* Set the default treeview.
     This treeview is already set in the COMPLETION thread,
     however, there may be a networking delay before it is displayed.
     So we set it here showing any available data. */
  MainDefaultTreeview(data);

  /* Start the gtk_main loop. */
  gtk_main();
}