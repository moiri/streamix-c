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

// TYPEDEFS -------------------------------------------------------------------
typedef struct net_con_s net_con_t;
typedef struct virt_net_s virt_net_t;
typedef struct virt_port_s virt_port_t;
typedef struct virt_port_list_s virt_port_list_t;
typedef enum virt_net_type_e virt_net_type_t;
typedef enum virt_port_state_e virt_port_state_t;

#include <igraph.h>
#include "symtab.h"
#include "insttab.h"

// ENUMS ----------------------------------------------------------------------
enum virt_net_type_e
{
    VNET_BOX,
    VNET_NET,
    VNET_PARALLEL,
    VNET_SERIAL,
    VNET_WRAP
};

enum virt_port_state_e
{
    VPORT_STATE_DISABLED,
    VPORT_STATE_OPEN,
    VPORT_STATE_TO_TEST,
    VPORT_STATE_CONNECTED
};

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
    net_con_t*          con;        /**< connection vector structure */
    virt_port_list_t*   ports;      /**< port list */
    virt_net_type_t     type;       /**< #virt_net_type_e */
};

struct virt_port_list_s
{
    int                 idx;
    virt_port_t*        port;
    virt_port_list_t*   next;
};

/**
 * @brief   port list of the virtual net
 */
struct virt_port_s
{
    instrec_t*          inst;       /**< pointer to net instance */
    char*               name;
    int                 attr_class; /**< updated class (VAL_NONE can be changed) */
    int                 attr_mode;  /**< updated mode for cp-sync (VAL_BI) */
    virt_port_state_t   state;
};

// FUNCTIONS ------------------------------------------------------------------
/**
 * @brief   Create a net connection structure initialised with one instance
 *
 * @param inst  pointer to a instance record
 * @return      pointer to a net connection structure
 */
net_con_t* net_con_create( instrec_t* );

/**
 * @brief   Make a copy of the port list of a symbol record
 *
 * @param ports a pointer to the port list to be copied
 * @param inst  a pointer to the instance the port belongs to
 * @return      pointer to the last element of the new list
 */
virt_port_list_t* virt_port_copy_box( symrec_list_t* ports, instrec_t* inst );

virt_port_t* virt_port_add( virt_net_t*, port_class_t, port_mode_t,
        instrec_t*, char* name );
virt_port_t* virt_port_create( port_class_t, port_mode_t, instrec_t*, char* );
void virt_port_remove( virt_net_t*, virt_port_t* );

/**
 * @brief   Create a new virtual net structure
 *
 * @param ports pointer to a virtual port list
 * @param con   pointer to a net connection structure
 * @param type  type of the virtual net to be created
 * @return      pointer to the newly created virtual net
 */
virt_net_t* virt_net_create_struct( virt_port_list_t*, net_con_t*, virt_net_type_t );

/**
 * @brief   Create a new virtual net of a box
 *
 * @param rec   pointer to a record from the symbol table
 * @param inst  pointer to a record of the instance table
 * @param type  type of the virtual net to be created
 * @return      pointer to the newly created virtual net
 */
virt_net_t* virt_net_create_box( symrec_t*, instrec_t* );

/**
 * @brief   Create a new virtual net of a wrapper or net
 *
 * @param ports pointer to a virtual port list
 * @param inst  pointer to a instance record
 * @param type  type of the virtual net to create
 * @return      pointer to the newly created virtual net
 */
virt_net_t* virt_net_create_vnet( virt_port_list_t*, instrec_t*,
        virt_net_type_t );

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
 * @brief   Destroy a virtual net and its conent.
 *
 * @param v_net pointer to the virtual net
 */
void virt_net_destroy( virt_net_t* );
void virt_net_destroy_shallow( virt_net_t* );
instrec_t* get_inst_from_virt_port( virt_port_t* );

/**
 * @brief   Print debug information of a port of a virtual net
 *
 * @param port  pointer to the port of a virtual net
 */
void debug_print_vport( virt_port_t* );

/**
 * @brief   Print debug information of all ports in a virtual net
 *
 * @param v_net pointer to the virtual net
 */
void debug_print_vports( virt_net_t* );
void debug_print_con( virt_net_t* );
#endif // VNET_H
