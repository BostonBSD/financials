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

#include "../include/class.h" /* portfolio_packet, equity_folder, metal, meta  */
#include "../include/gui.h"
#include "../include/workfuncs.h"

int MainFetchBTNLabel(void *data) {
  portfolio_packet *pkg = (portfolio_packet *)data;
  GtkWidget *label = GetWidget("FetchDataBtnLabel");

  if (pkg->IsFetchingData()) {
    gtk_label_set_label(GTK_LABEL(label), "Stop Updates");
  } else {
    gtk_label_set_label(GTK_LABEL(label), "Get Data");
  }

  return 0;
}

static int main_prog_bar(void *data) {
  double *fraction = (double *)data;

  GtkWidget *ProgressBar = GetWidget("ProgressBar");
  gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(ProgressBar), *fraction);

  return 0;
}

void MainProgBar(double *fraction)
/* Because this function is accessed outside the main
   loop thread, we can only pass pointers through. */
{
  if (*fraction > 1)
    *fraction = 1;
  if (*fraction < 0)
    *fraction = 0;
  gdk_threads_add_idle(main_prog_bar, fraction);
}

int MainProgBarReset() {
  GtkWidget *ProgressBar = GetWidget("ProgressBar");
  gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(ProgressBar), 0.0f);

  return 0;
}

static int main_tree_view_clr() {
  /* Clear the main window's GtkTreeView. */
  GtkWidget *treeview = GetWidget("TreeView");
  GtkTreeViewColumn *column;
  gushort n = gtk_tree_view_get_n_columns(GTK_TREE_VIEW(treeview));

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
  GtkWidget *list = GetWidget("TreeView");

  /* A hidden column indicating the type of row [bullion, equity, blank space,
   * etc] */
  renderer = gtk_cell_renderer_text_new();
  column = gtk_tree_view_column_new_with_attributes("type_text", renderer,
                                                    "text", GUI_TYPE, NULL);
  gtk_tree_view_column_set_visible(column, false);
  gtk_tree_view_column_set_min_width(column, 0);
  gtk_tree_view_append_column(GTK_TREE_VIEW(list), column);

  /* A hidden column indicating the item symbol at that row [gold, silver, stock
   * symbol, etc] */
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
  if (M->bullion_port_value_f) {
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, GUI_TYPE, "bullion_total", GUI_SYMBOL, "",
                       GUI_COLUMN_ONE, pri_h_mkd->bullion, -1);

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
    gtk_list_store_set(store, &iter, GUI_TYPE, "bullion_total", GUI_SYMBOL, "",
                       GUI_COLUMN_ONE, def_h_mkd->bullion, -1);

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
    if (M->Palladium->ounce_f) {
      gtk_list_store_append(store, &iter);
      gtk_list_store_set(store, &iter, GUI_TYPE, "bullion", GUI_SYMBOL,
                         "palladium", GUI_COLUMN_ONE, def_h_mkd->palladium,
                         GUI_COLUMN_TWO, M->Palladium->ounce_mrkd_ch,
                         GUI_COLUMN_THREE, M->Palladium->premium_mrkd_ch, -1);
    }
    if (M->Platinum->ounce_f) {
      gtk_list_store_append(store, &iter);
      gtk_list_store_set(store, &iter, GUI_TYPE, "bullion", GUI_SYMBOL,
                         "platinum", GUI_COLUMN_ONE, def_h_mkd->platinum,
                         GUI_COLUMN_TWO, M->Platinum->ounce_mrkd_ch,
                         GUI_COLUMN_THREE, M->Platinum->premium_mrkd_ch, -1);
    }
    if (M->Silver->ounce_f) {
      gtk_list_store_append(store, &iter);
      gtk_list_store_set(store, &iter, GUI_TYPE, "bullion", GUI_SYMBOL,
                         "silver", GUI_COLUMN_ONE, def_h_mkd->silver,
                         GUI_COLUMN_TWO, M->Silver->ounce_mrkd_ch,
                         GUI_COLUMN_THREE, M->Silver->premium_mrkd_ch, -1);
    }

    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, GUI_TYPE, "blank_space_default",
                       GUI_SYMBOL, "", GUI_COLUMN_ONE, "", -1);
    no_assets = false;
  }

  if (F->size) {
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, GUI_TYPE, "equity_total", GUI_SYMBOL, "",
                       GUI_COLUMN_ONE, def_h_mkd->equity, -1);

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
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, GUI_TYPE, "blank_space_default",
                       GUI_SYMBOL, "", GUI_COLUMN_ONE, "", -1);
    no_assets = false;
  }

  if (D->cash_f) {
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

static void show_indices(void *data) {
  /* Unpack the class package */
  portfolio_packet *package = (portfolio_packet *)data;
  meta *D = package->GetMetaClass();

  /* Set Revealer Bar */
  GtkWidget *revealer = GetWidget("MainIndicesRevealer");
  gtk_revealer_set_reveal_child(GTK_REVEALER(revealer),
                                D->index_bar_revealed_bool);
}

static void set_clock_header_fonts(void *data) {
  portfolio_packet *pkg = (portfolio_packet *)data;

  PangoAttrList *attrlist = pango_attr_list_new();
  PangoFontDescription *font_desc =
      pango_font_description_from_string(pkg->meta_class->main_font_ch);

  PangoAttribute *attr = pango_attr_font_desc_new(font_desc);
  pango_attr_list_insert(attrlist, attr);

  /* Set clock heading labels with attrlist */
  GtkWidget *label = GetWidget("NewYorkTimeLabel");
  gtk_label_set_attributes((GtkLabel *)label, attrlist);
  label = GetWidget("MarketCloseLabel");
  gtk_label_set_attributes((GtkLabel *)label, attrlist);

  pango_font_description_free(font_desc);
  pango_attr_list_unref(attrlist);

  /* make sure font is set on the clock value labels */
  MainDisplayTime(pkg);
  MainDisplayTimeRemaining(pkg);
}

static void set_indice_header_fonts(void *data) {
  portfolio_packet *pkg = (portfolio_packet *)data;

  /* Set the markup on the index header labels */
  const char *format = "<span foreground='DarkSlateGrey'>%s</span>";

  GtkWidget *label = GetWidget("DowLabel");
  char *markup = g_markup_printf_escaped(format, "Dow");
  gtk_label_set_markup(GTK_LABEL(label), markup);
  g_free(markup);

  label = GetWidget("NasdaqLabel");
  markup = g_markup_printf_escaped(format, "Nasdaq");
  gtk_label_set_markup(GTK_LABEL(label), markup);
  g_free(markup);

  label = GetWidget("SPLabel");
  markup = g_markup_printf_escaped(format, "S&P 500");
  gtk_label_set_markup(GTK_LABEL(label), markup);
  g_free(markup);

  label = GetWidget("BitcoinLabel");
  markup = g_markup_printf_escaped(format, "Bitcoin");
  gtk_label_set_markup(GTK_LABEL(label), markup);
  g_free(markup);

  label = GetWidget("GoldLabel");
  markup = g_markup_printf_escaped(format, "Gold");
  gtk_label_set_markup(GTK_LABEL(label), markup);
  g_free(markup);

  label = GetWidget("SilverLabel");
  markup = g_markup_printf_escaped(format, "Silver");
  gtk_label_set_markup(GTK_LABEL(label), markup);
  g_free(markup);

  label = GetWidget("GSLabel");
  markup = g_markup_printf_escaped(format, "Gold/Silver");
  gtk_label_set_markup(GTK_LABEL(label), markup);
  g_free(markup);

  /* Set the attribute list on the header labels */

  /* Need to free/destroy attrlist. */
  PangoAttrList *attrlist = pango_attr_list_new();

  PangoFontDescription *font_desc =
      pango_font_description_from_string(pkg->meta_class->main_font_ch);

  /* attr does not take ownership of font_desc, need to destroy font_desc */
  PangoAttribute *attr = pango_attr_font_desc_new(font_desc);
  /* attrlist takes ownership of attr, do not free attr */
  pango_attr_list_insert(attrlist, attr);

  /* Index label headers */
  label = GetWidget("DowLabel");
  gtk_label_set_attributes((GtkLabel *)label, attrlist);
  label = GetWidget("NasdaqLabel");
  gtk_label_set_attributes((GtkLabel *)label, attrlist);
  label = GetWidget("SPLabel");
  gtk_label_set_attributes((GtkLabel *)label, attrlist);
  label = GetWidget("BitcoinLabel");
  gtk_label_set_attributes((GtkLabel *)label, attrlist);
  label = GetWidget("GoldLabel");
  gtk_label_set_attributes((GtkLabel *)label, attrlist);
  label = GetWidget("SilverLabel");
  gtk_label_set_attributes((GtkLabel *)label, attrlist);
  label = GetWidget("GSLabel");
  gtk_label_set_attributes((GtkLabel *)label, attrlist);

  pango_font_description_free(font_desc);
  pango_attr_list_unref(attrlist);
}

void MainSetFonts(void *data) {
  /* Set the clock label fonts */
  set_clock_header_fonts(data);

  /* Make sure the font is set on the indice header labels */
  set_indice_header_fonts(data);

  /* Make sure the treeview heading fonts are set */
  portfolio_packet *pkg = (portfolio_packet *)data;
  pkg->meta_class->ToStringsHeadings();
}

static void set_indices_labels(void *data) {
  /* Unpack the class package */
  portfolio_packet *pkg = (portfolio_packet *)data;
  meta *D = pkg->GetMetaClass();
  metal *M = pkg->GetMetalClass();
  const gchar *red_format =
      "<span foreground='black'>%s\n</span><span foreground='darkred' "
      "size='small'>%s, %s</span>";
  const gchar *green_format =
      "<span foreground='black'>%s\n</span><span foreground='darkgreen' "
      "size='small'>%s, %s</span>";
  const gchar *format;
  gchar *markup, *spot = malloc(1), *chg_ounce = malloc(1);

  if (D->index_dow_value_chg_f >= 0) {
    format = green_format;
  } else {
    format = red_format;
  };

  GtkWidget *label = GetWidget("DowIndexValue");
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

  label = GetWidget("NasdaqIndexValue");
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

  label = GetWidget("SPIndexValue");
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

  label = GetWidget("BitcoinValue");
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

  label = GetWidget("GoldValue");
  DoubleToFormattedStr(&spot, M->Gold->spot_price_f, 2, MON_STR);
  DoubleToFormattedStr(&chg_ounce, M->Gold->change_ounce_f, 2, MON_STR);
  markup = g_markup_printf_escaped(format, spot, chg_ounce,
                                   M->Gold->change_percent_raw_ch);
  gtk_label_set_markup(GTK_LABEL(label), markup);
  g_free(markup);

  if (M->Silver->change_ounce_f >= 0) {
    format = green_format;
  } else {
    format = red_format;
  };

  label = GetWidget("SilverValue");
  DoubleToFormattedStr(&spot, M->Silver->spot_price_f, 2, MON_STR);
  DoubleToFormattedStr(&chg_ounce, M->Silver->change_ounce_f, 2, MON_STR);
  markup = g_markup_printf_escaped(format, spot, chg_ounce,
                                   M->Silver->change_percent_raw_ch);
  gtk_label_set_markup(GTK_LABEL(label), markup);
  g_free(markup);

  label = GetWidget("GSValue");
  markup = g_markup_printf_escaped("<span foreground='black'>%s</span>",
                                   M->gold_silver_ratio_ch);
  gtk_label_set_markup(GTK_LABEL(label), markup);
  g_free(markup);
  g_free(spot);
  g_free(chg_ounce);

  /* Make sure the labels have the application font set. */

  /* GtkLabels need the font set manually in an attribute list. */
  /* Need to free/destroy attrlist. */
  PangoAttrList *attrlist = pango_attr_list_new();

  PangoFontDescription *font_desc =
      pango_font_description_from_string(pkg->meta_class->main_font_ch);

  /* attr does not take ownership of font_desc, need to destroy font_desc */
  PangoAttribute *attr = pango_attr_font_desc_new(font_desc);
  /* attrlist takes ownership of attr, do not free attr */
  pango_attr_list_insert(attrlist, attr);

  /* Index value labels */
  label = GetWidget("DowIndexValue");
  gtk_label_set_attributes((GtkLabel *)label, attrlist);
  label = GetWidget("NasdaqIndexValue");
  gtk_label_set_attributes((GtkLabel *)label, attrlist);
  label = GetWidget("SPIndexValue");
  gtk_label_set_attributes((GtkLabel *)label, attrlist);
  label = GetWidget("BitcoinValue");
  gtk_label_set_attributes((GtkLabel *)label, attrlist);
  label = GetWidget("GoldValue");
  gtk_label_set_attributes((GtkLabel *)label, attrlist);
  label = GetWidget("SilverValue");
  gtk_label_set_attributes((GtkLabel *)label, attrlist);
  label = GetWidget("GSValue");
  gtk_label_set_attributes((GtkLabel *)label, attrlist);

  pango_font_description_free(font_desc);
  pango_attr_list_unref(attrlist);
}

int MainPrimaryTreeview(void *data) {
  /* Show the Indices Labels */
  show_indices(data);

  /* Set The Indices Labels */
  set_indices_labels(data);

  GtkListStore *store = NULL;
  GtkWidget *list = GetWidget("TreeView");

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

static void hide_indices() {
  GtkWidget *revealer = GetWidget("MainIndicesRevealer");
  gtk_revealer_set_reveal_child(GTK_REVEALER(revealer), false);
}

int MainDefaultTreeview(void *data) {
  GtkListStore *store = NULL;
  GtkWidget *list = GetWidget("TreeView");

  /* Hide the Indices */
  hide_indices();

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

int MainDisplayTime(void *data) {
  portfolio_packet *pkg = (portfolio_packet *)data;
  GtkWidget *NewYorkTimeLabel = GetWidget("NYTimeLabel");
  int ny_h, ny_m;
  char time_ch[10];
  char *format = "<span foreground='DimGray'>%s</span>";

  /* Get the current New York time */
  NYTime(&ny_h, &ny_m);

  /* Set the New York time label */
  snprintf(time_ch, 10, "%02d:%02d", ny_h, ny_m);
  char *markup = g_markup_printf_escaped(format, time_ch);

  /* gtk_label_set_markup resets the attribute list, so we need to
     set the markup and attribute list each time this func is called.
     gtk_label_set_markup doesn't display the font tag correctly,
     gtk_label_set_attributes doesn't display the color correctly, so we need to
     do both setting markup first then the attrib list. */
  gtk_label_set_markup(GTK_LABEL(NewYorkTimeLabel), markup);
  g_free(markup);

  PangoAttrList *attrlist = pango_attr_list_new();
  PangoFontDescription *font_desc =
      pango_font_description_from_string(pkg->meta_class->main_font_ch);
  PangoAttribute *attr = pango_attr_font_desc_new(font_desc);
  pango_attr_list_insert(attrlist, attr);
  gtk_label_set_attributes((GtkLabel *)NewYorkTimeLabel, attrlist);

  pango_font_description_free(font_desc);
  pango_attr_list_unref(attrlist);

  return 0;
}

int MainDisplayTimeRemaining(void *data) {
  /* Unpack the package */
  portfolio_packet *package = (portfolio_packet *)data;

  GtkWidget *CloseLabel = GetWidget("MarketCloseLabel");
  GtkWidget *TimeRemLabel = GetWidget("TimeLeftLabel");
  int h, m, s;
  char time_left_ch[10];
  bool isclosed;
  struct tm NY_Time;
  char *format = "<span foreground='DimGray'>%s</span>";
  char *markup;

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
    markup = g_markup_printf_escaped(format, time_left_ch);
    gtk_label_set_markup(GTK_LABEL(TimeRemLabel), markup);
    g_free(markup);

    PangoAttrList *attrlist = pango_attr_list_new();
    PangoFontDescription *font_desc =
        pango_font_description_from_string(package->meta_class->main_font_ch);
    PangoAttribute *attr = pango_attr_font_desc_new(font_desc);
    pango_attr_list_insert(attrlist, attr);
    gtk_label_set_attributes((GtkLabel *)TimeRemLabel, attrlist);

    pango_font_description_free(font_desc);
    pango_attr_list_unref(attrlist);

    gtk_widget_set_visible(TimeRemLabel, true);
  }

  return 0;
}

int MainHideWindow() {
  GtkWidget *window = GetWidget("MainWindow");
  gtk_widget_set_visible(window, false);

  window = GetWidget("AboutWindow");
  gtk_widget_set_visible(window, false);

  window = GetWidget("AddRemoveBullionWindow");
  gtk_widget_set_visible(window, false);

  window = GetWidget("AddRemoveCashWindow");
  gtk_widget_set_visible(window, false);

  window = GetWidget("ViewRSIWindow");
  gtk_widget_set_visible(window, false);

  window = GetWidget("AddRemoveSecurity");
  gtk_widget_set_visible(window, false);

  window = GetWidget("ShortcutWindow");
  gtk_widget_set_visible(window, false);

  window = GetWidget("ChangeApiInfoWindow");
  gtk_widget_set_visible(window, false);

  return 0;
}

int MainDisplayClocks(void *data) {
  portfolio_packet *pkg = (portfolio_packet *)data;

  GtkWidget *revealer = GetWidget("MainClockRevealer");
  gtk_revealer_set_reveal_child(GTK_REVEALER(revealer),
                                pkg->IsClockDisplayed());
  return 0;
}