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

#include "../include/class_types.h"
#include "../include/gui_types.h"
#include "../include/macros.h"
#include "../include/multicurl.h"
#include "../include/sqlite.h"
#include "../include/workfuncs.h"

static gint alpha_asc_sec_name(gconstpointer a, gconstpointer b, gpointer data)
/* This is a callback function for Glib sorting functions.
   It compares symbol_to_security_name_container in alphabetically
   ascending order, by the stock symbol data member.  Swapping only
   the pointer.
*/
{
  UNUSED(data)

  /* Cast the void pointer as a symbol_to_security_name_container double
   * pointer. */
  symbol_to_security_name_container **aa =
      (symbol_to_security_name_container **)a;
  symbol_to_security_name_container **bb =
      (symbol_to_security_name_container **)b;

  /* Compare the symbol data members. */
  return g_strcmp0(aa[0]->symbol, bb[0]->symbol);
}

static gchar *hash_table_lookup(gchar *s, const symbol_name_map *sn_map) {
  if (sn_map == NULL)
    return NULL;

  gchar *ret_val = NULL;
  symbol_to_security_name_container *item = NULL;
  item = (symbol_to_security_name_container *)g_hash_table_lookup(
      sn_map->hash_table, s);

  /* The item pointer is not freed. It points to an item in the
       sn_container_arr array and not a duplicate. */
  if (item)
    ret_val = g_strdup(item->security_name);

  return ret_val;
}

gchar *GetSecurityName(gchar *s, const symbol_name_map *sn_map, meta *D)
/* Look for a Security Name using the Symbol as a key value.
   Must free return value. */
{
  /* If we are currently adding a mapping to the db */
  if (D->snmap_db_busy_bool)
    /* Look for a name in the sn_map, in memory. */
    return hash_table_lookup(s, sn_map);
  else
    /* Otherwise, look for a name in the db, from hard drive. */
    return SqliteGetSNMapName(s, D);
}

void CreateHashTable(symbol_name_map *sn_map) {
  /* Create a hashing table of the sn_map->sn_container_arr elements */
  sn_map->hash_table = g_hash_table_new(g_str_hash, g_str_equal);

  gushort s = 0;
  while (s < sn_map->size) {
    /* The symbol is the key value. */
    g_hash_table_insert(sn_map->hash_table, sn_map->sn_container_arr[s]->symbol,
                        sn_map->sn_container_arr[s]);
    s++;
  }
}

/* Symbols we don't want. */
static const gchar *bad_syms[] = {"Symbol", "File Creation Time",
                                  "TEST",   "ZXYZ.A",
                                  "ZZT",    "ZEXIT",
                                  "ZIEXT",  "ZVZZC",
                                  "ZVV",    "ZXIET",
                                  "ZBZX"};

static gboolean check_symbol(const gchar *s)
/* Depository share symbols include dollar signs, which we don't want [Yahoo!
   doesn't recognize symbols with $ signs]. The first line of the file includes
   the "Symbol" string, which we don't want. The last line of the file includes
   the "File Creation Time" string, which we don't want. Some symbols are test
   stocks, which we don't want.
*/
{
  if (g_strrstr(s, "$"))
    return FALSE;

  gushort g = 0, size = sizeof bad_syms / sizeof bad_syms[0];

  while (g < size)
    if (g_strrstr(s, bad_syms[g++]))
      return FALSE;

  return TRUE;
}

static void sub_symbol(gchar **symbol_ch, const gchar *suffix_ch) {
  gchar *tmp = NULL;

  /* Locate the decimal point and replace with a null. */
  tmp = g_utf8_strchr(symbol_ch[0], -1, (gunichar)'.');
  *tmp = 0;

  /* Duplcate the string without the previous suffix. */
  tmp = g_strdup(symbol_ch[0]);
  g_free(symbol_ch[0]);

  /* Concatenate the new suffix to the string. */
  symbol_ch[0] = g_strconcat(tmp, suffix_ch, NULL);
  g_free(tmp);
}

static void substitute_warrant_and_unit_symbols(gchar **symbol_ch) {
  if (!symbol_ch[0])
    return;

  /* Warrants */
  if (g_strrstr(symbol_ch[0], ".W"))
    sub_symbol(symbol_ch, "-WT");

  /* Units */
  else if (g_strrstr(symbol_ch[0], ".U"))
    sub_symbol(symbol_ch, "-UN");

  /* .A, .B, .C, .R, and .V shares */
  else if (g_strrstr(symbol_ch[0], ".A"))
    sub_symbol(symbol_ch, "-A");

  else if (g_strrstr(symbol_ch[0], ".B"))
    sub_symbol(symbol_ch, "-B");

  else if (g_strrstr(symbol_ch[0], ".C"))
    sub_symbol(symbol_ch, "-C");

  else if (g_strrstr(symbol_ch[0], ".R"))
    sub_symbol(symbol_ch, "-R");

  else if (g_strrstr(symbol_ch[0], ".V"))
    sub_symbol(symbol_ch, "-V");
}

void AddSymbolToMap(const gchar *symbol, const gchar *name,
                    symbol_name_map *sn_map) {
  /* Increase the array to hold another pointer address */
  symbol_to_security_name_container **tmp = g_realloc(
      sn_map->sn_container_arr,
      sizeof(symbol_to_security_name_container *) * (sn_map->size + 1));

  sn_map->sn_container_arr = tmp;

  /* Allocate memory for an object and add the address of it to the array */
  sn_map->sn_container_arr[sn_map->size] =
      g_malloc(sizeof(symbol_to_security_name_container));
  /* Populate a data member with the symbol string */
  sn_map->sn_container_arr[sn_map->size]->symbol =
      g_strdup(symbol ? symbol : "EOF Incomplete Symbol List");
  /* Populate a data member with the name string and increment the size */
  sn_map->sn_container_arr[sn_map->size++]->security_name =
      g_strdup(name ? name : "EOF Incomplete Symbol List");
}

static symbol_name_map *sym_name_map_dup(symbol_name_map *sn_map)
/* Create a duplicate symbol_name_map. */
{
  symbol_name_map *sn_map_dup =
      (symbol_name_map *)g_malloc(sizeof(*sn_map_dup));
  sn_map_dup->sn_container_arr = g_malloc(1);
  sn_map_dup->size = 0;
  /* Not using the hash table. */
  sn_map_dup->hash_table = NULL;

  for (gushort g = 0; g < sn_map->size; g++)
    AddSymbolToMap(sn_map->sn_container_arr[g]->symbol,
                   sn_map->sn_container_arr[g]->security_name, sn_map_dup);

  return sn_map_dup;
}

/* Special Symbols Array */
static const symbol_to_security_name_container special_syms[] = {
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
    {"^IRX", "Treasury Bill ( 13 Weeks U.S. )"},
    {"^FVX", "Treasury Bond Yield ( 5 Years U.S. )"},
    {"^TNX", "Treasury Bond Yield ( 10 Years U.S. )"},
    {"^TYX", "Treasury Bond Yield ( 30 Years U.S. )"},

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
    {"MATIC-USD", "Polygon Coin ( US Dollars )"},
    {"PAXG-USD", "Gold Coin PAX, One Troy Ounce ( US Dollars )"},
    {"XAUT-USD", "Gold Coin Tether, One Troy Ounce ( US Dollars )"},
    {"DGX-USD", "Gold Token Digix ( US Dollars )"},
    {"KAG-USD", "Silver Token Kinesis, One Troy Ounce ( US Dollars )"},
    {"LSILVER-USD", "Silver Coin Lyfe, One Gram ( US Dollars )"},

    /* OTC and other Stocks */
    {"MSBHF", "Mitsubishi Corporation"},
    {"ARRNF", "American Rare Earths Limited"},
    {"THORF", "Thor Energy Plc"},
    {"SYHBF", "Skyharbour Resources Limited"}};

static void add_special_symbols(symbol_name_map *sn_map) {
  gushort size = (sizeof special_syms) / (sizeof special_syms[0]);

  for (gushort i = 0; i < size; i++)
    AddSymbolToMap(special_syms[i].symbol, special_syms[i].security_name,
                   sn_map);
}

static void symbol_list_fetch_cleanup(gchar *line, FILE **fp,
                                      symbol_name_map *sn_map, MemType *Nasdaq,
                                      MemType *NYSE) {
  if (line)
    g_free(line);
  if (fp && fp[0])
    fclose(fp[0]);
  if (fp && fp[1])
    fclose(fp[1]);
  FreeMemtype(Nasdaq);
  FreeMemtype(NYSE);

  if (sn_map) {
    SNMapDestruct(sn_map);
    g_free(sn_map);
  }
}

static symbol_name_map *symbol_list_fetch(portfolio_packet *pkg) {
  meta *D = pkg->GetMetaClass();

  gchar *line = NULL;
  gsize linecap = 0;
  gchar **token_array;
  gchar *symbol_token, *name_token, *tmp_symbol;

  symbol_name_map *sn_map = (symbol_name_map *)g_malloc(sizeof(*sn_map));
  sn_map->sn_container_arr = g_malloc(1);
  sn_map->size = 0;
  sn_map->hash_table = NULL;

  MemType Nasdaq_Struct, NYSE_Struct;
  Nasdaq_Struct.memory = NULL;
  Nasdaq_Struct.size = 0;
  NYSE_Struct.memory = NULL;
  NYSE_Struct.size = 0;

  SetUpCurlHandle(D->NASDAQ_completion_hnd, D->multicurl_cmpltn_hnd,
                  D->Nasdaq_Symbol_url_ch, &Nasdaq_Struct);
  SetUpCurlHandle(D->NYSE_completion_hnd, D->multicurl_cmpltn_hnd,
                  D->NYSE_Symbol_url_ch, &NYSE_Struct);
  if (PerformMultiCurl_no_prog(D->multicurl_cmpltn_hnd) != 0 ||
      pkg->IsExitingApp()) {
    symbol_list_fetch_cleanup(NULL, NULL, sn_map, &Nasdaq_Struct, &NYSE_Struct);
    return NULL;
  }

  /* Convert a String to a File Pointer Stream for Reading */
  FILE *fp[2];
  fp[0] = fmemopen((gpointer)Nasdaq_Struct.memory, Nasdaq_Struct.size + 1, "r");
  fp[1] = fmemopen((gpointer)NYSE_Struct.memory, NYSE_Struct.size + 1, "r");

  guint8 k = 0;
  while (k < 2) {
    /* Read the file stream one line at a time */
    /* Populate the Symbol-Name Array. The second list is added after the first
     * list. */
    while (getline(&line, &linecap, fp[k]) > 0) {
      g_strchomp(line);

      /* If we have an empty line, continue. */
      if (!line[0])
        continue;

      /* Invalid replies start with a tag. */
      if (g_strrstr(line, "<")) {
        symbol_list_fetch_cleanup(line, fp, sn_map, &Nasdaq_Struct,
                                  &NYSE_Struct);
        return NULL;
      }

      /* Get the symbol and the name from the line. */
      token_array = g_strsplit(line, "|", -1);
      if (g_strv_length(token_array) < 7) {
        symbol_list_fetch_cleanup(line, fp, sn_map, &Nasdaq_Struct,
                                  &NYSE_Struct);
        g_strfreev(token_array);
        return NULL;
      }
      symbol_token = token_array[0] ? token_array[0] : "";
      name_token = token_array[1] ? token_array[1] : "";

      /* Check if the symbol is valid. */
      if (check_symbol(symbol_token) == FALSE) {
        g_strfreev(token_array);
        continue;
      }

      /* Warrant, Unit, and stock class symbols [BRK.A -> BRK-A] are different
         on Yahoo! Make appropriate changes, if needed. */
      tmp_symbol = g_strdup(symbol_token);
      substitute_warrant_and_unit_symbols(&tmp_symbol);

      /* Add the symbol and the name to the symbol-name map. */
      if (symbol_token && name_token)
        AddSymbolToMap(tmp_symbol, name_token, sn_map);

      g_free(tmp_symbol);
      g_strfreev(token_array);

      /* If we are exiting the application, return immediately. */
      if (pkg->IsExitingApp()) {
        symbol_list_fetch_cleanup(line, fp, sn_map, &Nasdaq_Struct,
                                  &NYSE_Struct);
        return NULL;
      }
    } /* end while loop */
    k++;
  } /* end while loop */

  /* Add special symbols such as indices, commodities, bonds, and crypto to the
   * map. */

  add_special_symbols(sn_map);

  /* Sort the security symbol array, this reorders both lists into one sorted
   * list. */
  g_qsort_with_data(
      (gconstpointer)&sn_map->sn_container_arr[0], (gint)sn_map->size,
      (gsize)sizeof(sn_map->sn_container_arr[0]), alpha_asc_sec_name, NULL);

  /* Create a hashing table from the sn_container_arr, the symbol is the key
   * value. */
  CreateHashTable(sn_map);

  symbol_list_fetch_cleanup(line, fp, NULL, &Nasdaq_Struct, &NYSE_Struct);

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
  symbol_name_map *sn_map = SqliteGetSNMap(D);

  /* Sort the sn_map [it should already be sorted from the Db, but just to
   * make sure]. */
  if (sn_map)
    g_qsort_with_data(
        (gconstpointer)&sn_map->sn_container_arr[0], (gint)sn_map->size,
        (gsize)sizeof(sn_map->sn_container_arr[0]), alpha_asc_sec_name, NULL);

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
    /* Set the packet interface sym_map variable to the new sym_map. */
    pkg->SetSymNameMap(sn_map_new);

    /* Free the current symbol to security name mapping array. */
    if (sn_map) {
      SNMapDestruct(sn_map);
      g_free(sn_map);
    }

    /* Create a duplicate symbol-name map */
    sn_map_dup = sym_name_map_dup(sn_map_new);

    /* Add the symbol mapping to the db, sn_map_dup is freed in the sqlite
     * thread. */
    SqliteSNMapAdd(sn_map_dup, D);

    /* Return the new symbol-name mapping */
    return sn_map_new;
  } else {

    /* Return the current symbol-name mapping, unchanged */
    return sn_map;
  }
}

void SNMapDestruct(symbol_name_map *sn_map)
/* Frees and resets all members of the sn_map
   object.  Does not free the sn_map object. */
{
  if (sn_map == NULL)
    return;

  if (sn_map->sn_container_arr) {
    for (gushort i = 0; i < sn_map->size; i++) {
      /* Free the string members */
      g_free(sn_map->sn_container_arr[i]->symbol);
      sn_map->sn_container_arr[i]->symbol = NULL;
      g_free(sn_map->sn_container_arr[i]->security_name);
      sn_map->sn_container_arr[i]->security_name = NULL;
      /* Free the memory that the address is pointing to */
      g_free(sn_map->sn_container_arr[i]);
      sn_map->sn_container_arr[i] = NULL;
    }
    /* Free the array of pointer addresses */
    g_free(sn_map->sn_container_arr);
    sn_map->sn_container_arr = NULL;
    sn_map->size = 0;
  }

  /* Destroy the table data and free hash_table */
  if (sn_map->hash_table) {
    g_hash_table_destroy(sn_map->hash_table);
    sn_map->hash_table = NULL;
  }
}