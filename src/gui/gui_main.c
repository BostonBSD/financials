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
#include "../include/gui_globals.h"         /* GtkBuilder *builder */

#include "../include/class.h"               /* class_destruct_portfolio_packet(), portfolio_packet, 
                                                equity_folder, metal, meta  */
#include "../include/workfuncs.h"

int MainFetchBTNLabel (void *data){
    portfolio_packet *pkg = (portfolio_packet*)data;
    GtkWidget *Button = GTK_WIDGET ( gtk_builder_get_object (builder, "FetchDataBTN") );
    
    if( pkg->IsFetchingData () == true ){
        gtk_button_set_label ( GTK_BUTTON ( Button ), "Stop Updates" );
    } else {
        gtk_button_set_label ( GTK_BUTTON ( Button ), "Get Data" );
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
    metal *M = package->GetMetalClass ();
    equity_folder *F = package->GetEquityFolderClass ();
    meta *D = package->GetMetaClass ();

    GtkListStore *store = NULL;
    GtkTreeIter iter;
    char shares[13];

    /* Set up the storage container with the number of columns and column type */
    store = gtk_list_store_new( GUI_N_COLUMNS, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING );

    /* Add data to the storage container. */
    /* The column enum labels are generally true, but have been changed somewhat.
       They don't necessarily refer to the actual data in the column. */
    gtk_list_store_append ( store, &iter );
    gtk_list_store_set ( store, &iter, GUI_TYPE, "blank_space_primary", GUI_SYMBOL, "", -1 );
    if( *M->bullion_port_value_f > 0 ){
        gtk_list_store_append ( store, &iter );
        gtk_list_store_set ( store, &iter, GUI_TYPE, "bullion_total", GUI_SYMBOL, "Bullion", -1 );
        gtk_list_store_append ( store, &iter );
        gtk_list_store_set ( store, &iter, GUI_TYPE, "blank_space_primary", GUI_SYMBOL, "_______", -1 );
        gtk_list_store_append ( store, &iter );
        gtk_list_store_set ( store, &iter, GUI_TYPE, "bullion_total", GUI_SYMBOL, "Metal", GUI_SHARES_OUNCES, "Ounces", GUI_PREMIUM, "Spot Price", GUI_PRICE, "Premium", GUI_TOTAL, "High", GUI_EXTRA_ONE, "Low", GUI_EXTRA_TWO, "Prev Closing", GUI_EXTRA_THREE, "Ch/Ounce", GUI_EXTRA_FOUR, "Gain ($)", GUI_EXTRA_FIVE, "Total", GUI_EXTRA_SIX, "Gain (%)", -1 );
        if( *M->Gold->ounce_f > 0 ){
            gtk_list_store_append ( store, &iter );
            if( *M->Gold->change_value_f == 0 ) {
                gtk_list_store_set ( store, &iter, GUI_TYPE, "bullion", GUI_SYMBOL, "Gold", GUI_SHARES_OUNCES, M->Gold->ounce_ch, GUI_PREMIUM, M->Gold->spot_price_ch, GUI_PRICE, M->Gold->premium_ch, GUI_TOTAL, M->Gold->high_metal_ch, GUI_EXTRA_ONE, M->Gold->low_metal_ch, GUI_EXTRA_TWO, M->Gold->prev_closing_metal_ch, GUI_EXTRA_THREE, M->Gold->change_ounce_ch, GUI_EXTRA_FOUR, M->Gold->change_value_ch, GUI_EXTRA_FIVE, M->Gold->port_value_ch, GUI_EXTRA_SIX, M->Gold->change_percent_ch, -1 );
            } else if ( *M->Gold->change_value_f > 0 ) {
                gtk_list_store_set ( store, &iter, GUI_FOREGROUND_COLOR, "DarkGreen", GUI_TYPE, "bullion", GUI_SYMBOL, "Gold", GUI_SHARES_OUNCES, M->Gold->ounce_ch, GUI_PREMIUM, M->Gold->spot_price_ch, GUI_PRICE, M->Gold->premium_ch, GUI_TOTAL, M->Gold->high_metal_ch, GUI_EXTRA_ONE, M->Gold->low_metal_ch, GUI_EXTRA_TWO, M->Gold->prev_closing_metal_ch, GUI_EXTRA_THREE, M->Gold->change_ounce_ch, GUI_EXTRA_FOUR, M->Gold->change_value_ch, GUI_EXTRA_FIVE, M->Gold->port_value_ch, GUI_EXTRA_SIX, M->Gold->change_percent_ch, -1 );
            } else {
                gtk_list_store_set ( store, &iter, GUI_FOREGROUND_COLOR, "DarkRed", GUI_TYPE, "bullion", GUI_SYMBOL, "Gold", GUI_SHARES_OUNCES, M->Gold->ounce_ch, GUI_PREMIUM, M->Gold->spot_price_ch, GUI_PRICE, M->Gold->premium_ch, GUI_TOTAL, M->Gold->high_metal_ch, GUI_EXTRA_ONE, M->Gold->low_metal_ch, GUI_EXTRA_TWO, M->Gold->prev_closing_metal_ch, GUI_EXTRA_THREE, M->Gold->change_ounce_ch, GUI_EXTRA_FOUR, M->Gold->change_value_ch, GUI_EXTRA_FIVE, M->Gold->port_value_ch, GUI_EXTRA_SIX, M->Gold->change_percent_ch, -1 );
            }
        }
        if( *M->Silver->ounce_f > 0 ){
            gtk_list_store_append ( store, &iter );
            if( *M->Silver->change_value_f == 0 ) {
                gtk_list_store_set ( store, &iter, GUI_TYPE, "bullion", GUI_SYMBOL, "Silver", GUI_SHARES_OUNCES, M->Silver->ounce_ch, GUI_PREMIUM, M->Silver->spot_price_ch, GUI_PRICE, M->Silver->premium_ch, GUI_TOTAL, M->Silver->high_metal_ch, GUI_EXTRA_ONE, M->Silver->low_metal_ch, GUI_EXTRA_TWO, M->Silver->prev_closing_metal_ch, GUI_EXTRA_THREE, M->Silver->change_ounce_ch, GUI_EXTRA_FOUR, M->Silver->change_value_ch, GUI_EXTRA_FIVE, M->Silver->port_value_ch, GUI_EXTRA_SIX, M->Silver->change_percent_ch, -1 );
            } else if ( *M->Silver->change_value_f > 0 ) {
                gtk_list_store_set ( store, &iter, GUI_FOREGROUND_COLOR, "DarkGreen", GUI_TYPE, "bullion", GUI_SYMBOL, "Silver", GUI_SHARES_OUNCES, M->Silver->ounce_ch, GUI_PREMIUM, M->Silver->spot_price_ch, GUI_PRICE, M->Silver->premium_ch, GUI_TOTAL, M->Silver->high_metal_ch, GUI_EXTRA_ONE, M->Silver->low_metal_ch, GUI_EXTRA_TWO, M->Silver->prev_closing_metal_ch, GUI_EXTRA_THREE, M->Silver->change_ounce_ch, GUI_EXTRA_FOUR, M->Silver->change_value_ch, GUI_EXTRA_FIVE, M->Silver->port_value_ch, GUI_EXTRA_SIX, M->Silver->change_percent_ch, -1 );
            } else {
                gtk_list_store_set ( store, &iter, GUI_FOREGROUND_COLOR, "DarkRed", GUI_TYPE, "bullion", GUI_SYMBOL, "Silver", GUI_SHARES_OUNCES, M->Silver->ounce_ch, GUI_PREMIUM, M->Silver->spot_price_ch, GUI_PRICE, M->Silver->premium_ch, GUI_TOTAL, M->Silver->high_metal_ch, GUI_EXTRA_ONE, M->Silver->low_metal_ch, GUI_EXTRA_TWO, M->Silver->prev_closing_metal_ch, GUI_EXTRA_THREE, M->Silver->change_ounce_ch, GUI_EXTRA_FOUR, M->Silver->change_value_ch, GUI_EXTRA_FIVE, M->Silver->port_value_ch, GUI_EXTRA_SIX, M->Silver->change_percent_ch, -1 );
            }
        }

        if( *M->Platinum->ounce_f > 0 ){
            gtk_list_store_append ( store, &iter );
            if( *M->Platinum->change_value_f == 0 ) {
                gtk_list_store_set ( store, &iter, GUI_TYPE, "bullion", GUI_SYMBOL, "Platinum", GUI_SHARES_OUNCES, M->Platinum->ounce_ch, GUI_PREMIUM, M->Platinum->spot_price_ch, GUI_PRICE, M->Platinum->premium_ch, GUI_TOTAL, M->Platinum->high_metal_ch, GUI_EXTRA_ONE, M->Platinum->low_metal_ch, GUI_EXTRA_TWO, M->Platinum->prev_closing_metal_ch, GUI_EXTRA_THREE, M->Platinum->change_ounce_ch, GUI_EXTRA_FOUR, M->Platinum->change_value_ch, GUI_EXTRA_FIVE, M->Platinum->port_value_ch, GUI_EXTRA_SIX, M->Platinum->change_percent_ch, -1 );
            } else if ( *M->Platinum->change_value_f > 0 ) {
                gtk_list_store_set ( store, &iter, GUI_FOREGROUND_COLOR, "DarkGreen", GUI_TYPE, "bullion", GUI_SYMBOL, "Platinum", GUI_SHARES_OUNCES, M->Platinum->ounce_ch, GUI_PREMIUM, M->Platinum->spot_price_ch, GUI_PRICE, M->Platinum->premium_ch, GUI_TOTAL, M->Platinum->high_metal_ch, GUI_EXTRA_ONE, M->Platinum->low_metal_ch, GUI_EXTRA_TWO, M->Platinum->prev_closing_metal_ch, GUI_EXTRA_THREE, M->Platinum->change_ounce_ch, GUI_EXTRA_FOUR, M->Platinum->change_value_ch, GUI_EXTRA_FIVE, M->Platinum->port_value_ch, GUI_EXTRA_SIX, M->Platinum->change_percent_ch, -1 );
            } else {
                gtk_list_store_set ( store, &iter, GUI_FOREGROUND_COLOR, "DarkRed", GUI_TYPE, "bullion", GUI_SYMBOL, "Platinum", GUI_SHARES_OUNCES, M->Platinum->ounce_ch, GUI_PREMIUM, M->Platinum->spot_price_ch, GUI_PRICE, M->Platinum->premium_ch, GUI_TOTAL, M->Platinum->high_metal_ch, GUI_EXTRA_ONE, M->Platinum->low_metal_ch, GUI_EXTRA_TWO, M->Platinum->prev_closing_metal_ch, GUI_EXTRA_THREE, M->Platinum->change_ounce_ch, GUI_EXTRA_FOUR, M->Platinum->change_value_ch, GUI_EXTRA_FIVE, M->Platinum->port_value_ch, GUI_EXTRA_SIX, M->Platinum->change_percent_ch, -1 );
            }
        }

        if( *M->Palladium->ounce_f > 0 ){
            gtk_list_store_append ( store, &iter );
            if( *M->Palladium->change_value_f == 0 ) {
                gtk_list_store_set ( store, &iter, GUI_TYPE, "bullion", GUI_SYMBOL, "Palladium", GUI_SHARES_OUNCES, M->Palladium->ounce_ch, GUI_PREMIUM, M->Palladium->spot_price_ch, GUI_PRICE, M->Palladium->premium_ch, GUI_TOTAL, M->Palladium->high_metal_ch, GUI_EXTRA_ONE, M->Palladium->low_metal_ch, GUI_EXTRA_TWO, M->Palladium->prev_closing_metal_ch, GUI_EXTRA_THREE, M->Palladium->change_ounce_ch, GUI_EXTRA_FOUR, M->Palladium->change_value_ch, GUI_EXTRA_FIVE, M->Palladium->port_value_ch, GUI_EXTRA_SIX, M->Palladium->change_percent_ch, -1 );
            } else if ( *M->Palladium->change_value_f > 0 ) {
                gtk_list_store_set ( store, &iter, GUI_FOREGROUND_COLOR, "DarkGreen", GUI_TYPE, "bullion", GUI_SYMBOL, "Palladium", GUI_SHARES_OUNCES, M->Palladium->ounce_ch, GUI_PREMIUM, M->Palladium->spot_price_ch, GUI_PRICE, M->Palladium->premium_ch, GUI_TOTAL, M->Palladium->high_metal_ch, GUI_EXTRA_ONE, M->Palladium->low_metal_ch, GUI_EXTRA_TWO, M->Palladium->prev_closing_metal_ch, GUI_EXTRA_THREE, M->Palladium->change_ounce_ch, GUI_EXTRA_FOUR, M->Palladium->change_value_ch, GUI_EXTRA_FIVE, M->Palladium->port_value_ch, GUI_EXTRA_SIX, M->Palladium->change_percent_ch, -1 );
            } else {
                gtk_list_store_set ( store, &iter, GUI_FOREGROUND_COLOR, "DarkRed", GUI_TYPE, "bullion", GUI_SYMBOL, "Palladium", GUI_SHARES_OUNCES, M->Palladium->ounce_ch, GUI_PREMIUM, M->Palladium->spot_price_ch, GUI_PRICE, M->Palladium->premium_ch, GUI_TOTAL, M->Palladium->high_metal_ch, GUI_EXTRA_ONE, M->Palladium->low_metal_ch, GUI_EXTRA_TWO, M->Palladium->prev_closing_metal_ch, GUI_EXTRA_THREE, M->Palladium->change_ounce_ch, GUI_EXTRA_FOUR, M->Palladium->change_value_ch, GUI_EXTRA_FIVE, M->Palladium->port_value_ch, GUI_EXTRA_SIX, M->Palladium->change_percent_ch, -1 );
            }
        }
        gtk_list_store_append ( store, &iter );
        gtk_list_store_set ( store, &iter, GUI_TYPE, "blank_space_primary", GUI_SYMBOL, "_______", -1 );
    }

    if( F->size > 0 ){
        gtk_list_store_append ( store, &iter );
        gtk_list_store_set ( store, &iter, GUI_TYPE, "equity_total", GUI_SYMBOL, "Equity", -1 );
        gtk_list_store_append ( store, &iter );
        gtk_list_store_set ( store, &iter, GUI_TYPE, "blank_space_primary", GUI_SYMBOL, "_______", -1 );
        gtk_list_store_append ( store, &iter );
        gtk_list_store_set ( store, &iter, GUI_TYPE, "equity_total", GUI_SYMBOL, "Symbol", GUI_SHARES_OUNCES, "Shares", GUI_PREMIUM, "Price", GUI_PRICE, "High", GUI_TOTAL, "Low", GUI_EXTRA_ONE, "Opening", GUI_EXTRA_TWO, "Prev Closing", GUI_EXTRA_THREE, "Ch/Share", GUI_EXTRA_FOUR, "Gain ($)", GUI_EXTRA_FIVE, "Total", GUI_EXTRA_SIX, "Gain (%)", -1 );

        unsigned short c = 0;
        while( c < F->size ){
            snprintf(shares, 13, "%d",*F->Equity[ c ]->num_shares_stock_int);
            gtk_list_store_append ( store, &iter );
            if( *F->Equity[ c ]->change_value_f == 0 ) {
                gtk_list_store_set ( store, &iter, GUI_TYPE, "equity", GUI_SYMBOL, F->Equity[ c ]->symbol_stock_ch, GUI_SHARES_OUNCES, shares, GUI_PREMIUM, F->Equity[ c ]->current_price_stock_ch, GUI_PRICE, F->Equity[ c ]->high_stock_ch, GUI_TOTAL, F->Equity[ c ]->low_stock_ch, GUI_EXTRA_ONE, F->Equity[ c ]->opening_stock_ch, GUI_EXTRA_TWO, F->Equity[ c ]->prev_closing_stock_ch, GUI_EXTRA_THREE, F->Equity[ c ]->change_share_ch, GUI_EXTRA_FOUR, F->Equity[ c ]->change_value_ch, GUI_EXTRA_FIVE, F->Equity[ c ]->current_investment_stock_ch, GUI_EXTRA_SIX, F->Equity[ c ]->change_percent_ch, -1 );
            } else if( *F->Equity[ c ]->change_value_f > 0 ) {
                gtk_list_store_set ( store, &iter, GUI_FOREGROUND_COLOR, "DarkGreen", GUI_TYPE, "equity", GUI_SYMBOL, F->Equity[ c ]->symbol_stock_ch, GUI_SHARES_OUNCES, shares, GUI_PREMIUM, F->Equity[ c ]->current_price_stock_ch, GUI_PRICE, F->Equity[ c ]->high_stock_ch, GUI_TOTAL, F->Equity[ c ]->low_stock_ch, GUI_EXTRA_ONE, F->Equity[ c ]->opening_stock_ch, GUI_EXTRA_TWO, F->Equity[ c ]->prev_closing_stock_ch, GUI_EXTRA_THREE, F->Equity[ c ]->change_share_ch, GUI_EXTRA_FOUR, F->Equity[ c ]->change_value_ch, GUI_EXTRA_FIVE, F->Equity[ c ]->current_investment_stock_ch, GUI_EXTRA_SIX, F->Equity[ c ]->change_percent_ch, -1 );
            } else {
                gtk_list_store_set ( store, &iter, GUI_FOREGROUND_COLOR, "DarkRed", GUI_TYPE, "equity", GUI_SYMBOL, F->Equity[ c ]->symbol_stock_ch, GUI_SHARES_OUNCES, shares, GUI_PREMIUM, F->Equity[ c ]->current_price_stock_ch, GUI_PRICE, F->Equity[ c ]->high_stock_ch, GUI_TOTAL, F->Equity[ c ]->low_stock_ch, GUI_EXTRA_ONE, F->Equity[ c ]->opening_stock_ch, GUI_EXTRA_TWO, F->Equity[ c ]->prev_closing_stock_ch, GUI_EXTRA_THREE, F->Equity[ c ]->change_share_ch, GUI_EXTRA_FOUR, F->Equity[ c ]->change_value_ch, GUI_EXTRA_FIVE, F->Equity[ c ]->current_investment_stock_ch, GUI_EXTRA_SIX, F->Equity[ c ]->change_percent_ch, -1 );
            }
            c++;
        }
        gtk_list_store_append ( store, &iter );
        gtk_list_store_set ( store, &iter, GUI_TYPE, "blank_space_primary", GUI_SYMBOL, "_______", -1 );
    }

    if( *D->cash_f > 0 || *M->bullion_port_value_f > 0 || *F->stock_port_value_f > 0 ){
        gtk_list_store_append ( store, &iter );
        gtk_list_store_set ( store, &iter, GUI_TYPE, "blank_space_primary", GUI_SYMBOL, "Asset", GUI_SHARES_OUNCES, "Value", GUI_PREMIUM, "Gain ($)", GUI_PRICE, "Gain (%)",-1 );
    }

    if( *D->cash_f > 0 ){
        gtk_list_store_append ( store, &iter );
        gtk_list_store_set ( store, &iter, GUI_TYPE, "cash", GUI_SYMBOL, "Cash", GUI_SHARES_OUNCES, D->cash_ch,-1 );
    }

    if( *M->bullion_port_value_f > 0 ){
        gtk_list_store_append ( store, &iter );
        if ( *M->bullion_port_value_chg_f == 0 ){
            gtk_list_store_set ( store, &iter, GUI_TYPE, "bullion_total", GUI_SYMBOL, "Bullion", GUI_SHARES_OUNCES, M->bullion_port_value_ch, GUI_PREMIUM, M->bullion_port_value_chg_ch, GUI_PRICE, M->bullion_port_value_p_chg_ch, -1 );
        } else if ( *M->bullion_port_value_chg_f > 0 ) {
            gtk_list_store_set ( store, &iter, GUI_FOREGROUND_COLOR, "DarkGreen", GUI_TYPE, "bullion_total", GUI_SYMBOL, "Bullion", GUI_SHARES_OUNCES, M->bullion_port_value_ch, GUI_PREMIUM, M->bullion_port_value_chg_ch, GUI_PRICE, M->bullion_port_value_p_chg_ch, -1 );
        } else {
            gtk_list_store_set ( store, &iter, GUI_FOREGROUND_COLOR, "DarkRed", GUI_TYPE, "bullion_total", GUI_SYMBOL, "Bullion", GUI_SHARES_OUNCES, M->bullion_port_value_ch, GUI_PREMIUM, M->bullion_port_value_chg_ch, GUI_PRICE, M->bullion_port_value_p_chg_ch, -1 );
        }
    }

    if( *F->stock_port_value_f > 0 ){
        gtk_list_store_append ( store, &iter );
        if ( *F->stock_port_value_chg_f == 0 ){
            gtk_list_store_set ( store, &iter, GUI_TYPE, "equity_total", GUI_SYMBOL, "Equity", GUI_SHARES_OUNCES, F->stock_port_value_ch, GUI_PREMIUM, F->stock_port_value_chg_ch, GUI_PRICE, F->stock_port_value_p_chg_ch,-1 );
        } else if ( *F->stock_port_value_chg_f > 0 ){
            gtk_list_store_set ( store, &iter, GUI_FOREGROUND_COLOR, "DarkGreen", GUI_TYPE, "equity_total", GUI_SYMBOL, "Equity", GUI_SHARES_OUNCES, F->stock_port_value_ch, GUI_PREMIUM, F->stock_port_value_chg_ch, GUI_PRICE, F->stock_port_value_p_chg_ch,-1 );
        } else {
            gtk_list_store_set ( store, &iter, GUI_FOREGROUND_COLOR, "DarkRed", GUI_TYPE, "equity_total", GUI_SYMBOL, "Equity", GUI_SHARES_OUNCES, F->stock_port_value_ch, GUI_PREMIUM, F->stock_port_value_chg_ch, GUI_PRICE, F->stock_port_value_p_chg_ch,-1 );
        }
    }    
    
    if( *D->portfolio_port_value_f > 0 ){
        gtk_list_store_append ( store, &iter );
        gtk_list_store_set ( store, &iter, GUI_TYPE, "blank_space_primary", GUI_SYMBOL, "_______", -1 );
        gtk_list_store_append ( store, &iter );
        if ( *D->portfolio_port_value_chg_f == 0 ){
            gtk_list_store_set ( store, &iter, GUI_TYPE, "blank_space_primary", GUI_SYMBOL, "Portfolio", GUI_SHARES_OUNCES, D->portfolio_port_value_ch, GUI_PREMIUM, D->portfolio_port_value_chg_ch, GUI_PRICE, D->portfolio_port_value_p_chg_ch, -1 );
        } else if ( *D->portfolio_port_value_chg_f > 0 ){
            gtk_list_store_set ( store, &iter, GUI_FOREGROUND_COLOR, "DarkGreen", GUI_TYPE, "blank_space_primary", GUI_SYMBOL, "Portfolio", GUI_SHARES_OUNCES, D->portfolio_port_value_ch, GUI_PREMIUM, D->portfolio_port_value_chg_ch, GUI_PRICE, D->portfolio_port_value_p_chg_ch, -1 );
        } else {
            gtk_list_store_set ( store, &iter, GUI_FOREGROUND_COLOR, "DarkRed", GUI_TYPE, "blank_space_primary", GUI_SYMBOL, "Portfolio", GUI_SHARES_OUNCES, D->portfolio_port_value_ch, GUI_PREMIUM, D->portfolio_port_value_chg_ch, GUI_PRICE, D->portfolio_port_value_p_chg_ch, -1 );
        }
        gtk_list_store_append ( store, &iter );
        gtk_list_store_set ( store, &iter, GUI_TYPE, "blank_space_primary", GUI_SYMBOL, "_______", -1 );
    } else {
        gtk_list_store_append ( store, &iter );
        gtk_list_store_set ( store, &iter, GUI_TYPE, "blank_space_primary", GUI_SYMBOL, "Portfolio has no assets.", -1 );
    }

    return store;
}

static GtkListStore * main_default_store (void *data){
    /* Unpack the class package */
    portfolio_packet *package = (portfolio_packet*)data;
    metal *M = package->GetMetalClass ();
    equity_folder *F = package->GetEquityFolderClass ();
    meta *D = package->GetMetaClass ();
    symbol_name_map *sn_map = package->GetSymNameMap ();
    bool no_assets = true;

    GtkListStore *store = NULL;
    GtkTreeIter iter;
    char shares[13], *stock_name = NULL;

    /* Set up the storage container with the number of columns and column type */
    store = gtk_list_store_new( GUI_N_COLUMNS, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING );

    /* Add data to the storage container. */
    if ( *M->Gold->ounce_f > 0 || *M->Silver->ounce_f > 0 || *M->Platinum->ounce_f > 0 || *M->Palladium->ounce_f > 0 ) {
        gtk_list_store_append ( store, &iter );
        gtk_list_store_set ( store, &iter, GUI_TYPE, "blank_space_default", GUI_SYMBOL, "", -1 );
        gtk_list_store_append ( store, &iter );
        gtk_list_store_set ( store, &iter, GUI_TYPE, "bullion_total", GUI_SYMBOL, "Bullion", -1 );
        gtk_list_store_append ( store, &iter );
        gtk_list_store_set ( store, &iter, GUI_TYPE, "blank_space_default", GUI_SYMBOL, "_______", -1 );
        gtk_list_store_append ( store, &iter );
        gtk_list_store_set ( store, &iter, GUI_TYPE, "bullion_total", GUI_SYMBOL, "Metal", GUI_SHARES_OUNCES, "Ounces", GUI_PREMIUM, "Premium", -1 );
        if ( *M->Gold->ounce_f > 0 ) {
            gtk_list_store_append ( store, &iter );
            gtk_list_store_set ( store, &iter, GUI_TYPE, "bullion", GUI_SYMBOL, "Gold", GUI_SHARES_OUNCES, M->Gold->ounce_ch, GUI_PREMIUM, M->Gold->premium_ch, -1 );
        }
        if ( *M->Silver->ounce_f > 0 ) {
            gtk_list_store_append ( store, &iter );
            gtk_list_store_set ( store, &iter, GUI_TYPE, "bullion", GUI_SYMBOL, "Silver", GUI_SHARES_OUNCES, M->Silver->ounce_ch, GUI_PREMIUM, M->Silver->premium_ch, -1 );
        }
        if ( *M->Platinum->ounce_f > 0 ) {
            gtk_list_store_append ( store, &iter );
            gtk_list_store_set ( store, &iter, GUI_TYPE, "bullion", GUI_SYMBOL, "Platinum", GUI_SHARES_OUNCES, M->Platinum->ounce_ch, GUI_PREMIUM, M->Platinum->premium_ch, -1 );
        }
        if ( *M->Palladium->ounce_f > 0 ) {
            gtk_list_store_append ( store, &iter );
            gtk_list_store_set ( store, &iter, GUI_TYPE, "bullion", GUI_SYMBOL, "Palladium", GUI_SHARES_OUNCES, M->Palladium->ounce_ch, GUI_PREMIUM, M->Palladium->premium_ch, -1 );
        }
        no_assets = false;
    }
    
    if ( F->size > 0 ) {
        gtk_list_store_append ( store, &iter );
        gtk_list_store_set ( store, &iter, GUI_TYPE, "blank_space_default", GUI_SYMBOL, "", -1 );
        gtk_list_store_append ( store, &iter );
        gtk_list_store_set ( store, &iter, GUI_TYPE, "blank_space_default", GUI_SYMBOL, "_______", -1 );
        gtk_list_store_append ( store, &iter );
        gtk_list_store_set ( store, &iter, GUI_TYPE, "equity_total", GUI_SYMBOL, "Equity", -1 );
        gtk_list_store_append ( store, &iter );
        gtk_list_store_set ( store, &iter, GUI_TYPE, "blank_space_default", GUI_SYMBOL, "_______", -1 );
        gtk_list_store_append ( store, &iter );
        gtk_list_store_set ( store, &iter, GUI_TYPE, "equity_total", GUI_SYMBOL, "Symbol", GUI_SHARES_OUNCES, "Shares", -1 );

        unsigned short c = 0;
        while(c < F->size){
            snprintf(shares, 13, "%d",*F->Equity[ c ]->num_shares_stock_int);
            gtk_list_store_append ( store, &iter );
            if( sn_map ){
                stock_name = GetSecurityName( F->Equity[ c ]->symbol_stock_ch, sn_map );
                gtk_list_store_set ( store, &iter, GUI_TYPE, "equity", GUI_SYMBOL, F->Equity[ c ]->symbol_stock_ch, GUI_SHARES_OUNCES, shares, GUI_PREMIUM, stock_name ? stock_name : "", -1 );
                if( stock_name ) free ( stock_name );
            } else {
                gtk_list_store_set ( store, &iter, GUI_TYPE, "equity", GUI_SYMBOL, F->Equity[ c ]->symbol_stock_ch, GUI_SHARES_OUNCES, shares, -1 );
            }
            c++;
        }
        no_assets = false;
    }

    if ( *D->cash_f > 0 ) {
        gtk_list_store_append ( store, &iter );
        gtk_list_store_set ( store, &iter, GUI_TYPE, "blank_space_default", GUI_SYMBOL, "", -1 );
        gtk_list_store_append ( store, &iter );
        gtk_list_store_set ( store, &iter, GUI_TYPE, "blank_space_default", GUI_SYMBOL, "_______", -1 );
        gtk_list_store_append ( store, &iter );
        gtk_list_store_set ( store, &iter, GUI_TYPE, "cash", GUI_SYMBOL, "Cash", GUI_PREMIUM, D->cash_ch, -1 );
        gtk_list_store_append ( store, &iter );
        gtk_list_store_set ( store, &iter, GUI_TYPE, "blank_space_default", GUI_SYMBOL, "_______", -1 );
        no_assets = false;
    }

    if ( no_assets ){
        gtk_list_store_append ( store, &iter );
        gtk_list_store_set ( store, &iter, GUI_TYPE, "blank_space_default", GUI_SYMBOL, "", -1 );
        gtk_list_store_append ( store, &iter );
        gtk_list_store_set ( store, &iter, GUI_TYPE, "blank_space_default", GUI_SYMBOL, "Portfolio has no assets.", -1 );
    }

    return store;
}

static void show_indices_labels (void *data){
    /* Unpack the class package */
    portfolio_packet *package = (portfolio_packet*)data;
    meta *D = package->GetMetaClass ();

    GtkWidget* indexframe = GTK_WIDGET ( gtk_builder_get_object (builder, "IndexFrame") );
    gtk_widget_set_visible ( indexframe, true );

    /* Set Expander Bar */
    GtkWidget* expanderbar = GTK_WIDGET ( gtk_builder_get_object (builder, "IndicesExpander") );
    gtk_expander_set_expanded ( GTK_EXPANDER ( expanderbar ), *D->index_bar_expanded_bool );
}

static void set_indices_labels (void *data){
    /* Unpack the class package */
    portfolio_packet *package = (portfolio_packet*)data;
    meta *D = package->GetMetaClass ();
    metal *M = package->GetMetalClass ();
    const char *red_format = "<span foreground=\"black\">%s\n</span><span foreground=\"darkred\" size=\"small\">%s, %s</span>";
    const char *green_format = "<span foreground=\"black\">%s\n</span><span foreground=\"darkgreen\" size=\"small\">%s, %s</span>";
    const char *format;
    char *markup;

    if (*D->index_dow_value_chg_f >= 0) {
        format = green_format;
    } else {
        format = red_format;
    };

    GtkWidget* label = GTK_WIDGET ( gtk_builder_get_object (builder, "DowIndexValue") );
    markup = g_markup_printf_escaped ( format, D->index_dow_value_ch, D->index_dow_value_chg_ch, D->index_dow_value_p_chg_ch );
    gtk_label_set_markup ( GTK_LABEL (label), markup );
    g_free (markup);

    if (*D->index_nasdaq_value_chg_f >= 0) {
        format = green_format;
    } else {
        format = red_format;
    };

    label = GTK_WIDGET ( gtk_builder_get_object (builder, "NasdaqIndexValue") );
    markup = g_markup_printf_escaped (format, D->index_nasdaq_value_ch, D->index_nasdaq_value_chg_ch, D->index_nasdaq_value_p_chg_ch );
    gtk_label_set_markup ( GTK_LABEL (label), markup );
    g_free (markup);

    if (*D->index_sp_value_chg_f >= 0) {
        format = green_format;
    } else {
        format = red_format;
    };

    label = GTK_WIDGET ( gtk_builder_get_object (builder, "SPIndexValue") );
    markup = g_markup_printf_escaped ( format, D->index_sp_value_ch, D->index_sp_value_chg_ch, D->index_sp_value_p_chg_ch );
    gtk_label_set_markup ( GTK_LABEL (label), markup );
    g_free (markup);

    if (*D->crypto_bitcoin_value_chg_f >= 0) {
        format = green_format;
    } else {
        format = red_format;
    };

    label = GTK_WIDGET ( gtk_builder_get_object (builder, "BitcoinValue") );
    markup = g_markup_printf_escaped ( format, D->crypto_bitcoin_value_ch, D->crypto_bitcoin_value_chg_ch, D->crypto_bitcoin_value_p_chg_ch );
    gtk_label_set_markup ( GTK_LABEL (label), markup );
    g_free (markup);

    if (*M->Gold->change_ounce_f >= 0) {
        format = green_format;
    } else {
        format = red_format;
    };

    label = GTK_WIDGET ( gtk_builder_get_object (builder, "GoldValue") );
    markup = g_markup_printf_escaped ( format, M->Gold->spot_price_ch, M->Gold->change_ounce_ch, M->Gold->change_percent_raw_ch );
    gtk_label_set_markup ( GTK_LABEL (label), markup );
    g_free (markup);

    if (*M->Silver->change_ounce_f >= 0) {
        format = green_format;
    } else {
        format = red_format;
    };

    label = GTK_WIDGET ( gtk_builder_get_object (builder, "SilverValue") );
    markup = g_markup_printf_escaped ( format, M->Silver->spot_price_ch, M->Silver->change_ounce_ch, M->Silver->change_percent_raw_ch );
    gtk_label_set_markup ( GTK_LABEL (label), markup );
    g_free (markup);

    label = GTK_WIDGET ( gtk_builder_get_object (builder, "GSValue") );
    markup = g_markup_printf_escaped ( "<span foreground=\"black\">%s</span>", M->gold_silver_ratio_ch );
    gtk_label_set_markup ( GTK_LABEL (label), markup );
    g_free (markup);
}

int MainPrimaryTreeview (void *data)
{
    /* Reset the ProgressBar */ 
    GtkWidget* ProgressBar = GTK_WIDGET ( gtk_builder_get_object (builder, "ProgressBar") );
    gtk_progress_bar_set_fraction ( GTK_PROGRESS_BAR( ProgressBar ), 0.0f );

    /* Show the Indices Labels */ 
    show_indices_labels ( data );

    /* Set The Indices Labels */
    set_indices_labels ( data );

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

static void hide_indices_labels (){
    GtkWidget* indexframe = GTK_WIDGET ( gtk_builder_get_object (builder, "IndexFrame") );
    gtk_widget_set_visible ( indexframe, false );
}

int MainDefaultTreeview (void *data) {
    GtkListStore *store = NULL;
    GtkWidget* list = GTK_WIDGET ( gtk_builder_get_object (builder, "TreeView") );

    /* Hide the Indices Labels */
    hide_indices_labels ();

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
    if( package->IsClockDisplayed () == false ) return 0;

    GtkWidget* CloseLabel = GTK_WIDGET ( gtk_builder_get_object (builder, "MarketCloseLabel") );
    GtkWidget* TimeRemLabel = GTK_WIDGET ( gtk_builder_get_object (builder, "TimeLeftLabel") );
    int h, m, s;
    char time_left_ch[10];
    bool isclosed;
    struct tm NY_Time;

    isclosed = TimeToClose ( package->IsHoliday (), &h, &m, &s );
    
    if ( isclosed ) {
        if ( package->IsHoliday () ) {
            NY_Time = NYTimeComponents ();
            gtk_label_set_text ( GTK_LABEL ( CloseLabel ), WhichHoliday ( NY_Time ) );
        } else {
            gtk_label_set_text ( GTK_LABEL ( CloseLabel ), "Market Closed" );
        }
        gtk_widget_set_visible ( TimeRemLabel, false );
    } else {
        gtk_label_set_text ( GTK_LABEL ( CloseLabel ), "Market Closes In" );
        snprintf ( time_left_ch, 10, "%02d:%02d:%02d", h, m, s);
        gtk_label_set_text ( GTK_LABEL ( TimeRemLabel ), time_left_ch );
        gtk_widget_set_visible ( TimeRemLabel, true );
    }

    return 0;
}

int MainHideWindow (){
    GtkWidget* window = GTK_WIDGET ( gtk_builder_get_object (builder, "MainWindow") );
    gtk_widget_set_visible ( window, false );

    window = GTK_WIDGET ( gtk_builder_get_object (builder, "AboutWindow") );
    gtk_widget_set_visible ( window, false );

    window = GTK_WIDGET ( gtk_builder_get_object (builder, "AddRemoveBullionWindow") );
    gtk_widget_set_visible ( window, false );

    window = GTK_WIDGET ( gtk_builder_get_object (builder, "AddRemoveCashWindow") );
    gtk_widget_set_visible ( window, false );

    window = GTK_WIDGET ( gtk_builder_get_object (builder, "ViewRSIWindow") );
    gtk_widget_set_visible ( window, false );

    window = GTK_WIDGET ( gtk_builder_get_object (builder, "AddRemoveSecurity") );
    gtk_widget_set_visible ( window, false );

    window = GTK_WIDGET ( gtk_builder_get_object (builder, "ShortcutWindow") );
    gtk_widget_set_visible ( window, false );

    window = GTK_WIDGET ( gtk_builder_get_object (builder, "ChangeApiInfoWindow") );
    gtk_widget_set_visible ( window, false );

    return 0;
}

int MainDisplayClocks (void *data){
    portfolio_packet *pkg = (portfolio_packet*)data;

    if ( pkg->IsClockDisplayed () ){
        GtkWidget* widget = GTK_WIDGET ( gtk_builder_get_object (builder, "MainClockGrid") );
        gtk_widget_set_margin_bottom ( widget, 0 );
        widget = GTK_WIDGET ( gtk_builder_get_object (builder, "NewYorkTimeLabel") );
        gtk_widget_set_visible ( widget, true );
        widget = GTK_WIDGET ( gtk_builder_get_object (builder, "NYDateLabel") );
        gtk_widget_set_visible ( widget, true );
        widget = GTK_WIDGET ( gtk_builder_get_object (builder, "NYTimeLabel") );
        gtk_widget_set_visible ( widget, true );
        widget = GTK_WIDGET ( gtk_builder_get_object (builder, "MarketCloseLabel") );
        gtk_widget_set_visible ( widget, true );
        widget = GTK_WIDGET ( gtk_builder_get_object (builder, "TimeLeftLabel") );
        if( pkg->SecondsToOpen () ) {
            /* The Market Is Closed */
            gtk_widget_set_visible ( widget, false );
        } else {
            /* The Market Is Open */
            gtk_widget_set_visible ( widget, true );
        }
    } else {
        GtkWidget* widget = GTK_WIDGET ( gtk_builder_get_object (builder, "MainClockGrid") );
        gtk_widget_set_margin_bottom ( widget, 60 );
        widget = GTK_WIDGET ( gtk_builder_get_object (builder, "NewYorkTimeLabel") );
        gtk_widget_set_visible ( widget, false );
        widget = GTK_WIDGET ( gtk_builder_get_object (builder, "NYDateLabel") );
        gtk_widget_set_visible ( widget, false );
        widget = GTK_WIDGET ( gtk_builder_get_object (builder, "NYTimeLabel") );
        gtk_widget_set_visible ( widget, false );
        widget = GTK_WIDGET ( gtk_builder_get_object (builder, "MarketCloseLabel") );
        gtk_widget_set_visible ( widget, false );
        widget = GTK_WIDGET ( gtk_builder_get_object (builder, "TimeLeftLabel") );
        gtk_widget_set_visible ( widget, false );
    }

    return 0;    
}