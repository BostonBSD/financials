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

gint MainFetchBTNLabel(gpointer pkg_data) {
  portfolio_packet *pkg = (portfolio_packet *)pkg_data;
  GtkWidget *label = GetWidget("FetchDataBtnLabel");

  if (pkg->IsFetchingData())
    gtk_label_set_label(GTK_LABEL(label), "Stop Updates");
  else
    gtk_label_set_label(GTK_LABEL(label), "Get Data");

  return 0;
}

static gint main_prog_bar(gpointer data) {
  gdouble *fraction = (gdouble *)data;

  GtkWidget *ProgressBar = GetWidget("ProgressBar");
  gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(ProgressBar), *fraction);

  return 0;
}

void MainProgBar(gdouble *fraction)
/* Because this function is accessed outside the main
   loop thread, we can only pass pointers through. */
{
  if (*fraction > 1)
    *fraction = 1;
  if (*fraction < 0)
    *fraction = 0;
  gdk_threads_add_idle(main_prog_bar, fraction);
}

gint MainProgBarReset() {
  GtkWidget *ProgressBar = GetWidget("ProgressBar");
  gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(ProgressBar), 0.0f);

  return 0;
}

static gint main_tree_view_clr() {
  /* Clear the main window's GtkTreeView. */
  GtkWidget *treeview = GetWidget("MainTreeView");
  return TreeViewClear(treeview);
}

static void add_hidden_column(const gchar *col_name, const gint col_num,
                              GtkWidget *treeview) {
  GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
  GtkTreeViewColumn *column = gtk_tree_view_column_new_with_attributes(
      col_name, renderer, "text", col_num, NULL);
  gtk_tree_view_column_set_visible(column, FALSE);
  gtk_tree_view_column_set_min_width(column, 0);
  gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);
}

static gint main_set_columns(gint column_type) {
  GtkWidget *list = GetWidget("MainTreeView");

  /* A hidden column indicating the type of row [bullion, equity, blank space,
   * etc] */
  add_hidden_column("type_text", MAIN_COLUMN_TYPE, list);

  /* A hidden column indicating the item symbol at that row [gold, silver, stock
   * symbol, etc] */
  add_hidden_column("symbol_text", MAIN_COLUMN_SYMBOL, list);

  for (guint8 g = MAIN_COLUMN_ONE; g <= MAIN_COLUMN_THREE; g++)
    AddColumnToTreeview("column_name", g, list);

  if (column_type == GUI_COLUMN_PRIMARY)
    /* if PRIMARY treeview */
    for (guint8 g = MAIN_COLUMN_FOUR; g < MAIN_N_COLUMNS; g++)
      AddColumnToTreeview("column_name", g, list);

  return 0;
}

static gint main_prmry_add_bul_store(bullion *B, const gchar *unmarked_name_ch,
                                     const gchar *marked_name_ch,
                                     GtkListStore *store, GtkTreeIter *iter) {
  gtk_list_store_append(store, iter);
  gtk_list_store_set(
      store, iter, MAIN_COLUMN_TYPE, "bullion", MAIN_COLUMN_SYMBOL,
      unmarked_name_ch, MAIN_COLUMN_ONE, marked_name_ch, MAIN_COLUMN_TWO,
      B->ounce_mrkd_ch, MAIN_COLUMN_THREE, B->spot_price_mrkd_ch,
      MAIN_COLUMN_FOUR, B->premium_mrkd_ch, MAIN_COLUMN_FIVE,
      B->high_metal_mrkd_ch, MAIN_COLUMN_SIX, B->low_metal_mrkd_ch,
      MAIN_COLUMN_SEVEN, B->prev_closing_metal_mrkd_ch, MAIN_COLUMN_EIGHT,
      B->change_ounce_mrkd_ch, MAIN_COLUMN_NINE, B->change_value_mrkd_ch,
      MAIN_COLUMN_TEN, B->port_value_mrkd_ch, MAIN_COLUMN_ELEVEN,
      B->change_percent_mrkd_ch, -1);
  return 0;
}

static GtkListStore *main_primary_store(portfolio_packet *pkg) {
  metal *M = pkg->GetMetalClass();
  equity_folder *F = pkg->GetEquityFolderClass();
  meta *D = pkg->GetMetaClass();
  primary_heading *pri_h_mkd = pkg->GetPrimaryHeadings();

  GtkListStore *store = NULL;
  GtkTreeIter iter;
  gboolean no_assets = TRUE;

  /* Set up the storage container with the number of columns and column type */
  store = gtk_list_store_new(MAIN_N_COLUMNS, G_TYPE_STRING, G_TYPE_STRING,
                             G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING,
                             G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING,
                             G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING,
                             G_TYPE_STRING, G_TYPE_STRING);

  /* Add data to the storage container. */
  if (M->bullion_port_value_f) {
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, MAIN_COLUMN_TYPE, "bullion_total",
                       MAIN_COLUMN_SYMBOL, "", MAIN_COLUMN_ONE,
                       pri_h_mkd->bullion, -1);

    gtk_list_store_append(store, &iter);
    gtk_list_store_set(
        store, &iter, MAIN_COLUMN_TYPE, "bullion_total", MAIN_COLUMN_SYMBOL, "",
        MAIN_COLUMN_ONE, pri_h_mkd->metal, MAIN_COLUMN_TWO, pri_h_mkd->ounces,
        MAIN_COLUMN_THREE, pri_h_mkd->price, MAIN_COLUMN_FOUR,
        pri_h_mkd->premium, MAIN_COLUMN_FIVE, pri_h_mkd->high, MAIN_COLUMN_SIX,
        pri_h_mkd->low, MAIN_COLUMN_SEVEN, pri_h_mkd->prev_closing,
        MAIN_COLUMN_EIGHT, pri_h_mkd->chg, MAIN_COLUMN_NINE,
        pri_h_mkd->gain_sym, MAIN_COLUMN_TEN, pri_h_mkd->total,
        MAIN_COLUMN_ELEVEN, pri_h_mkd->gain_per, -1);

    if (M->Gold->ounce_f)
      main_prmry_add_bul_store(M->Gold, "gold", pri_h_mkd->gold, store, &iter);

    if (M->Palladium->ounce_f)
      main_prmry_add_bul_store(M->Palladium, "palladium", pri_h_mkd->palladium,
                               store, &iter);

    if (M->Platinum->ounce_f)
      main_prmry_add_bul_store(M->Platinum, "platinum", pri_h_mkd->platinum,
                               store, &iter);

    if (M->Silver->ounce_f)
      main_prmry_add_bul_store(M->Silver, "silver", pri_h_mkd->silver, store,
                               &iter);

    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, MAIN_COLUMN_TYPE, "blank_space_primary",
                       MAIN_COLUMN_SYMBOL, "", -1);

    no_assets = FALSE;
  }

  if (F->size) {
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, MAIN_COLUMN_TYPE, "equity_total",
                       MAIN_COLUMN_SYMBOL, "", MAIN_COLUMN_ONE,
                       pri_h_mkd->equity, -1);

    gtk_list_store_append(store, &iter);
    gtk_list_store_set(
        store, &iter, MAIN_COLUMN_TYPE, "equity_total", MAIN_COLUMN_SYMBOL, "",
        MAIN_COLUMN_ONE, pri_h_mkd->symbol, MAIN_COLUMN_TWO, pri_h_mkd->shares,
        MAIN_COLUMN_THREE, pri_h_mkd->price, MAIN_COLUMN_FOUR, pri_h_mkd->high,
        MAIN_COLUMN_FIVE, pri_h_mkd->low, MAIN_COLUMN_SIX, pri_h_mkd->opening,
        MAIN_COLUMN_SEVEN, pri_h_mkd->prev_closing, MAIN_COLUMN_EIGHT,
        pri_h_mkd->chg, MAIN_COLUMN_NINE, pri_h_mkd->gain_sym, MAIN_COLUMN_TEN,
        pri_h_mkd->total, MAIN_COLUMN_ELEVEN, pri_h_mkd->gain_per, -1);

    guint8 c;
    guint8 g = 0;

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
            store, &iter, MAIN_COLUMN_TYPE, "equity", MAIN_COLUMN_SYMBOL,
            F->Equity[c]->symbol_stock_ch, MAIN_COLUMN_ONE,
            F->Equity[c]->symbol_stock_mrkd_ch, MAIN_COLUMN_TWO,
            F->Equity[c]->num_shares_stock_mrkd_ch, MAIN_COLUMN_THREE,
            F->Equity[c]->current_price_stock_mrkd_ch, MAIN_COLUMN_FOUR,
            F->Equity[c]->high_stock_mrkd_ch, MAIN_COLUMN_FIVE,
            F->Equity[c]->low_stock_mrkd_ch, MAIN_COLUMN_SIX,
            F->Equity[c]->opening_stock_mrkd_ch, MAIN_COLUMN_SEVEN,
            F->Equity[c]->prev_closing_stock_mrkd_ch, MAIN_COLUMN_EIGHT,
            F->Equity[c]->change_share_stock_mrkd_ch, MAIN_COLUMN_NINE,
            F->Equity[c]->change_value_mrkd_ch, MAIN_COLUMN_TEN,
            F->Equity[c]->current_investment_stock_mrkd_ch, MAIN_COLUMN_ELEVEN,
            F->Equity[c]->change_percent_mrkd_ch, -1);

        c++;
      }
      g++;
    }

    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, MAIN_COLUMN_TYPE, "blank_space_primary",
                       MAIN_COLUMN_SYMBOL, "", -1);

    no_assets = FALSE;
  }

  if (D->cash_f || M->bullion_port_value_f || F->stock_port_value_f) {
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, MAIN_COLUMN_TYPE, "blank_space_primary",
                       MAIN_COLUMN_SYMBOL, "", MAIN_COLUMN_ONE,
                       pri_h_mkd->asset, MAIN_COLUMN_TWO, pri_h_mkd->value,
                       MAIN_COLUMN_THREE, pri_h_mkd->gain_sym, MAIN_COLUMN_FOUR,
                       pri_h_mkd->gain_per, -1);
  }

  if (D->cash_f) {
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, MAIN_COLUMN_TYPE, "cash",
                       MAIN_COLUMN_SYMBOL, "", MAIN_COLUMN_ONE, pri_h_mkd->cash,
                       MAIN_COLUMN_TWO, D->cash_mrkd_ch, -1);

    no_assets = FALSE;
  }

  if (M->bullion_port_value_f) {
    gtk_list_store_append(store, &iter);

    gtk_list_store_set(store, &iter, MAIN_COLUMN_TYPE, "bullion_total",
                       MAIN_COLUMN_SYMBOL, "", MAIN_COLUMN_ONE,
                       pri_h_mkd->bullion, MAIN_COLUMN_TWO,
                       M->bullion_port_value_mrkd_ch, MAIN_COLUMN_THREE,
                       M->bullion_port_value_chg_mrkd_ch, MAIN_COLUMN_FOUR,
                       M->bullion_port_value_p_chg_mrkd_ch, -1);
  }

  if (F->stock_port_value_f) {
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, MAIN_COLUMN_TYPE, "equity_total",
                       MAIN_COLUMN_SYMBOL, "", MAIN_COLUMN_ONE,
                       pri_h_mkd->equity, MAIN_COLUMN_TWO,
                       F->stock_port_value_mrkd_ch, MAIN_COLUMN_THREE,
                       F->stock_port_value_chg_mrkd_ch, MAIN_COLUMN_FOUR,
                       F->stock_port_value_p_chg_mrkd_ch, -1);
  }

  if (D->portfolio_port_value_f) {
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, MAIN_COLUMN_TYPE, "blank_space_primary",
                       MAIN_COLUMN_SYMBOL, "", -1);
    gtk_list_store_append(store, &iter);

    gtk_list_store_set(store, &iter, MAIN_COLUMN_TYPE, "blank_space_primary",
                       MAIN_COLUMN_SYMBOL, "", MAIN_COLUMN_ONE,
                       pri_h_mkd->portfolio, MAIN_COLUMN_TWO,
                       D->portfolio_port_value_mrkd_ch, MAIN_COLUMN_THREE,
                       D->portfolio_port_value_chg_mrkd_ch, MAIN_COLUMN_FOUR,
                       D->portfolio_port_value_p_chg_mrkd_ch, -1);

    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, MAIN_COLUMN_TYPE, "blank_space_primary",
                       MAIN_COLUMN_SYMBOL, "", -1);
  }

  if (no_assets) {
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, MAIN_COLUMN_TYPE, "blank_space_primary",
                       MAIN_COLUMN_SYMBOL, "", MAIN_COLUMN_ONE,
                       pri_h_mkd->no_assets, -1);
  }

  /* Indicate that the default view is not displayed. */
  pkg->SetDefaultView(FALSE);

  return store;
}

static gint main_def_add_bul_store(bullion *B, const gchar *unmarked_name_ch,
                                   const gchar *marked_name_ch,
                                   GtkListStore *store, GtkTreeIter *iter) {
  gtk_list_store_append(store, iter);
  gtk_list_store_set(store, iter, MAIN_COLUMN_TYPE, "bullion",
                     MAIN_COLUMN_SYMBOL, unmarked_name_ch, MAIN_COLUMN_ONE,
                     marked_name_ch, MAIN_COLUMN_TWO, B->ounce_mrkd_ch,
                     MAIN_COLUMN_THREE, B->premium_mrkd_ch, -1);
  return 0;
}

static GtkListStore *main_default_store(portfolio_packet *pkg) {
  metal *M = pkg->GetMetalClass();
  equity_folder *F = pkg->GetEquityFolderClass();
  meta *D = pkg->GetMetaClass();
  default_heading *def_h_mkd = pkg->GetDefaultHeadings();

  gboolean no_assets = TRUE;

  GtkListStore *store = NULL;
  GtkTreeIter iter;

  /* Set up the storage container with the number of columns and column type
   */
  store = gtk_list_store_new(5, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING,
                             G_TYPE_STRING, G_TYPE_STRING);

  /* Add data to the storage container. */
  if (M->Gold->ounce_f || M->Silver->ounce_f || M->Platinum->ounce_f ||
      M->Palladium->ounce_f) {

    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, MAIN_COLUMN_TYPE, "bullion_total",
                       MAIN_COLUMN_SYMBOL, "", MAIN_COLUMN_ONE,
                       def_h_mkd->bullion, -1);

    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, MAIN_COLUMN_TYPE, "bullion_total",
                       MAIN_COLUMN_SYMBOL, "", MAIN_COLUMN_ONE,
                       def_h_mkd->metal, MAIN_COLUMN_TWO, def_h_mkd->ounces,
                       MAIN_COLUMN_THREE, def_h_mkd->premium, -1);
    if (M->Gold->ounce_f)
      main_def_add_bul_store(M->Gold, "gold", def_h_mkd->gold, store, &iter);

    if (M->Palladium->ounce_f)
      main_def_add_bul_store(M->Palladium, "palladium", def_h_mkd->palladium,
                             store, &iter);

    if (M->Platinum->ounce_f)
      main_def_add_bul_store(M->Platinum, "platinum", def_h_mkd->platinum,
                             store, &iter);

    if (M->Silver->ounce_f)
      main_def_add_bul_store(M->Silver, "silver", def_h_mkd->silver, store,
                             &iter);

    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, MAIN_COLUMN_TYPE, "blank_space_default",
                       MAIN_COLUMN_SYMBOL, "", MAIN_COLUMN_ONE, "", -1);
    no_assets = FALSE;
  }

  if (F->size) {
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, MAIN_COLUMN_TYPE, "equity_total",
                       MAIN_COLUMN_SYMBOL, "", MAIN_COLUMN_ONE,
                       def_h_mkd->equity, -1);

    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, MAIN_COLUMN_TYPE, "equity_total",
                       MAIN_COLUMN_SYMBOL, "", MAIN_COLUMN_ONE,
                       def_h_mkd->symbol, MAIN_COLUMN_TWO, def_h_mkd->shares,
                       -1);
    guint8 c;
    guint8 g = 0;

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
            store, &iter, MAIN_COLUMN_TYPE, "equity", MAIN_COLUMN_SYMBOL,
            F->Equity[c]->symbol_stock_ch, MAIN_COLUMN_ONE,
            F->Equity[c]->symbol_stock_mrkd_ch, MAIN_COLUMN_TWO,
            F->Equity[c]->num_shares_stock_mrkd_ch, MAIN_COLUMN_THREE,
            F->Equity[c]->security_name_mrkd_ch, -1);
        c++;
      }
      g++;
    }
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, MAIN_COLUMN_TYPE, "blank_space_default",
                       MAIN_COLUMN_SYMBOL, "", MAIN_COLUMN_ONE, "", -1);
    no_assets = FALSE;
  }

  if (D->cash_f) {
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, MAIN_COLUMN_TYPE, "cash",
                       MAIN_COLUMN_SYMBOL, "", MAIN_COLUMN_ONE, def_h_mkd->cash,
                       MAIN_COLUMN_THREE, D->cash_mrkd_ch, -1);
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, MAIN_COLUMN_TYPE, "blank_space_default",
                       MAIN_COLUMN_SYMBOL, "", MAIN_COLUMN_ONE, "", -1);
    no_assets = FALSE;
  }

  if (no_assets) {
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, MAIN_COLUMN_TYPE, "blank_space_default",
                       MAIN_COLUMN_SYMBOL, "", MAIN_COLUMN_ONE, "", -1);
    gtk_list_store_append(store, &iter);

    gtk_list_store_set(store, &iter, MAIN_COLUMN_TYPE, "blank_space_default",
                       MAIN_COLUMN_SYMBOL, "", MAIN_COLUMN_ONE,
                       def_h_mkd->no_assets, -1);
  }

  /* Indicate that the default view is displayed. */
  pkg->SetDefaultView(TRUE);

  return store;
}

static void show_indices(portfolio_packet *pkg) {
  meta *D = pkg->GetMetaClass();

  /* Set Revealer Bar */
  GtkWidget *revealer = GetWidget("MainIndicesRevealer");
  gtk_revealer_set_reveal_child(GTK_REVEALER(revealer),
                                D->index_bar_revealed_bool);
}

static void set_indice_value_label(const gchar *label_name, const gdouble chg_f,
                                   const gchar *value, const gchar *chg,
                                   const gchar *per_chg) {

  const gchar *red_format =
      "<span foreground='black'>%s\n</span><span foreground='darkred' "
      "size='small'>%s, %s</span>";
  const gchar *green_format =
      "<span foreground='black'>%s\n</span><span foreground='darkgreen' "
      "size='small'>%s, %s</span>";
  const gchar *fmt;

  if (chg_f >= 0)
    fmt = green_format;
  else
    fmt = red_format;

  GtkWidget *label = GetWidget(label_name);
  gchar *markup = g_markup_printf_escaped(fmt, value, chg, per_chg);
  gtk_label_set_markup(GTK_LABEL(label), markup);
  g_free(markup);
}

static void set_indices_labels(portfolio_packet *pkg) {
  meta *D = pkg->GetMetaClass();
  metal *M = pkg->GetMetalClass();

  set_indice_value_label("DowIndexValue", D->index_dow_value_chg_f,
                         D->index_dow_value_ch, D->index_dow_value_chg_ch,
                         D->index_dow_value_p_chg_ch);

  set_indice_value_label("NasdaqIndexValue", D->index_nasdaq_value_chg_f,
                         D->index_nasdaq_value_ch, D->index_nasdaq_value_chg_ch,
                         D->index_nasdaq_value_p_chg_ch);

  set_indice_value_label("SPIndexValue", D->index_sp_value_chg_f,
                         D->index_sp_value_ch, D->index_sp_value_chg_ch,
                         D->index_sp_value_p_chg_ch);

  set_indice_value_label(
      "BitcoinValue", D->crypto_bitcoin_value_chg_f, D->crypto_bitcoin_value_ch,
      D->crypto_bitcoin_value_chg_ch, D->crypto_bitcoin_value_p_chg_ch);

  gchar *spot = NULL, *chg_ounce = NULL;

  DoubleToFormattedStr(&spot, M->Gold->spot_price_f, 2, MON_STR);
  DoubleToFormattedStr(&chg_ounce, M->Gold->change_ounce_f, 2, MON_STR);
  set_indice_value_label("GoldValue", M->Gold->change_ounce_f, spot, chg_ounce,
                         M->Gold->change_percent_raw_ch);

  DoubleToFormattedStr(&spot, M->Silver->spot_price_f, 2, MON_STR);
  DoubleToFormattedStr(&chg_ounce, M->Silver->change_ounce_f, 2, MON_STR);
  set_indice_value_label("SilverValue", M->Silver->change_ounce_f, spot,
                         chg_ounce, M->Silver->change_percent_raw_ch);

  g_free(spot);
  g_free(chg_ounce);

  GtkWidget *label = GetWidget("GSValue");
  gchar *markup = g_markup_printf_escaped("<span foreground='black'>%s</span>",
                                          M->gold_silver_ratio_ch);
  gtk_label_set_markup(GTK_LABEL(label), markup);
  g_free(markup);
}

gint MainPrimaryTreeview(gpointer pkg_data) {
  portfolio_packet *pkg = (portfolio_packet *)pkg_data;
  /* Show the Indices Labels */
  show_indices(pkg);

  /* Set The Indices Labels */
  set_indices_labels(pkg);

  GtkListStore *store = NULL;
  GtkWidget *list = GetWidget("MainTreeView");

  /* Clear the current TreeView */
  main_tree_view_clr();

  /* Set the columns for the new TreeView model */
  main_set_columns(GUI_COLUMN_PRIMARY);

  /* Set up the storage container */
  store = main_primary_store(pkg);

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
  gtk_revealer_set_reveal_child(GTK_REVEALER(revealer), FALSE);
}

gint MainDefaultTreeview(gpointer pkg_data) {
  portfolio_packet *pkg = (portfolio_packet *)pkg_data;
  GtkListStore *store = NULL;
  GtkWidget *list = GetWidget("MainTreeView");

  /* Hide the Indices */
  hide_indices();

  /* Clear the current TreeView model */
  main_tree_view_clr();

  /* Set the columns for the new TreeView model */
  main_set_columns(GUI_COLUMN_DEFAULT);

  /* Set up the storage container */
  store = main_default_store(pkg);

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

static gint main_display_time(const gint ny_h, const gint ny_m) {
  GtkWidget *NYTimeValLabel = GetWidget("NYTimeLabel");
  gchar time_ch[10];

  /* Set the New York time label */
  g_snprintf(time_ch, 10, "%02d:%02d", ny_h, ny_m);
  gtk_label_set_label(GTK_LABEL(NYTimeValLabel), time_ch);
  return 0;
}

static gint main_display_time_remaining(const gint h, const gint m,
                                        const gint s, const gboolean isclosed,
                                        const gboolean holiday,
                                        const gchar *holiday_str) {
  GtkWidget *CloseLabel = GetWidget("MarketCloseLabel");
  GtkWidget *TimeRemLabel = GetWidget("TimeLeftLabel");
  gchar time_left_ch[10];

  if (isclosed) {
    if (holiday)
      gtk_label_set_label(GTK_LABEL(CloseLabel),
                          holiday_str ? holiday_str : "");
    else
      gtk_label_set_label(GTK_LABEL(CloseLabel), "Market Closed");

    gtk_label_set_label(GTK_LABEL(TimeRemLabel), "");
  } else {
    gtk_label_set_label(GTK_LABEL(CloseLabel), "Market Closes In");
    g_snprintf(time_left_ch, 10, "%02d:%02d:%02d", h, m, s);
    gtk_label_set_label(GTK_LABEL(TimeRemLabel), time_left_ch);
  }
  return 0;
}

gint MainSetClocks(gpointer pkg_data) {
  portfolio_packet *pkg = (portfolio_packet *)pkg_data;
  gint h_left, m_left, s_left, h_cur, m_cur;
  gchar *holiday_str = NULL;
  gboolean holiday;
  gboolean isclosed = GetTimeData(&holiday, &holiday_str, &h_left, &m_left,
                                  &s_left, &h_cur, &m_cur);
  pkg->SetClosed(isclosed);

  main_display_time(h_cur, m_cur);
  main_display_time_remaining(h_left, m_left, s_left, isclosed, holiday,
                              holiday_str);

  return 0;
}

gint MainHideWindows() {
  GtkWidget *window = GetWidget("MainWindow");
  gtk_widget_set_visible(window, FALSE);

  window = GetWidget("AboutWindow");
  gtk_widget_set_visible(window, FALSE);

  window = GetWidget("BullionWindow");
  gtk_widget_set_visible(window, FALSE);

  window = GetWidget("CashWindow");
  gtk_widget_set_visible(window, FALSE);

  window = GetWidget("HistoryWindow");
  gtk_widget_set_visible(window, FALSE);

  window = GetWidget("SecurityWindow");
  gtk_widget_set_visible(window, FALSE);

  window = GetWidget("HotkeysWindow");
  gtk_widget_set_visible(window, FALSE);

  window = GetWidget("ApiWindow");
  gtk_widget_set_visible(window, FALSE);

  window = GetWidget("PrefWindow");
  gtk_widget_set_visible(window, FALSE);

  return 0;
}

gint MainDisplayClocks() {
  GtkWidget *revealer = GetWidget("MainClockRevealer");
  gtk_revealer_set_reveal_child(GTK_REVEALER(revealer), TRUE);
  return 0;
}

gint MainHideClocks() {
  GtkWidget *revealer = GetWidget("MainClockRevealer");
  gtk_revealer_set_reveal_child(GTK_REVEALER(revealer), FALSE);
  return 0;
}