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

#include <json-glib/json-glib.h>

typedef struct {
  gdouble *current_price_f;
  gdouble *high_f;
  gdouble *low_f;
  gdouble *opening_f;
  gdouble *prev_closing_f;
  gdouble *ch_share_f;
  gdouble *ch_percent_f;
} stock_stats;

static void reset_data(stock_stats *vals) {
  *vals->current_price_f = 0.0f;
  *vals->high_f = 0.0f;
  *vals->low_f = 0.0f;
  *vals->opening_f = 0.0f;
  *vals->prev_closing_f = 0.0f;
  *vals->ch_share_f = 0.0f;
  *vals->ch_percent_f = 0.0f;
}

static gboolean read_member_error(JsonReader *reader, JsonParser *parser) {
  json_reader_end_member(reader);
  g_object_unref(reader);
  g_object_unref(parser);
  /* Return FALSE when error occurs. */
  return FALSE;
}

static gboolean read_member(JsonParser *parser, JsonReader *reader,
                            const gchar *mem_name_ch, gdouble *value_f,
                            stock_stats *vals) {
  if (!json_reader_read_member(reader, mem_name_ch)) {
    reset_data(vals);
    return read_member_error(reader, parser);
  }

  *value_f = json_reader_get_double_value(reader);
  json_reader_end_member(reader);
  return TRUE;
}

gboolean JsonExtractEquity(gchar *str, gdouble *current_price_f,
                           gdouble *high_f, gdouble *low_f, gdouble *opening_f,
                           gdouble *prev_closing_f, gdouble *ch_share_f,
                           gdouble *ch_percent_f)
/* Take in a string of JSON data, and references to several double parameters.
   Return TRUE if data successfully extracted, FALSE otherwise.
*/
{
  stock_stats vals =
      (stock_stats){current_price_f, high_f,     low_f,       opening_f,
                    prev_closing_f,  ch_share_f, ch_percent_f};

  if (str == NULL) {
    reset_data(&vals);
    return FALSE;
  }

  JsonParser *parser = json_parser_new();
  if (parser == NULL) {
    reset_data(&vals);
    return FALSE;
  }

  /* Parse JSON data with json-glib */
  if (json_parser_load_from_data(parser, str, -1, NULL) == TRUE) {

    JsonReader *reader = json_reader_new(json_parser_get_root(parser));

    /* Extract current price from JSON data as a double. */
    if (!read_member(parser, reader, "c", current_price_f, &vals))
      return FALSE;

    /* Extract today's high from JSON data as a double. */
    if (!read_member(parser, reader, "h", high_f, &vals))
      return FALSE;

    /* Extract today's low from JSON data as a double. */
    if (!read_member(parser, reader, "l", low_f, &vals))
      return FALSE;

    /* Extract today's opening from JSON data as a double. */
    if (!read_member(parser, reader, "o", opening_f, &vals))
      return FALSE;

    /* Extract previous closing from JSON data as a double. */
    if (!read_member(parser, reader, "pc", prev_closing_f, &vals))
      return FALSE;

    /* Extract change in share price in USD from JSON data as a double. */
    if (!read_member(parser, reader, "d", ch_share_f, &vals))
      return FALSE;

    /* Extract change in share price as a percent from JSON data as a double. */
    if (!read_member(parser, reader, "dp", ch_percent_f, &vals))
      return FALSE;

    g_object_unref(reader);
  }

  g_object_unref(parser);
  return TRUE;
}