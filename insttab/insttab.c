#include "insttab.h"
#include "defines.h"
#include "ast.h"
#include <stdio.h>

/******************************************************************************/
inst_net* inst_net_get( inst_net** nets, int scope )
{
    inst_net* res = NULL;
    HASH_FIND_INT( *nets, &scope, res );
#if defined(DEBUG) || defined(DEBUG_INST)
    if( res != NULL )
        printf( "inst_net_get: found nets in scope %d\n", res->scope );
    else
        printf( "inst_net_get: no nets found in scope %d\n", scope );
#endif // DEBUG
    return res;
}

/******************************************************************************/
inst_net* inst_net_put( inst_net** nets, int scope )
{
    inst_net* item = NULL;
    inst_net* new_item = NULL;
    inst_net* previous_item = NULL;
    net_con* con = NULL;
    igraph_t g;

    // ADD ITEM TO THE INSTANCE TABLE
    // create new item structure
    new_item = ( inst_net* )malloc( sizeof( inst_net ) );
    new_item->scope = scope;
    new_item->recs_id = NULL;
    new_item->recs_name = NULL;
    // create new graph
    igraph_empty( &g, 0, true );
    new_item->g = g;
    // initialize the connection vectors
    con = ( net_con* )malloc( sizeof( net_con ) );
    igraph_vector_init( &con->left, 0 );
    igraph_vector_init( &con->right, 0 );
    new_item->con = con;
    // check wheter key already exists
    HASH_FIND_INT( *nets, &scope, item );
    if( item == NULL ) {
        // the key is new
        HASH_ADD_INT( *nets, scope, new_item );
    }
    else {
        // a collision accured or multiple instances of a net have been spawned
        // -> add the new item to the end of the linked list
        do {
            previous_item = item; // remember the last item of the list
            item = item->next;
        }
        while( item != NULL );

        previous_item->next = new_item;
    }
#if defined(DEBUG) || defined(DEBUG_INST)
    printf( "inst_net_put: add net in scope %d\n", new_item->scope );
#endif // DEBUG
    return new_item;
}

/******************************************************************************/
inst_rec* inst_rec_get_id( inst_rec** recs, int id )
{
    inst_rec* res = NULL;
    HASH_FIND( hh_id, *recs, &id, sizeof( int ), res );
#if defined(DEBUG) || defined(DEBUG_INST)
    if( res != NULL )
        printf( "inst_rec_get_id: found instance '%s'(%d)\n", res->name,
                res->id );
    else
        printf( "inst_rec_get_id: found no instances with id '%d'\n", id );
#endif // DEBUG
    return res;
}

/******************************************************************************/
inst_rec* inst_rec_get_name( inst_rec** recs, char* name )
{
    inst_rec* res = NULL;
    HASH_FIND( hh_name, *recs, name, strlen( name ), res );
#if defined(DEBUG) || defined(DEBUG_INST)
    if( res != NULL )
        printf( "inst_rec_get_name: found instances of '%s'\n", res->name );
    else
        printf( "inst_rec_get_name: found no instances of '%s'\n", name );
#endif // DEBUG
    return res;
}

/******************************************************************************/
inst_rec* inst_rec_put( inst_rec** recs_name, inst_rec** recs_id, char* name,
        int id, int line, int type, symrec* rec )
{
    inst_rec* item = NULL;
    inst_rec* new_item = NULL;
    inst_rec* previous_item = NULL;

    // ADD ITEM TO THE INSTANCE TABLE
    // create new item structure
    new_item = ( inst_rec* )malloc( sizeof( inst_rec ) );
    new_item->id = id;
    new_item->line = line;
    new_item->type = type;
    new_item->net = rec;
    new_item->name = ( char* )malloc( strlen( name ) + 1 );
    strcpy( new_item->name, name );
    // check whether key already exists
    HASH_FIND( hh_id, *recs_id, &id, sizeof( int ), item );
    if( item == NULL ) {
        // id must be unique in the net
        HASH_ADD( hh_id, *recs_id, id, sizeof( int ), new_item );
    }
    /* else { printf( "ERROR: Something went wrong!" ); } */
    item = NULL;
    HASH_FIND( hh_name, *recs_name, name, strlen( name ), item );
    if( item == NULL ) {
        // the key is new
        HASH_ADD_KEYPTR( hh_name, *recs_name, name, strlen( name ), new_item );
    }
    else {
        // a collision accured or multiple instances of a net have been spawned
        // -> add the new item to the end of the linked list
        do {
            previous_item = item; // remember the last item of the list
            item = item->next;
        }
        while( item != NULL );

        previous_item->next = new_item;
    }
#if defined(DEBUG) || defined(DEBUG_INST)
    printf( "inst_rec_put: add net instance %s with id %d\n", new_item->name,
            new_item->id );
#endif // DEBUG
    return new_item;
}

/******************************************************************************/
void inst_rec_del( inst_rec** recs_name, inst_rec** recs_id, inst_rec* rec )
{
    inst_rec* rec_name;
    rec_name = inst_rec_get_name( recs_name, rec->name );
    // delete name entry only if no other record with the same name exists
    if( rec_name->next == NULL ) HASH_DELETE( hh_name, *recs_id, rec );
    HASH_DELETE( hh_id, *recs_id, rec );
    free( rec->name );
    free( rec );
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
    igraph_vector_init( &v_net->con->left, 1 );
    VECTOR( v_net->con->left )[ 0 ] = inst->id;
    igraph_vector_copy( &v_net->con->right, &v_net->con->left );

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
    igraph_vector_destroy( &v_net->con->left );
    igraph_vector_destroy( &v_net->con->right );
    // free structures
    free( v_net->con );
    free( v_net );
}

/******************************************************************************/
virt_net* virt_net_alter_parallel( virt_net* v_net1, virt_net* v_net2 )
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
    igraph_vector_append( &v_net1->con->left, &v_net2->con->left );
    igraph_vector_append( &v_net1->con->right, &v_net2->con->right );

    // destroy obsolete connection vectors
    igraph_vector_destroy( &v_net2->con->left );
    igraph_vector_destroy( &v_net2->con->right );

    // free unsused structures
    free( v_net2->con );
    free( v_net2 );

    return v_net1;
}

/******************************************************************************/
virt_net* virt_net_alter_serial( virt_net* v_net1, virt_net* v_net2 )
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
    igraph_vector_destroy( &v_net1->con->right );
    igraph_vector_destroy( &v_net2->con->left );

    // alter connection vectors of virtual net
    v_net1->con->right = v_net2->con->right;

    // free unsused structures
    free( v_net2->con );
    free( v_net2 );

    return v_net1;
}
