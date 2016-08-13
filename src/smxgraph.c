/**
 * Small library to handle the dependancy graph according to the semantics of
 * streamix
 *
 * @file    smxgraph.c
 * @author  Simon Maurer
 *
 */

#include "smxgraph.h"
#include "defines.h"
#include "insttab.h"
#include "context.h"

/******************************************************************************/
int dgraph_find_bp_port( igraph_vector_ptr_t* bp_symbs, const char* name )
{
    int i;
    const char* symb_name;
    for( i = 0; i < igraph_vector_ptr_size( bp_symbs ); i++ ) {
        symb_name = ( ( symrec_t* )VECTOR( *bp_symbs )[i] )->name;
        if( strcmp( name, symb_name ) == 0 ) return i;
    }
    return -1;
}

/******************************************************************************/
virt_port_list_t* dgraph_merge_port_wrap( igraph_t* g, symrec_list_t* sps_src,
        virt_port_list_t* vps_net )
{
    virt_port_t* vp_new = NULL;
    virt_port_t* vp_net = NULL;
    virt_port_list_t* vps_last = NULL;
    virt_port_list_t* vps_new = NULL;
    symrec_list_t* sps_int = NULL;
    symrec_list_t* sps_bak = sps_src;
    int idx = 0, count = 0;
    igraph_vector_ptr_t bp_vnets;
    igraph_vector_ptr_t bp_symbs;
    virt_net_t* bp_vnet = NULL;
    int bp_idx = 0;

    igraph_vector_ptr_init( &bp_vnets, 0 );
    igraph_vector_ptr_init( &bp_symbs, 0 );
    sps_src = sps_bak;
    while( sps_src != NULL ) {
        sps_int = sps_src->rec->attr_port->ports_int;
        // if there are internal ports, copy them to the new virtual port
        // list use the alt name of the port but all other info from the
        // net port
        count = 0;
        while( sps_int != NULL ) {
            sps_int = sps_int->next;
            count++;
        }
        if( count == 1 ) {
            sps_int = sps_src->rec->attr_port->ports_int;
            // search for the port in the virtual net of the connection
            vp_net = virt_port_get_equivalent_by_name( vps_net,
                    sps_int->rec->name );
            if( vp_net == NULL ) {
                // it is a bypass port
                vp_new = virt_port_create( PORT_CLASS_NONE, PORT_MODE_BI,
                        NULL, sps_src->rec->name, NULL );
                bp_idx = dgraph_find_bp_port( &bp_symbs, sps_int->rec->name );
                if( bp_idx < 0 ) {
                    // create a copy synchronizer
                    bp_vnet = dgraph_vertex_add_sync( g, vp_new );
                    vp_new->symb = symrec_create( sps_int->rec->name, 0,
                            SYMREC_PORT, 0, 0 );
                    igraph_cattribute_VAN_set( g, INST_ATTR_SYMB,
                            bp_vnet->inst->id, ( uintptr_t )vp_new->symb );
                    igraph_vector_ptr_push_back( &bp_vnets, bp_vnet );
                    igraph_vector_ptr_push_back( &bp_symbs, vp_new->symb );
                }
                else {
                    vp_new->v_net = VECTOR( bp_vnets )[ bp_idx ];
                    vp_new->symb = VECTOR( bp_symbs )[ bp_idx ];
                }
                vps_new = malloc( sizeof( virt_port_list_t ) );
                vps_new->port = vp_new;
                vps_new->next = vps_last;
                vps_new->idx = idx;
                vps_last = vps_new;
                idx++;
            }
            else {
                // it is not a bypass
                vps_new = malloc( sizeof( virt_port_list_t ) );
                vp_new = virt_port_create( sps_src->rec->attr_port->collection,
                        sps_src->rec->attr_port->mode, vp_net->v_net,
                        sps_src->rec->name, vp_net->symb );
                // unknown direction, ignore class, modes have to be equal
                check_connection( vp_new, vp_net, g, false, true, true );
                /* vp_net->state = VPORT_STATE_CONNECTED; */
                vps_new->port = vp_new;
                vps_new->next = vps_last;
                vps_new->idx = idx;
                vps_last = vps_new;
                idx++;
            }
        }
        else if( count > 1 ) {
            sps_int = sps_src->rec->attr_port->ports_int;
            vp_net = virt_port_get_equivalent_by_name( vps_net,
                    sps_int->rec->name );
            // create a copy synchronizer
            vp_new = virt_port_create( sps_src->rec->attr_port->collection,
                    sps_src->rec->attr_port->mode, NULL, sps_src->rec->name,
                    vp_net->symb );
            dgraph_vertex_add_sync( g, vp_new );
            vps_new = malloc( sizeof( virt_port_list_t ) );
            vps_new->port = vp_new;
            vps_new->next = vps_last;
            vps_new->idx = idx;
            vps_last = vps_new;
            idx++;
            while( sps_int != NULL ) {
                // search for the port in the virtual net of the connection
                vp_net = virt_port_get_equivalent_by_name( vps_net,
                        sps_int->rec->name );
                // unknown direction, ignore class, modes have to be equal
                check_connection( vp_new, vp_net, g, false, true, true );
                sps_int = sps_int->next;
                /* vp_net->state = VPORT_STATE_CONNECTED; */
            }
        }
        else {
            // search for the port in the virtual net of the connection
            vp_net = virt_port_get_equivalent_by_name( vps_net,
                    sps_src->rec->name );
            // there was no internal port, copy the regular port to the new list
            vps_new = malloc( sizeof( virt_port_list_t ) );
            vp_new = virt_port_create( vp_net->attr_class, vp_net->attr_mode,
                    vp_net->v_net, vp_net->name, vp_net->symb );
            vps_new->port = vp_new;
            vps_new->next = vps_last;
            vps_new->idx = idx;
            vps_last = vps_new;
            idx++;
        }
        sps_src = sps_src->next;
    }
    return vps_last;
}

/******************************************************************************/
virt_port_list_t* dgraph_merge_port_net( igraph_t* g, symrec_list_t* sps_src,
        virt_port_list_t* vps_net )
{
    virt_port_t* vp_new = NULL;
    virt_port_t* vp_net = NULL;
    virt_port_list_t* vps_last = NULL;
    virt_port_list_t* vps_new = NULL;
    symrec_list_t* sps_int = NULL;
    int idx = 0, count = 0;

    while( sps_src != NULL  ) {
        // search for the port in the virtual net of the connection
        vp_net = virt_port_get_equivalent_by_name( vps_net,
                sps_src->rec->name );
        sps_int = sps_src->rec->attr_port->ports_int;
        // if there are internal ports, copy them to the new virtual port
        // list use the alt name of the port but all other info from the
        // net port
        count = 0;
        while( sps_int != NULL ) {
            sps_int = sps_int->next;
            count++;
        }
        if( count == 1 ) {
            sps_int = sps_src->rec->attr_port->ports_int;
            vps_new = malloc( sizeof( virt_port_list_t ) );
            vp_net->name = sps_int->rec->name;
            vps_new->port = vp_net;
            vps_new->next = vps_last;
            vps_new->idx = idx;
            vps_last = vps_new;
            idx++;
        }
        else if( count > 1 ) {
            // create a copy synchronizer
            vp_new = virt_port_create( vp_net->attr_class,
                    vp_net->attr_mode, vp_net->v_net, vp_net->name,
                    vp_net->symb );
            dgraph_vertex_add_sync( g, vp_new );
            // unknown direction, ignore class, modes have to be equal
            check_connection( vp_new, vp_net, g, false, true, true );
            vp_net = vp_new;
            sps_int = sps_src->rec->attr_port->ports_int;
            while( sps_int != NULL ) {
                vps_new = malloc( sizeof( virt_port_list_t ) );
                vp_new = virt_port_create( vp_net->attr_class,
                        vp_net->attr_mode, vp_net->v_net, sps_int->rec->name,
                        vp_net->symb );
                /* vp_net->state = VPORT_STATE_CONNECTED; */
                vps_new->port = vp_new;
                vps_new->next = vps_last;
                vps_new->idx = idx;
                vps_last = vps_new;
                idx++;
                sps_int = sps_int->next;
            }
        }
        else {
            // there was no internal port, copy the regula port to the new list
            vps_new = malloc( sizeof( virt_port_list_t ) );
            vps_new->port = vp_net;
            vps_new->next = vps_last;
            vps_new->idx = idx;
            vps_last = vps_new;
            idx++;
        }
        sps_src = sps_src->next;
    }
    return vps_last;
}

/******************************************************************************/
void dgraph_append( igraph_t* g, igraph_t* g_tpl, bool deep )
{
    const char* name;
    igraph_es_t es;
    igraph_vs_t vs;
    igraph_eit_t eit;
    igraph_vit_t vit;
    instrec_t** inst_map;
    virt_port_t *p_src, *p_dest;
    int id_inst, id_edge, id_from, id_to;
    // all vertexes are boxes - copy all vertices to the new graph
    inst_map = malloc( igraph_vcount( g_tpl ) * sizeof( instrec_t* ) );
    vs = igraph_vss_all();
    igraph_vit_create( g_tpl, vs, &vit );
    while( !IGRAPH_VIT_END( vit ) ) {
        id_inst = IGRAPH_VIT_GET( vit );
        inst_map[ id_inst ] = dgraph_vertex_copy( g_tpl, g, id_inst, deep );
        IGRAPH_VIT_NEXT( vit );
    }
    igraph_vit_destroy( &vit );
    igraph_vs_destroy( &vs );
    // copy all edges to the new graph
    es = igraph_ess_all( IGRAPH_EDGEORDER_ID );
    igraph_eit_create( g_tpl, es, &eit );
    while( !IGRAPH_EIT_END( eit ) ) {
        id_edge = IGRAPH_EIT_GET( eit );
        igraph_edge( g_tpl, IGRAPH_EIT_GET( eit ), &id_from, &id_to );
        p_src = dgraph_port_search_neighbour( g_tpl, g, id_edge,
                inst_map[ id_from ]->id, PORT_ATTR_PSRC );
        p_dest = dgraph_port_search_neighbour( g_tpl, g, id_edge,
                inst_map[ id_to ]->id, PORT_ATTR_PDST );
        name = p_src->name;
        if( p_dest->v_net->inst->type != INSTREC_SYNC ) name = p_dest->name;
        dgraph_edge_add( g, p_src, p_dest, name );
        IGRAPH_EIT_NEXT( eit );
    }
    igraph_eit_destroy( &eit );
    igraph_es_destroy( &es );
    free( inst_map );
}

/******************************************************************************/
int dgraph_edge_add( igraph_t* g, virt_port_t* p_src, virt_port_t* p_dest,
        const char* name )
{
    int id = igraph_ecount( g );
#if defined(DEBUG) || defined(DEBUG_CONNECT_GRAPH)
    printf( " add new edge %s(%d->%d)\n", name, p_src->v_net->inst->id,
            p_dest->v_net->inst->id );
#endif // DEBUG
    igraph_add_edge( g, p_src->v_net->inst->id, p_dest->v_net->inst->id );
    igraph_cattribute_EAS_set( g, PORT_ATTR_LABEL, id, name );
    igraph_cattribute_EAN_set( g, PORT_ATTR_PSRC, id, ( uintptr_t )p_src );
    igraph_cattribute_EAN_set( g, PORT_ATTR_PDST, id, ( uintptr_t )p_dest );
    return id;
}

/******************************************************************************/
void dgraph_destroy_attr( igraph_t* g )
{
    igraph_vs_t vs;
    igraph_vit_t vit;

    vs = igraph_vss_all();
    igraph_vit_create( g, vs, &vit );
    // iterate through all net instances of the graph
    while( !IGRAPH_VIT_END( vit ) ) {
        dgraph_vertex_destroy_attr( g, IGRAPH_VIT_GET( vit ), true );
        IGRAPH_VIT_NEXT( vit );
    }
    igraph_vit_destroy( &vit );
    igraph_vs_destroy( &vs );

    dgraph_destroy_attr_v( g, INST_ATTR_SYMB );
    dgraph_destroy_attr_v( g, INST_ATTR_GRAPH );
    dgraph_destroy_attr_v( g, INST_ATTR_VNET );
    dgraph_destroy_attr_e( g, PORT_ATTR_PDST );
    dgraph_destroy_attr_e( g, PORT_ATTR_PSRC );
}

/******************************************************************************/
void dgraph_destroy_attr_e( igraph_t* g, const char* attr )
{
    if( igraph_cattribute_has_attr( g, IGRAPH_ATTRIBUTE_EDGE, attr ) )
        igraph_cattribute_remove_e( g, attr );
}

/******************************************************************************/
void dgraph_destroy_attr_v( igraph_t* g, const char* attr )
{
    if( igraph_cattribute_has_attr( g, IGRAPH_ATTRIBUTE_VERTEX, attr ) )
        igraph_cattribute_remove_v( g, attr );
}

/******************************************************************************/
void dgraph_flatten( igraph_t* g_new, igraph_t* g )
{
    igraph_vs_t vs;
    igraph_vit_t vit;
    virt_net_t* v_net;
    symrec_t* symb;
    igraph_t g_child, g_in, *g_tmp;
    int inst_id;

    igraph_copy( &g_in, g );
    vs = igraph_vss_all();
    igraph_vit_create( &g_in, vs, &vit );
    // iterate through all net instances of the graph
    while( !IGRAPH_VIT_END( vit ) ) {
        inst_id = IGRAPH_VIT_GET( vit );
        symb = ( symrec_t* )( uintptr_t )igraph_cattribute_VAN( &g_in,
                INST_ATTR_SYMB, inst_id );
        v_net = ( virt_net_t* )( uintptr_t )igraph_cattribute_VAN( &g_in,
                INST_ATTR_VNET, inst_id );
        if( ( v_net->type == VNET_NET ) || ( v_net->type == VNET_WRAP ) ) {
        /* if( inst->type == INSTREC_NET ) { */
#if defined(DEBUG) || defined(DEBUG_FLATTEN_GRAPH)
            printf( "\nFlatten instance '%s(%d)' start\n", v_net->inst->name,
                    v_net->inst->id );
#endif // DEBUG_FLATTEN_GRAPH
            g_tmp = ( igraph_t* )( uintptr_t )igraph_cattribute_VAN( &g_in,
                    INST_ATTR_GRAPH, inst_id );
            // deep copy child graph to create new instances
            igraph_empty( &g_child, 0, IGRAPH_DIRECTED );
            dgraph_append( &g_child, g_tmp, true );
            // recoursively flatten further net instances
            dgraph_flatten( g, &g_child );
            dgraph_flatten_net( g, &g_child, symb, v_net );
#if defined(DEBUG) || defined(DEBUG_FLATTEN_GRAPH)
            printf( "Flatten instance end\n\n" );
#endif // DEBUG_FLATTEN_GRAPH
            igraph_destroy( &g_child );
        }
        IGRAPH_VIT_NEXT( vit );
    }
    igraph_vit_destroy( &vit );
    igraph_vs_destroy( &vs );
    dgraph_append( g_new, g, false );
    igraph_destroy( &g_in );
#if defined(DEBUG) || defined(DEBUG_FLATTEN_GRAPH)
    igraph_write_graph_dot( g_new, stdout );
#endif // DEBUG_FLATTEN_GRAPH
}

/******************************************************************************/
void dgraph_flatten_net( igraph_t* g_new, igraph_t* g_child, symrec_t* symb,
        virt_net_t* v_net )
{
    virt_port_t *p_src, *p_dest, *port, *port_net, *port_net_new;
    igraph_vs_t vs;
    igraph_es_t es;
    igraph_eit_t eit;
    int net_id = v_net->inst->id;

    // get all ports connecting to the net
    igraph_es_incident( &es, net_id, IGRAPH_ALL );
    igraph_eit_create( g_new, es, &eit );
    // for each edge connect to the actual nets form the graph
    while( !IGRAPH_EIT_END( eit ) ) {
        p_src = ( virt_port_t* )( uintptr_t ) igraph_cattribute_EAN( g_new,
                PORT_ATTR_PSRC, IGRAPH_EIT_GET( eit ) );
        p_dest = ( virt_port_t* )( uintptr_t ) igraph_cattribute_EAN( g_new,
                PORT_ATTR_PDST, IGRAPH_EIT_GET( eit ) );
        // get id of the non net end of the edge
        port = p_src;
        port_net = p_dest;
        if( p_dest->v_net->inst->id != net_id ) {
            port = p_dest;
            port_net = p_src;
        }
        if( v_net->type == VNET_NET ) {
            // get open port with same symbol pointer from child graph
            port_net_new = dgraph_port_search_child( g_child, port_net, true );
        }
        else if( v_net->type == VNET_WRAP ) {
            // get port from net of the wrapper
            port_net = dgraph_port_search_wrap( symb->attr_wrap->v_net,
                    port_net );
            // get open port with same symbol pointer from child graph
            port_net_new = dgraph_port_search_child( g_child, port_net, false );
            port_net_new->name = port_net->name;
        }
        // connect this port to the matching port of the virtual net
        // unknown direction, class matters, modes have to be different
        check_connection( port_net_new, port, g_new, false, false, false );
        check_connection_cp( NULL, port_net_new, port, g_new, false, false );
        IGRAPH_EIT_NEXT( eit );
    }
    igraph_eit_destroy( &eit );
    igraph_es_destroy( &es );
#if defined(DEBUG) || defined(DEBUG_FLATTEN_GRAPH)
    printf( "dgraph_flatten_net: remove vertice with id = %d\n", net_id );
#endif // DEBUG_FLATTEN_GRAPH
    dgraph_vertex_destroy_attr( g_new, net_id, true );
    vs = igraph_vss_1( net_id );
    igraph_delete_vertices( g_new, vs );
    igraph_vs_destroy( &vs );
    dgraph_vertex_update_ids( g_new, net_id );
}

/******************************************************************************/
virt_port_t* dgraph_port_search_neighbour( igraph_t* g, igraph_t* g_new,
        int id_edge, int id_inst, const char* attr )
{
    virt_net_t* v_net = NULL;
    virt_port_t* port = ( virt_port_t* )( uintptr_t )igraph_cattribute_EAN( g,
            attr, id_edge );
    v_net = ( virt_net_t* )( uintptr_t )igraph_cattribute_VAN( g_new,
            INST_ATTR_VNET, id_inst );
#if defined(DEBUG) || defined(DEBUG_SEARCH_PORT)
    printf( "dgraph_port_search_neighbour: Search port " );
    debug_print_vport( port );
    printf( "\n in virtual net: " );
    debug_print_vports( v_net );
#endif // DEBUG
    return virt_port_get_equivalent( v_net, port, true );
}

/******************************************************************************/
virt_port_t* dgraph_port_search_child( igraph_t* g, virt_port_t* port,
        bool clean )
{
    virt_port_t *port_res = NULL, *port_inst = NULL;
    virt_net_t* v_net;
    igraph_vs_t vs;
    igraph_vit_t vit;
    int id_inst;
#if defined(DEBUG) || defined(DEBUG_SEARCH_PORT_CHILD)
    printf( "dgrap_port_search_child: Search port " );
    debug_print_vport( port );
    printf( "\n" );
#endif // DEBUG
    vs = igraph_vss_all();
    igraph_vit_create( g, vs, &vit );
    while( !IGRAPH_VIT_END( vit ) ) {
        id_inst = IGRAPH_VIT_GET( vit );
        v_net = ( virt_net_t* )( uintptr_t )igraph_cattribute_VAN( g,
                INST_ATTR_VNET, id_inst );
#if defined(DEBUG) || defined(DEBUG_SEARCH_PORT_CHILD)
        printf( " in virtual net: " );
        debug_print_vports( v_net );
#endif // DEBUG
        port_inst = virt_port_get_equivalent( v_net, port, false );
        if( port_inst != NULL ) {
            port_res = port_inst;
            if( port_inst->v_net->inst->type == INSTREC_SYNC ) {
                if( clean ) {
                    // remove the cp sync to not search for it a second time
                    vs = igraph_vss_1( id_inst );
                    igraph_delete_vertices( g, vs );
                    igraph_vs_destroy( &vs );
                }
                break;
            }
        }
        IGRAPH_VIT_NEXT( vit );
    }
    igraph_vit_destroy( &vit );
    igraph_vs_destroy( &vs );

    return port_res;
}

/******************************************************************************/
virt_port_t* dgraph_port_search_wrap( virt_net_t* v_net, virt_port_t* port )
{
    // => find a namesake in the net interface of the wrapper
#if defined(DEBUG) || defined(DEBUG_SEARCH_PORT_WRAP)
    printf( "dgrap_port_search_wrap: Search port " );
    debug_print_vport( port );
    printf( "\n in virtual net: " );
    debug_print_vports( v_net );
#endif // DEBUG
    virt_port_t* port_net = NULL;
    virt_port_list_t* ports = v_net->ports;
    while( ports != NULL ) {
        if( are_port_names_ok( ports->port, port )
                && are_port_modes_ok( ports->port, port, true ) ) {
            port_net = ports->port;
#if defined(DEBUG) || defined(DEBUG_SEARCH_PORT_WRAP)
            printf( "Found port: " );
            debug_print_vport( ports->port  );
            printf( "\n" );
#endif // DEBUG
            if( port_net->v_net->inst->type == INSTREC_SYNC ) break;
        }
        ports = ports->next;
    }
    return port_net;
}

/******************************************************************************/
int dgraph_vertex_add( igraph_t* g, const char* name )
{
    int id = igraph_vcount( g );
    igraph_add_vertices( g, 1, NULL );
    igraph_cattribute_VAS_set( g, INST_ATTR_LABEL, id, name );
#if defined(DEBUG) || defined(DEBUG_CONNECT_GRAPH)
    printf( "dgraph_vertex_add: '%s(%d)'\n", name, id );
#endif // DEBUG
    return id;
}

/******************************************************************************/
void dgraph_vertex_add_attr( igraph_t* g, int id, const char* func,
        symrec_t* symb, virt_net_t* v_net, igraph_t* g_net )
{
    const char* f_name = TEXT_NULL;
    if( func != NULL ) f_name = func;
    igraph_cattribute_VAS_set( g, INST_ATTR_FUNC, id, f_name );
    igraph_cattribute_VAN_set( g, INST_ATTR_SYMB, id, ( uintptr_t )symb );
    igraph_cattribute_VAN_set( g, INST_ATTR_VNET, id, ( uintptr_t )v_net );
    igraph_cattribute_VAN_set( g, INST_ATTR_GRAPH, id, ( uintptr_t )g_net );
}

/******************************************************************************/
virt_net_t* dgraph_vertex_add_box( igraph_t* g, symrec_t* symb, int line )
{
    int id = dgraph_vertex_add( g, symb->name );
    instrec_t* inst = instrec_create( symb->name, id, line, INSTREC_BOX );
    virt_net_t* v_net = virt_net_create_box( symb, inst );
    dgraph_vertex_add_attr( g, id, symb->attr_box->impl_name, symb, v_net,
            NULL );
    return v_net;
}

/******************************************************************************/
virt_net_t* dgraph_vertex_add_net( igraph_t* g, symrec_t* symb, int line )
{
    int id = dgraph_vertex_add( g, symb->name );
    instrec_t* inst = instrec_create( symb->name, id, line, INSTREC_NET );
    virt_net_t* v_net = virt_net_create_net( symb->attr_net->v_net, inst );
    dgraph_vertex_add_attr( g, id, NULL, symb, v_net, &symb->attr_net->g );
    return v_net;
}

/******************************************************************************/
virt_net_t* dgraph_vertex_add_sync( igraph_t* g, virt_port_t* port )
{
    int id = dgraph_vertex_add( g, TEXT_CP );
    instrec_t* inst = instrec_create( TEXT_CP, id, -1, INSTREC_SYNC );
    virt_net_t* v_net = virt_net_create_sync( inst, port );
    dgraph_vertex_add_attr( g, id, NULL, NULL, v_net, NULL );
    return v_net;
}

/******************************************************************************/
virt_net_t* dgraph_vertex_add_wrap( igraph_t* g, symrec_t* symb, int line )
{
    int id = dgraph_vertex_add( g, symb->name );
    instrec_t* inst = instrec_create( symb->name, id, line, INSTREC_WRAP );
    virt_net_t* v_net = virt_net_create_wrap( symb, inst );
    dgraph_vertex_add_attr( g, id, NULL, symb, v_net, &symb->attr_wrap->g );
    return v_net;
}

/******************************************************************************/
instrec_t* dgraph_vertex_copy( igraph_t* g_src, igraph_t* g_dest, int id,
        bool deep )
{
    int new_id;
    virt_net_t *v_net;
    instrec_t *inst;
    // get old inst
    v_net = ( virt_net_t* )( uintptr_t ) igraph_cattribute_VAN( g_src,
            INST_ATTR_VNET, id );
    inst = v_net->inst;
    // add new vertex
    new_id = dgraph_vertex_add( g_dest, inst->name );
    // create new instance and add the attribute
    if( deep ) {
        inst = instrec_create( inst->name, new_id, inst->line, inst->type );
        v_net = virt_net_create_flatten( v_net, inst );
    }
    else inst->id = new_id;
    igraph_cattribute_VAN_set( g_dest, INST_ATTR_VNET, new_id,
            ( uintptr_t )v_net );
#if defined(DEBUG) || defined(DEBUG_FLATTEN_GRAPH)
    printf( "dgraph_vertex_copy: '%s(%d->%d)'\n", inst->name, id, new_id );
#endif // DEBUG_FLATTEN_GRAPH
    // add attr 'function name' if it exists
    if( igraph_cattribute_has_attr( g_src, IGRAPH_ATTRIBUTE_VERTEX,
                INST_ATTR_FUNC ) )
        igraph_cattribute_VAS_set( g_dest, INST_ATTR_FUNC, new_id,
            igraph_cattribute_VAS( g_src, INST_ATTR_FUNC, id ) );
    // add attr 'symbol' if it exists
    if( igraph_cattribute_has_attr( g_src, IGRAPH_ATTRIBUTE_VERTEX,
                INST_ATTR_SYMB ) )
        igraph_cattribute_VAN_set( g_dest, INST_ATTR_SYMB, new_id,
            igraph_cattribute_VAN( g_src, INST_ATTR_SYMB, id ) );
    // add attr 'graph' if it exists
    if( igraph_cattribute_has_attr( g_src, IGRAPH_ATTRIBUTE_VERTEX,
                INST_ATTR_GRAPH ) )
        igraph_cattribute_VAN_set( g_dest, INST_ATTR_GRAPH, new_id,
            igraph_cattribute_VAN( g_src, INST_ATTR_GRAPH, id ) );
    return inst;
}

/******************************************************************************/
void dgraph_vertex_destroy_attr( igraph_t* g, int id, bool deep )
{
    virt_net_t* v_net;

    v_net = ( virt_net_t* )( uintptr_t ) igraph_cattribute_VAN( g,
            INST_ATTR_VNET, id );
    virt_net_destroy( v_net, deep );
}

/******************************************************************************/
int dgraph_vertex_merge( igraph_t* g, int id1, int id2 )
{
    igraph_vector_t v_new;
    igraph_attribute_combination_t comb;
    int idx;
    int id_new = 0;
    int id_low = id1;
    int id_high = id2;
    if( id_high < id_low ) {
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
    igraph_attribute_combination( &comb,
            INST_ATTR_LABEL, IGRAPH_ATTRIBUTE_COMBINE_FIRST,
            INST_ATTR_FUNC, IGRAPH_ATTRIBUTE_COMBINE_FIRST,
            INST_ATTR_SYMB, IGRAPH_ATTRIBUTE_COMBINE_FIRST,
            INST_ATTR_VNET, IGRAPH_ATTRIBUTE_COMBINE_FIRST,
            INST_ATTR_GRAPH, IGRAPH_ATTRIBUTE_COMBINE_FIRST,
            IGRAPH_NO_MORE_ATTRIBUTES );
    igraph_contract_vertices( g, &v_new, &comb );
    igraph_attribute_combination_destroy( &comb );
    igraph_vector_destroy( &v_new );

    // id of deleted element
    return id_high;
}

/******************************************************************************/
void dgraph_vertex_update_ids( igraph_t* g, int id_start )
{
    int id;
    virt_net_t *v_net;
    for( id = id_start; id < igraph_vcount( g ); id++ ) {
        v_net = ( virt_net_t* )
            ( uintptr_t )igraph_cattribute_VAN( g, INST_ATTR_VNET, id );
        instrec_replace_id( v_net->inst, id + 1, id );
    }
}

/******************************************************************************/
void dgraph_vptr_to_v( igraph_vector_ptr_t* vptr, igraph_vector_t* v )
{
    int i;
    instrec_t* inst;
    igraph_vector_resize( v, igraph_vector_ptr_size( vptr ) );
    for( i = 0; i < igraph_vector_ptr_size( vptr ); i++ ) {
        inst = ( instrec_t* )igraph_vector_ptr_e( vptr, i );
        VECTOR( *v )[i] = inst->id;
    }
}
