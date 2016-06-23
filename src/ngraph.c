#include "ngraph.h"
#include "defines.h"

/******************************************************************************/
void cgraph_connect_full_ptr( igraph_t* g, igraph_vector_ptr_t* v1,
        igraph_vector_ptr_t* v2 )
{
    igraph_vector_t edges;
    igraph_vector_init( &edges, 0 );
    int i, j;
    for( i=0; i<igraph_vector_ptr_size( v1 ); i++ ) {
        for( j=0; j<igraph_vector_ptr_size( v2 ); j++ ) {
            igraph_vector_push_back( &edges,
                    *( int* )igraph_vector_ptr_e( v1, i ) );
            igraph_vector_push_back( &edges,
                    *( int* )igraph_vector_ptr_e( v2, j ) );
        }
    }
    igraph_add_edges( g, &edges, 0 );
    igraph_vector_destroy( &edges );
}

/******************************************************************************/
void cgraph_disconnect_full( igraph_t* g, igraph_vector_t* v1,
        igraph_vector_t* v2 )
{
    int i, j;
    int id;
    for( i=0; i<igraph_vector_size( v1 ); i++ ) {
        for( j=0; j<igraph_vector_size( v2 ); j++ ) {
            igraph_get_eid( g, &id, igraph_vector_e( v1, i ),
                    igraph_vector_e( v2, j ), 0, 0 );
            if( id >= 0 ) igraph_delete_edges( g, igraph_ess_1( id ) );
        }
    }
}

/******************************************************************************/
void cgraph_update( igraph_t* g_con, int id1, int id2, int t1, int t2,
        igraph_t* g)
{
    igraph_vector_t nvid_l;
    igraph_vector_t nvid_r;
    igraph_vector_init( &nvid_l, 0 );
    igraph_vector_init( &nvid_r, 0 );
    if( t1 == VAL_CP )
        igraph_neighbors( g, &nvid_l, id1, IGRAPH_ALL );
    else
        igraph_vector_push_back( &nvid_l, id1 );
    if( t2 == VAL_CP )
        igraph_neighbors( g, &nvid_r, id2, IGRAPH_ALL );
    else
        igraph_vector_push_back( &nvid_r, id2 );
    cgraph_disconnect_full( g_con, &nvid_l, &nvid_r );
    igraph_vector_destroy( &nvid_l );
    igraph_vector_destroy( &nvid_r );
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
    igraph_get_eid( g, &id_edge, id_from, id_to, IGRAPH_DIRECTED, 0 );
    igraph_cattribute_EAS_set( g, "name", id_edge, name );
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
