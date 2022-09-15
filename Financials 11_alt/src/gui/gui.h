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
#ifndef GUI_HEADER_H
#define GUI_HEADER_H

#include <glib-object.h>
#include <gtk/gtk.h>

#include <stdbool.h>

#include "ui.h"
#include "../csv/csv.h"

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
#define YAHOO_URL_END "&interval=1d&events=history&includeAdjustedClose=true"
#endif

#ifndef NASDAQ_SYMBOL_URL
#define NASDAQ_SYMBOL_URL "http://nasdaqtrader.com/dynamic/SymDir/nasdaqlisted.txt"
#endif

/* Includes other exchanges as well */
#ifndef NYSE_SYMBOL_URL
#define NYSE_SYMBOL_URL "http://www.nasdaqtrader.com/dynamic/SymDir/otherlisted.txt"
#endif

/* Structs and Enums */
typedef struct {
  char* symbol;
  char* security_name;
} symbol_to_security_name_container;

typedef struct {
  char* type;
  char* symbol;
} right_click_container;

typedef struct {
  int height;
  int width;
  int x_pos;
  int y_pos;
} main_window_data;

/* TreeView Data Column Type */
enum {
  GUI_COLUMN_DEFAULT,
  GUI_COLUMN_ONE
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

/* TreeView Column Equity Numbers */
enum {
  EQUITY_ID,
  EQUITY_DATE,
  EQUITY_SYMBOL,
  EQUITY_PRICE,
  EQUITY_SHARES,
  EQUITY_STAKE,
  EQUITY_HIGH,
  EQUITY_LOW,
  EQUITY_OPENING,
  EQUITY_PREV_CLOSING,
  EQUITY_CHANGE,
  EQUITY_CHANGE_PERCENT,
  EQUITY_RSI,
  EQUITY_FOREGROUND_COLOR,
  EQUITY_N_COLUMNS
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
    FETCH_DATA_BTN, 
    ABOUT_BTN,
    ADD_REMOVE_SEC_BTN,
    ADD_REMOVE_SEC_OK_BTN,
    ADD_REMOVE_SEC_CANCEL_BTN,
    ADD_REMOVE_SEC_SWITCH,
    ADD_REMOVE_SEC_COMBO_BOX,
    ADD_REMOVE_SEC_CURSOR_MOVE,
    ADD_REMOVE_BUL_BTN,
    ADD_REMOVE_BUL_OK_BTN,
    ADD_REMOVE_BUL_CANCEL_BTN,
    ADD_REMOVE_BUL_CURSOR_MOVE,
    ADD_REMOVE_CASH_BTN,
    ADD_REMOVE_CASH_OK_BTN,
    ADD_REMOVE_CASH_CANCEL_BTN,
    ADD_REMOVE_CASH_CURSOR_MOVE,
    CHANGE_API_BTN,
    CHANGE_API_OK_BTN,
    CHANGE_API_CANCEL_BTN,
    CHANGE_API_CURSOR_MOVE,
    VIEW_RSI_BTN,
    VIEW_RSI_FETCH_DATA_BTN,
    VIEW_RSI_CURSOR_MOVE,
    VIEW_RSI_COMPLETION,
    APPLICATION_WINDOW_SHOW,
    DISPLAY_TIME,
    DISPLAY_TIME_OPEN_INDICATOR,
    EXIT_APPLICATION, 
    EXIT_APPLICATION_DELETE_EVENT,
    SIGNAL_NUM
} cb_signal;

/* Global Variables */
extern GtkBuilder *builder;                                 /* The Gtk builder object */
extern main_window_data MainWindowStruct;
extern symbol_to_security_name_container **security_symbol; /* A mapping between sec symbols and sec names */
extern int symbolcount;

/* Working Functions */
void SetUpGUI ();
int MakeGUIOne ();
int ShowHideAddRemoveSecurityWindow ();
int SwitchChangeAddRemoveSecurityWindow ();
int OKSecurityAddRemoveSecurityWindow ();
int ChangedAddRemoveSecurityComboBox ();
int CursorMoveAddRemoveSecurityEntryBoxGUI ();
int ShowHideAboutWindow ();
int ShowHideBullionWindow ();
int OKBullionWindow ();
int CursorMoveBullionEntryBoxGUI ();
int ShowHideCashWindow ();
int OKCashWindow ();
int CursorMoveCashEntryBoxGUI ();
int ShowHideAPIWindow ();
int OKAPIWindow ();
int CursorMoveAPIEntryBoxGUI ();
int UpDateProgressBar (void*);
void UpDateProgressBarGUI (double*);
int DefaultTreeView ();
int FetchDataBTNLabel (void*);
int DisplayTime ();
int DisplayTimeRemaining ();
int ShowHideViewRSIWindow ();
int RSITreeViewClear();
int RSIMakeGUI ();
int RSICursorMove ();
int RSISetColumns ();
int ViewRSICompletionSet ();
GtkListStore* MakeRSIStore ();

/* GUI Callback Functions */
void AddRemoveSecuritySwitch (GtkSwitch*,bool,void*);
void GUICallbackHandler (GtkWidget*,void*);
gboolean CallbackHandler_alt (GtkWidget*);

/* GUI Thread Handler Function */
void *GUIThreadHandler(void*);

#endif  /* GUI_HEADER_H */