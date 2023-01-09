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

#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include <locale.h>
#include <monetary.h>

#include "../include/macros.h"
#include "../include/workfuncs.h"

/* Font and size file global. */
static const char *font_name;

void SetFont(const char *fnt) { font_name = fnt; }

static void create_markup(char **str, const char *fmt, ...)
/* Create a string from a format string and a list of arguments.
   Useful for creating marked up strings and their attribute components.

   Make sure *str = NULL or an allocated pointer address. */
{
  va_list arg_ptr;

  /* Set the arg pointer. */
  va_start(arg_ptr, fmt);
  /* Get the size of the potential string. */
  size_t len = vsnprintf(NULL, 0, fmt, arg_ptr);
  va_end(arg_ptr);

  /* Realloc str[0] to the string size. */
  if (str[0] == NULL)
    str[0] = malloc(1);
  char *tmp = realloc(str[0], len + 1);
  str[0] = tmp;

  /* Reset the arg pointer. */
  va_start(arg_ptr, fmt);
  /* Create the markup string from format and argument list. */
  vsnprintf(str[0], len + 1, fmt, arg_ptr);
  va_end(arg_ptr);
}

static char *font_attr(const char *fnt_name) {
  const char *fnt_fmt = "font_desc='%s'";
  char *attr = NULL;

  create_markup(&attr, fnt_fmt, fnt_name);
  return attr;
}

static char *fg_attr(const char *fg_color) {
  const char *fg_fmt = "foreground='%s'";
  char *attr = NULL;

  create_markup(&attr, fg_fmt, fg_color);
  return attr;
}

static char *wght_attr(const char *wght_name) {
  const char *wght_fmt = "weight='%s'";
  char *attr = NULL;

  create_markup(&attr, wght_fmt, wght_name);
  return attr;
}

static char *undln_attr(const char *undln_type) {
  const char *undln_fmt = "underline='%s'";
  char *attr = NULL;

  create_markup(&attr, undln_fmt, undln_type);
  return attr;
}

static char *undln__colr_attr(const char *undln_color) {
  const char *undln_colr_fmt = "underline_color='%s'";
  char *attr = NULL;

  create_markup(&attr, undln_colr_fmt, undln_color);
  return attr;
}

static char *style_attr(const char *style_type) {
  const char *style_fmt = "style='%s'";
  char *attr = NULL;

  create_markup(&attr, style_fmt, style_type);
  return attr;
}

void DoubleToMonStrPango(char **dst, const double num,
                         const unsigned short digits_right)
/* Take in a string buffer, a double, and the number of digits
   to the right of the decimal point, convert to a monetary format
   Pango Markup string, grouping the digits according to the locale
   [dec points or commas].

   Reallocs memory to fit the output string.

   Take care that *dst is not an unallocated ptr address.
   Set *dst = NULL first.*/
{
  if (!dst)
    return;
  if (dst[0] == NULL)
    dst[0] = malloc(1);

  char *num_ch = NULL;
  /* Get the monetary string from the number */
  DoubleToMonStr(&num_ch, num, digits_right);

  /* Set the attributes */
  char *font = font_attr(font_name);
  char *fg = fg_attr("Black");
  char *wght = wght_attr("Medium");
  const char *format = "<span %s %s %s>%s</span>";

  /* Create the marked up string from the attributes and the monetary string */
  create_markup(dst, format, font, fg, wght, num_ch);

  free(num_ch);
  free(font);
  free(fg);
  free(wght);
}

void DoubleToNumStrPango(char **dst, const double num,
                         const unsigned short digits_right)
/* Take in a string buffer, a double, and the number of digits
   to the right of the decimal point, convert to a number format
   Pango Markup string, grouping the digits according to the locale
   [dec points or commas].

   Reallocs memory to fit the output string.

   Take care that *dst is not an unallocated ptr address.
   Set *dst = NULL first.*/
{
  if (!dst)
    return;
  if (dst[0] == NULL)
    dst[0] = malloc(1);

  char *num_ch = NULL;
  /* Get the number string from the number */
  DoubleToNumStr(&num_ch, num, digits_right);

  /* Set the attributes */
  char *font = font_attr(font_name);
  char *fg = fg_attr("Black");
  char *wght = wght_attr("Medium");
  const char *format = "<span %s %s %s>%s</span>";

  /* Create the marked up string from the attributes and the number string */
  create_markup(dst, format, font, fg, wght, num_ch);

  free(num_ch);
  free(font);
  free(fg);
  free(wght);
}

static void double_to_mon_str_pango_color_ext(char **dst, const double num,
                                              const unsigned short digits_right,
                                              const unsigned int color)
/* Take in a string buffer, a double, and the number of digits
   to the right of the decimal point, convert to a monetary format
   Pango Markup string, grouping the digits according to the locale
   [dec points or commas].

   Reallocs memory to fit the output string.

   Take care that *dst is not an unallocated ptr address.
   Set *dst = NULL first. */
{
  if (!dst || digits_right > 3)
    return;
  if (dst[0] == NULL)
    dst[0] = malloc(1);

  char *num_ch = NULL;
  DoubleToMonStr(&num_ch, num, digits_right);

  char *font = font_attr(font_name);
  char *fg = NULL;
  char *wght = NULL;
  char *style = NULL;
  const char *format_reg = "<span %s %s %s>%s</span>";
  const char *format_itl = "<span %s %s %s %s>%s</span>";
  const char *format;

  /* Create the full string concatenating the end tag and the number
     to the start tag. */

  switch (color) {
  case NO_COLOR:
    fg = fg_attr("Black");
    wght = wght_attr("Medium");
    format = format_reg;
    create_markup(dst, format, font, fg, wght, num_ch);
    break;
  case BLACK:
    fg = fg_attr("Black");
    wght = wght_attr("Medium");
    format = format_reg;
    create_markup(dst, format, font, fg, wght, num_ch);
    break;
  case RED:
    fg = fg_attr("DarkRed");
    wght = wght_attr("Medium");
    format = format_reg;
    create_markup(dst, format, font, fg, wght, num_ch);
    break;
  case GREEN:
    fg = fg_attr("DarkGreen");
    wght = wght_attr("Medium");
    format = format_reg;
    create_markup(dst, format, font, fg, wght, num_ch);
    break;
  case BLUE:
    fg = fg_attr("MidnightBlue");
    wght = wght_attr("Demi-Bold");
    format = format_reg;
    create_markup(dst, format, font, fg, wght, num_ch);
    break;
  case BLACK_ITALIC:
    fg = fg_attr("Black");
    wght = wght_attr("Medium");
    style = style_attr("italic");
    format = format_itl;
    create_markup(dst, format, font, fg, wght, style, num_ch);
    break;
  case RED_ITALIC:
    fg = fg_attr("DarkRed");
    wght = wght_attr("Medium");
    style = style_attr("italic");
    format = format_itl;
    create_markup(dst, format, font, fg, wght, style, num_ch);
    break;
  default: /* GREEN_ITALIC */
    fg = fg_attr("DarkGreen");
    wght = wght_attr("Medium");
    style = style_attr("italic");
    format = format_itl;
    create_markup(dst, format, font, fg, wght, style, num_ch);
    break;
  }

  free(num_ch);
  if (font)
    free(font);
  if (fg)
    free(fg);
  if (wght)
    free(wght);
  if (style)
    free(style);
}

void DoubleToMonStrPangoColor(char **dst, const double num,
                              const unsigned short digits_right,
                              const unsigned int italic) {
  unsigned int color;

  switch (italic) {
  case NOT_ITALIC:
    if (num == 0) {
      color = BLACK;
    } else if (num > 0) {
      color = GREEN;
    } else {
      color = RED;
    }
    break;
  default: /* ITALIC */
    if (num == 0) {
      color = BLACK_ITALIC;
    } else if (num > 0) {
      color = GREEN_ITALIC;
    } else {
      color = RED_ITALIC;
    }
    break;
  }

  double_to_mon_str_pango_color_ext(dst, num, digits_right, color);
}

static void double_to_per_str_pango_color_ext(char **dst, const double num,
                                              const unsigned short digits_right,
                                              const unsigned int color)
/* Take in a string buffer, a double, and the number of digits
   to the right of the decimal point, convert to a percent format
   Pango Markup string, grouping the digits according to the locale
   [dec points or commas].

   Reallocs memory to fit the output string.

   Take care that *dst is not an unallocated ptr address.
   Set *dst = NULL first.*/
{
  if (!dst || digits_right > 3)
    return;
  if (dst[0] == NULL)
    dst[0] = malloc(1);

  char *num_ch = NULL;
  DoubleToPerStr(&num_ch, num, digits_right);

  char *font = font_attr(font_name);
  char *fg = NULL;
  char *wght = NULL;
  char *style = NULL;
  const char *format_reg = "<span %s %s %s>%s</span>";
  const char *format_itl = "<span %s %s %s %s>%s</span>";
  const char *format;

  switch (color) {
  case NO_COLOR:
    fg = fg_attr("Black");
    wght = wght_attr("Medium");
    format = format_reg;
    create_markup(dst, format, font, fg, wght, num_ch);
    break;
  case BLACK:
    fg = fg_attr("Black");
    wght = wght_attr("Medium");
    format = format_reg;
    create_markup(dst, format, font, fg, wght, num_ch);
    break;
  case RED:
    fg = fg_attr("DarkRed");
    wght = wght_attr("Medium");
    format = format_reg;
    create_markup(dst, format, font, fg, wght, num_ch);
    break;
  case GREEN:
    fg = fg_attr("DarkGreen");
    wght = wght_attr("Medium");
    format = format_reg;
    create_markup(dst, format, font, fg, wght, num_ch);
    break;
  case BLUE:
    fg = fg_attr("MidnightBlue");
    wght = wght_attr("Demi-Bold");
    format = format_reg;
    create_markup(dst, format, font, fg, wght, num_ch);
    break;
  case BLACK_ITALIC:
    fg = fg_attr("Black");
    wght = wght_attr("Medium");
    style = style_attr("italic");
    format = format_itl;
    create_markup(dst, format, font, fg, wght, style, num_ch);
    break;
  case RED_ITALIC:
    fg = fg_attr("DarkRed");
    wght = wght_attr("Medium");
    style = style_attr("italic");
    format = format_itl;
    create_markup(dst, format, font, fg, wght, style, num_ch);
    break;
  default: /* GREEN_ITALIC */
    fg = fg_attr("DarkGreen");
    wght = wght_attr("Medium");
    style = style_attr("italic");
    format = format_itl;
    create_markup(dst, format, font, fg, wght, style, num_ch);
    break;
  }

  free(num_ch);
  if (font)
    free(font);
  if (fg)
    free(fg);
  if (wght)
    free(wght);
  if (style)
    free(style);
}

void DoubleToPerStrPangoColor(char **dst, const double num,
                              const unsigned short digits_right,
                              const unsigned int italic) {
  unsigned int color;

  switch (italic) {
  case NOT_ITALIC:
    if (num == 0) {
      color = BLACK;
    } else if (num > 0) {
      color = GREEN;
    } else {
      color = RED;
    }
    break;
  default: /* ITALIC */
    if (num == 0) {
      color = BLACK_ITALIC;
    } else if (num > 0) {
      color = GREEN_ITALIC;
    } else {
      color = RED_ITALIC;
    }
    break;
  }

  double_to_per_str_pango_color_ext(dst, num, digits_right, color);
}

void StringToStrPangoColor(char **dst, const char *src,
                           const unsigned int color)
/* Take in a string buffer and a src string, convert to Pango Markup
   string.

   Reallocs memory to fit the output string.

   Take care that *dst is not an unallocated ptr address.
   Set *dst = NULL first.*/
{
  if (!dst || !src)
    return;
  if (dst[0] == NULL)
    dst[0] = malloc(1);

  char *font = font_attr(font_name);
  char *fg = NULL;
  char *wght = NULL;
  char *undln = NULL;
  char *undln_colr = NULL;
  char *style = NULL;
  const char *format_reg = "<span %s %s %s>%s</span>";
  const char *format_itl = "<span %s %s %s %s>%s</span>";
  const char *format_undln = "<span %s %s %s %s %s>%s</span>";
  const char *format;

  switch (color) {
  case NO_COLOR:
    fg = fg_attr("Black");
    wght = wght_attr("Medium");
    format = format_reg;
    create_markup(dst, format, font, fg, wght, src);
    break;
  case BLACK:
    fg = fg_attr("Black");
    wght = wght_attr("Medium");
    format = format_reg;
    create_markup(dst, format, font, fg, wght, src);
    break;
  case RED:
    fg = fg_attr("DarkRed");
    wght = wght_attr("Medium");
    format = format_reg;
    create_markup(dst, format, font, fg, wght, src);
    break;
  case GREEN:
    fg = fg_attr("DarkGreen");
    wght = wght_attr("Medium");
    format = format_reg;
    create_markup(dst, format, font, fg, wght, src);
    break;
  case BLUE:
    fg = fg_attr("MidnightBlue");
    wght = wght_attr("Demi-Bold");
    format = format_reg;
    create_markup(dst, format, font, fg, wght, src);
    break;
  case BLACK_ITALIC:
    fg = fg_attr("Black");
    wght = wght_attr("Medium");
    style = style_attr("italic");
    format = format_itl;
    create_markup(dst, format, font, fg, wght, style, src);
    break;
  case RED_ITALIC:
    fg = fg_attr("DarkRed");
    wght = wght_attr("Medium");
    style = style_attr("italic");
    format = format_itl;
    create_markup(dst, format, font, fg, wght, style, src);
    break;
  case GREEN_ITALIC:
    fg = fg_attr("DarkGreen");
    wght = wght_attr("Medium");
    style = style_attr("italic");
    format = format_itl;
    create_markup(dst, format, font, fg, wght, style, src);
    break;
  case BLUE_ITALIC:
    fg = fg_attr("MidnightBlue");
    wght = wght_attr("Demi-Bold");
    style = style_attr("italic");
    format = format_itl;
    create_markup(dst, format, font, fg, wght, style, src);
    break;
  case BOLD:
    fg = fg_attr("DarkSlateGray");
    wght = wght_attr("Demi-Bold");
    format = format_reg;
    create_markup(dst, format, font, fg, wght, src);
    break;
  default: /* BOLD_UNDERLINE */
    fg = fg_attr("SaddleBrown");
    wght = wght_attr("Demi-Bold");
    undln = undln_attr("single");
    undln_colr = undln__colr_attr("SaddleBrown");
    format = format_undln;
    create_markup(dst, format, font, fg, wght, undln, undln_colr, src);
    break;
  }

  if (font)
    free(font);
  if (fg)
    free(fg);
  if (wght)
    free(wght);
  if (undln)
    free(undln);
  if (undln_colr)
    free(undln_colr);
  if (style)
    free(style);
}