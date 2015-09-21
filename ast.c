#include <stdlib.h> /* For malloc to add nodes to a linked list */
#include <string.h> /* For strlen in ast_add_id */
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
    ptr->node_type = OP_ID;
    __node_id++;
    ptr->id = __node_id;
    // add left connection
    ptr->connect.left = con_add(ptr);
    /* printf(" %s(%p) con left(%p): add %s(%p)\n", ptr->name, ptr, ptr->connect.left, */
    /*         ptr->connect.left->ast_node->name, */
    /*         ptr->connect.left->ast_node); */
    con_ptr = tmp_con_ptr;
    tmp_con_ptr = (con_list*)0;
    // add right connection
    ptr->connect.right = con_add(ptr);
    /* printf(" %s(%p) con right(%p): add %s(%p)\n", ptr->name, ptr, ptr->connect.right, */
    /*         ptr->connect.right->ast_node->name, */
    /*         ptr->connect.right->ast_node); */
    con_ptr = tmp_con_ptr;
    tmp_con_ptr = (con_list*)0;
    return ptr;
}

/******************************************************************************/
ast_node* ast_add_op ( ast_node* left, ast_node* right, int node_type ) {
    ast_node* ptr;
    con_list* con_list_ptr;
    ptr = (ast_node*) malloc(sizeof(ast_node));
    /* printf("NEW NODE: S/P(%p)\n", ptr); */
    ptr->op.left = left;
    ptr->op.right = right;
    ptr->node_type = node_type;
    __node_id++;
    ptr->id = __node_id;
    // add left connection
    // c_left = op.left.c_left (serial and parallel)
    con_list_ptr = ptr->op.left->connect.left;
    do {
        ptr->connect.left = con_add(con_list_ptr->ast_node);
        /* printf(" AST_S/P con left(%p): add %s(%p)\n", ptr->connect.left, */
        /*         ptr->connect.left->ast_node->name, */
        /*         ptr->connect.left->ast_node); */
        con_list_ptr = con_list_ptr->next;
        /* printf(" next list pointer: %p\n", con_list_ptr); */
    }
    while (con_list_ptr != 0);
    if (ptr->node_type == OP_PARALLEL) {
        // c_left = op.right.c_left (parallel)
        con_list_ptr = ptr->op.right->connect.left;
        do {
            ptr->connect.left = con_add(con_list_ptr->ast_node);
            /* printf(" AST_P con left(%p): add %s(%p)\n", ptr->connect.left, */
            /*         ptr->connect.left->ast_node->name, */
            /*         ptr->connect.left->ast_node); */
            con_list_ptr = con_list_ptr->next;
            /* printf(" next list pointer: %p\n", con_list_ptr); */
        }
        while (con_list_ptr != 0);
    }
    con_ptr = tmp_con_ptr;
    tmp_con_ptr = (con_list*)0;

    // add right connection
    // c_right = op.right.c_right (serial and parallel)
    con_list_ptr = ptr->op.right->connect.right;
    do {
        ptr->connect.right = con_add(con_list_ptr->ast_node);
        /* printf(" AST_S/P con right(%p): add %s(%p)\n", ptr->connect.right, */
        /*         ptr->connect.right->ast_node->name, */
        /*         ptr->connect.right->ast_node); */
        con_list_ptr = con_list_ptr->next;
        /* printf(" next list pointer: %p\n", con_list_ptr); */
    }
    while (con_list_ptr != 0);
    if (ptr->node_type == OP_PARALLEL) {
        // c_right = op.left.c_right (parallel)
        con_list_ptr = ptr->op.left->connect.right;
        do {
            ptr->connect.right = con_add(con_list_ptr->ast_node);
            /* printf(" ST_P con right(%p): add %s(%p)\n", ptr->connect.right, */
            /*         ptr->connect.right->ast_node->name, */
            /*         ptr->connect.right->ast_node); */
            con_list_ptr = con_list_ptr->next;
            /* printf(" next list pointer: %p\n", con_list_ptr); */
        }
        while (con_list_ptr != 0);
    }
    con_ptr = tmp_con_ptr;
    tmp_con_ptr = (con_list*)0;

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
