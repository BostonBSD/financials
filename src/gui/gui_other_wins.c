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
#include "../include/gui.h" /* MainPrimaryTreeview */

#include "../include/class_types.h" /* portfolio_packet, equity_folder, metal, meta */
#include "../include/macros.h"
#include "../include/multicurl.h"
#include "../include/mutex.h"
#include "../include/workfuncs.h"

gint PrefShowHide(portfolio_packet *pkg) {
  meta *D = pkg->GetMetaClass();

  /* get the GObject and cast as a GtkWidget */
  GtkWidget *window = GetWidget("PrefWindow");
  gboolean visible = gtk_widget_is_visible(window);

  if (visible) {
    gtk_widget_set_visible(window, FALSE);
  } else {
    /* If the user deletes the spin button entry, the value is zero.
       This will display a zero when the window is shown. */
    GtkWidget *SpinButton = GetWidget("PrefHoursSpinBox");
    GtkAdjustment *Adjustment =
        gtk_spin_button_get_adjustment(GTK_SPIN_BUTTON(SpinButton));
    gtk_adjustment_set_value(Adjustment, D->updates_hours_f);
    g_object_set(G_OBJECT(SpinButton), "activates-default", TRUE, NULL);

    gtk_widget_set_visible(window, TRUE);
  }
  return 0;
}

gint PrefSymBtnStart() {
  GtkWidget *button = GetWidget("PrefStockSymbolUpdateBTN");
  gtk_widget_set_sensitive(button, FALSE);
  return 0;
}

gint PrefSymBtnStop() {
  GtkWidget *button = GetWidget("PrefStockSymbolUpdateBTN");
  gtk_widget_set_sensitive(button, TRUE);
  return 0;
}

gint PrefSetClockSwitch(gpointer pkg_data) {
  /* Unpack the package */
  portfolio_packet *pkg = (portfolio_packet *)pkg_data;
  meta *D = pkg->GetMetaClass();

  GtkWidget *Switch = GetWidget("PrefShowClocksSwitch");

  /* Temp. Disconnect switch signal handler. */
  g_signal_handlers_disconnect_by_func(
      G_OBJECT(Switch), G_CALLBACK(GUICallback_pref_clock_switch), NULL);

  gtk_switch_set_active(GTK_SWITCH(Switch), D->clocks_displayed_bool);

  g_signal_connect(G_OBJECT(Switch), "state-set",
                   G_CALLBACK(GUICallback_pref_clock_switch), NULL);
  return 0;
}

static void set_api_entry_box(const gchar *entry_box_name_ch,
                              const gchar *value_ch) {
  GtkWidget *EntryBox = GetWidget(entry_box_name_ch);
  gtk_entry_set_text(GTK_ENTRY(EntryBox), value_ch);
  /* If equity url entry box, grab focus */
  if (g_strcmp0(entry_box_name_ch, "ApiEquityUrlEntryBox") == 0)
    gtk_widget_grab_focus(EntryBox);
  g_object_set(G_OBJECT(EntryBox), "activates-default", TRUE, NULL);
}

gint APIShowHide(portfolio_packet *pkg) {
  meta *D = pkg->GetMetaClass();

  GtkWidget *window = GetWidget("ApiWindow");
  gboolean visible = gtk_widget_is_visible(window);

  if (visible) {
    gtk_widget_set_visible(window, FALSE);
  } else {
    GtkWidget *notebook = GetWidget("APINotebook");
    gtk_notebook_set_current_page(GTK_NOTEBOOK(notebook), 0);

    set_api_entry_box("ApiEquityUrlEntryBox", D->stock_url_ch);
    set_api_entry_box("ApiUrlKeyEntryBox", D->curl_key_ch);
    set_api_entry_box("ApiNasdaqSymbolsUrlEntryBox", D->Nasdaq_Symbol_url_ch);
    set_api_entry_box("ApiNYSESymbolsUrlEntryBox", D->NYSE_Symbol_url_ch);

    gtk_widget_set_visible(window, TRUE);
  }
  return 0;
}

static void process_api_data(gchar *cur_value, const gchar *new_value) {
  if ((g_strcmp0(cur_value, new_value) != 0))
    CopyString(&cur_value, new_value);
}

gint APIOk(portfolio_packet *pkg) {
  equity_folder *F = pkg->GetEquityFolderClass();
  meta *D = pkg->GetMetaClass();

  const gchar *new = GetEntryText("ApiEquityUrlEntryBox");
  process_api_data(D->stock_url_ch, new);

  new = GetEntryText("ApiUrlKeyEntryBox");
  process_api_data(D->curl_key_ch, new);

  new = GetEntryText("ApiNasdaqSymbolsUrlEntryBox");
  process_api_data(D->Nasdaq_Symbol_url_ch, new);

  new = GetEntryText("ApiNYSESymbolsUrlEntryBox");
  process_api_data(D->NYSE_Symbol_url_ch, new);

  /* Generate the Equity Request URLs. */
  F->GenerateURL(pkg);
  return 0;
}

gint APICursorMove() {
  GtkWidget *Button = GetWidget("ApiOKBTN");

  const gchar *Equity_URL = GetEntryText("ApiEquityUrlEntryBox");
  const gchar *URL_KEY = GetEntryText("ApiUrlKeyEntryBox");
  const gchar *Nasdaq_URL = GetEntryText("ApiNasdaqSymbolsUrlEntryBox");
  const gchar *NYSE_URL = GetEntryText("ApiNYSESymbolsUrlEntryBox");

  gboolean check = CheckValidString(Equity_URL) & CheckValidString(URL_KEY);
  check = check & CheckValidString(Nasdaq_URL) & CheckValidString(NYSE_URL);

  if (check)
    gtk_widget_set_sensitive(Button, TRUE);
  else
    gtk_widget_set_sensitive(Button, FALSE);

  return 0;
}

gint BullionComBoxChange() {
  GtkWidget *ComboBox = GetWidget("BullionComboBox");
  gint index = gtk_combo_box_get_active(GTK_COMBO_BOX(ComboBox));
  GtkWidget *gold_frame = GetWidget("BullionGoldFrame");
  GtkWidget *silver_frame = GetWidget("BullionSilverFrame");
  GtkWidget *platinum_frame = GetWidget("BullionPlatinumFrame");
  GtkWidget *palladium_frame = GetWidget("BullionPalladiumFrame");
  gboolean gold = FALSE;
  gboolean silver = FALSE;
  gboolean platinum = FALSE;
  gboolean palladium = FALSE;

  enum { GOLD, SILVER, PLATINUM };
  switch (index) {
  case GOLD:
    gold = TRUE;
    break;
  case SILVER:
    silver = TRUE;
    break;
  case PLATINUM:
    platinum = TRUE;
    break;
  default: /* PALLADIUM */
    palladium = TRUE;
    break;
  }

  gtk_widget_set_visible(gold_frame, gold);
  gtk_widget_set_visible(silver_frame, silver);
  gtk_widget_set_visible(platinum_frame, platinum);
  gtk_widget_set_visible(palladium_frame, palladium);
  return 0;
}

static void set_bullion_entry_box(const gchar *entry_box_name_ch, gdouble value,
                                  guint8 digits_right) {
  gchar *temp = NULL;
  GtkWidget *EntryBox = GetWidget(entry_box_name_ch);
  DoubleToFormattedStr(&temp, value, digits_right, NUM_STR);
  ToNumStr(temp); /* remove commas */
  gtk_entry_set_text(GTK_ENTRY(EntryBox), temp);
  /* If gold ounces entry box, grab focus */
  if (g_strcmp0(entry_box_name_ch, "BullionGoldOuncesEntryBox") == 0)
    gtk_widget_grab_focus(EntryBox);
  g_object_set(G_OBJECT(EntryBox), "activates-default", TRUE, NULL);
  g_free(temp);
}

gint BullionShowHide(portfolio_packet *pkg) {
  metal *M = pkg->GetMetalClass();

  GtkWidget *window = GetWidget("BullionWindow");
  gboolean visible = gtk_widget_is_visible(window);

  if (visible) {
    gtk_widget_set_visible(window, FALSE);
  } else {
    /* Set EntryBoxes */
    set_bullion_entry_box("BullionGoldOuncesEntryBox", M->Gold->ounce_f, 4);
    set_bullion_entry_box("BullionGoldPremiumEntryBox", M->Gold->premium_f, 2);
    set_bullion_entry_box("BullionGoldCostEntryBox", M->Gold->cost_basis_f, 2);

    set_bullion_entry_box("BullionSilverOuncesEntryBox", M->Silver->ounce_f, 4);
    set_bullion_entry_box("BullionSilverPremiumEntryBox", M->Silver->premium_f,
                          2);
    set_bullion_entry_box("BullionSilverCostEntryBox", M->Silver->cost_basis_f,
                          2);

    set_bullion_entry_box("BullionPlatinumOuncesEntryBox", M->Platinum->ounce_f,
                          4);
    set_bullion_entry_box("BullionPlatinumPremiumEntryBox",
                          M->Platinum->premium_f, 2);
    set_bullion_entry_box("BullionPlatinumCostEntryBox",
                          M->Platinum->cost_basis_f, 2);

    set_bullion_entry_box("BullionPalladiumOuncesEntryBox",
                          M->Palladium->ounce_f, 4);
    set_bullion_entry_box("BullionPalladiumPremiumEntryBox",
                          M->Palladium->premium_f, 2);
    set_bullion_entry_box("BullionPalladiumCostEntryBox",
                          M->Palladium->cost_basis_f, 2);

    GtkWidget *frame = GetWidget("BullionGoldFrame");
    gtk_widget_set_visible(frame, TRUE);
    frame = GetWidget("BullionSilverFrame");
    gtk_widget_set_visible(frame, FALSE);
    frame = GetWidget("BullionPlatinumFrame");
    gtk_widget_set_visible(frame, FALSE);
    frame = GetWidget("BullionPalladiumFrame");
    gtk_widget_set_visible(frame, FALSE);

    GtkWidget *combobox = GetWidget("BullionComboBox");
    gtk_combo_box_set_active(GTK_COMBO_BOX(combobox), 0);

    gtk_widget_set_visible(window, TRUE);
  }
  return 0;
}

static gboolean process_bullion_data(const gchar *new_ounces_ch,
                                     const gchar *new_premium_ch,
                                     const gchar *new_cost_ch, bullion *B) {
  gboolean new_bullion = FALSE;
  gdouble new_ounces_f = g_strtod(new_ounces_ch, NULL);
  gdouble new_premium_f = g_strtod(new_premium_ch, NULL);
  gdouble new_cost_f = g_strtod(new_cost_ch, NULL);

  if (new_ounces_f != B->ounce_f || new_premium_f != B->premium_f ||
      new_cost_f != B->cost_basis_f) {
    if (B->ounce_f > 0 || new_ounces_f == 0)
      /* We already have data for this bullion.
         Or we are deleting data for this bullion. */
      new_bullion = FALSE;
    else
      /* We need to fetch data for this bullion. */
      new_bullion = TRUE;

    B->ounce_f = new_ounces_f;
    B->premium_f = new_premium_f;
    B->cost_basis_f = new_cost_f;
  }
  return new_bullion;
}

static gboolean
process_bullion_entry_boxes(const gchar *ounces_entry_box_name_ch,
                            const gchar *premium_entry_box_name_ch,
                            const gchar *cost_entry_box_name_ch, bullion *B) {

  const gchar *new_ounces_ch = GetEntryText(ounces_entry_box_name_ch);
  const gchar *new_premium_ch = GetEntryText(premium_entry_box_name_ch);
  const gchar *new_cost_ch = GetEntryText(cost_entry_box_name_ch);

  return process_bullion_data(new_ounces_ch, new_premium_ch, new_cost_ch, B);
}

enum { GOLD, SILVER, PLATINUM, PALLADIUM };
static gboolean process_bullion(const gint metal_int, portfolio_packet *pkg) {
  metal *M = pkg->GetMetalClass();

  const gchar *ounce_entry_name_ch;
  const gchar *premium_entry_name_ch;
  const gchar *cost_entry_name_ch;
  bullion *B;

  switch (metal_int) {
  case GOLD:
    ounce_entry_name_ch = "BullionGoldOuncesEntryBox";
    premium_entry_name_ch = "BullionGoldPremiumEntryBox";
    cost_entry_name_ch = "BullionGoldCostEntryBox";
    B = M->Gold;
    break;
  case SILVER:
    ounce_entry_name_ch = "BullionSilverOuncesEntryBox";
    premium_entry_name_ch = "BullionSilverPremiumEntryBox";
    cost_entry_name_ch = "BullionSilverCostEntryBox";
    B = M->Silver;
    break;
  case PLATINUM:
    ounce_entry_name_ch = "BullionPlatinumOuncesEntryBox";
    premium_entry_name_ch = "BullionPlatinumPremiumEntryBox";
    cost_entry_name_ch = "BullionPlatinumCostEntryBox";
    B = M->Platinum;
    break;
  case PALLADIUM:
    ounce_entry_name_ch = "BullionPalladiumOuncesEntryBox";
    premium_entry_name_ch = "BullionPalladiumPremiumEntryBox";
    cost_entry_name_ch = "BullionPalladiumCostEntryBox";
    B = M->Palladium;
    break;
  }

  return process_bullion_entry_boxes(ounce_entry_name_ch, premium_entry_name_ch,
                                     cost_entry_name_ch, B);
}

gboolean BullionOk(portfolio_packet *pkg) {
  gboolean new_gold_bool = FALSE, new_silver_bool = FALSE,
           new_platinum_bool = FALSE, new_palladium_bool = FALSE;

  new_gold_bool = process_bullion(GOLD, pkg);
  new_silver_bool = process_bullion(SILVER, pkg);
  new_platinum_bool = process_bullion(PLATINUM, pkg);
  new_palladium_bool = process_bullion(PALLADIUM, pkg);

  return new_gold_bool || new_silver_bool || new_platinum_bool ||
         new_palladium_bool;
}

gint BullionCursorMove() {
  GtkWidget *Button = GetWidget("BullionOKBTN");

  const gchar *Gold_Ounces = GetEntryText("BullionGoldOuncesEntryBox");
  const gchar *Gold_Premium = GetEntryText("BullionGoldPremiumEntryBox");
  const gchar *Gold_Cost = GetEntryText("BullionGoldCostEntryBox");

  const gchar *Silver_Ounces = GetEntryText("BullionSilverOuncesEntryBox");
  const gchar *Silver_Premium = GetEntryText("BullionSilverPremiumEntryBox");
  const gchar *Silver_Cost = GetEntryText("BullionSilverCostEntryBox");

  const gchar *Platinum_Ounces = GetEntryText("BullionPlatinumOuncesEntryBox");
  const gchar *Platinum_Premium =
      GetEntryText("BullionPlatinumPremiumEntryBox");
  const gchar *Platinum_Cost = GetEntryText("BullionPlatinumCostEntryBox");

  const gchar *Palladium_Ounces =
      GetEntryText("BullionPalladiumOuncesEntryBox");
  const gchar *Palladium_Premium =
      GetEntryText("BullionPalladiumPremiumEntryBox");
  const gchar *Palladium_Cost = GetEntryText("BullionPalladiumCostEntryBox");

  gboolean valid_num = CheckIfStringDoublePositiveNumber(Gold_Ounces) &
                       CheckIfStringDoublePositiveNumber(Gold_Premium) &
                       CheckIfStringDoublePositiveNumber(Gold_Cost);

  valid_num = valid_num & CheckIfStringDoublePositiveNumber(Silver_Ounces) &
              CheckIfStringDoublePositiveNumber(Silver_Premium) &
              CheckIfStringDoublePositiveNumber(Silver_Cost);

  valid_num = valid_num & CheckIfStringDoublePositiveNumber(Platinum_Ounces) &
              CheckIfStringDoublePositiveNumber(Platinum_Premium) &
              CheckIfStringDoublePositiveNumber(Platinum_Cost);

  valid_num = valid_num & CheckIfStringDoublePositiveNumber(Palladium_Ounces) &
              CheckIfStringDoublePositiveNumber(Palladium_Premium) &
              CheckIfStringDoublePositiveNumber(Palladium_Cost);

  gboolean valid_string = CheckValidString(Gold_Ounces) &
                          CheckValidString(Gold_Premium) &
                          CheckValidString(Gold_Cost);

  valid_string = valid_string & CheckValidString(Silver_Ounces) &
                 CheckValidString(Silver_Premium) &
                 CheckValidString(Silver_Cost);

  valid_string = valid_string & CheckValidString(Platinum_Ounces) &
                 CheckValidString(Platinum_Premium) &
                 CheckValidString(Platinum_Cost);

  valid_string = valid_string & CheckValidString(Palladium_Ounces) &
                 CheckValidString(Palladium_Premium) &
                 CheckValidString(Palladium_Cost);

  if (valid_num && valid_string)
    gtk_widget_set_sensitive(Button, TRUE);
  else
    gtk_widget_set_sensitive(Button, FALSE);

  return 0;
}

gint CashShowHide(portfolio_packet *pkg) {
  meta *D = pkg->GetMetaClass();

  GtkWidget *window = GetWidget("CashWindow");
  gboolean visible = gtk_widget_is_visible(window);

  if (visible) {
    gtk_widget_set_visible(window, FALSE);
  } else {
    /* Set SpinButton's Value */
    GtkWidget *spinbutton = GetWidget("CashSpinBTN");

    gtk_spin_button_set_value(GTK_SPIN_BUTTON(spinbutton), D->cash_f);
    g_object_set(G_OBJECT(spinbutton), "activates-default", TRUE, NULL);

    gtk_widget_grab_focus(spinbutton);
    gtk_widget_set_visible(window, TRUE);
  }
  return 0;
}

gint CashOk(portfolio_packet *pkg) {
  meta *D = pkg->GetMetaClass();

  const gchar *new_value = GetEntryText("CashSpinBTN");
  gdouble new_f = g_strtod(new_value, NULL);

  if (new_f != D->cash_f)
    D->cash_f = new_f;
  return 0;
}

gint CashCursorMove() {
  GtkWidget *Button = GetWidget("CashOKBTN");

  const gchar *value = GetEntryText("CashSpinBTN");

  if (CheckIfStringDoublePositiveNumber(value) && CheckValidString(value))
    gtk_widget_set_sensitive(Button, TRUE);
  else
    gtk_widget_set_sensitive(Button, FALSE);

  return 0;
}

gint AboutShowHide() {
  /* get the GObject and cast as a GtkWidget */
  GtkWidget *window = GetWidget("AboutWindow");
  GtkWidget *stack = GetWidget("AboutStack");
  gboolean visible = gtk_widget_is_visible(window);

  if (visible) {
    gtk_widget_set_visible(window, FALSE);
    gtk_stack_set_visible_child_name(GTK_STACK(stack), "page0");
  } else {
    gtk_widget_set_visible(window, TRUE);
    gtk_stack_set_visible_child_name(GTK_STACK(stack), "page0");

    window = GetWidget("AboutScrolledWindow");
    gtk_scrolled_window_set_vadjustment(GTK_SCROLLED_WINDOW(window), NULL);
  }
  return 0;
}

void AboutSetLabel() {
  /* Set the About window labels. */
  const gchar *text =
      "<a href=\"https://github.com/BostonBSD/financials\">Website</a>";
  GtkWidget *label = GetWidget("AboutWebsiteLabel");
  gtk_label_set_markup(GTK_LABEL(label), text);

  text = "<a href=\"https://www.flaticon.com/free-icons/trends\">Trends "
         "icon</a> designed by Freepik from <a "
         "href=\"https://media.flaticon.com/license/license.pdf\">Flaticon</a>";
  label = GetWidget("AboutTrendsIconLabel");
  gtk_label_set_markup(GTK_LABEL(label), text);

  label = GetWidget("AboutVersionLabel");
  gtk_label_set_label(GTK_LABEL(label), VERSION_STRING);
}

gint HotkeysShowHide() {
  GtkWidget *window = GetWidget("HotkeysWindow");
  gboolean visible = gtk_widget_is_visible(window);

  if (visible)
    gtk_widget_set_visible(window, FALSE);
  else
    gtk_widget_set_visible(window, TRUE);

  return 0;
}