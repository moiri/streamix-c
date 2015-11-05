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
#include "ast.h"

// structures for symbol table record
typedef struct box_attr box_attr;
struct box_attr {
    bool state;
};
typedef struct port_attr port_attr;
struct port_attr {
    bool side;
    bool up;
    bool down;
    bool decoupled;
    int sync_id;
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

/**
 * Check the context of all identifiers in th eprogram
 *
 * @param ast_node*:    pointer to the ast node
 * */
void context_check( ast_node* ast );

/*
 * Get an identifier from the symbol table.
 *
 * @param hashtable_t*: pointer to the hashtable
 * @param char*:        name of the identifier
 * @param int:          position (line number) of the identifier
 * @return symrec*:
 *      a pointer to the location where the data was stored
 *      a null pointer if the element was not found
 * */
symrec* symrec_get( symrec**, char*, int );

/*
 * check whether the given identificator is in the symbol table.
 * This is a recursive function.
 *
 * @param symrec**:     pointer to the symbol table
 * @param ast_node*:    pointer to the ast node
 * */
void symtab_get( symrec**, ast_node* );

/*
 * Add an identifier to the symbol table.
 *
 * @param hashtable_t*  pointer to the hashtable
 * @param char*:        name of the net
 * @param int:          scope of the net
 * @param int:          type of the net
 * @param void*:        pointer to the attributes of the identifier
 * @param int:          position (line number) of the identifier
 * */
void symrec_put( symrec**, char*, int, int, void*, int );

/*
 * put symbol names into the symbol table. this includes collision and scope
 * handling. This is a recursive funtion.
 *
 * @param symrec**:     pointer to the symbol table
 * @param ast_node*:    pointer to the ast node
 * @param bool:         flag indicating wheter the ports are synchronized
 * */
void symtab_put( symrec**, ast_node*, bool );

#endif /* SYMTAB_H */
