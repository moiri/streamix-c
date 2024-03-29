/* 
 * The grammar of the coordination language streamix
 *
 * @file    lang.y
 * @author  Simon Maurer
 *
 * */

%{
/* Prologue */
    #include "ast.h"
    #include "defines.h"
    #include <stdio.h>
    extern int yylex();
    extern void yyerror ( void**, const char* );
%}

/* Bison declarations */
%parse-param { void** ast }
%define parse.error verbose
%define parse.lac full
%locations
%union {
    int ival;
    char *sval;
    struct ast_node_s* nval;
    struct ast_list_s* lval;
    struct timespec    tval;
};
/* keywods */
%token CONNECT TT TB TF RT
%token <ival> BOX WRAPPER NET IN OUT UP DOWN SIDE DECOUPLED COUPLED STATELESS STATIC EXTERN INTERN OPEN DYNAMIC BUFLEN
%token <tval> TIME_SEC TIME_MSEC TIME_USEC TIME_NSEC

/* optional and variable keyword tokens */
%type <nval> kw_opt_state
%type <nval> kw_opt_location
%type <nval> kw_opt_connection
%type <nval> kw_opt_static
%type <nval> kw_opt_decoupled
%type <nval> kw_port_class
%type <nval> kw_opt_port_class
%type <nval> kw_port_mode
%type <tval> kw_time
%type <tval> kw_time_sec
%type <tval> kw_time_msec
%type <tval> kw_time_usec
%type <tval> kw_time_nsec

/* idenitifiers */
%token <sval> IDENTIFIER

/* declarations */
%type <nval> net_decl
%type <nval> program
%type <nval> net
%type <nval> box_assign
%type <nval> net_assign
%type <nval> net_proto
%type <nval> net_port_decl
%type <nval> box_decl
%type <nval> box_port_decl
%type <nval> wrap_decl
%type <nval> wrap_port_decl
%type <nval> alt_port_decl
%type <nval> opt_alt_ports
%type <nval> opt_channel_len
%type <nval> opt_alt_port_name

/* lists */
%type <lval> net_decls
%type <lval> box_port_list
%type <lval> opt_box_port_list
%type <lval> net_port_list
%type <lval> opt_net_port_list
%type <lval> wrap_port_list
%type <lval> opt_wrap_port_list
%type <lval> alt_port_list
%type <lval> alt_ports
%type <lval> opt_alt_port_list


%left '|'
%left '!'
%left ':'
%left '.'
%start start

%%
/* Grammar rules */
/* start of the grammer */
start:
    program { *ast = $1; }
;

program:
    net_decls CONNECT net {
        $$ = ast_add_prog(
            ast_add_list( $1, AST_STMTS ),
            ast_add_net( $3 )
        );
    }
;

/* a list of statements describing the program */
net_decls:
    %empty { $$ = ( ast_list_t* )0; }
|   net_decl net_decls { $$ = ast_add_list_elem( $1, $2 ); }
;

net_decl:
    box_assign { $$ = $1; }
|   net_assign { $$ = $1; }
|   net_proto { $$ = $1; }
|   wrap_decl { $$ = $1; }
;

/* box definition */
box_assign:
    IDENTIFIER '=' box_decl {
        $$ = ast_add_assign(
            ast_add_symbol( $1, @1.last_line, ID_BOX ),
            $3,
            AST_BOX
        );
    }
;

/* net definition */
net_assign:
    IDENTIFIER '=' net {
        $$ = ast_add_assign(
            ast_add_symbol( $1, @1.last_line, ID_NET ),
            ast_add_net( $3 ),
            AST_NET
        );
    }
;

net:
    IDENTIFIER  { $$ = ast_add_symbol( $1, @1.last_line, ID_NET ); }
|   net '.' net { $$ = ast_add_op( $1, $3, AST_SERIAL ); }
|   net ':' net { $$ = ast_add_op( $1, $3, AST_SERIAL_PROP ); }
|   net '!' net { $$ = ast_add_op( $1, $3, AST_PARALLEL ); }
|   net '|' net { $$ = ast_add_op( $1, $3, AST_PARALLEL_DET ); }
|   '(' net ')' { $$ = $2; }
|   RT '(' net ')' {
        struct timespec time;
        time.tv_sec = 0;
        time.tv_nsec = 0;
        $$ = ast_add_time( $3, time, AST_RT, @3.last_line );
    }
|   TT '[' kw_time ']' '(' net ')' {
        $$ = ast_add_time( $6, $3, AST_TT, @3.last_line );
    }
|   TB '[' kw_time ']' '(' net ')' {
        $$ = ast_add_time( $6, $3, AST_TB, @3.last_line );
    }
|   TF '[' kw_time ']' '(' net ')' {
        $$ = ast_add_time( $6, $3, AST_TF, @3.last_line );
    }
;

/* net prototyping */
net_proto:
    NET IDENTIFIER '(' net_port_list ')' {
        $$ = ast_add_proto(
            ast_add_symbol( $2, @2.last_line, ID_NET ),
            ast_add_list( $4, AST_PORTS )
        );
    }
;

net_port_list:
    net_port_decl opt_net_port_list {
        $$ = ast_add_list_elem( $1, $2 );
    }
;

opt_net_port_list:
    %empty { $$ = ( ast_list_t* )0; }
|   ',' net_port_decl opt_net_port_list {
        $$ = ast_add_list_elem( $2, $3 );
    }
;

net_port_decl:
    kw_port_class kw_port_mode IDENTIFIER {
        $$ = ast_add_port(
            ast_add_symbol( $3, @3.last_line, ID_PORT ),
            ( ast_node_t* )0, // no internal ports
            $1,
            $2,
            ( ast_node_t* )0, // no coupling
            ( ast_node_t* )0, // no channel length
            ( ast_node_t* )0, // cannot be open or dynamic
            PORT_NET
        );
    }
;

/* box declarartion */
box_decl:
    kw_opt_location kw_opt_state BOX IDENTIFIER '(' box_port_list ')' {
        $$ = ast_add_box(
            ast_add_symbol( $4, @4.last_line, ID_BOX_IMPL ),
            ast_add_list( $6, AST_PORTS ),
            $2,
            $1
        );
    }
;

box_port_list:
    box_port_decl opt_box_port_list {
        $$ = ast_add_list_elem( $1, $2 );
    }
;

opt_box_port_list:
    %empty { $$ = ( ast_list_t* )0; }
|   ',' box_port_decl opt_box_port_list {
        $$ = ast_add_list_elem( $2, $3 );
    }
;

box_port_decl:
    kw_opt_decoupled kw_opt_port_class kw_port_mode IDENTIFIER opt_alt_port_name opt_channel_len kw_opt_connection {
        $$ = ast_add_port(
            ast_add_symbol( $4, @4.last_line, ID_PORT ),
            $5, // alternative port name
            $2, // port collection
            $3, // port mode
            $1, // port coupling
            $6, // channel length
            $7, // connection attr
            PORT_BOX
        );
    }
;

opt_alt_port_name:
    %empty { $$ = ( ast_node_t*)0; }
|   '(' IDENTIFIER ')' { $$ = ast_add_symbol( $2, @2.last_line, ID_PORT_ALT ); }
;

opt_channel_len:
    %empty { $$ = ( ast_node_t* )0; }
|   '[' BUFLEN ']' { $$ = ast_add_attr( $2, ATTR_INT ); }
;

/* wrapper declaration */
wrap_decl:
    kw_opt_static WRAPPER IDENTIFIER '(' wrap_port_list ')' '{' program '}'
    NET '(' net_port_list ')' {
        $$ = ast_add_wrap(
            ast_add_symbol( $3, @3.last_line, ID_WRAP ),
            ast_add_list( $5, AST_PORTS ),
            ast_add_list( $12, AST_PORTS ),
            $8,
            $1
        );
    }
;

wrap_port_list:
    wrap_port_decl opt_wrap_port_list {
        $$ = ast_add_list_elem( $1, $2 );
    }
;

opt_wrap_port_list:
    %empty { $$ = ( ast_list_t* )0; }
|   ',' wrap_port_decl opt_wrap_port_list {
        $$ = ast_add_list_elem( $2, $3 );
    }
;

wrap_port_decl:
    kw_opt_decoupled kw_opt_port_class kw_port_mode IDENTIFIER opt_alt_ports {
        $$ = ast_add_port(
            ast_add_symbol( $4, @4.last_line, ID_PORT ),
            $5,
            $2,
            $3,
            $1, // port coupling
            ( ast_node_t* )0, // no channel length
            ( ast_node_t* )0, // cannot be open nor dynamic
            PORT_WRAP
        );
    }
|   alt_ports {
        $$ = ast_add_port(
            // these internal ports are "turned off"
            ast_add_symbol( TEXT_NULL, @1.last_line, ID_NPORT ),
            ast_add_list( $1, AST_INT_PORTS ),
            ( ast_node_t* )0, // no collection
            ( ast_node_t* )0, // no mode
            ( ast_node_t* )0, // no coupling
            ( ast_node_t* )0, // no channel length
            ( ast_node_t* )0, // cannot be open nor dynamic
            PORT_WRAP_NULL
        );
    }
;

opt_alt_ports:
    %empty { $$ = ( ast_node_t* )0; }
|   alt_ports { $$ = ast_add_list( $1, AST_INT_PORTS ); }
;

alt_ports:
    '(' alt_port_list ')' { $$ = $2; }
;

alt_port_list:
    alt_port_decl opt_alt_port_list {
        $$ = ast_add_list_elem( $1, $2 );
    }
;

opt_alt_port_list:
    %empty { $$ = ( ast_list_t* )0; }
|   ',' alt_port_decl opt_alt_port_list {
        $$ = ast_add_list_elem( $2, $3 );
    }
;

alt_port_decl:
    IDENTIFIER {
        $$ = ast_add_port(
            ast_add_symbol( $1, @1.last_line, ID_IPORT ),
            ( ast_node_t* )0, // no internal id
            ( ast_node_t* )0, // no class
            ( ast_node_t* )0, // no mode
            ( ast_node_t* )0, // no coupling
            ( ast_node_t* )0, // no channel length
            ( ast_node_t* )0, // cannot be open nor dynamic
            PORT_BOX
        );
    }
;

/* keywords */
kw_opt_connection:
    %empty { $$ = ( ast_node_t* )0; }
|   OPEN   { $$ = ast_add_attr( $1, ATTR_OTHER ); }
|   DYNAMIC   { $$ = ast_add_attr( $1, ATTR_OTHER ); }
;

kw_opt_location:
    %empty { $$ = ( ast_node_t* )0; }
|   EXTERN { $$ = ast_add_attr( $1, ATTR_OTHER ); }
|   INTERN { $$ = ast_add_attr( $1, ATTR_OTHER ); }
;

kw_opt_state:
    %empty    { $$ = ( ast_node_t* )0; }
|   STATELESS { $$ = ast_add_attr( $1, ATTR_OTHER ); }
;

kw_opt_decoupled:
    %empty    { $$ = ( ast_node_t* )0; }
|   DECOUPLED { $$ = ast_add_attr( $1, ATTR_OTHER ); }
|   COUPLED   { $$ = ast_add_attr( $1, ATTR_OTHER ); }
;

kw_port_mode:
    IN   { $$ = ast_add_attr( $1, ATTR_PORT_MODE );}
|   OUT  { $$ = ast_add_attr( $1, ATTR_PORT_MODE );}
;

kw_opt_port_class:
    %empty { $$ = ( ast_node_t* )0; }
|   kw_port_class { $$ = $1; }
;

kw_port_class:
    UP   { $$ = ast_add_attr( $1, ATTR_PORT_CLASS ); }
|   DOWN { $$ = ast_add_attr( $1, ATTR_PORT_CLASS ); }
|   SIDE { $$ = ast_add_attr( $1, ATTR_PORT_CLASS ); }
;

kw_opt_static:
    %empty { $$ = ( ast_node_t* )0; }
|   STATIC { $$ = ast_add_attr( $1, ATTR_OTHER ); }
;

kw_time:
    kw_time_sec kw_time_msec kw_time_usec kw_time_nsec {
        $$.tv_sec = $1.tv_sec + $2.tv_sec + $3.tv_sec + $4.tv_sec;
        $$.tv_nsec = $1.tv_nsec + $2.tv_nsec + $3.tv_nsec + $4.tv_nsec;
    }
;

kw_time_sec:
    %empty {
        struct timespec time;
        time.tv_sec = 0;
        time.tv_nsec = 0;
        $$ = time;
    }
|   TIME_SEC { $$ = $1; }
;

kw_time_msec:
    %empty {
        struct timespec time;
        time.tv_sec = 0;
        time.tv_nsec = 0;
        $$ = time;
    }
|   TIME_MSEC { $$ = $1; }
;

kw_time_usec:
    %empty {
        struct timespec time;
        time.tv_sec = 0;
        time.tv_nsec = 0;
        $$ = time;
    }
|   TIME_USEC { $$ = $1; }
;

kw_time_nsec:
    %empty {
        struct timespec time;
        time.tv_sec = 0;
        time.tv_nsec = 0;
        $$ = time;
    }
|   TIME_NSEC { $$ = $1; }
;

%%
/* Epilogue */
