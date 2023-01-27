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

#ifndef MACROS_HEADER_H
#define MACROS_HEADER_H

/* To suppress unused argument warnings. */
#ifndef UNUSED
#define UNUSED(x) (void)x;
#endif

/* SQLite config db file location (from the user's home directory). */
#ifndef CONFIG_DIR
#define CONFIG_DIR "/financials"
#endif

#ifndef DB_FILE
#define DB_FILE CONFIG_DIR "/financials.db"
#endif

#ifndef SN_DB_FILE
#define SN_DB_FILE CONFIG_DIR "/financials_symbols.db"
#endif

/* The main treeview font. */
#ifndef MAIN_FONT
#define MAIN_FONT "Cantarell 10"
#endif

/* The current locale */
#ifndef LOCALE
#define LOCALE "en_US.UTF-8"
#endif

/* The New York time zone string */
#ifndef NEW_YORK_TIME_ZONE
#define NEW_YORK_TIME_ZONE "America/New_York"
#endif

/* The Yahoo! URL Macros are used by the GetYahooUrl ()
 * function */
#ifndef YAHOO_URL_START
#define YAHOO_URL_START "https://query1.finance.yahoo.com/v7/finance/download/"
#endif

#ifndef YAHOO_URL_MIDDLE_ONE
#define YAHOO_URL_MIDDLE_ONE "?period1="
#endif

#ifndef YAHOO_URL_MIDDLE_TWO
#define YAHOO_URL_MIDDLE_TWO "&period2="
#endif

#ifndef YAHOO_URL_END
#define YAHOO_URL_END "&interval=1d&events=history&includeAdjustedClose=TRUE"
#endif

/* The symbol URL macros are used by the ClassInitMeta () [class_meta.c] and
 * api_callback () [sqlite.c] functions */
#ifndef NASDAQ_SYMBOL_URL
#define NASDAQ_SYMBOL_URL                                                      \
  "http://www.nasdaqtrader.com/dynamic/SymDir/nasdaqlisted.txt"
#endif

/* Includes other exchanges as well */
#ifndef NYSE_SYMBOL_URL
#define NYSE_SYMBOL_URL                                                        \
  "http://www.nasdaqtrader.com/dynamic/SymDir/otherlisted.txt"
#endif

/* These two macros define initial URL values before the user sets up the
   application. */
/* The finnhub.io stock quote URL */
#ifndef FINNHUB_URL
#define FINNHUB_URL "https://finnhub.io/api/v1/quote?symbol="
#endif

/* The finnhub.io URL account token */
#ifndef FINNHUB_URL_TOKEN
#define FINNHUB_URL_TOKEN "&token=<YOUR ACCOUNT KEY>"
#endif

#endif /* MACROS_HEADER_H */