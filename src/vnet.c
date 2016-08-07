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
virt_net_t* virt_net_create_box( symrec_t* rec, instrec_t* inst )
{
    virt_net_t* v_net = NULL;
    virt_port_list_t* ports = virt_ports_copy_symb( rec->attr_box->ports,
            inst );
    net_con_t* con = net_con_create( inst );
    v_net = virt_net_create_struct( ports, con, inst, VNET_BOX );

    return v_net;
}

/******************************************************************************/
virt_net_t* virt_net_create_flatten( virt_net_t* v_net, instrec_t* inst )
{
    virt_net_t* v_net_new = NULL;
    virt_port_list_t*  ports = virt_ports_copy_vnet( v_net->ports, inst, false,
            true );
    v_net_new = virt_net_create_struct( ports, NULL, inst, v_net->type );
    return v_net_new;
}

/******************************************************************************/
virt_net_t* virt_net_create_net( virt_net_t* v_net, instrec_t* inst )
{
    virt_net_t* v_net_new = NULL;
    virt_port_list_t* ports = virt_ports_copy_vnet( v_net->ports, inst, true,
            false );
    net_con_t* con = net_con_create( inst );
    v_net_new = virt_net_create_struct( ports, con, inst, VNET_NET );

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
    ports1 = virt_port_assign( v_net1->ports, NULL, NULL );
    ports2 = virt_port_assign( v_net2->ports, ports1, NULL );
    con = malloc( sizeof( net_con_t ) );
    v_net = virt_net_create_struct( ports2, con, NULL, VNET_PARALLEL );
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
    ports1 = virt_port_assign( v_net1->ports, NULL, NULL );
    ports2 = virt_port_assign( v_net2->ports, ports1, NULL );
    con = malloc( sizeof( net_con_t ) );
    v_net = virt_net_create_struct( ports2, con, NULL, VNET_SERIAL );
    igraph_vector_ptr_copy( &v_net->con->left, &v_net1->con->left );
    igraph_vector_ptr_copy( &v_net->con->right, &v_net2->con->right );

    return v_net;
}

/******************************************************************************/
virt_net_t* virt_net_create_struct( virt_port_list_t* ports, net_con_t* con,
        instrec_t* inst, virt_net_type_t type )
{
    virt_net_t* v_net = malloc( sizeof( virt_net_t ) );
    v_net->inst = inst;
    v_net->ports = ports;
    v_net->con = con;
    v_net->type = type;
#if defined(DEBUG) || defined(DEBUG_CONNECT)
    printf( "virt_net_create: " );
    if( ( v_net->inst == NULL ) && ( v_net->type == VNET_SERIAL ) )
        printf( "serial: " );
    else if( ( v_net->inst == NULL ) && ( v_net->type == VNET_PARALLEL ) )
        printf( "parallel: " );
    else if( v_net->inst != NULL )
        printf( "%s(%d): ", v_net->inst->name, v_net->inst->id );
    debug_print_vports( v_net );
#endif // DEBUG_CONNECT
    return v_net;
}

/******************************************************************************/
virt_net_t* virt_net_create_sync( instrec_t* inst, virt_port_t* port )
{
    virt_net_t* v_net = NULL;
    virt_port_list_t* ports = malloc( sizeof( virt_port_list_t ) );
    ports->idx = 0;
    ports->port = port;
    ports->next = NULL;
    v_net = virt_net_create_struct( ports, NULL, inst, VNET_SYNC );

    return v_net;
}

/******************************************************************************/
virt_net_t* virt_net_create_wrap( symrec_t* symb, instrec_t* inst )
{
    virt_net_t* v_net_new = NULL;
    virt_port_list_t* ports = virt_ports_copy_symb( symb->attr_wrap->ports,
            inst );
    net_con_t* con = net_con_create( inst );
    v_net_new = virt_net_create_struct( ports, con, inst, VNET_WRAP );

    return v_net_new;
}

/******************************************************************************/
void virt_net_destroy( virt_net_t* v_net, bool deep )
{
    if( v_net == NULL ) return;

    virt_port_list_t* ports = NULL;
    // free instance
    if( v_net->inst != NULL ) instrec_destroy( v_net->inst );
    // free ports
    while( v_net->ports != NULL ) {
        ports = v_net->ports;
        v_net->ports = v_net->ports->next;
        if( deep && ( ports->port != NULL ) ) free( ports->port );
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
void virt_net_destroy_shallow( virt_net_t* v_net )
{
    virt_net_destroy( v_net, false );
}

/******************************************************************************/
void virt_net_update_class( virt_net_t* v_net, port_class_t port_class )
{
    virt_port_list_t* list = v_net->ports;
    while( list != NULL ) {
        if( ( list->port->attr_class != PORT_CLASS_SIDE )
                && ( list->port->state == VPORT_STATE_OPEN ) )
            list->port->attr_class = port_class;
        list = list->next;
    }
}

/******************************************************************************/
virt_port_t* virt_port_add( virt_net_t* v_net, port_class_t port_class,
        port_mode_t port_mode, instrec_t* port_inst, const char* name,
        symrec_t* symb )
{
    virt_port_t* new_port = NULL;
    new_port = virt_port_create( port_class, port_mode, port_inst, name, symb );
    if( v_net != NULL ) virt_port_append( v_net, new_port );
    return new_port;
}

/******************************************************************************/
void virt_port_append( virt_net_t* v_net, virt_port_t* port )
{
    virt_port_list_t* list_last = NULL;
    virt_port_list_t* list_new = NULL;
    virt_port_list_t* list = v_net->ports;
    int idx = 0;

    while( list != NULL ) {
        list_last = list;
        idx++;
        list = list->next;
    }

    list_new = malloc( sizeof( virt_port_list_t ) );
    list_new->port = port;
    list_new->idx = idx;
    list_new->next = NULL;
    if( list_last != NULL ) list_last->next = list_new;
    else v_net->ports = list_new;
#if defined(DEBUG) || defined(DEBUG_CONNECT)
    printf( "virt_port_append: Append port %s of inst %s(%d)\n", port->name,
            port->inst->name, port->inst->id );
#endif // DEBUG_CONNECT
}

/******************************************************************************/
virt_port_list_t* virt_port_assign( virt_port_list_t* old,
        virt_port_list_t* list_last, instrec_t* inst  )
{
    virt_port_list_t* new_list = NULL;
    virt_port_list_t* old_list = old;
    int idx = 0;
    if( list_last != NULL) idx = list_last->idx + 1;

    while( old_list != NULL ) {
        new_list = malloc( sizeof( virt_port_list_t ) );
        new_list->port = old_list->port;
        if( inst != NULL ) new_list->port->inst = inst;
        new_list->next = list_last;
        new_list->idx = idx;
        idx++;
        list_last = new_list;
        old_list = old_list->next;
    }
    return new_list;
}

/******************************************************************************/
virt_port_t* virt_port_create( port_class_t port_class, port_mode_t port_mode,
        instrec_t* port_inst, const char* name, symrec_t* symb )
{
    virt_port_t* new_port = NULL;

    new_port = malloc( sizeof( virt_port_t ) );
    new_port->attr_class = port_class;
    new_port->attr_mode = port_mode;
    new_port->inst = port_inst;
    new_port->name = name;
    new_port->symb = symb;
    new_port->state = VPORT_STATE_OPEN;

    return new_port;
}

/******************************************************************************/
virt_port_list_t* virt_ports_copy_symb( symrec_list_t* ports, instrec_t* inst )
{
    virt_port_t* new_port = NULL;
    virt_port_list_t* list_last = NULL;
    virt_port_list_t* vports = NULL;
    int idx = 0;

    while( ports != NULL  ) {
        vports = malloc( sizeof( virt_port_list_t ) );
        new_port = virt_port_create( ports->rec->attr_port->collection,
                ports->rec->attr_port->mode, inst, ports->rec->name,
                ports->rec );
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
virt_port_list_t* virt_ports_copy_vnet( virt_port_list_t* ports,
        instrec_t* inst, bool check_status, bool copy_status )
{
    virt_port_t* new_port = NULL;
    virt_port_list_t* list_last = NULL;
    virt_port_list_t* new_list = NULL;
    int idx = 0;

    while( ports != NULL ) {
        if( !check_status || ( ports->port->state == VPORT_STATE_OPEN ) ) {
            new_list = malloc( sizeof( virt_port_list_t ) );
            new_port = virt_port_create( ports->port->attr_class,
                    ports->port->attr_mode, inst, ports->port->name,
                    ports->port->symb );
            if( copy_status ) new_port->state = ports->port->state;
            new_list->port = new_port;
            new_list->next = list_last;
            new_list->idx = idx;
            list_last = new_list;
            idx++;
        }
        ports = ports->next;
    }
    return new_list;
}

/******************************************************************************/
virt_port_t* virt_port_get_equivalent( virt_net_t* v_net, virt_port_t* port,
        bool all )
{
    virt_port_list_t* ports = v_net->ports;
    while( ports != NULL ) {
        if( ( port->symb == ports->port->symb ) && ( all
                    || ( ports->port->state != VPORT_STATE_CONNECTED ) ) ) {
#if defined(DEBUG) || defined(DEBUG_SEARCH_PORT)
            printf( "Found port: " );
            debug_print_vport( ports->port  );
            printf( "\n" );
#endif // DEBUG
            return ports->port;
        }
        ports = ports->next;
    }
    return NULL;
}

/******************************************************************************/
virt_port_t* virt_port_get_equivalent_by_name( virt_net_t* v_net,
        const char* name )
{
    virt_port_t* vp_net = NULL;
    virt_port_list_t* vps_net = v_net->ports;
    vps_net = v_net->ports;
    while( vps_net != NULL ) {
        if( ( strlen( name ) == strlen( vps_net->port->name ) )
                && ( strcmp( name, vps_net->port->name ) == 0 ) ) {
            vp_net = vps_net->port;
            break;
        }
        vps_net = vps_net->next;
    }
    return vp_net;
}

/******************************************************************************/
virt_port_list_t* virt_ports_merge( symrec_list_t* sps_src, virt_net_t* v_net )
{
    virt_port_t* vp_new = NULL;
    virt_port_t* vp_net = NULL;
    virt_port_list_t* vps_last = NULL;
    virt_port_list_t* vps_new = NULL;
    symrec_list_t* sps_int = NULL;
    int idx = 0;

    while( sps_src != NULL  ) {
        // search for the port in the virtual net of the connection
        vp_net = virt_port_get_equivalent_by_name( v_net, sps_src->rec->name );
        sps_int = sps_src->rec->attr_port->ports_int;
        if( sps_int != NULL ) {
            // if there are internal ports, copy them to the new virtual port
            // list use the alt name of the port but all other info from the
            // net port
            while( sps_int != NULL ) {
                vps_new = malloc( sizeof( virt_port_list_t ) );
                vp_new = virt_port_create( vp_net->attr_class,
                        vp_net->attr_mode, vp_net->inst, sps_int->rec->name,
                        vp_net->symb );
                sps_int = sps_int->next;
                vp_net->state = VPORT_STATE_DISABLED;
                vps_new->port = vp_new;
                vps_new->next = vps_last;
                vps_new->idx = idx;
                vps_last = vps_new;
                idx++;
            }
        }
        else {
            // there was no internal port, copy the regula port to the new list
            vps_new = malloc( sizeof( virt_port_list_t ) );
            vp_new = virt_port_create( vp_net->attr_class, vp_net->attr_mode,
                    vp_net->inst, vp_net->name, vp_net->symb );
            vps_new->port = vp_new;
            vps_new->next = vps_last;
            vps_new->idx = idx;
            vps_last = vps_new;
            idx++;
        }
        sps_src = sps_src->next;
    }
    return vps_last;
}

/******************************************************************************/
void debug_print_vport( virt_port_t* port )
{
    if( port->state == VPORT_STATE_CONNECTED ) printf("+");
    else if( port->state == VPORT_STATE_DISABLED ) printf("!");
    printf( "%s(%d)", port->inst->name, port->inst->id );
    if( port->attr_class == PORT_CLASS_DOWN ) printf( "_" );
    else if( port->attr_class == PORT_CLASS_UP ) printf( "^" );
    else if( port->attr_class == PORT_CLASS_SIDE ) printf( "|" );
    if( port->attr_mode == PORT_MODE_IN ) printf( "<--" );
    else if( port->attr_mode == PORT_MODE_OUT ) printf( "-->" );
    else printf( "<->" );
    printf( "%s(%p)", port->name, port->symb );
}

/******************************************************************************/
void debug_print_vports( virt_net_t* v_net )
{
    virt_port_list_t* ports = NULL;
    if( v_net->ports != NULL )
        ports = v_net->ports;
    while( ports != NULL ) {
        debug_print_vport( ports->port );
        printf(", ");
        ports = ports->next;
    }
    printf("\n");
}
