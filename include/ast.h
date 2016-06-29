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

typedef struct ast_attr ast_attr;
typedef struct ast_box ast_box;
typedef struct ast_assign ast_assign;
typedef struct ast_list ast_list;
typedef struct ast_node ast_node;
typedef struct ast_op ast_op;
typedef struct ast_port ast_port;
typedef struct ast_prog ast_prog;
typedef struct ast_prot ast_prot;
typedef struct ast_symb ast_symb;
typedef struct ast_wrap ast_wrap;

typedef enum id_type
{
    ID_NET,
    ID_WRAP,
    ID_BOX,
    ID_BOX_IMPL,
    ID_PORT,
    ID_IPORT,
    ID_NPORT,
    ID_CNET,
    ID_LNET,
    ID_CPORT,
    ID_LPORT,
    ID_CPSYNC
} id_type;
typedef enum attr_type
{
    ATTR_PORT_MODE,
    ATTR_PORT_CLASS,
    ATTR_PORT_COUPLING,
    ATTR_BOX_STATE,
    ATTR_WRAP_STATIC
} attr_type;
typedef enum port_type
{
    PORT_SYNC,
    PORT_NET,
    PORT_WRAP,
    PORT_WRAP_NULL,
    PORT_BOX
} port_type;
// ATTENTION: the order of this enum matches the order of node names
// "node_label" in graph.c
typedef enum node_type
{
    AST_ASSIGN,
    AST_BOX,
    AST_INT_PORTS,
    AST_LINKS,
    AST_NET,
    AST_NET_PROTO,
    AST_PARALLEL,
    AST_PORT,
    AST_PORTS,
    AST_PROGRAM,
    AST_SERIAL,
    AST_STMTS,
    AST_SYNCS,
    AST_WRAP,
    AST_ATTR,
    AST_ID
} node_type;

// AST_ATTR
struct ast_attr
{
    attr_type   type;
    int         val;
};

// AST_BOX
struct ast_box
{
    ast_node*   impl;
    ast_node*   ports;
    ast_node*   attr_pure;
};

// AST_NET_DEF, AST_NET_DECL
struct ast_assign
{
    ast_node*   id;
    ast_node*   op;
};

// AST_ID
struct ast_symb
{
    char*   name;
    int     line;
    int     type;
};

// linked list structure containing AST node pointers
struct ast_list
{
    ast_node*   node;
    ast_list*   next;
};

// AST_SERIAL, AST_PARALLEL
struct ast_op
{
    ast_node*   left;
    ast_node*   right;
};

// AST_PORT
struct ast_port
{
    port_type   port_type;
    ast_node*   id;
    ast_node*   int_id;
    ast_node*   collection;
    ast_node*   mode;
    ast_node*   coupling;
    bool        is_connected;
};

// AST_PROGRAM
struct ast_prog
{
    ast_node*   net;
    ast_node*   stmts;
};
// AST_PROT
struct ast_prot
{
    ast_node*   id;
    ast_node*   ports;
};

// AST_WRAP
struct ast_wrap
{
    ast_node*   id;
    ast_node*   ports;
    ast_node*   stmts;
    ast_node*   attr_static;
};

// the AST structure
struct ast_node
{
    node_type   type;
    int         id;         // id of the node
    union {
        struct ast_attr     attr;       // AST_ATTR
        struct ast_box      box;        // AST_BOX
        struct ast_assign   assign;     // AST_ASSIGN
        // AST_STMTS, AST_LINKS, AST_PORTS, AST_SYNCS
        ast_list*           list;
        struct ast_prot     net_prot;   // AST_NET_PROT
        ast_node*           node;       // AST_NET
        struct ast_op       op;         // AST_SERIAL, AST_PARALLEL
        struct ast_port     port;       // AST_PORT
        struct ast_prog     program;    // AST_PROG
        struct ast_symb     symbol;     // AST_ID
        struct ast_wrap     wrap;       // AST_WRAP
    };
};

/**
 * Add an assignment to the AST.
 *
 * @param ast_node*:    pointer to the identifier
 * @param ast_node*:    pointer to the operand
 * @return ast_node*:
 *      a pointer to the location where the data was stored
 * */
ast_node* ast_add_assign ( ast_node*, ast_node* );

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
ast_node* ast_add_box ( ast_node*, ast_node*, ast_node* );

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
 * Add a net prototype to the AST.
 *
 * @param ast_node*:    pointer to an ast node of type AST_ID
 * @param ast_node*:    pointer to the ports list AST node
 * @return: ast_node*:
 *      a pointer to the location where the data was stored
 * */
ast_node* ast_add_net_proto ( ast_node*, ast_node* );

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
ast_node* ast_add_port (ast_node*, ast_node*, ast_node*, ast_node*, ast_node*,
        int);

/**
 * Add a program node to the AST.
 *
 * @param ast_node*:    pointer to a net
 * @param ast_node*:    pointer to a AST_STMTS node
 * @return: ast_node*:
 *      a pointer to the location where the data was stored
 * */
ast_node* ast_add_prog ( ast_node*, ast_node* );

/**
 * Add a symbol to the AST.
 *
 * @param char*:    name of the symbol
 * @param int:      line number of occurrence of the symbol
 * @param int:      type of the symbol
 * @return: ast_node*:
 *      a pointer to the location where the data was stored
 * */
ast_node* ast_add_symbol ( char*, int, int );

/**
 * Add a wrapper declaration to the AST.
 *
 * @param ast_node*:    pointer to an ast node of type AST_ID
 * @param ast_node*:    pointer to the ports list AST node
 * @param ast_node*:    pointer to the stmts list AST node
 * @param ast_node*:    pointer to the attr AST node
 * @return: ast_node*:
 *      a pointer to the location where the data was stored
 * */
ast_node* ast_add_wrap ( ast_node*, ast_node*, ast_node*, ast_node* );

/**
 * Destroy the complete ast structure including all subnodes and leaf
 * nodes. This is a recursive function.
 *
 * @param ast_node*     pointer to the root node of the ast
 */
void ast_destroy( ast_node* );

#endif /* AST_H */
