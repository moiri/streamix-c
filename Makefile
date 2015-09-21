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
TEST_FILE = $(TEST_PATH)/less_simple.test

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
	dot $(DOT_AST_FILE).dot -Tpng > $(DOT_AST_FILE).png
	dot $(DOT_CON_FILE).dot -Tpng > $(DOT_CON_FILE).png
