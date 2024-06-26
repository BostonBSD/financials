/*
Copyright (c) 2022-2024 BostonBSD. All rights reserved.

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

#include "../include/macros.h"
#include "../include/workfuncs.h"

/* Font name file global. */
static const gchar *font_name;

void SetFont(const gchar *fnt) { font_name = fnt; }

static void create_markup(gchar **str, const gchar *fmt, ...)
/* Create a string from a format string and a list of arguments.
   Useful for creating marked up strings and their attribute components.

   Make sure *str = NULL or an allocated pointer address. */
{
  va_list arg_ptr;

  /* Set the arg pointer. */
  va_start(arg_ptr, fmt);
  /* Get the size of the potential string. */
  gsize len = g_vsnprintf(NULL, 0, fmt, arg_ptr) + 1;
  va_end(arg_ptr);

  /* Realloc str[0] to the string size. */
  gchar *tmp = g_realloc(str[0], len);
  str[0] = tmp;

  /* Reset the arg pointer. */
  va_start(arg_ptr, fmt);
  /* Create the markup string from format and argument list. */
  g_vsnprintf(str[0], len, fmt, arg_ptr);
  va_end(arg_ptr);
}

static gchar *font_attr(const gchar *fnt_name) {
  const gchar *fnt_fmt = "font_desc='%s'";
  gchar *attr = NULL;

  create_markup(&attr, fnt_fmt, fnt_name);
  return attr;
}

static gchar *fg_attr(const gchar *fg_color) {
  const gchar *fg_fmt = "foreground='%s'";
  gchar *attr = NULL;

  create_markup(&attr, fg_fmt, fg_color);
  return attr;
}

static gchar *wght_attr(const gchar *wght_name) {
  const gchar *wght_fmt = "weight='%s'";
  gchar *attr = NULL;

  create_markup(&attr, wght_fmt, wght_name);
  return attr;
}

static gchar *undln_attr(const gchar *undln_type) {
  const gchar *undln_fmt = "underline='%s'";
  gchar *attr = NULL;

  create_markup(&attr, undln_fmt, undln_type);
  return attr;
}

static gchar *undln__colr_attr(const gchar *undln_color) {
  const gchar *undln_colr_fmt = "underline_color='%s'";
  gchar *attr = NULL;

  create_markup(&attr, undln_colr_fmt, undln_color);
  return attr;
}

static gchar *style_attr(const gchar *style_type) {
  const gchar *style_fmt = "style='%s'";
  gchar *attr = NULL;

  create_markup(&attr, style_fmt, style_type);
  return attr;
}

void StringToStrPango(gchar **dst, const gchar *src, const guint color)
/* Take in a string buffer, a src string, and a color macro, convert to Pango
   Markup string.

   Reallocs memory to fit the output string.

   Take care that *dst is not an unallocated ptr address.
   Set *dst = NULL first.*/
{
  if (!dst || !src)
    return;

  gchar *font = font_attr(font_name);
  gchar *fg = NULL;
  gchar *wght = NULL;
  gchar *undln = NULL;
  gchar *undln_colr = NULL;
  gchar *style = NULL;
  gchar *tmp = NULL;
  const gchar *format_three = "<span %s %s>%s</span>";
  const gchar *format_four = "<span %s %s %s>%s</span>";
  const gchar *format_five = "<span %s %s %s %s>%s</span>";
  const gchar *format_six = "<span %s %s %s %s %s>%s</span>";

  switch (color) {
  case NO_COLOR:
    wght = wght_attr("Medium");
    create_markup(dst, format_three, font, wght, src);
    break;
  case BLACK:
    fg = fg_attr("Black");
    wght = wght_attr("Medium");
    create_markup(dst, format_four, font, fg, wght, src);
    break;
  case RED:
    fg = fg_attr("DarkRed");
    wght = wght_attr("Medium");
    create_markup(dst, format_four, font, fg, wght, src);
    break;
  case GREEN:
    fg = fg_attr("DarkGreen");
    wght = wght_attr("Medium");
    create_markup(dst, format_four, font, fg, wght, src);
    break;
  case BLUE:
    fg = fg_attr("MidnightBlue");
    wght = wght_attr("Medium");
    create_markup(dst, format_four, font, fg, wght, src);
    break;
  case GREY:
    fg = fg_attr("DarkSlateGrey");
    wght = wght_attr("Medium");
    create_markup(dst, format_four, font, fg, wght, src);
    break;
  case CYAN:
    fg = fg_attr("DarkCyan");
    wght = wght_attr("Medium");
    create_markup(dst, format_four, font, fg, wght, src);
    break;
  case ORANGE:
    fg = fg_attr("OrangeRed");
    wght = wght_attr("Medium");
    create_markup(dst, format_four, font, fg, wght, src);
    break;
  case CHOCOLATE:
    fg = fg_attr("Chocolate");
    wght = wght_attr("Medium");
    create_markup(dst, format_four, font, fg, wght, src);
    break;
  case SIENNA:
    fg = fg_attr("Sienna");
    wght = wght_attr("Medium");
    create_markup(dst, format_four, font, fg, wght, src);
    break;
  case BLACK_ITALIC:
    fg = fg_attr("Black");
    wght = wght_attr("Medium");
    style = style_attr("italic");
    create_markup(dst, format_five, font, fg, wght, style, src);
    break;
  case RED_ITALIC:
    fg = fg_attr("DarkRed");
    wght = wght_attr("Medium");
    style = style_attr("italic");
    create_markup(dst, format_five, font, fg, wght, style, src);
    break;
  case GREEN_ITALIC:
    fg = fg_attr("DarkGreen");
    wght = wght_attr("Medium");
    style = style_attr("italic");
    create_markup(dst, format_five, font, fg, wght, style, src);
    break;
  case BLUE_ITALIC:
    fg = fg_attr("MidnightBlue");
    wght = wght_attr("Medium");
    style = style_attr("italic");
    create_markup(dst, format_five, font, fg, wght, style, src);
    break;
  case STR_TO_MON_STR:
    /* This is a special conversion, make sure the string can be converted to a
     * double */
    StringToMonStr(&tmp, src, 2);
    fg = fg_attr("DarkSlateGrey");
    wght = wght_attr("Medium");
    create_markup(dst, format_four, font, fg, wght, tmp);
    break;
  case HEADING_ASST_TYPE_FORMAT:
    fg = fg_attr("DarkSlateGray");
    wght = wght_attr("Demi-Bold");
    create_markup(dst, format_four, font, fg, wght, src);
    break;
  default: /* HEADING_UNLN_FORMAT */
    fg = fg_attr("SaddleBrown");
    wght = wght_attr("Demi-Bold");
    undln = undln_attr("single");
    undln_colr = undln__colr_attr("SaddleBrown");
    create_markup(dst, format_six, font, fg, wght, undln, undln_colr, src);
    break;
  }

  if (font)
    g_free(font);
  if (fg)
    g_free(fg);
  if (wght)
    g_free(wght);
  if (undln)
    g_free(undln);
  if (undln_colr)
    g_free(undln_colr);
  if (style)
    g_free(style);
  if (tmp)
    g_free(tmp);
}

void DoubleToFormattedStrPango(gchar **dst, const gdouble num,
                               const guint8 digits_right,
                               const guint format_type, const guint color)
/* Take in a string buffer, a double, the number of digits
   to the right of the decimal point, a format type, and a color.

   Convert to a formatted Pango Markup string, grouping the digits according to
   the locale [dec points or commas].

   Reallocs memory to fit the output string.

   Take care that *dst is not an unallocated ptr address.
   Set *dst = NULL first.

   The type macros are; MON_STR, PER_STR, NUM_STR.
   The available colors can be found in the enum definition.

   The macro enums are defined in workfuncs.h.
*/
{
  gchar *tmp = NULL;
  DoubleToFormattedStr(&tmp, num, digits_right, format_type);
  StringToStrPango(dst, tmp, color);
  g_free(tmp);
}

void RangeStrPango(gchar **dest, const double low_f, const double high_f,
                   const guint8 digits_right) {
  gchar *low_ch = NULL, *high_ch = NULL, *range_ch = NULL;

  /* Create the monetary format strings. */
  DoubleToFormattedStr(&low_ch, low_f, digits_right, MON_STR);

  DoubleToFormattedStr(&high_ch, high_f, digits_right, MON_STR);

  /* Create the unmarked string. */
  range_ch = SnPrint("%s - %s", low_ch, high_ch);

  /* Create the marked up string. */
  StringToStrPango(dest, range_ch, GREY);

  g_free(low_ch);
  g_free(high_ch);
  g_free(range_ch);
}

void ChangeStrPango(gchar **dest, const double value_f, const double percent_f,
                    const guint8 digits_right) {
  gchar *value_ch = NULL, *percent_ch = NULL, *val_per_ch = NULL;

  /* Create the monetary and percent format strings. */
  DoubleToFormattedStr(&value_ch, value_f, digits_right, MON_STR);

  DoubleToFormattedStr(&percent_ch, percent_f, digits_right, PER_STR);

  /* Create the unmarked string. */
  val_per_ch = SnPrint("%s\n%s", value_ch, percent_ch);

  /* Create the marked up string. */
  if (value_f < 0) {
    StringToStrPango(dest, val_per_ch, RED);
  } else if (value_f > 0) {
    StringToStrPango(dest, val_per_ch, GREEN);
  } else {
    StringToStrPango(dest, val_per_ch, BLACK);
  }

  g_free(value_ch);
  g_free(percent_ch);
  g_free(val_per_ch);
}

void TotalStrPango(gchar **dest, const double total_f, const double gain_f,
                   const guint8 digits_right) {
  gchar *total_ch = NULL, *gain_ch = NULL, *tmp_ch = NULL;
  gchar *total_mrkd_ch = NULL, *gain_mrkd_ch = NULL;

  /* Create the monetary format strings. */
  DoubleToFormattedStr(&tmp_ch, total_f, digits_right, MON_STR);
  total_ch = SnPrint("%s\n", tmp_ch);
  g_free(tmp_ch);

  DoubleToFormattedStr(&gain_ch, gain_f, digits_right, MON_STR);

  /* Create the first marked string. */
  StringToStrPango(&total_mrkd_ch, total_ch, BLACK);

  /* Create the second marked string. */
  if (gain_f < 0) {
    StringToStrPango(&gain_mrkd_ch, gain_ch, RED);
  } else if (gain_f > 0) {
    StringToStrPango(&gain_mrkd_ch, gain_ch, GREEN);
  } else {
    StringToStrPango(&gain_mrkd_ch, gain_ch, BLACK);
  }

  /* Find allocation size */
  gsize len = g_snprintf(NULL, 0, "%s%s", total_mrkd_ch, gain_mrkd_ch) + 1;
  tmp_ch = g_realloc(dest[0], len);
  dest[0] = tmp_ch;

  g_snprintf(dest[0], len, "%s%s", total_mrkd_ch, gain_mrkd_ch);

  g_free(total_ch);
  g_free(gain_ch);
  g_free(total_mrkd_ch);
  g_free(gain_mrkd_ch);
}

void SymbolStrPango(gchar **dest, const gchar *symbol_ch,
                    const double quantity_f, const guint8 digits_right,
                    const guint color) {
  gchar *quantity_ch = NULL, *tmp_ch = NULL;
  gchar *symbol_mrkd_ch = NULL, *quantity_mrkd_ch = NULL;

  /* Create the unmarked strings. */
  tmp_ch = SnPrint("%s\n", symbol_ch);

  DoubleToFormattedStr(&quantity_ch, quantity_f, digits_right, NUM_STR);

  /* Create the first marked string. */
  StringToStrPango(&symbol_mrkd_ch, tmp_ch, color);
  g_free(tmp_ch);

  /* Create the second marked string. */
  StringToStrPango(&quantity_mrkd_ch, quantity_ch, SIENNA);

  /* Find allocation size */
  gsize len = g_snprintf(NULL, 0, "%s%s", symbol_mrkd_ch, quantity_mrkd_ch) + 1;
  tmp_ch = g_realloc(dest[0], len);
  dest[0] = tmp_ch;

  g_snprintf(dest[0], len, "%s%s", symbol_mrkd_ch, quantity_mrkd_ch);

  g_free(quantity_ch);
  g_free(symbol_mrkd_ch);
  g_free(quantity_mrkd_ch);
}