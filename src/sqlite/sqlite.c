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

#include <sqlite3.h>

#include "../include/class_types.h" /* equity_folder, metal, meta, window_data */
#include "../include/gui_types.h"   /* symbol_to_security_name_container, 
                                       symbol_name_map symbol_name_map */
#include "../include/macros.h"
#include "../include/mutex.h"
#include "../include/workfuncs.h"

static gint equity_callback(gpointer data, gint argc, gchar **argv,
                            gchar **ColName) {
  /* argv[0] is id, argv[1] is symbol, argv[2] is shares */
  if (argc != 3)
    return 1;
  if (g_strcmp0(ColName[0], "Id") != 0)
    return 1;
  if (g_strcmp0(ColName[1], "Symbol") != 0)
    return 1;
  if (g_strcmp0(ColName[2], "Shares") != 0)
    return 1;

  equity_folder *F = (equity_folder *)data;
  F->AddStock(argv[1], argv[2]);

  return 0;
}

static void set_bul_values(bullion *B, const gchar *ounce_ch,
                           const gchar *premium_ch, gushort digits_right) {
  B->ounce_f = StringToDouble(ounce_ch);
  DoubleToFormattedStrPango(&B->ounce_mrkd_ch, B->ounce_f, 4, NUM_STR, BLACK);

  B->premium_f = StringToDouble(premium_ch);
  DoubleToFormattedStrPango(&B->premium_mrkd_ch, B->premium_f, digits_right,
                            MON_STR, BLACK);
}

static gint bullion_callback(gpointer data, gint argc, gchar **argv,
                             gchar **ColName) {
  /* argv[0] is Id, argv[1] is Metal, argv[2] is Ounces, argv[3] is Premium */
  g_mutex_lock(&mutexes[CLASS_MEMBER_MUTEX]);

  if (argc != 4)
    return 1;
  if (g_strcmp0(ColName[0], "Id") != 0)
    return 1;
  if (g_strcmp0(ColName[1], "Metal") != 0)
    return 1;
  if (g_strcmp0(ColName[2], "Ounces") != 0)
    return 1;
  if (g_strcmp0(ColName[3], "Premium") != 0)
    return 1;

  portfolio_packet *pkg = (portfolio_packet *)data;
  meta *D = pkg->GetMetaClass();
  metal *m = pkg->GetMetalClass();

  if (g_strcmp0(argv[1], "gold") == 0) {
    set_bul_values(m->Gold, argv[2] ? argv[2] : "0", argv[3] ? argv[3] : "0",
                   D->decimal_places_guint8);

  } else if (g_strcmp0(argv[1], "silver") == 0) {
    set_bul_values(m->Silver, argv[2] ? argv[2] : "0", argv[3] ? argv[3] : "0",
                   D->decimal_places_guint8);

  } else if (g_strcmp0(argv[1], "platinum") == 0) {
    set_bul_values(m->Platinum, argv[2] ? argv[2] : "0",
                   argv[3] ? argv[3] : "0", D->decimal_places_guint8);

  } else if (g_strcmp0(argv[1], "palladium") == 0) {
    set_bul_values(m->Palladium, argv[2] ? argv[2] : "0",
                   argv[3] ? argv[3] : "0", D->decimal_places_guint8);
  }

  g_mutex_unlock(&mutexes[CLASS_MEMBER_MUTEX]);
  return 0;
}

static gint cash_callback(gpointer data, gint argc, gchar **argv,
                          gchar **ColName) {
  g_mutex_lock(&mutexes[CLASS_MEMBER_MUTEX]);

  /* argv[0] is id, argv[1] is value */
  if (argc != 2)
    return 1;
  if (g_strcmp0(ColName[0], "Id") != 0)
    return 1;
  if (g_strcmp0(ColName[1], "Value") != 0)
    return 1;

  meta *mdata = (meta *)data;

  mdata->cash_f = StringToDouble(argv[1] ? argv[1] : "0");
  DoubleToFormattedStrPango(&mdata->cash_mrkd_ch, mdata->cash_f,
                            mdata->decimal_places_guint8, MON_STR, BLACK);

  g_mutex_unlock(&mutexes[CLASS_MEMBER_MUTEX]);
  return 0;
}

static gint api_callback(gpointer data, gint argc, gchar **argv,
                         gchar **ColName) {
  /* argv[0] is Id, argv[1] is Keyword, argv[2] is Data */
  g_mutex_lock(&mutexes[CLASS_MEMBER_MUTEX]);

  if (argc != 3)
    return 1;
  if (g_strcmp0(ColName[0], "Id") != 0)
    return 1;
  if (g_strcmp0(ColName[1], "Keyword") != 0)
    return 1;
  if (g_strcmp0(ColName[2], "Data") != 0)
    return 1;

  meta *mdata = (meta *)data;
  if (g_strcmp0(argv[1], "Stock_URL") == 0) {
    g_free(mdata->stock_url_ch);
    mdata->stock_url_ch = g_strdup(argv[2] ? argv[2] : FINNHUB_URL);

  } else if (g_strcmp0(argv[1], "URL_KEY") == 0) {
    g_free(mdata->curl_key_ch);
    mdata->curl_key_ch = g_strdup(argv[2] ? argv[2] : FINNHUB_URL_TOKEN);

  } else if (g_strcmp0(argv[1], "Nasdaq_Symbol_URL") == 0) {
    g_free(mdata->Nasdaq_Symbol_url_ch);
    mdata->Nasdaq_Symbol_url_ch =
        g_strdup(argv[2] ? argv[2] : NASDAQ_SYMBOL_URL);

  } else if (g_strcmp0(argv[1], "NYSE_Symbol_URL") == 0) {
    g_free(mdata->NYSE_Symbol_url_ch);
    mdata->NYSE_Symbol_url_ch = g_strdup(argv[2] ? argv[2] : NYSE_SYMBOL_URL);
  }

  g_mutex_unlock(&mutexes[CLASS_MEMBER_MUTEX]);

  return 0;
}

static gint pref_callback(gpointer data, gint argc, gchar **argv,
                          gchar **ColName) {
  /* argv[0] is Id, argv[1] is Keyword, argv[2] is Data */
  g_mutex_lock(&mutexes[CLASS_MEMBER_MUTEX]);

  if (argc != 3)
    return 1;
  if (g_strcmp0(ColName[0], "Id") != 0)
    return 1;
  if (g_strcmp0(ColName[1], "Keyword") != 0)
    return 1;
  if (g_strcmp0(ColName[2], "Data") != 0)
    return 1;

  meta *mdata = (meta *)data;
  if (g_strcmp0(argv[1], "Updates_Per_Min") == 0) {
    mdata->updates_per_min_f = g_strtod(argv[2] ? argv[2] : "6", NULL);

  } else if (g_strcmp0(argv[1], "Updates_Hours") == 0) {
    mdata->updates_hours_f = g_strtod(argv[2] ? argv[2] : "1", NULL);

  } else if (g_strcmp0(argv[1], "Decimal_Places") == 0) {
    guint8 d = (guint8)g_ascii_strtoll(argv[2] ? argv[2] : "3", NULL, 10);
    mdata->decimal_places_guint8 = d;

  } else if (g_strcmp0(argv[1], "Clocks_Displayed") == 0) {
    if (g_strcmp0(argv[2] ? argv[2] : "TRUE", "TRUE") == 0) {
      mdata->clocks_displayed_bool = TRUE;
    } else {
      mdata->clocks_displayed_bool = FALSE;
    }

  } else if (g_strcmp0(argv[1], "Indices_Displayed") == 0) {
    if (g_strcmp0(argv[2] ? argv[2] : "TRUE", "TRUE") == 0) {
      mdata->index_bar_revealed_bool = TRUE;
    } else {
      mdata->index_bar_revealed_bool = FALSE;
    }

  } else if (g_strcmp0(argv[1], "Main_Font") == 0) {
    g_free(mdata->font_ch);
    mdata->font_ch = g_strdup(argv[2] ? argv[2] : MAIN_FONT);
    SetFont(mdata->font_ch);
  }

  g_mutex_unlock(&mutexes[CLASS_MEMBER_MUTEX]);

  return 0;
}

static gint main_wndwsz_callback(gpointer data, gint argc, gchar **argv,
                                 gchar **ColName) {
  /* argv[0] is Id, argv[1] is width, argv[2] is height */
  g_mutex_lock(&mutexes[CLASS_MEMBER_MUTEX]);

  if (argc != 3)
    return 1;
  if (g_strcmp0(ColName[0], "Id") != 0)
    return 1;
  if (g_strcmp0(ColName[1], "Width") != 0)
    return 1;
  if (g_strcmp0(ColName[2], "Height") != 0)
    return 1;

  window_data *window = (window_data *)data;
  window->main_width =
      (gushort)g_ascii_strtoll(argv[1] ? argv[1] : "0", NULL, 10);
  window->main_height =
      (gushort)g_ascii_strtoll(argv[2] ? argv[2] : "0", NULL, 10);

  g_mutex_unlock(&mutexes[CLASS_MEMBER_MUTEX]);
  return 0;
}

static gint main_wndwpos_callback(gpointer data, gint argc, gchar **argv,
                                  gchar **ColName) {
  /* argv[0] is Id, argv[1] is X, argv[2] is Y */
  g_mutex_lock(&mutexes[CLASS_MEMBER_MUTEX]);

  if (argc != 3)
    return 1;
  if (g_strcmp0(ColName[0], "Id") != 0)
    return 1;
  if (g_strcmp0(ColName[1], "X") != 0)
    return 1;
  if (g_strcmp0(ColName[2], "Y") != 0)
    return 1;

  window_data *window = (window_data *)data;
  window->main_x_pos =
      (gushort)g_ascii_strtoll(argv[1] ? argv[1] : "0", NULL, 10);
  window->main_y_pos =
      (gushort)g_ascii_strtoll(argv[2] ? argv[2] : "0", NULL, 10);

  g_mutex_unlock(&mutexes[CLASS_MEMBER_MUTEX]);
  return 0;
}

static gint history_wndwsz_callback(gpointer data, gint argc, gchar **argv,
                                    gchar **ColName) {
  /* argv[0] is Id, argv[1] is height, argv[2] is width */
  g_mutex_lock(&mutexes[CLASS_MEMBER_MUTEX]);

  if (argc != 3)
    return 1;
  if (g_strcmp0(ColName[0], "Id") != 0)
    return 1;
  if (g_strcmp0(ColName[1], "Width") != 0)
    return 1;
  if (g_strcmp0(ColName[2], "Height") != 0)
    return 1;

  window_data *window = (window_data *)data;
  window->history_width =
      (gushort)g_ascii_strtoll(argv[1] ? argv[1] : "0", NULL, 10);
  window->history_height =
      (gushort)g_ascii_strtoll(argv[2] ? argv[2] : "0", NULL, 10);

  g_mutex_unlock(&mutexes[CLASS_MEMBER_MUTEX]);
  return 0;
}

static gint history_wndwpos_callback(gpointer data, gint argc, gchar **argv,
                                     gchar **ColName) {
  /* argv[0] is Id, argv[1] is X, argv[2] is Y */
  g_mutex_lock(&mutexes[CLASS_MEMBER_MUTEX]);

  if (argc != 3)
    return 1;
  if (g_strcmp0(ColName[0], "Id") != 0)
    return 1;
  if (g_strcmp0(ColName[1], "X") != 0)
    return 1;
  if (g_strcmp0(ColName[2], "Y") != 0)
    return 1;

  window_data *window = (window_data *)data;
  window->history_x_pos =
      (gushort)g_ascii_strtoll(argv[1] ? argv[1] : "0", NULL, 10);
  window->history_y_pos =
      (gushort)g_ascii_strtoll(argv[2] ? argv[2] : "0", NULL, 10);

  g_mutex_unlock(&mutexes[CLASS_MEMBER_MUTEX]);
  return 0;
}

static gint symbol_name_callback(gpointer data, gint argc, gchar **argv,
                                 gchar **ColName) {
  /* argv[0] is Id, argv[1] is symbol, argv[2] is name */
  if (argc != 3)
    return 1;
  if (g_strcmp0(ColName[0], "Id") != 0)
    return 1;
  if (g_strcmp0(ColName[1], "symbol") != 0)
    return 1;
  if (g_strcmp0(ColName[2], "name") != 0)
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
  gchar *err_msg = 0;
  sqlite3 *db;

  g_mutex_lock(&mutexes[SYMBOL_NAME_MAP_SQLITE_MUTEX]);

  /* Open the sqlite symbol-name database file. */
  if (sqlite3_open(D->sqlite_symbol_name_db_path_ch, &db) != SQLITE_OK)
    error_msg(db);

  /* Create the symbolname table if it doesn't already exist. */
  gchar *sql_cmd = "CREATE TABLE IF NOT EXISTS symbolname(Id INTEGER PRIMARY "
                   "KEY, symbol TEXT NOT NULL, name TEXT NOT NULL);";
  if (sqlite3_exec(db, sql_cmd, 0, 0, &err_msg) != SQLITE_OK)
    error_msg(db);

  /* Close the sqlite symbol-name database file. */
  sqlite3_close(db);

  g_mutex_unlock(&mutexes[SYMBOL_NAME_MAP_SQLITE_MUTEX]);
  g_mutex_lock(&mutexes[SQLITE_MUTEX]);

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

  g_mutex_unlock(&mutexes[SQLITE_MUTEX]);

  /* Sort the equity folder. */
  F->Sort();

  /* Generate the Equity Request URLs. */
  F->GenerateURL(pkg);
}

void SqliteEquityAdd(const gchar *symbol, const gchar *shares, meta *D) {
  g_mutex_lock(&mutexes[SQLITE_MUTEX]);

  gushort len;
  gchar *err_msg = 0;
  sqlite3 *db;
  const gchar *del_fmt = "DELETE FROM equity WHERE Symbol = '%s';";
  const gchar *ins_fmt = "INSERT INTO equity VALUES(null, '%s', '%s');";

  /* Open the sqlite database file. */
  if (sqlite3_open(D->sqlite_db_path_ch, &db) != SQLITE_OK)
    error_msg(db);

  /* Delete entry if already exists, then insert entry. */
  len = g_snprintf(NULL, 0, del_fmt, symbol) + 1;
  gchar *sql_cmd = (gchar *)g_malloc(len);
  g_snprintf(sql_cmd, len, del_fmt, symbol);
  if (sqlite3_exec(db, sql_cmd, 0, 0, &err_msg) != SQLITE_OK)
    error_msg(db);

  len = g_snprintf(NULL, 0, ins_fmt, symbol, shares) + 1;
  gchar *tmp = g_realloc(sql_cmd, len);
  sql_cmd = tmp;
  g_snprintf(sql_cmd, len, ins_fmt, symbol, shares);
  if (sqlite3_exec(db, sql_cmd, 0, 0, &err_msg) != SQLITE_OK)
    error_msg(db);
  g_free(sql_cmd);

  /* Close the sqlite database file. */
  sqlite3_close(db);

  g_mutex_unlock(&mutexes[SQLITE_MUTEX]);
}

void SqliteEquityRemove(const gchar *symbol, meta *D) {
  g_mutex_lock(&mutexes[SQLITE_MUTEX]);

  gushort len;
  gchar *err_msg = 0;
  sqlite3 *db;
  const gchar *del_fmt = "DELETE FROM equity WHERE Symbol = '%s';";

  /* Open the sqlite database file. */
  if (sqlite3_open(D->sqlite_db_path_ch, &db) != SQLITE_OK)
    error_msg(db);

  /* Delete entry if already exists. */
  len = g_snprintf(NULL, 0, del_fmt, symbol) + 1;
  gchar *sql_cmd = (gchar *)g_malloc(len);
  g_snprintf(sql_cmd, len, del_fmt, symbol);
  if (sqlite3_exec(db, sql_cmd, 0, 0, &err_msg) != SQLITE_OK)
    error_msg(db);
  g_free(sql_cmd);

  /* Close the sqlite database file. */
  sqlite3_close(db);

  g_mutex_unlock(&mutexes[SQLITE_MUTEX]);
}

void SqliteEquityRemoveAll(meta *D) {
  g_mutex_lock(&mutexes[SQLITE_MUTEX]);

  gchar *err_msg = 0;
  sqlite3 *db;

  /* Open the sqlite database file. */
  if (sqlite3_open(D->sqlite_db_path_ch, &db) != SQLITE_OK)
    error_msg(db);

  /* Drop the equity table and create a new one. */
  if (sqlite3_exec(db, "DROP TABLE equity;", 0, 0, &err_msg) != SQLITE_OK)
    error_msg(db);

  gchar *sql_cmd = "CREATE TABLE IF NOT EXISTS equity(Id INTEGER PRIMARY KEY, "
                   "Symbol TEXT NOT NULL, Shares TEXT NOT NULL);";
  if (sqlite3_exec(db, sql_cmd, 0, 0, &err_msg) != SQLITE_OK)
    error_msg(db);

  /* Close the sqlite database file. */
  sqlite3_close(db);

  g_mutex_unlock(&mutexes[SQLITE_MUTEX]);
}

void SqliteBullionAdd(const gchar *metal_name, const gchar *ounces,
                      const gchar *premium, meta *D) {
  g_mutex_lock(&mutexes[SQLITE_MUTEX]);

  gushort len;
  gchar *err_msg = 0;
  sqlite3 *db;
  const gchar *del_fmt = "DELETE FROM bullion WHERE Metal = '%s';";
  const gchar *ins_fmt = "INSERT INTO bullion VALUES(null, '%s', '%s', '%s');";

  /* Open the sqlite database file. */
  if (sqlite3_open(D->sqlite_db_path_ch, &db) != SQLITE_OK)
    error_msg(db);

  /* Delete entry if already exists, then insert entry. */
  len = g_snprintf(NULL, 0, del_fmt, metal_name) + 1;
  gchar *sql_cmd = (gchar *)g_malloc(len);
  g_snprintf(sql_cmd, len, del_fmt, metal_name);
  if (sqlite3_exec(db, sql_cmd, 0, 0, &err_msg) != SQLITE_OK)
    error_msg(db);

  len = g_snprintf(NULL, 0, ins_fmt, metal_name, ounces, premium) + 1;
  gchar *tmp = g_realloc(sql_cmd, len);
  sql_cmd = tmp;
  g_snprintf(sql_cmd, len, ins_fmt, metal_name, ounces, premium);
  if (sqlite3_exec(db, sql_cmd, 0, 0, &err_msg) != SQLITE_OK)
    error_msg(db);
  g_free(sql_cmd);

  /* Close the sqlite database file. */
  sqlite3_close(db);

  g_mutex_unlock(&mutexes[SQLITE_MUTEX]);
}

void SqliteCashAdd(const gchar *value, meta *D) {
  g_mutex_lock(&mutexes[SQLITE_MUTEX]);

  gushort len;
  gchar *err_msg = 0;
  sqlite3 *db;
  const gchar *del_fmt = "DELETE FROM cash WHERE Id = 1;";
  const gchar *ins_fmt = "INSERT INTO cash VALUES(1, '%s');";

  /* Open the sqlite database file. */
  if (sqlite3_open(D->sqlite_db_path_ch, &db) != SQLITE_OK)
    error_msg(db);

  /* Delete entry if already exists, then insert entry. */
  if (sqlite3_exec(db, del_fmt, 0, 0, &err_msg) != SQLITE_OK)
    error_msg(db);

  len = g_snprintf(NULL, 0, ins_fmt, value) + 1;
  gchar *sql_cmd = (gchar *)g_malloc(len);
  g_snprintf(sql_cmd, len, ins_fmt, value);
  if (sqlite3_exec(db, sql_cmd, 0, 0, &err_msg) != SQLITE_OK)
    error_msg(db);
  g_free(sql_cmd);

  /* Close the sqlite database file. */
  sqlite3_close(db);

  g_mutex_unlock(&mutexes[SQLITE_MUTEX]);
}

static void sqlite_api_pref_add(const gchar *del_fmt, const gchar *ins_fmt,
                                const gchar *keyword, const gchar *data,
                                meta *D) {
  g_mutex_lock(&mutexes[SQLITE_MUTEX]);

  gushort len;
  gchar *err_msg = 0;
  sqlite3 *db;

  /* Open the sqlite database file. */
  if (sqlite3_open(D->sqlite_db_path_ch, &db) != SQLITE_OK)
    error_msg(db);

  /* Delete entry if already exists, then insert entry. */
  len = g_snprintf(NULL, 0, del_fmt, keyword) + 1;
  gchar *sql_cmd = (gchar *)g_malloc(len);
  g_snprintf(sql_cmd, len, del_fmt, keyword);
  if (sqlite3_exec(db, sql_cmd, 0, 0, &err_msg) != SQLITE_OK)
    error_msg(db);

  len = g_snprintf(NULL, 0, ins_fmt, keyword, data) + 1;
  gchar *tmp = g_realloc(sql_cmd, len);
  sql_cmd = tmp;
  g_snprintf(sql_cmd, len, ins_fmt, keyword, data);
  if (sqlite3_exec(db, sql_cmd, 0, 0, &err_msg) != SQLITE_OK)
    error_msg(db);
  g_free(sql_cmd);

  /* Close the sqlite database file. */
  sqlite3_close(db);

  g_mutex_unlock(&mutexes[SQLITE_MUTEX]);
}

void SqliteAPIAdd(const gchar *keyword, const gchar *data, meta *D) {
  const gchar *del_fmt = "DELETE FROM apidata WHERE Keyword = '%s';";
  const gchar *ins_fmt = "INSERT INTO apidata VALUES(null, '%s', '%s');";
  sqlite_api_pref_add(del_fmt, ins_fmt, keyword, data, D);
}

void SqlitePrefAdd(const gchar *keyword, const gchar *data, meta *D) {
  const gchar *del_fmt = "DELETE FROM prefdata WHERE Keyword = '%s';";
  const gchar *ins_fmt = "INSERT INTO prefdata VALUES(null, '%s', '%s');";
  sqlite_api_pref_add(del_fmt, ins_fmt, keyword, data, D);
}

static void sqlite_window_data_add(const gchar *del_fmt, const gchar *ins_fmt,
                                   gushort w_x, gushort h_y, meta *D) {
  g_mutex_lock(&mutexes[SQLITE_MUTEX]);

  gushort len;
  gchar *err_msg = 0;
  sqlite3 *db;

  /* Open the sqlite database file. */
  if (sqlite3_open(D->sqlite_db_path_ch, &db) != SQLITE_OK)
    error_msg(db);

  /* Delete entry if already exists, then insert entry. */
  if (sqlite3_exec(db, del_fmt, 0, 0, &err_msg) != SQLITE_OK)
    error_msg(db);

  len = g_snprintf(NULL, 0, ins_fmt, w_x, h_y) + 1;
  gchar *sql_cmd = (gchar *)g_malloc(len);
  g_snprintf(sql_cmd, len, ins_fmt, w_x, h_y);
  if (sqlite3_exec(db, sql_cmd, 0, 0, &err_msg) != SQLITE_OK)
    error_msg(db);
  g_free(sql_cmd);

  /* Close the sqlite database file. */
  sqlite3_close(db);

  g_mutex_unlock(&mutexes[SQLITE_MUTEX]);
}

void SqliteMainWindowSizeAdd(gushort width, gushort height, meta *D) {
  const gchar *del_fmt = "DELETE FROM mainwinsize WHERE Id = 1;";
  const gchar *ins_fmt = "INSERT INTO mainwinsize VALUES(1, '%d', '%d');";
  sqlite_window_data_add(del_fmt, ins_fmt, width, height, D);
}

void SqliteMainWindowPosAdd(gushort x, gushort y, meta *D) {
  const gchar *del_fmt = "DELETE FROM mainwinpos WHERE Id = 1;";
  const gchar *ins_fmt = "INSERT INTO mainwinpos VALUES(1, '%d', '%d');";
  sqlite_window_data_add(del_fmt, ins_fmt, x, y, D);
}

void SqliteHistoryWindowSizeAdd(gushort width, gushort height, meta *D) {
  const gchar *del_fmt = "DELETE FROM historywinsize WHERE Id = 1;";
  const gchar *ins_fmt = "INSERT INTO historywinsize VALUES(1, '%d', '%d');";
  sqlite_window_data_add(del_fmt, ins_fmt, width, height, D);
}

void SqliteHistoryWindowPosAdd(gushort x, gushort y, meta *D) {
  const gchar *del_fmt = "DELETE FROM historywinpos WHERE Id = 1;";
  const gchar *ins_fmt = "INSERT INTO historywinpos VALUES(1, '%d', '%d');";
  sqlite_window_data_add(del_fmt, ins_fmt, x, y, D);
}

symbol_name_map *SqliteGetSNMap(meta *D) {
  g_mutex_lock(&mutexes[SYMBOL_NAME_MAP_SQLITE_MUTEX]);

  gchar *err_msg = 0;
  sqlite3 *db;
  symbol_name_map *sn_map = (symbol_name_map *)g_malloc(sizeof(*sn_map));
  sn_map->sn_container_arr = g_malloc(1);
  sn_map->size = 0;
  sn_map->hash_table = NULL;

  /* Open the sqlite database file. */
  if (sqlite3_open(D->sqlite_symbol_name_db_path_ch, &db) != SQLITE_OK)
    error_msg(db);

  gchar *sql_cmd = "SELECT * FROM symbolname;";
  if (sqlite3_exec(db, sql_cmd, symbol_name_callback, sn_map, &err_msg) !=
      SQLITE_OK)
    error_msg(db);

  /* Close the sqlite database file. */
  sqlite3_close(db);

  if (sn_map->size == 0) {
    g_free(sn_map->sn_container_arr);
    g_free(sn_map);
    sn_map = NULL;
  } else {
    /* Create a hashing table of the sn_map. */
    CreateHashTable(sn_map);
  }

  g_mutex_unlock(&mutexes[SYMBOL_NAME_MAP_SQLITE_MUTEX]);
  return sn_map;
}

static void escape_apostrophy(gchar **s)
/* If required, insert an sqlite escape apostrophy to the string. */
{
  if (s == NULL)
    return;
  if (s[0] == NULL)
    return;

  /* Read character by character until the null character is reached. */
  for (gushort i = 0; s[0][i]; i++) {

    /* If we find an ''' character, ASCII decimal code 39 */
    if (s[0][i] == 39) {

      /* Increase the character array by one character. */
      gchar *tmp =
          g_realloc(s[0], (g_utf8_strlen(s[0], -1) + 1 + 1) * sizeof(gchar));
      s[0] = tmp;

      /* Read each character from null back to that character */
      for (gushort j = g_utf8_strlen(s[0], -1); j >= i; j--) {
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

static gpointer add_mapping_to_database_thd(gpointer data) {
  g_mutex_lock(&mutexes[SYMBOL_NAME_MAP_SQLITE_MUTEX]);
  meta_map_container *mmc = (meta_map_container *)data;
  symbol_name_map *sn_map = mmc->map;
  meta *D = mmc->metadata;

  gchar *err_msg = 0;
  sqlite3 *db;
  gushort len;

  /* Open the sqlite database file. */
  if (sqlite3_open(D->sqlite_symbol_name_db_path_ch, &db) != SQLITE_OK)
    error_msg(db);

  /* Drop the symbolname table and create a new one. */
  gchar *sql_cmd = "DROP TABLE symbolname;";
  if (sqlite3_exec(db, sql_cmd, 0, 0, &err_msg) != SQLITE_OK)
    error_msg(db);

  sql_cmd = "CREATE TABLE IF NOT EXISTS symbolname(Id INTEGER PRIMARY KEY, "
            "symbol TEXT NOT NULL, name TEXT NOT NULL);";
  if (sqlite3_exec(db, sql_cmd, 0, 0, &err_msg) != SQLITE_OK)
    error_msg(db);

  /* Insert the mapping into the table. */
  const gchar *ins_fmt = "INSERT INTO symbolname VALUES(null, '%s', '%s');";
  for (gushort g = 0; g < sn_map->size; g++) {
    if (sn_map->sn_container_arr[g] == NULL || sn_map->sn_container_arr == NULL)
      break;

    /* Insert an escape apostrophy into the string where needed. */
    escape_apostrophy(&sn_map->sn_container_arr[g]->security_name);

    len = g_snprintf(NULL, 0, ins_fmt, sn_map->sn_container_arr[g]->symbol,
                     sn_map->sn_container_arr[g]->security_name) +
          1;
    sql_cmd = (gchar *)g_malloc(len);
    g_snprintf(sql_cmd, len, ins_fmt, sn_map->sn_container_arr[g]->symbol,
               sn_map->sn_container_arr[g]->security_name);
    if (sqlite3_exec(db, sql_cmd, 0, 0, &err_msg) != SQLITE_OK)
      error_msg(db);
    g_free(sql_cmd);
  }

  /* Close the sqlite database file. */
  sqlite3_close(db);

  /* Remove the duplicate map from memory. */
  SNMapDestruct(sn_map);
  g_free(sn_map);

  /* Don't Free the member pointers */
  g_free(mmc);

  g_mutex_unlock(&mutexes[SYMBOL_NAME_MAP_SQLITE_MUTEX]);
  g_thread_exit(NULL);
  return NULL;
}

void SqliteSNMapAdd(symbol_name_map *sn_map, meta *D) {
  meta_map_container *mmc = g_malloc(sizeof(*mmc));
  mmc->map = sn_map;
  mmc->metadata = D;

  /* Add the data in a separate thread; saves time. */
  GThread *g_thread_id;
  g_thread_id = g_thread_new(NULL, add_mapping_to_database_thd, mmc);
  g_thread_unref(g_thread_id);
}