#include "ngraph.h"
#include "defines.h"

/******************************************************************************/
void dgraph_vptr_to_v( igraph_vector_ptr_t* vptr, igraph_vector_t* v )
{
    int i;
    igraph_vector_resize( v, igraph_vector_ptr_size( vptr ) );
    for( i = 0; i < igraph_vector_ptr_size( vptr ); i++ )
        VECTOR( *v )[i] = *( int* )igraph_vector_ptr_e( vptr, i );
}

/******************************************************************************/
void dgraph_connect_1( igraph_t* g, int id1, int id2, int mode1, int mode2,
        const char* name )
{
    int id_from = id1;
    int id_to = id2;
    int id_edge;
    if( ( mode2 == VAL_OUT ) || ( mode1 == VAL_IN ) ) {
        id_to = id_from;
        id_from = id2;
    }
    igraph_add_edge( g, id_from, id_to );
    igraph_get_eid( g, &id_edge, id_from, id_to, 0, 0 );
    igraph_cattribute_EAS_set( g, "label", id_edge, name );
}

/******************************************************************************/
int dgraph_merge_vertice_1( igraph_t* g, int id1, int id2 )
{
    igraph_vector_t v_new;
    int idx;
    int id_new = 0;
    int id_low = id1;
    int id_high = id2;
    if( id2 < id1 ) {
        id_low = id2;
        id_high = id1;
    }
    igraph_vector_init( &v_new, igraph_vcount( g ) );
    for( idx=0; idx<igraph_vcount( g ); idx++ ) {
        if( idx == id_high ) VECTOR( v_new )[ idx ] = id_low;
        else {
            VECTOR( v_new )[ idx ] = id_new;
            id_new++;
        }
    }
    igraph_contract_vertices( g, &v_new, NULL );

    // id of deleted element
    return id_high;
}
