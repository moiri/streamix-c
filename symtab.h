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

// structures for symbol table record
typedef struct box_attr box_attr;
struct box_attr {
    bool state;
};
typedef struct sport_attr sport_attr;
struct sport_attr {
    bool side;
    bool up;
    bool down;
    bool decoupled;
    int sync_id;
    int mode;
};
typedef struct port_attr port_attr;
struct port_attr {
    bool side;
    bool up;
    bool down;
    int mode;
};
typedef struct symrec symrec;
struct symrec {
    char* name;                 // name of the symbol; key for the hashtable
    int type;                   // VAL_NET, VAL_BOX, VAL_PORT
    int scope;                  // scope of the record
    void* attr;
    symrec* next;
    UT_hash_handle hh;          // makes this structure hasable
};

/*
 * Get a net identifier from the symbol table.
 *
 * @param hashtable_t*  pointer to the hashtable
 * @param char*         name of the net
 * @return symrec*:
 *      a pointer to the location where the data was stored
 *      a null pointer if the element was not found
 * */
symrec* symrec_get( symrec**, char* );

/*
 * Add a net identifier to the symbol table.
 *
 * @param hashtable_t*  pointer to the hashtable
 * @param char*:        name of the net
 * @param int:          scope of the net
 * @param int:          type of the net
 * @return bool:        true if operation was successful,
 *                      false if duplicate entry
 * */
bool symrec_put( symrec**, char*, int, int, void* );


#endif /* SYMTAB_H */
