PROJECT = lang
PARSER = parser
PLUGIN_C = symtab.c \
		   graph.c \
		   ast.c

PLUGIN_H = symtab.h \
		   graph.h \
		   defines.h \
		   ast.h

PLUGIN = $(PLUGIN_C) $(PLUGIN_H)

$(PARSER): lex.yy.c $(PROJECT).tab.c $(PROJECT).tab.h $(PLUGIN)
	g++ $(PROJECT).tab.c lex.yy.c $(PLUGIN_C) -lfl -o $(PARSER)

lex.yy.c: $(PROJECT).lex $(PROJECT).tab.h
	flex $(PROJECT).lex

$(PROJECT).tab.c $(PROJECT).tab.h: $(PROJECT).y
	bison -d -Wall $(PROJECT).y

clean:
	rm -f $(PROJECT).tab.c
	rm -f $(PROJECT).tab.h
	rm -f $(PARSER)
	rm -f lex.yy.c

dot:
	dot congraph.dot -Tpng > congraph.png
	dot ast.dot -Tpng > ast.png
