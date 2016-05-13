#include "context.h"
#include "defines.h"
#include "ngraph.h"
#include "error.h"

/******************************************************************************/
bool are_port_names_ok( virt_ports* p1, virt_ports* p2, bool cpsync )
{
    // no, if port names do not match
    if( strcmp( p1->rec->name, p2->rec->name ) != 0 )
        return false;
    // are we checking copy synchronizer connections?
    if( cpsync ) {
        // we are good if both ports have the same class
        if( p1->attr_class == p2->attr_class )
            return true;
        // we are good if one port has no class and the other is not a side port
        if( ( p1->attr_class != VAL_SIDE ) && ( p2->attr_class == VAL_NONE ) )
            return true;
        if( ( p1->attr_class == VAL_NONE ) && ( p2->attr_class != VAL_SIDE ) )
            return true;
        // we came through here so none of the conditions matched
        return false;
    }
    // or normal connections?
    else {
        // no, if the left port is in another class than DS
        if( ( p1->attr_class != VAL_DOWN ) && ( p1->attr_class != VAL_NONE ) )
            return false;
        // no, if the right port is in another class than US
        if( ( p2->attr_class != VAL_UP ) && ( p2->attr_class != VAL_NONE ) )
            return false;
    }
    // we came through here, so all is good, names match
    return true;
}

/******************************************************************************/
bool are_port_modes_ok( virt_ports* p1, virt_ports* p2 )
{
    // yes, if modes are different
    if( p1->attr_mode != p2->attr_mode ) return true;
    // yes, if the left port is a copy synchronizer port
    if( p1->attr_mode == VAL_BI ) return true;
    // yes, if the right port is a copy synchronizer port
    if( p2->attr_mode == VAL_BI ) return true;

    // we came through here, so port modes are not compatible
    return false;
}

/******************************************************************************/
void check_connection( inst_net* net, virt_net* v_net1, virt_net* v_net2,
        igraph_t* g_con )
{
    char error_msg[ CONST_ERROR_LEN ];
    virt_ports* ports_l = NULL;
    virt_ports* ports_r = NULL;
    virt_ports* ports_last_l = NULL;
    virt_ports* ports_next_l = NULL;
    virt_ports* ports_last_r = NULL;
    virt_ports* ports_next_r = NULL;
    int edge_id, v1_id, v2_id;
    inst_rec* rec1 = NULL;
    inst_rec* rec2 = NULL;

    ports_l = v_net1->ports;
    while( ports_l != NULL ) {
        ports_next_l = ports_l->next;
        ports_r = v_net2->ports;
        ports_last_r = NULL;
        while( ports_r != NULL ) {
            ports_next_r = ports_r->next;
#if defined(DEBUG) || defined(DEBUG_CONNECT)
            printf( "check_connection: " );
            debug_print_port( ports_l );
            printf( "and " );
            debug_print_port( ports_r );
#endif // DEBUG_CONNECT
            if( are_port_names_ok( ports_l, ports_r, false ) ) {
                if( ( ports_l->inst->type == VAL_CP )
                        && ( ports_r->inst->type == VAL_CP ) ) {
                    cgraph_update( g_con, ports_l->inst->id,
                            ports_r->inst->id, ports_l->inst->type,
                            ports_l->inst->type, &net->g);
                    // merge copy synchronizers
                    cpsync_merge( net, ports_l, ports_r );
                    dgraph_merge_vertice_1( g_con, ports_l->inst->id,
                            ports_r->inst->id );
#if defined(DEBUG) || defined(DEBUG_CONNECT)
                    printf( " -> connection is valid\n" );
#endif // DEBUG_CONNECT
                }
                else if( are_port_modes_ok( ports_l, ports_r ) ) {
                    cgraph_update( g_con, ports_l->inst->id,
                            ports_r->inst->id, ports_l->inst->type,
                            ports_l->inst->type, &net->g);
                    dgraph_connect_1( &net->g, ports_l->inst->id,
                            ports_r->inst->id, ports_l->attr_mode,
                            ports_r->attr_mode );

#if defined(DEBUG) || defined(DEBUG_CONNECT)
                    printf( " -> connection is valid\n" );
#endif // DEBUG_CONNECT
                }
                else {
#if defined(DEBUG) || defined(DEBUG_CONNECT)
                    printf( " -> connection is invalid\n" );
#endif // DEBUG_CONNECT
                    sprintf( error_msg, ERROR_BAD_MODE, ERR_ERROR,
                            ports_l->rec->name, ports_l->inst->name,
                            ports_l->inst->id, ports_r->inst->name,
                            ports_r->inst->id, ports_r->rec->line );
                    report_yyerror( error_msg, ports_r->rec->line );
                }
                // remove both ports
                if( ports_last_l != NULL ) ports_last_l->next = ports_next_l;
                else v_net1->ports = ports_next_l;
                if( ports_last_r != NULL ) ports_last_r->next = ports_next_r;
                else v_net2->ports = ports_next_r;
                free( ports_l );
                free( ports_r );
                ports_l = ports_last_l;
                break;
            }
            else {
#if defined(DEBUG) || defined(DEBUG_CONNECT)
                printf( " -> connection is invalid\n" );
#endif // DEBUG_CONNECT
            }
            ports_last_r = ports_r;
            ports_r = ports_next_r;
        }
        ports_last_l = ports_l;
        ports_l = ports_next_l;
    }

    // perform further checks on port connections
    for( edge_id = 0; edge_id < igraph_ecount( g_con ); edge_id++ ) {
        igraph_edge( g_con, edge_id, &v1_id, &v2_id );
        rec1 = inst_rec_get_id( &net->recs_id, v1_id );
        rec2 = inst_rec_get_id( &net->recs_id, v2_id );
        // ERROR: there is no connection between the two nets
        sprintf( error_msg, ERROR_NO_NET_CON, ERR_ERROR, rec1->name, v1_id,
                rec2->name, v2_id );
        report_yyerror( error_msg, rec1->line );
    }
}

/******************************************************************************/
void check_context( ast_node* ast )
{
    inst_net* nets = NULL;        // hash table to store the nets
    symrec* symtab = NULL;        // hash table to store the symbols
    UT_array* scope_stack = NULL; // stack to handle the scope
    int scope = 0;

    utarray_new( scope_stack, &ut_int_icd );
    utarray_push_back( scope_stack, &scope );
    check_context_ast( &symtab, &nets, scope_stack, ast, false );
    /* install_ids( &symtab, scope_stack, ast, false ); */
    /* instrec_put( &insttab, VAL_THIS, *utarray_back( scope_stack ), */
    /*         VAL_SELF, -1, NULL ); */
    /* check_nets( &symtab, &nets, scope_stack, ast ); */
    /* check_instances( &nets ); */
    /* // check whether all ports are connected spawn synchronizers and draw the */
    /* // nodes, synchroniyers and connections */
    /* check_port_all( &insttab, ast ); */
}

/******************************************************************************/
void* check_context_ast( symrec** symtab, inst_net** nets,
        UT_array* scope_stack, ast_node* ast, bool is_sync )
{
    ast_list* list = NULL;
    box_attr* b_attr = NULL;
    wrap_attr* w_attr = NULL;
    port_attr* p_attr = NULL;
    void* attr = NULL;
    inst_net* net = NULL;
    symrec_list* ptr = NULL;
    symrec_list* port_list = NULL;
    void* res = NULL;
    bool set_sync = false;
    int type;
    static int _scope = 0;
    static int _sync_id = 0; // used to assemble ports to sync groups

    if( ast == NULL ) return NULL;

    switch( ast->type ) {
        case AST_PROGRAM:
            check_context_ast( symtab, nets, scope_stack, ast->program.stmts,
                    false );
            check_context_ast( symtab, nets, scope_stack, ast->program.net,
                    false );
            break;
        case AST_STMTS:
            list = ast->list;
            while (list != NULL) {
                check_context_ast( symtab, nets, scope_stack, list->node,
                        false );
                list = list->next;
            }
            break;
        case AST_ASSIGN:
            // get the box attributes
            attr = check_context_ast( symtab, nets, scope_stack, ast->assign.op,
                    false );
            // install the box symbol
            symrec_put( symtab, ast->assign.id->symbol.name,
                    *utarray_back( scope_stack ), ast->assign.op->type, attr,
                    ast->assign.id->symbol.line );
            break;
        case AST_NET:
            // install net instances
            net = inst_net_put( nets, *utarray_back( scope_stack ) );
            net->v_net = install_nets( symtab, net, scope_stack, ast->node );
#if defined(DEBUG) || defined(DEBUG_NET_DOT)
            igraph_write_graph_dot( &net->g, stdout );
#endif // DEBUG_NET_DOT
            break;
        case AST_NET_PROTO:
            break;
        case AST_SYNCS:
            _sync_id++;
            set_sync = true;
        case AST_PORTS:
            list = ast->list;
            while (list != NULL) {
                res = check_context_ast( symtab, nets, scope_stack, list->node,
                        set_sync );
                ptr = ( struct symrec_list* )res;
                while( ( ( struct symrec_list* ) res)->next != NULL )
                    res = ( ( struct symrec_list* )res )->next;
                ( ( struct symrec_list* )res )->next = port_list;
                port_list = ptr;
                list = list->next;
            }
            res = ( void* )ptr;   // return pointer to the port list
            break;
        case AST_BOX:
            _scope++;
            utarray_push_back( scope_stack, &_scope );
            port_list = ( struct symrec_list* )check_context_ast( symtab, nets,
                    scope_stack, ast->box.ports, set_sync );
            // prepare symbol attributes and install symbol
            b_attr = ( box_attr* )malloc( sizeof( box_attr ) );
            b_attr->attr_pure = ( ast->box.attr_pure != NULL ) ? true : false;
            b_attr->ports = port_list;
            // add internal name if available
            b_attr->impl_name = ( char* )malloc( strlen(
                        ast->box.impl->symbol.name ) + 1 );
            strcpy( b_attr->impl_name,
                    ast->box.impl->symbol.name );
            utarray_pop_back( scope_stack );
            res = ( void* )b_attr;
            break;
        case AST_WRAP:
            _scope++;
            utarray_push_back( scope_stack, &_scope );
            check_context_ast( symtab, nets, scope_stack, ast->wrap.stmts,
                    set_sync );
            port_list = ( struct symrec_list* )check_context_ast( symtab, nets,
                    scope_stack, ast->wrap.ports, set_sync );
            // prepare symbol attributes and install symbol
            w_attr = ( wrap_attr* )malloc( sizeof( wrap_attr ) );
            w_attr->attr_static =
                ( ast->wrap.attr_static != NULL ) ? true : false;
            w_attr->ports = port_list;
            utarray_pop_back( scope_stack );
            // install the wrapper symbol in the scope of its declaration
            symrec_put( symtab, ast->wrap.id->symbol.name,
                    *utarray_back( scope_stack ), VAL_NET, ( void* )w_attr,
                    ast->wrap.id->symbol.line );
            break;
        case AST_PORT:
            // prepare symbol attributes
            p_attr = ( port_attr* )malloc( sizeof( port_attr ) );
            if( ast->port.mode != NULL )
                p_attr->mode = ast->port.mode->attr.val;
            /* // add internal name if available */
            /* p_attr->int_name = NULL; */
            /* if( ast->port.int_id != NULL ) { */
            /*     p_attr->int_name = ( char* )malloc( strlen( */
            /*                 ast->port.int_id->ast_node->ast_id.name ) + 1 ); */
            /*     strcpy( p_attr->int_name, */
            /*             ast->port.int_id->ast_node->ast_id.name ); */
            /* } */
            type = VAL_PORT;
            // add sync attributes if port is a sync port
            if( is_sync ) {
                type = VAL_SPORT;
                p_attr->decoupled =
                    (ast->port.coupling == NULL) ? false : true;
                p_attr->sync_id = _sync_id;
            }
            // set collection
            if( ast->port.collection == NULL )
                p_attr->collection = VAL_NONE;
            else {
                p_attr->collection =
                    ast->port.collection->attr.val;
            }
            // install symbol and return pointer to the symbol record
            res = ( void* )symrec_put( symtab, ast->port.id->symbol.name,
                    *utarray_back( scope_stack ), type, p_attr,
                    ast->port.id->symbol.line );
            ptr = ( symrec_list* )malloc( sizeof( symrec_list ) );
            ptr->rec = ( struct symrec* )res;
            ptr->next = NULL;
            res = ( void* )ptr;   // return pointer to the port list
            break;
        default:
            ;
    }
    port_list = NULL;
    ptr = NULL;
    return res;

}

/******************************************************************************/
void cpsync_connect( inst_net* net, virt_net* v_net1, virt_net* v_net2 )
{
    virt_ports* port1 = NULL;
    virt_ports* port2 = NULL;
    virt_ports* port_last = NULL;
    virt_ports* port_next = NULL;
    inst_rec* cp_sync = NULL;

    port1 = v_net1->ports;
    while( port1 != NULL ) {
        port2 = v_net2->ports;
        port_last = NULL;
        while( port2 != NULL ) {
            port_next = port2->next;
            if( are_port_names_ok( port1, port2, true ) ) {
                // create copy synchronizer instance
                if( ( port1->inst->type == VAL_CP )
                        && ( port2->inst->type == VAL_CP ) ) {
                    // merge copy synchronizers
                    cp_sync = cpsync_merge( net, port1, port2 );
                }
                else if( port1->inst->type == VAL_CP ) {
                    cp_sync = port1->inst;
                    dgraph_connect_1( &net->g, cp_sync->id, port2->inst->id,
                            VAL_BI, port2->attr_mode );
                }
                else if( port2->inst->type == VAL_CP ) {
                    cp_sync = port2->inst;
                    dgraph_connect_1( &net->g, cp_sync->id, port1->inst->id,
                            VAL_BI, port1->attr_mode );
                }
                else {
                    cp_sync = inst_rec_put( &net->recs_name, &net->recs_id,
                            TEXT_CP, igraph_vcount( &net->g ), 0, VAL_CP,
                            NULL );
#if defined(DEBUG) || defined(DEBUG_CONNECT)
                    printf( "Create copy-synchronizer %s(%d)\n", cp_sync->name,
                            cp_sync->id );
#endif // DEBUG_CONNECT
                    igraph_add_vertices( &net->g, 1, NULL );
                    dgraph_connect_1( &net->g, cp_sync->id, port1->inst->id,
                            VAL_BI, port1->attr_mode );
                    dgraph_connect_1( &net->g, cp_sync->id, port2->inst->id,
                            VAL_BI, port2->attr_mode );
                }
                // change left port to copy synchronizer port
                port1->inst = cp_sync;
                // set mode of port
                /* if( port1->attr_mode != port2->attr_mode ) */
                port1->attr_mode = VAL_BI;
                // if possible, set port class to anything but VAL_NONE
                if( port1->attr_class == VAL_NONE )
                    port1->attr_class = port2->attr_class;
                // remove right port from portlist
                free( port2 );
                if( port_last != NULL ) {
                    port_last->next = port_next;
                    port2 = port_last;
                }
                else {
                    v_net2->ports = port_next;
                    break;
                }
            }
            port_last = port2;
            port2 = port2->next;
        }
        port1 = port1->next;
    }
}

/******************************************************************************/
inst_rec* cpsync_merge( inst_net* net, virt_ports* port1, virt_ports* port2 )
{
    int id, id_del;
    id_del = dgraph_merge_vertice_1( &net->g, port1->inst->id,
            port2->inst->id );
    // delete one copy synchronizer from insttab
    id = port2->inst->id;
    inst_rec_del( &net->recs_name, &net->recs_id, port2->inst );
    if( id_del != id )
        inst_rec_replace_id( &net->recs_id, id_del, id );
    // adjust all ids starting from the id of the deleted record
    for( id = id_del + 1; id < igraph_vcount( &net->g ); id++ )
        inst_rec_replace_id( &net->recs_id, id, id - 1 );
    return port1->inst;
}

/******************************************************************************/
void debug_print_port( virt_ports* port )
{
    if( port->attr_mode == VAL_IN ) printf( "in" );
    else if( port->attr_mode == VAL_OUT ) printf( "out" );
    printf( " %s(%d).%s", port->inst->name, port->inst->id, port->rec->name );
}

/******************************************************************************/
void debug_print_ports( virt_net* v_net )
{
    virt_ports* ports = NULL;
    if( v_net->ports != NULL )
        ports = v_net->ports;
    while( ports != NULL ) {
        debug_print_port( ports );
        printf(", ");
        ports = ports->next;
    }
    printf("\n");
}

/******************************************************************************/
virt_net* install_nets( symrec** symtab, inst_net* net,
        UT_array* scope_stack, ast_node* ast )
{
    symrec* rec = NULL;
    virt_net* v_net1 = NULL;
    virt_net* v_net2 = NULL;
    inst_rec* inst = NULL;
    igraph_t g;

    if( ast == NULL ) return NULL;

    switch( ast->type ) {
        case AST_PARALLEL:
            v_net1 = install_nets( symtab, net, scope_stack, ast->op.left );
            v_net2 = install_nets( symtab, net, scope_stack, ast->op.right );
            cpsync_connect( net, v_net1, v_net2 );
            v_net1 = virt_net_alter_parallel( v_net1, v_net2 );
            virt_net_destroy_struct( v_net2 );
#if defined(DEBUG) || defined(DEBUG_CONNECT)
            printf( "Parallel combination, v_net: " );
            debug_print_ports( v_net1 );
            printf("\n");
#endif // DEBUG_CONNECT
            break;
        case AST_SERIAL:
            v_net1 = install_nets( symtab, net, scope_stack, ast->op.left );
            v_net2 = install_nets( symtab, net, scope_stack, ast->op.right );
            // create connection graph
            igraph_empty( &g, igraph_vcount( &net->g ), IGRAPH_UNDIRECTED );
            cgraph_connect_full_ptr( &g, &v_net1->con->right,
                    &v_net2->con->left );
            // check connections and update virtual net
            check_connection( net, v_net1, v_net2, &g );
            cpsync_connect( net, v_net1, v_net2 );
            v_net1 = virt_net_alter_serial( v_net1, v_net2 );
            // cleanup virt net and connection graph
            virt_net_destroy_struct( v_net2 );
            igraph_destroy( &g );
#if defined(DEBUG) || defined(DEBUG_CONNECT)
            printf( "Serial combination, v_net: " );
            debug_print_ports( v_net1 );
            printf("\n");
#endif // DEBUG_CONNECT
            break;
        case AST_ID:
            // check the context of the symbol
            rec = symrec_get( symtab, scope_stack, ast->symbol.name,
                    ast->symbol.line );
            // add a net symbol to the instance table
            if( rec != NULL ) {
                inst = inst_rec_put( &net->recs_name, &net->recs_id,
                        ast->symbol.name, igraph_vcount( &net->g ),
                        ast->symbol.line, VAL_NET, rec );
                // add new vertex to graph
                igraph_add_vertices( &net->g, 1, NULL );
                v_net1 = virt_net_create( rec, inst );
            }
            break;
        default:
            ;
    }
    return v_net1;
}
