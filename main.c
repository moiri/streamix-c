#include <stdio.h>
#include <getopt.h>
#include <ctype.h>
#include "streamix.tab.h"
#include "context.h"
#include "smxerr.h"
#include "smxgraph.h"
#include "sia.h"
#ifdef DOT_AST
    #include "smxdot.h"
#endif // DOT_AST

char* __src_file_name;
extern FILE *yyin;
extern int yyparse( void** );
extern int yylex_destroy();
extern FILE *zzin;
extern int zzparse( void** );
extern int zzlex_destroy();


int main( int argc, char **argv ) {
    void* ast = NULL;
    void* sias = NULL;
    symrec_t* symtab = NULL;        // hash table to store the symbols
    sia_t* sia_symbols;
    char* out_file_name = NULL;
    const char* format = NULL;
    const char* sia_desc_file = NULL;
    const char* sia_desc_path = NULL;
    FILE* src_smx;
    FILE* src_sia;
    FILE* out_file;
    igraph_i_set_attribute_table( &igraph_cattribute_table );
    igraph_t g;
    int c;

    while( ( c = getopt( argc, argv, "hvs:p:o:f:" ) ) != -1 )
        switch( c ) {
            case 'h':
                printf( "Usage:\n  %s [OPTION...] FILE\n\n", argv[0] );
                printf( "Options:\n" );
                printf( "  -h            This message\n" );
                printf( "  -v            Version\n" );
                printf( "  -s 'path'     Path to file with SIA descriptions\n" );
                printf( "  -p 'path'     Path to folder to store the SIA graphs\n" );
                printf( "  -o 'path'     Path to file to store the SMX graph\n" );
                printf( "  -f 'format'   Format of the graph either 'gml' or 'graphml'\n" );
                return 0;
            case 'v':
                printf( "smxc-v0.0.1\n" );
                return 0;
            case 's':
                sia_desc_file = optarg;
                break;
            case 'p':
                sia_desc_path = optarg;
                break;
            case 'o':
                out_file_name = optarg;
                break;
            case 'f':
                format = optarg;
                break;
            case '?':
                if( ( optopt == 'o' ) || ( optopt == 'f' )
                        || ( optopt == 's' ) || ( optopt == 'p' ) )
                    fprintf ( stderr, "Option -%c requires an argument.\n",
                            optopt );
                else if ( isprint (optopt) )
                    fprintf ( stderr, "Unknown option `-%c'.\n", optopt );
                else
                    fprintf ( stderr, "Unknown option character `\\x%x'.\n",
                            optopt );
                return 1;
            default:
                abort();
        }
    if( argc <= optind ) {
        fprintf( stderr, "Missing argument!\n" );
        return -1;
    }
    __src_file_name = argv[ optind ];
    if( format == NULL ) format = "graphml";

    // PARSE SMX FILE
    src_smx = fopen( __src_file_name, "r" );
    // make sure it is valid:
    if( !src_smx ) {
        printf( "Cannot open file '%s'!\n", __src_file_name );
        return -1;
    }
    if( out_file_name == NULL ) {
        out_file_name = malloc( strlen( format ) + 5 );
        sprintf( out_file_name, "out.%s", format );
    }
    out_file = fopen( out_file_name, "w" );
    // set flex to read from it instead of defaulting to STDIN
    yyin = src_smx;

    // parse through the input until there is no more:
    do {
        yyparse( &ast );
    } while( !feof( yyin ) );
    fclose( src_smx );

    if( ast == NULL ) return -1;

    // CHECK SMX CONTEXT
    igraph_empty( &g, 0, true );
    ast_flatten( ast );
    check_context( ast, &symtab, &g );

    // PARSE SIA FILE
    if( sia_desc_file != NULL ) {
        src_sia = fopen( sia_desc_file, "r" );
        // make sure it is valid:
        if( !src_sia ) {
            printf( "Cannot open file '%s'!\n", sia_desc_file );
            return -1;
        }
        if( sia_desc_path == NULL ) sia_desc_path = "./";
        // set flex to read from it instead of defaulting to STDIN
        zzin = src_sia;

        // parse through the input until there is no more:
        do {
            zzparse( &sias );
        } while( !feof( zzin ) );
        fclose( src_sia );

        if( sias == NULL ) return -1;

        sia_check( sias, &sia_symbols );
        sia_write( &sia_symbols, sia_desc_path, format );
    }

    if( strcmp( format, "gml" ) == 0 ) {
        igraph_write_graph_gml( &g, out_file, NULL, "StreamixC" );
    }
    else if( strcmp( format, "graphml" ) == 0 ) {
        igraph_write_graph_graphml( &g, out_file, 0 );
    }
    else {
        printf( "Unknown format '%s'!\n", format );
        return -1;
    }

    fclose( out_file );

    if( yynerrs > 0 ) printf( " Error count: %d\n", yynerrs );
#ifdef DOT_CON
    out_file = fopen( P_CON_DOT_PATH, "w" );
    igraph_write_graph_dot( &g, out_file );
    fclose( out_file );
#endif // DOT_CON
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
