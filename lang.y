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
    void install ( char*, int, int, void*, int );
    void context_check ( char*, int );
    ast_node* ast;
    int num_errors = 0;
    char* src_file_name;
    char error_msg[255];
    FILE* con_graph;
    symrec* symtab = NULL;
    UT_array* scope_stack;
    int scope;
    int sync_id = 0;
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
%type <sval> scope_id opt_renaming
%type <nval> net nets stmt decl_box decl_net decl_connect
%type <nval> decl_bport syncport decl_nport port_mode port_class
%type <nval> opt_state opt_decoupled
%type <lval> stmts connect_list opt_connect_id
%type <lval> opt_decl_bport opt_syncport opt_decl_nport opt_port_class
%left '|'
%left '.'
%start start

%%
/* Grammar rules */

start:
    stmts {
        ast = ast_add_list($1, AST_STMTS);
    }
;

stmts:
    %empty {
        $$ = (ast_list*)0;
    }
|   stmt stmts {
        $$ = ast_add_list_elem($1, $2);
    }
;

stmt:
    nets {$$ = $1;}
|   decl_connect {$$ = $1;}
;

nets:
    net {
        draw_connection_graph(con_graph, $1);
        $$ = ast_add_node($1, AST_NET);
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
            ast_add_id($2, AST_ID),
            ast_add_list($4, AST_CONNECTS)
        );
    }
;

connect_list:
    IDENTIFIER opt_connect_id {
        context_check($1, @1.last_line);
        $$ = ast_add_list_elem(
            ast_add_id($1, AST_ID),
            $2
        );
    }
|   '*' {
        $$ = ast_add_list_elem(
            ast_add_star(),
            (ast_list*)0
        );
    }
;

opt_connect_id:
    %empty {
        $$ = (ast_list*)0;
    }
|   ',' IDENTIFIER opt_connect_id {
        context_check($2, @2.last_line);
        $$ = ast_add_list_elem(
            ast_add_id($2, AST_ID),
            $3
        );
    }
;

/* box declarartion */
decl_box:
    opt_state BOX scope_id '(' decl_bport opt_decl_bport ')' ON IDENTIFIER {
        box_attr *attr = ( box_attr* )malloc( sizeof( box_attr ) );
        attr->state = ( $1 == NULL ) ? false : true;
        utarray_pop_back( scope_stack );
        install($3, *( int* )utarray_back( scope_stack ), $2, ( void* )attr,
                @3.last_line);
        $$ = ast_add_box(
            ast_add_id($3, AST_ID),
            ast_add_list(
                ast_add_list_elem($5, $6),
                AST_PORTS
            ),
            ast_add_node($1, AST_STATE)
        );
    }
;

scope_id:
    IDENTIFIER {
        scope++;
        utarray_push_back( scope_stack, &scope );
        $$ = $1;
    }
;

opt_state:
    %empty { $$ = (ast_node*)0; }
|   STATELESS { $$ = ast_add_attr($1, ATTR_STATE); }
;

opt_decl_bport:
    %empty { $$ = (ast_list*)0; }
|   ',' decl_bport opt_decl_bport {
        $$ = ast_add_list_elem($2, $3);
    }
;

decl_bport:
    opt_port_class port_mode IDENTIFIER {
        ast_list* list = $1;
        port_attr *attr = ( port_attr* )malloc( sizeof( port_attr ) );
        attr->mode = $2->attr.val;
        attr->up = false;
        attr->down = false;
        attr->side = false;
        while( list != NULL ) {
            if( list->ast_node->attr.val == VAL_UP ) attr->up = true;
            else if( list->ast_node->attr.val == VAL_DOWN ) attr->down = true;
            else if( list->ast_node->attr.val == VAL_SIDE ) attr->side = true;
            list = list->next;
        }
        install($3, *( int* )utarray_back( scope_stack ), VAL_PORT,
                ( void* )attr, @3.last_line);
        $$ = ast_add_port(
            ast_add_id($3, AST_ID),
            (ast_node*)0, // no internal id
            ast_add_list($1, AST_COLLECT),
            ast_add_node($2, AST_MODE),
            (ast_node*)0, // no coupling
            PORT_BOX
        );
    }
|   SYNC '{' syncport opt_syncport '}' {
        sync_id++;
        $$ = ast_add_list(
            ast_add_list_elem($3, $4),
            AST_SYNC
        );
    }
;

opt_port_class:
    %empty { $$ = (ast_list*)0; }
|   port_class opt_port_class {
        $$ = ast_add_list_elem($1, $2);
    }
;

opt_syncport:
    %empty { $$ = (ast_list*)0; }
|   ',' syncport opt_syncport {
        $$ = ast_add_list_elem($2, $3);
    }
;

syncport:
    opt_decoupled opt_port_class IN IDENTIFIER {
        ast_list* list = $2;
        sport_attr *attr = ( sport_attr* )malloc( sizeof( sport_attr ) );
        attr->mode = $3;
        attr->sync_id = sync_id;
        attr->decoupled = ( $1 == NULL ) ? false : true;
        attr->up = false;
        attr->down = false;
        attr->side = false;
        while( list != NULL ) {
            if( list->ast_node->attr.val == VAL_UP ) attr->up = true;
            else if( list->ast_node->attr.val == VAL_DOWN ) attr->down = true;
            else if( list->ast_node->attr.val == VAL_SIDE ) attr->side = true;
            list = list->next;
        }
        install($4, *( int* )utarray_back( scope_stack ), VAL_SPORT,
                ( void* )attr, @4.last_line);
        $$ = ast_add_port(
            ast_add_id($4, AST_ID),
            (ast_node*)0, // no internal id
            ast_add_list($2, AST_COLLECT),
            ast_add_node(ast_add_attr($3, ATTR_MODE), AST_MODE),
            ast_add_node($1, AST_COUPLING),
            PORT_SYNC
        );
    }
;

opt_decoupled:
    %empty { $$ = (ast_node*)0; }
|   DECOUPLED { $$ = ast_add_attr($1, ATTR_COUPLING); }
;

port_mode:
    IN   { $$ = ast_add_attr($1, ATTR_MODE); }
|   OUT  { $$ = ast_add_attr($1, ATTR_MODE); }
;

port_class:
    UP   { $$ = ast_add_attr($1, ATTR_COLLECT); }
|   DOWN { $$ = ast_add_attr($1, ATTR_COLLECT); }
|   SIDE { $$ = ast_add_attr($1, ATTR_COLLECT); }
;

/* net declaration */
net:
    IDENTIFIER  { 
        context_check($1, @1.last_line);
        $$ = ast_add_id($1, AST_ID);
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
    NET scope_id '{' decl_nport opt_decl_nport '}' '{' stmts '}' {
        utarray_pop_back( scope_stack );
        install($2, *( int* )utarray_back( scope_stack ), $1, NULL,
                @2.last_line);
        $$ = ast_add_wrap(
            ast_add_id($2, AST_ID),
            ast_add_list(
                ast_add_list_elem($4, $5),
                AST_PORTS
            ),
            ast_add_list($8, AST_STMTS)
        );
    }
;

opt_decl_nport:
    %empty { $$ = (ast_list*)0; }
|   ',' decl_nport opt_decl_nport {
        $$ = ast_add_list_elem($2, $3);
    }
;

decl_nport:
    opt_port_class port_mode IDENTIFIER opt_renaming {
        ast_list* list = $1;
        port_attr *attr = ( port_attr* )malloc( sizeof( port_attr ) );
        attr->mode = $2->attr.val;
        attr->up = false;
        attr->down = false;
        attr->side = false;
        while( list != NULL ) {
            if( list->ast_node->attr.val == VAL_UP ) attr->up = true;
            else if( list->ast_node->attr.val == VAL_DOWN ) attr->down = true;
            else if( list->ast_node->attr.val == VAL_SIDE ) attr->side = true;
            list = list->next;
        }
        install($3, *( int* )utarray_back( scope_stack ), VAL_PORT,
                ( void* )attr, @3.last_line);
        $$ = ast_add_port(
            ast_add_id($3, AST_ID),
            ast_add_node(ast_add_id($4, AST_ID), AST_INT_ID),
            ast_add_list($1, AST_COLLECT),
            ast_add_node($2, AST_MODE),
            (ast_node*)0, // no coupling
            PORT_NET
        );
    }
;

opt_renaming:
    %empty { $$ = NULL; }
|   '(' IDENTIFIER ')' { $$ = $2; }
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
    // set flex to read from it instead of defaulting to STDIN    yyin = myfile;
    yyin = myfile;

    con_graph = fopen(CON_DOT_PATH, "w");
    utarray_new( scope_stack, &ut_int_icd );
    scope = 0;
    utarray_push_back( scope_stack, &scope );
    // parse through the input until there is no more:
    do {
        yyparse();
    } while (!feof(yyin));

    fclose(con_graph);
    if (num_errors > 0) printf(" Error count: %d\n", num_errors);
    else draw_ast_graph(ast);

    return 0;
}

/*
 * Add a new net identifier to the symbol table. If it is already there,
 * produce an error.
 *
 * @param: char* name:  name of the net
 * @param: int type:    type of the net
 * @param: int line:    line number of the symbol in the source file
 * */
void install ( char* name, int scope, int type, void* attr, int line ) {
    /* printf( "install" ); */
    /* if( type == VAL_BOX ) { */
    /*     if( ( ( struct box_attr* )attr )->state ) */
    /*         printf( " stateless" ); */
    /*     printf( " box" ); */
    /* } */
    /* else if( type == VAL_PORT || type == VAL_SPORT ) { */
    /*     if( ( ( struct port_attr* )attr )->up ) */
    /*         printf( " up" ); */
    /*     if( ( ( struct port_attr* )attr )->down ) */
    /*         printf( " down" ); */
    /*     if( ( ( struct port_attr* )attr )->side ) */
    /*         printf( " side" ); */
    /*     if( ( ( struct port_attr* )attr )->mode == VAL_IN ) */
    /*         printf( " in" ); */
    /*     else if( ( ( struct port_attr* )attr )->mode == VAL_OUT ) */
    /*         printf( " out" ); */
    /*     if( type == VAL_PORT ) printf( " port" ); */
    /*     else if( type == VAL_SPORT ) { */
    /*         if( ( ( struct sport_attr* )attr )->decoupled ) */
    /*             printf( " decoupled" ); */
    /*         printf( " sync(%d) port", ( ( struct sport_attr* )attr )->sync_id ); */
    /*     } */
    /* } */
    /* else if( type == VAL_NET ) */
    /*     printf( " net" ); */
    /* printf( " %s in scope %d\n", name, scope ); */
    if( !symrec_put( &symtab, name, scope, type, attr ) ) {
        sprintf( error_msg, ERROR_DUPLICATE_ID, line, name );
        yyerror( error_msg );
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
    int* p = NULL;
    bool in_scope = false;
    symrec* res = symrec_get( &symtab, name );
    if( res != NULL ) {
        /* iterate through all entries with the same name */
        do {
            /* check wheter their scope matches with a scope on the stack */
            while( ( p = ( int* )utarray_prev( scope_stack, p ) ) != NULL ) {
                if( *p == res->scope ) {
                    in_scope = true;
                    /* printf( "found" ); */
                    /* if( res->attr != NULL ) { */
                    /*     if( ( ( struct box_attr* )res->attr )->state ) */
                    /*         printf( " stateless" ); */
                    /*     printf( " box" ); */
                    /* } */
                    /* else if( res->type == VAL_NET ) */
                    /*     printf( " net" ); */
                    /* printf( " %s in scope %d\n", res->name, res->scope ); */
                    break;
                }
            }
            res = res->next;
        } while( !in_scope && res != NULL );
    }
    /* no hash was found or the scope does not match */
    if( !in_scope ) {
        sprintf( error_msg, ERROR_UNDEFINED_ID, line, name );
        yyerror( error_msg );
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
