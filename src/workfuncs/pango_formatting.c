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

#include <stdlib.h>
#include <string.h>

#include <locale.h>
#include <monetary.h>

#include "../include/macros.h"
#include "../include/workfuncs.h"

#define MARKUP_START_RED                                                       \
  "<span font_desc='Oxygen-Sans 11' foreground='DarkRed' weight='Medium'>"
#define MARKUP_START_GREEN                                                     \
  "<span font_desc='Oxygen-Sans 11' foreground='DarkGreen' weight='Medium'>"
#define MARKUP_START_RED_ITALIC                                                \
  "<span font_desc='Oxygen-Sans italic 11' foreground='IndianRed' "            \
  "weight='Medium'>"
#define MARKUP_START_GREEN_ITALIC                                              \
  "<span font_desc='Oxygen-Sans italic 11' foreground='LimeGreen' "            \
  "weight='Medium'>"
#define MARKUP_START_BLUE_ITALIC                                               \
  "<span font_desc='Oxygen-Sans italic 11' foreground='MidnightBlue' "         \
  "weight='Demi-Bold'>"
#define MARKUP_START_BLUE                                                      \
  "<span font_desc='Oxygen-Sans 11' foreground='MidnightBlue' "                \
  "weight='Demi-Bold'>"
#define MARKUP_START_BOLD                                                      \
  "<span font_desc='Oxygen-Sans 11' foreground='Black' weight='Bold'>"
#define MARKUP_START_BOLD_UNDERLINE                                            \
  "<span font_desc='Oxygen-Sans 11' foreground='Black' weight='Bold' "         \
  "underline='single'>"
#define MARKUP_START                                                           \
  "<span font_desc='Oxygen-Sans 11' foreground='Black' weight='Medium'>"
#define MARKUP_START_BLACK                                                     \
  "<span font_desc='Oxygen-Sans 11' foreground='Black' weight='Medium'>"
#define MARKUP_START_BLACK_ITAlIC                                              \
  "<span font_desc='Oxygen-Sans italic 11' foreground='Black' "                \
  "weight='Medium'>"
#define MARKUP_END "</span>"

void DoubToMonStrPango(char **dst, const double num,
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
  len = len + strlen(MARKUP_START) + strlen(MARKUP_END);
  /* Adjust the string length */
  char *tmp = realloc(dst[0], len);

  if (tmp == NULL) {
    printf("Not Enough Memory, realloc returned NULL.\n");
    exit(EXIT_FAILURE);
  }

  dst[0] = tmp;

  /* The C.UTF-8 locale does not have a monetary
     format and is the default in C.
  */
  setlocale(LC_ALL, LOCALE);

  /* Set the string value. */
  switch (digits_right) {
  case 0:
    strfmon(dst[0], len, MARKUP_START "%(.0n" MARKUP_END, num);
    break;
  case 1:
    strfmon(dst[0], len, MARKUP_START "%(.1n" MARKUP_END, num);
    break;
  case 2:
    strfmon(dst[0], len, MARKUP_START "%(.2n" MARKUP_END, num);
    break;
  default:
    strfmon(dst[0], len, MARKUP_START "%(.3n" MARKUP_END, num);
    break;
  }
}

void DoubToPerStrPango(char **dst, const double num,
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
  len = len + strlen(MARKUP_START) + strlen(MARKUP_END);
  /* Adjust the string length */
  char *tmp = realloc(dst[0], len);

  if (tmp == NULL) {
    printf("Not Enough Memory, realloc returned NULL.\n");
    exit(EXIT_FAILURE);
  }

  dst[0] = tmp;

  setlocale(LC_NUMERIC, LOCALE);
  switch (digits_right) {
  case 0:
    snprintf(dst[0], len, MARKUP_START "%'.0lf%%" MARKUP_END, num);
    break;
  case 1:
    snprintf(dst[0], len, MARKUP_START "%'.1lf%%" MARKUP_END, num);
    break;
  case 2:
    snprintf(dst[0], len, MARKUP_START "%'.2lf%%" MARKUP_END, num);
    break;
  default:
    snprintf(dst[0], len, MARKUP_START "%'.3lf%%" MARKUP_END, num);
    break;
  }
}

void DoubToNumStrPango(char **dst, const double num,
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
  len = len + strlen(MARKUP_START) + strlen(MARKUP_END);

  /* Adjust the string length */
  char *tmp = realloc(dst[0], len);

  if (tmp == NULL) {
    printf("Not Enough Memory, realloc returned NULL.\n");
    exit(EXIT_FAILURE);
  }

  dst[0] = tmp;

  setlocale(LC_NUMERIC, LOCALE);
  switch (digits_right) {
  case 0:
    snprintf(dst[0], len, MARKUP_START "%'.0lf" MARKUP_END, num);
    break;
  case 1:
    snprintf(dst[0], len, MARKUP_START "%'.1lf" MARKUP_END, num);
    break;
  case 2:
    snprintf(dst[0], len, MARKUP_START "%'.2lf" MARKUP_END, num);
    break;
  case 3:
    snprintf(dst[0], len, MARKUP_START "%'.3lf" MARKUP_END, num);
    break;
  default:
    snprintf(dst[0], len, MARKUP_START "%'.4lf" MARKUP_END, num);
    break;
  }
}

void StrToStrPango(char **dst, const char *src)
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
  len = len + strlen(MARKUP_START) + strlen(MARKUP_END);

  /* Adjust the string length */
  char *tmp = realloc(dst[0], len);

  if (tmp == NULL) {
    printf("Not Enough Memory, realloc returned NULL.\n");
    exit(EXIT_FAILURE);
  }

  dst[0] = tmp;

  snprintf(dst[0], len, MARKUP_START "%s" MARKUP_END, src);
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

  size_t len_monetary = LengthMonetary(num, digits_right);
  size_t len_end_tag = strlen(MARKUP_END);

  const char *start_tag;

  /* Create an end_tag string with the number and end tag */
  size_t len = len_monetary + len_end_tag + 1;
  char *end_tag = malloc(len);

  /* The C.UTF-8 locale does not have a monetary
     format and is the default in C.
  */
  setlocale(LC_ALL, LOCALE);

  /* Set the string value. */
  switch (digits_right) {
  case 0:
    strfmon(end_tag, len, "%(.0n" MARKUP_END, num);
    break;
  case 1:
    strfmon(end_tag, len, "%(.1n" MARKUP_END, num);
    break;
  case 2:
    strfmon(end_tag, len, "%(.2n" MARKUP_END, num);
    break;
  default:
    strfmon(end_tag, len, "%(.3n" MARKUP_END, num);
    break;
  }

  /* Create the full string concatenating the end tag
     to the start tag. */

  switch (color) {
  case NO_COLOR:
    start_tag = MARKUP_START;
    break;
  case BLACK:
    start_tag = MARKUP_START_BLACK;
    break;
  case RED:
    start_tag = MARKUP_START_RED;
    break;
  case GREEN:
    start_tag = MARKUP_START_GREEN;
    break;
  case BLUE:
    start_tag = MARKUP_START_BLUE;
    break;
  case BLACK_ITALIC:
    start_tag = MARKUP_START_BLACK_ITAlIC;
    break;
  case RED_ITALIC:
    start_tag = MARKUP_START_RED_ITALIC;
    break;
  default: /* GREEN_ITALIC */
    start_tag = MARKUP_START_GREEN_ITALIC;
    break;
  }

  len = strlen(start_tag) + strlen(end_tag) + 1;

  /* Adjust the destination string length */
  char *ptr = realloc(dst[0], len);

  if (ptr == NULL) {
    printf("Not Enough Memory, realloc returned NULL.\n");
    exit(EXIT_FAILURE);
  }

  dst[0] = ptr;

  snprintf(dst[0], len, "%s%s", start_tag, end_tag);
  free(end_tag);
}

void DoubToMonStrPangoColor(char **dst, const double num,
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
    start_tag = MARKUP_START;
    break;
  case BLACK:
    start_tag = MARKUP_START_BLACK;
    break;
  case RED:
    start_tag = MARKUP_START_RED;
    break;
  case GREEN:
    start_tag = MARKUP_START_GREEN;
    break;
  case BLUE:
    start_tag = MARKUP_START_BLUE;
    break;
  case BLACK_ITALIC:
    start_tag = MARKUP_START_BLACK_ITAlIC;
    break;
  case RED_ITALIC:
    start_tag = MARKUP_START_RED_ITALIC;
    break;
  default: /* GREEN_ITALIC */
    start_tag = MARKUP_START_GREEN_ITALIC;
    break;
  }

  size_t len = strlen(start_tag) + LengthPercent(num, digits_right) +
               strlen(MARKUP_END) + 1;
  /* Adjust the string length */
  char *tmp = realloc(dst[0], len);

  if (tmp == NULL) {
    printf("Not Enough Memory, realloc returned NULL.\n");
    exit(EXIT_FAILURE);
  }

  dst[0] = tmp;

  setlocale(LC_NUMERIC, LOCALE);
  switch (digits_right) {
  case 0:
    snprintf(dst[0], len, "%s%'.0lf%%" MARKUP_END, start_tag, num);
    break;
  case 1:
    snprintf(dst[0], len, "%s%'.1lf%%" MARKUP_END, start_tag, num);
    break;
  case 2:
    snprintf(dst[0], len, "%s%'.2lf%%" MARKUP_END, start_tag, num);
    break;
  default:
    snprintf(dst[0], len, "%s%'.3lf%%" MARKUP_END, start_tag, num);
    break;
  }
}

void DoubToPerStrPangoColor(char **dst, const double num,
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

void StrToStrPangoColor(char **dst, const char *src, const unsigned int color)
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
    start_tag = MARKUP_START;
    break;
  case BLACK:
    start_tag = MARKUP_START_BLACK;
    break;
  case RED:
    start_tag = MARKUP_START_RED;
    break;
  case GREEN:
    start_tag = MARKUP_START_GREEN;
    break;
  case BLUE:
    start_tag = MARKUP_START_BLUE;
    break;
  case BLACK_ITALIC:
    start_tag = MARKUP_START_BLACK_ITAlIC;
    break;
  case RED_ITALIC:
    start_tag = MARKUP_START_RED_ITALIC;
    break;
  case GREEN_ITALIC:
    start_tag = MARKUP_START_GREEN_ITALIC;
    break;
  case BLUE_ITALIC:
    start_tag = MARKUP_START_BLUE_ITALIC;
    break;
  case BOLD:
    start_tag = MARKUP_START_BOLD;
    break;
  default: /* BOLD_UNDERLINE */
    start_tag = MARKUP_START_BOLD_UNDERLINE;
    break;
  }

  size_t len = strlen(src) + 1;
  len = len + strlen(start_tag) + strlen(MARKUP_END);

  /* Adjust the string length */
  char *tmp = realloc(dst[0], len);

  if (tmp == NULL) {
    printf("Not Enough Memory, realloc returned NULL.\n");
    exit(EXIT_FAILURE);
  }

  dst[0] = tmp;

  snprintf(dst[0], len, "%s%s%s", start_tag, src, MARKUP_END);
}