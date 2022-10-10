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
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include <glib-object.h>
#include <gtk/gtk.h>

#include "../include/gui_types.h"           /* enums, etc */
#include "../include/gui_globals.h"

#include "../include/class_types.h"         /* portfolio_packet, equity_folder, metal, meta */
#include "../include/workfuncs.h"

int MainFetchBTNLabel (void* data){
    bool *fetching = (bool*)data;
    GtkWidget *Button = GTK_WIDGET ( gtk_builder_get_object (builder, "FetchDataBTN") );
    
    if( *fetching == true ){
        gtk_button_set_label ( GTK_BUTTON ( Button ), "Stop Fetching Data" );
    } else {
        gtk_button_set_label ( GTK_BUTTON ( Button ), "Fetch Data" );
    }
    
    return 0;
}

static int main_prog_bar (void *data) {
    double *fraction = (double*)data;

    GtkWidget* ProgressBar = GTK_WIDGET ( gtk_builder_get_object (builder, "ProgressBar") );
    gtk_progress_bar_set_fraction ( GTK_PROGRESS_BAR( ProgressBar ), *fraction );

    return 0;
}

void MainProgBar (double *fraction) {
    if(*fraction > 1) *fraction = 1;
    if(*fraction < 0) *fraction = 0;
    gdk_threads_add_idle( main_prog_bar, fraction );
}

static int main_tree_view_clr (){
    GtkWidget* ProgressBar = GTK_WIDGET ( gtk_builder_get_object (builder, "ProgressBar") );
    gtk_progress_bar_set_text ( GTK_PROGRESS_BAR( ProgressBar ), "Financial Client" );

    /* Clear the main window's GtkTreeView. */
    GtkWidget* list = GTK_WIDGET ( gtk_builder_get_object (builder, "TreeView") );
    GtkTreeViewColumn *column;
    gint n = gtk_tree_view_get_n_columns (GTK_TREE_VIEW(list));
    
    while(n > 0){
        n--;
        column = gtk_tree_view_get_column (GTK_TREE_VIEW(list), n);
        gtk_tree_view_remove_column (GTK_TREE_VIEW(list), column);
    }

    return 0;
}

static int main_set_columns (int column_type)
{
    GtkCellRenderer *renderer;
    GtkTreeViewColumn *column;
    GtkWidget* list = GTK_WIDGET ( gtk_builder_get_object (builder, "TreeView") );

    renderer = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes("Type", renderer, "text", GUI_TYPE, "foreground", GUI_FOREGROUND_COLOR, NULL);
    gtk_tree_view_column_set_resizable (column, true);
    gtk_tree_view_column_set_visible (column, false);
    gtk_tree_view_column_set_min_width (column, 0);
    gtk_tree_view_append_column(GTK_TREE_VIEW(list), column);

    renderer = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes("Symbol", renderer, "text", GUI_SYMBOL, "foreground", GUI_FOREGROUND_COLOR, NULL);
    gtk_tree_view_column_set_resizable (column, true);
    gtk_tree_view_column_set_visible (column, true);
    gtk_tree_view_append_column(GTK_TREE_VIEW(list), column);

    renderer = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes("Shares/Ounces", renderer, "text", GUI_SHARES_OUNCES, "foreground", GUI_FOREGROUND_COLOR, NULL);
    gtk_tree_view_column_set_resizable (column, true);
    gtk_tree_view_append_column(GTK_TREE_VIEW(list), column);

    renderer = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes("Premium", renderer, "text", GUI_PREMIUM, "foreground", GUI_FOREGROUND_COLOR, NULL);
    gtk_tree_view_column_set_resizable (column, true);
    gtk_tree_view_append_column(GTK_TREE_VIEW(list), column);

    if( column_type == GUI_COLUMN_PRIMARY ){
        /* if GUI PRIMARY */
        renderer = gtk_cell_renderer_text_new ();
        column = gtk_tree_view_column_new_with_attributes("Price", renderer, "text", GUI_PRICE, "foreground", GUI_FOREGROUND_COLOR, NULL);
        gtk_tree_view_column_set_resizable (column, true);
        gtk_tree_view_append_column(GTK_TREE_VIEW(list), column);

        renderer = gtk_cell_renderer_text_new ();
        column = gtk_tree_view_column_new_with_attributes("Total/High", renderer, "text", GUI_TOTAL, "foreground", GUI_FOREGROUND_COLOR, NULL);
        gtk_tree_view_column_set_resizable (column, true);
        gtk_tree_view_append_column(GTK_TREE_VIEW(list), column);

        renderer = gtk_cell_renderer_text_new ();
        column = gtk_tree_view_column_new_with_attributes("Low", renderer, "text", GUI_EXTRA_ONE, "foreground", GUI_FOREGROUND_COLOR, NULL);
        gtk_tree_view_column_set_resizable (column, true);
        gtk_tree_view_append_column(GTK_TREE_VIEW(list), column);

        renderer = gtk_cell_renderer_text_new ();
        column = gtk_tree_view_column_new_with_attributes("Opening", renderer, "text", GUI_EXTRA_TWO, "foreground", GUI_FOREGROUND_COLOR, NULL);
        gtk_tree_view_column_set_resizable (column, true);
        gtk_tree_view_append_column(GTK_TREE_VIEW(list), column);

        renderer = gtk_cell_renderer_text_new ();
        column = gtk_tree_view_column_new_with_attributes("Previous Closing", renderer, "text", GUI_EXTRA_THREE, "foreground", GUI_FOREGROUND_COLOR, NULL);
        gtk_tree_view_column_set_resizable (column, true);
        gtk_tree_view_append_column(GTK_TREE_VIEW(list), column);

        renderer = gtk_cell_renderer_text_new ();
        column = gtk_tree_view_column_new_with_attributes("Price Change Value", renderer, "text", GUI_EXTRA_FOUR, "foreground", GUI_FOREGROUND_COLOR, NULL);
        gtk_tree_view_column_set_resizable (column, true);
        gtk_tree_view_append_column(GTK_TREE_VIEW(list), column);

        renderer = gtk_cell_renderer_text_new ();
        column = gtk_tree_view_column_new_with_attributes("Price Change Percent", renderer, "text", GUI_EXTRA_FIVE, "foreground", GUI_FOREGROUND_COLOR, NULL);
        gtk_tree_view_column_set_resizable (column, true);
        gtk_tree_view_append_column(GTK_TREE_VIEW(list), column);

        renderer = gtk_cell_renderer_text_new ();
        column = gtk_tree_view_column_new_with_attributes("Price Change Share", renderer, "text", GUI_EXTRA_SIX, "foreground", GUI_FOREGROUND_COLOR, NULL);
        gtk_tree_view_column_set_resizable (column, true);
        gtk_tree_view_append_column(GTK_TREE_VIEW(list), column);

    }

    return 0;
}

static GtkListStore * main_primary_store (void *data){
    /* Unpack the class package */
    portfolio_packet *package = (portfolio_packet*)data;
    metal *M = package->metal_chest;
    equity_folder *F = package->securities_folder;
    meta *D = package->portfolio_meta_info;

    GtkListStore *store = NULL;
    GtkTreeIter iter;
    char shares[13];

    /* Set up the storage container with the number of columns and column type */
    store = gtk_list_store_new( GUI_N_COLUMNS, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING );

    /* Add data to the storage container. */
    /* The column enum labels are generally true, but have been changed somewhat.
       They don't necessarily refer to the actual data in the column. */
    gtk_list_store_append ( store, &iter );
    gtk_list_store_set ( store, &iter, GUI_TYPE, "bullion", GUI_SYMBOL, "Bullion", -1 );
    gtk_list_store_append ( store, &iter );
    gtk_list_store_set ( store, &iter, GUI_SYMBOL, "_______", -1 );
    gtk_list_store_append ( store, &iter );
    gtk_list_store_set ( store, &iter, GUI_TYPE, "bullion", GUI_SYMBOL, "Metal", GUI_SHARES_OUNCES, "Ounces", GUI_PREMIUM, "Spot Price", GUI_PRICE, "Premium", GUI_TOTAL, "High", GUI_EXTRA_ONE, "Low", GUI_EXTRA_TWO, "Prev Closing", GUI_EXTRA_THREE, "Ch/Ounce", GUI_EXTRA_FOUR, "Gain ($)", GUI_EXTRA_FIVE, "Total", GUI_EXTRA_SIX, "Gain (%)", -1 );
    gtk_list_store_append ( store, &iter );
    if( *M->Gold->change_value_f == 0 ) {
        gtk_list_store_set ( store, &iter, GUI_TYPE, "bullion", GUI_SYMBOL, "Gold", GUI_SHARES_OUNCES, M->Gold->ounce_ch, GUI_PREMIUM, M->Gold->spot_price_ch, GUI_PRICE, M->Gold->premium_ch, GUI_TOTAL, M->Gold->high_metal_ch, GUI_EXTRA_ONE, M->Gold->low_metal_ch, GUI_EXTRA_TWO, M->Gold->prev_closing_metal_ch, GUI_EXTRA_THREE, M->Gold->change_ounce_ch, GUI_EXTRA_FOUR, M->Gold->change_value_ch, GUI_EXTRA_FIVE, M->Gold->port_value_ch, GUI_EXTRA_SIX, M->Gold->change_percent_ch, -1 );
    } else if ( *M->Gold->change_value_f > 0 ) {
        gtk_list_store_set ( store, &iter, GUI_FOREGROUND_COLOR, "Green", GUI_TYPE, "bullion", GUI_SYMBOL, "Gold", GUI_SHARES_OUNCES, M->Gold->ounce_ch, GUI_PREMIUM, M->Gold->spot_price_ch, GUI_PRICE, M->Gold->premium_ch, GUI_TOTAL, M->Gold->high_metal_ch, GUI_EXTRA_ONE, M->Gold->low_metal_ch, GUI_EXTRA_TWO, M->Gold->prev_closing_metal_ch, GUI_EXTRA_THREE, M->Gold->change_ounce_ch, GUI_EXTRA_FOUR, M->Gold->change_value_ch, GUI_EXTRA_FIVE, M->Gold->port_value_ch, GUI_EXTRA_SIX, M->Gold->change_percent_ch, -1 );
    } else {
        gtk_list_store_set ( store, &iter, GUI_FOREGROUND_COLOR, "DarkRed", GUI_TYPE, "bullion", GUI_SYMBOL, "Gold", GUI_SHARES_OUNCES, M->Gold->ounce_ch, GUI_PREMIUM, M->Gold->spot_price_ch, GUI_PRICE, M->Gold->premium_ch, GUI_TOTAL, M->Gold->high_metal_ch, GUI_EXTRA_ONE, M->Gold->low_metal_ch, GUI_EXTRA_TWO, M->Gold->prev_closing_metal_ch, GUI_EXTRA_THREE, M->Gold->change_ounce_ch, GUI_EXTRA_FOUR, M->Gold->change_value_ch, GUI_EXTRA_FIVE, M->Gold->port_value_ch, GUI_EXTRA_SIX, M->Gold->change_percent_ch, -1 );
    }
    gtk_list_store_append ( store, &iter );
    if( *M->Silver->change_value_f == 0 ) {
        gtk_list_store_set ( store, &iter, GUI_TYPE, "bullion", GUI_SYMBOL, "Silver", GUI_SHARES_OUNCES, M->Silver->ounce_ch, GUI_PREMIUM, M->Silver->spot_price_ch, GUI_PRICE, M->Silver->premium_ch, GUI_TOTAL, M->Silver->high_metal_ch, GUI_EXTRA_ONE, M->Silver->low_metal_ch, GUI_EXTRA_TWO, M->Silver->prev_closing_metal_ch, GUI_EXTRA_THREE, M->Silver->change_ounce_ch, GUI_EXTRA_FOUR, M->Silver->change_value_ch, GUI_EXTRA_FIVE, M->Silver->port_value_ch, GUI_EXTRA_SIX, M->Silver->change_percent_ch, -1 );
    } else if ( *M->Silver->change_value_f > 0 ) {
        gtk_list_store_set ( store, &iter, GUI_FOREGROUND_COLOR, "Green", GUI_TYPE, "bullion", GUI_SYMBOL, "Silver", GUI_SHARES_OUNCES, M->Silver->ounce_ch, GUI_PREMIUM, M->Silver->spot_price_ch, GUI_PRICE, M->Silver->premium_ch, GUI_TOTAL, M->Silver->high_metal_ch, GUI_EXTRA_ONE, M->Silver->low_metal_ch, GUI_EXTRA_TWO, M->Silver->prev_closing_metal_ch, GUI_EXTRA_THREE, M->Silver->change_ounce_ch, GUI_EXTRA_FOUR, M->Silver->change_value_ch, GUI_EXTRA_FIVE, M->Silver->port_value_ch, GUI_EXTRA_SIX, M->Silver->change_percent_ch, -1 );
    } else {
        gtk_list_store_set ( store, &iter, GUI_FOREGROUND_COLOR, "DarkRed", GUI_TYPE, "bullion", GUI_SYMBOL, "Silver", GUI_SHARES_OUNCES, M->Silver->ounce_ch, GUI_PREMIUM, M->Silver->spot_price_ch, GUI_PRICE, M->Silver->premium_ch, GUI_TOTAL, M->Silver->high_metal_ch, GUI_EXTRA_ONE, M->Silver->low_metal_ch, GUI_EXTRA_TWO, M->Silver->prev_closing_metal_ch, GUI_EXTRA_THREE, M->Silver->change_ounce_ch, GUI_EXTRA_FOUR, M->Silver->change_value_ch, GUI_EXTRA_FIVE, M->Silver->port_value_ch, GUI_EXTRA_SIX, M->Silver->change_percent_ch, -1 );
    }

    gtk_list_store_append ( store, &iter );
    gtk_list_store_set ( store, &iter, GUI_SYMBOL, "_______", -1 );
    gtk_list_store_append ( store, &iter );
    gtk_list_store_set ( store, &iter, GUI_TYPE, "equity_total", GUI_SYMBOL, "Equity", -1 );
    gtk_list_store_append ( store, &iter );
    gtk_list_store_set ( store, &iter, GUI_SYMBOL, "_______", -1 );
    gtk_list_store_append ( store, &iter );
    gtk_list_store_set ( store, &iter, GUI_TYPE, "equity_total", GUI_SYMBOL, "Symbol", GUI_SHARES_OUNCES, "Shares", GUI_PREMIUM, "Price", GUI_PRICE, "High", GUI_TOTAL, "Low", GUI_EXTRA_ONE, "Opening", GUI_EXTRA_TWO, "Prev Closing", GUI_EXTRA_THREE, "Ch/Share", GUI_EXTRA_FOUR, "Gain ($)", GUI_EXTRA_FIVE, "Total", GUI_EXTRA_SIX, "Gain (%)", -1 );
    
    unsigned short c = 0;
    while(c < F->size){
        snprintf(shares, 13, "%d",*F->Equity[ c ]->num_shares_stock_int);
        gtk_list_store_append ( store, &iter );

        if( *F->Equity[ c ]->change_value_f == 0 ) {
            gtk_list_store_set ( store, &iter, GUI_TYPE, "equity", GUI_SYMBOL, F->Equity[ c ]->symbol_stock_ch, GUI_SHARES_OUNCES, shares, GUI_PREMIUM, F->Equity[ c ]->current_price_stock_ch, GUI_PRICE, F->Equity[ c ]->high_stock_ch, GUI_TOTAL, F->Equity[ c ]->low_stock_ch, GUI_EXTRA_ONE, F->Equity[ c ]->opening_stock_ch, GUI_EXTRA_TWO, F->Equity[ c ]->prev_closing_stock_ch, GUI_EXTRA_THREE, F->Equity[ c ]->change_share_ch, GUI_EXTRA_FOUR, F->Equity[ c ]->change_value_ch, GUI_EXTRA_FIVE, F->Equity[ c ]->current_investment_stock_ch, GUI_EXTRA_SIX, F->Equity[ c ]->change_percent_ch, -1 );
        } else if( *F->Equity[ c ]->change_value_f > 0 ) {
            gtk_list_store_set ( store, &iter, GUI_FOREGROUND_COLOR, "Green", GUI_TYPE, "equity", GUI_SYMBOL, F->Equity[ c ]->symbol_stock_ch, GUI_SHARES_OUNCES, shares, GUI_PREMIUM, F->Equity[ c ]->current_price_stock_ch, GUI_PRICE, F->Equity[ c ]->high_stock_ch, GUI_TOTAL, F->Equity[ c ]->low_stock_ch, GUI_EXTRA_ONE, F->Equity[ c ]->opening_stock_ch, GUI_EXTRA_TWO, F->Equity[ c ]->prev_closing_stock_ch, GUI_EXTRA_THREE, F->Equity[ c ]->change_share_ch, GUI_EXTRA_FOUR, F->Equity[ c ]->change_value_ch, GUI_EXTRA_FIVE, F->Equity[ c ]->current_investment_stock_ch, GUI_EXTRA_SIX, F->Equity[ c ]->change_percent_ch, -1 );
        } else {
            gtk_list_store_set ( store, &iter, GUI_FOREGROUND_COLOR, "DarkRed", GUI_TYPE, "equity", GUI_SYMBOL, F->Equity[ c ]->symbol_stock_ch, GUI_SHARES_OUNCES, shares, GUI_PREMIUM, F->Equity[ c ]->current_price_stock_ch, GUI_PRICE, F->Equity[ c ]->high_stock_ch, GUI_TOTAL, F->Equity[ c ]->low_stock_ch, GUI_EXTRA_ONE, F->Equity[ c ]->opening_stock_ch, GUI_EXTRA_TWO, F->Equity[ c ]->prev_closing_stock_ch, GUI_EXTRA_THREE, F->Equity[ c ]->change_share_ch, GUI_EXTRA_FOUR, F->Equity[ c ]->change_value_ch, GUI_EXTRA_FIVE, F->Equity[ c ]->current_investment_stock_ch, GUI_EXTRA_SIX, F->Equity[ c ]->change_percent_ch, -1 );
        }

        c++;
    }

    gtk_list_store_append ( store, &iter );
    gtk_list_store_set ( store, &iter, GUI_SYMBOL, "_______", -1 );
    gtk_list_store_append ( store, &iter );
    gtk_list_store_set ( store, &iter, GUI_SYMBOL, "Asset", GUI_SHARES_OUNCES, "Value", GUI_PREMIUM, "Gain ($)", GUI_PRICE, "Gain (%)",-1 );
    gtk_list_store_append ( store, &iter );
    gtk_list_store_set ( store, &iter, GUI_TYPE, "cash", GUI_SYMBOL, "Cash", GUI_SHARES_OUNCES, D->cash_ch,-1 );
    
    gtk_list_store_append ( store, &iter );
    if ( *M->bullion_port_value_chg_f == 0 ){
        gtk_list_store_set ( store, &iter, GUI_TYPE, "bullion", GUI_SYMBOL, "Bullion", GUI_SHARES_OUNCES, M->bullion_port_value_ch, GUI_PREMIUM, M->bullion_port_value_chg_ch, GUI_PRICE, M->bullion_port_value_p_chg_ch, -1 );
    } else if ( *M->bullion_port_value_chg_f > 0 ) {
        gtk_list_store_set ( store, &iter, GUI_FOREGROUND_COLOR, "Green", GUI_TYPE, "bullion", GUI_SYMBOL, "Bullion", GUI_SHARES_OUNCES, M->bullion_port_value_ch, GUI_PREMIUM, M->bullion_port_value_chg_ch, GUI_PRICE, M->bullion_port_value_p_chg_ch, -1 );
    } else {
        gtk_list_store_set ( store, &iter, GUI_FOREGROUND_COLOR, "DarkRed", GUI_TYPE, "bullion", GUI_SYMBOL, "Bullion", GUI_SHARES_OUNCES, M->bullion_port_value_ch, GUI_PREMIUM, M->bullion_port_value_chg_ch, GUI_PRICE, M->bullion_port_value_p_chg_ch, -1 );
    }

    gtk_list_store_append ( store, &iter );
    if ( *F->stock_port_value_chg_f == 0 ){
        gtk_list_store_set ( store, &iter, GUI_TYPE, "equity_total", GUI_SYMBOL, "Equity", GUI_SHARES_OUNCES, F->stock_port_value_ch, GUI_PREMIUM, F->stock_port_value_chg_ch, GUI_PRICE, F->stock_port_value_p_chg_ch,-1 );
    } else if ( *F->stock_port_value_chg_f > 0 ){
        gtk_list_store_set ( store, &iter, GUI_FOREGROUND_COLOR, "Green", GUI_TYPE, "equity_total", GUI_SYMBOL, "Equity", GUI_SHARES_OUNCES, F->stock_port_value_ch, GUI_PREMIUM, F->stock_port_value_chg_ch, GUI_PRICE, F->stock_port_value_p_chg_ch,-1 );
    } else {
        gtk_list_store_set ( store, &iter, GUI_FOREGROUND_COLOR, "DarkRed", GUI_TYPE, "equity_total", GUI_SYMBOL, "Equity", GUI_SHARES_OUNCES, F->stock_port_value_ch, GUI_PREMIUM, F->stock_port_value_chg_ch, GUI_PRICE, F->stock_port_value_p_chg_ch,-1 );
    }
    
    gtk_list_store_append ( store, &iter );
    gtk_list_store_set ( store, &iter, GUI_SYMBOL, "_______", -1 );
    
    gtk_list_store_append ( store, &iter );
    if ( *D->portfolio_port_value_chg_f == 0 ){
        gtk_list_store_set ( store, &iter, GUI_SYMBOL, "Portfolio", GUI_SHARES_OUNCES, D->portfolio_port_value_ch, GUI_PREMIUM, D->portfolio_port_value_chg_ch, GUI_PRICE, D->portfolio_port_value_p_chg_ch, -1 );
    } else if ( *D->portfolio_port_value_chg_f > 0 ){
        gtk_list_store_set ( store, &iter, GUI_FOREGROUND_COLOR, "Green", GUI_SYMBOL, "Portfolio", GUI_SHARES_OUNCES, D->portfolio_port_value_ch, GUI_PREMIUM, D->portfolio_port_value_chg_ch, GUI_PRICE, D->portfolio_port_value_p_chg_ch, -1 );
    } else {
        gtk_list_store_set ( store, &iter, GUI_FOREGROUND_COLOR, "DarkRed", GUI_SYMBOL, "Portfolio", GUI_SHARES_OUNCES, D->portfolio_port_value_ch, GUI_PREMIUM, D->portfolio_port_value_chg_ch, GUI_PRICE, D->portfolio_port_value_p_chg_ch, -1 );
    }
    
    gtk_list_store_append ( store, &iter );
    gtk_list_store_set ( store, &iter, GUI_SYMBOL, "_______", -1 );
    
    return store;
}

static GtkListStore * main_default_store (void *data){
    /* Unpack the class package */
    portfolio_packet *package = (portfolio_packet*)data;
    metal *M = package->metal_chest;
    equity_folder *F = package->securities_folder;
    meta *D = package->portfolio_meta_info;

    GtkListStore *store = NULL;
    GtkTreeIter iter;
    char shares[13];

    /* Set up the storage container with the number of columns and column type */
    store = gtk_list_store_new( GUI_N_COLUMNS, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING );

    /* Add data to the storage container. */
    gtk_list_store_append ( store, &iter );
    gtk_list_store_set ( store, &iter, GUI_TYPE, "bullion", GUI_SYMBOL, "Bullion", -1 );
    gtk_list_store_append ( store, &iter );
    gtk_list_store_set ( store, &iter, GUI_SYMBOL, "_______", -1 );
    gtk_list_store_append ( store, &iter );
    gtk_list_store_set ( store, &iter, GUI_TYPE, "bullion", GUI_SYMBOL, "Metal", GUI_SHARES_OUNCES, "Ounces", GUI_PREMIUM, "Premium", -1 );
    gtk_list_store_append ( store, &iter );
    gtk_list_store_set ( store, &iter, GUI_TYPE, "bullion", GUI_SYMBOL, "Gold", GUI_SHARES_OUNCES, M->Gold->ounce_ch, GUI_PREMIUM, M->Gold->premium_ch, -1 );
    gtk_list_store_append ( store, &iter );
    gtk_list_store_set ( store, &iter, GUI_TYPE, "bullion", GUI_SYMBOL, "Silver", GUI_SHARES_OUNCES, M->Silver->ounce_ch, GUI_PREMIUM, M->Silver->premium_ch, -1 );
    gtk_list_store_append ( store, &iter );
    gtk_list_store_append ( store, &iter );
    gtk_list_store_set ( store, &iter, GUI_SYMBOL, "_______", -1 );
    gtk_list_store_append ( store, &iter );
    gtk_list_store_set ( store, &iter, GUI_TYPE, "equity_total", GUI_SYMBOL, "Equity", -1 );
    gtk_list_store_append ( store, &iter );
    gtk_list_store_set ( store, &iter, GUI_SYMBOL, "_______", -1 );
    gtk_list_store_append ( store, &iter );
    gtk_list_store_set ( store, &iter, GUI_TYPE, "equity_total", GUI_SYMBOL, "Symbol", GUI_SHARES_OUNCES, "Shares", -1 );
    
    unsigned short c = 0;
    while(c < F->size){
        snprintf(shares, 13, "%d",*F->Equity[ c ]->num_shares_stock_int);
        gtk_list_store_append ( store, &iter );
        gtk_list_store_set ( store, &iter, GUI_TYPE, "equity", GUI_SYMBOL, F->Equity[ c ]->symbol_stock_ch, GUI_SHARES_OUNCES, shares, -1 );

        c++;
    }

    gtk_list_store_append ( store, &iter );
    gtk_list_store_set ( store, &iter, GUI_SYMBOL, "_______", -1 );
    gtk_list_store_append ( store, &iter );
    gtk_list_store_set ( store, &iter, GUI_TYPE, "cash", GUI_SYMBOL, "Cash", GUI_PREMIUM, D->cash_ch, -1 );
    gtk_list_store_append ( store, &iter );
    gtk_list_store_set ( store, &iter, GUI_SYMBOL, "_______", -1 );
    
    return store;
}

int MainPrimaryTreeview (void *data)
{
    /* Reset the ProgressBar */ 
    GtkWidget* ProgressBar = GTK_WIDGET ( gtk_builder_get_object (builder, "ProgressBar") );
    gtk_progress_bar_set_fraction ( GTK_PROGRESS_BAR( ProgressBar ), 0.0f );

    GtkListStore *store = NULL;
    GtkWidget* list = GTK_WIDGET ( gtk_builder_get_object (builder, "TreeView") );

    /* Clear the current TreeView model */ 
    main_tree_view_clr ();

    /* Set the columns for the new TreeView model */
    main_set_columns ( GUI_COLUMN_PRIMARY );

    /* Set up the storage container */
    store = main_primary_store ( data );    
    
    /* Add the store of data to the list. */
    gtk_tree_view_set_model(GTK_TREE_VIEW(list), GTK_TREE_MODEL(store));
    g_object_unref( store );
   
    /* Set the list header as invisible. */
    gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(list), FALSE);

    /* Remove Grid Lines. */
    gtk_tree_view_set_grid_lines ( GTK_TREE_VIEW ( list ), GTK_TREE_VIEW_GRID_LINES_NONE );

    return 0;
}

int MainDefaultTreeview (void *data) {
    GtkListStore *store = NULL;
    GtkWidget* list = GTK_WIDGET ( gtk_builder_get_object (builder, "TreeView") );

    /* Clear the current TreeView model */ 
    main_tree_view_clr ();

    /* Set the columns for the new TreeView model */
    main_set_columns ( GUI_COLUMN_DEFAULT );

    /* Set up the storage container */
    store = main_default_store ( data );    
    
    /* Add the store of data to the list. */
    gtk_tree_view_set_model(GTK_TREE_VIEW(list), GTK_TREE_MODEL(store));
    g_object_unref( store );
   
    /* Set the list header as invisible. */
    gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(list), FALSE);

    /* Remove Grid Lines. */
    gtk_tree_view_set_grid_lines ( GTK_TREE_VIEW ( list ), GTK_TREE_VIEW_GRID_LINES_NONE );
    return 0;
}

int MainDisplayTime (){
    GtkWidget* NewYorkTimeLabel = GTK_WIDGET ( gtk_builder_get_object (builder, "NYTimeLabel") );
    GtkWidget* NewYorkDateLabel = GTK_WIDGET ( gtk_builder_get_object (builder, "NYDateLabel") );
    int ny_h, ny_m, ny_month, ny_day_month, ny_day_week, ny_year;
    char time_ch[30];

    /* Get the current New York time */
    NYTime( &ny_h, &ny_m, &ny_month, &ny_day_month, &ny_day_week, &ny_year );

    /* Set the New York date and time labels */
    snprintf( time_ch, 30, "%s, %s %d, %d", WeekDayStr( ny_day_week ), MonthNameStr( ny_month ), ny_day_month, ny_year );
    gtk_label_set_text ( GTK_LABEL ( NewYorkDateLabel ), time_ch );
    snprintf( time_ch, 30, "%02d:%02d", ny_h, ny_m );
    gtk_label_set_text ( GTK_LABEL ( NewYorkTimeLabel ), time_ch );

    return 0;
}

int MainDisplayTimeRemaining (void *data){
    /* Unpack the package */
    portfolio_packet *package = (portfolio_packet*)data;
    meta *D = package->portfolio_meta_info;

    GtkWidget* CloseLabel = GTK_WIDGET ( gtk_builder_get_object (builder, "MarketCloseLabel") );
    GtkWidget* TimeRemLabel = GTK_WIDGET ( gtk_builder_get_object (builder, "TimeLeftLabel") );
    int h, m, s;
    char time_left_ch[10];
    bool isclosed;
    struct tm NY_Time; 

    isclosed = TimeToClose ( *D->holiday_bool, &h, &m, &s );
    snprintf ( time_left_ch, 10, "%02d:%02d:%02d", h, m, s);
    gtk_label_set_text ( GTK_LABEL ( TimeRemLabel ), time_left_ch );
    if ( !isclosed ) { 
        gtk_label_set_text ( GTK_LABEL ( CloseLabel ), "Market Closes In" ); 
        return 0; 
    }
    
    if ( isclosed ) {
        if ( !(*D->holiday_bool) ) {
            gtk_label_set_text ( GTK_LABEL ( CloseLabel ), "Market Closed" );
        } else {
            NY_Time = NYTimeComponents ();
            gtk_label_set_text ( GTK_LABEL ( CloseLabel ), WhichHoliday ( NY_Time ) );
        }
    }

    return 0;
}