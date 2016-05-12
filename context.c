#include "context.h"
#include "defines.h"
#include "cgraph.h"
#include "error.h"


/******************************************************************************/
void update_con_graph( igraph_t* g, igraph_t* g_con, inst_rec* inst1,
        inst_rec* inst2 )
{
    igraph_vector_t nvid_l;
    igraph_vector_t nvid_r;
    igraph_vector_init( &nvid_l, 0 );
    igraph_vector_init( &nvid_r, 0 );
    if( inst1->type == VAL_CP )
        igraph_neighbors( g, &nvid_l, inst1->id, IGRAPH_ALL );
    else
        igraph_vector_push_back( &nvid_l, inst1->id );
    if( inst2->type == VAL_CP )
        igraph_neighbors( g, &nvid_r, inst2->id, IGRAPH_ALL );
    else
        igraph_vector_push_back( &nvid_r, inst2->id );
    cgraph_disconnect( g_con, &nvid_l, &nvid_r );
    igraph_vector_destroy( &nvid_l );
    igraph_vector_destroy( &nvid_r );
}

/******************************************************************************/
void check_connection_cp( inst_net* net, virt_net* v_net1, virt_net* v_net2 )
{
    virt_ports* port1 = NULL;
    virt_ports* port2 = NULL;
    virt_ports* port_last = NULL;
    virt_ports* port_next = NULL;
    inst_rec* cp_sync = NULL;
    int net_id;

    port1 = v_net1->ports;
    while( port1 != NULL ) {
        port2 = v_net2->ports;
        port_last = NULL;
        while( port2 != NULL ) {
            port_next = port2->next;
            if( // the port names match?
                ( strcmp( port1->rec->name, port2->rec->name ) == 0 )
                // AND the port classes are compatible?
                && ( ( port1->attr_class == port2->attr_class )
                    || ( ( port1->attr_class != VAL_SIDE )
                        && ( port2->attr_class == VAL_NONE ) )
                    || ( ( port1->attr_class == VAL_NONE )
                        && ( port2->attr_class != VAL_SIDE ) )
                   )
            ) {
                // create copy synchronizer instance
                if( ( port1->inst->type == VAL_CP )
                        && ( port2->inst->type == VAL_CP ) ) {
                    // merge copy synchronizers
                    net_id = cgraph_merge_vertices( &net->g, port1->inst->id,
                            port2->inst->id );
                    inst_rec_cleanup( net, port2->inst, net_id,
                            igraph_vcount( &net->g ) );
                    cp_sync = port1->inst;
                }
                else if( port1->inst->type == VAL_CP ) {
                    cp_sync = port1->inst;
                    cgraph_connect_dir( &net->g, cp_sync->id, port2->inst->id,
                            VAL_BI, port2->attr_mode );
                }
                else if( port2->inst->type == VAL_CP ) {
                    cp_sync = port2->inst;
                    cgraph_connect_dir( &net->g, cp_sync->id, port1->inst->id,
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
                    cgraph_connect_dir( &net->g, cp_sync->id, port1->inst->id,
                            VAL_BI, port1->attr_mode );
                    cgraph_connect_dir( &net->g, cp_sync->id, port2->inst->id,
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
    int net_id;

    ports_l = v_net1->ports;
    while( ports_l != NULL ) {
        ports_next_l = ports_l->next;
        ports_r = v_net2->ports;
        ports_last_r = NULL;
        while( ports_r != NULL ) {
            ports_next_r = ports_r->next;
#if defined(DEBUG) || defined(DEBUG_CONNECT)
            printf( "check_connection: " );
            if( ports_l->attr_mode == VAL_IN ) printf( "in " );
            else if( ports_l->attr_mode == VAL_OUT ) printf( "out " );
            printf( "%s(%d).%s and ", ports_l->inst->name, ports_l->inst->id,
                    ports_l->rec->name );
            if( ports_r->attr_mode == VAL_IN ) printf( "in " );
            else if( ports_r->attr_mode == VAL_OUT ) printf( "out " );
            printf( "%s(%d).%s:", ports_r->inst->name, ports_r->inst->id,
                    ports_r->rec->name );
#endif // DEBUG_CONNECT
            if( // the port names match?
                ( strcmp( ports_l->rec->name, ports_r->rec->name ) == 0 )
                // AND the port classes are compatible?
                && (// the left port is in DS or in no class?
                    ( ( ports_l->attr_class == VAL_DOWN )
                        || ( ports_l->attr_class == VAL_NONE ) )
                    // AND the right port is in US or in no class?
                    && ( ( ports_r->attr_class == VAL_UP )
                        || ( ports_r->attr_class == VAL_NONE ) )
                   )
            ) {
                if( ( ports_l->inst->type == VAL_CP )
                        && ( ports_r->inst->type == VAL_CP ) ) {
                    update_con_graph( &net->g, g_con, ports_l->inst,
                            ports_r->inst );
                    // merge copy synchronizers
                    net_id = cgraph_merge_vertices( &net->g, ports_l->inst->id,
                            ports_r->inst->id );
                    cgraph_merge_vertices( g_con, ports_l->inst->id,
                            ports_r->inst->id );
                    inst_rec_cleanup( net, ports_r->inst, net_id,
                            igraph_vcount( &net->g ) );
#if defined(DEBUG) || defined(DEBUG_CONNECT)
                    printf( " -> connection is valid\n" );
#endif // DEBUG_CONNECT
                }
                else if( ( ports_l->attr_mode != ports_r->attr_mode )
                        || ( ports_l->attr_mode == VAL_BI )
                        || ( ports_r->attr_mode == VAL_BI )
                    ) {
                    update_con_graph( &net->g, g_con, ports_l->inst,
                            ports_r->inst );
                    cgraph_connect_dir( &net->g, ports_l->inst->id,
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
void debug_print_ports( virt_net* v_net )
{
    virt_ports* ports = NULL;
    if( v_net->ports != NULL )
        ports = v_net->ports;
    while( ports != NULL ) {
        printf( "%s, ", ports->rec->name );
        ports = ports->next;
    }
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
            check_connection_cp( net, v_net1, v_net2 );
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
            cgraph_connect( &g, &v_net1->con->right, &v_net2->con->left );
            // check connections and update virtual net
            check_connection( net, v_net1, v_net2, &g );
            check_connection_cp( net, v_net1, v_net2 );
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
