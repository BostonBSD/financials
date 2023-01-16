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

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <ctype.h>
#include <stdbool.h>

#include <locale.h>
#include <monetary.h>

#include "../include/macros.h"
#include "../include/workfuncs.h"

bool CheckValidString(const char *string) {
  size_t len = strlen(string);

  /* The string cannot begin with these characters  */
  if (strchr(" _\0", (int)string[0]))
    return false;

  /* The string cannot end with these characters  */
  if (strchr(" _.", (int)string[len - 1]))
    return false;

  /* The string cannot contain these characters  */
  if (strpbrk(string, "\n\"\'\\)(][}{~`, "))
    return false;

  return true;
}

bool CheckIfStringDoubleNumber(const char *string) {
  char *end_ptr;
  strtod(string, &end_ptr);

  /* If no conversion took place or if conversion not complete. */
  if ((end_ptr == string) || (*end_ptr != '\0')) {
    return false;
  }

  return true;
}

bool CheckIfStringDoublePositiveNumber(const char *string) {
  char *end_ptr;
  double num = strtod(string, &end_ptr);

  /* If no conversion took place or if conversion not complete. */
  if ((end_ptr == string) || (*end_ptr != '\0'))
    return false;
  if (num < 0)
    return false;

  return true;
}

bool CheckIfStringLongPositiveNumber(const char *string) {
  char *end_ptr;
  long num = strtol(string, &end_ptr, 10);

  /* If no conversion took place or if conversion not complete. */
  if ((end_ptr == string) || (*end_ptr != '\0'))
    return false;
  if (num < 0)
    return false;

  return true;
}

void CopyString(char **dst, const char *src)
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
  if (dst[0] == NULL)
    dst[0] = malloc(1);

  size_t len = strlen(src) + 1;
  char *tmp = realloc(dst[0], len);

  if (tmp == NULL) {
    printf("Not Enough Memory, realloc returned NULL.\n");
    exit(EXIT_FAILURE);
  }

  dst[0] = tmp;
  snprintf(dst[0], len, "%s", src);
}

void ToNumStr(char *s)
/* Remove all dollar signs '$', commas ',', braces '(',
   percent signs '%', negative signs '-', and plus
   signs '+'  from a string. */

/* This assumes a en_US locale, other locales would
   need to edit this function or provide their own
   [other locales use commas and decimals differently,
   different currency symbol]. */
{
  /* Read character by character until the null character is reached. */
  for (unsigned int i = 0; s[i]; i++) {
    /* If s[i] is one of these characters */
    if (strchr("$,()%-+", (int)s[i])) {
      /* Read each character thereafter and */
      for (unsigned int j = i; s[j]; j++) {
        /* Shift the array to the left one character [remove the character] */
        s[j] = s[j + 1];
      }
      /* Check the new value of this increment [if there were a duplicate
         character]. */
      i--;
    }
  }
}

void LowerCaseStr(char *s) {
  /* Convert the string to lowercase letters */
  for (unsigned short i = 0; s[i]; i++) {
    s[i] = tolower(s[i]);
  }
}

void UpperCaseStr(char *s) {
  /* Convert the string to uppercase letters */
  for (unsigned short i = 0; s[i]; i++) {
    s[i] = toupper(s[i]);
  }
}

void Chomp(char *s)
/* Locate first newline character '\n' in a string, replace with NULL */
{
  char *ch = strchr(s, (int)'\n');
  if (ch != NULL)
    *ch = 0;
}

static size_t abs_val(const double n) {
  if (n < 0)
    return (size_t)floor((-1.0f * n));
  return (size_t)floor(n);
}

static size_t length_doub_string(const double n, const unsigned short dec_pts,
                                 const unsigned int type) {

  size_t number = abs_val(n);
  size_t a = 1, b = 1, len = 0, chars = 0;
  unsigned int neg_sign = 0, commas = 0;

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

double StringToDouble(const char *str)
/* Take in a number string, convert to a double value.
   The string can be formatted as a monetary string, a percent string,
   a number formatted string [thousands gouping], or a regular number
   string [no thousands grouping]. */
{
  char *newstr = strdup(str);

  ToNumStr(newstr);
  double num = strtod(newstr, NULL);

  free(newstr);

  return num;
}

void DoubleToFormattedStr(char **dst, const double num,
                          const unsigned short digits_right,
                          const unsigned int format_type)
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

  if (dst[0] == NULL)
    dst[0] = malloc(1);

  size_t len = length_doub_string(num, digits_right, format_type) + 1;
  /* Adjust the string length */
  char *tmp = realloc(dst[0], len);

  if (tmp == NULL) {
    printf("Not Enough Memory, realloc returned NULL.\n");
    exit(EXIT_FAILURE);
  }

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
    snprintf(dst[0], len, tmp, num);
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
    snprintf(dst[0], len, tmp, num);
    break;
  default:
    printf("DoubleToFormattedStr format_type out of range.\n");
    exit(EXIT_FAILURE);
    break;
  }
}

void StringToMonStr(char **dst, const char *src,
                    const unsigned short digits_right)
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

  double n = StringToDouble(src);
  DoubleToFormattedStr(dst, n, digits_right, MON_STR);
}