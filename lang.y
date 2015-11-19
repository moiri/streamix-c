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
    #include "ast.h"
    #include "defines.h"
    #include "graph.h"
    #include "utarray.h"
    extern int yylex();
    extern int yyparse();
    extern FILE *yyin;
    void yyerror ( const char* );
    ast_node* ast;
    int num_errors = 0;
    char* src_file_name;
    /* FILE* con_graph; */
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
%token ON SYNC CONNECT
%token <ival> BOX NET IN OUT UP DOWN SIDE DECOUPLED STATELESS
%token <sval> IDENTIFIER
%type <sval> scope_id
%type <nval> net nets stmt decl_box decl_net decl_connect
%type <nval> decl_bport syncport decl_nport port_mode port_class
%type <nval> opt_state opt_decoupled opt_renaming
%type <lval> stmts connect_list opt_connect_id
%type <lval> opt_decl_bport opt_syncport opt_decl_nport opt_port_class
%left '|'
%left '.'
%start start

%%
/* Grammar rules */

start:
    stmts {
        ast = ast_add_list( $1, AST_STMTS );
    }
;

stmts:
    %empty {
        $$ = ( ast_list* )0;
    }
|   stmt stmts {
        $$ = ast_add_list_elem( $1, $2 );
    }
;

stmt:
    nets {$$ = $1;}
|   decl_connect {$$ = $1;}
;

nets:
    net {
        /* draw_connection_graph( con_graph, $1 ); */
        $$ = ast_add_node( $1, AST_NET );
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
        $$ = ast_add_connect(
            ast_add_id( $2, @2.last_line, ID_CPORT ),
            ast_add_list( $4, AST_CONNECTS )
        );
    }
;

connect_list:
    IDENTIFIER opt_connect_id {
        $$ = ast_add_list_elem(
            ast_add_id( $1, @1.last_line, ID_CNET ),
            $2
        );
    }
|   '*' {
        $$ = ast_add_list_elem(
            ast_add_star(),
            ( ast_list* )0
        );
    }
;

opt_connect_id:
    %empty {
        $$ = ( ast_list* )0;
    }
|   ',' IDENTIFIER opt_connect_id {
        $$ = ast_add_list_elem(
            ast_add_id( $2, @2.last_line, ID_CNET ),
            $3
        );
    }
;

/* box declarartion */
decl_box:
    opt_state BOX scope_id '(' decl_bport opt_decl_bport ')' ON IDENTIFIER {
        $$ = ast_add_box(
            ast_add_id( $3, @3.last_line, ID_BOX ),
            ast_add_list(
                ast_add_list_elem( $5, $6 ),
                AST_PORTS
            ),
            ast_add_node( $1, AST_STATE )
        );
    }
;

scope_id:
    IDENTIFIER { $$ = $1; }
;

opt_state:
    %empty { $$ = ( ast_node* )0; }
|   STATELESS { $$ = ast_add_attr( $1, ATTR_STATE ); }
;

opt_decl_bport:
    %empty { $$ = ( ast_list* )0; }
|   ',' decl_bport opt_decl_bport {
        $$ = ast_add_list_elem( $2, $3 );
    }
;

decl_bport:
    opt_port_class port_mode IDENTIFIER {
        $$ = ast_add_port(
            ast_add_id( $3, @3.last_line, ID_PORT ),
            ( ast_node* )0, // no internal id
            ast_add_list( $1, AST_COLLECT ),
            ast_add_node( $2, AST_MODE ),
            ( ast_node* )0, // no coupling
            PORT_BOX
        );
    }
|   SYNC '{' syncport opt_syncport '}' {
        $$ = ast_add_list(
            ast_add_list_elem( $3, $4 ),
            AST_SYNC
        );
    }
;

opt_port_class:
    %empty { $$ = ( ast_list* )0; }
|   port_class opt_port_class {
        $$ = ast_add_list_elem( $1, $2 );
    }
;

opt_syncport:
    %empty { $$ = ( ast_list* )0; }
|   ',' syncport opt_syncport {
        $$ = ast_add_list_elem( $2, $3 );
    }
;

syncport:
    opt_decoupled opt_port_class IN IDENTIFIER {
        $$ = ast_add_port(
            ast_add_id( $4, @4.last_line, ID_PORT ),
            (ast_node*)0, // no internal id
            ast_add_list( $2, AST_COLLECT ),
            ast_add_node( ast_add_attr( $3, ATTR_MODE ), AST_MODE ),
            ast_add_node( $1, AST_COUPLING ),
            PORT_SYNC
        );
    }
;

opt_decoupled:
    %empty { $$ = ( ast_node* )0; }
|   DECOUPLED { $$ = ast_add_attr( $1, ATTR_COUPLING ); }
;

port_mode:
    IN   { $$ = ast_add_attr( $1, ATTR_MODE );}
|   OUT  { $$ = ast_add_attr( $1, ATTR_MODE );}
;

port_class:
    UP   { $$ = ast_add_attr( $1, ATTR_COLLECT ); }
|   DOWN { $$ = ast_add_attr( $1, ATTR_COLLECT ); }
|   SIDE { $$ = ast_add_attr( $1, ATTR_COLLECT ); }
;

/* net declaration */
net:
    IDENTIFIER  { 
        $$ = ast_add_id( $1, @1.last_line, ID_NET );
    }
|   net '.' net {
        $$ = ast_add_op( $1, $3, AST_SERIAL );
    }
|   net '|' net {
        $$ = ast_add_op( $1, $3, AST_PARALLEL );
    }
|   '(' net ')' {
        $$ = $2;
    }
;

/* wrapper declaration */
decl_net:
    NET scope_id '{' decl_nport opt_decl_nport '}' '{' stmts '}' {
        /* install($2, *( int* )utarray_back( scope_stack ), $1, NULL, */
        /*         @2.last_line); */
        $$ = ast_add_wrap(
            ast_add_id( $2, @2.last_line, ID_WRAP ),
            ast_add_list(
                ast_add_list_elem( $4, $5 ),
                AST_PORTS
            ),
            ast_add_list( $8, AST_STMTS )
        );
    }
;

opt_decl_nport:
    %empty { $$ = ( ast_list* )0; }
|   ',' decl_nport opt_decl_nport {
        $$ = ast_add_list_elem( $2, $3 );
    }
;

decl_nport:
    opt_port_class port_mode IDENTIFIER opt_renaming {
        $$ = ast_add_port(
            ast_add_id( $3, @3.last_line, ID_PORT ),
            $4,
            ast_add_list( $1, AST_COLLECT ),
            ast_add_node( $2, AST_MODE ),
            ( ast_node* )0, // no coupling
            PORT_NET
        );
    }
;

opt_renaming:
    %empty { $$ = NULL; }
|   '(' IDENTIFIER ')' {
        $$ = ast_add_node(
            ast_add_id( $2, @2.last_line, ID_IPORT ),
            AST_INT_ID
        );
    }
;

%%
/* Epilogue */
int main( int argc, char **argv ) {
    // open a file handle to a particular file:
    if( argc != 2 ) {
        printf( "Missing argument!\n" );
        return -1;
    }
    src_file_name = argv[ 1 ];
    FILE *myfile = fopen( src_file_name, "r" );
    // make sure it is valid:
    if( !myfile ) {
        printf( "Cannot open file '%s'!\n", src_file_name );
        return -1;
    }
    // set flex to read from it instead of defaulting to STDIN    yyin = myfile;
    yyin = myfile;

    /* con_graph = fopen(CON_DOT_PATH, "w"); */
    // parse through the input until there is no more:
    do {
        yyparse();
    } while( !feof( yyin ) );

    context_check( ast );

    /* fclose(con_graph); */
    if( num_errors > 0 ) printf( " Error count: %d\n", num_errors );
    else draw_ast_graph( ast );

    return 0;
}

/*
 * error function of bison
 *
 * @param: char* s:  error string
 * */
void yyerror( const char* s ) {
    num_errors++;
    printf( "%s: %s\n", src_file_name, s );
}
