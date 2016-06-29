#include "vnet.h"
#include "defines.h"
#include "ast.h"

/******************************************************************************/
virt_net* virt_net_create( symrec* rec, inst_rec* inst )
{
    symrec_list* ports_ptr = NULL;
    virt_net* v_net = NULL;
    virt_ports* ports = NULL;
    virt_ports* ports_last = NULL;

    if( rec->type == AST_BOX )
        ports_ptr = ( ( struct box_attr* )rec->attr )->ports;
    else if( rec->type == AST_WRAP )
        ports_ptr = ( ( struct wrap_attr* )rec->attr )->ports;
    while( ports_ptr != NULL  ) {
        ports = malloc( sizeof( virt_ports ) );
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
    v_net = malloc( sizeof( virt_net ) );
    v_net->ports = ports_last;
    v_net->con = malloc( sizeof( net_con ) );
    igraph_vector_ptr_init( &v_net->con->left, 1 );
    VECTOR( v_net->con->left )[ 0 ] = &inst->id;
    igraph_vector_ptr_copy( &v_net->con->right, &v_net->con->left );

    return v_net;
}

/******************************************************************************/
void virt_net_destroy( virt_net* v_net )
{
    virt_ports* ports = NULL;
    // free ports
    while( v_net->ports != NULL ) {
        ports = v_net->ports;
        v_net->ports = v_net->ports->next;
        free( ports );
    }
    // free connection vectors
    igraph_vector_ptr_destroy( &v_net->con->left );
    igraph_vector_ptr_destroy( &v_net->con->right );
    virt_net_destroy_struct( v_net );
}

/******************************************************************************/
void virt_net_destroy_struct( virt_net* v_net )
{
    // free structures
    free( v_net->con );
    free( v_net );
}

/******************************************************************************/
virt_net* virt_net_merge_parallel( virt_net* v_net1, virt_net* v_net2 )
{
    virt_ports* ports = NULL;
    virt_ports* ports_last = NULL;

    // alter port list
    ports = v_net1->ports;
    while( ports != NULL ) {
        ports_last = ports;
        ports = ports->next;
    }
    ports_last->next = v_net2->ports;

    // alter connection lists
    igraph_vector_ptr_append( &v_net1->con->left, &v_net2->con->left );
    igraph_vector_ptr_append( &v_net1->con->right, &v_net2->con->right );

    // destroy obsolete connection vectors
    igraph_vector_ptr_destroy( &v_net2->con->left );
    igraph_vector_ptr_destroy( &v_net2->con->right );

    virt_net_destroy_struct( v_net2 );

    return v_net1;
}

/******************************************************************************/
virt_net* virt_net_merge_serial( virt_net* v_net1, virt_net* v_net2 )
{
    virt_ports* ports = NULL;
    virt_ports* ports_last = NULL;

    // alter ports
    ports = v_net1->ports;
    while( ports != NULL ) {
        if( ports->attr_class != VAL_SIDE ) ports->attr_class = VAL_UP;
        ports_last = ports;
        ports = ports->next;
    }
    if( ports_last != NULL ) ports_last->next = v_net2->ports;
    else v_net1->ports = v_net2->ports;
    ports = v_net2->ports;
    while( ports != NULL ) {
        if(ports->attr_class != VAL_SIDE ) ports->attr_class = VAL_DOWN;
        ports = ports->next;
    }

    // destroy obsolete connection vectors
    igraph_vector_ptr_destroy( &v_net1->con->right );
    igraph_vector_ptr_destroy( &v_net2->con->left );

    // alter connection vectors of virtual net
    v_net1->con->right = v_net2->con->right;

    virt_net_destroy_struct( v_net2 );

    return v_net1;
}

/******************************************************************************/
virt_net* virt_net_create_parallel( virt_net* v_net1, virt_net* v_net2 )
{
    virt_net* v_net = NULL;
    virt_ports* ports1 = NULL;
    virt_ports* ports2 = NULL;

    // alter ports
    ports1 = virt_net_copy_ports( v_net1->ports, NULL, -1 );
    ports2 = virt_net_copy_ports( v_net2->ports, ports1, -1 );
    v_net = malloc( sizeof( virt_net ) );
    v_net->ports = ports2;
    v_net->con = malloc( sizeof( net_con ) );
    igraph_vector_ptr_copy( &v_net->con->left, &v_net1->con->left );
    igraph_vector_ptr_append( &v_net->con->left, &v_net2->con->left );
    igraph_vector_ptr_copy( &v_net->con->right, &v_net1->con->right );
    igraph_vector_ptr_append( &v_net->con->right, &v_net2->con->right );

    return v_net;
}

/******************************************************************************/
virt_net* virt_net_create_serial( virt_net* v_net1, virt_net* v_net2 )
{
    virt_net* v_net = NULL;
    virt_ports* ports1 = NULL;
    virt_ports* ports2 = NULL;

    // alter ports
    ports1 = virt_net_copy_ports( v_net1->ports, NULL, VAL_UP );
    ports2 = virt_net_copy_ports( v_net2->ports, ports1, VAL_DOWN );
    v_net = malloc( sizeof( virt_net ) );
    v_net->ports = ports2;
    v_net->con = malloc( sizeof( net_con ) );
    igraph_vector_ptr_copy( &v_net->con->left, &v_net1->con->left );
    igraph_vector_ptr_copy( &v_net->con->right, &v_net2->con->right );

    return v_net;
}

/******************************************************************************/
virt_net* virt_net_copy( virt_net* v_net_old )
{
    virt_net* v_net = NULL;
    virt_ports* ports = NULL;

    // alter ports
    ports = virt_net_copy_ports( v_net_old->ports, NULL, -1 );
    v_net = malloc( sizeof( virt_net ) );
    v_net->ports = ports;
    v_net->con = malloc( sizeof( net_con ) );
    igraph_vector_ptr_copy( &v_net->con->left, &v_net_old->con->left );
    igraph_vector_ptr_copy( &v_net->con->right, &v_net_old->con->right );

    return v_net;
}

/******************************************************************************/
virt_ports* virt_net_copy_ports( virt_ports* old, virt_ports* last, int class )
    {
        virt_ports* new = NULL;

        while( old != NULL ) {
            new = malloc( sizeof( virt_ports ) );
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
