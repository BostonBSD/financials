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

#include "../include/class_types.h" /* portfolio_packet, window_data */
#include "../include/gui.h"         /* Gtk headers and funcs */
#include "../include/multicurl.h"
#include "../include/mutex.h"
#include "../include/workfuncs.h"

gint HistoryCompletionSet(gpointer sn_map_data) {
  CompletionSet((symbol_name_map *)sn_map_data, GUI_COMPLETION_HISTORY);
  return 0;
}

gint HistoryShowHide(portfolio_packet *pkg) {
  window_data *W = pkg->GetWindowData();

  /* get the GObject and cast as a GtkWidget */
  GtkWidget *window = GetWidget("HistoryWindow");
  gboolean visible = gtk_widget_is_visible(window);

  if (visible) {
    gtk_widget_set_visible(window, FALSE);
    gtk_window_resize(GTK_WINDOW(window), W->history_width, W->history_height);
    gtk_window_move(GTK_WINDOW(window), W->history_x_pos, W->history_y_pos);
  } else {
    gtk_window_resize(GTK_WINDOW(window), W->history_width, W->history_height);
    gtk_window_move(GTK_WINDOW(window), W->history_x_pos, W->history_y_pos);

    GtkWidget *Button = GetWidget("HistoryFetchDataBTN");
    gtk_widget_set_sensitive(Button, FALSE);
    g_object_set(G_OBJECT(Button), "can-default", TRUE, "has-default", TRUE,
                 NULL);

    /* Reset EntryBox */
    GtkWidget *EntryBox = GetWidget("HistorySymbolEntryBox");
    gtk_entry_set_text(GTK_ENTRY(EntryBox), "");
    g_object_set(G_OBJECT(EntryBox), "activates-default", TRUE, NULL);
    gtk_widget_grab_focus(EntryBox);

    GtkWidget *Label = GetWidget("HistoryStockSymbolLabel");
    gtk_label_set_text(GTK_LABEL(Label), "");

    GtkWidget *scrwindow = GetWidget("HistoryScrolledWindow");
    gtk_scrolled_window_set_vadjustment(GTK_SCROLLED_WINDOW(scrwindow), NULL);

    gtk_widget_set_visible(window, TRUE);
  }
  return 0;
}

gint HistoryCursorMove() {
  const gchar *s = GetEntryText("HistorySymbolEntryBox");
  GtkWidget *Button = GetWidget("HistoryFetchDataBTN");

  if (CheckValidString(s)) {
    gtk_widget_set_sensitive(Button, TRUE);
  } else {
    gtk_widget_set_sensitive(Button, FALSE);
  }

  return 0;
}

gint HistoryTreeViewClear() {
  /* Clear the GtkTreeView. */
  GtkWidget *treeview = GetWidget("HistoryTreeView");
  return TreeViewClear(treeview);
}

static gchar *col_names[HISTORY_N_COLUMNS] = {
    "Date", "Price",    "High", "Low", "Open",     "Pr. Close",
    "Chg",  "Gain (%)", "Vol.", "RSI", "Indicator"};

static void history_set_columns() {
  GtkWidget *list = GetWidget("HistoryTreeView");

  for (gushort g = 0; g < HISTORY_N_COLUMNS; g++) {
    AddColumnToTreeview(col_names[g], g, list);
  }
}

gint HistorySetSNLabel(gpointer string_font_data) {
  string_font *str_fnt_container = (string_font *)string_font_data;
  GtkWidget *label = GetWidget("HistoryStockSymbolLabel");
  const gchar *fmt = "<span foreground='MidnightBlue' size='large'>%s</span>";

  gushort len = g_utf8_strlen(
      str_fnt_container->string ? str_fnt_container->string : "", -1);
  if (len >= 96) {
    /* We don't want names longer than 95 characters. */
    str_fnt_container->string[95] = 0;
    /* Truncate unnecessary info after the comma. */
    gchar *ch = g_utf8_strchr(str_fnt_container->string, -1, (gunichar)',');
    if (ch != NULL)
      *ch = 0;
  }

  SetFormattedLabel(label, fmt, str_fnt_container->font,
                    str_fnt_container->string ? str_fnt_container->string : "");

  if (str_fnt_container->string)
    g_free(str_fnt_container->string);

  if (str_fnt_container->font)
    g_free(str_fnt_container->font);

  if (str_fnt_container)
    g_free(str_fnt_container);

  return 0;
}

gint HistoryGetSymbol(gchar **s)
/* Get the stock symbol from the EntryBox.
   Must free string buffer.

   Because we aren't setting any widgets, we shouldn't worry about crashing
   Gtk outside the Gtk Main Loop.
*/
{
  const gchar *tmp = GetEntryText("HistorySymbolEntryBox");
  /* Convert to uppercase letters, must free return value. */
  s[0] = g_ascii_strup(tmp, -1);

  return 0;
}

enum { RUN, RESET };
static gboolean history_rsi_ready(gdouble gain_f, gdouble *rsi_f, gint state) {
  /* We need to remember the running averages
     and the counter between iterations. */
  static gushort c = 0;
  static gdouble avg_gain_f = 0.0f, avg_loss_f = 0.0f;
  if (state == RESET) {
    c = 0;
    avg_gain_f = 0.0f;
    avg_loss_f = 0.0f;
    return FALSE;
  }

  /* For 13 days we sum the gains and losses. */
  if (c < 13) {
    CalcSumRsi(gain_f, &avg_gain_f, &avg_loss_f);
  }
  /* On the 14th day we calculate the regular average and use that to seed a
   * running average. */
  else if (c == 13) {
    CalcSumRsi(gain_f, &avg_gain_f, &avg_loss_f);
    avg_gain_f /= 14.0f;
    avg_loss_f /= 14.0f;
  }
  /* On the 15th day we start calculating the RSI. */
  else {
    /* Calculate the running average with a 14 day smoothing period. */
    CalcRunAvgRsi(gain_f, &avg_gain_f, &avg_loss_f, 14.0f);
    /* Calculate the rsi. */
    *rsi_f = CalcRsi(avg_gain_f, avg_loss_f);
    c++;
    return TRUE;
  }
  c++;
  return FALSE;
}

typedef struct {
  gchar *date_ch;
  gchar *price_ch;
  gchar *high_ch;
  gchar *low_ch;
  gchar *opening_ch;
  gchar *prev_closing_ch;
  gchar *change_ch;
  gchar *gain_ch;
  gchar *rsi_ch;
  gchar *volume_ch;
  gchar *indicator_ch;
} history_strings;

static gboolean history_rsi_calculate(gchar *line, history_strings *strings,
                                      gint state) {
  /* We need to remember the last closing price in the next iteration. */
  static gdouble cur_price_f = 0.0f;

  if (state == RESET) {
    /* Reset the static variables. */
    cur_price_f = 0.0f;
    history_rsi_ready(0.0, NULL, RESET);
    return FALSE;
  }

  gdouble gain_f, prev_price_f, rsi_f, change_f;
  gulong volume_long;

  gchar **token_arr = g_strsplit(line, ",", -1);
  if (g_strv_length(token_arr) < 7) {
    g_strfreev(token_arr);
    return FALSE;
  }

  prev_price_f = cur_price_f;
  cur_price_f = g_strtod(token_arr[4] ? token_arr[4] : "0", NULL);

  /* The initial closing price has no prev_price */
  if (prev_price_f == 0.0f) {
    g_strfreev(token_arr);
    return FALSE;
  }

  gain_f = CalcGain(cur_price_f, prev_price_f);
  if (!history_rsi_ready(gain_f, &rsi_f, RUN)) {
    /* Until we get 14 days of data return FALSE. */
    g_strfreev(token_arr);
    return FALSE;
  }

  StringToStrPango(&strings->date_ch,
                   token_arr[0] ? token_arr[0] : "0000-00-00", CHOCOLATE);
  DoubleToFormattedStrPango(&strings->prev_closing_ch, prev_price_f, 2, MON_STR,
                            BLACK);
  DoubleToFormattedStrPango(&strings->price_ch, cur_price_f, 2, MON_STR, BLACK);
  StringToStrPango(&strings->high_ch, token_arr[2] ? token_arr[2] : "0",
                   STR_TO_MON_STR);
  StringToStrPango(&strings->low_ch, token_arr[3] ? token_arr[3] : "0",
                   STR_TO_MON_STR);
  StringToStrPango(&strings->opening_ch, token_arr[1] ? token_arr[1] : "0",
                   STR_TO_MON_STR);
  change_f = cur_price_f - prev_price_f;
  if (change_f > 0) {
    DoubleToFormattedStrPango(&strings->change_ch, change_f, 2, MON_STR, GREEN);
    DoubleToFormattedStrPango(&strings->gain_ch, gain_f, 2, PER_STR, GREEN);
  } else if (change_f < 0) {
    DoubleToFormattedStrPango(&strings->change_ch, change_f, 2, MON_STR, RED);
    DoubleToFormattedStrPango(&strings->gain_ch, gain_f, 2, PER_STR, RED);
  } else {
    DoubleToFormattedStrPango(&strings->change_ch, change_f, 2, MON_STR, BLACK);
    DoubleToFormattedStrPango(&strings->gain_ch, gain_f, 2, PER_STR, BLACK);
  }
  DoubleToFormattedStrPango(&strings->rsi_ch, rsi_f, 2, NUM_STR, BLACK);
  volume_long =
      (gulong)g_ascii_strtoll(token_arr[6] ? token_arr[6] : "0", NULL, 10);
  DoubleToFormattedStrPango(&strings->volume_ch, (double)volume_long, 0,
                            NUM_STR, BLACK);

  if (rsi_f >= 70.0f) {
    StringToStrPango(&strings->indicator_ch, "Overbought", RED);
  } else if (rsi_f >= 60.0f) {
    StringToStrPango(&strings->indicator_ch, "Overbought Warning", ORANGE);
  } else if (rsi_f < 60.0f && rsi_f > 40.0f) {
    StringToStrPango(&strings->indicator_ch, "Neutral", GREY);
  } else if (rsi_f > 30.0f) {
    StringToStrPango(&strings->indicator_ch, "Oversold Warning", CYAN);
  } else {
    StringToStrPango(&strings->indicator_ch, "Oversold", GREEN);
  }

  g_strfreev(token_arr);
  return TRUE;
}

static void history_set_store_cleanup(history_strings *history_strs) {
  /* Reset the static variables. */
  history_rsi_calculate(NULL, NULL, RESET);

  g_free(history_strs->date_ch);
  g_free(history_strs->gain_ch);
  g_free(history_strs->rsi_ch);
  g_free(history_strs->volume_ch);
  g_free(history_strs->price_ch);
  g_free(history_strs->high_ch);
  g_free(history_strs->low_ch);
  g_free(history_strs->opening_ch);
  g_free(history_strs->change_ch);
  g_free(history_strs->prev_closing_ch);
  g_free(history_strs->indicator_ch);
}

static void history_set_store(GtkListStore *store, const gchar *curl_data) {
  if (!curl_data)
    return;

  history_strings history_strs = (history_strings){NULL};

  GtkTreeIter iter;
  gchar *line;

  /* Convert a String to a GDataInputStream for Reading */
  GDataInputStream *in_stream = StringToInputStream(curl_data);

  /* Ignore the header line */
  if (!(line = ReadLine(in_stream))) {
    CloseInputStream(in_stream);
    return;
  }
  free(line);
  while ((line = ReadLine(in_stream))) {

    /* If there is a null error in this line, ignore the line.
       Sometimes Yahoo! data is incomplete, the result is more
       correct if we ignore the incomplete portion of data. */
    /* If we have an empty line, continue. */

    if (g_strrstr(line, "null") || line[0] == '\0') {
      g_free(line);
      continue;
    }
    /* Invalid replies start with a tag. */
    if (g_strrstr(line, "<")) {
      g_free(line);
      break;
    }

    /* Don't start adding rows until we get 14 days of data. */
    if (!history_rsi_calculate(line, &history_strs, RUN)) {
      g_free(line);
      continue;
    }

    /* Add data to the storage container. */
    /* Yahoo! sends data with the earliest date first, so we prepend rows.
       The last [most recent] entry needs to be at the top. */
    gtk_list_store_prepend(store, &iter);
    gtk_list_store_set(
        store, &iter, HISTORY_COLUMN_ONE, history_strs.date_ch,
        HISTORY_COLUMN_TWO, history_strs.price_ch, HISTORY_COLUMN_THREE,
        history_strs.high_ch, HISTORY_COLUMN_FOUR, history_strs.low_ch,
        HISTORY_COLUMN_FIVE, history_strs.opening_ch, HISTORY_COLUMN_SIX,
        history_strs.prev_closing_ch, HISTORY_COLUMN_SEVEN,
        history_strs.change_ch, HISTORY_COLUMN_EIGHT, history_strs.gain_ch,
        HISTORY_COLUMN_NINE, history_strs.volume_ch, HISTORY_COLUMN_TEN,
        history_strs.rsi_ch, HISTORY_COLUMN_ELEVEN, history_strs.indicator_ch,
        -1);
    g_free(line);
  }

  CloseInputStream(in_stream);
  history_set_store_cleanup(&history_strs);
}

GtkListStore *HistoryMakeStore(const gchar *data_str) {
  GtkListStore *store;

  /* Set up the storage container with the number of columns and column type */
  store = gtk_list_store_new(HISTORY_N_COLUMNS, G_TYPE_STRING, G_TYPE_STRING,
                             G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING,
                             G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING,
                             G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);

  /* Add data to the storage container, pass in the data_str pointer. */
  history_set_store(store, data_str);
  return store;
}

int HistoryMakeTreeview(gpointer store_data) {
  GtkListStore *store = (GtkListStore *)store_data;
  GtkWidget *list = GetWidget("HistoryTreeView");

  /* Set the columns for the new TreeView model */
  history_set_columns();

  /* Add the store of data to the list. */
  gtk_tree_view_set_model(GTK_TREE_VIEW(list), GTK_TREE_MODEL(store));

  if (store)
    g_object_unref(store);

  /* Set the list header as visible. */
  gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(list), TRUE);

  /* Remove Grid Lines. */
  gtk_tree_view_set_grid_lines(GTK_TREE_VIEW(list),
                               GTK_TREE_VIEW_GRID_LINES_NONE);

  return 0;
}

MemType *HistoryFetchData(const gchar *symbol_ch, portfolio_packet *pkg) {
  meta *D = pkg->GetMetaClass();
  gchar *MyUrl_ch = NULL;
  MemType *MyOutputStruct = (MemType *)g_malloc(sizeof(*MyOutputStruct));
  MyOutputStruct->memory = NULL;
  MyOutputStruct->size = 0;

  /* Number of Seconds in a Year Plus Three Weeks */
  guint period = 33372000;
  GetYahooUrl(&MyUrl_ch, symbol_ch, period);

  SetUpCurlHandle(D->history_hnd, D->multicurl_history_hnd, MyUrl_ch,
                  MyOutputStruct);
  if (PerformMultiCurl_no_prog(D->multicurl_history_hnd) != 0) {
    g_free(MyUrl_ch);
    FreeMemtype(MyOutputStruct);
    g_free(MyOutputStruct);
    return NULL;
  }

  g_free(MyUrl_ch);
  return MyOutputStruct;
}