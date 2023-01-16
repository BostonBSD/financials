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

static void set_bul_values(bullion *B, const char *ounce_ch,
                           const char *premium_ch,
                           unsigned short digits_right) {
  B->ounce_f = StringToDouble(ounce_ch);
  DoubleToFormattedStrPango(&B->ounce_mrkd_ch, B->ounce_f, 4, NUM_STR, BLACK);

  B->premium_f = StringToDouble(premium_ch);
  DoubleToFormattedStrPango(&B->premium_mrkd_ch, B->premium_f, digits_right,
                            MON_STR, BLACK);
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
    set_bul_values(m->Gold, argv[2] ? argv[2] : "0", argv[3] ? argv[3] : "0",
                   D->decimal_places_shrt);
  }

  if (strcasecmp(argv[1], "silver") == 0) {
    set_bul_values(m->Silver, argv[2] ? argv[2] : "0", argv[3] ? argv[3] : "0",
                   D->decimal_places_shrt);
  }

  if (strcasecmp(argv[1], "platinum") == 0) {
    set_bul_values(m->Platinum, argv[2] ? argv[2] : "0",
                   argv[3] ? argv[3] : "0", D->decimal_places_shrt);
  }

  if (strcasecmp(argv[1], "palladium") == 0) {
    set_bul_values(m->Palladium, argv[2] ? argv[2] : "0",
                   argv[3] ? argv[3] : "0", D->decimal_places_shrt);
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

  pthread_mutex_unlock(&mutex_working[CLASS_MEMBER_MUTEX]);

  return 0;
}

static int pref_callback(void *data, int argc, char **argv, char **ColName) {
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

  if (strcasecmp(argv[1], "Main_Font") == 0) {
    free(mdata->main_font_ch);
    mdata->main_font_ch = strdup(argv[2] ? argv[2] : MAIN_FONT);
    SetFont(mdata->main_font_ch);
  }

  pthread_mutex_unlock(&mutex_working[CLASS_MEMBER_MUTEX]);

  return 0;
}

static int main_wndwsz_callback(void *data, int argc, char **argv,
                                char **ColName) {
  /* argv[0] is Id, argv[1] is width, argv[2] is height */
  pthread_mutex_lock(&mutex_working[CLASS_MEMBER_MUTEX]);

  if (argc != 3)
    return 1;
  if (strcmp(ColName[0], "Id") != 0)
    return 1;
  if (strcmp(ColName[1], "Width") != 0)
    return 1;
  if (strcmp(ColName[2], "Height") != 0)
    return 1;

  window_data *window = (window_data *)data;
  window->main_width =
      (unsigned short)strtol(argv[1] ? argv[1] : "0", NULL, 10);
  window->main_height =
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

static int history_wndwsz_callback(void *data, int argc, char **argv,
                                   char **ColName) {
  /* argv[0] is Id, argv[1] is height, argv[2] is width */
  pthread_mutex_lock(&mutex_working[CLASS_MEMBER_MUTEX]);

  if (argc != 3)
    return 1;
  if (strcmp(ColName[0], "Id") != 0)
    return 1;
  if (strcmp(ColName[1], "Width") != 0)
    return 1;
  if (strcmp(ColName[2], "Height") != 0)
    return 1;

  window_data *window = (window_data *)data;
  window->history_width =
      (unsigned short)strtol(argv[1] ? argv[1] : "0", NULL, 10);
  window->history_height =
      (unsigned short)strtol(argv[2] ? argv[2] : "0", NULL, 10);

  pthread_mutex_unlock(&mutex_working[CLASS_MEMBER_MUTEX]);
  return 0;
}

static int history_wndwpos_callback(void *data, int argc, char **argv,
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
  window->history_x_pos =
      (unsigned short)strtol(argv[1] ? argv[1] : "0", NULL, 10);
  window->history_y_pos =
      (unsigned short)strtol(argv[2] ? argv[2] : "0", NULL, 10);

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
  AddSymbolToMap(argv[1] ? argv[1] : "Db Error", argv[2] ? argv[2] : "Db Error",
                 sn_map);

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

  /* Create the prefdata table if it doesn't already exist. */
  sql_cmd = "CREATE TABLE IF NOT EXISTS prefdata(Id INTEGER PRIMARY KEY, "
            "Keyword TEXT NOT NULL, Data TEXT NOT NULL);";
  if (sqlite3_exec(db, sql_cmd, 0, 0, &err_msg) != SQLITE_OK)
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

  /* Create the mainwinsize table if it doesn't already exist. */
  sql_cmd = "CREATE TABLE IF NOT EXISTS mainwinsize(Id INTEGER PRIMARY KEY, "
            "Width TEXT NOT NULL, Height TEXT NOT NULL);";
  if (sqlite3_exec(db, sql_cmd, 0, 0, &err_msg) != SQLITE_OK)
    error_msg(db);

  /* Create the mainwinpos table if it doesn't already exist. */
  sql_cmd = "CREATE TABLE IF NOT EXISTS mainwinpos(Id INTEGER PRIMARY KEY, "
            "X TEXT NOT NULL, Y TEXT NOT NULL);";
  if (sqlite3_exec(db, sql_cmd, 0, 0, &err_msg) != SQLITE_OK)
    error_msg(db);

  /* Create the historywinsize table if it doesn't already exist. */
  sql_cmd = "CREATE TABLE IF NOT EXISTS historywinsize(Id INTEGER PRIMARY KEY, "
            "Width TEXT NOT NULL, Height TEXT NOT NULL);";
  if (sqlite3_exec(db, sql_cmd, 0, 0, &err_msg) != SQLITE_OK)
    error_msg(db);

  /* Create the historywinpos table if it doesn't already exist. */
  sql_cmd =
      "CREATE TABLE IF NOT EXISTS historywinpos(Id INTEGER PRIMARY KEY, X "
      "TEXT NOT NULL, Y TEXT NOT NULL);";
  if (sqlite3_exec(db, sql_cmd, 0, 0, &err_msg) != SQLITE_OK)
    error_msg(db);

  /* Reset Equity Folder */
  F->Reset();

  /* Populate class/struct instances with saved data. */
  sql_cmd =
      "SELECT * FROM prefdata;"; /* We always want this table selected first */
  if (sqlite3_exec(db, sql_cmd, pref_callback, D, &err_msg) != SQLITE_OK)
    error_msg(db);

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

  sql_cmd = "SELECT * FROM mainwinsize;";
  if (sqlite3_exec(db, sql_cmd, main_wndwsz_callback, W, &err_msg) != SQLITE_OK)
    error_msg(db);

  sql_cmd = "SELECT * FROM mainwinpos;";
  if (sqlite3_exec(db, sql_cmd, main_wndwpos_callback, W, &err_msg) !=
      SQLITE_OK)
    error_msg(db);

  sql_cmd = "SELECT * FROM historywinsize;";
  if (sqlite3_exec(db, sql_cmd, history_wndwsz_callback, W, &err_msg) !=
      SQLITE_OK)
    error_msg(db);

  sql_cmd = "SELECT * FROM historywinpos;";
  if (sqlite3_exec(db, sql_cmd, history_wndwpos_callback, W, &err_msg) !=
      SQLITE_OK)
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

  if (W->history_width == 0 || W->history_height == 0) {
    /* The Original Production Size, if never run before */
    W->history_width = 925;
    W->history_height = 700;
  }

  if (W->history_x_pos == 0 && W->history_y_pos == 0) {
    W->history_x_pos = 0;
    W->history_y_pos = 32;
  }

  /* Close the sqlite database file. */
  sqlite3_close(db);

  pthread_mutex_unlock(&mutex_working[SQLITE_MUTEX]);

  /* Sort the equity folder. */
  F->Sort();

  /* Generate the Equity Request URLs. */
  F->GenerateURL(pkg);
}

void SqliteEquityAdd(const char *symbol, const char *shares, meta *D) {
  pthread_mutex_lock(&mutex_working[SQLITE_MUTEX]);

  unsigned short len;
  char *err_msg = 0;
  sqlite3 *db;
  const char *del_fmt = "DELETE FROM equity WHERE Symbol = '%s';";
  const char *ins_fmt = "INSERT INTO equity VALUES(null, '%s', '%s');";

  /* Open the sqlite database file. */
  if (sqlite3_open(D->sqlite_db_path_ch, &db) != SQLITE_OK)
    error_msg(db);

  /* Delete entry if already exists, then insert entry. */
  len = snprintf(NULL, 0, del_fmt, symbol) + 1;
  char *sql_cmd = (char *)malloc(len);
  snprintf(sql_cmd, len, del_fmt, symbol);
  if (sqlite3_exec(db, sql_cmd, 0, 0, &err_msg) != SQLITE_OK)
    error_msg(db);

  len = snprintf(NULL, 0, ins_fmt, symbol, shares) + 1;
  char *tmp = realloc(sql_cmd, len);
  sql_cmd = tmp;
  snprintf(sql_cmd, len, ins_fmt, symbol, shares);
  if (sqlite3_exec(db, sql_cmd, 0, 0, &err_msg) != SQLITE_OK)
    error_msg(db);
  free(sql_cmd);

  /* Close the sqlite database file. */
  sqlite3_close(db);

  pthread_mutex_unlock(&mutex_working[SQLITE_MUTEX]);
}

void SqliteBullionAdd(const char *metal_name, const char *ounces,
                      const char *premium, meta *D) {
  pthread_mutex_lock(&mutex_working[SQLITE_MUTEX]);

  unsigned short len;
  char *err_msg = 0;
  sqlite3 *db;
  const char *del_fmt = "DELETE FROM bullion WHERE Metal = '%s';";
  const char *ins_fmt = "INSERT INTO bullion VALUES(null, '%s', '%s', '%s');";

  /* Open the sqlite database file. */
  if (sqlite3_open(D->sqlite_db_path_ch, &db) != SQLITE_OK)
    error_msg(db);

  /* Delete entry if already exists, then insert entry. */
  len = snprintf(NULL, 0, del_fmt, metal_name) + 1;
  char *sql_cmd = (char *)malloc(len);
  snprintf(sql_cmd, len, del_fmt, metal_name);
  if (sqlite3_exec(db, sql_cmd, 0, 0, &err_msg) != SQLITE_OK)
    error_msg(db);

  len = snprintf(NULL, 0, ins_fmt, metal_name, ounces, premium) + 1;
  char *tmp = realloc(sql_cmd, len);
  sql_cmd = tmp;
  snprintf(sql_cmd, len, ins_fmt, metal_name, ounces, premium);
  if (sqlite3_exec(db, sql_cmd, 0, 0, &err_msg) != SQLITE_OK)
    error_msg(db);
  free(sql_cmd);

  /* Close the sqlite database file. */
  sqlite3_close(db);

  pthread_mutex_unlock(&mutex_working[SQLITE_MUTEX]);
}

void SqliteCashAdd(const char *value, meta *D) {
  pthread_mutex_lock(&mutex_working[SQLITE_MUTEX]);

  unsigned short len;
  char *err_msg = 0;
  sqlite3 *db;
  const char *del_fmt = "DELETE FROM cash WHERE Id = 1;";
  const char *ins_fmt = "INSERT INTO cash VALUES(1, '%s');";

  /* Open the sqlite database file. */
  if (sqlite3_open(D->sqlite_db_path_ch, &db) != SQLITE_OK)
    error_msg(db);

  /* Delete entry if already exists, then insert entry. */
  if (sqlite3_exec(db, del_fmt, 0, 0, &err_msg) != SQLITE_OK)
    error_msg(db);

  len = snprintf(NULL, 0, ins_fmt, value) + 1;
  char *sql_cmd = (char *)malloc(len);
  snprintf(sql_cmd, len, ins_fmt, value);
  if (sqlite3_exec(db, sql_cmd, 0, 0, &err_msg) != SQLITE_OK)
    error_msg(db);
  free(sql_cmd);

  /* Close the sqlite database file. */
  sqlite3_close(db);

  pthread_mutex_unlock(&mutex_working[SQLITE_MUTEX]);
}

static void sqlite_api_pref_add(const char *del_fmt, const char *ins_fmt,
                                const char *keyword, const char *data,
                                meta *D) {
  pthread_mutex_lock(&mutex_working[SQLITE_MUTEX]);

  unsigned short len;
  char *err_msg = 0;
  sqlite3 *db;

  /* Open the sqlite database file. */
  if (sqlite3_open(D->sqlite_db_path_ch, &db) != SQLITE_OK)
    error_msg(db);

  /* Delete entry if already exists, then insert entry. */
  len = snprintf(NULL, 0, del_fmt, keyword) + 1;
  char *sql_cmd = (char *)malloc(len);
  snprintf(sql_cmd, len, del_fmt, keyword);
  if (sqlite3_exec(db, sql_cmd, 0, 0, &err_msg) != SQLITE_OK)
    error_msg(db);

  len = snprintf(NULL, 0, ins_fmt, keyword, data) + 1;
  char *tmp = realloc(sql_cmd, len);
  sql_cmd = tmp;
  snprintf(sql_cmd, len, ins_fmt, keyword, data);
  if (sqlite3_exec(db, sql_cmd, 0, 0, &err_msg) != SQLITE_OK)
    error_msg(db);
  free(sql_cmd);

  /* Close the sqlite database file. */
  sqlite3_close(db);

  pthread_mutex_unlock(&mutex_working[SQLITE_MUTEX]);
}

void SqliteAPIAdd(const char *keyword, const char *data, meta *D) {
  const char *del_fmt = "DELETE FROM apidata WHERE Keyword = '%s';";
  const char *ins_fmt = "INSERT INTO apidata VALUES(null, '%s', '%s');";
  sqlite_api_pref_add(del_fmt, ins_fmt, keyword, data, D);
}

void SqlitePrefAdd(const char *keyword, const char *data, meta *D) {
  const char *del_fmt = "DELETE FROM prefdata WHERE Keyword = '%s';";
  const char *ins_fmt = "INSERT INTO prefdata VALUES(null, '%s', '%s');";
  sqlite_api_pref_add(del_fmt, ins_fmt, keyword, data, D);
}

void SqliteEquityRemove(const char *symbol, meta *D) {
  pthread_mutex_lock(&mutex_working[SQLITE_MUTEX]);

  unsigned short len;
  char *err_msg = 0;
  sqlite3 *db;
  const char *del_fmt = "DELETE FROM equity WHERE Symbol = '%s';";

  /* Open the sqlite database file. */
  if (sqlite3_open(D->sqlite_db_path_ch, &db) != SQLITE_OK)
    error_msg(db);

  /* Delete entry if already exists. */
  len = snprintf(NULL, 0, del_fmt, symbol) + 1;
  char *sql_cmd = (char *)malloc(len);
  snprintf(sql_cmd, len, del_fmt, symbol);
  if (sqlite3_exec(db, sql_cmd, 0, 0, &err_msg) != SQLITE_OK)
    error_msg(db);
  free(sql_cmd);

  /* Close the sqlite database file. */
  sqlite3_close(db);

  pthread_mutex_unlock(&mutex_working[SQLITE_MUTEX]);
}

void SqliteEquityRemoveAll(meta *D) {
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

void sqlite_window_data_add(const char *del_fmt, const char *ins_fmt,
                            unsigned short w_x, unsigned short h_y, meta *D) {
  pthread_mutex_lock(&mutex_working[SQLITE_MUTEX]);

  unsigned short len;
  char *err_msg = 0;
  sqlite3 *db;

  /* Open the sqlite database file. */
  if (sqlite3_open(D->sqlite_db_path_ch, &db) != SQLITE_OK)
    error_msg(db);

  /* Delete entry if already exists, then insert entry. */
  if (sqlite3_exec(db, del_fmt, 0, 0, &err_msg) != SQLITE_OK)
    error_msg(db);

  len = snprintf(NULL, 0, ins_fmt, w_x, h_y) + 1;
  char *sql_cmd = (char *)malloc(len);
  snprintf(sql_cmd, len, ins_fmt, w_x, h_y);
  if (sqlite3_exec(db, sql_cmd, 0, 0, &err_msg) != SQLITE_OK)
    error_msg(db);
  free(sql_cmd);

  /* Close the sqlite database file. */
  sqlite3_close(db);

  pthread_mutex_unlock(&mutex_working[SQLITE_MUTEX]);
}

void SqliteMainWindowSizeAdd(unsigned short width, unsigned short height,
                             meta *D) {
  const char *del_fmt = "DELETE FROM mainwinsize WHERE Id = 1;";
  const char *ins_fmt = "INSERT INTO mainwinsize VALUES(1, '%d', '%d');";
  sqlite_window_data_add(del_fmt, ins_fmt, width, height, D);
}

void SqliteMainWindowPosAdd(unsigned short x, unsigned short y, meta *D) {
  const char *del_fmt = "DELETE FROM mainwinpos WHERE Id = 1;";
  const char *ins_fmt = "INSERT INTO mainwinpos VALUES(1, '%d', '%d');";
  sqlite_window_data_add(del_fmt, ins_fmt, x, y, D);
}

void SqliteHistoryWindowSizeAdd(unsigned short width, unsigned short height,
                                meta *D) {
  const char *del_fmt = "DELETE FROM historywinsize WHERE Id = 1;";
  const char *ins_fmt = "INSERT INTO historywinsize VALUES(1, '%d', '%d');";
  sqlite_window_data_add(del_fmt, ins_fmt, width, height, D);
}

void SqliteHistoryWindowPosAdd(unsigned short x, unsigned short y, meta *D) {
  const char *del_fmt = "DELETE FROM historywinpos WHERE Id = 1;";
  const char *ins_fmt = "INSERT INTO historywinpos VALUES(1, '%d', '%d');";
  sqlite_window_data_add(del_fmt, ins_fmt, x, y, D);
}

symbol_name_map *SqliteGetSNMap(meta *D) {
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
    CreateHashTable(sn_map);
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
  const char *ins_fmt = "INSERT INTO symbolname VALUES(null, '%s', '%s');";
  for (unsigned short g = 0; g < sn_map->size; g++) {
    if (sn_map->sn_container_arr[g] == NULL || sn_map->sn_container_arr == NULL)
      break;

    /* Insert an escape apostrophy into the string where needed. */
    escape_apostrophy(&sn_map->sn_container_arr[g]->security_name);

    len = snprintf(NULL, 0, ins_fmt, sn_map->sn_container_arr[g]->symbol,
                   sn_map->sn_container_arr[g]->security_name) +
          1;
    sql_cmd = (char *)malloc(len);
    snprintf(sql_cmd, len, ins_fmt, sn_map->sn_container_arr[g]->symbol,
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

void SqliteSNMapAdd(symbol_name_map *sn_map, meta *D) {
  meta_map_container *mmc = malloc(sizeof(*mmc));
  mmc->map = sn_map;
  mmc->metadata = D;

  /* Add the data in a separate thread; saves time. */
  pthread_t thread_id;
  pthread_create(&thread_id, NULL, add_mapping_to_database, mmc);
  pthread_detach(thread_id);
}