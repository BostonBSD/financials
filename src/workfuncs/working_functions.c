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

#include <time.h>

#include <stdlib.h>
#include <string.h>

#include "../include/class_types.h"
#include "../include/macros.h"
#include "../include/multicurl.h"

double CalcGain(double cur_price, double prev_price) {
  return (100 * ((cur_price - prev_price) / prev_price));
}

void CalcSumRsi(double current_gain, double *avg_gain, double *avg_loss) {
  if (current_gain >= 0) {
    *avg_gain += current_gain;
  } else {
    *avg_loss += (-1 * current_gain);
  }
}

void CalcRunAvgRsi(double current_gain, double *avg_gain, double *avg_loss) {
  if (current_gain >= 0) {
    *avg_gain = ((*avg_gain * 13) + current_gain) / 14;
    *avg_loss = ((*avg_loss * 13) + 0) / 14;
  } else {
    *avg_gain = ((*avg_gain * 13) + 0) / 14;
    *avg_loss = ((*avg_loss * 13) + (-1 * current_gain)) / 14;
  }
}

double CalcRsi(double avg_gain, double avg_loss) {
  double rs = avg_gain / avg_loss;
  return (100 - (100 / (1 + rs)));
}

static void history_url_period(time_t *currenttime, time_t *starttime) {

  /* Number of Seconds in a Year Plus Three Weeks */
  unsigned int period = 31557600 + (604800 * 3);

  time(currenttime);
  *starttime = *currenttime - (time_t)period;
}

static char *history_get_url(const char *symbol) {
  time_t end, start;
  unsigned short len;

  history_url_period(&end, &start);

  len = strlen(symbol) + strlen(YAHOO_URL_START) +
        strlen(YAHOO_URL_MIDDLE_ONE) + strlen(YAHOO_URL_MIDDLE_TWO) +
        strlen(YAHOO_URL_END) + 25;
  char *url = malloc(len);
  snprintf(url, len,
           YAHOO_URL_START "%s" YAHOO_URL_MIDDLE_ONE "%d" YAHOO_URL_MIDDLE_TWO
                           "%d" YAHOO_URL_END,
           symbol, (int)start, (int)end);

  return url;
}

MemType *FetchHistoryData(const char *symbol, portfolio_packet *pkg) {
  meta *D = pkg->GetMetaClass();
  char *MyUrl = NULL;
  MemType *MyOutputStruct = (MemType *)malloc(sizeof(*MyOutputStruct));
  MyOutputStruct->memory = NULL;
  MyOutputStruct->size = 0;

  MyUrl = history_get_url(symbol);

  SetUpCurlHandle(D->history_hnd, D->multicurl_history_hnd, MyUrl, MyOutputStruct);
  if (PerformMultiCurl_no_prog(D->multicurl_history_hnd) != 0) {
    free(MyUrl);
    FreeMemtype(MyOutputStruct);
    free(MyOutputStruct);
    return NULL;
  }

  free(MyUrl);
  return MyOutputStruct;
}