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

void GetYahooUrl(char **url_ch, const char *symbol_ch, unsigned int period) {
  time_t end_time, start_time;
  unsigned short len;

  time(&end_time);
  start_time = end_time - (time_t)period;

  const char *fmt = YAHOO_URL_START
      "%s" YAHOO_URL_MIDDLE_ONE "%d" YAHOO_URL_MIDDLE_TWO "%d" YAHOO_URL_END;

  len = snprintf(NULL, 0, fmt, symbol_ch, (int)start_time, (int)end_time) + 1;
  char *tmp = realloc(url_ch[0], len);

  if (tmp == NULL) {
    printf("Not Enough Memory, realloc returned NULL.\n");
    exit(EXIT_FAILURE);
  }

  url_ch[0] = tmp;
  snprintf(url_ch[0], len, fmt, symbol_ch, (int)start_time, (int)end_time);
}

MemType *FetchHistoryData(const char *symbol_ch, portfolio_packet *pkg) {
  meta *D = pkg->GetMetaClass();
  char *MyUrl_ch = NULL;
  MemType *MyOutputStruct = (MemType *)malloc(sizeof(*MyOutputStruct));
  MyOutputStruct->memory = NULL;
  MyOutputStruct->size = 0;

  /* Number of Seconds in a Year Plus Three Weeks */
  unsigned int period = 31557600 + (604800 * 3);
  GetYahooUrl(&MyUrl_ch, symbol_ch, period);

  SetUpCurlHandle(D->history_hnd, D->multicurl_history_hnd, MyUrl_ch,
                  MyOutputStruct);
  if (PerformMultiCurl_no_prog(D->multicurl_history_hnd) != 0) {
    free(MyUrl_ch);
    FreeMemtype(MyOutputStruct);
    free(MyOutputStruct);
    return NULL;
  }

  free(MyUrl_ch);
  return MyOutputStruct;
}