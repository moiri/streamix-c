/* 
 * A instance table table librabry
 *
 * @file    insttab.h
 * @author  Simon Maurer
 *
 * */

#ifndef INSTTAB_H
#define INSTTAB_H

#include "symtab.h"

typedef struct inst_attr inst_attr;
typedef struct inst_rec inst_rec;
typedef struct inst_net inst_net;

struct inst_net {
    int             scope;
    inst_rec**      recs;   // hashtable of the instances in the net
    inst_net*       next;
    UT_hash_handle  hh;     // makes this structure hashable
};

struct inst_rec {
    char*           name;
    int             id;
    symrec*         net;        // pointer to its definition
    symrec_list*    ports;      // pointer to the port list of the instance
                                // this list is also available through the
                                // definition of the net but it is copied in
                                // order to set the corresponding connection
                                // flag
    inst_rec*       next;
    UT_hash_handle  hh;         // makes this structure hashable
};

/**
 *
 */
inst_net* inst_net_get( inst_net**, int );

/**
 *
 */
inst_net* inst_net_put( inst_net**, int, inst_rec** );

/**
 *
 */
inst_rec* inst_rec_get( inst_rec**, char* );

/**
 *
 */
inst_rec* inst_rec_put( inst_rec**, char*, int, symrec* );

#endif // INSTTAB_H
