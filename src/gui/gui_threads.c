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
        pthread_mutex_lock( &mutex_working[2] );   
        
        *MetaData->fetching_data_bool = false;
        
        pthread_mutex_unlock( &mutex_working[2] );

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
                   EXIT_APPLICATION or ADD_REMOVE_SEC_OK_BTN
                   signal is run in parallel with this thread. */
                time( &start_curl );
                pthread_mutex_lock( &mutex_working[2] );                

                PopulateBullionPrice_Yahoo ();
                if ( MultiCurlProcessing () != 0 ) { 
                    gdk_threads_add_idle ( MakeGUIOne, NULL );
                    pthread_mutex_unlock( &mutex_working[2] );
                    break;
                 }
                JSONProcessing ();
                PerformCalculations ();

                /* Set Gtk treeview. */
                gdk_threads_add_idle ( MakeGUIOne, NULL );

                pthread_mutex_unlock( &mutex_working[2] );
           
                seconds_to_open = SecondsToOpen ();
                /* If the market is closed or today is a holiday only loop once. */
                if( seconds_to_open != 0 || *MetaData->holiday_bool ) {
                    break;
                }

                /* Find out how long cURL processing took. */
                time( &end_curl );
                diff = difftime( end_curl, start_curl );                

                /* Wait this many seconds, accounts for cURL processing time. 
                   We have double to int casting trunction here. */
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
            /* We can pull this function out of the mutex block because
               this thread is only run when the window is already visible
               and associated widgets are reset when the window is already 
               invisible. */
            gdk_threads_add_idle( ShowHideAddRemoveSecurityWindow, NULL );

            /* This mutex prevents the program from crashing if a
               FETCH_DATA_BTN signal is run in parallel with this thread. */
            pthread_mutex_lock( &mutex_working[2] );

            gdk_threads_add_idle( OKSecurityAddRemoveSecurityWindow, NULL );
            
            if( *MetaData->fetching_data_bool == false ) gdk_threads_add_idle( DefaultTreeView, NULL );            

            pthread_mutex_unlock( &mutex_working[2] );
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
            pthread_mutex_lock( &mutex_working[2] ); 

            OKBullionWindow ();

            pthread_mutex_unlock( &mutex_working[2] );

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
            pthread_mutex_lock( &mutex_working[2] );

            OKCashWindow ();

            pthread_mutex_unlock( &mutex_working[2] );

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
            pthread_mutex_lock( &mutex_working[2] );
            
            OKAPIWindow ();

            pthread_mutex_unlock( &mutex_working[2] );

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
            /* Clear the current TreeView model */ 
            gdk_threads_add_idle( RSITreeViewClear, NULL );

            /* Fetch, set, and display the RSI treeview model */
            gdk_threads_add_idle( RSIMakeGUI, NULL );
            break;
        case VIEW_RSI_CURSOR_MOVE:
            gdk_threads_add_idle( RSICursorMove, NULL );
            break;
        case VIEW_RSI_COMPLETION:
            /* This thread won't block the Gtk main loop, but it may crash Gtk. */
            /* If the program appears to crash on loading uncomment the next 
               line and comment the line after. */
            //gdk_threads_add_idle( ViewRSICompletionSet, NULL );
            ViewRSICompletionSet ();
            break;
        case DISPLAY_TIME:
            /* The Mutexes are in the LocalAndNYTime, TimeToClose, and 
               SecondsToOpen functions to reduce synchronization overhead.

               Mutexes are required here because we are changing the process 
               locale from local time to New York time and back again.

               This is a single process multithreaded application. 

               EDIT: I took the mutexes out because they were causing a timing 
               issue, if this proves consequential they can be put back.
               //pthread_mutex_lock( &mutex_working[3] );
               //pthread_mutex_unlock( &mutex_working[3] );
               Is the clock mutex. 
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
            /* This mutex prevents the program from crashing if a
               FETCH_DATA_BTN signal is run in parallel with this thread. */
            pthread_mutex_lock( &mutex_working[2] );

            /* Save the Window Size and Location. */
            SqliteChangeWindowSize ( MainWindowStruct.width, MainWindowStruct.height );
            SqliteChangeWindowPos ( MainWindowStruct.x_pos, MainWindowStruct.y_pos );

            /* Exit the GTK main loop. */
            gtk_main_quit ();

            /* Free the symbol to security name mapping array. */
            symbol_security_name_map_destruct ();

            pthread_mutex_unlock( &mutex_working[2] );
            break;

        default:
            break;
    }

    /* Reset the widget signal semaphore. */
    sem_post( &semaphore[ index_signal ] );
    return NULL;
}