/* 
 * A library to handle virtual net interfaces, i.e. the interface
 * of a resulting net of an operator
 *
 * @file    vnet.h
 * @author  Simon Maurer
 *
 * */

#ifndef VNET_H
#define VNET_H

#include <igraph.h>
#include "symtab.h"
#include "insttab.h"

typedef struct net_con net_con;
typedef struct virt_net virt_net;
typedef struct virt_ports virt_ports;

// vectors to store the connection ids
struct net_con
{
    igraph_vector_ptr_t left;
    igraph_vector_ptr_t right;
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
 * Check whether each list has the same number of elements
 *
 * @param symrec_list*  port list from a symbol record
 * @param virt_ports*   port list from a net
 * @return bool         true if port count matches, false if not
 */
bool do_port_cnts_match( symrec_list*, virt_ports* );

/**
 * Check whether each element on one list has a matching element in the
 * other list
 *
 * @param symrec_list*  port list from a symbol record
 * @param virt_ports*   port list from a net
 * @return bool         true if port attributes match, false if not
 */
bool do_port_attrs_match( symrec_list*, virt_ports* );

/**
 * Check whether types of a prototype and a net match
 *
 * @param symrec_list*  port list from a symbol record
 * @param virt_ports*   port list from a net
 * @param char*         name of the symbol
 */
void virt_net_check( symrec_list*, virt_ports*, char* );

/**
 * Create a new virtual net
 *
 * @param symrec*       pointer to a record from the symbol table
 * @param inst_rec*     pointer to a record of the instance table
 * @return virt_net*    pointer to the newly created virtual net
 */
virt_net* virt_net_create( symrec*, inst_rec* );

/**
 * Destroy a virtual net and its conent.
 *
 * @param virt_net*     pointer to th evirtual net
 */
void virt_net_destroy( virt_net* );

/**
 * Destroy a the structure of virtual net but not its content.
 *
 * @param virt_net*     pointer to th evirtual net
 */
void virt_net_destroy_struct( virt_net* );

/**
 * Change merge two virtual nets into one, following the
 * parallel connection semantics
 *
 * @param virt_net*     pointer to virtual net of left operand
 * @param virt_net*     pointer to virtual net of right operand
 */
virt_net* virt_net_merge_parallel( virt_net* , virt_net* );

/**
 * Change merge two virtual nets into one, following the
 * serial connection semantics
 *
 * @param virt_net*     pointer to virtual net of left operand
 * @param virt_net*     pointer to virtual net of right operand
 */
virt_net* virt_net_merge_serial( virt_net* , virt_net* );

#endif // VNET_H
