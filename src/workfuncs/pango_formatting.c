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

#include <stdlib.h>
#include <string.h>

#include <locale.h>
#include <monetary.h>

#include "../include/macros.h"
#include "../include/workfuncs.h"

const char *markup_start =
    "<span font_desc='Oxygen-Sans 11' foreground='Black' weight='Medium'>";
const char *markup_start_black =
    "<span font_desc='Oxygen-Sans 11' foreground='Black' weight='Medium'>";
const char *markup_start_red =
    "<span font_desc='Oxygen-Sans 11' foreground='DarkRed' weight='Medium'>";
const char *markup_start_green =
    "<span font_desc='Oxygen-Sans 11' foreground='DarkGreen' weight='Medium'>";
const char *markup_start_blue = "<span font_desc='Oxygen-Sans 11' "
                                "foreground='MidnightBlue' weight='Demi-Bold'>";
const char *markup_start_black_italic =
    "<span font_desc='Oxygen-Sans italic 11' foreground='Black' "
    "weight='Medium'>";
const char *markup_start_red_italic = "<span font_desc='Oxygen-Sans italic 11' "
                                      "foreground='IndianRed' weight='Medium'>";
const char *markup_start_green_italic =
    "<span font_desc='Oxygen-Sans italic 11' foreground='LimeGreen' "
    "weight='Medium'>";
const char *markup_start_blue_italic =
    "<span font_desc='Oxygen-Sans italic 11' foreground='MidnightBlue' "
    "weight='Demi-Bold'>";
const char *markup_start_bold =
    "<span font_desc='Oxygen-Sans 11' foreground='Black' weight='Bold'>";
const char *markup_start_bold_underline =
    "<span font_desc='Oxygen-Sans 11' foreground='Black' weight='Bold' "
    "underline='single'>";
const char *markup_end = "</span>";

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

  size_t len = LengthMonetary(num, digits_right) + 1;

  /* malloc a monetary number string */
  char *num_ch = malloc(len);

  /* The C.UTF-8 locale does not have a monetary
     format and is the default in C.
  */
  setlocale(LC_ALL, LOCALE);

  /* Set the string value. */
  switch (digits_right) {
  case 0:
    strfmon(num_ch, len, "%(.0n", num);
    break;
  case 1:
    strfmon(num_ch, len, "%(.1n", num);
    break;
  case 2:
    strfmon(num_ch, len, "%(.2n", num);
    break;
  default:
    strfmon(num_ch, len, "%(.3n", num);
    break;
  }

  len = strlen(markup_start) + strlen(num_ch) + strlen(markup_end) + 1;

  /* Adjust the memory allocation */
  char *tmp = realloc(dst[0], len);

  if (tmp == NULL) {
    printf("Not Enough Memory, realloc returned NULL.\n");
    exit(EXIT_FAILURE);
  }

  dst[0] = tmp;

  /* Pango markup the string. */
  snprintf(dst[0], len, "%s%s%s", markup_start, num_ch, markup_end);
  free(num_ch);
}

void DoubleToPerStrPango(char **dst, const double num,
                         const unsigned short digits_right)
/* Take in a string buffer, a double, and the number of digits
   to the right of the decimal point, convert to a percent format
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

  size_t len = LengthPercent(num, digits_right) + 1;
  len += strlen(markup_start) + strlen(markup_end);
  /* Adjust the memory allocation */
  char *tmp = realloc(dst[0], len);

  if (tmp == NULL) {
    printf("Not Enough Memory, realloc returned NULL.\n");
    exit(EXIT_FAILURE);
  }

  dst[0] = tmp;

  setlocale(LC_NUMERIC, LOCALE);
  switch (digits_right) {
  case 0:
    snprintf(dst[0], len, "%s%'.0lf%%%s", markup_start, num, markup_end);
    break;
  case 1:
    snprintf(dst[0], len, "%s%'.1lf%%%s", markup_start, num, markup_end);
    break;
  case 2:
    snprintf(dst[0], len, "%s%'.2lf%%%s", markup_start, num, markup_end);
    break;
  default:
    snprintf(dst[0], len, "%s%'.3lf%%%s", markup_start, num, markup_end);
    break;
  }
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

  size_t len = LengthNumber(num, digits_right) + 1;
  len += strlen(markup_start) + strlen(markup_end);

  /* Adjust the memory allocation */
  char *tmp = realloc(dst[0], len);

  if (tmp == NULL) {
    printf("Not Enough Memory, realloc returned NULL.\n");
    exit(EXIT_FAILURE);
  }

  dst[0] = tmp;

  setlocale(LC_NUMERIC, LOCALE);
  switch (digits_right) {
  case 0:
    snprintf(dst[0], len, "%s%'.0lf%s", markup_start, num, markup_end);
    break;
  case 1:
    snprintf(dst[0], len, "%s%'.1lf%s", markup_start, num, markup_end);
    break;
  case 2:
    snprintf(dst[0], len, "%s%'.2lf%s", markup_start, num, markup_end);
    break;
  case 3:
    snprintf(dst[0], len, "%s%'.3lf%s", markup_start, num, markup_end);
    break;
  default:
    snprintf(dst[0], len, "%s%'.4lf%s", markup_start, num, markup_end);
    break;
  }
}

void StringToStrPango(char **dst, const char *src)
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

  size_t len = strlen(src) + 1;
  len += strlen(markup_start) + strlen(markup_end);

  /* Adjust the string length */
  char *tmp = realloc(dst[0], len);

  if (tmp == NULL) {
    printf("Not Enough Memory, realloc returned NULL.\n");
    exit(EXIT_FAILURE);
  }

  dst[0] = tmp;

  snprintf(dst[0], len, "%s%s%s", markup_start, src, markup_end);
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

  size_t len = LengthMonetary(num, digits_right) + 1;

  /* malloc a monetary number string */
  char *num_ch = malloc(len);

  /* The C.UTF-8 locale does not have a monetary
     format and is the default in C.
  */
  setlocale(LC_ALL, LOCALE);

  /* Set the string value. */
  switch (digits_right) {
  case 0:
    strfmon(num_ch, len, "%(.0n", num);
    break;
  case 1:
    strfmon(num_ch, len, "%(.1n", num);
    break;
  case 2:
    strfmon(num_ch, len, "%(.2n", num);
    break;
  default:
    strfmon(num_ch, len, "%(.3n", num);
    break;
  }

  /* Create the full string concatenating the end tag and the number
     to the start tag. */
  const char *start_tag = NULL;

  switch (color) {
  case NO_COLOR:
    start_tag = markup_start;
    break;
  case BLACK:
    start_tag = markup_start_black;
    break;
  case RED:
    start_tag = markup_start_red;
    break;
  case GREEN:
    start_tag = markup_start_green;
    break;
  case BLUE:
    start_tag = markup_start_blue;
    break;
  case BLACK_ITALIC:
    start_tag = markup_start_black_italic;
    break;
  case RED_ITALIC:
    start_tag = markup_start_red_italic;
    break;
  default: /* GREEN_ITALIC */
    start_tag = markup_start_green_italic;
    break;
  }

  len = strlen(start_tag) + strlen(num_ch) + strlen(markup_end) + 1;

  /* Adjust the memory allocation */
  char *ptr = realloc(dst[0], len);

  if (ptr == NULL) {
    printf("Not Enough Memory, realloc returned NULL.\n");
    exit(EXIT_FAILURE);
  }

  dst[0] = ptr;

  snprintf(dst[0], len, "%s%s%s", start_tag, num_ch, markup_end);
  free(num_ch);
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

  const char *start_tag;

  switch (color) {
  case NO_COLOR:
    start_tag = markup_start;
    break;
  case BLACK:
    start_tag = markup_start_black;
    break;
  case RED:
    start_tag = markup_start_red;
    break;
  case GREEN:
    start_tag = markup_start_green;
    break;
  case BLUE:
    start_tag = markup_start_blue;
    break;
  case BLACK_ITALIC:
    start_tag = markup_start_black_italic;
    break;
  case RED_ITALIC:
    start_tag = markup_start_red_italic;
    break;
  default: /* GREEN_ITALIC */
    start_tag = markup_start_green_italic;
    break;
  }

  size_t len = strlen(start_tag) + LengthPercent(num, digits_right) +
               strlen(markup_end) + 1;
  /* Adjust the memory allocation */
  char *tmp = realloc(dst[0], len);

  if (tmp == NULL) {
    printf("Not Enough Memory, realloc returned NULL.\n");
    exit(EXIT_FAILURE);
  }

  dst[0] = tmp;

  setlocale(LC_NUMERIC, LOCALE);
  switch (digits_right) {
  case 0:
    snprintf(dst[0], len, "%s%'.0lf%%%s", start_tag, num, markup_end);
    break;
  case 1:
    snprintf(dst[0], len, "%s%'.1lf%%%s", start_tag, num, markup_end);
    break;
  case 2:
    snprintf(dst[0], len, "%s%'.2lf%%%s", start_tag, num, markup_end);
    break;
  default:
    snprintf(dst[0], len, "%s%'.3lf%%%s", start_tag, num, markup_end);
    break;
  }
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

  const char *start_tag;

  switch (color) {
  case NO_COLOR:
    start_tag = markup_start;
    break;
  case BLACK:
    start_tag = markup_start_black;
    break;
  case RED:
    start_tag = markup_start_red;
    break;
  case GREEN:
    start_tag = markup_start_green;
    break;
  case BLUE:
    start_tag = markup_start_blue;
    break;
  case BLACK_ITALIC:
    start_tag = markup_start_black_italic;
    break;
  case RED_ITALIC:
    start_tag = markup_start_red_italic;
    break;
  case GREEN_ITALIC:
    start_tag = markup_start_green_italic;
    break;
  case BLUE_ITALIC:
    start_tag = markup_start_blue_italic;
    break;
  case BOLD:
    start_tag = markup_start_bold;
    break;
  default: /* BOLD_UNDERLINE */
    start_tag = markup_start_bold_underline;
    break;
  }

  size_t len = strlen(src) + 1;
  len += strlen(start_tag) + strlen(markup_end);

  /* Adjust the memory allocation */
  char *tmp = realloc(dst[0], len);

  if (tmp == NULL) {
    printf("Not Enough Memory, realloc returned NULL.\n");
    exit(EXIT_FAILURE);
  }

  dst[0] = tmp;

  snprintf(dst[0], len, "%s%s%s", start_tag, src, markup_end);
}