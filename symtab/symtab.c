/**
 * A simple symbol table plugin
 *
 * @file    symtab.c
 * @author  Anthony A. Aaby
 * @author  Simon Maurer
 * @see     http://foja.dcs.fmph.uniba.sk/kompilatory/docs/compiler.pdf
 *
 */

#include <stdio.h>
#include "symtab.h"
#include "ast.h"
#include "smxgraph.h"
#ifdef TESTING
#else
#include "smxerr.h"
#endif

/******************************************************************************/
attr_box_t* symrec_attr_create_box( bool attr_pure, char* impl_name,
        symrec_list_t* ports )
{
    attr_box_t* new_attr = malloc( sizeof( attr_box_t ) );
    new_attr->attr_pure = attr_pure;
    new_attr->impl_name = malloc( strlen( impl_name ) + 1 );
    strcpy( new_attr->impl_name, impl_name );
    new_attr->ports = ports;
    return new_attr;
}

/******************************************************************************/
attr_net_t* symrec_attr_create_net( virt_net_t* v_net, igraph_t* g )
{
    attr_net_t* new_attr = malloc( sizeof( attr_net_t ) );
    new_attr->v_net = v_net;
    new_attr->g = *g;
    return new_attr;
}

/******************************************************************************/
attr_port_t* symrec_attr_create_port( symrec_list_t* ports_int,
        port_mode_t mode, port_class_t collection, bool decoupled, int sync_id )
{
    attr_port_t* new_attr = malloc( sizeof( attr_port_t ) );
    new_attr->ports_int = ports_int;
    new_attr->mode = mode;
    new_attr->collection = collection;
    new_attr->decoupled = decoupled;
    new_attr->sync_id = sync_id;
    return new_attr;
}

/******************************************************************************/
attr_prot_t* symrec_attr_create_proto( symrec_list_t* ports )
{
    attr_prot_t* new_attr = malloc( sizeof( attr_prot_t ) );
    new_attr->ports = ports;
    return new_attr;
}

/******************************************************************************/
attr_wrap_t* symrec_attr_create_wrap( bool attr_static, symrec_list_t* ports,
        virt_net_t* v_net, igraph_t* g )
{
    attr_wrap_t* new_attr = malloc( sizeof( attr_wrap_t ) );
    new_attr->attr_static = attr_static;
    new_attr->v_net = v_net;
    new_attr->ports = ports;
    new_attr->g = *g;
    return new_attr;
}

/******************************************************************************/
void symrec_attr_destroy_box( attr_box_t* attr )
{
    free( attr->impl_name );
    symrec_list_del( attr->ports );
    free( attr );
}

/******************************************************************************/
void symrec_attr_destroy_net( attr_net_t* attr )
{
    if( ( attr->v_net != NULL ) && ( ( attr->v_net->type == VNET_SERIAL )
                || ( attr->v_net->type ==  VNET_PARALLEL ) ) )
        virt_net_destroy_shallow( attr->v_net );
    dgraph_destroy( &attr->g );
    free( attr );
}

/******************************************************************************/
void symrec_attr_destroy_port( attr_port_t* attr ) 
{
    symrec_list_del( attr->ports_int );
    free( attr );
}

/******************************************************************************/
void symrec_attr_destroy_proto( attr_prot_t* attr )
{
    symrec_list_del( attr->ports );
    free( attr );
}

/******************************************************************************/
void symrec_attr_destroy_wrap( attr_wrap_t* attr )
{
    symrec_list_del( attr->ports );
    if( ( attr->v_net != NULL ) && ( ( attr->v_net->type == VNET_SERIAL )
                || ( attr->v_net->type ==  VNET_PARALLEL ) ) )
        virt_net_destroy_shallow( attr->v_net );
    dgraph_destroy( &attr->g );
    free( attr );
}

/******************************************************************************/
symrec_t* symrec_create( char* name, int scope, symrec_type_t type, int line,
        int attr_key )
{
    char key[ strlen( name ) + 1 + CONST_SCOPE_LEN + 1 ];
    symrec_t* new_item = NULL;

    // generate key
    sprintf( key, "%s%d%d", name, scope, attr_key );
    // create new item structure
    new_item = malloc( sizeof( symrec_t ) );
    new_item->scope = scope;
    new_item->type = type;
    new_item->line = line;
    new_item->key = ( char* )malloc( strlen( key ) + 1 );
    strcpy( new_item->key, key );
    new_item->name = ( char* )malloc( strlen( name ) + 1 );
    strcpy( new_item->name, name );
    new_item->next = NULL;

    return new_item;
}

/******************************************************************************/
symrec_t* symrec_create_box( char* name, int scope, int line, attr_box_t* attr )
{
    symrec_t* item = symrec_create( name, scope, SYMREC_BOX, line, 0 );
    item->attr_box = attr;
    return item;
}

/******************************************************************************/
symrec_t* symrec_create_net( char* name, int scope, int line, attr_net_t* attr )
{
    symrec_t* item = symrec_create( name, scope, SYMREC_NET, line, 0 );
    item->attr_net = attr;
    return item;
}

/******************************************************************************/
symrec_t* symrec_create_port( char* name, int scope, int line,
        attr_port_t* attr )
{
    symrec_t* item = symrec_create( name, scope, SYMREC_PORT, line,
            attr->collection + 1 );
    item->attr_port = attr;
    return item;
}

/******************************************************************************/
symrec_t* symrec_create_proto( char* name, int scope, int line,
        attr_prot_t* attr )
{
    symrec_t* item = symrec_create( name, scope, SYMREC_NET_PROTO, line, 0 );
    item->attr_proto = attr;
    return item;
}

/******************************************************************************/
symrec_t* symrec_create_wrap( char* name, int scope, int line,
        attr_wrap_t* attr )
{
    symrec_t* item = symrec_create( name, scope, SYMREC_WRAP, line, 0 );
    item->attr_wrap = attr;
    return item;
}

/******************************************************************************/
void symrec_del( symrec_t** symtab, symrec_t* rec )
{
    symrec_t* rec_temp;
    HASH_FIND_STR( *symtab, rec->key, rec_temp );
    if( rec_temp == NULL ) {
#if defined(DEBUG) || defined(DEBUG_SYMB)
        printf( "symrec_del: No record %s in scope %d to delete\n", rec->name,
                rec->scope );
#endif // DEBUG
        return;
    }
#if defined(DEBUG) || defined(DEBUG_SYMB)
    printf( "symrec_del: delete record %s in scope %d\n", rec_temp->name,
            rec_temp->scope );
#endif // DEBUG
    // delete name entry only if no other record with the same name exists
    if( rec_temp->next == NULL )
        HASH_DELETE( hh, *symtab, rec_temp );
    switch( rec_temp->type ) {
        case SYMREC_BOX:
            symrec_attr_destroy_box( rec->attr_box );
            break;
        case SYMREC_NET:
            symrec_attr_destroy_net( rec->attr_net );
            break;
        case SYMREC_NET_PROTO:
            symrec_attr_destroy_proto( rec->attr_proto );
            break;
        case SYMREC_PORT:
            symrec_attr_destroy_port( rec->attr_port );
            break;
        case SYMREC_WRAP:
            symrec_attr_destroy_wrap( rec->attr_wrap );
            break;
        default:
            ;
    }
    symrec_destroy( rec );
}

/******************************************************************************/
void symrec_del_all( symrec_t** recs )
{
    symrec_t *rec, *tmp;
    HASH_ITER( hh, *recs, rec, tmp ) {
        symrec_del( recs, rec );
    }
}

/******************************************************************************/
void symrec_destroy( symrec_t* rec )
{
    free( rec->name );
    free( rec->key );
    free( rec );
}

/******************************************************************************/
symrec_t* symrec_get( symrec_t** symtab, UT_array* scope_stack, char *name,
        int line, int attr_key )
{
    symrec_t* item = NULL;
#ifndef TESTING
    char error_msg[ CONST_ERROR_LEN ];
#endif // TESTING

    /* check whether their scope matches with a scope on the stack */
    item = symrec_search( symtab, scope_stack, name, attr_key );
    if( item == NULL ) {
#ifdef TESTING
        printf( ERROR_UNDEF_ID, ERR_ERROR, name );
        printf( "\n" );
#else
        sprintf( error_msg, ERROR_UNDEF_ID, ERR_ERROR, name );
        report_yyerror( error_msg, line );
#endif // TESTING
    }
#if defined(DEBUG) || defined(DEBUG_SYMB)
    else {
        printf( "found symbol %s in scope %d\n", item->name, item->scope );
    }
#endif // DEBUG
    return item;
}

/******************************************************************************/
void symrec_list_del( symrec_list_t* list )
{
    symrec_list_t* list_tmp;
    while( list != NULL ) {
        list_tmp = list;
        list = list->next;
        free( list_tmp );
    }
}

/******************************************************************************/
symrec_t* symrec_put( symrec_t** symtab, symrec_t* new_item )
{
#ifndef TESTING
    char error_msg[ CONST_ERROR_LEN ];
#endif // TESTING
    symrec_t* item = NULL;
    // check wheter key already exists
    HASH_FIND_STR( *symtab, new_item->key, item );
    // the key is new
    if( item == NULL ) {
        HASH_ADD_KEYPTR( hh, *symtab, new_item->key, strlen( new_item->key ),
                new_item );
        item = new_item;
#if defined(DEBUG) || defined(DEBUG_SYMB)
        printf( "added symbol %s in scope %d\n", item->name, item->scope );
#endif // DEBUG
    }
    /* hash exists */
    else {
        // throw an yyerror
#ifdef TESTING
        printf( ERROR_DUPLICATE_ID, ERR_ERROR, new_item->name );
        printf( "\n" );
#else
        sprintf( error_msg, ERROR_DUPLICATE_ID, ERR_ERROR, new_item->name );
        report_yyerror( error_msg, new_item->line );
#endif // TESTING
        item = NULL;
    }
    return item;
}

/******************************************************************************/
symrec_t* symrec_search( symrec_t** symtab, UT_array* scope_stack, char *name,
        int attr_key )
{
    int* p = NULL;
    symrec_t* item = NULL;
    char key[ strlen( name ) + 1 + CONST_SCOPE_LEN ];
    while( ( p = ( int* )utarray_prev( scope_stack, p ) ) != NULL ) {
        // generate key
        sprintf( key, "%s%d%d", name, *p, attr_key );
        HASH_FIND_STR( *symtab, key, item );
        if( item != NULL ) break; // found a match
    }
    return item;
}
