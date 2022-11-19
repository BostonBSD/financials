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
#ifndef GUI_TYPES_HEADER_H
#define GUI_TYPES_HEADER_H

/* Structs and Enums */
typedef struct {                    /* A mapping between sec symbols and sec names */
  char* symbol;
  char* security_name;
} symbol_to_security_name_container;

typedef struct {                    /* A handle to the symbol-name mapping array. */
  symbol_to_security_name_container** sn_container_arr; 
  int size;
} symbol_name_map;

typedef struct {                    /* A container to hold the type of row and symbol, on a right click */
  char* type;
  char* symbol;
} right_click_container;

typedef struct {
  int main_height;
  int main_width;
  int main_x_pos;
  int main_y_pos;
  int rsi_height;
  int rsi_width;
  int rsi_x_pos;
  int rsi_y_pos;
} window_data;

/* Window Signals */
enum {
  GUI_MAIN_WINDOW,
  GUI_RSI_WINDOW
};

/* TreeView Data Column Type */
enum {
  GUI_COLUMN_DEFAULT,
  GUI_COLUMN_PRIMARY
};

/* TreeView Data Column Numbers */
enum {
  GUI_TYPE,
  GUI_SYMBOL,
  GUI_SHARES_OUNCES,
  GUI_PREMIUM,
  GUI_PRICE,
  GUI_TOTAL,
  GUI_EXTRA_ONE,
  GUI_EXTRA_TWO,
  GUI_EXTRA_THREE,
  GUI_EXTRA_FOUR,
  GUI_EXTRA_FIVE,
  GUI_EXTRA_SIX,
  GUI_FOREGROUND_COLOR,
  GUI_N_COLUMNS
};

/* RSI TreeView Column Numbers */
enum {
  RSI_DATE,
  RSI_PRICE,
  RSI_HIGH,
  RSI_LOW,
  RSI_OPENING,
  RSI_PREV_CLOSING,
  RSI_CHANGE,
  RSI_CHANGE_PERCENT,
  RSI_VOLUME,
  RSI_RSI,
  RSI_INDICATOR,
  RSI_FOREGROUND_COLOR,
  RSI_N_COLUMNS
};

/* Callback/Thread Handler Index Signals */
typedef enum {
    MAIN_FETCH_BTN,
    MAIN_CLOCK,
    MAIN_TIME_CLOSE_INDICATOR,
    MAIN_EXIT, 
    EQUITY_TOGGLE_BTN,
    EQUITY_OK_BTN,
    EQUITY_CANCEL_BTN,
    EQUITY_SWITCH,
    EQUITY_COMBO_BOX,
    EQUITY_CURSOR_MOVE,
    BUL_TOGGLE_BTN,
    BUL_OK_BTN,
    BUL_CANCEL_BTN,
    BUL_CURSOR_MOVE,
    CASH_TOGGLE_BTN,
    CASH_OK_BTN,
    CASH_CANCEL_BTN,
    CASH_CURSOR_MOVE,
    API_TOGGLE_BTN,
    API_OK_BTN,
    API_CANCEL_BTN,
    API_CURSOR_MOVE,
    RSI_TOGGLE_BTN,
    RSI_FETCH_BTN,
    RSI_CURSOR_MOVE,
    COMPLETION,
    ABOUT_TOGGLE_BTN,
    SHORTCUT_TOGGLE_BTN,   
    SIGNAL_NUM
} cb_signal;

#endif  /* GUI_TYPES_HEADER_H */