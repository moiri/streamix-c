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
int __smxc_time_criticality_prio[TIME_CTITICALITY_COUNT] = { 1, 1, 2, 3 };

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

void print_usage( const char* name )
{
    printf( "Usage:\n  %s [OPTION...] FILE\n", name );
    printf( "\nMiscellaneous:\n" );
    printf( "  -h, --help                  display this help text and exit\n" );
    printf( "  -V, --version               display version information and"
            " exit\n" );
    printf( "\nChannels:\n" );
    printf( "  -l, --channel-len=LENGTH    set the default channel length\n" );
    printf( "\nReal-time Priorities:\n" );
    printf( "      --tt-prio-single=PRIO   set the rt-thread priority of"
            " isolated tt nets\n" );
    printf( "      --tt-prio-network=PRIO  set the rt-thread priority of"
            " networked tt nets\n" );
    printf( "      --rt-prio-single=PRIO   set the rt-thread priority of"
            " isolated rt nets\n" );
    printf( "      --rt-prio-network=PRIO  set the rt-thread priority of"
            " networked rt nets\n" );
    printf( "\nOutput Control:\n" );
    printf( "  -s, --sia-path=PATH         set the path to the input file with"
            " SIA\n" );
    printf( "                              descriptions\n" );
    printf( "  -S, --skip-sia              skip the SIA generation\n" );
    printf( "  -p, --build-path=PATH       set the build path to folder where"
            " the output\n" );
    printf( "                              files will be stored\n" );
    printf( "  -o, --graph-name=FILE       set the filename of the SMX graph"
            " output file\n" );
    printf( "  -f, --graph-format=FROMAT   set the format of the graph to"
            " either 'gml' or\n" );
    printf( "                              'graphml'\n" );
}

int main( int argc, char **argv )
{
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
    int i;
    int option_index = 0;
    struct option long_options[] = {
        { "tt-prio-single",  required_argument, 0,  0  },
        { "tt-prio-network", required_argument, 0,  0  },
        { "rt-prio-single",  required_argument, 0,  0  },
        { "rt-prio-network", required_argument, 0,  0  },
        { "help",            no_argument,       0, 'h' },
        { "version",         no_argument,       0, 'V' },
        { "channel-len",     required_argument, 0, 'l' },
        { "sia-path",        required_argument, 0, 's' },
        { "skip-sia",        required_argument, 0, 'S' },
        { "build-path",      required_argument, 0, 'p' },
        { "graph-name",      required_argument, 0, 'o' },
        { "graph-format",    required_argument, 0, 'f' },
        { 0,                 0,                 0,  0  }
    };

    while( 1 )
    {
        option_index = 0;
        c = getopt_long( argc, argv, "hVs:Sp:o:f:l:", long_options,
                &option_index );
        if( c == -1 )
            break;

        switch( c ) {
            case 'h':
                print_usage( argv[0] );
                return 0;
            case 0:
                if( option_index < 4 )
                {
                    __smxc_time_criticality_prio[option_index] = atoi( optarg );
                }
                break;
            case 'V':
                printf( "smxc-v0.5.1\n" );
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
                break;
            default:
                abort();
        }
    }
    __src_file_name = argv[ optind++ ];
    if( __src_file_name == NULL )
    {
        fprintf( stderr, "Missing FILE argument!\n" );
        print_usage( argv[0] );
        return -1;
    }

    if (optind < argc) {
        printf("non-option ARGV-elements: ");
        while (optind < argc)
            printf("%s ", argv[optind++]);
        printf("\n");
        print_usage( argv[0] );
        return -1;
    }

    if( __smxc_min_ch_len <= 0 ) {
        fprintf( stderr, "The argument of '-l' must be a positive integer,"
                " '%d' provided\n", __smxc_min_ch_len );
        return -1;
    }

    for( i = 0; i < 4; i++ )
    {
        if( __smxc_time_criticality_prio[i] <= 0 )
        {
            fprintf( stderr, "An RT thread priority must be a positive integer,"
                    " '%d' provided\n", __smxc_time_criticality_prio[i] );
            return -1;
        }
    }

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
