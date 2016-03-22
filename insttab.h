/* 
 * A instance table table librabry
 *
 * @file    symtab.h
 * @author  Simon Maurer
 *
 * */

#ifndef INSTTAB_H
#define INSTTAB_H

#include "symtab.h"

typedef struct inst_attr inst_attr;

#endif // INSTTAB_H
// attributes of instances
struct inst_attr {
    int             id;         // id of the instance
    int             cgraph_id;  // cgraph id of the instance
    symrec*         net;        // pointer to its definition
    symrec_list*    ports;      // pointer to the port list of the instance
                                // this list is also available through the
                                // definition of the net but it is copied in
                                // order to set the corresponding connection
                                // flag
};

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
