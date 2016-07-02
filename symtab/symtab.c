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
#ifdef TESTING
#include "defines.h"
#else
#include "smxerr.h"
#endif

/******************************************************************************/
bool is_symrec_identical( symrec_t* rec1, symrec_t* rec2, symrec_type_t type )
{
    if( ( rec1->type != rec2->type )
            || ( rec1->type != type ) || ( rec2->type != type ) )
        return false; // types do not match

    if( ( strlen( rec1->name ) != strlen( rec2->name ) )
            || ( memcmp( rec1->name, rec2->name, strlen( rec1->name ) ) == 0 ) )
        return false; // name does not match

    if( rec1->scope != rec2->scope )
        return false; // scope does not match

    if( ( type == SYMREC_PORT )
            && ( rec1->attr_port->collection != rec2->attr_port->collection ) )
        return false; // port collections do not match

    // if we came through here, the record is identical
    return true;
}

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
attr_net_t* symrec_attr_create_net( virt_net_t* v_net )
{
    attr_net_t* new_attr = malloc( sizeof( attr_net_t ) );
    new_attr->v_net = v_net;
    return new_attr;
}

/******************************************************************************/
attr_port_t* symrec_attr_create_port( char* int_name, int mode, int collection,
        bool decoupled, int sync_id )
{
    attr_port_t* new_attr = malloc( sizeof( attr_port_t ) );
    new_attr->int_name = int_name;
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
attr_wrap_t* symrec_attr_create_wrap( bool attr_static, symrec_list_t* ports )
{
    attr_wrap_t* new_attr = malloc( sizeof( attr_wrap_t ) );
    new_attr->attr_static = attr_static;
    new_attr->ports = ports;
    return new_attr;
}

/******************************************************************************/
void symrec_attr_destroy_box( symrec_t* rec )
{
    free( rec->attr_box->impl_name );
    symrec_list_del( rec->attr_box->ports );
    free( rec->attr_box );
}

/******************************************************************************/
void symrec_attr_destroy_net( symrec_t* rec )
{
    virt_net_destroy( rec->attr_net->v_net );
    free( rec->attr_net );
}

/******************************************************************************/
void symrec_attr_destroy_port( symrec_t* rec ) 
{
    free( rec->attr_port->int_name );
    free( rec->attr_port );
}

/******************************************************************************/
void symrec_attr_destroy_proto( attr_prot_t* attr )
{
    symrec_list_del( attr->ports );
    free( attr );
}

/******************************************************************************/
void symrec_attr_destroy_wrap( symrec_t* rec )
{
    symrec_list_del( rec->attr_wrap->ports );
    free( rec->attr_wrap );
}

/******************************************************************************/
symrec_t* symrec_create( char* name, int scope, symrec_type_t type, int line )
{
    char key[ strlen( name ) + 1 + CONST_SCOPE_LEN ];
    symrec_t* new_item = NULL;

    // generate key
    sprintf( key, "%s%d", name, scope );
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
void symrec_del( symrec_t** symtab, symrec_t* rec )
{
    symrec_t* rec_temp;
    HASH_FIND_STR( *symtab, rec->key, rec_temp );
#if defined(DEBUG) || defined(DEBUG_SYMB)
    printf( "symrec_del: delete record %s in scope %d\n", rec->name,
            rec->scope );
#endif // DEBUG
    if( rec_temp == NULL ) {
#if defined(DEBUG) || defined(DEBUG_SYMB)
        printf( "symrec_del: No such record to delete\n" );
#endif // DEBUG
        return;
    }
    // delete name entry only if no other record with the same name exists
    if( rec_temp->next == NULL ) HASH_DELETE( hh, *symtab, rec );
    switch( rec->type ) {
        case SYMREC_BOX:
            symrec_attr_destroy_box( rec );
            break;
        case SYMREC_NET:
            symrec_attr_destroy_net( rec );
            break;
        case SYMREC_NET_PROTO:
            symrec_attr_destroy_proto( rec->attr_proto );
            break;
        case SYMREC_PORT:
            symrec_attr_destroy_port( rec );
            break;
        case SYMREC_WRAP:
            symrec_attr_destroy_wrap( rec );
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
        int line )
{
    symrec_t* item = NULL;
#ifndef TESTING
    char error_msg[ CONST_ERROR_LEN ];
#endif // TESTING

    /* check whether their scope matches with a scope on the stack */
    item = symrec_search( symtab, scope_stack, name );
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
    bool is_identical = false;
    symrec_t* item = NULL;
    symrec_t* previous_item = NULL;
    // check wheter key already exists
    HASH_FIND_STR( *symtab, new_item->key, item );
    // the key is new
    if( item == NULL ) {
        HASH_ADD_KEYPTR( hh, *symtab, new_item->key, strlen( new_item->key ),
                new_item );
        item = new_item;
    }
    /* hash exists */
    else {
        // iterate through all entries with the same hash to handle collisions
        do {
            // check whether there is an identical entry (name and scope)
            is_identical = is_symrec_identical( new_item, item,
                    new_item->type );
            if( is_identical ) break;

            previous_item = item;   // remember the last item of the list
            item = item->next;
        } while( item != NULL );

        if( !is_identical ) {
            // the item was differen -> add it to the end of the linked list
            previous_item->next = new_item;
            item = new_item;
        }
    }
    if( is_identical ) {
        // the item already existed in the table -> free the allocated space
        // and throw an yyerror
#ifdef TESTING
        printf( ERROR_DUPLICATE_ID, ERR_ERROR, new_item->name );
        printf( "\n" );
#else
        sprintf( error_msg, ERROR_DUPLICATE_ID, ERR_ERROR, new_item->name );
        report_yyerror( error_msg, new_item->line );
#endif // TESTING
        symrec_destroy( new_item );
        item = NULL;
    }
#if defined(DEBUG) || defined(DEBUG_SYMB)
    else {
        printf( "added symbol %s in scope %d\n", item->name, item->scope );
    }
#endif // DEBUG
    return item;
}

/******************************************************************************/
symrec_t* symrec_create_box( char* name, int scope, int line, attr_box_t* attr )
{
    symrec_t* item = symrec_create( name, scope, SYMREC_BOX, line );
    item->attr_box = attr;
    return item;
}

/******************************************************************************/
symrec_t* symrec_create_net( char* name, int scope, int line, attr_net_t* attr )
{
    symrec_t* item = symrec_create( name, scope, SYMREC_NET, line );
    item->attr_net = attr;
    return item;
}

/******************************************************************************/
symrec_t* symrec_create_port( char* name, int scope, int line,
        attr_port_t* attr )
{
    symrec_t* item = symrec_create( name, scope, SYMREC_PORT, line );
    item->attr_port = attr;
    return item;
}

/******************************************************************************/
symrec_t* symrec_create_proto( char* name, int scope, int line,
        attr_prot_t* attr )
{
    symrec_t* item = symrec_create( name, scope, SYMREC_NET_PROTO, line );
    item->attr_proto = attr;
    return item;
}

/******************************************************************************/
symrec_t* symrec_create_wrap( char* name, int scope, int line,
        attr_wrap_t* attr )
{
    symrec_t* item = symrec_create( name, scope, SYMREC_WRAP, line );
    item->attr_wrap = attr;
    return item;
}

/******************************************************************************/
symrec_t* symrec_search( symrec_t** symtab, UT_array* scope_stack, char *name )
{
    int* p = NULL;
    symrec_t* item = NULL;
    char key[ strlen( name ) + 1 + CONST_SCOPE_LEN ];
    while( ( p = ( int* )utarray_prev( scope_stack, p ) ) != NULL ) {
        // generate key
        sprintf( key, "%s%d", name, *p );
        HASH_FIND_STR( *symtab, key, item );
        // iterate through all entries with the same key to handle collisions
        while( item != NULL ) {
            if( strlen( item->name ) == strlen( name )
                && memcmp( item->name, name, strlen( name ) ) == 0
                && item->scope == *p ) {
                break; // found a match
            }
            item = item->next;
        }
        if( item != NULL ) break; // found a match
    }
    return item;
}
