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
typedef struct cp_attr cp_attr;
typedef struct port_attr port_attr;
typedef struct inst_attr inst_attr;

// this is the definition of a record in a hashtable (uthash)
struct symrec {
    char*       key;
    char*       name;   // name of the symbol
    int         scope;  // scope of the record
    int         type;   // VAL_NET, VAL_BOX, VAL_PORT
    void*       attr;   // a struct of attributes
    symrec*     next;   // pointer to the next element (handle collisions)
    UT_hash_handle hh;  // makes this structure hashable
};
// linked list to associate ports to nets
struct symrec_list {
    symrec*         rec;            // pointer to port in symbol table
    bool            is_connected;   // flag to control the port connections
    symrec_list*    next;           // next element in the list
};
// attributes of a net (can also be a box)
struct net_attr {
    bool            state;      // a box can be stateless or stateful
                                // if no box declaration then false
    symrec_list*    ports;      // pointer to the port list of the net
    int             num_ports;  // #ports also counting multiple collections
};
// attributes of a copy synchronizer
struct cp_attr {
    int             num_ports;  // #ports also counting multiple collections
};
// attributes of ports (all kind of ports: box (sync) or net)
struct port_attr {
    int     mode;           // input or output
    // collections
    int     collection;     // VAL_UP, VAL_DOWN, VAL_SIDE
    // sync attributes
    bool    decoupled;
    int     sync_id;
};
// attributes of instances
struct inst_attr {
    int             id;         // id if the instance
    symrec*         net;        // pointer to its definition
    int             port_cnt;   // counter to control all port connections
    symrec_list*    ports;      // pointer to the port list of the instance
                                // this list is also available through the
                                // definition of the net but it is copied in
                                // order to set the corresponding connection
                                // flag
};

/**
 * Check the context of all identifiers in the program
 *
 * @param ast_node*:    pointer to the ast node
 * */
void context_check( ast_node* );

/**
 * check the connections of the network equations
 *
 * @param symrec**:     pointer to the instance table
 * @param ast_node*:    pointer to the ast node
 * */
void connection_check( symrec**, ast_node* );

/**
 * check the port connections
 *
 * */
void connection_check_port( symrec**, ast_node*, ast_node*, bool );

/**
 * check the side port connections
 *
 * */
void connection_check_sport( symrec**, ast_node*, int );

/**
 * check whether the given identificator is in the symbol table.
 * This is a recursive function.
 *
 * @param symrec**:     pointer to the symbol table
 * @param instrec**:    pointer to the instance table
 * @param ast_node*:    pointer to the ast node
 * */
void id_check( symrec**, symrec**, ast_node* );

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
void* id_install( symrec**, ast_node*, bool );

/**
 * Get an instance from the instance table.
 *
 * @param symrec**:     pointer to the hashtable
 * @param char*:        name of the record
 * @param int:          scope of the record
 * @param int:          id of the instance
 * @return symrec*:
 *      a pointer to the location where the data is stored
 * */
symrec* instrec_get( symrec**, char*, int, int );

/**
 * Add an instance to the instance table.
 *
 * @param symrec**:     pointer to the hashtable
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
