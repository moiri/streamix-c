#include "smxgen.h"
#include "codegen.h"
#include <igraph.h>

void smxgen_main( inst_net** nets )
{
    int ident = 0;
    cgen_header_c_file( "todo" );
    cgen_include_local( "smxrts.h" );
    cgen_print( "\n" );
    cgen_main_head();
    ident++;
    cgen_program_init( ident );
    cgen_print( "\n" );
    smxgen_network_create( nets, ident );
    cgen_print( "\n" );
    smxgen_network_run( nets, ident );
    cgen_print( "\n" );
    smxgen_network_wait_end( nets, ident );
    cgen_print( "\n" );
    smxgen_network_destroy( nets, ident );
    cgen_print( "\n" );
    cgen_program_cleanup( ident );
    ident--;
    cgen_function_end( ident );
}

/******************************************************************************/
void smxgen_network_create( inst_net** nets, int ident )
{
    inst_net* net;
    inst_rec* rec1;
    inst_rec* rec2;
    igraph_es_t e_sel;
    igraph_eit_t e_it;
    int eid, vid1, vid2;
    // for all scopes in the program
    for( net=*nets; net != NULL; net=net->hh.next ) {
        // for all boxes in the scope
        for( rec1=net->nodes; rec1 != NULL; rec1=rec1->hh.next ) {
            // generate box creation code
            cgen_box_create( ident, net->scope, rec1->id, rec1->name );
        }
        // for all channels in the scope
        e_sel = igraph_ess_all( IGRAPH_EDGEORDER_ID );
        igraph_eit_create( &net->g, e_sel, &e_it );
        while( !IGRAPH_EIT_END( e_it ) ) {
            // generate channel creation code
            eid = IGRAPH_EIT_GET( e_it );
            cgen_channel_create( ident, net->scope, eid );
            // generate connection code for a channel and its connecting boxes
            igraph_edge( &net->g, eid, &vid1, &vid2 );
            rec1 = inst_rec_get( &net->nodes, vid1 );
            cgen_connect( ident, net->scope, eid, vid1, rec1->name,
                    igraph_cattribute_EAS( &net->g, "label", eid ) );
            rec2 = inst_rec_get( &net->nodes, vid2 );
            cgen_connect( ident, net->scope, eid, vid2, rec2->name,
                    igraph_cattribute_EAS( &net->g, "label", eid ) );
            IGRAPH_EIT_NEXT( e_it );
        }
    }
}

/******************************************************************************/
void smxgen_network_destroy( inst_net** nets, int ident )
{
    inst_net* net;
    inst_rec* rec;
    igraph_es_t e_sel;
    igraph_eit_t e_it;
    int eid;
    // for all scopes in the program
    for( net=*nets; net != NULL; net=net->hh.next ) {
        // for all boxes in the scope
        for( rec=net->nodes; rec != NULL; rec=rec->hh.next ) {
            // generate box destruction code
            cgen_box_destroy( ident, net->scope, rec->id );
        }
        // for all channels in the scope
        e_sel = igraph_ess_all( IGRAPH_EDGEORDER_ID );
        igraph_eit_create( &net->g, e_sel, &e_it );
        while( !IGRAPH_EIT_END( e_it ) ) {
            // generate channel destruction code
            eid = IGRAPH_EIT_GET( e_it );
            cgen_channel_destroy( ident, net->scope, eid );
            IGRAPH_EIT_NEXT( e_it );
        }
    }
}

/******************************************************************************/
void smxgen_network_run( inst_net** nets, int ident )
{
    inst_net* net;
    inst_rec* rec;
    // for all scopes in the program
    for( net=*nets; net != NULL; net=net->hh.next ) {
        // for all boxes in the scope
        for( rec=net->nodes; rec != NULL; rec=rec->hh.next ) {
            // generate box running code
            cgen_box_run( ident, net->scope, rec->id, rec->name );
        }
    }
}

/******************************************************************************/
void smxgen_network_wait_end( inst_net** nets, int ident )
{
    inst_net* net;
    inst_rec* rec;
    // for all scopes in the program
    for( net=*nets; net != NULL; net=net->hh.next ) {
        // for all boxes in the scope
        for( rec=net->nodes; rec != NULL; rec=rec->hh.next ) {
            // generate box waiting code
            cgen_box_wait_end( ident, rec->name );
        }
    }
}
