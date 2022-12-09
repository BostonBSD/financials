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
#include "../include/gui.h"                 /* MainPrimaryTreeview */

#include "../include/class_types.h"         /* portfolio_packet, equity_folder, metal, meta */
#include "../include/sqlite.h"
#include "../include/workfuncs.h"
#include "../include/mutex.h"
#include "../include/multicurl.h"

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
        GtkWidget* stack = GTK_WIDGET ( gtk_builder_get_object (builder, "ChangeAPIInfoStack") );
        gtk_stack_set_visible_child_name ( GTK_STACK ( stack ), "page0"  );

        GtkWidget* Switch = GTK_WIDGET ( gtk_builder_get_object (builder, "ChangeApiInfoShowClocksSwitch") );
        gtk_switch_set_active ( GTK_SWITCH( Switch ), *D->clocks_displayed_bool );

        GtkWidget* EntryBox = GTK_WIDGET ( gtk_builder_get_object (builder, "ChangeApiInfoEquityUrlEntryBox") );
        gtk_entry_set_text ( GTK_ENTRY( EntryBox ), D->stock_url_ch );
        gtk_widget_grab_focus ( EntryBox );
        g_object_set ( G_OBJECT ( EntryBox ), "activates-default", TRUE, NULL );

        EntryBox = GTK_WIDGET ( gtk_builder_get_object (builder, "ChangeApiInfoUrlKeyEntryBox") );
        gtk_entry_set_text ( GTK_ENTRY( EntryBox ), D->curl_key_ch );
        g_object_set ( G_OBJECT ( EntryBox ), "activates-default", TRUE, NULL );

        EntryBox = GTK_WIDGET ( gtk_builder_get_object (builder, "ChangeApiInfoNasdaqSymbolsUrlEntryBox") );
        gtk_entry_set_text ( GTK_ENTRY( EntryBox ), D->Nasdaq_Symbol_url_ch );
        g_object_set ( G_OBJECT ( EntryBox ), "activates-default", TRUE, NULL );

        EntryBox = GTK_WIDGET ( gtk_builder_get_object (builder, "ChangeApiInfoNYSESymbolsUrlEntryBox") );
        gtk_entry_set_text ( GTK_ENTRY( EntryBox ), D->NYSE_Symbol_url_ch );
        g_object_set ( G_OBJECT ( EntryBox ), "activates-default", TRUE, NULL );

        GtkWidget* ComboBox = GTK_WIDGET ( gtk_builder_get_object (builder, "ChangeApiInfoUpPerMinComboBox") );
        gtk_combo_box_set_active ( GTK_COMBO_BOX( ComboBox ), (int)*D->updates_per_min_f);

        GtkWidget* SpinBox = GTK_WIDGET ( gtk_builder_get_object (builder, "ChangeApiInfoHoursSpinBox") );
        GtkAdjustment* Adjustment = gtk_spin_button_get_adjustment ( GTK_SPIN_BUTTON ( SpinBox ) );
        gtk_adjustment_set_value ( Adjustment, *D->updates_hours_f );
        g_object_set ( G_OBJECT ( SpinBox ), "activates-default", TRUE, NULL );

        GtkWidget* label = GTK_WIDGET ( gtk_builder_get_object (builder, "ChangeApiInfoStockSymbolUpdateLabel") );
        gtk_widget_set_visible ( label, false );
        label = GTK_WIDGET ( gtk_builder_get_object (builder, "ChangeApiInfoStockSymbolUpdateSpinner") );
        gtk_widget_set_visible ( label, false );

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
    char* cur = strdup( D->stock_url_ch );

    if( ( strcmp( cur, new ) != 0) ){
        SqliteAddAPIData("Stock_URL", new, D);
        free ( D->stock_url_ch );
        D->stock_url_ch = strdup ( new );
    }
    free( new );
    free( cur );

    EntryBox = GTK_WIDGET ( gtk_builder_get_object (builder, "ChangeApiInfoUrlKeyEntryBox") );
    new = strdup( gtk_entry_get_text ( GTK_ENTRY( EntryBox ) ) );
    cur = strdup( D->curl_key_ch );

    if( ( strcmp( cur, new ) != 0) ){
        SqliteAddAPIData("URL_KEY", new, D);
        free ( D->curl_key_ch );
        D->curl_key_ch = strdup ( new );
    }
    free( new );
    free( cur );

    EntryBox = GTK_WIDGET ( gtk_builder_get_object (builder, "ChangeApiInfoNasdaqSymbolsUrlEntryBox") );
    new = strdup( gtk_entry_get_text ( GTK_ENTRY( EntryBox ) ) );
    cur = strdup( D->Nasdaq_Symbol_url_ch );

    if( ( strcmp( cur, new ) != 0) ){
        SqliteAddAPIData("Nasdaq_Symbol_URL", new, D);
        free ( D->Nasdaq_Symbol_url_ch );
        D->Nasdaq_Symbol_url_ch = strdup ( new );
    }
    free( new );
    free( cur );

    EntryBox = GTK_WIDGET ( gtk_builder_get_object (builder, "ChangeApiInfoNYSESymbolsUrlEntryBox") );
    new = strdup( gtk_entry_get_text ( GTK_ENTRY( EntryBox ) ) );
    cur = strdup( D->NYSE_Symbol_url_ch );

    if( ( strcmp( cur, new ) != 0) ){
        SqliteAddAPIData("NYSE_Symbol_URL", new, D);
        free ( D->NYSE_Symbol_url_ch );
        D->NYSE_Symbol_url_ch = strdup ( new );
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

    GtkWidget* Switch = GTK_WIDGET ( gtk_builder_get_object (builder, "ChangeApiInfoShowClocksSwitch") );
    bool switch_set = gtk_switch_get_active ( GTK_SWITCH( Switch ) );

    if( switch_set != *D->clocks_displayed_bool ){
        if ( switch_set ) {
            SqliteAddAPIData("Clocks_Displayed", "true", D);
        } else {
            SqliteAddAPIData("Clocks_Displayed", "false", D);
        }
        package->SetClockDisplayed ( switch_set );
        gdk_threads_add_idle ( MainDisplayClocks, package );
        gdk_threads_add_idle ( MainDisplayTime, NULL );
        gdk_threads_add_idle ( MainDisplayTimeRemaining, package );
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

    EntryBox = GTK_WIDGET ( gtk_builder_get_object (builder, "ChangeApiInfoNasdaqSymbolsUrlEntryBox") );
    char* Nasdaq_URL = strdup( gtk_entry_get_text ( GTK_ENTRY( EntryBox ) ) );

    EntryBox = GTK_WIDGET ( gtk_builder_get_object (builder, "ChangeApiInfoNYSESymbolsUrlEntryBox") );
    char* NYSE_URL = strdup( gtk_entry_get_text ( GTK_ENTRY( EntryBox ) ) );

    EntryBox = GTK_WIDGET ( gtk_builder_get_object (builder, "ChangeApiInfoHoursSpinBox") );
    char* Updates_Hours = strdup( gtk_entry_get_text ( GTK_ENTRY( EntryBox ) ) );

    bool check = CheckValidString( Equity_URL ) & CheckValidString( URL_KEY );
    check = check & CheckValidString( Nasdaq_URL ) & CheckValidString( NYSE_URL );
    check = check & CheckValidString( Updates_Hours );
    check = check & ( strtod( Updates_Hours, NULL ) <= 7 ) & CheckIfStringDoublePositiveNumber( Updates_Hours );

    if ( check ){
        gtk_widget_set_sensitive ( Button, true );
    } else {
        gtk_widget_set_sensitive ( Button, false );
    }

    free ( Equity_URL );
    free ( URL_KEY );
    free ( Nasdaq_URL );
    free ( NYSE_URL );
    free ( Updates_Hours );
    return 0;
}

int APIStartSpinner ()
{
    GtkWidget* label = GTK_WIDGET ( gtk_builder_get_object (builder, "ChangeApiInfoStockSymbolUpdateSpinner") );
    gtk_widget_set_visible ( label, true );
    gtk_spinner_start ( GTK_SPINNER ( label ) );

    label = GTK_WIDGET ( gtk_builder_get_object (builder, "ChangeApiInfoStockSymbolUpdateLabel") );
    gtk_widget_set_visible ( label, false );

    GtkWidget* button = GTK_WIDGET ( gtk_builder_get_object (builder, "ChangeApiInfoStockSymbolUpdateBTN") );
    gtk_widget_set_sensitive ( button, false );
    gtk_button_set_label ( GTK_BUTTON ( button ), "Please Wait..." );

    return 0;
}

int APIStopSpinner ()
{
    GtkWidget* label = GTK_WIDGET ( gtk_builder_get_object (builder, "ChangeApiInfoStockSymbolUpdateSpinner") );
    gtk_widget_set_visible ( label, false );
    gtk_spinner_stop ( GTK_SPINNER ( label ) );

    label = GTK_WIDGET ( gtk_builder_get_object (builder, "ChangeApiInfoStockSymbolUpdateLabel") );
    gtk_widget_set_visible ( label, true );

    GtkWidget* button = GTK_WIDGET ( gtk_builder_get_object (builder, "ChangeApiInfoStockSymbolUpdateBTN") );
    gtk_widget_set_sensitive ( button, true );
    gtk_button_set_label ( GTK_BUTTON ( button ), "Get Symbols" );

    return 0;
}

int BullionComBoxChange ()
{
    GtkWidget* ComboBox = GTK_WIDGET ( gtk_builder_get_object (builder, "AddRemoveBullionComboBox") );
    short index = gtk_combo_box_get_active ( GTK_COMBO_BOX ( ComboBox ) );
    GtkWidget* frame;
    enum { GOLD, SILVER, PLATINUM };

    switch ( index )
    {
        case GOLD:
            frame = GTK_WIDGET ( gtk_builder_get_object (builder, "AddRemoveBullionGoldFrame") );
            gtk_widget_set_visible ( frame, true );
            frame = GTK_WIDGET ( gtk_builder_get_object (builder, "AddRemoveBullionSilverFrame") );
            gtk_widget_set_visible ( frame, false );
            frame = GTK_WIDGET ( gtk_builder_get_object (builder, "AddRemoveBullionPlatinumFrame") );
            gtk_widget_set_visible ( frame, false );
            frame = GTK_WIDGET ( gtk_builder_get_object (builder, "AddRemoveBullionPalladiumFrame") );
            gtk_widget_set_visible ( frame, false );

            break;
        case SILVER:
            frame = GTK_WIDGET ( gtk_builder_get_object (builder, "AddRemoveBullionGoldFrame") );
            gtk_widget_set_visible ( frame, false );
            frame = GTK_WIDGET ( gtk_builder_get_object (builder, "AddRemoveBullionSilverFrame") );
            gtk_widget_set_visible ( frame, true );
            frame = GTK_WIDGET ( gtk_builder_get_object (builder, "AddRemoveBullionPlatinumFrame") );
            gtk_widget_set_visible ( frame, false );
            frame = GTK_WIDGET ( gtk_builder_get_object (builder, "AddRemoveBullionPalladiumFrame") );
            gtk_widget_set_visible ( frame, false );

            break;
        case PLATINUM:
            frame = GTK_WIDGET ( gtk_builder_get_object (builder, "AddRemoveBullionGoldFrame") );
            gtk_widget_set_visible ( frame, false );
            frame = GTK_WIDGET ( gtk_builder_get_object (builder, "AddRemoveBullionSilverFrame") );
            gtk_widget_set_visible ( frame, false );
            frame = GTK_WIDGET ( gtk_builder_get_object (builder, "AddRemoveBullionPlatinumFrame") );
            gtk_widget_set_visible ( frame, true );
            frame = GTK_WIDGET ( gtk_builder_get_object (builder, "AddRemoveBullionPalladiumFrame") );
            gtk_widget_set_visible ( frame, false );

            break;
        default: /* PALLADIUM */
            frame = GTK_WIDGET ( gtk_builder_get_object (builder, "AddRemoveBullionGoldFrame") );
            gtk_widget_set_visible ( frame, false );
            frame = GTK_WIDGET ( gtk_builder_get_object (builder, "AddRemoveBullionSilverFrame") );
            gtk_widget_set_visible ( frame, false );
            frame = GTK_WIDGET ( gtk_builder_get_object (builder, "AddRemoveBullionPlatinumFrame") );
            gtk_widget_set_visible ( frame, false );
            frame = GTK_WIDGET ( gtk_builder_get_object (builder, "AddRemoveBullionPalladiumFrame") );
            gtk_widget_set_visible ( frame, true );

            break;
    }
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

        EntryBox = GTK_WIDGET ( gtk_builder_get_object (builder, "AddRemoveBullionPlatinumOuncesEntryBox") );
        temp = strdup( M->Platinum->ounce_ch );
        FormatStr( temp );
        gtk_entry_set_text ( GTK_ENTRY( EntryBox ), temp );
        free( temp );
        g_object_set ( G_OBJECT ( EntryBox ), "activates-default", TRUE, NULL );

        EntryBox = GTK_WIDGET ( gtk_builder_get_object (builder, "AddRemoveBullionPlatinumPremiumEntryBox") );
        temp = strdup( M->Platinum->premium_ch );
        FormatStr( temp );
        gtk_entry_set_text ( GTK_ENTRY( EntryBox ), temp );
        free( temp );
        g_object_set ( G_OBJECT ( EntryBox ), "activates-default", TRUE, NULL );

        EntryBox = GTK_WIDGET ( gtk_builder_get_object (builder, "AddRemoveBullionPalladiumOuncesEntryBox") );
        temp = strdup( M->Palladium->ounce_ch );
        FormatStr( temp );
        gtk_entry_set_text ( GTK_ENTRY( EntryBox ), temp );
        free( temp );
        g_object_set ( G_OBJECT ( EntryBox ), "activates-default", TRUE, NULL );

        EntryBox = GTK_WIDGET ( gtk_builder_get_object (builder, "AddRemoveBullionPalladiumPremiumEntryBox") );
        temp = strdup( M->Palladium->premium_ch );
        FormatStr( temp );
        gtk_entry_set_text ( GTK_ENTRY( EntryBox ), temp );
        free( temp );
        g_object_set ( G_OBJECT ( EntryBox ), "activates-default", TRUE, NULL );

        GtkWidget *frame = GTK_WIDGET ( gtk_builder_get_object (builder, "AddRemoveBullionGoldFrame") );
        gtk_widget_set_visible ( frame, true );
        frame = GTK_WIDGET ( gtk_builder_get_object (builder, "AddRemoveBullionSilverFrame") );
        gtk_widget_set_visible ( frame, false );
        frame = GTK_WIDGET ( gtk_builder_get_object (builder, "AddRemoveBullionPlatinumFrame") );
        gtk_widget_set_visible ( frame, false );
        frame = GTK_WIDGET ( gtk_builder_get_object (builder, "AddRemoveBullionPalladiumFrame") );
        gtk_widget_set_visible ( frame, false );

        GtkWidget *combobox = GTK_WIDGET ( gtk_builder_get_object (builder, "AddRemoveBullionComboBox") );
        gtk_combo_box_set_active ( GTK_COMBO_BOX ( combobox ), 0 );

        gtk_widget_set_visible ( window, true );
    }
    return 0;
}

static void *fetch_data_for_new_bullion ( void *data )
{
    portfolio_packet *pkg = (portfolio_packet*)data;
    metal *M = pkg->GetMetalClass ();
    short num_metals = 2;
    if( *M->Platinum->ounce_f > 0 ) num_metals++;
    if( *M->Palladium->ounce_f > 0 ) num_metals++;

    pthread_mutex_lock( &mutex_working [ CLASS_MEMBER_MUTEX ] );

    M->SetUpCurl ( pkg );

    /* Perform the cURL requests simultaneously using multi-cURL. */
    int return_code = PerformMultiCurl( pkg->multicurl_main_hnd, (double)num_metals );
    if( return_code ){
        free ( M->Gold->CURLDATA.memory );
        free ( M->Silver->CURLDATA.memory );
        M->Gold->CURLDATA.memory = NULL;
        M->Silver->CURLDATA.memory = NULL;
        if ( M->Platinum->CURLDATA.memory ) free ( M->Platinum->CURLDATA.memory );
        if ( M->Palladium->CURLDATA.memory ) free ( M->Palladium->CURLDATA.memory );
        M->Platinum->CURLDATA.memory = NULL;
        M->Palladium->CURLDATA.memory = NULL;
    }

    M->ExtractData ();

    pthread_mutex_unlock( &mutex_working [ CLASS_MEMBER_MUTEX ] );

    /* The following two statements lock the CLASS_MEMBER_MUTEX */

    pkg->Calculate ();
    pkg->ToStrings ();
    
    /* Update the main window treeview. */
    gdk_threads_add_idle ( MainPrimaryTreeview, data );

    return NULL;
}

int BullionOk (void *data)
{
    /* Unpack the package */
    portfolio_packet *package = (portfolio_packet*)data;
    metal *M = package->GetMetalClass ();
    meta *D = package->GetMetaClass ();
    bool new_gold = false, new_silver = false, new_platinum = false, new_palladium = false;

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
        new_gold = true;
    }

    free( new_ounces );
    free( cur_ounces );
    free( new_premium );
    free( cur_premium );

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
        new_silver = true;
    }

    free( new_ounces );
    free( cur_ounces );
    free( new_premium );
    free( cur_premium );

    EntryBox = GTK_WIDGET ( gtk_builder_get_object (builder, "AddRemoveBullionPlatinumOuncesEntryBox") );
    new_ounces = strdup( gtk_entry_get_text ( GTK_ENTRY( EntryBox ) ) );
    cur_ounces = strdup( M->Platinum->ounce_ch );
    FormatStr( cur_ounces );
    EntryBox = GTK_WIDGET ( gtk_builder_get_object (builder, "AddRemoveBullionPlatinumPremiumEntryBox") );
    new_premium = strdup( gtk_entry_get_text ( GTK_ENTRY( EntryBox ) ) );
    cur_premium = strdup( M->Platinum->premium_ch );
    FormatStr( cur_premium );

    if( (strcmp(cur_ounces, new_ounces) != 0) || (strcmp(cur_premium, new_premium) != 0) ){
        SqliteAddBullion("platinum", new_ounces, new_premium, M, D);
        new_platinum = true;
    }

    free( new_ounces );
    free( cur_ounces );
    free( new_premium );
    free( cur_premium );

    EntryBox = GTK_WIDGET ( gtk_builder_get_object (builder, "AddRemoveBullionPalladiumOuncesEntryBox") );
    new_ounces = strdup( gtk_entry_get_text ( GTK_ENTRY( EntryBox ) ) );
    cur_ounces = strdup( M->Palladium->ounce_ch );
    FormatStr( cur_ounces );
    EntryBox = GTK_WIDGET ( gtk_builder_get_object (builder, "AddRemoveBullionPalladiumPremiumEntryBox") );
    new_premium = strdup( gtk_entry_get_text ( GTK_ENTRY( EntryBox ) ) );
    cur_premium = strdup( M->Palladium->premium_ch );
    FormatStr( cur_premium );

    if( (strcmp(cur_ounces, new_ounces) != 0) || (strcmp(cur_premium, new_premium) != 0) ){
        SqliteAddBullion("palladium", new_ounces, new_premium, M, D);
        new_palladium = true;
    }

    free( new_ounces );
    free( cur_ounces );
    free( new_premium );
    free( cur_premium );

    bool new_entry = new_gold || new_silver || new_platinum || new_palladium;
    /* Fetch the data in a separate thread */
    pthread_t thread_id;
    if ( new_entry && package->IsFetchingData () ) pthread_create ( &thread_id, NULL, fetch_data_for_new_bullion, package );

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

    EntryBox = GTK_WIDGET ( gtk_builder_get_object (builder, "AddRemoveBullionPlatinumOuncesEntryBox") );
    char* Platinum_Ounces = strdup( gtk_entry_get_text ( GTK_ENTRY( EntryBox ) ) );

    EntryBox = GTK_WIDGET ( gtk_builder_get_object (builder, "AddRemoveBullionPlatinumPremiumEntryBox") );
    char* Platinum_Premium = strdup( gtk_entry_get_text ( GTK_ENTRY( EntryBox ) ) );

    EntryBox = GTK_WIDGET ( gtk_builder_get_object (builder, "AddRemoveBullionPalladiumOuncesEntryBox") );
    char* Palladium_Ounces = strdup( gtk_entry_get_text ( GTK_ENTRY( EntryBox ) ) );

    EntryBox = GTK_WIDGET ( gtk_builder_get_object (builder, "AddRemoveBullionPalladiumPremiumEntryBox") );
    char* Palladium_Premium = strdup( gtk_entry_get_text ( GTK_ENTRY( EntryBox ) ) );

    bool valid_num = CheckIfStringDoublePositiveNumber( Gold_Ounces ) & CheckIfStringDoublePositiveNumber( Gold_Premium );
    valid_num = valid_num & CheckIfStringDoublePositiveNumber( Silver_Ounces ) & CheckIfStringDoublePositiveNumber( Silver_Premium );
    valid_num = valid_num & CheckIfStringDoublePositiveNumber( Platinum_Ounces ) & CheckIfStringDoublePositiveNumber( Platinum_Premium );
    valid_num = valid_num & CheckIfStringDoublePositiveNumber( Palladium_Ounces ) & CheckIfStringDoublePositiveNumber( Palladium_Premium );
    
    bool valid_string = CheckValidString( Gold_Ounces ) & CheckValidString( Gold_Premium );
    valid_string = valid_string & CheckValidString( Silver_Ounces ) & CheckValidString( Silver_Premium );
    valid_string = valid_string & CheckValidString( Platinum_Ounces ) & CheckValidString( Platinum_Premium );
    valid_string = valid_string & CheckValidString( Palladium_Ounces ) & CheckValidString( Palladium_Premium );
    
    if ( valid_num && valid_string ){
        gtk_widget_set_sensitive ( Button, true );
    } else {
        gtk_widget_set_sensitive ( Button, false );
    }

    free ( Gold_Ounces );
    free ( Gold_Premium );
    free ( Silver_Ounces );
    free ( Silver_Premium );
    free ( Platinum_Ounces );
    free ( Platinum_Premium );
    free ( Palladium_Ounces );
    free ( Palladium_Premium );
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
        gtk_widget_set_visible ( window, true );
    }
    return 0;
}