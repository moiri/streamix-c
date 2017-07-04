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
typedef struct instrec_s instrec_t;
typedef enum instrec_type_e instrec_type_t;

// INCLUDES -------------------------------------------------------------------
#include <igraph.h>
#include "symtab.h"
#include "uthash.h"

// ENUMS ----------------------------------------------------------------------
/**
 * @brief   Type of a instance table record
 */
enum instrec_type_e
{
    INSTREC_BOX,
    INSTREC_NET,
    INSTREC_SYNC,
    INSTREC_TT,
    INSTREC_WRAP,
    INSTREC_UNDEF
};

// STRUCTURES------------------------------------------------------------------

/**
 * @brief   Definition of a node of a net graph in the hash table
 */
struct instrec_s
{
    char*           name;   /**< name of the instance symbol */
    int             id;     /**< id of the instance symbol */
    int             line;   /**< line number in the source code of the symbol */
    instrec_type_t  type;   /**< type of the symbol */
};

// FUNCTIONS ------------------------------------------------------------------

/**
 * @brief   Create an instance record
 *
 * @param name  name of the record
 * @param id    id of the record
 * @param line  line of the record
 * @param type  type of the record
 * @return      pointer to the record
 */
instrec_t* instrec_create( char*, int, int, instrec_type_t );

/**
 * @brief   Destroy an instance record
 *
 * @param rec   pointer to the record
 */
void instrec_destroy( instrec_t* );

/**
 * @brief   Replace the id of a record
 *
 * @param rec       pointer to the record
 * @param old_id    id to be replaced
 * @param new_id    new id to replace the old one with
 */
void instrec_replace_id( instrec_t*, int, int );

#endif // INSTTAB_H
