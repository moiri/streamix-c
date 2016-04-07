#include "context.h"
#include "defines.h"
#include "cgraph.h"
#include "error.h"


/******************************************************************************/
void check_connection( inst_rec* rec_l, inst_rec* rec_r )
{
    char error_msg[ CONST_ERROR_LEN ];
    port_attr* p_attr_l = NULL;
    port_attr* p_attr_r = NULL;
    symrec_list* ports_l = NULL;
    symrec_list* ports_r = NULL;
    bool is_connected = false;
    ports_l = rec_l->ports;
    while( ports_l != NULL ) {
        ports_r = rec_r->ports;
        p_attr_l = ports_l->rec->attr;
        while( ports_r != NULL ) {
            p_attr_r = ports_r->rec->attr;
#if defined(DEBUG) || defined(DEBUG_CONNECT)
            printf( "check_connection: " );
            if( p_attr_l->mode == VAL_IN ) printf( "in " );
            else if( p_attr_l->mode == VAL_OUT ) printf( "out " );
            printf( "%s(%d).%s and ", rec_l->name, rec_l->id,
                    ports_l->rec->name );
            if( p_attr_r->mode == VAL_IN ) printf( "in " );
            else if( p_attr_r->mode == VAL_OUT ) printf( "out " );
            printf( "%s(%d).%s:", rec_r->name, rec_r->id,
                    ports_r->rec->name );
#endif // DEBUG_CONNECT
            if( // the port names match?
                ( strcmp( ports_l->rec->name, ports_r->rec->name ) == 0 )
                // AND the port classes are compatible?
                && (// the left port is in DS or in no class?
                    ( ( p_attr_l->collection == VAL_DOWN )
                        || ( p_attr_l->collection == VAL_NONE ) )
                    // AND the right port is in US or in no class?
                    && ( ( p_attr_r->collection == VAL_UP )
                        || ( p_attr_r->collection == VAL_NONE ) )
                   )
            ) {
                if ( p_attr_l->mode != p_attr_r->mode ) {
                    is_connected = true;
#if defined(DEBUG) || defined(DEBUG_SERIAL)
                    printf( " -> connection is valid\n" );
#endif // DEBUG_SERIAL
                }
                else {
                    sprintf( error_msg, ERROR_BAD_MODE, ERR_ERROR,
                            ports_l->rec->name, rec_l->name, rec_l->id,
                            rec_r->name, rec_r->id, ports_r->rec->line );
                    report_yyerror( error_msg, ports_r->rec->line );
                }
            }
            else {
#if defined(DEBUG) || defined(DEBUG_SERIAL)
                printf( " -> connection is invalid\n" );
#endif // DEBUG_SERIAL
            }
            ports_r = ports_r->next;
        }
        ports_l = ports_l->next;
    }

    // perform further checks on port connections
    if( !is_connected ) {
        // ERROR: there is no connection between the two nets
        sprintf( error_msg, ERROR_NO_NET_CON, ERR_ERROR, rec_l->name,
                rec_l->id, rec_r->name, rec_r->id );
        report_yyerror( error_msg, rec_l->line );
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
    net_attr* n_attr = NULL;
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
                    *utarray_back( scope_stack ), VAL_NET, attr,
                    ast->assign.id->symbol.line );
            break;
        case AST_NET:
            // install net instances
            net = inst_net_put( nets, *utarray_back( scope_stack ) );
            install_nets( symtab, &net->recs_name, &net->recs_id, scope_stack,
                    ast->node, net->con, &net->g );
#if defined(DEBUG) || defined(DEBUG_NET_DOT)
            igraph_write_graph_dot( &net->g, stdout );
#endif // DEBUG_NET_DOT
            // check connections of the instances in this net
            check_nets( &net->recs_id, &net->g );
            // prepare attributes
            port_list = get_port_list_net( &net->recs_id, &net->con->left,
                    VAL_UP, NULL );
            port_list = get_port_list_net( &net->recs_id, &net->con->right,
                    VAL_DOWN, port_list );
            n_attr = ( net_attr* )malloc( sizeof( net_attr ) );
            n_attr->attr_static = false;
            n_attr->attr_pure = false;
            n_attr->ports = port_list;
            res = ( void* )n_attr;
            break;
        case AST_NET_PROTO:
            break;
        case AST_SYNCS:
            _sync_id++;
            set_sync = true;
        case AST_PORTS:
            list = ast->list;
            while (list != NULL) {
                res = check_context_ast( symtab, scope_stack, list->node,
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
            port_list = ( struct symrec_list* )check_context_ast( symtab, scope_stack,
                    ast->box.ports, set_sync );
            // prepare symbol attributes and install symbol
            n_attr = ( net_attr* )malloc( sizeof( net_attr ) );
            n_attr->attr_static = false;
            n_attr->attr_pure = ( ast->box.attr_pure != NULL ) ? true : false;
            n_attr->ports = port_list;
            // add internal name if available
            n_attr->impl_name = ( char* )malloc( strlen(
                        ast->box.impl->symbol.name ) + 1 );
            strcpy( n_attr->impl_name,
                    ast->box.impl->symbol.name );
            utarray_pop_back( scope_stack );
            res = ( void* )n_attr;
            break;
        case AST_WRAP:
            _scope++;
            utarray_push_back( scope_stack, &_scope );
            check_context_ast( symtab, scope_stack, ast->wrap.stmts, set_sync );
            port_list = ( struct symrec_list* )check_context_ast( symtab, scope_stack,
                    ast->wrap.ports, set_sync );
            // prepare symbol attributes and install symbol
            n_attr = ( net_attr* )malloc( sizeof( net_attr ) );
            n_attr->attr_static =
                ( ast->wrap.attr_static != NULL ) ? true : false;
            n_attr->attr_pure = false;
            n_attr->ports = port_list;
            // install 'this' in the scope of the wrapper
            symrec_put( symtab, VAL_THIS,
                    *utarray_back( scope_stack ), VAL_NET, ( void* )n_attr,
                    ast->wrap.id->symbol.line );
            utarray_pop_back( scope_stack );
            // install the wrapper symbol in the scope of its declaration
            symrec_put( symtab, ast->wrap.id->symbol.name,
                    *utarray_back( scope_stack ), VAL_NET, ( void* )n_attr,
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
symrec_list* get_port_list_net( inst_rec** recs, igraph_vector_t* v, int dir, 
        symrec_list* port_list )
{
    int i;
    inst_rec* inst;
    symrec_list* last_port = NULL;
    port_attr* p_attr = NULL;
    for( i=0; i<igraph_vector_size( v ); i++ ) {
        inst = inst_rec_get_id( recs, VECTOR( *v )[i] );
        while( inst->ports != NULL ) {
            p_attr = ( struct port_attr* )inst->ports->rec->attr;
            if( p_attr->collection == dir ) {
                last_port = port_list;
                port_list = ( symrec_list* )malloc( sizeof( symrec_list ) );
                port_list->rec = inst->ports->rec;
                port_list->next = last_port;
            }
        }
    }
    return port_list;
}

/******************************************************************************/
void install_nets( symrec** symtab, inst_rec** recs_name, inst_rec** recs_id,
        UT_array* scope_stack, ast_node* ast, net_con* con, igraph_t* g )
{
    symrec* rec = NULL;
    net_con* lcon = ( net_con* )0;

    if( ast == NULL ) return;

    switch( ast->type ) {
        case AST_PARALLEL:
        case AST_SERIAL:
            lcon = ( net_con* )malloc( sizeof( net_con ) );
            // traverse left side
            install_nets( symtab, recs_name, recs_id, scope_stack,
                    ast->op.left, con, g );
            // save connection vectors in a temporary var
            igraph_vector_copy( &lcon->left, &con->left );
            igraph_vector_copy( &lcon->right, &con->right );
            igraph_vector_clear( &con->left );
            igraph_vector_clear( &con->right );
            // traverse right side with again empty vectors
            install_nets( symtab, recs_name, recs_id, scope_stack,
                    ast->op.right, con, g );
            // connect tcon->right (left.con->right) with con->left
            // (right.con->left)
            if( ast->type == AST_SERIAL ) {
                cgraph_connect( g, &lcon->right, &con->left );
            }
            // update final connection vectors
            if( ast->type == AST_PARALLEL ) {
                // cleft = ( right.cleft, left.cleft )
                igraph_vector_append( &con->left, &lcon->left );
                // cright = ( right.cright, left.cright )
                igraph_vector_append( &con->right, &lcon->right );
            }
            else if( ast->type == AST_SERIAL ) {
                // cleft = ( left.cleft )
                igraph_vector_update( &con->left, &lcon->left );
                // cright = ( right.cright ) is already set in con->right
            }

            // free memory from temp connection var
            igraph_vector_destroy( &lcon->left );
            igraph_vector_destroy( &lcon->right );
            free( lcon );
            break;
        case AST_ID:
            // check the context of the symbol
            rec = symrec_get( symtab, scope_stack, ast->symbol.name,
                    ast->symbol.line );
            // add a net symbol to the instance table
            if( rec != NULL ) {
                // the vertice id is # of vertices before adding a new one
                igraph_vector_push_back( &con->left, igraph_vcount( g ) );
                igraph_vector_push_back( &con->right, igraph_vcount( g ) );
                inst_rec_put( recs_name, recs_id, ast->symbol.name,
                        igraph_vcount( g ), ast->symbol.line, rec );
                // add new vertex to graph
                igraph_add_vertices( g, 1, NULL );
            }
            break;
        default:
            ;
    }
}

/******************************************************************************/
void ___check_nets( symrec** symtab, inst_net** nets, UT_array* scope_stack,
        ast_node* ast )
{
    inst_net* net = NULL;
    ast_list* list = NULL;
    static int _scope = 0;

    if( ast == NULL ) return;

    switch( ast->type ) {
        case AST_PROGRAM:
            check_nets( symtab, nets, scope_stack, ast->program.stmts );
            check_nets( symtab, nets, scope_stack, ast->program.net );
            break;
        case AST_STMTS:
            list = ast->list;
            while( list != NULL ) {
                check_nets( symtab, nets, scope_stack, list->node );
                list = list->next;
            }
            break;
        case AST_BOX:
            _scope++;
            break;
        case AST_WRAP:
            _scope++;
            utarray_push_back( scope_stack, &_scope );
            check_nets( symtab, nets, scope_stack, ast->wrap.stmts );
            utarray_pop_back( scope_stack );
            break;
        case AST_ASSIGN:
            check_nets( symtab, nets, scope_stack, ast->assign.op );
            break;
        case AST_NET:
            net = inst_net_put( nets, *utarray_back( scope_stack ) );
            check_net( symtab, &net->recs_name, &net->recs_id, scope_stack,
                    ast->node, net->con, &net->g );
#if defined(DEBUG) || defined(DEBUG_NET_DOT)
            igraph_write_graph_dot( &net->g, stdout );
#endif // DEBUG_NET_DOT
            break;
        default:
            ;
    }
}

/******************************************************************************/
void check_nets( inst_rec** recs, igraph_t* g )
{
    inst_rec* rec;
    inst_rec* rec_c;
    igraph_vector_t vids;
    int idx;
    for( rec = *recs; rec != NULL; rec=rec->hh1.next ) {
        igraph_vector_init( &vids, 0 );
        // get all ids that connect to the right
        igraph_neighbors( g, &vids, rec->id, IGRAPH_OUT );
        for( idx = 0; idx < igraph_vector_size( &vids ); idx++ ) {
            rec_c = inst_rec_get_id( &rec, ( int )VECTOR( vids )[ idx ] );
#if defined(DEBUG) || defined(DEBUG_SERIAL)
            printf( "check_instances: connect instances %s(%d) and"
                    " %s(%d)\n", rec->name, rec->id, rec_c->name,
                    rec_c->id );
#endif // DEBUG_SERIAL
            check_connection( rec, rec_c );
        }
        igraph_vector_destroy( &vids );
    }
}

/******************************************************************************/
void check_instances( inst_net** nets )
{
    inst_net* net;
    inst_net* net_t;
    inst_rec* rec;
    inst_rec* rec_c;
    igraph_vector_t vids;
    int idx;
    // iterate through all scopes
    for( net = *nets; net != NULL; net=net->hh.next ) {
        net_t = net;
#if defined(DEBUG) || defined(DEBUG_SERIAL)
        printf( "check_instances: scope %d\n", net->scope );
#endif // DEBUG_SERIAL
        // in each net, iterate through all instance ids
        for( rec = net_t->recs_id; rec != NULL; rec=rec->hh1.next ) {
            igraph_vector_init( &vids, 0 );
            // get all ids that connect to the right
            igraph_neighbors( &net_t->g, &vids, rec->id, IGRAPH_OUT );
            for( idx = 0; idx < igraph_vector_size( &vids ); idx++ ) {
                rec_c = inst_rec_get_id( &net_t->recs_id,
                        ( int )VECTOR( vids )[ idx ] );
#if defined(DEBUG) || defined(DEBUG_SERIAL)
                printf( "check_instances: connect instances %s(%d) and"
                        " %s(%d)\n", rec->name, rec->id, rec_c->name,
                        rec_c->id );
#endif // DEBUG_SERIAL
                check_connection( rec, rec_c );
            }
            igraph_vector_destroy( &vids );
        }
    }
}

/******************************************************************************/
void* install_ids( symrec** symtab, UT_array* scope_stack, ast_node* ast,
        bool is_sync )
{
    ast_list* list = NULL;
    net_attr* b_attr = NULL;
    port_attr* p_attr = NULL;
    void* attr = NULL;
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
            install_ids( symtab, scope_stack, ast->program.stmts, set_sync );
            install_ids( symtab, scope_stack, ast->program.net, set_sync );
            break;
        case AST_ASSIGN:
            // get the box attributes
            attr = install_ids( symtab, scope_stack, ast->assign.op, set_sync );
            // install the box symbol
            symrec_put( symtab, ast->assign.id->symbol.name,
                    *utarray_back( scope_stack ), VAL_NET, attr,
                    ast->assign.id->symbol.line );
            break;
        case AST_SYNCS:
            _sync_id++;
            set_sync = true;
        case AST_PORTS:
            list = ast->list;
            while (list != NULL) {
                res = install_ids( symtab, scope_stack, list->node,
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
        case AST_STMTS:
            list = ast->list;
            while (list != NULL) {
                install_ids( symtab, scope_stack, list->node, set_sync );
                list = list->next;
            }
            break;
        case AST_BOX:
            _scope++;
            utarray_push_back( scope_stack, &_scope );
            port_list = ( struct symrec_list* )install_ids( symtab, scope_stack,
                    ast->box.ports, set_sync );
            // prepare symbol attributes and install symbol
            b_attr = ( net_attr* )malloc( sizeof( net_attr ) );
            b_attr->attr_static = false;
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
            install_ids( symtab, scope_stack, ast->wrap.stmts, set_sync );
            port_list = ( struct symrec_list* )install_ids( symtab, scope_stack,
                    ast->wrap.ports, set_sync );
            // prepare symbol attributes and install symbol
            b_attr = ( net_attr* )malloc( sizeof( net_attr ) );
            b_attr->attr_static =
                ( ast->wrap.attr_static != NULL ) ? true : false;
            b_attr->attr_pure = false;
            b_attr->ports = port_list;
            // install 'this' in the scope of the wrapper
            symrec_put( symtab, VAL_THIS,
                    *utarray_back( scope_stack ), VAL_NET, ( void* )b_attr,
                    ast->wrap.id->symbol.line );
            utarray_pop_back( scope_stack );
            // install the wrapper symbol in the scope of its declaration
            symrec_put( symtab, ast->wrap.id->symbol.name,
                    *utarray_back( scope_stack ), VAL_NET, ( void* )b_attr,
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
