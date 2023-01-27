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

#include "class_types.h"     /* portfolio_packet */
#include "gui_types.h"       /* symbol_name_map */
#include "multicurl_types.h" /* MemType */

/* gui */
GtkWidget *GetWidget(const gchar *widget_name_ch);
GObject *GetGObject(const gchar *gobject_name_ch);
const gchar *GetEntryText(const gchar *name_ch);
void GuiStart(gpointer data);

/* gui_workfuncs */
void AddColumnToTreeview(const gchar *col_name, const gint col_num,
                         GtkWidget *treeview);
int TreeViewClear(GtkWidget *treeview);
gint SetFormattedLabel(GtkWidget *label, const gchar *fmt, const gchar *font,
                       const gchar *text);
gint CompletionSet(gpointer data, guintptr gui_completion_sig);
void StartCompletionThread(gpointer data);
void StartClockThread(gpointer data);

/* gui_main */
gint MainSetFonts(gpointer data);
gint MainPrimaryTreeview(gpointer data);
gint MainDefaultTreeview(gpointer data);
gint MainFetchBTNLabel(gpointer data);
gint MainSetClocks(gpointer data);
void MainProgBar(gdouble *fraction);
gint MainProgBarReset();
gint MainHideWindows();
gint MainDisplayClocks();
gint MainHideClocks();

/* gui_security */
gint SecurityShowHide(gpointer data);
gint SecurityOk(gpointer data);
gint SecurityComBoxChange(gpointer data);
gint SecurityCursorMove();
gint SecurityCompletionSet(gpointer data);

/* gui_other_wins */
gint PrefShowHide(gpointer data);
gint PrefSetClockSwitch(gpointer data);
gint PrefSymBtnStart();
gint PrefSymBtnStop();
gint APIShowHide(gpointer data);
gint APIOk(gpointer data);
gint APICursorMove();
gint BullionComBoxChange();
gint BullionShowHide(gpointer data);
gboolean BullionOk(gpointer data);
gint BullionCursorMove();
gint CashShowHide(gpointer data);
gint CashOk(gpointer data);
gint CashCursorMove();
gint AboutShowHide();
void AboutSetLabel();
gint HotkeysShowHide();
void HotkeysSetTreeview();

/* gui_history */
gint HistoryShowHide(gpointer data);
gint HistoryTreeViewClear();
gint HistoryMakeTreeview(gpointer data);
gint HistoryCursorMove();
gint HistoryCompletionSet(gpointer data);
gint HistorySetSNLabel(gpointer data);
gint HistoryGetSymbol(gchar **s);
GtkListStore *HistoryMakeStore(const gchar *data_str);
MemType *FetchHistoryData(const gchar *symbol_ch, portfolio_packet *pkg);

/* GUI Callback Functions */
void GUICallbackHandler(GtkWidget *widget, gpointer data);
void GUICallback_security_stack(GObject *gobject);
void GUICallback_pref_font_button(GtkFontButton *widget);
gboolean GUICallback_pref_clock_switch(GtkSwitch *Switch,
                                              gboolean state);
gboolean GUICallback_pref_indices_switch(GtkSwitch *Switch,
                                                gboolean state);
void GUICallback_pref_dec_places_combobox(GtkComboBox *ComboBox);
void GUICallback_pref_up_min_combobox(GtkComboBox *ComboBox);
void GUICallback_pref_hours_spinbutton(GtkEditable *spin_button);
gboolean GUICallback_hide_window_on_delete(GtkWidget *window,
                                                  GdkEvent *event,
                                                  gpointer data);
gboolean GUICallback_window_data(GtkWidget *window, GdkEvent *event,
                                        gpointer data);
gboolean GUICallback_select_comp(GtkEntryCompletion *completion,
                                        GtkTreeModel *model, GtkTreeIter *iter,
                                        gpointer data);
gboolean GUICallback_cursor_comp(GtkEntryCompletion *completion,
                                        GtkTreeModel *model, GtkTreeIter *iter,
                                        gpointer data);
gboolean GUICallback_main_treeview_click(GtkWidget *treeview, GdkEventButton *event);

/* GUI Thread Handler Functions */
gpointer GUIThreadHandler_main_fetch_handler(gpointer data);
gpointer GUIThread_clock(gpointer data);
gpointer GUIThreadHandler_clock_handler(gpointer data);
gpointer GUIThread_history_fetch(gpointer data);
gpointer GUIThread_pref_sym_update(gpointer data);
gpointer GUIThread_completion_set(gpointer data);
gpointer GUIThread_api_ok(gpointer data);
gpointer GUIThread_recalculate(gpointer data);
gpointer GUIThread_bul_fetch(gpointer data);
gpointer GUIThread_main_exit(gpointer data);

#endif /* GUI_HEADER_H */