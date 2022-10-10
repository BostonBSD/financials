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
#include "../include/ui.h"

GtkBuilder *builder;
window_data WindowStruct;

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

static void about_set_label ()
{
    /* Set the About window license. */
    GtkWidget* Label = GTK_WIDGET ( gtk_builder_get_object (builder, "AboutLicenseLabel") );
    gtk_label_set_text ( GTK_LABEL ( Label ), LICENSE);
}

static void gui_signal_connect ()
/* Connect GUI index signals to the signal handlers. */
{
    GObject *window,*button;

    /* Connect signal handlers to the constructed widgets. */
    /* The last argument, the widget index signal, is an enum/int casted to a void*, 
       small to larger datatype, in the GUIThreadHandler we cast this back to an enum/int,
       because it started as an enum/int we should not worry about data loss through 
       casting truncation. */
    window = gtk_builder_get_object (builder, "MainWindow");
    g_signal_connect ( window, "destroy", G_CALLBACK ( GUICallbackHandler ), (void *)MAIN_EXIT );
    g_signal_connect ( window, "configure-event", G_CALLBACK ( GUICallbackHandler_window_data ), (void *)GUI_MAIN_WINDOW );
    gtk_window_resize ( GTK_WINDOW ( window ), WindowStruct.main_width, WindowStruct.main_height );
    gtk_window_move ( GTK_WINDOW ( window ), WindowStruct.main_x_pos, WindowStruct.main_y_pos );

    button = gtk_builder_get_object (builder, "FetchDataBTN");
    g_signal_connect ( button, "clicked", G_CALLBACK ( GUICallbackHandler ), (void *)MAIN_FETCH_BTN );
    gtk_widget_grab_focus ( GTK_WIDGET ( button ) );

    button = gtk_builder_get_object (builder, "MainMenuQuitBTN");
    g_signal_connect ( button, "activate", G_CALLBACK ( GUICallbackHandler ), (void *)MAIN_EXIT );

    button = gtk_builder_get_object (builder, "QuitBTN");
    g_signal_connect ( button, "clicked", G_CALLBACK ( GUICallbackHandler ), (void *)MAIN_EXIT );


    button = gtk_builder_get_object (builder, "MainMenuAddRemoveSecurityBTN");
    g_signal_connect ( button, "activate", G_CALLBACK ( GUICallbackHandler ), (void *)EQUITY_TOGGLE_BTN );

    window = gtk_builder_get_object (builder, "AddRemoveSecurity");
    g_signal_connect( window, "delete_event", G_CALLBACK( gtk_widget_hide_on_delete ), NULL);

    button = gtk_builder_get_object (builder, "AddRemoveSecurityOkBTN");
    g_signal_connect ( button, "clicked", G_CALLBACK ( GUICallbackHandler ), (void *)EQUITY_OK_BTN );

    button = gtk_builder_get_object (builder, "AddRemoveSecurityCancelBTN");
    g_signal_connect ( button, "clicked", G_CALLBACK ( GUICallbackHandler ), (void *)EQUITY_CANCEL_BTN );

    button = gtk_builder_get_object (builder, "AddRemoveSecuritySwitch");
    g_signal_connect ( button, "state-set", G_CALLBACK ( GUICallbackHandler_add_rem_switch ), (void *)EQUITY_SWITCH );

    button = gtk_builder_get_object (builder, "AddRemoveSecurityComboBox");
    g_signal_connect ( button, "changed", G_CALLBACK ( GUICallbackHandler ), (void *)EQUITY_COMBO_BOX );

    button = gtk_builder_get_object (builder, "AddRemoveSecuritySymbolEntryBox");
    g_signal_connect ( button, "changed", G_CALLBACK ( GUICallbackHandler ), (void *)EQUITY_CURSOR_MOVE );

    button = gtk_builder_get_object (builder, "AddRemoveSecuritySharesEntryBox");
    g_signal_connect ( button, "changed", G_CALLBACK ( GUICallbackHandler ), (void *)EQUITY_CURSOR_MOVE );

    
    button = gtk_builder_get_object (builder, "MainMenuAboutBTN");
    g_signal_connect ( button, "activate", G_CALLBACK ( GUICallbackHandler ), (void *)ABOUT_TOGGLE_BTN );

    window = gtk_builder_get_object (builder, "AboutWindow");
    g_signal_connect( window, "delete_event", G_CALLBACK( gtk_widget_hide_on_delete ), NULL);

    button = gtk_builder_get_object (builder, "AboutOKBTN");
    g_signal_connect ( button, "clicked", G_CALLBACK ( GUICallbackHandler ), (void *)ABOUT_TOGGLE_BTN );


    button = gtk_builder_get_object (builder, "MainMenuShortcutBTN");
    g_signal_connect ( button, "activate", G_CALLBACK ( GUICallbackHandler ), (void *)SHORTCUT_TOGGLE_BTN );

    window = gtk_builder_get_object (builder, "ShortcutWindow");
    g_signal_connect( window, "delete_event", G_CALLBACK( gtk_widget_hide_on_delete ), NULL);

    button = gtk_builder_get_object (builder, "ShortcutOKBTN");
    g_signal_connect ( button, "clicked", G_CALLBACK ( GUICallbackHandler ), (void *)SHORTCUT_TOGGLE_BTN );


    button = gtk_builder_get_object (builder, "MainMenuAddRemoveBullionBTN");
    g_signal_connect ( button, "activate", G_CALLBACK ( GUICallbackHandler ), (void *)BUL_TOGGLE_BTN );

    window = gtk_builder_get_object (builder, "AddRemoveBullionWindow");
    g_signal_connect( window, "delete_event", G_CALLBACK( gtk_widget_hide_on_delete ), NULL);

    button = gtk_builder_get_object (builder, "AddRemoveBullionOKBTN");
    g_signal_connect ( button, "clicked", G_CALLBACK ( GUICallbackHandler ), (void *)BUL_OK_BTN );
    gtk_widget_set_sensitive ( GTK_WIDGET ( button ) , false );

    button = gtk_builder_get_object (builder, "AddRemoveBullionCancelBTN");
    g_signal_connect ( button, "clicked", G_CALLBACK ( GUICallbackHandler ), (void *)BUL_CANCEL_BTN );

    button = gtk_builder_get_object (builder, "AddRemoveBullionGoldOuncesEntryBox");
    g_signal_connect ( button, "changed", G_CALLBACK ( GUICallbackHandler ), (void *)BUL_CURSOR_MOVE );

    button = gtk_builder_get_object (builder, "AddRemoveBullionGoldPremiumEntryBox");
    g_signal_connect ( button, "changed", G_CALLBACK ( GUICallbackHandler ), (void *)BUL_CURSOR_MOVE );

    button = gtk_builder_get_object (builder, "AddRemoveBullionSilverOuncesEntryBox");
    g_signal_connect ( button, "changed", G_CALLBACK ( GUICallbackHandler ), (void *)BUL_CURSOR_MOVE );

    button = gtk_builder_get_object (builder, "AddRemoveBullionSilverPremiumEntryBox");
    g_signal_connect ( button, "changed", G_CALLBACK ( GUICallbackHandler ), (void *)BUL_CURSOR_MOVE );


    button = gtk_builder_get_object (builder, "MainMenuAddRemoveCashBTN");
    g_signal_connect ( button, "activate", G_CALLBACK ( GUICallbackHandler ), (void *)CASH_TOGGLE_BTN );

    window = gtk_builder_get_object (builder, "AddRemoveCashWindow");
    g_signal_connect( window, "delete_event", G_CALLBACK( gtk_widget_hide_on_delete ), NULL);

    button = gtk_builder_get_object (builder, "AddRemoveCashOKBTN");
    g_signal_connect ( button, "clicked", G_CALLBACK ( GUICallbackHandler ), (void *)CASH_OK_BTN );
    gtk_widget_set_sensitive ( GTK_WIDGET ( button ) , false );

    button = gtk_builder_get_object (builder, "AddRemoveCashCancelBTN");
    g_signal_connect ( button, "clicked", G_CALLBACK ( GUICallbackHandler ), (void *)CASH_CANCEL_BTN );

    button = gtk_builder_get_object (builder, "AddRemoveCashValueEntryBox");
    g_signal_connect ( button, "changed", G_CALLBACK ( GUICallbackHandler ), (void *)CASH_CURSOR_MOVE );

    button = gtk_builder_get_object (builder, "MainMenuChangeAPIinfoBTN");
    g_signal_connect ( button, "activate", G_CALLBACK ( GUICallbackHandler ), (void *)API_TOGGLE_BTN );

    window = gtk_builder_get_object (builder, "ChangeApiInfoWindow");
    g_signal_connect( window, "delete_event", G_CALLBACK( gtk_widget_hide_on_delete ), NULL);

    button = gtk_builder_get_object (builder, "ChangeApiInfoOKBTN");
    g_signal_connect ( button, "clicked", G_CALLBACK ( GUICallbackHandler ), (void *)API_OK_BTN );
    gtk_widget_set_sensitive ( GTK_WIDGET ( button ) , false );

    button = gtk_builder_get_object (builder, "ChangeApiInfoCancelBTN");
    g_signal_connect ( button, "clicked", G_CALLBACK ( GUICallbackHandler ), (void *)API_CANCEL_BTN );

    button = gtk_builder_get_object (builder, "ChangeApiInfoEquityUrlEntryBox");
    g_signal_connect ( button, "changed", G_CALLBACK ( GUICallbackHandler ), (void *)API_CURSOR_MOVE );

    button = gtk_builder_get_object (builder, "ChangeApiInfoUrlKeyEntryBox");
    g_signal_connect ( button, "changed", G_CALLBACK ( GUICallbackHandler ), (void *)API_CURSOR_MOVE );

    button = gtk_builder_get_object (builder, "ChangeApiInfoHoursSpinBox");
    g_signal_connect ( button, "changed", G_CALLBACK ( GUICallbackHandler ), (void *)API_CURSOR_MOVE );


    button = gtk_builder_get_object (builder, "MainMenuViewRSIBTN");
    g_signal_connect ( button, "activate", G_CALLBACK ( GUICallbackHandler ), (void *)RSI_TOGGLE_BTN );

    window = gtk_builder_get_object (builder, "ViewRSIWindow");
    g_signal_connect( window, "delete_event", G_CALLBACK( gtk_widget_hide_on_delete ), NULL);
    g_signal_connect ( window, "configure-event", G_CALLBACK ( GUICallbackHandler_window_data ), (void *)GUI_RSI_WINDOW );
    gtk_window_resize ( GTK_WINDOW ( window ), WindowStruct.rsi_width, WindowStruct.rsi_height );
    gtk_window_move ( GTK_WINDOW ( window ), WindowStruct.rsi_x_pos, WindowStruct.rsi_y_pos );

    button = gtk_builder_get_object (builder, "ViewRSICloseBTN");
    g_signal_connect ( button, "clicked", G_CALLBACK ( GUICallbackHandler ), (void *)RSI_TOGGLE_BTN );

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

    /* Set up the RSIView Window's EntryBox Completion Widget 
       This will download the NYSE and NASDAQ symbol list when the application loads.
       If there is no internet connection or the server is unavailable cURL will return an
       error, but otherwise the application should run as normal.
    */
    pthread_create( &thread_id, NULL, GUIThreadHandler, (void *)RSI_COMPLETION );
}

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

    /* Add the keyboard shortcuts to the Keyboard Shortcut window. */
    shortcuts_set_treeview ();

    /* Add the license to the About window. */
    about_set_label ();

    /* Set the default treeview. */
    MainDefaultTreeview ( data );

    /* Connect callback functions to corresponding GUI signals. */
    gui_signal_connect ();

    /* Start the clock threads and download list of stock symbols */
    start_threads ();

    /* Start the gtk_main loop. */
    gtk_main ();
}