/**
 * Convert streamix box descriptions into SIA graphs
 *
 * @file    smx2sia.c
 * @author  Simon Maurer
 *
 */

#include "smx2sia.h"

/******************************************************************************/
void smx2sia( igraph_t* g, sia_t** smx_symbs, sia_t** desc_symbs )
{
    int vid;
    igraph_vs_t vs;
    igraph_vit_t vit;
    symrec_t* rec;
    virt_net_t* net;
    virt_port_list_t* ports;
    sia_t* sia = NULL;

    vs = igraph_vss_all();
    igraph_vit_create( g, vs, &vit );
    // iterate through all net instances of the graph
    while( !IGRAPH_VIT_END( vit ) ) {
        vid = IGRAPH_VIT_GET( vit );
        rec = ( symrec_t* )( uintptr_t )igraph_cattribute_VAN( g,
                INST_ATTR_SYMB, vid );
        net = ( virt_net_t* )( uintptr_t )igraph_cattribute_VAN( g,
                INST_ATTR_VNET, vid );
        HASH_FIND( hh_desc, *desc_symbs, rec->attr_box->impl_name,
                strlen( rec->attr_box->impl_name ), sia );
        if( sia == NULL ) {
            // no sia defined -> create a new one from box signature
            ports = net->ports;
            /* if( rec->attr_box->attr_pure ) */
            /*     sia = smx2sia_pure( ports, rec->attr_box->impl_name ); */
            /* else */
            sia = smx2sia_state( ports );
        }
        smx2sia_set_name( sia, rec );
        HASH_ADD( hh_smx, *smx_symbs, smx_name, strlen( sia->smx_name ), sia );
        IGRAPH_VIT_NEXT( vit );
    }
    igraph_vit_destroy( &vit );
    igraph_vs_destroy( &vs );

}

/******************************************************************************/
void smx2sia_add_loops( igraph_t* g, virt_port_t* port )
{
    int vid;
    igraph_vs_t vs;
    igraph_vit_t vit;

    vs = igraph_vss_all();
    igraph_vit_create( g, vs, &vit );
    // iterate through all net instances of the graph
    while( !IGRAPH_VIT_END( vit ) ) {
        vid = IGRAPH_VIT_GET( vit );
        smx2sia_add_transition( g, port, vid, vid );
        IGRAPH_VIT_NEXT( vit );
    }
    igraph_vit_destroy( &vit );
    igraph_vs_destroy( &vs );

}

/******************************************************************************/
void smx2sia_add_transition( igraph_t* g, virt_port_t* port, int id_src,
        int id_dst )
{
    const char* mode_port;
    char* name = malloc( strlen( port->name ) + strlen( SIA_PORT_INFIX )
            + CONST_ID_LEN + 1 );
    int id_edge = igraph_ecount( g );
    sprintf( name, "%s%s%d", port->name, SIA_PORT_INFIX, port->edge_id );
    if( port->attr_mode == PORT_MODE_IN )
        mode_port = "?";
    else if( port->attr_mode == PORT_MODE_OUT )
        mode_port = "!";
    igraph_add_edge( g, id_src, id_dst );
    igraph_cattribute_EAS_set( g, "name", id_edge, name );
    igraph_cattribute_EAS_set( g, "mode", id_edge, mode_port );
    free( name );
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
void smx2sia_set_name( sia_t* sia, symrec_t* box )
{
    char* smx_name = malloc( strlen( box->name ) + strlen( SIA_BOX_INFIX )
            + strlen( box->attr_box->impl_name ) + 1 );
    sprintf( smx_name, "%s%s%s", box->name, SIA_BOX_INFIX,
            box->attr_box->impl_name );
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
        if( smx2sia_is_decoupled( ports->port ) ) continue;
        if( ports->next != NULL ) {
            igraph_add_vertices( &sia->g, 1, NULL );
            id_dst = id_src + 1;
        }
        smx2sia_add_transition( &sia->g, ports->port, id_src, id_dst );
        id_src++;
        ports = ports->next;
    }
    id_src = 0;
    ports = ports_rec;
    while( ports != NULL ) {
        if( smx2sia_is_decoupled( ports->port ) )
            smx2sia_add_loops( &sia->g, ports->port );
        id_src++;
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
    HASH_ITER( hh_desc, *desc_symbols, sia, sia_tmp ) {
        HASH_DELETE( hh_desc, *desc_symbols, sia );
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
