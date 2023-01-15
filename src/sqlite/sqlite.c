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
#define _GNU_SOURCE /* hcreate_r, hsearch_r, hdestroy_r; these are GNU         \
                       extensions on GNU/Linux, this macro needs to be defined \
                       before all header files [on GNU systems]. */
#include <math.h>
#include <stdlib.h>
#include <string.h>

#include <sqlite3.h>

#include "../include/class_types.h" /* equity_folder, metal, meta, window_data */
#include "../include/gui_types.h"   /* symbol_to_security_name_container, 
                                       symbol_name_map symbol_name_map, hcreate_r, 
                                       hsearch_r */
#include "../include/macros.h"
#include "../include/mutex.h"
#include "../include/workfuncs.h"

static int equity_callback(void *data, int argc, char **argv, char **ColName) {
  /* argv[0] is id, argv[1] is symbol, argv[2] is shares */
  if (argc != 3)
    return 1;
  if (strcmp(ColName[0], "Id") != 0)
    return 1;
  if (strcmp(ColName[1], "Symbol") != 0)
    return 1;
  if (strcmp(ColName[2], "Shares") != 0)
    return 1;

  equity_folder *F = (equity_folder *)data;
  F->AddStock(argv[1], argv[2]);

  return 0;
}

static int cash_callback(void *data, int argc, char **argv, char **ColName) {
  pthread_mutex_lock(&mutex_working[CLASS_MEMBER_MUTEX]);

  /* argv[0] is id, argv[1] is value */
  if (argc != 2)
    return 1;
  if (strcmp(ColName[0], "Id") != 0)
    return 1;
  if (strcmp(ColName[1], "Value") != 0)
    return 1;

  meta *mdata = (meta *)data;

  mdata->cash_f = StringToDouble(argv[1] ? argv[1] : "0");
  DoubleToFormattedStrPango(&mdata->cash_mrkd_ch, mdata->cash_f,
                            mdata->decimal_places_shrt, MON_STR, BLACK);

  pthread_mutex_unlock(&mutex_working[CLASS_MEMBER_MUTEX]);
  return 0;
}

static int bullion_callback(void *data, int argc, char **argv, char **ColName) {
  /* argv[0] is Id, argv[1] is Metal, argv[2] is Ounces, argv[3] is Premium */
  pthread_mutex_lock(&mutex_working[CLASS_MEMBER_MUTEX]);

  if (argc != 4)
    return 1;
  if (strcmp(ColName[0], "Id") != 0)
    return 1;
  if (strcmp(ColName[1], "Metal") != 0)
    return 1;
  if (strcmp(ColName[2], "Ounces") != 0)
    return 1;
  if (strcmp(ColName[3], "Premium") != 0)
    return 1;

  portfolio_packet *pkg = (portfolio_packet *)data;
  meta *D = pkg->GetMetaClass();
  metal *m = pkg->GetMetalClass();

  if (strcasecmp(argv[1], "gold") == 0) {
    m->Gold->ounce_f = StringToDouble(argv[2] ? argv[2] : "0");
    DoubleToFormattedStrPango(&m->Gold->ounce_mrkd_ch, m->Gold->ounce_f, 4,
                              NUM_STR, BLACK);

    m->Gold->premium_f = StringToDouble(argv[3] ? argv[3] : "0");
    DoubleToFormattedStrPango(&m->Gold->premium_mrkd_ch, m->Gold->premium_f,
                              D->decimal_places_shrt, MON_STR, BLACK);
  }

  if (strcasecmp(argv[1], "silver") == 0) {
    m->Silver->ounce_f = StringToDouble(argv[2] ? argv[2] : "0");
    DoubleToFormattedStrPango(&m->Silver->ounce_mrkd_ch, m->Silver->ounce_f, 4,
                              NUM_STR, BLACK);

    m->Silver->premium_f = StringToDouble(argv[3] ? argv[3] : "0");
    DoubleToFormattedStrPango(&m->Silver->premium_mrkd_ch, m->Silver->premium_f,
                              D->decimal_places_shrt, MON_STR, BLACK);
  }

  if (strcasecmp(argv[1], "platinum") == 0) {
    m->Platinum->ounce_f = StringToDouble(argv[2] ? argv[2] : "0");
    DoubleToFormattedStrPango(&m->Platinum->ounce_mrkd_ch, m->Platinum->ounce_f,
                              4, NUM_STR, BLACK);

    m->Platinum->premium_f = StringToDouble(argv[3] ? argv[3] : "0");
    DoubleToFormattedStrPango(&m->Platinum->premium_mrkd_ch,
                              m->Platinum->premium_f, D->decimal_places_shrt,
                              MON_STR, BLACK);
  }

  if (strcasecmp(argv[1], "palladium") == 0) {
    m->Palladium->ounce_f = StringToDouble(argv[2] ? argv[2] : "0");
    DoubleToFormattedStrPango(&m->Palladium->ounce_mrkd_ch,
                              m->Palladium->ounce_f, 4, NUM_STR, BLACK);

    m->Palladium->premium_f = StringToDouble(argv[3] ? argv[3] : "0");
    DoubleToFormattedStrPango(&m->Palladium->premium_mrkd_ch,
                              m->Palladium->premium_f, D->decimal_places_shrt,
                              MON_STR, BLACK);
  }

  pthread_mutex_unlock(&mutex_working[CLASS_MEMBER_MUTEX]);
  return 0;
}

static int api_callback(void *data, int argc, char **argv, char **ColName) {
  /* argv[0] is Id, argv[1] is Keyword, argv[2] is Data */
  pthread_mutex_lock(&mutex_working[CLASS_MEMBER_MUTEX]);

  if (argc != 3)
    return 1;
  if (strcmp(ColName[0], "Id") != 0)
    return 1;
  if (strcmp(ColName[1], "Keyword") != 0)
    return 1;
  if (strcmp(ColName[2], "Data") != 0)
    return 1;

  meta *mdata = (meta *)data;
  if (strcasecmp(argv[1], "Stock_URL") == 0) {
    free(mdata->stock_url_ch);
    mdata->stock_url_ch = strdup(argv[2] ? argv[2] : FINNHUB_URL);
  }

  if (strcasecmp(argv[1], "URL_KEY") == 0) {
    free(mdata->curl_key_ch);
    mdata->curl_key_ch = strdup(argv[2] ? argv[2] : FINNHUB_URL_TOKEN);
  }

  if (strcasecmp(argv[1], "Nasdaq_Symbol_URL") == 0) {
    free(mdata->Nasdaq_Symbol_url_ch);
    mdata->Nasdaq_Symbol_url_ch = strdup(argv[2] ? argv[2] : NASDAQ_SYMBOL_URL);
  }

  if (strcasecmp(argv[1], "NYSE_Symbol_URL") == 0) {
    free(mdata->NYSE_Symbol_url_ch);
    mdata->NYSE_Symbol_url_ch = strdup(argv[2] ? argv[2] : NYSE_SYMBOL_URL);
  }

  if (strcasecmp(argv[1], "Updates_Per_Min") == 0) {
    mdata->updates_per_min_f = strtod(argv[2] ? argv[2] : "6", NULL);
  }

  if (strcasecmp(argv[1], "Updates_Hours") == 0) {
    mdata->updates_hours_f = strtod(argv[2] ? argv[2] : "1", NULL);
  }

  if (strcasecmp(argv[1], "Decimal_Places") == 0) {
    short d = (short)strtol(argv[2] ? argv[2] : "3", NULL, 10);
    mdata->decimal_places_shrt = d;
  }

  if (strcasecmp(argv[1], "Clocks_Displayed") == 0) {
    if (strcasecmp(argv[2] ? argv[2] : "true", "true") == 0) {
      mdata->clocks_displayed_bool = true;
    } else {
      mdata->clocks_displayed_bool = false;
    }
  }

  if (strcasecmp(argv[1], "Indices_Displayed") == 0) {
    if (strcasecmp(argv[2] ? argv[2] : "true", "true") == 0) {
      mdata->index_bar_revealed_bool = true;
    } else {
      mdata->index_bar_revealed_bool = false;
    }
  }

  if (strcasecmp(argv[1], "Main_TrVw_Font") == 0) {
    free(mdata->main_font_ch);
    mdata->main_font_ch = strdup(argv[2] ? argv[2] : MAIN_FONT);
    SetFont(mdata->main_font_ch);
  }

  pthread_mutex_unlock(&mutex_working[CLASS_MEMBER_MUTEX]);

  return 0;
}

static int main_wndwsz_callback(void *data, int argc, char **argv,
                                char **ColName) {
  /* argv[0] is Id, argv[1] is height, argv[2] is width */
  pthread_mutex_lock(&mutex_working[CLASS_MEMBER_MUTEX]);

  if (argc != 3)
    return 1;
  if (strcmp(ColName[0], "Id") != 0)
    return 1;
  if (strcmp(ColName[1], "Height") != 0)
    return 1;
  if (strcmp(ColName[2], "Width") != 0)
    return 1;

  window_data *window = (window_data *)data;
  window->main_height =
      (unsigned short)strtol(argv[1] ? argv[1] : "0", NULL, 10);
  window->main_width =
      (unsigned short)strtol(argv[2] ? argv[2] : "0", NULL, 10);

  pthread_mutex_unlock(&mutex_working[CLASS_MEMBER_MUTEX]);
  return 0;
}

static int main_wndwpos_callback(void *data, int argc, char **argv,
                                 char **ColName) {
  /* argv[0] is Id, argv[1] is X, argv[2] is Y */
  pthread_mutex_lock(&mutex_working[CLASS_MEMBER_MUTEX]);

  if (argc != 3)
    return 1;
  if (strcmp(ColName[0], "Id") != 0)
    return 1;
  if (strcmp(ColName[1], "X") != 0)
    return 1;
  if (strcmp(ColName[2], "Y") != 0)
    return 1;

  window_data *window = (window_data *)data;
  window->main_x_pos =
      (unsigned short)strtol(argv[1] ? argv[1] : "0", NULL, 10);
  window->main_y_pos =
      (unsigned short)strtol(argv[2] ? argv[2] : "0", NULL, 10);

  pthread_mutex_unlock(&mutex_working[CLASS_MEMBER_MUTEX]);
  return 0;
}

static int rsi_wndwsz_callback(void *data, int argc, char **argv,
                               char **ColName) {
  /* argv[0] is Id, argv[1] is height, argv[2] is width */
  pthread_mutex_lock(&mutex_working[CLASS_MEMBER_MUTEX]);

  if (argc != 3)
    return 1;
  if (strcmp(ColName[0], "Id") != 0)
    return 1;
  if (strcmp(ColName[1], "Height") != 0)
    return 1;
  if (strcmp(ColName[2], "Width") != 0)
    return 1;

  window_data *window = (window_data *)data;
  window->rsi_height =
      (unsigned short)strtol(argv[1] ? argv[1] : "0", NULL, 10);
  window->rsi_width = (unsigned short)strtol(argv[2] ? argv[2] : "0", NULL, 10);

  pthread_mutex_unlock(&mutex_working[CLASS_MEMBER_MUTEX]);
  return 0;
}

static int rsi_wndwpos_callback(void *data, int argc, char **argv,
                                char **ColName) {
  /* argv[0] is Id, argv[1] is X, argv[2] is Y */
  pthread_mutex_lock(&mutex_working[CLASS_MEMBER_MUTEX]);

  if (argc != 3)
    return 1;
  if (strcmp(ColName[0], "Id") != 0)
    return 1;
  if (strcmp(ColName[1], "X") != 0)
    return 1;
  if (strcmp(ColName[2], "Y") != 0)
    return 1;

  window_data *window = (window_data *)data;
  window->rsi_x_pos = (unsigned short)strtol(argv[1] ? argv[1] : "0", NULL, 10);
  window->rsi_y_pos = (unsigned short)strtol(argv[2] ? argv[2] : "0", NULL, 10);

  pthread_mutex_unlock(&mutex_working[CLASS_MEMBER_MUTEX]);
  return 0;
}

static int symbol_name_callback(void *data, int argc, char **argv,
                                char **ColName) {
  /* argv[0] is Id, argv[1] is symbol, argv[2] is name */
  if (argc != 3)
    return 1;
  if (strcmp(ColName[0], "Id") != 0)
    return 1;
  if (strcmp(ColName[1], "symbol") != 0)
    return 1;
  if (strcmp(ColName[2], "name") != 0)
    return 1;

  symbol_name_map *sn_map = (symbol_name_map *)data;
  AddSymbolToMap(argv[1], argv[2], sn_map);

  return 0;
}

static void error_msg(sqlite3 *db) {
  fprintf(stderr, "Sqlite3 database error: %s\n", sqlite3_errmsg(db));
  sqlite3_close(db);
  exit(EXIT_FAILURE);
}

void SqliteProcessing(portfolio_packet *pkg) {
  /* There are two database files, one holds the config info,
  the other holds the stock symbol-name mapping. */
  equity_folder *F = pkg->GetEquityFolderClass();
  meta *D = pkg->GetMetaClass();
  window_data *W = pkg->GetWindowData();
  char *err_msg = 0;
  sqlite3 *db;

  pthread_mutex_lock(&mutex_working[SYMBOL_NAME_MAP_SQLITE_MUTEX]);

  /* Open the sqlite symbol-name database file. */
  if (sqlite3_open(D->sqlite_symbol_name_db_path_ch, &db) != SQLITE_OK)
    error_msg(db);

  /* Create the symbolname table if it doesn't already exist. */
  char *sql_cmd = "CREATE TABLE IF NOT EXISTS symbolname(Id INTEGER PRIMARY "
                  "KEY, symbol TEXT NOT NULL, name TEXT NOT NULL);";
  if (sqlite3_exec(db, sql_cmd, 0, 0, &err_msg) != SQLITE_OK)
    error_msg(db);

  /* Close the sqlite symbol-name database file. */
  sqlite3_close(db);

  pthread_mutex_unlock(&mutex_working[SYMBOL_NAME_MAP_SQLITE_MUTEX]);
  pthread_mutex_lock(&mutex_working[SQLITE_MUTEX]);

  /* Open the regular config sqlite database file. */
  if (sqlite3_open(D->sqlite_db_path_ch, &db) != SQLITE_OK)
    error_msg(db);

  /* Create the apidata table if it doesn't already exist. */
  sql_cmd = "CREATE TABLE IF NOT EXISTS apidata(Id INTEGER PRIMARY KEY, "
            "Keyword TEXT NOT NULL, Data TEXT NOT NULL);";
  if (sqlite3_exec(db, sql_cmd, 0, 0, &err_msg) != SQLITE_OK)
    error_msg(db);

  /* Create the equity table if it doesn't already exist. */
  sql_cmd = "CREATE TABLE IF NOT EXISTS equity(Id INTEGER PRIMARY KEY, Symbol "
            "TEXT NOT NULL, Shares TEXT NOT NULL);";
  if (sqlite3_exec(db, sql_cmd, 0, 0, &err_msg) != SQLITE_OK)
    error_msg(db);

  /* Create the bullion table if it doesn't already exist. */
  sql_cmd = "CREATE TABLE IF NOT EXISTS bullion(Id INTEGER PRIMARY KEY, Metal "
            "TEXT NOT NULL, Ounces TEXT NOT NULL, Premium TEXT NOT NULL);";
  if (sqlite3_exec(db, sql_cmd, 0, 0, &err_msg) != SQLITE_OK)
    error_msg(db);

  /* Create the cash table if it doesn't already exist. */
  sql_cmd = "CREATE TABLE IF NOT EXISTS cash(Id INTEGER PRIMARY KEY, Value "
            "TEXT NOT NULL);";
  if (sqlite3_exec(db, sql_cmd, 0, 0, &err_msg) != SQLITE_OK)
    error_msg(db);

  /* Create the mainwindowsize table if it doesn't already exist. */
  sql_cmd = "CREATE TABLE IF NOT EXISTS mainwindowsize(Id INTEGER PRIMARY KEY, "
            "Height TEXT NOT NULL, Width TEXT NOT NULL);";
  if (sqlite3_exec(db, sql_cmd, 0, 0, &err_msg) != SQLITE_OK)
    error_msg(db);

  /* Create the mainwindowpos table if it doesn't already exist. */
  sql_cmd = "CREATE TABLE IF NOT EXISTS mainwindowpos(Id INTEGER PRIMARY KEY, "
            "X TEXT NOT NULL, Y TEXT NOT NULL);";
  if (sqlite3_exec(db, sql_cmd, 0, 0, &err_msg) != SQLITE_OK)
    error_msg(db);

  /* Create the rsiwindowsize table if it doesn't already exist. */
  sql_cmd = "CREATE TABLE IF NOT EXISTS rsiwindowsize(Id INTEGER PRIMARY KEY, "
            "Height TEXT NOT NULL, Width TEXT NOT NULL);";
  if (sqlite3_exec(db, sql_cmd, 0, 0, &err_msg) != SQLITE_OK)
    error_msg(db);

  /* Create the rsiwindowpos table if it doesn't already exist. */
  sql_cmd = "CREATE TABLE IF NOT EXISTS rsiwindowpos(Id INTEGER PRIMARY KEY, X "
            "TEXT NOT NULL, Y TEXT NOT NULL);";
  if (sqlite3_exec(db, sql_cmd, 0, 0, &err_msg) != SQLITE_OK)
    error_msg(db);

  /* Reset Equity Folder */
  F->Reset();

  /* Populate class/struct instances with saved data. */
  sql_cmd =
      "SELECT * FROM apidata;"; /* We always want this table selected first */
  if (sqlite3_exec(db, sql_cmd, api_callback, D, &err_msg) != SQLITE_OK)
    error_msg(db);

  sql_cmd = "SELECT * FROM equity;";
  if (sqlite3_exec(db, sql_cmd, equity_callback, F, &err_msg) != SQLITE_OK)
    error_msg(db);

  sql_cmd = "SELECT * FROM bullion;";
  if (sqlite3_exec(db, sql_cmd, bullion_callback, pkg, &err_msg) != SQLITE_OK)
    error_msg(db);

  sql_cmd = "SELECT * FROM cash;";
  if (sqlite3_exec(db, sql_cmd, cash_callback, D, &err_msg) != SQLITE_OK)
    error_msg(db);

  sql_cmd = "SELECT * FROM mainwindowsize;";
  if (sqlite3_exec(db, sql_cmd, main_wndwsz_callback, W, &err_msg) != SQLITE_OK)
    error_msg(db);

  sql_cmd = "SELECT * FROM mainwindowpos;";
  if (sqlite3_exec(db, sql_cmd, main_wndwpos_callback, W, &err_msg) !=
      SQLITE_OK)
    error_msg(db);

  sql_cmd = "SELECT * FROM rsiwindowsize;";
  if (sqlite3_exec(db, sql_cmd, rsi_wndwsz_callback, W, &err_msg) != SQLITE_OK)
    error_msg(db);

  sql_cmd = "SELECT * FROM rsiwindowpos;";
  if (sqlite3_exec(db, sql_cmd, rsi_wndwpos_callback, W, &err_msg) != SQLITE_OK)
    error_msg(db);

  if (W->main_width == 0 || W->main_height == 0) {
    /* The Original Production Size, if never run before */
    W->main_width = 900;
    W->main_height = 850;
  }

  if (W->main_x_pos == 0 && W->main_y_pos == 0) {
    W->main_x_pos = 0;
    W->main_y_pos = 32;
  }

  if (W->rsi_width == 0 || W->rsi_height == 0) {
    /* The Original Production Size, if never run before */
    W->rsi_width = 925;
    W->rsi_height = 700;
  }

  if (W->rsi_x_pos == 0 && W->rsi_y_pos == 0) {
    W->rsi_x_pos = 0;
    W->rsi_y_pos = 32;
  }

  /* Close the sqlite database file. */
  sqlite3_close(db);

  pthread_mutex_unlock(&mutex_working[SQLITE_MUTEX]);

  /* Sort the equity folder. */
  F->Sort();

  /* Generate the Equity Request URLs. */
  F->GenerateURL(pkg);
}

void SqliteAddEquity(const char *symbol, const char *shares, meta *D) {
  pthread_mutex_lock(&mutex_working[SQLITE_MUTEX]);

  unsigned short len;
  char *err_msg = 0;
  sqlite3 *db;

  /* Open the sqlite database file. */
  if (sqlite3_open(D->sqlite_db_path_ch, &db) != SQLITE_OK)
    error_msg(db);

  /* Delete entry if already exists, then insert entry. */
  len = strlen("DELETE FROM equity WHERE Symbol = '';") + strlen(symbol) + 1;
  char *sql_cmd = (char *)malloc(len);
  snprintf(sql_cmd, len, "DELETE FROM equity WHERE Symbol = '%s';", symbol);
  if (sqlite3_exec(db, sql_cmd, 0, 0, &err_msg) != SQLITE_OK)
    error_msg(db);

  len = strlen("INSERT INTO equity VALUES(null, '', '');") + strlen(symbol) +
        strlen(shares) + 1;
  char *tmp = realloc(sql_cmd, len);
  sql_cmd = tmp;
  snprintf(sql_cmd, len, "INSERT INTO equity VALUES(null, '%s', '%s');", symbol,
           shares);
  if (sqlite3_exec(db, sql_cmd, 0, 0, &err_msg) != SQLITE_OK)
    error_msg(db);
  free(sql_cmd);

  /* Close the sqlite database file. */
  sqlite3_close(db);

  pthread_mutex_unlock(&mutex_working[SQLITE_MUTEX]);
}

void SqliteAddBullion(const char *metal_name, const char *ounces,
                      const char *premium, meta *D) {
  pthread_mutex_lock(&mutex_working[SQLITE_MUTEX]);

  unsigned short len;
  char *err_msg = 0;
  sqlite3 *db;

  /* Open the sqlite database file. */
  if (sqlite3_open(D->sqlite_db_path_ch, &db) != SQLITE_OK)
    error_msg(db);

  /* Delete entry if already exists, then insert entry. */
  len =
      strlen("DELETE FROM bullion WHERE Metal = '';") + strlen(metal_name) + 1;
  char *sql_cmd = (char *)malloc(len);
  snprintf(sql_cmd, len, "DELETE FROM bullion WHERE Metal = '%s';", metal_name);
  if (sqlite3_exec(db, sql_cmd, 0, 0, &err_msg) != SQLITE_OK)
    error_msg(db);

  len = strlen("INSERT INTO bullion VALUES(null, '', '', '');") +
        strlen(metal_name) + strlen(ounces) + strlen(premium) + 1;
  char *tmp = realloc(sql_cmd, len);
  sql_cmd = tmp;
  snprintf(sql_cmd, len, "INSERT INTO bullion VALUES(null, '%s', '%s', '%s');",
           metal_name, ounces, premium);
  if (sqlite3_exec(db, sql_cmd, 0, 0, &err_msg) != SQLITE_OK)
    error_msg(db);
  free(sql_cmd);

  /* Close the sqlite database file. */
  sqlite3_close(db);

  pthread_mutex_unlock(&mutex_working[SQLITE_MUTEX]);
}

void SqliteAddCash(const char *value, meta *D) {
  pthread_mutex_lock(&mutex_working[SQLITE_MUTEX]);

  unsigned short len;
  char *err_msg = 0;
  sqlite3 *db;

  /* Open the sqlite database file. */
  if (sqlite3_open(D->sqlite_db_path_ch, &db) != SQLITE_OK)
    error_msg(db);

  /* Delete entry if already exists, then insert entry. */
  if (sqlite3_exec(db, "DELETE FROM cash WHERE Id = 1;", 0, 0, &err_msg) !=
      SQLITE_OK)
    error_msg(db);

  len = strlen("INSERT INTO cash VALUES(1, '');") + strlen(value) + 1;
  char *sql_cmd = (char *)malloc(len);
  snprintf(sql_cmd, len, "INSERT INTO cash VALUES(1, '%s');", value);
  if (sqlite3_exec(db, sql_cmd, 0, 0, &err_msg) != SQLITE_OK)
    error_msg(db);
  free(sql_cmd);

  /* Close the sqlite database file. */
  sqlite3_close(db);

  pthread_mutex_unlock(&mutex_working[SQLITE_MUTEX]);
}

void SqliteAddAPIData(const char *keyword, const char *data, meta *D) {
  pthread_mutex_lock(&mutex_working[SQLITE_MUTEX]);

  unsigned short len;
  char *err_msg = 0;
  sqlite3 *db;

  /* Open the sqlite database file. */
  if (sqlite3_open(D->sqlite_db_path_ch, &db) != SQLITE_OK)
    error_msg(db);

  /* Delete entry if already exists, then insert entry. */
  len = strlen("DELETE FROM apidata WHERE Keyword = '';") + strlen(keyword) + 1;
  char *sql_cmd = (char *)malloc(len);
  snprintf(sql_cmd, len, "DELETE FROM apidata WHERE Keyword = '%s';", keyword);
  if (sqlite3_exec(db, sql_cmd, 0, 0, &err_msg) != SQLITE_OK)
    error_msg(db);

  len = strlen("INSERT INTO apidata VALUES(null, '', '');") + strlen(keyword) +
        strlen(data) + 1;
  char *tmp = realloc(sql_cmd, len);
  sql_cmd = tmp;
  snprintf(sql_cmd, len, "INSERT INTO apidata VALUES(null, '%s', '%s');",
           keyword, data);
  if (sqlite3_exec(db, sql_cmd, 0, 0, &err_msg) != SQLITE_OK)
    error_msg(db);
  free(sql_cmd);

  /* Close the sqlite database file. */
  sqlite3_close(db);

  pthread_mutex_unlock(&mutex_working[SQLITE_MUTEX]);
}

void SqliteRemoveEquity(const char *symbol, meta *D) {
  pthread_mutex_lock(&mutex_working[SQLITE_MUTEX]);

  unsigned short len;
  char *err_msg = 0;
  sqlite3 *db;

  /* Open the sqlite database file. */
  if (sqlite3_open(D->sqlite_db_path_ch, &db) != SQLITE_OK)
    error_msg(db);

  /* Delete entry if already exists. */
  len = strlen("DELETE FROM equity WHERE Symbol = '';") + strlen(symbol) + 1;
  char *sql_cmd = (char *)malloc(len);
  snprintf(sql_cmd, len, "DELETE FROM equity WHERE Symbol = '%s';", symbol);
  if (sqlite3_exec(db, sql_cmd, 0, 0, &err_msg) != SQLITE_OK)
    error_msg(db);
  free(sql_cmd);

  /* Close the sqlite database file. */
  sqlite3_close(db);

  pthread_mutex_unlock(&mutex_working[SQLITE_MUTEX]);
}

void SqliteRemoveAllEquity(meta *D) {
  pthread_mutex_lock(&mutex_working[SQLITE_MUTEX]);

  char *err_msg = 0;
  sqlite3 *db;

  /* Open the sqlite database file. */
  if (sqlite3_open(D->sqlite_db_path_ch, &db) != SQLITE_OK)
    error_msg(db);

  /* Drop the equity table and create a new one. */
  if (sqlite3_exec(db, "DROP TABLE equity;", 0, 0, &err_msg) != SQLITE_OK)
    error_msg(db);

  char *sql_cmd = "CREATE TABLE IF NOT EXISTS equity(Id INTEGER PRIMARY KEY, "
                  "Symbol TEXT NOT NULL, Shares TEXT NOT NULL);";
  if (sqlite3_exec(db, sql_cmd, 0, 0, &err_msg) != SQLITE_OK)
    error_msg(db);

  /* Close the sqlite database file. */
  sqlite3_close(db);

  pthread_mutex_unlock(&mutex_working[SQLITE_MUTEX]);
}

void SqliteAddMainWindowSize(unsigned short width, unsigned short height,
                             meta *D) {
  pthread_mutex_lock(&mutex_working[SQLITE_MUTEX]);

  unsigned short len;
  char *err_msg = 0;
  sqlite3 *db;

  /* Open the sqlite database file. */
  if (sqlite3_open(D->sqlite_db_path_ch, &db) != SQLITE_OK)
    error_msg(db);

  /* Delete entry if already exists, then insert entry. */
  if (sqlite3_exec(db, "DELETE FROM mainwindowsize WHERE Id = 1;", 0, 0,
                   &err_msg) != SQLITE_OK)
    error_msg(db);

  len = strlen("INSERT INTO mainwindowsize VALUES(1, '', '');") + 64;
  char *sql_cmd = (char *)malloc(len);
  snprintf(sql_cmd, len, "INSERT INTO mainwindowsize VALUES(1, '%d', '%d');",
           height, width);
  if (sqlite3_exec(db, sql_cmd, 0, 0, &err_msg) != SQLITE_OK)
    error_msg(db);
  free(sql_cmd);

  /* Close the sqlite database file. */
  sqlite3_close(db);

  pthread_mutex_unlock(&mutex_working[SQLITE_MUTEX]);
}

void SqliteAddMainWindowPos(unsigned short x, unsigned short y, meta *D) {
  pthread_mutex_lock(&mutex_working[SQLITE_MUTEX]);

  unsigned short len;
  char *err_msg = 0;
  sqlite3 *db;

  /* Open the sqlite database file. */
  if (sqlite3_open(D->sqlite_db_path_ch, &db) != SQLITE_OK)
    error_msg(db);

  /* Delete entry if already exists, then insert entry. */
  if (sqlite3_exec(db, "DELETE FROM mainwindowpos WHERE Id = 1;", 0, 0,
                   &err_msg) != SQLITE_OK)
    error_msg(db);

  len = strlen("INSERT INTO mainwindowpos VALUES(1, '', '');") + 64;
  char *sql_cmd = (char *)malloc(len);
  snprintf(sql_cmd, len, "INSERT INTO mainwindowpos VALUES(1, '%d', '%d');", x,
           y);
  if (sqlite3_exec(db, sql_cmd, 0, 0, &err_msg) != SQLITE_OK)
    error_msg(db);
  free(sql_cmd);

  /* Close the sqlite database file. */
  sqlite3_close(db);

  pthread_mutex_unlock(&mutex_working[SQLITE_MUTEX]);
}

void SqliteAddRSIWindowSize(int width, int height, meta *D) {
  pthread_mutex_lock(&mutex_working[SQLITE_MUTEX]);

  unsigned short len;
  char *err_msg = 0;
  sqlite3 *db;

  /* Open the sqlite database file. */
  if (sqlite3_open(D->sqlite_db_path_ch, &db) != SQLITE_OK)
    error_msg(db);

  /* Delete entry if already exists, then insert entry. */
  if (sqlite3_exec(db, "DELETE FROM rsiwindowsize WHERE Id = 1;", 0, 0,
                   &err_msg) != SQLITE_OK)
    error_msg(db);

  len = strlen("INSERT INTO rsiwindowsize VALUES(1, '', '');") + 64;
  char *sql_cmd = (char *)malloc(len);
  snprintf(sql_cmd, len, "INSERT INTO rsiwindowsize VALUES(1, '%d', '%d');",
           height, width);
  if (sqlite3_exec(db, sql_cmd, 0, 0, &err_msg) != SQLITE_OK)
    error_msg(db);
  free(sql_cmd);

  /* Close the sqlite database file. */
  sqlite3_close(db);

  pthread_mutex_unlock(&mutex_working[SQLITE_MUTEX]);
}

void SqliteAddRSIWindowPos(int x, int y, meta *D) {
  pthread_mutex_lock(&mutex_working[SQLITE_MUTEX]);

  unsigned short len;
  char *err_msg = 0;
  sqlite3 *db;

  /* Open the sqlite database file. */
  if (sqlite3_open(D->sqlite_db_path_ch, &db) != SQLITE_OK)
    error_msg(db);

  /* Delete entry if already exists, then insert entry. */
  if (sqlite3_exec(db, "DELETE FROM rsiwindowpos WHERE Id = 1;", 0, 0,
                   &err_msg) != SQLITE_OK)
    error_msg(db);

  len = strlen("INSERT INTO rsiwindowpos VALUES(1, '', '');") + 64;
  char *sql_cmd = (char *)malloc(len);
  snprintf(sql_cmd, len, "INSERT INTO rsiwindowpos VALUES(1, '%d', '%d');", x,
           y);
  if (sqlite3_exec(db, sql_cmd, 0, 0, &err_msg) != SQLITE_OK)
    error_msg(db);
  free(sql_cmd);

  /* Close the sqlite database file. */
  sqlite3_close(db);

  pthread_mutex_unlock(&mutex_working[SQLITE_MUTEX]);
}

symbol_name_map *SqliteGetSymbolNameMap(meta *D) {
  pthread_mutex_lock(&mutex_working[SYMBOL_NAME_MAP_SQLITE_MUTEX]);

  char *err_msg = 0;
  sqlite3 *db;
  symbol_name_map *sn_map = (symbol_name_map *)malloc(sizeof(*sn_map));
  sn_map->sn_container_arr = malloc(1);
  sn_map->size = 0;
  sn_map->htab = NULL;

  /* Open the sqlite database file. */
  if (sqlite3_open(D->sqlite_symbol_name_db_path_ch, &db) != SQLITE_OK)
    error_msg(db);

  char *sql_cmd = "SELECT * FROM symbolname;";
  if (sqlite3_exec(db, sql_cmd, symbol_name_callback, sn_map, &err_msg) !=
      SQLITE_OK)
    error_msg(db);

  /* Close the sqlite database file. */
  sqlite3_close(db);

  if (sn_map->size == 0) {
    free(sn_map->sn_container_arr);
    free(sn_map);
    sn_map = NULL;
  } else {
    /* Create a hashing table of the sn_map. */
    sn_map->htab = (struct hsearch_data *)malloc(sizeof(struct hsearch_data));
    /*zeroize the table.*/
    memset(sn_map->htab, 0, sizeof(struct hsearch_data));

    ENTRY e, *ep;
    /* GNU suggests the table size be 25% larger than needed.  On FreeBSD table
     * size is dynamic. */
    size_t tab_size = (size_t)floor((double)(sn_map->size * 1.25));
    hcreate_r(tab_size, sn_map->htab);
    unsigned short s = 0;
    while (s < sn_map->size) {
      e.key = sn_map->sn_container_arr[s]->symbol;
      e.data = sn_map->sn_container_arr[s];
      if (hsearch_r(e, ENTER, &ep, sn_map->htab) == 0) {
        fprintf(stderr, "entry failed\n");
        exit(EXIT_FAILURE);
      }
      s++;
    }
  }

  pthread_mutex_unlock(&mutex_working[SYMBOL_NAME_MAP_SQLITE_MUTEX]);
  return sn_map;
}

static void escape_apostrophy(char **s)
/* If required, insert an sqlite escape apostrophy to the string. */
{
  if (s == NULL)
    return;
  if (s[0] == NULL)
    return;

  /* Read character by character until the null character is reached. */
  for (unsigned short i = 0; s[0][i]; i++) {

    /* If we find an ''' character, ASCII decimal code 39 */
    if (s[0][i] == 39) {

      /* Increase the character array by one character. */
      char *tmp = realloc(s[0], (strlen(s[0]) + 1 + 1) * sizeof(char));
      s[0] = tmp;

      /* Read each character from null back to that character */
      for (unsigned short j = strlen(s[0]); j >= i; j--) {
        /* Shift the array one character to the right [duplicate the character]
         */
        s[0][j + 1] = s[0][j];
      }

      /* Because we have a duplicate we need to skip the next increment of i */
      i++;
    }
  }
}

typedef struct {
  symbol_name_map *map;
  meta *metadata;
} meta_map_container;

static void *add_mapping_to_database(void *data) {
  pthread_mutex_lock(&mutex_working[SYMBOL_NAME_MAP_SQLITE_MUTEX]);

  meta_map_container *mmc = (meta_map_container *)data;
  symbol_name_map *sn_map = mmc->map;
  meta *D = mmc->metadata;

  char *err_msg = 0;
  sqlite3 *db;
  unsigned short len;

  /* Open the sqlite database file. */
  if (sqlite3_open(D->sqlite_symbol_name_db_path_ch, &db) != SQLITE_OK)
    error_msg(db);

  /* Drop the symbolname table and create a new one. */
  char *sql_cmd = "DROP TABLE symbolname;";
  if (sqlite3_exec(db, sql_cmd, 0, 0, &err_msg) != SQLITE_OK)
    error_msg(db);

  sql_cmd = "CREATE TABLE IF NOT EXISTS symbolname(Id INTEGER PRIMARY KEY, "
            "symbol TEXT NOT NULL, name TEXT NOT NULL);";
  if (sqlite3_exec(db, sql_cmd, 0, 0, &err_msg) != SQLITE_OK)
    error_msg(db);

  /* Insert the mapping into the table. */
  for (unsigned short g = 0; g < sn_map->size; g++) {
    if (sn_map->sn_container_arr[g] == NULL || sn_map->sn_container_arr == NULL)
      break;

    /* Insert an escape apostrophy into the string where needed. */
    escape_apostrophy(&sn_map->sn_container_arr[g]->security_name);

    len = strlen("INSERT INTO symbolname VALUES(null, '', '');") +
          strlen(sn_map->sn_container_arr[g]->symbol) +
          strlen(sn_map->sn_container_arr[g]->security_name) + 1;
    sql_cmd = (char *)malloc(len);
    snprintf(sql_cmd, len, "INSERT INTO symbolname VALUES(null, '%s', '%s');",
             sn_map->sn_container_arr[g]->symbol,
             sn_map->sn_container_arr[g]->security_name);
    if (sqlite3_exec(db, sql_cmd, 0, 0, &err_msg) != SQLITE_OK)
      error_msg(db);
    free(sql_cmd);
  }

  /* Close the sqlite database file. */
  sqlite3_close(db);

  /* Remove the duplicate map from memory. */
  SNMapDestruct(sn_map);
  free(sn_map);

  /* Don't Free the member pointers */
  free(mmc);

  pthread_mutex_unlock(&mutex_working[SYMBOL_NAME_MAP_SQLITE_MUTEX]);
  pthread_exit(NULL);
}

void SqliteAddMap(symbol_name_map *sn_map, meta *D) {
  meta_map_container *mmc = malloc(sizeof(*mmc));
  mmc->map = sn_map;
  mmc->metadata = D;

  /* Add the data in a separate thread; saves time. */
  pthread_t thread_id;
  pthread_create(&thread_id, NULL, add_mapping_to_database, mmc);
  pthread_detach(thread_id);
}