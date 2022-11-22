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

#include "../include/gui_globals.h"         /* GtkBuilder *builder */

#include "../include/class_types.h"         /* portfolio_packet, equity_folder, metal, meta */
#include "../include/sqlite.h"
#include "../include/workfuncs.h"

int APIShowHide (void *data)
{
    /* Unpack the package */
    portfolio_packet *package = (portfolio_packet*)data;
    meta *D = package->GetMetaClass ();

    /* get the GObject and cast as a GtkWidget */
    GtkWidget* window = GTK_WIDGET ( gtk_builder_get_object (builder, "ChangeApiInfoWindow") );
    gboolean visible = gtk_widget_is_visible ( window );

    if ( visible ){
        gtk_widget_set_visible ( window, false );
    } else {
        GtkWidget* EntryBox = GTK_WIDGET ( gtk_builder_get_object (builder, "ChangeApiInfoEquityUrlEntryBox") );
        gtk_entry_set_text ( GTK_ENTRY( EntryBox ), D->stock_url );
        gtk_widget_grab_focus ( EntryBox );
        g_object_set ( G_OBJECT ( EntryBox ), "activates-default", TRUE, NULL );

        EntryBox = GTK_WIDGET ( gtk_builder_get_object (builder, "ChangeApiInfoUrlKeyEntryBox") );
        gtk_entry_set_text ( GTK_ENTRY( EntryBox ), D->curl_key );
        g_object_set ( G_OBJECT ( EntryBox ), "activates-default", TRUE, NULL );

        GtkWidget* ComboBox = GTK_WIDGET ( gtk_builder_get_object (builder, "ChangeApiInfoUpPerMinComboBox") );
        gtk_combo_box_set_active ( GTK_COMBO_BOX( ComboBox ), (int)*D->updates_per_min_f);

        GtkWidget* SpinBox = GTK_WIDGET ( gtk_builder_get_object (builder, "ChangeApiInfoHoursSpinBox") );
        GtkAdjustment* Adjustment = gtk_spin_button_get_adjustment ( GTK_SPIN_BUTTON ( SpinBox ) );
        gtk_adjustment_set_value ( Adjustment, *D->updates_hours_f );
        g_object_set ( G_OBJECT ( SpinBox ), "activates-default", TRUE, NULL );

        gtk_widget_set_visible ( window, true );
    }
    return 0;
}

int APIOk (void *data)
{
    /* Unpack the package */
    portfolio_packet *package = (portfolio_packet*)data;
    equity_folder *F = package->GetEquityFolderClass ();
    meta *D = package->GetMetaClass ();

    GtkWidget* EntryBox = GTK_WIDGET ( gtk_builder_get_object (builder, "ChangeApiInfoEquityUrlEntryBox") );
    char* new = strdup( gtk_entry_get_text ( GTK_ENTRY( EntryBox ) ) );
    char* cur = strdup( D->stock_url );

    if( ( strcmp( cur, new ) != 0) ){
        SqliteAddAPIData("Stock_URL", new, D);
        free ( D->stock_url );
        D->stock_url = strdup ( new );
    }
    free( new );
    free( cur );

    EntryBox = GTK_WIDGET ( gtk_builder_get_object (builder, "ChangeApiInfoUrlKeyEntryBox") );
    new = strdup( gtk_entry_get_text ( GTK_ENTRY( EntryBox ) ) );
    cur = strdup( D->curl_key );

    if( ( strcmp( cur, new ) != 0) ){
        SqliteAddAPIData("URL_KEY", new, D);
        free ( D->curl_key );
        D->curl_key = strdup ( new );
    }
    free( new );
    free( cur );

    GtkWidget* ComboBox = GTK_WIDGET ( gtk_builder_get_object (builder, "ChangeApiInfoUpPerMinComboBox") );
    new = strdup( gtk_combo_box_text_get_active_text ( GTK_COMBO_BOX_TEXT ( ComboBox ) ) );
    float new_f = strtod( new, NULL );
    float cur_f = *D->updates_per_min_f;

    if( new_f != cur_f ){
        SqliteAddAPIData("Updates_Per_Min", new, D);
        *D->updates_per_min_f = (double)new_f;
    }
    free( new );

    GtkWidget* SpinBox = GTK_WIDGET ( gtk_builder_get_object (builder, "ChangeApiInfoHoursSpinBox") );
    GtkAdjustment* Adjustment = gtk_spin_button_get_adjustment ( GTK_SPIN_BUTTON ( SpinBox ) );
    new_f = (float)gtk_adjustment_get_value ( Adjustment );
    cur_f = *D->updates_hours_f;

    if( new_f != cur_f ){
        new = (char*)malloc( 10 );
        snprintf(new, 10, "%f", new_f);
        SqliteAddAPIData("Updates_Hours", new, D);
        free( new );
        *D->updates_hours_f = (double)new_f;
    }

    /* Generate the Equity Request URLs. */
    F->GenerateURL ( package );
    return 0;
}

int APICursorMove ()
{
    GtkWidget* Button = GTK_WIDGET ( gtk_builder_get_object (builder, "ChangeApiInfoOKBTN") );
    GtkWidget* EntryBox = GTK_WIDGET ( gtk_builder_get_object (builder, "ChangeApiInfoEquityUrlEntryBox") );
    char* Equity_URL = strdup( gtk_entry_get_text ( GTK_ENTRY( EntryBox ) ) );

    EntryBox = GTK_WIDGET ( gtk_builder_get_object (builder, "ChangeApiInfoUrlKeyEntryBox") );
    char* URL_KEY = strdup( gtk_entry_get_text ( GTK_ENTRY( EntryBox ) ) );

    EntryBox = GTK_WIDGET ( gtk_builder_get_object (builder, "ChangeApiInfoHoursSpinBox") );
    char* Updates_Hours = strdup( gtk_entry_get_text ( GTK_ENTRY( EntryBox ) ) );

    bool check = CheckValidString( Equity_URL ) & CheckValidString( URL_KEY );
    check = check & CheckValidString( Updates_Hours );
    check = check & ( strtod( Updates_Hours, NULL ) <= 7 ) & CheckIfStringDoublePositiveNumber( Updates_Hours );

    if ( check ){
        gtk_widget_set_sensitive ( Button, true );
    } else {
        gtk_widget_set_sensitive ( Button, false );
    }

    free ( Equity_URL );
    free ( URL_KEY );
    free ( Updates_Hours );
    return 0;
}

int BullionShowHide (void *data)
{
    /* Unpack the package */
    portfolio_packet *package = (portfolio_packet*)data;
    metal *M = package->GetMetalClass ();

    /* get the GObject and cast as a GtkWidget */
    GtkWidget* window = GTK_WIDGET ( gtk_builder_get_object (builder, "AddRemoveBullionWindow") );
    gboolean visible = gtk_widget_is_visible ( window );

    if ( visible ){
        gtk_widget_set_visible ( window, false );
    } else {
        /* Reset EntryBoxes */
        GtkWidget* EntryBox = GTK_WIDGET ( gtk_builder_get_object (builder, "AddRemoveBullionGoldOuncesEntryBox") );
        char *temp = strdup( M->Gold->ounce_ch );
        FormatStr( temp );
        gtk_entry_set_text ( GTK_ENTRY( EntryBox ), M->Gold->ounce_ch );
        free( temp );
        gtk_widget_grab_focus ( EntryBox );
        g_object_set ( G_OBJECT ( EntryBox ), "activates-default", TRUE, NULL );

        EntryBox = GTK_WIDGET ( gtk_builder_get_object (builder, "AddRemoveBullionGoldPremiumEntryBox") );
        temp = strdup( M->Gold->premium_ch );
        FormatStr( temp );
        gtk_entry_set_text ( GTK_ENTRY( EntryBox ), temp );
        free( temp );
        g_object_set ( G_OBJECT ( EntryBox ), "activates-default", TRUE, NULL );

        EntryBox = GTK_WIDGET ( gtk_builder_get_object (builder, "AddRemoveBullionSilverOuncesEntryBox") );
        temp = strdup( M->Silver->ounce_ch );
        FormatStr( temp );
        gtk_entry_set_text ( GTK_ENTRY( EntryBox ), temp );
        free( temp );
        g_object_set ( G_OBJECT ( EntryBox ), "activates-default", TRUE, NULL );

        EntryBox = GTK_WIDGET ( gtk_builder_get_object (builder, "AddRemoveBullionSilverPremiumEntryBox") );
        temp = strdup( M->Silver->premium_ch );
        FormatStr( temp );
        gtk_entry_set_text ( GTK_ENTRY( EntryBox ), temp );
        free( temp );
        g_object_set ( G_OBJECT ( EntryBox ), "activates-default", TRUE, NULL );

        gtk_widget_set_visible ( window, true );
    }
    return 0;
}

int BullionOk (void *data)
{
    /* Unpack the package */
    portfolio_packet *package = (portfolio_packet*)data;
    metal *M = package->GetMetalClass ();
    meta *D = package->GetMetaClass ();

    GtkWidget* EntryBox = GTK_WIDGET ( gtk_builder_get_object (builder, "AddRemoveBullionGoldOuncesEntryBox") );
    char* new_ounces = strdup( gtk_entry_get_text ( GTK_ENTRY( EntryBox ) ) );
    char* cur_ounces = strdup( M->Gold->ounce_ch );
    FormatStr( cur_ounces );
    EntryBox = GTK_WIDGET ( gtk_builder_get_object (builder, "AddRemoveBullionGoldPremiumEntryBox") );
    char* new_premium = strdup( gtk_entry_get_text ( GTK_ENTRY( EntryBox ) ) );
    char* cur_premium = strdup( M->Gold->premium_ch );
    FormatStr( cur_premium );

    if( (strcmp(cur_ounces, new_ounces) != 0) || (strcmp(cur_premium, new_premium) != 0) ){
        SqliteAddBullion("gold", new_ounces, new_premium, M, D);
    }

    free( new_ounces );
    free( cur_ounces );
    free( cur_premium );
    free( new_premium );

    EntryBox = GTK_WIDGET ( gtk_builder_get_object (builder, "AddRemoveBullionSilverOuncesEntryBox") );
    new_ounces = strdup( gtk_entry_get_text ( GTK_ENTRY( EntryBox ) ) );
    cur_ounces = strdup( M->Silver->ounce_ch );
    FormatStr( cur_ounces );
    EntryBox = GTK_WIDGET ( gtk_builder_get_object (builder, "AddRemoveBullionSilverPremiumEntryBox") );
    new_premium = strdup( gtk_entry_get_text ( GTK_ENTRY( EntryBox ) ) );
    cur_premium = strdup( M->Silver->premium_ch );
    FormatStr( cur_premium );

    if( (strcmp(cur_ounces, new_ounces) != 0) || (strcmp(cur_premium, new_premium) != 0) ){
        SqliteAddBullion("silver", new_ounces, new_premium, M, D);
    }

    free( new_ounces );
    free( cur_ounces );
    free( cur_premium );
    free( new_premium );

    return 0;
}

int BullionCursorMove ()
{
    GtkWidget* Button = GTK_WIDGET ( gtk_builder_get_object (builder, "AddRemoveBullionOKBTN") );
    GtkWidget* EntryBox = GTK_WIDGET ( gtk_builder_get_object (builder, "AddRemoveBullionGoldOuncesEntryBox") );
    char* Gold_Ounces = strdup( gtk_entry_get_text ( GTK_ENTRY( EntryBox ) ) );

    EntryBox = GTK_WIDGET ( gtk_builder_get_object (builder, "AddRemoveBullionGoldPremiumEntryBox") );
    char* Gold_Premium = strdup( gtk_entry_get_text ( GTK_ENTRY( EntryBox ) ) );

    EntryBox = GTK_WIDGET ( gtk_builder_get_object (builder, "AddRemoveBullionSilverOuncesEntryBox") );
    char* Silver_Ounces = strdup( gtk_entry_get_text ( GTK_ENTRY( EntryBox ) ) );

    EntryBox = GTK_WIDGET ( gtk_builder_get_object (builder, "AddRemoveBullionSilverPremiumEntryBox") );
    char* Silver_Premium = strdup( gtk_entry_get_text ( GTK_ENTRY( EntryBox ) ) );

    bool valid_num = CheckIfStringDoublePositiveNumber( Gold_Ounces ) & CheckIfStringDoublePositiveNumber( Gold_Premium );
    valid_num = valid_num & CheckIfStringDoublePositiveNumber( Silver_Ounces ) & CheckIfStringDoublePositiveNumber( Silver_Premium );
    
    bool valid_string = CheckValidString( Gold_Ounces ) & CheckValidString( Gold_Premium );
    valid_string = valid_string & CheckValidString( Silver_Ounces ) & CheckValidString( Silver_Premium );
    
    if ( valid_num && valid_string ){
        gtk_widget_set_sensitive ( Button, true );
    } else {
        gtk_widget_set_sensitive ( Button, false );
    }

    free ( Gold_Ounces );
    free ( Gold_Premium );
    free ( Silver_Ounces );
    free ( Silver_Premium );
    return 0;
}

int CashShowHide (void *data)
{
    /* Unpack the package */
    portfolio_packet *package = (portfolio_packet*)data;
    meta *D = package->GetMetaClass ();

    /* get the GObject and cast as a GtkWidget */
    GtkWidget* window = GTK_WIDGET ( gtk_builder_get_object (builder, "AddRemoveCashWindow") );
    gboolean visible = gtk_widget_is_visible ( window );

    if ( visible ){
        gtk_widget_set_visible ( window, false );
    } else {
        /* Reset EntryBoxes */
        GtkWidget* EntryBox = GTK_WIDGET ( gtk_builder_get_object (builder, "AddRemoveCashValueEntryBox") );
        
        char *temp = strdup( D->cash_ch );
        FormatStr( temp );
        gtk_entry_set_text ( GTK_ENTRY( EntryBox ), temp );
        free( temp );
        g_object_set ( G_OBJECT ( EntryBox ), "activates-default", TRUE, NULL );

        gtk_widget_grab_focus ( EntryBox );
        gtk_widget_set_visible ( window, true );
    }
    return 0;
}

int CashOk (void *data)
{
    /* Unpack the package */
    portfolio_packet *package = (portfolio_packet*)data;
    meta *D = package->GetMetaClass ();

    GtkWidget* EntryBox = GTK_WIDGET ( gtk_builder_get_object (builder, "AddRemoveCashValueEntryBox") );
    char* new_value = strdup( gtk_entry_get_text ( GTK_ENTRY( EntryBox ) ) );
    char* cur_value = strdup( D->cash_ch );
    FormatStr( cur_value );

    if( strcmp(cur_value, new_value) != 0 ){
        SqliteAddCash( new_value, D );
    }

    free( new_value );
    free( cur_value );

    return 0;
}

int CashCursorMove ()
{
    GtkWidget* Button = GTK_WIDGET ( gtk_builder_get_object (builder, "AddRemoveCashOKBTN") );
    GtkWidget* EntryBox = GTK_WIDGET ( gtk_builder_get_object (builder, "AddRemoveCashValueEntryBox") );

    char* value = strdup( gtk_entry_get_text ( GTK_ENTRY( EntryBox ) ) );

    if ( CheckIfStringDoublePositiveNumber( value ) && CheckValidString( value ) ){
        gtk_widget_set_sensitive ( Button, true );
    } else {
        gtk_widget_set_sensitive ( Button, false );
    }

    free ( value );
    return 0;
}

int AboutShowHide ()
{
    /* get the GObject and cast as a GtkWidget */
    GtkWidget* window = GTK_WIDGET ( gtk_builder_get_object (builder, "AboutWindow") );
    gboolean visible = gtk_widget_is_visible ( window );

    if ( visible ){
        gtk_widget_set_visible ( window, false );
    } else {
        gtk_widget_set_visible ( window, true );

        window = GTK_WIDGET ( gtk_builder_get_object (builder, "AboutScrolledWindow") );
        gtk_scrolled_window_set_vadjustment (GTK_SCROLLED_WINDOW( window ), NULL);
    }
    return 0;
}

int ShortcutShowHide ()
{
    /* get the GObject and cast as a GtkWidget */
    GtkWidget* window = GTK_WIDGET ( gtk_builder_get_object (builder, "ShortcutWindow") );
    gboolean visible = gtk_widget_is_visible ( window );

    if ( visible ){
        gtk_widget_set_visible ( window, false );
    } else {
        GtkWidget* button = GTK_WIDGET ( gtk_builder_get_object (builder, "ShortcutOKBTN") );
        gtk_widget_grab_focus ( button );

        gtk_widget_set_visible ( window, true );
    }
    return 0;
}