#include <stdlib.h> /* For malloc in symbol table */
#include <stdio.h>
#include "symtab.h"
#include "utarray.h"
#include "defines.h"
#ifdef DOT_CON
#include "graph.h"
#endif // DOT_CON

/* handle errors with the bison error function */
extern void yyerror ( const char* );
extern int yylineno;
extern int yynerrs;
extern int __node_id;
char __error_msg[ CONST_ERROR_LEN ];

/* global variables */
UT_array*   __scope_stack;  // stack to handle the scope
#ifdef DOT_CON
FILE*       __n_con_graph;  // file handler for the net connection graph
FILE*       __p_con_graph;  // file handler for the port connection graph
extern int  __node_id;
#endif // DOT_CON

/******************************************************************************/
void check_context( ast_node* ast ) {
    symrec* insttab = NULL;       // hash table to store the instances
    symrec* symtab = NULL;        // hash table to store the symbols
    int scope = 0;

#ifdef DOT_CON
    __n_con_graph = fopen( N_CON_DOT_PATH, "w" );
    __p_con_graph = fopen( P_CON_DOT_PATH, "w" );
#endif // DOT_CON

    utarray_new( __scope_stack, &ut_int_icd );
    utarray_push_back( __scope_stack, &scope );
    // install all symbols in the symtab
    id_install( &symtab, ast, false );
    // check the context of all symbols and install instances in the insttab
    id_check( &symtab, &insttab, ast );
    // check the connections and draw the connection graphs
    inst_check( &insttab, ast );

#ifdef DOT_CON
    fclose( __n_con_graph );
    fclose( __p_con_graph );
    graph_fix_dot( TEMP_DOT_PATH, N_CON_DOT_PATH );
    graph_fix_dot( TEMP_DOT_PATH, P_CON_DOT_PATH );
#endif // DOT_CON
}

/******************************************************************************/
void connection_check( symrec** insttab, ast_node* ast, bool connect ) {
    ast_list* i_ptr;
    ast_list* j_ptr;
    ast_node* op_left;
    ast_node* op_right;
    // match ports of left.con_right with right.con_left
    i_ptr = ast->op.left->op.con_right;
    do {
        if (ast->op.left->node_type == AST_ID) {
            // left operator is an ID
            op_left = ast->op.left;
            i_ptr = ( ast_list* )0;   // stop condition of outer loop
        }
        else {
            // left operator is a net (opeartion)
            op_left = i_ptr->ast_node;
            i_ptr = i_ptr->next;
        }
        j_ptr = ast->op.right->op.con_left;
        do {
            if (ast->op.right->node_type == AST_ID) {
                // right operator is an ID
                op_right = ast->op.right;
                j_ptr = ( ast_list* )0;
            }
            else {
                // right operator is a net (opeartion)
                op_right = j_ptr->ast_node;
                j_ptr = j_ptr->next;
            }
            /* printf( "'%s' conncets with '%s'\n", op_left->ast_id.name, */
            /*         op_right->ast_id.name ); */
#ifdef DOT_CON
            if( connect ) graph_add_edge( __n_con_graph, op_left->id,
                    op_right->id, NULL, STYLE_E_DEFAULT );
#endif // DOT_CON
            connection_check_port( insttab, op_left, op_right, connect );
        }
        while (j_ptr != 0);
    }
    while (i_ptr != 0);

    // perform checks on port connections
    if( connect ) {
        // do it only after the connections have been established
        // left operators
        connection_check_port_all( insttab, ast->op.left,
                ast->op.left->op.con_right );

        // right operators
        connection_check_port_all( insttab, ast->op.right,
                ast->op.right->op.con_left );
    }
}

/******************************************************************************/
void connection_check_port( symrec** insttab, ast_node* net1, ast_node* net2,
        bool connect ) {
    bool is_connected = false;
    symrec_list* ports_left = NULL;
    symrec_list* ports_right = NULL;
    port_attr* p_attr_left = NULL;
    port_attr* p_attr_right = NULL;

    // get net instances from instance tables
    symrec* op_left = instrec_get( insttab, net1->ast_id.name,
            *utarray_back( __scope_stack ), net1->id );
    if ( op_left == NULL ) return;
    symrec* op_right = instrec_get( insttab, net2->ast_id.name,
            *utarray_back( __scope_stack ), net2->id );
    if ( op_right == NULL ) return;

    // check whether ports can connect
    ports_left = ( ( struct inst_attr* ) op_left->attr )->ports;
    while ( ports_left != NULL ) {
        p_attr_left = ( struct port_attr* )ports_left->rec->attr;
        ports_right = ( ( struct inst_attr* ) op_right->attr )->ports;
        while (ports_right != NULL ) {
            p_attr_right = ( struct port_attr* )ports_right->rec->attr;
            if( // ports have the same name
                ( strcmp( ports_left->rec->name, ports_right->rec->name ) == 0 )
                // the left port is in DS
                && ( ( p_attr_left->collection == VAL_DOWN )
                    // or we are doing checks and the port is in no collection
                        || ( !connect
                            && ( p_attr_left->collection == VAL_NONE ) ) )
                // the right port is in US
                && ( ( p_attr_right->collection == VAL_UP )
                    // or we are doing checks and the port is in no collection
                        || ( !connect
                            && ( p_attr_right->collection == VAL_NONE ) ) )
              ) {
                if( connect ) {
                    // checks heve been done previously, now do connections
                    connect_port( insttab, op_left, op_right, ports_left,
                            ports_right, false );
                }
                else if( p_attr_left->mode == p_attr_right->mode ) {
                    // ERROR: cannot connect ports with the same name and mode
                    sprintf( __error_msg, ERROR_BAD_MODE, ERR_ERROR,
                            ports_right->rec->name, op_left->name,
                            ( ( struct inst_attr* )op_left->attr )->id,
                            op_right->name,
                            ( ( struct inst_attr* )op_right->attr )->id,
                            ports_left->rec->line );
                    report_yyerror( __error_msg, ports_right->rec->line );
                }
                else {
                    // we are only checking and there is no mode error
                    // -> assign collections and increase connection count
                    p_attr_left->collection = VAL_DOWN;
                    p_attr_right->collection = VAL_UP;
                    ports_left->connect_cnt++;
                    ports_right->connect_cnt++;
                    is_connected = true;
                    /* printf( "Connection check of %s.%s and %s.%s\n", */
                    /*         net1->ast_id.name, ports_left->rec->name, */
                    /*         net2->ast_id.name, ports_right->rec->name ); */
                }
            }
            ports_right = ports_right->next;
        }
        ports_left = ports_left->next;
    }

    // perform further checks on port connections
    if( !connect && !is_connected ) {
        // ERROR: there is no connection between the two nets
        sprintf( __error_msg, ERROR_NO_NET_CON, ERR_ERROR, op_left->name,
                ( ( struct inst_attr* )op_left->attr )->id, op_right->name,
                ( ( struct inst_attr* )op_right->attr )->id );
        report_yyerror( __error_msg, net2->ast_id.line );
    }
}

/******************************************************************************/
void connection_check_port_all( symrec** insttab, ast_node* op,
        ast_list* ptr ) {
    symrec* op_inst = NULL;
    symrec_list* ports = NULL;
    // left operators
    do {
        if (op->node_type == AST_ID) {
            // left operator is an ID
            ptr = ( ast_list* )0;   // stop condition of loop
        }
        else {
            // left operator is a net (opeartion)
            op = ptr->ast_node;
            ptr = ptr->next;
        }
        // get net instance from instance table
        op_inst = instrec_get( insttab, op->ast_id.name,
                *utarray_back( __scope_stack ), op->id );
        if ( op_inst == NULL ) return;
        // iterate trough ports and check the connection count
        ports = ( ( struct inst_attr* ) op_inst->attr )->ports;
        while ( ports != NULL ) {
            if( ports->connect_cnt == 0 ) {
                // ERROR: this port is left unconnected
                sprintf( __error_msg, ERROR_NO_PORT_CON, ERR_ERROR,
                        ports->rec->name, op_inst->name,
                        ( ( struct inst_attr* )op_inst->attr )->id );
                report_yyerror( __error_msg, op->ast_id.line );
            }
            ports = ports->next;
        }
    }
    while (ptr != 0);
}

/******************************************************************************/
void connection_check_side( symrec** insttab, ast_node* ast, bool connect ) {
    ast_list* list = NULL;
    ast_list* list_next = NULL;
    symrec* net = NULL;
    symrec* net_next = NULL;
    symrec* op_left = NULL;
    symrec* op_right = NULL;
    bool first = true;

    // iterate through all symbols in the connection list
    list_next = list = ast->connect.connects->ast_list;
    do {
        /* printf( "====> new run\n" ); */
        while( list != NULL ) {
            /* printf( "list: %p\n", list ); */
            // the IDs of all the instances connect referres to is unknown
            if( net == NULL )
                net = instrec_get( insttab, list->ast_node->ast_id.name,
                        *utarray_back( __scope_stack ), -1 );
            // iterate through all the instances with the same symbol
            while( net != NULL ) {
                /* printf( "net: %p\n", net ); */
                if( op_left == NULL ) {
                    // left operand is not yet assigned
                    op_left = net;
                    /* printf( "op_left: %s\n", op_left->name ); */
                }
                else {
                    // left operand is already assigned, lets assign the right one
                    op_right = net;
                    /* printf( "op_right: %s\n", op_right->name ); */
                    // do the connection check
                    /* printf( "Check side port connection %s(%d) -- %s(%d)\n", */
                    /*         op_left->name, */
                    /*         ( ( struct inst_attr* )op_left->attr )->id, */
                    /*         op_right->name, */
                    /*         ( ( struct inst_attr* )op_right->attr )->id ); */
                    connection_check_side_port( insttab, op_left, op_right,
                            ast->connect.id, connect );
                }
                net = net->next;
                if( first ) {
                    /* printf( "net_next: %p\n", net ); */
                    net_next = net;
                    if( net_next == NULL ) list_next = list_next->next;
                    first = false;
                }
            }
            list = list->next;
            if( first ) {
                /* printf( "list_next: %p\n", list ); */
                list_next = list;
                first = false;
            }
        }
        // check the next element in the list with every other element
        op_left = NULL;
        op_right = NULL;
        first = true;
        list = list_next;
        net = net_next;
    } while( ( list_next != NULL ) );

    /* printf( "We are done here\n\n" ); */

}

/******************************************************************************/
void connection_check_side_port( symrec** insttab, symrec* op_left,
        symrec* op_right, ast_node* ast_con_id, bool connect ) {
    symrec_list* port_left = NULL;
    symrec_list* port_right = NULL;

    port_left = connection_check_side_port_get( op_left, ast_con_id );
    port_right = connection_check_side_port_get( op_right, ast_con_id );

    // check whether ports can connect
    if( ( port_left != NULL ) && ( port_right != NULL )
        && ( ( ( struct port_attr* )port_left->rec->attr )->mode !=
            ( ( struct port_attr* )port_right->rec->attr )->mode ) ) {
        if( connect ) {
            // checks heve been done previously, now do connections
            connect_port( insttab, op_left, op_right, port_left, port_right,
                    true );
        }
        else {
            // we are only checking and there is no mode error
            // -> assign collections and increase connection count
            port_left->connect_cnt++;
            port_right->connect_cnt++;
            /* printf( "Connection check of %s.%s and %s.%s\n", */
            /*         net1->ast_id.name, ports_left->rec->name, */
            /*         net2->ast_id.name, ports_right->rec->name ); */
        }
    }
}

/******************************************************************************/
symrec_list* connection_check_side_port_get( symrec* op,
        ast_node* ast_con_id ) {
    symrec_list* ports = NULL;
    port_attr* p_attr = NULL;

    ports = ( ( struct inst_attr* ) op->attr )->ports;
    while ( ports != NULL ) {
        p_attr = ( struct port_attr* )ports->rec->attr;
        if( // ports have the same name
            ( strcmp( ports->rec->name, ast_con_id->ast_id.name ) == 0 )
            // the left port is in SP
            && ( p_attr->collection == VAL_SIDE )
              ) {
            break;
        }
        ports = ports->next;
    }
    if( ports == NULL ) {
        // ERROR: this net has no such side port
        sprintf( __error_msg, ERROR_NO_PORT, ERR_ERROR,
                ast_con_id->ast_id.name, op->name,
                ( ( struct inst_attr* )op->attr)->id );
        report_yyerror( __error_msg, ast_con_id->ast_id.line );
    }
    return ports;
}

/******************************************************************************/
void connect_port( symrec** insttab, symrec* op_left, symrec* op_right,
        symrec_list* ports_left, symrec_list* ports_right, bool side ) {
#ifdef DOT_CON
    int id_node_start, id_node_end, id_temp;
    int style_edge = STYLE_E_DEFAULT;
    if( side ) style_edge = STYLE_E_SIDE;
#endif // DOT_CON
    /* printf( "Connect %s.%s with %s.%s\n", op_left->name, */
    /*         ports_left->rec->name, op_right->name, */
    /*         ports_right->rec->name ); */
    if( ( ports_left->connect_cnt > 1 )
            && ( ports_left->cp_sync == NULL ) )
        spawn_synchronizer( insttab, ports_left,
                ( ( struct inst_attr* )op_left->attr )->id, side );
    if( ( ports_right->connect_cnt > 1 )
            && ( ports_right->cp_sync == NULL ) )
        spawn_synchronizer( insttab, ports_right,
                ( ( struct inst_attr* )op_right->attr )->id, side );

#ifdef DOT_CON
    if( ports_left->cp_sync != NULL ) {
        id_node_start = ( ( struct inst_attr* )ports_left->cp_sync->attr )->id;
    }
    else id_node_start = ( ( struct inst_attr* )op_left->attr )->id;
    if( ports_right->cp_sync != NULL ) {
        id_node_end = ( ( struct inst_attr* )ports_right->cp_sync->attr )->id;
    }
    else id_node_end = ( ( struct inst_attr* )op_right->attr )->id;

    if( ( ( struct port_attr* )ports_left->rec->attr )->mode == VAL_IN ) {
        id_temp = id_node_start;
        id_node_start = id_node_end;
        id_node_end = id_temp;
    }
    graph_add_edge( __p_con_graph, id_node_start, id_node_end,
            ports_left->rec->name, style_edge );
#endif // DOT_CON
}

/******************************************************************************/
void id_check( symrec** symtab, symrec** insttab, ast_node* ast ) {
    ast_list* list = NULL;
    symrec* rec = NULL;
    static int _scope = 0;

    if( ast == NULL ) return;

    switch( ast->node_type ) {
        case AST_CONNECT:
            id_check( symtab, insttab, ast->connect.connects );
            break;
        case AST_CONNECTS:
            list = ast->ast_list;
            while( list != NULL ) {
                id_check( symtab, insttab, list->ast_node );
                list = list->next;
            }
            break;
        case AST_STMTS:
            list = ast->ast_list;
            while( list != NULL ) {
                id_check( symtab, insttab, list->ast_node );
                list = list->next;
            }
            break;
        case AST_BOX:
            _scope++;
            break;
        case AST_WRAP:
            _scope++;
            utarray_push_back( __scope_stack, &_scope );
            id_check( symtab, insttab, ast->wrap.stmts );
            utarray_pop_back( __scope_stack );
            break;
        case AST_NET:
            id_check( symtab, insttab, ast->ast_node );
            break;
        case AST_PARALLEL:
            id_check( symtab, insttab, ast->op.left );
            id_check( symtab, insttab, ast->op.right );
            break;
        case AST_SERIAL:
            id_check( symtab, insttab, ast->op.left );
            id_check( symtab, insttab, ast->op.right );
            break;
        case AST_ID:
            // check the context of the symbol
            rec = symrec_get( symtab, ast->ast_id.name, ast->ast_id.line );
            // add a net symbol to the instance table
            if( ast->ast_id.type == ID_NET && rec != NULL ) {
                /* printf( "put instance %s(%d)\n", ast->ast_id.name, ast->id ); */
                instrec_put( insttab, ast->ast_id.name,
                        *utarray_back( __scope_stack ), ast->ast_id.type,
                        ast->id, rec );
            }
            break;
        default:
            ;
    }
}

/******************************************************************************/
void* id_install( symrec** symtab, ast_node* ast, bool is_sync ) {
    ast_list* list = NULL;
    net_attr* b_attr = NULL;
    port_attr* p_attr = NULL;
    symrec_list* ptr = NULL;
    symrec_list* port_list = NULL;
    void* res = NULL;
    bool set_sync = false;
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
                res = id_install( symtab, list->ast_node, set_sync );
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
                id_install( symtab, list->ast_node, set_sync );
                list = list->next;
            }
            break;
        case AST_BOX:
            _scope++;
            utarray_push_back( __scope_stack, &_scope );
            port_list = ( struct symrec_list* )id_install( symtab,
                    ast->box.ports, set_sync );
            utarray_pop_back( __scope_stack );
            // prepare symbol attributes and install symbol
            b_attr = ( net_attr* )malloc( sizeof( net_attr ) );
            b_attr->state = ( ast->box.state == NULL ) ? true : false;
            b_attr->ports = port_list;
            symrec_put( symtab, ast->box.id->ast_id.name,
                    *utarray_back( __scope_stack ), ast->box.id->ast_id.type,
                    ( void* )b_attr, ast->box.id->ast_id.line );
            break;
        case AST_WRAP:
            _scope++;
            utarray_push_back( __scope_stack, &_scope );
            id_install( symtab, ast->wrap.stmts, set_sync );
            port_list = ( struct symrec_list* )id_install( symtab,
                    ast->wrap.ports, set_sync );
            utarray_pop_back( __scope_stack );
            // prepare symbol attributes and install symbol
            b_attr = ( net_attr* )malloc( sizeof( net_attr ) );
            b_attr->state = false;
            b_attr->ports = port_list;
            symrec_put( symtab, ast->wrap.id->ast_id.name,
                    *utarray_back( __scope_stack ), ast->wrap.id->ast_id.type,
                    ( void* )b_attr, ast->wrap.id->ast_id.line );
            break;
        case AST_PORT:
            // prepare symbol attributes
            if( ast->port.collection != NULL )
                list = ast->port.collection->ast_list;
            // insert one port for each collection attribute but only one if no
            // collection is set.
            do {
                p_attr = ( port_attr* )malloc( sizeof( port_attr ) );
                p_attr->mode = ast->port.mode->ast_node->ast_attr.val;
                // add sync attributes if port is a sync port
                if( is_sync ) {
                    p_attr->decoupled =
                        (ast->port.coupling == NULL) ? false : true;
                    p_attr->sync_id = _sync_id;
                }
                // set collection
                if ( list == NULL )
                    p_attr->collection = VAL_NONE;
                else {
                    p_attr->collection = list->ast_node->ast_attr.val;
                    list = list->next;
                }
                // install symbol and return pointer to the symbol record
                res = ( void* )symrec_put( symtab,
                        ast->port.id->ast_id.name,
                        *utarray_back( __scope_stack ),
                        ast->port.id->ast_id.type, p_attr,
                        ast->port.id->ast_id.line );
                ptr = ( symrec_list* )malloc( sizeof( symrec_list ) );
                ptr->rec = ( struct symrec* )res;
                ptr->next = port_list;
                port_list = ptr;
            }
            while( list != NULL );
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
void inst_check( symrec** insttab, ast_node* ast ) {
    ast_list* list = NULL;
    static int _scope = 0;

    if( ast == NULL ) return;

    switch( ast->node_type ) {
        case AST_CONNECT:
/* #ifdef DOT_CON */
/*             // create a copy synchroniyer for each connect instruction */
/*             graph_add_divider ( __p_con_graph, *utarray_back( __scope_stack ), */
/*                     FLAG_CONNECT ); */
/*             graph_add_node( __p_con_graph, ast->connect.id->id, "+", */
/*                     STYLE_N_NET_CP ); */
/* #endif // DOT_CON */
            connection_check_side( insttab, ast, false );
            connection_check_side( insttab, ast, true );
            break;
        case AST_STMTS:
#ifdef DOT_CON
            graph_add_divider ( __n_con_graph, *utarray_back( __scope_stack ),
                    FLAG_STMTS );
            graph_add_divider ( __p_con_graph, *utarray_back( __scope_stack ),
                    FLAG_STMTS );
            graph_init( __n_con_graph, STYLE_G_CON_NET );
            graph_init( __p_con_graph, STYLE_G_CON_PORT );
#endif // DOT_CON
            list = ast->ast_list;
            while( list != NULL ) {
                inst_check( insttab, list->ast_node );
                list = list->next;
            }
#ifdef DOT_CON
            graph_add_divider ( __n_con_graph, *utarray_back( __scope_stack ),
                    FLAG_STMTS_END );
            graph_add_divider ( __p_con_graph, *utarray_back( __scope_stack ),
                    FLAG_STMTS_END );
            graph_finish( __n_con_graph );
            graph_finish( __p_con_graph );
#endif // DOT_CON
            break;
        case AST_BOX:
            _scope++;
            break;
        case AST_WRAP:
            _scope++;
            utarray_push_back( __scope_stack, &_scope );
#ifdef DOT_CON
            graph_add_divider ( __n_con_graph, *utarray_back( __scope_stack ),
                    FLAG_WRAP );
            graph_add_divider ( __p_con_graph, *utarray_back( __scope_stack ),
                    FLAG_WRAP );
            graph_init_subgraph( __n_con_graph, ast->wrap.id->ast_id.name,
                    STYLE_SG_WRAPPER );
            graph_init_subgraph( __p_con_graph, ast->wrap.id->ast_id.name,
                    STYLE_SG_WRAPPER );
#endif // DOT_CON
            inst_check( insttab, ast->wrap.stmts );
            // add synchroniyers if necessary
            // ...
#ifdef DOT_CON
            graph_add_divider ( __n_con_graph, *utarray_back( __scope_stack ),
                    FLAG_WRAP_END );
            graph_add_divider ( __p_con_graph, *utarray_back( __scope_stack ),
                    FLAG_WRAP_END );
            graph_finish_subgraph( __n_con_graph );
            graph_finish_subgraph( __p_con_graph );
#endif // DOT_CON
            // add invisible elements outside the wrapper and their connections
            utarray_pop_back( __scope_stack );
            break;
        case AST_NET:
            inst_check( insttab, ast->ast_node );
            break;
        case AST_PARALLEL:
#if defined(DOT_CON) && defined(DOT_STRUCT)
            graph_add_divider ( __n_con_graph, *utarray_back( __scope_stack ),
                    FLAG_NET );
            graph_add_divider ( __p_con_graph, *utarray_back( __scope_stack ),
                    FLAG_NET );
            graph_init_subgraph( __n_con_graph, "", STYLE_SG_PARALLEL );
            graph_init_subgraph( __p_con_graph, "", STYLE_SG_PARALLEL );
#endif // DOT_CON && DOT_STRUCT
            inst_check( insttab, ast->op.left );
            inst_check( insttab, ast->op.right );
#if defined(DOT_CON) && defined(DOT_STRUCT)
            graph_add_divider ( __n_con_graph, *utarray_back( __scope_stack ),
                    FLAG_NET );
            graph_add_divider ( __p_con_graph, *utarray_back( __scope_stack ),
                    FLAG_NET );
            graph_finish_subgraph( __n_con_graph );
            graph_finish_subgraph( __p_con_graph );
#endif // DOT_CON && DOT_STRUCT
            break;
        case AST_SERIAL:
#if defined(DOT_CON) && defined(DOT_STRUCT)
            graph_add_divider ( __n_con_graph, *utarray_back( __scope_stack ),
                    FLAG_NET );
            graph_add_divider ( __p_con_graph, *utarray_back( __scope_stack ),
                    FLAG_NET );
            graph_init_subgraph( __n_con_graph, "", STYLE_SG_SERIAL );
            graph_init_subgraph( __p_con_graph, "", STYLE_SG_SERIAL );
#endif // DOT_CON && DOT_STRUCT
            inst_check( insttab, ast->op.left );
            inst_check( insttab, ast->op.right );
            // count connections and check context
            connection_check( insttab, ast, false );
            // spawn copy synchronizers and draw connections
            connection_check( insttab, ast, true );
#if defined(DOT_CON) && defined(DOT_STRUCT)
            graph_add_divider ( __n_con_graph, *utarray_back( __scope_stack ),
                    FLAG_NET );
            graph_add_divider ( __p_con_graph, *utarray_back( __scope_stack ),
                    FLAG_NET );
            graph_finish_subgraph( __n_con_graph );
            graph_finish_subgraph( __p_con_graph );
#endif // DOT_CON && DOT_STRUCT
            break;
        case AST_ID:
#ifdef DOT_CON
            if( ast->ast_id.type == ID_NET ) {
                graph_add_divider ( __n_con_graph,
                        *utarray_back( __scope_stack ), FLAG_NET );
                graph_add_divider ( __p_con_graph,
                        *utarray_back( __scope_stack ), FLAG_NET );
                graph_add_node( __n_con_graph, ast->id, ast->ast_id.name,
                        STYLE_N_NET_BOX );
                graph_add_node( __p_con_graph, ast->id, ast->ast_id.name,
                        STYLE_N_NET_BOX );
            }
#endif // DOT_CON
            break;
        default:
            ;
    }
}

/******************************************************************************/
symrec* instrec_get( symrec** insttab, char* name, int scope, int id ) {
    symrec* item = NULL;
    char key[ strlen( name ) + 1 + CONST_SCOPE_LEN ];

    // generate key
    sprintf( key, "%s%d", name, scope );
    HASH_FIND_STR( *insttab, key, item );

    if( id == -1 )
        // if no id is available, collision handling must be done manually
        return item;

    // find the instance with the matching id and handle key collisions
    while( item != NULL ) {
        if( strlen( item->name ) == strlen( name )
            && memcmp( item->name, name, strlen( name ) ) == 0
            && item->scope == scope
            && ( ( struct inst_attr* )item->attr )->id == id ) {
            /* printf( "found instance %s with id %d in scope %d\n", item->name, */
            /*         ( ( struct inst_attr* )item->attr)->id, item->scope ); */
            break; // found a match
        }
        item = item->next;
    }
    return item;
}

/******************************************************************************/
symrec* instrec_put( symrec** insttab, char* name, int scope, int type, int id,
        symrec* rec ) {
    char key[ strlen( name ) + 1 + CONST_SCOPE_LEN ];
    inst_attr* attr = NULL;
    symrec_list* inst_ports = NULL;
    symrec_list* sym_ports = NULL;
    symrec_list* port_list = NULL;
    symrec* item = NULL;
    symrec* new_item = NULL;
    symrec* previous_item = NULL;

    // PREPARE INSTREC ATTRIBUTES
    attr = ( inst_attr* )malloc( sizeof( inst_attr ) );
    attr->id = id;
    attr->net = rec;
    // copy portlist from symtab to insttab
    if( rec != NULL )
        sym_ports = ( ( struct net_attr* )rec->attr )->ports;
    while( sym_ports != NULL  ) {
        inst_ports = ( struct symrec_list* )malloc( sizeof( symrec_list ) );
        inst_ports->rec = sym_ports->rec;
        inst_ports->next = port_list;
        inst_ports->connect_cnt = 0;
        inst_ports->cp_sync = NULL;
        port_list = inst_ports;
        sym_ports = sym_ports->next;
    }
    attr->ports = port_list;

    // ADD ITEM TO THE INSTANCE TABLE
    // generate key
    sprintf( key, "%s%d", name, scope );
    // create new iten structure
    new_item = ( symrec* )malloc( sizeof( symrec ) );
    new_item->scope = scope;
    new_item->type = type;
    new_item->key = ( char* )malloc( strlen( key ) + 1 );
    strcpy( new_item->key, key );
    new_item->attr = attr;
    new_item->name = ( char* )malloc( strlen( name ) + 1 );
    strcpy( new_item->name, name );
    new_item->attr = attr;
    // check wheter key already exists
    HASH_FIND_STR( *insttab, key, item );
    if( item == NULL ) {
        // the key is new
        HASH_ADD_KEYPTR( hh, *insttab, new_item->key, strlen( key ), new_item );
    }
    else {
        // a collision accured or multiple instances of a net have been spawned
        // -> add the new item to the end of the linked list
        do {
            previous_item = item; // remember the last item of the list
            item = item->next;
        }
        while( item != NULL );

        previous_item->next = new_item;
    }
    /* printf( "put net instance %s with id %d in scope %d\n", new_item->name, */
    /*         ( ( struct inst_attr* )new_item->attr)->id, new_item->scope ); */
    return new_item;
}

/******************************************************************************/
void report_yyerror( const char* msg, int line ) {
    yylineno = line;
    yynerrs++;
    yyerror( msg );
}

/******************************************************************************/
void spawn_synchronizer( symrec** insttab, symrec_list* port, int net_id,
        bool side ) {
#ifdef DOT_CON
    int id_node_start, id_node_end, id_temp;
    char symbol[4];
    int style_edge = STYLE_E_DEFAULT;
    int style_node = STYLE_N_NET_CP;
    if( side ) {
        style_edge = STYLE_E_SIDE;
        style_node = STYLE_N_NET_CPS;
    }
    sprintf( symbol, "×" );
#endif // DOT_CON
    char name[10];
    // this port connects to multiple other ports -> a copy synchronizer
    // is needed
    __node_id++;
    sprintf( name, "%d", __node_id );
    port->cp_sync = instrec_put( insttab, name, *utarray_back( __scope_stack ),
            ID_CPSYNC, __node_id, NULL );
#ifdef DOT_CON
    // create a copy synchroniyer for each connect instruction
    graph_add_divider ( __p_con_graph, *utarray_back( __scope_stack ),
            FLAG_NET );
    id_node_start = __node_id;
    id_node_end = net_id;
    if( ( ( struct port_attr* )port->rec->attr )->mode == VAL_OUT ) {
        id_temp = id_node_start;
        id_node_start = id_node_end;
        id_node_end = id_temp;
        sprintf( symbol, "+" );
    }
    graph_add_node( __p_con_graph, __node_id, symbol, style_node );
    graph_add_edge( __p_con_graph, id_node_start, id_node_end,
            port->rec->name, style_edge );
#endif // DOT_CON
}

/******************************************************************************/
symrec* symrec_get( symrec** symtab, char *name, int line ) {
    int* p = NULL;
    symrec* item = NULL;
    char key[ strlen( name ) + 1 + CONST_SCOPE_LEN ];

    /* check whether their scope matches with a scope on the stack */
    while( ( p = ( int* )utarray_prev( __scope_stack, p ) ) != NULL ) {
        // generate key
        sprintf( key, "%s%d", name, *p );
        HASH_FIND_STR( *symtab, key, item );
        // iterate through all entries with the same key to handle collisions
        while( item != NULL ) {
            if( strlen( item->name ) == strlen( name )
                && memcmp( item->name, name, strlen( name ) ) == 0
                && item->scope == *p ) {
                break; // found a match
            }
            item = item->next;
        }
        if( item != NULL ) break; // found a match
    }
    if( item == NULL ) {
        sprintf( __error_msg, ERROR_UNDEFINED_ID, ERR_ERROR, name );
        report_yyerror( __error_msg, line );
    }
    return item;
}

/******************************************************************************/
symrec* symrec_put( symrec** symtab, char *name, int scope, int type,
        void* attr, int line ) {
    symrec* item = NULL;
    symrec* new_item = NULL;
    symrec* previous_item = NULL;
    bool is_identical = false;
    char key[ strlen( name ) + 1 + CONST_SCOPE_LEN ];

    // generate key
    sprintf( key, "%s%d", name, scope );
    // create new iten structure
    new_item = ( symrec* )malloc( sizeof( symrec ) );
    new_item->scope = scope;
    new_item->type = type;
    new_item->line = line;
    new_item->key = ( char* )malloc( strlen( key ) + 1 );
    strcpy( new_item->key, key );
    new_item->attr = attr;
    new_item->name = ( char* )malloc( strlen( name ) + 1 );
    strcpy( new_item->name, name );
    new_item->attr = attr;
    // check wheter key already exists
    HASH_FIND_STR( *symtab, key, item );
    // the key is new
    if( item == NULL ) {
        HASH_ADD_KEYPTR( hh, *symtab, new_item->key, strlen( key ), new_item );
        item = new_item;
    }
    /* hash exists */
    else {
        // iterate through all entries with the same hash to handle collisions
        // and catch ports with the same name and scope but a with different
        // collections
        do {
            // check whether there is an identical entry (name, scope, mode,
            // and collections are the same)
            if( strlen( item->name ) == strlen( name )
                && memcmp( item->name, name, strlen( name ) ) == 0
                && item->scope == scope
                && ( ( item->type == VAL_BOX || item->type == VAL_NET )
                    || ( ( type == VAL_PORT || type == VAL_SPORT )
                        && ( ( ( struct port_attr* )attr )->mode
                            == ( ( struct port_attr* )item->attr )->mode
                        && ( ( struct port_attr* )attr )->collection
                            == ( ( struct port_attr* )item->attr )->collection )
                       )
                    ) ) {
                // found an identical entry -> error
                is_identical = true;
                break;
            }
            previous_item = item;   // remember the last item of the list
            item = item->next;
        }
        while( item != NULL );

        if( !is_identical ) {
            // the item was differen -> add it to the end of the linked list
            previous_item->next = new_item;
            item = new_item;
        }
    }
    if( is_identical ) {
        // the item already existed in the table -> free the allocated space
        // and throw a yyerror
        free( new_item->name );
        free( new_item->key );
        free( new_item );
        sprintf( __error_msg, ERROR_DUPLICATE_ID, ERR_ERROR, name );
        report_yyerror( __error_msg, line );
        item = NULL;
    }
    return item;
}
