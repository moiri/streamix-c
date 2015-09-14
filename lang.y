%{
/* Prologue */
    /* int yylex (void); */
    void yyerror (char const *);
%}

/* Bison declarations */
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
void yyerror(char *s) {
    fprintf(stdout, "%s\n", s);
}
