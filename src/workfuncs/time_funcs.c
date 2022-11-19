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

#include <stdbool.h>
#include <time.h>
#include <math.h>

struct tm NYTimeComponents (){
    time_t currenttime;
    struct tm NY_tz;

    time( &currenttime );

    /* Get the localtime in NY from Epoch seconds */
    localtime_r( &currenttime, &NY_tz );
    return NY_tz;
}

unsigned int ClockSleepSeconds (){
    time_t currenttime;
    struct tm tm_struct;

    time( &currenttime );

    /* The timezone here isn't relevant. */
    localtime_r( &currenttime, &tm_struct );
    return ( 60 - tm_struct.tm_sec );
}

char* MonthNameStr ( int month ){
    switch ( month ){
        case 0:
            return "January";
            break;
        case 1:
            return "February";
            break;
        case 2:
            return "March";
            break;
        case 3:
            return "April";
            break;
        case 4:
            return "May";
            break;
        case 5:
            return "June";
            break;
        case 6:
            return "July";
            break;
        case 7:
            return "August";
            break;
        case 8:
            return "September";
            break;
        case 9:
            return "October";
            break;
        case 10:
            return "November";
            break;
        default:
            return "December";
            break;
    }
}

char* WeekDayStr ( int weekday ){
    switch ( weekday ){
        case 0:
            return "Sunday";
            break;
        case 1:
            return "Monday";
            break;
        case 2:
            return "Tuesday";
            break;
        case 3:
            return "Wednesday";
            break;
        case 4:
            return "Thursday";
            break;
        case 5:
            return "Friday";
            break;
        default:
            return "Saturday";
            break;
    }
}

void NYTime (int *ny_h, int *ny_m, int *ny_month, int *ny_day_month, int *ny_day_week, int *ny_year) {
    struct tm NY_tz;

    /* Get the NY time from Epoch seconds */
    NY_tz = NYTimeComponents ();

    *ny_h = (NY_tz.tm_hour)%24;
    *ny_m = NY_tz.tm_min;
    *ny_month = NY_tz.tm_mon;
    *ny_day_month = NY_tz.tm_mday;
    *ny_day_week = NY_tz.tm_wday;
    *ny_year = NY_tz.tm_year + 1900;  
}

static void easter (int year, int *month, int *day)
/* Copied From: https://c-for-dummies.com/blog/?p=2446 by dgookin */
/* For any given year, will determine the month and day of Easter Sunday. 
   Month numbering starts at 1; March is 3, April is 4, etc. */
{
    int Y,a,c,e,h,k,L;
    double b,d,f,g,i,m;

    Y = year;
    a = Y%19;
    b = floor(Y/100);
    c = Y%100;
    d = floor(b/4);
    e = (int)b%4;
    f = floor((b+8)/25);
    g = floor((b-f+1)/3);
    h = (19*a+(int)b-(int)d-(int)g+15)%30;
    i = floor(c/4);
    k = c%4;
    L = (32+2*e+2*(int)i-h-k)%7;
    m = floor((a+11*h+22*L)/451);
    *month = (int)floor((h+L-7*m+114)/31);
    *day = (int)((h+L-7*(int)m+114)%31)+1;
}

char* WhichHoliday (struct tm NY_tz){
    /* New Years Day */
    if(NY_tz.tm_mon == 0 && NY_tz.tm_mday == 1 ) return "Market Closed - New Years Day";
    /* Presidents Day */
    if(NY_tz.tm_mon == 0 && NY_tz.tm_wday == 1 && NY_tz.tm_mday >= 15 && NY_tz.tm_mday <= 21) return "Market Closed - Presidents Day";
    /* Martin Luther King Jr. Day */
    if(NY_tz.tm_mon == 1 && NY_tz.tm_wday == 1 && NY_tz.tm_mday >= 15 && NY_tz.tm_mday <= 21) return "Market Closed - Martin Luther King Jr. Day";
    /* Memorial Day */
    if(NY_tz.tm_mon == 4 && NY_tz.tm_wday == 1 && NY_tz.tm_mday >= 25 && NY_tz.tm_mday <= 31) return "Market Closed - Memorial Day";
    /* Juneteenth Day */
    if(NY_tz.tm_mon == 5 && NY_tz.tm_mday == 19 ) return "Market Closed - Juneteenth Day";
    if(NY_tz.tm_mon == 5 && NY_tz.tm_wday == 1 && NY_tz.tm_mday >= 19 && NY_tz.tm_mday <= 21) return "Market Closed - Juneteenth Holiday";
    /* US Independence Day */
    if(NY_tz.tm_mon == 6 && NY_tz.tm_mday == 4 ) return "Market Closed - US Independence Day";
    if(NY_tz.tm_mon == 6 && NY_tz.tm_wday == 1 && NY_tz.tm_mday >= 4 && NY_tz.tm_mday <= 6) return "Market Closed - US Independence Holiday";
    /* Labor Day */
    if(NY_tz.tm_mon == 8 && NY_tz.tm_wday == 1 && NY_tz.tm_mday >= 1 && NY_tz.tm_mday <= 7) return "Market Closed - Labor Day";
    /* Thanksgiving Day */
    if(NY_tz.tm_mon == 10 && NY_tz.tm_wday == 4 && NY_tz.tm_mday >= 22 && NY_tz.tm_mday <= 28) return "Market Closed - Thanksgiving Day";
    /* Black Friday */
    if(NY_tz.tm_mon == 10 && NY_tz.tm_wday == 5 && NY_tz.tm_mday >= 23 && NY_tz.tm_mday <= 29 && NY_tz.tm_hour >= 13) return "Market Closed Early - Black Friday";
    /* Christmas Day */
    if(NY_tz.tm_mon == 11 && NY_tz.tm_mday == 25 ) return "Market Closed - Christmas Day";
    if(NY_tz.tm_mon == 11 && NY_tz.tm_wday == 1 && NY_tz.tm_mday >= 25 && NY_tz.tm_mday <= 27) return "Market Closed - Christmas Holiday";
    /* Good Friday */
    int month, day;
    easter( (int)NY_tz.tm_year + 1900, &month, &day ); /* Finds the date of easter for a given year. */
    if( ( (int)NY_tz.tm_mon + 1 ) == month && (int)NY_tz.tm_mday == ( day - 2 ) ) return "Market Closed - Good Friday";

    return "Not A Holiday - Error";
}

bool CheckHoliday (struct tm NY_tz){
    /* New Years Day */
    if(NY_tz.tm_mon == 0 && NY_tz.tm_mday == 1 ) return true;
    /* Presidents Day */
    if(NY_tz.tm_mon == 0 && NY_tz.tm_wday == 1 && NY_tz.tm_mday >= 15 && NY_tz.tm_mday <= 21) return true;
    /* Martin Luther King Junior Day */
    if(NY_tz.tm_mon == 1 && NY_tz.tm_wday == 1 && NY_tz.tm_mday >= 15 && NY_tz.tm_mday <= 21) return true;
    /* Memorial Day */
    if(NY_tz.tm_mon == 4 && NY_tz.tm_wday == 1 && NY_tz.tm_mday >= 25 && NY_tz.tm_mday <= 31) return true;
    /* Juneteenth Day */
    if(NY_tz.tm_mon == 5 && NY_tz.tm_mday == 19 ) return true;
    if(NY_tz.tm_mon == 5 && NY_tz.tm_wday == 1 && NY_tz.tm_mday >= 19 && NY_tz.tm_mday <= 21) return true;
    /* US Independence Day */
    if(NY_tz.tm_mon == 6 && NY_tz.tm_mday == 4 ) return true;
    if(NY_tz.tm_mon == 6 && NY_tz.tm_wday == 1 && NY_tz.tm_mday >= 4 && NY_tz.tm_mday <= 6) return true;
    /* Labor Day */
    if(NY_tz.tm_mon == 8 && NY_tz.tm_wday == 1 && NY_tz.tm_mday >= 1 && NY_tz.tm_mday <= 7) return true;
    /* Thanksgiving Day */
    if(NY_tz.tm_mon == 10 && NY_tz.tm_wday == 4 && NY_tz.tm_mday >= 22 && NY_tz.tm_mday <= 28) return true;
    /* Black Friday */
    if(NY_tz.tm_mon == 10 && NY_tz.tm_wday == 5 && NY_tz.tm_mday >= 23 && NY_tz.tm_mday <= 29 && NY_tz.tm_hour >= 13) return true;
    /* Christmas Day */
    if(NY_tz.tm_mon == 11 && NY_tz.tm_mday == 25 ) return true;
    if(NY_tz.tm_mon == 11 && NY_tz.tm_wday == 1 && NY_tz.tm_mday >= 25 && NY_tz.tm_mday <= 27) return true;
    /* Good Friday */
    int month, day;
    easter( (int)NY_tz.tm_year + 1900, &month, &day ); /* Finds the date of easter for a given year. */
    if( ( (int)NY_tz.tm_mon + 1 ) == month && (int)NY_tz.tm_mday == ( day - 2 ) ) return true;

    return false;
}

bool TimeToClose ( bool holiday, int *h_r, int *m_r, int *s_r ) {
    /* The NYSE/NASDAQ Markets are open from 09:30 to 16:00 EST. */
    struct tm NY_tz;
    bool closed;
    
    /* Get the localtime in NY from Epoch seconds */
    /* Accounts if it is DST or STD time in NY. */
    NY_tz = NYTimeComponents ();

    int hour = (NY_tz.tm_hour)%24;
    int min = NY_tz.tm_min;
    int sec = NY_tz.tm_sec;
    int weekday = NY_tz.tm_wday;

    int hour_open_NY, hour_closed_NY;    

    hour_open_NY = 9;
    hour_closed_NY = 16;
    
    if( holiday || hour < hour_open_NY || ( hour == hour_open_NY && min < 30 ) || hour >= hour_closed_NY || weekday == 0 || weekday == 6 ){
        *h_r = 0;
        *m_r = 0;
        *s_r = 0;
        closed = true;
    } else {
        *h_r = hour_closed_NY - 1 - hour;
        *m_r = 59 - min;
        *s_r = 59 - sec;
        closed = false;
    }
    return closed;
}

unsigned int SecondsToOpen () {
    /* The seconds until the NYSE/NASDAQ markets open, does not account
    for holidays. */
    struct tm NY_tz;
    time_t currenttime, futuretime;
    unsigned int diff;

    /* The current time in Epoch seconds */
    time( &currenttime );

    /* Get the localtime in NY from Epoch seconds 
       We need to do this to account for DST changes when
       determining the future Epoch time. 
    */

    NY_tz = NYTimeComponents ();
    

    int hour = (NY_tz.tm_hour)%24;
    int min = NY_tz.tm_min;
    int weekday = NY_tz.tm_wday;
    int hour_open_NY, hour_closed_NY;

    hour_open_NY = 9;
    hour_closed_NY = 16;

    if( ( hour > hour_open_NY || ( hour == hour_open_NY && min >= 30 ) ) && hour < hour_closed_NY && weekday != 0 && weekday != 6 ){

        /* The market is open. */
        return 0;
    }
    
    /* Today is not Fri and today is not Sat and today is not Sun and it is past 4 PM EST/EDST */
    if( weekday != 5 && weekday != 6 && weekday != 0 && hour >= hour_closed_NY ){
    	NY_tz.tm_mday++;
    }
    
    /* Today is Sun */
    if( weekday == 0 ) {
    	NY_tz.tm_mday++;
    }
    
    /* Today is Sat */
    if( weekday == 6 ) {
    	NY_tz.tm_mday += 2;
    }

    /* Today is Fri and it is past 4 PM EST/EDST */
    if( weekday == 5 && hour >= hour_closed_NY ) {
    	NY_tz.tm_mday += 3;
    }
     
    /* The market always opens at 09:30:00 EST/EDST */
    NY_tz.tm_hour = hour_open_NY;
    NY_tz.tm_min = 30;
    NY_tz.tm_sec = 0;
    
    /* The future NY EST/EDST date to time in Epoch seconds. */
    futuretime = mktime( &NY_tz );
    
    /* The market is closed, reopens in this many seconds. */
    diff = (unsigned int) difftime( futuretime, currenttime );

    return ( diff );
}