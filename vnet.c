#include "vnet.h"
#include "defines.h"
#include "ast.h"
#ifndef TESTING
#include "error.h"
#endif

/******************************************************************************/
bool do_port_cnts_match( symrec_list* r_ports, virt_ports* v_ports )
{
    symrec_list* r_port_ptr = r_ports;
    virt_ports* v_port_ptr = v_ports;
    int v_count = 0;
    int r_count = 0;

    while( r_port_ptr != NULL  ) {
        r_count++;
        r_port_ptr = r_port_ptr->next;
    }

    while( v_port_ptr != NULL  ) {
        v_count++;
        v_port_ptr = v_port_ptr->next;
    }

    return (r_count == v_count);
}

/******************************************************************************/
bool do_port_attrs_match( symrec_list* r_ports, virt_ports* v_ports )
{
    symrec_list* r_port_ptr = r_ports;
    virt_ports* v_port_ptr = v_ports;
    port_attr* r_port_attr = NULL;
    bool match = false;

    r_port_ptr = r_ports;
    while( r_port_ptr != NULL  ) {
        match = false;
        v_port_ptr = v_ports;
        r_port_attr = ( struct port_attr* )r_port_ptr->rec->attr;
        while( v_port_ptr != NULL  ) {
            if( strlen( r_port_ptr->rec->name )
                    == strlen( v_port_ptr->rec->name )
                && strcmp( r_port_ptr->rec->name, v_port_ptr->rec->name ) == 0
                && r_port_attr->collection == v_port_ptr->attr_class
                && ( r_port_attr->mode == v_port_ptr->attr_mode
                    || v_port_ptr->attr_mode == VAL_BI )
                ) {
                match = true;
                break;
            }
            v_port_ptr = v_port_ptr->next;
        }
        if( !match ) break;
        r_port_ptr = r_port_ptr->next;
    }

    return match;
}

/******************************************************************************/
void virt_net_check( symrec_list* r_ports, virt_ports* v_ports, char *name )
{
#ifdef TESTING
    char error_msg[ CONST_ERROR_LEN ];
#endif // TESTING

    if( !do_port_cnts_match( r_ports, v_ports )
        || !do_port_attrs_match( r_ports, v_ports ) ) {
#ifndef TESTING
        printf( ERROR_TYPE_CONFLICT, ERR_ERROR, name );
        printf( "\n" );
#else
        sprintf( error_msg, ERROR_TYPE_CONFLICT, ERR_ERROR, name );
        report_yyerror( error_msg, r_ports->rec->line );
#endif // TESTING
    }
}

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
        ports->attr_class = VAL_UP;
        ports_last = ports;
        ports = ports->next;
    }
    if( ports_last != NULL ) ports_last->next = v_net2->ports;
    else v_net1->ports = v_net2->ports;
    ports = v_net2->ports;
    while( ports != NULL ) {
        ports->attr_class = VAL_DOWN;
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
