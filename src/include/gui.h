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
#ifndef GUI_HEADER_H
#define GUI_HEADER_H

#include <gtk/gtk.h> /* gboolean, GtkSwitch, GtkWidget, GdkEvent, GtkEntryCompletion,
                         GtkTreeModel, GtkTreeIter, GdkEventButton, GtkListStore */

#include <stdbool.h> /* bool */

#include "gui_types.h" /* symbol_name_map */

/* gui */
GtkWidget *GetWidget(const gchar *);
GObject *GetGObject(const gchar *);
const gchar *GetEntryText(const char *);
void GuiStart(void *);
int CompletionSet(void *, uintptr_t);

/* gui_main */
int MainPrimaryTreeview(void *);
int MainDefaultTreeview(void *);
int MainFetchBTNLabel(void *);
int MainDisplayTime();
int MainDisplayTimeRemaining(void *);
void MainProgBar(double *);
int MainProgBarReset();
int MainHideWindow();
int MainDisplayClocks(void *);

/* gui_equity */
int AddRemShowHide(void *);
int AddRemOk(void *);
int AddRemComBoxChange(void *);
int AddRemCursorMove();
int AddRemCompletionSet(void *);

/* gui_other_wins */
int PrefShowHide(void *);
int PrefSymBtnStart();
int PrefSymBtnStop();
int APIShowHide(void *);
int APIOk(void *);
int APICursorMove();
int BullionComBoxChange();
int BullionShowHide(void *);
int BullionOk(void *);
int BullionCursorMove();
int CashShowHide(void *);
int CashOk(void *);
int CashCursorMove();
int AboutShowHide();
int HotkeysShowHide();

/* gui_rsi */
int RSIShowHide(void *);
int RSITreeViewClear();
int RSIMakeTreeview(void *);
int RSICursorMove();
int RSICompletionSet(void *);
int RSISetSNLabel(void *);
int RSIGetSymbol(char **);
GtkListStore *RSIMakeStore(const char *);

/* GUI Callback Functions */
void GUICallbackHandler(GtkWidget *, void *);
void GUICallbackHandler_add_rem_stack(GObject *);
void GUICallbackHandler_pref_font_button(GtkFontButton *, void *);
gboolean GUICallbackHandler_pref_clock_switch(GtkSwitch *, bool);
gboolean GUICallbackHandler_pref_indices_switch(GtkSwitch *, bool);
void GUICallbackHandler_pref_dec_places_combobox(GtkComboBox *);
void GUICallbackHandler_pref_up_min_combobox(GtkComboBox *);
void GUICallbackHandler_pref_hours_spinbutton(GtkEditable *);
gboolean GUICallbackHandler_hide_window_on_delete(GtkWidget *, GdkEvent *,
                                                  void *);
gboolean GUICallbackHandler_window_data(GtkWidget *, GdkEvent *, void *);
gboolean GUICallbackHandler_select_comp(GtkEntryCompletion *, GtkTreeModel *,
                                        GtkTreeIter *, void *);
gboolean GUICallbackHandler_cursor_comp(GtkEntryCompletion *, GtkTreeModel *,
                                        GtkTreeIter *, void *);
gboolean view_onButtonPressed(GtkWidget *, GdkEventButton *);

/* GUI Thread Handler Functions */
void *GUIThreadHandler_api_ok(void *);
void *GUIThreadHandler_cash_ok(void *);
void *GUIThreadHandler_bul_ok(void *);
void *GUIThreadHandler_main_fetch_data(void *);
void *GUIThreadHandler_rsi_fetch(void *);
void *GUIThreadHandler_sym_name_update(void *);
void *GUIThreadHandler_completion_set(void *);
void *GUIThreadHandler_main_clock();
void *GUIThreadHandler_time_to_close(void *);
void *GUIThreadHandler_main_exit(void *);

#endif /* GUI_HEADER_H */