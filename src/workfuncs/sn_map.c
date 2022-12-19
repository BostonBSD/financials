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

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/class_types.h"
#include "../include/gui_types.h"
#include "../include/macros.h"
#include "../include/multicurl.h"
#include "../include/sqlite.h"

static int sym_search_func(const void *a, const void *b) {
  /* Cast the void pointer to a char pointer. */
  char *aa = (char *)a;
  /* Cast the void pointer to a double struct pointer. */
  symbol_to_security_name_container **bb =
      (symbol_to_security_name_container **)b;

  return strcasecmp(aa, bb[0]->symbol);
}

char *GetSecurityName(const char *s, const symbol_name_map *sn_map)
/* Look for a Security Name using the Symbol as a key value.
   Must free return value. */
{
  symbol_to_security_name_container **item = NULL;
  if (sn_map == NULL)
    return NULL;

  /* The second parameter is the same type as the return value. A double pointer
   * to a struct. */
  /* It's basically searching through an array of pointer addresses, the compare
     function dereferences the pointer address to get the string we are
     comparing against. */
  /* The array must already be sorted for bsearch to work. */
  item = (symbol_to_security_name_container **)bsearch(
      s, &sn_map->sn_container_arr[0], sn_map->size,
      sizeof(symbol_to_security_name_container *), sym_search_func);

  if (item) {
    /* The item pointer is not freed. It points to an item in the
       sn_container_arr array and not a duplicate. */
    return strdup(item[0]->security_name);
  } else {
    return NULL;
  }
}

void SNMapDestruct(symbol_name_map *sn_map) {
  if (sn_map == NULL)
    return;

  if (sn_map->sn_container_arr) {
    for (int i = 0; i < sn_map->size; i++) {
      /* Free the string members */
      free(sn_map->sn_container_arr[i]->symbol);
      sn_map->sn_container_arr[i]->symbol = NULL;
      free(sn_map->sn_container_arr[i]->security_name);
      sn_map->sn_container_arr[i]->security_name = NULL;
      /* Free the memory that the address is pointing to */
      free(sn_map->sn_container_arr[i]);
      sn_map->sn_container_arr[i] = NULL;
    }
    /* Free the array of pointer addresses */
    free(sn_map->sn_container_arr);
    sn_map->sn_container_arr = NULL;
    sn_map->size = 0;
  }
}

static int alpha_asc_sec_name(const void *a, const void *b)
/* This is a callback function for stdlib sorting functions.
   It compares symbol_to_security_name_container in alphabetically
   ascending order, by the stock symbol data member.  Swapping only
   the pointer.
*/
{
  /* Cast the void pointer as a symbol_to_security_name_container double
   * pointer. */
  symbol_to_security_name_container **aa =
      (symbol_to_security_name_container **)a;
  symbol_to_security_name_container **bb =
      (symbol_to_security_name_container **)b;

  /* Compare the symbol data members ignoring for case. */
  return strcasecmp(aa[0]->symbol, bb[0]->symbol);
}

static bool check_symbol(const char *s)
/* Depository share symbols include dollars signs, which we don't want.
   The first line of the file includes the "Symbol" string, which we don't want.
   The last line of the file includes the "File Creation Time" string, which we
   don't want. Some symbols are test stocks, which we don't want.
*/
{
  if (strpbrk(s, "$"))
    return false;
  if (strstr(s, "Symbol"))
    return false;
  if (strstr(s, "File Creation Time"))
    return false;
  /* Test Stock Symbols */
  if (strstr(s, "TEST"))
    return false;
  if (strstr(s, "ZXYZ.A"))
    return false;
  if (strstr(s, "ZZT"))
    return false;
  if (strstr(s, "ZEXIT"))
    return false;
  if (strstr(s, "ZIEXT"))
    return false;
  if (strstr(s, "ZVZZC"))
    return false;
  if (strstr(s, "ZVV"))
    return false;
  if (strstr(s, "ZXIET"))
    return false;
  if (strstr(s, "ZBZX"))
    return false;

  return true;
}

void AddSymbolToMap(const char *symbol, const char *name,
                    symbol_name_map *sn_map) {
  /* Increase the array to hold another pointer address */
  symbol_to_security_name_container **tmp =
      realloc(sn_map->sn_container_arr,
              sizeof(symbol_to_security_name_container *) * (sn_map->size + 1));
  sn_map->sn_container_arr = tmp;
  /* Allocate memory for a pointer address and add it to the array */
  sn_map->sn_container_arr[sn_map->size] =
      malloc(sizeof(symbol_to_security_name_container));
  /* Populate a data member with the symbol string */
  sn_map->sn_container_arr[sn_map->size]->symbol =
      strdup(symbol ? symbol : "EOF Incomplete Symbol List");
  /* Populate a data member with the name string and increment the size */
  sn_map->sn_container_arr[sn_map->size++]->security_name =
      strdup(name ? name : "EOF Incomplete Symbol List");
}

static symbol_name_map *sym_name_map_dup(symbol_name_map *sn_map)
/* Create a duplicate symbol_name_map. */
{
  symbol_name_map *sn_map_dup = (symbol_name_map *)malloc(sizeof(*sn_map_dup));
  sn_map_dup->sn_container_arr = malloc(1);
  sn_map_dup->size = 0;

  for (int g = 0; g < sn_map->size; g++) {
    AddSymbolToMap(sn_map->sn_container_arr[g]->symbol,
                   sn_map->sn_container_arr[g]->security_name, sn_map_dup);
  }

  return sn_map_dup;
}

/* Special Symbols Array */
static symbol_to_security_name_container special_syms[] = {
    /* Indices. */
    {"^DJI", "Dow Jones Industrial Average ( Dow 30 Index )"},
    {"^IXIC", "Nasdaq Composite Index"},
    {"^GSPC", "S&P 500 Index"},
    {"^RUT", "Russell 2000 Index"},
    {"^N225", "Nikkei 225 Index"},
    {"^N100", "Euronext 100 Index"},
    {"^HSI", "Hang Seng Index"},
    {"^NYA", "NYSE Composite (DJ) Index"},
    {"^XAX", "NYSE - AMEX Composite Index"},
    {"IMOEX.ME", "MOEX Russia Index"},
    {"^JKSE", "Jakarta Composite Index"},
    {"399001.SZ", "Shenzhen Index"},
    {"^STI", "STI Index"},
    {"^BUK100P", "Cboe UK 100 Index"},
    {"^MXX", "IPC MEXICO Index"},
    {"^KS11", "KOSPI Composite Index"},
    {"^KLSE", "FTSE Bursa Malaysia KLCI Index"},
    {"^FTSE", "FTSE 100 Index"},
    {"^GDAXI", "DAX Performance Index"},

    /* Commodities. */
    {"GC=F", "Gold Futures"},
    {"SI=F", "Silver Futures"},
    {"PL=F", "Platinum Futures"},
    {"PA=F", "Palladium Futures"},
    {"HG=F", "Copper Futures"},
    {"CL=F", "Crude Oil Futures"},
    {"BZ=F", "Crude Oil ( Brent ) Futures"},
    {"NG=F", "Natural Gas Futures"},
    {"ZC=F", "Corn Futures"},
    {"ZO=F", "Oat Futures"},
    {"KC=F", "Coffee Futures"},
    {"CT=F", "Cotton Futures"},
    {"CC=F", "Cocoa Futures"},
    {"SB=F", "Sugar Futures"},
    {"ZS=F", "Soybean Futures"},
    {"ZL=F", "Soybean Oil Futures"},
    {"HO=F", "Heating Oil Futures"},
    {"OJ=F", "Orange Juice Futures"},
    {"GNF=F", "Milk ( Nonfat Dry ) Futures"},
    {"ZW=F", "Wheat ( Chicago SRW ) Futures"},
    {"KE=F", "Wheat ( KC HRW ) Futures"},
    {"ZWT=F", "Wheat ( Chicago SRW ) TAS Futures"},
    {"LBR=F", "Lumber Futures"},
    {"LBS=F", "Lumber ( Random Length ) Futures"},

    /* Bond Futures */
    {"ZB=F", "Treasury Bond ( U.S. ) Futures"},
    {"ZN=F", "Treasury Note ( 10-Year U.S. ) Futures"},
    {"ZF=F", "Treasury Note ( 5-Year U.S. ) Futures"},
    {"ZT=F", "Treasury Note ( 2-Year U.S. ) Futures"},

    /* Bond Rates */
    {"^IRX", "Treasury Bill ( 13 Weeks )"},
    {"^FVX", "Treasury Bond Yield ( 5 Years )"},
    {"^TNX", "Treasury Bond Yield ( 10 Years )"},
    {"^TYX", "Treasury Bond Yield ( 30 Years )"},

    /* Crypto Coins/Tokens/Stablecoins/Etcetera */
    {"BTC-USD", "Bitcoin ( US Dollars )"},
    {"BCH-USD", "Bitcoin Cash ( US Dollars )"},
    {"ETH-USD", "Ethereum ( US Dollars )"},
    {"LTC-USD", "Litecoin ( US Dollars )"},
    {"XRP-USD", "XRP Token ( US Dollars )"},
    {"USDT-USD", "Tether Stablecoin ( US Dollars )"},
    {"BUSD-USD", "Binance Stablecoin ( US Dollars )"},
    {"BNB-USD", "Binance Coin ( US Dollars )"},
    {"DASH-USD", "Dash Digital Cash ( US Dollars )"},
    {"DOGE-USD", "Dogecoin ( US Dollars )"},
    {"ADA-USD", "Cardano Token ( US Dollars )"},
    {"MATIC-USD", "Polygon Coin ( US Dollars )"}};

static void add_special_symbols(symbol_name_map *sn_map) {
  int size = (sizeof special_syms) / (sizeof special_syms[0]);

  for (int i = 0; i < size; i++) {
    AddSymbolToMap(special_syms[i].symbol, special_syms[i].security_name,
                   sn_map);
  }
}

static symbol_name_map *symbol_list_fetch(portfolio_packet *pkg) {
  meta *D = pkg->GetMetaClass();

  char *line = malloc(1024), *line_start;
  char *symbol_token, *name_token;
  line_start = line;

  symbol_name_map *sn_map = (symbol_name_map *)malloc(sizeof(*sn_map));
  sn_map->sn_container_arr = malloc(1);
  sn_map->size = 0;

  MemType Nasdaq_Struct, NYSE_Struct;
  Nasdaq_Struct.memory = NULL;
  NYSE_Struct.memory = NULL;

  SetUpCurlHandle(D->NASDAQ_completion_hnd, D->multicurl_cmpltn_hnd,
                  D->Nasdaq_Symbol_url_ch, &Nasdaq_Struct);
  SetUpCurlHandle(D->NYSE_completion_hnd, D->multicurl_cmpltn_hnd,
                  D->NYSE_Symbol_url_ch, &NYSE_Struct);
  if (PerformMultiCurl_no_prog(D->multicurl_cmpltn_hnd) != 0 ||
      pkg->IsCurlCanceled()) {
    if (Nasdaq_Struct.memory)
      free(Nasdaq_Struct.memory);
    if (NYSE_Struct.memory)
      free(NYSE_Struct.memory);
    if (sn_map->sn_container_arr)
      free(sn_map->sn_container_arr);
    if (sn_map)
      free(sn_map);
    if (line)
      free(line);
    return NULL;
  }

  /* Convert a String to a File Pointer Stream for Reading */
  FILE *fp[2];
  fp[0] = fmemopen((void *)Nasdaq_Struct.memory,
                   strlen(Nasdaq_Struct.memory) + 1, "r");
  fp[1] =
      fmemopen((void *)NYSE_Struct.memory, strlen(NYSE_Struct.memory) + 1, "r");

  short k = 0;
  while (k < 2) {
    /* Read the file stream one line at a time */
    /* Populate the Symbol-Name Array. The second list is added after the first
     * list. */
    while (fgets(line, 1024, fp[k]) != NULL) {

      /* Extract the symbol from the line. */
      symbol_token = strsep(&line, "|");
      if (check_symbol(symbol_token) == false) {
        line = line_start;
        continue;
      }

      /* Extract the security name from the line. */
      name_token = strsep(&line, "|");

      /* Add the symbol and the name to the symbol-name map. */
      if (symbol_token && name_token)
        AddSymbolToMap(symbol_token, name_token, sn_map);

      /* strsep moves the line pointer, we need to reset it so the pointer
       * memory can be reused */
      line = line_start;

      /* If we are exiting the application, return immediately. */
      if (pkg->IsCurlCanceled()) {
        if (fp[0])
          fclose(fp[0]);
        if (fp[1])
          fclose(fp[1]);
        if (Nasdaq_Struct.memory)
          free(Nasdaq_Struct.memory);
        if (NYSE_Struct.memory)
          free(NYSE_Struct.memory);
        if (line)
          free(line);
        return sn_map;
      }
    }
    k++;

  } /* end while loop */
  free(line);

  /* Add special symbols such as indices, commodities, bonds, and crypto to the
   * map. */
  add_special_symbols(sn_map);

  /* Sort the security symbol array, this reorders both lists into one sorted
   * list. */
  qsort(&sn_map->sn_container_arr[0], sn_map->size,
        sizeof(symbol_to_security_name_container *), alpha_asc_sec_name);

  fclose(fp[0]);
  fclose(fp[1]);
  free(Nasdaq_Struct.memory);
  free(NYSE_Struct.memory);
  return sn_map;
}

symbol_name_map *SymNameFetch(portfolio_packet *pkg)
/* This function is only meant to be run once, at application startup.
   Populate the symbol-name map from the local Db.
   Does not initiate a remote server download.
*/
{
  meta *D = pkg->GetMetaClass();

  /* Check the database */
  symbol_name_map *sn_map = SqliteGetSymbolNameMap(D);

  if (sn_map) {
    /* Sort the sn_map [it should already be sorted from the Db, but just to
     * make sure]. */
    qsort(&sn_map->sn_container_arr[0], sn_map->size,
          sizeof(symbol_to_security_name_container *), alpha_asc_sec_name);
  }

  return sn_map;
}

symbol_name_map *SymNameFetchUpdate(portfolio_packet *pkg,
                                    symbol_name_map *sn_map)
/* Initates a remote server download; downloads NYSE and NASDAQ stock symbols
   and names. Update the symbol to name mapping in the database. If a new
   symbol-name map can be populated this function will free the current
   symbol_name_map, otherwise no change performed.
*/
{
  meta *D = pkg->GetMetaClass();
  symbol_name_map *sn_map_dup = NULL;

  /* Download the symbol lists from the server */
  symbol_name_map *sn_map_new = symbol_list_fetch(pkg);

  if (sn_map_new) {
    /* Free the current symbol to security name mapping array. */
    if (sn_map) {
      SNMapDestruct(sn_map);
      free(sn_map);
    }

    /* Create a duplicate symbol-name map */
    sn_map_dup = sym_name_map_dup(sn_map_new);

    /* Add the symbol mapping to the db, sn_map_dup is freed in the sqlite
     * thread. */
    SqliteAddMap(sn_map_dup, D);

    /* Return the new symbol-name mapping */
    return sn_map_new;
  } else {

    /* Return the current symbol-name mapping, unchanged */
    return sn_map;
  }
}