PROJECT = test
OUT = test

SOURCES = symtab.c \
		  ../src/vnet.c
INCLUDES = symtab.h
INCLUDES_DIR = -I../uthash/src -I. -I.. -I../include \
			   -I../insttab \
			   -I/usr/local/include/igraph

LINK_DIR = -L/usr/local/lib
LINK_FILE = -ligraph

CFLAGS = -Wall -DTESTING
DEBUG_FLAGS = -g -O0

CC = gcc

all: $(PROJECT)
# compile with dot stuff and debug flags
debug: CFLAGS += $(DEBUG_FLAGS)
debug: $(PROJECT)

# compile project
$(PROJECT): $(PROJECT).c $(SOURCES) $(INCLUDES)
	$(CC) $(CFLAGS) $(SOURCES) $(PROJECT).c $(INCLUDES_DIR) $(LINK_DIR) $(LINK_FILE) -o $(OUT)

.PHONY: clean

clean:
	rm -f $(PROJECT)
