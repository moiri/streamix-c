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
                inst_map[ id_from ]->id, GE_PSRC );
        p_dest = dgraph_port_search_neighbour( g_tpl, g, id_edge,
                inst_map[ id_to ]->id, GE_PDST );
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
    const char* name_src = TEXT_NULL;
    const char* name_dst = TEXT_NULL;
    rate_type_t type;
    p_src->edge_id = id;
    p_dest->edge_id = id;
#if defined(DEBUG) || defined(DEBUG_CONNECT_GRAPH)
    printf( " add new edge %s(%d->%d)\n", name, p_src->v_net->inst->id,
            p_dest->v_net->inst->id );
#endif // DEBUG
    igraph_add_edge( g, p_src->v_net->inst->id, p_dest->v_net->inst->id );
    if( p_src->symb != NULL && p_src->symb->attr_port->alt_name != NULL )
        name_src = p_src->symb->attr_port->alt_name;
    if( p_dest->symb != NULL && p_dest->symb->attr_port->alt_name != NULL )
        name_dst = p_dest->symb->attr_port->alt_name;
    if( ( p_src->rate.type == TIME_TT ) || ( p_dest->rate.type == TIME_TT ) )
        type = TIME_TT;
    else if( ( p_src->rate.type == TIME_TB ) || ( p_dest->rate.type == TIME_TB ) )
        type = TIME_TB;
    else type = TIME_NONE;
    dgraph_edge_add_attr( g, id, name, p_src, p_dest, name_src, name_dst,
            p_src->descoupled, p_dest->descoupled, get_ch_len( p_dest, p_src ),
            p_src->rate.time.tv_sec, p_src->rate.time.tv_nsec,
            p_dest->rate.time.tv_sec, p_dest->rate.time.tv_nsec, type,
            p_src->is_dynamic, p_dest->is_dynamic );
    return id;
}

/******************************************************************************/
void dgraph_edge_add_attr( igraph_t* g, int id, const char* name,
        virt_port_t* p_src, virt_port_t* p_dst, const char* n_src,
        const char* n_dst, bool d_src, bool d_dst, int len, int ts_src,
        int tns_src, int ts_dst, int tns_dst, rate_type_t type,
        bool is_dyn_src, bool is_dyn_dst )
{
    igraph_cattribute_EAS_set( g, GE_LABEL, id, name );
    igraph_cattribute_EAN_set( g, GE_PSRC, id, ( uintptr_t )p_src );
    igraph_cattribute_EAN_set( g, GE_PDST, id, ( uintptr_t )p_dst );
    igraph_cattribute_EAS_set( g, GE_NSRC, id, n_src );
    igraph_cattribute_EAS_set( g, GE_NDST, id, n_dst );
    igraph_cattribute_EAN_set( g, GE_DSRC, id, d_src );
    igraph_cattribute_EAN_set( g, GE_DDST, id, d_dst );
    igraph_cattribute_EAN_set( g, GE_LEN, id, len );
    igraph_cattribute_EAN_set( g, GE_LEN, id, len );
    igraph_cattribute_EAN_set( g, GE_DTS, id, ts_dst );
    igraph_cattribute_EAN_set( g, GE_DTNS, id, tns_dst );
    igraph_cattribute_EAN_set( g, GE_STS, id, ts_src );
    igraph_cattribute_EAN_set( g, GE_STNS, id, tns_src );
    igraph_cattribute_EAN_set( g, GE_TYPE, id, type );
    igraph_cattribute_EAN_set( g, GE_DYNSRC, id, is_dyn_src );
    igraph_cattribute_EAN_set( g, GE_DYNDST, id, is_dyn_dst );
}

/******************************************************************************/
void dgraph_destroy_attr( igraph_t* g )
{
    igraph_vs_t vs;
    igraph_vit_t vit;

    if( igraph_vcount( g ) == 0 ) return;

    vs = igraph_vss_all();
    igraph_vit_create( g, vs, &vit );
    // iterate through all net instances of the graph
    while( !IGRAPH_VIT_END( vit ) ) {
        dgraph_vertex_destroy_attr( g, IGRAPH_VIT_GET( vit ), true );
        IGRAPH_VIT_NEXT( vit );
    }
    igraph_vit_destroy( &vit );
    igraph_vs_destroy( &vs );

    dgraph_destroy_attr_v( g, GV_SYMB );
    dgraph_destroy_attr_v( g, GV_GRAPH );
    dgraph_destroy_attr_v( g, GV_VNET );
    dgraph_destroy_attr_e( g, GE_PDST );
    dgraph_destroy_attr_e( g, GE_PSRC );
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
    virt_net_t* v_net_i;
    igraph_t g_child, g_in, *g_tmp;
    int inst_id;

    igraph_copy( &g_in, g );
    vs = igraph_vss_all();
    igraph_vit_create( &g_in, vs, &vit );
    // iterate through all net instances of the graph
    while( !IGRAPH_VIT_END( vit ) ) {
        inst_id = IGRAPH_VIT_GET( vit );
        v_net_i = ( virt_net_t* )( uintptr_t )igraph_cattribute_VAN( &g_in,
                GV_VNET, inst_id );
        if( ( v_net_i->type == VNET_NET ) || ( v_net_i->type == VNET_WRAP ) ) {
#if defined(DEBUG) || defined(DEBUG_FLATTEN_GRAPH)
            printf( "\nFlatten instance '%s(%d)' start\n", v_net_i->inst->name,
                    v_net_i->inst->id );
#endif // DEBUG_FLATTEN_GRAPH
            g_tmp = ( igraph_t* )( uintptr_t )igraph_cattribute_VAN( &g_in,
                    GV_GRAPH, inst_id );
            if( igraph_vcount( g_tmp ) == 0 ) {
                // something went wrong
                IGRAPH_VIT_NEXT( vit );
                continue;
            }
            // deep copy child graph to create new instances
            igraph_empty( &g_child, 0, IGRAPH_DIRECTED );
            dgraph_append( &g_child, g_tmp, true );
            // propagate attributes static and tt
            dgraph_vertex_propagate_attrs( &g_in, &g_child, inst_id );
            // recoursively flatten further net instances
            dgraph_flatten( g, &g_child );
            dgraph_flatten_net( g, &g_child, v_net_i );
            dgraph_vertex_remove( g, v_net_i->inst->id );
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
void dgraph_flatten_net( igraph_t* g_new, igraph_t* g_child, virt_net_t* v_net )
{
    virt_port_t *p_src, *p_dest, *port, *port_net, *port_net_open;
    igraph_t g;
    igraph_es_t es;
    igraph_eit_t eit;
    int i, id_from, id_to, net_id = v_net->inst->id, eid;
    int ecnt = igraph_ecount( g_new );
    bool done[ecnt];

    for( i=0; i<ecnt; i++ ) done[i] = false;

    igraph_copy( &g, g_new );
    // get all ports connecting to the net
    igraph_es_incident( &es, net_id, IGRAPH_ALL );
    igraph_eit_create( &g, es, &eit );
    // for each edge connect to the actual nets form the graph
    while( !IGRAPH_EIT_END( eit ) ) {
        eid = IGRAPH_EIT_GET( eit );
        if( done[eid] ) {
            // do self loop sonly once
            IGRAPH_EIT_NEXT( eit );
            continue;
        }
        done[eid] = true;
        igraph_edge( g_new, eid, &id_from, &id_to );
        p_src = ( virt_port_t* )( uintptr_t ) igraph_cattribute_EAN( g_new,
                GE_PSRC, IGRAPH_EIT_GET( eit ) );
        p_dest = ( virt_port_t* )( uintptr_t ) igraph_cattribute_EAN( g_new,
                GE_PDST, IGRAPH_EIT_GET( eit ) );
        // get id of the non net end of the edge
        port = p_src;
        port_net = p_dest;
        if( p_dest->v_net->inst->id != net_id ) {
            port = p_dest;
            port_net = p_src;
        }
        // get open port with same symbol pointer from child graph
        port_net_open = dgraph_port_search_child( g_child, port_net );
        if( port_net_open == NULL )
        {
            printf("no matching open port to port '%s'\n", port_net->name );
        }
        if( id_from == id_to )
            port = dgraph_port_search_child( g_child, port );
        // connect this port to the matching port of the virtual net
        if( v_net->type == VNET_NET ) {
            // unknown direction, class matters, modes have to be different
            check_connection( port_net_open, port, g_new, false, false, false );
            // connect rn of parallel nets to children
            check_connection_cp_net( port_net_open, port, g_new );
            // spawn rn if necessary and connect side ports
            check_connection_cp( NULL, port_net_open, port, g_new, AST_NET,
                igraph_cattribute_VAN( g_new, GV_TT, v_net->inst->id ) );
        }
        else if( v_net->type == VNET_WRAP ) {
            // unknown direction, ignore class, modes have to be different
            check_connection( port_net_open, port, g_new, false, true, false );
        }
        IGRAPH_EIT_NEXT( eit );
    }
    igraph_eit_destroy( &eit );
    igraph_es_destroy( &es );
    igraph_destroy( &g );
}

/******************************************************************************/
virt_port_t* dgraph_port_search_neighbour( igraph_t* g, igraph_t* g_new,
        int id_edge, int id_inst, const char* attr )
{
    virt_net_t* v_net = NULL;
    virt_port_t* port = ( virt_port_t* )( uintptr_t )igraph_cattribute_EAN( g,
            attr, id_edge );
    v_net = ( virt_net_t* )( uintptr_t )igraph_cattribute_VAN( g_new,
            GV_VNET, id_inst );
#if defined(DEBUG) || defined(DEBUG_SEARCH_PORT)
    printf( "dgraph_port_search_neighbour: Search port " );
    debug_print_vport( port );
    printf( "\n in virtual net: " );
    debug_print_vports( v_net );
#endif // DEBUG
    return virt_port_get_equivalent( v_net, port, true );
}

/******************************************************************************/
virt_port_t* dgraph_port_search_child( igraph_t* g, virt_port_t* port )
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
                GV_VNET, id_inst );
#if defined(DEBUG) || defined(DEBUG_SEARCH_PORT_CHILD)
        printf( " in virtual net: " );
        debug_print_vports( v_net );
#endif // DEBUG
        port_inst = virt_port_get_equivalent( v_net, port, false );
        if( port_inst != NULL ) {
            port_res = port_inst;
            if( port_inst->v_net->inst->type == INSTREC_SYNC ) {
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
int dgraph_vertex_add( igraph_t* g, const char* name )
{
    int id = igraph_vcount( g );
    igraph_add_vertices( g, 1, NULL );
    igraph_cattribute_VAS_set( g, GV_LABEL, id, name );
#if defined(DEBUG) || defined(DEBUG_CONNECT_GRAPH)
    printf( "dgraph_vertex_add: '%s(%d)'\n", name, id );
#endif // DEBUG
    return id;
}

/******************************************************************************/
void dgraph_vertex_add_attr( igraph_t* g, int id, const char* func,
        symrec_t* symb, virt_net_t* v_net, igraph_t* g_net, bool attr_static,
        bool attr_pure, bool attr_ext )
{
    const char* f_name = TEXT_NULL;
    if( func != NULL ) f_name = func;
    igraph_cattribute_VAS_set( g, GV_IMPL, id, f_name );
    igraph_cattribute_VAN_set( g, GV_SYMB, id, ( uintptr_t )symb );
    igraph_cattribute_VAN_set( g, GV_VNET, id, ( uintptr_t )v_net );
    igraph_cattribute_VAN_set( g, GV_GRAPH, id, ( uintptr_t )g_net );
    igraph_cattribute_VAN_set( g, GV_STATIC, id, attr_static );
    igraph_cattribute_VAN_set( g, GV_PURE, id, attr_pure );
    igraph_cattribute_VAN_set( g, GV_EXT, id, attr_ext );
    igraph_cattribute_VAN_set( g, GV_TT, id, false );
}

/******************************************************************************/
void dgraph_vertex_add_attr_tt( igraph_t* g, int id, int attr_tt )
{
    igraph_cattribute_VAN_set( g, GV_TT, id, attr_tt );
}

/******************************************************************************/
virt_net_t* dgraph_vertex_add_box( igraph_t* g, symrec_t* symb, int line )
{
    int id = dgraph_vertex_add( g, symb->name );
    instrec_t* inst = instrec_create( symb->name, id, line, INSTREC_BOX );
    virt_net_t* v_net = virt_net_create_box( symb, inst );
    dgraph_vertex_add_attr( g, id, symb->attr_box->impl_name, symb, v_net,
            NULL, false, symb->attr_box->attr_pure, symb->attr_box->attr_ext );
    return v_net;
}

/******************************************************************************/
virt_net_t* dgraph_vertex_add_net( igraph_t* g, symrec_t* symb, int line )
{
    int id = dgraph_vertex_add( g, symb->name );
    instrec_t* inst = instrec_create( symb->name, id, line, INSTREC_NET );
    virt_net_t* v_net = virt_net_create_net( symb->attr_net->v_net, inst );
    dgraph_vertex_add_attr( g, id, NULL, symb, v_net, &symb->attr_net->g,
            false, false, false );
    return v_net;
}

/******************************************************************************/
virt_net_t* dgraph_vertex_add_sync( igraph_t* g, const char* name )
{
    int id = dgraph_vertex_add( g, name );
    instrec_t* inst = instrec_create( ( char* )name, id, -1, INSTREC_SYNC );
    virt_net_t* v_net = virt_net_create_sync( inst );
    dgraph_vertex_add_attr( g, id, name, NULL, v_net, NULL, false, false,
            false );
    return v_net;
}

/******************************************************************************/
virt_net_t* dgraph_vertex_add_wrap( igraph_t* g, symrec_t* symb, int line )
{
    int id = dgraph_vertex_add( g, symb->name );
    instrec_t* inst = instrec_create( symb->name, id, line, INSTREC_WRAP );
    virt_net_t* v_net = virt_net_create_wrap( symb, inst );
    dgraph_vertex_add_attr( g, id, NULL, symb, v_net, &symb->attr_wrap->g,
            symb->attr_wrap->attr_static, false, false );
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
            GV_VNET, id );
    inst = v_net->inst;
    // add new vertex
    new_id = dgraph_vertex_add( g_dest, inst->name );
    // create new instance and add the attribute
    if( deep ) {
        inst = instrec_create( inst->name, new_id, inst->line, inst->type );
        v_net = virt_net_create_flatten( v_net, inst );
    }
    else inst->id = new_id;
    igraph_cattribute_VAN_set( g_dest, GV_VNET, new_id,
            ( uintptr_t )v_net );
#if defined(DEBUG) || defined(DEBUG_FLATTEN_GRAPH)
    printf( "dgraph_vertex_copy: '%s(%d->%d)'\n", inst->name, id, new_id );
#endif // DEBUG_FLATTEN_GRAPH
    // add attr 'function name' if it exists
    if( igraph_cattribute_has_attr( g_src, IGRAPH_ATTRIBUTE_VERTEX,
                GV_IMPL ) )
        igraph_cattribute_VAS_set( g_dest, GV_IMPL, new_id,
            igraph_cattribute_VAS( g_src, GV_IMPL, id ) );
    // add attr 'symbol' if it exists
    if( igraph_cattribute_has_attr( g_src, IGRAPH_ATTRIBUTE_VERTEX,
                GV_SYMB ) )
        igraph_cattribute_VAN_set( g_dest, GV_SYMB, new_id,
            igraph_cattribute_VAN( g_src, GV_SYMB, id ) );
    // add attr 'graph' if it exists
    if( igraph_cattribute_has_attr( g_src, IGRAPH_ATTRIBUTE_VERTEX,
                GV_GRAPH ) )
        igraph_cattribute_VAN_set( g_dest, GV_GRAPH, new_id,
            igraph_cattribute_VAN( g_src, GV_GRAPH, id ) );
    if( igraph_cattribute_has_attr( g_src, IGRAPH_ATTRIBUTE_VERTEX,
                GV_STATIC ) )
        igraph_cattribute_VAN_set( g_dest, GV_STATIC, new_id,
            igraph_cattribute_VAN( g_src, GV_STATIC, id ) );
    if( igraph_cattribute_has_attr( g_src, IGRAPH_ATTRIBUTE_VERTEX,
                GV_PURE ) )
        igraph_cattribute_VAN_set( g_dest, GV_PURE, new_id,
            igraph_cattribute_VAN( g_src, GV_PURE, id ) );
    if( igraph_cattribute_has_attr( g_src, IGRAPH_ATTRIBUTE_VERTEX,
                GV_EXT ) )
        igraph_cattribute_VAN_set( g_dest, GV_EXT, new_id,
            igraph_cattribute_VAN( g_src, GV_EXT, id ) );
    if( igraph_cattribute_has_attr( g_src, IGRAPH_ATTRIBUTE_VERTEX,
                GV_TT ) )
        igraph_cattribute_VAN_set( g_dest, GV_TT, new_id,
            igraph_cattribute_VAN( g_src, GV_TT, id ) );
    return inst;
}

/******************************************************************************/
void dgraph_vertex_destroy_attr( igraph_t* g, int id, bool deep )
{
    virt_net_t* v_net;

    v_net = ( virt_net_t* )( uintptr_t ) igraph_cattribute_VAN( g,
            GV_VNET, id );
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
            GV_LABEL, IGRAPH_ATTRIBUTE_COMBINE_FIRST,
            GV_IMPL, IGRAPH_ATTRIBUTE_COMBINE_FIRST,
            GV_SYMB, IGRAPH_ATTRIBUTE_COMBINE_FIRST,
            GV_VNET, IGRAPH_ATTRIBUTE_COMBINE_FIRST,
            GV_GRAPH, IGRAPH_ATTRIBUTE_COMBINE_FIRST,
            GV_STATIC, IGRAPH_ATTRIBUTE_COMBINE_FIRST,
            GV_PURE, IGRAPH_ATTRIBUTE_COMBINE_FIRST,
            GV_EXT, IGRAPH_ATTRIBUTE_COMBINE_FIRST,
            GV_TT, IGRAPH_ATTRIBUTE_COMBINE_FIRST,
            IGRAPH_NO_MORE_ATTRIBUTES );
    igraph_contract_vertices( g, &v_new, &comb );
    igraph_attribute_combination_destroy( &comb );
    igraph_vector_destroy( &v_new );

    // id of deleted element
    return id_high;
}

/******************************************************************************/
void dgraph_vertex_propagate_attrs( igraph_t* g_in, igraph_t* g, int id )
{
    int vid;
    igraph_vs_t vs;
    igraph_vit_t vit;
    int v_static = igraph_cattribute_VAN( g_in, GV_STATIC, id );
    int v_tt = igraph_cattribute_VAN( g_in, GV_TT, id );
    vs = igraph_vss_all();
    igraph_vit_create( g, vs, &vit );
    while( !IGRAPH_VIT_END( vit ) ) {
        vid = IGRAPH_VIT_GET( vit );
        if( v_static )
            igraph_cattribute_VAN_set( g, GV_STATIC, vid, v_static );
        if( v_tt )
            igraph_cattribute_VAN_set( g, GV_TT, vid, v_tt );
        IGRAPH_VIT_NEXT( vit );
    }
    igraph_vit_destroy( &vit );
    igraph_vs_destroy( &vs );
}

/******************************************************************************/
void dgraph_vertex_remove( igraph_t* g, int id )
{
    igraph_vs_t vs;
#if defined(DEBUG) || defined(DEBUG_FLATTEN_GRAPH)
    printf( "dgraph_remove_vertex: id = %d\n", id );
#endif // DEBUG_FLATTEN_GRAPH
    dgraph_vertex_destroy_attr( g, id, true );
    vs = igraph_vss_1( id );
    igraph_delete_vertices( g, vs );
    igraph_vs_destroy( &vs );
    dgraph_vertex_update_ids( g, id );
}

/******************************************************************************/
void dgraph_vertex_update_ids( igraph_t* g, int id_start )
{
    int id;
    virt_net_t *v_net;
    for( id = id_start; id < igraph_vcount( g ); id++ ) {
        v_net = ( virt_net_t* )
            ( uintptr_t )igraph_cattribute_VAN( g, GV_VNET, id );
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

/******************************************************************************/
void dgraph_wrap_sync_create( igraph_t* g, igraph_vector_ptr_t* syncs,
        virt_net_t* v_net_i, virt_net_t* v_net )
{
    struct timespec tb;
    int i = 0, j = 0;
    port_mode_t mode;
    sync_t* sync = NULL;
    virt_port_t* vp_new = NULL;
    virt_port_t* vp_net = NULL;
    symrec_t* sp_src = NULL;
    symrec_t* sp_int = NULL;
    virt_net_t* cp_sync = NULL;
    tb.tv_sec = 0;
    tb.tv_nsec = 0;

    for( i = 0; i < igraph_vector_ptr_size( syncs ); i++ ) {
        sync = VECTOR( *syncs )[i];
        if( igraph_vector_ptr_size( &sync->p_int ) == 0 ) {
            // there was no internal port, copy the regular port to the new list
            sp_src = VECTOR( sync->p_ext )[0];
            // search for the port in the virtual net of the connection
            vp_net = virt_port_get_equivalent_by_symb_attr( v_net_i, sp_src );
            vp_new = virt_port_create( vp_net->attr_class, vp_net->attr_mode,
                    vp_net->v_net, vp_net->name, vp_net->symb,
                    vp_net->rate.time, vp_net->rate.type, vp_net->descoupled,
                    vp_net->is_open, vp_net->is_dynamic, vp_net->ch_len );
            virt_port_append( v_net, vp_new );
        }
        else {
            // create a copy synchronizer
            cp_sync = dgraph_vertex_add_sync( g, TEXT_CP );
            for( j = 0; j < igraph_vector_ptr_size( &sync->p_ext ); j++ ) {
                sp_src = VECTOR( sync->p_ext )[j];
                igraph_cattribute_VAN_set( g, GV_SYMB, cp_sync->inst->id,
                        ( uintptr_t )sp_src );
                // create a new external virtual port, cp_sync ports have no
                // rate control
                vp_net = virt_port_create( sp_src->attr_port->collection,
                        sp_src->attr_port->mode, cp_sync, sp_src->name,
                        sp_src, tb, TIME_NONE, sp_src->attr_port->decoupled,
                        false, false, 0 );
                virt_port_append( v_net, vp_net );
                virt_port_append( cp_sync, virt_port_copy( vp_net ) );
            }
            for( j = 0; j < igraph_vector_ptr_size( &sync->p_int ); j++ ) {
                sp_int = VECTOR( sync->p_int )[j];
                // search for the port in the virtual net of the connection
                vp_net = virt_port_get_equivalent_by_symb_attr( v_net_i,
                        sp_int );
                // if the port cannot be found in the net its a bypass
                if( vp_net == NULL ) {
#if defined(DEBUG) || defined(DEBUG_CONNECT)
                    printf( "wrap_sync_create_cp: bypass\n" );
#endif // DEBUG_CONNECT
                    igraph_cattribute_VAN_set( g, GV_SYMB,
                            cp_sync->inst->id, ( uintptr_t )sp_int );
                    break;
                }
                mode = vp_net->attr_mode;
                if( mode == PORT_MODE_OUT )
                    mode = PORT_MODE_IN;
                else if( mode == PORT_MODE_IN )
                    mode = PORT_MODE_OUT;
                vp_new = virt_port_create( vp_net->attr_class, mode, cp_sync,
                        vp_net->name, vp_net->symb, tb, TIME_NONE,
                        false, false, false, 0 );
                virt_port_append( cp_sync, vp_new );
                // unknown direction, ignore class, modes have to be equal
                check_connection( vp_new, vp_net, g, false, true, true );
            }
        }
    }
}

/******************************************************************************/
int get_ch_len( virt_port_t* p1, virt_port_t* p2 )
{
    int res = p1->ch_len + p2->ch_len;
    if( res == 0 ) res = 1;
    return res;
}

/******************************************************************************/
bool is_wrap_sync_merge_int( igraph_vector_ptr_t* v1, igraph_vector_ptr_t* v2 )
{
    symrec_t* p1 = NULL;
    symrec_t* p2 = NULL;
    int i = 0, j = 0;
    for( i = 0; i < igraph_vector_ptr_size( v1 ); i++ ) {
        p1 = VECTOR( *v1 )[i];
        for( j = 0; j < igraph_vector_ptr_size( v2 ); j++ ) {
            p2 = VECTOR( *v2 )[j];
            if( strcmp( p1->name, p2->name ) == 0 ) return true;
        }
    }
    return false;
}

/******************************************************************************/
virt_net_t* wrap_connect_int( symrec_list_t* wrap_ports, virt_net_t* v_net_n,
        igraph_t* g )
{
    igraph_vector_ptr_t syncs;
    virt_net_t* v_net;

    // group ports in order to create copy synchronizers
    igraph_vector_ptr_init( &syncs, 0 );
    wrap_sync_init( &syncs, wrap_ports );
#if defined(DEBUG) || defined(DEBUG_CONNECT_WRAP)
    printf( "check_wrap:\n" );
    debug_print_syncs( &syncs );
#endif // DEBUG
    wrap_sync_merge( &syncs );
#if defined(DEBUG) || defined(DEBUG_CONNECT_WRAP)
    printf( "connect_wrap:\n" );
    debug_print_syncs( &syncs );
#endif // DEBUG

    v_net = malloc( sizeof( virt_net_t ) );
    v_net->type = VNET_NET;
    v_net->inst = NULL;
    v_net->con = NULL;
    v_net->ports = NULL;
    // create copy synchronizers
    dgraph_wrap_sync_create( g, &syncs, v_net_n, v_net );

    // cleanup
    wrap_sync_destroy( &syncs );

    return v_net;
}

/******************************************************************************/
void wrap_sync_destroy( igraph_vector_ptr_t* syncs )
{
    sync_t* sync = NULL;
    int i = 0;
    for( i = 0; i < igraph_vector_ptr_size( syncs ); i++ ) {
        sync = VECTOR( *syncs )[i];
        igraph_vector_ptr_destroy( &sync->p_ext );
        igraph_vector_ptr_destroy( &sync->p_int );
        free( sync );
    }
    igraph_vector_ptr_destroy( syncs );
}

/******************************************************************************/
void wrap_sync_init( igraph_vector_ptr_t* syncs, symrec_list_t* sps_src )
{
    symrec_list_t* sps_int = NULL;
    sync_t* sync = NULL;

    while( sps_src != NULL ) {
        sps_int = sps_src->rec->attr_port->ports_int;
        sync = malloc( sizeof( sync_t ) );
        igraph_vector_ptr_init( &sync->p_ext, 1 );
        VECTOR( sync->p_ext )[0] = sps_src->rec;
        igraph_vector_ptr_init( &sync->p_int, 0 );
        igraph_vector_ptr_push_back( syncs, sync );
        while( sps_int != NULL ) {
            igraph_vector_ptr_push_back( &sync->p_int, sps_int->rec );
            sps_int = sps_int->next;
        }
        sps_src = sps_src->next;
    }
}

/******************************************************************************/
void wrap_sync_merge( igraph_vector_ptr_t* syncs )
{
    sync_t* sync1 = NULL;
    sync_t* sync2 = NULL;
    int i = 0, j = 0;
    for( i = 0; i < igraph_vector_ptr_size( syncs ); i++ ) {
        sync1 = VECTOR( *syncs )[i];
        for( j = i + 1; j < igraph_vector_ptr_size( syncs ); j++ ) {
            sync2 = VECTOR( *syncs )[j];
            if( is_wrap_sync_merge_int( &sync1->p_int, &sync2->p_int ) ) {
                wrap_sync_merge_port( &sync1->p_int, &sync2->p_int, true );
                wrap_sync_merge_port( &sync1->p_ext, &sync2->p_ext, false );
                igraph_vector_ptr_remove( syncs, j );
                igraph_vector_ptr_destroy( &sync2->p_int );
                igraph_vector_ptr_destroy( &sync2->p_ext );
                free( sync2 );
                if( i > 0 ) i--;
                j--;
            }
        }
    }
}

/******************************************************************************/
void wrap_sync_merge_port( igraph_vector_ptr_t* v1, igraph_vector_ptr_t* v2,
        bool check_name )
{
    symrec_t* p1 = NULL;
    symrec_t* p2 = NULL;
    bool add = true;
    int i = 0, j = 0;
    for( i = 0; i < igraph_vector_ptr_size( v2 ); i++ ) {
        add = true;
        p2 = VECTOR( *v2 )[i];
        for( j = 0; j < igraph_vector_ptr_size( v1 ); j++ ) {
            p1 = VECTOR( *v1 )[j];
            if( check_name && ( strcmp( p1->name, p2->name ) == 0 ) ) {
                add = false;
                break;
            }
        }
        if( add ) igraph_vector_ptr_push_back( v1, p2 );
    }
}

/******************************************************************************/
void debug_print_syncs( igraph_vector_ptr_t* syncs )
{
    sync_t* sync = NULL;
    symrec_t* port = NULL;
    int i = 0, j = 0;
    for( i = 0; i < igraph_vector_ptr_size( syncs ); i++ ) {
        sync = VECTOR( *syncs )[i];
        printf( " cp%d: [ ", i );
        for( j = 0; j < igraph_vector_ptr_size( &sync->p_ext ); j++ ) {
            port = VECTOR( sync->p_ext )[j];
            printf( "%s, ", port->name );
        }
        printf( "][ " );
        for( j = 0; j < igraph_vector_ptr_size( &sync->p_int ); j++ ) {
            port = VECTOR( sync->p_int )[j];
            printf( "%s, ", port->name );
        }
        printf( "]\n" );
    }
}
