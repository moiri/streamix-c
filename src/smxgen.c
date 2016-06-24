#include "smxgen.h"
#include "codegen.h"
#include <igraph.h>

/******************************************************************************/
void smxgen_network( inst_net** nets )
{
    inst_net* net;
    inst_rec* rec1;
    inst_rec* rec2;
    igraph_es_t e_sel;
    igraph_eit_t e_it;
    int eid, vid1, vid2;
    // for all scopes in th program
    for( net=*nets; net != NULL; net=net->hh.next ) {
        // for all boxes in the scope
        for( rec1=net->nodes; rec1 != NULL; rec1=rec1->hh.next ) {
            // generate box creation code
            cgen_box_create( net->scope, rec1->id, rec1->name );
        }
        // for all channels in the scope
        e_sel = igraph_ess_all( IGRAPH_EDGEORDER_ID );
        igraph_eit_create( &net->g, e_sel, &e_it );
        while( !IGRAPH_EIT_END( e_it ) ) {
            // generate channel creation code
            eid = IGRAPH_EIT_GET( e_it );
            cgen_channel_create( net->scope, eid );
            // generate connection code for a channel and its connecting boxes
            igraph_edge( &net->g, eid, &vid1, &vid2 );
            rec1 = inst_rec_get( &net->nodes, vid1 );
            cgen_connect( net->scope, eid, vid1, rec1->name,
                    igraph_cattribute_EAS( &net->g, "name", eid ) );
            rec2 = inst_rec_get( &net->nodes, vid2 );
            cgen_connect( net->scope, eid, vid2, rec2->name,
                    igraph_cattribute_EAS( &net->g, "name", eid ) );
            IGRAPH_EIT_NEXT( e_it );
        }
    }
}
