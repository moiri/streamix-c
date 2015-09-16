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
    #include "symtab.h"
    #include <string.h>
    extern int yylex();
    extern int yyparse();
    extern int yyerror(const char *);
    extern FILE *yyin;
    extern int num_errors;
    char* src_file_name;
    int port_mode, port_class;
    void install ( char *name, int type ) {
        if (getsym_net (name) == 0)
            putsym_net (name, type);
        else {
            const char *error = " is already defined";
            char msg[strlen(name) + strlen(error)];
            strcpy(msg, name);
            strcat(msg, error);
            yyerror(msg);
        }
    }
    void install_port ( char *name, int pclass, int mode ) {
        printf("install port %s, %d, %d\n", name, pclass, mode);
        if (getsym_port (name, pclass, mode) == 0)
            putsym_port (name, pclass, mode);
        else {
            const char *error = " is already defined in this scope";
            char msg[strlen(name) + strlen(error)];
            strcpy(msg, name);
            strcat(msg, error);
            yyerror(msg);
        }
    }
    void context_check ( char *name ) {
        if (getsym_net (name) == 0) {
            const char *error = " is an undeclared identifier";
            char msg[strlen(name) + strlen(error)];
            strcpy(msg, name);
            strcat(msg, error);
            yyerror(msg);
        }
    }
    void context_check_port ( char *name ) {
        if (getsym_port (name, -1, -1) == 0) {
            const char *error = " is an undeclared identifier";
            char msg[strlen(name) + strlen(error)];
            strcpy(msg, name);
            strcat(msg, error);
            yyerror(msg);
        }
    }
%}

/* Bison declarations */
%define parse.error verbose
%define parse.lac full
%union {
    int ival;
    char *sval;
}
%token ON BOX WRAP STATELESS DECOUPLED SYNC
%token <ival> UP DOWN SIDE IN OUT
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
        install($3, BOX);
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
    IDENTIFIER ':' port_mode port_class {install_port($1, port_mode, port_class);}
|   SYNC '{' syncport opt_syncport '}'
;

opt_syncport:
    %empty
|   ',' syncport opt_syncport
;

syncport:
    IDENTIFIER ':' port_class opt_decoupled {install_port($1, 0, port_class);}
;

opt_decoupled:
    %empty
|   DECOUPLED
;

port_mode:
    IN   {port_mode = $1;}
|   OUT  {port_mode = $1;}
;

port_class:
    UP   {port_class = $1;}
|   DOWN {port_class = $1;}
|   SIDE {port_class = $1;}
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
    WRAP IDENTIFIER '{' wportlist '}' ON net {
        install($2, WRAP);
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
        install_port($1, port_mode, port_class);
        context_check_port($3);
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
    // set flex to read from it instead of defaulting to STDIN:
    yyin = myfile;

    // parse through the input until there is no more:
    do {
        yyparse();
    } while (!feof(yyin));

    if (num_errors > 0) printf(" Error count: %d\n", num_errors);
}
