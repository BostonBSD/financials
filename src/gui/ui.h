/*
Copyright (c) 2022 BostonBSD. All rights reserved.

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
#ifndef UI_HEADER_H
#define UI_HEADER_H

/* This header file is for any ancillary strings or
   other data that the UI requires prior to the Gtk
   main loop. */

/* A string of the BSD 3 Clause License displayed in the About Window. */
#ifndef LICENSE
#define LICENSE                                                                \
  "Copyright (c) 2022 BostonBSD. All rights reserved.\n\nRedistribution and "  \
  "use in source and binary forms, with or without\nmodification, are "        \
  "permitted provided that the following conditions "                          \
  "are\nmet:\n\n\n\t(1) Redistributions of source code must retain the above " \
  "copyright\n\tnotice, this list of conditions and the following "            \
  "disclaimer.\n\n\t(2) Redistributions in binary form must reproduce the "    \
  "above copyright\n\tnotice, this list of conditions and the following "      \
  "disclaimer\n\tin the documentation and/or other materials provided with "   \
  "the\n\tdistribution.\n\n\t(3) The name of the author may not be used "      \
  "to\n\tendorse or promote products derived from this software "              \
  "without\n\tspecific prior written permission.\n\nTHIS SOFTWARE IS "         \
  "PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR\nIMPLIED WARRANTIES, "  \
  "INCLUDING, BUT NOT LIMITED TO, THE IMPLIED\nWARRANTIES OF MERCHANTABILITY " \
  "AND FITNESS FOR A PARTICULAR\nPURPOSE ARE DISCLAIMED.IN NO EVENT SHALL "    \
  "THE AUTHOR BE LIABLE FOR\nANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, "      \
  "EXEMPLARY, OR\nCONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED "          \
  "TO,\nPROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, "      \
  "OR\nPROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY\nTHEORY "  \
  "OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT\n(INCLUDING "  \
  "NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE\nUSE OF THIS "       \
  "SOFTWARE EVEN IF ADVISED OF THE POSSIBILITY OF SUCH\nDAMAGE."               \
  "\n\n----------------------------------------------------\n"                 \
  "CSV parser license:\n"                                                      \
  "----------------------------------------------------\n"                     \
  "Copyright 2015--2021 Samuel Alexander\n\n"                                  \
  "Permission is hereby granted, free of charge, to any person obtaining a "   \
  "copy of\nthis software and associated documentation files (the "            \
  "\"Software\"), to deal in\nthe Software without restriction, including "    \
  "without limitation "                                                        \
  "the rights to\nuse, copy, modify, merge, publish, distribute, sublicense, " \
  "and/or "                                                                    \
  "sell copies of\nthe Software, and to permit persons to whom the Software "  \
  "is "                                                                        \
  "furnished to do so,\nsubject to the following conditions:\n\nThe above "    \
  "copyright "                                                                 \
  "notice and this permission notice shall be included in all\ncopies or "     \
  "substantial portions of the Software.\n\nTHE SOFTWARE "                     \
  "IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR\nIMPLIED, " \
  "INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF "                            \
  "MERCHANTABILITY, FITNESS\nFOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. "   \
  "IN NO EVENT SHALL "                                                         \
  "THE AUTHORS OR\nCOPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR "     \
  "OTHER "                                                                     \
  "LIABILITY, WHETHER\nIN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING "  \
  "FROM, OUT "                                                                 \
  "OF OR IN\nCONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN "    \
  "THE SOFTWARE."                                                              \
  "\n\n----------------------------------------------------\nApplication "     \
  "Icon\n----------------------------------------------------\nTrends icon "   \
  "designed by Freepik from Flaticon\n\n"
#endif /* LICENSE */

#endif /* UI_HEADER_H */