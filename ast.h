/* 
 * A simple AST plugin
 *
 * @file    ast.h
 * @author  Simon Maurer
 *
 * */

#ifndef AST_H
#define AST_H

typedef struct ast_list ast_list;
typedef struct port_list port_list;
typedef struct ast_node ast_node;
typedef struct op op;
typedef struct box box;
typedef struct wrap wrap;
typedef struct port port;

// linked list structure containing AST node pointers
struct ast_list {
    ast_node*   ast_node;
    ast_list*   next;
};

// linked list structure containing AST port pointers
struct port_list {
    port*       port_node;
    port_list*  next;
};

// AST_SERIAL, AST_PARALLEL
struct op {
    ast_node* left;
    ast_list* con_left;
    ast_node* right;
    ast_list* con_right;
};

// AST_BOX
struct box {
    ast_node*   id;
    port_list*  port_list;
};

// AST_WRAP
struct wrap {
    ast_node*   id;
    ast_node*   net;
    port_list*  port_list;
};

// AST_PORT
struct port {
    char*   name_ext;
    char*   name_int;
    int     mode;
    int     pclass;
};

// the AST structure
struct ast_node {
    int     node_type;  // OP_ID, OP_SERIAL, OP_PARALLEL
    int     id;         // id of the node -> atm only used for dot graphs
    union {
        char*           name;   // AST_ID
        struct op       op;     // AST_SERIAL, AST_PARALLEL
        ast_node*       net;    // AST_NET
        struct box      box;    // AST_BOX
        struct wrap     wrap;   // AST_WRAP
        struct port     port;   // AST_PORT
        ast_list*       stmts;  // AST_STMTS
    };
};

/**
 * Add a box declaration to the AST.
 *
 * @param ast_node*:    pointer to an ast node of type AST_ID
 * @return: ast_node*:
 *      a pointer to the location where the data was stored
 * */
ast_node* ast_add_box ( ast_node* );

/**
 * Add a net identifier to the AST.
 *
 * @param char*:    name of the net
 * @return: ast_node*:
 *      a pointer to the location where the data was stored
 * */
ast_node* ast_add_id ( char* );

/**
 * Add a net to the AST.
 *
 * @param ast_node*:    pointer to the first AST element of the net
 * @return: ast_node*:
 *      a pointer to the location where the data was stored
 * */
ast_node* ast_add_net ( ast_node* );

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
 * Add a statement to the AST.
 *
 * @param ast_node*:    pointer to the statement
 * @return: ast_node*:
 *      a pointer to the location where the data was stored
 * */
ast_node* ast_add_stmt ( ast_node* );

/**
 * Add a wrapper declaration to the AST.
 *
 * @param ast_node*:    pointer to an ast node of type AST_ID
 * @param ast_node*:    pointer to the beginning of the wrapper net
 * @return: ast_node*:
 *      a pointer to the location where the data was stored
 * */
ast_node* ast_add_wrap ( ast_node*, ast_node* );

/**
 * Add an AST node to the connection list
 *
 * @param ast_node*:    pointer to the left operand
 * @return con_list*:
 *      a pointer to the location where the data was stored
 * */
ast_list* con_add ( ast_node* );

#endif /* AST_H */
