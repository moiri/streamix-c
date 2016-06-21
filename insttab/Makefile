PROJECT = test
OUT = test

SOURCES = insttab.c
INCLUDES = insttab.h

SYMTAB_DIR = ../symtab
SYMTAB_OBJ = $(SYMTAB_DIR)/symtab.o
SYMTAB_SRC = $(SYMTAB_DIR)/symtab.c $(SYMTAB_DIR)/symtab.h

INCLUDES_DIR = -I../uthash/src \
			   -I/usr/local/include/igraph \
			   -I../include \
			   -I../symtab \
			   -I.. \
			   -I.

LINK_DIR = -L/usr/local/lib
LINK_FILE = -ligraph

CFLAGS = -Wall
DEBUG_FLAGS = -g -O0

CC = gcc

all: $(PROJECT)
# compile with dot stuff and debug flags
debug: CFLAGS += $(DEBUG_FLAGS)
debug: $(PROJECT)

# compile project
$(PROJECT): $(PROJECT).c $(SOURCES) $(INCLUDES) $(SYMTAB_OBJ)
	$(CC) $(CFLAGS) $(SOURCES) $(PROJECT).c $(INCLUDES_DIR) $(LINK_DIR) $(LINK_FILE) -o $(OUT)

# compile symtab libarary
$(SYMTAB_OBJ): $(SYMTAB_SRC)
	$(CC) $(CFLAGS) $< $(INCLUDES_DIR) -c -o $@

.PHONY: clean

clean:
	rm -f $(PROJECT)
	rm -f $(SYMTAB_OBJ)
