#include "context.h"
#include "defines.h"
#include "ngraph.h"
#include "smxerr.h"

/******************************************************************************/
bool are_port_names_ok( virt_ports* p1, virt_ports* p2, bool cpp, bool cps )
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
            if( ( p1->attr_class != VAL_SIDE )
                    && ( p2->attr_class == VAL_NONE ) )
                return true;
            if( ( p1->attr_class == VAL_NONE )
                    && ( p2->attr_class != VAL_SIDE ) )
                return true;
        }
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
bool check_connection( inst_net* net, virt_ports* ports_l,
        virt_ports* ports_r, igraph_t* g_con )
{
    bool res = false;
    char error_msg[ CONST_ERROR_LEN ];
#if defined(DEBUG) || defined(DEBUG_CONNECT)
    printf( "check_connection:\n " );
    debug_print_vport( ports_l );
    printf( " and " );
    debug_print_vport( ports_r );
#endif // DEBUG_CONNECT
    if( are_port_names_ok( ports_l, ports_r, false, false ) ) {
        if( ( ports_l->inst->type == VAL_CP )
                && ( ports_r->inst->type == VAL_CP ) ) {
            cgraph_update( g_con, ports_l->inst->id, ports_r->inst->id,
                    ports_l->inst->type, ports_r->inst->type, &net->g);
#if defined(DEBUG) || defined(DEBUG_CONNECT)
            printf( "\n  => connection is valid\n" );
#endif // DEBUG_CONNECT
            // merge copy synchronizers
            cpsync_merge( net, ports_l, ports_r );
            dgraph_merge_vertice_1( g_con, ports_l->inst->id,
                    ports_r->inst->id );
            res = true;
        }
        else if( are_port_modes_ok( ports_l, ports_r ) ) {
            cgraph_update( g_con, ports_l->inst->id, ports_r->inst->id,
                    ports_l->inst->type, ports_r->inst->type, &net->g);
            dgraph_connect_1( &net->g, ports_l->inst->id, ports_r->inst->id,
                    ports_l->attr_mode, ports_r->attr_mode );

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
void check_connections( inst_net* net, virt_net* v_net1, virt_net* v_net2,
        igraph_t* g_con )
{
    virt_ports* ports_l = NULL;
    virt_ports* ports_r = NULL;
    virt_ports* ports_last_l = NULL;
    virt_ports* ports_next_l = NULL;
    virt_ports* ports_last_r = NULL;
    virt_ports* ports_next_r = NULL;

    ports_l = v_net1->ports;
    while( ports_l != NULL ) {
        ports_next_l = ports_l->next;
        ports_r = v_net2->ports;
        ports_last_r = NULL;
        while( ports_r != NULL ) {
            ports_next_r = ports_r->next;
            if( check_connection( net, ports_l, ports_r, g_con ) ) {
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
void check_connection_missing( inst_net* net, igraph_t* g_con )
{
    char error_msg[ CONST_ERROR_LEN ];
    int edge_id, v1_id, v2_id;
    inst_rec* rec1 = NULL;
    inst_rec* rec2 = NULL;

    for( edge_id = 0; edge_id < igraph_ecount( g_con ); edge_id++ ) {
        igraph_edge( g_con, edge_id, &v1_id, &v2_id );
        rec1 = inst_rec_get( &net->nodes, v1_id );
        rec2 = inst_rec_get( &net->nodes, v2_id );
        // ERROR: there is no connection between the two nets
        sprintf( error_msg, ERROR_NO_NET_CON, ERR_ERROR, rec1->name, v1_id,
                rec2->name, v2_id );
        report_yyerror( error_msg, rec1->line );
    }
}

/******************************************************************************/
void check_context( ast_node* ast, inst_net** nets )
{
    symrec* symtab = NULL;        // hash table to store the symbols
    UT_array* scope_stack = NULL; // stack to handle the scope
    int scope = 0;

    utarray_new( scope_stack, &ut_int_icd );
    utarray_push_back( scope_stack, &scope );
    check_context_ast( &symtab, nets, scope_stack, ast, false );
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
    symrec* rec = NULL;
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
            // install net instances
            net = inst_net_put( nets, *utarray_back( scope_stack ) );
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
            // get the attributes
            attr = check_context_ast( symtab, nets, scope_stack, ast->assign.op,
                    false );
            // check prototype if available
            rec = symrec_search( symtab, scope_stack,
                    ast->assign.id->symbol.name );
            if( rec == NULL || rec->type != AST_NET_PROTO ) {
                // install the symbol
                symrec_put( symtab, ast->assign.id->symbol.name,
                        *utarray_back( scope_stack ), ast->assign.op->type,
                        attr, ast->assign.id->symbol.line );
            }
            else {
                // check whether types of prototype and definition match
                port_list = ( struct symrec_list* )rec->attr;
                check_prototype( port_list, attr, rec->name );
                // update record attributes to the real net
                rec->type = ast->assign.op->type;
                rec->attr = attr;
                rec->line = ast->assign.id->symbol.line;
                // free symtab ports
                while( port_list == NULL ) {
                    symrec_del( symtab, port_list->rec );
                    port_list = port_list->next;
                }
                free( port_list );
            }
            break;
        case AST_NET:
            net = inst_net_get( nets, *utarray_back( scope_stack ) );
            res = ( void* )install_nets( symtab, net, scope_stack, ast->node );
#if defined(DEBUG) || defined(DEBUG_NET_DOT)
            igraph_write_graph_dot( &net->g, stdout );
#endif // DEBUG_NET_DOT
            break;
        case AST_NET_PROTO:
            _scope++;
            utarray_push_back( scope_stack, &_scope );
            port_list = ( struct symrec_list* )check_context_ast( symtab, nets,
                    scope_stack, ast->net_prot.ports, false );
            utarray_pop_back( scope_stack );
            // install the symbol (use port list as attributes)
            symrec_put( symtab, ast->net_prot.id->symbol.name,
                    *utarray_back( scope_stack ), ast->type, port_list,
                    ast->net_prot.id->symbol.line );
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
                    scope_stack, ast->box.ports, false );
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
                    *utarray_back( scope_stack ), ast->type, ( void* )w_attr,
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
    return res;
}

/******************************************************************************/
void check_prototype( symrec_list* r_ports, virt_net* v_net, char *name )
{
    char error_msg[ CONST_ERROR_LEN ];

#if defined(DEBUG) || defined(DEBUG_PROTO)
    printf("check_prototype:\n");
    printf(" ports prot: ");
    debug_print_rports( r_ports, name );
    printf(" ports vnet: ");
    debug_print_vports( v_net );
#endif // DEBUG_PROTO
    if( !do_port_cnts_match( r_ports, v_net->ports )
        || !do_port_attrs_match( r_ports, v_net->ports ) ) {
        sprintf( error_msg, ERROR_TYPE_CONFLICT, ERR_ERROR, name );
        report_yyerror( error_msg, r_ports->rec->line );
    }
}

/******************************************************************************/
void cpsync_connect( inst_net* net, virt_ports* port1, virt_ports* port2 )
{
    inst_rec* cp_sync = NULL;
    // create copy synchronizer instance
    if( ( port1->inst->type == VAL_CP ) && ( port2->inst->type == VAL_CP ) ) {
        // merge copy synchronizers
        cp_sync = cpsync_merge( net, port1, port2 );
    }
    else if( port1->inst->type == VAL_CP ) {
        cp_sync = port1->inst;
        dgraph_connect_1( &net->g, cp_sync->id, port2->inst->id, VAL_BI,
                port2->attr_mode );
    }
    else if( port2->inst->type == VAL_CP ) {
        cp_sync = port2->inst;
        dgraph_connect_1( &net->g, cp_sync->id, port1->inst->id, VAL_BI,
                port1->attr_mode );
    }
    else {
        cp_sync = inst_rec_put( &net->nodes, TEXT_CP, igraph_vcount( &net->g ),
                0, VAL_CP, NULL );
#if defined(DEBUG) || defined(DEBUG_CONNECT)
        printf( "Create copy-synchronizer %s(%d)\n", cp_sync->name,
                cp_sync->id );
#endif // DEBUG_CONNECT
        igraph_add_vertices( &net->g, 1, NULL );
        dgraph_connect_1( &net->g, cp_sync->id, port1->inst->id, VAL_BI,
                port1->attr_mode );
        dgraph_connect_1( &net->g, cp_sync->id, port2->inst->id, VAL_BI,
                port2->attr_mode );
    }
    // change left port to copy synchronizer port
    port1->inst = cp_sync;
    // set mode of port
    port1->attr_mode = VAL_BI;
    // if possible, set port class to anything but VAL_NONE
    if( port1->attr_class == VAL_NONE ) port1->attr_class = port2->attr_class;
}

/******************************************************************************/
void cpsync_connects( inst_net* net, virt_net* v_net1, virt_net* v_net2,
        bool parallel )
{
    virt_ports* port1 = NULL;
    virt_ports* port2 = NULL;
    virt_ports* port_last = NULL;
    virt_ports* port_next = NULL;

    port1 = v_net1->ports;
    while( port1 != NULL ) {
        port2 = v_net2->ports;
        port_last = NULL;
        while( port2 != NULL ) {
            port_next = port2->next;
            if( are_port_names_ok( port1, port2, parallel, !parallel ) ) {
                cpsync_connect( net, port1, port2 );
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
    inst_rec_del( &net->nodes, port2->inst );
    if( id_del != id )
        inst_rec_replace_id( &net->nodes, id_del, id );
    // adjust all ids starting from the id of the deleted record
    for( id = id_del; id < igraph_vcount( &net->g ); id++ )
        inst_rec_replace_id( &net->nodes, id + 1, id );
    return port1->inst;
}

/******************************************************************************/
void debug_print_rport( symrec* port, char* name )
{
    port_attr* p_attr = port->attr;
    printf( "%s", name );
    if( p_attr->collection == VAL_DOWN ) printf( "_" );
    else if( p_attr->collection == VAL_UP ) printf( "^" );
    else if( p_attr->collection == VAL_SIDE ) printf( "|" );
    if( p_attr->mode == VAL_IN ) printf( "<--" );
    else if( p_attr->mode == VAL_OUT ) printf( "-->" );
    else printf( "<->" );
    printf( "%s", port->name );
}

/******************************************************************************/
void debug_print_rports( symrec_list* rports, char* name )
{
    symrec_list* ports = rports;
    while( ports != NULL ) {
        debug_print_rport( ports->rec, name );
        printf(", ");
        ports = ports->next;
    }
    printf("\n");
}

/******************************************************************************/
void debug_print_vport( virt_ports* port )
{
    printf( "%s(%d)", port->inst->name, port->inst->id );
    if( port->attr_class == VAL_DOWN ) printf( "_" );
    else if( port->attr_class == VAL_UP ) printf( "^" );
    else if( port->attr_class == VAL_SIDE ) printf( "|" );
    if( port->attr_mode == VAL_IN ) printf( "<--" );
    else if( port->attr_mode == VAL_OUT ) printf( "-->" );
    else printf( "<->" );
    printf( "%s", port->rec->name );
}

/******************************************************************************/
void debug_print_vports( virt_net* v_net )
{
    virt_ports* ports = NULL;
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
bool do_port_cnts_match( symrec_list* r_ports, virt_ports* v_ports )
{
    symrec_list* r_port_ptr = r_ports;
    virt_ports* v_port_ptr = v_ports;
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
bool do_port_attrs_match( symrec_list* r_ports, virt_ports* v_ports )
{
    symrec_list* r_port_ptr = r_ports;
    virt_ports* v_port_ptr = v_ports;
    port_attr* r_port_attr = NULL;
    bool match = false;

    r_port_ptr = r_ports;
    while( r_port_ptr != NULL  ) {
        match = false;
        v_port_ptr = v_ports;
        r_port_attr = ( struct port_attr* )r_port_ptr->rec->attr;
        while( v_port_ptr != NULL  ) {
            if( strlen( r_port_ptr->rec->name )
                    == strlen( v_port_ptr->rec->name )
                && strcmp( r_port_ptr->rec->name, v_port_ptr->rec->name ) == 0
                && r_port_attr->collection == v_port_ptr->attr_class
                && ( r_port_attr->mode == v_port_ptr->attr_mode
                    || v_port_ptr->attr_mode == VAL_BI )
                ) {
                // use more specific mode from prototype
                if( v_port_ptr->attr_mode == VAL_BI )
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
virt_net* install_nets( symrec** symtab, inst_net* net,
        UT_array* scope_stack, ast_node* ast )
{
    symrec* rec = NULL;
    virt_net* v_net1 = NULL;
    virt_net* v_net2 = NULL;
    inst_rec* inst = NULL;
    igraph_t g;
    char error_msg[ CONST_ERROR_LEN ];

    if( ast == NULL ) return NULL;

    switch( ast->type ) {
        case AST_PARALLEL:
            v_net1 = install_nets( symtab, net, scope_stack, ast->op.left );
            if( v_net1 == NULL ) return NULL;
            v_net2 = install_nets( symtab, net, scope_stack, ast->op.right );
            if( v_net2 == NULL ) return NULL;
            cpsync_connects( net, v_net1, v_net2, true );
            v_net1 = virt_net_merge_parallel( v_net1, v_net2 );
#if defined(DEBUG) || defined(DEBUG_CONNECT)
            printf( "Parallel combination done, v_net:\n " );
            debug_print_vports( v_net1 );
            printf( "\n" );
#endif // DEBUG_CONNECT
            break;
        case AST_SERIAL:
            v_net1 = install_nets( symtab, net, scope_stack, ast->op.left );
            if( v_net1 == NULL ) return NULL;
            v_net2 = install_nets( symtab, net, scope_stack, ast->op.right );
            if( v_net2 == NULL ) return NULL;
            // create connection graph
            igraph_empty( &g, igraph_vcount( &net->g ), IGRAPH_UNDIRECTED );
            cgraph_connect_full_ptr( &g, &v_net1->con->right,
                    &v_net2->con->left );
            // check connections and update virtual net
            check_connections( net, v_net1, v_net2, &g );
            check_connection_missing( net, &g );
            cpsync_connects( net, v_net1, v_net2, false );
            v_net1 = virt_net_merge_serial( v_net1, v_net2 );
            // cleanup connection graph
            igraph_destroy( &g );
#if defined(DEBUG) || defined(DEBUG_CONNECT)
            printf( "Serial combination done, v_net:\n " );
            debug_print_vports( v_net1 );
            printf( "\n" );
#endif // DEBUG_CONNECT
            break;
        case AST_ID:
            // check the context of the symbol
            rec = symrec_get( symtab, scope_stack, ast->symbol.name,
                    ast->symbol.line );
            if( rec == NULL ) return NULL;
            // add a net symbol to the instance table
            if( rec->type == AST_NET ) {
                v_net1 = ( struct virt_net* )rec->attr;
            }
            else if( rec->type == AST_NET_PROTO ) {
                sprintf( error_msg, ERROR_UNDEF_NET, ERR_ERROR, rec->name );
                report_yyerror( error_msg, ast->symbol.line );
            }
            else {
                inst = inst_rec_put( &net->nodes, ast->symbol.name,
                        igraph_vcount( &net->g ), ast->symbol.line, VAL_NET,
                        rec );
                // update graph and virtual net
                igraph_add_vertices( &net->g, 1, NULL );
                v_net1 = virt_net_create( rec, inst );
            }
            break;
        default:
            ;
    }
    return v_net1;
}
