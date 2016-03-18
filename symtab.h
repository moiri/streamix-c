/* 
 * A simple symbol table plugin
 *
 * @file    symtab.h
 * @author  Anthony A. Aaby
 * @author  Simon Maurer
 * @see     http://foja.dcs.fmph.uniba.sk/kompilatory/docs/compiler.pdf
 *
 * */


#ifndef SYMTAB_H
#define SYMTAB_H

#include <stdbool.h>
#include "uthash.h"
#include "ast.h"

typedef struct symrec symrec;
typedef struct symrec_key symrec_key;
typedef struct symrec_list symrec_list;
typedef struct instrec instrec;
typedef struct net_attr net_attr;
typedef struct port_attr port_attr;
typedef struct inst_attr inst_attr;

// this is the definition of a record in a hashtable (uthash)
struct symrec {
    char*       key;
    char*       name;   // name of the symbol
    int         scope;  // scope of the record
    int         type;   // VAL_NET, VAL_BOX, VAL_PORT, VAL_WRAPPER
    int         line;   // line position in the source file
    void*       attr;   // a struct of attributes
    symrec*     next;   // pointer to the next element (handle collisions)
    UT_hash_handle hh;  // makes this structure hashable
};
// linked list to associate ports to nets
struct symrec_list {
    symrec*         rec;            // pointer to port in symbol table
    symrec*         cp_sync;        // pointer to copy synchronizer in instance table
    int             connect_cnt;    // counter to control the port connections
    symrec_list*    next;           // next element in the list
};
// attributes of a net (can also be a box)
struct net_attr {
    bool            state;      // a box can be stateless or stateful
                                // if no box declaration then false
    symrec_list*    ports;      // pointer to the port list of the net
};
// attributes of ports (all kind of ports: box (sync) or net)
struct port_attr {
    char*   int_name;       // internal name or NULL
    int     mode;           // input or output
    int     collection;     // VAL_UP, VAL_DOWN, VAL_SIDE
    // sync attributes
    bool    decoupled;
    int     sync_id;
};
// attributes of instances
struct inst_attr {
    int             id;         // id if the instance
    symrec*         net;        // pointer to its definition
    symrec_list*    ports;      // pointer to the port list of the instance
                                // this list is also available through the
                                // definition of the net but it is copied in
                                // order to set the corresponding connection
                                // flag
};

/**
 * Check the context of all identifiers in the program
 *
 * @param ast_node*:    pointer to the root ast node
 * */
void check_context( ast_node* );

/**
 * check whether the given identificator is in the symbol table.
 * This is a recursive function.
 *
 * @param symrec**:     pointer to the symbol table
 * @param instrec**:    pointer to the instance table
 * @param ast_node*:    pointer to the root ast node
 * */
void check_ids( symrec**, symrec**, ast_node* );

/**
 * checks the connect construct and establishes the corresponding side port
 * connections and copy synchronizers
 *
 * @param symrec**:     pointer to the instance table
 * @param ast_node*:    pointer to the root ast node
 * */
void check_instances( symrec**, ast_node* );

/**
 * check whether all ports are connected spawn synchronizers and draw the nodes,
 * synchroniyers and connections
 *
 * @param symrec**:     pointer to the instance table
 * @param ast_node*:    pointer to the root ast node
 * */
void check_port_all( symrec**, ast_node* );

/**
 * establish the connections of the connect instructions
 *
 * @param symrec**:     pointer to the instance table
 * @param ast_node*:    pointer to the ast node
 * @param bool:         flag to indicate wheather it is a check run (false) or
 *                      whether copy sinchronizers are spawned and connections
 *                      are drawn
 * */
void connection_check_connect( symrec**, ast_node*, bool );

/**
 * establish and check 'connect' port connections by searching for a specific
 * port name given by the ast_id parameter. This function establishes the port
 * connections and adds the corresponding edges to the port connection graph
 *
 * @param symrec**:     pointer to the symbol table
 * @param symrec*:      pointer to the first net operand
 * @param symrec*:      pointer to the second net operand
 * @param ast_node*:    pointer to the id ast node
 * @param bool:         flag to indicate wheather it is a check run (false) or
 *                      whether copy sinchronizers are spawned and connections
 *                      are drawn
 * @return bool:
 *      true if the connection check was successful, false otherwise
 * */
bool connection_check_connect_port( symrec**, symrec*, symrec*, ast_node*,
        bool );

/**
 * Get the pointer to the corresponding side port or return a NULL pointer
 *
 * @param symrec*:      pointer to the net operand
 * @param ast_node*:    pointer to the connect ast node
 * @param bool:         flag to indicate wheather it is a check run (false) or
 *                      whether copy sinchronizers are spawned and connections
 *                      are drawn
 * @return symrec_list:
 *      pointer to the side port or NULL if no side port has been found
 * */
symrec_list* connection_check_connect_port_get( symrec*, ast_node*, bool );

/**
 * check and establish the connections of the link instructions
 *
 * @param symrec**:     pointer to the instance table
 * @param ast_node*:    pointer to the ast node
 * @param bool:         flag to indicate wheather it is a check run (false) or
 *                      whether copy sinchronizers are spawned and connections
 *                      are drawn
 * */
void connection_check_link( symrec**, ast_node*, bool );

/**
 * establish and check 'link' port connections by searching for a specific port
 * name given by the ast_id parameter. This function establishes the port
 * connections and adds the corresponding edges to the port connection graph
 *
 * @param symrec**:     pointer to the symbol table
 * @param symrec*:      pointer to the first net operand
 * @param symrec*:      pointer to the second net operand
 * @param ast_node*:    pointer to the id ast node
 * @param int:          id of the invisible helper node to draw the links
 * @param bool:         flag to indicate wheather it is a check run (false) or
 *                      whether copy sinchronizers are spawned and connections
 *                      are drawn
 * */
void connection_check_link_port( symrec**, symrec*, symrec*, ast_node*, int,
        bool );

/**
 * check whether all ports of the net are connected
 *
 * @param symrec**:     pointer to the symbol table
 * @param char*:        name of the net
 * @param int:          id of the net
 * @param int:          line number of the occurance in the source code
 * */
void connection_check_port_all( symrec**, char*, int, int );

/**
 * establish the connections of the network equations
 *
 * @param symrec**:     pointer to the instance table
 * @param ast_node*:    pointer to the ast node
 * @param bool:         flag to indicate wheather it is a check run (false) or
 *                      whether copy sinchronizers are spawned and connections
 *                      are drawn
 * */
void connection_check_serial( symrec**, ast_node*, bool );

/**
 * establish and check the port connections. This function establishes the port
 * connections and adds the corresponding edges to the port connection graph
 *
 * @param symrec**:     pointer to the symbol table
 * @param ast_node*:    pointer to the first net operand
 * @param ast_node*:    pointer to the second ast node
 * @param bool:         flag to indicate wheather it is a check run (false) or
 *                      whether copy sinchronizers are spawned and connections
 *                      are drawn
 * */
void connection_check_serial_port( symrec**, ast_node*, ast_node*, bool );

/**
 * spawn copy synchronizers if necessary and connect the ports of a connect
 * instruction
 *
 * @param symrec**:     pointer to the instance table
 * @param symrec*:      pointer to the instance of the left operand
 * @param symrec*:      pointer to the instance of the right operand
 * @param symrec_list*: pointer to the port of the left operand
 * @param symrec_list*: pointer to the port of the right operand
 * */
void connect_port_connect( symrec**, symrec*, symrec*, symrec_list*,
        symrec_list* );

/**
 * spawn copy synchronizers if necessary and connect the ports of a link
 * instruction
 *
 * @param symrec**:     pointer to the instance table
 * @param symrec*:      pointer to the instance of the left operand
 * @param symrec*:      pointer to the instance of the right operand
 * @param symrec_list*: pointer to the port of the left operand
 * @param symrec_list*: pointer to the port of the right operand
 * @param int:          id of the invisible helper node to draw the links
 * */
void connect_port_link( symrec**, symrec*, symrec*, symrec_list*, symrec_list*,
        int );

/**
 * spawn copy synchronizers if necessary and connect the ports of a serial
 * combination
 *
 * @param symrec**:     pointer to the instance table
 * @param symrec*:      pointer to the instance of the left operand
 * @param symrec*:      pointer to the instance of the right operand
 * @param symrec_list*: pointer to the port of the left operand
 * @param symrec_list*: pointer to the port of the right operand
 * */
void connect_port_serial( symrec**, symrec*, symrec*, symrec_list*,
        symrec_list* );

/**
 * put symbol names into the symbol table. this includes collision and scope
 * handling. This is a recursive funtion.
 *
 * @param symrec**:     pointer to the symbol table
 * @param ast_node*:    pointer to the ast node
 * @param bool:         flag indicating wheter the ports are synchronized
 * @return void*:
 *      the return value is used to propagate back the information of which
 *      ports belong to which net
 * */
void* install_ids( symrec**, ast_node*, bool );

/**
 * Get an instance from the instance table.
 *
 * @param symrec**:     pointer to the instance hashtable
 * @param char*:        name of the record
 * @param int:          scope of the record
 * @param int:          id of the record
 * @return symrec*:
 *      a pointer to the location where the data is stored
 * */
symrec* instrec_get( symrec**, char*, int, int );

/**
 * Add an instance to the instance table.
 *
 * @param symrec**:     pointer to the instance hashtable
 * @param char*:        name of the record
 * @param int:          scope of the record
 * @param int:          type of the record
 * @param int:          id of the instance
 * @param symrec*:      pointer to the symbol record
 * @return symrec*:
 *      a pointer to the location where the data was stored
 * */
symrec* instrec_put( symrec**, char*, int, int, int, symrec* );

/**
 * Helper function to report an error with the yyerror function
 *
 * @param const char*:  error string
 * @param int:          line number
 * */
void report_yyerror( const char*, int );

/**
 * add a copy synchronizer instance to the instance table
 *
 * @param symrec**:     pointer to the instance table
 * @param symrec_list*: pointer to the port, assoziated with the copy
 *                      synchronizer
 * @param int:          id of the net instance the port belongs to
 * @param int:          type of the synchronizer to distinguish between mode
 *                      checks and colors
 * */
void spawn_synchronizer( symrec**, symrec_list*, int, int );

/**
 * Get an identifier from the symbol table.
 *
 * @param symrec**:     pointer to the hashtable
 * @param char*:        name of the identifier
 * @param int:          position (line number) of the identifier
 * @return symrec*:
 *      a pointer to the location where the data is stored
 *      a null pointer if the element was not found
 * */
symrec* symrec_get( symrec**, char*, int );

/**
 * Add an identifier to the symbol table.
 *
 * @param symrec**      pointer to the hashtable
 * @param char*:        name of the record
 * @param int:          scope of the record
 * @param int:          type of the record
 * @param void*:        pointer to the attributes of the identifier
 * @param int:          position (line number) of the identifier
 * @return symrec*:
 *      a pointer to the location where the data was stored
 *      a null pointer if the element was not stored
 * */
symrec* symrec_put( symrec**, char*, int, int, void*, int );

#endif /* SYMTAB_H */
