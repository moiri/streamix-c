#include "context.h"
#include "defines.h"

/******************************************************************************/
void check_context( ast_node* ast )
{
    inst_net* nets = NULL;        // hash table to store the nets
    symrec* symtab = NULL;        // hash table to store the symbols
    UT_array* scope_stack = NULL; // stack to handle the scope
    int scope = 0;

    utarray_new( scope_stack, &ut_int_icd );
    utarray_push_back( scope_stack, &scope );
    // install all symbols in the symtab
    install_ids( &symtab, scope_stack, ast->node, false );
    // check the context of all symbols and install instances in the insttab
    /* instrec_put( &insttab, VAL_THIS, *utarray_back( scope_stack ), */
    /*         VAL_SELF, -1, NULL ); */
    check_ids( &symtab, &nets, scope_stack, ast->node );
    /* // check the connections and count the connection of each port */
    /* check_instances( &insttab, ast ); */
    /* // check whether all ports are connected spawn synchronizers and draw the */
    /* // nodes, synchroniyers and connections */
    /* check_port_all( &insttab, ast ); */
}

/******************************************************************************/
void check_ids( symrec** symtab, inst_net** nets, UT_array* scope_stack,
        ast_node* ast )
{
    ast_list* list = NULL;
    /* symrec* rec = NULL; */
    inst_rec* recs_id = NULL;
    inst_rec* recs_name = NULL;
    static int _scope = 0;

    if( ast == NULL ) return;

    switch( ast->type ) {
        case AST_LINKS:
        case AST_STMTS:
            list = ast->list;
            while( list != NULL ) {
                check_ids( symtab, nets, scope_stack, list->ast_node );
                list = list->next;
            }
            break;
        case AST_NET_DEF:
            check_ids( symtab, nets, scope_stack, ast->def.op );
            break;
        case AST_BOX_DEF:
            _scope++;
            break;
        case AST_WRAP:
            // in order to add the this instance to the instance table we need
            // to get the decalration of the net before we put another scope on
            // the stack
            /* rec = symrec_get( symtab, scope_stack, ast->wrap.id->ast_id.name, */
            /*         ast->wrap.id->ast_id.line ); */
            _scope++;
            utarray_push_back( scope_stack, &_scope );
            // add the symbol 'this' to the instance table referring to this net
            /* instrec_put( insttab, VAL_THIS, *utarray_back( scope_stack ), */
            /*         VAL_SELF, ast->wrap.id->id, rec ); */
            check_ids( symtab, nets, scope_stack, ast->wrap.stmts );
            utarray_pop_back( scope_stack );
            break;
        case AST_NET:
            check_ids_net( symtab, &recs_name, &recs_id, scope_stack,
                    ast->node );
            inst_net_put( nets, *utarray_back( scope_stack ), &recs_name,
                    &recs_id );
            (*nets)->recs_id = &recs_id;
            (*nets)->recs_name = &recs_name;
            break;
        default:
            ;
    }
}

/******************************************************************************/
void check_ids_net( symrec** symtab, inst_rec** recs_name, inst_rec** recs_id,
        UT_array* scope_stack, ast_node* ast )
{
    symrec* rec = NULL;

    if( ast == NULL ) return;

    switch( ast->type ) {
        case AST_PARALLEL:
        case AST_SERIAL:
            check_ids_net( symtab, recs_name, recs_id, scope_stack,
                    ast->op.left );
            check_ids_net( symtab, recs_name, recs_id, scope_stack,
                    ast->op.right );
            break;
        case AST_ID:
            // check the context of the symbol
            rec = symrec_get( symtab, scope_stack, ast->symbol.name,
                    ast->symbol.line );
            // add a net symbol to the instance table
            if( ast->symbol.type == ID_NET && rec != NULL ) {
                inst_rec_put( recs_name, recs_id, ast->symbol.name, ast->id,
                        rec );
            }
            break;
        default:
            ;
    }
}

/******************************************************************************/
void* install_ids( symrec** symtab, UT_array* scope_stack, ast_node* ast,
        bool is_sync )
{
    ast_list* list = NULL;
    net_attr* b_attr = NULL;
    port_attr* p_attr = NULL;
    symrec_list* ptr = NULL;
    symrec_list* port_list = NULL;
    void* res = NULL;
    bool set_sync = false;
    int type;
    static int _scope = 0;
    static int _sync_id = 0; // used to assemble ports to sync groups

    if( ast == NULL ) return NULL;

    switch( ast->type ) {
        case AST_NET_DEF:
            // install the net symbol without attributes
            symrec_put( symtab, ast->def.id->symbol.name,
                    *utarray_back( scope_stack ), VAL_NET, NULL,
                    ast->def.id->symbol.line );
            break;
        case AST_BOX_DEF:
            // get the box attributes
            _scope++;
            utarray_push_back( scope_stack, &_scope );
            b_attr = ( struct net_attr* )install_ids( symtab, scope_stack,
                    ast->def.op, set_sync );
            utarray_pop_back( scope_stack );
            // install the box symbol
            symrec_put( symtab, ast->def.id->symbol.name,
                    *utarray_back( scope_stack ), VAL_BOX, ( void* )b_attr,
                    ast->def.id->symbol.line );
            break;
        case AST_SYNC:
            _sync_id++;
            set_sync = true;
        case AST_PORTS:
            list = ast->list;
            while (list != NULL) {
                res = install_ids( symtab, scope_stack, list->ast_node,
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
                install_ids( symtab, scope_stack, list->ast_node, set_sync );
                list = list->next;
            }
            break;
        case AST_BOX:
            port_list = ( struct symrec_list* )install_ids( symtab, scope_stack,
                    ast->box.ports, set_sync );
            // prepare symbol attributes and install symbol
            b_attr = ( net_attr* )malloc( sizeof( net_attr ) );
            b_attr->state = ( ast->box.attr == NULL ) ? true : false;
            b_attr->ports = port_list;
            // add internal name if available
            b_attr->impl_name = ( char* )malloc( strlen(
                        ast->box.impl->symbol.name ) + 1 );
            strcpy( b_attr->impl_name,
                    ast->box.impl->symbol.name );
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
            b_attr->state = false;
            b_attr->ports = port_list;
            // install 'this' in the scope of the wrapper
            symrec_put( symtab, VAL_THIS,
                    *utarray_back( scope_stack ), VAL_WRAPPER, ( void* )b_attr,
                    ast->wrap.id->symbol.line );
            utarray_pop_back( scope_stack );
            // install the wrapper symbol in the scope of its declaration
            symrec_put( symtab, ast->wrap.id->symbol.name,
                    *utarray_back( scope_stack ), VAL_WRAPPER, ( void* )b_attr,
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
