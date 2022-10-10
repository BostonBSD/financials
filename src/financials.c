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

#include "financials.h"

/* These four globals are only accessible from financials.c, 
   gui_callbacks.c, and gui_threads.c */
/* All other functions access the class data through pass by reference */
equity_folder* Folder;  /* A class handle to an array of stock class objects, can change dynamically. */    
metal* Precious;        /* A class handle to the bullion class object pointers. */
meta* MetaData;         /* A class object pointer called MetaData. */
portfolio_packet packet;                    /* The gui worker functions need access to the classes, 
                                               so we package them and send a packet through 
                                               gdk_threads_add_idle */


pthread_mutex_t mutex_working[ MUTEX_NUMBER ];    /* A Posix Mutex Array */
/*  Semaphores are the same as mutexes with the added capability to conduct
    interprocess communication, we use them here the same way as a mutex just 
    to illustrate their use. */
sem_t semaphore[ SIGNAL_NUM ];                    /* A Posix Semaphore Array */

static void mutex_init ()
/* Initialize Mutexes */
{
    short g = 0;
    while (g < MUTEX_NUMBER){
        if ( pthread_mutex_init( &mutex_working[g], NULL ) != 0 ) {
            printf( "\nPosix mutex init has failed\n" );
            exit( EXIT_FAILURE );
        }
        g++;
    }
}

static void mutex_destruct ()
/* Free Mutex Resources */
{
    short g = 0;
    while (g < MUTEX_NUMBER){
        pthread_mutex_destroy ( &mutex_working[g] );
        g++;
    }

}

static void semaphore_init ()
/* Initialize Semaphores */
{
    short g = 0;
    while (g < SIGNAL_NUM){
        if ( sem_init( &semaphore[g], 0, 1 ) != 0 ) {
            printf( "\nPosix semaphore init has failed\n" );
            exit( EXIT_FAILURE );
        }
        g++;
    }
}

static void semaphore_destruct ()
/* Free Semaphore Resources */
{
    short g = 0;
    while (g < SIGNAL_NUM){
        sem_destroy ( &semaphore[g] );
        g++;
    }
}

static void window_data_init ()
/* Initialize the Main and RSI Window size and position. */
{
    WindowStruct.main_height = 0;
    WindowStruct.main_width = 0;
    WindowStruct.main_x_pos = 0;
    WindowStruct.main_y_pos = 0;
    WindowStruct.rsi_height = 0;
    WindowStruct.rsi_width = 0;
    WindowStruct.rsi_x_pos = 0;
    WindowStruct.rsi_y_pos = 0;
}

static void class_instance_init ()
/* Initialize some of our class instances */
{
    Folder = class_init_equity_folder ();
    Precious = class_init_metal ();
    MetaData = class_init_meta_data ();
}

static void class_package_init ()
/* Initialize the class package */
{
    /* Set the packet members to the class object pointers */
    /* These pointers should not change for the duration of the runtime. */
    packet.metal_chest = Precious;
    packet.securities_folder = Folder;
    packet.portfolio_meta_info = MetaData;
}

static void class_instance_destruct ()
/* Free Memory */
{
    pthread_mutex_lock( &mutex_working [CLASS_MEMBER_MUTEX ] );

    if ( Precious ) class_destruct_metal ( Precious );
    if ( Folder ) class_destruct_equity_folder ( Folder );
    if ( MetaData ) class_destruct_meta_data ( MetaData );

    pthread_mutex_unlock( &mutex_working [CLASS_MEMBER_MUTEX ] );

}

static void set_new_york_time_zone ()
/* Set the process time zone to New York */
{
    setenv("TZ", NEW_YORK_TIME_ZONE, 1);
    tzset();
}

int main (int argc, char *argv[])
{
    /* Set the process time zone to New York */
    set_new_york_time_zone ();

    /* Initialize some of our class instances */
    /* This needs to be initialized before ReadConfig */
    class_instance_init ();

    /* Initialize Xlib support for concurrent threads */
    /* This needs to be initialized before gtk_init */

    /* Some sources say this is not required because gdk_threads_add_idle
       will do this automatically.  Other sources say they've experienced
       problems without it while using gdk_threads_add_idle. */
    XInitThreads ();

    /* Initialize gtk */
    gtk_init ( &argc, &argv );

    /* Initialize Mutexes */
    /* This needs to be initialized before ReadConfig */
    mutex_init ();

    /* Initialize Semaphores */
    semaphore_init ();

    /* Initialize the Main and RSI Window size and position struct. */
    /* This needs to be initialized before ReadConfig */
    window_data_init ();

    /* Read config file and populate associated variables */
	ReadConfig ( Precious, MetaData, Folder, &WindowStruct );

    /* Initialize the package of class objects */
    /* This needs to be initialized before GuiStart */
    class_package_init ();

    /* Set up GUI widgets and display the GUI */
    GuiStart ( &packet );

    /* Free Mutex Resources */
    mutex_destruct ();

    /* Free Semaphore Resources */
    semaphore_destruct ();
    
    /* Free Remaining Memory. */
    class_instance_destruct ();

    return 0;
}