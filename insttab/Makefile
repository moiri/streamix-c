PROJECT = test
OUT = test

SOURCES = insttab.c
INCLUDES = insttab.h

SYMTAB_DIR = ../symtab
SYMTAB_OBJ = $(SYMTAB_DIR)/symtab.o
SYMTAB_SRC = $(SYMTAB_DIR)/symtab.c $(SYMTAB_DIR)/symtab.h

INCLUDES_DIR = -I../uthash/include -I. -I.. -I../symtab

CFLAGS = -Wall
DEBUG_FLAGS = -g -O0

CC = gcc

all: $(PROJECT)
# compile with dot stuff and debug flags
debug: CFLAGS += $(DEBUG_FLAGS)
debug: $(PROJECT)

# compile project
$(PROJECT): $(PROJECT).c $(SOURCES) $(INCLUDES) $(SYMTAB_OBJ)
	$(CC) $(CFLAGS) $(SOURCES) $(PROJECT).c $(INCLUDES_DIR) -o $(OUT)

# compile symtab libarary
$(SYMTAB_OBJ): $(SYMTAB_SRC)
	$(CC) $(CFLAGS) $< $(INCLUDES_DIR) -c -o $@

.PHONY: clean

clean:
	rm -f $(PROJECT)
	rm -f $(SYMTAB_OBJ)
