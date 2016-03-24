PROJECT = test
OUT = test_symtab

SOURCES = symtab.c
INCLUDES = symtab.h
INCLUDES_DIR = -I../uthash/include -I. -I..

CFLAGS = -Wall -DTESTING
DEBUG_FLAGS = -g -O0

CC = gcc

all: $(PROJECT)
# compile with dot stuff and debug flags
debug: CFLAGS += $(DEBUG_FLAGS)
debug: $(PROJECT)

# compile project
$(PROJECT): $(PROJECT).c $(SOURCES) $(INCLUDES)
	$(CC) $(CFLAGS) $(SOURCES) $(PROJECT).c $(INCLUDES_DIR) -o $(OUT)

.PHONY: clean

clean:
	rm -f $(PROJECT)
