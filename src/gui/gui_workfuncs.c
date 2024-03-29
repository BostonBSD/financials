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
#include "../include/class_types.h" /* portfolio_packet, meta */
#include "../include/gui.h"
#include "../include/macros.h"
#include "../include/workfuncs.h"

void AddColumnToTreeview(const gchar *col_name, const gint col_num,
                         GtkWidget *treeview) {
  GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
  GtkTreeViewColumn *column = gtk_tree_view_column_new_with_attributes(
      col_name, renderer, "markup", col_num, NULL);
  gtk_tree_view_column_set_resizable(column, TRUE);
  gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);
}

gint TreeViewClear(GtkWidget *treeview) {
  /* Clear the GtkTreeView. */
  GtkTreeViewColumn *column;
  gushort n = gtk_tree_view_get_n_columns(GTK_TREE_VIEW(treeview));

  while (n) {
    n--;
    column = gtk_tree_view_get_column(GTK_TREE_VIEW(treeview), n);
    gtk_tree_view_remove_column(GTK_TREE_VIEW(treeview), column);
  }

  return 0;
}

/* Set completion widgets for both the security and history entry boxes. */
static GtkListStore *completion_set_store(symbol_name_map *sn_map) {
  if (sn_map == NULL)
    return NULL;
  GtkListStore *store =
      gtk_list_store_new(3, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);
  GtkTreeIter iter;

  gchar item[35];
  /* Populate the GtkListStore with the string of stock symbols in column 0,
     stock names in column 1, and symbols & names in column 2. */
  for (gushort i = 0; i < sn_map->size; i++) {
    g_snprintf(item, 35, "%s - %s", sn_map->sn_container_arr[i]->symbol,
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
                                 const gchar *key, GtkTreeIter *iter,
                                 gpointer data) {
  UNUSED(data)

  GtkTreeModel *model = gtk_entry_completion_get_model(completion);
  gchar *item_symb, *item_name;
  /* We are finding matches based off of column 0 and 1, however,
     we display column 2 in our 3 column model */
  gtk_tree_model_get(model, iter, 0, &item_symb, 1, &item_name, -1);
  gboolean ans = FALSE, symbol_match = TRUE, name_match = TRUE;

  gushort N = 0;
  while (key[N]) {
    /* Only compare new key char if prev char was a match. */
    if (symbol_match)
      symbol_match = (g_ascii_tolower(key[N]) == g_ascii_tolower(item_symb[N]));
    if (name_match)
      name_match = (g_ascii_tolower(key[N]) == g_ascii_tolower(item_name[N]));
    /* Break the loop if both the symbol and the name are not a match. */
    if ((symbol_match == FALSE) && (name_match == FALSE))
      break;
    N++;
  }

  /* if either the symbol or the name match the key value, return TRUE. */
  ans = symbol_match || name_match;
  g_free(item_symb);
  g_free(item_name);

  return ans;
}

gint CompletionSet(symbol_name_map *sn_map, guintptr gui_completion_sig) {
  if (sn_map == NULL)
    return 0;

  GtkWidget *EntryBox = NULL;
  if (gui_completion_sig == GUI_COMPLETION_HISTORY)
    EntryBox = GetWidget("HistorySymbolEntryBox");
  else
    EntryBox = GetWidget("SecuritySymbolEntryBox");

  GtkEntryCompletion *completion = gtk_entry_completion_new();
  GtkListStore *store = completion_set_store(sn_map);
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

  /* Connect GtkEntryCompletion signals to callback */

  /* The text column to insert is column 0
     We use a callback on the 'match-selected' signal and insert the text from
     column 0 instead of column 2 We use a callback on the 'cursor-on-match'
     signal and insert the text from column 0 instead of column 2
  */
  g_signal_connect(G_OBJECT(completion), "match-selected",
                   G_CALLBACK(GUICallback_comp), (gpointer)gui_completion_sig);
  g_signal_connect(G_OBJECT(completion), "cursor-on-match",
                   G_CALLBACK(GUICallback_comp), (gpointer)gui_completion_sig);

  g_object_unref(G_OBJECT(completion));

  return 0;
}

void StartCompletionThread(portfolio_packet *pkg) {
  GThread *g_thread_id;

  /* Set up the EntryBox Completion Widgets
     This will populate the symbol to name mapping,
     from an sqlite Db when the application loads.

     The list of symbol to name mappings need to be
     downloaded by the user [in the preferences window]
     if the db isn't already populated. */
  g_thread_id = g_thread_new(NULL, GUIThread_completion_set, pkg);
  g_thread_unref(g_thread_id);
}

void StartClockThread(portfolio_packet *pkg)
/* Set the initial display of the clocks.
   We don't want the revealer animation on startup. */
{
  meta *D = pkg->GetMetaClass();

  GtkWidget *revealer = GetWidget("MainClockRevealer");
  if (pkg->IsClockDisplayed()) {
    /* Start the clock thread. */
    /* gdk_threads_add_idle (in this thread) creates a pending event for the
       gtk_main loop. When the gtk_main loop starts the event will be processed.
    */
    g_cond_init(&D->gthread_clocks_cond);
    D->gthread_clocks_id = g_thread_new(NULL, GUIThread_clock, pkg);
  }

  /* Revealer animation set to 0 milliseconds */
  gtk_revealer_set_transition_duration(GTK_REVEALER(revealer), 0);
  /* Show/Hide the clocks */
  gtk_revealer_set_reveal_child(GTK_REVEALER(revealer),
                                pkg->IsClockDisplayed());
  /* Revealer animation set to 300 milliseconds */
  gtk_revealer_set_transition_duration(GTK_REVEALER(revealer), 300);
}

static void set_widget_css(GtkWidget *widget, const gchar *css) {
  GtkCssProvider *css_provider = gtk_css_provider_new();

  gtk_css_provider_load_from_data(css_provider, css, -1, NULL);
  GtkStyleContext *context = gtk_widget_get_style_context(widget);
  gtk_style_context_add_provider(context, GTK_STYLE_PROVIDER(css_provider),
                                 GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

  g_object_unref(G_OBJECT(css_provider));
}

static void set_label_font_css(const gchar *widget_name_ch, const gchar *fmt_ch,
                               const gchar *font_ch, const gchar *fnt_sz_ch,
                               const gchar *fg_colr_ch,
                               const gchar *bg_colr_ch) {
  gchar *css = SnPrint(fmt_ch, font_ch, fnt_sz_ch, fg_colr_ch, bg_colr_ch);
  GtkWidget *widget = GetWidget(widget_name_ch);
  set_widget_css(widget, css);
  g_free(css);
}

void SetLabelFonts(const gchar *font_str) {
  gchar *css_fnt_str = PangoToCssFontStr(font_str);

  /* We set the CSS font here at runtime [at startup and during font selection],
   * the color and size attributes can still be altered with Pango markup
   * afterwards.
   *
   * If the font-size attribute is removed from the CSS, the size
   * will vary with the selected fontname, although this would change the user
   * experience [ :-) => :-( ].
   *
   * The font can also be added to the Pango markup
   * for a similar effect in set_indice_value_label(), set_indices_labels(),
   * main_display_time(), main_display_time_remaining(), HistorySetSNLabel(),
   * and SecurityComBoxChange(): gui_main.c, gui_history.c, and gui_security.c.
   */
  gchar *css_fmt = "label{font:%s;font-size:%s;color:%s;background-color:%s;}";
  set_label_font_css("SecurityWindowLabel", css_fmt, css_fnt_str, "medium",
                     "MidnightBlue", "White");
  set_label_font_css("HistoryStockSymbolLabel", css_fmt, css_fnt_str, "medium",
                     "MidnightBlue", "White");
  set_label_font_css("NewYorkTimeLabel", css_fmt, css_fnt_str, "small", "Black",
                     "White");
  set_label_font_css("MarketCloseLabel", css_fmt, css_fnt_str, "small", "Black",
                     "White");
  set_label_font_css("NYTimeLabel", css_fmt, css_fnt_str, "small", "DimGray",
                     "White");
  set_label_font_css("TimeLeftLabel", css_fmt, css_fnt_str, "small", "DimGray",
                     "White");

  css_fmt = "grid*{font:%s;font-size:%s;color:%s;background-color:%s;}";
  set_label_font_css("MainIndicesHeaderGrid", css_fmt, css_fnt_str, "medium",
                     "DarkSlateGrey", "Bisque");
  /* Notice that some of the values within the following grid are red and green,
   * this is due to Pango markup. */
  set_label_font_css("MainIndicesValueGrid", css_fmt, css_fnt_str, "medium",
                     "Black", "Bisque");

  /* Make sure the background of the unused clock grid cells are set. */
  css_fmt = "grid{background-color:%s;}";
  gchar *css = SnPrint(css_fmt, "White");
  GtkWidget *widget = GetWidget("MainClockGrid");
  set_widget_css(widget, css);
  g_free(css);

  g_free(css_fnt_str);
}