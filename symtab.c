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
void check_context( ast_node* ast )
{
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
    install_ids( &symtab, ast, false );
    // check the context of all symbols and install instances in the insttab
    instrec_put( &insttab, VAL_THIS, *utarray_back( __scope_stack ),
            ID_WRAP, -1, NULL );
    check_ids( &symtab, &insttab, ast );
    // check the connections and count the connection of each port
    check_instances( &insttab, ast );
    // check whether all ports are connected spawn synchronizers and draw the
    // nodes, synchroniyers and connections
    check_port_all( &insttab, ast );

#ifdef DOT_CON
    fclose( __n_con_graph );
    fclose( __p_con_graph );
    graph_fix_dot( TEMP_DOT_PATH, N_CON_DOT_PATH );
    graph_fix_dot( TEMP_DOT_PATH, P_CON_DOT_PATH );
#endif // DOT_CON
}

/******************************************************************************/
void check_ids( symrec** symtab, symrec** insttab, ast_node* ast )
{
    ast_list* list = NULL;
    symrec* rec = NULL;
    static int _scope = 0;

    if( ast == NULL ) return;

    switch( ast->node_type ) {
        case AST_LINK:
        case AST_CONNECT:
            check_ids( symtab, insttab, ast->connect.connects );
            break;
        case AST_LINKS:
        case AST_CONNECTS:
            list = ast->ast_list;
            while( list != NULL ) {
                check_ids( symtab, insttab, list->ast_node );
                list = list->next;
            }
            break;
        case AST_STMTS:
            list = ast->ast_list;
            while( list != NULL ) {
                check_ids( symtab, insttab, list->ast_node );
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
            rec = symrec_get( symtab, ast->wrap.id->ast_id.name,
                    ast->wrap.id->ast_id.line );
            _scope++;
            utarray_push_back( __scope_stack, &_scope );
            // add the symbol 'this' to the instance table referring to this net
            instrec_put( insttab, VAL_THIS, *utarray_back( __scope_stack ),
                    ast->wrap.id->ast_id.type, ast->wrap.id->id, rec );
            check_ids( symtab, insttab, ast->wrap.stmts );
            utarray_pop_back( __scope_stack );
            break;
        case AST_NET:
            check_ids( symtab, insttab, ast->ast_node );
            break;
        case AST_PARALLEL:
            check_ids( symtab, insttab, ast->op.left );
            check_ids( symtab, insttab, ast->op.right );
            break;
        case AST_SERIAL:
            check_ids( symtab, insttab, ast->op.left );
            check_ids( symtab, insttab, ast->op.right );
            break;
        case AST_ID:
            // check the context of the symbol
            rec = symrec_get( symtab, ast->ast_id.name, ast->ast_id.line );
            // add a net symbol to the instance table
            if( ast->ast_id.type == ID_NET && rec != NULL ) {
                instrec_put( insttab, ast->ast_id.name,
                        *utarray_back( __scope_stack ), rec->type, ast->id,
                        rec );
            }
            break;
        default:
            ;
    }
}

/******************************************************************************/
void check_instances( symrec** insttab, ast_node* ast )
{
    ast_list* list = NULL;
    static int _scope = 0;

    if( ast == NULL ) return;

    switch( ast->node_type ) {
        case AST_LINK:
            connection_check_link( insttab, ast, false );
            break;
        case AST_CONNECT:
            connection_check_connect( insttab, ast, false );
            break;
        case AST_STMTS:
            list = ast->ast_list;
            while( list != NULL ) {
                check_instances( insttab, list->ast_node );
                list = list->next;
            }
            break;
        case AST_BOX:
            _scope++;
            break;
        case AST_WRAP:
            _scope++;
            utarray_push_back( __scope_stack, &_scope );
            check_instances( insttab, ast->wrap.stmts );
            utarray_pop_back( __scope_stack );
            break;
        case AST_NET:
            check_instances( insttab, ast->ast_node );
            break;
        case AST_PARALLEL:
            check_instances( insttab, ast->op.left );
            check_instances( insttab, ast->op.right );
            break;
        case AST_SERIAL:
            check_instances( insttab, ast->op.left );
            check_instances( insttab, ast->op.right );
            // count connections and check context
            connection_check_serial( insttab, ast, false );
            break;
        default:
            ;
    }
}

/******************************************************************************/
void check_port_all( symrec** insttab, ast_node* ast )
{
    ast_list* list = NULL;
    symrec* net = NULL;
    int net_type;
    static int _scope = 0;

    if( ast == NULL ) return;

    switch( ast->node_type ) {
        case AST_LINK:
            connection_check_link( insttab, ast, true );
            break;
        case AST_CONNECT:
            connection_check_connect( insttab, ast, true );
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
                check_port_all( insttab, list->ast_node );
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
                    ast->id, STYLE_SG_WRAPPER );
            graph_init_subgraph( __p_con_graph, ast->wrap.id->ast_id.name,
                    ast->id, STYLE_SG_WRAPPER );
#endif // DOT_CON
            connection_check_port_all( insttab, VAL_THIS, ast->wrap.id->id,
                    ast->wrap.id->ast_id.line );
#ifdef DOT_CON
            graph_add_divider ( __n_con_graph, *utarray_back( __scope_stack ),
                    FLAG_WRAP_END );
            graph_add_divider ( __p_con_graph, *utarray_back( __scope_stack ),
                    FLAG_WRAP_END );
            graph_finish_subgraph( __n_con_graph );
            graph_finish_subgraph( __p_con_graph );
#endif // DOT_CON
            check_port_all( insttab, ast->wrap.stmts );
            utarray_pop_back( __scope_stack );
            break;
        case AST_NET:
            check_port_all( insttab, ast->ast_node );
            break;
        case AST_PARALLEL:
#if defined(DOT_CON) && defined(DOT_STRUCT)
            graph_add_divider ( __n_con_graph, *utarray_back( __scope_stack ),
                    FLAG_NET );
            graph_add_divider ( __p_con_graph, *utarray_back( __scope_stack ),
                    FLAG_NET );
            graph_init_subgraph( __n_con_graph, "", ast->id,
                    STYLE_SG_PARALLEL );
            graph_init_subgraph( __p_con_graph, "", ast->id,
                    STYLE_SG_PARALLEL );
#endif // DOT_CON && DOT_STRUCT
            check_port_all( insttab, ast->op.left );
            check_port_all( insttab, ast->op.right );
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
            graph_init_subgraph( __n_con_graph, "", ast->id, STYLE_SG_SERIAL );
            graph_init_subgraph( __p_con_graph, "", ast->id, STYLE_SG_SERIAL );
#endif // DOT_CON && DOT_STRUCT
            check_port_all( insttab, ast->op.left );
            check_port_all( insttab, ast->op.right );
            // spawn copy synchronizers and draw connections
            connection_check_serial( insttab, ast, true );
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
                net = instrec_get( insttab, ast->ast_id.name,
                        *utarray_back( __scope_stack ), ast->id );
                graph_add_divider ( __n_con_graph,
                        *utarray_back( __scope_stack ), FLAG_NET );
                graph_add_divider ( __p_con_graph,
                        *utarray_back( __scope_stack ), FLAG_NET );
                net_type = STYLE_N_NET_BOX;
                if( net->type == VAL_NET ) net_type = STYLE_N_NET_WRAP;
                graph_add_node( __n_con_graph, ast->id, ast->ast_id.name,
                        net_type );
                graph_add_node( __p_con_graph, ast->id, ast->ast_id.name,
                        net_type );
            }
#endif // DOT_CON
            connection_check_port_all( insttab, ast->ast_id.name, ast->id,
                    ast->ast_id.line );
            break;
        default:
            ;
    }
}

/******************************************************************************/
void connection_check_connect( symrec** insttab, ast_node* ast, bool connect )
{
    ast_list* list = NULL;
    ast_list* list_next = NULL;
    symrec* net = NULL;
    symrec* net_next = NULL;
    symrec* op_left = NULL;
    symrec* op_right = NULL;
    bool first = true;
    bool is_connected = false;

    // iterate through all symbols in the connection list
    list_next = list = ast->connect.connects->ast_list;
    do {
        while( list != NULL ) {
            // the IDs of all the instances connect referres to is unknown
            if( net == NULL )
                net = instrec_get( insttab, list->ast_node->ast_id.name,
                        *utarray_back( __scope_stack ), -1 );
            // iterate through all the instances with the same symbol
            while( net != NULL ) {
                if( op_left == NULL ) {
                    // left operand is not yet assigned
                    op_left = net;
                }
                else {
                    // left operand is already assigned, lets assign the right one
                    op_right = net;
#if defined(DEBUG) || defined(DEBUG_CONNECT)
                    printf( "%d Check side port connection %s(%d) -- %s(%d)\n",
                            connect, op_left->name,
                            ( ( struct inst_attr* )op_left->attr )->id,
                            op_right->name,
                            ( ( struct inst_attr* )op_right->attr )->id );
#endif // DEBUG
                    // do the connection check
                    is_connected |= connection_check_connect_port( insttab,
                            op_left, op_right, ast->connect.id, connect );
                }
                net = net->next;
                if( first ) {
                    net_next = net;
                    if( net_next == NULL ) list_next = list_next->next;
                    first = false;
                }
            }
            list = list->next;
            if( first ) {
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

    if ( !connect && !is_connected ) {
        // ERROR: cannot connect side ports with the same mode
        net = instrec_get( insttab, VAL_THIS, *utarray_back( __scope_stack ),
                -1 );
        sprintf( __error_msg, ERROR_BAD_MODE_SIDE, ERR_ERROR,
                ast->connect.id->ast_id.name, net->name,
                ( ( struct inst_attr* )net->attr )->id );
        report_yyerror( __error_msg, ast->connect.id->ast_id.line );
    }
}

/******************************************************************************/
bool connection_check_connect_port( symrec** insttab, symrec* op_left,
        symrec* op_right, ast_node* ast_id, bool connect )
{
    symrec_list* port_left = NULL;
    symrec_list* port_right = NULL;
    bool is_connected = false;

    port_left = connection_check_connect_port_get( op_left, ast_id,
            connect );
    port_right = connection_check_connect_port_get( op_right, ast_id,
            connect );

    // check whether ports can connect
    if( ( port_left != NULL ) && ( port_right != NULL ) ) {
        if ( ( ( struct port_attr* )port_left->rec->attr )->mode !=
                    ( ( struct port_attr* )port_right->rec->attr )->mode ) {
            // each operand has a side port with the appropriate name and the
            // mode matches
            if( connect ) {
                // checks have been done previously, now do connections
                connect_port_connect( insttab, op_left, op_right, port_left,
                        port_right );
            }
            else {
                // we are only checking and there is no mode error
                is_connected = true;
                port_left->connect_cnt++;
                port_right->connect_cnt++;
#if defined(DEBUG) || defined(DEBUG_CONNECT)
                printf( "Connection of %s.%s and %s.%s is valid\n",
                        op_left->name, port_left->rec->name,
                        op_right->name, port_right->rec->name );
#endif // DEBUG
            }
        }
    }
    return is_connected;
}

/******************************************************************************/
symrec_list* connection_check_connect_port_get( symrec* op, ast_node* ast_id,
        bool connect )
{
    symrec_list* ports = NULL;
    port_attr* p_attr = NULL;

    ports = ( ( struct inst_attr* ) op->attr )->ports;
    while ( ports != NULL ) {
        p_attr = ( struct port_attr* )ports->rec->attr;
        if( ( strcmp( ports->rec->name, ast_id->ast_id.name ) == 0 )
            && ( p_attr->collection == VAL_SIDE ) ) {
            // ports have the same name and the collection matches
            break;
        }
        ports = ports->next;
    }
    if( ( ports == NULL ) && !connect ) {
        // ERROR: this net has no such port
        sprintf( __error_msg, ERROR_UNDEF_PORT, ERR_ERROR,
                ast_id->ast_id.name, op->name,
                ( ( struct inst_attr* )op->attr)->id );
        report_yyerror( __error_msg, ast_id->ast_id.line );
    }
    return ports;
}

/******************************************************************************/
void connection_check_link( symrec** insttab, ast_node* ast, bool connect )
{
    ast_list* list = NULL;
    symrec* op_left = NULL;
    symrec* op_right = NULL;
    int hnode_id = -1;

    // there can only be one instance of 'this' in this scope
    op_left = instrec_get( insttab, VAL_THIS, *utarray_back( __scope_stack ),
            -1 );

    // iterate through all symbols in the connection list
    list = ast->connect.connects->ast_list;
    while( list != NULL ) {
        // the IDs of all the instances connect referres to is unknown
        op_right = instrec_get( insttab, list->ast_node->ast_id.name,
                *utarray_back( __scope_stack ), -1 );

#ifdef DOT_CON
        // create an invisible node to draw the wrapper connections
        if( connect ) {
            __node_id++;
            hnode_id = __node_id;
            graph_add_divider ( __p_con_graph, *utarray_back( __scope_stack ),
                    FLAG_WRAP_PRE );
            graph_add_node( __p_con_graph, hnode_id, "", STYLE_N_NET_INVIS );
        }
#endif // DOT_CON
        // iterate through all the instances with the same symbol
        while( op_right != NULL ) {
#if defined(DEBUG) || defined(DEBUG_LINK)
            printf( "%d Check link '%s'(%s, %d) -- '%s'(%d)\n",
                    connect, op_left->name,
                    ( ( struct inst_attr* )op_left->attr )->net->name,
                    ( ( struct inst_attr* )op_left->attr )->id,
                    op_right->name,
                    ( ( struct inst_attr* )op_right->attr )->id );
#endif // DEBUG
            // do the connection check
            connection_check_link_port( insttab, op_left, op_right,
                    ast->connect.id, hnode_id, connect );
            op_right = op_right->next;
        }
        list = list->next;
    }
}

/******************************************************************************/
void connection_check_link_port( symrec** insttab, symrec* op_left,
        symrec* op_right, ast_node* ast_id, int hnode_id, bool connect )
{
    bool is_connected = false;
    symrec_list* ports_left = NULL;
    symrec_list* ports_right = NULL;
    port_attr* p_attr_left = NULL;
    port_attr* p_attr_right = NULL;
    char* name_left = NULL;
    char* name_right = NULL;
    inst_attr* i_attr_left = ( struct inst_attr* )op_left->attr;
    inst_attr* i_attr_right = ( struct inst_attr* )op_right->attr;

    // check whether ports can connect
    ports_left = ( ( struct inst_attr* ) op_left->attr )->ports;
    while ( ports_left != NULL ) {
        p_attr_left = ( struct port_attr* )ports_left->rec->attr;
        name_left = ports_left->rec->name;
        if( p_attr_left->int_name != NULL )
            name_left = p_attr_left->int_name;
        if( strcmp( name_left, ast_id->ast_id.name ) != 0 ) {
#if defined(DEBUG) || defined(DEBUG_LINK)
            printf("%s != %s in '%s'(%s, %d)\n", ast_id->ast_id.name, name_left,
                    op_left->name,
                    ( ( struct inst_attr* )op_left->attr )->net->name,
                    ( ( struct inst_attr* )op_left->attr )->id );
#endif // DEBUG
            ports_left = ports_left->next;
            continue;
        }
        ports_right = ( ( struct inst_attr* ) op_right->attr )->ports;
        while (ports_right != NULL ) {
            p_attr_right = ( struct port_attr* )ports_right->rec->attr;
            // we always need the 'outer' name of the right operand
            name_right = ports_right->rec->name;
            if( strcmp( name_right, ast_id->ast_id.name ) != 0 ) {
#if defined(DEBUG) || defined(DEBUG_LINK)
                printf("%s != %s in '%s'(%s, %d)\n", ast_id->ast_id.name,
                        name_right, op_right->name,
                        ( ( struct inst_attr* )op_right->attr )->net->name,
                        ( ( struct inst_attr* )op_right->attr )->id );
#endif // DEBUG
                ports_right = ports_right->next;
                continue;
            }
            // at this point we have two port with the same name as the link
            if( p_attr_left->mode == p_attr_right->mode ) {
                if( connect ) {
                    // checks have been done previously, now do connections
                    connect_port_link( insttab, op_left, op_right, ports_left,
                            ports_right, hnode_id );
                }
                else {
                    // we are only checking and there is no mode error
                    ports_left->connect_cnt++;
                    ports_right->connect_cnt++;
                    is_connected = true;
#if defined(DEBUG) || defined(DEBUG_LINK)
                    printf( "Link connection of %s(%d).%s and %s(%d).%s"
                            " is valid\n",
                            op_left->name, i_attr_left->id, name_left,
                            op_right->name, i_attr_right->id, name_right
                    );
#endif // DEBUG
                }
            }
            else if( !connect ) {
                // ERROR: cannot link ports with different modes
                sprintf( __error_msg, ERROR_BAD_MODE, ERR_ERROR,
                        ast_id->ast_id.name, op_left->name, i_attr_left->id,
                        op_right->name, i_attr_right->id,
                        ports_left->rec->line );
                report_yyerror( __error_msg, ports_right->rec->line );
            }
            ports_right = ports_right->next;
        }
        ports_left = ports_left->next;
    }

    // perform further checks on port connections
    if( !connect && !is_connected ) {
        // ERROR: there is no connection between the wrapper and this net
        sprintf( __error_msg, ERROR_UNDEF_PORT, ERR_ERROR, ast_id->ast_id.name,
                op_right->name, i_attr_right->id );
        report_yyerror( __error_msg, ast_id->ast_id.line );
    }
}

/******************************************************************************/
void connection_check_port_all( symrec** insttab, char* name, int id, int line )
{
    symrec* op_inst = NULL;
    symrec_list* ports = NULL;
    port_attr* p_attr = NULL;
    inst_attr* i_attr = NULL;
    char* port_name = NULL;
    // get net instance from instance table
    op_inst = instrec_get( insttab, name, *utarray_back( __scope_stack ), id );
    if ( op_inst == NULL ) return;
    i_attr = ( struct inst_attr* )op_inst->attr;
    // iterate trough ports and check the connection count
    ports = ( ( struct inst_attr* ) op_inst->attr )->ports;
    p_attr = ( struct port_attr* )ports->rec->attr;
    while ( ports != NULL ) {
        port_name = ports->rec->name;
        if( ( p_attr->int_name != NULL )
                && ( strcmp( name, VAL_THIS ) == 0 ) ) {
            port_name = p_attr->int_name;
        }
#if defined(DEBUG) || defined(DEBUG_PORT)
        printf( "Is %s(%d).%s connected?\n", op_inst->name, i_attr->id,
                port_name );
#endif // DEBUG
        if( ports->connect_cnt == 0 ) {
            // ERROR: this port is left unconnected
            sprintf( __error_msg, ERROR_NO_PORT_CON, ERR_ERROR, port_name,
                    i_attr->net->name, op_inst->name, i_attr->id );
            report_yyerror( __error_msg, line );
        }
        ports = ports->next;
    }
}

/******************************************************************************/
void connection_check_serial( symrec** insttab, ast_node* ast, bool connect )
{
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
#if defined(DEBUG) || defined(DEBUG_SERIAL)
            printf( "'%s' conncets with '%s'\n", op_left->ast_id.name,
                    op_right->ast_id.name );
#endif // DEBUG
#ifdef DOT_CON
            if( connect ) graph_add_edge( __n_con_graph, op_left->id,
                    op_right->id, NULL, STYLE_E_DEFAULT );
#endif // DOT_CON
            connection_check_serial_port( insttab, op_left, op_right, connect );
        }
        while (j_ptr != 0);
    }
    while (i_ptr != 0);
/* #ifdef DOT_CON */
/*         if( connect ) graph_add_rank( __p_con_graph, op_left->id, */
/*                 op_right->id ); */
/* #endif // DOT_CON */

}

/******************************************************************************/
void connection_check_serial_port( symrec** insttab, ast_node* net1,
        ast_node* net2, bool connect )
{
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
                        /* || ( !connect */
                            /* && ( p_attr_left->collection == VAL_NONE ) ) ) */
                    || ( p_attr_left->collection == VAL_NONE ) )
                // the right port is in US
                && ( ( p_attr_right->collection == VAL_UP )
                    // or we are doing checks and the port is in no collection
                        /* || ( !connect */
                        /*     && ( p_attr_right->collection == VAL_NONE ) ) ) */
                    || ( p_attr_right->collection == VAL_NONE ) )
              ) {
                if( p_attr_left->mode != p_attr_right->mode ) {
                    if( connect ) {
                        // checks have been done previously, now do connections
                        connect_port_serial( insttab, op_left, op_right,
                                ports_left, ports_right );
                    }
                    else {
                        // we are only checking and there is no mode error
                        ports_left->connect_cnt++;
                        ports_right->connect_cnt++;
                        is_connected = true;
#if defined(DEBUG) || defined(DEBUG_SERIAL)
                        printf( "Serial connection of %s(%d).%s and"
                                " %s(%d).%s is valid \n", op_left->name,
                                ( ( struct inst_attr* )op_left->attr )->id,
                                ports_left->rec->name, op_right->name,
                                ( ( struct inst_attr* )op_right->attr )->id,
                                ports_right->rec->name );
#endif // DEBUG
                    }
                }
                else if( !connect ) {
                    // ERROR: cannot connect ports with the same name and mode
                    sprintf( __error_msg, ERROR_BAD_MODE, ERR_ERROR,
                            ports_right->rec->name, op_left->name,
                            ( ( struct inst_attr* )op_left->attr )->id,
                            op_right->name,
                            ( ( struct inst_attr* )op_right->attr )->id,
                            ports_left->rec->line );
                    report_yyerror( __error_msg, ports_right->rec->line );
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
void connect_port_connect( symrec** insttab, symrec* op_left, symrec* op_right,
        symrec_list* ports_left, symrec_list* ports_right )
{
#ifdef DOT_CON
    int id_node_start, id_node_end, id_temp;
    int style_edge = STYLE_E_SPORT;
#endif // DOT_CON
#if defined(DEBUG) || defined(DEBUG_CONNECT)
    printf( "Connect %s.%s with %s.%s\n", op_left->name, ports_left->rec->name,
            op_right->name, ports_right->rec->name );
#endif // DEBUG
    if( ( ports_left->connect_cnt > 1 )
            && ( ports_left->cp_sync == NULL ) )
        spawn_synchronizer( insttab, ports_left,
                ( ( struct inst_attr* )op_left->attr )->id, TYPE_CONNECT );
    if( ( ports_right->connect_cnt > 1 )
            && ( ports_right->cp_sync == NULL ) )
        spawn_synchronizer( insttab, ports_right,
                ( ( struct inst_attr* )op_right->attr )->id, TYPE_CONNECT );

#ifdef DOT_CON
    if( ports_left->cp_sync != NULL ) {
        id_node_start = ( ( struct inst_attr* )ports_left->cp_sync->attr )->id;
        style_edge = STYLE_E_SPORT;
    }
    else {
        id_node_start = ( ( struct inst_attr* )op_left->attr )->id;
        style_edge = STYLE_E_SPORT_IN;
    }
    if( ports_right->cp_sync != NULL ) {
        id_node_end = ( ( struct inst_attr* )ports_right->cp_sync->attr )->id;
    }
    else {
        id_node_end = ( ( struct inst_attr* )op_right->attr )->id;
        if( style_edge == STYLE_E_SPORT_IN )
            style_edge = STYLE_E_SPORT_BI;
        else if( style_edge == STYLE_E_SPORT )
            style_edge = STYLE_E_SPORT_OUT;
    }

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
void connect_port_link( symrec** insttab, symrec* op_left, symrec* op_right,
        symrec_list* ports_left, symrec_list* ports_right, int id_invis )
{
#ifdef DOT_CON
    int id_node_start, id_node_end, id_temp;
#endif // DOT_CON
#if defined(DEBUG) || defined(DEBUG_LINK)
    printf( "Connect %s.%s with %s.%s\n", op_left->name, ports_left->rec->name,
            op_right->name, ports_right->rec->name );
#endif // DEBUG
    if( ( ports_left->connect_cnt > 1 )
            && ( ports_left->cp_sync == NULL ) )
        spawn_synchronizer( insttab, ports_left, id_invis, TYPE_LINK );
    if( ( ports_right->connect_cnt > 1 )
            && ( ports_right->cp_sync == NULL ) )
        spawn_synchronizer( insttab, ports_right,
                ( ( struct inst_attr* )op_right->attr )->id, TYPE_LS );

#ifdef DOT_CON
    if( ports_left->cp_sync != NULL ) {
        id_node_start = ( ( struct inst_attr* )ports_left->cp_sync->attr )->id;
    }
    else {
        id_node_start = id_invis;
    }
    if( ports_right->cp_sync != NULL ) {
        id_node_end = ( ( struct inst_attr* )ports_right->cp_sync->attr )->id;
    }
    else {
        id_node_end = ( ( struct inst_attr* )op_right->attr )->id;
    }

    if( ( ( struct port_attr* )ports_right->rec->attr )->mode == VAL_OUT ) {
        id_temp = id_node_start;
        id_node_start = id_node_end;
        id_node_end = id_temp;
    }
    graph_add_divider ( __p_con_graph, *utarray_back( __scope_stack ),
            FLAG_WRAP_PRE );
    graph_add_edge( __p_con_graph, id_node_start, id_node_end,
            ports_left->rec->name, STYLE_E_LPORT );
#endif // DOT_CON
}

/******************************************************************************/
void connect_port_serial( symrec** insttab, symrec* op_left, symrec* op_right,
        symrec_list* ports_left, symrec_list* ports_right )
{
#ifdef DOT_CON
    int id_node_start, id_node_end, id_temp;
#endif // DOT_CON
#if defined(DEBUG) || defined(DEBUG_SERIAL)
    printf( "Connect %s.%s with %s.%s\n", op_left->name, ports_left->rec->name,
            op_right->name, ports_right->rec->name );
#endif // DEBUG
    if( ( ports_left->connect_cnt > 1 )
            && ( ports_left->cp_sync == NULL ) )
        spawn_synchronizer( insttab, ports_left,
                ( ( struct inst_attr* )op_left->attr )->id, TYPE_SERIAL );
    if( ( ports_right->connect_cnt > 1 )
            && ( ports_right->cp_sync == NULL ) )
        spawn_synchronizer( insttab, ports_right,
                ( ( struct inst_attr* )op_right->attr )->id, TYPE_SERIAL );

#ifdef DOT_CON
    if( ports_left->cp_sync != NULL ) {
        id_node_start = ( ( struct inst_attr* )ports_left->cp_sync->attr )->id;
    }
    else {
        id_node_start = ( ( struct inst_attr* )op_left->attr )->id;
    }
    if( ports_right->cp_sync != NULL ) {
        id_node_end = ( ( struct inst_attr* )ports_right->cp_sync->attr )->id;
    }
    else {
        id_node_end = ( ( struct inst_attr* )op_right->attr )->id;
    }

    if( ( ( struct port_attr* )ports_left->rec->attr )->mode == VAL_IN ) {
        id_temp = id_node_start;
        id_node_start = id_node_end;
        id_node_end = id_temp;
    }
    graph_add_edge( __p_con_graph, id_node_start, id_node_end,
            ports_left->rec->name, STYLE_E_PORT );
#endif // DOT_CON
}

/******************************************************************************/
void* install_ids( symrec** symtab, ast_node* ast, bool is_sync )
{
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
                res = install_ids( symtab, list->ast_node, set_sync );
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
                install_ids( symtab, list->ast_node, set_sync );
                list = list->next;
            }
            break;
        case AST_BOX:
            _scope++;
            utarray_push_back( __scope_stack, &_scope );
            port_list = ( struct symrec_list* )install_ids( symtab,
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
            install_ids( symtab, ast->wrap.stmts, set_sync );
            port_list = ( struct symrec_list* )install_ids( symtab,
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
                // add internal name if available
                p_attr->int_name = NULL;
                if( ast->port.int_id != NULL ) {
                    p_attr->int_name = ( char* )malloc( strlen(
                                ast->port.int_id->ast_node->ast_id.name ) + 1 );
                    strcpy( p_attr->int_name,
                            ast->port.int_id->ast_node->ast_id.name );
                }
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
symrec* instrec_get( symrec** insttab, char* name, int scope, int id )
{
    symrec* item = NULL;
    char key[ strlen( name ) + 1 + CONST_SCOPE_LEN ];

#if defined(DEBUG) || defined(DEBUG_INST)
    printf( "search instance '%s' with id %d in scope %d\n", name, id, scope );
#endif // DEBUG

    // generate key
    sprintf( key, "%s%d", name, scope );
    HASH_FIND_STR( *insttab, key, item );

    if( id == -1 ) {
        // if no id is available, collision handling must be done manually
#if defined(DEBUG) || defined(DEBUG_INST)
        if( item != NULL )
            printf( "found instances of '%s'(%s) in scope %d\n", item->name,
                    ( ( struct inst_attr*) item->attr )->net->name,
                    item->scope );
#endif // DEBUG
        return item;
    }

    // find the instance with the matching id and handle key collisions
    while( item != NULL ) {
        if( strlen( item->name ) == strlen( name )
            && memcmp( item->name, name, strlen( name ) ) == 0
            && item->scope == scope
            && ( ( struct inst_attr* )item->attr )->id == id ) {
#if defined(DEBUG) || defined(DEBUG_INST)
            printf( "found instance %s with id %d in scope %d\n", item->name,
                    ( ( struct inst_attr* )item->attr)->id, item->scope );
#endif // DEBUG
            break; // found a match
        }
        item = item->next;
    }
    return item;
}

/******************************************************************************/
symrec* instrec_put( symrec** insttab, char* name, int scope, int type, int id,
        symrec* rec )
{
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
#if defined(DEBUG) || defined(DEBUG_INST)
    printf( "put net instance %s with id %d in scope %d\n", new_item->name,
            ( ( struct inst_attr* )new_item->attr)->id, new_item->scope );
#endif // DEBUG
    return new_item;
}

/******************************************************************************/
void report_yyerror( const char* msg, int line )
{
    yylineno = line;
    yynerrs++;
    yyerror( msg );
}

/******************************************************************************/
void spawn_synchronizer( symrec** insttab, symrec_list* port, int net_id,
        int type )
{
#ifdef DOT_CON
    int id_node_start, id_node_end, id_temp, style_edge, style_node,
        flag_node, flag_edge, mode;
    char symbol[4];
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
    // set properties according to type
    switch( type ) {
        case TYPE_CONNECT:
            mode = VAL_OUT;
            flag_edge = flag_node = FLAG_CONNECT;
            style_edge = STYLE_E_SPORT_OUT;
            style_node = STYLE_N_NET_CPS;
            break;
        case TYPE_LINK:
            mode = VAL_IN;
            flag_edge = FLAG_WRAP_PRE;
            flag_node = FLAG_NET;
            style_edge = STYLE_E_LPORT;
            style_node = STYLE_N_NET_CPL;
            break;
        case TYPE_SERIAL:
            mode = VAL_OUT;
            flag_edge = flag_node = FLAG_NET;
            style_edge = STYLE_E_PORT;
            style_node = STYLE_N_NET_CP;
            break;
        case TYPE_LS:
            mode = VAL_OUT;
            flag_edge = flag_node = FLAG_NET;
            style_edge = STYLE_E_LPORT;
            style_node = STYLE_N_NET_CPL;
            break;
    }

    // create a copy synchroniyer for each connect instruction
    id_node_start = __node_id;
    id_node_end = net_id;
    if( ( ( struct port_attr* )port->rec->attr )->mode == mode ) {
        if( type == TYPE_CONNECT ) style_edge = STYLE_E_SPORT_IN;
        id_temp = id_node_start;
        id_node_start = id_node_end;
        id_node_end = id_temp;
        sprintf( symbol, "+" );
    }
    graph_add_divider ( __p_con_graph, *utarray_back( __scope_stack ),
            flag_node );
    graph_add_node( __p_con_graph, __node_id, symbol, style_node );
    graph_add_divider ( __p_con_graph, *utarray_back( __scope_stack ),
            flag_edge );
    graph_add_edge( __p_con_graph, id_node_start, id_node_end,
            port->rec->name, style_edge );
#endif // DOT_CON
}

/******************************************************************************/
symrec* symrec_get( symrec** symtab, char *name, int line )
{
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
        sprintf( __error_msg, ERROR_UNDEF_ID, ERR_ERROR, name );
        report_yyerror( __error_msg, line );
    }
#if defined(DEBUG) || defined(DEBUG_SYMB)
    else {
        printf( "found symbol %s in scope %d\n", item->name, item->scope );
    }
#endif // DEBUG
    return item;
}

/******************************************************************************/
symrec* symrec_put( symrec** symtab, char *name, int scope, int type,
        void* attr, int line )
{
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
            // check whether there is an identical entry (name, scope, and
            // collections are the same; the mode must always be different)
            if( strlen( item->name ) == strlen( name )
                && memcmp( item->name, name, strlen( name ) ) == 0
                && item->scope == scope
                && ( ( item->type == VAL_BOX || item->type == VAL_NET )
                    || ( ( type == VAL_PORT || type == VAL_SPORT )
                        /* && ( ( ( struct port_attr* )attr )->mode */
                        /*     == ( ( struct port_attr* )item->attr )->mode ) */
                        && ( ( ( struct port_attr* )attr )->collection
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
#if defined(DEBUG) || defined(DEBUG_SYMB)
    else {
        printf( "added symbol %s in scope %d\n", item->name, item->scope );
    }
#endif // DEBUG
    return item;
}
