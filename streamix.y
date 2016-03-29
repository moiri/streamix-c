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
    #include "context.h"
    #include "ast.h"
    #include "defines.h"
    /* #include "cgraph.h" */
#ifdef DOT_AST
    #include "dot.h"
#endif // DOT_AST
    extern int yylex();
    extern int yyparse();
    extern FILE *yyin;
    extern int yylineno;
    extern char* yytext;
    void yyerror ( const char* );
    ast_node* ast;
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
/* keywods */
%token SYNC CONNECT LINK
%token <ival> BOX WRAPPER NET IN OUT UP DOWN SIDE DECOUPLED STATELESS STATIC

/* optional and variable keyword tokens */
%type <ival> kw_opt_state
%type <ival> kw_opt_static
%type <ival> kw_opt_decoupled
%type <ival> kw_port_class
%type <ival> kw_opt_port_class
%type <ival> kw_port_mode

/* idenitifiers */
%token <sval> IDENTIFIER

/* declarations */
%type <nval> stmt
%type <nval> stmt_wrap
%type <nval> net
%type <nval> def_net
%type <nval> opt_def_net
%type <nval> decl_net
%type <nval> proto_net
%type <nval> decl_net_port
%type <nval> decl_link
%type <nval> decl_link_id
%type <nval> def_box
%type <nval> decl_box
%type <nval> decl_box_port
%type <nval> decl_sync_port
%type <nval> decl_wrap
%type <nval> decl_wrap_port
%type <nval> decl_int_port

/* lists */
%type <lval> stmts
%type <lval> stmts_wrap
%type <lval> link_list
%type <lval> opt_link_list
%type <lval> box_port_list
%type <lval> opt_box_port_list
%type <lval> sync_port_list
%type <lval> opt_sync_port_list
%type <lval> net_port_list
%type <lval> opt_net_port_list
%type <lval> wrap_port_list
%type <lval> opt_wrap_port_list
%type <lval> int_port_list
%type <lval> opt_int_ports
%type <lval> opt_int_port_list


%left '|'
%left '.'
%start program

%%
/* Grammar rules */
/* start of the grammer */
program:
    stmts { ast = ast_add_node( ast_add_list( $1, AST_STMTS ), AST_PROGRAM ); }
;

/* a list of statements describing the program */
stmts:
    %empty { $$ = ( ast_list* )0; }
|   stmt stmts { $$ = ast_add_list_elem( $1, $2 ); }
;

stmt:
    def_net { $$ = $1; }
|   def_box { $$ = $1; }
|   decl_wrap { $$ = $1; }
;

/* net definition */
def_net:
    opt_def_net decl_net { $$ = ast_add_def( $1, $2, AST_NET_DEF ); }
;

opt_def_net:
    %empty { $$ = ( ast_node* )0; }
|   IDENTIFIER '=' { $$ = ast_add_symbol( $1, @1.last_line, ID_NET ); }
;

/* net declaration */
decl_net:
    net { $$ = ast_add_net( $1 ); }
|   proto_net { $$ = $1; }
;

net:
    IDENTIFIER  { $$ = ast_add_symbol( $1, @1.last_line, ID_NET ); }
|   net '.' net { $$ = ast_add_op( $1, $3, AST_SERIAL ); }
|   net '|' net { $$ = ast_add_op( $1, $3, AST_PARALLEL ); }
|   '(' net ')' { $$ = $2; }
;

/* net prototyping */
proto_net:
    NET IDENTIFIER '{' net_port_list '}' {
        $$ = ast_add_net_prot(
            ast_add_symbol( $2, @2.last_line, ID_NET ),
            ast_add_list( $4, AST_PORTS )
        );
    }
;

net_port_list:
    decl_net_port opt_net_port_list {
        $$ = ast_add_list_elem( $1, $2 );
    }
;

opt_net_port_list:
    %empty { $$ = ( ast_list* )0; }
|   ',' decl_net_port opt_net_port_list {
        $$ = ast_add_list_elem( $2, $3 );
    }
;

decl_net_port:
    kw_port_class kw_port_mode IDENTIFIER opt_int_ports {
        $$ = ast_add_port(
            ast_add_symbol( $3, @3.last_line, ID_PORT ),
            ast_add_list( $4, AST_INT_PORTS ),
            ast_add_attr( $1, ATTR_PORT_CLASS ),
            ast_add_attr( $2, ATTR_PORT_MODE ),
            ( ast_node* )0, // no coupling
            PORT_NET
        );
    }
;

opt_int_ports:
    %empty { $$ = ( ast_list* )0; }
|   '{' int_port_list '}' { $$ = $2; }
;

int_port_list:
    decl_int_port opt_int_port_list {
        $$ = ast_add_list_elem( $1, $2 );
    }
;

opt_int_port_list:
    %empty { $$ = ( ast_list* )0; }
|   ',' decl_int_port opt_int_port_list {
        $$ = ast_add_list_elem( $2, $3 );
    }
;

decl_int_port:
    IDENTIFIER {
        $$ = ast_add_symbol( $1, @1.last_line, ID_IPORT );
    }
;

/* box definition */
def_box:
    IDENTIFIER '=' decl_box {
        $$ = ast_add_def(
            ast_add_symbol( $1, @1.last_line, ID_BOX ),
            $3,
            AST_BOX_DEF
        );
    }
;

/* box declarartion */
decl_box:
    kw_opt_state BOX IDENTIFIER '(' box_port_list ')' {
        $$ = ast_add_box(
            ast_add_symbol( $3, @3.last_line, ID_BOX_IMPL ),
            ast_add_list( $5, AST_PORTS ),
            ast_add_attr( $1, ATTR_BOX_STATE )
        );
    }
;

box_port_list:
    decl_box_port opt_box_port_list {
        $$ = ast_add_list_elem( $1, $2 );
    }
;

opt_box_port_list:
    %empty { $$ = ( ast_list* )0; }
|   ',' decl_box_port opt_box_port_list {
        $$ = ast_add_list_elem( $2, $3 );
    }
;

decl_box_port:
    kw_opt_port_class kw_port_mode IDENTIFIER {
        $$ = ast_add_port(
            ast_add_symbol( $3, @3.last_line, ID_PORT ),
            ( ast_node* )0, // no internal id
            ast_add_attr( $1, ATTR_PORT_CLASS ),
            ast_add_attr( $2, ATTR_PORT_MODE ),
            ( ast_node* )0, // no coupling
            PORT_BOX
        );
    }
|   SYNC '{' sync_port_list '}' {
        $$ = ast_add_list( $3, AST_SYNC );
    }
;

sync_port_list:
    decl_sync_port opt_sync_port_list {
        $$ = ast_add_list_elem( $1, $2 );
    }
;

opt_sync_port_list:
    %empty { $$ = ( ast_list* )0; }
|   ',' decl_sync_port opt_sync_port_list {
        $$ = ast_add_list_elem( $2, $3 );
    }
;

decl_sync_port:
    kw_opt_decoupled kw_opt_port_class IN IDENTIFIER {
        $$ = ast_add_port(
            ast_add_symbol( $4, @4.last_line, ID_PORT ),
            (ast_node*)0, // no internal id
            ast_add_attr( $2, ATTR_PORT_CLASS ),
            ast_add_attr( $3, ATTR_PORT_MODE ),
            ast_add_attr( $1, ATTR_PORT_COUPLING ),
            PORT_SYNC
        );
    }
;

/* wrapper declaration */
decl_wrap:
    kw_opt_static WRAPPER IDENTIFIER '{' wrap_port_list '}' '{' stmts_wrap '}' {
        $$ = ast_add_wrap(
            ast_add_symbol( $3, @3.last_line, ID_WRAP ),
            ast_add_list( $5, AST_PORTS ),
            ast_add_list( $8, AST_STMTS ),
            ast_add_attr( $1, ATTR_WRAP_STATIC )
        );
    }
;

wrap_port_list:
    decl_wrap_port opt_wrap_port_list {
        $$ = ast_add_list_elem( $1, $2 );
    }
;

opt_wrap_port_list:
    %empty { $$ = ( ast_list* )0; }
|   ',' decl_wrap_port opt_wrap_port_list {
        $$ = ast_add_list_elem( $2, $3 );
    }
;

decl_wrap_port:
    kw_opt_port_class kw_port_mode IDENTIFIER opt_int_ports {
        $$ = ast_add_port(
            ast_add_symbol( $3, @3.last_line, ID_PORT ),
            ast_add_list( $4, AST_INT_PORTS ),
            ast_add_attr( $1, ATTR_PORT_CLASS ),
            ast_add_attr( $2, ATTR_PORT_MODE ),
            ( ast_node* )0, // no coupling
            PORT_WRAP
        );
    }
|   '{' int_port_list '}' {
        $$ = ast_add_port(
            // these internal ports are "turned off"
            ast_add_symbol( VAL_NULL, @2.last_line, ID_NPORT ),
            ast_add_list( $2, AST_INT_PORTS ),
            ( ast_node* )0, // no collection
            ( ast_node* )0, // no mode
            ( ast_node* )0, // no coupling
            PORT_WRAP_NULL
        );
    }
;

/* list of statements in a wrapper */
stmts_wrap:
    %empty {
        $$ = ( ast_list* )0;
    }
|   stmt_wrap stmts_wrap {
        $$ = ast_add_list_elem( $1, $2 );
    }
;

stmt_wrap:
    stmt { $$ = $1; }
|   decl_link { $$ = $1; }
;

/* an explicit declaration of a connection of sideports or ports to the */
/* wrapper interface */
decl_link:
    LINK '{' link_list '}' {
        $$ = ast_add_list( $3, AST_LINKS );
    }
;

link_list:
    decl_link_id ',' decl_link_id opt_link_list {
        $$ = ast_add_list_elem(
            $1,
            ast_add_list_elem( $3, $4 )
        );
    }
;

opt_link_list:
    %empty {
        $$ = ( ast_list* )0;
    }
|   ',' decl_link_id opt_link_list {
        $$ = ast_add_list_elem( $2, $3 );
    }
;

decl_link_id:
    IDENTIFIER {
        $$ = ast_add_symbol( $1, @1.last_line, ID_LNET );
    }
;

/* keywords */
kw_opt_state:
    %empty { $$ = 0; }
|   STATELESS { $$ = $1; }
;

kw_opt_decoupled:
    %empty { $$ = 0; }
|   DECOUPLED { $$ = $1; }
;

kw_port_mode:
    IN   { $$ = $1;}
|   OUT  { $$ = $1;}
;

kw_opt_port_class:
    %empty { $$ = 0; }
|   kw_port_class { $$ = $1; }
;

kw_port_class:
    UP   { $$ = $1; }
|   DOWN { $$ = $1; }
|   SIDE { $$ = $1; }
;

kw_opt_static:
    %empty { $$ = 0; }
|   STATIC { $$ = $1; }
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

    /* cgraph_init( ast ); */
    check_context( ast );

    /* fclose(con_graph); */
    if( yynerrs > 0 ) printf( " Error count: %d\n", yynerrs );
#ifdef DOT_AST
    draw_ast_graph( ast );
#endif // DOT_AST

    return 0;
}

/*
 * error function of bison
 *
 * @param: char* s:  error string
 * */
void yyerror( const char* s ) {
    if( strlen(yytext) == 0 )
        printf( "%s: %d: %s\n", src_file_name, yylineno, s );
    else
        printf( "%s: %d: %s '%s'\n", src_file_name, yylineno, s, yytext );
}
