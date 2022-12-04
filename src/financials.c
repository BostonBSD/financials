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

static void class_package_init ()
/* Initialize the class package */
{
    packet = class_init_portfolio_packet ();
}

static void class_package_destruct ()
/* Free Memory */
{
    pthread_mutex_lock( &mutex_working [ CLASS_MEMBER_MUTEX ] );

    class_destruct_portfolio_packet ( packet );

    pthread_mutex_unlock( &mutex_working [ CLASS_MEMBER_MUTEX ] );

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
    class_package_init ();

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

    /* Read config file and populate associated variables */
	ReadConfig ( packet );

    /* Set up GUI widgets and display the GUI */
    GuiStart ( packet );

    /* Free Class Instances. */
    class_package_destruct ();

    /* Free Mutex Resources */
    mutex_destruct ();

    /* Free Semaphore Resources */
    semaphore_destruct ();

    return 0;
}