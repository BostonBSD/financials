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

equity_folder* Folder;  /* A handle to an array of stock class objects, can change dynamically. */    
metal* Precious;        /* A handle to the bullion class object pointers. */
meta* MetaData;         /* A class object pointer called MetaData. */

/*  gthread is just a wrapper for pthread.  
    They both work the same way. */
GMutex mutex_interface;  /* A Glib Mutex (we don't really need two types of mutexes, 
                            but here to illustrate both) */
pthread_mutex_t mutex_working[ MUTEX_NUMBER ];    /* A Posix Mutex Array */
/*  Semaphores are the same as mutexes with the added capability to conduct
    interprocess communication, we use them here the same way as a mutex just 
    to illustrate their use. */
sem_t semaphore[ SIGNAL_NUM ];                    /* A Posix Semaphore Array */

void Destruct ()
/* Free Memory */
{
    if ( Precious ) class_destruct_metal ( Precious );
    if ( Folder ) class_destruct_equity_folder ( Folder );
    if ( MetaData ) class_destruct_meta_data ( MetaData );
}

void Mutex_Init ()
/* Initialize Mutexes */
{
    g_mutex_init ( &mutex_interface );

    short g = 0;
    while (g < MUTEX_NUMBER){
        if ( pthread_mutex_init( &mutex_working[g], NULL ) != 0 ) {
            printf( "\nPosix mutex init has failed\n" );
            exit( EXIT_FAILURE );
        }
        g++;
    }
}

void Mutex_Destruct ()
/* Free Mutex Resources */
{
    g_mutex_clear ( &mutex_interface );

    short g = 0;
    while (g < MUTEX_NUMBER){
        pthread_mutex_destroy ( &mutex_working[g] );
        g++;
    }

}

void Semaphore_Init ()
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

void Semaphore_Destruct ()
/* Free Semaphore Resources */
{
    short g = 0;
    while (g < SIGNAL_NUM){
        sem_destroy ( &semaphore[g] );
        g++;
    }
}

int main (int argc, char *argv[])
{
    /* Initialize some of our class instances */
    Folder = class_init_equity_folder ();
    Precious = class_init_metal ();
    MetaData = class_init_meta_data ();

    /* Initialize the Main and RSI Window size and position. */
    WindowStruct.main_height = 0;
    WindowStruct.main_width = 0;
    WindowStruct.main_x_pos = 0;
    WindowStruct.main_y_pos = 0;
    WindowStruct.rsi_height = 0;
    WindowStruct.rsi_width = 0;
    WindowStruct.rsi_x_pos = 0;
    WindowStruct.rsi_y_pos = 0;

    /* Read config file and populate associated variables */
	ReadConfig( Precious, MetaData, Folder );

    /* Initialize gtk */
    gtk_init ( &argc, &argv );

    /* Initialize Mutexes */
    Mutex_Init ();

    /* Initialize Semaphores */
    Semaphore_Init ();

    /* Setup GUI widgets and display the GUI */
    SetUpGUI ();

    /* Free Mutex Resources */
    Mutex_Destruct ();

    /* Free Semaphore Resources */
    Semaphore_Destruct ();
    
    /* Free Remaining Memory. */
    Destruct ();

    return 0;
}