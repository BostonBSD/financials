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
#include "../include/gui.h"
#include "../include/class_types.h" /* portfolio_packet, window_data */
#include "../include/macros.h"
#include "../include/workfuncs.h"

static GtkBuilder *builder;

/* Three Convenience Functions */
GtkWidget *GetWidget(const gchar *widget_name_ch) {
  return GTK_WIDGET(gtk_builder_get_object(builder, widget_name_ch));
}

GObject *GetGObject(const gchar *gobject_name_ch) {
  return gtk_builder_get_object(builder, gobject_name_ch);
}

const gchar *GetEntryText(const gchar *name_ch) {
  GtkWidget *EntryBox = GetWidget(name_ch);
  return gtk_entry_get_text(GTK_ENTRY(EntryBox));
}

/* GtkWidget/GObject signal connect functions. */
static void main_window_sig_connect(portfolio_packet *pkg) {
  window_data *W = pkg->GetWindowData();
  GObject *window, *object;

  /* Connect signal handlers to the constructed widgets/objects. */
  window = GetGObject("MainWindow");
  g_signal_connect(window, "destroy", G_CALLBACK(GUICallbackHandler),
                   (gpointer)MAIN_EXIT);
  if (pkg->meta_class->window_struct.main_win_maximized_bool) {
    gtk_window_maximize(GTK_WINDOW(window));
  } else {
    gtk_window_resize(GTK_WINDOW(window), W->main_width, W->main_height);
  }
  gtk_window_move(GTK_WINDOW(window), W->main_x_pos, W->main_y_pos);

  g_signal_connect(window, "configure-event",
                   G_CALLBACK(GUICallback_window_data),
                   (gpointer)GUI_MAIN_WINDOW);

  object = GetGObject("FetchDataBTN");
  g_signal_connect(object, "clicked", G_CALLBACK(GUICallbackHandler),
                   (gpointer)MAIN_FETCH_BTN);
  gtk_widget_grab_focus(GTK_WIDGET(object));

  object = GetGObject("MainFileMenuHistory");
  g_signal_connect(object, "activate", G_CALLBACK(GUICallbackHandler),
                   (gpointer)HISTORY_TOGGLE_BTN);

  object = GetGObject("MainFileMenuQuit");
  g_signal_connect(object, "activate", G_CALLBACK(GUICallbackHandler),
                   (gpointer)MAIN_EXIT);

  object = GetGObject("MainEditMenuSecurity");
  g_signal_connect(object, "activate", G_CALLBACK(GUICallbackHandler),
                   (gpointer)SECURITY_TOGGLE_BTN);

  object = GetGObject("MainEditMenuBullion");
  g_signal_connect(object, "activate", G_CALLBACK(GUICallbackHandler),
                   (gpointer)BUL_TOGGLE_BTN);

  object = GetGObject("MainEditMenuCash");
  g_signal_connect(object, "activate", G_CALLBACK(GUICallbackHandler),
                   (gpointer)CASH_TOGGLE_BTN);

  object = GetGObject("MainEditMenuAPI");
  g_signal_connect(object, "activate", G_CALLBACK(GUICallbackHandler),
                   (gpointer)API_TOGGLE_BTN);

  object = GetGObject("MainEditMenuPref");
  g_signal_connect(object, "activate", G_CALLBACK(GUICallbackHandler),
                   (gpointer)PREF_TOGGLE_BTN);

  object = GetGObject("MainHelpMenuHotkeys");
  g_signal_connect(object, "activate", G_CALLBACK(GUICallbackHandler),
                   (gpointer)HOTKEYS_TOGGLE_BTN);

  object = GetGObject("MainHelpMenuAbout");
  g_signal_connect(object, "activate", G_CALLBACK(GUICallbackHandler),
                   (gpointer)ABOUT_TOGGLE_BTN);

  object = GetGObject("MainTreeView");
  g_signal_connect(object, "button-press-event",
                   G_CALLBACK(GUICallback_main_treeview_click), NULL);
}

static void security_window_sig_connect() {
  GObject *window, *object;

  object = GetGObject("SecurityMenuClose");
  g_signal_connect(object, "activate", G_CALLBACK(GUICallbackHandler),
                   (gpointer)SECURITY_TOGGLE_BTN);

  window = GetGObject("SecurityWindow");
  g_signal_connect(window, "delete_event",
                   G_CALLBACK(GUICallback_hide_window_on_delete),
                   (gpointer)SECURITY_TOGGLE_BTN);

  object = GetGObject("SecurityOkBTN");
  g_signal_connect(object, "clicked", G_CALLBACK(GUICallbackHandler),
                   (gpointer)SECURITY_OK_BTN);

  object = GetGObject("SecurityStack");
  g_signal_connect(object, "notify::visible-child",
                   G_CALLBACK(GUICallback_security_stack), NULL);

  object = GetGObject("SecurityComboBox");
  g_signal_connect(object, "changed", G_CALLBACK(GUICallbackHandler),
                   (gpointer)SECURITY_COMBO_BOX);

  object = GetGObject("SecuritySymbolEntryBox");
  g_signal_connect(object, "changed", G_CALLBACK(GUICallbackHandler),
                   (gpointer)SECURITY_CURSOR_MOVE);

  object = GetGObject("SecuritySharesEntryBox");
  g_signal_connect(object, "changed", G_CALLBACK(GUICallbackHandler),
                   (gpointer)SECURITY_CURSOR_MOVE);

  object = GetGObject("SecurityCostEntryBox");
  g_signal_connect(object, "changed", G_CALLBACK(GUICallbackHandler),
                   (gpointer)SECURITY_CURSOR_MOVE);
}

static void bullion_window_sig_connect() {
  GObject *window, *object;

  object = GetGObject("BullionMenuClose");
  g_signal_connect(object, "activate", G_CALLBACK(GUICallbackHandler),
                   (gpointer)BUL_TOGGLE_BTN);

  window = GetGObject("BullionWindow");
  g_signal_connect(window, "delete_event",
                   G_CALLBACK(GUICallback_hide_window_on_delete),
                   (gpointer)BUL_TOGGLE_BTN);

  object = GetGObject("BullionOKBTN");
  g_signal_connect(object, "clicked", G_CALLBACK(GUICallbackHandler),
                   (gpointer)BUL_OK_BTN);
  gtk_widget_set_sensitive(GTK_WIDGET(object), FALSE);

  object = GetGObject("BullionGoldOuncesEntryBox");
  g_signal_connect(object, "changed", G_CALLBACK(GUICallbackHandler),
                   (gpointer)BUL_CURSOR_MOVE);

  object = GetGObject("BullionGoldPremiumEntryBox");
  g_signal_connect(object, "changed", G_CALLBACK(GUICallbackHandler),
                   (gpointer)BUL_CURSOR_MOVE);

  object = GetGObject("BullionGoldCostEntryBox");
  g_signal_connect(object, "changed", G_CALLBACK(GUICallbackHandler),
                   (gpointer)BUL_CURSOR_MOVE);

  object = GetGObject("BullionSilverOuncesEntryBox");
  g_signal_connect(object, "changed", G_CALLBACK(GUICallbackHandler),
                   (gpointer)BUL_CURSOR_MOVE);

  object = GetGObject("BullionSilverPremiumEntryBox");
  g_signal_connect(object, "changed", G_CALLBACK(GUICallbackHandler),
                   (gpointer)BUL_CURSOR_MOVE);

  object = GetGObject("BullionSilverCostEntryBox");
  g_signal_connect(object, "changed", G_CALLBACK(GUICallbackHandler),
                   (gpointer)BUL_CURSOR_MOVE);

  object = GetGObject("BullionPlatinumOuncesEntryBox");
  g_signal_connect(object, "changed", G_CALLBACK(GUICallbackHandler),
                   (gpointer)BUL_CURSOR_MOVE);

  object = GetGObject("BullionPlatinumPremiumEntryBox");
  g_signal_connect(object, "changed", G_CALLBACK(GUICallbackHandler),
                   (gpointer)BUL_CURSOR_MOVE);

  object = GetGObject("BullionPlatinumCostEntryBox");
  g_signal_connect(object, "changed", G_CALLBACK(GUICallbackHandler),
                   (gpointer)BUL_CURSOR_MOVE);

  object = GetGObject("BullionPalladiumOuncesEntryBox");
  g_signal_connect(object, "changed", G_CALLBACK(GUICallbackHandler),
                   (gpointer)BUL_CURSOR_MOVE);

  object = GetGObject("BullionPalladiumPremiumEntryBox");
  g_signal_connect(object, "changed", G_CALLBACK(GUICallbackHandler),
                   (gpointer)BUL_CURSOR_MOVE);

  object = GetGObject("BullionPalladiumCostEntryBox");
  g_signal_connect(object, "changed", G_CALLBACK(GUICallbackHandler),
                   (gpointer)BUL_CURSOR_MOVE);

  object = GetGObject("BullionComboBox");
  g_signal_connect(object, "changed", G_CALLBACK(GUICallbackHandler),
                   (gpointer)BUL_COMBO_BOX);
}

static void preferences_window_sig_connect(portfolio_packet *pkg) {
  meta *D = pkg->GetMetaClass();
  GObject *window, *object;

  object = GetGObject("PrefMenuClose");
  g_signal_connect(object, "activate", G_CALLBACK(GUICallbackHandler),
                   (gpointer)PREF_TOGGLE_BTN);

  window = GetGObject("PrefWindow");
  g_signal_connect(window, "delete_event",
                   G_CALLBACK(GUICallback_hide_window_on_delete),
                   (gpointer)PREF_TOGGLE_BTN);

  object = GetGObject("PrefStockSymbolUpdateBTN");
  g_signal_connect(object, "clicked", G_CALLBACK(GUICallbackHandler),
                   (gpointer)PREF_SYMBOL_UPDATE_BTN);

  object = GetGObject("PrefHoursSpinBox");
  g_signal_connect(object, "changed",
                   G_CALLBACK(GUICallback_pref_hours_spinbutton), NULL);

  object = GetGObject("PrefShowClocksSwitch");
  /* Make sure the switch is set before connecting a signal to it. */
  gtk_switch_set_active(GTK_SWITCH(object), D->clocks_displayed_bool);
  g_signal_connect(object, "state-set",
                   G_CALLBACK(GUICallback_pref_clock_switch), NULL);

  object = GetGObject("PrefShowIndicesSwitch");
  /* Make sure the switch is set before connecting a signal to it. */
  gtk_switch_set_active(GTK_SWITCH(object), D->index_bar_revealed_bool);
  g_signal_connect(object, "state-set",
                   G_CALLBACK(GUICallback_pref_indices_switch), NULL);

  object = GetGObject("PrefUpPerMinComboBox");
  gtk_combo_box_set_active(GTK_COMBO_BOX(object), (int)D->updates_per_min_f);
  g_signal_connect(object, "changed",
                   G_CALLBACK(GUICallback_pref_up_min_combobox), NULL);

  object = GetGObject("PrefDecPlacesComboBox");
  /* The combobox index 0, is 2 dec places, index 1 is 3 dec places. So
   * minus 2. */
  gtk_combo_box_set_active(GTK_COMBO_BOX(object),
                           (int)D->decimal_places_guint8 - 2);
  g_signal_connect(object, "changed",
                   G_CALLBACK(GUICallback_pref_dec_places_combobox), NULL);

  object = GetGObject("PrefFontChooserBTN");
  /* Make sure the font is set before connecting a signal to it. */
  gtk_font_chooser_set_font(GTK_FONT_CHOOSER(object), D->font_ch);
  g_signal_connect(object, "font-set", G_CALLBACK(GUICallback_pref_font_button),
                   NULL);
}

static void api_window_sig_connect() {
  GObject *window, *object;

  object = GetGObject("APIMenuClose");
  g_signal_connect(object, "activate", G_CALLBACK(GUICallbackHandler),
                   (gpointer)API_TOGGLE_BTN);

  window = GetGObject("ApiWindow");
  g_signal_connect(window, "delete_event",
                   G_CALLBACK(GUICallback_hide_window_on_delete),
                   (gpointer)API_TOGGLE_BTN);

  object = GetGObject("ApiOKBTN");
  g_signal_connect(object, "clicked", G_CALLBACK(GUICallbackHandler),
                   (gpointer)API_OK_BTN);
  gtk_widget_set_sensitive(GTK_WIDGET(object), FALSE);

  object = GetGObject("ApiEquityUrlEntryBox");
  g_signal_connect(object, "changed", G_CALLBACK(GUICallbackHandler),
                   (gpointer)API_CURSOR_MOVE);

  object = GetGObject("ApiUrlKeyEntryBox");
  g_signal_connect(object, "changed", G_CALLBACK(GUICallbackHandler),
                   (gpointer)API_CURSOR_MOVE);

  object = GetGObject("ApiNasdaqSymbolsUrlEntryBox");
  g_signal_connect(object, "changed", G_CALLBACK(GUICallbackHandler),
                   (gpointer)API_CURSOR_MOVE);

  object = GetGObject("ApiNYSESymbolsUrlEntryBox");
  g_signal_connect(object, "changed", G_CALLBACK(GUICallbackHandler),
                   (gpointer)API_CURSOR_MOVE);
}

static void history_window_sig_connect(portfolio_packet *pkg) {
  window_data *W = pkg->GetWindowData();
  GObject *window, *object;

  window = GetGObject("HistoryWindow");
  g_signal_connect(window, "delete_event",
                   G_CALLBACK(GUICallback_hide_window_on_delete),
                   (gpointer)HISTORY_TOGGLE_BTN);
  if (pkg->meta_class->window_struct.histry_win_maximized_bool) {
    gtk_window_maximize(GTK_WINDOW(window));
  } else {
    gtk_window_resize(GTK_WINDOW(window), W->history_width, W->history_height);
  }
  gtk_window_move(GTK_WINDOW(window), W->history_x_pos, W->history_y_pos);

  g_signal_connect(window, "configure-event",
                   G_CALLBACK(GUICallback_window_data),
                   (gpointer)GUI_HISTORY_WINDOW);

  object = GetGObject("HistoryMenuCloseBTN");
  g_signal_connect(object, "activate", G_CALLBACK(GUICallbackHandler),
                   (gpointer)HISTORY_TOGGLE_BTN);

  object = GetGObject("HistoryFetchDataBTN");
  g_signal_connect(object, "clicked", G_CALLBACK(GUICallbackHandler),
                   (gpointer)HISTORY_FETCH_BTN);

  object = GetGObject("HistorySymbolEntryBox");
  g_signal_connect(object, "changed", G_CALLBACK(GUICallbackHandler),
                   (gpointer)HISTORY_CURSOR_MOVE);
}

static void other_window_sig_connect() {
  GObject *window, *object;

  object = GetGObject("AboutMenuClose");
  g_signal_connect(object, "activate", G_CALLBACK(GUICallbackHandler),
                   (gpointer)ABOUT_TOGGLE_BTN);

  window = GetGObject("AboutWindow");
  g_signal_connect(window, "delete_event",
                   G_CALLBACK(GUICallback_hide_window_on_delete),
                   (gpointer)ABOUT_TOGGLE_BTN);

  object = GetGObject("HotkeysMenuClose");
  g_signal_connect(object, "activate", G_CALLBACK(GUICallbackHandler),
                   (gpointer)HOTKEYS_TOGGLE_BTN);

  window = GetGObject("HotkeysWindow");
  g_signal_connect(window, "delete_event",
                   G_CALLBACK(GUICallback_hide_window_on_delete),
                   (gpointer)HOTKEYS_TOGGLE_BTN);

  object = GetGObject("CashMenuClose");
  g_signal_connect(object, "activate", G_CALLBACK(GUICallbackHandler),
                   (gpointer)CASH_TOGGLE_BTN);

  window = GetGObject("CashWindow");
  g_signal_connect(window, "delete_event",
                   G_CALLBACK(GUICallback_hide_window_on_delete),
                   (gpointer)CASH_TOGGLE_BTN);

  object = GetGObject("CashOKBTN");
  g_signal_connect(object, "clicked", G_CALLBACK(GUICallbackHandler),
                   (gpointer)CASH_OK_BTN);
  gtk_widget_set_sensitive(GTK_WIDGET(object), FALSE);

  object = GetGObject("CashSpinBTN");
  g_signal_connect(object, "changed", G_CALLBACK(GUICallbackHandler),
                   (gpointer)CASH_CURSOR_MOVE);
}

static void gui_signal_connect(portfolio_packet *pkg)
/* Connect widget signals to signal handlers. */
{
  main_window_sig_connect(pkg);
  security_window_sig_connect();
  bullion_window_sig_connect();
  preferences_window_sig_connect(pkg);
  api_window_sig_connect();
  history_window_sig_connect(pkg);
  other_window_sig_connect();
}

static void set_application_css() {
  /* This will set CSS data for the screen [application-wide].

     Right now we're only setting the treeview
     [so they display correctly on dark window manager themes]. */
  GdkScreen *screen = gdk_display_get_default_screen(gdk_display_get_default());
  GtkCssProvider *provider = gtk_css_provider_new();

  gchar *bg_colr_css = "treeview{color:Black;background-color:White;}treeview:"
                       "hover,treeview:selected{background-color:Coral;}";
  gtk_css_provider_load_from_data(provider, bg_colr_css, -1, NULL);
  gtk_style_context_add_provider_for_screen(
      screen, GTK_STYLE_PROVIDER(provider),
      GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

  g_object_unref(G_OBJECT(provider));
}

/* Engineering Note */

/* After the gtk_main() loop starts nearly every widget signal is connected
   to the GUICallbackHandler function in gui_callbacks.c.

   Some signals require separate threads to perform computational tasks, these
   are dispatched from the GUICallback functions [in gui_callbacks.c].
   The clock threads are dispatched by separate functions [StartClockThread(),
   GUICallback_pref_clock_switch()].

   There are ancillary widget signals to perform other tasks depending upon
   widget states, all of these callback functions are also in gui_callbacks.c.

   In order to make window delete [closing a window by pressing the windowbar X]
   operate the same as a close button, we have our own "delete-event" signal
   handler called GUICallback_hide_window_on_delete().

   Widget index signals are defined in gui_types.h [these are application
   defined and not the same as GtkWidget signals].  These index signals are
   convenient for adding new widgets with a generic callback function.
*/

void GuiStart(portfolio_packet *pkg)
/* Setup GUI widgets and display the GUI. */
{
  GError *error = NULL;

  /* Compiling the resources.c file ensures the resources are in the
     GResources namespace (basically ensures the UI description file
     and the icon file are in the namespace, our only resources).
     The icon file is referenced in the UI description file. */

  /* Construct a GtkBuilder instance and load our UI description. */
  builder = gtk_builder_new();
  if (gtk_builder_add_from_resource(builder, "/financials.glade", &error) ==
      0) {
    g_printerr("Error loading user interface: %s\n", error->message);
    g_clear_error(&error);
    exit(EXIT_FAILURE);
  }

  /* Set application-wide CSS. */
  set_application_css();

  /* Set whether the clocks are displayed or not.
     Start clock threads if the clocks are displayed. */
  StartClockThread(pkg);

  /* Add the list of stock symbols from sqlite to a struct.
     Set two entrybox completion widgets. */
  StartCompletionThread(pkg);

  /* Add hyperlink markup to the About window labels. */
  AboutSetLabel();

  /* Make sure the security names are set with pango style markups. */
  pkg->SetSecurityNames();

  /* Make sure the label and treeview header fonts are set */
  SetLabelFonts(pkg->meta_class->font_ch);
  pkg->meta_class->ToStringsHeadings();

  /* Set the default treeview. */
  pkg->ToStrings();
  MainDefaultTreeview(pkg);

  /* Connect callback functions to corresponding GUI signals. */
  gui_signal_connect(pkg);

  /* Start the gtk_main loop. */
  gtk_main();
}