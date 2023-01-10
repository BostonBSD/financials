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

#include <stdbool.h> /* bool */

#include <sys/time.h> /* struct tm  */

#include "class_types.h"     /* equity_folder, metal, meta, portfolio_packet */
#include "gui_types.h"       /* symbol_name_map */
#include "multicurl_types.h" /* MemType */

/* pango_formatting */
enum {
  NO_COLOR,
  BLACK,
  GREEN,
  RED,
  BLUE,
  HEADING_ASST_TYPE_FORMAT,
  HEADING_UNLN_FORMAT,
  GREEN_ITALIC,
  RED_ITALIC,
  BLUE_ITALIC,
  BLACK_ITALIC
};

enum {
  NOT_ITALIC,
  ITALIC,
};

void SetFont(const char *);
void DoubleToMonStrPango(char **, const double, const unsigned short);
void DoubleToPerStrPango(char **, const double, const unsigned short);
void DoubleToNumStrPango(char **, const double, const unsigned short);
void StringToStrPango(char **, const char *);
void DoubleToMonStrPangoColor(char **, const double, const unsigned short,
                              const unsigned int);
void DoubleToPerStrPangoColor(char **, const double, const unsigned short,
                              unsigned int);
void StringToStrPangoColor(char **, const char *, const unsigned int);

/* sn_map */
void AddSymbolToMap(const char *, const char *, symbol_name_map *);
symbol_name_map *SymNameFetch(portfolio_packet *);
symbol_name_map *SymNameFetchUpdate(portfolio_packet *, symbol_name_map *);
char *GetSecurityName(char *, const symbol_name_map *);
void SNMapDestruct(symbol_name_map *);

/* string_formatting */
bool CheckValidString(const char *);
bool CheckIfStringDoubleNumber(const char *);
bool CheckIfStringDoublePositiveNumber(const char *);
bool CheckIfStringLongPositiveNumber(const char *);
void LowerCaseStr(char *);
void UpperCaseStr(char *);
void Chomp(char *);
void CopyString(char **, const char *);
void ToNumStr(char *);
size_t LengthMonetary(const double, const unsigned short);
size_t LengthPercent(const double, const unsigned short);
size_t LengthNumber(const double, const unsigned short);
void StringToMonStr(char **, const char *, const unsigned short);
void DoubleToMonStr(char **, const double, const unsigned short);
void DoubleToPerStr(char **, const double, const unsigned short);
void DoubleToNumStr(char **, const double, const unsigned short);
double StringToDouble(const char *);

/* time_funcs */
void NYTime(int *, int *);
char *MonthNameStr(int);
char *WeekDayStr(int);
bool TimeToClose(int *, int *, int *);
unsigned int SecondsToOpen();
struct tm NYTimeComponents();
char *WhichHoliday(struct tm);
bool CheckHoliday(struct tm);
unsigned int ClockSleepSeconds();
unsigned int ClockSleepMicroSeconds();
bool MarketOpen();

/* working_functions */
MemType *FetchRSIData(const char *, portfolio_packet *);
double CalcGain(double, double);
void Summation(double, double *, double *);
void CalcAvg(double, double *, double *);
double CalcRsi(double, double);
const char *RsiIndicator(double);

#endif /* WORKFUNCS_HEADER_H */