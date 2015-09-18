PROJECT = lang
PARSER = parser
DOT_PATH = dot
PLUGIN_C = symtab.c \
		   graph.c \
		   ast.c

PLUGIN_H = symtab.h \
		   graph.h \
		   defines.h \
		   ast.h

PLUGIN = $(PLUGIN_C) $(PLUGIN_H)

TEST_PATH = test
TEST_FILE = simple.test

$(PARSER): lex.yy.c $(PROJECT).tab.c $(PROJECT).tab.h $(PLUGIN)
	g++ $(PROJECT).tab.c lex.yy.c $(PLUGIN_C) -lfl -o $(PARSER)

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
	./$(PARSER) $(TEST_PATH)/$(TEST_FILE)

.PHONY: dot

dot:
	dot $(DOT_PATH)/congraph.dot -Tpng > $(DOT_PATH)/congraph.png
	dot $(DOT_PATH)/ast.dot -Tpng > $(DOT_PATH)/ast.png
