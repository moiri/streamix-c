PROJECT = lang
PARSER = parser

PLUGIN_C = symtab.c \
		   ast.c \
		   graph.c \

PLUGIN_H = symtab.h \
		   ast.h \
		   graph.h \
		   defines.h \

PLUGIN = $(PLUGIN_C) $(PLUGIN_H)

DOT_PATH = dot
DOT_AST_FILE = $(DOT_PATH)/ast_graph
DOT_CON_FILE = $(DOT_PATH)/connection_graph

TEST_PATH = test
TEST_FILE = $(TEST_PATH)/cpa.test

$(PARSER): lex.yy.c $(PROJECT).tab.c $(PROJECT).tab.h $(PLUGIN)
	gcc $(PROJECT).tab.c lex.yy.c $(PLUGIN_C) -lfl -o $(PARSER)

lex.yy.c: $(PROJECT).lex $(PROJECT).tab.h
	flex $(PROJECT).lex

$(PROJECT).tab.c $(PROJECT).tab.h: $(PROJECT).y
	bison -d -Wall $(PROJECT).y

debug: clean $(PARSER) run dot

clean:
	rm -f $(PROJECT).tab.c
	rm -f $(PROJECT).tab.h
	rm -f $(PARSER)
	rm -f lex.yy.c
	rm -f $(DOT_PATH)/*

run:
	./$(PARSER) $(TEST_FILE)

.PHONY: dot

dot:
	# generate ast pdf file
	# sort -V $(DOT_AST_FILE).dot -o $(DOT_AST_FILE).dot
	dot $(DOT_AST_FILE).dot -Tpdf > $(DOT_AST_FILE).pdf
	# generate multiple temporary pdf files of the networks
	dot -Tpdf $(DOT_CON_FILE).dot | csplit --quiet --elide-empty-files --prefix=dot/tmpfile - "/%%EOF/+1" "{*}"
	# merge temporary pdf files to one pdf
	gs -dBATCH -dNOPAUSE -q -sDEVICE=pdfwrite -sOutputFile=$(DOT_CON_FILE).pdf dot/tmpfile*
	# remove temporary pdf files
	rm -f dot/tmpfile*
