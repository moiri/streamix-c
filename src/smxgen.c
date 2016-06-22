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
    int e_id;
    for( net=*nets; net != NULL; net=net->hh.next ) {
        // generate boxes
        for( rec=net->nodes; rec != NULL; rec=rec->hh.next ) {
            smxgen_box( net->scope, rec->id, 2 );
        }
        // generate channels and the connections to the boxes
        e_sel = igraph_ess_all( IGRAPH_EDGEORDER_ID );
        igraph_eit_create( &net->g, e_sel, &e_it );
        while( !IGRAPH_EIT_END( e_it ) ) {
            e_id = IGRAPH_EIT_GET( e_it );
            smxgen_channel( net->scope, e_id );
            for( rec=net->nodes; rec != NULL; rec=rec->hh.next ) {
                smxgen_connect( net->scope, e_id, rec->id );
            }
            IGRAPH_EIT_NEXT( e_it );
        }
    }
}
