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
        dgraph_edge_add( g, p_src, p_dest );
        IGRAPH_EIT_NEXT( eit );
    }
    igraph_eit_destroy( &eit );
    igraph_es_destroy( &es );
    free( inst_map );
}

/******************************************************************************/
int dgraph_edge_add( igraph_t* g, virt_port_t* p_src, virt_port_t* p_dest )
{
    int id = igraph_ecount( g );
#if defined(DEBUG) || defined(DEBUG_FLATTEN_GRAPH)
    printf( " add new edge %s(%d->%d)\n", p_src->name, p_src->inst->id,
            p_dest->inst->id );
#endif // DEBUG_FLATTEN_GRAPH
    igraph_add_edge( g, p_src->inst->id, p_dest->inst->id );
    igraph_cattribute_EAS_set( g, PORT_ATTR_LABEL, id, p_src->name );
    igraph_cattribute_EAN_set( g, PORT_ATTR_PSRC, id, ( uintptr_t )p_src );
    igraph_cattribute_EAN_set( g, PORT_ATTR_PDST, id, ( uintptr_t )p_dest );
    return id;
}

/******************************************************************************/
void dgraph_flatten( igraph_t* g_new, igraph_t* g )
{
    igraph_vs_t vs;
    igraph_vit_t vit;
    instrec_t *inst;
    igraph_t g_child, g_in, *g_tmp;
    int inst_id;

    igraph_copy( &g_in, g );
    vs = igraph_vss_all();
    igraph_vit_create( &g_in, vs, &vit );
    // iterate through all net instances of the graph
    while( !IGRAPH_VIT_END( vit ) ) {
        inst_id = IGRAPH_VIT_GET( vit );
        inst = ( instrec_t* )( uintptr_t )igraph_cattribute_VAN( &g_in,
                INST_ATTR_INST, inst_id );
        if( inst->type == INSTREC_NET ) {
#if defined(DEBUG) || defined(DEBUG_FLATTEN_GRAPH)
            printf( "Flatten instance '%s(%d)'\n", inst->name, inst->id );
#endif // DEBUG_FLATTEN_GRAPH
            g_tmp = ( igraph_t* )( uintptr_t )igraph_cattribute_VAN( &g_in,
                    INST_ATTR_GRAPH, inst_id );
            // deep copy child graph to create new instances
            igraph_empty( &g_child, 0, IGRAPH_DIRECTED );
            dgraph_append( &g_child, g_tmp, true );
            // recoursively flatten further net instances
            dgraph_flatten( g, &g_child );
            dgraph_flatten_net( g, &g_child, inst->id );
            igraph_destroy( &g_child );
        }
        IGRAPH_VIT_NEXT( vit );
    }
    igraph_vit_destroy( &vit );
    igraph_vs_destroy( &vs );
    dgraph_append( g_new, g, false );
}

/******************************************************************************/
void dgraph_flatten_net( igraph_t* g_new, igraph_t* g_child, int net_id )
{
    virt_port_t *p_src, *p_dest, *port, *port_net, *port_net_new;
    igraph_vs_t vs;
    igraph_es_t es;
    igraph_eit_t eit;

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
        if( p_dest->inst->id != net_id ) {
            port = p_dest;
            port_net = p_src;
        }
        // get open port with same symbol pointer from child graph
        port_net_new = dgraph_port_search_child( g_child, port_net );
        // connect this port to the matching port of the virtual net
        check_connection( port_net_new, port, g_new, false );
        IGRAPH_EIT_NEXT( eit );
    }
    igraph_eit_destroy( &eit );
    igraph_es_destroy( &es );
#if defined(DEBUG) || defined(DEBUG_FLATTEN_GRAPH)
    printf( "dgraph_flatten_net: remove vertice with id = %d\n", net_id );
#endif // DEBUG_FLATTEN_GRAPH
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
    printf( "Search port " );
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
    // copy graph
#if defined(DEBUG) || defined(DEBUG_SEARCH_PORT)
    printf( "Search port " );
    debug_print_vport( port );
    printf( "\n" );
#endif // DEBUG
    vs = igraph_vss_all();
    igraph_vit_create( g, vs, &vit );
    while( !IGRAPH_VIT_END( vit ) ) {
        id_inst = IGRAPH_VIT_GET( vit );
        v_net = ( virt_net_t* )( uintptr_t )igraph_cattribute_VAN( g,
                INST_ATTR_VNET, id_inst );
#if defined(DEBUG) || defined(DEBUG_SEARCH_PORT)
        printf( " in virtual net: " );
        debug_print_vports( v_net );
#endif // DEBUG
        port_inst = virt_port_get_equivalent( v_net, port, false );
        if( port_inst != NULL ) {
            port_res = port_inst;
            if( port_inst->inst->type == INSTREC_SYNC )
                break;
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
    igraph_cattribute_VAS_set( g, INST_ATTR_LABEL, id, name );
#if defined(DEBUG) || defined(DEBUG_FLATTEN_GRAPH)
    printf( "dgraph_vertex_add: '%s(%d)'\n", name, id );
#endif // DEBUG_FLATTEN_GRAPH
    return id;
}

/******************************************************************************/
void dgraph_vertex_add_attr( igraph_t* g, int id, const char* func,
        symrec_t* symb, instrec_t* inst, virt_net_t* v_net, igraph_t* g_net )
{
    const char* f_name = TEXT_NULL;
    if( func != NULL ) f_name = func;
    igraph_cattribute_VAS_set( g, INST_ATTR_FUNC, id, f_name );
    igraph_cattribute_VAN_set( g, INST_ATTR_SYMB, id, ( uintptr_t )symb );
    igraph_cattribute_VAN_set( g, INST_ATTR_INST, id, ( uintptr_t )inst );
    igraph_cattribute_VAN_set( g, INST_ATTR_VNET, id, ( uintptr_t )v_net );
    igraph_cattribute_VAN_set( g, INST_ATTR_GRAPH, id, ( uintptr_t )g_net );
}

/******************************************************************************/
virt_net_t* dgraph_vertex_add_box( igraph_t* g, symrec_t* symb, int line )
{
    int id = dgraph_vertex_add( g, symb->name );
    instrec_t* inst = instrec_create( symb->name, id, line, INSTREC_BOX );
    virt_net_t* v_net = virt_net_create_box( symb, inst );
    dgraph_vertex_add_attr( g, id, symb->attr_box->impl_name, symb, inst,
            v_net, NULL);
    return v_net;
}

/******************************************************************************/
virt_net_t* dgraph_vertex_add_net( igraph_t* g, symrec_t* symb, int line )
{
    int id = dgraph_vertex_add( g, symb->name );
    instrec_t* inst = instrec_create( symb->name, id, line, INSTREC_NET );
    virt_net_t* v_net = virt_net_create_net( symb->attr_net->v_net, inst );
    dgraph_vertex_add_attr( g, id, NULL, symb, inst, v_net,
            &symb->attr_net->g );
    return v_net;
}

/******************************************************************************/
virt_net_t* dgraph_vertex_add_sync( igraph_t* g, virt_port_t* port1,
        virt_port_t* port2 )
{
    int id = dgraph_vertex_add( g, TEXT_CP );
    instrec_t* inst = instrec_create( TEXT_CP, id, -1, INSTREC_SYNC );
    virt_net_t* v_net = virt_net_create_sync( inst, port1, port2 );
    dgraph_vertex_add_attr( g, id, NULL, NULL, inst, v_net, NULL );
    return v_net;
}

/******************************************************************************/
virt_net_t* dgraph_vertex_add_wrap( igraph_t* g, symrec_t* symb, int line )
{
    int id = dgraph_vertex_add( g, symb->name );
    instrec_t* inst = instrec_create( symb->name, id, line, INSTREC_WRAP );
    virt_net_t* v_net = virt_net_create_wrap( symb->attr_wrap->v_net, inst );
    dgraph_vertex_add_attr( g, id, NULL, symb, inst, v_net,
            &symb->attr_wrap->g );
    return v_net;
}

/******************************************************************************/
instrec_t* dgraph_vertex_copy( igraph_t* g_src, igraph_t* g_dest, int id,
        bool deep )
{
    int new_id;
    virt_net_t *v_net_old, *v_net_new;
    instrec_t *inst_old, *inst_new;
    // get old inst
    inst_new = inst_old = ( instrec_t* )( uintptr_t )
            igraph_cattribute_VAN( g_src, INST_ATTR_INST, id );
    // add new vertex
    new_id = dgraph_vertex_add( g_dest, inst_old->name );
    // create new instance and add the attribute
    if( deep ) inst_new = instrec_create( inst_old->name, new_id,
            inst_old->line, inst_old->type );
    else inst_new->id = new_id;
    igraph_cattribute_VAN_set( g_dest, INST_ATTR_INST, new_id,
            ( uintptr_t )inst_new );
#if defined(DEBUG) || defined(DEBUG_FLATTEN_GRAPH)
    printf( "dgraph_vertex_copy: '%s(%d->%d)'\n", inst_new->name, id, new_id );
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
    // create and add attr 'virtual net' if it exists
    if( igraph_cattribute_has_attr( g_src, IGRAPH_ATTRIBUTE_VERTEX,
                INST_ATTR_VNET ) ) {
        v_net_new = v_net_old = ( virt_net_t* )( uintptr_t )
            igraph_cattribute_VAN( g_src, INST_ATTR_VNET, id );
        if( deep ) v_net_new = virt_net_create_flatten( v_net_old, inst_new );
        igraph_cattribute_VAN_set( g_dest, INST_ATTR_VNET, new_id,
                ( uintptr_t )v_net_new );
    }
    return inst_new;
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
    igraph_attribute_combination( &comb, INST_ATTR_LABEL,
            IGRAPH_ATTRIBUTE_COMBINE_FIRST, INST_ATTR_FUNC,
            IGRAPH_ATTRIBUTE_COMBINE_FIRST, INST_ATTR_INST,
            IGRAPH_ATTRIBUTE_COMBINE_FIRST, INST_ATTR_SYMB,
            IGRAPH_ATTRIBUTE_COMBINE_FIRST, INST_ATTR_VNET,
            IGRAPH_ATTRIBUTE_COMBINE_FIRST, INST_ATTR_GRAPH,
            IGRAPH_ATTRIBUTE_COMBINE_FIRST, IGRAPH_NO_MORE_ATTRIBUTES );
    igraph_contract_vertices( g, &v_new, &comb );
    igraph_vector_destroy( &v_new );

    // id of deleted element
    return id_high;
}

/******************************************************************************/
void dgraph_vertex_update_ids( igraph_t* g, int id_start )
{
    int id;
    instrec_t *inst;
    for( id = id_start; id < igraph_vcount( g ); id++ ) {
        inst = ( instrec_t* )
            ( uintptr_t )igraph_cattribute_VAN( g, INST_ATTR_INST, id );
        instrec_replace_id( inst, id + 1, id );
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
