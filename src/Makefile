CC=cc

MAIN_SRC=financials.c config/config.c csv/csv.c
LIB_INT_SRC=multicurl/multicurl.c json/json.c sqlite/sqlite.c resources/resources.c
GUI_SRC=gui/gui.c gui/gui_other_wins.c gui/gui_security.c gui/gui_main.c gui/gui_history.c gui/gui_callbacks.c gui/gui_threads.c
CLASS_SRC=class/class_bullion.c class/class_equity.c class/class_meta.c class/class_packet.c
WK_SRC=workfuncs/pango_formatting.c workfuncs/sn_map.c workfuncs/string_formatting.c workfuncs/time_funcs.c workfuncs/working_functions.c
SRC=$(MAIN_SRC) $(LIB_INT_SRC) $(GUI_SRC) $(CLASS_SRC) $(WK_SRC)

CFLAGS=`pkg-config --cflags gtk+-3.0 json-glib-1.0 glib-2.0 libcurl sqlite3`
LIBS=`pkg-config --libs gtk+-3.0 json-glib-1.0 glib-2.0 libcurl sqlite3`
WARN=-Wall 

MKDIR=mkdir -p
OUT_DIR=../build
# Compiling everything into one executable yields a more compact binary
# than splitting into obj files [20% smaller], the recompilation time is 
# arbitrary on the dev system.
financials:
	${MKDIR} ${OUT_DIR}
	$(CC) -Ofast -s -lm -o $(OUT_DIR)/$@ $(SRC) $(CFLAGS) $(LIBS) $(WARN)