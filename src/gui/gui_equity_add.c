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
#include <ctype.h>

#include <glib-object.h>
#include <gtk/gtk.h>

#include "../include/gui_globals.h"         /* GtkBuilder *builder */
#include "../include/gui.h"

#include "../include/class_types.h"         /* portfolio_packet, equity_folder, metal, meta, window_data */
#include "../include/multicurl.h"
#include "../include/json.h"
#include "../include/sqlite.h"
#include "../include/workfuncs.h"
#include "../include/mutex.h"

static GtkListStore * addrem_completion_set_store (void *data){
    symbol_name_map *sn_map = ( symbol_name_map* ) data;
    GtkListStore *store = gtk_list_store_new(3, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);
    GtkTreeIter iter;

    char item[35];
    /* Populate the GtkListStore with the string of stock symbols in column 0, stock names in column 1, 
       and symbols & names in column 2. */
    for ( int i=0; i < sn_map->size; i++ ){
        snprintf(item, 35, "%s - %s", sn_map->sn_container_arr[ i ]->symbol, sn_map->sn_container_arr[ i ]->security_name );

        gtk_list_store_append( store, &iter );
        /* Completion is going to match off of columns 0 and 1, but display column 2 */
        /* Completion matches based off of the symbol or the company name, inserts the symbol, displays both */
        gtk_list_store_set( store, &iter, 0, sn_map->sn_container_arr[ i ]->symbol, 1, sn_map->sn_container_arr[ i ]->security_name, 2, item, -1 );
    }
    return store;
}

static gboolean addrem_completion_match (GtkEntryCompletion *completion, const gchar *key, GtkTreeIter *iter, void *data) {
    /* We aren't using the data parameter, but the next statement
       silences a warning/error. */
    if( data != NULL) data = NULL;
    
    GtkTreeModel *model = gtk_entry_completion_get_model ( completion );
    gchar *item_symb, *item_name;
    /* We are finding matches based off of column 0 and 1, however, 
       we display column 2 in our 3 column model */
    gtk_tree_model_get ( model, iter, 0, &item_symb, -1 );
    gtk_tree_model_get ( model, iter, 1, &item_name, -1 );
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

int AddRemCompletionSet (void *data){
    pthread_mutex_lock( &mutex_working[ COMPLETION_FETCH_MUTEX ] );
    if( data == NULL ) return 0;

    GtkWidget* EntryBox = GTK_WIDGET ( gtk_builder_get_object (builder, "AddRemoveSecuritySymbolEntryBox") );
    GtkEntryCompletion *completion = gtk_entry_completion_new();
    GtkListStore *store = addrem_completion_set_store ( data );

    gtk_entry_completion_set_model(completion, GTK_TREE_MODEL( store ));
    g_object_unref( G_OBJECT( store ) );
    gtk_entry_completion_set_match_func(completion, (GtkEntryCompletionMatchFunc)addrem_completion_match, NULL, NULL);
    /* Set AddRemoveSecuritySymbol entrybox completion widget. */
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

    pthread_mutex_unlock( &mutex_working[ COMPLETION_FETCH_MUTEX ] );
    return 0;
}

int AddRemCursorMove ()
{
    GtkWidget* Button = GTK_WIDGET ( gtk_builder_get_object (builder, "AddRemoveSecurityOkBTN") );
    GtkWidget* EntryBoxSymbol = GTK_WIDGET ( gtk_builder_get_object (builder, "AddRemoveSecuritySymbolEntryBox") );
    GtkWidget* EntryBoxShares = GTK_WIDGET ( gtk_builder_get_object (builder, "AddRemoveSecuritySharesEntryBox") );

    char* symbol = strdup( gtk_entry_get_text ( GTK_ENTRY( EntryBoxSymbol ) ) );
    char* shares = strdup( gtk_entry_get_text ( GTK_ENTRY( EntryBoxShares ) ) );

    if ( CheckValidString( symbol ) && CheckValidString( shares ) && CheckIfStringLongPositiveNumber( shares ) ){
        gtk_widget_set_sensitive ( Button, true );
    } else {
        gtk_widget_set_sensitive ( Button, false );
    }

    free ( symbol );
    free ( shares );
    return 0;
}

int AddRemComBoxChange ()
{
    GtkWidget* ComboBox = GTK_WIDGET ( gtk_builder_get_object (builder, "AddRemoveSecurityComboBox") );
    int index = gtk_combo_box_get_active ( GTK_COMBO_BOX ( ComboBox ) );

    GtkWidget* Switch = GTK_WIDGET ( gtk_builder_get_object (builder, "AddRemoveSecuritySwitch") );
    gboolean SwitchSet = gtk_switch_get_active ( GTK_SWITCH( Switch ) );

    GtkWidget* Button = GTK_WIDGET ( gtk_builder_get_object (builder, "AddRemoveSecurityOkBTN") );

    if(SwitchSet == false) {
        return 0;
    }

    if( index != 0 ){
        gtk_widget_set_sensitive ( Button, true );
    } else {
        gtk_widget_set_sensitive ( Button, false );
    }
    return 0;
}

int AddRemSwitchChange ()
{
    GtkWidget* ComboBox = GTK_WIDGET ( gtk_builder_get_object (builder, "AddRemoveSecurityComboBox") );
    GtkWidget* Switch = GTK_WIDGET ( gtk_builder_get_object (builder, "AddRemoveSecuritySwitch") );
    gboolean SwitchSet = gtk_switch_get_active ( GTK_SWITCH( Switch ) );

    GtkWidget* EntryBoxSymbol = GTK_WIDGET ( gtk_builder_get_object (builder, "AddRemoveSecuritySymbolEntryBox") );
    GtkWidget* EntryBoxShares = GTK_WIDGET ( gtk_builder_get_object (builder, "AddRemoveSecuritySharesEntryBox") );
    GtkWidget* Button = GTK_WIDGET ( gtk_builder_get_object (builder, "AddRemoveSecurityOkBTN") );

    if ( SwitchSet == false ) {
        gtk_combo_box_set_button_sensitivity (GTK_COMBO_BOX ( ComboBox ), GTK_SENSITIVITY_OFF);
        gtk_combo_box_set_active (GTK_COMBO_BOX ( ComboBox ), 0 );
        gtk_widget_set_sensitive ( EntryBoxSymbol, true );
        gtk_widget_set_sensitive ( EntryBoxShares, true );
        gtk_widget_set_sensitive ( Button, false );

        /* Reset EntryBoxes */
        gtk_entry_set_text ( GTK_ENTRY( EntryBoxSymbol ), "" );
        gtk_entry_set_text ( GTK_ENTRY( EntryBoxShares ), "" );
    } else {
        gtk_combo_box_set_button_sensitivity (GTK_COMBO_BOX ( ComboBox ), GTK_SENSITIVITY_AUTO);
        gtk_widget_set_sensitive ( EntryBoxSymbol, false );
        gtk_widget_set_sensitive ( EntryBoxShares, false );
        gtk_widget_set_sensitive ( Button, false );

        /* Reset EntryBoxes */
        gtk_entry_set_text ( GTK_ENTRY( EntryBoxSymbol ), "" );
        gtk_entry_set_text ( GTK_ENTRY( EntryBoxShares ), "" );
    }

    return 0;
}

static void *fetch_data_for_new_stock ( void *data )
{
    portfolio_packet *pkg = (portfolio_packet*)data;
    equity_folder *F = pkg->GetEquityFolderClass ();

    pthread_mutex_lock( &mutex_working [ CLASS_MEMBER_MUTEX ] );

    /* The new stock is currently at the end of the Equity array. */
    SetUpCurlHandle ( F->Equity[ F->size - 1 ]->easy_hnd, pkg->multicurl_main_hnd, F->Equity[ F->size - 1 ]->curl_url_stock_ch, &F->Equity[ F->size - 1 ]->JSON );
    PerformMultiCurl ( pkg->multicurl_main_hnd, 1.0f );
    /* Extract double values from JSON data using JSON-glib */
    JsonExtractEquity( F->Equity[ F->size - 1 ]->JSON.memory, F->Equity[ F->size - 1 ]->current_price_stock_f, F->Equity[ F->size - 1 ]->high_stock_f, F->Equity[ F->size - 1 ]->low_stock_f, F->Equity[ F->size - 1 ]->opening_stock_f, F->Equity[ F->size - 1 ]->prev_closing_stock_f, F->Equity[ F->size - 1 ]->change_share_f, F->Equity[ F->size - 1 ]->change_percent_f);

    /* Free memory. */
    free( F->Equity[ F->size - 1 ]->JSON.memory );
    F->Equity[ F->size - 1 ]->JSON.memory = NULL;

    pthread_mutex_unlock( &mutex_working [ CLASS_MEMBER_MUTEX ] );

    /* Sort the equity folder. */
    F->Sort (); /* The new stock is in alphabetical order within the array. */

    pkg->Calculate ();
    pkg->ToStrings ();
    
    /* Update the main window treeview. */
    gdk_threads_add_idle ( MainPrimaryTreeview, data );

    return NULL;
}

static void add_equity_to_folder ( char *symbol, char *shares, portfolio_packet *pkg ){
    equity_folder *F = pkg->GetEquityFolderClass ();

    /* Remove the stock if it already exists. */
    F->RemoveStock ( symbol );

    /* Add a new stock object to the end of the folder's Equity array. */
    F->AddStock ( symbol, shares );

    /* Generate the Equity Request URLs. */
    F->GenerateURL ( pkg );

    if( pkg->IsFetchingData () ){
        /* Fetch the data in a separate thread */
        pthread_t thread_id;
        pthread_create( &thread_id, NULL, fetch_data_for_new_stock, pkg ); 
    } else {
        /* Sort the equity folder. */
        F->Sort ();
    }  
}

static int add_security_ok ( void *data )
{
    /* Unpack the package */
    portfolio_packet *package = (portfolio_packet*)data;
    meta *D = package->GetMetaClass ();

    GtkWidget* EntryBox = GTK_WIDGET ( gtk_builder_get_object (builder, "AddRemoveSecuritySymbolEntryBox") );
    char* symbol = strdup( gtk_entry_get_text ( GTK_ENTRY( EntryBox ) ) );

    EntryBox = GTK_WIDGET ( gtk_builder_get_object (builder, "AddRemoveSecuritySharesEntryBox") );
    char* shares = strdup( gtk_entry_get_text ( GTK_ENTRY( EntryBox ) ) );

    UpperCaseStr( symbol );
    FormatStr( shares );
        
    SqliteAddEquity( symbol, shares, D );
    add_equity_to_folder ( symbol, shares, package );

    free( symbol );
    free( shares );

    return 0;
}

static int remove_security_ok ( void *data )
{
    /* Unpack the package */
    portfolio_packet *package = (portfolio_packet*)data;
    equity_folder *F = package->GetEquityFolderClass ();
    meta *D = package->GetMetaClass ();

    GtkWidget* ComboBox = GTK_WIDGET ( gtk_builder_get_object (builder, "AddRemoveSecurityComboBox") );
    int index = gtk_combo_box_get_active ( GTK_COMBO_BOX ( ComboBox ) );
    if( index == 0 ) return 0;
    if( index == 1) {     
        SqliteRemoveAllEquity( D );
        /* Reset Equity Folder */
        F->Reset ();
    } else {
        char* symbol = gtk_combo_box_text_get_active_text ( GTK_COMBO_BOX_TEXT ( ComboBox ) );
        SqliteRemoveEquity( symbol, D );
        F->RemoveStock ( symbol );
        free( symbol );
    }
    return 0;
}

int AddRemOk ( void *data )
{
    /* Unpack the package */
    portfolio_packet *package = (portfolio_packet*)data;

    /* This mutex prevents the program from crashing if a
        FETCH_DATA_BTN signal is run in parallel with this thread. */
    pthread_mutex_lock( &mutex_working[ FETCH_DATA_MUTEX ] );

    GtkWidget* Switch = GTK_WIDGET ( gtk_builder_get_object (builder, "AddRemoveSecuritySwitch") );
    gboolean SwitchSet = gtk_switch_get_active ( GTK_SWITCH( Switch ) );

    if ( SwitchSet == false ) {
        add_security_ok ( data );
    } else {
        remove_security_ok ( data );
    }
    
    if( package->IsFetchingData () == false ) {
        MainDefaultTreeview ( data );
    }

    pthread_mutex_unlock( &mutex_working[ FETCH_DATA_MUTEX ] );

    return 0;
}

int AddRemShowHide ( void *data )
{
    /* Unpack the package */
    portfolio_packet *package = (portfolio_packet*)data;
    equity_folder *F = package->GetEquityFolderClass ();

    GtkWidget* window = GTK_WIDGET ( gtk_builder_get_object (builder, "AddRemoveSecurity") );
    gboolean visible = gtk_widget_is_visible ( window );

    GtkWidget* EntryBox, *Switch;    
    Switch = GTK_WIDGET ( gtk_builder_get_object (builder, "AddRemoveSecuritySwitch") );
    GtkWidget* ComboBox = GTK_WIDGET ( gtk_builder_get_object (builder, "AddRemoveSecurityComboBox") );
    GtkWidget* Button = GTK_WIDGET ( gtk_builder_get_object (builder, "AddRemoveSecurityOkBTN") );

    if (visible == false){
        gtk_switch_set_active (GTK_SWITCH( Switch ), false);
        gtk_widget_set_sensitive ( Button, false );

        /* Reset EntryBoxes */
        EntryBox = GTK_WIDGET ( gtk_builder_get_object (builder, "AddRemoveSecuritySymbolEntryBox") );
        gtk_entry_set_text ( GTK_ENTRY( EntryBox ), "" );
        gtk_widget_grab_focus ( EntryBox );

        EntryBox = GTK_WIDGET ( gtk_builder_get_object (builder, "AddRemoveSecuritySharesEntryBox") );
        gtk_entry_set_text ( GTK_ENTRY( EntryBox ), "" );
        g_object_set ( G_OBJECT ( EntryBox ), "activates-default", TRUE, NULL );

        /* Reset ComboBox */
        gtk_combo_box_text_remove_all ( GTK_COMBO_BOX_TEXT ( ComboBox ) );
        gtk_combo_box_text_append ( GTK_COMBO_BOX_TEXT ( ComboBox ), NULL, "Select a security for removal" );

        if( F->size > 0 ){
            gtk_combo_box_text_append ( GTK_COMBO_BOX_TEXT ( ComboBox ), NULL, "Remove All Securities" );
        }

        for(short i = 0; i < F->size; i++){
            gtk_combo_box_text_append ( GTK_COMBO_BOX_TEXT ( ComboBox ), NULL, F->Equity[ i ]->symbol_stock_ch );
        }

        gtk_combo_box_set_active ( GTK_COMBO_BOX ( ComboBox ), 0 );

        gtk_widget_set_visible ( window, true );
    } else {
        gtk_widget_set_visible ( window, false );
    }

    return 0;
}