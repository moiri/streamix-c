PROJECT = lang
PARSER = parser

SOURCES = symtab.c \
		   ast.c \
		   graph.c

INCLUDES = symtab.h \
		   ast.h \
		   graph.h \
		   defines.h

INCLUDES_DIR = -Iuthash/include
CFLAGS = -Wall -lfl

DOT_PATH = dot
DOT_AST_FILE = $(DOT_PATH)/ast_graph
DOT_N_CON_FILE = $(DOT_PATH)/net_connection_graph
DOT_P_CON_FILE = $(DOT_PATH)/port_connection_graph
DOT_FLAGS = -DDOT_AST -DDOT_CON

TEST_PATH = test
TEST_FILE = cpa.test
TEST = $(TEST_PATH)/$(TEST_FILE)

all: $(PARSER)

# compile with dot stuff (executable generates '.dot' files when run)
# use 'make graph' to generate '.pdf' files from the '.dot' files
dot: CFLAGS += $(DOT_FLAGS)
dot: $(PARSER)

# compile with dot stuff and debug flags, run the executable after
# compilation and generate graph pdfs
debug: CFLAGS += -g -O0 $(DOT_FLAGS)
debug: clean $(PARSER) run graph

# compile project
$(PARSER): lex.yy.c $(PROJECT).tab.c $(PROJECT).tab.h $(SOURCES) $(INCLUDES)
	gcc $(SOURCES) $(INCLUDES_DIR) $(PROJECT).tab.c lex.yy.c -o $(PARSER) $(CFLAGS)

# compile lexer (flex)
lex.yy.c: $(PROJECT).lex $(PROJECT).tab.h
	flex $(PROJECT).lex

# compile parser (bison)
$(PROJECT).tab.c $(PROJECT).tab.h: $(PROJECT).y
	bison -d -Wall $(PROJECT).y

# generate ast graph pdf file
$(DOT_AST_FILE).pdf: $(DOT_AST_FILE).dot
	dot $(DOT_AST_FILE).dot -Tpdf > $(DOT_AST_FILE).pdf

# generate connection graph pdf file
# 1. generate multiple temporary pdf files of the networks
# 2. merge temporary pdf files to one pdf
# 3. remove temporary pdf files
$(DOT_N_CON_FILE).pdf: $(DOT_N_CON_FILE).dot
	dot -Tpdf $(DOT_N_CON_FILE).dot | csplit --quiet --elide-empty-files --prefix=dot/tmpfile - "/%%EOF/+1" "{*}"
	gs -dBATCH -dNOPAUSE -q -sDEVICE=pdfwrite -sOutputFile=$(DOT_N_CON_FILE).pdf dot/tmpfile*
	rm -f dot/tmpfile*

$(DOT_P_CON_FILE).pdf: $(DOT_P_CON_FILE).dot
	dot -Tpdf $(DOT_P_CON_FILE).dot | csplit --quiet --elide-empty-files --prefix=dot/tmpfile - "/%%EOF/+1" "{*}"
	gs -dBATCH -dNOPAUSE -q -sDEVICE=pdfwrite -sOutputFile=$(DOT_P_CON_FILE).pdf dot/tmpfile*
	rm -f dot/tmpfile*

.PHONY: clean graph run

clean:
	rm -f $(PROJECT).tab.c
	rm -f $(PROJECT).tab.h
	rm -f $(PARSER)
	rm -f lex.yy.c
	rm -f $(DOT_PATH)/*

# generate '.pdf' files from the '.dot' files
graph: $(DOT_AST_FILE).pdf $(DOT_N_CON_FILE).pdf $(DOT_P_CON_FILE).pdf

# used for debugging to save time
run:
	./$(PARSER) $(TEST)
