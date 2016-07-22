/**
 * A instance table librabry
 *
 * @file    insttab.h
 * @author  Simon Maurer
 *
 */

#ifndef INSTTAB_H
#define INSTTAB_H

// TYPEDEFS -------------------------------------------------------------------
typedef struct inst_attr_s inst_attr_t;
typedef struct inst_rec_s inst_rec_t;
typedef struct inst_net_s inst_net_t;
typedef enum inst_rec_type_e inst_rec_type_t;

// INCLUDES -------------------------------------------------------------------
#include <igraph.h>
#include "symtab.h"
#include "uthash.h"

// ENUMS ----------------------------------------------------------------------
/**
 * @brief   Type of a instance table record
 */
enum inst_rec_type_e
{
    INSTREC_BOX,
    INSTREC_NET,
    INSTREC_SYNC,
    INSTREC_WRAP
};

// STRUCTURES------------------------------------------------------------------
/**
 * @brief   Definition of a net gaph in the hash table
 */
struct inst_net_s
{
    int             scope;  /**< scope of the graph */
    inst_rec_t*     nodes;  /**< hashtable of the node instances in the net */
    UT_hash_handle  hh;     /**< makes this structure hashable */
};

/**
 * @brief   Definition of a node of a net graph in the hash table
 */
struct inst_rec_s
{
    char*           name;   /**< name of the instance symbol */
    int             id;     /**< id of the instance symbol */
    int             line;   /**< line number in the source code of the symbol */
    inst_rec_type_t type;   /**< type of the symbol */
    symrec_t*       net;    /**< pointer to the declaration in the symtab */
    UT_hash_handle  hh;     /**< makes this structure hashable */
};

// FUNCTIONS ------------------------------------------------------------------
/**
 * @brief   Delete all nets and its records in the instance table
 *
 * @param nets  pointer to the net instance table
 */
void inst_net_del_all( inst_net_t** );

/**
 * @brief   Get net from instance table
 *
 * @param nets      pointer to net instance table
 * @param scope     scope of the net
 * @return          pointer to the net record
 */
inst_net_t* inst_net_get( inst_net_t**, int );

/**
 * @brief   Put net into the instance table
 *
 * @param nets      pointer to net instance table
 * @param scope     scope of the net
 * @return          pointer to the net record
 */
inst_net_t* inst_net_put( inst_net_t**, int );

/**
 * @brief   Delete a record from the instance table
 *
 * @param recs  pointer to rec instance table
 * @param rec   poiner to the record to be removed
 */
void inst_rec_del( inst_rec_t**, inst_rec_t* );

/**
 * @brief   Delete all records from the instance table
 *
 * @param recs  pointer to rec instance table
 */
void inst_rec_del_all( inst_rec_t** );

/**
 * @brief   Get rec from instance table using an id as key
 *
 * @param recs  pointer to rec instance table
 * @param id    id of the record
 * @return      pointer to the record
 */
inst_rec_t* inst_rec_get( inst_rec_t**, int );

/**
 * @brief   Put rec into instance table
 *
 * @param recs  pointer to rec instance table
 * @param name  name of the record
 * @param id    id of the record
 * @param line  line of the record
 * @param type  type of the record
 * @param rec   pointer to the symbol record
 * @return      pointer to the record
 */
inst_rec_t* inst_rec_put( inst_rec_t**, char*, int, int, inst_rec_type_t,
        symrec_t* );

/**
 * @brief   Replace the id of a record
 *
 * @param recs      pointer to the record hashtable
 * @param old_id    id to be replaced
 * @param new_id    new id to replace the old one with
 */
void inst_rec_replace_id( inst_rec_t**, int, int );

#endif // INSTTAB_H
