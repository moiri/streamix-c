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
bool are_port_names_ok( virt_port_t* p1, virt_port_t* p2, bool cpp, bool cps )
{
    // no, if port names do not match
    if( strcmp( p1->rec->name, p2->rec->name ) != 0 )
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
    // or normal connections?
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
bool check_connection( inst_net_t* net, virt_port_t* ports_l,
        virt_port_t* ports_r, igraph_t* g )
{
    bool res = false;
    char error_msg[ CONST_ERROR_LEN ];
    const char* port_name;
#if defined(DEBUG) || defined(DEBUG_CONNECT)
    printf( "check_connection:\n " );
    debug_print_vport( ports_l );
    printf( " and " );
    debug_print_vport( ports_r );
#endif // DEBUG_CONNECT
    if( are_port_names_ok( ports_l, ports_r, false, false ) ) {
        if( ( ports_l->inst->type == INSTREC_SYNC )
                && ( ports_r->inst->type == INSTREC_SYNC ) ) {
#if defined(DEBUG) || defined(DEBUG_CONNECT)
            printf( "\n  => connection is valid\n" );
#endif // DEBUG_CONNECT
            // merge copy synchronizers
            cpsync_merge( net, ports_l, ports_r, g );
            res = true;
        }
        else if( are_port_modes_ok( ports_l, ports_r ) ) {
            if( ports_l->rec != NULL ) port_name = ports_l->rec->name;
            else port_name = ports_r->rec->name;
            dgraph_connect_1( g, ports_l->inst->id, ports_r->inst->id,
                    ports_l->attr_mode, ports_r->attr_mode, port_name );

#if defined(DEBUG) || defined(DEBUG_CONNECT)
            printf( "\n  => connection is valid\n" );
#endif // DEBUG_CONNECT
            res = true;
        }
        else {
#if defined(DEBUG) || defined(DEBUG_CONNECT)
            printf( "\n  => connection is invalid (bad mode)\n" );
#endif // DEBUG_CONNECT
            sprintf( error_msg, ERROR_BAD_MODE, ERR_ERROR, ports_l->rec->name,
                    ports_l->inst->name, ports_l->inst->id, ports_r->inst->name,
                    ports_r->inst->id, ports_r->rec->line );
            report_yyerror( error_msg, ports_r->rec->line );
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
void check_connections( inst_net_t* net, virt_net_t* v_net1, virt_net_t* v_net2,
        igraph_t* g )
{
    virt_port_t* ports_l = NULL;
    virt_port_t* ports_r = NULL;
    virt_port_t* ports_last_l = NULL;
    virt_port_t* ports_next_l = NULL;
    virt_port_t* ports_last_r = NULL;
    virt_port_t* ports_next_r = NULL;

    ports_l = v_net1->ports;
    while( ports_l != NULL ) {
        ports_next_l = ports_l->next;
        ports_r = v_net2->ports;
        ports_last_r = NULL;
        while( ports_r != NULL ) {
            ports_next_r = ports_r->next;
            if( check_connection( net, ports_l, ports_r, g ) ) {
                // remove both ports from their corresponding virtual nets
                if( ports_last_l != NULL ) ports_last_l->next = ports_next_l;
                else v_net1->ports = ports_next_l;
                if( ports_last_r != NULL ) ports_last_r->next = ports_next_r;
                else v_net2->ports = ports_next_r;
                free( ports_l );
                free( ports_r );
                ports_l = ports_last_l;
                break;
            }
            ports_last_r = ports_r;
            ports_r = ports_next_r;
        }
        ports_last_l = ports_l;
        ports_l = ports_next_l;
    }
}

/******************************************************************************/
void check_connection_missing( inst_net_t* net, virt_net_t* v_net_l,
        virt_net_t* v_net_r, igraph_t* g )
{
    char error_msg[ CONST_ERROR_LEN ];
    int i, j;
    igraph_vs_t vs1, vs2;
    igraph_vector_t v1, v2;
    igraph_matrix_t res1, res2;
    inst_rec_t* rec1 = NULL;
    inst_rec_t* rec2 = NULL;

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
            if( ( MATRIX( res1, i, j ) > 2 ) &&
                ( MATRIX( res2, i, j ) > 2 ) ) {
                rec1 = inst_rec_get( &net->nodes, VECTOR( v1 )[i] );
                rec2 = inst_rec_get( &net->nodes, VECTOR( v2 )[j] );
                // ERROR: there is no connection between the two nets
                sprintf( error_msg, ERROR_NO_NET_CON, ERR_ERROR,
                        rec1->name, ( int )VECTOR( v1 )[i],
                        rec2->name, ( int )VECTOR( v2 )[j] );
                report_yyerror( error_msg, rec1->line );
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
void check_context( ast_node_t* ast, inst_net_t** nets, igraph_t* g )
{
    symrec_t* symtab = NULL;        // hash table to store the symbols
    UT_array* scope_stack = NULL; // stack to handle the scope
    attr_net_t* n_attr = NULL;
    int scope = 0;

    utarray_new( scope_stack, &ut_int_icd );
    utarray_push_back( scope_stack, &scope );
    n_attr = check_context_ast( &symtab, nets, scope_stack, ast, g );
    if( n_attr != NULL ) symrec_attr_destroy_net( n_attr );
    /* install_ids( &symtab, scope_stack, ast, false ); */
    /* instrec_put( &insttab, VAL_THIS, *utarray_back( scope_stack ), */
    /*         VAL_SELF, -1, NULL ); */
    /* check_nets( &symtab, &nets, scope_stack, ast ); */
    /* check_instances( &nets ); */
    /* // check whether all ports are connected spawn synchronizers and draw the */
    /* // nodes, synchroniyers and connections */
    /* check_port_all( &insttab, ast ); */

    // cleanup
    utarray_free( scope_stack );
    symrec_del_all( &symtab );
}

/******************************************************************************/
void* check_context_ast( symrec_t** symtab, inst_net_t** nets,
        UT_array* scope_stack, ast_node_t* ast, igraph_t* g )
{
    ast_list_t* list = NULL;
    attr_box_t* b_attr = NULL;
    attr_net_t* n_attr = NULL;
    attr_prot_t* np_attr = NULL;
    attr_port_t* p_attr = NULL;
    attr_wrap_t* w_attr = NULL;
    void* attr = NULL;
    inst_net_t* net = NULL;
    symrec_t* rec = NULL;
    symrec_list_t* ptr = NULL;
    symrec_list_t* port_list = NULL;
    virt_net_t* v_net;
    void* res = NULL;
    static int _scope = 0;

    if( ast == NULL ) return NULL;

    switch( ast->type ) {
        case AST_PROGRAM:
            // install net instances
            net = inst_net_put( nets, *utarray_back( scope_stack ) );
            check_context_ast( symtab, nets, scope_stack,
                    ast->program->stmts, g );
            res = check_context_ast( symtab, nets, scope_stack,
                    ast->program->net, g );
            break;
        case AST_STMTS:
            list = ast->list;
            while (list != NULL) {
                check_context_ast( symtab, nets, scope_stack, list->node, g );
                list = list->next;
            }
            break;
        case AST_ASSIGN:
            // get the attributes
            attr = check_context_ast( symtab, nets, scope_stack,
                    ast->assign->op, g );
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
        case AST_NET:
            net = inst_net_get( nets, *utarray_back( scope_stack ) );
            v_net = ( void* )install_nets( symtab, net, scope_stack,
                    ast->network->net, g );
            n_attr = symrec_attr_create_net( v_net );
            res = ( void* )n_attr;
            break;
        case AST_NET_PROTO:
            _scope++;
            utarray_push_back( scope_stack, &_scope );
            port_list = ( symrec_list_t* )check_context_ast( symtab, nets,
                    scope_stack, ast->proto->ports, g );
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
        case AST_PORTS:
            list = ast->list;
            while (list != NULL) {
                ptr = ( symrec_list_t* )malloc( sizeof( symrec_list_t ) );
                res = check_context_ast( symtab, nets, scope_stack,
                        list->node, g );
                ptr->rec = ( symrec_t* )res;
                ptr->next = port_list;
                port_list = ptr;
                list = list->next;
            }
            res = ( void* )port_list;   // return pointer to the port list
            break;
        case AST_BOX:
            _scope++;
            utarray_push_back( scope_stack, &_scope );
            port_list = ( symrec_list_t* )check_context_ast( symtab, nets,
                    scope_stack, ast->box->ports, g );
            utarray_pop_back( scope_stack );
            // prepare symbol attributes and create symbol
            b_attr = symrec_attr_create_box( false,
                    ast->box->impl->symbol->name, port_list );
            if( ast->box->attr_pure != NULL ) b_attr->attr_pure = true;
            // return box attributes
            res = ( void* )b_attr;
            break;
        case AST_WRAP:
            _scope++;
            utarray_push_back( scope_stack, &_scope );
            n_attr = check_context_ast( symtab, nets, scope_stack,
                    ast->wrap->stmts, g );
            if( n_attr == NULL ) return NULL;
            port_list = ( symrec_list_t* )check_context_ast( symtab, nets,
                    scope_stack, ast->wrap->ports, g );
            utarray_pop_back( scope_stack );
            // prepare symbol attributes and create symbol
            w_attr = symrec_attr_create_wrap( false, port_list,
                    virt_net_copy( n_attr->v_net ) );
            symrec_attr_destroy_net( n_attr );
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
            net = inst_net_get( nets, *utarray_back( scope_stack ) );
            break;
        case AST_PORT:
            // prepare symbol attributes and create symbol
            p_attr = symrec_attr_create_port( NULL, PORT_MODE_BI, PORT_CLASS_NONE,
                    false, ast->port->sync_id );
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
void cpsync_connect( inst_net_t* net, virt_port_t* port1, virt_port_t* port2,
        igraph_t* g )
{
    int node_id;
    inst_rec_t* cp_sync = NULL;
    // create copy synchronizer instance
    if( ( port1->inst->type == INSTREC_SYNC )
            && ( port2->inst->type == INSTREC_SYNC ) ) {
        // merge copy synchronizers
        cp_sync = cpsync_merge( net, port1, port2, g );
    }
    else if( port1->inst->type == INSTREC_SYNC ) {
        cp_sync = port1->inst;
        dgraph_connect_1( g, cp_sync->id, port2->inst->id, PORT_MODE_BI,
                port2->attr_mode, port2->rec->name );
    }
    else if( port2->inst->type == INSTREC_SYNC ) {
        cp_sync = port2->inst;
        dgraph_connect_1( g, cp_sync->id, port1->inst->id, PORT_MODE_BI,
                port1->attr_mode, port1->rec->name );
    }
    else {
        cp_sync = inst_rec_put( &net->nodes, TEXT_CP, igraph_vcount( g ),
                0, INSTREC_SYNC, NULL );
#if defined(DEBUG) || defined(DEBUG_CONNECT)
        printf( "Create copy-synchronizer %s(%d)\n", cp_sync->name,
                cp_sync->id );
#endif // DEBUG_CONNECT
        node_id = igraph_vcount( g );
        igraph_add_vertices( g, 1, NULL );
        igraph_cattribute_VAS_set( g, "label", node_id, TEXT_CP );
        dgraph_connect_1( g, cp_sync->id, port1->inst->id, PORT_MODE_BI,
                port1->attr_mode, port1->rec->name );
        dgraph_connect_1( g, cp_sync->id, port2->inst->id, PORT_MODE_BI,
                port2->attr_mode, port2->rec->name );
    }
    // change left port to copy synchronizer port
    port1->inst = cp_sync;
    // set mode of port
    port1->attr_mode = PORT_MODE_BI;
    // if possible, set port class to anything but PORT_CLASS_NONE
    if( port1->attr_class == PORT_CLASS_NONE )
        port1->attr_class = port2->attr_class;
}

/******************************************************************************/
void cpsync_connects( inst_net_t* net, virt_net_t* v_net1, virt_net_t* v_net2,
        bool parallel, igraph_t* g )
{
    virt_port_t* port1 = NULL;
    virt_port_t* port2 = NULL;
    virt_port_t* port_last = NULL;
    virt_port_t* port_next = NULL;

    port1 = v_net1->ports;
    while( port1 != NULL ) {
        port2 = v_net2->ports;
        port_last = NULL;
        while( port2 != NULL ) {
            port_next = port2->next;
            if( are_port_names_ok( port1, port2, parallel, !parallel ) ) {
                cpsync_connect( net, port1, port2, g );
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
inst_rec_t* cpsync_merge( inst_net_t* net, virt_port_t* port1,
        virt_port_t* port2, igraph_t* g )
{
    int id, id_del;
    id_del = dgraph_merge_vertice_1( g, port1->inst->id,
            port2->inst->id );
    // delete one copy synchronizer from insttab
    id = port2->inst->id;
    inst_rec_del( &net->nodes, port2->inst );
    if( id_del != id )
        inst_rec_replace_id( &net->nodes, id_del, id );
    // adjust all ids starting from the id of the deleted record
    for( id = id_del; id < igraph_vcount( g ); id++ )
        inst_rec_replace_id( &net->nodes, id + 1, id );
    return port1->inst;
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
void debug_print_vport( virt_port_t* port )
{
    printf( "%s(%d)", port->inst->name, port->inst->id );
    if( port->attr_class == PORT_CLASS_DOWN ) printf( "_" );
    else if( port->attr_class == PORT_CLASS_UP ) printf( "^" );
    else if( port->attr_class == PORT_CLASS_SIDE ) printf( "|" );
    if( port->attr_mode == PORT_MODE_IN ) printf( "<--" );
    else if( port->attr_mode == PORT_MODE_OUT ) printf( "-->" );
    else printf( "<->" );
    printf( "%s", port->rec->name );
}

/******************************************************************************/
void debug_print_vports( virt_net_t* v_net )
{
    virt_port_t* ports = NULL;
    if( v_net->ports != NULL )
        ports = v_net->ports;
    while( ports != NULL ) {
        debug_print_vport( ports );
        printf(", ");
        ports = ports->next;
    }
    printf("\n");
}

/******************************************************************************/
bool do_port_cnts_match( symrec_list_t* r_ports, virt_port_t* v_ports )
{
    symrec_list_t* r_port_ptr = r_ports;
    virt_port_t* v_port_ptr = v_ports;
    int v_count = 0;
    int r_count = 0;

    while( r_port_ptr != NULL  ) {
        r_count++;
        r_port_ptr = r_port_ptr->next;
    }

    while( v_port_ptr != NULL  ) {
        v_count++;
        v_port_ptr = v_port_ptr->next;
    }

    return (r_count == v_count);
}

/******************************************************************************/
bool do_port_attrs_match( symrec_list_t* r_ports, virt_port_t* v_ports )
{
    symrec_list_t* r_port_ptr = r_ports;
    virt_port_t* v_port_ptr = v_ports;
    attr_port_t* r_port_attr = NULL;
    bool match = false;

    r_port_ptr = r_ports;
    while( r_port_ptr != NULL  ) {
        match = false;
        v_port_ptr = v_ports;
        r_port_attr = r_port_ptr->rec->attr_port;
        while( v_port_ptr != NULL  ) {
            if( strlen( r_port_ptr->rec->name )
                    == strlen( v_port_ptr->rec->name )
                && strcmp( r_port_ptr->rec->name, v_port_ptr->rec->name ) == 0
                && r_port_attr->collection == v_port_ptr->attr_class
                && ( r_port_attr->mode == v_port_ptr->attr_mode
                    || v_port_ptr->attr_mode == PORT_MODE_BI )
                ) {
                // use more specific mode from prototype
                if( v_port_ptr->attr_mode == PORT_MODE_BI )
                    v_port_ptr->attr_mode = r_port_attr->mode;
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
virt_net_t* install_nets( symrec_t** symtab, inst_net_t* net,
        UT_array* scope_stack, ast_node_t* ast, igraph_t* g )
{
    int node_id;
    symrec_t* rec = NULL;
    virt_net_t* v_net = NULL;
    virt_net_t* v_net1 = NULL;
    virt_net_t* v_net2 = NULL;
    inst_rec_t* inst = NULL;
    char error_msg[ CONST_ERROR_LEN ];

    if( ast == NULL ) return NULL;

    switch( ast->type ) {
        case AST_PARALLEL:
            v_net1 = install_nets( symtab, net, scope_stack, ast->op->left, g );
            if( v_net1 == NULL ) return NULL;
            v_net2 = install_nets( symtab, net, scope_stack, ast->op->right, g );
            if( v_net2 == NULL ) return NULL;
            cpsync_connects( net, v_net1, v_net2, true, g );
            v_net = virt_net_create_parallel( v_net1, v_net2 );
            virt_net_destroy( v_net1 );
            virt_net_destroy( v_net2 );
#if defined(DEBUG) || defined(DEBUG_CONNECT)
            printf( "Parallel combination done, v_net:\n " );
            debug_print_vports( v_net );
            printf( "\n" );
#endif // DEBUG_CONNECT
            break;
        case AST_SERIAL:
            v_net1 = install_nets( symtab, net, scope_stack, ast->op->left, g );
            if( v_net1 == NULL ) return NULL;
            v_net2 = install_nets( symtab, net, scope_stack, ast->op->right, g );
            if( v_net2 == NULL ) return NULL;
            // check connections and update virtual net
            check_connections( net, v_net1, v_net2, g );
            check_connection_missing( net, v_net1, v_net2, g );
            cpsync_connects( net, v_net1, v_net2, false, g );
            v_net = virt_net_create_serial( v_net1, v_net2 );
            virt_net_destroy( v_net1 );
            virt_net_destroy( v_net2 );
#if defined(DEBUG) || defined(DEBUG_CONNECT)
            printf( "Serial combination done, v_net:\n " );
            debug_print_vports( v_net );
            printf( "\n" );
#endif // DEBUG_CONNECT
            break;
        case AST_ID:
            // check the context of the symbol
            rec = symrec_get( symtab, scope_stack, ast->symbol->name,
                    ast->symbol->line, 0 );
            if( rec == NULL ) return NULL;

            // check type of the record
            if( rec->type == SYMREC_NET ) {
                // net -> copy the virtual net
                v_net = virt_net_copy( rec->attr_net->v_net );
            }
            else if( rec->type == SYMREC_NET_PROTO ) {
                // prototype -> net definition is missing
                sprintf( error_msg, ERROR_UNDEF_NET, ERR_ERROR, rec->name );
                report_yyerror( error_msg, ast->symbol->line );
            }
            else {
                // symbol -> add net symbol to the instance table
                node_id = igraph_vcount( g );
                inst = inst_rec_put( &net->nodes, ast->symbol->name,
                        node_id, ast->symbol->line, INSTREC_NET, rec );
                // update graph and virtual net
                igraph_add_vertices( g, 1, NULL );
                igraph_cattribute_VAS_set( g, "label", node_id,
                        rec->name );
                igraph_cattribute_VAS_set( g, "func", node_id,
                        rec->attr_box->impl_name );
                v_net = virt_net_create( rec, inst );
            }
            break;
        default:
            ;
    }
    return v_net;
}
