%{
/* Prologue */
    #include <stdio.h>
    #include "symtab.h"
    #include <string.h>
    extern int yylex();
    extern int yyparse();
    extern int yyerror(const char *);
    extern FILE *yyin;
    extern int num_errors;
    char* src_file_name;
    void install ( char *sym_name ) {
        symrec *s;
        s = getsym (sym_name);
        if (s == 0)
            s = putsym (sym_name);
        else
            yyerror(strcat(sym_name, " is already defined"));
    }
    void context_check( char *sym_name ) {
        if ( getsym( sym_name ) == 0 )
            yyerror(strcat(sym_name, " is an undeclared identifier"));
    }
    
%}

/* Bison declarations */
%define parse.error verbose
%define parse.lac full
%union {
    int ival;
    char *sval;
}
%token ON UP DOWN SIDE IN OUT BOX WRAP STATELESS DECOUPLED SYNC
%token <sval> IDENTIFIER
%left '|'
%left '.'

%%
/* Grammar rules */
stmts:
    %empty
|   stmts stmt
;

stmt:
    net
|   decl_box
|   decl_wrapper
;

/* box declarartion */
decl_box:
    opt_state BOX IDENTIFIER '(' decl_port opt_decl_port ')' ON IDENTIFIER {
        install($3);
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
    IDENTIFIER ':' port_mode port_class
|   IDENTIFIER ':' SYNC '(' syncport opt_syncport ')'
;

opt_syncport:
    %empty
|   ',' syncport opt_syncport
;

syncport:
    IDENTIFIER ':' port_class opt_decoupled
;

opt_decoupled:
    %empty
|   DECOUPLED
;

port_mode:
    IN
|   OUT
;

port_class:
    UP
|   DOWN
|   SIDE
;

/* net declaration */
net:
    IDENTIFIER  { context_check($1); }
|   net '.' net
|   net '|' net
|   '(' net ')'
;

/* wrapper declaration */
decl_wrapper:
    WRAP IDENTIFIER '{'
        UP '{' wportlist '}' ','
        DOWN '{' wportlist '}' ','
        SIDE '{' wportlist '}'
    '}' ON net
    { install($2); }
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
    IDENTIFIER '(' IDENTIFIER ')' ':' port_mode
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
    // set flex to read from it instead of defaulting to STDIN:
    yyin = myfile;

    // parse through the input until there is no more:
    do {
        yyparse();
    } while (!feof(yyin));

    if (num_errors > 0) printf(" Error count: %d\n", num_errors);
}
