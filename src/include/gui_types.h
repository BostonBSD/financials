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
#ifndef GUI_TYPES_HEADER_H
#define GUI_TYPES_HEADER_H

#include <glib.h>

/* Structs and Enums */
typedef struct { /* A mapping between sec symbols and sec names */
  gchar *symbol;
  gchar *security_name;
} symbol_to_security_name_container;

typedef struct { /* A handle to the symbol-name mapping array. */
  symbol_to_security_name_container **sn_container_arr;
  GHashTable *hash_table;
  gushort size;
} symbol_name_map;

typedef struct { /* A container to pass a string and a font name between
                    threads. */
  gchar *string;
  gchar *font;
} string_font;

/* Window Signals */
enum { GUI_MAIN_WINDOW, GUI_HISTORY_WINDOW };

/* Completion Set Signals */
enum { GUI_COMPLETION_SECURITY, GUI_COMPLETION_HISTORY };

/* Main TreeView Data Column Type */
enum { GUI_COLUMN_DEFAULT, GUI_COLUMN_PRIMARY };

/* Main TreeView Data Column Numbers */
enum {
  GUI_TYPE,
  GUI_SYMBOL,
  GUI_COLUMN_ONE,
  GUI_COLUMN_TWO,
  GUI_COLUMN_THREE,
  GUI_COLUMN_FOUR,
  GUI_COLUMN_FIVE,
  GUI_COLUMN_SIX,
  GUI_COLUMN_SEVEN,
  GUI_COLUMN_EIGHT,
  GUI_COLUMN_NINE,
  GUI_COLUMN_TEN,
  GUI_COLUMN_ELEVEN,
  GUI_N_COLUMNS
};

/* History TreeView Column Numbers */
enum {
  HISTORY_COLUMN_ONE,
  HISTORY_COLUMN_TWO,
  HISTORY_COLUMN_THREE,
  HISTORY_COLUMN_FOUR,
  HISTORY_COLUMN_FIVE,
  HISTORY_COLUMN_SIX,
  HISTORY_COLUMN_SEVEN,
  HISTORY_COLUMN_EIGHT,
  HISTORY_COLUMN_NINE,
  HISTORY_COLUMN_TEN,
  HISTORY_COLUMN_ELEVEN,
  HISTORY_N_COLUMNS
};

/* Callback/Thread Handler Index Signals */
typedef enum {
  MAIN_FETCH_BTN,
  MAIN_EXIT,
  SECURITY_TOGGLE_BTN,
  SECURITY_OK_BTN,
  SECURITY_COMBO_BOX,
  SECURITY_CURSOR_MOVE,
  BUL_TOGGLE_BTN,
  BUL_OK_BTN,
  BUL_COMBO_BOX,
  BUL_CURSOR_MOVE,
  CASH_TOGGLE_BTN,
  CASH_OK_BTN,
  CASH_CURSOR_MOVE,
  API_TOGGLE_BTN,
  API_OK_BTN,
  API_CURSOR_MOVE,
  HISTORY_TOGGLE_BTN,
  HISTORY_FETCH_BTN,
  HISTORY_CURSOR_MOVE,
  ABOUT_TOGGLE_BTN,
  HOTKEYS_TOGGLE_BTN,
  PREF_TOGGLE_BTN,
  PREF_SYMBOL_UPDATE_BTN,
  SIGNAL_NUM
} cb_signal;

#endif /* GUI_TYPES_HEADER_H */