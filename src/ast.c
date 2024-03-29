/**
 * A simple AST plugin
 *
 * @file    ast.c
 * @author  Simon Maurer
 *
 */

#include <stdlib.h> /* For malloc to add nodes to a linked list */
#include <string.h> /* For strlen in ast_add_id */
#include <stdio.h>
#include "defines.h"
#include "ast.h"

/******************************************************************************/
ast_node_t* ast_add_assign( ast_node_t* id, ast_node_t* op, node_type_t type )
{
    ast_node_t* node = ast_add_node( AST_ASSIGN );
    node->assign = malloc( sizeof( ast_assign_t ) );
    node->assign->id = id;
    node->assign->op = op;
    node->assign->type = type;
    return node;
}

/******************************************************************************/
ast_node_t* ast_add_attr( int val, attr_type_t type )
{
    ast_node_t *node = ast_add_node( AST_ATTR );
    node->attr = malloc( sizeof( ast_attr_t ) );
    node->attr->type = type;
    node->attr->val = val;
    return node;
}

/******************************************************************************/
ast_node_t* ast_add_box( ast_node_t* id, ast_node_t* ports, ast_node_t* state,
        ast_node_t* location )
{
    ast_node_t *node = ast_add_node( AST_BOX );
    node->box = malloc( sizeof( ast_box_t ) );
    node->box->impl = id;
    node->box->ports = ports;
    node->box->attr_pure = state;
    node->box->attr_location = location;
    return node;
}

/******************************************************************************/
ast_node_t* ast_add_list( ast_list_t* list, node_type_t type )
{
    if( list == NULL ) return NULL;
    ast_node_t *node = ast_add_node( type );
    node->list = list;
    return node;
}

/******************************************************************************/
ast_list_t* ast_add_list_elem( ast_node_t* node, ast_list_t* list )
{
    ast_list_t* list_ptr;
    list_ptr = malloc( sizeof( ast_list_t ) );
    list_ptr->node = node;
    list_ptr->next = list;
    return list_ptr;
}

/******************************************************************************/
ast_node_t* ast_add_net( ast_node_t* net )
{
    if( net == NULL ) return NULL;
    ast_node_t *node = ast_add_node( AST_NET );
    node->network = malloc( sizeof( ast_net_t ) );
    node->network->net = net;
    return node;
}

/******************************************************************************/
ast_node_t* ast_add_node( node_type_t type )
{
    static int _node_id;
    ast_node_t* node;
    node = malloc( sizeof( ast_node_t ) );
    _node_id++;
    node->id = _node_id;
    node->type = type;
    return node;
}

/******************************************************************************/
ast_node_t* ast_add_op( ast_node_t* left, ast_node_t* right,
        node_type_t type )
{
    ast_node_t *node = ast_add_node( type );
    node->op = malloc( sizeof( ast_op_t ) );
    node->op->left = left;
    node->op->right = right;
    return node;
}

/******************************************************************************/
ast_node_t* ast_add_port( ast_node_t* id, ast_node_t* int_id,
        ast_node_t* collection, ast_node_t* mode, ast_node_t* coupling,
        ast_node_t* channel_len, ast_node_t* connection, port_type_t type )
{
    ast_node_t *node = ast_add_node( AST_PORT );
    node->port = malloc( sizeof( ast_port_t ) );
    node->port->id = id;
    node->port->int_id = int_id;
    node->port->type = type;
    node->port->mode = mode;
    node->port->collection = collection;
    node->port->coupling = coupling;
    node->port->connection = connection;
    node->port->ch_len = channel_len;
    return node;
}

/******************************************************************************/
ast_node_t* ast_add_prog( ast_node_t* stmts, ast_node_t* net )
{
    ast_node_t *node = ast_add_node( AST_PROGRAM );
    node->program = malloc( sizeof( ast_prog_t ) );
    node->program->net = net;
    node->program->stmts = stmts;
    return node;
}

/******************************************************************************/
ast_node_t* ast_add_proto( ast_node_t* id, ast_node_t* ports )
{
    ast_node_t *node = ast_add_node( AST_NET_PROTO );
    node->proto = malloc( sizeof( ast_prot_t ) );
    node->proto->id = id;
    node->proto->ports = ports;
    return node;
}

/******************************************************************************/
ast_node_t* ast_add_symbol( char* name, int line, id_type_t type )
{
    if( name == NULL ) return NULL;
    ast_node_t *node = ast_add_node( AST_ID );
    node->symbol = malloc( sizeof( ast_prog_t ) );
    node->symbol->name = name;
    node->symbol->type = type;
    node->symbol->line = line;
    return node;
}

/******************************************************************************/
ast_node_t* ast_add_time( ast_node_t* op, struct timespec time,
        node_type_t type, int line )
{
    ast_node_t *node = ast_add_node( type );
    node->time = malloc( sizeof( ast_time_t ) );
    node->time->op = op;
    node->time->time = time;
    node->time->line = line;
    return node;
}

/******************************************************************************/
ast_node_t* ast_add_wrap( ast_node_t* id, ast_node_t* ports_wrap,
        ast_node_t* ports_net, ast_node_t* stmts, ast_node_t* attr )
{
    ast_node_t *node = ast_add_node( AST_WRAP );
    node->wrap = malloc( sizeof( ast_wrap_t ) );
    node->wrap->id = id;
    node->wrap->ports_wrap = ports_wrap;
    node->wrap->ports_net = ports_net;
    node->wrap->stmts = stmts;
    node->wrap->attr_static = attr;
    return node;
}

/******************************************************************************/
void ast_destroy( ast_node_t* ast )
{
    ast_list_t* list_next = NULL;
    ast_list_t* list_prev = NULL;
    if( ast == NULL ) return;

    switch( ast->type ) {
        case AST_ASSIGN:
            ast_destroy( ast->assign->id );
            ast_destroy( ast->assign->op );
            free( ast->assign );
            free( ast );
            break;
        case AST_ATTR:
            free( ast->attr );
            free( ast );
            break;
        case AST_BOX:
            ast_destroy( ast->box->impl );
            ast_destroy( ast->box->ports );
            ast_destroy( ast->box->attr_pure );
            ast_destroy( ast->box->attr_location );
            free( ast->box );
            free( ast );
            break;
        case AST_ID:
            free( ast->symbol->name );
            free( ast->symbol );
            free( ast );
            break;
        case AST_INT_PORTS:
        case AST_LINKS:
        case AST_PORTS:
        case AST_STMTS:
        case AST_SYNCS:
            list_next = ast->list;
            while( list_next != NULL ) {
                ast_destroy( list_next->node );
                list_prev = list_next;
                list_next = list_next->next;
                free( list_prev );
            }
            free( ast );
            break;
        case AST_NET:
            ast_destroy( ast->network->net );
            free( ast->network );
            free( ast );
            break;
        case AST_NET_PROTO:
            ast_destroy( ast->proto->id );
            ast_destroy( ast->proto->ports );
            free( ast->proto );
            free( ast );
            break;
        case AST_PORT:
            ast_destroy( ast->port->id );
            ast_destroy( ast->port->int_id );
            ast_destroy( ast->port->collection );
            ast_destroy( ast->port->mode );
            ast_destroy( ast->port->coupling );
            ast_destroy( ast->port->ch_len );
            ast_destroy( ast->port->connection );
            free( ast->port );
            free( ast );
            break;
        case AST_PROGRAM:
            ast_destroy( ast->program->net );
            ast_destroy( ast->program->stmts );
            free( ast->program );
            free( ast );
            break;
        case AST_PARALLEL:
        case AST_PARALLEL_DET:
        case AST_SERIAL:
        case AST_SERIAL_PROP:
            ast_destroy( ast->op->right );
            ast_destroy( ast->op->left );
            free( ast->op );
            free( ast );
            break;
        case AST_WRAP:
            ast_destroy( ast->wrap->id );
            ast_destroy( ast->wrap->ports_wrap );
            ast_destroy( ast->wrap->ports_net );
            ast_destroy( ast->wrap->stmts );
            ast_destroy( ast->wrap->attr_static );
            free( ast->wrap );
            free( ast );
            break;
        case AST_TT:
        case AST_RT:
        case AST_TB:
        case AST_TF:
            ast_destroy( ast->time->op );
            free( ast->time );
            free( ast );
    }
}
