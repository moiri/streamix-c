/**
 * A simple AST plugin
 *
 * @file    ast.h
 * @author  Simon Maurer
 *
 */

#ifndef AST_H
#define AST_H

#include <stdbool.h>
#include <time.h>

// TYPEDEFS -------------------------------------------------------------------
typedef struct ast_assign_s ast_assign_t;
typedef struct ast_attr_s ast_attr_t;
typedef struct ast_box_s ast_box_t;
typedef struct ast_list_s ast_list_t;
typedef struct ast_net_s ast_net_t;
typedef struct ast_node_s ast_node_t;
typedef struct ast_op_s ast_op_t;
typedef struct ast_port_s ast_port_t;
typedef struct ast_prog_s ast_prog_t;
typedef struct ast_prot_s ast_prot_t;
typedef struct ast_symb_s ast_symb_t;
typedef struct ast_time_s ast_time_t;
typedef struct ast_wrap_s ast_wrap_t;

typedef enum attr_type_e attr_type_t;
typedef enum id_type_e id_type_t;
typedef enum node_type_e node_type_t;
typedef enum port_type_e port_type_t;

// ENUMS ----------------------------------------------------------------------
/**
 * @brief   all possible types of AST_ATTR
 */
enum attr_type_e
{
    ATTR_PORT_MODE,
    ATTR_PORT_CLASS,
    ATTR_OTHER,
    ATTR_INT,
};

/**
 * @brief   all possible types of AST_ID
 */
enum id_type_e
{
    ID_NET,
    ID_WRAP,
    ID_BOX,
    ID_BOX_IMPL,
    ID_PORT,
    ID_PORT_ALT,
    ID_IPORT,
    ID_NPORT,
    ID_CNET,
    ID_LNET,
    ID_CPORT,
    ID_LPORT,
    ID_CPSYNC
};

/**
 * @brief       all possible types of AST_NODE
 * @attention   the order of this enum matches the order of node names
 *              "node_label" in smxdot.c
 */
enum node_type_e
{
    AST_ASSIGN,
    AST_BOX,
    AST_INT_PORTS,
    AST_LINKS,
    AST_NET,
    AST_NET_PROTO,
    AST_PARALLEL,
    AST_PARALLEL_DET,
    AST_PORT,
    AST_PORTS,
    AST_PROGRAM,
    AST_SERIAL,
    AST_SERIAL_PROP,
    AST_STMTS,
    AST_SYNCS,
    AST_TB,
    AST_TT,
    AST_TF,
    AST_WRAP,
    AST_ATTR,
    AST_ID
};

/**
 * @brief   all possible types of AST_PORT
 */
enum port_type_e
{
    PORT_NET,
    PORT_WRAP,
    PORT_WRAP_NULL,
    PORT_BOX
};

// STRUCTS --------------------------------------------------------------------
/**
 * @brief   AST structure of node type AST_ASSIGN
 */
struct ast_assign_s
{
    ast_node_t*   id;   /**< ::ast_symb_t */
    ast_node_t*   op;   /**< ::ast_box_t, ::ast_prot_t, ::ast_net_t */
    node_type_t   type; /**< #node_tpe_e */
};

/**
 * @brief   AST structure of node type AST_ATTR
 */
struct ast_attr_s
{
    attr_type_t type;   /**< #attr_type_e */
    int         val;    /**< value of the attribute */
};

/**
 * @brief   AST structure of node type AST_BOX
 */
struct ast_box_s
{
    ast_node_t*   impl;         /**< ::ast_symb_t */
    ast_node_t*   ports;        /**< ::ast_list_t */
    ast_node_t*   attr_pure;    /**< ::ast_attr_t */
};

/**
 * @brief   AST structure of linked list (AST_STMTS, AST_PORTS)
 */
struct ast_list_s
{
    ast_node_t* node;   /**< list data element (any type) */
    ast_list_t* next;   /**< pointer to the next list structure */
};

/**
 * @brief   AST structure of node type AST_NET
 */
struct ast_net_s
{
    ast_node_t* net;    /**< ::ast_symb_t, ::ast_ap_t */
};

/**
 * @brief   AST structure of the node wrapper
 */
struct ast_node_s
{
    node_type_t type;               /**< #node_type_e */
    int         id;                 /**< id of the node */
    union {
        ast_attr_t*     attr;       /**< AST_ATTR */
        ast_box_t*      box;        /**< AST_BOX */
        ast_assign_t*   assign;     /**< AST_ASSIGN */
        ast_list_t*     list;       /**< AST_STMTS, AST_PORTS */
        ast_prot_t*     proto;      /**< AST_NET_PROT */
        ast_net_t*      network;    /**< AST_NET */
        ast_op_t*       op;         /**< AST_SERIAL, AST_PARALLEL */
        ast_port_t*     port;       /**< AST_PORT */
        ast_prog_t*     program;    /**< AST_PROG */
        ast_symb_t*     symbol;     /**< AST_ID */
        ast_time_t*     time;       /**< AST_TT, AST_TB */
        ast_wrap_t*     wrap;       /**< AST_WRAP */
    };
};

/**
 * @brief   AST structure of node type AST_ID
 */
struct ast_symb_s
{
    char*       name;   /**< the name of the id */
    int         line;   /**< the line number of the occurence in the code */
    id_type_t   type;   /**< #id_type_e */
};

/**
 * @brief   AST structure of node types AST_SERIAL and AST_PARALLEL
 */
struct ast_op_s
{
    ast_node_t*   left;     /**< ::ast_symb_t, ::ast_net_t, ::ast_op_t */
    ast_node_t*   right;    /**< ::ast_symb_t, ::ast_net_t, ::ast_op_t */
};

/**
 * @brief   AST structure of node type AST_PORT
 */
struct ast_port_s
{
    port_type_t type;       /**< #port_type_e */
    ast_node_t* id;         /**< ::ast_symb_t */
    ast_node_t* int_id;     /**< tbd */
    ast_node_t* collection; /**< ::ast_attr_t */
    ast_node_t* mode;       /**< ::ast_attr_t */
    ast_node_t* coupling;   /**< ::ast_attr_t */
    ast_node_t* ch_len;     /**< ::ast_attr_t, length of the channel */
};

/**
 * @brief   AST structure of node type AST_PROGRAM
 */
struct ast_prog_s
{
    ast_node_t*   net;      /**< ::ast_net_t */
    ast_node_t*   stmts;    /**< ::ast_list_t */
};

/**
 * @brief   AST structure of node type AST_PROT
 */
struct ast_prot_s
{
    ast_node_t*   id;       /**< ::ast_symb_t */
    ast_node_t*   ports;    /**< ::ast_list_t */
};

/**
 * @brief   AST structure of node type AST_TT
 */
struct ast_time_s
{
    int             line;   /**< the line number of the operation */
    ast_node_t*     op;     /**< ::ast_symb_t */
    struct timespec time;   /**< ::ast_attr_t, ferquency of the clock */
};

/**
 * @brief   AST structure of node type AST_WRAP
 */
struct ast_wrap_s
{
    ast_node_t*   id;           /**< ::ast_symb_t */
    ast_node_t*   ports_wrap;   /**< ::ast_list_t */
    ast_node_t*   ports_net;    /**< ::ast_list_t */
    ast_node_t*   stmts;        /**< ::ast_list_t */
    ast_node_t*   attr_static;  /**< ::ast_attr_t */
};

// FUNCTIONS ------------------------------------------------------------------
/**
 * @brief   Add an assignment to the AST.
 *
 * @param id    pointer to the identifier
 * @param op    pointer to the operand
 * @param type  node type of operand
 * @return      a pointer to the location where the data was stored
 */
ast_node_t* ast_add_assign( ast_node_t*, ast_node_t*, node_type_t );

/**
 * @brief   Add a leaf (end node) attribute to the AST.
 *
 * @param val   value of the attribute
 * @param type  type of the attribute
 * @return      a pointer to the location where the data was stored
 */
ast_node_t* ast_add_attr( int, attr_type_t );

/**
 * @brief   Add a box declaration to the AST.
 *
 * @param id        pointer to an ast node of type AST_ID
 * @param ports     pointer to the ports list AST node
 * @param state     pointer to the state AST node
 * @return          a pointer to the location where the data was stored
 */
ast_node_t* ast_add_box( ast_node_t*, ast_node_t*, ast_node_t* );

/**
 * @brief   Add a list as node to the AST.
 *
 * @param list  pointer to the list
 * @param type  type of AST node
 * @return      a pointer to the location where the data was stored
 */
ast_node_t* ast_add_list( ast_list_t*, node_type_t );

/**
 * @brief   Add a node to the a list.
 *
 * @param node  pointer to the new node
 * @param list  pointer to the list
 * @return      a pointer to the location where the data was stored
 */
ast_list_t* ast_add_list_elem( ast_node_t*, ast_list_t* );

/**
 * @brief   Add a net to the AST.
 *
 * @param node  pointer to net node
 * @return      a pointer to the location where the data was stored
 */
ast_node_t* ast_add_net( ast_node_t* );

/**
 * @brief   Add a node to the AST.
 *
 * @param type  type of AST node
 * @return      a pointer to the location where the data was stored
 */
ast_node_t* ast_add_node( node_type_t );

/**
 * @brief   Add a an operation to the AST.
 *
 * @param left   pointer to the left operand
 * @param right  pointer to the right operand
 * @param type   AST_SERIAL, AST_PARALLEL
 * @return       a pointer to the location where the data was stored
 */
ast_node_t* ast_add_op( ast_node_t*, ast_node_t*, node_type_t );

/**
 * @brief   Add a port to the AST.
 *
 * @param id            pointer to the port ID
 * @param int_id        pointer to the internal port ID
 * @param collection    pointer to the collection node
 * @param mode          pointer to the mode node
 * @param coupling      pointer to the coupling node
 * @param channel_len   the length of the channel
 * @param type          PORT_BOX, PORT_NET, PORT_SYNC
 * @return              a pointer to the location where the data was stored
 */
ast_node_t* ast_add_port( ast_node_t*, ast_node_t*, ast_node_t*, ast_node_t*,
        ast_node_t*, ast_node_t*, port_type_t );

/**
 * @brief   Add a program node to the AST.
 *
 * @param stmts     pointer to a statements node
 * @param net       pointer to a net node
 * @return          a pointer to the location where the data was stored
 */
ast_node_t* ast_add_prog( ast_node_t*, ast_node_t* );

/**
 * @brief   Add a net prototype to the AST.
 *
 * @param id        pointer to an ast node of type AST_ID
 * @param ports     pointer to the ports list AST node
 * @return          a pointer to the location where the data was stored
 */
ast_node_t* ast_add_proto( ast_node_t*, ast_node_t* );

/**
 * @brief   Add a symbol to the AST.
 *
 * @param name  name of the symbol
 * @param line  line number of occurrence of the symbol
 * @param type  type of the symbol
 * @return      a pointer to the location where the data was stored
 */
ast_node_t* ast_add_symbol( char*, int, id_type_t );

/**
 * @brief Add a time triggered strcuture to the AST
 *
 * @param op    a pointer to the operand
 * @param freq  the ferquency of the time trigered structure
 *              or the inter-arrival-time, depending on the type
 * @param type  AST_TT or AST_TB or AST_TF
 * @param line  line number of the operator
 */
ast_node_t* ast_add_time( ast_node_t*, struct timespec, node_type_t, int );

/**
 * @brief   Add a wrapper declaration to the AST.
 *
 * @param id            pointer to an ast node of type AST_ID
 * @param ports_wrap    pointer to the ports list AST node
 * @param ports_net     pointer to the ports list AST node
 * @param stmts         pointer to the stmts list AST node
 * @param attr          pointer to the attr AST node
 * @return              pointer to the location where the data was stored
 */
ast_node_t* ast_add_wrap( ast_node_t*, ast_node_t*, ast_node_t*, ast_node_t*,
        ast_node_t* );

/**
 * @brief   Destroy the AST
 *
 * Destroy the complete ast structure including all subnodes and leaf
 * nodes. This is a recursive function.
 *
 * @param ast   pointer to the root node of the ast
 */
void ast_destroy( ast_node_t* );

#endif /* AST_H */
