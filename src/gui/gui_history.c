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
#include "../include/gui.h"         /* Gtk headers and funcs */

#include "../include/csv.h"
#include "../include/mutex.h"
#include "../include/workfuncs.h"

int HistoryCompletionSet(void *data) {
  CompletionSet(data, GUI_COMPLETION_HISTORY);
  return 0;
}

int HistoryShowHide(void *data) {
  portfolio_packet *pkg = (portfolio_packet *)data;
  window_data *W = pkg->GetWindowData();

  /* get the GObject and cast as a GtkWidget */
  GtkWidget *window = GetWidget("HistoryWindow");
  gboolean visible = gtk_widget_is_visible(window);

  if (visible) {
    gtk_widget_set_visible(window, false);
    gtk_window_resize(GTK_WINDOW(window), W->history_width, W->history_height);
    gtk_window_move(GTK_WINDOW(window), W->history_x_pos, W->history_y_pos);
  } else {
    gtk_window_resize(GTK_WINDOW(window), W->history_width, W->history_height);
    gtk_window_move(GTK_WINDOW(window), W->history_x_pos, W->history_y_pos);

    GtkWidget *Button = GetWidget("HistoryFetchDataBTN");
    gtk_widget_set_sensitive(Button, false);
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

    gtk_widget_set_visible(window, true);
  }
  return 0;
}

int HistoryCursorMove() {
  const gchar *s = GetEntryText("HistorySymbolEntryBox");
  GtkWidget *Button = GetWidget("HistoryFetchDataBTN");

  if (CheckValidString(s)) {
    gtk_widget_set_sensitive(Button, true);
  } else {
    gtk_widget_set_sensitive(Button, false);
  }

  return 0;
}

int HistoryTreeViewClear() {
  /* Clear the GtkTreeView. */
  GtkWidget *treeview = GetWidget("HistoryTreeView");
  GtkTreeViewColumn *column;
  gushort n = gtk_tree_view_get_n_columns(GTK_TREE_VIEW(treeview));

  while (n) {
    n--;
    column = gtk_tree_view_get_column(GTK_TREE_VIEW(treeview), n);
    gtk_tree_view_remove_column(GTK_TREE_VIEW(treeview), column);
  }

  return 0;
}

static char *col_names[HISTORY_N_COLUMNS] = {
    "Date", "Price",    "High", "Low", "Open",     "Pr. Close",
    "Chg",  "Gain (%)", "Vol.", "RSI", "Indicator"};

static void history_set_columns() {
  GtkWidget *list = GetWidget("HistoryTreeView");

  for (gushort g = 0; g < HISTORY_N_COLUMNS; g++) {
    AddColumnToTreeview(col_names[g], g, list);
  }
}

int HistorySetSNLabel(void *data) {
  gchar *sec_name;
  data ? (sec_name = (gchar *)data) : (sec_name = NULL);

  GtkWidget *Label = GetWidget("HistoryStockSymbolLabel");

  gushort len = strlen(sec_name ? sec_name : "");
  if (len >= 96) {
    sec_name[95] = 0;
    gchar *ch = strchr(sec_name, (int)',');
    if (ch != NULL)
      *ch = 0;
  }

  gtk_label_set_text(GTK_LABEL(Label), sec_name ? sec_name : "");
  if (sec_name)
    g_free(sec_name);

  return 0;
}

int HistoryGetSymbol(char **s)
/* Get the stock symbol from the EntryBox.
   Must free string buffer.

   Because we aren't setting any widgets, we shouldn't worry about crashing
   Gtk outside the Gtk Main Loop.
*/
{
  s[0] = strdup(GetEntryText("HistorySymbolEntryBox"));
  UpperCaseStr(s[0]);

  return 0;
}

enum { RUN, RESET };
static bool history_rsi_ready(double gain_f, double *rsi_f, int state) {
  /* We need to remember the running averages
     and the counter between iterations. */
  static unsigned short c = 0;
  static double avg_gain_f = 0.0f, avg_loss_f = 0.0f;
  if (state == RESET) {
    c = 0;
    avg_gain_f = 0.0f;
    avg_loss_f = 0.0f;
    return false;
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
    /* Calculate the running average. */
    CalcRunAvgRsi(gain_f, &avg_gain_f, &avg_loss_f);
    /* Calculate the rsi. */
    *rsi_f = CalcRsi(avg_gain_f, avg_loss_f);
    c++;
    return true;
  }
  c++;
  return false;
}

typedef struct {
  char *date_ch;
  char *price_ch;
  char *high_ch;
  char *low_ch;
  char *opening_ch;
  char *prev_closing_ch;
  char *change_ch;
  char *gain_ch;
  char *rsi_ch;
  char *volume_ch;
  char *indicator_ch;
} history_strings;

static bool history_rsi_calculate(char *line, history_strings *strings,
                                  int state) {
  /* We need to remember the last closing price in the next iteration. */
  static double cur_price_f = 0.0f;

  if (state == RESET) {
    /* Reset the static variables. */
    cur_price_f = 0.0f;
    history_rsi_ready(0.0, NULL, RESET);
    return false;
  }

  double gain_f, prev_price_f, rsi_f, change_f;
  unsigned long volume_long;

  Chomp(line);
  char **csv_array = parse_csv(line);
  if (csv_array == NULL)
    return false;
  prev_price_f = cur_price_f;
  cur_price_f = strtod(csv_array[4] ? csv_array[4] : "0", NULL);

  /* The initial closing price has no prev_price */
  if (prev_price_f == 0.0f) {
    free_csv_line(csv_array);
    return false;
  }

  gain_f = CalcGain(cur_price_f, prev_price_f);
  if (!history_rsi_ready(gain_f, &rsi_f, RUN)) {
    /* Until we get 14 days of data return false. */
    free_csv_line(csv_array);
    return false;
  }

  StringToStrPango(&strings->date_ch,
                   csv_array[0] ? csv_array[0] : "0000-00-00", CHOCOLATE);
  DoubleToFormattedStrPango(&strings->prev_closing_ch, prev_price_f, 2, MON_STR,
                            BLACK);
  DoubleToFormattedStrPango(&strings->price_ch, cur_price_f, 2, MON_STR, BLACK);
  StringToStrPango(&strings->high_ch, csv_array[2] ? csv_array[2] : "0",
                   STR_TO_MON_STR);
  StringToStrPango(&strings->low_ch, csv_array[3] ? csv_array[3] : "0",
                   STR_TO_MON_STR);
  StringToStrPango(&strings->opening_ch, csv_array[1] ? csv_array[1] : "0",
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
      (unsigned long)strtol(csv_array[6] ? csv_array[6] : "0", NULL, 10);
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

  free_csv_line(csv_array);
  return true;
}

static void history_set_store_thd_cleanup(void *data) {
  history_strings *history_strs = (history_strings *)data;

  /* Reset the static variables. */
  history_rsi_calculate(NULL, NULL, RESET);

  free(history_strs->date_ch);
  free(history_strs->gain_ch);
  free(history_strs->rsi_ch);
  free(history_strs->volume_ch);
  free(history_strs->price_ch);
  free(history_strs->high_ch);
  free(history_strs->low_ch);
  free(history_strs->opening_ch);
  free(history_strs->change_ch);
  free(history_strs->prev_closing_ch);
  free(history_strs->indicator_ch);
}

static void history_set_store(GtkListStore *store, const char *curl_data) {
  if (curl_data == NULL) {
    return;
  }

  history_strings history_strs = (history_strings){NULL};
  pthread_cleanup_push(history_set_store_thd_cleanup, &history_strs);

  GtkTreeIter iter;

  /* Convert a String to a File Pointer Stream for Reading */
  FILE *fp = fmemopen((void *)curl_data, strlen(curl_data) + 1, "r");
  char line[1024];

  /* Ignore the header line */
  if (fgets(line, 1024, fp) == NULL) {
    fclose(fp);
    return;
  }
  while (fgets(line, 1024, fp) != NULL) {
    /* If there is a null error in this line, ignore the line.
       Sometimes Yahoo! data is incomplete, the result is more
       correct if we ignore the incomplete portion of data. */
    if (strstr(line, "null"))
      continue;
    /* 404 replies start with an html tag after the header line. */
    if (strstr(line, "<html>"))
      break;

    /* Don't start adding rows until we get 14 days of data. */
    if (!history_rsi_calculate(line, &history_strs, RUN))
      continue;

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
  }

  fclose(fp);
  pthread_cleanup_pop(1);
}

GtkListStore *HistoryMakeStore(const char *data_str) {
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

int HistoryMakeTreeview(void *data) {
  GtkListStore *store = (GtkListStore *)data;
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