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

#include "../include/gui_types.h"     /* symbol_name_map, cb_signal, etc */
#include "../include/gui_globals.h"   /* sem_t semaphore[ SIGNAL_NUM ] */
#include "../include/gui.h"

#include "../include/workfuncs.h"

#include "../include/globals.h"       /* portfolio_packet packet */
#include "../include/mutex.h"         /* pthread_mutex_t mutex_working[ MUTEX_NUMBER ] */

static symbol_name_map *sym_map = NULL;    /* A symbol to name map handle, only global to this file. */

static bool check_fetch_data_double_clicked (cb_signal index_signal){
    /* If MAIN_FETCH_BTN double clicked looping stops, change btn label. */
    if ( index_signal == MAIN_FETCH_BTN && packet->IsFetchingData () == true ){
        
        packet->SetFetchingData ( false );
        gdk_threads_add_idle ( MainFetchBTNLabel, packet );
        
        /* If we are already fetching data, set fetching data to false, change button label and
           exit this thread. */
        return true;
    } else if ( index_signal == MAIN_FETCH_BTN && packet->IsFetchingData () == false ){

        packet->SetFetchingData ( true );
        gdk_threads_add_idle ( MainFetchBTNLabel, packet );

        /* If we are not fetching data, set fetching data to true, change button label and
           continue this thread. */
        return false;

    } else {
        /* Otherwise continue this thread. */
        return false;
    }
}

void *GUIThreadHandler (void *data){
    /* We're using data as a value rather than a pointer here.
       Memory addresses are the same size as uintptr_t/long.  cb_signal 
       is an enum/int [a smaller datatype], we can cast uintptr_t/long 
       to enum/int, but not pointers to enum/int, so we cast void* to a 
       uintptr_t/long first [because they are the same size], then cast 
       to enum/int (which we typedefed as cb_signal).
       
       The C standard guarantees that you can convert a pointer to 
       uintptr_t and back again. 
       
       We do not need to worry about [large type to small type] truncation 
       because the data in the void* originally was an enum/int and is 
       guaranteed to never overflow an enum/int datatype. */
    cb_signal index_signal = (cb_signal)((uintptr_t)data);
    
    /* If MAIN_FETCH_BTN double clicked looping stops, change btn label. */
    /* This has to be outside the semaphore block otherwise the second thread, same signal,
       would never reach it. */
    if ( check_fetch_data_double_clicked ( index_signal ) ) return NULL;

    /* This semaphore will prevent double clicking from corrupting data. 
       New threads, with the same index signal, have to wait for the previous
       thread [same index signal] to terminate, before running. */
    sem_wait( &semaphore[ index_signal ] );

    unsigned int seconds_to_open;
    double loop_val, diff, seconds_per_iteration;
    time_t current_time, end_time;
    time_t start_curl, end_curl;
    struct tm NY_Time; 
    MemType *RSIOutput;
    char *sec_name, *symbol;

    switch ( index_signal )
    {
        case MAIN_FETCH_BTN:
            /* The number of seconds between data fetch operations. */
            if( packet->GetUpdatesPerMinute () <= 0.0f ){
                seconds_per_iteration = 0.0f;
            } else {
                seconds_per_iteration = 60.0f / packet->GetUpdatesPerMinute ();
            }

            /* Because loop_val is not always evenly divisible by the
               seconds_per_iteration value, our loop will finish in an 
               approximate length of time plus the slack seconds. */

            /* The number of seconds to keep looping. */
            loop_val = 3600 * packet->GetHoursOfUpdates ();
            /* If the hours to run is zero, run one loop iteration. */
            if( loop_val == 0.0f ) loop_val = 1.0f;

            time( &current_time );
            end_time = current_time + (time_t)loop_val;
            
            while( current_time < end_time && packet->IsFetchingData () ){
                /* This mutex prevents the program from crashing if an
                   MAIN_EXIT, EQUITY_OK_BTN, 
                   BUL_OK_BTN, CASH_OK_BTN, 
                   or API_OK_BTN signal is run in parallel with 
                   this thread. */

                time( &start_curl );
                pthread_mutex_lock( &mutex_working[ FETCH_DATA_MUTEX ] );     
                
                if ( packet->IsCurlCanceled () ){
                    pthread_mutex_unlock ( &mutex_working[ FETCH_DATA_MUTEX ] );
                    break;
                }

                packet->GetData ();

                if ( packet->IsCurlCanceled () ){
                    pthread_mutex_unlock ( &mutex_working[ FETCH_DATA_MUTEX ] );
                    break;
                }

                packet->ExtractData ();
                packet->Calculate ();
                packet->ToStrings ();

                pthread_mutex_unlock ( &mutex_working[ FETCH_DATA_MUTEX ] );

                /* Set Gtk treeview. */
                gdk_threads_add_idle ( MainPrimaryTreeview, packet );
           
                /* If the market is closed or today is a holiday only loop once. */
                if( loop_val == 1.0f || !packet->IsFetchingData () || packet->SecondsToOpen () || packet->IsHoliday () ) {
                    break;
                }

                /* Find out how long cURL processing took. */
                time( &end_curl );
                diff = difftime( end_curl, start_curl );                

                /* Wait this many seconds, accounts for cURL processing time. 
                   We have double to int casting truncation here. */
                if( diff < seconds_per_iteration ){
                    sleep( (unsigned int)( seconds_per_iteration - diff ) );
                } else if ( seconds_per_iteration == 0 ) {
                    /* Continuous updating for an unlimited number of API calls 
                       per minute [subscription accounts]. */
                    sleep ( 1 );
                }

                /* Find the current epoch time. */
                time( &current_time );
            }
            packet->SetFetchingData ( false );
            gdk_threads_add_idle ( MainFetchBTNLabel, packet );
            break;

        case ABOUT_TOGGLE_BTN:
            gdk_threads_add_idle( AboutShowHide, NULL );
            break;

        case EQUITY_TOGGLE_BTN:
            gdk_threads_add_idle ( AddRemShowHide, packet );
            break;

        case EQUITY_OK_BTN: 
            gdk_threads_add_idle ( AddRemShowHide, packet );            

            /* The Mutex block is within this function. */
            gdk_threads_add_idle ( AddRemOk, packet );           

            break;

        case EQUITY_SWITCH:
            gdk_threads_add_idle( AddRemSwitchChange, NULL );
            break;

        case EQUITY_COMBO_BOX:
            gdk_threads_add_idle( AddRemComBoxChange, NULL );
            break;

        case EQUITY_CURSOR_MOVE:
            gdk_threads_add_idle( AddRemCursorMove, NULL );
            break;

        case BUL_TOGGLE_BTN:
            gdk_threads_add_idle( BullionShowHide, packet );
            break;

        case BUL_OK_BTN:
            pthread_mutex_lock ( &mutex_working[ FETCH_DATA_MUTEX ] ); 

            BullionOk ( packet );

            pthread_mutex_unlock ( &mutex_working[ FETCH_DATA_MUTEX ] );

            gdk_threads_add_idle ( BullionShowHide, packet );
            if( packet->IsFetchingData () == false ) {
                gdk_threads_add_idle ( MainDefaultTreeview, packet );
            } else {
                packet->Calculate ();
                packet->ToStrings ();
                /* Set Gtk treeview. */
                gdk_threads_add_idle ( MainPrimaryTreeview, packet );
            }
            break;

        case BUL_COMBO_BOX:
            gdk_threads_add_idle( BullionComBoxChange, NULL );
            break;

        case BUL_CURSOR_MOVE:
            gdk_threads_add_idle( BullionCursorMove, NULL );
            break;

        case CASH_TOGGLE_BTN:
            gdk_threads_add_idle ( CashShowHide, packet );
            break;

        case CASH_OK_BTN:
            pthread_mutex_lock ( &mutex_working[ FETCH_DATA_MUTEX ] );

            CashOk ( packet );

            pthread_mutex_unlock ( &mutex_working[ FETCH_DATA_MUTEX ] );

            gdk_threads_add_idle ( CashShowHide, packet );
            if( packet->IsFetchingData () == false ) {
                gdk_threads_add_idle ( MainDefaultTreeview, packet );
            } else {
                packet->Calculate ();
                packet->ToStrings ();
                /* Set Gtk treeview. */
                gdk_threads_add_idle ( MainPrimaryTreeview, packet );
            }
            break;

        case CASH_CURSOR_MOVE:
            gdk_threads_add_idle( CashCursorMove, NULL );
            break;

        case API_TOGGLE_BTN:
            gdk_threads_add_idle ( APIShowHide, packet );
            break;

        case API_OK_BTN:
            pthread_mutex_lock ( &mutex_working[ FETCH_DATA_MUTEX ] );
            
            APIOk ( packet );

            pthread_mutex_unlock ( &mutex_working[ FETCH_DATA_MUTEX ] );

            gdk_threads_add_idle ( APIShowHide, packet );
            break;

        case API_SYMBOL_UPDATE_BTN:
            pthread_mutex_lock( &mutex_working[ SYMBOL_NAME_MAP_MUTEX ] );
            /* Free the symbol to security name mapping array. */
            SNMapDestruct ( sym_map );
            if ( sym_map ) { free( sym_map ); sym_map = NULL; }

            gdk_threads_add_idle( APIStartSpinner, NULL );
            sym_map = SymNameFetchUpdate ( packet );
            gdk_threads_add_idle( APIStopSpinner, NULL );

            if ( packet->IsCurlCanceled () ) {
                pthread_mutex_unlock( &mutex_working[ SYMBOL_NAME_MAP_MUTEX ] );
                break;
            }

            /* gdk_threads_add_idle is non-blocking, we need the mutex
               in the RSICompletionSet and AddRemCompletionSet functions. */
            if( sym_map ) {
                gdk_threads_add_idle( RSICompletionSet, sym_map );
                gdk_threads_add_idle( AddRemCompletionSet, sym_map );
            }
            pthread_mutex_unlock( &mutex_working[ SYMBOL_NAME_MAP_MUTEX ] );

            break;

        case API_CURSOR_MOVE:
            gdk_threads_add_idle( APICursorMove, NULL );
            break;

        case RSI_TOGGLE_BTN:
            gdk_threads_add_idle( RSITreeViewClear, NULL );
            gdk_threads_add_idle( RSIShowHide, packet );
            break;
        case RSI_FETCH_BTN:
            /* Get the symbol string and perform multicurl,
               doesn't block the gui main loop. 
               */
            RSIGetSymbol( &symbol );
            RSIOutput = FetchRSIData ( symbol, packet );
            if ( packet->IsCurlCanceled () ){ 
                free( symbol );
                if( RSIOutput ) { 
                    free ( RSIOutput->memory ); 
                    free ( RSIOutput ); 
                }
                break;
            }

            /* Clear the current TreeView model */ 
            gdk_threads_add_idle ( RSITreeViewClear, NULL );

            /* Get the security name from the symbol map. */ 
            pthread_mutex_lock( &mutex_working[ SYMBOL_NAME_MAP_MUTEX ] );
            sec_name = GetSecurityName ( symbol, sym_map );
            pthread_mutex_unlock( &mutex_working[ SYMBOL_NAME_MAP_MUTEX ] );
            free ( symbol );

            /* Set the security name label, this function runs inside the Gtk Loop. 
               And will free the sec_name string. */
            gdk_threads_add_idle ( RSISetSNLabel, sec_name );

            /* Set and display the RSI treeview model.
               This function frees RSIOutput */
            /* gdk_threads_add_idle doesn't block this thread, we cannot free 
               RSIOutput here */
            gdk_threads_add_idle ( RSIMakeTreeview, RSIOutput );
            
            break;
        case RSI_CURSOR_MOVE:
            gdk_threads_add_idle ( RSICursorMove, NULL );
            break;
        case COMPLETION:
            /* Fetch the stock symbols and names outside the Gtk
               main loop, then create a GtkListStore and set it into
               a GtkEntryCompletion widget. */

            /* This mutex prevents the program from crashing if an
               MAIN_EXIT signal is run in parallel with this thread.

               This signal is only run once at application start.
            */

            pthread_mutex_lock( &mutex_working[ SYMBOL_NAME_MAP_MUTEX ] );

            sym_map = SymNameFetch ( packet );

            if ( packet->IsCurlCanceled () ) {
                pthread_mutex_unlock( &mutex_working[ SYMBOL_NAME_MAP_MUTEX ] );
                break;
            }

            /* gdk_threads_add_idle is non-blocking, we need the mutex
               in the RSICompletionSet and AddRemCompletionSet functions. */
            if( sym_map ) {
                gdk_threads_add_idle( RSICompletionSet, sym_map );
                gdk_threads_add_idle( AddRemCompletionSet, sym_map );
            }
            pthread_mutex_unlock( &mutex_working[ SYMBOL_NAME_MAP_MUTEX ] );
            
            break;
        case SHORTCUT_TOGGLE_BTN:
            gdk_threads_add_idle( ShortcutShowHide, NULL );
            break;
        case MAIN_CLOCK:
            /* 
               This is a single process multithreaded application. 

               The process is always using New York time.
            */

            while(1){
                gdk_threads_add_idle ( MainDisplayTime, NULL );

                sleep( ClockSleepSeconds () );
            }
            break;
        case MAIN_TIME_CLOSE_INDICATOR:
            NY_Time = packet->SetHoliday ();            
            seconds_to_open = packet->SecondsToOpen ();

            while(1){
                if ( packet->IsHoliday () ) {
                    gdk_threads_add_idle ( MainDisplayTimeRemaining, packet );
                    sleep(  3600 * ( (9 + 24) - NY_Time.tm_hour ) );
                    NY_Time = packet->SetHoliday ();
                }

                if ( ( packet->IsHoliday () == false ) && ( seconds_to_open == 0 )){
                    gdk_threads_add_idle ( MainDisplayTimeRemaining, packet );

                    sleep( 1 );

                    /* On Black Friday the market closes @ 13:00 EST */
                    if( NY_Time.tm_mon == 10 && NY_Time.tm_mday >= 23 && NY_Time.tm_mday <= 29 && NY_Time.tm_wday == 5 ){
                        NY_Time = packet->SetHoliday ();
                    }
                    
                } else if ( ( packet->IsHoliday () == false ) && seconds_to_open > 0 ){
                    gdk_threads_add_idle ( MainDisplayTimeRemaining, packet );
                    sleep( seconds_to_open );
                    NY_Time = packet->SetHoliday ();
                } 
                seconds_to_open = packet->SecondsToOpen ();             
            }
            break;

        case MAIN_EXIT:

            packet->SetCurlCanceled ( true );
            packet->StopMultiCurl ();

            /* This mutex prevents the program from crashing if a
               MAIN_FETCH_BTN signal is run in parallel with this thread. */
            pthread_mutex_lock( &mutex_working[ FETCH_DATA_MUTEX ] );

             /* Save the Window Size, Location, and Expander Bar setting. */
            packet->SetWindowDataSql ();

            /* Hide the windows. Prevents them from hanging open if a db write is in progress. */
            gdk_threads_add_idle ( MainHideWindow, NULL );

            /* This mutex prevents the program from crashing if a
               COMPLETION signal is run in parallel with this thread. */
            pthread_mutex_lock( &mutex_working[ SYMBOL_NAME_MAP_MUTEX ] ); 

            /* Hold the application until the Sqlite thread is finished [prevents db write errors]. */
            pthread_mutex_lock( &mutex_working[ SYMBOL_NAME_MAP_SQLITE_MUTEX ] );          

            /* Exit the GTK main loop. */
            gtk_main_quit ();

            /* Free the symbol to security name mapping array. */
            SNMapDestruct ( sym_map );
            if ( sym_map ) free( sym_map );

            
            pthread_mutex_unlock( &mutex_working[ SYMBOL_NAME_MAP_SQLITE_MUTEX ] );
            pthread_mutex_unlock( &mutex_working[ SYMBOL_NAME_MAP_MUTEX ] );
            pthread_mutex_unlock( &mutex_working[ FETCH_DATA_MUTEX ] );
            break;

        default:
            break;
    }

    /* Reset the widget signal semaphore. */
    sem_post( &semaphore[ index_signal ] );
    return NULL;
}