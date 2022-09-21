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

#include "../financials.h"

GtkBuilder *builder;
main_window_data MainWindowStruct;
symbol_to_security_name_container **security_symbol;
int symbolcount = 0;

int FetchDataBTNLabel (void* data){
    bool *fetching = (bool*)data;
    GtkWidget *Button = GTK_WIDGET ( gtk_builder_get_object (builder, "FetchDataBTN") );
    
    if( *fetching == true ){
        gtk_button_set_label ( GTK_BUTTON ( Button ), "Stop Fetching Data" );
    } else {
        gtk_button_set_label ( GTK_BUTTON ( Button ), "Fetch Data" );
    }
    
    return 0;
}

void UpDateProgressBarGUI (double *fraction) {
    if ( *fraction >= 0 ) gdk_threads_add_idle( UpDateProgressBar, fraction );
}

int UpDateProgressBar (void *data) {
    double *fraction = (double*)data;

    GtkWidget* ProgressBar = GTK_WIDGET ( gtk_builder_get_object (builder, "ProgressBar") );
    gtk_progress_bar_set_fraction ( GTK_PROGRESS_BAR( ProgressBar ), *fraction );
    return 0;
}

int ShowHideAPIWindow ()
{
    /* get the GObject and cast as a GtkWidget */
    GtkWidget* window = GTK_WIDGET ( gtk_builder_get_object (builder, "ChangeApiInfoWindow") );
    gboolean visible = gtk_widget_is_visible ( window );

    if ( visible ){
        gtk_widget_set_visible ( window, false );
    } else {
        GtkWidget* EntryBox = GTK_WIDGET ( gtk_builder_get_object (builder, "ChangeApiInfoEquityUrlEntryBox") );
        gtk_entry_set_text ( GTK_ENTRY( EntryBox ), MetaData->stock_url );
        gtk_widget_grab_focus ( EntryBox );
        g_object_set ( G_OBJECT ( EntryBox ), "activates-default", TRUE, NULL );

        EntryBox = GTK_WIDGET ( gtk_builder_get_object (builder, "ChangeApiInfoUrlKeyEntryBox") );
        gtk_entry_set_text ( GTK_ENTRY( EntryBox ), MetaData->curl_key );
        g_object_set ( G_OBJECT ( EntryBox ), "activates-default", TRUE, NULL );

        GtkWidget* ComboBox = GTK_WIDGET ( gtk_builder_get_object (builder, "ChangeApiInfoUpPerMinComboBox") );
        gtk_combo_box_set_active ( GTK_COMBO_BOX( ComboBox ), (int)*MetaData->updates_per_min_f);

        GtkWidget* SpinBox = GTK_WIDGET ( gtk_builder_get_object (builder, "ChangeApiInfoHoursSpinBox") );
        GtkAdjustment* Adjustment = gtk_spin_button_get_adjustment ( GTK_SPIN_BUTTON ( SpinBox ) );
        gtk_adjustment_set_value ( Adjustment, *MetaData->updates_hours_f );
        g_object_set ( G_OBJECT ( SpinBox ), "activates-default", TRUE, NULL );

        gtk_widget_set_visible ( window, true );
    }
    return 0;
}

int OKAPIWindow ()
{
    GtkWidget* EntryBox = GTK_WIDGET ( gtk_builder_get_object (builder, "ChangeApiInfoEquityUrlEntryBox") );
    char* new = strdup( gtk_entry_get_text ( GTK_ENTRY( EntryBox ) ) );
    char* cur = strdup( MetaData->stock_url );

    if( ( strcmp( cur, new ) != 0) ){
        SqliteAddAPIData("Stock_URL", new, MetaData);
    }
    free( new );
    free( cur );

    EntryBox = GTK_WIDGET ( gtk_builder_get_object (builder, "ChangeApiInfoUrlKeyEntryBox") );
    new = strdup( gtk_entry_get_text ( GTK_ENTRY( EntryBox ) ) );
    cur = strdup( MetaData->curl_key );

    if( ( strcmp( cur, new ) != 0) ){
        SqliteAddAPIData("URL_KEY", new, MetaData);
    }
    free( new );
    free( cur );

    GtkWidget* ComboBox = GTK_WIDGET ( gtk_builder_get_object (builder, "ChangeApiInfoUpPerMinComboBox") );
    new = strdup( gtk_combo_box_text_get_active_text ( GTK_COMBO_BOX_TEXT ( ComboBox ) ) );
    float new_f = strtod( new, NULL );
    float cur_f = *MetaData->updates_per_min_f;

    if( new_f != cur_f ){
        SqliteAddAPIData("Updates_Per_Min", new, MetaData);
    }
    free( new );

    GtkWidget* SpinBox = GTK_WIDGET ( gtk_builder_get_object (builder, "ChangeApiInfoHoursSpinBox") );
    GtkAdjustment* Adjustment = gtk_spin_button_get_adjustment ( GTK_SPIN_BUTTON ( SpinBox ) );
    new_f = (float)gtk_adjustment_get_value ( Adjustment );
    cur_f = *MetaData->updates_hours_f;

    if( new_f != cur_f ){
        new = (char*)malloc( 10 );
        snprintf(new, 10, "%f", new_f);
        SqliteAddAPIData("Updates_Hours", new, MetaData);
        free( new );
    }

    return 0;
}

int CursorMoveAPIEntryBoxGUI ()
{
    GtkWidget* Button = GTK_WIDGET ( gtk_builder_get_object (builder, "ChangeApiInfoOKBTN") );
    GtkWidget* EntryBox = GTK_WIDGET ( gtk_builder_get_object (builder, "ChangeApiInfoEquityUrlEntryBox") );
    char* Equity_URL = strdup( gtk_entry_get_text ( GTK_ENTRY( EntryBox ) ) );

    EntryBox = GTK_WIDGET ( gtk_builder_get_object (builder, "ChangeApiInfoUrlKeyEntryBox") );
    char* URL_KEY = strdup( gtk_entry_get_text ( GTK_ENTRY( EntryBox ) ) );

    EntryBox = GTK_WIDGET ( gtk_builder_get_object (builder, "ChangeApiInfoHoursSpinBox") );
    char* Updates_Hours = strdup( gtk_entry_get_text ( GTK_ENTRY( EntryBox ) ) );

    bool check = check_valid_string( Equity_URL ) & check_valid_string( URL_KEY );
    check = check & check_valid_string( Updates_Hours );
    check = check & ( strtod( Updates_Hours, NULL ) <= 7 ) & check_if_string_double_positive_number( Updates_Hours );

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

int ShowHideCashWindow ()
{
    /* get the GObject and cast as a GtkWidget */
    GtkWidget* window = GTK_WIDGET ( gtk_builder_get_object (builder, "AddRemoveCashWindow") );
    gboolean visible = gtk_widget_is_visible ( window );

    if ( visible ){
        gtk_widget_set_visible ( window, false );
    } else {
        /* Reset EntryBoxes */
        GtkWidget* EntryBox = GTK_WIDGET ( gtk_builder_get_object (builder, "AddRemoveCashValueEntryBox") );
        
        char *temp = strdup( MetaData->cash_ch );
        FormatStr( temp );
        gtk_entry_set_text ( GTK_ENTRY( EntryBox ), temp );
        free( temp );
        g_object_set ( G_OBJECT ( EntryBox ), "activates-default", TRUE, NULL );

        gtk_widget_grab_focus ( EntryBox );
        gtk_widget_set_visible ( window, true );
    }
    return 0;
}

int OKCashWindow ()
{
    GtkWidget* EntryBox = GTK_WIDGET ( gtk_builder_get_object (builder, "AddRemoveCashValueEntryBox") );
    char* new_value = strdup( gtk_entry_get_text ( GTK_ENTRY( EntryBox ) ) );
    char* cur_value = strdup( MetaData->cash_ch );
    FormatStr( cur_value );

    if( strcmp(cur_value, new_value) != 0 ){
        SqliteAddCash( new_value, MetaData );
    }

    free( new_value );
    free( cur_value );

    return 0;
}

int CursorMoveCashEntryBoxGUI ()
{
    GtkWidget* Button = GTK_WIDGET ( gtk_builder_get_object (builder, "AddRemoveCashOKBTN") );
    GtkWidget* EntryBox = GTK_WIDGET ( gtk_builder_get_object (builder, "AddRemoveCashValueEntryBox") );

    char* value = strdup( gtk_entry_get_text ( GTK_ENTRY( EntryBox ) ) );

    if ( check_if_string_double_number( value ) && check_valid_string( value ) ){
        gtk_widget_set_sensitive ( Button, true );
    } else {
        gtk_widget_set_sensitive ( Button, false );
    }

    free ( value );
    return 0;
}

int ShowHideBullionWindow ()
{
    /* get the GObject and cast as a GtkWidget */
    GtkWidget* window = GTK_WIDGET ( gtk_builder_get_object (builder, "AddRemoveBullionWindow") );
    gboolean visible = gtk_widget_is_visible ( window );

    if ( visible ){
        gtk_widget_set_visible ( window, false );
    } else {
        /* Reset EntryBoxes */
        GtkWidget* EntryBox = GTK_WIDGET ( gtk_builder_get_object (builder, "AddRemoveBullionGoldOuncesEntryBox") );
        char *temp = strdup( Precious->Gold->ounce_ch );
        FormatStr( temp );
        gtk_entry_set_text ( GTK_ENTRY( EntryBox ), Precious->Gold->ounce_ch );
        free( temp );
        gtk_widget_grab_focus ( EntryBox );
        g_object_set ( G_OBJECT ( EntryBox ), "activates-default", TRUE, NULL );

        EntryBox = GTK_WIDGET ( gtk_builder_get_object (builder, "AddRemoveBullionGoldPremiumEntryBox") );
        temp = strdup( Precious->Gold->premium_ch );
        FormatStr( temp );
        gtk_entry_set_text ( GTK_ENTRY( EntryBox ), temp );
        free( temp );
        g_object_set ( G_OBJECT ( EntryBox ), "activates-default", TRUE, NULL );

        EntryBox = GTK_WIDGET ( gtk_builder_get_object (builder, "AddRemoveBullionSilverOuncesEntryBox") );
        temp = strdup( Precious->Silver->ounce_ch );
        FormatStr( temp );
        gtk_entry_set_text ( GTK_ENTRY( EntryBox ), temp );
        free( temp );
        g_object_set ( G_OBJECT ( EntryBox ), "activates-default", TRUE, NULL );

        EntryBox = GTK_WIDGET ( gtk_builder_get_object (builder, "AddRemoveBullionSilverPremiumEntryBox") );
        temp = strdup( Precious->Silver->premium_ch );
        FormatStr( temp );
        gtk_entry_set_text ( GTK_ENTRY( EntryBox ), temp );
        free( temp );
        g_object_set ( G_OBJECT ( EntryBox ), "activates-default", TRUE, NULL );

        gtk_widget_set_visible ( window, true );
    }
    return 0;
}

int OKBullionWindow ()
{
    GtkWidget* EntryBox = GTK_WIDGET ( gtk_builder_get_object (builder, "AddRemoveBullionGoldOuncesEntryBox") );
    char* new_ounces = strdup( gtk_entry_get_text ( GTK_ENTRY( EntryBox ) ) );
    char* cur_ounces = strdup( Precious->Gold->ounce_ch );
    FormatStr( cur_ounces );
    EntryBox = GTK_WIDGET ( gtk_builder_get_object (builder, "AddRemoveBullionGoldPremiumEntryBox") );
    char* new_premium = strdup( gtk_entry_get_text ( GTK_ENTRY( EntryBox ) ) );
    char* cur_premium = strdup( Precious->Gold->premium_ch );
    FormatStr( cur_premium );

    if( (strcmp(cur_ounces, new_ounces) != 0) || (strcmp(cur_premium, new_premium) != 0) ){
        SqliteAddBullion("gold", new_ounces, new_premium, Precious);
    }

    free( new_ounces );
    free( cur_ounces );
    free( cur_premium );
    free( new_premium );

    EntryBox = GTK_WIDGET ( gtk_builder_get_object (builder, "AddRemoveBullionSilverOuncesEntryBox") );
    new_ounces = strdup( gtk_entry_get_text ( GTK_ENTRY( EntryBox ) ) );
    cur_ounces = strdup( Precious->Silver->ounce_ch );
    FormatStr( cur_ounces );
    EntryBox = GTK_WIDGET ( gtk_builder_get_object (builder, "AddRemoveBullionSilverPremiumEntryBox") );
    new_premium = strdup( gtk_entry_get_text ( GTK_ENTRY( EntryBox ) ) );
    cur_premium = strdup( Precious->Silver->premium_ch );
    FormatStr( cur_premium );

    if( (strcmp(cur_ounces, new_ounces) != 0) || (strcmp(cur_premium, new_premium) != 0) ){
        SqliteAddBullion("silver", new_ounces, new_premium, Precious);
    }

    free( new_ounces );
    free( cur_ounces );
    free( cur_premium );
    free( new_premium );

    return 0;
}

int CursorMoveBullionEntryBoxGUI ()
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

    bool valid_num = check_if_string_double_positive_number( Gold_Ounces ) & check_if_string_double_number( Gold_Premium );
    valid_num = valid_num & check_if_string_double_positive_number( Silver_Ounces ) & check_if_string_double_number( Silver_Premium );
    
    bool valid_string = check_valid_string( Gold_Ounces ) & check_valid_string( Gold_Premium );
    valid_string = valid_string & check_valid_string( Silver_Ounces ) & check_valid_string( Silver_Premium );
    
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

int ShowHideAboutWindow ()
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

int ShowHideShortcutWindow ()
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

int CursorMoveAddRemoveSecurityEntryBoxGUI ()
{
    GtkWidget* Button = GTK_WIDGET ( gtk_builder_get_object (builder, "AddRemoveSecurityOkBTN") );
    GtkWidget* EntryBoxSymbol = GTK_WIDGET ( gtk_builder_get_object (builder, "AddRemoveSecuritySymbolEntryBox") );
    GtkWidget* EntryBoxShares = GTK_WIDGET ( gtk_builder_get_object (builder, "AddRemoveSecuritySharesEntryBox") );

    char* symbol = strdup( gtk_entry_get_text ( GTK_ENTRY( EntryBoxSymbol ) ) );
    char* shares = strdup( gtk_entry_get_text ( GTK_ENTRY( EntryBoxShares ) ) );

    if ( check_valid_string( symbol ) && check_valid_string( shares ) && check_if_string_long_positive_number( shares ) ){
        gtk_widget_set_sensitive ( Button, true );
    } else {
        gtk_widget_set_sensitive ( Button, false );
    }

    free ( symbol );
    free ( shares );
    return 0;
}

int ChangedAddRemoveSecurityComboBox ()
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

int SwitchChangeAddRemoveSecurityWindow ()
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

int OKAddSecurityAddRemoveSecurityWindow ()
{
    GtkWidget* EntryBox = GTK_WIDGET ( gtk_builder_get_object (builder, "AddRemoveSecuritySymbolEntryBox") );
    char* symbol = strdup( gtk_entry_get_text ( GTK_ENTRY( EntryBox ) ) );

    EntryBox = GTK_WIDGET ( gtk_builder_get_object (builder, "AddRemoveSecuritySharesEntryBox") );
    char* shares = strdup( gtk_entry_get_text ( GTK_ENTRY( EntryBox ) ) );

    UpperCaseStr( symbol );
    FormatStr( shares );
        
    SqliteAddEquity( symbol, shares, Folder );

    free( symbol );
    free( shares );

    return 0;
}

int OKRemoveSecurityAddRemoveSecurityWindow ()
{
    GtkWidget* ComboBox = GTK_WIDGET ( gtk_builder_get_object (builder, "AddRemoveSecurityComboBox") );
    int index = gtk_combo_box_get_active ( GTK_COMBO_BOX ( ComboBox ) );
    if( index == 0 ) return 0;
    if( index == 1) {     
        SqliteRemoveAllEquity( Folder );
    } else {
        char* symbol = gtk_combo_box_text_get_active_text ( GTK_COMBO_BOX_TEXT ( ComboBox ) );
        SqliteRemoveEquity( symbol, Folder );
        free( symbol );
    }
    return 0;
}

int OKSecurityAddRemoveSecurityWindow ()
{
    GtkWidget* Switch = GTK_WIDGET ( gtk_builder_get_object (builder, "AddRemoveSecuritySwitch") );
    gboolean SwitchSet = gtk_switch_get_active ( GTK_SWITCH( Switch ) );

    if ( SwitchSet == false ) {
        OKAddSecurityAddRemoveSecurityWindow ();
    } else {
        OKRemoveSecurityAddRemoveSecurityWindow ();
    }

    return 0;
}

int ShowHideAddRemoveSecurityWindow ()
{
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

        if( Folder->size > 0 ){
            gtk_combo_box_text_append ( GTK_COMBO_BOX_TEXT ( ComboBox ), NULL, "Remove All Securities" );
        }

        for(short i = 0; i < Folder->size; i++){
            gtk_combo_box_text_append ( GTK_COMBO_BOX_TEXT ( ComboBox ), NULL, Folder->Equity[ i ]->symbol_stock_ch );
        }

        gtk_combo_box_set_active ( GTK_COMBO_BOX ( ComboBox ), 0 );

        gtk_widget_set_visible ( window, true );
    } else {
        gtk_widget_set_visible ( window, false );
    }

    return 0;
}

void symbol_security_name_map_destruct () {
    if ( security_symbol ){
        for(int i=0; i<symbolcount; i++ ){
            /* Free the string members */
            free( security_symbol[ i ]->symbol );
            free( security_symbol[ i ]->security_name );
            /* Free the memory that the address is pointing to */
            free( security_symbol[ i ] );
        }
        /* Free the array of pointer addresses */
        free( security_symbol );
        security_symbol = NULL;
        symbolcount = 0;
    }
}

gboolean ViewRSICompletionMatchFunc( GtkEntryCompletion *completion, const gchar *key, GtkTreeIter *iter ) {
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

GtkListStore *CompletionSymbolFetch ()
/* This function is only meant to be run once, at application startup. */
{
    GtkListStore *store = gtk_list_store_new(3, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);
    char *tofree;
    GtkTreeIter iter;

    char *line = malloc ( 1024 ), *line_start;
	char *token;
	char *output, *tmp, *original;
	short firstline = 1;
	line_start = line;

    security_symbol = malloc ( 1 );
    symbol_to_security_name_container **sec_sym_tmp = NULL;

	CURLM *mult_hnd = SetUpMultiCurlHandle ();
    char* Nasdaq_Url = NASDAQ_SYMBOL_URL; 
    char* NYSE_Url = NYSE_SYMBOL_URL; 
   	MemType Nasdaq_Struct, NYSE_Struct;
    SetUpCurlHandle( mult_hnd, Nasdaq_Url, &Nasdaq_Struct );
    SetUpCurlHandle( mult_hnd, NYSE_Url, &NYSE_Struct );
    if ( PerformMultiCurl_no_prog( mult_hnd ) != 0 ) { free( line ); free( security_symbol ); return store; }

    /* Convert a String to a File Pointer Stream for Reading */
    FILE* fp[2];
    fp[0] = fmemopen( (void*)Nasdaq_Struct.memory, strlen( Nasdaq_Struct.memory ) + 1, "r" );
    fp[1] = fmemopen( (void*)NYSE_Struct.memory, strlen( NYSE_Struct.memory ) + 1, "r" );
    
    short k = 0;
	while( k < 2 ){
        /* Initialize the output string handle. */
        output = malloc ( 1 );

	    /* Get rid of the header line from the file stream */
	    if ( fgets( line, 1024, fp[k] ) == NULL ) {
            	free( output );
            	fclose( fp[0] ); 
                fclose( fp[1] );
                symbol_security_name_map_destruct ();
            	return 0;
        }

        /* Read the file stream one line at a time */
        while ( fgets( line, 1024, fp[k] ) != NULL ) {
            
            /* Extract the symbol from the line. */
           	if( ( token = strsep( &line, "|" ) ) != NULL ){
           		original = strdup ( output );
           		tmp = realloc( output, strlen ( token ) + strlen ( output ) + 2 );
           		output = tmp;

           		if ( firstline ) {
           			snprintf( output, strlen ( token ) + 2, "%s", token );
           			firstline = 0;
           		} else {
           			snprintf( output, strlen ( token ) + strlen ( output ) + 2, "%s|%s", original, token );
           		}

           		free ( original );
           	}

            /* Extract the security name from the line. */
           	if( ( token = strsep( &line, "|" ) ) != NULL ){
           		original = strdup ( output );
           		tmp = realloc( output, strlen ( token ) + strlen ( output ) + 2 );
           		output = tmp;

           		snprintf( output, strlen ( token ) + strlen ( output ) + 2, "%s|%s", original, token );

           		free ( original );
                /* strsep moves the line pointer, we need to reset it so the pointer memory can be reused */
                line = line_start;
           	}
        }
        /* We aren't sure of the current location of the line pointer 
           [the server's data places arbitrary separators at the end].
           so we reset the line pointer before reading a second file. */
        line = line_start;
        firstline = 1;
        fclose( fp[k] );    

        /* Delete the last three entries from the output string */
        tmp = strrchr( output, '|' );
        *tmp = 0;
        tmp = strrchr( output, '|' );
        *tmp = 0;
        tmp = strrchr( output, '|' );
        *tmp = 0;

        /* Populate the Security Symbol Array. The second list is concatenated after the first list. */
        tofree = output;
        bool symbol = true;
        while ( ( token = strsep( &output, "|" ) ) != NULL ) {
            if( symbol ){
                /* Add another pointer address to the array */
                sec_sym_tmp = realloc( security_symbol, sizeof(symbol_to_security_name_container*) * (symbolcount + 1) );
                assert( sec_sym_tmp );
                security_symbol = sec_sym_tmp;
                /* Allocate memory for that pointer address */
                security_symbol[ symbolcount ] = malloc( sizeof(symbol_to_security_name_container) );
                /* Populate the memory with the character string */
                security_symbol[ symbolcount ]->symbol = strdup( token );
                symbol = false;

            } else {
                /* Populate the memory with the character string and increment the symbolcount */
                security_symbol[ symbolcount++ ]->security_name = strdup( token );
                symbol = true;

            }
        }

        /* Free the output string, so the handle can be reused on the second list. */
        free( tofree );
        k++;
    } /* end while loop */
    free( line );

    /* Sort the security symbol array, this merges both lists into one sorted list. */
    qsort( &security_symbol[ 0 ], symbolcount, sizeof(symbol_to_security_name_container*), AlphaAscSecName );

    char item[35];
    /* Populate the GtkListStore with the string of stock symbols in column 0, stock names in column 1, 
       and symbols & names in column 2. */
    for ( int i=0; i<symbolcount; i++ ){
        snprintf(item, 35, "%s - %s", security_symbol[ i ]->symbol, security_symbol[ i ]->security_name );

        gtk_list_store_append( store, &iter );
        /* Completion is going to match off of columns 0 and 1, but display column 2 */
        /* Completion matches based off of the symbol or the company name, inserts the symbol, displays both */
        gtk_list_store_set( store, &iter, 0, security_symbol[ i ]->symbol, 1, security_symbol[ i ]->security_name, 2, item, -1 );
    }

    free( Nasdaq_Struct.memory );
    free( NYSE_Struct.memory );
    return store;
}

int ViewRSICompletionSet (){
    GtkWidget* EntryBox = GTK_WIDGET ( gtk_builder_get_object (builder, "ViewRSISymbolEntryBox") );
    GtkEntryCompletion *completion = gtk_entry_completion_new();
    GtkListStore *store = CompletionSymbolFetch ();

    gtk_entry_completion_set_model(completion, GTK_TREE_MODEL( store ));
    g_object_unref( store );
    gtk_entry_completion_set_match_func(completion, (GtkEntryCompletionMatchFunc)ViewRSICompletionMatchFunc, NULL, NULL);
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

    return 0;
}

int ShowHideViewRSIWindow ()
{
    /* get the GObject and cast as a GtkWidget */
    GtkWidget* window = GTK_WIDGET ( gtk_builder_get_object (builder, "ViewRSIWindow") );
    gboolean visible = gtk_widget_is_visible ( window );
    
    if ( visible ){
        gtk_widget_set_visible ( window, false );
    } else {
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
    char* s = strdup( gtk_entry_get_text ( GTK_ENTRY( EntryBox ) ) );

    if( check_valid_string ( s ) ){
        gtk_widget_set_sensitive ( Button, true );
    } else {
        gtk_widget_set_sensitive ( Button, false );
    }

    free( s );    
    return 0;
}

int RSITreeViewClear(){
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

int RSISetColumns ()
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
    column = gtk_tree_view_column_new_with_attributes("Change ($)", renderer, "text", RSI_CHANGE, "foreground", RSI_FOREGROUND_COLOR, NULL);
    gtk_tree_view_column_set_resizable (column, true);
    gtk_tree_view_append_column(GTK_TREE_VIEW(list), column);

    renderer = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes("Change (%)", renderer, "text", RSI_CHANGE_PERCENT, "foreground", RSI_FOREGROUND_COLOR, NULL);
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

    return 0;
}

void RSIPeriod(time_t *currenttime, time_t *starttime){
    
    /* Number of Seconds in a Year Plus Three Weeks */
    int period = 31557600 + ( 604800 * 3 ); 

    time( currenttime );
    *starttime = *currenttime - (time_t)period;

}

int symsearchfunc(const void * a, const void * b){
    /* Cast the void pointer to a char pointer. */
   char* aa = (char *)a;
   /* Cast the void pointer to a double struct pointer. */
   symbol_to_security_name_container** bb = (symbol_to_security_name_container **)b;

   return strcasecmp( aa, bb[0]->symbol ); 
}

char *GetSecurityNameFromMapping(char *s)
/* Look for a Security Name using the Symbol as a key value.
   Must free return value. */
{
    symbol_to_security_name_container **item = NULL;

    /* The second parameter is the same type as the return value. A double pointer to a struct. */
    /* It's basically searching through an array of pointer addresses, the compare function dereferences
       the pointer address to get the string we are comparing against. */
    /* The array must already be sorted for bsearch to work. */
    item = (symbol_to_security_name_container**) bsearch (s, &security_symbol[0], symbolcount, sizeof (symbol_to_security_name_container*), symsearchfunc);

    if ( item != NULL ){
        /* The item pointer is not freed. It points to an item in the 
           security_symbol array and not a duplicate. */
        return strdup( item[ 0 ]->security_name );
    } else {
        return NULL;
    }

}

char* RSIGetSymbol()
/* Get the stock symbol from the EntryBox. Get the security name using the symbol.
   Set the security name to a Gtklabel, return the symbol.
   Must free return value. */
{
    GtkWidget* EntryBox = GTK_WIDGET ( gtk_builder_get_object (builder, "ViewRSISymbolEntryBox") );
    char* s = strdup( gtk_entry_get_text ( GTK_ENTRY( EntryBox ) ) );
    UpperCaseStr ( s );

    GtkWidget* Label = GTK_WIDGET ( gtk_builder_get_object (builder, "ViewRSIStockSymbolLabel") );

    char* sec_name = GetSecurityNameFromMapping( s );

    if ( sec_name != NULL ){
        gtk_label_set_text ( GTK_LABEL ( Label ), sec_name );
        free ( sec_name );
    } else {
        gtk_label_set_text ( GTK_LABEL ( Label ), "Unknown Security Name" );
    }

    return s;
}

char* RSIGetURL(){
    time_t start, end;
    size_t len;

    RSIPeriod( &end, &start );
    char *symbol_ch = RSIGetSymbol();

    len = strlen( symbol_ch ) + strlen( YAHOO_URL_START ) + strlen( YAHOO_URL_MIDDLE_ONE ) + strlen( YAHOO_URL_MIDDLE_TWO ) + strlen( YAHOO_URL_END ) + 25;
    char* Url = malloc ( len );
    snprintf(Url, len, YAHOO_URL_START"%s"YAHOO_URL_MIDDLE_ONE"%d"YAHOO_URL_MIDDLE_TWO"%d"YAHOO_URL_END, symbol_ch, (int)start, (int)end);

    free( symbol_ch );
    return Url;
}

void SetRSIStore (GtkListStore *store) {
    double gain, avg_gain = 0, avg_loss = 0, cur_price, prev_price, rsi, change;
    char *price_ch, *high_ch, *low_ch, *opening_ch, *prev_closing_ch, *change_ch, *indicator_ch;
    char gain_ch[10], rsi_ch[10], volume_ch[15];
    char *MyUrl = RSIGetURL ();
    long volume;
    int *new_order = (int*)malloc (1), *tmp;

    GtkTreeIter iter;

    CURLM *mult_hnd = SetUpMultiCurlHandle();
    MemType MyOutputStruct;
    SetUpCurlHandle( mult_hnd, MyUrl, &MyOutputStruct );
    if ( PerformMultiCurl_no_prog( mult_hnd ) != 0 ) { free( new_order ); free ( MyUrl ); return; }
    free ( MyUrl );

    if( MyOutputStruct.memory == NULL ) { free( new_order ); return; }

    /* Convert a String to a File Pointer Stream for Reading */
    FILE* fp = fmemopen( (void*)MyOutputStruct.memory, strlen( MyOutputStruct.memory ) + 1, "r" );
    
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
    
    chomp( line );
    csv_array = parse_csv( line );
    prev_price = strtod( csv_array[ 4 ], NULL );
    free_csv_line( csv_array );

    while ( fgets( line, 1024, fp) != NULL ) {       

        chomp( line );
    	csv_array = parse_csv( line );
    	
    	cur_price = strtod( csv_array[ 4 ], NULL );
    	gain = calc_gain ( cur_price, prev_price );
        change = cur_price - prev_price;
        prev_closing_ch = MetaData->DoubToStr( &prev_price );
    	prev_price = cur_price;
    	if ( counter < 14 ) summation ( gain, &avg_gain, &avg_loss);
    	if ( counter == 13 ) { avg_gain = avg_gain / 14; avg_loss = avg_loss / 14; }
    	if ( counter >= 14 ) {
            tmp = (int*)realloc( new_order, sizeof(int) * (c + 1) );
            new_order = tmp;

    		calc_avg ( gain, &avg_gain, &avg_loss);
    		rsi = calc_rsi ( avg_gain, avg_loss );
            /* The rsi_indicator return value is stored in the stack, do not free. */
            indicator_ch = rsi_indicator ( rsi );

            price_ch = StringToMonetary( csv_array[ 4 ] );
            high_ch = StringToMonetary( csv_array[ 2 ] );
            low_ch = StringToMonetary( csv_array[ 3 ] );
            opening_ch = StringToMonetary( csv_array[ 1 ] );
            change_ch = MetaData->DoubToStr( &change );
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

            c++;
    	}        
    	
        free( prev_closing_ch );
    	free_csv_line( csv_array );
    	counter++;
    }
    
    fclose( fp );
    free( MyOutputStruct.memory ); 
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
        free( new_order );
    }
}

GtkListStore* MakeRSIStore ()
{ 
    GtkListStore *store;

    /* Set up the storage container with the number of columns and column type */
    store = gtk_list_store_new(RSI_N_COLUMNS, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);

    /* Add data to the storage container. */
    SetRSIStore (store); 
    return store;
}

int RSIMakeGUI ()
{
    GtkListStore *store = NULL;
    GtkWidget* list = GTK_WIDGET ( gtk_builder_get_object (builder, "ViewRSITreeView") );

    /* Set the columns for the new TreeView model */
    RSISetColumns ();

    /* Set up the storage container */
    store = MakeRSIStore ();   
    
    /* Add the store of data to the list. */
    gtk_tree_view_set_model(GTK_TREE_VIEW(list), GTK_TREE_MODEL(store));
    g_object_unref( store );
   
    /* Set the list header as visible. */
    gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(list), TRUE);

    /* Remove Grid Lines. */
    gtk_tree_view_set_grid_lines ( GTK_TREE_VIEW ( list ), GTK_TREE_VIEW_GRID_LINES_NONE );
    return 0;
}

int TreeViewClear(){
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

int SetColumns (int column_type)
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

    if( column_type == GUI_COLUMN_ONE ){
        /* if GUI one */
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

GtkListStore * MakeGUIOneStore (){
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
    gtk_list_store_set ( store, &iter, GUI_TYPE, "bullion", GUI_SYMBOL, "Metal", GUI_SHARES_OUNCES, "Ounces", GUI_PREMIUM, "Premium", GUI_PRICE, "Spot Price", GUI_TOTAL, "Total", -1 );
    gtk_list_store_append ( store, &iter );
    gtk_list_store_set ( store, &iter, GUI_TYPE, "bullion", GUI_SYMBOL, "Gold", GUI_SHARES_OUNCES, Precious->Gold->ounce_ch, GUI_PREMIUM, Precious->Gold->premium_ch, GUI_PRICE, Precious->Gold->spot_price_ch, GUI_TOTAL, Precious->Gold->port_value_ch, -1 );
    gtk_list_store_append ( store, &iter );
    gtk_list_store_set ( store, &iter, GUI_TYPE, "bullion", GUI_SYMBOL, "Silver", GUI_SHARES_OUNCES, Precious->Silver->ounce_ch, GUI_PREMIUM, Precious->Silver->premium_ch, GUI_PRICE, Precious->Silver->spot_price_ch, GUI_TOTAL, Precious->Silver->port_value_ch, -1 );
    gtk_list_store_append ( store, &iter );
    gtk_list_store_set ( store, &iter, GUI_SYMBOL, "_______", -1 );
    gtk_list_store_append ( store, &iter );
    gtk_list_store_set ( store, &iter, GUI_TYPE, "equity_total", GUI_SYMBOL, "Equity", -1 );
    gtk_list_store_append ( store, &iter );
    gtk_list_store_set ( store, &iter, GUI_SYMBOL, "_______", -1 );
    gtk_list_store_append ( store, &iter );
    gtk_list_store_set ( store, &iter, GUI_TYPE, "equity_total", GUI_SYMBOL, "Symbol", GUI_SHARES_OUNCES, "Shares", GUI_PREMIUM, "Price", GUI_PRICE, "High", GUI_TOTAL, "Low", GUI_EXTRA_ONE, "Opening", GUI_EXTRA_TWO, "Prev Closing", GUI_EXTRA_THREE, "Ch/Share", GUI_EXTRA_FOUR, "Gain ($)", GUI_EXTRA_FIVE, "Total", GUI_EXTRA_SIX, "Gain (%)", -1 );
    
    unsigned short c = 0;
    while(c < Folder->size){
        snprintf(shares, 13, "%d",*Folder->Equity[ c ]->num_shares_stock_int);
        gtk_list_store_append ( store, &iter );

        if( *Folder->Equity[ c ]->change_value_f == 0 ) {
            gtk_list_store_set ( store, &iter, GUI_TYPE, "equity", GUI_SYMBOL, Folder->Equity[ c ]->symbol_stock_ch, GUI_SHARES_OUNCES, shares, GUI_PREMIUM, Folder->Equity[ c ]->current_price_stock_ch, GUI_PRICE, Folder->Equity[ c ]->high_stock_ch, GUI_TOTAL, Folder->Equity[ c ]->low_stock_ch, GUI_EXTRA_ONE, Folder->Equity[ c ]->opening_stock_ch, GUI_EXTRA_TWO, Folder->Equity[ c ]->prev_closing_stock_ch, GUI_EXTRA_THREE, Folder->Equity[ c ]->change_share_ch, GUI_EXTRA_FOUR, Folder->Equity[ c ]->change_value_ch, GUI_EXTRA_FIVE, Folder->Equity[ c ]->current_investment_stock_ch, GUI_EXTRA_SIX, Folder->Equity[ c ]->change_percent_ch, -1 );
        } else if( *Folder->Equity[ c ]->change_value_f > 0 ) {
            gtk_list_store_set ( store, &iter, GUI_FOREGROUND_COLOR, "Green", GUI_TYPE, "equity", GUI_SYMBOL, Folder->Equity[ c ]->symbol_stock_ch, GUI_SHARES_OUNCES, shares, GUI_PREMIUM, Folder->Equity[ c ]->current_price_stock_ch, GUI_PRICE, Folder->Equity[ c ]->high_stock_ch, GUI_TOTAL, Folder->Equity[ c ]->low_stock_ch, GUI_EXTRA_ONE, Folder->Equity[ c ]->opening_stock_ch, GUI_EXTRA_TWO, Folder->Equity[ c ]->prev_closing_stock_ch, GUI_EXTRA_THREE, Folder->Equity[ c ]->change_share_ch, GUI_EXTRA_FOUR, Folder->Equity[ c ]->change_value_ch, GUI_EXTRA_FIVE, Folder->Equity[ c ]->current_investment_stock_ch, GUI_EXTRA_SIX, Folder->Equity[ c ]->change_percent_ch, -1 );
        } else {
            gtk_list_store_set ( store, &iter, GUI_FOREGROUND_COLOR, "DarkRed", GUI_TYPE, "equity", GUI_SYMBOL, Folder->Equity[ c ]->symbol_stock_ch, GUI_SHARES_OUNCES, shares, GUI_PREMIUM, Folder->Equity[ c ]->current_price_stock_ch, GUI_PRICE, Folder->Equity[ c ]->high_stock_ch, GUI_TOTAL, Folder->Equity[ c ]->low_stock_ch, GUI_EXTRA_ONE, Folder->Equity[ c ]->opening_stock_ch, GUI_EXTRA_TWO, Folder->Equity[ c ]->prev_closing_stock_ch, GUI_EXTRA_THREE, Folder->Equity[ c ]->change_share_ch, GUI_EXTRA_FOUR, Folder->Equity[ c ]->change_value_ch, GUI_EXTRA_FIVE, Folder->Equity[ c ]->current_investment_stock_ch, GUI_EXTRA_SIX, Folder->Equity[ c ]->change_percent_ch, -1 );
        }

        c++;
    }

    gtk_list_store_append ( store, &iter );
    gtk_list_store_set ( store, &iter, GUI_SYMBOL, "_______", -1 );
    gtk_list_store_append ( store, &iter );
    gtk_list_store_set ( store, &iter, GUI_SYMBOL, "Asset", GUI_SHARES_OUNCES, "Value", GUI_PREMIUM, "Gain ($)", GUI_PRICE, "Gain (%)",-1 );
    gtk_list_store_append ( store, &iter );
    gtk_list_store_set ( store, &iter, GUI_TYPE, "cash", GUI_SYMBOL, "Cash", GUI_SHARES_OUNCES, MetaData->cash_ch, -1 );
    gtk_list_store_append ( store, &iter );
    gtk_list_store_set ( store, &iter, GUI_TYPE, "bullion", GUI_SYMBOL, "Bullion", GUI_SHARES_OUNCES, MetaData->bullion_port_value_ch, -1 );

    gtk_list_store_append ( store, &iter );
    if ( *MetaData->stock_port_value_chg_f == 0 ){
        gtk_list_store_set ( store, &iter, GUI_TYPE, "equity_total", GUI_SYMBOL, "Equity", GUI_SHARES_OUNCES, MetaData->stock_port_value_ch, GUI_PREMIUM, MetaData->stock_port_value_chg_ch, GUI_PRICE, MetaData->stock_port_value_p_chg_ch,-1 );
    } else if ( *MetaData->stock_port_value_chg_f > 0 ){
        gtk_list_store_set ( store, &iter, GUI_FOREGROUND_COLOR, "Green", GUI_TYPE, "equity_total", GUI_SYMBOL, "Equity", GUI_SHARES_OUNCES, MetaData->stock_port_value_ch, GUI_PREMIUM, MetaData->stock_port_value_chg_ch, GUI_PRICE, MetaData->stock_port_value_p_chg_ch,-1 );
    } else {
        gtk_list_store_set ( store, &iter, GUI_FOREGROUND_COLOR, "DarkRed", GUI_TYPE, "equity_total", GUI_SYMBOL, "Equity", GUI_SHARES_OUNCES, MetaData->stock_port_value_ch, GUI_PREMIUM, MetaData->stock_port_value_chg_ch, GUI_PRICE, MetaData->stock_port_value_p_chg_ch,-1 );
    }
    
    gtk_list_store_append ( store, &iter );
    gtk_list_store_set ( store, &iter, GUI_SYMBOL, "_______", -1 );
    
    gtk_list_store_append ( store, &iter );
    if ( *MetaData->portfolio_port_value_chg_f == 0 ){
        gtk_list_store_set ( store, &iter, GUI_SYMBOL, "Portfolio", GUI_SHARES_OUNCES, MetaData->portfolio_port_value_ch, GUI_PREMIUM, MetaData->portfolio_port_value_chg_ch, GUI_PRICE, MetaData->portfolio_port_value_p_chg_ch, -1 );
    } else if ( *MetaData->portfolio_port_value_chg_f > 0 ){
        gtk_list_store_set ( store, &iter, GUI_FOREGROUND_COLOR, "Green", GUI_SYMBOL, "Portfolio", GUI_SHARES_OUNCES, MetaData->portfolio_port_value_ch, GUI_PREMIUM, MetaData->portfolio_port_value_chg_ch, GUI_PRICE, MetaData->portfolio_port_value_p_chg_ch, -1 );
    } else {
        gtk_list_store_set ( store, &iter, GUI_FOREGROUND_COLOR, "DarkRed", GUI_SYMBOL, "Portfolio", GUI_SHARES_OUNCES, MetaData->portfolio_port_value_ch, GUI_PREMIUM, MetaData->portfolio_port_value_chg_ch, GUI_PRICE, MetaData->portfolio_port_value_p_chg_ch, -1 );
    }
    
    gtk_list_store_append ( store, &iter );
    gtk_list_store_set ( store, &iter, GUI_SYMBOL, "_______", -1 );
    
    return store;
}

GtkListStore * MakeDefaultStore (){
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
    gtk_list_store_set ( store, &iter, GUI_TYPE, "bullion", GUI_SYMBOL, "Gold", GUI_SHARES_OUNCES, Precious->Gold->ounce_ch, GUI_PREMIUM, Precious->Gold->premium_ch, -1 );
    gtk_list_store_append ( store, &iter );
    gtk_list_store_set ( store, &iter, GUI_TYPE, "bullion", GUI_SYMBOL, "Silver", GUI_SHARES_OUNCES, Precious->Silver->ounce_ch, GUI_PREMIUM, Precious->Silver->premium_ch, -1 );
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
    while(c < Folder->size){
        snprintf(shares, 13, "%d",*Folder->Equity[ c ]->num_shares_stock_int);
        gtk_list_store_append ( store, &iter );
        gtk_list_store_set ( store, &iter, GUI_TYPE, "equity", GUI_SYMBOL, Folder->Equity[ c ]->symbol_stock_ch, GUI_SHARES_OUNCES, shares, -1 );

        c++;
    }

    gtk_list_store_append ( store, &iter );
    gtk_list_store_set ( store, &iter, GUI_SYMBOL, "_______", -1 );
    gtk_list_store_append ( store, &iter );
    gtk_list_store_set ( store, &iter, GUI_TYPE, "cash", GUI_SYMBOL, "Cash", GUI_PREMIUM, MetaData->cash_ch, -1 );
    gtk_list_store_append ( store, &iter );
    gtk_list_store_set ( store, &iter, GUI_SYMBOL, "_______", -1 );
    
    return store;
}

int MakeGUIOne ()
{ 
    GtkListStore *store = NULL;
    GtkWidget* list = GTK_WIDGET ( gtk_builder_get_object (builder, "TreeView") );

    /* Clear the current TreeView model */ 
    TreeViewClear ();

    /* Set the columns for the new TreeView model */
    SetColumns ( GUI_COLUMN_ONE );

    /* Set up the storage container */
    store = MakeGUIOneStore ();    
    
    /* Add the store of data to the list. */
    gtk_tree_view_set_model(GTK_TREE_VIEW(list), GTK_TREE_MODEL(store));
    g_object_unref( store );
   
    /* Set the list header as invisible. */
    gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(list), FALSE);

    /* Remove Grid Lines. */
    gtk_tree_view_set_grid_lines ( GTK_TREE_VIEW ( list ), GTK_TREE_VIEW_GRID_LINES_NONE );

    return 0;
}

int DefaultTreeView () {
    GtkListStore *store = NULL;
    GtkWidget* list = GTK_WIDGET ( gtk_builder_get_object (builder, "TreeView") );

    /* Clear the current TreeView model */ 
    TreeViewClear ();

    /* Set the columns for the new TreeView model */
    SetColumns ( GUI_COLUMN_DEFAULT );

    /* Set up the storage container */
    store = MakeDefaultStore ();    
    
    /* Add the store of data to the list. */
    gtk_tree_view_set_model(GTK_TREE_VIEW(list), GTK_TREE_MODEL(store));
    g_object_unref( store );
   
    /* Set the list header as invisible. */
    gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(list), FALSE);

    /* Remove Grid Lines. */
    gtk_tree_view_set_grid_lines ( GTK_TREE_VIEW ( list ), GTK_TREE_VIEW_GRID_LINES_NONE );
    return 0;
}

void view_popup_menu_onDeleteRow (GtkWidget *menuitem, void *userdata)
{
    if (menuitem == NULL) return;
    right_click_container *my_data = (right_click_container*) userdata;

    /* Prevent's Program From Crashing During A Data Fetch Operation */
    pthread_mutex_lock( &mutex_working[2] );

    SqliteRemoveEquity(my_data->symbol, Folder);

    pthread_mutex_unlock( &mutex_working[2] );

    if ( Folder->size == 0 && *MetaData->fetching_data_bool == true ){
        *MetaData->fetching_data_bool = false;
        gdk_threads_add_idle ( FetchDataBTNLabel, (void*)MetaData->fetching_data_bool );
    }

    if ( *MetaData->fetching_data_bool == false ) gdk_threads_add_idle( DefaultTreeView, NULL );
    
    free( my_data->symbol );
    free( my_data->type );
    free( my_data );
}

void view_popup_menu_onDeleteAllEquityRows (GtkWidget *menuitem, void *userdata)
{
    if (menuitem == NULL) return;
    right_click_container *my_data = (right_click_container*) userdata;

    /* Prevent's Program From Crashing During A Data Fetch Operation */
    pthread_mutex_lock( &mutex_working[2] );

    SqliteRemoveAllEquity( Folder );

    pthread_mutex_unlock( &mutex_working[2] );
    
    if (*MetaData->fetching_data_bool == true){ 
        *MetaData->fetching_data_bool = false;
        gdk_threads_add_idle ( FetchDataBTNLabel, (void*)MetaData->fetching_data_bool );
        gdk_threads_add_idle( DefaultTreeView, NULL );
    } else {
        gdk_threads_add_idle( DefaultTreeView, NULL );
    }

    free( my_data->symbol );
    free( my_data->type );
    free( my_data );
}

void view_popup_menu_onAddRow (GtkWidget *menuitem, void *userdata)
{
    if (menuitem == NULL) return;
    right_click_container *my_data = (right_click_container*) userdata;

    if ( strcmp( my_data->type, "equity" ) == 0 ){
        gdk_threads_add_idle( ShowHideAddRemoveSecurityWindow, NULL );
    } else if ( strcmp( my_data->type, "equity_total" ) == 0 ) {
        gdk_threads_add_idle( ShowHideAddRemoveSecurityWindow, NULL );
    } else if ( strcmp( my_data->type, "bullion" ) == 0 ) {
        gdk_threads_add_idle( ShowHideBullionWindow, NULL );
    } else if ( strcmp( my_data->type, "cash" ) == 0 ) {
        gdk_threads_add_idle( ShowHideCashWindow, NULL );
    }

    free( my_data->symbol );
    free( my_data->type );
    free( my_data );
}

gboolean view_onButtonPressed (GtkWidget *treeview, GdkEventButton *event)
{
    GtkTreeModel *model;
    GtkTreeIter iter;
    GtkWidget *menu, *menuitem;
    char *type = NULL;
    char *symbol = NULL;

    /* single click with the right mouse button */
    if (event->type == GDK_BUTTON_PRESS  &&  event->button == 3 )
    {
    /* optional: select row if no row is selected or only
     *  one other row is selected (will only do something
     *  if you set a tree selection mode as described later
     *  in the tutorial) */
        if (1)
        {
            GtkTreeSelection *selection;          

            selection = gtk_tree_view_get_selection( GTK_TREE_VIEW( treeview ) );
            gtk_tree_selection_set_mode ( selection, GTK_SELECTION_SINGLE );
            if (gtk_tree_selection_get_selected( selection, &model, &iter ) )
            {
                gtk_tree_model_get (model, &iter, GUI_TYPE, &type, GUI_SYMBOL, &symbol, -1);
                if ( !type || !symbol ) return GDK_EVENT_STOP;

                int eq_flag = strcmp( type, "equity" );
                int eq_tot_flag = strcmp( type, "equity_total" );
                int bu_flag = strcmp( type, "bullion" );
                int ca_flag = strcmp( type, "cash" );
                bool show_menu = false;

                right_click_container *my_data = (right_click_container *) malloc ( sizeof( right_click_container ) );
                my_data->type = strdup( type );
                my_data->symbol = strdup( symbol );

                free( type );
                free( symbol );

                if( eq_flag == 0 )
                /* If the type is an equity enable row deletion. */
                {   
                    show_menu = true;

                    size_t len = strlen("Delete Entry ") + strlen( my_data->symbol ) + 1;
                    char *menu_label = (char*)malloc( len );
                    snprintf(menu_label, len, "Delete Entry %s", my_data->symbol );

                    menu = gtk_menu_new();
                    menuitem = gtk_menu_item_new_with_label( menu_label );
                    g_signal_connect(menuitem, "activate", G_CALLBACK( view_popup_menu_onDeleteRow ), my_data);
                    gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
                    free ( menu_label );

                    menuitem = gtk_menu_item_new_with_label("Delete All Equity");
                    g_signal_connect(menuitem, "activate", G_CALLBACK( view_popup_menu_onDeleteAllEquityRows ), my_data);
                    gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);

                    menuitem = gtk_menu_item_new_with_label("Add / Edit Equity");
                    g_signal_connect(menuitem, "activate", G_CALLBACK( view_popup_menu_onAddRow ), my_data);
                    gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);

                    gtk_widget_show_all(menu);
                    gtk_menu_popup_at_pointer (GTK_MENU(menu), (GdkEvent*)event); 
                    
                    g_object_ref_sink( G_OBJECT ( menu ) );

                } else if (eq_tot_flag == 0 ) 
                {
                    show_menu = true;

                    menu = gtk_menu_new();
                    menuitem = gtk_menu_item_new_with_label("Delete All Equity");
                    g_signal_connect(menuitem, "activate", G_CALLBACK( view_popup_menu_onDeleteAllEquityRows ), my_data);
                    gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);

                    menuitem = gtk_menu_item_new_with_label("Add / Edit Equity");
                    g_signal_connect(menuitem, "activate", G_CALLBACK( view_popup_menu_onAddRow ), my_data);
                    gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);

                    gtk_widget_show_all(menu);
                    gtk_menu_popup_at_pointer (GTK_MENU(menu), (GdkEvent*)event);

                    g_object_ref_sink( G_OBJECT ( menu ) );

                } else if ( (bu_flag == 0) || (ca_flag == 0) ) 
                {
                    show_menu = true;

                    if (bu_flag == 0){
                        menuitem = gtk_menu_item_new_with_label("Edit Bullion");
                    } else {
                        menuitem = gtk_menu_item_new_with_label("Edit Cash");
                    }

                    menu = gtk_menu_new();
                    g_signal_connect(menuitem, "activate", G_CALLBACK( view_popup_menu_onAddRow ), my_data);
                    gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);

                    gtk_widget_show_all(menu);
                    gtk_menu_popup_at_pointer (GTK_MENU(menu), (GdkEvent*)event); 

                    g_object_ref_sink( G_OBJECT ( menu ) );

                }  

                if (!show_menu) 
                /* If the menu was not displayed we need to free the data structure. */
                {
                    free( my_data->type );
                    free( my_data->symbol );
                    free( my_data );
                }
            }
      
            /* Note: gtk_tree_selection_count_selected_rows() does not
            *   exist in gtk+-2.0, only in gtk+ >= v2.2 ! */
            if (gtk_tree_selection_count_selected_rows(selection)  <= 1)
            {
                GtkTreePath *path;

                /* Get tree path for row that was clicked */
                if (gtk_tree_view_get_path_at_pos(GTK_TREE_VIEW(treeview), (gint) event->x, (gint) event->y, &path, NULL, NULL, NULL))
                {
                    gtk_tree_selection_unselect_all(selection);
                    gtk_tree_selection_select_path(selection, path);
                    gtk_tree_path_free(path);
                }
            }
        } /* end of optional bit */
        return GDK_EVENT_STOP;
    }
    return GDK_EVENT_PROPAGATE;
}

static void SetKeyboardShortcuts (){
    GtkWidget* TreeView = GTK_WIDGET ( gtk_builder_get_object (builder, "ShortcutWindowTreeView") );

    GtkCellRenderer *renderer;
    GtkTreeViewColumn *column;

    /* In order to display a model/store we need to set the TreeView Columns. */
    renderer = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes("Column Header 1", renderer, "text", 0, NULL);
    gtk_tree_view_column_set_resizable (column, true);
    gtk_tree_view_column_set_visible (column, true);
    gtk_tree_view_column_set_min_width (column, 0);
    gtk_tree_view_append_column(GTK_TREE_VIEW(TreeView), column);

    renderer = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes("Column Header 2", renderer, "text", 1, NULL);
    gtk_tree_view_column_set_resizable (column, true);
    gtk_tree_view_column_set_visible (column, true);
    gtk_tree_view_column_set_min_width (column, 0);
    gtk_tree_view_append_column(GTK_TREE_VIEW(TreeView), column);

    /* Here we set the rows for the 2 column store */
    GtkListStore *store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_STRING);
    GtkTreeIter iter;

    gtk_list_store_append( store, &iter );
    gtk_list_store_set( store, &iter, 0, "Application Window", -1 );
    gtk_list_store_append( store, &iter );
    gtk_list_store_set( store, &iter, 0, "----------------------------------", -1 );
    gtk_list_store_append( store, &iter );
    gtk_list_store_set( store, &iter, 0, "File", 1, "<Alt> f", -1 );
    gtk_list_store_append( store, &iter );
    gtk_list_store_set( store, &iter, 0, "RSI", 1, "<Ctrl> r", -1 );
    gtk_list_store_append( store, &iter );
    gtk_list_store_set( store, &iter, 0, "Quit", 1, "<Ctrl> q", -1 );
    gtk_list_store_append( store, &iter );
    gtk_list_store_append( store, &iter );
    gtk_list_store_set( store, &iter, 0, "Edit", 1, "<Alt> e", -1 );
    gtk_list_store_append( store, &iter );
    gtk_list_store_set( store, &iter, 0, "API", 1, "<Ctrl> p", -1 );
    gtk_list_store_append( store, &iter );
    gtk_list_store_set( store, &iter, 0, "Securities", 1, "<Ctrl> s", -1 );
    gtk_list_store_append( store, &iter );
    gtk_list_store_set( store, &iter, 0, "Bullion", 1, "<Ctrl> b", -1 );
    gtk_list_store_append( store, &iter );
    gtk_list_store_set( store, &iter, 0, "Cash", 1, "<Ctrl> c", -1 );
    gtk_list_store_append( store, &iter );
    gtk_list_store_append( store, &iter );
    gtk_list_store_set( store, &iter, 0, "Help", 1, "<Alt> h", -1 );
    gtk_list_store_append( store, &iter );
    gtk_list_store_set( store, &iter, 0, "Keyboard Shortcuts", 1, "<Ctrl> k", -1 );
    gtk_list_store_append( store, &iter );
    gtk_list_store_set( store, &iter, 0, "About", 1, "<Ctrl> a", -1 );
    gtk_list_store_append( store, &iter );
    gtk_list_store_append( store, &iter );
    gtk_list_store_set( store, &iter, 0, "Fetch Data", 1, "<Ctrl> d", -1 );
    gtk_list_store_append( store, &iter );
    gtk_list_store_set( store, &iter, 0, "Quit", 1, "<Ctrl> q", -1 );
    gtk_list_store_append( store, &iter );
    gtk_list_store_append( store, &iter );
    gtk_list_store_set( store, &iter, 0, "RSI Window", -1 );
    gtk_list_store_append( store, &iter );
    gtk_list_store_set( store, &iter, 0, "----------------------------------", -1 );
    gtk_list_store_append( store, &iter );
    gtk_list_store_set( store, &iter, 0, "Fetch Data", 1, "<Ctrl> d", -1 );
    gtk_list_store_append( store, &iter );
    gtk_list_store_set( store, &iter, 0, "Close", 1, "<Ctrl> c", -1 );
    gtk_list_store_append( store, &iter );
    gtk_list_store_append( store, &iter );
    gtk_list_store_set( store, &iter, 0, "All Other Windows", -1 );
    gtk_list_store_append( store, &iter );
    gtk_list_store_set( store, &iter, 0, "----------------------------------", -1 );
    gtk_list_store_append( store, &iter );
    gtk_list_store_set( store, &iter, 0, "OK", 1, "<Ctrl> o", -1 );
    gtk_list_store_append( store, &iter );
    gtk_list_store_set( store, &iter, 0, "Cancel", 1, "<Ctrl> c", -1 );


    /* Add the store of data to the TreeView. */
    gtk_tree_view_set_model( GTK_TREE_VIEW(TreeView), GTK_TREE_MODEL ( store ) );
    g_object_unref( store );

    /* Set the TreeView header as invisible. */
    gtk_tree_view_set_headers_visible( GTK_TREE_VIEW(TreeView), false );

    /* Make the TreeView unselectable. */
    GtkTreeSelection *select = gtk_tree_view_get_selection ( GTK_TREE_VIEW ( TreeView ) );
    gtk_tree_selection_set_mode ( select, GTK_SELECTION_NONE );

    /* Remove TreeView Grid Lines. */
    gtk_tree_view_set_grid_lines ( GTK_TREE_VIEW ( TreeView ), GTK_TREE_VIEW_GRID_LINES_NONE );
}

static void SetAboutInfoLabel ()
{
    /* Set the About window license. */
    GtkWidget* Label = GTK_WIDGET ( gtk_builder_get_object (builder, "AboutLicenseLabel") );
    gtk_label_set_text ( GTK_LABEL ( Label ), LICENSE);
}

int DisplayTime (){
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

int DisplayTimeRemaining (){
    GtkWidget* CloseLabel = GTK_WIDGET ( gtk_builder_get_object (builder, "MarketCloseLabel") );
    GtkWidget* TimeRemLabel = GTK_WIDGET ( gtk_builder_get_object (builder, "TimeLeftLabel") );
    int h, m, s;
    char time_left_ch[10];
    bool isclosed;
    struct tm NY_Time; 

    isclosed = TimeToClose ( &h, &m, &s );
    snprintf ( time_left_ch, 10, "%02d:%02d:%02d", h, m, s);
    gtk_label_set_text ( GTK_LABEL ( TimeRemLabel ), time_left_ch );
    if ( !isclosed ) { 
        gtk_label_set_text ( GTK_LABEL ( CloseLabel ), "Market Closes In" ); 
        return 0; 
    }
    
    if ( isclosed ) {
        if ( !(*MetaData->holiday_bool) ) {
            gtk_label_set_text ( GTK_LABEL ( CloseLabel ), "Market Closed" );
        } else {
            NY_Time = NYTimeComponents ();
            gtk_label_set_text ( GTK_LABEL ( CloseLabel ), WhichHoliday ( NY_Time ) );
        }
    }

    return 0;
}

void GUISignalConnect ()
/* Connect GUI index signals to the signal handlers. */
{
    GObject *window,*button;

    /* Connect signal handlers to the constructed widgets. */
    /* The last argument, the widget index signal, is an enum/int casted to a void*, 
       small to larger datatype, in the GUIThreadHandler we cast this back to an enum/int,
       because it started as an enum/int we should not worry about data loss through 
       casting truncation. */
    window = gtk_builder_get_object (builder, "MainWindow");
    g_signal_connect ( window, "destroy", G_CALLBACK ( GUICallbackHandler ), (void *)EXIT_APPLICATION );
    g_signal_connect ( window, "configure-event", G_CALLBACK ( CallbackHandler_alt ), NULL );
    gtk_window_resize ( GTK_WINDOW ( window ), MainWindowStruct.width, MainWindowStruct.height );
    gtk_window_move ( GTK_WINDOW ( window ), MainWindowStruct.x_pos, MainWindowStruct.y_pos );

    button = gtk_builder_get_object (builder, "FetchDataBTN");
    g_signal_connect ( button, "clicked", G_CALLBACK ( GUICallbackHandler ), (void *)FETCH_DATA_BTN );
    gtk_widget_grab_focus ( GTK_WIDGET ( button ) );

    button = gtk_builder_get_object (builder, "MainMenuQuitBTN");
    g_signal_connect ( button, "activate", G_CALLBACK ( GUICallbackHandler ), (void *)EXIT_APPLICATION );

    button = gtk_builder_get_object (builder, "QuitBTN");
    g_signal_connect ( button, "clicked", G_CALLBACK ( GUICallbackHandler ), (void *)EXIT_APPLICATION );


    button = gtk_builder_get_object (builder, "MainMenuAddRemoveSecurityBTN");
    g_signal_connect ( button, "activate", G_CALLBACK ( GUICallbackHandler ), (void *)ADD_REMOVE_SEC_BTN );

    window = gtk_builder_get_object (builder, "AddRemoveSecurity");
    g_signal_connect( window, "delete_event", G_CALLBACK( gtk_widget_hide_on_delete ), NULL);

    button = gtk_builder_get_object (builder, "AddRemoveSecurityOkBTN");
    g_signal_connect ( button, "clicked", G_CALLBACK ( GUICallbackHandler ), (void *)ADD_REMOVE_SEC_OK_BTN );

    button = gtk_builder_get_object (builder, "AddRemoveSecurityCancelBTN");
    g_signal_connect ( button, "clicked", G_CALLBACK ( GUICallbackHandler ), (void *)ADD_REMOVE_SEC_CANCEL_BTN );

    button = gtk_builder_get_object (builder, "AddRemoveSecuritySwitch");
    g_signal_connect ( button, "state-set", G_CALLBACK ( AddRemoveSecuritySwitch ), (void *)ADD_REMOVE_SEC_SWITCH );

    button = gtk_builder_get_object (builder, "AddRemoveSecurityComboBox");
    g_signal_connect ( button, "changed", G_CALLBACK ( GUICallbackHandler ), (void *)ADD_REMOVE_SEC_COMBO_BOX );

    button = gtk_builder_get_object (builder, "AddRemoveSecuritySymbolEntryBox");
    g_signal_connect ( button, "changed", G_CALLBACK ( GUICallbackHandler ), (void *)ADD_REMOVE_SEC_CURSOR_MOVE );

    button = gtk_builder_get_object (builder, "AddRemoveSecuritySharesEntryBox");
    g_signal_connect ( button, "changed", G_CALLBACK ( GUICallbackHandler ), (void *)ADD_REMOVE_SEC_CURSOR_MOVE );

    
    button = gtk_builder_get_object (builder, "MainMenuAboutBTN");
    g_signal_connect ( button, "activate", G_CALLBACK ( GUICallbackHandler ), (void *)ABOUT_BTN );

    window = gtk_builder_get_object (builder, "AboutWindow");
    g_signal_connect( window, "delete_event", G_CALLBACK( gtk_widget_hide_on_delete ), NULL);

    button = gtk_builder_get_object (builder, "AboutOKBTN");
    g_signal_connect ( button, "clicked", G_CALLBACK ( GUICallbackHandler ), (void *)ABOUT_BTN );


    button = gtk_builder_get_object (builder, "MainMenuShortcutBTN");
    g_signal_connect ( button, "activate", G_CALLBACK ( GUICallbackHandler ), (void *)SHORTCUT_KEYS_BTN );

    window = gtk_builder_get_object (builder, "ShortcutWindow");
    g_signal_connect( window, "delete_event", G_CALLBACK( gtk_widget_hide_on_delete ), NULL);

    button = gtk_builder_get_object (builder, "ShortcutOKBTN");
    g_signal_connect ( button, "clicked", G_CALLBACK ( GUICallbackHandler ), (void *)SHORTCUT_KEYS_BTN );


    button = gtk_builder_get_object (builder, "MainMenuAddRemoveBullionBTN");
    g_signal_connect ( button, "activate", G_CALLBACK ( GUICallbackHandler ), (void *)ADD_REMOVE_BUL_BTN );

    window = gtk_builder_get_object (builder, "AddRemoveBullionWindow");
    g_signal_connect( window, "delete_event", G_CALLBACK( gtk_widget_hide_on_delete ), NULL);

    button = gtk_builder_get_object (builder, "AddRemoveBullionOKBTN");
    g_signal_connect ( button, "clicked", G_CALLBACK ( GUICallbackHandler ), (void *)ADD_REMOVE_BUL_OK_BTN );
    gtk_widget_set_sensitive ( GTK_WIDGET ( button ) , false );

    button = gtk_builder_get_object (builder, "AddRemoveBullionCancelBTN");
    g_signal_connect ( button, "clicked", G_CALLBACK ( GUICallbackHandler ), (void *)ADD_REMOVE_BUL_CANCEL_BTN );

    button = gtk_builder_get_object (builder, "AddRemoveBullionGoldOuncesEntryBox");
    g_signal_connect ( button, "changed", G_CALLBACK ( GUICallbackHandler ), (void *)ADD_REMOVE_BUL_CURSOR_MOVE );

    button = gtk_builder_get_object (builder, "AddRemoveBullionGoldPremiumEntryBox");
    g_signal_connect ( button, "changed", G_CALLBACK ( GUICallbackHandler ), (void *)ADD_REMOVE_BUL_CURSOR_MOVE );

    button = gtk_builder_get_object (builder, "AddRemoveBullionSilverOuncesEntryBox");
    g_signal_connect ( button, "changed", G_CALLBACK ( GUICallbackHandler ), (void *)ADD_REMOVE_BUL_CURSOR_MOVE );

    button = gtk_builder_get_object (builder, "AddRemoveBullionSilverPremiumEntryBox");
    g_signal_connect ( button, "changed", G_CALLBACK ( GUICallbackHandler ), (void *)ADD_REMOVE_BUL_CURSOR_MOVE );


    button = gtk_builder_get_object (builder, "MainMenuAddRemoveCashBTN");
    g_signal_connect ( button, "activate", G_CALLBACK ( GUICallbackHandler ), (void *)ADD_REMOVE_CASH_BTN );

    window = gtk_builder_get_object (builder, "AddRemoveCashWindow");
    g_signal_connect( window, "delete_event", G_CALLBACK( gtk_widget_hide_on_delete ), NULL);

    button = gtk_builder_get_object (builder, "AddRemoveCashOKBTN");
    g_signal_connect ( button, "clicked", G_CALLBACK ( GUICallbackHandler ), (void *)ADD_REMOVE_CASH_OK_BTN );
    gtk_widget_set_sensitive ( GTK_WIDGET ( button ) , false );

    button = gtk_builder_get_object (builder, "AddRemoveCashCancelBTN");
    g_signal_connect ( button, "clicked", G_CALLBACK ( GUICallbackHandler ), (void *)ADD_REMOVE_CASH_CANCEL_BTN );

    button = gtk_builder_get_object (builder, "AddRemoveCashValueEntryBox");
    g_signal_connect ( button, "changed", G_CALLBACK ( GUICallbackHandler ), (void *)ADD_REMOVE_CASH_CURSOR_MOVE );

    button = gtk_builder_get_object (builder, "MainMenuChangeAPIinfoBTN");
    g_signal_connect ( button, "activate", G_CALLBACK ( GUICallbackHandler ), (void *)CHANGE_API_BTN );

    window = gtk_builder_get_object (builder, "ChangeApiInfoWindow");
    g_signal_connect( window, "delete_event", G_CALLBACK( gtk_widget_hide_on_delete ), NULL);

    button = gtk_builder_get_object (builder, "ChangeApiInfoOKBTN");
    g_signal_connect ( button, "clicked", G_CALLBACK ( GUICallbackHandler ), (void *)CHANGE_API_OK_BTN );
    gtk_widget_set_sensitive ( GTK_WIDGET ( button ) , false );

    button = gtk_builder_get_object (builder, "ChangeApiInfoCancelBTN");
    g_signal_connect ( button, "clicked", G_CALLBACK ( GUICallbackHandler ), (void *)CHANGE_API_CANCEL_BTN );

    button = gtk_builder_get_object (builder, "ChangeApiInfoEquityUrlEntryBox");
    g_signal_connect ( button, "changed", G_CALLBACK ( GUICallbackHandler ), (void *)CHANGE_API_CURSOR_MOVE );

    button = gtk_builder_get_object (builder, "ChangeApiInfoUrlKeyEntryBox");
    g_signal_connect ( button, "changed", G_CALLBACK ( GUICallbackHandler ), (void *)CHANGE_API_CURSOR_MOVE );

    button = gtk_builder_get_object (builder, "ChangeApiInfoHoursSpinBox");
    g_signal_connect ( button, "changed", G_CALLBACK ( GUICallbackHandler ), (void *)CHANGE_API_CURSOR_MOVE );


    button = gtk_builder_get_object (builder, "MainMenuViewRSIBTN");
    g_signal_connect ( button, "activate", G_CALLBACK ( GUICallbackHandler ), (void *)VIEW_RSI_BTN );

    window = gtk_builder_get_object (builder, "ViewRSIWindow");
    g_signal_connect( window, "delete_event", G_CALLBACK( gtk_widget_hide_on_delete ), NULL);

    button = gtk_builder_get_object (builder, "ViewRSICloseBTN");
    g_signal_connect ( button, "clicked", G_CALLBACK ( GUICallbackHandler ), (void *)VIEW_RSI_BTN );

    button = gtk_builder_get_object (builder, "ViewRSIFetchDataBTN");
    g_signal_connect ( button, "clicked", G_CALLBACK ( GUICallbackHandler ), (void *)VIEW_RSI_FETCH_DATA_BTN );

    button = gtk_builder_get_object (builder, "ViewRSISymbolEntryBox");
    g_signal_connect ( button, "changed", G_CALLBACK ( GUICallbackHandler ), (void *)VIEW_RSI_CURSOR_MOVE );

    /* This is the main window's TreeView ( There was only one treeview when I started ). */
    button = gtk_builder_get_object (builder, "TreeView");
    g_signal_connect ( button, "button-press-event", G_CALLBACK( view_onButtonPressed ), NULL );

    /* Start the clock threads. */
    pthread_t thread_id;
    pthread_create( &thread_id, NULL, GUIThreadHandler, (void *)DISPLAY_TIME );
    pthread_create( &thread_id, NULL, GUIThreadHandler, (void *)DISPLAY_TIME_OPEN_INDICATOR );

    /* Set up the RSIView Window's EntryBox Completion Widget 
       This will download the NYSE and NASDAQ symbol list when the application loads.
       If there is no internet connection or the server is unavailable cURL will return an
       error, but otherwise the application should run as normal.
    */
    pthread_create( &thread_id, NULL, GUIThreadHandler, (void *)VIEW_RSI_COMPLETION );
}

void SetUpGUI ()
/* Setup GUI widgets and display the GUI. */
{
    GError *error = NULL;

    /* Compiling the resources.c file ensures the resources are in the 
       GResources namespace (basically ensures the UI description file 
       and the icon file are in the namespace, our only resources).  
       The icon file is referenced in the UI description file. */

    /* Construct a GtkBuilder instance and load our UI description. */
    builder = gtk_builder_new ();
    if ( gtk_builder_add_from_resource ( builder, "/financials.glade", &error ) == 0 )
    {
        g_printerr ( "Error loading user interface: %s\n", error->message );
        g_clear_error ( &error );
        exit( EXIT_FAILURE );
    }

    SetKeyboardShortcuts ();

    /* Add the license to the About window. */
    SetAboutInfoLabel ();

    /* Set the default treeview. */
    DefaultTreeView ();

    /* Connect callback functions to corresponding GUI signals. */
    GUISignalConnect ();

    /* Display the GUI window. */
    gtk_main ();
}