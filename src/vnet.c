/**
 * A library to handle virtual net interfaces, i.e. the interface
 * of a resulting net of an operator
 *
 * @file    vnet.c
 * @author  Simon Maurer
 *
 */

#include "vnet.h"
#include "defines.h"
#include "ast.h"

/******************************************************************************/
net_con_t* net_con_create( inst_rec_t* inst )
{
    net_con_t* con = malloc( sizeof( net_con_t ) );
    igraph_vector_ptr_init( &con->left, 1 );
    igraph_vector_ptr_init( &con->right, 1 );
    VECTOR( con->left )[ 0 ] = inst;
    VECTOR( con->right )[ 0 ] = inst;
    return con;
}

/******************************************************************************/
virt_net_t* virt_net_create_struct( virt_port_t* ports, net_con_t* con,
        virt_net_type_t type )
{
    virt_net_t* v_net = malloc( sizeof( virt_net_t ) );
    v_net->ports = ports;
    v_net->con = con;
    v_net->type = type;
    return v_net;
}

/******************************************************************************/
virt_port_t* virt_port_copy_inst( symrec_list_t* ports, inst_rec_t* inst )
{
    virt_port_t* new_ports = NULL;
    virt_port_t* ports_last = NULL;

    while( ports != NULL  ) {
        new_ports = malloc( sizeof( virt_port_t ) );
        new_ports->rec = ports->rec;
        new_ports->next = ports_last;
        new_ports->attr_class = ports->rec->attr_port->collection;
        new_ports->attr_mode = ports ->rec->attr_port->mode;
        new_ports->inst = inst;
        ports_last = new_ports;
        ports = ports->next;
    }
    return ports_last;
}

/******************************************************************************/
virt_port_t* virt_port_copy_vnet( virt_port_t* old, virt_port_t* last,
        int class )
{
    virt_port_t* new = NULL;

    while( old != NULL ) {
        new = malloc( sizeof( virt_port_t ) );
        new->rec = old->rec;
        new->inst = old->inst;
        new->next = last;
        if( ( old->attr_class != PORT_CLASS_SIDE ) && ( class >= 0 ) )
            new->attr_class = class;
        else new->attr_class = old->attr_class;
        new->attr_mode = old->attr_mode;
        last = new;
        old = old->next;
    }
    return last;
}

/******************************************************************************/
virt_net_t* virt_net_create_box( symrec_t* rec, inst_rec_t* inst )
{
    virt_net_t* v_net = NULL;
    virt_port_t* ports = virt_port_copy_inst( rec->attr_box->ports, inst );
    net_con_t* con = net_con_create( inst );
    v_net = virt_net_create_struct( ports, con, VNET_BOX );

    return v_net;
}

/******************************************************************************/
virt_net_t* virt_net_create_vnet( virt_port_t* ports, inst_rec_t* inst,
        virt_net_type_t type )
{
    virt_net_t* v_net_new = NULL;
    virt_port_t* ports_new = virt_port_copy_vnet( ports, NULL, -1 );
    net_con_t* con = net_con_create( inst );
    v_net_new = virt_net_create_struct( ports_new, con, type );

    return v_net_new;
}

/******************************************************************************/
virt_net_t* virt_net_create_parallel( virt_net_t* v_net1, virt_net_t* v_net2 )
{
    virt_net_t* v_net = NULL;
    virt_port_t* ports1 = NULL;
    virt_port_t* ports2 = NULL;
    net_con_t* con = NULL;

    // alter ports
    ports1 = virt_port_copy_vnet( v_net1->ports, NULL, -1 );
    ports2 = virt_port_copy_vnet( v_net2->ports, ports1, -1 );
    con = malloc( sizeof( net_con_t ) );
    v_net = virt_net_create_struct( ports2, con, VNET_PARALLEL );
    igraph_vector_ptr_copy( &v_net->con->left, &v_net1->con->left );
    igraph_vector_ptr_append( &v_net->con->left, &v_net2->con->left );
    igraph_vector_ptr_copy( &v_net->con->right, &v_net1->con->right );
    igraph_vector_ptr_append( &v_net->con->right, &v_net2->con->right );

    return v_net;
}

/******************************************************************************/
virt_net_t* virt_net_create_serial( virt_net_t* v_net1, virt_net_t* v_net2 )
{
    virt_net_t* v_net = NULL;
    virt_port_t* ports1 = NULL;
    virt_port_t* ports2 = NULL;
    net_con_t* con = NULL;

    // alter ports
    ports1 = virt_port_copy_vnet( v_net1->ports, NULL, PORT_CLASS_UP );
    ports2 = virt_port_copy_vnet( v_net2->ports, ports1, PORT_CLASS_DOWN );
    con = malloc( sizeof( net_con_t ) );
    v_net = virt_net_create_struct( ports2, con, VNET_SERIAL );
    igraph_vector_ptr_copy( &v_net->con->left, &v_net1->con->left );
    igraph_vector_ptr_copy( &v_net->con->right, &v_net2->con->right );

    return v_net;
}

/******************************************************************************/
virt_net_t* virt_net_copy( virt_net_t* v_net )
{
    virt_net_t* v_net_new = NULL;
    virt_port_t* ports = NULL;
    net_con_t* con = NULL;

    // alter ports
    ports = virt_port_copy_vnet( v_net->ports, NULL, -1 );
    con = malloc( sizeof( net_con_t ) );
    v_net_new = virt_net_create_struct( ports, con, v_net->type );
    igraph_vector_ptr_copy( &v_net_new->con->left, &v_net->con->left );
    igraph_vector_ptr_copy( &v_net_new->con->right, &v_net->con->right );

    return v_net_new;
}

/******************************************************************************/
void virt_net_destroy( virt_net_t* v_net )
{
    virt_port_t* ports = NULL;
    // free ports
    while( v_net->ports != NULL ) {
        ports = v_net->ports;
        v_net->ports = v_net->ports->next;
        free( ports );
    }
    // free connection vectors
    if( v_net->con != NULL ) {
        igraph_vector_ptr_destroy( &v_net->con->left );
        igraph_vector_ptr_destroy( &v_net->con->right );
        free( v_net->con );
    }
    free( v_net );
}
