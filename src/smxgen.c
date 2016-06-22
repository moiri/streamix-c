#include "smxgen.h"
#include <igraph.h>
#include <stdio.h>

/******************************************************************************/
void smxgen_box( int scope, int id, int port_cnt )
{
    printf( "smx_box_t* box_%d_%d = smx_box_create( %d );\n", id, scope,
            port_cnt );
}

/******************************************************************************/
void smxgen_channel( int scope, int id )
{
    printf( "smx_channel_t* ch_%d_%d = smx_channel_create();\n", id, scope );
}

/******************************************************************************/
void smxgen_connect( int scope, int id_ch, int id_box )
{
    printf( "SMX_CONNECT( ch_%d_%d, box_%d_%d );\n", id_ch, scope, id_box,
            scope );
}

/******************************************************************************/
void smxgen_network( inst_net** nets )
{
    inst_net* net;
    inst_rec* rec;
    igraph_es_t e_sel;
    igraph_eit_t e_it;
    igraph_vector_t eids;
    int eid, vid1, vid2;
    // for all scopes in th program
    for( net=*nets; net != NULL; net=net->hh.next ) {
        // for all boxes in the scope
        for( rec=net->nodes; rec != NULL; rec=rec->hh.next ) {
            // generate box creation code
            igraph_vector_init( &eids, 0 );
            igraph_incident( &net->g, &eids, rec->id, IGRAPH_ALL );
            smxgen_box( net->scope, rec->id, igraph_vector_size( &eids ) );
        }
        // for all channels in the scope
        e_sel = igraph_ess_all( IGRAPH_EDGEORDER_ID );
        igraph_eit_create( &net->g, e_sel, &e_it );
        while( !IGRAPH_EIT_END( e_it ) ) {
            // generate channel creation code
            eid = IGRAPH_EIT_GET( e_it );
            smxgen_channel( net->scope, eid );
            // generate connection code for a channel and its connecting boxes
            igraph_edge( &net->g, eid, &vid1, &vid2 );
            smxgen_connect( net->scope, eid, vid1 );
            smxgen_connect( net->scope, eid, vid2 );
            IGRAPH_EIT_NEXT( e_it );
        }
    }
}
