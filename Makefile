PROJECT = lang
PARSER = parser

$(PARSER): lex.yy.c $(PROJECT).tab.c $(PROJECT).tab.h symtab.c symtab.h
	g++ $(PROJECT).tab.c lex.yy.c symtab.c -lfl -o $(PARSER)

lex.yy.c: $(PROJECT).lex $(PROJECT).tab.h
	flex $(PROJECT).lex

$(PROJECT).tab.c $(PROJECT).tab.h: $(PROJECT).y
	bison -d -Wall $(PROJECT).y

clean:
	rm -f $(PROJECT).tab.c
	rm -f $(PROJECT).tab.h
	rm -f $(PARSER)
	rm -f lex.yy.c
