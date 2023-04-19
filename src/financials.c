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
#include "financials.h"

/* PascalCase for public functions, snake_case for private functions.
   The handler functions are PascalCase with snake_case suffix for greater
   clarity, these are public functions.

   In the class files, PascalCase names are class methods, snake_case names
   are private [static to that file]. */

GMutex mutexes[MUTEX_NUMBER]; /* A Glib Mutex Array */

static void mutex_init()
/* Initialize Mutexes */
{
  gushort g = 0;
  while (g < MUTEX_NUMBER)
    g_mutex_init(&mutexes[g++]);
}

static void mutex_destruct()
/* Free Mutex Resources */
{
  gushort g = 0;
  while (g < MUTEX_NUMBER)
    g_mutex_clear(&mutexes[g++]);
}

static void class_package_init()
/* Initialize the class package */
{
  ClassInitPortfolioPacket();
}

static void class_package_destruct()
/* Free Memory */
{
  ClassDestructPortfolioPacket(packet);
}

static void simple_arg_parse(gchar **argv, portfolio_packet *pkg) {
  if (!g_strcmp0("-h", argv[1]) || !g_strcmp0("--help", argv[1])) {

    g_print("Help Message\n"
            "---------------\n"
            "-h --help\tPrint this help message.\n"
            "-v --version\tDisplay the version.\n"
            "-r --reset\tRemove %s database files.\n\n",
            argv[0]);

  } else if (!g_strcmp0("-v", argv[1]) || !g_strcmp0("--version", argv[1])) {

    g_print(VERSION_STRING "\n");

  } else if (!g_strcmp0("-r", argv[1]) || !g_strcmp0("--reset", argv[1])) {

    gint ret = RemoveConfigFiles(pkg->GetMetaClass());
    if (ret) {
      g_print(
          "File removal error, is the '%s' directory empty or non-existent?\n",
          pkg->meta_class->config_dir_ch);

      /* Free Class Instances. */
      class_package_destruct();
      exit(EXIT_FAILURE);
    } else {
      g_print("Config file directory '%s' removed successfully.\n",
              pkg->meta_class->config_dir_ch);
    }
  }
  /* Free Class Instances. */
  class_package_destruct();
  exit(EXIT_SUCCESS);
}

gint main(gint argc, gchar *argv[]) {
  /* Initialize some of our class instances */
  /* This needs to be initialized before ReadConfig ()
     and simple_arg_parse () */
  class_package_init();

  /* Parse commandline args, if any. */
  if (argc > 1)
    simple_arg_parse(argv, packet);

  /* Initialize gtk */
  gtk_init(&argc, &argv);

  /* Initialize Mutexes */
  /* This needs to be initialized before ReadConfig */
  mutex_init();

  /* Read config file and populate associated variables */
  ReadConfig(packet);

  /* Set up GUI widgets and display the GUI */
  GuiStart(packet);

  /* Free Class Instances. */
  class_package_destruct();

  /* Free Mutex Resources */
  mutex_destruct();

  return 0;
}