/*
Copyright (c) 2022-2024 BostonBSD. All rights reserved.

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

#ifndef MUTEX_HEADER_H
#define MUTEX_HEADER_H

#include <glib.h>

/* System mutex enum */
enum {
  CLASS_MEMBER_MUTEX,
  CLASS_CALCULATE_MUTEX,
  CLASS_EXTRACT_DATA_MUTEX,
  CLASS_TOSTRINGS_MUTEX,
  CLOCKS_COND_MUTEX,
  CLOCKS_HANDLER_MUTEX,
  FETCH_DATA_MUTEX,
  FETCH_DATA_COND_MUTEX,
  FETCH_DATA_HANDLER_MUTEX,
  HISTORY_FETCH_MUTEX,
  MULTICURL_PROG_MUTEX,
  MULTICURL_NO_PROG_MUTEX,
  MULTICURL_REM_HAND_MUTEX,
  SYMBOL_NAME_MAP_SQLITE_MUTEX,
  SQLITE_MUTEX,
  MUTEX_NUMBER
};

/* Globals */
extern GMutex mutexes[MUTEX_NUMBER]; /* A Glib Mutex Array */

#endif /* MUTEX_HEADER_H */