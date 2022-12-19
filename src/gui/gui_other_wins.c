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
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include <glib-object.h>
#include <gtk/gtk.h>

#include "../include/gui.h"         /* MainPrimaryTreeview */
#include "../include/gui_globals.h" /* GtkBuilder *builder */

#include "../include/class_types.h" /* portfolio_packet, equity_folder, metal, meta */
#include "../include/multicurl.h"
#include "../include/mutex.h"
#include "../include/sqlite.h"
#include "../include/workfuncs.h"

int PrefShowHide(void *data) {
  /* Unpack the package */
  portfolio_packet *package = (portfolio_packet *)data;
  meta *D = package->GetMetaClass();

  /* get the GObject and cast as a GtkWidget */
  GtkWidget *window =
      GTK_WIDGET(gtk_builder_get_object(builder, "PreferencesWindow"));
  gboolean visible = gtk_widget_is_visible(window);

  if (visible) {
    gtk_widget_set_visible(window, false);
  } else {
    /* The combobox index 0, is 2 dec places, index 1 is 3 dec places. So
     * minus 2. */
    GtkWidget *ComboBox =
        GTK_WIDGET(gtk_builder_get_object(builder, "PrefDecPlacesComboBox"));
    gtk_combo_box_set_active(GTK_COMBO_BOX(ComboBox),
                             (int)D->decimal_places_shrt - 2);

    ComboBox =
        GTK_WIDGET(gtk_builder_get_object(builder, "PrefUpPerMinComboBox"));
    gtk_combo_box_set_active(GTK_COMBO_BOX(ComboBox),
                             (int)D->updates_per_min_f);

    GtkWidget *SpinBox =
        GTK_WIDGET(gtk_builder_get_object(builder, "PrefHoursSpinBox"));
    GtkAdjustment *Adjustment =
        gtk_spin_button_get_adjustment(GTK_SPIN_BUTTON(SpinBox));
    gtk_adjustment_set_value(Adjustment, D->updates_hours_f);
    g_object_set(G_OBJECT(SpinBox), "activates-default", TRUE, NULL);

    GtkWidget *Switch =
        GTK_WIDGET(gtk_builder_get_object(builder, "PrefShowClocksSwitch"));
    gtk_switch_set_active(GTK_SWITCH(Switch), D->clocks_displayed_bool);

    Switch =
        GTK_WIDGET(gtk_builder_get_object(builder, "PrefShowIndicesSwitch"));
    gtk_switch_set_active(GTK_SWITCH(Switch), D->index_bar_revealed_bool);

    gtk_widget_set_visible(window, true);
  }
  return 0;
}

int PrefSymBtnStart() {
  GtkWidget *button =
      GTK_WIDGET(gtk_builder_get_object(builder, "PrefStockSymbolUpdateBTN"));
  gtk_widget_set_sensitive(button, false);

  return 0;
}

int PrefSymBtnStop() {
  GtkWidget *button =
      GTK_WIDGET(gtk_builder_get_object(builder, "PrefStockSymbolUpdateBTN"));
  gtk_widget_set_sensitive(button, true);

  return 0;
}

int APIShowHide(void *data) {
  /* Unpack the package */
  portfolio_packet *package = (portfolio_packet *)data;
  meta *D = package->GetMetaClass();

  /* get the GObject and cast as a GtkWidget */
  GtkWidget *window =
      GTK_WIDGET(gtk_builder_get_object(builder, "ChangeApiInfoWindow"));
  gboolean visible = gtk_widget_is_visible(window);

  if (visible) {
    gtk_widget_set_visible(window, false);
  } else {
    GtkWidget *notebook =
        GTK_WIDGET(gtk_builder_get_object(builder, "ChangeAPINotebook"));
    gtk_notebook_set_current_page(GTK_NOTEBOOK(notebook), 0);

    GtkWidget *EntryBox = GTK_WIDGET(
        gtk_builder_get_object(builder, "ChangeApiInfoEquityUrlEntryBox"));
    gtk_entry_set_text(GTK_ENTRY(EntryBox), D->stock_url_ch);
    gtk_widget_grab_focus(EntryBox);
    g_object_set(G_OBJECT(EntryBox), "activates-default", TRUE, NULL);

    EntryBox = GTK_WIDGET(
        gtk_builder_get_object(builder, "ChangeApiInfoUrlKeyEntryBox"));
    gtk_entry_set_text(GTK_ENTRY(EntryBox), D->curl_key_ch);
    g_object_set(G_OBJECT(EntryBox), "activates-default", TRUE, NULL);

    EntryBox = GTK_WIDGET(gtk_builder_get_object(
        builder, "ChangeApiInfoNasdaqSymbolsUrlEntryBox"));
    gtk_entry_set_text(GTK_ENTRY(EntryBox), D->Nasdaq_Symbol_url_ch);
    g_object_set(G_OBJECT(EntryBox), "activates-default", TRUE, NULL);

    EntryBox = GTK_WIDGET(
        gtk_builder_get_object(builder, "ChangeApiInfoNYSESymbolsUrlEntryBox"));
    gtk_entry_set_text(GTK_ENTRY(EntryBox), D->NYSE_Symbol_url_ch);
    g_object_set(G_OBJECT(EntryBox), "activates-default", TRUE, NULL);

    gtk_widget_set_visible(window, true);
  }
  return 0;
}

int APIOk(void *data) {
  /* Unpack the package */
  portfolio_packet *package = (portfolio_packet *)data;
  equity_folder *F = package->GetEquityFolderClass();
  meta *D = package->GetMetaClass();

  GtkWidget *EntryBox = GTK_WIDGET(
      gtk_builder_get_object(builder, "ChangeApiInfoEquityUrlEntryBox"));
  const gchar *new = gtk_entry_get_text(GTK_ENTRY(EntryBox));

  if ((strcmp(D->stock_url_ch, new) != 0)) {
    SqliteAddAPIData("Stock_URL", new, D);
    CopyString(&D->stock_url_ch, new);
  }

  EntryBox = GTK_WIDGET(
      gtk_builder_get_object(builder, "ChangeApiInfoUrlKeyEntryBox"));
  new = gtk_entry_get_text(GTK_ENTRY(EntryBox));

  if ((strcmp(D->curl_key_ch, new) != 0)) {
    SqliteAddAPIData("URL_KEY", new, D);
    CopyString(&D->curl_key_ch, new);
  }

  EntryBox = GTK_WIDGET(
      gtk_builder_get_object(builder, "ChangeApiInfoNasdaqSymbolsUrlEntryBox"));
  new = gtk_entry_get_text(GTK_ENTRY(EntryBox));

  if ((strcmp(D->Nasdaq_Symbol_url_ch, new) != 0)) {
    SqliteAddAPIData("Nasdaq_Symbol_URL", new, D);
    CopyString(&D->Nasdaq_Symbol_url_ch, new);
  }

  EntryBox = GTK_WIDGET(
      gtk_builder_get_object(builder, "ChangeApiInfoNYSESymbolsUrlEntryBox"));
  new = gtk_entry_get_text(GTK_ENTRY(EntryBox));

  if ((strcmp(D->NYSE_Symbol_url_ch, new) != 0)) {
    SqliteAddAPIData("NYSE_Symbol_URL", new, D);
    CopyString(&D->NYSE_Symbol_url_ch, new);
  }

  /* Generate the Equity Request URLs. */
  F->GenerateURL(package);
  return 0;
}

int APICursorMove() {
  GtkWidget *Button =
      GTK_WIDGET(gtk_builder_get_object(builder, "ChangeApiInfoOKBTN"));
  GtkWidget *EntryBox = GTK_WIDGET(
      gtk_builder_get_object(builder, "ChangeApiInfoEquityUrlEntryBox"));
  const gchar *Equity_URL = gtk_entry_get_text(GTK_ENTRY(EntryBox));

  EntryBox = GTK_WIDGET(
      gtk_builder_get_object(builder, "ChangeApiInfoUrlKeyEntryBox"));
  const gchar *URL_KEY = gtk_entry_get_text(GTK_ENTRY(EntryBox));

  EntryBox = GTK_WIDGET(
      gtk_builder_get_object(builder, "ChangeApiInfoNasdaqSymbolsUrlEntryBox"));
  const gchar *Nasdaq_URL = gtk_entry_get_text(GTK_ENTRY(EntryBox));

  EntryBox = GTK_WIDGET(
      gtk_builder_get_object(builder, "ChangeApiInfoNYSESymbolsUrlEntryBox"));
  const gchar *NYSE_URL = gtk_entry_get_text(GTK_ENTRY(EntryBox));

  gboolean check = CheckValidString(Equity_URL) & CheckValidString(URL_KEY);
  check = check & CheckValidString(Nasdaq_URL) & CheckValidString(NYSE_URL);

  if (check) {
    gtk_widget_set_sensitive(Button, true);
  } else {
    gtk_widget_set_sensitive(Button, false);
  }

  return 0;
}

int BullionComBoxChange() {
  GtkWidget *ComboBox =
      GTK_WIDGET(gtk_builder_get_object(builder, "AddRemoveBullionComboBox"));
  gint index = gtk_combo_box_get_active(GTK_COMBO_BOX(ComboBox));
  GtkWidget *gold_frame =
      GTK_WIDGET(gtk_builder_get_object(builder, "AddRemoveBullionGoldFrame"));
  GtkWidget *silver_frame = GTK_WIDGET(
      gtk_builder_get_object(builder, "AddRemoveBullionSilverFrame"));
  GtkWidget *platinum_frame = GTK_WIDGET(
      gtk_builder_get_object(builder, "AddRemoveBullionPlatinumFrame"));
  GtkWidget *palladium_frame = GTK_WIDGET(
      gtk_builder_get_object(builder, "AddRemoveBullionPalladiumFrame"));

  enum { GOLD, SILVER, PLATINUM };

  switch (index) {
  case GOLD:
    gtk_widget_set_visible(gold_frame, true);
    gtk_widget_set_visible(silver_frame, false);
    gtk_widget_set_visible(platinum_frame, false);
    gtk_widget_set_visible(palladium_frame, false);

    break;
  case SILVER:
    gtk_widget_set_visible(gold_frame, false);
    gtk_widget_set_visible(silver_frame, true);
    gtk_widget_set_visible(platinum_frame, false);
    gtk_widget_set_visible(palladium_frame, false);

    break;
  case PLATINUM:
    gtk_widget_set_visible(gold_frame, false);
    gtk_widget_set_visible(silver_frame, false);
    gtk_widget_set_visible(platinum_frame, true);
    gtk_widget_set_visible(palladium_frame, false);

    break;
  default: /* PALLADIUM */
    gtk_widget_set_visible(gold_frame, false);
    gtk_widget_set_visible(silver_frame, false);
    gtk_widget_set_visible(platinum_frame, false);
    gtk_widget_set_visible(palladium_frame, true);

    break;
  }
  return 0;
}

int BullionShowHide(void *data) {
  /* Unpack the package */
  portfolio_packet *package = (portfolio_packet *)data;
  metal *M = package->GetMetalClass();
  gchar *temp = NULL;

  /* get the GObject and cast as a GtkWidget */
  GtkWidget *window =
      GTK_WIDGET(gtk_builder_get_object(builder, "AddRemoveBullionWindow"));
  gboolean visible = gtk_widget_is_visible(window);

  if (visible) {
    gtk_widget_set_visible(window, false);
  } else {
    /* Reset EntryBoxes */
    GtkWidget *EntryBox = GTK_WIDGET(
        gtk_builder_get_object(builder, "AddRemoveBullionGoldOuncesEntryBox"));
    DoubToNumStr(&temp, M->Gold->ounce_f, 4);
    ToNumStr( temp );
    gtk_entry_set_text(GTK_ENTRY(EntryBox), temp);
    gtk_widget_grab_focus(EntryBox);
    g_object_set(G_OBJECT(EntryBox), "activates-default", TRUE, NULL);

    EntryBox = GTK_WIDGET(
        gtk_builder_get_object(builder, "AddRemoveBullionGoldPremiumEntryBox"));
    DoubToNumStr(&temp, M->Gold->premium_f, 2);
    ToNumStr( temp );
    gtk_entry_set_text(GTK_ENTRY(EntryBox), temp);
    g_object_set(G_OBJECT(EntryBox), "activates-default", TRUE, NULL);

    EntryBox = GTK_WIDGET(gtk_builder_get_object(
        builder, "AddRemoveBullionSilverOuncesEntryBox"));
    DoubToNumStr(&temp, M->Silver->ounce_f, 4);
    ToNumStr( temp );
    gtk_entry_set_text(GTK_ENTRY(EntryBox), temp);
    g_object_set(G_OBJECT(EntryBox), "activates-default", TRUE, NULL);

    EntryBox = GTK_WIDGET(gtk_builder_get_object(
        builder, "AddRemoveBullionSilverPremiumEntryBox"));
    DoubToNumStr(&temp, M->Silver->premium_f, 2);
    ToNumStr( temp );
    gtk_entry_set_text(GTK_ENTRY(EntryBox), temp);
    g_object_set(G_OBJECT(EntryBox), "activates-default", TRUE, NULL);

    EntryBox = GTK_WIDGET(gtk_builder_get_object(
        builder, "AddRemoveBullionPlatinumOuncesEntryBox"));
    DoubToNumStr(&temp, M->Platinum->ounce_f, 4);
    ToNumStr( temp );
    gtk_entry_set_text(GTK_ENTRY(EntryBox), temp);
    g_object_set(G_OBJECT(EntryBox), "activates-default", TRUE, NULL);

    EntryBox = GTK_WIDGET(gtk_builder_get_object(
        builder, "AddRemoveBullionPlatinumPremiumEntryBox"));
    DoubToNumStr(&temp, M->Platinum->premium_f, 2);
    ToNumStr( temp );
    gtk_entry_set_text(GTK_ENTRY(EntryBox), temp);
    g_object_set(G_OBJECT(EntryBox), "activates-default", TRUE, NULL);

    EntryBox = GTK_WIDGET(gtk_builder_get_object(
        builder, "AddRemoveBullionPalladiumOuncesEntryBox"));
    DoubToNumStr(&temp, M->Palladium->ounce_f, 4);
    ToNumStr( temp );
    gtk_entry_set_text(GTK_ENTRY(EntryBox), temp);
    g_object_set(G_OBJECT(EntryBox), "activates-default", TRUE, NULL);

    EntryBox = GTK_WIDGET(gtk_builder_get_object(
        builder, "AddRemoveBullionPalladiumPremiumEntryBox"));
    DoubToNumStr(&temp, M->Palladium->premium_f, 2);
    ToNumStr( temp );
    gtk_entry_set_text(GTK_ENTRY(EntryBox), temp);
    g_object_set(G_OBJECT(EntryBox), "activates-default", TRUE, NULL);

    free(temp);

    GtkWidget *frame = GTK_WIDGET(
        gtk_builder_get_object(builder, "AddRemoveBullionGoldFrame"));
    gtk_widget_set_visible(frame, true);
    frame = GTK_WIDGET(
        gtk_builder_get_object(builder, "AddRemoveBullionSilverFrame"));
    gtk_widget_set_visible(frame, false);
    frame = GTK_WIDGET(
        gtk_builder_get_object(builder, "AddRemoveBullionPlatinumFrame"));
    gtk_widget_set_visible(frame, false);
    frame = GTK_WIDGET(
        gtk_builder_get_object(builder, "AddRemoveBullionPalladiumFrame"));
    gtk_widget_set_visible(frame, false);

    GtkWidget *combobox =
        GTK_WIDGET(gtk_builder_get_object(builder, "AddRemoveBullionComboBox"));
    gtk_combo_box_set_active(GTK_COMBO_BOX(combobox), 0);

    gtk_widget_set_visible(window, true);
  }
  return 0;
}

static void *fetch_data_for_new_bullion(void *data) {
  portfolio_packet *pkg = (portfolio_packet *)data;
  metal *M = pkg->GetMetalClass();
  short num_metals = 2;
  if (M->Platinum->ounce_f > 0)
    num_metals++;
  if (M->Palladium->ounce_f > 0)
    num_metals++;

  pthread_mutex_lock(&mutex_working[CLASS_MEMBER_MUTEX]);

  M->SetUpCurl(pkg);

  /* Perform the cURL requests simultaneously using multi-cURL. */
  int return_code =
      PerformMultiCurl(pkg->multicurl_main_hnd, (double)num_metals);
  if (return_code) {
    free(M->Gold->CURLDATA.memory);
    free(M->Silver->CURLDATA.memory);
    M->Gold->CURLDATA.memory = NULL;
    M->Silver->CURLDATA.memory = NULL;
    if (M->Platinum->CURLDATA.memory)
      free(M->Platinum->CURLDATA.memory);
    if (M->Palladium->CURLDATA.memory)
      free(M->Palladium->CURLDATA.memory);
    M->Platinum->CURLDATA.memory = NULL;
    M->Palladium->CURLDATA.memory = NULL;
  }

  M->ExtractData();

  pthread_mutex_unlock(&mutex_working[CLASS_MEMBER_MUTEX]);

  /* The following two statements lock the CLASS_MEMBER_MUTEX */

  pkg->Calculate();
  pkg->ToStrings();

  /* Update the main window treeview. */
  gdk_threads_add_idle(MainPrimaryTreeview, data);

  return NULL;
}

int BullionOk(void *data) {
  /* Unpack the package */
  portfolio_packet *package = (portfolio_packet *)data;
  metal *M = package->GetMetalClass();
  meta *D = package->GetMetaClass();
  gboolean new_gold = false, new_silver = false, new_platinum = false,
           new_palladium = false;

  GtkWidget *EntryBox = GTK_WIDGET(
      gtk_builder_get_object(builder, "AddRemoveBullionGoldOuncesEntryBox"));
  const gchar *new_ounces = gtk_entry_get_text(GTK_ENTRY(EntryBox));
  EntryBox = GTK_WIDGET(
      gtk_builder_get_object(builder, "AddRemoveBullionGoldPremiumEntryBox"));
  const gchar *new_premium = gtk_entry_get_text(GTK_ENTRY(EntryBox));

  if (strtod(new_ounces, NULL) != M->Gold->ounce_f ||
      strtod(new_premium, NULL) != M->Gold->premium_f) {
    SqliteAddBullion("gold", new_ounces, new_premium, M, D);
    new_gold = true;
  }

  EntryBox = GTK_WIDGET(
      gtk_builder_get_object(builder, "AddRemoveBullionSilverOuncesEntryBox"));
  new_ounces = gtk_entry_get_text(GTK_ENTRY(EntryBox));
  EntryBox = GTK_WIDGET(
      gtk_builder_get_object(builder, "AddRemoveBullionSilverPremiumEntryBox"));
  new_premium = gtk_entry_get_text(GTK_ENTRY(EntryBox));

  if (strtod(new_ounces, NULL) != M->Silver->ounce_f ||
      strtod(new_premium, NULL) != M->Silver->premium_f) {
    SqliteAddBullion("silver", new_ounces, new_premium, M, D);
    new_silver = true;
  }

  EntryBox = GTK_WIDGET(gtk_builder_get_object(
      builder, "AddRemoveBullionPlatinumOuncesEntryBox"));
  new_ounces = gtk_entry_get_text(GTK_ENTRY(EntryBox));
  EntryBox = GTK_WIDGET(gtk_builder_get_object(
      builder, "AddRemoveBullionPlatinumPremiumEntryBox"));
  new_premium = gtk_entry_get_text(GTK_ENTRY(EntryBox));

  if (strtod(new_ounces, NULL) != M->Platinum->ounce_f ||
      strtod(new_premium, NULL) != M->Platinum->premium_f) {
    SqliteAddBullion("platinum", new_ounces, new_premium, M, D);
    new_platinum = true;
  }

  EntryBox = GTK_WIDGET(gtk_builder_get_object(
      builder, "AddRemoveBullionPalladiumOuncesEntryBox"));
  new_ounces = gtk_entry_get_text(GTK_ENTRY(EntryBox));
  EntryBox = GTK_WIDGET(gtk_builder_get_object(
      builder, "AddRemoveBullionPalladiumPremiumEntryBox"));
  new_premium = gtk_entry_get_text(GTK_ENTRY(EntryBox));

  if (strtod(new_ounces, NULL) != M->Palladium->ounce_f ||
      strtod(new_premium, NULL) != M->Palladium->premium_f) {
    SqliteAddBullion("palladium", new_ounces, new_premium, M, D);
    new_palladium = true;
  }

  gboolean new_entry = new_gold || new_silver || new_platinum || new_palladium;
  /* Fetch the data in a separate thread */
  pthread_t thread_id;
  if (new_entry && !package->IsDefaultView())
    pthread_create(&thread_id, NULL, fetch_data_for_new_bullion, package);

  return 0;
}

int BullionCursorMove() {
  GtkWidget *Button =
      GTK_WIDGET(gtk_builder_get_object(builder, "AddRemoveBullionOKBTN"));
  GtkWidget *EntryBox = GTK_WIDGET(
      gtk_builder_get_object(builder, "AddRemoveBullionGoldOuncesEntryBox"));
  const gchar *Gold_Ounces = gtk_entry_get_text(GTK_ENTRY(EntryBox));

  EntryBox = GTK_WIDGET(
      gtk_builder_get_object(builder, "AddRemoveBullionGoldPremiumEntryBox"));
  const gchar *Gold_Premium = gtk_entry_get_text(GTK_ENTRY(EntryBox));

  EntryBox = GTK_WIDGET(
      gtk_builder_get_object(builder, "AddRemoveBullionSilverOuncesEntryBox"));
  const gchar *Silver_Ounces = gtk_entry_get_text(GTK_ENTRY(EntryBox));

  EntryBox = GTK_WIDGET(
      gtk_builder_get_object(builder, "AddRemoveBullionSilverPremiumEntryBox"));
  const gchar *Silver_Premium = gtk_entry_get_text(GTK_ENTRY(EntryBox));

  EntryBox = GTK_WIDGET(gtk_builder_get_object(
      builder, "AddRemoveBullionPlatinumOuncesEntryBox"));
  const gchar *Platinum_Ounces = gtk_entry_get_text(GTK_ENTRY(EntryBox));

  EntryBox = GTK_WIDGET(gtk_builder_get_object(
      builder, "AddRemoveBullionPlatinumPremiumEntryBox"));
  const gchar *Platinum_Premium = gtk_entry_get_text(GTK_ENTRY(EntryBox));

  EntryBox = GTK_WIDGET(gtk_builder_get_object(
      builder, "AddRemoveBullionPalladiumOuncesEntryBox"));
  const gchar *Palladium_Ounces = gtk_entry_get_text(GTK_ENTRY(EntryBox));

  EntryBox = GTK_WIDGET(gtk_builder_get_object(
      builder, "AddRemoveBullionPalladiumPremiumEntryBox"));
  const gchar *Palladium_Premium = gtk_entry_get_text(GTK_ENTRY(EntryBox));

  gboolean valid_num = CheckIfStringDoublePositiveNumber(Gold_Ounces) &
                       CheckIfStringDoublePositiveNumber(Gold_Premium);
  valid_num = valid_num & CheckIfStringDoublePositiveNumber(Silver_Ounces) &
              CheckIfStringDoublePositiveNumber(Silver_Premium);
  valid_num = valid_num & CheckIfStringDoublePositiveNumber(Platinum_Ounces) &
              CheckIfStringDoublePositiveNumber(Platinum_Premium);
  valid_num = valid_num & CheckIfStringDoublePositiveNumber(Palladium_Ounces) &
              CheckIfStringDoublePositiveNumber(Palladium_Premium);

  gboolean valid_string =
      CheckValidString(Gold_Ounces) & CheckValidString(Gold_Premium);
  valid_string = valid_string & CheckValidString(Silver_Ounces) &
                 CheckValidString(Silver_Premium);
  valid_string = valid_string & CheckValidString(Platinum_Ounces) &
                 CheckValidString(Platinum_Premium);
  valid_string = valid_string & CheckValidString(Palladium_Ounces) &
                 CheckValidString(Palladium_Premium);

  if (valid_num && valid_string) {
    gtk_widget_set_sensitive(Button, true);
  } else {
    gtk_widget_set_sensitive(Button, false);
  }

  return 0;
}

int CashShowHide(void *data) {
  /* Unpack the package */
  portfolio_packet *package = (portfolio_packet *)data;
  meta *D = package->GetMetaClass();

  /* get the GObject and cast as a GtkWidget */
  GtkWidget *window =
      GTK_WIDGET(gtk_builder_get_object(builder, "AddRemoveCashWindow"));
  gboolean visible = gtk_widget_is_visible(window);

  if (visible) {
    gtk_widget_set_visible(window, false);
  } else {
    /* Set SpinButton's Value */
    GtkWidget *spinbutton = GTK_WIDGET(
        gtk_builder_get_object(builder, "AddRemoveCashValueSpinBTN"));

    gtk_spin_button_set_value(GTK_SPIN_BUTTON(spinbutton), D->cash_f);
    g_object_set(G_OBJECT(spinbutton), "activates-default", TRUE, NULL);

    gtk_widget_grab_focus(spinbutton);
    gtk_widget_set_visible(window, true);
  }
  return 0;
}

int CashOk(void *data) {
  /* Unpack the package */
  portfolio_packet *package = (portfolio_packet *)data;
  meta *D = package->GetMetaClass();

  GtkWidget *EntryBox =
      GTK_WIDGET(gtk_builder_get_object(builder, "AddRemoveCashValueSpinBTN"));
  const gchar *new_value = gtk_entry_get_text(GTK_ENTRY(EntryBox));

  if (strtod(new_value, NULL) != D->cash_f) {
    SqliteAddCash(new_value, D);
  }

  return 0;
}

int CashCursorMove() {
  GtkWidget *Button =
      GTK_WIDGET(gtk_builder_get_object(builder, "AddRemoveCashOKBTN"));
  GtkWidget *EntryBox =
      GTK_WIDGET(gtk_builder_get_object(builder, "AddRemoveCashValueSpinBTN"));

  const gchar *value = gtk_entry_get_text(GTK_ENTRY(EntryBox));

  if (CheckIfStringDoublePositiveNumber(value) && CheckValidString(value)) {
    gtk_widget_set_sensitive(Button, true);
  } else {
    gtk_widget_set_sensitive(Button, false);
  }

  return 0;
}

int AboutShowHide() {
  /* get the GObject and cast as a GtkWidget */
  GtkWidget *window =
      GTK_WIDGET(gtk_builder_get_object(builder, "AboutWindow"));
  GtkWidget *stack = GTK_WIDGET(gtk_builder_get_object(builder, "AboutStack"));
  gboolean visible = gtk_widget_is_visible(window);

  if (visible) {
    gtk_widget_set_visible(window, false);
    gtk_stack_set_visible_child_name(GTK_STACK(stack), "page0");
  } else {
    gtk_widget_set_visible(window, true);
    gtk_stack_set_visible_child_name(GTK_STACK(stack), "page0");

    window = GTK_WIDGET(gtk_builder_get_object(builder, "AboutScrolledWindow"));
    gtk_scrolled_window_set_vadjustment(GTK_SCROLLED_WINDOW(window), NULL);
  }
  return 0;
}

int ShortcutShowHide() {
  /* get the GObject and cast as a GtkWidget */
  GtkWidget *window =
      GTK_WIDGET(gtk_builder_get_object(builder, "ShortcutWindow"));
  gboolean visible = gtk_widget_is_visible(window);

  if (visible) {
    gtk_widget_set_visible(window, false);
  } else {
    gtk_widget_set_visible(window, true);
  }
  return 0;
}