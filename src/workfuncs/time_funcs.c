/*
Copyright (c) 2022-2023 BostonBSD. All rights reserved.

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

#include <glib.h>
#include <math.h>

#include "../include/macros.h"

/* The open/closing time in the NY timezone. */
#define OPEN_HOUR 9
#define OPEN_MINUTE 30
#define CLOSING_HOUR 16
#define CLOSING_HOUR_BLACK_FRIDAY 13

/*  ISO 8601 (Glib) enumerates months and weekdays starting with 1.
    This is different from ISO 9945-1:1996 (struct tm), which
    enumerates starting at 0. */
enum {
  MONTH_ERROR,
  JAN,
  FEB,
  MAR,
  APR,
  MAY,
  JUN,
  JUL,
  AUG,
  SEP,
  OCT,
  NOV,
  DEC
};
enum { DAY_ERROR, MON, TUES, WEDS, THURS, FRI, SAT, SUN };

/* Time calculations are based off of the New York timezone due to
 * daylight-savings time adjustments, the process timezone remains unchanged. */
static const gchar *ny_tz_str = NEW_YORK_TIME_ZONE;

static gint64 time_span_remaining(gint64 time_span_usec) {
  /* Return the number of microseconds left in the current timespan [hour, min,
  sec, etc]

  The timespan minus the number of microsec which have past in the current time
  span. */
  gint64 cur_time = g_get_real_time();
  return time_span_usec - (cur_time % time_span_usec);
}

guint64 ClockSleepSecond() {
  /* Return the number of micro-seconds remaining in the current second. */
  return (guint64)time_span_remaining(G_TIME_SPAN_SECOND);
}

guint64 ClockSleepMinute() {
  /* Return the number of microseconds remaining in the current minute. */
  return (guint64)time_span_remaining(G_TIME_SPAN_MINUTE);
}

static void easter(gint year, gint *month, gint *day)
/* Modified From: https://c-for-dummies.com/blog/?p=2446 by dgookin */
/* For any given year, will determine the month and day of Easter Sunday.
   Month numbering starts at 1; March is 3, April is 4, etc. */
{
  gint Y, a, c, e, h, k, L;
  gdouble b, d, f, g, i, m;

  Y = year;
  a = Y % 19;
  b = floor(Y / 100);
  c = Y % 100;
  d = floor(b / 4);
  e = (gint)b % 4;
  f = floor((b + 8) / 25);
  g = floor((b - f + 1) / 3);
  h = (19 * a + (gint)b - (gint)d - (gint)g + 15) % 30;
  i = floor(c / 4);
  k = c % 4;
  L = (32 + 2 * e + 2 * (gint)i - h - k) % 7;
  m = floor((a + 11 * h + 22 * L) / 451);
  *month = (gint)floor((h + L - 7 * m + 114) / 31);
  *day = (gint)((h + L - 7 * (gint)m + 114) % 31) + 1;
}

static gboolean check_holiday(gint year, gint month, gint dayofmonth,
                              gint weekday, gint hour, gchar **holiday_str) {
  /* Take in the year, month, dayofmonth, weekday, hour, and a holiday string
   * buffer. Return whether it is a holiday or not. Populate *holiday_str with
   * a string, if holiday, otherwise NULL. */
  gint easter_month, easter_day;

  holiday_str[0] = NULL;

  switch (month) {
  case JAN:
    /* New Years Day */
    if (dayofmonth == 1) {
      holiday_str[0] = "Market Closed - New Years Day";
      return TRUE;
      break;
    }
    if (weekday == MON && dayofmonth > 1 && dayofmonth <= 3) {
      holiday_str[0] = "Market Closed - New Years Holiday";
      return TRUE;
      break;
    }

    /* Martin Luther King Junior Day */
    if (weekday == MON && dayofmonth >= 15 && dayofmonth <= 21) {
      holiday_str[0] = "Market Closed - Martin Luther King Jr. Day";
      return TRUE;
    }
    break;
  case FEB:
    /* Presidents Day */
    if (weekday == MON && dayofmonth >= 15 && dayofmonth <= 21) {
      holiday_str[0] = "Market Closed - Presidents Day";
      return TRUE;
    }
    break;
  case MAY:
    /* Memorial Day */
    if (weekday == MON && dayofmonth >= 25 && dayofmonth <= 31) {
      holiday_str[0] = "Market Closed - Memorial Day";
      return TRUE;
    }
    break;
  case JUN:
    /* Juneteenth Day */
    if (dayofmonth == 19) {
      holiday_str[0] = "Market Closed - Juneteenth Day";
      return TRUE;
      break;
    }
    if (weekday == MON && dayofmonth > 19 && dayofmonth <= 21) {
      holiday_str[0] = "Market Closed - Juneteenth Holiday";
      return TRUE;
    }
    break;
  case JUL:
    /* US Independence Day */
    if (dayofmonth == 4) {
      holiday_str[0] = "Market Closed - US Independence Day";
      return TRUE;
      break;
    }
    if (weekday == MON && dayofmonth > 4 && dayofmonth <= 6) {
      holiday_str[0] = "Market Closed - US Independence Holiday";
      return TRUE;
    }
    break;
  case AUG:
    break;
  case SEP:
    /* Labor Day */
    if (weekday == MON && dayofmonth >= 1 && dayofmonth <= 7) {
      holiday_str[0] = "Market Closed - Labor Day";
      return TRUE;
    }
    break;
  case OCT:
    break;
  case NOV:
    /* Thanksgiving Day */
    if (weekday == THURS && dayofmonth >= 22 && dayofmonth <= 28) {
      holiday_str[0] = "Market Closed - Thanksgiving Day";
      return TRUE;
      break;
    }
    /* Black Friday */
    if (weekday == FRI && dayofmonth >= 23 && dayofmonth <= 29 &&
        hour >= CLOSING_HOUR_BLACK_FRIDAY) {
      holiday_str[0] = "Market Closed Early - Black Friday";
      return TRUE;
    }
    break;
  case DEC:
    /* Christmas Day */
    if (dayofmonth == 25) {
      holiday_str[0] = "Market Closed - Christmas Day";
      return TRUE;
      break;
    }
    if (weekday == MON && dayofmonth > 25 && dayofmonth <= 27) {
      holiday_str[0] = "Market Closed - Christmas Holiday";
      return TRUE;
    }
    break;
  default: /* MAR || APR */
    /* Good Friday */
    if (weekday != FRI)
      break;

    easter(year, &easter_month,
           &easter_day); /* Finds the date of easter for a given year. */
    if (month == easter_month && dayofmonth == (easter_day - 2)) {
      holiday_str[0] = "Market Closed - Good Friday";
      return TRUE;
    }
    break;
  }

  return FALSE;
}

gboolean GetTimeData(gboolean *holiday, gchar **holiday_str, gint *h_r,
                     gint *m_r, gint *s_r, gint *h_cur, gint *m_cur)

{
  /* Return TRUE if the markets are closed, populate other args accordingly. */
  /* Will populate any combination of args, unused args should be sent as NULL.

  *holiday; TRUE or FALSE whether today is a holiday or not.
  holiday_str[0]; If today is a holiday will set the holiday string, otherwise,
   holiday_str[0] = NULL.
  *h_r, *m_r, *s_r; The hours, minutes, and seconds remaining until market
  close.
  *h_cur, *m_cur; The current hour and minute in the America/New_York timezone.

  */

  /* The NYSE/NASDAQ Markets are open from 09:30 to 16:00 EST. */
  gboolean closed;

  /* Get the GDateTime object for the New York timezone. */
  /* g_time_zone_new() is deprecated in Glib 2.68, however, its replacement;
   * g_time_zone_new_identifier() is unavailable on earlier Glib versions used
   * by some OSs; Debian. */
  GTimeZone *ny_tz = g_time_zone_new(ny_tz_str);
  GDateTime *dt = g_date_time_new_now(ny_tz);
  g_time_zone_unref(ny_tz);
  gint year = g_date_time_get_year(dt);
  gint month = g_date_time_get_month(dt);
  gint dayofmonth = g_date_time_get_day_of_month(dt);
  gint weekday = g_date_time_get_day_of_week(dt);
  gint hour = g_date_time_get_hour(dt);
  gint min = g_date_time_get_minute(dt);
  gint sec = g_date_time_get_second(dt);
  g_date_time_unref(dt);

  gchar *hol_str;
  gboolean hol_bool =
      check_holiday(year, month, dayofmonth, weekday, hour, &hol_str);

  if (holiday)
    *holiday = hol_bool;
  if (holiday_str)
    holiday_str[0] = hol_str;

  /* Set the current hour and minute in the NY timezone */
  if (h_cur)
    *h_cur = hour;
  if (m_cur)
    *m_cur = min;

  /* Closed */
  if (weekday == 6 || weekday == 7 || hol_bool || hour < OPEN_HOUR ||
      (hour == OPEN_HOUR && min < OPEN_MINUTE) || hour >= CLOSING_HOUR) {
    if (h_r)
      *h_r = 0;
    if (m_r)
      *m_r = 0;
    if (s_r)
      *s_r = 0;
    closed = TRUE;
    /* Open: closes early on black friday */
  } else if (month == 11 && weekday == 5 && dayofmonth >= 23 &&
             dayofmonth <= 29) {
    if (h_r)
      *h_r = CLOSING_HOUR_BLACK_FRIDAY - 1 - hour;
    if (m_r)
      *m_r = 59 - min;
    if (s_r)
      *s_r = 59 - sec;
    closed = FALSE;
    /* Open */
  } else {
    if (h_r)
      *h_r = CLOSING_HOUR - 1 - hour;
    if (m_r)
      *m_r = 59 - min;
    if (s_r)
      *s_r = 59 - sec;
    closed = FALSE;
  }
  return closed;
}