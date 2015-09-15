%{
/* Prologue */
    #include <stdio.h>
    extern int yylex();
    extern int yyparse();
    extern int yyerror(const char *);
    extern FILE *yyin;
%}

/* Bison declarations */
%union {
    int ival;
    float fval;
    char *sval;
}
%token IDENTIFIER
%token ON UP DOWN SIDE IN OUT BOX WRAP STATELESS DECOUPLED SYNC
%left '|'
%left '.'

%%
/* Grammar rules */
stmt:
    ';'
|   net ';'
|   decl_box
|   decl_wrapper
;

/* box declarartion */
decl_box:
    opt_state BOX IDENTIFIER '(' decl_port opt_decl_port ')' ON IDENTIFIER
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
    IDENTIFIER
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
    FILE *myfile = fopen("a.lang.file", "r");
    // make sure it is valid:
    if (!myfile) {
        fprintf(stdout, "I can't open a.lang.file!\n");
        return -1;
    }
    // set flex to read from it instead of defaulting to STDIN:
    yyin = myfile;

    // parse through the input until there is no more:
    do {
        yyparse();
    } while (!feof(yyin));
}
