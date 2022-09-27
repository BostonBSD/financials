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
static symbol_name_map *sym_map;            /* A symbol to name map handle, only global to this file. */

void *GUIThreadHandler(void *data){
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
    
    /* If FETCH_DATA_BTN double clicked looping stops, change btn label. */
    if ( index_signal == FETCH_DATA_BTN && *MetaData->fetching_data_bool == false ){

        *MetaData->fetching_data_bool = true;
        gdk_threads_add_idle ( FetchDataBTNLabel, (void*)MetaData->fetching_data_bool );

    } else if ( index_signal == FETCH_DATA_BTN && *MetaData->fetching_data_bool == true ){
        pthread_mutex_lock( &mutex_working[ FETCH_DATA_MUTEX ] );   
        
        *MetaData->fetching_data_bool = false;
        
        pthread_mutex_unlock( &mutex_working[ FETCH_DATA_MUTEX ] );

        gdk_threads_add_idle ( FetchDataBTNLabel, (void*)MetaData->fetching_data_bool );
        
        return NULL;
    }

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
        case FETCH_DATA_BTN:
            /* The number of seconds between data fetch operations. */
            if( *MetaData->updates_per_min_f <= 0 ){
                seconds_per_iteration = 0;
            } else {
                seconds_per_iteration = 60 / *MetaData->updates_per_min_f;
            }

            /* Because loop_val is not always evenly divisible by the
               seconds_per_iteration value, our loop will finish in an 
               approximate length of time plus the slack seconds. */

            /* The number of seconds to keep looping. */
            loop_val = 3600 * *MetaData->updates_hours_f;
            /* If the hours to run is zero, run one loop iteration. */
            if( loop_val == 0 ) loop_val = 1;

            time( &current_time );
            end_time = current_time + (time_t)loop_val;
            
            while( current_time < end_time && *MetaData->fetching_data_bool == true ){
                /* This mutex prevents the program from crashing if an
                   EXIT_APPLICATION, ADD_REMOVE_SEC_OK_BTN, 
                   ADD_REMOVE_BUL_OK_BTN, ADD_REMOVE_CASH_OK_BTN, 
                   or CHANGE_API_OK_BTN signal is run in parallel with 
                   this thread. */

                time( &start_curl );
                pthread_mutex_lock( &mutex_working[ FETCH_DATA_MUTEX ] ); 
               

                PopulateBullionPrice_Yahoo ();
                if ( *MetaData->multicurl_cancel_bool == true ){
                    pthread_mutex_unlock( &mutex_working[ FETCH_DATA_MUTEX ] );
                    break;
                }

                if ( MultiCurlProcessing () != 0 ) { 
                    gdk_threads_add_idle ( MakeGUIOne, NULL );
                    pthread_mutex_unlock( &mutex_working[ FETCH_DATA_MUTEX ] );
                    break;
                }
                if ( *MetaData->multicurl_cancel_bool == true ){
                    pthread_mutex_unlock( &mutex_working[ FETCH_DATA_MUTEX ] );
                    break;
                }

                JSONProcessing ();
                PerformCalculations ();

                pthread_mutex_unlock( &mutex_working[ FETCH_DATA_MUTEX ] );

                /* Set Gtk treeview. */
                gdk_threads_add_idle ( MakeGUIOne, NULL );
           
                seconds_to_open = SecondsToOpen ();
                /* If the market is closed or today is a holiday only loop once. */
                if( seconds_to_open != 0 || *MetaData->holiday_bool ) {
                    break;
                }

                /* Find out how long cURL processing took. */
                time( &end_curl );
                diff = difftime( end_curl, start_curl );                

                /* Wait this many seconds, accounts for cURL processing time. 
                   We have double to int casting truncation here. */
                if( diff < seconds_per_iteration ){
                    sleep( (unsigned int)( seconds_per_iteration - diff ) );
                }

                /* Find the current epoch time. */
                time( &current_time );
            }
            *MetaData->fetching_data_bool = false;
            gdk_threads_add_idle ( FetchDataBTNLabel, (void*)MetaData->fetching_data_bool );
            break;

        case ABOUT_BTN:
            gdk_threads_add_idle( ShowHideAboutWindow, NULL );
            break;

        case ADD_REMOVE_SEC_BTN:
            gdk_threads_add_idle( ShowHideAddRemoveSecurityWindow, NULL );
            break;

        case ADD_REMOVE_SEC_OK_BTN:            
            gdk_threads_add_idle( ShowHideAddRemoveSecurityWindow, NULL );            

            /* The Mutex block is within this function. */
            gdk_threads_add_idle( OKSecurityAddRemoveSecurityWindow, NULL );
            
            if( *MetaData->fetching_data_bool == false ) gdk_threads_add_idle( DefaultTreeView, NULL );            

            break;

        case ADD_REMOVE_SEC_CANCEL_BTN:
            gdk_threads_add_idle( ShowHideAddRemoveSecurityWindow, NULL );
            break;

        case ADD_REMOVE_SEC_SWITCH:
            gdk_threads_add_idle( SwitchChangeAddRemoveSecurityWindow, NULL );
            break;

        case ADD_REMOVE_SEC_COMBO_BOX:
            gdk_threads_add_idle( ChangedAddRemoveSecurityComboBox, NULL );
            break;

        case ADD_REMOVE_SEC_CURSOR_MOVE:
            gdk_threads_add_idle( CursorMoveAddRemoveSecurityEntryBoxGUI, NULL );
            break;

        case ADD_REMOVE_BUL_BTN:
            gdk_threads_add_idle( ShowHideBullionWindow, NULL );
            break;

        case ADD_REMOVE_BUL_OK_BTN:
            pthread_mutex_lock( &mutex_working[ FETCH_DATA_MUTEX ] ); 

            OKBullionWindow ();

            pthread_mutex_unlock( &mutex_working[ FETCH_DATA_MUTEX ] );

            gdk_threads_add_idle( ShowHideBullionWindow, NULL );
            if( *MetaData->fetching_data_bool == false ) gdk_threads_add_idle( DefaultTreeView, NULL );
            break;

        case ADD_REMOVE_BUL_CANCEL_BTN:
            gdk_threads_add_idle( ShowHideBullionWindow, NULL );
            break;

        case ADD_REMOVE_BUL_CURSOR_MOVE:
            gdk_threads_add_idle( CursorMoveBullionEntryBoxGUI, NULL );
            break;

        case ADD_REMOVE_CASH_BTN:
            gdk_threads_add_idle( ShowHideCashWindow, NULL );
            break;

        case ADD_REMOVE_CASH_OK_BTN:
            pthread_mutex_lock( &mutex_working[ FETCH_DATA_MUTEX ] );

            OKCashWindow ();

            pthread_mutex_unlock( &mutex_working[ FETCH_DATA_MUTEX ] );

            gdk_threads_add_idle( ShowHideCashWindow, NULL );
            if( *MetaData->fetching_data_bool == false ) gdk_threads_add_idle( DefaultTreeView, NULL );
            break;

        case ADD_REMOVE_CASH_CANCEL_BTN:
            gdk_threads_add_idle( ShowHideCashWindow, NULL );
            break;

        case ADD_REMOVE_CASH_CURSOR_MOVE:
            gdk_threads_add_idle( CursorMoveCashEntryBoxGUI, NULL );
            break;

        case CHANGE_API_BTN:
            gdk_threads_add_idle( ShowHideAPIWindow, NULL );
            break;

        case CHANGE_API_OK_BTN:
            pthread_mutex_lock( &mutex_working[ FETCH_DATA_MUTEX ] );
            
            OKAPIWindow ();

            pthread_mutex_unlock( &mutex_working[ FETCH_DATA_MUTEX ] );

            gdk_threads_add_idle( ShowHideAPIWindow, NULL );
            break;

        case CHANGE_API_CANCEL_BTN:
            gdk_threads_add_idle( ShowHideAPIWindow, NULL );
            break;

        case CHANGE_API_CURSOR_MOVE:
            gdk_threads_add_idle( CursorMoveAPIEntryBoxGUI, NULL );
            break;

        case VIEW_RSI_BTN:
            gdk_threads_add_idle( RSITreeViewClear, NULL );
            gdk_threads_add_idle( ShowHideViewRSIWindow, NULL );
            break;
        case VIEW_RSI_FETCH_DATA_BTN:
            /* Get the symbol string and perform multicurl here,
               doesn't block the gui main loop. 
               */
            RSIGetSymbol( &symbol );
            RSIOutput = RSIMulticurlProcessing ( symbol );
            if ( *MetaData->multicurl_cancel_bool == true ){ 
                free( symbol );
                free ( RSIOutput->memory ); 
                free ( RSIOutput );  
                break;
            }

            /* Clear the current TreeView model */ 
            gdk_threads_add_idle( RSITreeViewClear, NULL );

            /* Get the security name from the symbol map. */ 
            sec_name = GetSecurityNameFromMapping( symbol, sym_map );
            free( symbol );

            /* Set the security name label, this function runs inside the Gtk Loop. 
               And will free the sec_name string. */
            gdk_threads_add_idle( SetSecurityNameLabel, (void*)sec_name );

            /* Set and display the RSI treeview model.
               This function frees RSIOutput */
            /* gdk_threads_add_idle doesn't block this thread, we cannot free 
               RSIOutput here */
            gdk_threads_add_idle( RSIMakeGUI, (void*)RSIOutput );
            
            break;
        case VIEW_RSI_CURSOR_MOVE:
            gdk_threads_add_idle( RSICursorMove, NULL );
            break;
        case VIEW_RSI_COMPLETION:
            /* Fetch the stock symbols and names outside the Gtk
               main loop, then create a GtkListStore and set it into
               a GtkEntryCompletion widget. */

            /* This mutex prevents the program from crashing if an
               EXIT_APPLICATION signal is run in parallel with this thread.

               This signal is only run once at application start.
            */
            pthread_mutex_lock( &mutex_working[ RSI_COMPLETION_FETCH_MUTEX ] );

            sym_map = CompletionSymbolFetch ();

            if ( *MetaData->multicurl_cancel_bool == true ) {
                pthread_mutex_unlock( &mutex_working[ RSI_COMPLETION_FETCH_MUTEX ] );
                break;
            }

            /* gdk_threads_add_idle is non-blocking, we need the mutex
               in the ViewRSICompletionSet function. */
            if(sym_map) {
                gdk_threads_add_idle( ViewRSICompletionSet, (void*)sym_map );
                pthread_mutex_unlock( &mutex_working[ RSI_COMPLETION_FETCH_MUTEX ] );
            }

            
            break;
        case SHORTCUT_KEYS_BTN:
            gdk_threads_add_idle( ShowHideShortcutWindow, NULL );
            break;
        case DISPLAY_TIME:
            /* 
               This is a single process multithreaded application. 

               The process is always using New York time.
            */

            while(1){
                gdk_threads_add_idle ( DisplayTime, NULL );

                sleep( ClockSleepSeconds () );
            }
            break;
        case DISPLAY_TIME_OPEN_INDICATOR:
            NY_Time = NYTimeComponents ();
            *MetaData->holiday_bool = IsHoliday ( NY_Time );
            seconds_to_open = SecondsToOpen ();

            while(1){
                if ( *MetaData->holiday_bool ) {
                    gdk_threads_add_idle ( DisplayTimeRemaining, NULL );
                    sleep(  3600 * ( (9 + 24) - NY_Time.tm_hour ) );
                    NY_Time = NYTimeComponents ();
                    *MetaData->holiday_bool = IsHoliday ( NY_Time );
                }

                if ( !(*MetaData->holiday_bool) && seconds_to_open == 0 ){
                    gdk_threads_add_idle ( DisplayTimeRemaining, NULL );
                    sleep( 1 );
                    
                } else if ( !(*MetaData->holiday_bool) && seconds_to_open > 0 ){
                    gdk_threads_add_idle ( DisplayTimeRemaining, NULL );
                    sleep( seconds_to_open );
                    NY_Time = NYTimeComponents ();
                    *MetaData->holiday_bool = IsHoliday ( NY_Time );
                } 
                seconds_to_open = SecondsToOpen ();             
            }
            break;

        case EXIT_APPLICATION:

            *MetaData->multicurl_cancel_bool = true;
            stop_multicurl ();

            /* This mutex prevents the program from crashing if a
               FETCH_DATA_BTN signal is run in parallel with this thread. */
            pthread_mutex_lock( &mutex_working[ FETCH_DATA_MUTEX ] );

            /* Save the Window Size and Location. */
            SqliteAddMainWindowSize ( WindowStruct.main_width , WindowStruct.main_height );
            SqliteAddMainWindowPos ( WindowStruct.main_x_pos, WindowStruct.main_y_pos );
            SqliteAddRSIWindowSize ( WindowStruct.rsi_width, WindowStruct.rsi_height );
            SqliteAddRSIWindowPos ( WindowStruct.rsi_x_pos, WindowStruct.rsi_y_pos );

            /* This mutex prevents the program from crashing if a
               VIEW_RSI_COMPLETION signal is run in parallel with this thread. */
            pthread_mutex_lock( &mutex_working[ RSI_COMPLETION_FETCH_MUTEX ] );

            /* Exit the GTK main loop. */
            gtk_main_quit ();

            /* Free the symbol to security name mapping array. */
            symbol_security_name_map_destruct ( sym_map );
            free( sym_map );

            pthread_mutex_unlock( &mutex_working[ RSI_COMPLETION_FETCH_MUTEX ] );
            pthread_mutex_unlock( &mutex_working[ FETCH_DATA_MUTEX ] );
            break;

        default:
            break;
    }

    /* Reset the widget signal semaphore. */
    sem_post( &semaphore[ index_signal ] );
    return NULL;
}