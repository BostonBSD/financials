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

#include <locale.h>
#include <math.h>
#include <monetary.h>

#include "../include/macros.h"
#include "../include/workfuncs.h"

gboolean CheckValidString(const gchar *string) {
  gsize len = g_utf8_strlen(string, -1);

  /* The string cannot begin with these characters  */
  if (g_utf8_strchr(" _\0", -1, (gunichar)string[0]))
    return FALSE;

  /* The string cannot end with these characters  */
  if (g_utf8_strchr(" _.", -1, (gunichar)string[len - 1]))
    return FALSE;

  /* The string cannot contain these characters  */
  gint g = 0;
  while (string[g])
    if (g_utf8_strchr("\n\"\'\\)(][}{~`, ", -1, (gunichar)string[g++]))
      return FALSE;

  return TRUE;
}

gboolean CheckIfStringDoubleNumber(const gchar *string) {
  gchar *end_ptr;
  g_strtod(string, &end_ptr);

  /* If no conversion took place or if conversion not complete. */
  if ((end_ptr == string) || (*end_ptr != '\0'))
    return FALSE;

  return TRUE;
}

gboolean CheckIfStringDoublePositiveNumber(const gchar *string) {
  gchar *end_ptr;
  gdouble num = g_strtod(string, &end_ptr);

  /* If no conversion took place or if conversion not complete. */
  if ((end_ptr == string) || (*end_ptr != '\0'))
    return FALSE;
  if (num < 0)
    return FALSE;

  return TRUE;
}

gboolean CheckIfStringLongPositiveNumber(const gchar *string) {
  gchar *end_ptr;
  gint64 num = g_ascii_strtoll(string, &end_ptr, 10);

  /* If no conversion took place or if conversion not complete. */
  if ((end_ptr == string) || (*end_ptr != '\0'))
    return FALSE;
  if (num < 0)
    return FALSE;

  return TRUE;
}

void CopyString(gchar **dst, const gchar *src)
/* Take in a string buffer, resize it to fit the src
   Copy the src to the *dst.  If either src or dst is NULL
   do nothing. If *dst = NULL, allocate memory for the buffer.

   Reallocs memory to fit the src string.

   Take care that *dst is not an unallocated address.
   Set *dst = NULL first or send an allocated address.
   */
{
  if (!dst || !src)
    return;

  gsize len = g_utf8_strlen(src, -1) + 1;
  gchar *tmp = g_realloc(dst[0], len);
  dst[0] = tmp;

  g_snprintf(dst[0], len, "%s", src);
}

void ToNumStr(gchar *s)
/* Remove all dollar signs '$', commas ',', braces '(' ')',
   percent signs '%', and plus signs '+'  from a string. */

/* This assumes a en_US locale, other locales would
   need to edit this function or provide their own
   [other locales use commas and decimals differently,
   different currency symbol]. */
{
  /* If we find a starting brace, replace with a negative sign. */
  gchar *ch = g_utf8_strchr(s, -1, (gunichar)'(');
  if (ch)
    *ch = '-';

  /* Read character by character until the null character is reached. */
  for (guint i = 0; s[i]; i++) {

    /* If s[i] is one of these characters */
    if (g_utf8_strchr("$,%+)", -1, (gunichar)s[i])) {

      /* Read each character thereafter and */
      for (guint j = i; s[j]; j++)
        /* Shift the array to the left one character [remove the character] */
        s[j] = s[j + 1];

      /* Check the new value of this increment [if there were a duplicate
         character]. */
      i--;
    }
  }
}

gdouble StringToDouble(const gchar *str)
/* Take in a number string, convert to a double value.
   The string can be formatted as a monetary string, a percent string,
   a number formatted string [thousands gouping], or a regular number
   string [no thousands grouping]. */
{
  gchar *newstr = g_strdup(str);

  ToNumStr(newstr);
  gdouble num = g_strtod(newstr, NULL);
  g_free(newstr);

  return num;
}

static gsize abs_val(const gdouble n) {
  /* If n is 9.9999 we want it to round up, to account for the strfmon and
   * snprintf rounding at zero, one, two, and three decimal places. */
  if (n < 0)
    return (gsize)ceil(-1.0f * n);
  return (gsize)ceil(n);
}

static gsize length_doub_string(const gdouble n, const guint8 dec_pts,
                                const guint type) {

  gsize number = abs_val(n);
  gsize a = 1, b = 1, len = 0, chars = 0;
  guint8 neg_sign = 0, commas = 0;

  do {
    chars++;
    if (a % (1000 * b) == 0) {
      b *= 1000;
      commas++;
    }
    a *= 10;
  } while (number >= a);

  switch (type) {
  case MON_STR:
    /* The currency symbol and
       the negative brackets. */
    len++;
    if (n < 0)
      neg_sign += 2;
    break;
  case PER_STR:
    /* The percent sign and
       the negative sign. */
    len++;
    if (n < 0)
      neg_sign++;
    break;
  default: /* NUM_STR */
    /* The negative sign. */
    if (n < 0)
      neg_sign++;
    break;
  };

  /* The len value includes space for thousands
     grouping [usually commas] and a negative sign [if needed]. */
  len += neg_sign + commas + chars;

  /* Add one for the decimal point char [usually a point]. */
  if (dec_pts != 0)
    len += (dec_pts + 1);

  /* The string length not including the null character. */
  return len;
}

void DoubleToFormattedStr(gchar **dst, const gdouble num,
                          const guint8 digits_right, const guint format_type)
/* Take in a string buffer, a double, a precision variable, and a format type
   convert to a formatted string [monetary, percent, or number].

   The type macros are; MON_STR, PER_STR, NUM_STR.

   If *dst = NULL, will allocate memory.
   If dst = NULL or precision is > 4
   do nothing.

   Reallocs memory to fit the string.

   Take care that *dst is not an unallocated ptr address.
   Set *dst = NULL first.
*/

{

  if (!dst || digits_right > 4)
    return;

  gsize len = length_doub_string(num, digits_right, format_type) + 1;
  /* Adjust the string length */
  gchar *tmp = g_realloc(dst[0], len);
  dst[0] = tmp;

  switch (format_type) {
  case MON_STR:
    switch (digits_right) {
    case 0:
      tmp = "%(.0n";
      break;
    case 1:
      tmp = "%(.1n";
      break;
    case 2:
      tmp = "%(.2n";
      break;
    case 3:
      tmp = "%(.3n";
      break;
    default:
      tmp = "%(.4n";
      break;
    }
    setlocale(LC_ALL, LOCALE);
    strfmon(dst[0], len, tmp, num);
    break;
  case PER_STR:
    switch (digits_right) {
    case 0:
      tmp = "%'.0lf%%";
      break;
    case 1:
      tmp = "%'.1lf%%";
      break;
    case 2:
      tmp = "%'.2lf%%";
      break;
    case 3:
      tmp = "%'.3lf%%";
      break;
    default:
      tmp = "%'.4lf%%";
      break;
    }
    setlocale(LC_NUMERIC, LOCALE);
    g_snprintf(dst[0], len, tmp, num);
    break;
  case NUM_STR:
    switch (digits_right) {
    case 0:
      tmp = "%'.0lf";
      break;
    case 1:
      tmp = "%'.1lf";
      break;
    case 2:
      tmp = "%'.2lf";
      break;
    case 3:
      tmp = "%'.3lf";
      break;
    default:
      tmp = "%'.4lf";
      break;
    }
    setlocale(LC_NUMERIC, LOCALE);
    g_snprintf(dst[0], len, tmp, num);
    break;
  default:
    g_print("DoubleToFormattedStr format_type out of range.\n");
    exit(EXIT_FAILURE);
    break;
  }
}

void StringToMonStr(gchar **dst, const gchar *src, const guint8 digits_right)
/* Take in a string buffer, a number string, and the precision,
   Convert the number string, src, to a monetary string, dst[0].
   If the src string cannot be converted to a double, undefined behavior.

   If *dst = NULL, will allocate memory.
   If dst = NULL, src = NULL, or precision is > 4
   do nothing.

   Reallocs memory to fit the monetary string.

   Take care that *dst is not an unallocated ptr address.
   Set *dst = NULL first.
   */
{
  if (!dst || !src)
    return;

  gdouble n = StringToDouble(src);
  DoubleToFormattedStr(dst, n, digits_right, MON_STR);
}

static gchar *snprint_helper(const gchar *fmt, va_list arg_ptr)
/* Return a newly allocated string from a format string and a va_list of
   arguments. Uses the same formatting as printf. Must free return value. */
{
  va_list arg_ptr_cpy;
  va_copy(arg_ptr_cpy, arg_ptr);

  /* Get the size of the potential string. */
  gsize len = g_vsnprintf(NULL, 0, fmt, arg_ptr_cpy) + 1;
  va_end(arg_ptr_cpy);

  /* Malloc the string. */
  gchar *str = g_malloc(len);

  /* Create the string from format and argument list. */
  g_vsnprintf(str, len, fmt, arg_ptr);
  return str;
}

gchar *SnPrint(const gchar *fmt, ...)
/* Return a newly allocated string from a format string and a list of
   arguments. Uses the same formatting as printf. Must free return value. */
{
  va_list arg_ptr;

  /* Set the arg pointer. */
  va_start(arg_ptr, fmt);
  gchar *str = snprint_helper(fmt, arg_ptr);
  va_end(arg_ptr);

  return str;
}

gchar *PangoToCssFontStr(const gchar *pango_fnt_str) {
  /* This is kind of hacky, but there doesn't appear to be a canonical way of
     converting a Pango font string to a CSS font string.

     [It was the position of Gnome to use CSS at startup/compile-time,
     and to use Pango at runtime, however, we are using CSS theming at runtime
     due to efficiency reasons; label font attributes can be altered more
     efficiently at runtime with CSS.  Treeview fonts are more efficiently
     altered with Pango at runtime.]

     Take in a Pango formatted font string, return a CSS formatted font
     string.

     "Arial Regular 8" -> "8px Arial Regular"

     Must free return value.
  */
  const gchar *fmt = "%spx %s";

  gchar *fnt_name_str = g_strdup(pango_fnt_str);

  /* Set the last space to NULL [truncate the size portion] */
  gchar *ch = g_utf8_strrchr(fnt_name_str, -1, (gunichar)' ');
  if (ch)
    *ch = 0;

  /* Duplicate the size portion into a separate string. */
  gchar *fnt_size_str = g_strdup(++ch);

  /* If there are any commas, set those to NULL [Pango sometimes uses a comma
   * and a space to separate the fontname from the size] */
  ch = g_utf8_strrchr(fnt_name_str, -1, (gunichar)',');
  if (ch)
    *ch = 0;

  /* Create a new CSS style font string from the size and fontname. */
  gchar *css_fnt_str = SnPrint(fmt, fnt_size_str, fnt_name_str);

  g_free(fnt_name_str);
  g_free(fnt_size_str);
  return css_fnt_str;
}

void ClearStr(gchar **str) {
  gchar *tmp = g_realloc(str[0], g_utf8_strlen("", -1) + 1);
  str[0] = tmp;
  g_snprintf(str[0], 1, "");
}