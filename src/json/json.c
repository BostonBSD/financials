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

#include <json-glib/json-glib.h>

void *JsonExtractEquity(char *str, double *current_price_f, double *high_f,
                        double *low_f, double *opening_f,
                        double *prev_closing_f, double *ch_share_f,
                        double *ch_percent_f)
/* Take in a string of JSON data, and references to several double parameters.
 */
{
  if (str == NULL)
    return NULL;

  JsonParser *parser = json_parser_new();
  if (parser == NULL)
    return NULL;

  /* Parse JSON data with json-glib */
  if (json_parser_load_from_data(parser, str, -1, NULL) == TRUE) {

    JsonReader *reader = json_reader_new(json_parser_get_root(parser));

    /* Extract current price from JSON data as a double. */
    json_reader_read_member(reader, "c");
    *current_price_f = json_reader_get_double_value(reader);
    json_reader_end_member(reader);

    /* Extract today's high from JSON data as a double. */
    json_reader_read_member(reader, "h");
    *high_f = json_reader_get_double_value(reader);
    json_reader_end_member(reader);

    /* Extract today's low from JSON data as a double. */
    json_reader_read_member(reader, "l");
    *low_f = json_reader_get_double_value(reader);
    json_reader_end_member(reader);

    /* Extract today's opening from JSON data as a double. */
    json_reader_read_member(reader, "o");
    *opening_f = json_reader_get_double_value(reader);
    json_reader_end_member(reader);

    /* Extract previous closing from JSON data as a double. */
    json_reader_read_member(reader, "pc");
    *prev_closing_f = json_reader_get_double_value(reader);
    json_reader_end_member(reader);

    /* Extract change in share price in USD from JSON data as a double. */
    json_reader_read_member(reader, "d");
    *ch_share_f = json_reader_get_double_value(reader);
    json_reader_end_member(reader);

    /* Extract change in share price as a percent from JSON data as a double. */
    json_reader_read_member(reader, "dp");
    *ch_percent_f = json_reader_get_double_value(reader);
    json_reader_end_member(reader);

    g_object_unref(reader);
  }

  g_object_unref(parser);
  return NULL;
}