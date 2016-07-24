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
instrec_t* get_inst_from_virt_port( virt_port_t* port )
{
    /* if( port->cp_sync != NULL ) return port->cp_sync; */
    /* else return port->box; */
    return port->inst;
}

/******************************************************************************/
net_con_t* net_con_create( instrec_t* inst )
{
    net_con_t* con = malloc( sizeof( net_con_t ) );
    igraph_vector_ptr_init( &con->left, 1 );
    igraph_vector_ptr_init( &con->right, 1 );
    VECTOR( con->left )[ 0 ] = inst;
    VECTOR( con->right )[ 0 ] = inst;
    return con;
}

/******************************************************************************/
virt_net_t* virt_net_create_struct( virt_port_list_t* ports, net_con_t* con,
        virt_net_type_t type )
{
    virt_net_t* v_net = malloc( sizeof( virt_net_t ) );
    v_net->ports = ports;
    v_net->con = con;
    v_net->type = type;
    return v_net;
}

/******************************************************************************/
virt_port_list_t* virt_port_copy_box( symrec_list_t* ports, instrec_t* inst )
{
    virt_port_t* new_port = NULL;
    virt_port_list_t* list_last = NULL;
    virt_port_list_t* vports = NULL;
    int idx = 0;

    while( ports != NULL  ) {
        vports = malloc( sizeof( virt_port_list_t ) );
        new_port = virt_port_create( ports->rec->attr_port->collection,
                ports->rec->attr_port->mode, inst, ports->rec->name );
        vports->port = new_port;
        vports->next = list_last;
        vports->idx = idx;
        list_last = vports;
        ports = ports->next;
        idx++;
    }
    return list_last;
}

/******************************************************************************/
virt_port_t* virt_port_create( port_class_t port_class, port_mode_t port_mode,
        instrec_t* port_inst, char* name )
{
    virt_port_t* new_port = NULL;

    new_port = malloc( sizeof( virt_port_t ) );
    new_port->attr_class = port_class;
    new_port->attr_mode = port_mode;
    new_port->inst = port_inst;
    new_port->name = name;
    new_port->connected = false;

    return new_port;
}

/******************************************************************************/
virt_port_list_t* virt_port_add( virt_net_t* v_net, port_class_t port_class,
        port_mode_t port_mode, instrec_t* port_inst, char* name )
{
    virt_port_list_t* list_last = NULL;
    virt_port_list_t* list_new = NULL;
    virt_port_list_t* list = v_net->ports;
    virt_port_t* new_port = NULL;
    int idx = 0;

    while( list != NULL ) {
        list_last = list;
        idx++;
        list = list->next;
    }

    new_port = virt_port_create( port_class, port_mode, port_inst, name );
    list_new = malloc( sizeof( virt_port_list_t ) );
    list_new->port = new_port;
    list_new->idx = idx;
    list_new->next = NULL;
    list_last->next = list_new;
#if defined(DEBUG) || defined(DEBUG_CONNECT)
    printf( "virt_port_add: Add port %s of inst %s(%d)\n", new_port->name,
            new_port->inst->name, new_port->inst->id );
#endif // DEBUG_CONNECT

    return list_last;
}

/******************************************************************************/
void virt_port_remove( virt_net_t* v_net, virt_port_t* port )
{
    virt_port_list_t* list_last = NULL;
    virt_port_list_t* list = v_net->ports;

    while( list != NULL ) {
        if( list->port == port ) {
            if( list_last == NULL ) v_net->ports = list->next;
            else list_last->next = list->next;
#if defined(DEBUG) || defined(DEBUG_CONNECT)
            printf( "virt_port_remove: Remove port %s of inst %s(%d)\n",
                    list->port->name, list->port->inst->name,
                    list->port->inst->id );
#endif // DEBUG_CONNECT
            free( list->port );
            free( list );
            break;
        }
        list_last = list;
        list = list->next;
    }
}

/******************************************************************************/
virt_port_list_t* virt_port_assign( virt_port_list_t* old,
        virt_port_list_t* list_last, int port_class )
{
    virt_port_list_t* new = NULL;
    int idx = 0;
    if( list_last != NULL) idx = list_last->idx + 1;

    /* printf("virt_port_assign: \n"); */
    while( old != NULL ) {
        /* printf(" %s,", old->port->name ); */
        new = malloc( sizeof( virt_port_list_t ) );
        if( ( old->port->attr_class != PORT_CLASS_SIDE )
                && ( port_class >= 0 )
                && !old->port->connected )
            old->port->attr_class = port_class;
        new->port = old->port;
        new->next = list_last;
        new->idx = idx;
        idx++;
        list_last = new;
        old = old->next;
    }
    /* printf("\n"); */
    return list_last;
}

/******************************************************************************/
virt_net_t* virt_net_create_box( symrec_t* rec, instrec_t* inst )
{
    virt_net_t* v_net = NULL;
    virt_port_list_t* ports = virt_port_copy_box( rec->attr_box->ports, inst );
    net_con_t* con = net_con_create( inst );
    v_net = virt_net_create_struct( ports, con, VNET_BOX );

    return v_net;
}

/******************************************************************************/
virt_net_t* virt_net_create_vnet( virt_port_list_t* ports, instrec_t* inst,
        virt_net_type_t type )
{
    virt_net_t* v_net_new = NULL;
    virt_port_list_t*  ports_new = virt_port_assign( ports, NULL, -1 );
    net_con_t* con = net_con_create( inst );
    v_net_new = virt_net_create_struct( ports_new, con, type );

    return v_net_new;
}

/******************************************************************************/
virt_net_t* virt_net_create_parallel( virt_net_t* v_net1, virt_net_t* v_net2 )
{
    virt_net_t* v_net = NULL;
    virt_port_list_t* ports1 = NULL;
    virt_port_list_t* ports2 = NULL;
    net_con_t* con = NULL;

    // alter ports
    ports1 = virt_port_assign( v_net1->ports, NULL, -1 );
    ports2 = virt_port_assign( v_net2->ports, ports1, -1 );
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
    virt_port_list_t* ports1 = NULL;
    virt_port_list_t* ports2 = NULL;
    net_con_t* con = NULL;

    // alter ports
    ports1 = virt_port_assign( v_net1->ports, NULL, PORT_CLASS_UP );
    ports2 = virt_port_assign( v_net2->ports, ports1, PORT_CLASS_DOWN );
    con = malloc( sizeof( net_con_t ) );
    v_net = virt_net_create_struct( ports2, con, VNET_SERIAL );
    igraph_vector_ptr_copy( &v_net->con->left, &v_net1->con->left );
    igraph_vector_ptr_copy( &v_net->con->right, &v_net2->con->right );

    return v_net;
}

/******************************************************************************/
void virt_net_destroy_shallow( virt_net_t* v_net )
{
    virt_port_list_t* ports = NULL;
    // free port list structures
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

/******************************************************************************/
void virt_net_destroy( virt_net_t* v_net )
{
    virt_port_list_t* ports = NULL;
    // free ports
    while( v_net->ports != NULL ) {
        ports = v_net->ports;
        v_net->ports = v_net->ports->next;
        if( ports->port != NULL ) free( ports->port );
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

/******************************************************************************/
void debug_print_con( virt_net_t* v_net )
{
    int i;
    instrec_t* rec = NULL;
    igraph_vector_ptr_t con;
    con= v_net->con->left;
    printf( "v_net con left:" );
    for( i=0; i<igraph_vector_ptr_size( &con ); i++ ) {
        rec = VECTOR( con )[i];
        printf( "\n %s: ", rec->name );
        if( rec->type == INSTREC_NET )
            debug_print_vports( rec->net->attr_net->v_net, true );
    }
    con = v_net->con->right;
    printf( "\nv_net con right:" );
    for( i=0; i<igraph_vector_ptr_size( &con ); i++ ) {
        rec = VECTOR( con )[i];
        printf( "\n %s: ", rec->name );
        if( rec->type == INSTREC_NET )
            debug_print_vports( rec->net->attr_net->v_net, true );
    }
    printf( "\n" );
}

/******************************************************************************/
void debug_print_vport( virt_port_t* port )
{
    instrec_t* inst = get_inst_from_virt_port( port );
    if( port->connected ) printf("!");
    printf( "%s(%d)", inst->name, inst->id );
    if( port->attr_class == PORT_CLASS_DOWN ) printf( "_" );
    else if( port->attr_class == PORT_CLASS_UP ) printf( "^" );
    else if( port->attr_class == PORT_CLASS_SIDE ) printf( "|" );
    if( port->attr_mode == PORT_MODE_IN ) printf( "<--" );
    else if( port->attr_mode == PORT_MODE_OUT ) printf( "-->" );
    else printf( "<->" );
    printf( "%s", port->name );
}

/******************************************************************************/
void debug_print_vports( virt_net_t* v_net, bool connected )
{
    virt_port_list_t* ports = NULL;
    if( v_net->ports != NULL )
        ports = v_net->ports;
    while( ports != NULL ) {
        if( connected || !ports->port->connected ) {
            debug_print_vport( ports->port );
            printf(", ");
        }
        ports = ports->next;
    }
    printf("\n");
}
