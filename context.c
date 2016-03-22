#include "context.h"
#include "defines.h"
#include "insttab.h"

/******************************************************************************/
void check_context( ast_node* ast )
{
    symrec* insttab = NULL;       // hash table to store the instances
    symrec* symtab = NULL;        // hash table to store the symbols
    UT_array* scope_stack = NULL; // stack to handle the scope
    int scope = 0;

    utarray_new( scope_stack, &ut_int_icd );
    utarray_push_back( scope_stack, &scope );
    // install all symbols in the symtab
    install_ids( &symtab, scope_stack, ast, false );
    // check the context of all symbols and install instances in the insttab
    instrec_put( &insttab, VAL_THIS, *utarray_back( scope_stack ),
            VAL_SELF, -1, NULL );
    check_ids( &symtab, &insttab, scope_stack, ast );
    /* // check the connections and count the connection of each port */
    /* check_instances( &insttab, ast ); */
    /* // check whether all ports are connected spawn synchronizers and draw the */
    /* // nodes, synchroniyers and connections */
    /* check_port_all( &insttab, ast ); */
}

/******************************************************************************/
void check_ids( symrec** symtab, symrec** insttab, UT_array* scope_stack,
        ast_node* ast )
{
    ast_list* list = NULL;
    symrec* rec = NULL;
    static int _scope = 0;

    if( ast == NULL ) return;

    switch( ast->node_type ) {
        case AST_LINKS:
        case AST_STMTS:
            list = ast->ast_list;
            while( list != NULL ) {
                check_ids( symtab, insttab, scope_stack, list->ast_node );
                list = list->next;
            }
            break;
        case AST_BOX:
            _scope++;
            break;
        case AST_WRAP:
            // in order to add the this instance to the instance table we need
            // to get the decalration of the net before we put another scope on
            // the stack
            rec = symrec_get( symtab, scope_stack, ast->wrap.id->ast_id.name,
                    ast->wrap.id->ast_id.line );
            _scope++;
            utarray_push_back( scope_stack, &_scope );
            // add the symbol 'this' to the instance table referring to this net
            instrec_put( insttab, VAL_THIS, *utarray_back( scope_stack ),
                    VAL_SELF, ast->wrap.id->id, rec );
            check_ids( symtab, insttab, scope_stack, ast->wrap.stmts );
            utarray_pop_back( scope_stack );
            break;
        case AST_NET:
            check_ids( symtab, insttab, scope_stack, ast->ast_node );
            break;
        case AST_PARALLEL:
        case AST_SERIAL:
            check_ids( symtab, insttab, scope_stack, ast->op.left );
            check_ids( symtab, insttab, scope_stack, ast->op.right );
            break;
        case AST_ID:
            // check the context of the symbol
            rec = symrec_get( symtab, scope_stack, ast->ast_id.name,
                    ast->ast_id.line );
            // add a net symbol to the instance table
            if( ast->ast_id.type == ID_NET && rec != NULL ) {
                instrec_put( insttab, ast->ast_id.name,
                        *utarray_back( scope_stack ), rec->type, ast->id,
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

    switch( ast->node_type ) {
        case AST_SYNC:
            _sync_id++;
            set_sync = true;
        case AST_PORTS:
            list = ast->ast_list;
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
            list = ast->ast_list;
            while (list != NULL) {
                install_ids( symtab, scope_stack, list->ast_node, set_sync );
                list = list->next;
            }
            break;
        case AST_BOX:
            _scope++;
            utarray_push_back( scope_stack, &_scope );
            port_list = ( struct symrec_list* )install_ids( symtab, scope_stack,
                    ast->box.ports, set_sync );
            utarray_pop_back( scope_stack );
            // prepare symbol attributes and install symbol
            b_attr = ( net_attr* )malloc( sizeof( net_attr ) );
            b_attr->state = ( ast->box.state == NULL ) ? true : false;
            b_attr->ports = port_list;
            symrec_put( symtab, ast->box.id->ast_id.name,
                    *utarray_back( scope_stack ), VAL_BOX, ( void* )b_attr,
                    ast->box.id->ast_id.line );
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
                    ast->wrap.id->ast_id.line );
            utarray_pop_back( scope_stack );
            // install the wrapper symbol in the scope of its declaration
            symrec_put( symtab, ast->wrap.id->ast_id.name,
                    *utarray_back( scope_stack ), VAL_WRAPPER, ( void* )b_attr,
                    ast->wrap.id->ast_id.line );
            break;
        case AST_PORT:
            // prepare symbol attributes
            p_attr = ( port_attr* )malloc( sizeof( port_attr ) );
            if( ast->port.mode != NULL )
                p_attr->mode = ast->port.mode->ast_node->ast_attr.val;
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
                    ast->port.collection->ast_node->ast_attr.val;
            }
            // install symbol and return pointer to the symbol record
            res = ( void* )symrec_put( symtab, ast->port.id->ast_id.name,
                    *utarray_back( scope_stack ), type, p_attr,
                    ast->port.id->ast_id.line );
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
