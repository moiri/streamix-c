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
    VPORT_STATE_OPEN,       /**< port is ready to be connected */
    VPORT_STATE_CP_OPEN,    /**< port is already connected but is still open */
    VPORT_STATE_CONNECTED,  /**< port is connected */
    VPORT_STATE_DISABLED,   /**< port is disabled */
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
    virt_net_t*         v_net;      /**< pointer to net instance */
    symrec_t*           symb;       /**< pointer to the port symbol */
    const char*         name;       /**< pointer to the name (not allocated) */
    int                 attr_class; /**< updated class */
    int                 attr_mode;  /**< updated mode for cp-sync (VAL_BI) */
    virt_port_state_t   state;      /**< #virt_port_state_e */
};

// FUNCTIONS ------------------------------------------------------------------

/**
 * @brief   Checkes whether port names match
 *
 * @param p1    pointer the a port of a virtual net
 * @param p2    pointer the a port of a virtual net
 * @return      true if ports can connect, false if not
 */
bool are_port_names_ok( virt_port_t*, virt_port_t* );

/**
 * @brief   Checkes whether port classes for normal connections match
 *
 * @param p1        pointer the a port of a virtual net
 * @param p2        pointer the a port of a virtual net
 * @param directed  flag to indicate wheter the direction needs to be taken
 *                  into account
 * @return          true if ports can connect, false if not
 */
bool are_port_classes_ok( virt_port_t*, virt_port_t*, bool );

/**
 * @brief   Checkes port classes for copy synchronizer connections
 *
 * @param p1    pointer the a port of a virtual net
 * @param p2    pointer the a port of a virtual net
 * @param cpp   flag to indicate wheter copy synchronizer connections in
 *              parallel operators are checked
 * @return      true if ports can connect, false if not
 */
bool are_port_cp_classes_ok( virt_port_t*, virt_port_t*, bool );

/**
 * @brief    Checkes whether port modes match
 *
 * @param p1    pointer the a port of a virtual net
 * @param p2    pointer the a port of a virtual net
 * @param equal if set to true, the function checks wheter modes are equal
 *              if set to false, the function checks wheter modes are differnt
 * @return      true if ports can connect, false if not
 */
bool are_port_modes_ok( virt_port_t*, virt_port_t*, bool );

/**
 * @brief   Create a net connection structure initialised with one instance
 *
 * @param inst  pointer to a instance record
 * @return      pointer to a net connection structure
 */
net_con_t* net_con_create( instrec_t* );

/**
 * @brief   Create a new virtual net of a box
 *
 * @param rec   pointer to the symbol of the box
 * @param inst  pointer to the instance of the box
 * @return      pointer to the newly created virtual net
 */
virt_net_t* virt_net_create_box( symrec_t*, instrec_t* );

/**
 * @brief   Create a virtual net from any other virtual net
 *
 * Creates a virtual net from another virtual net. Copy all ports
 * but ommit the connection structure.
 *
 * @param v_net pointer to the initial virtual net
 * @param inst  pointer to the instance to be referred to by the ports of the
 *              new virtual net and the virtual net itself
 * @return      pointer to the newly crated virtual net
 */
virt_net_t* virt_net_create_flatten( virt_net_t*, instrec_t* );

/**
 * @brief   Create a new virtual net out of virtial net of a net
 *
 * @param ports pointer to the initial virtual net
 * @param inst  pointer to the instance to be referred to by the ports of the
 *              new virtual net and the virtual net itself
 * @return      pointer to the newly crated virtual net
 */
virt_net_t* virt_net_create_net( virt_net_t*, instrec_t* );

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
 * @breif   Create a virtual net of a copy synchronizer
 *
 * @param inst  pointer to the instance of the copy synchronizer
 * @return      pointer to the newly created virtual net
 */
virt_net_t* virt_net_create_sync( instrec_t* );

/**
 * @brief   Create a new virtual net out of virtial net of a wrapper
 *
 * @param symb  pointer to the symbol of the wrapper
 * @param inst  pointer to the instance to be referred to by the ports of the
 *              new virtual net and the virtual net itself
 * @return      pointer to the newly crated virtual net
 */
virt_net_t* virt_net_create_wrap( symrec_t*, instrec_t* );

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
 * @brief   Append a port to the port list of a virtual net
 *
 * @param v_net pointer to the virtual net to appemd the port
 * @param port  pointer to the port to append
 */
void virt_port_append( virt_net_t*, virt_port_t* );

/**
 * @brief   Append all ports of one virtual net to the port list of
 * another virtual net
 *
 * @param v_net pointer to the virtual net to append the port
 * @param v_net pointer to the source virtual net
 * @param update_inst   if true update the instances of the source ports
 *                      with the instance of v_net1
 */
void virt_port_append_all( virt_net_t*, virt_net_t*, bool );

/**
 * @brief   Assign ports to a virtual net
 *
 * Create a new port list and assign existing ports to it. Assign the new
 * list to a virtual net
 *
 * @param old       port list containing the ports to assign
 * @param last_list a pointer to a list which will be chained to the new list
 * @param v_net     if not NULL, assign it to all ports
 * @return          a pointer to the new port list
 */
virt_port_list_t* virt_port_assign( virt_port_list_t*, virt_port_list_t*,
        virt_net_t* );

/**
 * @brief   Create a new virtual port
 *
 * @param port_class    the class of the new port
 * @param port_mode     the mode of the new port
 * @param vnet          the virtual net the port is part of
 * @param name          a pointer to the name of the port
 * @param symb          a pointer to the symbol of the port
 * @return              a pointer to the newly created port
 */
virt_port_t* virt_port_create( port_class_t, port_mode_t, virt_net_t*,
        const char*, symrec_t* );

/**
 * @brief   Create a copy of a virtual port
 *
 * The port status is set to VPORT_STATE_OPEN
 *
 * @param port  pointer to the virtual port to be copied
 * @return      pointer to the new virtual port
 */
virt_port_t* virt_port_copy( virt_port_t* );

/**
 * @brief   Make a copy of the port list of a symbol record
 *
 * @param ports     a pointer to the port list to be copied
 * @param v_net     a pointer to the virtual net the port belongs to
 * @param v_net_i   a pointer to the virtual net of the content of a wrapper
 * @return          pointer to the last element of the new list
 */
virt_port_list_t* virt_ports_copy_symb( symrec_list_t*, virt_net_t*,
        virt_net_t* );

/**
 * @brief   Make a copy of the port list of a symbol record
 *
 * @param ports         a pointer to the port list to be copied
 * @param inst          a pointer to the instance the port belongs to
 * @param check_status  if true only copy open ports
 *                      if false copy all ports
 * @param copy_status   if true the port status is copied
 *                      if false the port status is set to PORT_STATE_OPEN
 * @return              pointer to the last element of the new list
 */
virt_port_list_t* virt_ports_copy_vnet( virt_port_list_t*, virt_net_t*, bool,
        bool );

/**
 * @brief   Get an equivalent port from a virtual net
 *
 * @param v_net virtual net to search for the port
 * @param port  port to search for
 * @param all   if ture search through ports of all states
 *              if false search only for open ports
 * @return      pointer to the port, NULL if no port was found
 */
virt_port_t* virt_port_get_equivalent( virt_net_t*, virt_port_t*, bool );

/**
 * @brief   Get a port from a virtual net port list by compairing names
 *
 * @param v_net virtual net to search for the port
 * @param name  name to search for
 * @return      pointer to the port, NULL if no port was found
 */
virt_port_t* virt_port_get_equivalent_by_name( virt_net_t*, const char* );

/**
 * @brief   Get the namesake of a port out of a net interface of a wrapper
 *          by compairing the names and the modes
 *
 * @param v_net     pointer to the virtual net
 * @param port      pointer to the port of wich a namesake should be found
 * @return          pointer to the port, NULL if no port was found
 */
virt_port_t* virt_port_get_equivalent_in_wrap( virt_net_t*, virt_port_t* );

/**
 * @brief   update the instance of a port
 *
 * @param port  port to update
 * @param v_net virtual net to assign
 */
void virt_port_update_inst( virt_port_t*, virt_net_t* );

/**
 * @brief   Print debug information of a port of a virtual net
 *
 * @param port  pointer to the port of a virtual net
 */
void debug_print_vport( virt_port_t* );

/**
 * @brief   Wrapper function for debug_print_vports_s( ... )
 *
 * @param v_net pointer to the virtual net
 */
void debug_print_vports( virt_net_t* );

/**
 * @brief   Print debug information of all ports in a virtual net
 *
 * @param v_net pointer to the virtual net
 * @param all   flag to indicate whether all ports or only the open ports
 *              should be printed
 */
void debug_print_vports_s( virt_net_t*, bool );
#endif // VNET_H
