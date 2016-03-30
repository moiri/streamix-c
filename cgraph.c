#include "cgraph.h"

/******************************************************************************/
void cgraph_connect( igraph_t* g, igraph_vector_t* v1, igraph_vector_t* v2 )
{
    igraph_vector_t edges;
    int i, j;
    igraph_vector_init( &edges, 0 );
    for( i=0; i<igraph_vector_size( v1 ); i++ ) {
        for( j=0; j<igraph_vector_size( v2 ); j++ ) {
            igraph_vector_push_back( &edges, VECTOR( *v1 )[i] );
            igraph_vector_push_back( &edges, VECTOR( *v2 )[j] );
        }
    }
    igraph_add_edges( g, &edges, 0 );
    igraph_vector_destroy( &edges );
}
