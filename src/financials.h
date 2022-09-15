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

#ifndef FINANCIALS_HEADER_H
#define FINANCIALS_HEADER_H

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

#include <pthread.h>
#include <semaphore.h>

#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#include <sys/stat.h>
#include <sys/types.h>

#include <dirent.h>
#include <errno.h>

#include <assert.h>
#include <pwd.h>
#include <unistd.h>

#include <locale.h>
#include <monetary.h>

#include <math.h>

#include "gui/gui.h"

#include "class/class.h"
#include "multicurl/multicurl.h"

#include "sqlite/sqlite.h"

/* SQLite config db file location (from the user's home directory). */
#ifndef CONFIG_DIR
#define CONFIG_DIR "/.config"
#endif

#ifndef CONFIG_FILE_DIR
#define CONFIG_FILE_DIR CONFIG_DIR "/financials"
#endif

#ifndef DB_FILE
#define DB_FILE CONFIG_FILE_DIR "/financials.db"
#endif

/* The number of system mutexes */
#ifndef MUTEX_NUMBER
#define MUTEX_NUMBER 6
#endif

/* The current locale */
#ifndef LOCALE
#define LOCALE "en_US.UTF-8"
#endif

/* Structures */

/* Globals */
extern GMutex mutex_interface;                        /* A GMutex */
extern pthread_mutex_t mutex_working[ MUTEX_NUMBER ]; /* A Posix Mutex Array */
extern sem_t semaphore[ SIGNAL_NUM ];                 /* A Posix Semaphore Array */

extern equity_folder *Folder; /* A handle to an array of stock class objects, */
                              /* can change dynamically. */
extern metal *Precious; /* A handle to the bullion class object pointers. */
extern meta *MetaData;  /* A class object pointer called MetaData. */

/* Declarations  */
void ReadConfig(metal*,meta*,equity_folder*);
void ParseFinancialOptions(int,char*[]);
void AddStock(equity_folder*);
int AlphaAsc(const void*,const void*);
int AlphaAscSecName(const void*, const void*);
void ResetEquity(equity_folder*);

void *JsonExtractEquity(char*,double*,double*,double*,double*,double*,double*,double*);

void PerformCalculations();
int MultiCurlProcessing();
void PopulateBullionPrice_Yahoo ();
void JSONProcessing();

bool check_valid_string(const char *);
bool check_if_string_double_number(const char *);
bool check_if_string_double_positive_number(const char*);
bool check_if_string_long_positive_number(const char *);
void FormatStr(char *);
char *StringToMonetary(const char *);
void LowerCaseStr(char *);
void UpperCaseStr(char *);
void LocalAndNYTime (int*,int*,int*,int*,int*,int*,int*,int*,int*,int*,int*,int*);
char* MonthNameStr (int);
char* WeekDayStr (int);
bool TimeToClose (int*,int*,int*);
unsigned int SecondsToOpen ();
struct tm NYTimeComponents();
char* WhichHoliday (struct tm);
bool IsHoliday (struct tm);
unsigned int ClockSleepSeconds ();
double calc_gain (double,double);
void summation(double,double*,double*);
void calc_avg (double,double*,double*);
double calc_rsi (double,double);
void chomp(char*);
char *rsi_indicator( double );

#endif /* FINANCIALS_HEADER_H */