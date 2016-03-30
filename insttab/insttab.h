/* 
 * A instance table table librabry
 *
 * @file    insttab.h
 * @author  Simon Maurer
 *
 * */

#ifndef INSTTAB_H
#define INSTTAB_H

#include <igraph.h>
#include "symtab.h"
#include "uthash.h"

typedef struct inst_attr inst_attr;
typedef struct inst_rec inst_rec;
typedef struct inst_net inst_net;
typedef struct net_con net_con;

struct inst_net
{
    int             scope;
    inst_rec*       recs_id;   // hashtable of the instances in the net (id)
    inst_rec*       recs_name; // hashtable of the instances in the net (name)
    igraph_t        g;
    net_con*        con;
    inst_net*       next;
    UT_hash_handle  hh;     // makes this structure hashable
};

// vectors to store the connection ids
struct net_con
{
    igraph_vector_t left;
    igraph_vector_t right;
};

struct inst_rec
{
    char*           name;
    int             id;
    symrec*         net;        // pointer to its definition
    symrec_list*    ports;      // pointer to the port list of the instance
                                // this list is also available through the
                                // definition of the net but it is copied in
                                // order to set the corresponding connection
                                // flag
    inst_rec*       next;
    UT_hash_handle  hh1;        // makes this structure hashable (id)
    UT_hash_handle  hh2;        // makes this structure hashable (name)
};

/**
 *
 */
inst_net* inst_net_get( inst_net**, int );

/**
 *
 */
inst_net* inst_net_put( inst_net**, int );

/**
 *
 */
inst_rec* inst_rec_get_name( inst_rec**, char* );
inst_rec* inst_rec_get_id( inst_rec**, int );

/**
 *
 */
inst_rec* inst_rec_put( inst_rec**, inst_rec**, char*, int, symrec* );

#endif // INSTTAB_H
