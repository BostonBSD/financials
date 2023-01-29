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

#include "../include/class_types.h" /* equity_folder, metal, meta, window_data */
#include "../include/macros.h"
#include "../include/sqlite.h"

static void config_dir_processing(const gchar *usr_config_dir)
/* Check if the "~/.config" and "~/.config/financials" directories exist. */
{
  /* Append the /financials directory to the end of user config directory path. */
  gchar *path = g_strconcat(usr_config_dir, CONFIG_DIR, NULL);

  /* Make dir if it doesn't exist, along with parent dirs. */
  g_mkdir_with_parents(path, 0764);
  g_free(path);
}

void ReadConfig(portfolio_packet *pkg) {
  meta *D = pkg->GetMetaClass();

  /* Make sure the config directory exists. */
  config_dir_processing(D->config_dir_ch);

  /* Process the sqlite db file and populate initial varables. */
  SqliteProcessing(pkg);
}