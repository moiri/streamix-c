#include "insttab.h"
#include "defines.h"
#include "ast.h"
#ifndef TESTING
#include "error.h"
#endif
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
void inst_rec_cleanup( inst_net* net, inst_rec* inst, int id_start,
        int id_end ) {
    int id;
    // delete one copy synchronizer from insttab
    inst_rec_del( &net->recs_name, &net->recs_id, inst );
    // adjust all ids starting from the id of the deleted record
    for( id = id_start ; id < id_end; id++ )
        inst_rec_replace_id( &net->recs_id, id, id - 1 );
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
void inst_rec_replace_id( inst_rec** recs, int id_old, int id_new )
{
    inst_rec* item = NULL;
    HASH_FIND( hh_id, *recs, &id_old, sizeof( int ), item );
    HASH_DELETE( hh_id, *recs, item );
    item->id = id_new;
    HASH_ADD( hh_id, *recs, id, sizeof( int ), item );
#if defined(DEBUG) || defined(DEBUG_INST)
    printf( "inst_rec_replace: replace inst id %d with %d\n", id_old, id_new );
#endif // DEBUG
}

/******************************************************************************/
void inst_rec_del( inst_rec** recs_name, inst_rec** recs_id, inst_rec* rec )
{
    inst_rec* rec_name;
#if defined(DEBUG) || defined(DEBUG_INST)
    printf( "inst_rec_del: delete instance %s(%d)\n", rec->name, rec->id );
#endif // DEBUG
    rec_name = inst_rec_get_name( recs_name, rec->name );
    // delete name entry only if no other record with the same name exists
    if( rec_name->next == NULL ) HASH_DELETE( hh_name, *recs_name, rec );
    HASH_DELETE( hh_id, *recs_id, rec );
    free( rec->name );
    free( rec );
}

/******************************************************************************/
void virt_net_check( symrec_list* r_ports, virt_ports* v_ports, char *name )
{
    symrec_list* r_port_ptr = r_ports;
    virt_ports* v_port_ptr = v_ports;
    port_attr* r_port_attr = NULL;
    int match = false;
    int v_count = 0;
    int r_count = 0;
#ifndef TESTING
    char error_msg[ CONST_ERROR_LEN ];
#endif // TESTING

    while( r_port_ptr != NULL  ) {
        r_count++;
        r_port_ptr = r_port_ptr->next;
    }

    while( v_port_ptr != NULL  ) {
        v_count++;
        v_port_ptr = v_port_ptr->next;
    }

    if( r_count != v_count ) {
#ifdef TESTING
        printf( ERROR_TYPE_CONFLICT, ERR_ERROR, name );
        printf( "\n" );
#else
        sprintf( error_msg, ERROR_TYPE_CONFLICT, ERR_ERROR, name );
        report_yyerror( error_msg, r_ports->rec->line );
#endif // TESTING
        return;
    }

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

    if( !match ) {
#ifdef TESTING
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
    igraph_vector_ptr_append( &v_net1->con->left, &v_net2->con->left );
    igraph_vector_ptr_append( &v_net1->con->right, &v_net2->con->right );

    // destroy obsolete connection vectors
    igraph_vector_ptr_destroy( &v_net2->con->left );
    igraph_vector_ptr_destroy( &v_net2->con->right );

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
    igraph_vector_ptr_destroy( &v_net1->con->right );
    igraph_vector_ptr_destroy( &v_net2->con->left );

    // alter connection vectors of virtual net
    v_net1->con->right = v_net2->con->right;

    return v_net1;
}
