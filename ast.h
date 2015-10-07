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

typedef enum { CLASS_UP, CLASS_DOWN, CLASS_SIDE } pclass;
typedef enum { MODE_IN, MODE_OUT } mode;
typedef enum { C_DECOUPLED, C_COUPLED } coupling;
typedef enum { STATE_STATELESS, STATE_STATEFUL } state;
typedef enum {
    PORT_SYNC,
    PORT_NET,
    PORT_BOX
} port_type;
typedef enum {
    AST_ATTR,
    AST_BOX,
    AST_CLASS,
    AST_CONNECT,
    AST_CONNECTS,
    AST_COUPLING,
    AST_MODE,
    AST_NET,
    AST_PARALLEL,
    AST_PORT,
    AST_PORTS,
    AST_STAR,
    AST_STATE,
    AST_SERIAL,
    AST_STMT,
    AST_STMTS,
    AST_SYNC,
    AST_WRAP,
    AST_ID
} node_type;

// linked list structure containing AST node pointers
struct ast_list {
    ast_node*   ast_node;
    ast_list*   next;
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
    ast_node*   ports;
    state state;
};

// AST_WRAP
struct wrap {
    ast_node*   id;
    ast_node*   stmts;
    ast_node*   ports;
};

// AST_CONNECT
struct connect {
    ast_node*   id;
    ast_node*   connects;
};

// AST_BOX_PORT
struct port_box {
    mode mode;
};

// AST_NET_PORT
struct port_net {
    ast_node* int_id;
    mode mode;
};

// AST_SYNC_PORT
struct port_sync {
    coupling coupling;
};

// AST_PORT
struct port {
    port_type port_type;
    ast_node* id;
    pclass pclass;
    union {
        struct port_box box;
        struct port_net net;
        struct port_sync sync;
    };
};

// the AST structure
struct ast_node {
    node_type node_type;
    int     id;         // id of the node -> atm only used for dot graphs
    union {
        char*           name;           // AST_ID
        ast_node*       ast_node;       // AST_NET, AST_MODE, AST_COUPLING, AST_STATE
        ast_list*       ast_list;       // AST_STMTS, AST_CONNECTS, AST_PORTS, AST_SYNC, AST_CLASS
        struct box      box;            // AST_BOX
        struct connect  connect;        // AST_CONNECT
        struct op       op;             // AST_SERIAL, AST_PARALLEL
        struct port     port;           // AST_PORT
        struct wrap     wrap;           // AST_WRAP
    };
};

/**
 * Add a box declaration to the AST.
 *
 * @param ast_node*:    pointer to an ast node of type AST_ID
 * @param ast_node*:    pointer to the ports list AST node
 * @return: ast_node*:
 *      a pointer to the location where the data was stored
 * */
ast_node* ast_add_box ( ast_node*, ast_node* );

/**
 * Add a connection declaration to the AST.
 *
 * @param ast_node*:    pointer to the signal id ast
 * @param ast_node*:    pointer to the connecting net list node
 * @return: ast_node*:
 *      a pointer to the location where the data was stored
 * */
ast_node* ast_add_connect ( ast_node*, ast_node* );

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
 * @param int:          OP_SERIAL, OP_PARALLEL
 * @return ast_node*:
 *      a pointer to the location where the data was stored
 * */
ast_node* ast_add_op ( ast_node*, ast_node*, int );

/**
 * Add a port to the AST.
 *
 * @param ast_node*:    pointer to the port ID
 * @param int:          PORT_BOX, PORT_NET, PORT_SYNC
 * @return ast_node*:
 *      a pointer to the location where the data was stored
 * */
ast_node* ast_add_port (ast_node*, int);

/**
 * Add a star operator to the connect AST.
 *
 * @return: ast_node*:
 *      a pointer to the location where the data was stored
 * */
ast_node* ast_add_star ();

/**
 * Add a leaf (end node) string to the AST.
 *
 * @param char*:    name of the attribute
 * @return: ast_node*:
 *      a pointer to the location where the data was stored
 * */
ast_node* ast_add_str ( char*, int );

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
