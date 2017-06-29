SHELL := /bin/bash
PROJECT = streamix
PARSER = smxc

LOC_INC_DIR = include
LOC_SRC_DIR = src
SIA_LANG = sia2graph
SIA_LANG_DIR = streamix-sia-lang

SOURCES = main.c \
		  $(LOC_SRC_DIR)/* \
		  lex.yy.c \
		  $(PROJECT).tab.c \
		  $(SIA_LANG_DIR)/lex.zz.c \
		  $(SIA_LANG_DIR)/$(SIA_LANG).tab.c \
		  $(SIA_LANG_DIR)/$(LOC_SRC_DIR)/*

INCLUDES = $(LOC_INC_DIR)/* \
		   $(PROJECT).tab.h \
		   $(SIA_LANG_DIR)/include/sia.h \
		   $(SIA_LANG_DIR)/$(SIA_LANG).tab.h \
		   $(SIA_LANG_DIR)/$(LOC_INC_DIR)/*

UTHASH_DIR = $(SIA_LANG_DIR)/uthash
INSTTAB_DIR = insttab
SYMTAB_DIR = symtab
SYMTAB_OBJ = $(SYMTAB_DIR)/symtab.o
SYMTAB_SRC = $(SYMTAB_DIR)/symtab.c $(SYMTAB_DIR)/symtab.h
INSTTAB_OBJ = $(INSTTAB_DIR)/insttab.o
INSTTAB_SRC = $(INSTTAB_DIR)/insttab.c $(INSTTAB_DIR)/insttab.h

OBJECTS = $(SYMTAB_OBJ) $(INSTTAB_OBJ) $(SIA_LANG_OBJ)

INCLUDES_DIR = -I$(UTHASH_DIR)/src \
			   -I/usr/local/include/igraph \
			   -I$(INSTTAB_DIR) \
			   -I$(SYMTAB_DIR) \
			   -I$(LOC_INC_DIR) \
			   -I$(SIA_LANG_DIR)/$(LOC_INC_DIR) \
			   -I.
LINK_DIR = -L/usr/local/lib
LINK_FILE = -ligraph

CFLAGS = -Wall
DEBUG_FLAGS = -g -O0
BFLAGS = -d -Wall
BDEBUG_FLAGS = --verbose
# prevent test command from failing due to debug messages -> turn off debug mode
# to debug use 'make rdebug'
TEST_FLAGS = -DMAKE_TEST

CC = gcc

BUILD_PATH = build
DOT_PATH = dot
DOT_AST_FILE = $(DOT_PATH)/ast_graph
DOT_N_CON_FILE = $(DOT_PATH)/net_connection_graph
DOT_P_CON_FILE = $(DOT_PATH)/port_connection_graph
# generate a dot file representing the ast
DOT_FLAGS = -DDOT_AST
# generate a dot file showing NW connections
DOT_FLAGS += -DDOT_CON
# force a structure in NW connections
DOT_FLAGS += -DDOT_STRUCT
# colorize the structure (no effect if DOT_STRUCT is undefined)
DOT_FLAGS += -DDOT_COLOR
# show labels on edges
DOT_FLAGS += -DDOT_EDGE_LABEL
# position the syncroniyers before the nets
DOT_FLAGS += -DDOT_SYNC_FIRST

TEST_IN = smx
TEST_SUSPENDED = sus
TEST_OUT = res
TEST_VAL = valgrind
MSG_VAL = "ERROR SUMMARY: 0 errors from 0 contexts (suppressed: 0 from 0)"
MEM = 0
TEST_SOL = sol
TEST_GML = gml
TEST_PATH = test
IN_FILE = cpa
INPUT = $(TEST_PATH)/$(IN_FILE).$(TEST_IN)

all: $(PARSER)

# compile with dot stuff (executable generates '.dot' files when run)
# use 'make graph' to generate '.pdf' files from the '.dot' files
dot: CFLAGS += $(DOT_FLAGS)
dot: $(PARSER)

ddot: CFLAGS += $(DOT_FLAGS) $(DEBUG_FLAGS)
ddot: $(PARSER)

# compile with dot stuff and debug flags
debug: CFLAGS += $(DEBUG_FLAGS)
debug: BFLAGS += $(BDEBUG_FLAGS)
debug: $(PARSER)

# compile with dot stuff and debug flags, run, and generate graphs
rdebug: CFLAGS += $(DEBUG_FLAGS) $(DOT_FLAGS)
rdebug: BFLAGS += $(BDEBUG_FLAGS)
rdebug: clean $(PARSER) run graph

# run tests on all files in the test path
test: CFLAGS += $(DEBUG_FLAGS) $(DOT_FLAGS) $(TEST_FLAGS)
test: BFLAGS += $(BDEBUG_FLAGS)
test: clean $(PARSER) run_test_all

# run tests on one file in the input
test1: CFLAGS += $(DEBUG_FLAGS) $(DOT_FLAGS) $(TEST_FLAGS)
test1: BFLAGS += $(BDEBUG_FLAGS)
test1: clean $(PARSER) run_test

# compile project
$(PARSER): $(SOURCES) $(INCLUDES) $(INSTTAB_OBJ)
	$(CC) $(CFLAGS) $(SOURCES) $(OBJECTS) $(INCLUDES_DIR) $(LINK_DIR) $(LINK_FILE) -o $(PARSER)

# compile lexer (flex)
lex.yy.c: $(PROJECT).lex $(PROJECT).tab.h
	flex $(PROJECT).lex

# compile parser (bison)
$(PROJECT).tab.c $(PROJECT).tab.h: $(PROJECT).y
	bison $(BFLAGS) $(PROJECT).y

# compile insttab libarary
$(INSTTAB_OBJ): $(INSTTAB_SRC) $(SYMTAB_OBJ)
	$(CC) $(CFLAGS) $< $(INCLUDES_DIR) -c -o $@

# compile symtab libarary
$(SYMTAB_OBJ): $(SYMTAB_SRC)
	$(CC) $(CFLAGS) $< $(INCLUDES_DIR) -c -o $@

# compile lexer (flex)
$(SIA_LANG_DIR)/lex.zz.c: $(SIA_LANG_DIR)/$(SIA_LANG).lex $(SIA_LANG_DIR)/$(SIA_LANG).tab.h
	flex -o $(SIA_LANG_DIR)/lex.zz.c $(SIA_LANG_DIR)/$(SIA_LANG).lex

# compile parser (bison)
$(SIA_LANG_DIR)/$(SIA_LANG).tab.c $(SIA_LANG_DIR)/$(SIA_LANG).tab.h: $(SIA_LANG_DIR)/$(SIA_LANG).y
	bison $(BFLAGS) -o $(SIA_LANG_DIR)/$(SIA_LANG).tab.c $(SIA_LANG_DIR)/$(SIA_LANG).y

# $(OBJECTS): $(OBJ_SRC)
# 	$(CC) $(CFLAGS) $< $(INCLUDES_DIR) -c -o $@

$(DOT_AST_FILE).pdf: $(DOT_AST_FILE).dot
	@# generate ast graph pdf file
	@dot $(DOT_AST_FILE).dot -Tpdf > $(DOT_AST_FILE).pdf

# generate connection graph pdf file
# 1. generate multiple temporary pdf files of the networks
# 2. merge temporary pdf files to one pdf
# 3. remove temporary pdf files
$(DOT_N_CON_FILE).pdf: $(DOT_N_CON_FILE).dot
	# generate connection graph pdf file
	@dot -Tpdf $(DOT_N_CON_FILE).dot | csplit --quiet --elide-empty-files --prefix=dot/tmpfile - "/%%EOF/+1" "{*}"
	@gs -dBATCH -dNOPAUSE -q -sDEVICE=pdfwrite -sOutputFile=$(DOT_N_CON_FILE).pdf dot/tmpfile*
	@rm -f dot/tmpfile*

$(DOT_P_CON_FILE).pdf: $(DOT_P_CON_FILE).dot
	# generate port connection graph pdf file
	@dot -Tpdf -Gnewrank $(DOT_P_CON_FILE).dot | csplit --quiet --elide-empty-files --prefix=dot/tmpfile - "/%%EOF/+1" "{*}"
	@gs -dBATCH -dNOPAUSE -q -sDEVICE=pdfwrite -sOutputFile=$(DOT_P_CON_FILE).pdf dot/tmpfile*
	@rm -f dot/tmpfile*

.PHONY: clean graph run run_test run_test_all install

clean:
	rm -f $(PROJECT).tab.c
	rm -f $(PROJECT).tab.h
	rm -f $(PROJECT).output
	rm -f $(PARSER)
	rm -f lex.yy.c
	rm -rf $(DOT_PATH)
	rm -rf $(BUILD_PATH)
	rm -f $(OBJECTS)
	rm -f $(SIA_LANG_DIR)/$(SIA_LANG).tab.c
	rm -f $(SIA_LANG_DIR)/$(SIA_LANG).tab.h
	rm -f $(SIA_LANG_DIR)/$(SIA_LANG).output
	rm -f $(SIA_LANG_DIR)/$(SIA_LANG)
	rm -f $(SIA_LANG_DIR)/lex.zz.c

install:
	mkdir -p /usr/local/bin
	cp -a $(PARSER) /usr/local/bin/.


# generate '.pdf' files from the '.dot' files
# graph: $(DOT_AST_FILE).pdf $(DOT_N_CON_FILE).pdf $(DOT_P_CON_FILE).pdf
graph: $(DOT_AST_FILE).pdf $(DOT_P_CON_FILE).pdf
# graph: $(DOT_AST_FILE).pdf

# used for debugging to save time
run:
	./$(PARSER) $(INPUT)

run_test:
	@touch $(INPUT:.$(TEST_IN)=.$(TEST_SOL))
	@./$(PARSER) -f gml -S -p ./ -o $(PROJECT).gml $(INPUT) > $(INPUT:.$(TEST_IN)=.$(TEST_OUT))
	@echo "testing $(INPUT)"
	@diff <(sed -r 's/-?[0-9]+\)/*)/g' $(INPUT:.$(TEST_IN)=.$(TEST_OUT))) $(INPUT:.$(TEST_IN)=.$(TEST_SOL))
	$(MAKE) -s graph
	mv $(DOT_AST_FILE).pdf $(INPUT:.$(TEST_IN)=_ast.pdf)
	mv $(DOT_P_CON_FILE).pdf $(INPUT:.$(TEST_IN)=_gp.pdf)
	mv $(PROJECT).gml $(INPUT:.$(TEST_IN)=_$(TEST_GML).$(TEST_OUT))
	@diff $(INPUT:.$(TEST_IN)=_$(TEST_GML).$(TEST_OUT)) $(INPUT:.$(TEST_IN)=_$(TEST_GML).$(TEST_SOL))
ifeq ($(MEM),1)
	@valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes -v ./$(PARSER) $(INPUT) &> $(INPUT:.$(TEST_IN)=.$(TEST_VAL))
	@diff <(tail -n1 $(INPUT:.$(TEST_IN)=.$(TEST_VAL)) | sed 's/==[0-9]*== //g') <(echo $(MSG_VAL))
endif
	# cp $(DOT_N_CON_FILE).pdf $(INPUT:.$(TEST_IN)=_gn.pdf)

run_test_all:
	@printf "\n Testlog " | tee $(TEST_PATH)/test.log
	@date | tee -a $(TEST_PATH)/test.log
	@printf "======================================\n" | tee -a $(TEST_PATH)/test.log
	@for file in $(TEST_PATH)/*.$(TEST_IN); do \
		echo $$file | tee -a $(TEST_PATH)/test.log; \
		./$(PARSER) -f gml -S -p ./ -o $(PROJECT).gml $$file > $${file%.*}.$(TEST_OUT); \
		diff <(sed -r 's/-?[0-9]+\)/*)/g' $${file%.*}.$(TEST_OUT)) $${file%.*}.$(TEST_SOL) | tee -a $(TEST_PATH)/test.log; \
		$(MAKE) -s graph; \
		mv $(DOT_AST_FILE).pdf $${file%.*}_ast.pdf; \
		mv $(DOT_P_CON_FILE).pdf $${file%.*}_gp.pdf; \
		mv $(PROJECT).gml $${file%.*}_$(TEST_GML).$(TEST_OUT); \
		diff $${file%.*}_$(TEST_GML).$(TEST_OUT) $${file%.*}_$(TEST_GML).$(TEST_SOL) | tee -a $(TEST_PATH)/test.log; \
		if [ $(MEM) -eq 1 ]; then \
			valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes -v ./$(PARSER) $$file &> $${file%.*}.$(TEST_VAL); \
			diff <(tail -n1 $${file%.*}.$(TEST_VAL) | sed 's/==[0-9]*== //g') <(echo $(MSG_VAL)); \
		fi;\
		# cp $(DOT_N_CON_FILE).pdf $${file%.*}_gn.pdf; \
	done
	@printf "\nSuspended Tests:\n" | tee -a $(TEST_PATH)/test.log
	@for file in $(TEST_PATH)/*.$(TEST_SUSPENDED); do \
		echo $$file | tee -a $(TEST_PATH)/test.log;\
	done

