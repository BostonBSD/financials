CC=cc
SRC=financials.c config/config.c multicurl/multicurl.c json/json.c gui/gui.c gui/gui_callbacks.c gui/gui_threads.c class/class.c workfuncs/string_formatting.c workfuncs/working_functions.c sqlite/sqlite.c resources/resources.c csv/csv.c
CFLAGS=`pkg-config --cflags gtk+-3.0 json-glib-1.0 glib-2.0 libcurl sqlite3 x11`
LIBS=`pkg-config --libs gtk+-3.0 json-glib-1.0 glib-2.0 libcurl sqlite3 x11`
WARN=-Wall -Wextra

financials: $(SRC)
	$(CC) -Ofast -s -lm -o ../build/$@ $(SRC) $(CFLAGS) $(LIBS) $(WARN)
