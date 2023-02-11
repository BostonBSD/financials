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

#ifndef WORKFUNCS_HEADER_H
#define WORKFUNCS_HEADER_H

#include "class_types.h" /* equity_folder, metal, meta, portfolio_packet */
#include "gui_types.h"   /* symbol_name_map */

/* pango_formatting */
enum {
  NO_COLOR,
  BLACK,
  GREEN,
  RED,
  BLUE,
  GREY,
  CYAN,
  ORANGE,
  CHOCOLATE,
  HEADING_ASST_TYPE_FORMAT,
  HEADING_UNLN_FORMAT,
  STR_TO_MON_STR,
  GREEN_ITALIC,
  RED_ITALIC,
  BLUE_ITALIC,
  BLACK_ITALIC
};

enum { MON_STR, PER_STR, NUM_STR };

void SetFont(const gchar *fnt);
void DoubleToFormattedStrPango(gchar **dst, const gdouble num,
                               const guint8 digits_right,
                               const guint format_type, const guint color);
void StringToStrPango(gchar **dst, const gchar *src, const guint color);

/* sn_map */
void AddSymbolToMap(const gchar *symbol, const gchar *name,
                    symbol_name_map *sn_map);
symbol_name_map *SymNameFetch(portfolio_packet *pkg);
symbol_name_map *SymNameFetchUpdate(portfolio_packet *pkg,
                                    symbol_name_map *sn_map);
gchar *GetSecurityName(gchar *s, const symbol_name_map *sn_map, meta *D);
void SNMapDestruct(symbol_name_map *sn_map);
void CreateHashTable(symbol_name_map *sn_map);

/* string_formatting */
gboolean CheckValidString(const gchar *string);
gboolean CheckIfStringDoubleNumber(const gchar *string);
gboolean CheckIfStringDoublePositiveNumber(const gchar *string);
gboolean CheckIfStringLongPositiveNumber(const gchar *string);
void CopyString(gchar **dst, const gchar *src);
void ToNumStr(gchar *s);
void StringToMonStr(gchar **dst, const gchar *src, const guint8 digits_right);
void DoubleToFormattedStr(gchar **dst, const gdouble num,
                          const guint8 digits_right, const guint format_type);
gdouble StringToDouble(const gchar *str);
gchar *PangoToCssFontStr(const gchar *pango_fnt_str);
gchar *SnPrint(const gchar *fmt, ...);

/* time_funcs */
guint64 ClockSleepMinute();
guint64 ClockSleepSecond();
gboolean GetTimeData(gboolean *holiday, gchar **holiday_str, gint *h_r,
                     gint *m_r, gint *s_r, gint *h_cur, gint *m_cur);

/* working_functions */
gdouble CalcGain(gdouble cur_price, gdouble prev_price);
void CalcSumRsi(gdouble current_gain, gdouble *avg_gain, gdouble *avg_loss);
void CalcRunAvgRsi(gdouble current_gain, gdouble *avg_gain, gdouble *avg_loss,
                   gdouble period);
gdouble CalcRsi(gdouble avg_gain, gdouble avg_loss);
gchar *ExtractYahooData(FILE *fp, gdouble *prev_closing_f,
                        gdouble *cur_price_f);
void GetYahooUrl(gchar **url_ch, const gchar *symbol_ch, guint period);

#endif /* WORKFUNCS_HEADER_H */