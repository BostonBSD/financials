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
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include <gtk/gtk.h>

#include "../include/gui.h" /* MainPrimaryTreeview */

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
  GtkWidget *window = GetWidget("PreferencesWindow");
  gboolean visible = gtk_widget_is_visible(window);

  if (visible) {
    gtk_widget_set_visible(window, false);
  } else {
    /* If the user deletes the spin button entry, the value is zero.
       This will display a zero when the window is shown. */
    GtkWidget *SpinButton = GetWidget("PrefHoursSpinBox");
    GtkAdjustment *Adjustment =
        gtk_spin_button_get_adjustment(GTK_SPIN_BUTTON(SpinButton));
    gtk_adjustment_set_value(Adjustment, D->updates_hours_f);
    g_object_set(G_OBJECT(SpinButton), "activates-default", TRUE, NULL);

    gtk_widget_set_visible(window, true);
  }
  return 0;
}

int PrefSymBtnStart() {
  GtkWidget *button = GetWidget("PrefStockSymbolUpdateBTN");
  gtk_widget_set_sensitive(button, false);
  return 0;
}

int PrefSymBtnStop() {
  GtkWidget *button = GetWidget("PrefStockSymbolUpdateBTN");
  gtk_widget_set_sensitive(button, true);
  return 0;
}

static void set_api_entry_box(const char *entry_box_name_ch,
                              const char *value_ch) {
  GtkWidget *EntryBox = GetWidget(entry_box_name_ch);
  gtk_entry_set_text(GTK_ENTRY(EntryBox), value_ch);
  /* If equity url entry box, grab focus */
  if (strcasecmp(entry_box_name_ch, "ChangeApiInfoEquityUrlEntryBox") == 0)
    gtk_widget_grab_focus(EntryBox);
  g_object_set(G_OBJECT(EntryBox), "activates-default", TRUE, NULL);
}

int APIShowHide(void *data) {
  /* Unpack the package */
  portfolio_packet *package = (portfolio_packet *)data;
  meta *D = package->GetMetaClass();

  GtkWidget *window = GetWidget("ChangeApiInfoWindow");
  gboolean visible = gtk_widget_is_visible(window);

  if (visible) {
    gtk_widget_set_visible(window, false);
  } else {
    GtkWidget *notebook = GetWidget("ChangeAPINotebook");
    gtk_notebook_set_current_page(GTK_NOTEBOOK(notebook), 0);

    set_api_entry_box("ChangeApiInfoEquityUrlEntryBox", D->stock_url_ch);
    set_api_entry_box("ChangeApiInfoUrlKeyEntryBox", D->curl_key_ch);
    set_api_entry_box("ChangeApiInfoNasdaqSymbolsUrlEntryBox",
                      D->Nasdaq_Symbol_url_ch);
    set_api_entry_box("ChangeApiInfoNYSESymbolsUrlEntryBox",
                      D->NYSE_Symbol_url_ch);

    gtk_widget_set_visible(window, true);
  }
  return 0;
}

static void process_api_data(char *keyword, char *cur_value,
                             const char *new_value, meta *D) {
  if ((strcmp(cur_value, new_value) != 0)) {
    SqliteAddAPIData(keyword, new_value, D);
    CopyString(&cur_value, new_value);
  }
}

int APIOk(void *data) {
  /* Unpack the package */
  portfolio_packet *package = (portfolio_packet *)data;
  equity_folder *F = package->GetEquityFolderClass();
  meta *D = package->GetMetaClass();

  const gchar *new = GetEntryText("ChangeApiInfoEquityUrlEntryBox");
  process_api_data("Stock_URL", D->stock_url_ch, new, D);

  new = GetEntryText("ChangeApiInfoUrlKeyEntryBox");
  process_api_data("URL_KEY", D->curl_key_ch, new, D);

  new = GetEntryText("ChangeApiInfoNasdaqSymbolsUrlEntryBox");
  process_api_data("Nasdaq_Symbol_URL", D->Nasdaq_Symbol_url_ch, new, D);

  new = GetEntryText("ChangeApiInfoNYSESymbolsUrlEntryBox");
  process_api_data("NYSE_Symbol_URL", D->NYSE_Symbol_url_ch, new, D);

  /* Generate the Equity Request URLs. */
  F->GenerateURL(package);
  return 0;
}

int APICursorMove() {
  GtkWidget *Button = GetWidget("ChangeApiInfoOKBTN");

  const gchar *Equity_URL = GetEntryText("ChangeApiInfoEquityUrlEntryBox");
  const gchar *URL_KEY = GetEntryText("ChangeApiInfoUrlKeyEntryBox");
  const gchar *Nasdaq_URL =
      GetEntryText("ChangeApiInfoNasdaqSymbolsUrlEntryBox");
  const gchar *NYSE_URL = GetEntryText("ChangeApiInfoNYSESymbolsUrlEntryBox");

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
  GtkWidget *ComboBox = GetWidget("AddRemoveBullionComboBox");
  gint index = gtk_combo_box_get_active(GTK_COMBO_BOX(ComboBox));
  GtkWidget *gold_frame = GetWidget("AddRemoveBullionGoldFrame");
  GtkWidget *silver_frame = GetWidget("AddRemoveBullionSilverFrame");
  GtkWidget *platinum_frame = GetWidget("AddRemoveBullionPlatinumFrame");
  GtkWidget *palladium_frame = GetWidget("AddRemoveBullionPalladiumFrame");

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

static void set_bullion_entry_box(const char *entry_box_name_ch, double value,
                                  unsigned short digits_right) {
  gchar *temp = NULL;
  GtkWidget *EntryBox = GetWidget(entry_box_name_ch);
  DoubleToNumStr(&temp, value, digits_right);
  ToNumStr(temp); /* remove commas */
  gtk_entry_set_text(GTK_ENTRY(EntryBox), temp);
  /* If gold ounces entry box, grab focus */
  if (strcasecmp(entry_box_name_ch, "AddRemoveBullionGoldOuncesEntryBox") == 0)
    gtk_widget_grab_focus(EntryBox);
  g_object_set(G_OBJECT(EntryBox), "activates-default", TRUE, NULL);
  g_free(temp);
}

int BullionShowHide(void *data) {
  /* Unpack the package */
  portfolio_packet *package = (portfolio_packet *)data;
  metal *M = package->GetMetalClass();

  GtkWidget *window = GetWidget("AddRemoveBullionWindow");
  gboolean visible = gtk_widget_is_visible(window);

  if (visible) {
    gtk_widget_set_visible(window, false);
  } else {
    /* Set EntryBoxes */
    set_bullion_entry_box("AddRemoveBullionGoldOuncesEntryBox",
                          M->Gold->ounce_f, 4);
    set_bullion_entry_box("AddRemoveBullionGoldPremiumEntryBox",
                          M->Gold->premium_f, 2);

    set_bullion_entry_box("AddRemoveBullionSilverOuncesEntryBox",
                          M->Silver->ounce_f, 4);
    set_bullion_entry_box("AddRemoveBullionSilverPremiumEntryBox",
                          M->Silver->premium_f, 2);

    set_bullion_entry_box("AddRemoveBullionPlatinumOuncesEntryBox",
                          M->Platinum->ounce_f, 4);
    set_bullion_entry_box("AddRemoveBullionPlatinumPremiumEntryBox",
                          M->Platinum->premium_f, 2);

    set_bullion_entry_box("AddRemoveBullionPalladiumOuncesEntryBox",
                          M->Palladium->ounce_f, 4);
    set_bullion_entry_box("AddRemoveBullionPalladiumPremiumEntryBox",
                          M->Palladium->premium_f, 2);

    GtkWidget *frame = GetWidget("AddRemoveBullionGoldFrame");
    gtk_widget_set_visible(frame, true);
    frame = GetWidget("AddRemoveBullionSilverFrame");
    gtk_widget_set_visible(frame, false);
    frame = GetWidget("AddRemoveBullionPlatinumFrame");
    gtk_widget_set_visible(frame, false);
    frame = GetWidget("AddRemoveBullionPalladiumFrame");
    gtk_widget_set_visible(frame, false);

    GtkWidget *combobox = GetWidget("AddRemoveBullionComboBox");
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

  /* Ensures that pkg->multicurl_main_hnd is free to use. */
  pthread_mutex_lock(&mutex_working[CLASS_MEMBER_MUTEX]);

  /* This func doesn't have a mutex. */
  M->SetUpCurl(pkg);

  /* Perform the cURL requests simultaneously using multi-cURL. */
  int return_code =
      PerformMultiCurl(pkg->multicurl_main_hnd, (double)num_metals);
  if (return_code) {
    FreeMemtype(&M->Gold->CURLDATA);
    FreeMemtype(&M->Silver->CURLDATA);
    FreeMemtype(&M->Platinum->CURLDATA);
    FreeMemtype(&M->Palladium->CURLDATA);
  }

  /* This func doesn't have a mutex. */
  M->ExtractData();

  pthread_mutex_unlock(&mutex_working[CLASS_MEMBER_MUTEX]);

  /* These funcs have mutexes */
  pkg->Calculate();
  pkg->ToStrings();

  /* Reset the progressbar */
  gdk_threads_add_idle(MainProgBarReset, NULL);

  /* Update the main window treeview. */
  gdk_threads_add_idle(MainPrimaryTreeview, data);

  pthread_exit(NULL);
}

static bool process_bullion_data(const char *metal_ch,
                                 const char *new_ounces_ch,
                                 const char *new_premium_ch, bullion *B,
                                 meta *D) {
  bool new_bullion = false;
  double new_ounces_f = strtod(new_ounces_ch, NULL);
  double new_premium_f = strtod(new_premium_ch, NULL);

  if (new_ounces_f != B->ounce_f || new_premium_f != B->premium_f) {
    if (B->ounce_f > 0 || new_ounces_f == 0) {
      /* We already have data for this bullion.
         Or we are deleting data for this bullion. */
      new_bullion = false;
    } else {
      /* We need to fetch data for this bullion. */
      new_bullion = true;
    }
    SqliteAddBullion(metal_ch, new_ounces_ch, new_premium_ch, D);
    B->ounce_f = new_ounces_f;
    B->premium_f = new_premium_f;
  }
  return new_bullion;
}

static bool process_bullion_entry_boxes(const char *metal_ch,
                                        const char *ounces_entry_box_name_ch,
                                        const char *premium_entry_box_name_ch,
                                        bullion *B, meta *D) {

  const gchar *new_ounces_ch = GetEntryText(ounces_entry_box_name_ch);
  const gchar *new_premium_ch = GetEntryText(premium_entry_box_name_ch);

  return process_bullion_data(metal_ch, new_ounces_ch, new_premium_ch, B, D);
}

enum { GOLD, SILVER, PLATINUM, PALLADIUM };
static bool process_bullion(const int metal_int, portfolio_packet *pkg) {
  metal *M = pkg->GetMetalClass();
  meta *D = pkg->GetMetaClass();

  const char *ounce_entry_name_ch;
  const char *premium_entry_name_ch;
  const char *metal_ch;
  bullion *B;

  switch (metal_int) {
  case GOLD:
    ounce_entry_name_ch = "AddRemoveBullionGoldOuncesEntryBox";
    premium_entry_name_ch = "AddRemoveBullionGoldPremiumEntryBox";
    metal_ch = "gold";
    B = M->Gold;
    break;
  case SILVER:
    ounce_entry_name_ch = "AddRemoveBullionSilverOuncesEntryBox";
    premium_entry_name_ch = "AddRemoveBullionSilverPremiumEntryBox";
    metal_ch = "silver";
    B = M->Silver;
    break;
  case PLATINUM:
    ounce_entry_name_ch = "AddRemoveBullionPlatinumOuncesEntryBox";
    premium_entry_name_ch = "AddRemoveBullionPlatinumPremiumEntryBox";
    metal_ch = "platinum";
    B = M->Platinum;
    break;
  case PALLADIUM:
    ounce_entry_name_ch = "AddRemoveBullionPalladiumOuncesEntryBox";
    premium_entry_name_ch = "AddRemoveBullionPalladiumPremiumEntryBox";
    metal_ch = "palladium";
    B = M->Palladium;
    break;
  }

  return process_bullion_entry_boxes(metal_ch, ounce_entry_name_ch,
                                     premium_entry_name_ch, B, D);
}

int BullionOk(void *data) {
  /* Unpack the package */
  portfolio_packet *package = (portfolio_packet *)data;
  bool new_gold_bool = false, new_silver_bool = false,
       new_platinum_bool = false, new_palladium_bool = false;

  new_gold_bool = process_bullion(GOLD, package);
  new_silver_bool = process_bullion(SILVER, package);
  new_platinum_bool = process_bullion(PLATINUM, package);
  new_palladium_bool = process_bullion(PALLADIUM, package);

  bool new_entry_bool = new_gold_bool || new_silver_bool || new_platinum_bool ||
                        new_palladium_bool;

  /* If we need data to update the main treeview with new bullion data. */
  if (new_entry_bool && !package->IsDefaultView()) {
    /* Fetch the data in a separate thread */
    pthread_t thread_id;
    pthread_create(&thread_id, NULL, fetch_data_for_new_bullion, package);
    pthread_detach(thread_id);

    /* If we need to update the main treeview. */
  } else if (!package->IsDefaultView()) {
    package->Calculate();
    package->ToStrings();
    gdk_threads_add_idle(MainPrimaryTreeview, package);

    /* If we need to update the default treeview. */
  } else {
    package->ToStrings();
    gdk_threads_add_idle(MainDefaultTreeview, package);
  }

  return 0;
}

int BullionCursorMove() {
  GtkWidget *Button = GetWidget("AddRemoveBullionOKBTN");

  const gchar *Gold_Ounces = GetEntryText("AddRemoveBullionGoldOuncesEntryBox");
  const gchar *Gold_Premium =
      GetEntryText("AddRemoveBullionGoldPremiumEntryBox");

  const gchar *Silver_Ounces =
      GetEntryText("AddRemoveBullionSilverOuncesEntryBox");
  const gchar *Silver_Premium =
      GetEntryText("AddRemoveBullionSilverPremiumEntryBox");

  const gchar *Platinum_Ounces =
      GetEntryText("AddRemoveBullionPlatinumOuncesEntryBox");
  const gchar *Platinum_Premium =
      GetEntryText("AddRemoveBullionPlatinumPremiumEntryBox");

  const gchar *Palladium_Ounces =
      GetEntryText("AddRemoveBullionPalladiumOuncesEntryBox");
  const gchar *Palladium_Premium =
      GetEntryText("AddRemoveBullionPalladiumPremiumEntryBox");

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

  GtkWidget *window = GetWidget("AddRemoveCashWindow");
  gboolean visible = gtk_widget_is_visible(window);

  if (visible) {
    gtk_widget_set_visible(window, false);
  } else {
    /* Set SpinButton's Value */
    GtkWidget *spinbutton = GetWidget("AddRemoveCashValueSpinBTN");

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

  const gchar *new_value = GetEntryText("AddRemoveCashValueSpinBTN");
  double new_f = strtod(new_value, NULL);

  if (new_f != D->cash_f) {
    D->cash_f = new_f;
    SqliteAddCash(new_value, D);
  }
  return 0;
}

int CashCursorMove() {
  GtkWidget *Button = GetWidget("AddRemoveCashOKBTN");

  const gchar *value = GetEntryText("AddRemoveCashValueSpinBTN");

  if (CheckIfStringDoublePositiveNumber(value) && CheckValidString(value)) {
    gtk_widget_set_sensitive(Button, true);
  } else {
    gtk_widget_set_sensitive(Button, false);
  }

  return 0;
}

int AboutShowHide() {
  /* get the GObject and cast as a GtkWidget */
  GtkWidget *window = GetWidget("AboutWindow");
  GtkWidget *stack = GetWidget("AboutStack");
  gboolean visible = gtk_widget_is_visible(window);

  if (visible) {
    gtk_widget_set_visible(window, false);
    gtk_stack_set_visible_child_name(GTK_STACK(stack), "page0");
  } else {
    gtk_widget_set_visible(window, true);
    gtk_stack_set_visible_child_name(GTK_STACK(stack), "page0");

    window = GetWidget("AboutScrolledWindow");
    gtk_scrolled_window_set_vadjustment(GTK_SCROLLED_WINDOW(window), NULL);
  }
  return 0;
}

int HotkeysShowHide() {
  GtkWidget *window = GetWidget("ShortcutWindow");
  gboolean visible = gtk_widget_is_visible(window);

  if (visible) {
    gtk_widget_set_visible(window, false);
  } else {
    gtk_widget_set_visible(window, true);
  }
  return 0;
}