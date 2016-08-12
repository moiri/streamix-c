#include <stdio.h>
#include "streamix.tab.h"
#include "context.h"
#include "smxerr.h"
#include "smxgraph.h"
#ifdef DOT_AST
    #include "smxdot.h"
#endif // DOT_AST

char* __src_file_name;
extern FILE *yyin;
extern FILE *yyout;
extern int yyparse( void** );
extern int yylex_destroy();


int main( int argc, char **argv ) {
    void* ast = NULL;
    symrec_t* symtab = NULL;        // hash table to store the symbols
    FILE* src_smx;
    FILE* dest_gml;
    igraph_i_set_attribute_table( &igraph_cattribute_table );
    igraph_t g;
    // open a file handle to a particular file:
    if( argc != 2 ) {
        printf( "Missing argument!\n" );
        return -1;
    }
    __src_file_name = argv[ 1 ];
    src_smx = fopen( __src_file_name, "r" );
    // make sure it is valid:
    if( !src_smx ) {
        printf( "Cannot open file '%s'!\n", __src_file_name );
        return -1;
    }
    // set flex to read from it instead of defaulting to STDIN    yyin = myfile;
    yyin = src_smx;

    // parse through the input until there is no more:
    do {
        yyparse( &ast );
    } while( !feof( yyin ) );
    fclose( src_smx );

    if( ast == NULL ) return -1;

    igraph_empty( &g, 0, true );
    ast_flatten( ast );
    check_context( ast, &symtab, &g );
    dest_gml = fopen( "streamix.gml", "w" );
    igraph_write_graph_gml( &g, dest_gml, NULL, "StreamixC" );
    fclose( dest_gml );
    dest_gml = fopen( P_CON_DOT_PATH, "w" );
    igraph_write_graph_dot( &g, dest_gml );
    fclose( dest_gml );

    if( yynerrs > 0 ) printf( " Error count: %d\n", yynerrs );
#ifdef DOT_AST
    draw_ast_graph( ast );
#endif // DOT_AST

    // cleanup
    igraph_destroy( &g );
    ast_destroy( ast );
    symrec_del_all( &symtab );
    yylex_destroy();

    return 0;
}
