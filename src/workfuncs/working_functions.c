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

#include <gio/gio.h>
#include <glib/gprintf.h>

#include "../include/class_types.h"
#include "../include/macros.h"

/* For auditing purposes, I included the effective formulas in comments. */
gdouble CalcGain(gdouble cur_price, gdouble prev_price) {
  /* return (100 * ((cur_price - prev_price) / prev_price)); */
  return ((100 * (cur_price / prev_price)) - 100);
}

void CalcSumRsi(gdouble current_gain, gdouble *avg_gain, gdouble *avg_loss) {
  if (current_gain >= 0) {
    *avg_gain += current_gain;
  } else {
    /* *avg_loss += (-1 * current_gain); */
    *avg_loss -= current_gain;
  }
}

void CalcRunAvgRsi(gdouble current_gain, gdouble *avg_gain, gdouble *avg_loss,
                   gdouble period) {
  if (current_gain >= 0) {
    /*
     *avg_gain = ((*avg_gain * (period - 1)) + current_gain) / period;
     *avg_loss = ((*avg_loss * (period - 1)) + 0) / period; */
    *avg_gain += ((current_gain - *avg_gain) / period);
    *avg_loss -= (*avg_loss / period);
  } else {
    /*
     *avg_gain = ((*avg_gain * (period - 1)) + 0) / period;
     *avg_loss = ((*avg_loss * (period - 1)) + (-1 * current_gain)) / period; */
    *avg_gain -= (*avg_gain / period);
    *avg_loss -= (*avg_loss + current_gain) / period;
  }
}

gdouble CalcRsi(gdouble avg_gain, gdouble avg_loss) {
  /* gdouble rs = avg_gain / avg_loss;
     return (100 - (100 / (1 + rs))); */
  return (100 * avg_gain) / (avg_loss + avg_gain);
}

GDataInputStream *StringToInputStream(const gchar *str) {
  /* Take in a gchar string and convert to GDataInputStream object [this will
   * duplicate the gchar string in memory]. */
  GBytes *bytes =
      g_bytes_new((gconstpointer)str, (gsize)g_utf8_strlen(str, -1) + 1);
  GInputStream *in_stream = g_memory_input_stream_new_from_bytes(bytes);
  g_bytes_unref(bytes);
  return g_data_input_stream_new(in_stream);
}

gchar *ReadLine(GDataInputStream *in) {
  /* Read one line from the input stream.

     Return a newly allocated NULL terminated string without a newline char.
     Must free return value. Will return NULL if the end of the stream has been
     reached. */
  GError *error = NULL;
  gchar *str = g_data_input_stream_read_line(in, NULL, NULL, &error);
  if (error) {
    g_printf("Read Line (from GDataInputStream) Error: %s\n", error->message);
    g_error_free(error);
  }
  return str;
}

void CloseInputStream(GDataInputStream *in) {
  /* Free the GDataInputStream object and all resources associated with it. */
  GError *error = NULL;
  g_input_stream_close(G_INPUT_STREAM(in), NULL, &error);
  if (error) {
    g_printf("Close Input Stream Error: %s\n", error->message);
    g_error_free(error);
  }
}

gchar *ExtractYahooData(GDataInputStream *in, gdouble *prev_closing_f,
                        gdouble *cur_price_f)
/* Take in a GDataInputStream, and references to two doubles; prev_closing_f and
   cur_price_f.  Will populate the previous closing price and the current price.

   Returns the last line of the GDataInputStream.
   Must free return value.

   This is useful for finding the current stats on a security/index/commodity
   from Yahoo! finance.
*/
{
  gchar *ret_value = NULL, *line = NULL;
  gchar **token_arr;

  /* Yahoo! sometimes updates data when the equities markets are closed.
     The while loop iterates to the end of file to get the latest data. */
  *prev_closing_f = 0.0f;
  *cur_price_f = 0.0f;
  while ((line = ReadLine(in))) {
    *prev_closing_f = *cur_price_f;
    /* Sometimes the API gives us a null value for certain days.
       using the closing price from the day prior gives us a more accurate
       gain value. */
    /* If we have an empty line, continue. */
    if (g_strrstr(line, "null") || g_strrstr(line, "Date") || line[0] == '\0') {
      g_free(line);
      continue;
    }

    /* Invalid replies start with a tag. */
    if (g_strrstr(line, "<")) {
      g_free(ret_value);
      g_free(line);
      return NULL;
    }

    token_arr = g_strsplit(line, ",", -1);
    if (g_strv_length(token_arr) < 7) {
      g_free(ret_value);
      g_free(line);
      g_strfreev(token_arr);
      return NULL;
    }
    *cur_price_f = g_strtod(token_arr[4] ? token_arr[4] : "0", NULL);
    g_strfreev(token_arr);

    g_free(ret_value);
    ret_value = g_strdup(line);
    g_free(line);
  };
  return ret_value;
}

static gint64 unix_time_sec() {
  /* Return the unix time in seconds; rounded down to the nearest second */
  gint64 time_usec = g_get_real_time();
  return (time_usec - (time_usec % G_TIME_SPAN_SECOND)) / G_TIME_SPAN_SECOND;
}

void GetYahooUrl(gchar **url_ch, const gchar *symbol_ch, guint period) {
  gint64 end_time, start_time;
  gushort len;

  end_time = unix_time_sec();
  start_time = end_time - (gint64)period;

  const gchar *fmt = YAHOO_URL_START
      "%s" YAHOO_URL_MIDDLE_ONE "%ld" YAHOO_URL_MIDDLE_TWO "%ld" YAHOO_URL_END;

  len = g_snprintf(NULL, 0, fmt, symbol_ch, start_time, end_time) + 1;
  gchar *tmp = g_realloc(url_ch[0], len);
  url_ch[0] = tmp;

  g_snprintf(url_ch[0], len, fmt, symbol_ch, start_time, end_time);
}