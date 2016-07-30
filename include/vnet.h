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
/**
 * @brief   type of virtual net
 */
enum virt_net_type_e
{
    VNET_BOX,
    VNET_NET,
    VNET_FLATTEN,
    VNET_PARALLEL,
    VNET_SYNC,
    VNET_SERIAL,
    VNET_WRAP
};

/**
 * @brief   state of a virtual port
 */
enum virt_port_state_e
{
    VPORT_STATE_DISABLED,   /**< port can not be used anymore */
    VPORT_STATE_OPEN,       /**< port is ready to be connected */
    VPORT_STATE_CONNECTED   /**< port is connected */
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
    instrec_t*          inst;       /**< pointer to net instance */
    net_con_t*          con;        /**< connection vector structure */
    virt_port_list_t*   ports;      /**< port list */
    virt_net_type_t     type;       /**< #virt_net_type_e */
};

/**
 * @brief   a list element of a virtual port list
 */
struct virt_port_list_s
{
    int                 idx;    /**< index of the element */
    virt_port_t*        port;   /**< pointer to the port */
    virt_port_list_t*   next;   /**< pointer to the next element */
};

/**
 * @brief   port list of the virtual net
 */
struct virt_port_s
{
    instrec_t*          inst;       /**< pointer to net instance */
    symrec_t*           symb;       /**< pointer to the port symbol */
    char*               name;       /**< pointer to the name (not allocated) */
    int                 attr_class; /**< updated class */
    int                 attr_mode;  /**< updated mode for cp-sync (VAL_BI) */
    virt_port_state_t   state;      /**< #virt_port_state_e */
};

// FUNCTIONS ------------------------------------------------------------------
/**
 * @brief   Create a net connection structure initialised with one instance
 *
 * @param inst  pointer to a instance record
 * @return      pointer to a net connection structure
 */
net_con_t* net_con_create( instrec_t* );

virt_net_t* virt_net_copy_flatten( virt_net_t*, instrec_t* );

/**
 * @brief   Create a new virtual net structure
 *
 * @param ports pointer to a virtual port list
 * @param con   pointer to a net connection structure
 * @param type  type of the virtual net to be created
 * @return      pointer to the newly created virtual net
 */
virt_net_t* virt_net_create_struct( virt_port_list_t*, net_con_t*,
        instrec_t*, virt_net_type_t );

/**
 * @brief   Create a new virtual net of a box
 *
 * @param rec   pointer to a record from the symbol table
 * @param inst  pointer to a record of the instance table
 * @return      pointer to the newly created virtual net
 */
virt_net_t* virt_net_create_box( symrec_t*, instrec_t* );
virt_net_t* virt_net_create_net( virt_net_t*, instrec_t*, virt_net_type_t );

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
virt_net_t* virt_net_create_sync( instrec_t*, virt_port_t*, virt_port_t* );
virt_net_t* virt_net_create_sync_merge( virt_net_t*, virt_net_t*, instrec_t* );

/**
 * @brief   Destroy a virtual net and its conent.
 *
 * @param v_net     pointer to the virtual net
 * @parma shallow   if true do not free the port structures
 */
void virt_net_destroy( virt_net_t*, bool );

/**
 * @brief   Destroy a virtual net and its conent except the port structures.
 *
 * @param v_net     pointer to the virtual net
 */
void virt_net_destroy_shallow( virt_net_t* );

/**
 * @brief   update the port class of all open ports in the v_net
 *
 * @param v_net         a pointer to a virtual net to update the ports
 * @param port_class    the class the ports will be updated to
 */
void virt_net_update_class( virt_net_t*, port_class_t );

/**
 * @brief   Add a new virtual port to a virtual net
 *
 * @param v_net         a pointer to a virtual net where the port will be added
 * @param port_class    the class of the new port
 * @param port_mode     the mode of the new port
 * @param port_inst     the instance the port is part of
 * @param name          a pointer to the name of the port ( no allocation )
 * @return              a pointer to the newly created port
 */
virt_port_t* virt_port_add( virt_net_t*, port_class_t, port_mode_t, instrec_t*,
        char*, symrec_t* );
void virt_port_append( virt_net_t*, virt_port_t* );
/**
 * @brief   Assign ports to a virtual net
 *
 * Create a new port list and assign existing ports to it. Assign the new
 * list to a virtual net
 *
 * @param old       port list containing the ports to assign
 * @param last_list a pointer to a list which will be chained to the new list
 * @return          a pointer to the new port list
 */
virt_port_list_t* virt_port_assign( virt_port_list_t*, virt_port_list_t*,
        instrec_t* );

/**
 * @brief   Create a new virtual port
 *
 * @param port_class    the class of the new port
 * @param port_mode     the mode of the new port
 * @param port_inst     the instance the port is part of
 * @param name          a pointer to the name of the port ( no allocation )
 * @return              a pointer to the newly created port
 */
virt_port_t* virt_port_create( port_class_t, port_mode_t, instrec_t*, char*,
        symrec_t* );

virt_port_t* virt_port_copy( virt_port_t* );
/**
 * @brief   Make a copy of the port list of a symbol record
 *
 * @param ports a pointer to the port list to be copied
 * @param inst  a pointer to the instance the port belongs to
 * @return      pointer to the last element of the new list
 */
virt_port_list_t* virt_ports_copy_box( symrec_list_t*, instrec_t* );
virt_port_list_t* virt_ports_copy_net( virt_port_list_t*, instrec_t*, bool );
virt_port_t* virt_port_get_equivalent( virt_net_t*, virt_port_t*, bool );

/**
 * @brief   remove a port from a port list
 *
 * @depricated  as port lists are copied it is not consistent to remove ports
 *              port states are used instead (e.g. VPORT_STATE_DISABLED)
 *
 * @param v_net a pointer to the virtual net where to port will be removed
 * @param port  a pointer to the port to be removed
 */
void virt_port_remove( virt_net_t*, virt_port_t* );

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
#endif // VNET_H
