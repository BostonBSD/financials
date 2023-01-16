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

#include "../include/class_types.h" /* portfolio_packet, window_data */
#include "../include/gui.h"
#include "../include/mutex.h"
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

void AddColumnToTreeview(const char *col_name, const int col_num,
                         GtkWidget *treeview) {
  GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
  GtkTreeViewColumn *column = gtk_tree_view_column_new_with_attributes(
      col_name, renderer, "markup", col_num, NULL);
  gtk_tree_view_column_set_resizable(column, true);
  gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);
}

/* Set completion widgets for both the security and history entry boxes. */
static GtkListStore *completion_set_store(symbol_name_map *sn_map) {
  GtkListStore *store =
      gtk_list_store_new(3, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);
  GtkTreeIter iter;

  gchar item[35];
  /* Populate the GtkListStore with the string of stock symbols in column 0,
     stock names in column 1, and symbols & names in column 2. */
  for (gushort i = 0; i < sn_map->size; i++) {
    snprintf(item, 35, "%s - %s", sn_map->sn_container_arr[i]->symbol,
             sn_map->sn_container_arr[i]->security_name);

    gtk_list_store_append(store, &iter);
    /* Completion is going to match off of columns 0 and 1, but display column 2
     */
    /* Completion matches based off of the symbol or the company name, inserts
     * the symbol, displays both */
    gtk_list_store_set(store, &iter, 0, sn_map->sn_container_arr[i]->symbol, 1,
                       sn_map->sn_container_arr[i]->security_name, 2, item, -1);
  }
  return store;
}

static gboolean completion_match(GtkEntryCompletion *completion,
                                 const gchar *key, GtkTreeIter *iter) {
  GtkTreeModel *model = gtk_entry_completion_get_model(completion);
  gchar *item_symb, *item_name;
  /* We are finding matches based off of column 0 and 1, however,
     we display column 2 in our 3 column model */
  gtk_tree_model_get(model, iter, 0, &item_symb, 1, &item_name, -1);
  gboolean ans = false, symbol_match = true, name_match = true;

  gushort N = 0;
  while (key[N]) {
    /* Only compare new key char if prev char was a match. */
    if (symbol_match)
      symbol_match = (tolower(key[N]) == tolower(item_symb[N]));
    if (name_match)
      name_match = (tolower(key[N]) == tolower(item_name[N]));
    /* Break the loop if both the symbol and the name are not a match. */
    if ((symbol_match == false) && (name_match == false))
      break;
    N++;
  }

  /* if either the symbol or the name match the key value, return true. */
  ans = symbol_match || name_match;
  g_free(item_symb);
  g_free(item_name);

  return ans;
}

int CompletionSet(void *data, uintptr_t gui_completion_sig) {
  if (data == NULL)
    return 0;

  GtkWidget *EntryBox = NULL;
  if (gui_completion_sig == GUI_COMPLETION_HISTORY) {
    EntryBox = GetWidget("HistorySymbolEntryBox");
  } else {
    EntryBox = GetWidget("SecuritySymbolEntryBox");
  }

  GtkEntryCompletion *completion = gtk_entry_completion_new();
  GtkListStore *store = completion_set_store((symbol_name_map *)data);
  gtk_entry_completion_set_model(completion, GTK_TREE_MODEL(store));
  g_object_unref(G_OBJECT(store));

  gtk_entry_completion_set_match_func(
      completion, (GtkEntryCompletionMatchFunc)completion_match, NULL, NULL);

  /* Set entrybox completion widget. */
  gtk_entry_set_completion(GTK_ENTRY(EntryBox), completion);

  /* The text column to display is column 2 */
  gtk_entry_completion_set_text_column(completion, 2);
  gtk_entry_completion_set_inline_completion(completion, FALSE);
  gtk_entry_completion_set_inline_selection(completion, TRUE);
  gtk_entry_completion_set_popup_completion(completion, TRUE);
  /* Must type at least two characters for completion to make suggestions,
     reduces the number of results for single character keys. */
  gtk_entry_completion_set_minimum_key_length(completion, 2);

  /* Connect GtkEntryCompletion signals to callbacks */

  /* The text column to insert is column 0
     We use a callback on the 'match-selected' signal and insert the text from
     column 0 instead of column 2 We use a callback on the 'cursor-on-match'
     signal and insert the text from column 0 instead of column 2
  */
  g_signal_connect(G_OBJECT(completion), "match-selected",
                   G_CALLBACK(GUICallbackHandler_select_comp),
                   (void *)gui_completion_sig);
  g_signal_connect(G_OBJECT(completion), "cursor-on-match",
                   G_CALLBACK(GUICallbackHandler_cursor_comp),
                   (void *)gui_completion_sig);

  g_object_unref(G_OBJECT(completion));

  return 0;
}

static void completion_set_start_thd(void *data) {
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

/* Some other preliminary functions. */
static struct {
  const char *name;
  const char *shortcut;
} commands[] = {{"Application Window", ""},
                {"      File", "Ctrl - F"},
                {"      History", "Ctrl - R"},
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
                {"History Window", ""},
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
  GtkWidget *TreeView = GetWidget("HotkeysTreeView");
  char *cmd_name_markup = NULL, *cmd_shortcut_markup = NULL;

  /* In order to display a model/store we need to set the TreeView Columns. */
  AddColumnToTreeview("column_one", 0, TreeView);
  AddColumnToTreeview("column_two", 1, TreeView);

  /* Here we set the rows for the 2 column store */
  GtkListStore *store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_STRING);
  GtkTreeIter iter;

  const char *fmt =
      "<span font_desc='Cantarell Regular 10' foreground='black'>%s</span>";
  for (int i = 0; i < G_N_ELEMENTS(commands); i++) {
    /* We mark them up slightly first. */
    cmd_name_markup = g_markup_printf_escaped(fmt, commands[i].name);
    cmd_shortcut_markup = g_markup_printf_escaped(fmt, commands[i].shortcut);

    /* We add them to a new row. */
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, cmd_name_markup, 1, cmd_shortcut_markup,
                       -1);
    free(cmd_name_markup);
    free(cmd_shortcut_markup);
  }

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
    pthread_create(&D->thread_id_clock, NULL, GUIThreadHandler_main_clock, pkg);
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

/* GtkWidget/GObject signal connect functions. */
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

  object = GetGObject("MainFileMenuHistory");
  g_signal_connect(object, "activate", G_CALLBACK(GUICallbackHandler),
                   (void *)HISTORY_TOGGLE_BTN);

  object = GetGObject("MainFileMenuQuit");
  g_signal_connect(object, "activate", G_CALLBACK(GUICallbackHandler),
                   (void *)MAIN_EXIT);

  object = GetGObject("MainEditMenuSecurity");
  g_signal_connect(object, "activate", G_CALLBACK(GUICallbackHandler),
                   (void *)SECURITY_TOGGLE_BTN);

  object = GetGObject("MainEditMenuBullion");
  g_signal_connect(object, "activate", G_CALLBACK(GUICallbackHandler),
                   (void *)BUL_TOGGLE_BTN);

  object = GetGObject("MainEditMenuCash");
  g_signal_connect(object, "activate", G_CALLBACK(GUICallbackHandler),
                   (void *)CASH_TOGGLE_BTN);

  object = GetGObject("MainEditMenuAPI");
  g_signal_connect(object, "activate", G_CALLBACK(GUICallbackHandler),
                   (void *)API_TOGGLE_BTN);

  object = GetGObject("MainEditMenuPref");
  g_signal_connect(object, "activate", G_CALLBACK(GUICallbackHandler),
                   (void *)PREF_TOGGLE_BTN);

  object = GetGObject("MainHelpMenuHotkeys");
  g_signal_connect(object, "activate", G_CALLBACK(GUICallbackHandler),
                   (void *)HOTKEYS_TOGGLE_BTN);

  object = GetGObject("MainHelpMenuAbout");
  g_signal_connect(object, "activate", G_CALLBACK(GUICallbackHandler),
                   (void *)ABOUT_TOGGLE_BTN);

  object = GetGObject("MainTreeView");
  g_signal_connect(object, "button-press-event",
                   G_CALLBACK(view_onButtonPressed), NULL);
}

static void security_window_sig_connect() {
  GObject *window, *object;

  object = GetGObject("SecurityMenuClose");
  g_signal_connect(object, "activate", G_CALLBACK(GUICallbackHandler),
                   (void *)SECURITY_TOGGLE_BTN);

  window = GetGObject("SecurityWindow");
  g_signal_connect(window, "delete_event",
                   G_CALLBACK(GUICallbackHandler_hide_window_on_delete),
                   (void *)SECURITY_TOGGLE_BTN);

  object = GetGObject("SecurityOkBTN");
  g_signal_connect(object, "clicked", G_CALLBACK(GUICallbackHandler),
                   (void *)SECURITY_OK_BTN);

  object = GetGObject("SecurityStack");
  g_signal_connect(object, "notify::visible-child",
                   G_CALLBACK(GUICallbackHandler_security_stack), NULL);

  object = GetGObject("SecurityComboBox");
  g_signal_connect(object, "changed", G_CALLBACK(GUICallbackHandler),
                   (void *)SECURITY_COMBO_BOX);

  object = GetGObject("SecuritySymbolEntryBox");
  g_signal_connect(object, "changed", G_CALLBACK(GUICallbackHandler),
                   (void *)SECURITY_CURSOR_MOVE);

  object = GetGObject("SecuritySharesEntryBox");
  g_signal_connect(object, "changed", G_CALLBACK(GUICallbackHandler),
                   (void *)SECURITY_CURSOR_MOVE);
}

static void bullion_window_sig_connect() {
  GObject *window, *object;

  object = GetGObject("BullionMenuClose");
  g_signal_connect(object, "activate", G_CALLBACK(GUICallbackHandler),
                   (void *)BUL_TOGGLE_BTN);

  window = GetGObject("BullionWindow");
  g_signal_connect(window, "delete_event",
                   G_CALLBACK(GUICallbackHandler_hide_window_on_delete),
                   (void *)BUL_TOGGLE_BTN);

  object = GetGObject("BullionOKBTN");
  g_signal_connect(object, "clicked", G_CALLBACK(GUICallbackHandler),
                   (void *)BUL_OK_BTN);
  gtk_widget_set_sensitive(GTK_WIDGET(object), false);

  object = GetGObject("BullionGoldOuncesEntryBox");
  g_signal_connect(object, "changed", G_CALLBACK(GUICallbackHandler),
                   (void *)BUL_CURSOR_MOVE);

  object = GetGObject("BullionGoldPremiumEntryBox");
  g_signal_connect(object, "changed", G_CALLBACK(GUICallbackHandler),
                   (void *)BUL_CURSOR_MOVE);

  object = GetGObject("BullionSilverOuncesEntryBox");
  g_signal_connect(object, "changed", G_CALLBACK(GUICallbackHandler),
                   (void *)BUL_CURSOR_MOVE);

  object = GetGObject("BullionSilverPremiumEntryBox");
  g_signal_connect(object, "changed", G_CALLBACK(GUICallbackHandler),
                   (void *)BUL_CURSOR_MOVE);

  object = GetGObject("BullionPlatinumOuncesEntryBox");
  g_signal_connect(object, "changed", G_CALLBACK(GUICallbackHandler),
                   (void *)BUL_CURSOR_MOVE);

  object = GetGObject("BullionPlatinumPremiumEntryBox");
  g_signal_connect(object, "changed", G_CALLBACK(GUICallbackHandler),
                   (void *)BUL_CURSOR_MOVE);

  object = GetGObject("BullionPalladiumOuncesEntryBox");
  g_signal_connect(object, "changed", G_CALLBACK(GUICallbackHandler),
                   (void *)BUL_CURSOR_MOVE);

  object = GetGObject("BullionPalladiumPremiumEntryBox");
  g_signal_connect(object, "changed", G_CALLBACK(GUICallbackHandler),
                   (void *)BUL_CURSOR_MOVE);

  object = GetGObject("BullionComboBox");
  g_signal_connect(object, "changed", G_CALLBACK(GUICallbackHandler),
                   (void *)BUL_COMBO_BOX);
}

static void preferences_window_sig_connect(void *data) {
  portfolio_packet *pkg = (portfolio_packet *)data;
  meta *D = pkg->GetMetaClass();
  GObject *window, *object;

  object = GetGObject("PrefMenuClose");
  g_signal_connect(object, "activate", G_CALLBACK(GUICallbackHandler),
                   (void *)PREF_TOGGLE_BTN);

  window = GetGObject("PrefWindow");
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

  object = GetGObject("PrefFontChooserBTN");
  /* Make sure the font is set before connecting a signal to it. */
  gtk_font_chooser_set_font(GTK_FONT_CHOOSER(object), D->main_font_ch);
  g_signal_connect(object, "font-set",
                   G_CALLBACK(GUICallbackHandler_pref_font_button), NULL);
}

static void api_window_sig_connect() {
  GObject *window, *object;

  object = GetGObject("APIMenuClose");
  g_signal_connect(object, "activate", G_CALLBACK(GUICallbackHandler),
                   (void *)API_TOGGLE_BTN);

  window = GetGObject("ApiWindow");
  g_signal_connect(window, "delete_event",
                   G_CALLBACK(GUICallbackHandler_hide_window_on_delete),
                   (void *)API_TOGGLE_BTN);

  object = GetGObject("ApiOKBTN");
  g_signal_connect(object, "clicked", G_CALLBACK(GUICallbackHandler),
                   (void *)API_OK_BTN);
  gtk_widget_set_sensitive(GTK_WIDGET(object), false);

  object = GetGObject("ApiEquityUrlEntryBox");
  g_signal_connect(object, "changed", G_CALLBACK(GUICallbackHandler),
                   (void *)API_CURSOR_MOVE);

  object = GetGObject("ApiUrlKeyEntryBox");
  g_signal_connect(object, "changed", G_CALLBACK(GUICallbackHandler),
                   (void *)API_CURSOR_MOVE);

  object = GetGObject("ApiNasdaqSymbolsUrlEntryBox");
  g_signal_connect(object, "changed", G_CALLBACK(GUICallbackHandler),
                   (void *)API_CURSOR_MOVE);

  object = GetGObject("ApiNYSESymbolsUrlEntryBox");
  g_signal_connect(object, "changed", G_CALLBACK(GUICallbackHandler),
                   (void *)API_CURSOR_MOVE);
}

static void history_window_sig_connect(void *data) {
  portfolio_packet *pkg = (portfolio_packet *)data;
  window_data *W = pkg->GetWindowData();
  GObject *window, *object;

  window = GetGObject("HistoryWindow");
  g_signal_connect(window, "delete_event",
                   G_CALLBACK(GUICallbackHandler_hide_window_on_delete),
                   (void *)HISTORY_TOGGLE_BTN);
  gtk_window_resize(GTK_WINDOW(window), W->history_width, W->history_height);
  gtk_window_move(GTK_WINDOW(window), W->history_x_pos, W->history_y_pos);

  g_signal_connect(window, "configure-event",
                   G_CALLBACK(GUICallbackHandler_window_data),
                   (void *)GUI_HISTORY_WINDOW);

  object = GetGObject("HistoryMenuCloseBTN");
  g_signal_connect(object, "activate", G_CALLBACK(GUICallbackHandler),
                   (void *)HISTORY_TOGGLE_BTN);

  object = GetGObject("HistoryFetchDataBTN");
  g_signal_connect(object, "clicked", G_CALLBACK(GUICallbackHandler),
                   (void *)HISTORY_FETCH_BTN);

  object = GetGObject("HistorySymbolEntryBox");
  g_signal_connect(object, "changed", G_CALLBACK(GUICallbackHandler),
                   (void *)HISTORY_CURSOR_MOVE);
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

  object = GetGObject("HotkeysMenuClose");
  g_signal_connect(object, "activate", G_CALLBACK(GUICallbackHandler),
                   (void *)HOTKEYS_TOGGLE_BTN);

  window = GetGObject("HotkeysWindow");
  g_signal_connect(window, "delete_event",
                   G_CALLBACK(GUICallbackHandler_hide_window_on_delete),
                   (void *)HOTKEYS_TOGGLE_BTN);

  object = GetGObject("CashMenuClose");
  g_signal_connect(object, "activate", G_CALLBACK(GUICallbackHandler),
                   (void *)CASH_TOGGLE_BTN);

  window = GetGObject("CashWindow");
  g_signal_connect(window, "delete_event",
                   G_CALLBACK(GUICallbackHandler_hide_window_on_delete),
                   (void *)CASH_TOGGLE_BTN);

  object = GetGObject("CashOKBTN");
  g_signal_connect(object, "clicked", G_CALLBACK(GUICallbackHandler),
                   (void *)CASH_OK_BTN);
  gtk_widget_set_sensitive(GTK_WIDGET(object), false);

  object = GetGObject("CashSpinBTN");
  g_signal_connect(object, "changed", G_CALLBACK(GUICallbackHandler),
                   (void *)CASH_CURSOR_MOVE);
}

static void gui_signal_connect(void *data)
/* Connect widget signals to signal handlers. */
{
  main_window_sig_connect(data);
  security_window_sig_connect();
  bullion_window_sig_connect();
  preferences_window_sig_connect(data);
  api_window_sig_connect();
  history_window_sig_connect(data);
  other_window_sig_connect();
}

/* Engineering Note */

/* After the gtk_main() loop starts nearly every widget signal is connected
   to the GUICallbackHandler function in gui_callbacks.c.

   Some signals require separate threads to perform computational tasks, these
   are dispatched from the GUICallbackHandler functions [in gui_callbacks.c].
   The clock threads are dispatched by separate functions [clock_display(),
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
  completion_set_start_thd(data);

  /* Add the keyboard shortcuts to the Hotkeys window. */
  hotkeys_set_treeview();

  /* Add hyperlink markup to the About window labels. */
  about_set_label();

  /* Make sure the main labels and treeview header fonts are set */
  MainSetFonts(data);

  /* Connect callback functions to corresponding GUI signals. */
  gui_signal_connect(data);

  /* Set the default treeview.
     This treeview is already set in the GUIThreadHandler_completion_set thread
     [created in completion_set_start_thd()], however, there may be a db read
     delay before it is displayed. So we set it here showing any available data.
   */
  portfolio_packet *pkg = (portfolio_packet *)data;
  pkg->ToStrings();
  MainDefaultTreeview(data);

  /* Start the gtk_main loop. */
  gtk_main();
}