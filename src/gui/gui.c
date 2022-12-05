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

#include <glib-object.h>
#include <gtk/gtk.h>

#include "../include/gui.h"
#include "../include/gui_types.h"
#include "../include/class_types.h" /* portfolio_packet, window_data */
#include "../include/ui.h"

GtkBuilder *builder;

static void shortcuts_set_treeview (){
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
    gtk_list_store_set( store, &iter, 0, "      File", 1, "Ctrl - F", -1 );
    gtk_list_store_append( store, &iter );
    gtk_list_store_set( store, &iter, 0, "      RSI", 1, "Ctrl - R", -1 );
    gtk_list_store_append( store, &iter );
    gtk_list_store_set( store, &iter, 0, "      Quit", 1, "Ctrl - Q", -1 );
    gtk_list_store_append( store, &iter );
    gtk_list_store_append( store, &iter );
    gtk_list_store_set( store, &iter, 0, "      Edit", 1, "Ctrl - E", -1 );
    gtk_list_store_append( store, &iter );
    gtk_list_store_set( store, &iter, 0, "      API", 1, "Ctrl - P", -1 );
    gtk_list_store_append( store, &iter );
    gtk_list_store_set( store, &iter, 0, "      Securities", 1, "Ctrl - S", -1 );
    gtk_list_store_append( store, &iter );
    gtk_list_store_set( store, &iter, 0, "      Bullion", 1, "Ctrl - B", -1 );
    gtk_list_store_append( store, &iter );
    gtk_list_store_set( store, &iter, 0, "      Cash", 1, "Ctrl - C", -1 );
    gtk_list_store_append( store, &iter );
    gtk_list_store_append( store, &iter );
    gtk_list_store_set( store, &iter, 0, "      Help", 1, "Ctrl - H", -1 );
    gtk_list_store_append( store, &iter );
    gtk_list_store_set( store, &iter, 0, "      Keyboard Shortcuts", 1, "Ctrl - K", -1 );
    gtk_list_store_append( store, &iter );
    gtk_list_store_set( store, &iter, 0, "      About", 1, "Ctrl - A", -1 );
    gtk_list_store_append( store, &iter );
    gtk_list_store_append( store, &iter );
    gtk_list_store_set( store, &iter, 0, "      Get Data", 1, "Ctrl - D", -1 );
    gtk_list_store_append( store, &iter );
    gtk_list_store_set( store, &iter, 0, "      Quit", 1, "Ctrl - Q", -1 );
    gtk_list_store_append( store, &iter );
    gtk_list_store_append( store, &iter );
    gtk_list_store_set( store, &iter, 0, "RSI Window", -1 );
    gtk_list_store_append( store, &iter );
    gtk_list_store_set( store, &iter, 0, "      Get Data", 1, "Ctrl - D", -1 );
    gtk_list_store_append( store, &iter );
    gtk_list_store_append( store, &iter );
    gtk_list_store_set( store, &iter, 0, "API Window", -1 );
    gtk_list_store_append( store, &iter );
    gtk_list_store_set( store, &iter, 0, "      Get Symbols", 1, "Ctrl - S", -1 );
    gtk_list_store_append( store, &iter );
    gtk_list_store_append( store, &iter );
    gtk_list_store_set( store, &iter, 0, "All Other Windows", -1 );
    gtk_list_store_append( store, &iter );
    gtk_list_store_set( store, &iter, 0, "      File", 1, "Ctrl - F", -1 );
    gtk_list_store_append( store, &iter );
    gtk_list_store_set( store, &iter, 0, "      OK", 1, "Ctrl - O", -1 );
    gtk_list_store_append( store, &iter );
    gtk_list_store_set( store, &iter, 0, "      Close", 1, "Ctrl - C", -1 );


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

static void about_set_label ()
{
    /* Set the About window license. */
    GtkWidget* Label = GTK_WIDGET ( gtk_builder_get_object (builder, "AboutLicenseLabel") );
    gtk_label_set_text ( GTK_LABEL ( Label ), LICENSE);
}

static void gui_signal_connect ( void *data )
/* Connect GUI index signals to the signal handlers. */
{
    portfolio_packet *pkg = (portfolio_packet*)data;
    window_data *W = pkg->GetWindowData ();
    GObject *window,*button;

    /* Connect signal handlers to the constructed widgets. */
    /* The last argument, the widget index signal, is an enum/int casted to a void*, 
       small to larger datatype, in the GUIThreadHandler we cast this back to an enum/int,
       because it started as an enum/int we should not worry about data loss through 
       casting truncation. */
    
    window = gtk_builder_get_object (builder, "MainWindow");
    g_signal_connect ( window, "destroy", G_CALLBACK ( GUICallbackHandler ), (void *)MAIN_EXIT );
    g_signal_connect ( window, "configure-event", G_CALLBACK ( GUICallbackHandler_window_data ), (void *)GUI_MAIN_WINDOW );
    gtk_window_resize ( GTK_WINDOW ( window ), W->main_width, W->main_height );
    gtk_window_move ( GTK_WINDOW ( window ), W->main_x_pos, W->main_y_pos );

    button = gtk_builder_get_object (builder, "IndicesExpander");
    g_signal_connect ( button, "activate", G_CALLBACK ( GUICallbackHandler_expander_bar ), NULL );

    button = gtk_builder_get_object (builder, "FetchDataBTN");
    g_signal_connect ( button, "clicked", G_CALLBACK ( GUICallbackHandler ), (void *)MAIN_FETCH_BTN );
    gtk_widget_grab_focus ( GTK_WIDGET ( button ) );

    button = gtk_builder_get_object (builder, "MainFileMenuQuit");
    g_signal_connect ( button, "activate", G_CALLBACK ( GUICallbackHandler ), (void *)MAIN_EXIT );


    button = gtk_builder_get_object (builder, "MainEditMenuAddRemoveSecurity");
    g_signal_connect ( button, "activate", G_CALLBACK ( GUICallbackHandler ), (void *)EQUITY_TOGGLE_BTN );

    button = gtk_builder_get_object (builder, "SecuritiesMenuClose");
    g_signal_connect ( button, "activate", G_CALLBACK ( GUICallbackHandler ), (void *)EQUITY_TOGGLE_BTN );

    window = gtk_builder_get_object (builder, "AddRemoveSecurity");
    g_signal_connect( window, "delete_event", G_CALLBACK( gtk_widget_hide_on_delete ), NULL);

    button = gtk_builder_get_object (builder, "AddRemoveSecurityOkBTN");
    g_signal_connect ( button, "clicked", G_CALLBACK ( GUICallbackHandler ), (void *)EQUITY_OK_BTN );

    button = gtk_builder_get_object (builder, "AddRemoveSecuritySwitch");
    g_signal_connect ( button, "state-set", G_CALLBACK ( GUICallbackHandler_add_rem_switch ), (void *)EQUITY_SWITCH );

    button = gtk_builder_get_object (builder, "AddRemoveSecurityComboBox");
    g_signal_connect ( button, "changed", G_CALLBACK ( GUICallbackHandler ), (void *)EQUITY_COMBO_BOX );

    button = gtk_builder_get_object (builder, "AddRemoveSecuritySymbolEntryBox");
    g_signal_connect ( button, "changed", G_CALLBACK ( GUICallbackHandler ), (void *)EQUITY_CURSOR_MOVE );

    button = gtk_builder_get_object (builder, "AddRemoveSecuritySharesEntryBox");
    g_signal_connect ( button, "changed", G_CALLBACK ( GUICallbackHandler ), (void *)EQUITY_CURSOR_MOVE );

    
    button = gtk_builder_get_object (builder, "MainHelpMenuAbout");
    g_signal_connect ( button, "activate", G_CALLBACK ( GUICallbackHandler ), (void *)ABOUT_TOGGLE_BTN );

    button = gtk_builder_get_object (builder, "AboutMenuClose");
    g_signal_connect ( button, "activate", G_CALLBACK ( GUICallbackHandler ), (void *)ABOUT_TOGGLE_BTN );

    window = gtk_builder_get_object (builder, "AboutWindow");
    g_signal_connect( window, "delete_event", G_CALLBACK( gtk_widget_hide_on_delete ), NULL);


    button = gtk_builder_get_object (builder, "MainHelpMenuKeyboardShortcuts");
    g_signal_connect ( button, "activate", G_CALLBACK ( GUICallbackHandler ), (void *)SHORTCUT_TOGGLE_BTN );

    button = gtk_builder_get_object (builder, "ShortcutsMenuClose");
    g_signal_connect ( button, "activate", G_CALLBACK ( GUICallbackHandler ), (void *)SHORTCUT_TOGGLE_BTN );

    window = gtk_builder_get_object (builder, "ShortcutWindow");
    g_signal_connect( window, "delete_event", G_CALLBACK( gtk_widget_hide_on_delete ), NULL);


    button = gtk_builder_get_object (builder, "MainEditMenuBullion");
    g_signal_connect ( button, "activate", G_CALLBACK ( GUICallbackHandler ), (void *)BUL_TOGGLE_BTN );

    button = gtk_builder_get_object (builder, "BullionMenuClose");
    g_signal_connect ( button, "activate", G_CALLBACK ( GUICallbackHandler ), (void *)BUL_TOGGLE_BTN );

    window = gtk_builder_get_object (builder, "AddRemoveBullionWindow");
    g_signal_connect( window, "delete_event", G_CALLBACK( gtk_widget_hide_on_delete ), NULL);

    button = gtk_builder_get_object (builder, "AddRemoveBullionOKBTN");
    g_signal_connect ( button, "clicked", G_CALLBACK ( GUICallbackHandler ), (void *)BUL_OK_BTN );
    gtk_widget_set_sensitive ( GTK_WIDGET ( button ) , false );

    button = gtk_builder_get_object (builder, "AddRemoveBullionGoldOuncesEntryBox");
    g_signal_connect ( button, "changed", G_CALLBACK ( GUICallbackHandler ), (void *)BUL_CURSOR_MOVE );

    button = gtk_builder_get_object (builder, "AddRemoveBullionGoldPremiumEntryBox");
    g_signal_connect ( button, "changed", G_CALLBACK ( GUICallbackHandler ), (void *)BUL_CURSOR_MOVE );

    button = gtk_builder_get_object (builder, "AddRemoveBullionSilverOuncesEntryBox");
    g_signal_connect ( button, "changed", G_CALLBACK ( GUICallbackHandler ), (void *)BUL_CURSOR_MOVE );

    button = gtk_builder_get_object (builder, "AddRemoveBullionSilverPremiumEntryBox");
    g_signal_connect ( button, "changed", G_CALLBACK ( GUICallbackHandler ), (void *)BUL_CURSOR_MOVE );

    button = gtk_builder_get_object (builder, "AddRemoveBullionPlatinumOuncesEntryBox");
    g_signal_connect ( button, "changed", G_CALLBACK ( GUICallbackHandler ), (void *)BUL_CURSOR_MOVE );

    button = gtk_builder_get_object (builder, "AddRemoveBullionPlatinumPremiumEntryBox");
    g_signal_connect ( button, "changed", G_CALLBACK ( GUICallbackHandler ), (void *)BUL_CURSOR_MOVE );

    button = gtk_builder_get_object (builder, "AddRemoveBullionPalladiumOuncesEntryBox");
    g_signal_connect ( button, "changed", G_CALLBACK ( GUICallbackHandler ), (void *)BUL_CURSOR_MOVE );

    button = gtk_builder_get_object (builder, "AddRemoveBullionPalladiumPremiumEntryBox");
    g_signal_connect ( button, "changed", G_CALLBACK ( GUICallbackHandler ), (void *)BUL_CURSOR_MOVE );

    button = gtk_builder_get_object (builder, "AddRemoveBullionComboBox");
    g_signal_connect ( button, "changed", G_CALLBACK ( GUICallbackHandler ), (void *)BUL_COMBO_BOX );


    button = gtk_builder_get_object (builder, "MainEditMenuCash");
    g_signal_connect ( button, "activate", G_CALLBACK ( GUICallbackHandler ), (void *)CASH_TOGGLE_BTN );

    button = gtk_builder_get_object (builder, "CashMenuClose");
    g_signal_connect ( button, "activate", G_CALLBACK ( GUICallbackHandler ), (void *)CASH_TOGGLE_BTN );

    window = gtk_builder_get_object (builder, "AddRemoveCashWindow");
    g_signal_connect( window, "delete_event", G_CALLBACK( gtk_widget_hide_on_delete ), NULL);

    button = gtk_builder_get_object (builder, "AddRemoveCashOKBTN");
    g_signal_connect ( button, "clicked", G_CALLBACK ( GUICallbackHandler ), (void *)CASH_OK_BTN );
    gtk_widget_set_sensitive ( GTK_WIDGET ( button ) , false );

    button = gtk_builder_get_object (builder, "AddRemoveCashValueEntryBox");
    g_signal_connect ( button, "changed", G_CALLBACK ( GUICallbackHandler ), (void *)CASH_CURSOR_MOVE );


    button = gtk_builder_get_object (builder, "MainEditMenuAPI");
    g_signal_connect ( button, "activate", G_CALLBACK ( GUICallbackHandler ), (void *)API_TOGGLE_BTN );

    button = gtk_builder_get_object (builder, "APIMenuClose");
    g_signal_connect ( button, "activate", G_CALLBACK ( GUICallbackHandler ), (void *)API_TOGGLE_BTN );

    window = gtk_builder_get_object (builder, "ChangeApiInfoWindow");
    g_signal_connect( window, "delete_event", G_CALLBACK( gtk_widget_hide_on_delete ), NULL);

    button = gtk_builder_get_object (builder, "ChangeApiInfoOKBTN");
    g_signal_connect ( button, "clicked", G_CALLBACK ( GUICallbackHandler ), (void *)API_OK_BTN );
    gtk_widget_set_sensitive ( GTK_WIDGET ( button ) , false );

    button = gtk_builder_get_object (builder, "ChangeApiInfoStockSymbolUpdateBTN");
    g_signal_connect ( button, "clicked", G_CALLBACK ( GUICallbackHandler ), (void *)API_SYMBOL_UPDATE_BTN );

    button = gtk_builder_get_object (builder, "ChangeApiInfoEquityUrlEntryBox");
    g_signal_connect ( button, "changed", G_CALLBACK ( GUICallbackHandler ), (void *)API_CURSOR_MOVE );

    button = gtk_builder_get_object (builder, "ChangeApiInfoUrlKeyEntryBox");
    g_signal_connect ( button, "changed", G_CALLBACK ( GUICallbackHandler ), (void *)API_CURSOR_MOVE );

    button = gtk_builder_get_object (builder, "ChangeApiInfoHoursSpinBox");
    g_signal_connect ( button, "changed", G_CALLBACK ( GUICallbackHandler ), (void *)API_CURSOR_MOVE );


    button = gtk_builder_get_object (builder, "MainFileMenuRSI");
    g_signal_connect ( button, "activate", G_CALLBACK ( GUICallbackHandler ), (void *)RSI_TOGGLE_BTN );

    window = gtk_builder_get_object (builder, "ViewRSIWindow");
    g_signal_connect( window, "delete_event", G_CALLBACK( GUICallbackHandler_hide_rsi_on_delete ), NULL);
    g_signal_connect ( window, "configure-event", G_CALLBACK ( GUICallbackHandler_window_data ), (void *)GUI_RSI_WINDOW );
    gtk_window_resize ( GTK_WINDOW ( window ), W->rsi_width, W->rsi_height );
    gtk_window_move ( GTK_WINDOW ( window ), W->rsi_x_pos, W->rsi_y_pos );

    button = gtk_builder_get_object (builder, "RSIMenuCloseBTN");
    g_signal_connect ( button, "activate", G_CALLBACK ( GUICallbackHandler ), (void *)RSI_TOGGLE_BTN );

    button = gtk_builder_get_object (builder, "ViewRSIFetchDataBTN");
    g_signal_connect ( button, "clicked", G_CALLBACK ( GUICallbackHandler ), (void *)RSI_FETCH_BTN );

    button = gtk_builder_get_object (builder, "ViewRSISymbolEntryBox");
    g_signal_connect ( button, "changed", G_CALLBACK ( GUICallbackHandler ), (void *)RSI_CURSOR_MOVE );


    /* This is the main window's TreeView ( There was only one treeview when I started ). */
    button = gtk_builder_get_object (builder, "TreeView");
    g_signal_connect ( button, "button-press-event", G_CALLBACK( view_onButtonPressed ), NULL );
}

static void start_threads ()
{
    /* Start the clock threads. */
    /* gdk_threads_add_idle (in these threads) creates a pending event for the gtk_main loop.
       When the gtk_main loop starts the event will be processed. */
    pthread_t thread_id;
    pthread_create( &thread_id, NULL, GUIThreadHandler, (void *)MAIN_CLOCK );
    pthread_create( &thread_id, NULL, GUIThreadHandler, (void *)MAIN_TIME_CLOSE_INDICATOR );

    /* Set up the RSIView and AddRemSecurity Window's EntryBox Completion Widgets
       This will download the NYSE and NASDAQ symbol lists when the application loads.
       If there is no internet connection or the server is unavailable cURL will return an
       error, but otherwise the application should run as normal.

       Comment out the next line to prevent the symbol list download. */
    pthread_create( &thread_id, NULL, GUIThreadHandler, (void *)COMPLETION );
}

/* Engineering Note */

/* After the gtk_main loop starts nearly all computational tasks 
   are running in a separate thread concurrently with the main thread. 

   The GUIThreadHandler function in gui_threads.c is essentially the 
   heart of the application after the main loop starts. Nearly every 
   gui signal is connected to a callback function which then creates 
   a new thread and passes a signal to the GUIThreadHandler function.

   When the GUI needs to be updated a gdk_threads_add_idle statement 
   sends a function into the main loop. */

void GuiStart (void *data)
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

    /* Start the clock threads and download the list of stock symbols */
    start_threads ();

    /* Add the keyboard shortcuts to the Keyboard Shortcut window. */
    shortcuts_set_treeview ();

    /* Add the license to the About window. */
    about_set_label ();

    /* Connect callback functions to corresponding GUI signals. */
    gui_signal_connect ( data );

    /* Set the default treeview.
       This treeview is already set in the COMPLETION thread,
       however, there may be a networking delay before it is displayed.
       So we set it here showing any available data. */
    MainDefaultTreeview ( data );

    /* Start the gtk_main loop. */
    gtk_main ();
}