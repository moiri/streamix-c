/**
 * Convert streamix box descriptions into SIA graphs
 *
 * @file    smx2sia.c
 * @author  Simon Maurer
 *
 */

#include "smx2sia.h"

/******************************************************************************/
void smx2sia( igraph_t* g, sia_t** symbols )
{
    int vid;
    igraph_vs_t vs;
    igraph_vit_t vit;
    symrec_t* rec;
    symrec_list_t* ports;
    sia_t* sia = NULL;

    vs = igraph_vss_all();
    igraph_vit_create( g, vs, &vit );
    // iterate through all net instances of the graph
    while( !IGRAPH_VIT_END( vit ) ) {
        vid = IGRAPH_VIT_GET( vit );
        rec = ( symrec_t* )( uintptr_t )igraph_cattribute_VAN( g,
                INST_ATTR_SYMB, vid );
        HASH_FIND_STR( *symbols, rec->attr_box->impl_name, sia );
        if( sia == NULL ) {
            // no sia defined -> create a new one from box signature
            ports = rec->attr_box->ports;
            /* if( rec->attr_box->attr_pure ) */
            /*     sia = smx2sia_pure( ports, rec->attr_box->impl_name ); */
            /* else */
                sia = smx2sia_state( ports, rec->attr_box->impl_name );
            HASH_ADD_STR( *symbols, name, sia );
        }
        IGRAPH_VIT_NEXT( vit );
    }
    igraph_vit_destroy( &vit );
    igraph_vs_destroy( &vs );

}

/******************************************************************************/
void smx2sia_add_loops( igraph_t* g, symrec_t* port )
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
void smx2sia_add_transition( igraph_t* g, symrec_t* port, int id_src,
        int id_dst )
{
    char* name_port_cp;
    char* mode_port;
    int id_edge = igraph_ecount( g );
    name_port_cp = malloc( strlen( port->name ) + 1 );
    strcpy( name_port_cp, port->name );
    mode_port = malloc( 2 );
    if( port->attr_port->mode == PORT_MODE_IN )
        mode_port = "?";
    else if( port->attr_port->mode == PORT_MODE_OUT )
        mode_port = "!";
    igraph_add_edge( g, id_src, id_dst );
    igraph_cattribute_EAS_set( g, "name", id_edge, name_port_cp );
    igraph_cattribute_EAS_set( g, "mode", id_edge, mode_port );

}

/******************************************************************************/
sia_t* smx2sia_state( symrec_list_t* ports_rec, const char* name )
{
    char* name_cp = malloc( strlen( name ) + 1 );
    symrec_list_t* ports = ports_rec;
    int id_dst, id_src = 0;
    strcpy( name_cp, name );
    sia_t* sia = sia_create( name_cp, NULL );
#if defined(DEBUG) || defined(DEBUG_SIA)
    printf( "SIA '%s' not yet defined, adding\n", sia->name );
#endif // DEBUG_SIA
    igraph_add_vertices( &sia->g, 1, NULL );
    while( ports != NULL ) {
        id_dst = 0;
        if( ports->rec->attr_port->decoupled ) continue;
        if( ports->next != NULL ) {
            igraph_add_vertices( &sia->g, 1, NULL );
            id_dst = id_src + 1;
        }
        smx2sia_add_transition( &sia->g, ports->rec, id_src, id_dst );
        id_src++;
        ports = ports->next;
    }
    id_src = 0;
    ports = ports_rec;
    while( ports != NULL ) {
        if( ports->rec->attr_port->decoupled ) {
            smx2sia_add_loops( &sia->g, ports->rec );
        }
        id_src++;
        ports = ports->next;
    }
    return sia;
}
