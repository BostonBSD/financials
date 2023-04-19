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

#include "../include/class_types.h" /* Includes portfolio_packet, metal, meta, 
                                             and equity_folder class types */
#include "../include/macros.h"
#include "../include/multicurl.h"
#include "../include/mutex.h"
#include "../include/workfuncs.h"

/* The init and destruct function prototypes for the nested class objects. */
static bullion *class_init_bullion();
static void class_destruct_bullion(bullion *);

/* The static global variable 'MetalClassObject' is always accessed via
 * these functions. */
/* This is an ad-hoc way of self referencing a class.
   It prevents multiple instances of the metal class. */

static metal *
    MetalClassObject; /* A class handle to the bullion class object pointers. */

/* Class Method (also called Function) Definitions */
static void convert_bullion_to_strings(bullion *B, gchar *metal_ch,
                                       guint8 digits_right) {
  /* Basic metal data */
  SymbolStrPango(&B->metal_mrkd_ch, metal_ch, B->ounce_f, 4, BLUE);

  DoubleToFormattedStrPango(&B->premium_mrkd_ch, B->premium_f, digits_right,
                            MON_STR, BLACK);

  DoubleToFormattedStrPango(&B->prev_closing_metal_mrkd_ch,
                            B->prev_closing_metal_f, digits_right, MON_STR,
                            BLACK);

  DoubleToFormattedStrPango(&B->cost_mrkd_ch, B->cost_basis_f, digits_right,
                            MON_STR, BLACK);

  RangeStrPango(&B->range_mrkd_ch, B->low_metal_f, B->high_metal_f,
                digits_right);

  DoubleToFormattedStrPango(&B->spot_price_mrkd_ch, B->spot_price_f,
                            digits_right, MON_STR, BLACK);

  DoubleToFormattedStrPango(&B->premium_mrkd_ch, B->premium_f, digits_right,
                            MON_STR, BLACK);

  /* The change in spot price per ounce. */
  ChangeStrPango(&B->change_ounce_mrkd_ch, B->change_ounce_f,
                 B->change_percent_f, digits_right);

  /* The total invested in this metal */
  TotalStrPango(&B->port_value_mrkd_ch, B->port_value_f, B->change_value_f,
                digits_right);

  /* The raw change in bullion as a percentage. */
  DoubleToFormattedStr(&B->change_percent_raw_ch, B->change_percent_raw_f, 2,
                       PER_STR);

  /* The total cost of this metal. */
  DoubleToFormattedStrPango(&B->total_cost_mrkd_ch, B->total_cost_f,
                            digits_right, MON_STR, BLACK);

  /* The total gain since purchase [value and percentage]. */
  ChangeStrPango(&B->total_gain_mrkd_ch, B->total_gain_value_f,
                 B->total_gain_percent_f, digits_right);
}

static void ToStrings(guint8 digits_right) {
  metal *M = MetalClassObject;
  convert_bullion_to_strings(M->Gold, "Gold", digits_right);
  convert_bullion_to_strings(M->Silver, "Silver", digits_right);
  if (M->Platinum->ounce_f > 0)
    convert_bullion_to_strings(M->Platinum, "Platinum", digits_right);
  if (M->Palladium->ounce_f > 0)
    convert_bullion_to_strings(M->Palladium, "Palladium", digits_right);

  /* The total investment in bullion. */
  DoubleToFormattedStrPango(&M->bullion_port_value_mrkd_ch,
                            M->bullion_port_value_f, digits_right, MON_STR,
                            BLACK);

  /* The change in total investment in bullion. */
  ChangeStrPango(&M->bullion_port_day_gain_mrkd_ch,
                 M->bullion_port_day_gain_val_f, M->bullion_port_day_gain_per_f,
                 digits_right);

  /* The Gold to Silver Ratio */
  DoubleToFormattedStr(&M->gold_silver_ratio_ch, M->gold_silver_ratio_f, 2,
                       NUM_STR);

  /* The total cost of the metal portfolio. */
  DoubleToFormattedStrPango(&M->bullion_port_cost_mrkd_ch,
                            M->bullion_port_cost_f, digits_right, MON_STR,
                            BLACK);

  /* The total portfolio gain since purchase [value and percentage]. */
  ChangeStrPango(&M->bullion_port_total_gain_mrkd_ch,
                 M->bullion_port_total_gain_value_f,
                 M->bullion_port_total_gain_percent_f, digits_right);
}

static void bullion_calculations(bullion *B) {
  /* The total invested in this metal */
  B->port_value_f = (B->spot_price_f + B->premium_f) * B->ounce_f;

  /* The change in spot price per ounce. */
  B->change_ounce_f = B->spot_price_f - B->prev_closing_metal_f;

  /* The change in total investment in this metal. */
  B->change_value_f = B->change_ounce_f * B->ounce_f;

  /* The change in total investment in this metal as a percentage. */
  gdouble prev_total = B->port_value_f - B->change_value_f;
  /* Bullion gain is calculated based off of the total bullion holdings, if
     there is no bullion, the gain is calculated based off of the current and
     prev spot price. These gains are ordinarily
     different because of premiums. */
  if (prev_total == 0)
    B->change_percent_f = CalcGain(B->spot_price_f, B->prev_closing_metal_f);
  else
    B->change_percent_f = CalcGain(B->port_value_f, prev_total);

  /* The raw change in bullion as a percentage. */
  /* This if statement prevent's a "nan%" string in the index label. */
  if (B->prev_closing_metal_f == 0)
    B->change_percent_raw_f = 0.0f;
  else
    B->change_percent_raw_f =
        CalcGain(B->spot_price_f, B->prev_closing_metal_f);

  /* The total cost */
  B->total_cost_f = B->cost_basis_f * B->ounce_f;

  /* The total gain since purchase */
  B->total_gain_value_f = B->port_value_f - B->total_cost_f;

  /* The total gain since purchase, percentage*/
  B->total_gain_percent_f = CalcGain(B->port_value_f, B->total_cost_f);
}

static void Calculate() {
  metal *M = MetalClassObject;

  bullion_calculations(M->Gold);
  bullion_calculations(M->Silver);
  /* There's no if statement here.  If the user removes either of the following
   two metals, we want the port_value_f to be zero (notice that the following
   statements always account for all metals, these two statements reset the
   values). */
  bullion_calculations(M->Platinum);
  bullion_calculations(M->Palladium);

  /* The total investment in bullion. */
  M->bullion_port_value_f = M->Gold->port_value_f + M->Silver->port_value_f +
                            M->Platinum->port_value_f +
                            M->Palladium->port_value_f;

  /* The change in total investment in bullion. */
  M->bullion_port_day_gain_val_f = M->Gold->change_value_f;
  M->bullion_port_day_gain_val_f += M->Silver->change_value_f;
  M->bullion_port_day_gain_val_f += M->Platinum->change_value_f;
  M->bullion_port_day_gain_val_f += M->Palladium->change_value_f;

  /* The change in total investment in bullion as a percentage. */
  gdouble prev_total = M->bullion_port_value_f - M->bullion_port_day_gain_val_f;
  if (prev_total == 0.0f)
    M->bullion_port_day_gain_per_f = 0.0f;
  else
    M->bullion_port_day_gain_per_f =
        CalcGain(M->bullion_port_value_f, prev_total);

  /* The Gold to Silver Ratio */
  if (M->Silver->spot_price_f > 0)
    M->gold_silver_ratio_f = M->Gold->spot_price_f / M->Silver->spot_price_f;

  /* The total cost of all bullion */
  M->bullion_port_cost_f = M->Gold->total_cost_f + M->Silver->total_cost_f +
                           M->Platinum->total_cost_f +
                           M->Palladium->total_cost_f;

  /* The total gain of all bullion, since purchase, value. */
  M->bullion_port_total_gain_value_f =
      M->bullion_port_value_f - M->bullion_port_cost_f;

  /* The total gain of all bullion, since purchase, percentage. */
  M->bullion_port_total_gain_percent_f =
      CalcGain(M->bullion_port_value_f, M->bullion_port_cost_f);
}

static gint SetUpCurl(portfolio_packet *pkg) {
  metal *M = pkg->GetMetalClass();

  /* The start time needs to be a week before the current time, so seven days.
   * This compensates for weekends and holidays and ensures enough data. */
  guint period = (86400 * 7);
  GetYahooUrl(&M->Gold->url_ch, "GC=F", period);
  GetYahooUrl(&M->Silver->url_ch, "SI=F", period);
  if (M->Platinum->ounce_f > 0)
    GetYahooUrl(&M->Platinum->url_ch, "PL=F", period);
  if (M->Palladium->ounce_f > 0)
    GetYahooUrl(&M->Palladium->url_ch, "PA=F", period);

  SetUpCurlHandle(M->Silver->YAHOO_hnd, pkg->multicurl_main_hnd,
                  M->Silver->url_ch, &M->Silver->CURLDATA);
  SetUpCurlHandle(M->Gold->YAHOO_hnd, pkg->multicurl_main_hnd, M->Gold->url_ch,
                  &M->Gold->CURLDATA);
  if (M->Platinum->ounce_f > 0)
    SetUpCurlHandle(M->Platinum->YAHOO_hnd, pkg->multicurl_main_hnd,
                    M->Platinum->url_ch, &M->Platinum->CURLDATA);
  if (M->Palladium->ounce_f > 0)
    SetUpCurlHandle(M->Palladium->YAHOO_hnd, pkg->multicurl_main_hnd,
                    M->Palladium->url_ch, &M->Palladium->CURLDATA);

  return 0;
}

static void extract_bullion_data_reset(bullion *B) {
  B->prev_closing_metal_f = 0.0f;
  B->high_metal_f = 0.0f;
  B->low_metal_f = 0.0f;
  B->spot_price_f = 0.0f;
  FreeMemtype(&B->CURLDATA);
}

static void extract_bullion_data(bullion *B) {
  if (B->CURLDATA.memory == NULL) {
    extract_bullion_data_reset(B);
    return;
  }

  /* Convert a String to a File Pointer Stream for Reading */
  FILE *fp = fmemopen((gpointer)B->CURLDATA.memory, B->CURLDATA.size + 1, "r");

  if (fp == NULL) {
    extract_bullion_data_reset(B);
    return;
  }

  char **token_arr;
  gdouble prev_closing = 0.0f, cur_price = 0.0f;
  gchar *line = ExtractYahooData(fp, &prev_closing, &cur_price);

  if (line) {
    token_arr = g_strsplit(line, ",", -1);
    B->prev_closing_metal_f = prev_closing;
    B->high_metal_f = g_strtod(token_arr[2] ? token_arr[2] : "0", NULL);
    B->low_metal_f = g_strtod(token_arr[3] ? token_arr[3] : "0", NULL);
    B->spot_price_f = cur_price;

    g_strfreev(token_arr);
  }
  g_free(line);
  fclose(fp);
  FreeMemtype(&B->CURLDATA);
}

static void ExtractData() {
  metal *M = MetalClassObject;

  extract_bullion_data(M->Gold);
  extract_bullion_data(M->Silver);
  if (M->Platinum->ounce_f > 0)
    extract_bullion_data(M->Platinum);
  if (M->Palladium->ounce_f > 0)
    extract_bullion_data(M->Palladium);
}

/* Class Init Functions */
static bullion *class_init_bullion() {
  /* Allocate Memory For A New Class Object */
  bullion *new_class = (bullion *)g_malloc(sizeof(*new_class));

  /* Initialize Variables */
  new_class->ounce_f = 0.0f;
  new_class->spot_price_f = 0.0f;
  new_class->premium_f = 0.0f;
  new_class->port_value_f = 0.0f;

  new_class->high_metal_f = 0.0f;
  new_class->low_metal_f = 0.0f;
  new_class->prev_closing_metal_f = 0.0f;
  new_class->change_ounce_f = 0.0f;
  new_class->change_value_f = 0.0f;
  new_class->change_percent_f = 0.0f;
  new_class->change_percent_raw_f = 0.0f;

  new_class->cost_basis_f = 0.0f;
  new_class->total_cost_f = 0.0f;
  new_class->total_gain_value_f = 0.0f;
  new_class->total_gain_percent_f = 0.0f;

  new_class->url_ch = NULL;

  new_class->metal_mrkd_ch = NULL;
  new_class->spot_price_mrkd_ch = NULL;
  new_class->premium_mrkd_ch = NULL;
  new_class->port_value_mrkd_ch = NULL;

  new_class->cost_mrkd_ch = NULL;
  new_class->range_mrkd_ch = NULL;
  new_class->prev_closing_metal_mrkd_ch = NULL;
  new_class->change_ounce_mrkd_ch = NULL;
  new_class->change_percent_raw_ch = NULL;
  new_class->total_cost_mrkd_ch = NULL;
  new_class->total_gain_mrkd_ch = NULL;

  new_class->YAHOO_hnd = curl_easy_init();
  new_class->CURLDATA.memory = NULL;
  new_class->CURLDATA.size = 0;

  /* Return Our Initialized Class */
  return new_class;
}

metal *ClassInitMetal() {
  /* Allocate Memory For A New Class */
  metal *new_class = (metal *)g_malloc(sizeof(*new_class));

  /* Initialize Nested Class Objects. */
  new_class->Gold = class_init_bullion();
  new_class->Silver = class_init_bullion();
  new_class->Platinum = class_init_bullion();
  new_class->Palladium = class_init_bullion();

  /* Initialize Variables */
  new_class->bullion_port_value_f = 0.0f;
  new_class->bullion_port_day_gain_val_f = 0.0f;
  new_class->bullion_port_day_gain_per_f = 0.0f;
  new_class->gold_silver_ratio_f = 0.0f;

  new_class->bullion_port_cost_f = 0.0f;
  new_class->bullion_port_total_gain_value_f = 0.0f;
  new_class->bullion_port_total_gain_percent_f = 0.0f;

  new_class->bullion_port_value_mrkd_ch = NULL;
  new_class->bullion_port_day_gain_mrkd_ch = NULL;
  new_class->gold_silver_ratio_ch = NULL;

  new_class->bullion_port_cost_mrkd_ch = NULL;
  new_class->bullion_port_total_gain_mrkd_ch = NULL;

  /* Connect Function Pointers To Function Definitions */
  new_class->ToStrings = ToStrings;
  new_class->Calculate = Calculate;
  new_class->SetUpCurl = SetUpCurl;
  new_class->ExtractData = ExtractData;

  /* Set the static global variable so we can self-reference this class. */
  MetalClassObject = new_class;

  /* Return Our Initialized Class */
  return new_class;
}

/* Class Destruct Functions */
static void class_destruct_bullion(bullion *bullion_class) {
  /* Free Memory */
  if (bullion_class->metal_mrkd_ch)
    g_free(bullion_class->metal_mrkd_ch);
  if (bullion_class->url_ch)
    g_free(bullion_class->url_ch);
  if (bullion_class->spot_price_mrkd_ch)
    g_free(bullion_class->spot_price_mrkd_ch);
  if (bullion_class->premium_mrkd_ch)
    g_free(bullion_class->premium_mrkd_ch);
  if (bullion_class->port_value_mrkd_ch)
    g_free(bullion_class->port_value_mrkd_ch);

  if (bullion_class->cost_mrkd_ch)
    g_free(bullion_class->cost_mrkd_ch);
  if (bullion_class->range_mrkd_ch)
    g_free(bullion_class->range_mrkd_ch);
  if (bullion_class->prev_closing_metal_mrkd_ch)
    g_free(bullion_class->prev_closing_metal_mrkd_ch);
  if (bullion_class->change_ounce_mrkd_ch)
    g_free(bullion_class->change_ounce_mrkd_ch);
  if (bullion_class->change_percent_raw_ch)
    g_free(bullion_class->change_percent_raw_ch);
  if (bullion_class->total_cost_mrkd_ch)
    g_free(bullion_class->total_cost_mrkd_ch);
  if (bullion_class->total_gain_mrkd_ch)
    g_free(bullion_class->total_gain_mrkd_ch);

  if (bullion_class->YAHOO_hnd)
    curl_easy_cleanup(bullion_class->YAHOO_hnd);

  FreeMemtype(&bullion_class->CURLDATA);

  /* Free Memory From Class Object */
  if (bullion_class) {
    g_free(bullion_class);
    bullion_class = NULL;
  }
}

void ClassDestructMetal(metal *metal_handle) {
  /* Free Memory From Class Objects */
  if (metal_handle->Gold)
    class_destruct_bullion(metal_handle->Gold);
  if (metal_handle->Silver)
    class_destruct_bullion(metal_handle->Silver);
  if (metal_handle->Platinum)
    class_destruct_bullion(metal_handle->Platinum);
  if (metal_handle->Palladium)
    class_destruct_bullion(metal_handle->Palladium);

  /* Free Pointer Memory */
  if (metal_handle->bullion_port_value_mrkd_ch)
    g_free(metal_handle->bullion_port_value_mrkd_ch);
  if (metal_handle->bullion_port_day_gain_mrkd_ch)
    g_free(metal_handle->bullion_port_day_gain_mrkd_ch);
  if (metal_handle->gold_silver_ratio_ch)
    g_free(metal_handle->gold_silver_ratio_ch);
  if (metal_handle->bullion_port_cost_mrkd_ch)
    g_free(metal_handle->bullion_port_cost_mrkd_ch);
  if (metal_handle->bullion_port_total_gain_mrkd_ch)
    g_free(metal_handle->bullion_port_total_gain_mrkd_ch);

  if (metal_handle)
    g_free(metal_handle);
}