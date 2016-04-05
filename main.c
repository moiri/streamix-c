#include <stdio.h>
#include "streamix.tab.h"
#include "context.h"
#include "error.h"
#ifdef DOT_AST
    #include "dot.h"
#endif // DOT_AST

char* __src_file_name;
extern FILE *yyin;
extern FILE *yyout;
extern int yyparse( void** ) ;


int main( int argc, char **argv ) {
    void* ast = NULL;
    // open a file handle to a particular file:
    if( argc != 2 ) {
        printf( "Missing argument!\n" );
        return -1;
    }
    __src_file_name = argv[ 1 ];
    FILE *myfile = fopen( __src_file_name, "r" );
    // make sure it is valid:
    if( !myfile ) {
        printf( "Cannot open file '%s'!\n", __src_file_name );
        return -1;
    }
    // set flex to read from it instead of defaulting to STDIN    yyin = myfile;
    yyin = myfile;

    /* con_graph = fopen(CON_DOT_PATH, "w"); */
    // parse through the input until there is no more:
    do {
        yyparse( &ast );
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
