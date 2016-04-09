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
typedef struct virt_net virt_net;
typedef struct virt_ports virt_ports;
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

// a virtual net used as an intermediate interface to construct a net out of a
// network equation
struct virt_net
{
    net_con*    con;
    virt_ports* ports;
};

// port list of the virtual net
struct virt_ports
{
    symrec*     rec;        // pointer to port symbol (if declared)
    inst_rec*   inst;       // pointer to net or cp-sync instance
    int         attr_class; // updated class (VAL_NONE can be overwritten)
    int         attr_mode;  // updated mode for cp-sync (VAL_BI)
    virt_ports* next;
};

struct inst_rec
{
    char*           name;
    int             id;
    int             line;
    int             type;
    symrec*         net;        // pointer to its declaration
    inst_rec*       next;
    UT_hash_handle  hh_id;      // makes this structure hashable (id)
    UT_hash_handle  hh_name;    // makes this structure hashable (name)
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
inst_rec* inst_rec_put( inst_rec**, inst_rec**, char*, int, int, int, symrec* );

void inst_rec_del( inst_rec**, inst_rec**, inst_rec* );

#endif // INSTTAB_H
