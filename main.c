#include <sys/stat.h>
#include <stdio.h>
#include <getopt.h>
#include <ctype.h>
#include "streamix.tab.h"
#include "context.h"
#include "smxerr.h"
#include "smxgraph.h"
#include "sia.h"
#include "smx2sia.h"
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

int __smxc_min_ch_len = 1;

int get_path_size( const char* str )
{
    const char* slash;
    int path_size;
    slash = strrchr( str, '/' );
    if( slash == NULL ) {
        return 0;
    }
    else {
        path_size = slash - str + 1;
    }
    return path_size;
}

int main( int argc, char **argv ) {
    void* ast = NULL;
    void* sias = NULL;
    symrec_t* symtab = NULL;        // hash table to store the symbols
    sia_t* sia_desc_symbols = NULL;
    sia_t* sia_smx_symbols = NULL;
    char* out_file_path = NULL;
    const char* out_file_name = NULL;
    const char* format = NULL;
    const char* sia_desc_file = NULL;
    const char* build_path = NULL;
    char* build_path_sia = NULL;
    int name_size;
    int path_size;
    char* file_name;
    FILE* src_smx;
    FILE* src_sia;
    FILE* out_file;
    bool skip_sia = false;
    igraph_i_set_attribute_table( &igraph_cattribute_table );
    igraph_t g;
    int c;

    while( ( c = getopt( argc, argv, "hvs:Sp:o:f:l:" ) ) != -1 )
        switch( c ) {
            case 'h':
                printf( "Usage:\n  %s [OPTION...] FILE\n\n", argv[0] );
                printf( "Options:\n" );
                printf( "  -h            This message\n" );
                printf( "  -v            Version\n" );
                printf( "  -l 'length'   The minimal channel length if no lenght is provided\n" );
                printf( "  -s 'path'     Path to input file with SIA descriptions\n" );
                printf( "  -S            Skip the SIA generation\n" );
                printf( "  -p 'path'     Build path to folder where the output files will be stored\n" );
                printf( "  -o 'file'     Filename of the SMX graph output file\n" );
                printf( "  -f 'format'   Format of the graph either 'gml' or 'graphml'\n" );
                return 0;
            case 'v':
                printf( "smxc-v0.3.0\n" );
                return 0;
            case 's':
                sia_desc_file = optarg;
                break;
            case 'l':
                __smxc_min_ch_len = atoi( optarg );
                break;
            case 'S':
                skip_sia = true;
                break;
            case 'p':
                build_path = optarg;
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

    if( __smxc_min_ch_len <= 0 ) {
        fprintf( stderr, "The argument of '-l' must be a positive integer,"
                " '%d' provided\n", __smxc_min_ch_len );
        return -1;
    }

    __src_file_name = argv[ optind ];
    path_size = get_path_size( __src_file_name );
    name_size = strlen( __src_file_name ) - path_size - 4;
    file_name = malloc( name_size + 1 ); // minus ".smx"
    memcpy( file_name, &__src_file_name[path_size], name_size );
    file_name[ name_size ] = '\0';

    if( format == NULL ) format = G_FMT_GRAPHML;
    if( build_path == NULL ) build_path = "./build";
    mkdir( build_path, 0755 );

    // PARSE SMX FILE
    src_smx = fopen( __src_file_name, "r" );
    // make sure it is valid:
    if( !src_smx ) {
        printf( "Cannot open file '%s'!\n", __src_file_name );
        return -1;
    }
    if( out_file_name == NULL ) {
        out_file_path = malloc( strlen( build_path ) + strlen( format ) + strlen( file_name ) + 3 );
        sprintf( out_file_path, "%s/%s.%s", build_path, file_name, format );
    }
    else {
        out_file_path = malloc( strlen( build_path ) + strlen( out_file_name )
                + 2 );
        sprintf( out_file_path, "%s/%s", build_path, out_file_name );
    }
    out_file = fopen( out_file_path, "w" );
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
    check_context( ast, &symtab, &g );

    // PARSE SIA FILE
    if( sia_desc_file != NULL ) {
        src_sia = fopen( sia_desc_file, "r" );
        // make sure it is valid:
        if( !src_sia ) {
            printf( "Cannot open file '%s'!\n", sia_desc_file );
            return -1;
        }
        // set flex to read from it instead of defaulting to STDIN
        zzin = src_sia;

        // parse through the input until there is no more:
        do {
            zzparse( &sias );
        } while( !feof( zzin ) );
        fclose( src_sia );

        if( sias == NULL ) return -1;

        // CHECK SIA CONTEXT
        sia_check( sias, &sia_desc_symbols );
    }

    // CREATE SIAs WHERE NO DESCRIPTION EXISTS
    if( !skip_sia ) {
        smx2sia( &g, &sia_smx_symbols, &sia_desc_symbols );
        build_path_sia = malloc( strlen( build_path ) + 5 );
        sprintf( build_path_sia, "%s/sia", build_path );
        mkdir( build_path_sia, 0755 );
        smx2sia_sias_write( &sia_smx_symbols, build_path_sia, format );
    }

    // WRITE OUT SMX
    dgraph_destroy_attr( &g );

    if( strcmp( format, G_FMT_GML ) == 0 ) {
        igraph_write_graph_gml( &g, out_file, NULL, G_GML_HEAD );
    }
    else if( strcmp( format, G_FMT_GRAPHML ) == 0 ) {
        igraph_write_graph_graphml( &g, out_file, 0 );
    }
    else {
        printf( "Unknown format '%s'!\n", format );
        return -1;
    }

    fclose( out_file );

    if( yynerrs > 0 ) printf( " Error count: %d\n", yynerrs );
#ifdef DOT_CON
    mkdir( DOT_FOLDER, 0755 );
    out_file = fopen( P_CON_DOT_PATH, "w" );
    igraph_write_graph_dot( &g, out_file );
    fclose( out_file );
#endif // DOT_CON
#ifdef DOT_AST
    draw_ast_graph( ast );
#endif // DOT_AST

    // cleanup
    free( out_file_path );
    free( build_path_sia );
    free( file_name );
    igraph_destroy( &g );
    smx2sia_sias_destroy( sias, &sia_desc_symbols, &sia_smx_symbols );
    ast_destroy( ast );
    symrec_del_all( &symtab );
    yylex_destroy();

    return 0;
}
