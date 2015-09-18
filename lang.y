/* 
 * The grammar of the coordination language xyz
 *
 * @file    lang.y
 * @author  Simon Maurer
 *
 * */

%{
/* Prologue */
    #include <stdio.h>
    #include <string.h>
    #include "symtab.h"
    #include "graph.h"
    #include "defines.h"
    #include "ast.h"
    extern int yylex();
    extern int yyparse();
    extern FILE *yyin;
    FILE* congraph;
    ast_node* ast_ptr;
    void yyerror (char const *);
    void install ( char*, int, int );
    void install_port ( char*, int, int, int );
    symrec *context_check ( char*, int );
    void context_check_port ( char*, int );
    int num_errors = 0;
    char* src_file_name;
    char error_msg[255];
%}

/* Bison declarations */
%define parse.error verbose
%define parse.lac full
%union {
    int ival;
    char *sval;
    struct ast_node* nval;
};
%token ON STATELESS DECOUPLED SYNC
%token <ival> UP DOWN SIDE IN OUT BOX WRAP
%token <sval> IDENTIFIER
%type <ival> port_mode port_class
%type <nval> net
%left '|'
%left '.'

%%
/* Grammar rules */
stmts:
    %empty
|   stmts stmt
;

stmt:
    net {
        ast_ptr = $1;
    }
|   decl_box
|   decl_wrapper
;

/* box declarartion */
decl_box:
    opt_state BOX IDENTIFIER '(' decl_port opt_decl_port ')' ON IDENTIFIER {
        install($3, $2, @3.last_line);
        addNode(congraph, $3, $3, SHAPE_BOX);
    }
;

opt_state:
    %empty
|   STATELESS
;

opt_decl_port:
    %empty
|   ',' decl_port opt_decl_port
;

decl_port:
    IDENTIFIER ':' port_mode port_class {
        install_port($1, $3, $4, @1.last_line);
    }
|   SYNC '{' syncport opt_syncport '}'
;

opt_syncport:
    %empty
|   ',' syncport opt_syncport
;

syncport:
    IDENTIFIER ':' port_class opt_decoupled {
        install_port($1, VAL_IN, $3, @1.last_line);
    }
;

opt_decoupled:
    %empty
|   DECOUPLED
;

port_mode:
    IN   {$$ = $1;}
|   OUT  {$$ = $1;}
;

port_class:
    UP   {$$ = $1;}
|   DOWN {$$ = $1;}
|   SIDE {$$ = $1;}
;

/* net declaration */
net:
    IDENTIFIER  { 
        context_check($1, @1.last_line);
        $$ = ast_add_id($1);
        /* printf("id: %s\n", $$->name); */
    }
|   net '.' net {
        $$ = ast_add_op($1, $3, OP_SERIAL);
    }
|   net '|' net {
        $$ = ast_add_op($1, $3, OP_PARALLEL);
    }
|   '(' net ')' {
        $$ = $2;
    }
;

/* wrapper declaration */
decl_wrapper:
    WRAP IDENTIFIER '{' wportlist '}' ON net {
        install($2, $1, @2.last_line);
    }
;

wportlist:
    %empty
|   decl_wport opt_decl_wport
;

opt_decl_wport:
    %empty
|   ',' decl_wport opt_decl_wport
;

decl_wport:
    IDENTIFIER '(' IDENTIFIER ')' ':' port_mode port_class {
        install_port($1, $6, $7, @1.last_line);
        context_check_port($3, @3.last_line);
    }
;

%%
/* Epilogue */
int main(int argc, char **argv) {
    // open a file handle to a particular file:
    if (argc != 2) {
        printf("Missing argument!\n");
        return -1;
    }
    src_file_name = argv[1];
    FILE *myfile = fopen(src_file_name, "r");
    // make sure it is valid:
    if (!myfile) {
        printf("Cannot open file '%s'!\n", src_file_name);
        return -1;
    }
    // set flex to read from it instead of defaulting to STDIN    yyin = myfile;
    yyin = myfile;

    congraph = fopen("congraph.dot", "w");
    initGraph(congraph);

    // parse through the input until there is no more:
    do {
        yyparse();
    } while (!feof(yyin));

    finishGraph(congraph);
    draw_ast(ast_ptr);

    if (num_errors > 0) printf(" Error count: %d\n", num_errors);
}

/*
 * Add a new net identifier to the symbol table. If it is already there,
 * produce an error.
 *
 * @param: char* name:  name of the net
 * @param: int type:    type of the net
 * @param: int line:    line number of the symbol in the source file
 * */
void install ( char *name, int type, int line ) {
    /* printf("install %s, %d\n", name, type); */
    if (getsym_net (name) == 0)
        putsym_net (name, type);
    else {
        sprintf(error_msg, "%d: error: %s is already defined", line, name);
        yyerror(error_msg);
    }
}

/*
 * Add a new port identifier to the symbol table. If it is already there,
 * produce an error.
 *
 * @param: char* name:  name of the port
 * @param: int pclass:  calss of the port (up, down, side)
 * @param: int mode:    mode of the port (in, out)
 * @param: int line:    line number of the symbol in the source file
 * */
void install_port ( char *name, int pclass, int mode, int line ) {
    /* printf("install_port %s, %d, %d\n", name, pclass, mode); */
    if (getsym_port (name, pclass, mode) == 0)
        putsym_port (name, pclass, mode);
    else {
        sprintf(error_msg, "%d: error: %s is already defined in this scope", line, name);
        yyerror(error_msg);
    }
}

/*
 * Check whether a net identifier is properly declared. If not,
 * produce an error.
 *
 * @param: char* name:  name of the net
 * @param: int line:    line number of the symbol in the source file
 * */
symrec *context_check ( char *name, int line ) {
    /* printf("context_check %s\n", name); */
    symrec *s;
    s = getsym_net (name);
    if (s == 0) {
        sprintf(error_msg, "%d: error: %s is an undeclared identifier", line, name);
        yyerror(error_msg);
    }
    return s;
}

/*
 * check whether a port identifier is properly declared. If not,
 * produce an error.
 *
 * @param: char* name:  name of the port
 * @param: int line:    line number of the symbol in the source file
 * */
void context_check_port ( char *name, int line ) {
    /* printf("context_check_port %s\n", name); */
    if (getsym_port_all (name) == 0) {
        sprintf(error_msg, "%d: error: %s is an undeclared identifier", line, name);
        yyerror(error_msg);
    }
}

/*
 * error function of bison
 *
 * @param: char* s:  error string
 * */
void yyerror(const char *s) {
    num_errors++;
    printf("%s: %s\n", src_file_name, s);
}
