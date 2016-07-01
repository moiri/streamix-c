/**
 * A library to handle virtual net interfaces, i.e. the interface
 * of a resulting net of an operator
 *
 * @file    vnet.h
 * @author  Simon Maurer
 *
 */

#ifndef VNET_H
#define VNET_H

#include <igraph.h>
#include "symtab.h"
#include "insttab.h"

// TYPEDEFS -------------------------------------------------------------------
typedef struct net_con_s net_con_t;
typedef struct virt_net_s virt_net_t;
typedef struct virt_port_s virt_port_t;

// STRUCTS --------------------------------------------------------------------
/**
 * @brief   vectors to store the connection ids
 */
struct net_con_s
{
    igraph_vector_ptr_t left;   /**< left connection vector */
    igraph_vector_ptr_t right;  /**< right connection vector */
};

/**
 * @brief   structure of a virtual net
 *
 * a virtual net used as an intermediate interface to construct a net out of a
 * network equation
 */
struct virt_net_s
{
    net_con_t*      con;    /**< connection vector structure */
    virt_port_t*    ports;  /**< port list */
};

/**
 * @brief   port list of the virtual net
 */
struct virt_port_s
{
    symrec*         rec;        /**< pointer to port symbol (if declared) */
    inst_rec*       inst;       /**< pointer to net or cp-sync instance */
    int             attr_class; /**< updated class (VAL_NONE can be changed) */
    int             attr_mode;  /**< updated mode for cp-sync (VAL_BI) */
    virt_port_t* next;
};

// FUNCTIONS ------------------------------------------------------------------

/**
 * @brief   Create a new virtual net
 *
 * @param rec   pointer to a record from the symbol table
 * @param inst  pointer to a record of the instance table
 * @return      pointer to the newly created virtual net
 */
virt_net_t* virt_net_create( symrec*, inst_rec* );

/**
 * @brief   Create a virtual net from two operands op1|op2
 *
 * Create a new virtial net from two parallel nets, following the
 * parallel connection semantics of Streamix
 *
 * @param v_net1    pointer to virtual net of left operand
 * @param v_net2    pointer to virtual net of right operand
 * @return          pointer to the new virtual net
 */
virt_net_t* virt_net_create_parallel( virt_net_t*, virt_net_t* );

/**
 * @brief   Create a virtual net from two operands op1.op2
 *
 * Create a new virtial net from two serial nets, following the
 * serial connection semantics of Streamix
 *
 * @param v_net1    pointer to virtual net of left operand
 * @param v_net2    pointer to virtual net of right operand
 * @return          pointer to the new virtual net
 */
virt_net_t* virt_net_create_serial( virt_net_t*, virt_net_t* );

/**
 * @brief   Make a deep copy of a virtual net
 *
 * Create a deep copy from a virtual net. Note that all all ports of a virtual
 * net only point to the port record in the symbol table. Hence only the
 * pointer list is copied but not the port record itself
 *
 * @param v_net pointer to a virtual net
 * @return      pointer to the new virtual net
 */
virt_net_t* virt_net_copy( virt_net_t* );

/**
 * @brief   Make a copy of the port list in a virtual net
 *
 * @param old   a pointer to the port list to be copied
 * @param last  a pointer to the last element of a potential other list
 * @param class force a class to each port in the list
 *              if set to -1 this is ignored
 * @return      pointer to the last element of the new list
 */
virt_port_t* virt_net_copy_ports( virt_port_t*, virt_port_t*, int );

/**
 * @brief   Destroy a virtual net and its conent.
 *
 * @param v_net pointer to the virtual net
 */
void virt_net_destroy( virt_net_t* );
#endif // VNET_H
