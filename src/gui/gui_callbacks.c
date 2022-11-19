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

#include "../include/gui.h"
#include "../include/gui_types.h"
#include "../include/gui_globals.h"

#include "../include/sqlite.h"
#include "../include/class_types.h"         /* portfolio_packet, equity_folder, metal, meta */
#include "../include/globals.h"             /* portfolio_packet packet */
#include "../include/mutex.h"               /* pthread_mutex_t mutex_working[ MUTEX_NUMBER ] */

void GUICallbackHandler (GtkWidget *widget, void *data)
/* The widget callback functions block the gui loop until they return,
   therefore we do not want a pthread_join statement in this function. */
{
    /* The Gtk3.0 callback function prototype includes a widget parameter, 
       which we do not use, the following statement prevents a compiler 
       warning/error. */
    if( !( widget ) ) return; 
    
    pthread_t thread_id;
    /* Create a thread, pass the func and widget signal to it. */
    pthread_create( &thread_id, NULL, GUIThreadHandler, data );
}

void GUICallbackHandler_add_rem_switch (GtkSwitch *widget, bool state, void *data)
/* The "state-set" signal handler requires three parameters:

"gboolean user_function (GtkSwitch *widget, gboolean state, gpointer user_data)"

This prevents this signal from being handled by the general GUICallbackHandler
function, which takes two parameters. */
{   
    /* The Gtk3.0 callback function prototype includes a state and 
       widget parameter, which we do not use, the following two 
       statements prevent a compiler warning/error. */
    if( !( widget ) ) return;
    if ( state == false && state == true ) return;

    /* Initiate a thread */
        
    /* Threads will prevent the program from blocking the Gtk loop. 
    (we could also use gthreads). */
    
    pthread_t thread_id;
    /* Create a thread, pass the func and widget signal to it. */
    pthread_create( &thread_id, NULL, GUIThreadHandler, data );
}

gboolean GUICallbackHandler_expander_bar (GtkWidget *expander, void *data)
{
    meta *D = packet->portfolio_meta_info;
    *D->index_bar_expanded_bool = gtk_expander_get_expanded ( GTK_EXPANDER ( expander ) );
    /* The expansion appears to be the state prior to the signal, so we invert the state */
    if ( gtk_expander_get_expanded ( GTK_EXPANDER ( expander ) ) ){
        *D->index_bar_expanded_bool = false;
    } else {
        *D->index_bar_expanded_bool = true;
    }

    /* TRUE to stop other handlers from being invoked for the event. 
       FALSE to propagate the event further. 
    */
    return false;
}

gboolean GUICallbackHandler_window_data (GtkWidget *window, GdkEvent *event, void *data)
{
    /*
        The "event->configure.x" and "event->configure.y" data members are slightly
        less accurate than the gtk_window_get_position function, so we're not using 
        the GdkEvent data type.
    */
    if( !( event ) ) return false;
    gint width, height, x, y;
    gtk_window_get_size ( GTK_WINDOW( window ), &width, &height );
    gtk_window_get_position ( GTK_WINDOW ( window ), &x, &y );

    int s = (int)((uintptr_t)data);    

    switch( s ){
        case GUI_MAIN_WINDOW:
            WindowStruct.main_width = (int)width;
            WindowStruct.main_height = (int)height;

            WindowStruct.main_x_pos = (int)x;
            WindowStruct.main_y_pos = (int)y;
            break;
        case GUI_RSI_WINDOW:
            WindowStruct.rsi_width = (int)width;
            WindowStruct.rsi_height = (int)height;
            
            WindowStruct.rsi_x_pos = (int)x;
            WindowStruct.rsi_y_pos = (int)y;
            break;    
    }

    /* TRUE to stop other handlers from being invoked for the event. 
       FALSE to propagate the event further. 
    */
    return false;
}

gboolean GUICallbackHandler_select_comp (GtkEntryCompletion *completion, GtkTreeModel *model, GtkTreeIter *iter )
/* activated when an item is selected from the completion list */
{
    if( !( completion ) ) return true;
    GtkWidget* EntryBoxRSI = GTK_WIDGET ( gtk_builder_get_object (builder, "ViewRSISymbolEntryBox") );
    GtkWidget* EntryBoxAddRem = GTK_WIDGET ( gtk_builder_get_object (builder, "AddRemoveSecuritySymbolEntryBox") );
    GtkWidget* EntryBox = NULL;
    if ( gtk_widget_has_focus ( EntryBoxRSI ) ){
        EntryBox = EntryBoxRSI;
    } else {
        EntryBox = EntryBoxAddRem;
    }
    gchar *item;
    /* when a match is selected insert column zero instead of column 2 */
    gtk_tree_model_get ( model, iter, 0, &item, -1 );
    /* This function is already blocking the gtk main loop, it's ok 
       to change widgets here without using a "gdk_threads_add_idle" wrapper
       function. */
    gtk_entry_set_text ( GTK_ENTRY( EntryBox ), item );
    int pos = strlen( item );
    free( item );

    /* move the cursor to the end of the string */
    gtk_editable_set_position ( GTK_EDITABLE( EntryBox ), pos );
    return true;
}

gboolean GUICallbackHandler_cursor_comp (GtkEntryCompletion *completion, GtkTreeModel *model, GtkTreeIter *iter )
/* activated when an item is highlighted from the completion list */
{
    if( !( completion ) ) return true;
    GtkWidget* EntryBoxRSI = GTK_WIDGET ( gtk_builder_get_object (builder, "ViewRSISymbolEntryBox") );
    GtkWidget* EntryBoxAddRem = GTK_WIDGET ( gtk_builder_get_object (builder, "AddRemoveSecuritySymbolEntryBox") );
    GtkWidget* EntryBox = NULL;
    if ( gtk_widget_has_focus ( EntryBoxRSI ) ){
        EntryBox = EntryBoxRSI;
    } else {
        EntryBox = EntryBoxAddRem;
    }
    gchar *item;
    /* when a match is highlighted insert column zero instead of column 2 */
    gtk_tree_model_get ( model, iter, 0, &item, -1 );
    /* This function is already blocking the gtk main loop, it's ok 
       to change widgets here without using a "gdk_threads_add_idle" wrapper
       function. */
    gtk_entry_set_text ( GTK_ENTRY( EntryBox ), item );
    int pos = strlen( item );
    free( item );

    /* move the cursor to the end of the string */
    gtk_editable_set_position ( GTK_EDITABLE( EntryBox ), pos );
    return true;
}

static void view_popup_menu_onViewRSIData (GtkWidget *menuitem, void *userdata)
{
    if (menuitem == NULL) return;
    char *symbol = (char*) userdata;

    GtkWidget* Window = GTK_WIDGET ( gtk_builder_get_object (builder, "ViewRSIWindow") );
    GtkWidget* EntryBox = GTK_WIDGET ( gtk_builder_get_object (builder, "ViewRSISymbolEntryBox") );
    GtkWidget* Button = GTK_WIDGET ( gtk_builder_get_object (builder, "ViewRSIFetchDataBTN") );
    gboolean visible = gtk_widget_is_visible ( Window );

    if( !visible ) RSIShowHide ();

    gtk_entry_set_text ( GTK_ENTRY( EntryBox ), symbol );
    /* move the cursor to the end of the string */
    gtk_editable_set_position ( GTK_EDITABLE( EntryBox ), strlen( symbol ) );

    gtk_button_clicked ( GTK_BUTTON ( Button ) );
}

static void view_popup_menu_onDeleteRow (GtkWidget *menuitem, void *userdata)
{
    if (menuitem == NULL) return;
    char *symbol = (char*) userdata;

    portfolio_packet *pkg = packet;
    equity_folder *F = pkg->securities_folder;
    meta *D = pkg->portfolio_meta_info;

    /* Prevents Program From Crashing During A Data Fetch Operation */
    pthread_mutex_lock( &mutex_working[ FETCH_DATA_MUTEX ] );

    SqliteRemoveEquity( symbol, D );
    F->RemoveStock ( symbol );

    pthread_mutex_unlock( &mutex_working[ FETCH_DATA_MUTEX ] );

    if( packet->IsFetchingData () == false ) {
        gdk_threads_add_idle ( MainDefaultTreeview, packet );
    } else {
        packet->Calculate ();
        packet->ToStrings ();
        /* Set Gtk treeview. */
        gdk_threads_add_idle ( MainPrimaryTreeview, packet );
    }
}

static void view_popup_menu_onDeleteAllEquityRows (GtkWidget *menuitem)
{
    if (menuitem == NULL) return;
    portfolio_packet *pkg = packet;
    equity_folder *F = pkg->securities_folder;
    meta *D = pkg->portfolio_meta_info;

    /* Prevents Program From Crashing During A Data Fetch Operation */
    pthread_mutex_lock( &mutex_working[ FETCH_DATA_MUTEX ] );

    SqliteRemoveAllEquity( D );
    
    /* Reset Equity Folder */
    F->Reset ();

    pthread_mutex_unlock( &mutex_working[ FETCH_DATA_MUTEX ] );

    if( packet->IsFetchingData () == false ) {
        gdk_threads_add_idle ( MainDefaultTreeview, packet );
    } else {
        packet->Calculate ();
        packet->ToStrings ();
        /* Set Gtk treeview. */
        gdk_threads_add_idle ( MainPrimaryTreeview, packet );
    }
}

static void view_popup_menu_onAddRow (GtkWidget *menuitem, void *userdata)
{
    if (menuitem == NULL) return;
    char *type = (char*) userdata;

    if ( strcmp( type, "equity" ) == 0 ){
        gdk_threads_add_idle( AddRemShowHide, packet );
    } else if ( strcmp( type, "equity_total" ) == 0 ) {
        gdk_threads_add_idle( AddRemShowHide, packet );
    } else if ( strcmp( type, "bullion" ) == 0 ) {
        gdk_threads_add_idle( BullionShowHide, packet );
    } else if ( strcmp( type, "cash" ) == 0 ) {
        gdk_threads_add_idle( CashShowHide, packet );
    }
}

gboolean view_onButtonPressed (GtkWidget *treeview, GdkEventButton *event)
{
    GtkTreeModel *model;
    GtkTreeIter iter;
    GtkWidget *menu, *menuitem;
    char *type = NULL;
    char *symbol = NULL;

    /* single click with the right mouse button */
    if ( event->type == GDK_BUTTON_PRESS  &&  event->button == 3 )
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
                gtk_tree_model_get ( model, &iter, GUI_TYPE, &type, GUI_SYMBOL, &symbol, -1 );
                if ( !type || !symbol ) return GDK_EVENT_STOP;

                int eq_flag = strcmp( type, "equity" );
                int eq_tot_flag = strcmp( type, "equity_total" );
                int bu_flag = strcmp( type, "bullion" );
                int ca_flag = strcmp( type, "cash" );
                int bs_flag = strcmp( type, "blank_space" );

                /* Some of the menu signal connections need the type and symbol strings.
                   We make the container static and free it on subsequent clicks, which keeps the
                   strings available to the signal connections. */
                static right_click_container *my_data = NULL;
                if ( my_data ){
                    free( my_data->type );
                    free( my_data->symbol );
                    free( my_data );
                }
                my_data = ( right_click_container* ) malloc ( sizeof( right_click_container ) );
                my_data->type = strdup( type );
                my_data->symbol = strdup( symbol );

                free( type );
                free( symbol );

                if( eq_flag == 0 )
                /* If the type is an equity enable row deletion. */
                {  
                    menu = gtk_menu_new();
                    menuitem = gtk_menu_item_new_with_label("View RSI Data");
                    g_signal_connect(menuitem, "activate", G_CALLBACK( view_popup_menu_onViewRSIData ), my_data->symbol);
                    gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
                    g_object_ref_sink( G_OBJECT ( menuitem ) );

                    menuitem = gtk_menu_item_new_with_label("Add / Edit Equity");
                    g_signal_connect(menuitem, "activate", G_CALLBACK( view_popup_menu_onAddRow ), my_data->type);
                    gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
                    g_object_ref_sink( G_OBJECT ( menuitem ) );

                    size_t len = strlen("Delete Entry ") + strlen( my_data->symbol ) + 1;
                    char *menu_label = (char*)malloc( len );
                    snprintf(menu_label, len, "Delete Entry %s", my_data->symbol );
                    menuitem = gtk_menu_item_new_with_label( menu_label );
                    g_signal_connect(menuitem, "activate", G_CALLBACK( view_popup_menu_onDeleteRow ), my_data->symbol);
                    gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
                    free ( menu_label );
                    g_object_ref_sink( G_OBJECT ( menuitem ) );

                    menuitem = gtk_menu_item_new_with_label("Delete All Equity");
                    g_signal_connect(menuitem, "activate", G_CALLBACK( view_popup_menu_onDeleteAllEquityRows ), NULL);
                    gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
                    g_object_ref_sink( G_OBJECT ( menuitem ) );

                    gtk_widget_show_all(menu);
                    gtk_menu_popup_at_pointer (GTK_MENU(menu), (GdkEvent*)event); 
                    
                    g_object_ref_sink( G_OBJECT ( menu ) );

                } else if ( eq_tot_flag == 0 ) 
                {
                    menu = gtk_menu_new();
                    menuitem = gtk_menu_item_new_with_label("Delete All Equity");
                    g_signal_connect(menuitem, "activate", G_CALLBACK( view_popup_menu_onDeleteAllEquityRows ), NULL);
                    gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
                    g_object_ref_sink( G_OBJECT ( menuitem ) );

                    menuitem = gtk_menu_item_new_with_label("Add / Edit Equity");
                    g_signal_connect(menuitem, "activate", G_CALLBACK( view_popup_menu_onAddRow ), my_data->type);
                    gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
                    g_object_ref_sink( G_OBJECT ( menuitem ) );

                    gtk_widget_show_all(menu);
                    gtk_menu_popup_at_pointer (GTK_MENU(menu), (GdkEvent*)event);

                    g_object_ref_sink( G_OBJECT ( menu ) );

                } else if ( (bu_flag == 0) || (ca_flag == 0) ) 
                {
                    if (bu_flag == 0){
                        menuitem = gtk_menu_item_new_with_label("Edit Bullion");
                    } else {
                        menuitem = gtk_menu_item_new_with_label("Edit Cash");
                    }

                    menu = gtk_menu_new();
                    g_signal_connect(menuitem, "activate", G_CALLBACK( view_popup_menu_onAddRow ), my_data->type);
                    gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
                    g_object_ref_sink( G_OBJECT ( menuitem ) );

                    gtk_widget_show_all(menu);
                    gtk_menu_popup_at_pointer (GTK_MENU(menu), (GdkEvent*)event); 

                    g_object_ref_sink( G_OBJECT ( menu ) );

                } else if ( bs_flag == 0 ){
                    menu = gtk_menu_new();

                    menuitem = gtk_menu_item_new_with_label("Edit Cash");
                    g_signal_connect(menuitem, "activate", G_CALLBACK( view_popup_menu_onAddRow ), "cash");
                    gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
                    g_object_ref_sink( G_OBJECT ( menuitem ) );

                    menuitem = gtk_menu_item_new_with_label("Edit Bullion");
                    g_signal_connect(menuitem, "activate", G_CALLBACK( view_popup_menu_onAddRow ), "bullion");
                    gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
                    g_object_ref_sink( G_OBJECT ( menuitem ) );

                    menuitem = gtk_menu_item_new_with_label("Add / Edit Equity");
                    g_signal_connect(menuitem, "activate", G_CALLBACK( view_popup_menu_onAddRow ), "equity");
                    gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
                    g_object_ref_sink( G_OBJECT ( menuitem ) );                   

                    gtk_widget_show_all(menu);
                    gtk_menu_popup_at_pointer (GTK_MENU(menu), (GdkEvent*)event);

                    g_object_ref_sink( G_OBJECT ( menu ) );
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