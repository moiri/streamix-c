#include <stdlib.h> /* For malloc to add nodes to a linked list */
#include <string.h> /* For strlen in ast_add_id */
#include <stdio.h>
#include "defines.h"
#include "ast.h"

int __node_id = 0;
con_list* con_ptr = (con_list*)0;
con_list* tmp_con_ptr = (con_list*)0;

/******************************************************************************/
ast_node* ast_add_id ( char* name ) {
    ast_node *ptr;
    ptr = (ast_node*) malloc(sizeof(ast_node));
    /* printf("NEW NODE: %s(%p)\n", name, ptr); */
    ptr->name = (char*) malloc(strlen(name)+1);
    strcpy (ptr->name, name);
    ptr->node_type = AST_ID;
    __node_id++;
    ptr->id = __node_id;
    return ptr;
}

/******************************************************************************/
ast_node* ast_add_op ( ast_node* left, ast_node* right, int node_type ) {
    ast_node* ptr;
    con_list* con_list_ptr = (con_list*)0;
    ptr = (ast_node*) malloc(sizeof(ast_node));
    ptr->op.left = left;
    ptr->op.right = right;
    ptr->node_type = node_type;
    __node_id++;
    ptr->id = __node_id;

    // left connection
    if (left->node_type == AST_ID) {
        // c_left = left
        ptr->op.con_left = con_add(left);
    }
    else {
        // c_left = left.c_left
        con_list_ptr = left->op.con_left;
        do {
            ptr->op.con_left = con_add(con_list_ptr->ast_node);
            con_list_ptr = con_list_ptr->next;
        }
        while (con_list_ptr != 0);
    }
    if (ptr->node_type == AST_PARALLEL) {
        if (right->node_type == AST_ID) {
            // c_left = right
            ptr->op.con_left = con_add(right);
        }
        else {
            // c_left = right.c_left
            con_list_ptr = right->op.con_left;
            do {
                ptr->op.con_left = con_add(con_list_ptr->ast_node);
                con_list_ptr = con_list_ptr->next;
            }
            while (con_list_ptr != 0);
        }
    }
    con_ptr = tmp_con_ptr;
    tmp_con_ptr = (con_list*)0;

    // right connection
    if (right->node_type == AST_ID) {
        // c_right = right
        ptr->op.con_right = con_add(right);
    }
    else {
        // c_right = right.c_right
        con_list_ptr = right->op.con_right;
        do {
            ptr->op.con_right = con_add(con_list_ptr->ast_node);
            con_list_ptr = con_list_ptr->next;
        }
        while (con_list_ptr != 0);
    }
    if (ptr->node_type == AST_PARALLEL) {
        if (left->node_type == AST_ID) {
            // c_right = left
            ptr->op.con_right = con_add(left);
        }
        else {
            // c_right = left.c_right
            con_list_ptr = left->op.con_right;
            do {
                ptr->op.con_right = con_add(con_list_ptr->ast_node);
                con_list_ptr = con_list_ptr->next;
            }
            while (con_list_ptr != 0);
        }
    }
    con_ptr = tmp_con_ptr;
    tmp_con_ptr = (con_list*)0;

    // debug
    /* printf("NEW NODE: OP(%p)\n", ptr); */
    /* con_list_ptr = ptr->op.con_left; */
    /* do { */
    /*     printf(" AST_OP con left(%p): add %s(%p)\n", con_list_ptr, */
    /*             con_list_ptr->ast_node->name, */
    /*             con_list_ptr->ast_node); */
    /*     con_list_ptr = con_list_ptr->next; */
    /* } */
    /* while (con_list_ptr != 0); */
    /* con_list_ptr = ptr->op.con_right; */
    /* do { */
    /*     printf(" AST_OP con right(%p): add %s(%p)\n", con_list_ptr, */
    /*             con_list_ptr->ast_node->name, */
    /*             con_list_ptr->ast_node); */
    /*     con_list_ptr = con_list_ptr->next; */
    /* } */
    /* while (con_list_ptr != 0); */

    return ptr;
}

/******************************************************************************/
con_list* con_add ( ast_node* node ) {
    con_list* ptr;
    ptr = (con_list*) malloc(sizeof(con_list));
    ptr->ast_node = node;
    ptr->next = tmp_con_ptr;
    /* printf("NEW pointer: %p\n", ptr); */
    /* printf(" next pointer: %p\n", ptr->next); */
    if (tmp_con_ptr == 0) {
        // first port in this scope
        tmp_con_ptr = con_ptr;
    }
    tmp_con_ptr = ptr;
    return ptr;
}
