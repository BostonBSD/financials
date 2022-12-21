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
#include <locale.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include <glib-object.h>
#include <gtk/gtk.h>

#include "../include/gui_globals.h" /* GtkBuilder *builder */
#include "../include/gui_types.h"   /* enums, etc */

#include "../include/class.h" /* ClassDestructPortfolioPacket(), portfolio_packet, 
                                                equity_folder, metal, meta  */
#include "../include/macros.h"
#include "../include/workfuncs.h"

int MainFetchBTNLabel(void *data) {
  portfolio_packet *pkg = (portfolio_packet *)data;
  GtkWidget *label =
      GTK_WIDGET(gtk_builder_get_object(builder, "FetchDataBtnLabel"));

  if (pkg->IsFetchingData() == true) {
    gtk_label_set_label(GTK_LABEL(label), "Stop Updates");
  } else {
    gtk_label_set_label(GTK_LABEL(label), "Get Data");
  }

  return 0;
}

static int main_prog_bar(void *data) {
  double *fraction = (double *)data;

  GtkWidget *ProgressBar =
      GTK_WIDGET(gtk_builder_get_object(builder, "ProgressBar"));
  gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(ProgressBar), *fraction);

  return 0;
}

void MainProgBar(double *fraction) {
  if (*fraction > 1)
    *fraction = 1;
  if (*fraction < 0)
    *fraction = 0;
  gdk_threads_add_idle(main_prog_bar, fraction);
}

static int main_tree_view_clr() {
  /* Clear the main window's GtkTreeView. */
  GtkWidget *treeview = GTK_WIDGET(gtk_builder_get_object(builder, "TreeView"));
  GtkTreeViewColumn *column;
  guint n = gtk_tree_view_get_n_columns(GTK_TREE_VIEW(treeview));

  while (n) {
    n--;
    column = gtk_tree_view_get_column(GTK_TREE_VIEW(treeview), n);
    gtk_tree_view_remove_column(GTK_TREE_VIEW(treeview), column);
  }

  return 0;
}

static int main_set_columns(int column_type) {
  GtkCellRenderer *renderer;
  GtkTreeViewColumn *column;
  GtkWidget *list = GTK_WIDGET(gtk_builder_get_object(builder, "TreeView"));

  renderer = gtk_cell_renderer_text_new();
  column = gtk_tree_view_column_new_with_attributes("type_text", renderer,
                                                    "text", GUI_TYPE, NULL);
  gtk_tree_view_column_set_visible(column, false);
  gtk_tree_view_column_set_min_width(column, 0);
  gtk_tree_view_append_column(GTK_TREE_VIEW(list), column);

  renderer = gtk_cell_renderer_text_new();
  column = gtk_tree_view_column_new_with_attributes("symbol_text", renderer,
                                                    "text", GUI_SYMBOL, NULL);
  gtk_tree_view_column_set_visible(column, false);
  gtk_tree_view_append_column(GTK_TREE_VIEW(list), column);

  renderer = gtk_cell_renderer_text_new();
  column = gtk_tree_view_column_new_with_attributes(
      "column_one", renderer, "markup", GUI_COLUMN_ONE, NULL);
  gtk_tree_view_column_set_resizable(column, true);
  gtk_tree_view_column_set_visible(column, true);
  gtk_tree_view_append_column(GTK_TREE_VIEW(list), column);

  renderer = gtk_cell_renderer_text_new();
  column = gtk_tree_view_column_new_with_attributes(
      "column_two", renderer, "markup", GUI_COLUMN_TWO, NULL);
  gtk_tree_view_column_set_resizable(column, true);
  gtk_tree_view_append_column(GTK_TREE_VIEW(list), column);

  renderer = gtk_cell_renderer_text_new();
  column = gtk_tree_view_column_new_with_attributes(
      "column_three", renderer, "markup", GUI_COLUMN_THREE, NULL);
  gtk_tree_view_column_set_resizable(column, true);
  gtk_tree_view_append_column(GTK_TREE_VIEW(list), column);

  if (column_type == GUI_COLUMN_PRIMARY) {
    /* if PRIMARY treeview */
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes(
        "column_four", renderer, "markup", GUI_COLUMN_FOUR, NULL);
    gtk_tree_view_column_set_resizable(column, true);
    gtk_tree_view_append_column(GTK_TREE_VIEW(list), column);

    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes(
        "column_five", renderer, "markup", GUI_COLUMN_FIVE, NULL);
    gtk_tree_view_column_set_resizable(column, true);
    gtk_tree_view_append_column(GTK_TREE_VIEW(list), column);

    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes(
        "column_six", renderer, "markup", GUI_COLUMN_SIX, NULL);
    gtk_tree_view_column_set_resizable(column, true);
    gtk_tree_view_append_column(GTK_TREE_VIEW(list), column);

    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes(
        "column_seven", renderer, "markup", GUI_COLUMN_SEVEN, NULL);
    gtk_tree_view_column_set_resizable(column, true);
    gtk_tree_view_append_column(GTK_TREE_VIEW(list), column);

    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes(
        "column_eight", renderer, "markup", GUI_COLUMN_EIGHT, NULL);
    gtk_tree_view_column_set_resizable(column, true);
    gtk_tree_view_append_column(GTK_TREE_VIEW(list), column);

    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes(
        "column_nine", renderer, "markup", GUI_COLUMN_NINE, NULL);
    gtk_tree_view_column_set_resizable(column, true);
    gtk_tree_view_append_column(GTK_TREE_VIEW(list), column);

    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes(
        "column_ten", renderer, "markup", GUI_COLUMN_TEN, NULL);
    gtk_tree_view_column_set_resizable(column, true);
    gtk_tree_view_append_column(GTK_TREE_VIEW(list), column);

    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes(
        "column_eleven", renderer, "markup", GUI_COLUMN_ELEVEN, NULL);
    gtk_tree_view_column_set_resizable(column, true);
    gtk_tree_view_append_column(GTK_TREE_VIEW(list), column);
  }

  return 0;
}

static GtkListStore *main_primary_store(void *data) {
  /* Unpack the class package */
  portfolio_packet *package = (portfolio_packet *)data;
  metal *M = package->GetMetalClass();
  equity_folder *F = package->GetEquityFolderClass();
  meta *D = package->GetMetaClass();
  primary_heading *pri_h_mkd = package->GetPrimaryHeadings();

  GtkListStore *store = NULL;
  GtkTreeIter iter;
  bool no_assets = true;

  /* Set up the storage container with the number of columns and column type */
  store = gtk_list_store_new(
      GUI_N_COLUMNS, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING,
      G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING,
      G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);

  /* Add data to the storage container. */
  gtk_list_store_append(store, &iter);
  gtk_list_store_set(store, &iter, GUI_TYPE, "blank_space_primary", GUI_SYMBOL,
                     "", GUI_COLUMN_ONE, "", -1);
  if (M->bullion_port_value_f) {
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, GUI_TYPE, "bullion_total", GUI_SYMBOL, "",
                       GUI_COLUMN_ONE, pri_h_mkd->bullion, -1);
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, GUI_TYPE, "blank_space_primary",
                       GUI_SYMBOL, "", -1);
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(
        store, &iter, GUI_TYPE, "bullion_total", GUI_SYMBOL, "", GUI_COLUMN_ONE,
        pri_h_mkd->metal, GUI_COLUMN_TWO, pri_h_mkd->ounces, GUI_COLUMN_THREE,
        pri_h_mkd->spot_price, GUI_COLUMN_FOUR, pri_h_mkd->premium,
        GUI_COLUMN_FIVE, pri_h_mkd->high, GUI_COLUMN_SIX, pri_h_mkd->low,
        GUI_COLUMN_SEVEN, pri_h_mkd->prev_closing, GUI_COLUMN_EIGHT,
        pri_h_mkd->chg_ounce, GUI_COLUMN_NINE, pri_h_mkd->gain_sym,
        GUI_COLUMN_TEN, pri_h_mkd->total, GUI_COLUMN_ELEVEN,
        pri_h_mkd->gain_per, -1);

    if (M->Gold->ounce_f) {
      gtk_list_store_append(store, &iter);
      gtk_list_store_set(store, &iter, GUI_TYPE, "bullion", GUI_SYMBOL, "gold",
                         GUI_COLUMN_ONE, pri_h_mkd->gold, GUI_COLUMN_TWO,
                         M->Gold->ounce_mrkd_ch, GUI_COLUMN_THREE,
                         M->Gold->spot_price_mrkd_ch, GUI_COLUMN_FOUR,
                         M->Gold->premium_mrkd_ch, GUI_COLUMN_FIVE,
                         M->Gold->high_metal_mrkd_ch, GUI_COLUMN_SIX,
                         M->Gold->low_metal_mrkd_ch, GUI_COLUMN_SEVEN,
                         M->Gold->prev_closing_metal_mrkd_ch, GUI_COLUMN_EIGHT,
                         M->Gold->change_ounce_mrkd_ch, GUI_COLUMN_NINE,
                         M->Gold->change_value_mrkd_ch, GUI_COLUMN_TEN,
                         M->Gold->port_value_mrkd_ch, GUI_COLUMN_ELEVEN,
                         M->Gold->change_percent_mrkd_ch, -1);
    }

    if (M->Silver->ounce_f) {
      gtk_list_store_append(store, &iter);
      gtk_list_store_set(
          store, &iter, GUI_TYPE, "bullion", GUI_SYMBOL, "silver",
          GUI_COLUMN_ONE, pri_h_mkd->silver, GUI_COLUMN_TWO,
          M->Silver->ounce_mrkd_ch, GUI_COLUMN_THREE,
          M->Silver->spot_price_mrkd_ch, GUI_COLUMN_FOUR,
          M->Silver->premium_mrkd_ch, GUI_COLUMN_FIVE,
          M->Silver->high_metal_mrkd_ch, GUI_COLUMN_SIX,
          M->Silver->low_metal_mrkd_ch, GUI_COLUMN_SEVEN,
          M->Silver->prev_closing_metal_mrkd_ch, GUI_COLUMN_EIGHT,
          M->Silver->change_ounce_mrkd_ch, GUI_COLUMN_NINE,
          M->Silver->change_value_mrkd_ch, GUI_COLUMN_TEN,
          M->Silver->port_value_mrkd_ch, GUI_COLUMN_ELEVEN,
          M->Silver->change_percent_mrkd_ch, -1);
    }

    if (M->Platinum->ounce_f) {
      gtk_list_store_append(store, &iter);
      gtk_list_store_set(
          store, &iter, GUI_TYPE, "bullion", GUI_SYMBOL, "platinum",
          GUI_COLUMN_ONE, pri_h_mkd->platinum, GUI_COLUMN_TWO,
          M->Platinum->ounce_mrkd_ch, GUI_COLUMN_THREE,
          M->Platinum->spot_price_mrkd_ch, GUI_COLUMN_FOUR,
          M->Platinum->premium_mrkd_ch, GUI_COLUMN_FIVE,
          M->Platinum->high_metal_mrkd_ch, GUI_COLUMN_SIX,
          M->Platinum->low_metal_mrkd_ch, GUI_COLUMN_SEVEN,
          M->Platinum->prev_closing_metal_mrkd_ch, GUI_COLUMN_EIGHT,
          M->Platinum->change_ounce_mrkd_ch, GUI_COLUMN_NINE,
          M->Platinum->change_value_mrkd_ch, GUI_COLUMN_TEN,
          M->Platinum->port_value_mrkd_ch, GUI_COLUMN_ELEVEN,
          M->Platinum->change_percent_mrkd_ch, -1);
    }
    if (M->Palladium->ounce_f) {
      gtk_list_store_append(store, &iter);
      gtk_list_store_set(
          store, &iter, GUI_TYPE, "bullion", GUI_SYMBOL, "palladium",
          GUI_COLUMN_ONE, pri_h_mkd->palladium, GUI_COLUMN_TWO,
          M->Palladium->ounce_mrkd_ch, GUI_COLUMN_THREE,
          M->Palladium->spot_price_mrkd_ch, GUI_COLUMN_FOUR,
          M->Palladium->premium_mrkd_ch, GUI_COLUMN_FIVE,
          M->Palladium->high_metal_mrkd_ch, GUI_COLUMN_SIX,
          M->Palladium->low_metal_mrkd_ch, GUI_COLUMN_SEVEN,
          M->Palladium->prev_closing_metal_mrkd_ch, GUI_COLUMN_EIGHT,
          M->Palladium->change_ounce_mrkd_ch, GUI_COLUMN_NINE,
          M->Palladium->change_value_mrkd_ch, GUI_COLUMN_TEN,
          M->Palladium->port_value_mrkd_ch, GUI_COLUMN_ELEVEN,
          M->Palladium->change_percent_mrkd_ch, -1);
    }

    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, GUI_TYPE, "blank_space_primary",
                       GUI_SYMBOL, "", -1);

    no_assets = false;
  }

  if (F->size) {
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, GUI_TYPE, "equity_total", GUI_SYMBOL, "",
                       GUI_COLUMN_ONE, pri_h_mkd->equity, -1);
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, GUI_TYPE, "blank_space_primary",
                       GUI_SYMBOL, "", -1);
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(
        store, &iter, GUI_TYPE, "equity_total", GUI_SYMBOL, "", GUI_COLUMN_ONE,
        pri_h_mkd->symbol, GUI_COLUMN_TWO, pri_h_mkd->shares, GUI_COLUMN_THREE,
        pri_h_mkd->price, GUI_COLUMN_FOUR, pri_h_mkd->high, GUI_COLUMN_FIVE,
        pri_h_mkd->low, GUI_COLUMN_SIX, pri_h_mkd->opening, GUI_COLUMN_SEVEN,
        pri_h_mkd->prev_closing, GUI_COLUMN_EIGHT, pri_h_mkd->chg_share,
        GUI_COLUMN_NINE, pri_h_mkd->gain_sym, GUI_COLUMN_TEN, pri_h_mkd->total,
        GUI_COLUMN_ELEVEN, pri_h_mkd->gain_per, -1);

    unsigned short c;
    unsigned short g = 0;

    /* Iterate through the equities twice.
       Add equities that have shares first.
       Add equities that have no shares second. */
    while (g < 2) {
      c = 0;
      while (c < F->size) {
        if (g == 0) {
          if (!F->Equity[c]->num_shares_stock_int) {
            c++;
            continue;
          }
        } else {
          if (F->Equity[c]->num_shares_stock_int) {
            c++;
            continue;
          }
        }

        gtk_list_store_append(store, &iter);

        gtk_list_store_set(
            store, &iter, GUI_TYPE, "equity", GUI_SYMBOL,
            F->Equity[c]->symbol_stock_ch, GUI_COLUMN_ONE,
            F->Equity[c]->symbol_stock_mrkd_ch, GUI_COLUMN_TWO,
            F->Equity[c]->num_shares_stock_mrkd_ch, GUI_COLUMN_THREE,
            F->Equity[c]->current_price_stock_mrkd_ch, GUI_COLUMN_FOUR,
            F->Equity[c]->high_stock_mrkd_ch, GUI_COLUMN_FIVE,
            F->Equity[c]->low_stock_mrkd_ch, GUI_COLUMN_SIX,
            F->Equity[c]->opening_stock_mrkd_ch, GUI_COLUMN_SEVEN,
            F->Equity[c]->prev_closing_stock_mrkd_ch, GUI_COLUMN_EIGHT,
            F->Equity[c]->change_share_stock_mrkd_ch, GUI_COLUMN_NINE,
            F->Equity[c]->change_value_mrkd_ch, GUI_COLUMN_TEN,
            F->Equity[c]->current_investment_stock_mrkd_ch, GUI_COLUMN_ELEVEN,
            F->Equity[c]->change_percent_mrkd_ch, -1);

        c++;
      }
      g++;
    }

    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, GUI_TYPE, "blank_space_primary",
                       GUI_SYMBOL, "", -1);

    no_assets = false;
  }

  if (D->cash_f || M->bullion_port_value_f || F->stock_port_value_f) {
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, GUI_TYPE, "blank_space_primary",
                       GUI_SYMBOL, "", GUI_COLUMN_ONE, pri_h_mkd->asset,
                       GUI_COLUMN_TWO, pri_h_mkd->value, GUI_COLUMN_THREE,
                       pri_h_mkd->gain_sym, GUI_COLUMN_FOUR,
                       pri_h_mkd->gain_per, -1);
  }

  if (D->cash_f) {
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, GUI_TYPE, "cash", GUI_SYMBOL, "",
                       GUI_COLUMN_ONE, pri_h_mkd->cash, GUI_COLUMN_TWO,
                       D->cash_mrkd_ch, -1);

    no_assets = false;
  }

  if (M->bullion_port_value_f) {
    gtk_list_store_append(store, &iter);

    gtk_list_store_set(store, &iter, GUI_TYPE, "bullion_total", GUI_SYMBOL, "",
                       GUI_COLUMN_ONE, pri_h_mkd->bullion, GUI_COLUMN_TWO,
                       M->bullion_port_value_mrkd_ch, GUI_COLUMN_THREE,
                       M->bullion_port_value_chg_mrkd_ch, GUI_COLUMN_FOUR,
                       M->bullion_port_value_p_chg_mrkd_ch, -1);
  }

  if (F->stock_port_value_f) {
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, GUI_TYPE, "equity_total", GUI_SYMBOL, "",
                       GUI_COLUMN_ONE, pri_h_mkd->equity, GUI_COLUMN_TWO,
                       F->stock_port_value_ch, GUI_COLUMN_THREE,
                       F->stock_port_value_chg_ch, GUI_COLUMN_FOUR,
                       F->stock_port_value_p_chg_ch, -1);
  }

  if (D->portfolio_port_value_f) {
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, GUI_TYPE, "blank_space_primary",
                       GUI_SYMBOL, "", -1);
    gtk_list_store_append(store, &iter);

    gtk_list_store_set(store, &iter, GUI_TYPE, "blank_space_primary",
                       GUI_SYMBOL, "", GUI_COLUMN_ONE, pri_h_mkd->portfolio,
                       GUI_COLUMN_TWO, D->portfolio_port_value_mrkd_ch,
                       GUI_COLUMN_THREE, D->portfolio_port_value_chg_mrkd_ch,
                       GUI_COLUMN_FOUR, D->portfolio_port_value_p_chg_mrkd_ch,
                       -1);

    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, GUI_TYPE, "blank_space_primary",
                       GUI_SYMBOL, "", -1);
  }

  if (no_assets) {
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, GUI_TYPE, "blank_space_primary",
                       GUI_SYMBOL, "", GUI_COLUMN_ONE, pri_h_mkd->no_assets,
                       -1);
  }

  /* Indicate that the default view is not displayed. */
  package->SetDefaultView(false);

  return store;
}

static GtkListStore *main_default_store(void *data) {
  /* Unpack the class package */
  portfolio_packet *package = (portfolio_packet *)data;
  metal *M = package->GetMetalClass();
  equity_folder *F = package->GetEquityFolderClass();
  meta *D = package->GetMetaClass();
  default_heading *def_h_mkd = package->GetDefaultHeadings();

  bool no_assets = true;

  GtkListStore *store = NULL;
  GtkTreeIter iter;

  /* Set up the storage container with the number of columns and column type */
  store = gtk_list_store_new(5, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING,
                             G_TYPE_STRING, G_TYPE_STRING);

  /* Add data to the storage container. */
  if (M->Gold->ounce_f || M->Silver->ounce_f || M->Platinum->ounce_f ||
      M->Palladium->ounce_f) {
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, GUI_TYPE, "blank_space_default",
                       GUI_SYMBOL, "", -1);

    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, GUI_TYPE, "bullion_total", GUI_SYMBOL, "",
                       GUI_COLUMN_ONE, def_h_mkd->bullion, -1);

    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, GUI_TYPE, "blank_space_default",
                       GUI_SYMBOL, "", GUI_COLUMN_ONE, "", -1);

    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, GUI_TYPE, "bullion_total", GUI_SYMBOL, "",
                       GUI_COLUMN_ONE, def_h_mkd->metal, GUI_COLUMN_TWO,
                       def_h_mkd->ounces, GUI_COLUMN_THREE, def_h_mkd->premium,
                       -1);
    if (M->Gold->ounce_f) {
      gtk_list_store_append(store, &iter);
      gtk_list_store_set(store, &iter, GUI_TYPE, "bullion", GUI_SYMBOL, "gold",
                         GUI_COLUMN_ONE, def_h_mkd->gold, GUI_COLUMN_TWO,
                         M->Gold->ounce_mrkd_ch, GUI_COLUMN_THREE,
                         M->Gold->premium_mrkd_ch, -1);
    }
    if (M->Silver->ounce_f) {
      gtk_list_store_append(store, &iter);
      gtk_list_store_set(store, &iter, GUI_TYPE, "bullion", GUI_SYMBOL,
                         "silver", GUI_COLUMN_ONE, def_h_mkd->silver,
                         GUI_COLUMN_TWO, M->Silver->ounce_mrkd_ch,
                         GUI_COLUMN_THREE, M->Silver->premium_mrkd_ch, -1);
    }
    if (M->Platinum->ounce_f) {
      gtk_list_store_append(store, &iter);
      gtk_list_store_set(store, &iter, GUI_TYPE, "bullion", GUI_SYMBOL,
                         "platinum", GUI_COLUMN_ONE, def_h_mkd->platinum,
                         GUI_COLUMN_TWO, M->Platinum->ounce_mrkd_ch,
                         GUI_COLUMN_THREE, M->Platinum->premium_mrkd_ch, -1);
    }
    if (M->Palladium->ounce_f) {
      gtk_list_store_append(store, &iter);
      gtk_list_store_set(store, &iter, GUI_TYPE, "bullion", GUI_SYMBOL,
                         "palladium", GUI_COLUMN_ONE, def_h_mkd->palladium,
                         GUI_COLUMN_TWO, M->Palladium->ounce_mrkd_ch,
                         GUI_COLUMN_THREE, M->Palladium->premium_mrkd_ch, -1);
    }
    no_assets = false;
  }

  if (F->size) {
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, GUI_TYPE, "blank_space_default",
                       GUI_SYMBOL, "", GUI_COLUMN_ONE, "", -1);
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, GUI_TYPE, "blank_space_default",
                       GUI_SYMBOL, "", GUI_COLUMN_ONE, "", -1);
    gtk_list_store_append(store, &iter);

    gtk_list_store_set(store, &iter, GUI_TYPE, "equity_total", GUI_SYMBOL, "",
                       GUI_COLUMN_ONE, def_h_mkd->equity, -1);
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, GUI_TYPE, "blank_space_default",
                       GUI_SYMBOL, "", GUI_COLUMN_ONE, "", -1);
    gtk_list_store_append(store, &iter);

    gtk_list_store_set(store, &iter, GUI_TYPE, "equity_total", GUI_SYMBOL, "",
                       GUI_COLUMN_ONE, def_h_mkd->symbol, GUI_COLUMN_TWO,
                       def_h_mkd->shares, -1);
    unsigned short c;
    unsigned short g = 0;

    /* Iterate through the equities twice.
       Add equities that have shares first.
       Add equities that have no shares second. */
    while (g < 2) {
      c = 0;
      while (c < F->size) {
        if (g == 0) {
          if (!F->Equity[c]->num_shares_stock_int) {
            c++;
            continue;
          }
        } else {
          if (F->Equity[c]->num_shares_stock_int) {
            c++;
            continue;
          }
        }
        gtk_list_store_append(store, &iter);
        gtk_list_store_set(store, &iter, GUI_TYPE, "equity", GUI_SYMBOL,
                           F->Equity[c]->symbol_stock_ch, GUI_COLUMN_ONE,
                           F->Equity[c]->symbol_stock_mrkd_ch, GUI_COLUMN_TWO,
                           F->Equity[c]->num_shares_stock_mrkd_ch,
                           GUI_COLUMN_THREE,
                           F->Equity[c]->security_name_mrkd_ch, -1);
        c++;
      }
      g++;
    }
    no_assets = false;
  }

  if (D->cash_f) {
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, GUI_TYPE, "blank_space_default",
                       GUI_SYMBOL, "", GUI_COLUMN_ONE, "", -1);
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, GUI_TYPE, "blank_space_default",
                       GUI_SYMBOL, "", GUI_COLUMN_ONE, "", -1);
    gtk_list_store_append(store, &iter);

    gtk_list_store_set(store, &iter, GUI_TYPE, "cash", GUI_SYMBOL, "",
                       GUI_COLUMN_ONE, def_h_mkd->cash, GUI_COLUMN_THREE,
                       D->cash_mrkd_ch, -1);
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, GUI_TYPE, "blank_space_default",
                       GUI_SYMBOL, "", GUI_COLUMN_ONE, "", -1);
    no_assets = false;
  }

  if (no_assets) {
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, GUI_TYPE, "blank_space_default",
                       GUI_SYMBOL, "", GUI_COLUMN_ONE, "", -1);
    gtk_list_store_append(store, &iter);

    gtk_list_store_set(store, &iter, GUI_TYPE, "blank_space_default",
                       GUI_SYMBOL, "", GUI_COLUMN_ONE, def_h_mkd->no_assets,
                       -1);
  }

  /* Indicate that the default view is displayed. */
  package->SetDefaultView(true);

  return store;
}

static void show_indices_labels(void *data) {
  /* Unpack the class package */
  portfolio_packet *package = (portfolio_packet *)data;
  meta *D = package->GetMetaClass();

  /* Set Revealer Bar */
  GtkWidget *revealer =
      GTK_WIDGET(gtk_builder_get_object(builder, "MainIndicesRevealer"));
  gtk_revealer_set_reveal_child(GTK_REVEALER(revealer),
                                D->index_bar_revealed_bool);
}

static void set_indices_labels(void *data) {
  /* Unpack the class package */
  portfolio_packet *package = (portfolio_packet *)data;
  meta *D = package->GetMetaClass();
  metal *M = package->GetMetalClass();
  const gchar *red_format =
      "<span foreground=\"black\">%s\n</span><span foreground=\"darkred\" "
      "size=\"small\">%s, %s</span>";
  const gchar *green_format =
      "<span foreground=\"black\">%s\n</span><span foreground=\"darkgreen\" "
      "size=\"small\">%s, %s</span>";
  const gchar *format;
  gchar *markup, *spot = malloc(1), *chg_ounce = malloc(1);

  if (D->index_dow_value_chg_f >= 0) {
    format = green_format;
  } else {
    format = red_format;
  };

  GtkWidget *label =
      GTK_WIDGET(gtk_builder_get_object(builder, "DowIndexValue"));
  markup = g_markup_printf_escaped(format, D->index_dow_value_ch,
                                   D->index_dow_value_chg_ch,
                                   D->index_dow_value_p_chg_ch);
  gtk_label_set_markup(GTK_LABEL(label), markup);
  g_free(markup);

  if (D->index_nasdaq_value_chg_f >= 0) {
    format = green_format;
  } else {
    format = red_format;
  };

  label = GTK_WIDGET(gtk_builder_get_object(builder, "NasdaqIndexValue"));
  markup = g_markup_printf_escaped(format, D->index_nasdaq_value_ch,
                                   D->index_nasdaq_value_chg_ch,
                                   D->index_nasdaq_value_p_chg_ch);
  gtk_label_set_markup(GTK_LABEL(label), markup);
  g_free(markup);

  if (D->index_sp_value_chg_f >= 0) {
    format = green_format;
  } else {
    format = red_format;
  };

  label = GTK_WIDGET(gtk_builder_get_object(builder, "SPIndexValue"));
  markup = g_markup_printf_escaped(format, D->index_sp_value_ch,
                                   D->index_sp_value_chg_ch,
                                   D->index_sp_value_p_chg_ch);
  gtk_label_set_markup(GTK_LABEL(label), markup);
  g_free(markup);

  if (D->crypto_bitcoin_value_chg_f >= 0) {
    format = green_format;
  } else {
    format = red_format;
  };

  label = GTK_WIDGET(gtk_builder_get_object(builder, "BitcoinValue"));
  markup = g_markup_printf_escaped(format, D->crypto_bitcoin_value_ch,
                                   D->crypto_bitcoin_value_chg_ch,
                                   D->crypto_bitcoin_value_p_chg_ch);
  gtk_label_set_markup(GTK_LABEL(label), markup);
  g_free(markup);

  if (M->Gold->change_ounce_f >= 0) {
    format = green_format;
  } else {
    format = red_format;
  };

  label = GTK_WIDGET(gtk_builder_get_object(builder, "GoldValue"));
  DoubleToMonStr(&spot, M->Gold->spot_price_f, 2);
  DoubleToMonStr(&chg_ounce, M->Gold->change_ounce_f, 2);
  markup = g_markup_printf_escaped(format, spot, chg_ounce,
                                   M->Gold->change_percent_raw_ch);
  gtk_label_set_markup(GTK_LABEL(label), markup);
  g_free(markup);

  if (M->Silver->change_ounce_f >= 0) {
    format = green_format;
  } else {
    format = red_format;
  };

  label = GTK_WIDGET(gtk_builder_get_object(builder, "SilverValue"));
  DoubleToMonStr(&spot, M->Silver->spot_price_f, 2);
  DoubleToMonStr(&chg_ounce, M->Silver->change_ounce_f, 2);
  markup = g_markup_printf_escaped(format, spot, chg_ounce,
                                   M->Silver->change_percent_raw_ch);
  gtk_label_set_markup(GTK_LABEL(label), markup);
  g_free(markup);

  label = GTK_WIDGET(gtk_builder_get_object(builder, "GSValue"));
  markup = g_markup_printf_escaped("<span foreground=\"black\">%s</span>",
                                   M->gold_silver_ratio_ch);
  gtk_label_set_markup(GTK_LABEL(label), markup);
  g_free(markup);

  g_free(spot);
  g_free(chg_ounce);
}

int MainPrimaryTreeview(void *data) {
  /* Reset the ProgressBar */
  GtkWidget *ProgressBar =
      GTK_WIDGET(gtk_builder_get_object(builder, "ProgressBar"));
  gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(ProgressBar), 0.0f);

  /* Show the Indices Labels */
  show_indices_labels(data);

  /* Set The Indices Labels */
  set_indices_labels(data);

  GtkListStore *store = NULL;
  GtkWidget *list = GTK_WIDGET(gtk_builder_get_object(builder, "TreeView"));

  /* Clear the current TreeView */
  main_tree_view_clr();

  /* Set the columns for the new TreeView model */
  main_set_columns(GUI_COLUMN_PRIMARY);

  /* Set up the storage container */
  store = main_primary_store(data);

  /* Add the store of data to the TreeView. */
  gtk_tree_view_set_model(GTK_TREE_VIEW(list), GTK_TREE_MODEL(store));
  g_object_unref(store);

  /* Set the header as invisible. */
  gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(list), FALSE);

  /* Remove Grid Lines. */
  gtk_tree_view_set_grid_lines(GTK_TREE_VIEW(list),
                               GTK_TREE_VIEW_GRID_LINES_NONE);

  return 0;
}

static void hide_indices_labels() {
  GtkWidget *revealer =
      GTK_WIDGET(gtk_builder_get_object(builder, "MainIndicesRevealer"));
  gtk_revealer_set_reveal_child(GTK_REVEALER(revealer), false);
}

int MainDefaultTreeview(void *data) {
  GtkListStore *store = NULL;
  GtkWidget *list = GTK_WIDGET(gtk_builder_get_object(builder, "TreeView"));

  /* Hide the Indices Labels */
  hide_indices_labels();

  /* Clear the current TreeView model */
  main_tree_view_clr();

  /* Set the columns for the new TreeView model */
  main_set_columns(GUI_COLUMN_DEFAULT);

  /* Set up the storage container */
  store = main_default_store(data);

  /* Add the store of data to the TreeView. */
  gtk_tree_view_set_model(GTK_TREE_VIEW(list), GTK_TREE_MODEL(store));
  g_object_unref(store);

  /* Set the header as invisible. */
  gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(list), FALSE);

  /* Remove Grid Lines. */
  gtk_tree_view_set_grid_lines(GTK_TREE_VIEW(list),
                               GTK_TREE_VIEW_GRID_LINES_NONE);
  return 0;
}

int MainDisplayTime() {
  GtkWidget *NewYorkTimeLabel =
      GTK_WIDGET(gtk_builder_get_object(builder, "NYTimeLabel"));
  int ny_h, ny_m;
  char time_ch[10];

  /* Get the current New York time */
  NYTime(&ny_h, &ny_m);

  /* Set the New York time label */
  snprintf(time_ch, 10, "%02d:%02d", ny_h, ny_m);
  gtk_label_set_text(GTK_LABEL(NewYorkTimeLabel), time_ch);

  return 0;
}

int MainDisplayTimeRemaining(void *data) {
  /* Unpack the package */
  portfolio_packet *package = (portfolio_packet *)data;
  if (package->IsClockDisplayed() == false)
    return 0;

  GtkWidget *CloseLabel =
      GTK_WIDGET(gtk_builder_get_object(builder, "MarketCloseLabel"));
  GtkWidget *TimeRemLabel =
      GTK_WIDGET(gtk_builder_get_object(builder, "TimeLeftLabel"));
  int h, m, s;
  char time_left_ch[10];
  bool isclosed;
  struct tm NY_Time;

  isclosed = TimeToClose(&h, &m, &s);

  if (isclosed) {
    if (package->IsHoliday()) {
      NY_Time = NYTimeComponents();
      gtk_label_set_text(GTK_LABEL(CloseLabel), WhichHoliday(NY_Time));
    } else {
      gtk_label_set_text(GTK_LABEL(CloseLabel), "Market Closed");
    }
    gtk_label_set_text(GTK_LABEL(TimeRemLabel), "");
  } else {
    gtk_label_set_text(GTK_LABEL(CloseLabel), "Market Closes In");
    snprintf(time_left_ch, 10, "%02d:%02d:%02d", h, m, s);
    gtk_label_set_text(GTK_LABEL(TimeRemLabel), time_left_ch);
    gtk_widget_set_visible(TimeRemLabel, true);
  }

  return 0;
}

int MainHideWindow() {
  GtkWidget *window = GTK_WIDGET(gtk_builder_get_object(builder, "MainWindow"));
  gtk_widget_set_visible(window, false);

  window = GTK_WIDGET(gtk_builder_get_object(builder, "AboutWindow"));
  gtk_widget_set_visible(window, false);

  window =
      GTK_WIDGET(gtk_builder_get_object(builder, "AddRemoveBullionWindow"));
  gtk_widget_set_visible(window, false);

  window = GTK_WIDGET(gtk_builder_get_object(builder, "AddRemoveCashWindow"));
  gtk_widget_set_visible(window, false);

  window = GTK_WIDGET(gtk_builder_get_object(builder, "ViewRSIWindow"));
  gtk_widget_set_visible(window, false);

  window = GTK_WIDGET(gtk_builder_get_object(builder, "AddRemoveSecurity"));
  gtk_widget_set_visible(window, false);

  window = GTK_WIDGET(gtk_builder_get_object(builder, "ShortcutWindow"));
  gtk_widget_set_visible(window, false);

  window = GTK_WIDGET(gtk_builder_get_object(builder, "ChangeApiInfoWindow"));
  gtk_widget_set_visible(window, false);

  return 0;
}

int MainDisplayClocks(void *data) {
  portfolio_packet *pkg = (portfolio_packet *)data;

  GtkWidget *revealer =
      GTK_WIDGET(gtk_builder_get_object(builder, "MainClockRevealer"));
  gtk_revealer_set_reveal_child(GTK_REVEALER(revealer),
                                pkg->IsClockDisplayed());

  if (pkg->IsClockDisplayed()) {
    GtkWidget *widget =
        GTK_WIDGET(gtk_builder_get_object(builder, "TimeLeftLabel"));
    gtk_label_set_text(GTK_LABEL(widget), "");
  }

  return 0;
}