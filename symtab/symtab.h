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
#include "utarray.h"

typedef struct symrec symrec;
typedef struct symrec_key symrec_key;
typedef struct symrec_list symrec_list;
typedef struct net_attr net_attr;
typedef struct port_attr port_attr;

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
    bool            attr_pure;  // a box can be pure
    bool            attr_static;// net attribute
    char*           impl_name;  // implementation name
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

/**
 * Get an identifier from the symbol table.
 *
 * @param symrec**:     pointer to the hashtable
 * @param UT_array**:   pointer to the scope stack
 * @param char*:        name of the identifier
 * @param int:          position (line number) of the identifier
 * @return symrec*:
 *      a pointer to the location where the data is stored
 *      a null pointer if the element was not found
 * */
symrec* symrec_get( symrec**, UT_array*, char*, int );

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
