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

#include "../financials.h"

int equity_callback(void *data, int argc, char **argv, char **ColName) {
    /* argv[0] is id, argv[1] is symbol, argv[2] is shares */
    if ( argc != 3 ) return 1;
    if ( strcmp( ColName[0], "Id") != 0 ) return 1;
    if ( strcmp( ColName[1], "Symbol") != 0 ) return 1;
    if ( strcmp( ColName[2], "Shares") != 0 ) return 1;

    equity_folder *F = (equity_folder*)data;
    AddStock( F );

    /* Add The Stock Symbol To the Folder */
    free( F->Equity[ F->size ]->symbol_stock_ch ); 

    F->Equity[ F->size ]->symbol_stock_ch = (char*) malloc( strlen( argv[1] ? argv[1] : "NULL" )+1 );
    strcpy( F->Equity[ F->size ]->symbol_stock_ch, argv[1] ? argv[1] : "NULL" );

    /* Add The Shares To the Folder */
    *F->Equity[ F->size ]->num_shares_stock_int = (unsigned int)strtol( argv[2] ? argv[2] : "0", NULL, 10 );
    F->size++;

    return 0;
}

int cash_callback(void *data, int argc, char **argv, char **ColName) {
    pthread_mutex_lock( &mutex_working[0] );

    /* argv[0] is id, argv[1] is value */
    if ( argc != 2 ) return 1;
    if ( strcmp( ColName[0], "Id") != 0 ) return 1;
    if ( strcmp( ColName[1], "Value") != 0 ) return 1;

    meta *mdata = (meta*)data;

    *mdata->cash_f = mdata->StrToDoub( argv[1] ? argv[1] : "0" );
    free( mdata->cash_ch );

    /* To make sure it's formatted correctly. */
    mdata->cash_ch = mdata->DoubToStr( mdata->cash_f );

    pthread_mutex_unlock( &mutex_working[0] );
    return 0;
}

int bullion_callback(void *data, int argc, char **argv, char **ColName) {
    /* argv[0] is Id, argv[1] is Metal, argv[2] is Ounces, argv[3] is Premium */
    pthread_mutex_lock( &mutex_working[0] );

    if ( argc != 4 ) return 1;
    if ( strcmp( ColName[0], "Id") != 0 ) return 1;
    if ( strcmp( ColName[1], "Metal") != 0 ) return 1;
    if ( strcmp( ColName[2], "Ounces") != 0 ) return 1;
    if ( strcmp( ColName[3], "Premium") != 0 ) return 1;

    metal *m = (metal*)data;

    if( strcasecmp( argv[1], "gold" ) == 0 ){
        *m->Gold->ounce_f = m->Gold->StrToDoub ( argv[2] ? argv[2] : "0" );

        free( m->Gold->ounce_ch );
        m->Gold->ounce_ch = strdup ( argv[2] ? argv[2] : "0" );
        /* To make sure it's formatted correctly. */
        FormatStr( m->Gold->ounce_ch );
        
        *m->Gold->premium_f = m->Gold->StrToDoub ( argv[3] ? argv[3] : "0" );

        free( m->Gold->premium_ch );
        /* To make sure it's formatted correctly. */
        m->Gold->premium_ch = m->Gold->DoubToStr( m->Gold->premium_f );
    }

    if( strcasecmp( argv[1], "silver" ) == 0 ){
        *m->Silver->ounce_f = m->Silver->StrToDoub ( argv[2] ? argv[2] : "0" );

        free( m->Silver->ounce_ch );
        m->Silver->ounce_ch = strdup ( argv[2] ? argv[2] : "0" );
        /* To make sure it's formatted correctly. */
        FormatStr( m->Silver->ounce_ch );
        
        *m->Silver->premium_f = m->Silver->StrToDoub ( argv[3] ? argv[3] : "0" );

        free( m->Silver->premium_ch );
        /* To make sure it's formatted correctly. */
        m->Silver->premium_ch = m->Silver->DoubToStr( m->Silver->premium_f );
    }

    pthread_mutex_unlock( &mutex_working[0] );
    return 0;
}

int api_callback(void *data, int argc, char **argv, char **ColName) {
    /* argv[0] is Id, argv[1] is Keyword, argv[2] is Data */
    if ( argc != 3 ) return 1;
    if ( strcmp( ColName[0], "Id") != 0 ) return 1;
    if ( strcmp( ColName[1], "Keyword") != 0 ) return 1;
    if ( strcmp( ColName[2], "Data") != 0 ) return 1;

    meta *mdata = (meta*)data;
    if ( strcasecmp( argv[1], "Stock_URL") == 0){
        free ( mdata->stock_url );
        mdata->stock_url = strdup( argv[2] ? argv[2] : FINNHUB_URL );
    }

    if ( strcasecmp( argv[1], "URL_KEY") == 0){
        free ( mdata->curl_key );
        mdata->curl_key = strdup( argv[2] ? argv[2] : FINNHUB_URL_TOKEN );
    }

    if ( strcasecmp( argv[1], "Updates_Per_Min") == 0){
        *mdata->updates_per_min_f = strtod( argv[2] ? argv[2] : "6", NULL );
    }

    if ( strcasecmp( argv[1], "Updates_Hours") == 0){
        *mdata->updates_hours_f = strtod( argv[2] ? argv[2] : "1", NULL );
    }

    return 0;
}

int main_wndwsz_callback(void *data, int argc, char **argv, char **ColName) {
    /* argv[0] is Id, argv[1] is height, argv[2] is width */
    if ( argc != 3 ) return 1;
    if ( strcmp( ColName[0], "Id") != 0 ) return 1;
    if ( strcmp( ColName[1], "Height") != 0 ) return 1;
    if ( strcmp( ColName[2], "Width") != 0 ) return 1;

    window_data *window = (window_data*)data;
    window->main_height = (int)strtol( argv[1] ? argv[1] : "0", NULL, 10 );
    window->main_width = (int)strtol( argv[2] ? argv[2] : "0", NULL, 10 );
    
    return 0;
}

int main_wndwpos_callback(void *data, int argc, char **argv, char **ColName) {
    /* argv[0] is Id, argv[1] is X, argv[2] is Y */
    if ( argc != 3 ) return 1;
    if ( strcmp( ColName[0], "Id") != 0 ) return 1;
    if ( strcmp( ColName[1], "X") != 0 ) return 1;
    if ( strcmp( ColName[2], "Y") != 0 ) return 1;

    window_data *window = (window_data*)data;
    window->main_x_pos = (int)strtol( argv[1] ? argv[1] : "0", NULL, 10 );
    window->main_y_pos = (int)strtol( argv[2] ? argv[2] : "0", NULL, 10 );
    
    return 0;
}

int rsi_wndwsz_callback(void *data, int argc, char **argv, char **ColName) {
    /* argv[0] is Id, argv[1] is height, argv[2] is width */
    if ( argc != 3 ) return 1;
    if ( strcmp( ColName[0], "Id") != 0 ) return 1;
    if ( strcmp( ColName[1], "Height") != 0 ) return 1;
    if ( strcmp( ColName[2], "Width") != 0 ) return 1;

    window_data *window = (window_data*)data;
    window->rsi_height = (int)strtol( argv[1] ? argv[1] : "0", NULL, 10 );
    window->rsi_width = (int)strtol( argv[2] ? argv[2] : "0", NULL, 10 );
    
    return 0;
}

int rsi_wndwpos_callback(void *data, int argc, char **argv, char **ColName) {
    /* argv[0] is Id, argv[1] is X, argv[2] is Y */
    if ( argc != 3 ) return 1;
    if ( strcmp( ColName[0], "Id") != 0 ) return 1;
    if ( strcmp( ColName[1], "X") != 0 ) return 1;
    if ( strcmp( ColName[2], "Y") != 0 ) return 1;

    window_data *window = (window_data*)data;
    window->rsi_x_pos = (int)strtol( argv[1] ? argv[1] : "0", NULL, 10 );
    window->rsi_y_pos = (int)strtol( argv[2] ? argv[2] : "0", NULL, 10 );
    
    return 0;
}

void ErrorMsg( sqlite3 *db ){
    fprintf( stderr, "Cannot open sqlite3 database: %s\n", sqlite3_errmsg( db ) );
    sqlite3_close(db);
    exit ( EXIT_FAILURE );
}

void SqliteProcessing (equity_folder* F, metal *M, meta *D){
    char    *err_msg = 0;
    sqlite3 *db;

    /* Open the sqlite database file. */
    if ( sqlite3_open( D->sqlite_db_path_ch, &db) != SQLITE_OK ) ErrorMsg( db );

    /* Create the equity table if it doesn't already exist. */
    char *sql_cmd = "CREATE TABLE IF NOT EXISTS equity(Id INTEGER PRIMARY KEY, Symbol TEXT NOT NULL, Shares TEXT NOT NULL);";
    if ( sqlite3_exec(db, sql_cmd, 0, 0, &err_msg) != SQLITE_OK ) ErrorMsg( db );

    /* Create the bullion table if it doesn't already exist. */
    sql_cmd = "CREATE TABLE IF NOT EXISTS bullion(Id INTEGER PRIMARY KEY, Metal TEXT NOT NULL, Ounces TEXT NOT NULL, Premium TEXT NOT NULL);";
    if ( sqlite3_exec(db, sql_cmd, 0, 0, &err_msg) != SQLITE_OK ) ErrorMsg( db );

    /* Create the cash table if it doesn't already exist. */
    sql_cmd = "CREATE TABLE IF NOT EXISTS cash(Id INTEGER PRIMARY KEY, Value TEXT NOT NULL);";
    if ( sqlite3_exec(db, sql_cmd, 0, 0, &err_msg) != SQLITE_OK ) ErrorMsg( db );

    /* Create the apidata table if it doesn't already exist. */
    sql_cmd = "CREATE TABLE IF NOT EXISTS apidata(Id INTEGER PRIMARY KEY, Keyword TEXT NOT NULL, Data TEXT NOT NULL);";
    if ( sqlite3_exec(db, sql_cmd, 0, 0, &err_msg) != SQLITE_OK ) ErrorMsg( db );

    /* Create the mainwindowsize table if it doesn't already exist. */
    sql_cmd = "CREATE TABLE IF NOT EXISTS mainwindowsize(Id INTEGER PRIMARY KEY, Height TEXT NOT NULL, Width TEXT NOT NULL);";
    if ( sqlite3_exec(db, sql_cmd, 0, 0, &err_msg) != SQLITE_OK ) ErrorMsg( db );

    /* Create the mainwindowpos table if it doesn't already exist. */
    sql_cmd = "CREATE TABLE IF NOT EXISTS mainwindowpos(Id INTEGER PRIMARY KEY, X TEXT NOT NULL, Y TEXT NOT NULL);";
    if ( sqlite3_exec(db, sql_cmd, 0, 0, &err_msg) != SQLITE_OK ) ErrorMsg( db );

    /* Create the rsiwindowsize table if it doesn't already exist. */
    sql_cmd = "CREATE TABLE IF NOT EXISTS rsiwindowsize(Id INTEGER PRIMARY KEY, Height TEXT NOT NULL, Width TEXT NOT NULL);";
    if ( sqlite3_exec(db, sql_cmd, 0, 0, &err_msg) != SQLITE_OK ) ErrorMsg( db );

    /* Create the rsiwindowpos table if it doesn't already exist. */
    sql_cmd = "CREATE TABLE IF NOT EXISTS rsiwindowpos(Id INTEGER PRIMARY KEY, X TEXT NOT NULL, Y TEXT NOT NULL);";
    if ( sqlite3_exec(db, sql_cmd, 0, 0, &err_msg) != SQLITE_OK ) ErrorMsg( db );

    /* Reset Equity Folder */
    ResetEquity ( F );

    /* Populate class/struct instances with saved data. */
    sql_cmd = "SELECT * FROM equity;";
    if ( sqlite3_exec(db, sql_cmd, equity_callback, F, &err_msg) != SQLITE_OK ) ErrorMsg( db );

    sql_cmd = "SELECT * FROM bullion;";
    if ( sqlite3_exec(db, sql_cmd, bullion_callback, M, &err_msg) != SQLITE_OK ) ErrorMsg( db );

    sql_cmd = "SELECT * FROM cash;";
    if ( sqlite3_exec(db, sql_cmd, cash_callback, D, &err_msg) != SQLITE_OK ) ErrorMsg( db );

    sql_cmd = "SELECT * FROM apidata;";
    if ( sqlite3_exec(db, sql_cmd, api_callback, D, &err_msg) != SQLITE_OK ) ErrorMsg( db );

    sql_cmd = "SELECT * FROM mainwindowsize;";
    if ( sqlite3_exec(db, sql_cmd, main_wndwsz_callback, &WindowStruct, &err_msg) != SQLITE_OK ) ErrorMsg( db );

    sql_cmd = "SELECT * FROM mainwindowpos;";
    if ( sqlite3_exec(db, sql_cmd, main_wndwpos_callback, &WindowStruct, &err_msg) != SQLITE_OK ) ErrorMsg( db );

    sql_cmd = "SELECT * FROM rsiwindowsize;";
    if ( sqlite3_exec(db, sql_cmd, rsi_wndwsz_callback, &WindowStruct, &err_msg) != SQLITE_OK ) ErrorMsg( db );

    sql_cmd = "SELECT * FROM rsiwindowpos;";
    if ( sqlite3_exec(db, sql_cmd, rsi_wndwpos_callback, &WindowStruct, &err_msg) != SQLITE_OK ) ErrorMsg( db );

    if( WindowStruct.main_width == 0 || WindowStruct.main_height == 0 ){
        /* The Original Production Size, if never run before */
        WindowStruct.main_width = 900;
        WindowStruct.main_height = 850;
    }

    if( WindowStruct.main_x_pos == 0 && WindowStruct.main_y_pos == 0 ){
        WindowStruct.main_x_pos = 0;
        WindowStruct.main_y_pos = 32;
    }

    if( WindowStruct.rsi_width == 0 || WindowStruct.rsi_height == 0 ){
        /* The Original Production Size, if never run before */
        WindowStruct.rsi_width = 925;
        WindowStruct.rsi_height = 700;
    }

    if( WindowStruct.rsi_x_pos == 0 && WindowStruct.rsi_y_pos == 0 ){
        WindowStruct.rsi_x_pos = 0;
        WindowStruct.rsi_y_pos = 32;
    }

    /* Close the sqlite database file. */
    sqlite3_close( db );

    /* Sort the equity folder. */
    qsort(&F->Equity[0], (size_t)F->size, sizeof(stock*), AlphaAsc);
}

void SqliteAddEquity (char *symbol, char *shares, equity_folder *F){
    size_t  len;
    char    *err_msg = 0;
    sqlite3 *db;

    /* Open the sqlite database file. */
    if ( sqlite3_open( MetaData->sqlite_db_path_ch, &db) != SQLITE_OK ) ErrorMsg( db );

    /* Delete entry if already exists, then insert entry. */
    len = strlen("DELETE FROM equity WHERE Symbol = '';") + strlen( symbol ) + 1;
    char *sql_cmd = (char*) malloc( len );
    snprintf( sql_cmd, len, "DELETE FROM equity WHERE Symbol = '%s';", symbol );
    if ( sqlite3_exec(db, sql_cmd, 0, 0, &err_msg) != SQLITE_OK ) ErrorMsg( db );
    free( sql_cmd );

    len = strlen("INSERT INTO equity VALUES(null, '', '');") + strlen( symbol ) + strlen( shares ) + 1;
    sql_cmd = (char*) malloc( len );
    snprintf( sql_cmd, len, "INSERT INTO equity VALUES(null, '%s', '%s');", symbol, shares );
    if ( sqlite3_exec(db, sql_cmd, 0, 0, &err_msg) != SQLITE_OK ) ErrorMsg( db );
    free( sql_cmd );
    
    /* Reset Equity Folder */
    ResetEquity ( F );

    sql_cmd = "SELECT * FROM equity;";
    if ( sqlite3_exec(db, sql_cmd, equity_callback, F, &err_msg) != SQLITE_OK ) ErrorMsg( db );

    /* Close the sqlite database file. */
    sqlite3_close( db );

    /* Sort the equity folder. */
    qsort(&F->Equity[0], (size_t)F->size, sizeof(stock*), AlphaAsc);
}

void SqliteAddBullion (char *metal_name, char *ounces, char *premium, metal *M){
    size_t  len;
    char    *err_msg = 0;
    sqlite3 *db;

    /* Open the sqlite database file. */
    if ( sqlite3_open( MetaData->sqlite_db_path_ch, &db) != SQLITE_OK ) ErrorMsg( db );

    /* Delete entry if already exists, then insert entry. */
    len = strlen("DELETE FROM bullion WHERE Metal = '';") + strlen( metal_name ) + 1;
    char *sql_cmd = (char*) malloc( len );
    snprintf( sql_cmd, len, "DELETE FROM bullion WHERE Metal = '%s';", metal_name);
    if ( sqlite3_exec(db, sql_cmd, 0, 0, &err_msg) != SQLITE_OK ) ErrorMsg( db );
    free( sql_cmd );
    len = strlen("INSERT INTO bullion VALUES(null, '', '', '');") + strlen( metal_name ) + strlen( ounces ) + strlen( premium ) + 1;
    sql_cmd = (char*) malloc( len );
    snprintf( sql_cmd, len, "INSERT INTO bullion VALUES(null, '%s', '%s', '%s');", metal_name, ounces, premium );
    if ( sqlite3_exec(db, sql_cmd, 0, 0, &err_msg) != SQLITE_OK ) ErrorMsg( db );
    free( sql_cmd );

    /* Update the metal handle. */
    sql_cmd = "SELECT * FROM bullion;";
    if ( sqlite3_exec(db, sql_cmd, bullion_callback, M, &err_msg) != SQLITE_OK ) ErrorMsg( db );

    /* Close the sqlite database file. */
    sqlite3_close( db );
}

void SqliteAddCash (char *value, meta *D){
    size_t  len;
    char    *err_msg = 0;
    sqlite3 *db;

    /* Open the sqlite database file. */
    if ( sqlite3_open( D->sqlite_db_path_ch, &db) != SQLITE_OK ) ErrorMsg( db );

    /* Delete entry if already exists, then insert entry. */
    len = strlen("DELETE FROM cash WHERE Id = 1;") + 1;
    char *sql_cmd = (char*) malloc( len );
    snprintf( sql_cmd, len, "DELETE FROM cash WHERE Id = 1;");
    if ( sqlite3_exec(db, sql_cmd, 0, 0, &err_msg) != SQLITE_OK ) ErrorMsg( db );
    free( sql_cmd );

    len = strlen("INSERT INTO cash VALUES(1, '');") + strlen( value ) + 1;
    sql_cmd = (char*) malloc( len );
    snprintf( sql_cmd, len, "INSERT INTO cash VALUES(1, '%s');", value );
    if ( sqlite3_exec(db, sql_cmd, 0, 0, &err_msg) != SQLITE_OK ) ErrorMsg( db );
    free( sql_cmd );
    
    /* Update the cash value in the meta class. */
    sql_cmd = "SELECT * FROM cash;";
    if ( sqlite3_exec(db, sql_cmd, cash_callback, D, &err_msg) != SQLITE_OK ) ErrorMsg( db );

    /* Close the sqlite database file. */
    sqlite3_close( db );
}

void SqliteAddAPIData (char *keyword, char *data, meta *D){
    size_t  len;
    char    *err_msg = 0;
    sqlite3 *db;

    /* Open the sqlite database file. */
    if ( sqlite3_open( D->sqlite_db_path_ch, &db) != SQLITE_OK ) ErrorMsg( db );

    /* Delete entry if already exists, then insert entry. */
    len = strlen("DELETE FROM apidata WHERE Keyword = '';") + strlen( keyword ) + 1;
    char *sql_cmd = (char*) malloc( len );
    snprintf( sql_cmd, len, "DELETE FROM apidata WHERE Keyword = '%s';", keyword);
    if ( sqlite3_exec(db, sql_cmd, 0, 0, &err_msg) != SQLITE_OK ) ErrorMsg( db );
    free( sql_cmd );

    len = strlen("INSERT INTO apidata VALUES(null, '', '');") + strlen( keyword ) + strlen( data ) + 1;
    sql_cmd = (char*) malloc( len );
    snprintf( sql_cmd, len, "INSERT INTO apidata VALUES(null, '%s', '%s');", keyword, data );
    if ( sqlite3_exec(db, sql_cmd, 0, 0, &err_msg) != SQLITE_OK ) ErrorMsg( db );
    free( sql_cmd );
    
    /* Update the API data in the MetaData class. */
    sql_cmd = "SELECT * FROM apidata;";
    if ( sqlite3_exec(db, sql_cmd, api_callback, D, &err_msg) != SQLITE_OK ) ErrorMsg( db );

    /* Close the sqlite database file. */
    sqlite3_close( db );
}

void SqliteRemoveEquity (char *symbol, equity_folder *F){
    size_t  len;
    char    *err_msg = 0;
    sqlite3 *db;

    /* Open the sqlite database file. */
    if ( sqlite3_open( MetaData->sqlite_db_path_ch, &db) != SQLITE_OK ) ErrorMsg( db );

    /* Delete entry if already exists. */
    len = strlen("DELETE FROM equity WHERE Symbol = '';") + strlen( symbol ) + 1;
    char *sql_cmd = (char*) malloc( len );
    snprintf( sql_cmd, len, "DELETE FROM equity WHERE Symbol = '%s';", symbol );
    if ( sqlite3_exec(db, sql_cmd, 0, 0, &err_msg) != SQLITE_OK ) ErrorMsg( db );
    free( sql_cmd );
    
    /* Reset Equity Folder */
    ResetEquity ( F );

    sql_cmd = "SELECT * FROM equity;";
    if ( sqlite3_exec(db, sql_cmd, equity_callback, F, &err_msg) != SQLITE_OK ) ErrorMsg( db );

    /* Close the sqlite database file. */
    sqlite3_close( db );

    /* Sort the equity folder. */
    qsort(&F->Equity[0], (size_t)F->size, sizeof(stock*), AlphaAsc);
}

void SqliteRemoveAllEquity (equity_folder *F){
    char    *err_msg = 0;
    sqlite3 *db;

    /* Open the sqlite database file. */
    if ( sqlite3_open( MetaData->sqlite_db_path_ch, &db) != SQLITE_OK ) ErrorMsg( db );

    /* Drop the equity table and create a new one. */
    char *sql_cmd = "DROP TABLE equity;";
    if ( sqlite3_exec(db, sql_cmd, 0, 0, &err_msg) != SQLITE_OK ) ErrorMsg( db );

    sql_cmd = "CREATE TABLE IF NOT EXISTS equity(Id INTEGER PRIMARY KEY, Symbol TEXT NOT NULL, Shares TEXT NOT NULL);";
    if ( sqlite3_exec(db, sql_cmd, 0, 0, &err_msg) != SQLITE_OK ) ErrorMsg( db );

    /* Close the sqlite database file. */
    sqlite3_close( db );
    
    /* Reset Equity Folder */
    ResetEquity ( F );
}

void SqliteChangeMainWindowSize (int width, int height){
    size_t  len;
    char    *err_msg = 0;
    sqlite3 *db;

    /* Open the sqlite database file. */
    if ( sqlite3_open( MetaData->sqlite_db_path_ch, &db) != SQLITE_OK ) ErrorMsg( db );

    /* Delete entry if already exists, then insert entry. */
    if ( sqlite3_exec(db, "DELETE FROM mainwindowsize WHERE Id = 1;", 0, 0, &err_msg) != SQLITE_OK ) ErrorMsg( db );

    len = strlen("INSERT INTO mainwindowsize VALUES(1, '', '');") + 64;
    char *sql_cmd = (char*) malloc( len );
    snprintf( sql_cmd, len, "INSERT INTO mainwindowsize VALUES(1, '%d', '%d');", height, width );
    if ( sqlite3_exec(db, sql_cmd, 0, 0, &err_msg) != SQLITE_OK ) ErrorMsg( db );
    free( sql_cmd );

    /* Close the sqlite database file. */
    sqlite3_close( db );
}

void SqliteChangeMainWindowPos (int x, int y){
    size_t  len;
    char    *err_msg = 0;
    sqlite3 *db;

    /* Open the sqlite database file. */
    if ( sqlite3_open( MetaData->sqlite_db_path_ch, &db) != SQLITE_OK ) ErrorMsg( db );

    /* Delete entry if already exists, then insert entry. */
    if ( sqlite3_exec(db, "DELETE FROM mainwindowpos WHERE Id = 1;", 0, 0, &err_msg) != SQLITE_OK ) ErrorMsg( db );

    len = strlen("INSERT INTO mainwindowpos VALUES(1, '', '');") + 64;
    char *sql_cmd = (char*) malloc( len );
    snprintf( sql_cmd, len, "INSERT INTO mainwindowpos VALUES(1, '%d', '%d');", x, y );
    if ( sqlite3_exec(db, sql_cmd, 0, 0, &err_msg) != SQLITE_OK ) ErrorMsg( db );
    free( sql_cmd );

    /* Close the sqlite database file. */
    sqlite3_close( db );
}

void SqliteChangeRSIWindowSize (int width, int height){
    size_t  len;
    char    *err_msg = 0;
    sqlite3 *db;

    /* Open the sqlite database file. */
    if ( sqlite3_open( MetaData->sqlite_db_path_ch, &db) != SQLITE_OK ) ErrorMsg( db );

    /* Delete entry if already exists, then insert entry. */
    if ( sqlite3_exec(db, "DELETE FROM rsiwindowsize WHERE Id = 1;", 0, 0, &err_msg) != SQLITE_OK ) ErrorMsg( db );

    len = strlen("INSERT INTO rsiwindowsize VALUES(1, '', '');") + 64;
    char *sql_cmd = (char*) malloc( len );
    snprintf( sql_cmd, len, "INSERT INTO rsiwindowsize VALUES(1, '%d', '%d');", height, width );
    if ( sqlite3_exec(db, sql_cmd, 0, 0, &err_msg) != SQLITE_OK ) ErrorMsg( db );
    free( sql_cmd );

    /* Close the sqlite database file. */
    sqlite3_close( db );
}

void SqliteChangeRSIWindowPos (int x, int y){
    size_t  len;
    char    *err_msg = 0;
    sqlite3 *db;

    /* Open the sqlite database file. */
    if ( sqlite3_open( MetaData->sqlite_db_path_ch, &db) != SQLITE_OK ) ErrorMsg( db );

    /* Delete entry if already exists, then insert entry. */
    if ( sqlite3_exec(db, "DELETE FROM rsiwindowpos WHERE Id = 1;", 0, 0, &err_msg) != SQLITE_OK ) ErrorMsg( db );

    len = strlen("INSERT INTO rsiwindowpos VALUES(1, '', '');") + 64;
    char *sql_cmd = (char*) malloc( len );
    snprintf( sql_cmd, len, "INSERT INTO rsiwindowpos VALUES(1, '%d', '%d');", x, y );
    if ( sqlite3_exec(db, sql_cmd, 0, 0, &err_msg) != SQLITE_OK ) ErrorMsg( db );
    free( sql_cmd );

    /* Close the sqlite database file. */
    sqlite3_close( db );
}