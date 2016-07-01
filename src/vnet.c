#include "vnet.h"
#include "defines.h"
#include "ast.h"

/******************************************************************************/
virt_net_t* virt_net_create( symrec* rec, inst_rec* inst )
{
    symrec_list* ports_ptr = NULL;
    virt_net_t* v_net = NULL;
    virt_port_t* ports = NULL;
    virt_port_t* ports_last = NULL;

    if( rec->type == AST_BOX )
        ports_ptr = ( ( struct box_attr* )rec->attr )->ports;
    else if( rec->type == AST_WRAP )
        ports_ptr = ( ( struct wrap_attr* )rec->attr )->ports;
    while( ports_ptr != NULL  ) {
        ports = malloc( sizeof( virt_port_t ) );
        ports->rec = ports_ptr->rec;
        ports->next = ports_last;
        ports->attr_class =
            ( ( struct port_attr* )ports_ptr->rec->attr )->collection;
        ports->attr_mode =
            ( ( struct port_attr* )ports_ptr ->rec->attr )->mode;
        ports->inst = inst;
        ports_last = ports;
        ports_ptr = ports_ptr->next;
    }
    v_net = malloc( sizeof( virt_net_t ) );
    v_net->ports = ports_last;
    v_net->con = malloc( sizeof( net_con_t ) );
    igraph_vector_ptr_init( &v_net->con->left, 1 );
    VECTOR( v_net->con->left )[ 0 ] = &inst->id;
    igraph_vector_ptr_copy( &v_net->con->right, &v_net->con->left );

    return v_net;
}

/******************************************************************************/
virt_net_t* virt_net_create_parallel( virt_net_t* v_net1, virt_net_t* v_net2 )
{
    virt_net_t* v_net = NULL;
    virt_port_t* ports1 = NULL;
    virt_port_t* ports2 = NULL;

    // alter ports
    ports1 = virt_net_copy_ports( v_net1->ports, NULL, -1 );
    ports2 = virt_net_copy_ports( v_net2->ports, ports1, -1 );
    v_net = malloc( sizeof( virt_net_t ) );
    v_net->ports = ports2;
    v_net->con = malloc( sizeof( net_con_t ) );
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

    // alter ports
    ports1 = virt_net_copy_ports( v_net1->ports, NULL, VAL_UP );
    ports2 = virt_net_copy_ports( v_net2->ports, ports1, VAL_DOWN );
    v_net = malloc( sizeof( virt_net_t ) );
    v_net->ports = ports2;
    v_net->con = malloc( sizeof( net_con_t ) );
    igraph_vector_ptr_copy( &v_net->con->left, &v_net1->con->left );
    igraph_vector_ptr_copy( &v_net->con->right, &v_net2->con->right );

    return v_net;
}

/******************************************************************************/
virt_net_t* virt_net_copy( virt_net_t* v_net )
{
    virt_net_t* v_net_new = NULL;
    virt_port_t* ports = NULL;

    // alter ports
    ports = virt_net_copy_ports( v_net->ports, NULL, -1 );
    v_net_new = malloc( sizeof( virt_net_t ) );
    v_net_new->ports = ports;
    v_net_new->con = malloc( sizeof( net_con_t ) );
    igraph_vector_ptr_copy( &v_net_new->con->left, &v_net->con->left );
    igraph_vector_ptr_copy( &v_net_new->con->right, &v_net->con->right );

    return v_net_new;
}

/******************************************************************************/
virt_port_t* virt_net_copy_ports( virt_port_t* old, virt_port_t* last,
        int class )
{
    virt_port_t* new = NULL;

    while( old != NULL ) {
        new = malloc( sizeof( virt_port_t ) );
        new->rec = old->rec;
        new->inst = old->inst;
        new->next = last;
        if( ( old->attr_class != VAL_SIDE ) && ( class >= 0 ) )
            new->attr_class = class;
        else new->attr_class = old->attr_class;
        new->attr_mode = old->attr_mode;
        last = new;
        old = old->next;
    }

    return last;
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
    igraph_vector_ptr_destroy( &v_net->con->left );
    igraph_vector_ptr_destroy( &v_net->con->right );

    // free structures
    free( v_net->con );
    free( v_net );
}
