/**
 * Convert streamix box descriptions into SIA graphs
 *
 * @file    smx2sia.c
 * @author  Simon Maurer
 *
 */

#include "smx2sia.h"
#include "smxerr.h"

/******************************************************************************/
void smx2sia( igraph_t* g, sia_t** smx_symbs, sia_t** desc_symbs )
{
    int vid;
    igraph_vs_t vs;
    igraph_vit_t vit;
    virt_net_t* net;
    virt_port_list_t* ports;
    sia_t* sia;
    const char* impl_name;
    const char* name;

    vs = igraph_vss_all();
    igraph_vit_create( g, vs, &vit );
    // iterate through all net instances of the graph
    while( !IGRAPH_VIT_END( vit ) ) {
        vid = IGRAPH_VIT_GET( vit );
        net = ( virt_net_t* )( uintptr_t )igraph_cattribute_VAN( g,
                INST_ATTR_VNET, vid );
        impl_name = igraph_cattribute_VAS( g, INST_ATTR_FUNC, vid );
        name = igraph_cattribute_VAS( g, INST_ATTR_LABEL, vid );
        ports = net->ports;
        printf( "%s_%s_%d\n", name, impl_name, vid );
        if( net->type == VNET_SYNC ) {
            sia = smx2sia_cpsync( g, vid );
        }
        else if( net->type == VNET_BOX ) {
            HASH_FIND_STR( *desc_symbs, impl_name, sia );
            if( sia == NULL ) {
                // no sia defined -> create a new one from box signature
                /* if( rec->attr_box->attr_pure ) */
                /*     sia = smx2sia_pure( ports, rec->attr_box->impl_name ); */
                /* else */
                sia = smx2sia_state( ports );
            }
            else {
                smx2sia_update( &sia->g, ports );
            }
        }
        else printf( "bad vertex type\n" );
        smx2sia_set_name_box( sia, name, impl_name, vid );
        HASH_ADD( hh_smx, *smx_symbs, smx_name, strlen( sia->smx_name ), sia );
        IGRAPH_VIT_NEXT( vit );
    }
    igraph_vit_destroy( &vit );
    igraph_vs_destroy( &vs );

}

/******************************************************************************/
void smx2sia_add_edge( igraph_t* g, int id_src, int id_dst, int smx_id,
        const char* name, const char* mode )
{
    int id_edge = igraph_ecount( g );
    igraph_add_edge( g, id_src, id_dst );
    igraph_cattribute_EAN_set( g, G_SIA_NAME, id_edge, smx_id );
    igraph_cattribute_EAS_set( g, G_SIA_PNAME, id_edge, name );
    igraph_cattribute_EAS_set( g, G_SIA_MODE, id_edge, mode );
}

/******************************************************************************/
void smx2sia_add_loops( igraph_t* g, virt_port_t* port )
{
    int vid, eid, smx_id;
    bool match;
    igraph_vs_t vs;
    igraph_vit_t vit;
    igraph_es_t e_sel;
    igraph_eit_t e_it;

    vs = igraph_vss_all();
    igraph_vit_create( g, vs, &vit );
    // iterate through all net instances of the graph
    while( !IGRAPH_VIT_END( vit ) ) {
        vid = IGRAPH_VIT_GET( vit );
        igraph_es_incident( &e_sel, vid, IGRAPH_OUT );
        igraph_eit_create( g, e_sel, &e_it );
        // check if an outgoing edge with the same name alrady exists
        match = false;
        while( !IGRAPH_EIT_END( e_it ) ) {
            eid = IGRAPH_EIT_GET( e_it );
            smx_id = igraph_cattribute_EAN( g, G_SIA_NAME, eid );
            if( smx_id == port->edge_id ) match = true;
            IGRAPH_EIT_NEXT( e_it );
        }
        // don't add a loop if a transition with the same action already exists
        if( !match ) smx2sia_add_transition( g, port, vid, vid );
        IGRAPH_VIT_NEXT( vit );
    }
    igraph_vit_destroy( &vit );
    igraph_vs_destroy( &vs );
    igraph_eit_destroy( &e_it );
    igraph_es_destroy( &e_sel );

}

/******************************************************************************/
void smx2sia_add_transition( igraph_t* g, virt_port_t* port, int id_src,
        int id_dst )
{
    const char* mode;
    if( port->attr_mode == PORT_MODE_IN )
        mode = G_SIA_MODE_IN;
    else if( port->attr_mode == PORT_MODE_OUT )
        mode = G_SIA_MODE_OUT;
    smx2sia_add_edge( g, id_src, id_dst, port->edge_id, port->name, mode );
}

/******************************************************************************/
sia_t* smx2sia_cpsync( igraph_t* nw, int vid )
{
    int eid, smx_id, id_dst, id_src = 1;
    igraph_es_t e_sel;
    igraph_eit_t e_it;
    const char* name;
    sia_t* sia = sia_create( NULL, NULL );
    igraph_add_vertices( &sia->g, 2, NULL );

    // handle all input actions
    igraph_es_incident( &e_sel, vid, IGRAPH_IN );
    igraph_eit_create( nw, e_sel, &e_it );
    while( !IGRAPH_EIT_END( e_it ) ) {
        eid = IGRAPH_EIT_GET( e_it );
        smx_id = igraph_cattribute_EAN( nw, PORT_ATTR_SIA, eid );
        name = igraph_cattribute_EAS( nw, PORT_ATTR_LABEL, eid );
        smx2sia_add_edge( &sia->g, 0, 1, smx_id, name, G_SIA_MODE_IN );
        IGRAPH_EIT_NEXT( e_it );
    }

    // handle all output actions
    igraph_es_incident( &e_sel, vid, IGRAPH_OUT );
    igraph_eit_create( nw, e_sel, &e_it );
    igraph_add_vertices( &sia->g, IGRAPH_EIT_SIZE( e_it ) - 1, NULL );
    while( !IGRAPH_EIT_END( e_it ) ) {
        eid = IGRAPH_EIT_GET( e_it );
        smx_id = igraph_cattribute_EAN( nw, PORT_ATTR_SIA, eid );
        name = igraph_cattribute_EAS( nw, PORT_ATTR_LABEL, eid );
        id_dst = id_src + 1;
        if( id_dst == igraph_vcount( &sia->g ) ) id_dst = 0;
        smx2sia_add_edge( &sia->g, id_src, id_dst, smx_id, name,
                G_SIA_MODE_OUT );
        id_src++;
        IGRAPH_EIT_NEXT( e_it );
    }
    igraph_eit_destroy( &e_it );
    igraph_es_destroy( &e_sel );
    return sia;
}

/******************************************************************************/
bool smx2sia_is_decoupled( virt_port_t* port )
{
    if( port->symb == NULL )
        // port belongs to a synchronizer
        return false;
    else if( port->symb->attr_port->decoupled )
        // port is decoupled
        return true;
    else if( ( port->attr_class == PORT_CLASS_SIDE )
            && ( port->attr_mode == PORT_MODE_OUT ) )
        // side port and output is automatically decoupled
        return true;
    return false;
}

/******************************************************************************/
void smx2sia_set_name_box( sia_t* sia, const char* box_name,
        const char* impl_name, int id )
{
    char* smx_name = malloc( strlen( box_name ) + 2 * strlen( SIA_BOX_INFIX )
            + strlen( impl_name ) + CONST_ID_LEN + 1 );
    sprintf( smx_name, "%s%s%s%s%d", box_name, SIA_BOX_INFIX, impl_name,
            SIA_BOX_INFIX, id );
    sia->smx_name = smx_name;
}

/******************************************************************************/
sia_t* smx2sia_state( virt_port_list_t* ports_rec )
{
    virt_port_list_t* ports = ports_rec;
    int id_dst, id_src = 0;
    sia_t* sia = sia_create( NULL, NULL );
    igraph_add_vertices( &sia->g, 1, NULL );
    while( ports != NULL ) {
        id_dst = 0;
        if( !smx2sia_is_decoupled( ports->port ) ) {
            if( ports->next != NULL ) {
                igraph_add_vertices( &sia->g, 1, NULL );
                id_dst = id_src + 1;
            }
            smx2sia_add_transition( &sia->g, ports->port, id_src, id_dst );
            id_src++;
        }
        ports = ports->next;
    }
    ports = ports_rec;
    while( ports != NULL ) {
        if( smx2sia_is_decoupled( ports->port ) )
            smx2sia_add_loops( &sia->g, ports->port );
        ports = ports->next;
    }
    return sia;
}

/******************************************************************************/
void smx2sia_sias_destroy( sias_t* sias, sia_t** desc_symbols,
        sia_t** smx_symbols )
{
    sia_t* sia;
    sia_t* sia_tmp;
    HASH_ITER( hh_smx, *smx_symbols, sia, sia_tmp ) {
        HASH_DELETE( hh_smx, *smx_symbols, sia );
        sia_destroy( sia );
    }
    HASH_ITER( hh, *desc_symbols, sia, sia_tmp ) {
        HASH_DEL( *desc_symbols, sia );
    }
    sias_destroy_list( sias );
}

/******************************************************************************/
void smx2sia_sias_write( sia_t** symbols, const char* out_path,
        const char* format )
{
    sia_t* sia;
    sia_t* tmp;

    HASH_ITER( hh_smx, *symbols, sia, tmp ) {
        sia_write( sia, sia->smx_name, out_path, format );
    }
}

/******************************************************************************/
void smx2sia_update( igraph_t* g, virt_port_list_t* ports_rec )
{
    int eid;
    igraph_es_t es;
    igraph_eit_t eit;
    const char* name;
    char error_msg[ CONST_ERROR_LEN ];
    virt_port_list_t* ports;
    bool match;

    es = igraph_ess_all( IGRAPH_EDGEORDER_ID );
    igraph_eit_create( g, es, &eit );
    // remove name attr to add the smx name
    igraph_cattribute_remove_e( g, G_SIA_NAME );
    // iterate through all net instances of the graph
    while( !IGRAPH_EIT_END( eit ) ) {
        eid = IGRAPH_EIT_GET( eit );
        name = igraph_cattribute_EAS( g, G_SIA_PNAME, eid );
        match = false;
        ports = ports_rec;
        // search for a matching port in the signature
        while( ports != NULL ) {
            if( strcmp( name, ports->port->name ) == 0 ) {
                igraph_cattribute_EAN_set( g, G_SIA_NAME, eid,
                        ports->port->edge_id );
                match = true;
            }
            ports = ports->next;
        }
        if( !match ) {
            sprintf( error_msg, ERROR_BAD_SIA_PORT, ERR_ERROR, name );
            report_yyerror( error_msg, 0 );
        }
        IGRAPH_EIT_NEXT( eit );
    }
    igraph_eit_destroy( &eit );
    igraph_es_destroy( &es );

    // add self loops for decoupled ports
    ports = ports_rec;
    // search for a matching port in the signature
    while( ports != NULL ) {
        if( smx2sia_is_decoupled( ports->port ) )
            smx2sia_add_loops( g, ports->port );
        ports = ports->next;
    }
}
