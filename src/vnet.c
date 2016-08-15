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
bool are_port_names_ok( virt_port_t* p1, virt_port_t* p2 )
{
    // no, if port names do not match
    if( strlen( p1->name ) != strlen( p2->name ) )
        return false;
    if( strcmp( p1->name, p2->name ) != 0 )
        return false;
    // we came through here, so all is good, names match
    return true;
}

/******************************************************************************/
bool are_port_classes_ok( virt_port_t* p1, virt_port_t* p2, bool directed )
{
    // normal undirected connections?
    if( !directed ) {
        // ok, if either of the ports has no class speciefied
        if( ( p1->attr_class == PORT_CLASS_NONE )
                || ( p2->attr_class == PORT_CLASS_NONE ) )
            return true;
        // ok if one port has class DOWN and one UP
        if( ( p1->attr_class == PORT_CLASS_DOWN )
                && ( p2->attr_class == PORT_CLASS_UP ) )
            return true;
        // ok if one port has class UP and one DOWN
        if( ( p1->attr_class == PORT_CLASS_UP )
                && ( p2->attr_class == PORT_CLASS_DOWN ) )
            return true;
        // we came through here so none of the conditions matched
        return false;
    }
    // or normal directed connections?
    else {
        // no, if the left port is in another class than DS
        if( ( p1->attr_class != PORT_CLASS_DOWN )
                && ( p1->attr_class != PORT_CLASS_NONE ) )
            return false;
        // no, if the right port is in another class than US
        if( ( p2->attr_class != PORT_CLASS_UP )
                && ( p2->attr_class != PORT_CLASS_NONE ) )
            return false;
    }
    // we came through here, so all is good, names match
    return true;
}

/******************************************************************************/
bool are_port_cp_classes_ok( virt_port_t* p1, virt_port_t* p2, bool cpp )
{
    // we are good if both ports have the same class
    if( p1->attr_class == p2->attr_class )
        return true;
    // are we checking parallel combinators?
    if( cpp ) {
        // we are good if one port has no class and the other is not a side port
        if( ( p1->attr_class != PORT_CLASS_SIDE )
                && ( p2->attr_class == PORT_CLASS_NONE ) )
            return true;
        if( ( p1->attr_class == PORT_CLASS_NONE )
                && ( p2->attr_class != PORT_CLASS_SIDE ) )
            return true;
    }
    // we came through here so none of the conditions matched
    return false;
}

/******************************************************************************/
bool are_port_modes_ok( virt_port_t* p1, virt_port_t* p2, bool equal )
{
    // yes, we are checking whether modes are different and they are
    if( !equal && ( p1->attr_mode != p2->attr_mode ) ) return true;
    // yes, we are checking whether modes are equal and they are
    if( equal && ( p1->attr_mode == p2->attr_mode ) ) return true;
    // yes, if the left port is a copy synchronizer port
    if( p1->attr_mode == PORT_MODE_BI ) return true;
    // yes, if the right port is a copy synchronizer port
    if( p2->attr_mode == PORT_MODE_BI ) return true;

    // we came through here, so port modes are not compatible
    return false;
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
virt_net_t* virt_net_create_box( symrec_t* rec, instrec_t* inst )
{
    virt_net_t* v_net = malloc( sizeof( virt_net_t ) );
    v_net->ports = virt_ports_copy_symb( rec->attr_box->ports, v_net, NULL );
    v_net->con = net_con_create( inst );
    v_net->inst = inst;
    v_net->type = VNET_BOX;
#if defined(DEBUG) || defined(DEBUG_VNET)
    printf( "virt_net_create_box:\n %s(%d): ", inst->name, inst->id );
    debug_print_vports( v_net );
#endif // DEBUG_CONNECT

    return v_net;
}

/******************************************************************************/
virt_net_t* virt_net_create_flatten( virt_net_t* v_net_n, instrec_t* inst )
{
    virt_net_t* v_net = malloc( sizeof( virt_net_t ) );
    v_net->ports = virt_ports_copy_vnet( v_net_n->ports, v_net, false, true );
    v_net->con = NULL;
    v_net->inst = inst;
    v_net->type = v_net_n->type;
#if defined(DEBUG) || defined(DEBUG_VNET)
    printf( "virt_net_create_flatten:\n %s(%d): ", inst->name, inst->id );
    debug_print_vports( v_net );
#endif // DEBUG_CONNECT
    return v_net;
}


/******************************************************************************/
virt_net_t* virt_net_create_net( virt_net_t* v_net_n, instrec_t* inst )
{
    virt_net_t* v_net = malloc( sizeof( virt_net_t ) );
    v_net->ports = virt_ports_copy_vnet( v_net_n->ports, v_net, true, false );
    v_net->con = net_con_create( inst );
    v_net->inst = inst;
    v_net->type = VNET_NET;
#if defined(DEBUG) || defined(DEBUG_VNET)
    printf( "virt_net_create_net:\n %s(%d): ", inst->name, inst->id );
    debug_print_vports( v_net );
#endif // DEBUG_CONNECT
    return v_net;
}

/******************************************************************************/
virt_net_t* virt_net_create_parallel( virt_net_t* v_net1, virt_net_t* v_net2 )
{
    virt_port_list_t* ports_tmp = NULL;
    virt_net_t* v_net = malloc( sizeof( virt_net_t ) );
    v_net->inst = NULL;
    v_net->type = VNET_PARALLEL;

    // alter ports
    ports_tmp = virt_port_assign( v_net1->ports, NULL, NULL );
    v_net->ports = virt_port_assign( v_net2->ports, ports_tmp, NULL );

    // create connection list
    v_net->con = malloc( sizeof( net_con_t ) );
    igraph_vector_ptr_copy( &v_net->con->left, &v_net1->con->left );
    igraph_vector_ptr_append( &v_net->con->left, &v_net2->con->left );
    igraph_vector_ptr_copy( &v_net->con->right, &v_net1->con->right );
    igraph_vector_ptr_append( &v_net->con->right, &v_net2->con->right );
#if defined(DEBUG) || defined(DEBUG_VNET)
    printf( "virt_net_create_parallel:\n " );
    debug_print_vports( v_net );
#endif // DEBUG_CONNECT
    return v_net;
}

/******************************************************************************/
virt_net_t* virt_net_create_serial( virt_net_t* v_net1, virt_net_t* v_net2 )
{
    virt_port_list_t* ports_tmp = NULL;
    virt_net_t* v_net = malloc( sizeof( virt_net_t ) );
    v_net->inst = NULL;
    v_net->type = VNET_SERIAL;

    // alter ports
    ports_tmp = virt_port_assign( v_net1->ports, NULL, NULL );
    v_net->ports = virt_port_assign( v_net2->ports, ports_tmp, NULL );

    // create connection list
    v_net->con = malloc( sizeof( net_con_t ) );
    igraph_vector_ptr_copy( &v_net->con->left, &v_net1->con->left );
    igraph_vector_ptr_copy( &v_net->con->right, &v_net2->con->right );
#if defined(DEBUG) || defined(DEBUG_VNET)
    printf( "virt_net_create_serial:\n " );
    debug_print_vports( v_net );
#endif // DEBUG_CONNECT
    return v_net;
}

/******************************************************************************/
virt_net_t* virt_net_create_sync( instrec_t* inst, virt_port_t* port )
{
    virt_net_t* v_net = malloc( sizeof( virt_net_t ) );
    v_net->ports = NULL;
    if( port != NULL ) {
        v_net->ports = malloc( sizeof( virt_port_list_t ) );
        v_net->ports->idx = 0;
        v_net->ports->next = NULL;
        v_net->ports->port = port;
        v_net->ports->port->v_net = v_net;
    }
    v_net->inst = inst;
    v_net->con = NULL;
    v_net->type = VNET_SYNC;
#if defined(DEBUG) || defined(DEBUG_VNET)
    printf( "virt_net_create_sync:\n %s(%d): ", inst->name, inst->id );
    debug_print_vports( v_net );
#endif // DEBUG_CONNECT
    return v_net;
}

/******************************************************************************/
virt_net_t* virt_net_create_wrap( symrec_t* symb, instrec_t* inst )
{
    virt_net_t* v_net = malloc( sizeof( virt_net_t ) );
    v_net->ports = virt_ports_copy_symb( symb->attr_wrap->ports, v_net,
            symb->attr_wrap->v_net );
    v_net->con = net_con_create( inst );
    v_net->inst = inst;
    v_net->type = VNET_WRAP;
#if defined(DEBUG) || defined(DEBUG_VNET)
    printf( "virt_net_create_wrap:\n %s(%d): ", inst->name, inst->id );
    debug_print_vports( v_net );
#endif // DEBUG_CONNECT
    return v_net;
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
                && ( list->port->state < VPORT_STATE_CONNECTED ) )
            list->port->attr_class = port_class;
        list = list->next;
    }
}

/******************************************************************************/
virt_port_t* virt_port_add( virt_net_t* v_net, port_class_t port_class,
        port_mode_t port_mode, const char* name,
        symrec_t* symb )
{
    virt_port_t* new_port = NULL;
    new_port = virt_port_create( port_class, port_mode, v_net, name, symb );
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
    printf( "virt_port_append: Append port %s to", port->name );
    if( ( v_net != NULL ) && ( v_net->inst != NULL ) )
        printf( " inst %s(%d)\n", v_net->inst->name, v_net->inst->id );
    else printf( " inner net\n" ) ;
#endif // DEBUG_CONNECT
}

/******************************************************************************/
void virt_port_append_all( virt_net_t* v_net1, virt_net_t* v_net2,
        bool update_inst )
{
    virt_port_list_t* ports = v_net2->ports;
    while( ports != NULL ) {
        if( update_inst ) virt_port_update_inst( ports->port, v_net1 );
        virt_port_append( v_net1, ports->port );
        ports = ports->next;
    }
}

/******************************************************************************/
virt_port_list_t* virt_port_assign( virt_port_list_t* old,
        virt_port_list_t* list_last, virt_net_t* v_net  )
{
    virt_port_list_t* new_list = NULL;
    virt_port_list_t* old_list = old;
    int idx = 0;
    if( list_last != NULL) idx = list_last->idx + 1;

    while( old_list != NULL ) {
        new_list = malloc( sizeof( virt_port_list_t ) );
        new_list->port = old_list->port;
        if( v_net != NULL ) new_list->port->v_net = v_net;
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
        virt_net_t* port_vnet, const char* name, symrec_t* symb )
{
    virt_port_t* new_port = NULL;

    new_port = malloc( sizeof( virt_port_t ) );
    new_port->attr_class = port_class;
    new_port->attr_mode = port_mode;
    new_port->v_net = port_vnet;
    new_port->name = name;
    new_port->symb = symb;
    new_port->state = VPORT_STATE_OPEN;

    return new_port;
}

/******************************************************************************/
virt_port_list_t* virt_ports_copy_symb( symrec_list_t* ports,
        virt_net_t* v_net, virt_net_t* v_net_i )
{
    virt_port_t* new_port = NULL;
    virt_port_t* port_net = NULL;
    virt_port_list_t* list_last = NULL;
    virt_port_list_t* vports = NULL;
    int idx = 0;

    while( ports != NULL  ) {
        vports = malloc( sizeof( virt_port_list_t ) );
        new_port = virt_port_create( ports->rec->attr_port->collection,
                ports->rec->attr_port->mode, v_net, ports->rec->name,
                ports->rec );
        if( v_net_i != NULL ) {
            port_net = dgraph_port_search_wrap( v_net_i, new_port );
            new_port->symb = port_net->symb;
            /* printf( "symbols: %p -> %p\n", ports->rec, port_net->symb ); */
        }
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
        virt_net_t* v_net, bool check_status, bool copy_status )
{
    virt_port_t* new_port = NULL;
    virt_port_list_t* list_last = NULL;
    virt_port_list_t* new_list = NULL;
    int idx = 0;

    while( ports != NULL ) {
        if( !check_status || ( ports->port->state < VPORT_STATE_CONNECTED ) ) {
        /* if( !check_status || ( ports->port->state == VPORT_STATE_OPEN ) ) { */
            new_list = malloc( sizeof( virt_port_list_t ) );
            new_port = virt_port_create( ports->port->attr_class,
                    ports->port->attr_mode, v_net, ports->port->name,
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
                    || ( ports->port->state <= VPORT_STATE_CP_OPEN ) ) ) {
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
virt_port_t* virt_port_get_equivalent_by_name( virt_port_list_t* vps_net,
        const char* name )
{
    virt_port_t* vp_net = NULL;
#if defined(DEBUG) || defined(DEBUG_SEARCH_PORT_WRAP)
    virt_port_list_t* ports = vps_net;
    printf( "virt_port_get_equivalent_by_name: Search port '%s'\n", name );
    printf( " in virtual net: " );
    while( ports != NULL ) {
        debug_print_vport( ports->port );
        printf(", ");
        ports = ports->next;
    }
    printf("\n");
#endif // DEBUG
    while( vps_net != NULL ) {
        if( ( strlen( name ) == strlen( vps_net->port->name ) )
                && ( strcmp( name, vps_net->port->name ) == 0 ) ) {
            vp_net = vps_net->port;
            if( vp_net->v_net->type == VNET_SYNC ) break;
            /* break; */
        }
        vps_net = vps_net->next;
    }
#if defined(DEBUG) || defined(DEBUG_SEARCH_PORT_WRAP)
    printf( "Found port: " );
    debug_print_vport( vp_net );
    printf( "\n" );
#endif // DEBUG
    return vp_net;
}

/******************************************************************************/
virt_port_t* dgraph_port_search_wrap( virt_net_t* v_net, virt_port_t* port )
{
    // => find a namesake in the net interface of the wrapper
#if defined(DEBUG) || defined(DEBUG_SEARCH_PORT_WRAP)
    printf( "dgrap_port_search_wrap: Search port " );
    debug_print_vport( port );
    printf( "\n in net interface of wrapper: " );
    debug_print_vports( v_net );
#endif // DEBUG
    virt_port_t* port_net = NULL;
    virt_port_list_t* ports = v_net->ports;
    while( ports != NULL ) {
        if( are_port_names_ok( ports->port, port )
                && are_port_modes_ok( ports->port, port, true ) ) {
            port_net = ports->port;
#if defined(DEBUG) || defined(DEBUG_SEARCH_PORT_WRAP)
            printf( "Found port: " );
            debug_print_vport( ports->port  );
            printf( "\n" );
#endif // DEBUG
            if( port_net->v_net->inst->type == INSTREC_SYNC ) break;
        }
        ports = ports->next;
    }
    return port_net;
}

/******************************************************************************/
void virt_port_update_inst( virt_port_t* port, virt_net_t* v_net )
{
#if defined(DEBUG) || defined(DEBUG_CONNECT)
    printf( "virt_port_update_inst: %s(%s(%d)->%s(%d))\n", port->name,
            port->v_net->inst->name, port->v_net->inst->id, v_net->inst->name,
            v_net->inst->id );
#endif // DEBUG
    port->v_net = v_net;
}

/******************************************************************************/
void debug_print_vport( virt_port_t* port )
{
    if( port->state == VPORT_STATE_CONNECTED ) printf("+");
    else if( port->state == VPORT_STATE_CP_OPEN ) printf("-");
    else if( port->state == VPORT_STATE_DISABLED ) printf("!");
    if( port->v_net->inst == NULL ) printf( "UNDEF" );
    else printf( "%s(%p,%d)", port->v_net->inst->name, port->v_net->inst,
            port->v_net->inst->id );
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
