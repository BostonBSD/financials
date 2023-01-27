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

#include "../include/gui.h" /* MainProgBar func */
#include "../include/multicurl_types.h"
#include "../include/mutex.h"

#define MAX_WAIT_MSECS 50

void FreeMemtype(MemType *mem_data) {
  /* Reset the MemType members, doesn't free the
     object pointer. */
  if (mem_data->memory) {
    g_free(mem_data->memory);
    mem_data->memory = NULL;
  }
  mem_data->size = 0;
}

gushort remove_handles(CURLM *mh) {
  CURLMsg *msg = NULL;
  CURL *hnd = NULL;
  CURLcode rc = 0;
  gint msgs_left = 0;
  gushort return_value = 0;

  /* This portion of the code will remove the easy handles from the
   * mulltihandle. */
  while ((msg = curl_multi_info_read(mh, &msgs_left))) {
    if (msg->msg == CURLMSG_DONE) {
      hnd = msg->easy_handle;
      rc = msg->data.result;
      if (rc != CURLE_OK) {
        fprintf(stderr, "CURL code: %d\n", msg->data.result);
        return_value++;
      }
      curl_multi_remove_handle(mh, hnd);
    } else {
      fprintf(stderr, "error: after curl_multi_info_read(), CURLMsg=%d\n",
              msg->msg);
    }
  }
  return return_value;
}

/* This callback function example can be found here:
   https://everything.curl.dev/libcurl/callbacks/write
*/
static gsize write_callback(gchar *ptr, gsize size, gsize nmemb,
                            gpointer userdata)
/* cURL callback function [read in datastream to memory]
   This prototype is provided by cURL, with an argument at the end for our data
   structure.
   This function is repeatedly called by cURL until there is no more data in the
   data stream; *ptr. */
{
  /* The number of bytes in the datastream [there is no NULL char] */
  gsize realsize = size * nmemb;

  MemType *mem = (MemType *)userdata;
  gchar *tmp = g_realloc(mem->memory, mem->size + realsize +
                                          1); /* We add 1 for the NULL char. */

  if (tmp == NULL) {
    printf("Not Enough Memory, realloc returned NULL.\n");
    exit(EXIT_FAILURE);
  }

  mem->memory = tmp;
  memcpy(&(mem->memory[mem->size]), ptr,
         realsize); /* Starting at the last element copy in
                        datastream */
  mem->size +=
      realsize;               /* The actual size is realsize + 1, however
                                 realsize gives us the location of the last element. */
  mem->memory[mem->size] = 0; /* The datastream doesn't include a NULL char,
                                 so we zeroize the last element. */
  /* We overwrite the NULL char {the zeroized element} on the next callback
   * iteration, if any. */

  /* cURL crosschecks the datastream with this return value. */
  return realsize;
}

gpointer SetUpCurlHandle(CURL *hnd, CURLM *mh, gchar *url, MemType *output)
/* Take in an easy handle pointer address, a multihandle pointer address, a URL,
   and a struct pointer address, add easy handle to multi handle. */
{
  /* Make sure output->memory = NULL
     or an allocated pointer addr. */
  if (output->memory)
    FreeMemtype(output); /* Resets the output members, doesn't free the output
                            pointer. */
  output->memory =
      g_malloc(1);  /* Initialize the memory component of the structure. */
  output->size = 0; /* Initialize the size component of the structure. */

  if (hnd) {
    /* Setup the cURL options. */
    curl_easy_setopt(hnd, CURLOPT_BUFFERSIZE, 102400L);
    /* Set the request URL */
    curl_easy_setopt(hnd, CURLOPT_URL, url);
    curl_easy_setopt(hnd, CURLOPT_NOPROGRESS, 1L);
    curl_easy_setopt(hnd, CURLOPT_USERAGENT, "curl/7.80.0");
    /* Some URLs require redirection. */
    curl_easy_setopt(hnd, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(hnd, CURLOPT_MAXREDIRS, 5L);
    curl_easy_setopt(hnd, CURLOPT_HTTP_VERSION, (long)CURL_HTTP_VERSION_2TLS);
    curl_easy_setopt(hnd, CURLOPT_TCP_KEEPALIVE, 0L);
    /* cURL advises to use this option in a multithreaded environment. */
    /* It prevents unix signals during socket operations. */
    curl_easy_setopt(hnd, CURLOPT_NOSIGNAL, 1L);
    /* The callback function to write data to. */
    curl_easy_setopt(hnd, CURLOPT_WRITEFUNCTION, write_callback);
    /* Send the address of the data struct to callback func. */
    curl_easy_setopt(hnd, CURLOPT_WRITEDATA, (gpointer)output);
    /* Connection timeout after 5 seconds. */
    curl_easy_setopt(hnd, CURLOPT_CONNECTTIMEOUT_MS, 5000);
    /* Total data transfer timeout after 10 seconds,
       this is a static use case, don't use in a dynamic use case. */
    curl_easy_setopt(hnd, CURLOPT_TIMEOUT_MS, 10000);
    /* Uncomment the next line for detailed info on
       the connections cURL is communicating over. */
    // curl_easy_setopt(hnd, CURLOPT_VERBOSE, 1L);

    curl_multi_add_handle(mh, hnd);
  } else {
    g_free(output->memory);
    printf("cURL Library Failed, curl_easy_init() returned NULL.\n");
    exit(EXIT_FAILURE);
  }

  /* The output struct was passed by reference no need to return anything. */
  return NULL;
}

gushort PerformMultiCurl(CURLM *mh, gdouble size)
/* Take in a multi handle pointer and the number of easy handles,
   request data from remote server asynchronously. Update the main
   window progress bar during transfer.

   Returns 0 on success, otherwise the number of failed transfers.
*/
{
  curl_global_init(CURL_GLOBAL_ALL);
  if (!mh) {
    printf("cURL Library Failed, curl_multi_init() returned NULL.\n");
    exit(EXIT_FAILURE);
  }

  CURLMcode mc = 0;
  gint still_running = 0;
  gushort return_value = 0;
  gdouble fraction = 0.0f;

  do {
    gint numfds = 0;

    /* This will prevent other threads from interupting the transfer
       [they need to use the same mutex for this to work]. */
    g_mutex_lock(&mutexes[MULTICURL_PROG_MUTEX]);

    mc = curl_multi_perform(mh, &still_running);

    /* Notice MAX_WAIT_MSECS is a very small value,
       now curl_multi_poll can be as fast as curl_multi_wait */
    if (mc == CURLM_OK)
      mc = curl_multi_poll(mh, NULL, 0, MAX_WAIT_MSECS, &numfds);

    /* This will allow other threads to interupt the transfer without
       causing cURL to fail [they need to use the same mutex]. */
    g_mutex_unlock(&mutexes[MULTICURL_PROG_MUTEX]);

    if (mc != CURLM_OK) {
      fprintf(stderr, "curl_multi returned %d\n", (int)mc);
      break;
    }

    /* Update the GUI Progress Bar */
    fraction = 1 - (still_running / size);
    MainProgBar(&fraction);

    /* if there are still transfers, loop! */
  } while (still_running);

  /* Reset the Progress Bar outside this function.
     [ the MainProgBarReset function ]
     Prevents data corruption through pointer passing.
  */

  /* We don't want to remove handles if they're already being
     removed elsewhere. */
  if (g_mutex_trylock(&mutexes[MULTICURL_REM_HAND_MUTEX])) {

    /* Remove the easy handles from the mulltihandle. */
    return_value = remove_handles(mh);

    g_mutex_unlock(&mutexes[MULTICURL_REM_HAND_MUTEX]);
  }

  curl_global_cleanup();
  return return_value;
}

gushort PerformMultiCurl_no_prog(CURLM *mh)
/* Take in a multi handle pointer, request data from remote server
   asynchronously. Doesn't update any gui widgets during transfer.

   Returns 0 on success, otherwise the number of failed transfers.
*/
{
  curl_global_init(CURL_GLOBAL_ALL);
  if (!mh) {
    printf("cURL Library Failed, curl_multi_init() returned NULL.\n");
    exit(EXIT_FAILURE);
  }

  CURLMcode mc = 0;
  gint still_running = 0;
  gushort return_value = 0;

  do {
    gint numfds = 0;

    /* This will prevent other threads from interupting the transfer
       [they need to use the same mutex for this to work]. */
    g_mutex_lock(&mutexes[MULTICURL_NO_PROG_MUTEX]);

    mc = curl_multi_perform(mh, &still_running);

    /* Notice MAX_WAIT_MSECS is a very small value,
       now curl_multi_poll can be as fast as curl_multi_wait */
    if (mc == CURLM_OK)
      mc = curl_multi_poll(mh, NULL, 0, MAX_WAIT_MSECS, &numfds);

    /* This will allow other threads to interupt the transfer without
       causing cURL to fail [they need to use the same mutex]. */
    g_mutex_unlock(&mutexes[MULTICURL_NO_PROG_MUTEX]);

    if (mc != CURLM_OK) {
      fprintf(stderr, "curl_multi returned %d\n", (int)mc);
      break;
    }

    /* if there are still transfers, loop! */
  } while (still_running);

  /* Remove the easy handles from the mulltihandle. */
  return_value = remove_handles(mh);

  curl_global_cleanup();
  return return_value;
}