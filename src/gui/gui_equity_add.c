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

static GtkListStore * addrem_completion_set_store ( symbol_name_map *sn_map ){
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

static gboolean addrem_completion_match (GtkEntryCompletion *completion, const gchar *key, GtkTreeIter *iter, void *data) {
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

int AddRemCompletionSet (void *data){
    pthread_mutex_lock( &mutex_working[ SYMBOL_NAME_MAP_MUTEX ] );
    if( data == NULL ) {
        pthread_mutex_unlock( &mutex_working[ SYMBOL_NAME_MAP_MUTEX ] );
        return 0;
    }

    GtkWidget* EntryBox = GTK_WIDGET ( gtk_builder_get_object (builder, "AddRemoveSecuritySymbolEntryBox") );
    GtkEntryCompletion *completion = gtk_entry_completion_new();
    GtkListStore *store = addrem_completion_set_store ( (symbol_name_map*)data );

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

    pthread_mutex_unlock( &mutex_working[ SYMBOL_NAME_MAP_MUTEX ] );
    return 0;
}

int AddRemCursorMove ()
{
    GtkWidget* Button = GTK_WIDGET ( gtk_builder_get_object (builder, "AddRemoveSecurityOkBTN") );
    GtkWidget* EntryBoxSymbol = GTK_WIDGET ( gtk_builder_get_object (builder, "AddRemoveSecuritySymbolEntryBox") );
    GtkWidget* EntryBoxShares = GTK_WIDGET ( gtk_builder_get_object (builder, "AddRemoveSecuritySharesEntryBox") );

    const gchar* symbol = gtk_entry_get_text ( GTK_ENTRY( EntryBoxSymbol ) );
    const gchar* shares = gtk_entry_get_text ( GTK_ENTRY( EntryBoxShares ) );

    if ( CheckValidString( symbol ) && CheckValidString( shares ) && CheckIfStringLongPositiveNumber( shares ) ){
        gtk_widget_set_sensitive ( Button, true );
    } else {
        gtk_widget_set_sensitive ( Button, false );
    }

    return 0;
}

static void remove_dash (char *s)
/* Locate first dash character '-' in a string, 
   replace prior space with NULL character.
   If the string pointer is NULL or the first 
   character is a dash; do nothing */
{
    if (s == NULL || s[0] == '-') return;
	char *ch = strchr( s, (int)'-' );
    if ( ch != NULL ) ch[-1] = 0;
}

int AddRemComBoxChange (void *data)
{
    portfolio_packet *pkg = (portfolio_packet*)data;
    symbol_name_map *sn_map = pkg->GetSymNameMap ();

    GtkWidget* ComboBox = GTK_WIDGET ( gtk_builder_get_object (builder, "AddRemoveSecurityComboBox") );
    gint index = gtk_combo_box_get_active ( GTK_COMBO_BOX ( ComboBox ) );

    GtkWidget* button = GTK_WIDGET ( gtk_builder_get_object (builder, "AddRemoveSecurityOkBTN") );
    GtkWidget* label = GTK_WIDGET ( gtk_builder_get_object (builder, "AddRemoveSecurityLabel") );

    gtk_label_set_label ( GTK_LABEL ( label ), "" );

    if( index != 0 ){
        gchar* symbol = gtk_combo_box_text_get_active_text ( GTK_COMBO_BOX_TEXT ( ComboBox ) );
        char* name = GetSecurityName ( symbol, sn_map );

        remove_dash ( name );
        gtk_label_set_label ( GTK_LABEL ( label ), name ? name : "" );
        g_free( symbol );
        if ( name ) free ( name );

        gtk_widget_set_sensitive ( button, true );
    } else {
        gtk_label_set_label ( GTK_LABEL ( label ), "" );
        gtk_widget_set_sensitive ( button, false );
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
    JsonExtractEquity( F->Equity[ F->size - 1 ]->JSON.memory, &F->Equity[ F->size - 1 ]->current_price_stock_f, &F->Equity[ F->size - 1 ]->high_stock_f, &F->Equity[ F->size - 1 ]->low_stock_f, &F->Equity[ F->size - 1 ]->opening_stock_f, &F->Equity[ F->size - 1 ]->prev_closing_stock_f, &F->Equity[ F->size - 1 ]->change_share_f, &F->Equity[ F->size - 1 ]->change_percent_f );

    /* Free memory. */
    free( F->Equity[ F->size - 1 ]->JSON.memory );
    F->Equity[ F->size - 1 ]->JSON.memory = NULL;

    pthread_mutex_unlock( &mutex_working [ CLASS_MEMBER_MUTEX ] );

    /* Sort the equity folder, the following three statements lock the CLASS_MEMBER_MUTEX */
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

    if( !pkg->IsDefaultView () ){
        /* Fetch the data in a separate thread */
        pthread_t thread_id;
        pthread_create( &thread_id, NULL, fetch_data_for_new_stock, pkg ); 
    } else {
        /* Sort the equity folder. */
        F->Sort ();
    }  
}

static void add_security_ok ( void *data )
{
    /* Unpack the package */
    portfolio_packet *package = (portfolio_packet*)data;
    meta *D = package->GetMetaClass ();

    GtkWidget* EntryBox = GTK_WIDGET ( gtk_builder_get_object (builder, "AddRemoveSecuritySymbolEntryBox") );
    gchar* symbol = strdup( gtk_entry_get_text ( GTK_ENTRY( EntryBox ) ) );

    EntryBox = GTK_WIDGET ( gtk_builder_get_object (builder, "AddRemoveSecuritySharesEntryBox") );
    gchar* shares = strdup( gtk_entry_get_text ( GTK_ENTRY( EntryBox ) ) );

    UpperCaseStr( symbol );
    ToNumStr( shares );
        
    SqliteAddEquity( symbol, shares, D );
    add_equity_to_folder ( symbol, shares, package );

    g_free( symbol );
    g_free( shares );
}

static void remove_security_ok ( void *data )
{
    /* Unpack the package */
    portfolio_packet *pkg = (portfolio_packet*)data;
    equity_folder *F = pkg->GetEquityFolderClass ();
    meta *D = pkg->GetMetaClass ();

    GtkWidget* ComboBox = GTK_WIDGET ( gtk_builder_get_object (builder, "AddRemoveSecurityComboBox") );
    gint index = gtk_combo_box_get_active ( GTK_COMBO_BOX ( ComboBox ) );
    if( index == 0 ) return;
    if( index == 1) {     
        SqliteRemoveAllEquity( D );
        /* Reset Equity Folder */
        F->Reset ();

        pkg->Calculate ();
        pkg->ToStrings ();
    
        if( !pkg->IsDefaultView () ) {    
            /* Update the main window treeview. */
            MainPrimaryTreeview ( data );
        }
    } else {
        gchar* symbol = gtk_combo_box_text_get_active_text ( GTK_COMBO_BOX_TEXT ( ComboBox ) );
        SqliteRemoveEquity( symbol, D );
        F->RemoveStock ( symbol );
        g_free( symbol );

        /* Sort the equity folder, the following three statements lock the CLASS_MEMBER_MUTEX */
        F->Sort (); /* The new stock is in alphabetical order within the array. */

        pkg->Calculate ();
        pkg->ToStrings ();
    
        if( !pkg->IsDefaultView () ) {    
            /* Update the main window treeview. */
            MainPrimaryTreeview ( data );
        }
    }
}

int AddRemOk ( void *data )
{
    /* Unpack the package */
    portfolio_packet *package = (portfolio_packet*)data;

    /* This mutex prevents the program from crashing if a
        FETCH_DATA_BTN signal is run in parallel with this thread. */
    pthread_mutex_lock( &mutex_working[ FETCH_DATA_MUTEX ] );

    GtkWidget* stack = GTK_WIDGET ( gtk_builder_get_object (builder, "AddRemoveSecurityStack") );
    const gchar *name = gtk_stack_get_visible_child_name ( GTK_STACK ( stack ) );

    if ( strcasecmp ( name, "add" ) == 0 ) {
        add_security_ok ( data );
    } else {
        remove_security_ok ( data );
    }
    
    if( package->IsDefaultView () ) {
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

    GtkWidget* stack = GTK_WIDGET ( gtk_builder_get_object (builder, "AddRemoveSecurityStack") );
    GtkWidget* ComboBox = GTK_WIDGET ( gtk_builder_get_object (builder, "AddRemoveSecurityComboBox") );
    GtkWidget* Button = GTK_WIDGET ( gtk_builder_get_object (builder, "AddRemoveSecurityOkBTN") );

    if (visible == false){
        gtk_stack_set_visible_child_name ( GTK_STACK ( stack ), "add" );

        /* Set the OK button sensitivity to false. */
        gtk_widget_set_sensitive ( Button, false );

        /* Reset EntryBoxes */
        GtkWidget* EntryBox = GTK_WIDGET ( gtk_builder_get_object (builder, "AddRemoveSecuritySymbolEntryBox") );
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