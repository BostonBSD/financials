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

#include <glib/gprintf.h> /* g_fprintf() */
#include <sqlite3.h>

#include "../include/class_types.h" /* equity_folder, metal, meta, window_data */
#include "../include/gui_types.h"   /* symbol_to_security_name_container, 
                                       symbol_name_map symbol_name_map */
#include "../include/macros.h"
#include "../include/mutex.h"
#include "../include/sqlite.h"
#include "../include/workfuncs.h"

/* Create the symbolname table if it doesn't already exist. */
static gchar *new_symname_tbl =
    "CREATE TABLE IF NOT EXISTS symbolname(Id INTEGER PRIMARY "
    "KEY, symbol TEXT NOT NULL, name TEXT NOT NULL); CREATE UNIQUE "
    "INDEX IF NOT EXISTS idx_symbolname_symbol ON symbolname (symbol);";

/* Create the app_tbl table if it doesn't already exist. */
static gchar *new_app_tbl =
    "CREATE TABLE IF NOT EXISTS app_tbl(id INTEGER PRIMARY KEY, keyword TEXT "
    "NOT NULL, data_one TEXT NOT NULL, data_two TEXT NOT NULL); CREATE UNIQUE "
    "INDEX IF NOT EXISTS idx_app_tbl_keyword ON app_tbl (keyword);";

/* Create the equity table if it doesn't already exist. */
static gchar *new_eqty_tbl =
    "CREATE TABLE IF NOT EXISTS equity(Id INTEGER PRIMARY KEY, Symbol TEXT NOT "
    "NULL, Shares TEXT NOT NULL, Cost TEXT NOT NULL); CREATE UNIQUE INDEX IF "
    "NOT EXISTS idx_equity_symbol ON equity (Symbol);";

/* Create the bullion table if it doesn't already exist. */
static gchar *new_bul_tbl =
    "CREATE TABLE IF NOT EXISTS bullion(Id INTEGER PRIMARY KEY, Metal TEXT NOT "
    "NULL, Ounces TEXT NOT NULL, Premium TEXT NOT NULL, Cost TEXT NOT NULL); "
    "CREATE UNIQUE INDEX IF NOT EXISTS idx_bullion_metal ON bullion (Metal);";

static gint app_callback(gpointer data, gint argc, gchar **argv,
                         gchar **ColName) {
  /* argv[0] is id, argv[1] is keyword, argv[2] is data_one, argv[3] is
   * data_two. */
  if (argc != 4)
    return 1;
  if (g_strcmp0(ColName[0], "id") != 0)
    return 1;
  if (g_strcmp0(ColName[1], "keyword") != 0)
    return 1;
  if (g_strcmp0(ColName[2], "data_one") != 0)
    return 1;
  if (g_strcmp0(ColName[3], "data_two") != 0)
    return 1;

  portfolio_packet *pkg = (portfolio_packet *)data;
  meta *D = pkg->GetMetaClass();

  if (!g_strcmp0(argv[1], "Stock_URL")) {
    g_free(D->stock_url_ch);
    D->stock_url_ch = g_strdup(argv[2] ? argv[2] : FINNHUB_URL);

  } else if (!g_strcmp0(argv[1], "URL_KEY")) {
    g_free(D->curl_key_ch);
    D->curl_key_ch = g_strdup(argv[2] ? argv[2] : FINNHUB_URL_TOKEN);

  } else if (!g_strcmp0(argv[1], "Nasdaq_Symbol_URL")) {
    g_free(D->Nasdaq_Symbol_url_ch);
    D->Nasdaq_Symbol_url_ch = g_strdup(argv[2] ? argv[2] : NASDAQ_SYMBOL_URL);

  } else if (!g_strcmp0(argv[1], "NYSE_Symbol_URL")) {
    g_free(D->NYSE_Symbol_url_ch);
    D->NYSE_Symbol_url_ch = g_strdup(argv[2] ? argv[2] : NYSE_SYMBOL_URL);

  } else if (!g_strcmp0(argv[1], "Updates_Per_Min")) {
    D->updates_per_min_f = g_strtod(argv[2] ? argv[2] : "6", NULL);

  } else if (!g_strcmp0(argv[1], "Updates_Hours")) {
    D->updates_hours_f = g_strtod(argv[2] ? argv[2] : "1", NULL);

  } else if (!g_strcmp0(argv[1], "Decimal_Places")) {
    guint8 d = (guint8)g_ascii_strtoll(argv[2] ? argv[2] : "3", NULL, 10);
    D->decimal_places_guint8 = d;

  } else if (!g_strcmp0(argv[1], "Clocks_Displayed")) {
    if (!g_strcmp0(argv[2] ? argv[2] : "TRUE", "TRUE")) {
      D->clocks_displayed_bool = TRUE;
    } else {
      D->clocks_displayed_bool = FALSE;
    }

  } else if (!g_strcmp0(argv[1], "Indices_Displayed")) {
    if (!g_strcmp0(argv[2] ? argv[2] : "TRUE", "TRUE")) {
      D->index_bar_revealed_bool = TRUE;
    } else {
      D->index_bar_revealed_bool = FALSE;
    }

  } else if (!g_strcmp0(argv[1], "Main_Win_Maxmzd")) {
    if (!g_strcmp0(argv[2] ? argv[2] : "TRUE", "TRUE")) {
      D->window_struct.main_win_maximized_bool = TRUE;
    } else {
      D->window_struct.main_win_maximized_bool = FALSE;
    }

  } else if (!g_strcmp0(argv[1], "Hstry_Win_Maxmzd")) {
    if (!g_strcmp0(argv[2] ? argv[2] : "TRUE", "TRUE")) {
      D->window_struct.histry_win_maximized_bool = TRUE;
    } else {
      D->window_struct.histry_win_maximized_bool = FALSE;
    }

  } else if (!g_strcmp0(argv[1], "Main_Font")) {
    g_free(D->font_ch);
    D->font_ch = g_strdup(argv[2] ? argv[2] : MAIN_FONT);
    SetFont(D->font_ch);

  } else if (!g_strcmp0(argv[1], "Cash")) {
    D->cash_f = StringToDouble(argv[2] ? argv[2] : "0");
    DoubleToFormattedStrPango(&D->cash_mrkd_ch, D->cash_f,
                              D->decimal_places_guint8, MON_STR, BLACK);

  } else if (!g_strcmp0(argv[1], "Main_Win_Sz")) {
    D->window_struct.main_width =
        (gint)g_ascii_strtoll(argv[2] ? argv[2] : "0", NULL, 10);
    D->window_struct.main_height =
        (gint)g_ascii_strtoll(argv[3] ? argv[3] : "0", NULL, 10);

  } else if (!g_strcmp0(argv[1], "Main_Win_Pos")) {
    D->window_struct.main_x_pos =
        (gint)g_ascii_strtoll(argv[2] ? argv[2] : "0", NULL, 10);
    D->window_struct.main_y_pos =
        (gint)g_ascii_strtoll(argv[3] ? argv[3] : "0", NULL, 10);

  } else if (!g_strcmp0(argv[1], "Hstry_Win_Sz")) {
    D->window_struct.history_width =
        (gint)g_ascii_strtoll(argv[2] ? argv[2] : "0", NULL, 10);
    D->window_struct.history_height =
        (gint)g_ascii_strtoll(argv[3] ? argv[3] : "0", NULL, 10);

  } else if (!g_strcmp0(argv[1], "Hstry_Win_Pos")) {
    D->window_struct.history_x_pos =
        (gint)g_ascii_strtoll(argv[2] ? argv[2] : "0", NULL, 10);
    D->window_struct.history_y_pos =
        (gint)g_ascii_strtoll(argv[3] ? argv[3] : "0", NULL, 10);
  }
  return 0;
}

static gint equity_callback(gpointer data, gint argc, gchar **argv,
                            gchar **ColName) {
  /* argv[0] is Id, argv[1] is Symbol, argv[2] is Shares, argv[3] is Cost */
  if (argc != 4)
    return 1;
  if (g_strcmp0(ColName[0], "Id") != 0)
    return 1;
  if (g_strcmp0(ColName[1], "Symbol") != 0)
    return 1;
  if (g_strcmp0(ColName[2], "Shares") != 0)
    return 1;
  if (g_strcmp0(ColName[3], "Cost") != 0)
    return 1;

  equity_folder *F = (equity_folder *)data;
  F->AddStock(argv[1], argv[2], argv[3]);

  return 0;
}

static void set_bul_values(bullion *B, const gchar *ounce_ch,
                           const gchar *premium_ch, const gchar *cost_ch,
                           gushort digits_right) {
  B->ounce_f = StringToDouble(ounce_ch);

  B->premium_f = StringToDouble(premium_ch);
  DoubleToFormattedStrPango(&B->premium_mrkd_ch, B->premium_f, digits_right,
                            MON_STR, BLACK);

  B->cost_basis_f = StringToDouble(cost_ch);
  DoubleToFormattedStrPango(&B->cost_mrkd_ch, B->cost_basis_f, digits_right,
                            MON_STR, BLACK);
}

static gint bullion_callback(gpointer data, gint argc, gchar **argv,
                             gchar **ColName) {
  /* argv[0] is Id, argv[1] is Metal, argv[2] is Ounces, argv[3] is Premium,
   * argv[4] is Cost */
  if (argc != 5)
    return 1;
  if (g_strcmp0(ColName[0], "Id") != 0)
    return 1;
  if (g_strcmp0(ColName[1], "Metal") != 0)
    return 1;
  if (g_strcmp0(ColName[2], "Ounces") != 0)
    return 1;
  if (g_strcmp0(ColName[3], "Premium") != 0)
    return 1;
  if (g_strcmp0(ColName[4], "Cost") != 0)
    return 1;

  portfolio_packet *pkg = (portfolio_packet *)data;
  meta *D = pkg->GetMetaClass();
  metal *m = pkg->GetMetalClass();

  if (!g_strcmp0(argv[1], "gold"))
    set_bul_values(m->Gold, argv[2] ? argv[2] : "0", argv[3] ? argv[3] : "0",
                   argv[4] ? argv[4] : "0", D->decimal_places_guint8);

  else if (!g_strcmp0(argv[1], "silver"))
    set_bul_values(m->Silver, argv[2] ? argv[2] : "0", argv[3] ? argv[3] : "0",
                   argv[4] ? argv[4] : "0", D->decimal_places_guint8);

  else if (!g_strcmp0(argv[1], "platinum"))
    set_bul_values(m->Platinum, argv[2] ? argv[2] : "0",
                   argv[3] ? argv[3] : "0", argv[4] ? argv[4] : "0",
                   D->decimal_places_guint8);

  else if (!g_strcmp0(argv[1], "palladium"))
    set_bul_values(m->Palladium, argv[2] ? argv[2] : "0",
                   argv[3] ? argv[3] : "0", argv[4] ? argv[4] : "0",
                   D->decimal_places_guint8);

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
  AddSymbolToMap(argv[1] ? argv[1] : "null", argv[2] ? argv[2] : "null",
                 sn_map);

  return 0;
}

static gint get_snmap_name_callback(gpointer data, gint argc, gchar **argv,
                                    gchar **ColName) {
  /* argv[0] is name */
  if (argc != 1)
    return 1;
  if (g_strcmp0(ColName[0], "name") != 0)
    return 1;

  gchar **name_ch = (gchar **)data;
  name_ch[0] = g_strdup(argv[0] ? argv[0] : NULL);
  return 0;
}

static void error_msg(sqlite3 *db) {
  g_fprintf(stderr, "Sqlite3 database error: %s\n", sqlite3_errmsg(db));
  sqlite3_close(db);
  exit(EXIT_FAILURE);
}

static void sqlite_run_cmd(GMutex *mutex, const gchar *db_path, gint (*func)(),
                           void *data, const gchar *cmd) {
  g_mutex_lock(mutex);

  gchar *err_msg = 0;
  sqlite3 *db;

  /* Open the sqlite database file. */
  if (sqlite3_open(db_path, &db) != SQLITE_OK) {
    g_mutex_unlock(mutex);
    error_msg(db);
  }

  /* Run the command. */
  if (sqlite3_exec(db, cmd, func, data, &err_msg) != SQLITE_OK) {
    g_mutex_unlock(mutex);
    error_msg(db);
  }

  /* Close the sqlite database file. */
  if (sqlite3_close(db) != SQLITE_OK) {
    g_mutex_unlock(mutex);
    error_msg(db);
  }

  g_mutex_unlock(mutex);
}

void SqliteProcessing(portfolio_packet *pkg) {
  /* There are two database files, one holds the config info,
  the other holds the stock symbol-name mapping. */
  equity_folder *F = pkg->GetEquityFolderClass();
  meta *D = pkg->GetMetaClass();
  window_data *W = pkg->GetWindowData();

  /* Create the symbolname table if it doesn't exist. */
  gchar *sql_cmd = new_symname_tbl;
  sqlite_run_cmd(&mutexes[SYMBOL_NAME_MAP_SQLITE_MUTEX],
                 D->sqlite_symbol_name_db_path_ch, 0, 0, sql_cmd);

  /* Create the application tables if they don't exist. */
  sql_cmd = g_strconcat(new_app_tbl, new_eqty_tbl, new_bul_tbl, NULL);
  sqlite_run_cmd(&mutexes[SQLITE_MUTEX], D->sqlite_db_path_ch, 0, 0, sql_cmd);
  g_free(sql_cmd);

  /* Reset Equity Folder */
  F->Reset();

  /* Populate class/struct instances with saved data. */
  sql_cmd = "SELECT * FROM app_tbl;";
  sqlite_run_cmd(&mutexes[SQLITE_MUTEX], D->sqlite_db_path_ch, app_callback,
                 pkg, sql_cmd);

  sql_cmd =
      "SELECT * FROM equity;"; /* We always want the next two tables selected
                                  after app_tbl [equity urls and dec places] */
  sqlite_run_cmd(&mutexes[SQLITE_MUTEX], D->sqlite_db_path_ch, equity_callback,
                 F, sql_cmd);

  sql_cmd = "SELECT * FROM bullion;";
  sqlite_run_cmd(&mutexes[SQLITE_MUTEX], D->sqlite_db_path_ch, bullion_callback,
                 pkg, sql_cmd);

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

  /* Sort the equity folder. */
  F->Sort();

  /* Generate the Equity Request URLs. */
  F->GenerateURL(pkg);
}

static gchar *vradic_sqlte_cmd(gint8 num_args_cmd, const gchar *fmt,
                               va_list arg_ptr)
/* construct a series of sql commands from the number of args per command, a
 * format string, and a va_list of the args in the series.
   Each command is the same, with a different set of args.

   For example, this func might return this string:

   "REPLACE INTO bullion (Metal, Ounces, Premium) VALUES('gold', '2.2500',
 '100.2500');REPLACE INTO bullion (Metal, Ounces, Premium) VALUES('silver',
 '75.0000', '4.2500');"

  The REPLACE instruction only allows for one row alteration per instruction.
 */
{
  gushort args_count = 0;
  gchar *sql_cmd_tmp, *first_arg;
  va_list arg_ptr_cpy_1, arg_ptr_cpy_2;
  va_copy(arg_ptr_cpy_1, arg_ptr);
  va_copy(arg_ptr_cpy_2, arg_ptr);

  /* Count the number of args. */
  first_arg = va_arg(arg_ptr_cpy_1, gchar *);
  for (gchar *i = first_arg; i != NULL; i = va_arg(arg_ptr_cpy_1, gchar *))
    args_count++;
  va_end(arg_ptr_cpy_1);

  if (args_count % num_args_cmd)
    return NULL;

  /* Create the long format string [includes all args]. */
  gchar *sql_cmd_fmt = strdup(fmt);
  for (gushort g = 0; g < (args_count / num_args_cmd) - 1; g++) {
    sql_cmd_tmp = g_strconcat(sql_cmd_fmt, fmt, NULL);
    g_free(sql_cmd_fmt);
    sql_cmd_fmt = sql_cmd_tmp;
  }

  /* Get the size of the potential sql_cmd string. */
  gsize len = g_vsnprintf(NULL, 0, sql_cmd_fmt, arg_ptr_cpy_2) + 1;
  va_end(arg_ptr_cpy_2);

  /* Malloc sql_cmd to the string size. */
  gchar *sql_cmd = g_malloc(len);

  /* Create the sql command from format and argument list. */
  g_vsnprintf(sql_cmd, len, sql_cmd_fmt, arg_ptr);
  g_free(sql_cmd_fmt);

  return sql_cmd;
}

void SqliteAppAdd(meta *D, ...)
/* Insert or Replace a variable number of Keyword-Data associations into the
   app_tbl table.

   Take in a meta class pointer, and at least three more args: keyword,
   data_one, and data_two; formatted as character strings.

   There can be any number of Keyword-Data associations, however, each
   association must have these three strings. The last arg must be NULL.

   For example:
   SqliteAppAdd(D,
      "Stock_URL", D->stock_url_ch, "null",
      "URL_KEY", D->curl_key_ch, "null",
      "Main_Win_Sz", main_wdth_value, main_hght_value,
      "Main_Win_Pos", main_xpos_value, main_ypos_value, NULL);
   */
{
  const gchar *fmt = "REPLACE INTO app_tbl (keyword, data_one, data_two) "
                     "VALUES('%s', '%s', '%s');";
  va_list arg_ptr;

  /* Get the sqlite command. */
  va_start(arg_ptr, D);
  gchar *sql_cmd = vradic_sqlte_cmd(3, fmt, arg_ptr);
  va_end(arg_ptr);

  /* Run the command. */
  sqlite_run_cmd(&mutexes[SQLITE_MUTEX], D->sqlite_db_path_ch, 0, 0, sql_cmd);
  g_free(sql_cmd);
}

void SqliteBullionAdd(meta *D, ...)
/* Insert or Replace a variable number of metals into the bullion table.

   Take in a meta class pointer, and at least four more args:
   Metal, Ounces, Premium, Cost formatted as character strings.

   There can be any number of metals, however, each metal must have these four
   strings. The last arg must be NULL.

   For example:
   SqliteBullionAdd(D, "gold", gold_ounces_ch, gold_premium_ch, gold_cost_ch,
   "silver", silver_ounces_ch, silver_premium_ch, silver_cost_ch, NULL);
*/
{

  const gchar *fmt = "REPLACE INTO bullion (Metal, Ounces, Premium, Cost) "
                     "VALUES('%s', '%s', '%s', '%s');";

  va_list arg_ptr;

  /* Get the sqlite command. */
  va_start(arg_ptr, D);
  gchar *sql_cmd = vradic_sqlte_cmd(4, fmt, arg_ptr);
  va_end(arg_ptr);

  /* Run the command. */
  sqlite_run_cmd(&mutexes[SQLITE_MUTEX], D->sqlite_db_path_ch, 0, 0, sql_cmd);
  g_free(sql_cmd);
}

void SqliteEquityAdd(const gchar *symbol, const gchar *shares,
                     const gchar *cost, meta *D) {
  const gchar *fmt =
      "REPLACE INTO equity (Symbol, Shares, Cost) VALUES('%s', '%s', '%s');";

  /* Create sqlite command. */
  gchar *sql_cmd = SnPrint(fmt, symbol, shares, cost);

  /* Run the command. */
  sqlite_run_cmd(&mutexes[SQLITE_MUTEX], D->sqlite_db_path_ch, 0, 0, sql_cmd);
  g_free(sql_cmd);
}

void SqliteEquityRemove(const gchar *symbol, meta *D) {
  const gchar *fmt = "DELETE FROM equity WHERE Symbol = '%s';";

  /* Create sqlite command. */
  gchar *sql_cmd = SnPrint(fmt, symbol);

  /* Run the command. */
  sqlite_run_cmd(&mutexes[SQLITE_MUTEX], D->sqlite_db_path_ch, 0, 0, sql_cmd);
  g_free(sql_cmd);
}

void SqliteEquityRemoveAll(meta *D) {
  /* Drop the equity table then create a new one. */
  gchar *sql_cmd = g_strconcat("DROP TABLE equity;", new_eqty_tbl, NULL);
  sqlite_run_cmd(&mutexes[SQLITE_MUTEX], D->sqlite_db_path_ch, 0, 0, sql_cmd);
  g_free(sql_cmd);
}

symbol_name_map *SqliteGetSNMap(meta *D) {
  symbol_name_map *sn_map = (symbol_name_map *)g_malloc(sizeof(*sn_map));
  sn_map->sn_container_arr = g_malloc(1);
  sn_map->size = 0;
  sn_map->hash_table = NULL;

  /* Run the command. */
  gchar *sql_cmd = "SELECT * FROM symbolname;";
  sqlite_run_cmd(&mutexes[SYMBOL_NAME_MAP_SQLITE_MUTEX],
                 D->sqlite_symbol_name_db_path_ch, symbol_name_callback, sn_map,
                 sql_cmd);

  if (sn_map->size == 0) {
    g_free(sn_map->sn_container_arr);
    g_free(sn_map);
    sn_map = NULL;
  } else {
    /* Create a hashing table of the sn_map. */
    CreateHashTable(sn_map);
  }

  /* Return sn_map if size > 0, if size = 0 return NULL */
  return sn_map;
}

gchar *SqliteGetSNMapName(const gchar *symbol_ch, meta *D) {
  /* Take in a symbol char string, the meta class and return the name string, if
     found, if not found return NULL.  Must free return value. */
  const gchar *fmt = "SELECT name FROM symbolname WHERE symbol = '%s';";
  gchar *name_ch = NULL;

  /* Create the command. */
  gchar *sql_cmd = SnPrint(fmt, symbol_ch);

  /* Run the command. */
  sqlite_run_cmd(&mutexes[SYMBOL_NAME_MAP_SQLITE_MUTEX],
                 D->sqlite_symbol_name_db_path_ch, get_snmap_name_callback,
                 &name_ch, sql_cmd);
  g_free(sql_cmd);
  return name_ch;
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

    /* If we find an ''' character, ASCII/UTF8 decimal code 39 */
    if (s[0][i] == 39) {

      /* Increase the character array by one character. */
      gchar *tmp =
          g_realloc(s[0], (g_utf8_strlen(s[0], -1) + 1 + 1) * sizeof(gchar));
      s[0] = tmp;

      /* Read each character from null back to that character */
      for (gushort j = g_utf8_strlen(s[0], -1); j >= i; j--)
        /* Shift the array one character to the right [duplicate the character]
         */
        s[0][j + 1] = s[0][j];

      /* Because we have a duplicate we need to skip the next increment of i */
      i++;
    }
  }
}

typedef struct {
  symbol_name_map *map;
  meta *metadata;
} meta_map_container;

static gchar *add_map_to_db_cmd(symbol_name_map *sn_map) {
  gchar *tmp, *sql_cmd_tmp;
  gchar *sql_cmd = g_strdup("INSERT INTO symbolname (symbol, name) VALUES");
  const gchar *fmt = "('%s', '%s'),";

  for (gushort g = 0; g < sn_map->size; g++) {
    /* Insert an escape apostrophy into the string if needed. */
    escape_apostrophy(&sn_map->sn_container_arr[g]->security_name);

    /* Create the sub-string to add to the sql command. */
    tmp = SnPrint(fmt, sn_map->sn_container_arr[g]->symbol,
                  sn_map->sn_container_arr[g]->security_name);

    /* Concatenate the sub-string to the end of the sql command */
    sql_cmd_tmp = g_strconcat(sql_cmd, tmp, NULL);
    g_free(tmp);
    g_free(sql_cmd);
    sql_cmd = sql_cmd_tmp;
  }

  /* Replace the last comma, in the command, with a semi-colon. */
  gchar *ch = g_utf8_strrchr(sql_cmd, -1, (gunichar)',');
  if (ch)
    *ch = ';';

  return sql_cmd;
}

static gpointer add_mapping_to_database_thd(gpointer data) {
  meta_map_container *mmc = (meta_map_container *)data;
  symbol_name_map *sn_map = mmc->map;
  meta *D = mmc->metadata;

  /* Drop the symbolname table and create a new one. */
  gchar *sql_cmd = g_strconcat("DROP TABLE symbolname;", new_symname_tbl, NULL);
  sqlite_run_cmd(&mutexes[SYMBOL_NAME_MAP_SQLITE_MUTEX],
                 D->sqlite_symbol_name_db_path_ch, 0, 0, sql_cmd);
  g_free(sql_cmd);

  /* Create the command. */
  sql_cmd = add_map_to_db_cmd(sn_map);

  /* Perform the command. */
  D->snmap_db_busy_bool = TRUE;
  sqlite_run_cmd(&mutexes[SYMBOL_NAME_MAP_SQLITE_MUTEX],
                 D->sqlite_symbol_name_db_path_ch, 0, 0, sql_cmd);
  g_free(sql_cmd);
  D->snmap_db_busy_bool = FALSE;

  /* Remove the duplicate map from memory. */
  SNMapDestruct(sn_map);
  g_free(sn_map);

  /* Don't Free the member pointers */
  g_free(mmc);
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