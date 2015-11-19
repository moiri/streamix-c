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
typedef struct symrec_list symrec_list;
typedef struct instance_list instance_list;
typedef struct box_attr box_attr;
typedef struct port_attr port_attr;
// structures for symbol table record
struct box_attr {
    bool            state;
    symrec_list*    ports;
};
struct port_attr {
    bool    side;
    bool    up;
    bool    down;
    bool    decoupled;
    int     sync_id;
    int     mode;
};
struct symrec {
    char*   name;               // name of the symbol; key for the hashtable
    int     type;               // VAL_NET, VAL_BOX, VAL_PORT
    int     scope;              // scope of the record
    void*   attr;
    symrec* next;
    UT_hash_handle hh;          // makes this structure hasable
};
struct symrec_list {
    symrec*         rec;
    symrec_list*    next;
};
struct instance_list {
    symrec*         port;
    bool            connected;
    instance_list*  next;
};

/**
 * Check the context of all identifiers in th eprogram
 *
 * @param ast_node*:    pointer to the ast node
 * */
void context_check( ast_node* );

void connection_check_net( symrec**, ast_node* );
void connection_check_port( symrec*, symrec* );

/*
 * check whether the given identificator is in the symbol table.
 * This is a recursive function.
 *
 * @param symrec**:     pointer to the symbol table
 * @param ast_node*:    pointer to the ast node
 * */
void id_check( symrec**, ast_node* );

/*
 * put symbol names into the symbol table. this includes collision and scope
 * handling. This is a recursive funtion.
 *
 * @param symrec**:     pointer to the symbol table
 * @param ast_node*:    pointer to the ast node
 * @param bool:         flag indicating wheter the ports are synchronized
 * */
symrec* id_install( symrec**, ast_node*, bool );

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
 * Add an identifier to the symbol table.
 *
 * @param hashtable_t*  pointer to the hashtable
 * @param char*:        name of the net
 * @param int:          scope of the net
 * @param int:          type of the net
 * @param void*:        pointer to the attributes of the identifier
 * @param int:          position (line number) of the identifier
 * */
symrec* symrec_put( symrec**, char*, int, int, void*, int );

#endif /* SYMTAB_H */
