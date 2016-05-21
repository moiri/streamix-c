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

void virt_net_check( symrec_list*, virt_ports*, char* );
virt_net* virt_net_create( symrec*, inst_rec* );

virt_net* virt_net_alter_parallel( virt_net* , virt_net* );
virt_net* virt_net_alter_serial( virt_net* , virt_net* );
void virt_net_destroy( virt_net* );
void virt_net_destroy_struct( virt_net* );

#endif // VNET_H
