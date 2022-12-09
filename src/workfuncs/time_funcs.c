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
#include <time.h>       /* localtime_r(), mktime(), time(), difftime(), struct tm */
#include <sys/time.h>   /* gettimeofday(), struct timeval */
#include <math.h>

#define OPEN_HOUR 9
#define OPEN_MINUTE 30
#define CLOSING_HOUR 16
#define CLOSING_HOUR_BLACK_FRIDAY 13

struct tm NYTimeComponents (){
    time_t currenttime;
    struct tm NY_tz;

    time( &currenttime );

    /* Get the localtime in NY from Epoch seconds */
    localtime_r( &currenttime, &NY_tz );
    return NY_tz;
}

unsigned int ClockSleepMicroSeconds (){
    struct timeval tv;
    gettimeofday(&tv, NULL);

    /* The number of microseconds in a second
       minus the number of microseconds which have
       passed in the current second. */
    return ( 1000000 - tv.tv_usec );
}

unsigned int ClockSleepSeconds (){
    time_t currenttime;
    struct tm tm_struct;

    time( &currenttime );

    localtime_r( &currenttime, &tm_struct );
    return ( 60 - tm_struct.tm_sec );
}

char* MonthNameStr ( int month ){
    enum { JAN, FEB, MAR, APR, MAY, JUN, JUL, AUG, SEP, OCT, NOV };

    switch ( month ){
        case JAN:
            return "January";
            break;
        case FEB:
            return "February";
            break;
        case MAR:
            return "March";
            break;
        case APR:
            return "April";
            break;
        case MAY:
            return "May";
            break;
        case JUN:
            return "June";
            break;
        case JUL:
            return "July";
            break;
        case AUG:
            return "August";
            break;
        case SEP:
            return "September";
            break;
        case OCT:
            return "October";
            break;
        case NOV:
            return "November";
            break;
        default: /* DEC */
            return "December";
            break;
    }
}

char* WeekDayStr ( int weekday ){
    enum { SUN, MON, TUES, WEDS, THURS, FRI };

    switch ( weekday ){
        case SUN:
            return "Sunday";
            break;
        case MON:
            return "Monday";
            break;
        case TUES:
            return "Tuesday";
            break;
        case WEDS:
            return "Wednesday";
            break;
        case THURS:
            return "Thursday";
            break;
        case FRI:
            return "Friday";
            break;
        default: /* SAT */
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
    enum { JAN, FEB, MAR, APR, MAY, JUN, JUL, AUG, SEP, OCT, NOV, DEC };
    enum { SUN, MON, TUES, WEDS, THURS, FRI };
    int month, day;

    switch ( NY_tz.tm_mon % 12 ){
        case JAN:
            /* New Years Day */
            if( NY_tz.tm_mday == 1 ) return "Market Closed - New Years Day";

            /* Presidents Day */
            if( NY_tz.tm_wday == MON && NY_tz.tm_mday >= 15 && NY_tz.tm_mday <= 21 ) return "Market Closed - Presidents Day";
            break;
        case FEB:
            /* Martin Luther King Junior Day */
            if( NY_tz.tm_wday == MON && NY_tz.tm_mday >= 15 && NY_tz.tm_mday <= 21 ) return "Market Closed - Martin Luther King Jr. Day";
            break;
        case MAY:
            /* Memorial Day */
            if( NY_tz.tm_wday == MON && NY_tz.tm_mday >= 25 && NY_tz.tm_mday <= 31 ) return "Market Closed - Memorial Day";
            break;
        case JUN:
            /* Juneteenth Day */
            if( NY_tz.tm_mday == 19 ) return "Market Closed - Juneteenth Day";
            if( NY_tz.tm_wday == MON && NY_tz.tm_mday >= 19 && NY_tz.tm_mday <= 21 ) return "Market Closed - Juneteenth Holiday";
            break;
        case JUL:
            /* US Independence Day */
            if( NY_tz.tm_mday == 4 ) return "Market Closed - US Independence Day";
            if( NY_tz.tm_wday == MON && NY_tz.tm_mday >= 4 && NY_tz.tm_mday <= 6 ) return "Market Closed - US Independence Holiday";
            break;
        case AUG:
            return "Not A Holiday - Error";
            break;
        case SEP:
            /* Labor Day */
            if( NY_tz.tm_wday == MON && NY_tz.tm_mday >= 1 && NY_tz.tm_mday <= 7 ) return "Market Closed - Labor Day";
            break;
        case OCT:
            return "Not A Holiday - Error";
            break;
        case NOV:
            /* Thanksgiving Day */
            if( NY_tz.tm_wday == THURS && NY_tz.tm_mday >= 22 && NY_tz.tm_mday <= 28 ) return "Market Closed - Thanksgiving Day";
            /* Black Friday */
            if( NY_tz.tm_wday == FRI && NY_tz.tm_mday >= 23 && NY_tz.tm_mday <= 29 && NY_tz.tm_hour >= CLOSING_HOUR_BLACK_FRIDAY ) return "Market Closed Early - Black Friday";
            break;
        case DEC:
            /* Christmas Day */
            if( NY_tz.tm_mday == 25 ) return "Market Closed - Christmas Day";
            if( NY_tz.tm_wday == MON && NY_tz.tm_mday >= 25 && NY_tz.tm_mday <= 27 ) return "Market Closed - Christmas Holiday";
            break;
        default: /* MAR || APR */
            /* Good Friday */
            easter( NY_tz.tm_year + 1900, &month, &day ); /* Finds the date of easter for a given year. */
            if( ( NY_tz.tm_mon + 1 ) == month && NY_tz.tm_mday == ( day - 2 ) ) return "Market Closed - Good Friday";
            break;
    }

    return "Not A Holiday - Error";
}

bool CheckHoliday (struct tm NY_tz){
    enum { JAN, FEB, MAR, APR, MAY, JUN, JUL, AUG, SEP, OCT, NOV, DEC };
    enum { SUN, MON, TUES, WEDS, THURS, FRI };
    int easter_month, easter_day;

    switch ( NY_tz.tm_mon % 12 ){
        case JAN:
            /* New Years Day */
            if( NY_tz.tm_mday == 1 ) return true;

            /* Presidents Day */
            if( NY_tz.tm_wday == MON && NY_tz.tm_mday >= 15 && NY_tz.tm_mday <= 21 ) return true;
            break;
        case FEB:
            /* Martin Luther King Junior Day */
            if( NY_tz.tm_wday == MON && NY_tz.tm_mday >= 15 && NY_tz.tm_mday <= 21 ) return true;
            break;
        case MAY:
            /* Memorial Day */
            if( NY_tz.tm_wday == MON && NY_tz.tm_mday >= 25 && NY_tz.tm_mday <= 31 ) return true;
            break;
        case JUN:
            /* Juneteenth Day */
            if( NY_tz.tm_mday == 19 ) return true;
            if( NY_tz.tm_wday == MON && NY_tz.tm_mday >= 19 && NY_tz.tm_mday <= 21 ) return true;
            break;
        case JUL:
            /* US Independence Day */
            if( NY_tz.tm_mday == 4 ) return true;
            if( NY_tz.tm_wday == MON && NY_tz.tm_mday >= 4 && NY_tz.tm_mday <= 6 ) return true;
            break;
        case AUG:
            return false;
            break;
        case SEP:
            /* Labor Day */
            if( NY_tz.tm_wday == MON && NY_tz.tm_mday >= 1 && NY_tz.tm_mday <= 7 ) return true;
            break;
        case OCT:
            return false;
            break;
        case NOV:
            /* Thanksgiving Day */
            if( NY_tz.tm_wday == THURS && NY_tz.tm_mday >= 22 && NY_tz.tm_mday <= 28 ) return true;
            /* Black Friday */
            if( NY_tz.tm_wday == FRI && NY_tz.tm_mday >= 23 && NY_tz.tm_mday <= 29 && NY_tz.tm_hour >= CLOSING_HOUR_BLACK_FRIDAY ) return true;
            break;
        case DEC:
            /* Christmas Day */
            if( NY_tz.tm_mday == 25 ) return true;
            if( NY_tz.tm_wday == MON && NY_tz.tm_mday >= 25 && NY_tz.tm_mday <= 27 ) return true;
            break;
        default: /* MAR || APR */
            /* Good Friday */
            if( NY_tz.tm_wday != FRI ) return false;
            easter( NY_tz.tm_year + 1900, &easter_month, &easter_day ); /* Finds the date of easter for a given year. */
            if( ( NY_tz.tm_mon + 1 ) == easter_month && NY_tz.tm_mday == ( easter_day - 2 ) ) return true;
            break;
    }

    return false;
}

bool TimeToClose ( bool holiday, int *h_r, int *m_r, int *s_r ) {
    /* The NYSE/NASDAQ Markets are open from 09:30 to 16:00 EST. */
    struct tm NY_tz;
    bool closed;
    
    /* Get the localtime in NY from Epoch seconds */
    /* Accounts if it is DST or STD time in NY. */
    NY_tz = NYTimeComponents ();

    int hour = NY_tz.tm_hour % 24;
    int min = NY_tz.tm_min % 60;
    int sec = NY_tz.tm_sec % 60;
    int weekday = NY_tz.tm_wday % 7;
    int month = NY_tz.tm_mon % 12;
    int dayofmonth = NY_tz.tm_mday;

    int hour_open_NY, hour_closed_NY, hour_closed_black_friday_NY;    

    hour_open_NY = OPEN_HOUR;
    hour_closed_NY = CLOSING_HOUR;
    hour_closed_black_friday_NY = CLOSING_HOUR_BLACK_FRIDAY;
    
    /* Closed */
    if( holiday || hour < hour_open_NY || ( hour == hour_open_NY && min < OPEN_MINUTE ) || hour >= hour_closed_NY || weekday % 6 == 0 ){
        *h_r = 0;
        *m_r = 0;
        *s_r = 0;
        closed = true;
    /* Open: closes early on black friday */
    } else if ( month == 10 && weekday == 5 && dayofmonth >= 23 && dayofmonth <= 29 ){
        *h_r = hour_closed_black_friday_NY - 1 - hour;
        *m_r = 59 - min;
        *s_r = 59 - sec;
        closed = false;
    /* Open */    
    } else {
        *h_r = hour_closed_NY - 1 - hour;
        *m_r = 59 - min;
        *s_r = 59 - sec;
        closed = false;
    }
    return closed;
}

static bool is_leap_year ( struct tm NY_tz ){
    short year = (short)( NY_tz.tm_year + 1900 );
    /* The year is evenly divisible by 4 and it is not evenly divisible by 100 */
    /* Or the year is evenly divisible by 400. */
    if( ( ( year % 4 == 0 ) && ( year % 100 != 0 ) ) || ( year % 400 == 0 ) )
    {
        /* Leap year. */
        return true;
    } else {
        /* Not a leap year. */
        return false;
    }
}

static void days_in_month_and_year ( struct tm NY_tz, short *days_in_month, short *days_in_year ){
    enum { JAN, FEB, MAR, APR, MAY, JUN, JUL, AUG, SEP, OCT, NOV, DEC };

    /* The number of days in the year is only relevent in December. */
    switch ( NY_tz.tm_mon ){
        /* Months with 30 days. */
        case ( APR ):
            *days_in_month = 30;
            *days_in_year = 365;
            return;
            break;
        case ( JUN ):
            *days_in_month = 30;
            *days_in_year = 365;
            return;
            break;
        case ( SEP ):
            *days_in_month = 30;
            *days_in_year = 365;
            return;
            break;
        case ( NOV ):
            *days_in_month = 30;
            *days_in_year = 365;
            return;
            break;
        /* Months with 31 days. */
        case ( JAN ):
            *days_in_month = 31;
            *days_in_year = 365;
            return;
            break;
        case ( MAR ):
            *days_in_month = 31;
            *days_in_year = 365;
            return;
            break;
        case ( MAY ):
            *days_in_month = 31;
            *days_in_year = 365;
            return;
            break;
        case ( JUL ):
            *days_in_month = 31;
            *days_in_year = 365;
            return;
            break;
        case ( AUG ):
            *days_in_month = 31;
            *days_in_year = 365;
            return;
            break;
        case ( OCT ):
            *days_in_month = 31;
            *days_in_year = 365;
            return;
            break;
        case ( DEC ):
            *days_in_month = 31;
            if( is_leap_year ( NY_tz ) )
            {
                /* Leap year. */
                *days_in_year = 366;
                return;
            } else {
                /* Not a leap year. */
                *days_in_year = 365;
                return;
            }
            break;
        /* February */
        default:
            if( is_leap_year ( NY_tz ) )
            {
                /* Leap year. */
                *days_in_month = 29;
                *days_in_year = 366;
                return;
            } else {
                /* Not a leap year. */
                *days_in_month = 28;
                *days_in_year = 365;
                return;
            }
            break;
    }
}

static void date_adjustment ( struct tm *NY_tz )
{
    short hour = (short)( (NY_tz->tm_hour)%24 );
    short weekday = (short)NY_tz->tm_wday;
    short hour_open_NY, hour_closed_NY;
    short days_in_mon, days_in_year, day_adjustment;
    short day_of_month = (short)NY_tz->tm_mday;
    short day_of_year = (short)NY_tz->tm_yday;
    short day_of_week = (short)NY_tz->tm_wday;
    enum { SUN, MON, TUES, WEDS, THURS, FRI, SAT };

    days_in_month_and_year ( *NY_tz, &days_in_mon, &days_in_year );

    hour_open_NY = OPEN_HOUR;
    hour_closed_NY = CLOSING_HOUR; /* The early close on Black Friday will yield a holiday = true */

    day_adjustment = 0;
    switch ( weekday ){
        case SUN:
            day_adjustment = 1;
            break;
        case FRI:
            /* It is past 16:00 EST/EDST or today is a holiday. */
            if ( hour >= hour_closed_NY || CheckHoliday ( *NY_tz ) ) day_adjustment = 3;
            break;
        case SAT:
            day_adjustment = 2;
            break;
        default: /* MON, TUES, WEDS, THURS */
            if ( hour >= hour_closed_NY || CheckHoliday ( *NY_tz ) ) day_adjustment = 1;
            break;
    };

    if ( ( day_of_month + day_adjustment ) > days_in_mon ) {
        /* Adjustment overflowed the month. */

        NY_tz->tm_mday = (int)( ( day_of_month + day_adjustment ) % days_in_mon );
        /* CheckHoliday () uses the weekday to determine if it is a holiday. */
        NY_tz->tm_wday = (int)( ( day_of_week + day_adjustment ) % 7 );
        NY_tz->tm_mon = ( NY_tz->tm_mon + 1 ) % 12;
    } else {
        /* Adjustment didn't overflow the month. */
        
        NY_tz->tm_mday += (int)day_adjustment;
        NY_tz->tm_wday = (int)( ( day_of_week + day_adjustment ) % 7 );
    }

    if ( ( day_of_year + day_adjustment ) > days_in_year ) {
        /* Adjustment overflowed the year. */
        NY_tz->tm_year++;
    }

    /* The market always opens at 09:30:00 EST/EDST */
    NY_tz->tm_hour = (int)hour_open_NY;
    NY_tz->tm_min = OPEN_MINUTE;
    NY_tz->tm_sec = 0;
    /* Let mktime () determine whether to use daylight savings time or not. */
    NY_tz->tm_isdst = -1;

    /* Check if the future date is a holiday.
       If so, increment the future day by 1. */
    /* If Memorial Day falls on the 31st, mktime() will adjust
       the month and day accordingly. The 32nd day of May is June 1st. */
    if ( CheckHoliday ( *NY_tz ) ) NY_tz->tm_mday++;
    /* If the future day is now a Saturday the next iteration of the close indicator loop
       will catch it.  The Thurs before Good Friday, for example. */
    return;
}

unsigned int SecondsToOpen () {
    /* The seconds until the NYSE/NASDAQ markets open. */
    struct tm NY_tz;
    time_t currenttime, futuretime;
    unsigned int diff;

    /* Get the localtime in NY from Epoch seconds 
       We need to do this to account for DST changes when
       determining the future Epoch time. 
    */

    NY_tz = NYTimeComponents ();    

    int hour = (NY_tz.tm_hour)%24;
    int min = NY_tz.tm_min;
    int weekday = NY_tz.tm_wday;
    int hour_open_NY, hour_closed_NY;

    hour_open_NY = OPEN_HOUR;
    hour_closed_NY = CLOSING_HOUR;

    /* If today isn't Sat or Sun */
    /* 6 % 6 == 0, 0 % 6 == 0 */
    if( weekday % 6 != 0 ){
        /* If it is currently normal operating hours */
        if ( ( hour > hour_open_NY || ( hour == hour_open_NY && min >= OPEN_MINUTE ) ) && hour < hour_closed_NY ){
            /* If today is not a holiday */
            if ( !CheckHoliday ( NY_tz ) ) {
                /* The market is open */
                return 0;
            }
        }
    }
    
    /* Find when the market opens */
    date_adjustment ( &NY_tz );
    
    /* The future NY EST/EDST date to time in Epoch seconds. */
    /* Takes into account daylight-savings time. */
    futuretime = mktime( &NY_tz );

    /* The current time in Epoch seconds */
    time( &currenttime );
    
    /* The market is closed, reopens in this many seconds. */
    diff = (unsigned int) difftime( futuretime, currenttime );

    return diff;
}