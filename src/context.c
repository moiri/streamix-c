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

/******************************************************************************/
bool are_port_names_ok( virt_port_t* p1, virt_port_t* p2, bool cpp, bool cps,
        bool directed )
{
    // no, if port names do not match
    if( strcmp( p1->name, p2->name ) != 0 )
        return false;
    // are we checking copy synchronizer connections?
    if( cps || cpp ) {
        // we are good if both ports have the same class
        if( p1->attr_class == p2->attr_class )
            return true;
        // are we checking parallel combinators?
        if( cpp ) {
            // we are good if one port has no class and the other is not a side
            // port
            if( ( p1->attr_class != PORT_CLASS_SIDE )
                    && ( p2->attr_class == PORT_CLASS_NONE ) )
                return true;
            if( ( p1->attr_class == PORT_CLASS_NONE )
                    && ( p2->attr_class != PORT_CLASS_SIDE ) )
                return true;
        }
        // we came through here so none of the conditions matched
        return false;
    }
    // or normal undirected connections?
    else if( !directed ) {
        // ok, if either of the ports has no class speciefied
        if( ( p1->attr_class == PORT_CLASS_NONE )
                || ( p2->attr_class == PORT_CLASS_NONE ) )
            return true;
        // ok if one port has class DOWN and one UP
        if( ( p1->attr_class == PORT_CLASS_DOWN )
                && ( p2->attr_class == PORT_CLASS_UP ) )
            return true;
        // ok if one port has class UP and one DOWN
        if( ( p1->attr_class == PORT_CLASS_UP )
                && ( p2->attr_class == PORT_CLASS_DOWN ) )
            return true;
        // we came through here so none of the conditions matched
        return false;
    }
    // or normal directed connections?
    else {
        // no, if the left port is in another class than DS
        if( ( p1->attr_class != PORT_CLASS_DOWN )
                && ( p1->attr_class != PORT_CLASS_NONE ) )
            return false;
        // no, if the right port is in another class than US
        if( ( p2->attr_class != PORT_CLASS_UP )
                && ( p2->attr_class != PORT_CLASS_NONE ) )
            return false;
    }
    // we came through here, so all is good, names match
    return true;
}

/******************************************************************************/
bool are_port_modes_ok( virt_port_t* p1, virt_port_t* p2 )
{
    // yes, if modes are different
    if( p1->attr_mode != p2->attr_mode ) return true;
    // yes, if the left port is a copy synchronizer port
    if( p1->attr_mode == PORT_MODE_BI ) return true;
    // yes, if the right port is a copy synchronizer port
    if( p2->attr_mode == PORT_MODE_BI ) return true;

    // we came through here, so port modes are not compatible
    return false;
}

/******************************************************************************/
bool check_connection( virt_port_t* port_l, virt_port_t* port_r, igraph_t* g,
        bool directed )
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
    inst_l = port_l->inst;
    inst_r = port_r->inst;
    if( are_port_names_ok( port_l, port_r, false, false, directed ) ) {
        if( ( inst_l->type == INSTREC_SYNC )
                && ( inst_r->type == INSTREC_SYNC ) ) {
#if defined(DEBUG) || defined(DEBUG_CONNECT)
            printf( "\n  => connection is valid\n" );
#endif // DEBUG_CONNECT
            // merge copy synchronizers
            port_l->state = VPORT_STATE_DISABLED;
            port_r->state = VPORT_STATE_DISABLED;
            cpsync_merge( port_l, port_r, g );
            res = true;
        }
        else if( are_port_modes_ok( port_l, port_r ) ) {
#if defined(DEBUG) || defined(DEBUG_CONNECT)
            printf( "\n  => connection is valid\n" );
#endif // DEBUG_CONNECT
            connect_ports( port_l, port_r, g, true );
            port_l->attr_class = PORT_CLASS_DOWN;
            port_r->attr_class = PORT_CLASS_UP;
            res = true;
        }
        else {
#if defined(DEBUG) || defined(DEBUG_CONNECT)
            printf( "\n  => connection is invalid (bad mode)\n" );
#endif // DEBUG_CONNECT
            sprintf( error_msg, ERROR_BAD_MODE, ERR_ERROR, port_l->name,
                    inst_l->name, inst_l->id, inst_r->name, inst_r->id,
                    port_r->inst->line );
            report_yyerror( error_msg, port_r->inst->line );
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
void check_connection_missing( virt_net_t* v_net_l, virt_net_t* v_net_r,
        igraph_t* g )
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
                sprintf( error_msg, ERROR_NO_NET_CON, ERR_ERROR, inst1->name,
                        inst1->id, inst2->name, inst2->id );
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
void check_connections( virt_net_t* v_net1, virt_net_t* v_net2, igraph_t* g )
{
    virt_port_list_t* ports_l = NULL;
    virt_port_list_t* ports_r = NULL;
    bool res = false;

    ports_l = v_net1->ports;
    while( ports_l != NULL ) {
        ports_r = v_net2->ports;
        while( ports_r != NULL ) {
            if( ( ports_l->port->state == VPORT_STATE_OPEN )
                    && ( ports_r->port->state == VPORT_STATE_OPEN ) ) {
                res = check_connection( ports_l->port, ports_r->port, g,
                        true );
                if( res ) break;
            }
            ports_r = ports_r->next;
        }
        ports_l = ports_l->next;
    }
}

/******************************************************************************/
void check_context( ast_node_t* ast, symrec_t** symtab, igraph_t* g )
{
    UT_array* scope_stack = NULL; // stack to handle the scope
    attr_net_t* n_attr = NULL;
    int scope = 0;
    igraph_t g_tmp;

    utarray_new( scope_stack, &ut_int_icd );
    utarray_push_back( scope_stack, &scope );
    n_attr = check_context_ast( symtab, scope_stack, ast );
    igraph_empty( &g_tmp, 0, IGRAPH_DIRECTED );
    dgraph_append( &g_tmp, &n_attr->g, true );
    dgraph_flatten( g, &g_tmp );

    // cleanup
    igraph_destroy( &g_tmp );
    if( n_attr != NULL ) symrec_attr_destroy_net( n_attr );
    utarray_free( scope_stack );
}

/******************************************************************************/
void* check_context_ast( symrec_t** symtab, UT_array* scope_stack,
        ast_node_t* ast )
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
    virt_net_t* v_net;
    void* res = NULL;
    static int _scope = 0;
    igraph_t g_net;

    if( ast == NULL ) return NULL;

    switch( ast->type ) {
        case AST_PROGRAM:
            // install net instances
            check_context_ast( symtab, scope_stack, ast->program->stmts );
            res = check_context_ast( symtab, scope_stack, ast->program->net );
            break;
        case AST_STMTS:
            list = ast->list;
            while (list != NULL) {
                check_context_ast( symtab, scope_stack, list->node );
                list = list->next;
            }
            break;
        case AST_ASSIGN:
            // get the attributes
            attr = check_context_ast( symtab, scope_stack, ast->assign->op );
            if( ast->assign->type == AST_NET ) {
                // check prototype if available
                rec = symrec_search( symtab, scope_stack,
                        ast->assign->id->symbol->name, 0 );
                if( rec == NULL ) {
                    // no prototype, install the symbol
                    rec = symrec_create_net( ast->assign->id->symbol->name,
                        *utarray_back( scope_stack ),
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
            }
            if( ast->assign->type == AST_BOX ) {
                rec = symrec_create_box( ast->assign->id->symbol->name,
                        *utarray_back( scope_stack ),
                        ast->assign->id->symbol->line, attr );
                // install the symbol
                if( symrec_put( symtab, rec ) == NULL ) {
                    symrec_attr_destroy_box( rec->attr_box );
                    symrec_destroy( rec );
                }
            }
            break;
        case AST_NET_PROTO:
            _scope++;
            utarray_push_back( scope_stack, &_scope );
            port_list = ( symrec_list_t* )check_context_ast( symtab,
                    scope_stack, ast->proto->ports );
            utarray_pop_back( scope_stack );
            // prepare symbol attribute and create symbol
            np_attr = symrec_attr_create_proto( port_list );
            rec = symrec_create_proto( ast->proto->id->symbol->name,
                    *utarray_back( scope_stack ), ast->proto->id->symbol->line,
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
                    ast->network->net, &g_net );
            n_attr = symrec_attr_create_net( v_net, &g_net );
#if defined(DEBUG) || defined(DEBUG_NET_DOT)
            igraph_write_graph_dot( &g_net, stdout );
#endif // DEBUG
            res = ( void* )n_attr;
            break;
        case AST_WRAP:
            _scope++;
            utarray_push_back( scope_stack, &_scope );
            n_attr = check_context_ast( symtab, scope_stack, ast->wrap->stmts );
            if( n_attr == NULL ) return NULL;
            port_list = ( symrec_list_t* )check_context_ast( symtab,
                    scope_stack, ast->wrap->ports );
            utarray_pop_back( scope_stack );
            // prepare symbol attributes and create symbol
            w_attr = symrec_attr_create_wrap( false, port_list,
                    n_attr->v_net );
            if( ast->wrap->attr_static != NULL ) w_attr->attr_static = true;
            rec = symrec_create_wrap( ast->wrap->id->symbol->name,
                    *utarray_back( scope_stack ), ast->wrap->id->symbol->line,
                    w_attr );
            // install the wrapper symbol in the scope of its declaration
            res = ( void* )symrec_put( symtab, rec );
            if( res == NULL ) {
                symrec_attr_destroy_wrap( rec->attr_wrap );
                symrec_destroy( rec );
                return NULL;
            }
            break;
        case AST_BOX:
            _scope++;
            utarray_push_back( scope_stack, &_scope );
            port_list = ( symrec_list_t* )check_context_ast( symtab,
                    scope_stack, ast->box->ports );
            utarray_pop_back( scope_stack );
            // prepare symbol attributes and create symbol
            b_attr = symrec_attr_create_box( false,
                    ast->box->impl->symbol->name, port_list );
            if( ast->box->attr_pure != NULL ) b_attr->attr_pure = true;
            // return box attributes
            res = ( void* )b_attr;
            break;
        case AST_PORTS:
            list = ast->list;
            while (list != NULL) {
                ptr = ( symrec_list_t* )malloc( sizeof( symrec_list_t ) );
                res = check_context_ast( symtab, scope_stack, list->node );
                ptr->rec = ( symrec_t* )res;
                ptr->next = port_list;
                port_list = ptr;
                list = list->next;
            }
            res = ( void* )port_list;   // return pointer to the port list
            break;
        case AST_PORT:
            // prepare symbol attributes and create symbol
            p_attr = symrec_attr_create_port( NULL, PORT_MODE_BI,
                    PORT_CLASS_NONE, false, ast->port->sync_id );
            if( ast->port->mode != NULL )
                p_attr->mode = ast->port->mode->attr->val;
            if( ast->port->coupling != NULL ) p_attr->decoupled = true;
            if( ast->port->collection != NULL ) {
                p_attr->collection = ast->port->collection->attr->val;
            }
            rec = symrec_create_port( ast->port->id->symbol->name,
                    *utarray_back( scope_stack ), ast->port->id->symbol->line,
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
        res = false;
    }

    return res;
}

/******************************************************************************/
void connect_ports( virt_port_t* port_l, virt_port_t* port_r, igraph_t* g,
        bool connect_sync )
{
    virt_port_t *p_src, *p_dest;
    // set source and dest id
    if( ( ( port_l->inst->type == INSTREC_SYNC )
                && ( port_r->attr_mode == PORT_MODE_OUT ) )
            || ( ( port_r->inst->type == INSTREC_SYNC )
                && ( port_l->attr_mode == PORT_MODE_IN ) )
            || ( ( port_l->attr_mode == PORT_MODE_IN )
                && ( port_r->attr_mode == PORT_MODE_OUT ) ) ) {
        p_dest = port_l;
        p_src = port_r;
    }
    else if( ( ( port_l->inst->type == INSTREC_SYNC )
                && ( port_r->attr_mode == PORT_MODE_IN ) )
            || ( ( port_r->inst->type == INSTREC_SYNC )
                && ( port_l->attr_mode == PORT_MODE_OUT ) )
            || ( ( port_l->attr_mode == PORT_MODE_OUT )
                && ( port_r->attr_mode == PORT_MODE_IN ) ) ) {
        p_dest = port_r;
        p_src = port_l;
    }
    else return;

    // set port state
    if( ( port_l->inst->type == INSTREC_BOX ) || connect_sync )
        port_l->state = VPORT_STATE_CONNECTED;
    if( ( port_r->inst->type == INSTREC_BOX ) || connect_sync )
        port_r->state = VPORT_STATE_CONNECTED;
    // add edge to the graph and set attributes
    dgraph_edge_add( g, p_src, p_dest );
}

/******************************************************************************/
void cpsync_connect( virt_net_t* v_net, virt_port_t* port1,
        virt_port_t* port2, igraph_t* g )
{
    virt_net_t* v_net_sync = NULL;
    instrec_t* inst1 = port1->inst;
    instrec_t* inst2 = port2->inst;
    virt_port_t* port_new;
    port_class_t port_class;
    port_mode_t port_mode;
    // if possible, set port class to anything but PORT_CLASS_NONE
    if( port1->attr_class == PORT_CLASS_NONE )
        port_class = port2->attr_class;
    else if( port2->attr_class == PORT_CLASS_NONE )
        port_class = port1->attr_class;
    // connect ports and cerate copy synchronizer if necessary
    if( ( inst1->type == INSTREC_SYNC )
            && ( inst2->type == INSTREC_SYNC ) ) {
        // merge copy synchronizers
        port2->state = VPORT_STATE_DISABLED;
        cpsync_merge( port1, port2, g );
    }
    else if( ( inst1->type == INSTREC_SYNC )
            || ( inst2->type == INSTREC_SYNC ) ) {
        connect_ports( port1, port2, g, false );
    }
    else {
        // create copy synchronizer instance and update the graph
        v_net_sync = dgraph_vertex_add_sync( g, port1, port2 );
#if defined(DEBUG) || defined(DEBUG_CONNECT)
        printf( "Create copy-synchronizer %s(%d)\n", v_net_sync->inst->name,
                v_net_sync->inst->id );
#endif // DEBUG_CONNECT
        if( port1->attr_class == port2->attr_class )
            port_class = port1->attr_class;
        else port_class = PORT_CLASS_NONE;
        if( port1->attr_mode == port2->attr_mode )
            port_mode = port1->attr_mode;
        else port_mode = PORT_MODE_BI;
        port_new = virt_port_add( v_net, port_class, port_mode,
                v_net_sync->inst, port1->name, port1->symb );
        virt_port_append( v_net_sync, port_new );
        connect_ports( port_new, port1, g, false );
        connect_ports( port_new, port2, g, false );
    }
}

/******************************************************************************/
void cpsync_connects( virt_net_t* v_net, bool parallel, igraph_t* g )
{
    virt_port_list_t* ports1 = NULL;
    virt_port_list_t* ports2 = NULL;
    int new_idx = 0;
    ports1 = v_net->ports;
    while( ports1 != NULL ) {
        new_idx++;
        ports1 = ports1->next;
    }

    ports1 = v_net->ports;
    while( ports1 != NULL ) {
        ports2 = v_net->ports;
        while( ports2 != NULL ) {
            if( ( ports1->idx != ports2->idx )
                    && ( ports1->idx < new_idx ) && ( ports2->idx < new_idx )
                    && ( ports1->port->state == VPORT_STATE_OPEN )
                    && ( ports2->port->state == VPORT_STATE_OPEN )
                    && are_port_names_ok( ports1->port, ports2->port, parallel,
                        !parallel, true ) ) {
                cpsync_connect( v_net, ports1->port, ports2->port, g );
            }
            ports2 = ports2->next;
        }
        ports1 = ports1->next;
    }
}

/******************************************************************************/
instrec_t* cpsync_merge( virt_port_t* port1, virt_port_t* port2, igraph_t* g )
{
    int id_del;
    instrec_t *res;
    virt_net_t* v_net1;
    virt_net_t* v_net2;
    instrec_t* inst1 = port1->inst;
    instrec_t* inst2 = port2->inst;
#if defined(DEBUG) || defined(DEBUG_CONNECT)
    printf( "Merge %s(%d) and %s(%d)", inst1->name, inst1->id, inst2->name,
            inst2->id );
#endif // DEBUG_CONNECT
    v_net1 = ( virt_net_t* )( uintptr_t )igraph_cattribute_VAN( g,
            INST_ATTR_VNET, inst1->id );
    v_net2 = ( virt_net_t* )( uintptr_t )igraph_cattribute_VAN( g,
            INST_ATTR_VNET, inst2->id );
    id_del = dgraph_vertex_merge( g, inst1->id, inst2->id );
    // delete one copy synchronizer
    if( id_del == inst1->id ) {
        instrec_destroy( inst1 );
        res = port1->inst = inst2;
    }
    else {
        instrec_destroy( inst2 );
        res = port2->inst = inst1;
    }
#if defined(DEBUG) || defined(DEBUG_CONNECT)
    printf( " into %s(%d)\n", res->name, res->id );
#endif // DEBUG_CONNECT
    // adjust all ids starting from the id of the deleted record
    dgraph_vertex_update_ids( g, id_del );
    // update v_net
    igraph_cattribute_VAN_set( g, INST_ATTR_VNET, res->id,
            ( uintptr_t )virt_net_create_sync_merge( v_net1, v_net2, res ) );
    virt_net_destroy_shallow( v_net1 );
    virt_net_destroy_shallow( v_net2 );

    return res;
}

/******************************************************************************/
void debug_print_rport( symrec_t* port, char* name )
{
    printf( "%s", name );
    if( port->attr_port->collection == PORT_CLASS_DOWN ) printf( "_" );
    else if( port->attr_port->collection == PORT_CLASS_UP ) printf( "^" );
    else if( port->attr_port->collection == PORT_CLASS_SIDE ) printf( "|" );
    if( port->attr_port->mode == PORT_MODE_IN ) printf( "<--" );
    else if( port->attr_port->mode == PORT_MODE_OUT ) printf( "-->" );
    else printf( "<->" );
    printf( "%s", port->name );
}

/******************************************************************************/
void debug_print_rports( symrec_list_t* rports, char* name )
{
    symrec_list_t* ports = rports;
    while( ports != NULL ) {
        debug_print_rport( ports->rec, name );
        printf(", ");
        ports = ports->next;
    }
    printf("\n");
}

/******************************************************************************/
bool do_port_cnts_match( symrec_list_t* r_ports, virt_port_list_t* v_ports )
{
    symrec_list_t* r_port_ptr = r_ports;
    virt_port_list_t* v_port_ptr = v_ports;
    int v_count = 0;
    int r_count = 0;

    while( r_port_ptr != NULL  ) {
        r_count++;
        r_port_ptr = r_port_ptr->next;
    }

    while( v_port_ptr != NULL  ) {
        if( v_port_ptr->port->state == VPORT_STATE_OPEN ) v_count++;
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
        match = false;
        v_port_ptr = v_ports;
        r_port_attr = r_port_ptr->rec->attr_port;
        while( v_port_ptr != NULL  ) {
            if( strlen( r_port_ptr->rec->name )
                    == strlen( v_port_ptr->port->name )
                && strcmp( r_port_ptr->rec->name,
                    v_port_ptr->port->name ) == 0
                && ( r_port_attr->collection == v_port_ptr->port->attr_class
                    || v_port_ptr->port->attr_class == PORT_CLASS_NONE )
                && ( r_port_attr->mode == v_port_ptr->port->attr_mode
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
virt_net_t* install_nets( symrec_t** symtab, UT_array* scope_stack,
        ast_node_t* ast, igraph_t* g )
{
    symrec_t* rec = NULL;
    virt_net_t* v_net = NULL;
    virt_net_t* v_net1 = NULL;
    virt_net_t* v_net2 = NULL;
    char error_msg[ CONST_ERROR_LEN ];

    if( ast == NULL ) return NULL;

    switch( ast->type ) {
        case AST_PARALLEL:
            v_net1 = install_nets( symtab, scope_stack, ast->op->left, g );
            if( v_net1 == NULL ) return NULL;
            v_net2 = install_nets( symtab, scope_stack, ast->op->right, g );
            if( v_net2 == NULL ) return NULL;
            v_net = virt_net_create_parallel( v_net1, v_net2 );
            cpsync_connects( v_net, true, g );
            break;
        case AST_SERIAL:
            v_net1 = install_nets( symtab, scope_stack, ast->op->left, g );
            if( v_net1 == NULL ) return NULL;
            v_net2 = install_nets( symtab, scope_stack, ast->op->right, g );
            if( v_net2 == NULL ) return NULL;
            // check connections and update virtual net
            check_connections( v_net1, v_net2, g );
            virt_net_update_class( v_net1, PORT_CLASS_UP );
            virt_net_update_class( v_net2, PORT_CLASS_DOWN );
            check_connection_missing( v_net1, v_net2, g );
            v_net = virt_net_create_serial( v_net1, v_net2 );
            cpsync_connects( v_net, false, g );
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
                    break;
                case SYMREC_NET:
                    v_net = dgraph_vertex_add_net( g, rec, ast->symbol->line );
                    break;
                case SYMREC_WRAP:
                    v_net = dgraph_vertex_add_wrap( g, rec, ast->symbol->line );
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
    if( ( v_net1 != NULL ) && ( ( v_net1->type == VNET_SERIAL )
                || ( v_net1->type == VNET_PARALLEL ) ) )
        virt_net_destroy_shallow( v_net1 );
    if( ( v_net2 != NULL ) && ( ( v_net2->type == VNET_SERIAL )
                || ( v_net2->type == VNET_PARALLEL ) ) )
        virt_net_destroy_shallow( v_net2 );
    return v_net;
}
