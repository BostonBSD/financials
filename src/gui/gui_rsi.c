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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>

#include <stdbool.h>
#include <ctype.h>

#include <glib-object.h>
#include <gtk/gtk.h>

#include "../include/class_types.h"                  /* portfolio_packet, window_data */

#include "../include/gui_types.h"                    /* enums, etc */
#include "../include/gui_globals.h"                  /* GtkBuilder *builder */
#include "../include/gui.h"

#include "../include/workfuncs.h"
#include "../include/multicurl.h"
#include "../include/csv.h"
#include "../include/macros.h"
#include "../include/mutex.h"

static GtkListStore * rsi_completion_set_store (symbol_name_map *sn_map){
    GtkListStore *store = gtk_list_store_new(3, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);
    GtkTreeIter iter;

    gchar item[35];
    /* Populate the GtkListStore with the string of stock symbols in column 0, stock names in column 1, 
       and symbols & names in column 2. */
    for ( gint i=0; i < sn_map->size; i++ ){
        snprintf(item, 35, "%s - %s", sn_map->sn_container_arr[ i ]->symbol, sn_map->sn_container_arr[ i ]->security_name );

        gtk_list_store_append( store, &iter );
        /* Completion is going to match off of columns 0 and 1, but display column 2 */
        /* Completion matches based off of the symbol or the company name, inserts the symbol, displays both */
        gtk_list_store_set( store, &iter, 0, sn_map->sn_container_arr[ i ]->symbol, 1, sn_map->sn_container_arr[ i ]->security_name, 2, item, -1 );
    }
    return store;
}

static gboolean rsi_completion_match (GtkEntryCompletion *completion, const gchar *key, GtkTreeIter *iter, void *data) {
    /* We aren't using the data parameter, but the next statement
       silences a warning/error. */
    if( data != NULL) data = NULL;
    
    GtkTreeModel *model = gtk_entry_completion_get_model ( completion );
    gchar *item_symb, *item_name;
    /* We are finding matches based off of column 0 and 1, however, 
       we display column 2 in our 3 column model */
    gtk_tree_model_get ( model, iter, 0, &item_symb, 1, &item_name, -1 );
    gboolean ans = false, symbol_match = true, name_match = true;

    int N = 0;
    while( key[ N ] ){
        /* Only compare new key char if prev char was a match. */
        if(symbol_match) symbol_match = ( tolower( key [ N ] ) == tolower( item_symb [ N ] ) );
        if(name_match) name_match = ( tolower( key [ N ] ) == tolower( item_name [ N ] ) );
        /* Break the loop if both the symbol and the name are not a match. */
        if( (symbol_match == false) && (name_match == false) ) break;
        N++;
    }

    /* if either the symbol or the name match the key value, return true. */
    ans = symbol_match || name_match;
    g_free( item_symb );
    g_free( item_name );

    return ans;
}

int RSICompletionSet (void *data){
    pthread_mutex_lock( &mutex_working[ SYMBOL_NAME_MAP_MUTEX ] );
    if( data == NULL ) {
        pthread_mutex_unlock( &mutex_working[ SYMBOL_NAME_MAP_MUTEX ] );
        return 0;
    }

    GtkWidget* EntryBox = GTK_WIDGET ( gtk_builder_get_object (builder, "ViewRSISymbolEntryBox") );
    GtkEntryCompletion *completion = gtk_entry_completion_new();
    GtkListStore *store = rsi_completion_set_store( (symbol_name_map*)data );

    gtk_entry_completion_set_model(completion, GTK_TREE_MODEL( store ));
    g_object_unref( G_OBJECT( store ) );
    gtk_entry_completion_set_match_func(completion, (GtkEntryCompletionMatchFunc)rsi_completion_match, NULL, NULL);
    /* Set RSIView entrybox completion widget. */
    gtk_entry_set_completion( GTK_ENTRY( EntryBox ), completion );

    /* The text column to display is column 2 */
    gtk_entry_completion_set_text_column( completion, 2 );
    gtk_entry_completion_set_inline_completion ( completion, FALSE );
    gtk_entry_completion_set_inline_selection ( completion, TRUE );
    gtk_entry_completion_set_popup_completion ( completion, TRUE );
    /* Must type at least two characters for completion to make suggestions,
       reduces the number of results for single character keys. */
    gtk_entry_completion_set_minimum_key_length ( completion, 2 );
    /* The text column to insert is column 0
       We use a callback on the match-selected signal and insert the text from column 0 instead of column 2
       We use a callback on the cursor-on-match signal and insert the text from column 0 instead of column 2
    */
    g_signal_connect ( G_OBJECT( completion ), "match-selected", G_CALLBACK ( GUICallbackHandler_select_comp ), NULL );
    g_signal_connect ( G_OBJECT( completion ), "cursor-on-match", G_CALLBACK ( GUICallbackHandler_cursor_comp ), NULL );

    g_object_unref( G_OBJECT( completion ) );

    pthread_mutex_unlock( &mutex_working[ SYMBOL_NAME_MAP_MUTEX ] );
    return 0;
}

int RSIShowHide (void *data)
{
    portfolio_packet *pkg = (portfolio_packet*)data;
    window_data *W = pkg->GetWindowData ();

    /* get the GObject and cast as a GtkWidget */
    GtkWidget* window = GTK_WIDGET ( gtk_builder_get_object (builder, "ViewRSIWindow") );
    gboolean visible = gtk_widget_is_visible ( window );
    
    if ( visible ){
        gtk_widget_set_visible ( window, false );
        gtk_window_resize ( GTK_WINDOW ( window ), W->rsi_width, W->rsi_height );
        gtk_window_move ( GTK_WINDOW ( window ), W->rsi_x_pos, W->rsi_y_pos );
    } else {
        gtk_window_resize ( GTK_WINDOW ( window ), W->rsi_width, W->rsi_height );
        gtk_window_move ( GTK_WINDOW ( window ), W->rsi_x_pos, W->rsi_y_pos );

        GtkWidget* Button = GTK_WIDGET ( gtk_builder_get_object (builder, "ViewRSIFetchDataBTN") );
        gtk_widget_set_sensitive ( Button, false );
        g_object_set ( G_OBJECT ( Button ), "can-default", TRUE, "has-default", TRUE, NULL );

        /* Reset EntryBox */
        GtkWidget* EntryBox = GTK_WIDGET ( gtk_builder_get_object (builder, "ViewRSISymbolEntryBox") );
        gtk_entry_set_text ( GTK_ENTRY( EntryBox ), "" );
        g_object_set ( G_OBJECT ( EntryBox ), "activates-default", TRUE, NULL );
        gtk_widget_grab_focus ( EntryBox );

        GtkWidget* Label = GTK_WIDGET ( gtk_builder_get_object (builder, "ViewRSIStockSymbolLabel") );
        gtk_label_set_text ( GTK_LABEL ( Label ), "" );
        
        GtkWidget* scrwindow = GTK_WIDGET ( gtk_builder_get_object (builder, "ViewRSIScrolledWindow") );
        gtk_scrolled_window_set_vadjustment (GTK_SCROLLED_WINDOW( scrwindow ), NULL);

        gtk_widget_set_visible ( window, true );
    }
    return 0;
}

int RSICursorMove (){
    GtkWidget* EntryBox = GTK_WIDGET ( gtk_builder_get_object (builder, "ViewRSISymbolEntryBox") );
    GtkWidget* Button = GTK_WIDGET ( gtk_builder_get_object (builder, "ViewRSIFetchDataBTN") );
    const gchar* s = gtk_entry_get_text ( GTK_ENTRY( EntryBox ) );

    if( CheckValidString ( s ) ){
        gtk_widget_set_sensitive ( Button, true );
    } else {
        gtk_widget_set_sensitive ( Button, false );
    }
  
    return 0;
}

int RSITreeViewClear (){
    /* Clear the GtkTreeView. */
    GtkWidget* list = GTK_WIDGET ( gtk_builder_get_object (builder, "ViewRSITreeView") );
    GtkTreeViewColumn *column;
    gint n = gtk_tree_view_get_n_columns (GTK_TREE_VIEW(list));
    
    while(n > 0){
        n--;
        column = gtk_tree_view_get_column (GTK_TREE_VIEW(list), n);
        gtk_tree_view_remove_column (GTK_TREE_VIEW(list), column);
    }

    return 0;
}

static void rsi_set_columns ()
{
    GtkCellRenderer *renderer;
    GtkTreeViewColumn *column;
    GtkWidget* list = GTK_WIDGET ( gtk_builder_get_object (builder, "ViewRSITreeView") );

    renderer = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes("Date", renderer, "text", RSI_DATE, "foreground", RSI_FOREGROUND_COLOR, NULL);
    gtk_tree_view_column_set_resizable (column, true);
    gtk_tree_view_append_column(GTK_TREE_VIEW(list), column);

    renderer = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes("Price", renderer, "text", RSI_PRICE, "foreground", RSI_FOREGROUND_COLOR, NULL);
    gtk_tree_view_column_set_resizable (column, true);
    gtk_tree_view_append_column(GTK_TREE_VIEW(list), column);

    renderer = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes("High", renderer, "text", RSI_HIGH, "foreground", RSI_FOREGROUND_COLOR, NULL);
    gtk_tree_view_column_set_resizable (column, true);
    gtk_tree_view_append_column(GTK_TREE_VIEW(list), column);

    renderer = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes("Low", renderer, "text", RSI_LOW, "foreground", RSI_FOREGROUND_COLOR, NULL);
    gtk_tree_view_column_set_resizable (column, true);
    gtk_tree_view_append_column(GTK_TREE_VIEW(list), column);

    renderer = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes("Opening", renderer, "text", RSI_OPENING, "foreground", RSI_FOREGROUND_COLOR, NULL);
    gtk_tree_view_column_set_resizable (column, true);
    gtk_tree_view_append_column(GTK_TREE_VIEW(list), column);

    renderer = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes("Prev Closing", renderer, "text", RSI_PREV_CLOSING, "foreground", RSI_FOREGROUND_COLOR, NULL);
    gtk_tree_view_column_set_resizable (column, true);
    gtk_tree_view_append_column(GTK_TREE_VIEW(list), column);

    renderer = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes("Chg ($)", renderer, "text", RSI_CHANGE, "foreground", RSI_FOREGROUND_COLOR, NULL);
    gtk_tree_view_column_set_resizable (column, true);
    gtk_tree_view_append_column(GTK_TREE_VIEW(list), column);

    renderer = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes("Gain (%)", renderer, "text", RSI_CHANGE_PERCENT, "foreground", RSI_FOREGROUND_COLOR, NULL);
    gtk_tree_view_column_set_resizable (column, true);
    gtk_tree_view_append_column(GTK_TREE_VIEW(list), column);

    renderer = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes("Volume", renderer, "text", RSI_VOLUME, "foreground", RSI_FOREGROUND_COLOR, NULL);
    gtk_tree_view_column_set_resizable (column, true);
    gtk_tree_view_append_column(GTK_TREE_VIEW(list), column);

    renderer = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes("RSI", renderer, "text", RSI_RSI, "foreground", RSI_FOREGROUND_COLOR, NULL);
    gtk_tree_view_column_set_resizable (column, true);
    gtk_tree_view_append_column(GTK_TREE_VIEW(list), column);

    renderer = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes("Indicator", renderer, "text", RSI_INDICATOR, "foreground", RSI_FOREGROUND_COLOR, NULL);
    gtk_tree_view_column_set_resizable (column, true);
    gtk_tree_view_append_column(GTK_TREE_VIEW(list), column);
}

int RSISetSNLabel (void *data){
    gchar* sec_name;
    data ? (sec_name = (gchar*)data) : (sec_name = NULL);

    GtkWidget* Label = GTK_WIDGET ( gtk_builder_get_object (builder, "ViewRSIStockSymbolLabel") );

    gtk_label_set_text ( GTK_LABEL ( Label ), sec_name ? sec_name : "" );
    if ( sec_name ) g_free ( sec_name );

    return 0;
}

int RSIGetSymbol (char **s)
/* Get the stock symbol from the EntryBox.
   Must free return value. 
   
   Because we are not setting any widgets, we should not worry about crashing Gtk outside
   the Gtk Main Loop.

   If there are any problems, FetchRSIData can be placed into the Gtk Main Loop,
   it will block the Gtk loop, but it won't crash Gtk, put "FetchRSIData ();" into
   the "RSIMakeTreeview();" function.
*/
{
    GtkWidget* EntryBox = GTK_WIDGET ( gtk_builder_get_object (builder, "ViewRSISymbolEntryBox") );
    s[0] = strdup( gtk_entry_get_text ( GTK_ENTRY( EntryBox ) ) );
    UpperCaseStr ( s[0] );

    return 0;
}

static void rsi_set_store (GtkListStore *store, void *data){
    MemType *MyOutputStruct;
    data ? (MyOutputStruct = (MemType*)data) : (MyOutputStruct = NULL);

    double gain, avg_gain = 0, avg_loss = 0, cur_price, prev_price, rsi, change;
    char *price_ch, *high_ch, *low_ch, *opening_ch, *prev_closing_ch, *change_ch, *indicator_ch;
    char gain_ch[10], rsi_ch[10], volume_ch[15];
    long volume;
    int *new_order = (int*)malloc (1), *tmp;

    GtkTreeIter iter;

    if( MyOutputStruct == NULL ) { free( new_order ); return; }
    if( MyOutputStruct->memory == NULL ) { free( new_order ); free( MyOutputStruct ); return; }

    /* Convert a String to a File Pointer Stream for Reading */
    FILE* fp = fmemopen( (void*)MyOutputStruct->memory, strlen( MyOutputStruct->memory ) + 1, "r" );
    
    int counter = 0, c = 0;
    char line[1024];
    char **csv_array;
    
    /* Get rid of the header line from the file stream */
    if (fgets( line, 1024, fp) == NULL) {
        free(new_order);
        fclose( fp ); 
        return;
    }
    
    /* Get the initial closing price */
    if (fgets( line, 1024, fp) == NULL) {
        free(new_order);
        fclose( fp ); 
        return;
    }
    
    Chomp( line );
    csv_array = parse_csv( line );
    prev_price = strtod( csv_array[ 4 ], NULL );
    free_csv_line( csv_array );

    while ( fgets( line, 1024, fp) != NULL ) {
        /* If there is a null error in this line, ignore the line.
           Sometimes Yahoo! data is incomplete, the result is more
           correct if we ignore the incomplete portion of data. */
        if ( strstr( line, "null" ) ) continue;

        Chomp( line );
    	csv_array = parse_csv( line );
    	
    	cur_price = strtod( csv_array[ 4 ], NULL );
    	gain = CalcGain ( cur_price, prev_price );
        change = cur_price - prev_price;
        prev_closing_ch = DoubleToMonetary( prev_price );
    	prev_price = cur_price;

        /* Until we get 14 days of data we sum the gains and losses. */
    	if ( counter < 14 ) Summation ( gain, &avg_gain, &avg_loss);
        /* On the 14th day we calculate the regular average and use that to seed a running average. */
    	if ( counter == 13 ) { avg_gain = avg_gain / 14; avg_loss = avg_loss / 14; }
        /* We only start adding rows if we have enough data to calculate the RSI,
           which is at least 14 days of data plus the initial closing price. */
    	if ( counter >= 14 ) {
            tmp = (int*)realloc( new_order, sizeof(int) * (c + 1) );
            new_order = tmp;

            /* Calculate the running average. */
    		CalcAvg ( gain, &avg_gain, &avg_loss);
            /* Calculate the rsi. */
    		rsi = CalcRsi ( avg_gain, avg_loss );
            /* The RsiIndicator return value is stored in the stack, do not free. */
            indicator_ch = RsiIndicator ( rsi );

            price_ch = StringToMonetary( csv_array[ 4 ] );
            high_ch = StringToMonetary( csv_array[ 2 ] );
            low_ch = StringToMonetary( csv_array[ 3 ] );
            opening_ch = StringToMonetary( csv_array[ 1 ] );
            change_ch = DoubleToMonetary( change );
            volume = strtol( csv_array[ 6 ], NULL, 10 );

            setlocale(LC_NUMERIC, LOCALE);
            snprintf(gain_ch, 10, "%0.03f%%", gain);
            snprintf(rsi_ch, 10, "%0.03f", rsi);
            snprintf(volume_ch, 15, "%'ld", volume);

            /* Add data to the storage container. */
            gtk_list_store_append ( store, &iter );
            if( gain > 0 ){
                gtk_list_store_set ( store, &iter, RSI_FOREGROUND_COLOR, "Green", RSI_DATE, csv_array[ 0 ], RSI_PRICE, price_ch, RSI_HIGH, high_ch, RSI_LOW, low_ch, RSI_OPENING, opening_ch, RSI_PREV_CLOSING, prev_closing_ch, RSI_CHANGE, change_ch, RSI_CHANGE_PERCENT, gain_ch, RSI_VOLUME, volume_ch, RSI_RSI, rsi_ch, RSI_INDICATOR, indicator_ch, -1 );
            } else if ( gain < 0){
                gtk_list_store_set ( store, &iter, RSI_FOREGROUND_COLOR, "DarkRed", RSI_DATE, csv_array[ 0 ], RSI_PRICE, price_ch, RSI_HIGH, high_ch, RSI_LOW, low_ch, RSI_OPENING, opening_ch, RSI_PREV_CLOSING, prev_closing_ch, RSI_CHANGE, change_ch, RSI_CHANGE_PERCENT, gain_ch, RSI_VOLUME, volume_ch, RSI_RSI, rsi_ch, RSI_INDICATOR, indicator_ch, -1 );
            } else {
                gtk_list_store_set ( store, &iter, RSI_DATE, csv_array[ 0 ], RSI_PRICE, price_ch, RSI_HIGH, high_ch, RSI_LOW, low_ch, RSI_OPENING, opening_ch, RSI_PREV_CLOSING, prev_closing_ch, RSI_CHANGE, change_ch, RSI_CHANGE_PERCENT, gain_ch, RSI_VOLUME, volume_ch, RSI_RSI, rsi_ch, RSI_INDICATOR, indicator_ch, -1 );
            }

            free( price_ch );
            free( high_ch );
            free( low_ch );
            free( opening_ch );
            free( change_ch );

            c++; /* The number of rows added to the TreeView */
    	}        
    	
        free( prev_closing_ch );
    	free_csv_line( csv_array );
    	counter++;
    }
    
    fclose( fp );

    /* Free MyOutputStruct */
    free( MyOutputStruct->memory );
    free( MyOutputStruct );  

    /* The rows were added to the TreeView first [earliest] to last [most recent],
       we need to reverse this from last to first. 
       The last [most recent] entry needs to be at the top. */
    if ( c > 0){
        /* For the zero row, c = 1 after one iteration, so subtract one
           before the next calculation. */
        c-=1;
        int b = 0;
        while(c >= 0){
            /* new_order [newpos] = oldpos */
            new_order[ b ] = c;
            c--;
            b++;
        }

        gtk_list_store_reorder (store, new_order);
    }
    free( new_order );
}

static GtkListStore* rsi_make_store (void *data)
{ 
    GtkListStore *store;

    /* Set up the storage container with the number of columns and column type */
    store = gtk_list_store_new(RSI_N_COLUMNS, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);

    /* Add data to the storage container, pass in the outputstruct pointer. */
    /* This function frees data. */
    rsi_set_store ( store, data ); 
    return store;
}

int RSIMakeTreeview (void *data)
{
    GtkListStore *store = NULL;
    GtkWidget* list = GTK_WIDGET ( gtk_builder_get_object (builder, "ViewRSITreeView") );

    /* Set the columns for the new TreeView model */
    rsi_set_columns ();

    /* Set up the storage container, pass in the outputstruct pointer. */
    /* This function frees data. */
    store = rsi_make_store ( data );   
    
    /* Add the store of data to the list. */
    gtk_tree_view_set_model ( GTK_TREE_VIEW ( list ), GTK_TREE_MODEL ( store ) );
    g_object_unref ( store );
   
    /* Set the list header as visible. */
    gtk_tree_view_set_headers_visible ( GTK_TREE_VIEW ( list ), TRUE );

    /* Remove Grid Lines. */
    gtk_tree_view_set_grid_lines ( GTK_TREE_VIEW ( list ), GTK_TREE_VIEW_GRID_LINES_NONE );

    return 0;
}