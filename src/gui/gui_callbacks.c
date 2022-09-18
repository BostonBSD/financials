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

void AddRemoveSecuritySwitch (GtkSwitch *widget, bool state, void *data)
/* The "state-set" signal handler requires three parameters:

"gboolean user_function (GtkSwitch *widget, gboolean state, gpointer user_data)"

This prevents this signal from being handled by the general GUICallbackHandler
function, which takes two parameters. */
{   
    /* The Gtk3.0 callback function prototype includes a state and 
       widget parameter, which we do not use, the following two 
       statements prevent a compiler warning/error. */
    if( !( gtk_widget_get_sensitive ( GTK_WIDGET ( widget ) ) ) ) return;
    if ( state != false && state != true ) return;

    /* Initiate a thread */
        
    /* Threads will prevent the program from blocking the Gtk loop. 
    (we could also use gthreads). */
    
    pthread_t thread_id;
    /* Create a thread, pass the func and widget signal to it. */
    pthread_create( &thread_id, NULL, GUIThreadHandler, data );
}

void GUICallbackHandler ( GtkWidget *widget, void *data )
/* The widget callback functions block the gui loop until they return,
   therefore we do not want a pthread_join statement in this function. */
{
    /* The Gtk3.0 callback function prototype includes a widget parameter, 
       which we do not use, the following statement prevents a compiler 
       warning/error. */
    if( !( gtk_widget_get_sensitive ( widget ) ) ) return;    
    
    pthread_t thread_id;
    /* Create a thread, pass the func and widget signal to it. */
    pthread_create( &thread_id, NULL, GUIThreadHandler, data );
}

gboolean CallbackHandler_alt ( GtkWidget *window )
{
    gint width, height, x, y;
    gtk_window_get_size ( GTK_WINDOW( window ), &width, &height );
    gtk_window_get_position ( GTK_WINDOW ( window ), &x, &y );

    MainWindowStruct.width = (int)width;
    MainWindowStruct.height = (int)height;
    
    MainWindowStruct.x_pos = (int)x;
    MainWindowStruct.y_pos = (int)y;

    /* TRUE to stop other handlers from being invoked for the event. 
       FALSE to propagate the event further. 
    */
    return false;
}