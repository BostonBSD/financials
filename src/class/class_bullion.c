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

#include <stdlib.h>
#include <string.h>
#include <time.h> /* time_t, struct tm, time ()  */

#include "../include/class_types.h" /* Includes portfolio_packet, metal, meta, 
                                             and equity_folder class types */

#include "../include/csv.h"
#include "../include/macros.h"
#include "../include/multicurl.h"
#include "../include/mutex.h"
#include "../include/workfuncs.h"

/* The init and destruct function prototypes for the nested class objects. */
static bullion *class_init_bullion();
static void class_destruct_bullion(bullion *);

/* The static file-global variable 'MetalClassObject' is always accessed via
 * these functions. */
/* This is an ad-hoc way of self referencing a class.
   It prevents multiple instances of the metal class. */

static metal *
    MetalClassObject; /* A class handle to the bullion class object pointers. */

/* Class Method (also called Function) Definitions */
static void convert_bullion_to_strings(bullion *B,
                                       unsigned short digits_right) {
  /* Basic metal data */
  DoubleToNumStrPango(&B->ounce_mrkd_ch, B->ounce_f, 4);

  DoubleToMonStrPango(&B->premium_mrkd_ch, B->premium_f, digits_right);

  DoubleToMonStrPango(&B->prev_closing_metal_mrkd_ch, B->prev_closing_metal_f,
                      digits_right);

  DoubleToMonStrPango(&B->high_metal_mrkd_ch, B->high_metal_f, digits_right);

  DoubleToMonStrPango(&B->low_metal_mrkd_ch, B->low_metal_f, digits_right);

  DoubleToMonStrPango(&B->spot_price_mrkd_ch, B->spot_price_f, digits_right);

  DoubleToMonStrPango(&B->premium_mrkd_ch, B->premium_f, digits_right);

  /* The total invested in this metal */
  DoubleToMonStrPango(&B->port_value_mrkd_ch, B->port_value_f, digits_right);

  /* The change in spot price per ounce. */
  DoubleToMonStrPangoColor(&B->change_ounce_mrkd_ch, B->change_ounce_f,
                           digits_right, NOT_ITALIC);

  /* The change in total investment in this metal. */
  DoubleToMonStrPangoColor(&B->change_value_mrkd_ch, B->change_value_f,
                           digits_right, NOT_ITALIC);

  /* The change in total investment in this metal as a percentage. */
  DoubleToPerStrPangoColor(&B->change_percent_mrkd_ch, B->change_percent_f,
                           digits_right, NOT_ITALIC);

  /* The raw change in bullion as a percentage. */
  DoubleToPerStr(&B->change_percent_raw_ch, B->change_percent_raw_f, 2);
}

static void ToStrings(unsigned short digits_right) {
  metal *M = MetalClassObject;
  convert_bullion_to_strings(M->Gold, digits_right);
  convert_bullion_to_strings(M->Silver, digits_right);
  if (M->Platinum->ounce_f > 0)
    convert_bullion_to_strings(M->Platinum, digits_right);
  if (M->Palladium->ounce_f > 0)
    convert_bullion_to_strings(M->Palladium, digits_right);

  /* The total investment in bullion. */
  DoubleToMonStrPango(&M->bullion_port_value_mrkd_ch, M->bullion_port_value_f,
                      digits_right);

  /* The change in total investment in bullion. */
  DoubleToMonStrPangoColor(&M->bullion_port_value_chg_mrkd_ch,
                           M->bullion_port_value_chg_f, digits_right,
                           NOT_ITALIC);

  /* The change in total investment in bullion as a percentage. */
  DoubleToPerStrPangoColor(&M->bullion_port_value_p_chg_mrkd_ch,
                           M->bullion_port_value_p_chg_f, digits_right,
                           NOT_ITALIC);

  /* The Gold to Silver Ratio */
  DoubleToNumStr(&M->gold_silver_ratio_ch, M->gold_silver_ratio_f, 2);
}

static double stake(const double ounces, const double prem,
                    const double price) {
  return ((prem + price) * ounces);
}

static void bullion_calculations(bullion *B) {
  /* The total invested in this metal */
  B->port_value_f = stake(B->ounce_f, B->premium_f, B->spot_price_f);

  /* The change in spot price per ounce. */
  B->change_ounce_f = B->spot_price_f - B->prev_closing_metal_f;

  /* The change in total investment in this metal. */
  B->change_value_f = B->change_ounce_f * B->ounce_f;

  /* The change in total investment in this metal as a percentage. */
  double prev_total = B->port_value_f - B->change_value_f;
  /* Bullion gain is calculated based off of the total bullion holdings, if
     there is no bullion, the gain is calculated based off of the current and
     prev spot price. These gains are ordinarily
     different because of premiums. */
  if (prev_total == 0) {
    B->change_percent_f = CalcGain(B->spot_price_f, B->prev_closing_metal_f);
  } else {
    B->change_percent_f = CalcGain(B->port_value_f, prev_total);
  }

  /* The raw change in bullion as a percentage. */
  /* This if statement prevent's a "nan%" string in the index label. */
  if (B->prev_closing_metal_f == 0) {
    B->change_percent_raw_f = 0.0f;
  } else {
    B->change_percent_raw_f =
        CalcGain(B->spot_price_f, B->prev_closing_metal_f);
  }
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
  M->bullion_port_value_chg_f = M->Gold->change_value_f;
  M->bullion_port_value_chg_f += M->Silver->change_value_f;
  M->bullion_port_value_chg_f += M->Platinum->change_value_f;
  M->bullion_port_value_chg_f += M->Palladium->change_value_f;

  /* The change in total investment in bullion as a percentage. */
  double prev_total = M->bullion_port_value_f - M->bullion_port_value_chg_f;
  if (prev_total == 0.0f) {
    M->bullion_port_value_p_chg_f = 0.0f;
  } else {
    M->bullion_port_value_p_chg_f =
        CalcGain(M->bullion_port_value_f, prev_total);
  }

  /* The Gold to Silver Ratio */
  if (M->Silver->spot_price_f > 0)
    M->gold_silver_ratio_f = M->Gold->spot_price_f / M->Silver->spot_price_f;
}

static void create_bullion_url(bullion *B, const char *symbol_ch) {
  time_t end_time, start_time;
  unsigned short len;

  time(&end_time);
  /* The start time needs to be a week before the current time, so minus seven
     days This compensates for weekends and holidays and ensures enough data. */
  start_time = end_time - (86400 * 7);

  len = strlen(symbol_ch) + strlen(YAHOO_URL_START) +
        strlen(YAHOO_URL_MIDDLE_ONE) + strlen(YAHOO_URL_MIDDLE_TWO) +
        strlen(YAHOO_URL_END) + 25;
  char *tmp = realloc(B->url_ch, len);
  B->url_ch = tmp;
  snprintf(B->url_ch, len,
           YAHOO_URL_START "%s" YAHOO_URL_MIDDLE_ONE "%d" YAHOO_URL_MIDDLE_TWO
                           "%d" YAHOO_URL_END,
           symbol_ch, (int)start_time, (int)end_time);
}

static int SetUpCurl(void *data) {
  portfolio_packet *pkg = (portfolio_packet *)data;
  metal *M = MetalClassObject;

  create_bullion_url(M->Gold, "GC=F");
  create_bullion_url(M->Silver, "SI=F");
  if (M->Platinum->ounce_f > 0)
    create_bullion_url(M->Platinum, "PL=F");
  if (M->Palladium->ounce_f > 0)
    create_bullion_url(M->Palladium, "PA=F");

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

static void extract_bullion_data(bullion *B) {
  if (B->CURLDATA.memory == NULL) {
    B->prev_closing_metal_f = 0.0f;
    B->high_metal_f = 0.0f;
    B->low_metal_f = 0.0f;
    B->spot_price_f = 0.0f;
    return;
  }

  /* Convert a String to a File Pointer Stream for Reading */
  FILE *fp =
      fmemopen((void *)B->CURLDATA.memory, strlen(B->CURLDATA.memory) + 1, "r");

  if (fp == NULL) {
    B->prev_closing_metal_f = 0.0f;
    B->high_metal_f = 0.0f;
    B->low_metal_f = 0.0f;
    B->spot_price_f = 0.0f;

    FreeMemtype(&B->CURLDATA);
    return;
  }

  char line[1024];
  char **csv_array;

  /* Yahoo! sometimes updates bullion when the equities markets are closed.
     The while loop iterates to the end of file to get the latest data. */
  double prev_closing = 0.0f, cur_price = 0.0f;
  while (fgets(line, 1024, fp) != NULL) {
    prev_closing = cur_price;
    /* Sometimes the API gives us a null value for certain days.
       using the closing price from the day prior gives us a more accurate
       gain value. */
    if (strstr(line, "null") || strstr(line, "Date"))
      continue;
    Chomp(line);
    csv_array = parse_csv(line);
    cur_price = strtod(csv_array[4] ? csv_array[4] : "0", NULL);
    free_csv_line(csv_array);
  };

  Chomp(line);
  csv_array = parse_csv(line);
  B->prev_closing_metal_f = prev_closing;
  B->high_metal_f = strtod(csv_array[2] ? csv_array[2] : "0", NULL);
  B->low_metal_f = strtod(csv_array[3] ? csv_array[3] : "0", NULL);
  B->spot_price_f = cur_price;

  free_csv_line(csv_array);
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
  bullion *new_class = (bullion *)malloc(sizeof(*new_class));

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

  new_class->url_ch = NULL;

  new_class->spot_price_mrkd_ch = malloc(1);
  new_class->premium_mrkd_ch = malloc(1);
  new_class->port_value_mrkd_ch = malloc(1);
  new_class->ounce_mrkd_ch = malloc(1);

  new_class->high_metal_mrkd_ch = malloc(1);
  new_class->low_metal_mrkd_ch = malloc(1);
  new_class->prev_closing_metal_mrkd_ch = malloc(1);
  new_class->change_ounce_mrkd_ch = malloc(1);
  new_class->change_value_mrkd_ch = malloc(1);
  new_class->change_percent_mrkd_ch = malloc(1);
  new_class->change_percent_raw_ch = malloc(1);

  new_class->YAHOO_hnd = curl_easy_init();
  new_class->CURLDATA.memory = NULL;

  /* Return Our Initialized Class */
  return new_class;
}

metal *ClassInitMetal() {
  /* Allocate Memory For A New Class */
  metal *new_class = (metal *)malloc(sizeof(*new_class));

  /* Initialize Nested Class Objects,
     technically these are structs because
     we removed their function pointers.
   */
  new_class->Gold = class_init_bullion();
  new_class->Silver = class_init_bullion();
  new_class->Platinum = class_init_bullion();
  new_class->Palladium = class_init_bullion();

  /* Initialize Variables */
  new_class->bullion_port_value_f = 0.0f;
  new_class->bullion_port_value_chg_f = 0.0f;
  new_class->bullion_port_value_p_chg_f = 0.0f;
  new_class->gold_silver_ratio_f = 0.0f;

  new_class->bullion_port_value_mrkd_ch = malloc(1);
  new_class->bullion_port_value_chg_mrkd_ch = malloc(1);
  new_class->bullion_port_value_p_chg_mrkd_ch = malloc(1);
  new_class->gold_silver_ratio_ch = malloc(1);

  /* Connect Function Pointers To Function Definitions */
  new_class->ToStrings = ToStrings;
  new_class->Calculate = Calculate;
  new_class->SetUpCurl = SetUpCurl;
  new_class->ExtractData = ExtractData;

  /* Set the file global variable so we can self-reference this class. */
  MetalClassObject = new_class;

  /* Return Our Initialized Class */
  return new_class;
}

/* Class Destruct Functions */
static void class_destruct_bullion(bullion *bullion_class) {
  /* Free Memory */
  if (bullion_class->url_ch)
    free(bullion_class->url_ch);
  if (bullion_class->spot_price_mrkd_ch)
    free(bullion_class->spot_price_mrkd_ch);
  if (bullion_class->premium_mrkd_ch)
    free(bullion_class->premium_mrkd_ch);
  if (bullion_class->port_value_mrkd_ch)
    free(bullion_class->port_value_mrkd_ch);
  if (bullion_class->ounce_mrkd_ch)
    free(bullion_class->ounce_mrkd_ch);

  if (bullion_class->high_metal_mrkd_ch)
    free(bullion_class->high_metal_mrkd_ch);
  if (bullion_class->low_metal_mrkd_ch)
    free(bullion_class->low_metal_mrkd_ch);
  if (bullion_class->prev_closing_metal_mrkd_ch)
    free(bullion_class->prev_closing_metal_mrkd_ch);
  if (bullion_class->change_ounce_mrkd_ch)
    free(bullion_class->change_ounce_mrkd_ch);
  if (bullion_class->change_value_mrkd_ch)
    free(bullion_class->change_value_mrkd_ch);
  if (bullion_class->change_percent_mrkd_ch)
    free(bullion_class->change_percent_mrkd_ch);
  if (bullion_class->change_percent_raw_ch)
    free(bullion_class->change_percent_raw_ch);

  if (bullion_class->YAHOO_hnd)
    curl_easy_cleanup(bullion_class->YAHOO_hnd);

  FreeMemtype(&bullion_class->CURLDATA);

  /* Free Memory From Class Object */
  if (bullion_class) {
    free(bullion_class);
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
    free(metal_handle->bullion_port_value_mrkd_ch);
  if (metal_handle->bullion_port_value_chg_mrkd_ch)
    free(metal_handle->bullion_port_value_chg_mrkd_ch);
  if (metal_handle->bullion_port_value_p_chg_mrkd_ch)
    free(metal_handle->bullion_port_value_p_chg_mrkd_ch);
  if (metal_handle->gold_silver_ratio_ch)
    free(metal_handle->gold_silver_ratio_ch);

  if (metal_handle)
    free(metal_handle);
}