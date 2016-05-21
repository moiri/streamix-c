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
typedef struct box_attr box_attr;
typedef struct wrap_attr wrap_attr;
typedef struct port_attr port_attr;

// this is the definition of a record in a hashtable (uthash)
struct symrec
{
    char*       key;
    char*       name;   // name of the symbol
    int         scope;  // scope of the record
    int         type;   // AST_NET, AST_NET_PROTO, AST_BOX
    int         line;   // line position in the source file
    void*       attr;   // a struct of attributes
                            // AST_BOX:         box_attr
                            // AST_PORT:        port_attr
                            // AST_NET_PROTO:   symrec_list
                            // AST_NET:         virt_net
    symrec*     next;   // pointer to the next element (handle collisions)
    UT_hash_handle hh;  // makes this structure hashable
};
// linked list to associate ports to nets
struct symrec_list
{
    symrec*         rec;            // pointer to port in symbol table
    symrec_list*    next;           // next element in the list
};
// attributes of a box
struct box_attr
{
    bool            attr_pure;  // a box can be pure
    char*           impl_name;  // implementation name
    symrec_list*    ports;      // pointer to the port list of the net
};
// attributes of a wrapper
struct wrap_attr
{
    bool            attr_static;
    symrec_list*    ports;      // pointer to the port list of the net
};
// attributes of ports (all kind of ports: box (sync) or net)
struct port_attr
{
    char*   int_name;       // internal name or NULL
    int     mode;           // input or output
    int     collection;     // VAL_UP, VAL_DOWN, VAL_SIDE
    // sync attributes
    bool    decoupled;
    int     sync_id;
};

/**
* Remove a record from the symbol table and free the allocated space
*
* @param symrec**:     pointer to the hashtable
* @param symrec*:      pointer to the record to be removed
* */
void symrec_del( symrec**, symrec* );

/**
 * Get an identifier from the symbol table and produce an error if it was not
 * found
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

/**
 * Search an identifier in the symbol table and rturn it if found
 *
 * @param symrec**:     pointer to the hashtable
 * @param UT_array**:   pointer to the scope stack
 * @param char*:        name of the identifier
 * @return symrec*:
 *      a pointer to the location where the data is stored
 *      a null pointer if the element was not found
 * */
symrec* symrec_search( symrec**, UT_array*, char* );

#endif /* SYMTAB_H */
