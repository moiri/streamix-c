/* 
 * A instance table librabry
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

/* FUNCTION PROTOTYPES                                                        */
/******************************************************************************/

/**
 * Get net from instance table
 *
 * @param inst_net**    pointer to net instance table
 * @param int           scope of the net
 * @return inst_net     pointer to the net record
 */
inst_net* inst_net_get( inst_net**, int );

/**
 * Put net into the instance table
 *
 * @param inst_net**    pointer to net instance table
 * @param int           scope of the net
 * @return inst_net*    pointer to the net record
 */
inst_net* inst_net_put( inst_net**, int );

/**
 * Get rec from instance table using a name as key
 *
 * @param inst_rec**    pointer to rec instance table
 * @param char*         name of the record
 * @return inst_rec*    pointer to the record
 */
inst_rec* inst_rec_get_name( inst_rec**, char* );

/**
 * Get rec from instance table using an id as key
 *
 * @param inst_rec**    pointer to rec instance table
 * @param int           id of the record
 * @return inst_rec*    pointer to the record
 */
inst_rec* inst_rec_get_id( inst_rec**, int );

/**
 * Put rec into instance table
 *
 * @param inst_rec**    pointer to rec (name) instance table
 * @param inst_rec**    pointer to rec (id) instance table
 * @param char*         name of the record
 * @param int           id of the record
 * @param int           line of the record
 * @param int           type of the record
 * @param symrec*       pointer to the symbol record
 * @return inst_rec*    pointer to the record
 */
inst_rec* inst_rec_put( inst_rec**, inst_rec**, char*, int, int, int, symrec* );

/**
 * Delete record from the instance table
 *
 * @param inst_rec**    pointer to rec (name) instance table
 * @param inst_rec**    pointer to rec (id) instance table
 * @param inst_rec*     poiner to the record to be removed
 */
void inst_rec_del( inst_rec**, inst_rec**, inst_rec* );

#endif // INSTTAB_H
