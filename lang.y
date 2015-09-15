%{
/* Prologue */
    #include <stdio.h>
    extern int yylex();
    extern int yyerror(const char *);
    extern FILE *yyin;
%}

/* Bison declarations */
%union {
    int ival;
    float fval;
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
    net         { fprintf(stdout, "net\n"); }
|   decl_box
|   decl_wrapper
;

/* box declarartion */
decl_box:
    opt_state BOX IDENTIFIER '(' decl_port opt_decl_port ')' ON IDENTIFIER {
        fprintf(stdout, "box %s\n", $3);
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
    IDENTIFIER ':' port_mode port_class                 { fprintf(stdout, " port %s\n", $1); }
|   IDENTIFIER ':' SYNC '(' syncport opt_syncport ')'   { fprintf(stdout, " syncport\n"); }
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
    IDENTIFIER  { fprintf(stdout, "  ID\n"); }
|   net '.' net { fprintf(stdout, " Serial combination\n"); }
|   net '|' net { fprintf(stdout, " Parallel combination\n"); }
|   '(' net ')' { fprintf(stdout, " Subexpression\n"); }
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
    if (argc != 2) {
        fprintf(stdout, "Missing argument!\n");
        return -1;
    }
    FILE *myfile = fopen(argv[1], "r");
    // make sure it is valid:
    if (!myfile) {
        fprintf(stdout, "Cannot open file '%s'!\n", argv[1]);
        return -1;
    }
    // set flex to read from it instead of defaulting to STDIN:
    yyin = myfile;

    // parse through the input until there is no more:
    do {
        yyparse();
    } while (!feof(yyin));
}
