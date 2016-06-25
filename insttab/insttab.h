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

struct inst_net
{
    int             scope;
    inst_rec*       nodes;   // hashtable of the node instances in the net
    igraph_t        g;
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
    UT_hash_handle  hh;      // makes this structure hashable
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
 * Delete record from the instance table
 *
 * @param inst_rec**    pointer to rec (id) instance table
 * @param inst_rec*     poiner to the record to be removed
 */
void inst_rec_del( inst_rec**, inst_rec* );

/**
 * Get rec from instance table using an id as key
 *
 * @param inst_rec**    pointer to rec instance table
 * @param int           id of the record
 * @return inst_rec*    pointer to the record
 */
inst_rec* inst_rec_get( inst_rec**, int );

/**
 * Put rec into instance table
 *
 * @param inst_rec**    pointer to rec instance table
 * @param char*         name of the record
 * @param int           id of the record
 * @param int           line of the record
 * @param int           type of the record
 * @param symrec*       pointer to the symbol record
 * @return inst_rec*    pointer to the record
 */
inst_rec* inst_rec_put( inst_rec**, char*, int, int, int, symrec* );

/**
 * Replace the id of a record
 *
 * @param inst_rec**    pointer to the record hashtable
 * @param int           old id
 * @param int           new id
 */
void inst_rec_replace_id( inst_rec**, int, int );

#endif // INSTTAB_H
