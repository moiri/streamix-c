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
    #include "ast.h"
    #include "defines.h"
    extern int yylex();
    extern int yyparse();
    extern FILE *yyin;
    void yyerror ( const char* );
    void install ( char*, int, int );
    void install_port ( char*, int, int, int );
    void context_check ( char*, int );
    void context_check_port ( char*, int );
    ast_node* ast;
    int num_errors = 0;
    char* src_file_name;
    char error_msg[255];
    FILE* con_graph;
%}

/* Bison declarations */
%define parse.error verbose
%define parse.lac full
%locations
%union {
    int ival;
    char *sval;
    struct ast_node* nval;
    struct ast_list* lval;
};
%token ON STATELESS DECOUPLED SYNC CONNECT
%token <ival> UP DOWN SIDE IN OUT BOX NET
%token <sval> IDENTIFIER
%type <ival> port_mode port_class
%type <nval> net nets stmt decl_box decl_net decl_connect
%type <lval> stmts
%left '|'
%left '.'
%start start

%%
/* Grammar rules */

start:
    stmts { ast = ast_add_stmts($1); }
;

stmts:
    %empty {$$ = (ast_list*)0;}
|   stmt stmts {
        $$ = ast_add_stmt($1, $2);
        /* if ($1 == 0) printf("0"); */
        /* else printf("stmt"); */
        /* if ($2 == 0) printf(" 0\n"); */
        /* else printf(" stmts\n"); */
    }
;

stmt:
    nets {$$ = $1;}
|   decl_connect {$$ = $1;}
;

nets:
    net {
        draw_connection_graph(con_graph, $1);
        $$ = ast_add_net($1);
        /* draw_ast_graph($$); */
    }
|   decl_box {
        $$ = $1;
    }
|   decl_net {
        $$ = $1;
    }
;

decl_connect:
    CONNECT IDENTIFIER '{' connect_list '}' {
        $$ = ast_add_connect(ast_add_id($2));
    }
;

connect_list:
    IDENTIFIER opt_connect_id
|   '*'
;

opt_connect_id:
    %empty
|   ',' IDENTIFIER
;

/* box declarartion */
decl_box:
    opt_state BOX IDENTIFIER '(' decl_bport opt_decl_bport ')' ON IDENTIFIER {
        /* install($3, $2, @3.last_line); */
        $$ = ast_add_box(ast_add_id($3));
    }
;

opt_state:
    %empty
|   STATELESS
;

opt_decl_bport:
    %empty
|   ',' decl_bport opt_decl_bport
;

decl_bport:
    opt_port_class port_mode IDENTIFIER
|   SYNC '{' syncport opt_syncport '}'
;

opt_port_class:
    %empty
|   opt_port_class port_class
;

/* decl_bport: */
/*     IDENTIFIER ':' port_mode port_class { */
/*         install_port($1, $3, $4, @1.last_line); */
/*     } */
/* |   SYNC '{' syncport opt_syncport '}' */
/* ; */

opt_syncport:
    %empty
|   ',' syncport opt_syncport
;

syncport:
    opt_decoupled opt_port_class IN IDENTIFIER {
        /* install_port($1, VAL_IN, $3, @1.last_line); */
    }
;

/* syncport: */
/*     IDENTIFIER ':' port_class opt_decoupled { */
/*         install_port($1, VAL_IN, $3, @1.last_line); */
/*     } */
/* ; */

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
        /* context_check($1, @1.last_line); */
        $$ = ast_add_id($1);
        /* printf("id: %s\n", $$->name); */
    }
|   net '.' net {
        $$ = ast_add_op($1, $3, AST_SERIAL);
    }
|   net '|' net {
        $$ = ast_add_op($1, $3, AST_PARALLEL);
    }
|   '(' net ')' {
        $$ = $2;
    }
;

/* wrapper declaration */
decl_net:
    NET IDENTIFIER '{' nportlist '}' '{' stmts '}' {
        /* install($2, $1, @2.last_line); */
        /* draw_connection_graph(con_graph, $7); */
        /* $$ = ast_add_wrap(ast_add_id($2), ast_add_net($7)); */
        $$ = ast_add_wrap(ast_add_id($2), ast_add_stmts($7));
    }
;

nportlist:
    %empty
|   decl_nport opt_decl_nport
;

opt_decl_nport:
    %empty
|   ',' decl_nport opt_decl_nport
;

decl_nport:
    opt_port_class port_mode IDENTIFIER '(' IDENTIFIER ')' {
        /* install_port($1, $6, $7, @1.last_line); */
        /* context_check_port($3, @3.last_line); */
    }
;

/* decl_nport: */
/*     IDENTIFIER '(' IDENTIFIER ')' ':' port_mode port_class { */
/*         install_port($1, $6, $7, @1.last_line); */
/*         context_check_port($3, @3.last_line); */
/*     } */
/* ; */

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

    con_graph = fopen(CON_DOT_PATH, "w");
    // parse through the input until there is no more:
    do {
        yyparse();
    } while (!feof(yyin));

    fclose(con_graph);
    if (num_errors > 0) printf(" Error count: %d\n", num_errors);
    else draw_ast_graph(ast);
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
        sprintf(error_msg, ERROR_DUPLICATE_ID, line, name);
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
        sprintf(error_msg, ERROR_DUPLICATE_PORT, line, name);
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
void context_check ( char *name, int line ) {
    /* printf("context_check %s\n", name); */
    if (getsym_net (name) == 0) {
        sprintf(error_msg, ERROR_UNDEFINED_ID, line, name);
        yyerror(error_msg);
    }
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
        sprintf(error_msg, ERROR_UNDEFINED_ID, line, name);
        yyerror(error_msg);
    }
}

/*
 * error function of bison
 *
 * @param: char* s:  error string
 * */
void yyerror(const char* s ) {
    num_errors++;
    printf("%s: %s\n", src_file_name, s);
}
