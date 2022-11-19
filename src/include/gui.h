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

#include <gtk/gtk.h>  /* gboolean, GtkSwitch, GtkWidget, GdkEvent, GtkEntryCompletion,
                         GtkTreeModel, GtkTreeIter, GdkEventButton */
#include <stdbool.h>  /* bool */

/* gui */
void GuiStart (void*);

/* gui_main */
int MainPrimaryTreeview (void*);
int MainDefaultTreeview (void*);
int MainFetchBTNLabel (void*);
int MainDisplayTime ();
int MainDisplayTimeRemaining (void*);
void MainProgBar (double*);

/* gui_equity */
int AddRemShowHide (void*);
int AddRemSwitchChange ();
int AddRemOk (void*);
int AddRemComBoxChange ();
int AddRemCursorMove ();
int AddRemCompletionSet (void*);

/* gui_other_wins */
int APIShowHide (void*);
int APIOk (void*);
int APICursorMove ();
int BullionShowHide (void*);
int BullionOk (void*);
int BullionCursorMove ();
int CashShowHide (void*);
int CashOk (void*);
int CashCursorMove ();
int AboutShowHide ();
int ShortcutShowHide ();

/* gui_rsi */
int RSIShowHide ();
int RSITreeViewClear();
int RSIMakeTreeview (void*);
int RSICursorMove ();
int RSICompletionSet (void*);
int RSISetSNLabel (void*);
int RSIGetSymbol (char**);

/* GUI Callback Functions */
void GUICallbackHandler_add_rem_switch (GtkSwitch*,bool,void*);
void GUICallbackHandler (GtkWidget*,void*);
gboolean GUICallbackHandler_expander_bar (GtkWidget*,void*);
gboolean GUICallbackHandler_window_data (GtkWidget*,GdkEvent*,void*);
gboolean GUICallbackHandler_select_comp (GtkEntryCompletion*,GtkTreeModel*,GtkTreeIter*);
gboolean GUICallbackHandler_cursor_comp (GtkEntryCompletion*,GtkTreeModel*,GtkTreeIter*);
gboolean view_onButtonPressed (GtkWidget*,GdkEventButton*);

/* GUI Thread Handler Function */
void* GUIThreadHandler(void*);

#endif  /* GUI_HEADER_H */