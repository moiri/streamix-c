/* 
 * A simple AST plugin
 *
 * @file    ast.h
 * @author  Simon Maurer
 *
 * */

#ifndef AST_H
#define AST_H

#include <stdbool.h>

typedef struct ast_assign ast_assign;
typedef struct ast_list ast_list;
typedef struct ast_node ast_node;
typedef struct ast_attr ast_attr;
typedef struct ast_id ast_id;
typedef struct op op;
typedef struct box box;
typedef struct wrap wrap;
typedef struct port port;

typedef enum {
    ID_NET,
    ID_WRAP,
    ID_BOX,
    ID_BOXIMPL,
    ID_PORT,
    ID_IPORT,
    ID_CNET,
    ID_LNET,
    ID_CPORT,
    ID_LPORT,
    ID_CPSYNC
} id_type;
typedef enum {
    ATTR_MODE,
    ATTR_COLLECT,
    ATTR_COUPLING,
    ATTR_STATE,
    ATTR_STATIC
} attr_type;
typedef enum {
    PORT_SYNC,
    PORT_NET,
    PORT_BOX
} port_type;
// ATTENTION: the order of this enum matches the order of node names
// "node_label" in graph.c
typedef enum {
    AST_BOX,
    AST_BOX_IMPL,
    AST_COLLECT,
    AST_COUPLING,
    AST_INT_PORTS,
    AST_LINKS,
    AST_MODE,
    AST_NET,
    AST_NET_DEF,
    AST_PARALLEL,
    AST_PORT,
    AST_PORTS,
    AST_SERIAL,
    AST_STAR,
    AST_STATE,
    AST_STMT,
    AST_STMTS,
    AST_SYNC,
    AST_WRAP,
    AST_ATTR,
    AST_ID
} node_type;

// linked list structure containing AST node pointers
struct ast_list {
    ast_node*   ast_node;
    ast_list*   next;
};

// AST_NET_DEF
struct ast_assign {
    ast_node*   id;
    ast_node*   op;
};

// AST_ATTR
struct ast_attr {
    attr_type attr_type;
    int val;
};

// AST_ID
struct ast_id {
    char*   name;
    int     line;
    int     type;
};

// AST_SERIAL, AST_PARALLEL
struct op {
    ast_node*   left;
    ast_list*   con_left;
    ast_node*   right;
    ast_list*   con_right;
};

// AST_BOX
struct box {
    ast_node*   id;
    ast_node*   impl;
    ast_node*   ports;
    ast_node*   state;
};

// AST_WRAP
struct wrap {
    ast_node*   id;
    ast_node*   stmts;
    ast_node*   ports;
};

// AST_PORT
struct port {
    port_type   port_type;
    ast_node*   id;
    ast_node*   int_id;
    ast_node*   collection;
    ast_node*   mode;
    ast_node*   coupling;
    bool        is_connected;
};

// the AST structure
struct ast_node {
    node_type   node_type;
    int         id;         // id of the node
    union {
        // AST_NET, AST_MODE, AST_COUPLING, AST_STATE
        ast_node*       ast_node;
        // AST_STMTS, AST_LINKS, AST_PORTS, AST_SYNC, AST_COLLECT
        ast_list*       ast_list;
        struct ast_assign ast_assign; // AST_NET_DEF
        struct ast_attr ast_attr;   // AST_ATTR
        struct ast_id   ast_id;     // AST_ID
        struct box      box;        // AST_BOX
        struct op       op;         // AST_SERIAL, AST_PARALLEL
        struct port     port;       // AST_PORT
        struct wrap     wrap;       // AST_WRAP
    };
};

/**
 * Add a an assignment to the AST.
 *
 * @param ast_node*:    pointer to the identifier
 * @param ast_node*:    pointer to the operand
 * @param int:          AST_NET_DEF
 * @return ast_node*:
 *      a pointer to the location where the data was stored
 * */
ast_node* ast_add_assign ( ast_node*, ast_node*, int );

/**
 * Add a leaf (end node) attribute to the AST.
 *
 * @param int:    value of the attribute
 * @param int:    type of the attribute
 * @return: ast_node*:
 *      a pointer to the location where the data was stored
 * */
ast_node* ast_add_attr ( int, int );

/**
 * Add a box declaration to the AST.
 *
 * @param ast_node*:    pointer to an ast node of type AST_ID
 * @param ast_node*:    pointer to the ports list AST node
 * @param ast_node*:    pointer to the state AST node
 * @return: ast_node*:
 *      a pointer to the location where the data was stored
 * */
ast_node* ast_add_box ( ast_node*, ast_node*, ast_node*, ast_node* );

/**
 * Add a leaf (end node) id to the AST.
 *
 * @param char*:    name of the id
 * @param int:      line number of occurrence of th id
 * @param int:      type of the id
 * @return: ast_node*:
 *      a pointer to the location where the data was stored
 * */
ast_node* ast_add_id ( char*, int, int );

/**
 * Add a list as node to the AST.
 *
 * @param ast_list*:    pointer to the list
 * @param int type:     type of AST node
 * @return: ast_list*:
 *      a pointer to the location where the data was stored
 * */
ast_node* ast_add_list ( ast_list*, int );

/**
 * Add a node to the a list.
 *
 * @param ast_node*:    pointer to the new node
 * @param ast_list*:    pointer to the list
 * @return: ast_list*:
 *      a pointer to the location where the data was stored
 * */
ast_list* ast_add_list_elem (ast_node*, ast_list*);

/**
 * Add a node to the AST.
 *
 * @param ast_node*:    pointer to AST node
 * @param int type:     type of AST node
 * @return: ast_node*:
 *      a pointer to the location where the data was stored
 * */
ast_node* ast_add_node ( ast_node*, int );

/**
 * Add a an operation to the AST.
 *
 * @param ast_node*:    pointer to the left operand
 * @param ast_node*:    pointer to the right operand
 * @param int:          AST_SERIAL, AST_PARALLEL
 * @return ast_node*:
 *      a pointer to the location where the data was stored
 * */
ast_node* ast_add_op ( ast_node*, ast_node*, int );

/**
 * Add a port to the AST.
 *
 * @param ast_node*:    pointer to the port ID
 * @param ast_node*:    pointer to the internal port ID
 * @param ast_node*:    pointer to the collection node
 * @param ast_node*:    pointer to the mode node
 * @param ast_node*:    pointer to the coupling node
 * @param int:          PORT_BOX, PORT_NET, PORT_SYNC
 * @return ast_node*:
 *      a pointer to the location where the data was stored
 * */
ast_node* ast_add_port (ast_node*, ast_node*, ast_node*,
        ast_node*, ast_node*, int);

/**
 * Add a star operator to the connect AST.
 *
 * @return: ast_node*:
 *      a pointer to the location where the data was stored
 * */
ast_node* ast_add_star ();

/**
 * Add a wrapper declaration to the AST.
 *
 * @param ast_node*:    pointer to an ast node of type AST_ID
 * @param ast_node*:    pointer to the ports list AST node
 * @param ast_node*:    pointer to the stmts list AST node
 * @return: ast_node*:
 *      a pointer to the location where the data was stored
 * */
ast_node* ast_add_wrap ( ast_node*, ast_node*, ast_node* );

/**
 * Add an AST node to the connection list
 *
 * @param ast_node*:    pointer to the left operand
 * @return con_list*:
 *      a pointer to the location where the data was stored
 * */
ast_list* con_add ( ast_node* );

#endif /* AST_H */
