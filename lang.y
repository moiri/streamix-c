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
    extern FILE *yyin;
    void yyerror (char const *);
    void install ( char*, int, int );
    void install_port ( char*, int, int, int );
    void context_check ( char*, int );
    void context_check_port ( char*, int );
    int num_errors = 0;
    char* src_file_name;
    char error_msg[255];
    int port_mode, port_class;
%}

/* Bison declarations */
%define parse.error verbose
%define parse.lac full
%union {
    int ival;
    char *sval;
}
%token ON STATELESS DECOUPLED SYNC
%token <ival> UP DOWN SIDE IN OUT BOX WRAP
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
        install($3, $2, @3.last_line);
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
        install_port($1, port_mode, port_class, @1.last_line);
    }
|   SYNC '{' syncport opt_syncport '}'
;

opt_syncport:
    %empty
|   ',' syncport opt_syncport
;

syncport:
    IDENTIFIER ':' port_class opt_decoupled {
        install_port($1, 0, port_class, @1.last_line);
    }
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
    IDENTIFIER  { 
        context_check($1, @1.last_line);
    }
|   net '.' net {
    /* fprintf (stderr, "%d-%d: test\n", @1.last_line, @3.last_line); */
    }
|   net '|' net
|   '(' net ')'
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
        install_port($1, port_mode, port_class, @1.last_line);
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
    // set flex to read from it instead of defaulting to STDIN:
    yyin = myfile;

    // parse through the input until there is no more:
    do {
        yyparse();
    } while (!feof(yyin));

    if (num_errors > 0) printf(" Error count: %d\n", num_errors);
}

/*
 * add net identifier to the symbol table
 *
 * @param: char* name:  name of the net
 * @param: int type:    type of the net
 * @param: int line:    line number of the symbol in the source file
 * */
void install ( char *name, int type, int line ) {
    if (getsym_net (name) == 0)
        putsym_net (name, type);
    else {
        sprintf(error_msg, "%d: error: %s is already defined", line, name);
        yyerror(error_msg);
    }
}

/*
 * add port identifier to the symbol table
 *
 * @param: char* name:  name of the port
 * @param: int type:    calss of the port (up, down, side)
 * @param: int mode:    mode of the port (in, out)
 * @param: int line:    line number of the symbol in the source file
 * */
void install_port ( char *name, int pclass, int mode, int line ) {
    /* printf("install port %s, %d, %d\n", name, pclass, mode); */
    if (getsym_port (name, pclass, mode) == 0)
        putsym_port (name, pclass, mode);
    else {
        sprintf(error_msg, "%d: error: %s is already defined in this scope", line, name);
        yyerror(error_msg);
    }
}

/*
 * check whether a net identifier is decleared
 *
 * @param: char* name:  name of the net
 * @param: int line:    line number of the symbol in the source file
 * */
void context_check ( char *name, int line ) {
    if (getsym_net (name) == 0) {
        sprintf(error_msg, "%d: error: %s is an undeclared identifier", line, name);
        yyerror(error_msg);
    }
}

/*
 * check whether a port identifier is decleared
 *
 * @param: char* name:  name of the port
 * @param: int line:    line number of the symbol in the source file
 * */
void context_check_port ( char *name, int line ) {
    if (getsym_port (name, -1, -1) == 0) {
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
