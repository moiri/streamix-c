/* 
 * A simple AST plugin
 *
 * @file    ast.h
 * @author  Simon Maurer
 *
 * */

#ifndef AST_H
#define AST_H

typedef struct ast_node ast_node;
typedef struct con_list con_list;

// linked list structure containing AST node pointers
struct con_list {
    ast_node* ast_node;
    con_list* next;
};

// the AST structure
struct ast_node {
    int node_type;  // OP_ID, OP_SERIAL, OP_PARALLEL
    int id;         // id of the node -> atm only used for dot graphs
    struct {
        con_list* left;
        con_list* right;
    } connect;      // left and right connections at this AST node
    union {
        char* name; // name of the node (only if OP_ID)
        struct {
            ast_node* left;
            ast_node* right;
        } op;       // left and right operand (only if OP_SERIAL and OP_PARALLEL)
    };
};

/**
 * Add a net identifier to the AST.
 *
 * @param char*:    name of the net
 * @return: ast_node*:
 *      a pointer to the location where the data was stored
 * */
ast_node* ast_add_id ( char* );

/**
 * Add a an operation to the symbol table.
 *
 * @param char*:        name of the net
 * @param ast_node*:    pointer to the left operand
 * @param ast_node*:    pointer to the right operand
 * @param int:          OP_ID, OP_SERIAL, OP_PARALLEL
 * @return ast_node*:
 *      a pointer to the location where the data was stored
 * */
ast_node* ast_add_op ( ast_node*, ast_node*, int );

/**
 * Add an AST node to the connection list
 *
 * @param ast_node*:    pointer to the left operand
 * @return con_list*:
 *      a pointer to the location where the data was stored
 * */
con_list* con_add ( ast_node* );

#endif /* AST_H */
