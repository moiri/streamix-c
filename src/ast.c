#include <stdlib.h> /* For malloc to add nodes to a linked list */
#include <string.h> /* For strlen in ast_add_id */
#include <stdio.h>
#include "defines.h"
#include "ast.h"

int __node_id = 0;
ast_list* con_ptr = (ast_list*)0;
ast_list* tmp_con_ptr = (ast_list*)0;

/******************************************************************************/
ast_node* ast_add_assign( ast_node* id, ast_node* op )
{
    ast_node* ptr;
    ptr = (ast_node*) malloc(sizeof(ast_node));
    ptr->assign.id = id;
    ptr->assign.op = op;
    ptr->type = AST_ASSIGN;
    __node_id++;
    ptr->id = __node_id;
    return ptr;
}

/******************************************************************************/
ast_node* ast_add_attr ( int val, int type )
{
    ast_node *ptr;
    ptr = (ast_node*) malloc(sizeof(ast_node));
    ptr->type = AST_ATTR;
    __node_id++;
    ptr->id = __node_id;
    ptr->attr.type = type;
    ptr->attr.val = val;
    return ptr;
}

/******************************************************************************/
ast_node* ast_add_box ( ast_node* id, ast_node* ports, ast_node* state )
{
    ast_node* ptr;
    ptr = (ast_node*) malloc(sizeof(ast_node));
    ptr->box.impl = id;
    ptr->box.ports = ports;
    ptr->box.attr_pure = state;
    __node_id++;
    ptr->id = __node_id;
    ptr->type = AST_BOX;
    return ptr;
}

/******************************************************************************/
ast_node* ast_add_list (ast_list* list, int type)
{
    if (list == 0) return (ast_node*)0;
    ast_node* ptr;
    ptr = (ast_node*) malloc(sizeof(ast_node));
    ptr->list = list;
    __node_id++;
    ptr->id = __node_id;
    ptr->type = type;
    return ptr;
}

/******************************************************************************/
ast_list* ast_add_list_elem (ast_node* node, ast_list* list)
{
    ast_list* list_ptr;
    list_ptr = (ast_list*) malloc(sizeof(ast_list));
    list_ptr->node = node;
    list_ptr->next = list;
    return list_ptr;
}

/******************************************************************************/
ast_node* ast_add_net_proto ( ast_node* id, ast_node* ports )
{
    ast_node* ptr;
    ptr = (ast_node*) malloc(sizeof(ast_node));
    ptr->net_prot.id = id;
    ptr->net_prot.ports = ports;
    __node_id++;
    ptr->id = __node_id;
    ptr->type = AST_NET_PROTO;
    return ptr;
}

/******************************************************************************/
ast_node* ast_add_node ( ast_node* node, int type )
{
    if (node == 0) return (ast_node*)0;
    ast_node* ptr;
    ptr = (ast_node*) malloc(sizeof(ast_node));
    ptr->node = node;
    __node_id++;
    ptr->id = __node_id;
    ptr->type = type;
    return ptr;
}

/******************************************************************************/
ast_node* ast_add_op ( ast_node* left, ast_node* right, int node_type )
{
    ast_node* ptr;
    ptr = (ast_node*) malloc(sizeof(ast_node));
    ptr->op.left = left;
    ptr->op.right = right;
    ptr->type = node_type;
    __node_id++;
    ptr->id = __node_id;
    return ptr;
}

/******************************************************************************/
ast_node* ast_add_port (ast_node* id, ast_node* int_id, ast_node* collection,
        ast_node* mode, ast_node* coupling, int type)
{
    ast_node* ptr;
    ptr = (ast_node*) malloc(sizeof(ast_node));
    __node_id++;
    ptr->id = __node_id;
    ptr->type = AST_PORT;
    ptr->port.id = id;
    ptr->port.int_id = int_id;
    ptr->port.port_type = type;
    ptr->port.mode = mode;
    ptr->port.collection = collection;
    ptr->port.coupling = coupling;
    return ptr;
}

/******************************************************************************/
ast_node* ast_add_prog ( ast_node* stmts, ast_node* net )
{
    ast_node* ptr;
    ptr = (ast_node*) malloc(sizeof(ast_node));
    ptr->program.net = net;
    ptr->program.stmts = stmts;
    __node_id++;
    ptr->id = __node_id;
    ptr->type = AST_PROGRAM;
    return ptr;
}

/******************************************************************************/
ast_node* ast_add_symbol ( char* name, int line, int type )
{
    if (name == 0) return (ast_node*)0;
    ast_node *ptr;
    ptr = (ast_node*) malloc(sizeof(ast_node));
    ptr->symbol.name = (char*) malloc(strlen(name)+1);
    strcpy (ptr->symbol.name, name);
    ptr->symbol.type = type;
    ptr->symbol.line = line;
    ptr->type = AST_ID;
    __node_id++;
    ptr->id = __node_id;
    return ptr;
}

/******************************************************************************/
ast_node* ast_add_wrap ( ast_node* id, ast_node* ports, ast_node* stmts,
        ast_node* attr )
{
    ast_node* ptr;
    ptr = (ast_node*) malloc(sizeof(ast_node));
    ptr->wrap.id = id;
    ptr->wrap.ports = ports;
    ptr->wrap.stmts = stmts;
    ptr->wrap.attr_static = attr;
    __node_id++;
    ptr->id = __node_id;
    ptr->type = AST_WRAP;
    return ptr;
}

/******************************************************************************/
void ast_destroy( ast_node* ast )
{
    ast_list* list_next = NULL;
    ast_list* list_prev = NULL;
    if( ast == NULL ) return;

    switch( ast->type ) {
        case AST_ASSIGN:
            ast_destroy( ast->assign.id );
            ast_destroy( ast->assign.op );
            free( ast );
            break;
        case AST_ATTR:
            free( ast );
            break;
        case AST_BOX:
            ast_destroy( ast->box.impl );
            ast_destroy( ast->box.ports );
            ast_destroy( ast->box.attr_pure );
            free( ast );
            break;
        case AST_ID:
            free( ast->symbol.name );
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
            ast_destroy( ast->node );
            free( ast );
            break;
        case AST_NET_PROTO:
            ast_destroy( ast->net_prot.id );
            ast_destroy( ast->net_prot.ports );
            free( ast );
            break;
        case AST_PORT:
            ast_destroy( ast->port.id );
            ast_destroy( ast->port.int_id );
            ast_destroy( ast->port.collection );
            ast_destroy( ast->port.mode );
            ast_destroy( ast->port.coupling );
            free( ast );
            break;
        case AST_PROGRAM:
            ast_destroy( ast->program.net );
            ast_destroy( ast->program.stmts );
            free( ast );
            break;
        case AST_PARALLEL:
        case AST_SERIAL:
            ast_destroy( ast->op.right );
            ast_destroy( ast->op.left );
            free( ast );
            break;
        case AST_WRAP:
            ast_destroy( ast->wrap.id );
            ast_destroy( ast->wrap.ports );
            ast_destroy( ast->wrap.stmts );
            ast_destroy( ast->wrap.attr_static );
            free( ast );
            break;
    }
}
