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

#include "../include/gui_globals.h"

#include "../include/class_types.h"         /* portfolio_packet, equity_folder, metal, meta */
#include "../include/sqlite.h"
#include "../include/workfuncs.h"
#include "../include/mutex.h"

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

static int add_security_ok (void *data)
{
    /* Unpack the package */
    portfolio_packet *package = (portfolio_packet*)data;
    equity_folder *F = package->securities_folder;
    meta *D = package->portfolio_meta_info;

    GtkWidget* EntryBox = GTK_WIDGET ( gtk_builder_get_object (builder, "AddRemoveSecuritySymbolEntryBox") );
    char* symbol = strdup( gtk_entry_get_text ( GTK_ENTRY( EntryBox ) ) );

    EntryBox = GTK_WIDGET ( gtk_builder_get_object (builder, "AddRemoveSecuritySharesEntryBox") );
    char* shares = strdup( gtk_entry_get_text ( GTK_ENTRY( EntryBox ) ) );

    UpperCaseStr( symbol );
    FormatStr( shares );
        
    SqliteAddEquity( symbol, shares, F, D );

    free( symbol );
    free( shares );

    return 0;
}

static int remove_security_ok (void *data)
{
    /* Unpack the package */
    portfolio_packet *package = (portfolio_packet*)data;
    equity_folder *F = package->securities_folder;
    meta *D = package->portfolio_meta_info;

    GtkWidget* ComboBox = GTK_WIDGET ( gtk_builder_get_object (builder, "AddRemoveSecurityComboBox") );
    int index = gtk_combo_box_get_active ( GTK_COMBO_BOX ( ComboBox ) );
    if( index == 0 ) return 0;
    if( index == 1) {     
        SqliteRemoveAllEquity( F, D );
    } else {
        char* symbol = gtk_combo_box_text_get_active_text ( GTK_COMBO_BOX_TEXT ( ComboBox ) );
        SqliteRemoveEquity( symbol, F, D );
        free( symbol );
    }
    return 0;
}

int AddRemOk (void *data)
{
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

    pthread_mutex_unlock( &mutex_working[ FETCH_DATA_MUTEX ] );

    return 0;
}

int AddRemShowHide (void *data)
{
    /* Unpack the package */
    portfolio_packet *package = (portfolio_packet*)data;
    equity_folder *F = package->securities_folder;

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