/**
 * Checks the context of symbols and connections
 *
 * @file    context.c
 * @author  Simon Maurer
 *
 */

#include "context.h"
#include "defines.h"
#include "smxgraph.h"
#include "smxerr.h"

igraph_vector_ptr_t __rm_cp; // remember removed cp-sync pointers for cleanup

/******************************************************************************/
bool check_connection( virt_port_t* port_l, virt_port_t* port_r, igraph_t* g,
        bool directed, bool ignore_class, bool mode_equal )
{
    instrec_t *inst_l, *inst_r;
    bool res = false;
    char error_msg[ CONST_ERROR_LEN ];
#if defined(DEBUG) || defined(DEBUG_CONNECT)
    printf( "check_connection:\n " );
    debug_print_vport( port_l );
    printf( " and " );
    debug_print_vport( port_r );
#endif // DEBUG_CONNECT
    inst_l = port_l->v_net->inst;
    inst_r = port_r->v_net->inst;
    if( ignore_class || are_port_classes_ok( port_l, port_r, directed ) ) {
        if( !check_connection_on_open_ports( port_l, port_r, false ) )
            return false;
        if( ( inst_l->type == INSTREC_SYNC )
                && ( inst_r->type == INSTREC_SYNC ) ) {
#if defined(DEBUG) || defined(DEBUG_CONNECT)
            printf( "\n  => connection is valid\n" );
#endif // DEBUG_CONNECT
            // either merge copy synchronizers or connect them
            if( check_cpsync_merge_pre_connect( g, port_l, port_r ) )
            {
                propagate_decoupling_attributes( g, port_l, port_r );
                cpsync_merge( port_l, port_r, g );
            }
            else
                connect_ports( port_l, port_r, g, directed );
            res = true;
        }
        else if( ( inst_l->type == INSTREC_SYNC )
                || ( inst_r->type == INSTREC_SYNC )
                || are_port_modes_ok( port_l, port_r, mode_equal ) ) {
#if defined(DEBUG) || defined(DEBUG_CONNECT)
            printf( "\n  => connection is valid\n" );
#endif // DEBUG_CONNECT
            // only change connected status if it is a regular connection and
            // not one invoked from the flatten net
            connect_ports( port_l, port_r, g, directed );
            if( !ignore_class ) {
                port_l->attr_class = PORT_CLASS_DOWN;
                port_r->attr_class = PORT_CLASS_UP;
            }
            res = true;
        }
        else {
#if defined(DEBUG) || defined(DEBUG_CONNECT)
            printf( "\n  => connection is invalid (bad mode)\n" );
#endif // DEBUG_CONNECT
            sprintf( error_msg, ERROR_BAD_MODE, ERR_ERROR, port_l->name,
                    inst_l->name, inst_l->id, inst_r->name, inst_r->id,
                    inst_r->line );
            report_yyerror( error_msg, inst_r->line );
        }
    }
    else {
#if defined(DEBUG) || defined(DEBUG_CONNECT)
        printf( "\n  => connection is invalid (no match)\n" );
#endif // DEBUG_CONNECT
    }
    return res;
}

/******************************************************************************/
void check_connection_cp( virt_net_t* v_net, virt_port_t* port1,
        virt_port_t* port2, igraph_t* g, node_type_t parallel,
        time_criticality_t tc )
{
    char error_msg[ CONST_ERROR_LEN ];
    bool b_parallel = ( ( parallel == AST_PARALLEL )
            || ( parallel == AST_PARALLEL_DET ) );
    struct timespec tb;
    tb.tv_sec = 0;
    tb.tv_nsec = 0;
    virt_net_t* v_net_sync = NULL;
    instrec_t* inst1 = port1->v_net->inst;
    instrec_t* inst2 = port2->v_net->inst;
    virt_port_t* port_new;
    port_class_t port_class;
    port_mode_t port_mode;
#if defined(DEBUG) || defined(DEBUG_CONNECT)
    printf( "check_connection_cp:\n " );
    debug_print_vport( port1 );
    printf( " and " );
    debug_print_vport( port2 );
#endif // DEBUG_CONNECT
    // if possible, set port class to anything but PORT_CLASS_NONE
    if( port1->attr_class == PORT_CLASS_NONE )
        port_class = port2->attr_class;
    else if( port2->attr_class == PORT_CLASS_NONE )
        port_class = port1->attr_class;
    if ( are_port_cp_classes_ok( port1, port2, b_parallel ) ) {
        if( !check_connection_on_open_ports( port1, port2, b_parallel ) )
            return;
#if defined(DEBUG) || defined(DEBUG_CONNECT)
        printf( "\n  => connection is valid\n" );
#endif // DEBUG_CONNECT
        // check modes for parallel connection on non-sied-ports
        if( ( port1->attr_class != PORT_CLASS_SIDE )
                && ( port2->attr_class != PORT_CLASS_SIDE ) ) {
            if( ( parallel == AST_PARALLEL )
                    && are_port_modes_ok( port1, port2, false ) ) {
                sprintf( error_msg, ERROR_BAD_MODE, ERR_ERROR, port1->name,
                        port1->v_net->inst->name, port1->v_net->inst->id,
                        port2->v_net->inst->name, port2->v_net->inst->id,
                        port2->symb->line );
                report_yyerror( error_msg, port1->symb->line );
                return;
            }
            else if( ( parallel == AST_PARALLEL_DET )
                && ( port1->attr_mode == PORT_MODE_OUT )
                && ( port2->attr_mode == PORT_MODE_OUT ) ) {
                sprintf( error_msg, ERROR_NONDET, ERR_ERROR,
                        port2->v_net->inst->name, port1->v_net->inst->name );
                report_yyerror( error_msg, port1->symb->line );
                return;
            }
        }
        // connect ports and cerate copy synchronizer if necessary
        if( ( inst1->type == INSTREC_SYNC )
                && ( inst2->type == INSTREC_SYNC ) ) {
            // merge copy synchronizers
            cpsync_merge( port1, port2, g );
        }
        else if( ( inst1->type == INSTREC_SYNC )
                || ( inst2->type == INSTREC_SYNC ) ) {
            connect_ports( port1, port2, g, false );
        }
        else {
            // create copy synchronizer instance and update the graph
            if( port1->attr_class == port2->attr_class )
                port_class = port1->attr_class;
            else port_class = PORT_CLASS_NONE;
            if( port1->attr_mode == port2->attr_mode )
                port_mode = port1->attr_mode;
            else port_mode = PORT_MODE_BI;
            v_net_sync = dgraph_vertex_add_sync( g, TEXT_CP );
            dgraph_vertex_add_attr_tt( g, v_net_sync->inst->id,
                    get_time_criticality_prio( tc, false ) );
            // channel length is set to zero: the connecting box port might
            // overwrite this due to the max function
            port_new = virt_port_create( port_class, port_mode, v_net_sync,
                    port1->name, port1->symb, tb, TIME_NONE, false, false,
                    false, 0 );
            virt_port_append( v_net_sync, port_new );
            virt_port_append( v_net, port_new );
            connect_ports( port_new, port1, g, false );
            connect_ports( port_new, port2, g, false );
        }
    }
    else {
#if defined(DEBUG) || defined(DEBUG_CONNECT)
        printf( "\n  => connection is invalid (no match)\n" );
#endif // DEBUG_CONNECT
    }
}

/******************************************************************************/
void check_connection_cp_net( virt_port_t* port1, virt_port_t* port2,
        igraph_t* g )
{
    instrec_t* inst1 = port1->v_net->inst;
    instrec_t* inst2 = port2->v_net->inst;
    if( ( port1->attr_class == PORT_CLASS_SIDE )
            && ( port2->attr_class == PORT_CLASS_SIDE ) )
        return;
    if( port1->attr_class == port2->attr_class
            && port1->attr_mode == port2->attr_mode
            && ( inst1->type == INSTREC_SYNC || inst2->type == INSTREC_SYNC )
            && ( inst1->type != inst2->type ) )
        connect_ports( port1, port2, g, false );
}

/******************************************************************************/
void check_connection_missing( virt_net_t* v_net_l, virt_net_t* v_net_r,
        igraph_t* g, bool is_prop )
{
    int i, j;
    char error_msg[ CONST_ERROR_LEN ];
    igraph_vs_t vs1, vs2;
    igraph_vector_t v1, v2;
    instrec_t *inst1, *inst2;
    igraph_matrix_t res1, res2;
    igraph_vector_init( &v1, 0 );
    igraph_vector_init( &v2, 0 );
    dgraph_vptr_to_v( &v_net_l->con->right, &v1 );
    dgraph_vptr_to_v( &v_net_r->con->left, &v2 );
    igraph_matrix_init( &res1, igraph_vector_size( &v1 ),
            igraph_vector_size( &v2 ) );
    igraph_matrix_init( &res2, igraph_vector_size( &v1 ),
            igraph_vector_size( &v2 ) );
    igraph_vs_vector( &vs1, &v1 );
    igraph_vs_vector( &vs2, &v2 );
    igraph_shortest_paths( g, &res1, vs1, vs2, IGRAPH_OUT );
    igraph_shortest_paths( g, &res2, vs1, vs2, IGRAPH_IN );

    for( i=0; i<igraph_vector_size( &v1 ); i++ ) {
        for( j=0; j<igraph_vector_size( &v2 ); j++ ) {
#if defined(DEBUG) || defined(DEBUG_CONNECT_MISSING)
            printf(" check ids %d and %d\n", ( int )VECTOR( v1 )[i],
                    ( int )VECTOR( v2 )[j]);
#endif // DEBUG_CONNECT_MISSING
            if( ( MATRIX( res1, i, j ) > 2 ) && ( MATRIX( res2, i, j ) > 2 ) ) {
                inst1 = VECTOR( v_net_l->con->right )[i];
                inst2 = VECTOR( v_net_r->con->left )[j];
                // ERROR: there is no connection between the two nets
                if( is_prop )
                    sprintf( error_msg, WARNING_NO_NET_CON, ERR_WARNING,
                            inst1->name, inst1->id, inst2->name, inst2->id );
                else
                    sprintf( error_msg, ERROR_NO_NET_CON, ERR_ERROR,
                            inst1->name, inst1->id, inst2->name, inst2->id );
                report_yyerror( error_msg, inst1->line );
            }
        }
    }

    igraph_vector_destroy( &v1 );
    igraph_vector_destroy( &v2 );
    igraph_matrix_destroy( &res1 );
    igraph_matrix_destroy( &res2 );
    igraph_vs_destroy( &vs1 );
    igraph_vs_destroy( &vs1 );
}

/******************************************************************************/
bool check_connection_on_open_ports( virt_port_t* port_l, virt_port_t* port_r,
        bool is_parallel )
{
    instrec_t *inst_l, *inst_r;
    char error_msg[ CONST_ERROR_LEN ];
    inst_l = port_l->v_net->inst;
    inst_r = port_r->v_net->inst;

    if( port_r->is_open || port_l->is_open )
    {
#if defined(DEBUG) || defined(DEBUG_CONNECT)
        printf( "\n  => connection is invalid (port is open)\n" );
#endif // DEBUG_CONNECT
        if( port_r->is_open != port_l->is_open )
        {
            if( !is_parallel && port_r->is_open )
            {
                sprintf( error_msg, ERROR_CONNECT_OPEN, ERR_ERROR, port_r->name,
                        inst_r->name, inst_r->id );
                report_yyerror( error_msg, inst_r->line );
            }
            if( !is_parallel && port_l->is_open )
            {
                sprintf( error_msg, ERROR_CONNECT_OPEN, ERR_ERROR, port_l->name,
                        inst_l->name, inst_l->id );
                report_yyerror( error_msg, inst_l->line );
            }
        }
        return false;
    }
    return true;
}

/******************************************************************************/
void check_connections( virt_net_t* v_net1, virt_net_t* v_net2, igraph_t* g )
{
    virt_port_list_t* ports_l = NULL;
    virt_port_list_t* ports_r = NULL;
    bool res = false;

    ports_l = v_net1->ports;
    while( ports_l != NULL ) {
        ports_r = v_net2->ports;
        while( ports_r != NULL ) {
            if( ( ports_l->port->state < VPORT_STATE_CONNECTED )
                    && ( ports_r->port->state < VPORT_STATE_CONNECTED )
                    && are_port_names_ok( ports_l->port, ports_r->port ) ) {
                // direction matters, class matters, modes have to be different
                res = check_connection( ports_l->port, ports_r->port, g,
                        true, false, false );
                if( res ) break;
            }
            ports_r = ports_r->next;
        }
        ports_l = ports_l->next;
    }
}

/******************************************************************************/
void check_connections_cp( virt_net_t* v_net, igraph_t* g,
        node_type_t parallel, time_criticality_t tc )
{
    virt_port_list_t* ports1 = NULL;
    virt_port_list_t* ports2 = NULL;
    int new_idx = 0;
    if( v_net->ports != NULL ) new_idx = v_net->ports->idx;

    // in the list of ports, test each combination only once and do not compare
    // a port with itself (we use the new_idx counter for this) note that the
    // port indices ports->idx start from the highest value
    ports1 = v_net->ports;
    while( ports1 != NULL ) {
        ports2 = v_net->ports;
        while( ports2 != NULL ) {
            if( ( ports2->idx < new_idx )
                    && ( ports1->port->state < VPORT_STATE_CONNECTED )
                    && ( ports2->port->state < VPORT_STATE_CONNECTED )
                    && are_port_names_ok( ports1->port, ports2->port ) ) {
                check_connection_cp( v_net, ports1->port, ports2->port, g,
                        parallel, tc );
            }
            ports2 = ports2->next;
        }
        ports1 = ports1->next;
        new_idx--;
    }
#if defined(DEBUG) || defined(DEBUG_CONNECT)
    printf( "check_connections_cp, updated v_net: " );
    debug_print_vports( v_net );
#endif // DEBUG
}

/******************************************************************************/
void check_connections_open( virt_net_t* vnet_l, virt_net_t* vnet_r )
{
    char error_msg[ CONST_ERROR_LEN ];
    virt_port_list_t* ports = vnet_l->ports;
    instrec_t *inst;
    while( ports != NULL ) {
        if( ( ports->port->state < VPORT_STATE_CONNECTED )
                && ( ports->port->attr_class == PORT_CLASS_DOWN )
                && !ports->port->is_open ) {
            // a left opernad must have all ports with class down connected
            inst = ports->port->v_net->inst;
            sprintf( error_msg, ERROR_NO_PORT_CON_CLASS, ERR_ERROR,
                    ports->port->name, inst->name, inst->id, inst->name, "*" );
            report_yyerror( error_msg, ports->port->symb->line );
            ports->port->state = VPORT_STATE_DISABLED;
        }
        ports = ports->next;
    }
    ports = vnet_r->ports;
    while( ports != NULL ) {
        if( ( ports->port->state < VPORT_STATE_CONNECTED )
                && ( ports->port->attr_class == PORT_CLASS_UP )
                && !ports->port->is_open ) {
            // a right opernad must have all ports with class up connected
            inst = ports->port->v_net->inst;
            sprintf( error_msg, ERROR_NO_PORT_CON_CLASS, ERR_ERROR,
                    ports->port->name, inst->name, inst->id, inst->name, "*" );
            report_yyerror( error_msg, ports->port->symb->line );
            ports->port->state = VPORT_STATE_DISABLED;
        }
        ports = ports->next;
    }
}

/******************************************************************************/
void check_connections_self( igraph_t* g, virt_net_t* v_net )
{
    virt_port_list_t* ports1 = NULL;
    virt_port_list_t* ports2 = NULL;
    int new_idx = 0;
    if( v_net->ports != NULL ) new_idx = v_net->ports->idx;

    // in the list of ports, test each combination only once and do not compare
    // a port with itself (we use the new_idx counter for this) not that the
    // port indices ports->idx start from the highest value
    ports1 = v_net->ports;
    while( ports1 != NULL ) {
        ports2 = v_net->ports;
        while( ports2 != NULL ) {
            if( ( ports2->idx < new_idx )
                    && ( ports1->port->state < VPORT_STATE_CONNECTED )
                    && ( ports2->port->state < VPORT_STATE_CONNECTED )
                    && ( ports1->port->attr_class == PORT_CLASS_NONE )
                    && ( ports2->port->attr_class == PORT_CLASS_NONE )
                    && are_port_modes_ok( ports1->port, ports2->port, false )
                    && are_port_names_ok( ports1->port, ports2->port ) ) {
                connect_ports( ports1->port, ports2->port, g, true );
            }
            ports2 = ports2->next;
        }
        ports1 = ports1->next;
        new_idx--;
    }
#if defined(DEBUG) || defined(DEBUG_CONNECT)
    printf( "check_connections_self, updated v_net: " );
    debug_print_vports( v_net );
#endif // DEBUG
}

/******************************************************************************/
void check_context( ast_node_t* ast, symrec_t** symtab, igraph_t* g )
{
    UT_array* scope_stack = NULL; // stack to handle the scope
    attr_net_t* n_attr = NULL;
    int scope = 0;
    igraph_t g_tmp;
    int i;

    igraph_vector_ptr_init( &__rm_cp, 0 );
    utarray_new( scope_stack, &ut_int_icd );
    utarray_push_back( scope_stack, &scope );
    n_attr = check_context_ast( symtab, scope_stack, ast, &scope );

    utarray_free( scope_stack );
    if( n_attr->v_net != NULL ) {
        // flatten graph and detect open ports
        igraph_empty( &g_tmp, 0, IGRAPH_DIRECTED );
        dgraph_append( &g_tmp, &n_attr->g, true );
        dgraph_flatten( g, &g_tmp );
        post_process( g );
        igraph_destroy( &g_tmp );
    }

    // cleanup
    symrec_attr_destroy_net( n_attr, true );
    for( i = 0; i < igraph_vector_ptr_size( &__rm_cp ); i++ ) {
        virt_net_destroy( VECTOR( __rm_cp )[i], false );
    }
    igraph_vector_ptr_destroy( &__rm_cp );
}

/******************************************************************************/
void* check_context_ast( symrec_t** symtab, UT_array* scope_stack,
        ast_node_t* ast, int* scope )
{
    ast_list_t* list = NULL;
    attr_box_t* b_attr = NULL;
    attr_net_t* n_attr = NULL;
    attr_prot_t* np_attr = NULL;
    attr_port_t* p_attr = NULL;
    attr_wrap_t* w_attr = NULL;
    void* attr = NULL;
    symrec_t* rec = NULL;
    symrec_list_t* ptr = NULL;
    symrec_list_t* port_list = NULL;
    symrec_list_t* port_list_net = NULL;
    virt_net_t* v_net;
    void* res = NULL;
    igraph_t g_net;

    if( ast == NULL ) return NULL;

    switch( ast->type ) {
        case AST_PROGRAM:
            // install net instances
            check_context_ast( symtab, scope_stack, ast->program->stmts, scope );
            res = check_context_ast( symtab, scope_stack, ast->program->net, scope );
            break;
        case AST_STMTS:
            list = ast->list;
            while (list != NULL) {
                check_context_ast( symtab, scope_stack, list->node, scope );
                list = list->next;
            }
            break;
        case AST_ASSIGN:
            // get the attributes
            attr = check_context_ast( symtab, scope_stack, ast->assign->op, scope );
            if( ast->assign->type == AST_NET ) {
                // check prototype if available
                rec = symrec_search( symtab, scope_stack,
                        ast->assign->id->symbol->name, 0 );
                if( rec == NULL ) {
                    // no prototype, install the symbol
                    rec = symrec_create_net( ast->assign->id->symbol->name,
                        *( int* )utarray_back( scope_stack ),
                        ast->assign->id->symbol->line, attr );
                    res = symrec_put( symtab, rec );
                }
                // check whether types of prototype and definition match
                else if( check_prototype( rec->attr_proto->ports,
                            ( ( attr_net_t* )attr )->v_net, rec->name ) ) {
                    // match -> update record attributes to the real net
                    rec->type = SYMREC_NET;
                    symrec_attr_destroy_proto( rec->attr_proto );
                    rec->attr_net = attr;
                    rec->line = ast->assign->id->symbol->line;
                }
                else {
                    symrec_attr_destroy_net( attr, true );
                }
            }
            if( ast->assign->type == AST_BOX ) {
                rec = symrec_create_box( ast->assign->id->symbol->name,
                        *( int* )utarray_back( scope_stack ),
                        ast->assign->id->symbol->line, attr );
                // install the symbol
                if( symrec_put( symtab, rec ) == NULL ) {
                    symrec_attr_destroy_box( rec->attr_box );
                    symrec_destroy( rec );
                }
            }
            break;
        case AST_NET_PROTO:
            ( *scope )++;
            utarray_push_back( scope_stack, scope );
            port_list = ( symrec_list_t* )check_context_ast( symtab,
                    scope_stack, ast->proto->ports, scope );
            utarray_pop_back( scope_stack );
            // prepare symbol attribute and create symbol
            np_attr = symrec_attr_create_proto( port_list );
            rec = symrec_create_proto( ast->proto->id->symbol->name,
                    *( int* )utarray_back( scope_stack ), ast->proto->id->symbol->line,
                    np_attr );
            // install the symbol (use port list as attributes)
            res = ( void* )symrec_put( symtab, rec );
            if( res == NULL ) {
                symrec_attr_destroy_proto( rec->attr_proto );
                symrec_destroy( rec );
            }
            break;
        case AST_NET:
            igraph_empty( &g_net, 0, true );
            v_net = ( void* )install_nets( symtab, scope_stack,
                    ast->network->net, &g_net, TIME_CTITICALITY_NONE );
            n_attr = symrec_attr_create_net( v_net, &g_net );
#if defined(DEBUG) || defined(DEBUG_NET_DOT)
            igraph_write_graph_dot( &g_net, stdout );
#endif // DEBUG
            res = ( void* )n_attr;
            break;
        case AST_WRAP:
            ( *scope )++;
            utarray_push_back( scope_stack, scope );
            n_attr = check_context_ast( symtab, scope_stack, ast->wrap->stmts, scope );
            if( n_attr == NULL ) return NULL;
            port_list = ( symrec_list_t* )check_context_ast( symtab,
                    scope_stack, ast->wrap->ports_wrap, scope );
            ( *scope )++;
            utarray_push_back( scope_stack, scope );
            port_list_net = ( symrec_list_t* )check_context_ast( symtab,
                    scope_stack, ast->wrap->ports_net, scope );
            utarray_pop_back( scope_stack );
            utarray_pop_back( scope_stack );

            // prepare symbol attributes and create symbol
            w_attr = symrec_attr_create_wrap( false, port_list, NULL, NULL );
            if( ast->wrap->attr_static != NULL ) w_attr->attr_static = true;
            rec = symrec_create_wrap( ast->wrap->id->symbol->name,
                    *( int* )utarray_back( scope_stack ), ast->wrap->id->symbol->line,
                    w_attr );
            if( check_prototype( port_list_net, n_attr->v_net, rec->name ) ) {
                // create virtual port list of the prototyped net with instances
                // of the real net
                igraph_copy( &g_net, &n_attr->g );
                v_net = wrap_connect_int( port_list, n_attr->v_net, &g_net );
                rec->attr_wrap->v_net = v_net;
                rec->attr_wrap->g = g_net;
#if defined(DEBUG) || defined(DEBUG_CONNECT_WRAP)
                printf( "check_contect_ast: wrap: \n" );
                debug_print_vports( v_net );
#endif // DEBUG
            }
            // cleanup
            symrec_attr_destroy_net( n_attr, false );
            symrec_list_del( port_list_net );
            // install the wrapper symbol in the scope of its declaration
            res = ( void* )symrec_put( symtab, rec );

            if( res == NULL ) {
                symrec_attr_destroy_wrap( rec->attr_wrap );
                symrec_destroy( rec );
                return NULL;
            }
            break;
        case AST_BOX:
            ( *scope )++;
            utarray_push_back( scope_stack, scope );
            port_list = ( symrec_list_t* )check_context_ast( symtab,
                    scope_stack, ast->box->ports, scope );
            utarray_pop_back( scope_stack );
            // prepare symbol attributes and create symbol
            b_attr = symrec_attr_create_box( false, LOCATION_LOCAL,
                    ast->box->impl->symbol->name, port_list );
            if( ast->box->attr_pure != NULL ) b_attr->attr_pure = true;
            if( ast->box->attr_location != NULL ) {
                if( ast->box->attr_location->attr->val == PARSE_ATTR_EXTERN )
                {
                    b_attr->attr_location = LOCATION_EXTERN;
                }
                else if( ast->box->attr_location->attr->val
                        == PARSE_ATTR_INTERN )
                {
                    b_attr->attr_location = LOCATION_INTERN;
                }
            }
            // return box attributes
            res = ( void* )b_attr;
            break;
        case AST_PORTS:
        case AST_INT_PORTS:
            list = ast->list;
            while (list != NULL) {
                res = check_context_ast( symtab, scope_stack, list->node, scope );
                if( res != NULL ) {
                    ptr = ( symrec_list_t* )malloc( sizeof( symrec_list_t ) );
                    ptr->rec = ( symrec_t* )res;
                    ptr->next = port_list;
                    port_list = ptr;
                }
                list = list->next;
            }
            /* check_ports_decoupled( port_list ); */
            res = ( void* )port_list;   // return pointer to the port list
            break;
        case AST_PORT:
            // prepare symbol attributes and create symbol
            p_attr = symrec_attr_create_port( NULL, PORT_MODE_BI,
                    PORT_CLASS_NONE, false, false, false, 0 );
            if( ast->port->ch_len != NULL )
                p_attr->ch_len = ast->port->ch_len->attr->val;
            if( ast->port->mode != NULL )
                p_attr->mode = ast->port->mode->attr->val;
            if( ast->port->collection != NULL )
                p_attr->collection = ast->port->collection->attr->val;
            if( ast->port->connection != NULL )
            {
                p_attr->is_open = ( ast->port->connection->attr->val
                        == PARSE_ATTR_OPEN );
                p_attr->is_dynamic = ( ast->port->connection->attr->val
                        == PARSE_ATTR_DYNAMIC );
            }
            if( ( ast->port->coupling != NULL
                        && ast->port->coupling->attr->val == PARSE_ATTR_DECOUPLED )
                    || ( ( p_attr->collection == PORT_CLASS_SIDE )
                        && ( p_attr->mode == PORT_MODE_OUT )
                        && ( ast->port->coupling == NULL ) ) )
                p_attr->decoupled = true;
            if( ( ast->port->int_id != NULL )
                    && ( ast->port->int_id->type == AST_INT_PORTS ) ) {
                // internal port list
                ( *scope )++;
                utarray_push_back( scope_stack, scope );
                p_attr->ports_int = ( symrec_list_t* )check_context_ast( symtab,
                        scope_stack, ast->port->int_id, scope );
                utarray_pop_back( scope_stack );
            }
            else if( ( ast->port->int_id != NULL )
                    && ( ast->port->int_id->type == AST_ID ) ) {
                // alternative port name
                p_attr->alt_name = ast->port->int_id->symbol->name;
            }
            rec = symrec_create_port( ast->port->id->symbol->name,
                    *( int* )utarray_back( scope_stack ), ast->port->id->symbol->line,
                    p_attr );
            // install symbol and return pointer to the symbol record
            res = ( void* )symrec_put( symtab, rec );
            if( res == NULL ) {
                symrec_attr_destroy_port( rec->attr_port );
                symrec_destroy( rec );
            }
            break;
        default:
            ;
    }
    return res;
}

/******************************************************************************/
bool check_cpsync_merge_post_connect( igraph_t* g, int eid )
{
    bool res;
    int id_l;
    int id_r;
    igraph_vector_t in, out;
    igraph_vs_t vs;

    igraph_edge( g, eid, &id_l, &id_r );

    igraph_vs_vector_small( &vs, id_l, id_r, -1 );
    igraph_vector_init( &in, 0 );
    igraph_vector_init( &out, 0 );
    igraph_degree( g, &in, vs, IGRAPH_IN, false );
    igraph_degree( g, &out, vs, IGRAPH_OUT, false );
    igraph_vs_destroy( &vs );

    VECTOR(out)[0]--;
    VECTOR(in)[1]--;

    res = check_cpsync_merge( VECTOR(out)[0], VECTOR(in)[0], VECTOR(out)[1],
            VECTOR(in)[1], true, false );
    igraph_vector_destroy( &in );
    igraph_vector_destroy( &out );

    return res;
}

/******************************************************************************/
bool check_cpsync_merge_pre_connect( igraph_t* g, virt_port_t* port_l,
        virt_port_t* port_r )
{
    bool res;
    int id_l = port_l->v_net->inst->id;
    int id_r = port_r->v_net->inst->id;
    port_mode_t mode_l = port_l->attr_mode;
    port_mode_t mode_r = port_r->attr_mode;
    igraph_vector_t in, out;
    igraph_vs_t vs;
    int deg = 0;

    if( mode_l == PORT_MODE_BI && mode_r == PORT_MODE_BI )
        return true;

    // out:in, out:out
    bool l2r = ( mode_l == PORT_MODE_OUT );
    // in:out, in:in
    bool r2l = ( mode_l == PORT_MODE_IN );

    if( id_l == id_r )
        return false;

    // This following part is a bit messy. It is used to get the current degree
    // of teh copy sync vertex. However, the graph and its corresponding the
    // virtual net may have non-matching degrees. This is due to the following:
    // 1. Side-port conncetions only have one port in a vnet however the graph
    //    represents all available connections.
    // 2. When flattening a wrapper the connections due to port renaming are
    //    happening one at a time, hence the graph degree is lower than the vnet
    //    degree
    // Hence both, the vnet and the graph is used to compute the degree and the
    // max is taken.
    igraph_vs_vector_small(&vs, id_l, id_r, -1);
    igraph_vector_init( &in, 0 );
    igraph_vector_init( &out, 0 );
    igraph_degree( g, &in, vs, IGRAPH_IN, false );
    igraph_degree( g, &out, vs, IGRAPH_OUT, false );
    igraph_vs_destroy( &vs );

    // Note that in case of the vnet, the potentiually connecting port is not
    // taken into account (hence the substraction).
    deg = virt_net_get_indegree( port_l->v_net ) - (mode_l == PORT_MODE_IN);
    if( VECTOR(in)[0] < deg )
        VECTOR(in)[0] = deg;
    deg = virt_net_get_indegree( port_r->v_net ) - (mode_r == PORT_MODE_IN);
    if( VECTOR(in)[1] < deg )
        VECTOR(in)[1] = deg;
    deg = virt_net_get_outdegree( port_l->v_net ) - (mode_l == PORT_MODE_OUT);
    if( VECTOR(out)[0] < deg )
        VECTOR(out)[0] = deg;
    deg = virt_net_get_outdegree( port_r->v_net ) - (mode_r == PORT_MODE_OUT);
    if( VECTOR(out)[1] < deg )
        VECTOR(out)[1] = deg;

    res = check_cpsync_merge( VECTOR(out)[0], VECTOR(in)[0], VECTOR(out)[1],
            VECTOR(in)[1], l2r, r2l );

    igraph_vector_destroy( &in );
    igraph_vector_destroy( &out );
    return res;
}

/******************************************************************************/
bool check_cpsync_merge( double v1_out, double v1_in, double v2_out,
        double v2_in, bool l2r, bool r2l )
{
    bool res = false;
    // The following rules define whether two cp_sync nets may be merged: Two
    // cp_syncs can only be merged if the possible paths before and after the
    // merging remain the same.
    if( v1_out == 0 && v2_out == 0 )
        res = true;
    else if( v1_in == 0 && v2_in == 0 )
        res = true;
    else if( v1_out == 0 && v2_in == 0 && l2r )
        res = true;
    else if( v1_in == 0 && v2_out == 0 && r2l )
        res = true;
    else if( v1_out > 0 && v1_in > 0 && v2_in == 0 && l2r )
        res = true;
    else if( v1_in == 0 && v2_out > 0 && v2_in > 0 && r2l )
        res = true;
    else if( v1_out == 0 && v2_in > 0 && v2_out > 0 && l2r )
        res = true;
    else if( v1_in > 0 && v1_out > 0 && v2_out == 0 && r2l )
        res = true;

    return res;
}

/******************************************************************************/
void check_ports_decoupled( symrec_list_t* ports )
{
    char error_msg[ CONST_ERROR_LEN ];
    int input_cnt = 0;
    int decoupled_cnt = 0;
    int line;

    while( ports != NULL ) {
        if( ports->rec->attr_port->mode == PORT_MODE_IN ) {
            input_cnt++;
            if( ports->rec->attr_port->decoupled ) {
                decoupled_cnt++;
                line = ports->rec->line;
            }
        }
        ports = ports->next;
    }
    if( input_cnt &&  ( input_cnt == decoupled_cnt ) ) {
        sprintf( error_msg, ERROR_ALL_IN_DEC, ERR_ERROR );
        report_yyerror( error_msg, line );
    }
}

/******************************************************************************/
void check_ports_open( virt_net_t* v_net )
{
    virt_port_list_t* ports;
    char error_msg[ CONST_ERROR_LEN ];

    ports = v_net->ports;
    while( ports != NULL ) {
        if( ports->port->state == VPORT_STATE_OPEN && !ports->port->is_open ) {
            sprintf( error_msg, ERROR_NO_PORT_CON, ERR_ERROR, ports->port->name,
                    v_net->inst->name, v_net->inst->id );
            report_yyerror( error_msg, v_net->inst->line );
        }
        ports = ports->next;
    }
}

/******************************************************************************/
bool check_prototype( symrec_list_t* r_ports, virt_net_t* v_net, char *name )
{
    char error_msg[ CONST_ERROR_LEN ];
    bool res = true;

#if defined(DEBUG) || defined(DEBUG_PROTO)
    printf( "check_prototype:\n" );
    printf( " ports prot: " );
    debug_print_rports( r_ports, name );
    printf( " ports vnet: " );
    debug_print_vports( v_net );
#endif // DEBUG_PROTO
    if( !do_port_cnts_match( r_ports, v_net->ports )
        || !do_port_attrs_match( r_ports, v_net->ports ) ) {
        sprintf( error_msg, ERROR_TYPE_CONFLICT, ERR_ERROR, name );
        report_yyerror( error_msg, r_ports->rec->line );
        printf( " net:\n  " );
        debug_print_vports_s( v_net, false );
        printf( " prototype:\n  " );
        debug_print_rports( r_ports, name );
        res = false;
    }

    return res;
}

/******************************************************************************/
bool check_single_mode_cp( igraph_t* g, int id )
{
    bool res = true;
    igraph_vector_t eids_in;
    igraph_vector_t eids_out;
    char error_msg[ CONST_ERROR_LEN ];

    igraph_vector_init( &eids_in, 0 );
    igraph_incident( g, &eids_in, id, IGRAPH_IN );
    igraph_vector_init( &eids_out, 0 );
    igraph_incident( g, &eids_out, id, IGRAPH_OUT );
    if( ( igraph_vector_size( &eids_in ) == 0 )
            || ( igraph_vector_size( &eids_out ) == 0 ) ) {
        sprintf( error_msg, ERROR_SMODE_CP, ERR_ERROR, TEXT_CP, id );
        report_yyerror( error_msg, 0 );
        res = false;
    }
    igraph_vector_destroy( &eids_in );
    igraph_vector_destroy( &eids_out );
    return res;
}

/******************************************************************************/
void connect_ports( virt_port_t* port_l, virt_port_t* port_r, igraph_t* g,
        bool connect_sync )
{
    const char* name = port_l->name;
    instrec_type_t inst_type_l = port_l->v_net->inst->type;
    instrec_type_t inst_type_r = port_r->v_net->inst->type;
    if( inst_type_r != INSTREC_SYNC ) name = port_r->name;
    virt_port_t *p_src, *p_dest;
    // set source and dest id
    if( ( inst_type_l == INSTREC_SYNC ) && ( inst_type_r == INSTREC_SYNC ) )
    {
        // connect two cp_sync nets
        if( ( ( port_l->attr_mode == PORT_MODE_IN
                        || port_l->attr_mode == PORT_MODE_BI )
                    && ( port_r->attr_mode == PORT_MODE_OUT
                        || port_r->attr_mode == PORT_MODE_BI ) )
                || ( ( port_l->attr_mode == PORT_MODE_IN )
                        && ( port_r->attr_mode == PORT_MODE_IN ) ) ) {
            // both are input or the connection is r2l
            p_dest = port_l;
            p_src = port_r;
        }
        else
        {
            // both are output or the connection is r2l
            p_dest = port_r;
            p_src = port_l;
        }
    }
    else if( ( ( port_l->attr_mode == PORT_MODE_IN )
                && ( port_r->attr_mode == PORT_MODE_OUT ) )
            || ( ( inst_type_l == INSTREC_SYNC )
                && ( port_r->attr_mode == PORT_MODE_OUT ) )
            || ( ( inst_type_r == INSTREC_SYNC )
                && ( port_l->attr_mode == PORT_MODE_IN ) ) ) {
        p_dest = port_l;
        p_src = port_r;
    }
    else if( ( ( port_l->attr_mode == PORT_MODE_OUT )
                && ( port_r->attr_mode == PORT_MODE_IN ) )
            || ( ( inst_type_l == INSTREC_SYNC )
                && ( port_r->attr_mode == PORT_MODE_IN ) )
            || ( ( inst_type_r == INSTREC_SYNC )
                && ( port_l->attr_mode == PORT_MODE_OUT ) ) ) {
        p_dest = port_r;
        p_src = port_l;
    }
    else return;

    // set port state
    if( ( inst_type_l != INSTREC_SYNC ) || connect_sync )
        port_l->state = VPORT_STATE_CONNECTED;
    if( ( inst_type_r != INSTREC_SYNC ) || connect_sync )
        port_r->state = VPORT_STATE_CONNECTED;
    // add edge to the graph and set attributes
    dgraph_edge_add( g, p_src, p_dest, name );
}

/******************************************************************************/
void cpsync_merge( virt_port_t* port1, virt_port_t* port2, igraph_t* g )
{
    int id_del;
    virt_net_t *v_net1 = port1->v_net;
    virt_net_t *v_net2 = port2->v_net;
#if defined(DEBUG) || defined(DEBUG_CONNECT)
    printf( "Merge %s(%d) and %s(%d)\n", v_net1->inst->name, v_net1->inst->id,
            v_net2->inst->name, v_net2->inst->id );
#endif // DEBUG_CONNECT
    id_del = dgraph_vertex_merge( g, v_net1->inst->id, v_net2->inst->id );
    // delete one copy synchronizer
    if( id_del == v_net1->inst->id ) {
        port1->state = VPORT_STATE_DISABLED;
        port2->state = VPORT_STATE_CP_OPEN;
        virt_port_append_all( v_net2, v_net1, true );
        igraph_vector_ptr_push_back( &__rm_cp, v_net1 );
    }
    else {
        port1->state = VPORT_STATE_CP_OPEN;
        port2->state = VPORT_STATE_DISABLED;
        virt_port_append_all( v_net1, v_net2, true );
        igraph_vector_ptr_push_back( &__rm_cp, v_net2 );
    }
    // adjust all ids starting from the id of the deleted record
    dgraph_vertex_update_ids( g, id_del );
}

/******************************************************************************/
bool cpsync_reduce( igraph_t* g, int id )
{
    bool res = false;
    virt_port_t *p_src, *p_dest, *p_from, *p_to;
    igraph_vector_t eids;
    const char* name;
    symrec_t* port_symb;

    igraph_vector_init( &eids, 0 );
    igraph_incident( g, &eids, id, IGRAPH_ALL );
    if( igraph_vector_size( &eids ) == 2 ) {
        p_src = ( virt_port_t* )( uintptr_t ) igraph_cattribute_EAN( g,
                GE_PSRC, VECTOR( eids )[ 0 ] );
        p_dest = ( virt_port_t* )( uintptr_t ) igraph_cattribute_EAN( g,
                GE_PDST, VECTOR( eids )[ 0 ] );
        // usually the symb entry of a cp-sync is NULL but if it is a wrapper
        // cp-sync, symb points to an external port symbol. It is a hack and I
        // I will probably hate myself for this at some point in the future.
        port_symb = ( symrec_t* )( uintptr_t )igraph_cattribute_VAN( g,
                GV_SYMB, id );
        // get id of the non net end of the edge
        if( p_dest->v_net->inst->id != id ) p_to = p_dest;
        else p_from = p_src;
        p_src = ( virt_port_t* )( uintptr_t ) igraph_cattribute_EAN( g,
                GE_PSRC, VECTOR( eids )[ 1 ] );
        p_dest = ( virt_port_t* )( uintptr_t ) igraph_cattribute_EAN( g,
                GE_PDST, VECTOR( eids )[ 1 ] );
        // get id of the non net end of the edge
        if( p_dest->v_net->inst->id != id ) { p_to = p_dest; }
        else { p_from = p_src; }
        // set name
        if( port_symb != NULL ) name = port_symb->name;
        else name = p_from->name;
        dgraph_edge_add( g, p_from, p_to, name );
        res = true;
    }
    igraph_vector_destroy( &eids );
    return res;
}

/******************************************************************************/
bool do_port_cnts_match( symrec_list_t* r_ports, virt_port_list_t* v_ports )
{
    symrec_list_t* r_port_ptr = r_ports;
    virt_port_list_t* v_port_ptr = v_ports;
    int v_count = 0;
    int r_count = 0;

    while( r_port_ptr != NULL  ) {
        if( !r_port_ptr->rec->attr_port->is_open ) r_count++;
        r_port_ptr = r_port_ptr->next;
    }

    while( v_port_ptr != NULL  ) {
        if( v_port_ptr->port->state == VPORT_STATE_OPEN
                && !v_port_ptr->port->is_open ) v_count++;
        v_port_ptr = v_port_ptr->next;
    }

    return (r_count == v_count);
}

/******************************************************************************/
bool do_port_attrs_match( symrec_list_t* r_ports, virt_port_list_t* v_ports )
{
    symrec_list_t* r_port_ptr = r_ports;
    virt_port_list_t* v_port_ptr = v_ports;
    attr_port_t* r_port_attr = NULL;
    bool match = false;

    r_port_ptr = r_ports;
    while( r_port_ptr != NULL  ) {
        if( r_port_ptr->rec->attr_port->is_open )
        {
            // ignore open ports
            r_port_ptr = r_port_ptr->next;
            continue;
        }
        match = false;
        v_port_ptr = v_ports;
        r_port_attr = r_port_ptr->rec->attr_port;
        while( v_port_ptr != NULL  ) {
            if( strlen( r_port_ptr->rec->name )
                    == strlen( v_port_ptr->port->name )
                && strcmp( r_port_ptr->rec->name,
                    v_port_ptr->port->name ) == 0
                && ( (int)r_port_attr->collection == v_port_ptr->port->attr_class
                    || v_port_ptr->port->attr_class == PORT_CLASS_NONE )
                && ( (int)r_port_attr->mode == v_port_ptr->port->attr_mode
                    || v_port_ptr->port->attr_mode == PORT_MODE_BI )
                ) {
                // use more specific mode from prototype
                if( v_port_ptr->port->attr_mode == PORT_MODE_BI )
                    v_port_ptr->port->attr_mode = r_port_attr->mode;
                // use more specific class from prototype
                if( v_port_ptr->port->attr_class == PORT_CLASS_NONE )
                    v_port_ptr->port->attr_class = r_port_attr->collection;
                match = true;
                break;
            }
            v_port_ptr = v_port_ptr->next;
        }
        if( !match ) break;
        r_port_ptr = r_port_ptr->next;
    }

    return match;
}

/******************************************************************************/
int get_time_criticality_prio( time_criticality_t tc, bool is_single )
{
    extern int __smxc_time_criticality_prio[TIME_CTITICALITY_COUNT];
    switch( tc ) {
        case TIME_CTITICALITY_TT:
            if( is_single ) {
                return __smxc_time_criticality_prio[TIME_CTITICALITY_TT_SINGLE];
            }
            else {
                return __smxc_time_criticality_prio[TIME_CTITICALITY_TT_NETWORK];
            }
        case TIME_CTITICALITY_RT:
            if( is_single ) {
                return __smxc_time_criticality_prio[TIME_CTITICALITY_RT_SINGLE];
            }
            else {
                return __smxc_time_criticality_prio[TIME_CTITICALITY_RT_NETWORK];
            }
        case TIME_CTITICALITY_NONE:
            return 0;
    }
    return 0;
}

/******************************************************************************/
virt_net_t* install_nets( symrec_t** symtab, UT_array* scope_stack,
        ast_node_t* ast, igraph_t* g, time_criticality_t tc )
{
    symrec_t* rec = NULL;
    virt_net_t* v_net = NULL;
    virt_net_t* v_net1 = NULL;
    virt_net_t* v_net2 = NULL;
    char error_msg[ CONST_ERROR_LEN ];

    if( ast == NULL ) return NULL;

    switch( ast->type ) {
        case AST_PARALLEL:
        case AST_PARALLEL_DET:
            v_net1 = install_nets( symtab, scope_stack, ast->op->left, g, tc );
            if( v_net1 == NULL ) return NULL;
            v_net2 = install_nets( symtab, scope_stack, ast->op->right, g, tc );
            if( v_net2 == NULL ) {
                virt_net_destroy_shallow( v_net1 );
                return NULL;
            }
            v_net = virt_net_create_parallel( v_net1, v_net2 );
            virt_net_destroy_shallow( v_net1 );
            virt_net_destroy_shallow( v_net2 );
            check_connections_cp( v_net, g, ast->type, tc );
            break;
        case AST_SERIAL:
        case AST_SERIAL_PROP:
            v_net1 = install_nets( symtab, scope_stack, ast->op->left, g, tc );
            if( v_net1 == NULL ) return NULL;
            v_net2 = install_nets( symtab, scope_stack, ast->op->right, g, tc );
            if( v_net2 == NULL ) {
                virt_net_destroy_shallow( v_net1 );
                return NULL;
            }
            // check connections and update virtual net
            check_connections( v_net1, v_net2, g );
            /* force = ( ast->type == AST_SERIAL); */
            if( ast->type == AST_SERIAL ) {
                virt_net_update_class( v_net1, PORT_CLASS_UP );
                virt_net_update_class( v_net2, PORT_CLASS_DOWN );
                check_connections_open( v_net1, v_net2 );
            }
            check_connection_missing( v_net1, v_net2, g,
                    ast->type == AST_SERIAL_PROP );
            v_net = virt_net_create_serial( v_net1, v_net2 );
            virt_net_destroy_shallow( v_net1 );
            virt_net_destroy_shallow( v_net2 );
            check_connections_cp( v_net, g, AST_SERIAL, tc );
            break;
        case AST_TB:
            v_net = install_nets( symtab, scope_stack, ast->time->op, g, tc );
            virt_port_add_time_bound( v_net, ast->time->time, TIME_TB );
            break;
        case AST_TT:
            v_net = install_nets( symtab, scope_stack, ast->time->op, g,
                TIME_CTITICALITY_TT );
            virt_port_add_time_bound( v_net, ast->time->time, TIME_TT );
            break;
        case AST_RT:
            v_net = install_nets( symtab, scope_stack, ast->time->op, g,
                TIME_CTITICALITY_RT );
            break;
        case AST_ID:
            // check the context of the symbol
            rec = symrec_get( symtab, scope_stack, ast->symbol->name,
                    ast->symbol->line, 0 );
            if( rec == NULL ) return NULL;
            // check type of the record
            switch( rec->type ) {
                case SYMREC_BOX:
                    v_net = dgraph_vertex_add_box( g, rec, ast->symbol->line );
                    dgraph_vertex_add_attr_tt( g, v_net->inst->id,
                            get_time_criticality_prio( tc, true ) );
                    v_net = virt_net_create_symbol( v_net );
                    /* virt_net_destroy_shallow( v_net1 ); */
                    check_connections_self( g, v_net );
                    break;
                case SYMREC_NET:
                    v_net = dgraph_vertex_add_net( g, rec, ast->symbol->line );
                    dgraph_vertex_add_attr_tt( g, v_net->inst->id,
                            get_time_criticality_prio( tc, false ) );
                    v_net = virt_net_create_symbol( v_net );
                    /* virt_net_destroy_shallow( v_net1 ); */
                    break;
                case SYMREC_WRAP:
                    v_net = dgraph_vertex_add_wrap( g, rec, ast->symbol->line );
                    dgraph_vertex_add_attr_tt( g, v_net->inst->id,
                            get_time_criticality_prio( tc, false ) );
                    v_net = virt_net_create_symbol( v_net );
                    /* virt_net_destroy_shallow( v_net1 ); */
                    check_connections_self( g, v_net );
                    break;
                case SYMREC_NET_PROTO:
                    // prototype -> net definition is missing
                    sprintf( error_msg, ERROR_UNDEF_NET, ERR_ERROR, rec->name );
                    report_yyerror( error_msg, ast->symbol->line );
                    break;
                default:
                    ;
            }
            break;
        default:
            ;
    }
    return v_net;
}

/******************************************************************************/
void post_process( igraph_t* g )
{
    igraph_vs_t vs;
    igraph_es_t es;
    igraph_es_t esd;
    igraph_vit_t vit;
    igraph_eit_t eit;
    igraph_vector_t dids;
    virt_net_t *v_net;
    int inst_id, eid;
    virt_port_t *p_src, *p_dest;
    bool has_changed = true;

    while( has_changed )
    {
        has_changed = false;
        es = igraph_ess_all( IGRAPH_EDGEORDER_ID );
        igraph_eit_create( g, es, &eit );
        // iterate through all edges of the graph
        while( !IGRAPH_EIT_END( eit ) ) {
            p_src = ( virt_port_t* )( uintptr_t ) igraph_cattribute_EAN( g,
                    GE_PSRC, IGRAPH_EIT_GET( eit ) );
            p_dest = ( virt_port_t* )( uintptr_t ) igraph_cattribute_EAN( g,
                    GE_PDST, IGRAPH_EIT_GET( eit ) );
            if( ( p_src ->v_net->type == VNET_SYNC )
                    && ( p_dest->v_net->type == VNET_SYNC ) )
            {
                if( check_cpsync_merge_post_connect( g, IGRAPH_VIT_GET( eit ) ) )
                {
                    propagate_decoupling_attributes( g, p_src, p_dest );
                    cpsync_merge( p_src, p_dest, g );
                    has_changed = true;
                    eid = IGRAPH_VIT_GET( eit );
                    break;
                }
            }
            IGRAPH_EIT_NEXT( eit );
        }
        igraph_eit_destroy( &eit );
        igraph_es_destroy( &es );
        if( has_changed )
        {
            esd = igraph_ess_1( eid );
            igraph_delete_edges( g, esd );
            igraph_es_destroy( &esd );
        }
    }

    // note that the reduction must be done AFTER the merge. The merge process
    // carefully rearranges IDs such that no conflicts occurr, however, the
    // reduction does not. This is because after the reduction vnet IDs don't
    // matter anymore. This is rather sloppy but it works if the reduction is
    // the very last process applied to the graph.
    vs = igraph_vss_all();
    igraph_vit_create( g, vs, &vit );
    igraph_vector_init( &dids, 0 );
    // iterate through all net instances of the graph
    while( !IGRAPH_VIT_END( vit ) ) {
        inst_id = IGRAPH_VIT_GET( vit );
        v_net = ( virt_net_t* )( uintptr_t )igraph_cattribute_VAN( g,
                GV_VNET, inst_id );
        if( v_net->type == VNET_BOX ) {
            check_ports_open( v_net );
        }
        else if( v_net->type == VNET_SYNC ) {
            if( check_single_mode_cp( g, inst_id ) )
            {
                if( cpsync_reduce( g, inst_id ) ) {
                    igraph_vector_push_back( &dids, inst_id );
                    dgraph_vertex_destroy_attr( g, inst_id, true );
                }
            }
        }
        IGRAPH_VIT_NEXT( vit );
    }
    igraph_delete_vertices( g, igraph_vss_vector( &dids ) );
    igraph_vit_destroy( &vit );
    igraph_vs_destroy( &vs );
    igraph_vector_destroy( &dids );
}

/******************************************************************************/
void propagate_decoupling_attributes( igraph_t* g, virt_port_t* port1,
        virt_port_t* port2 )
{
    virt_port_t* p_src = port2;
    virt_port_t* p_dest = port1;
    virt_port_t* pg;
    igraph_es_t es;
    igraph_eit_t eit;
    int eid;

    if( port1->attr_mode == PORT_MODE_OUT )
    {
        p_src = port1;
        p_dest = port2;
    }

    igraph_es_incident( &es, p_dest->v_net->inst->id, IGRAPH_OUT );
    igraph_eit_create( g, es, &eit );
    // for each edge connect to the actual nets form the graph
    while( !IGRAPH_EIT_END( eit ) ) {
        eid = IGRAPH_EIT_GET( eit );
        if( p_src->descoupled )
        {
            pg = ( virt_port_t* )( uintptr_t ) igraph_cattribute_EAN( g,
                    GE_PSRC, IGRAPH_EIT_GET( eit ) );
            pg->descoupled = true;
            igraph_cattribute_EAN_set( g, GE_DSRC, eid, true );
        }
        if( p_dest->descoupled )
        {
            pg = ( virt_port_t* )( uintptr_t ) igraph_cattribute_EAN( g,
                    GE_PDST, IGRAPH_EIT_GET( eit ) );
            pg->descoupled = true;
            igraph_cattribute_EAN_set( g, GE_DDST, eid, true );
        }
        IGRAPH_EIT_NEXT( eit );
    }
    igraph_eit_destroy( &eit );
    igraph_es_destroy( &es );
}
