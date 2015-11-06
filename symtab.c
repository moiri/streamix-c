#include <stdlib.h> /* For malloc in symbol table */
#include <string.h> /* For strcmp in symbol table */
#include <stdio.h>
#include "symtab.h"
#include "defines.h"

int scope = 0;
int sync_id = 0;
UT_array* scope_stack;
symrec* symtab = NULL;
extern void yyerror ( const char* );
char error_msg[255];

/******************************************************************************/
void context_check( ast_node* ast ) {
    /* int* p = NULL; */
    scope = 0;
    utarray_new( scope_stack, &ut_int_icd );
    utarray_push_back( scope_stack, &scope );
    id_install( &symtab, ast, false );
    /* printf("stack:"); */
    /* while( ( p = ( int* )utarray_prev( scope_stack, p ) ) != NULL ) { */
    /*     printf(" %d,", *p); */
    /* } */
    /* printf("\n"); */
    scope = 0;
    id_check( &symtab, ast );
}

void connection_check( symrec** symtab, ast_node* ast ) {
    int node_id, tmp_node_id;
    ast_list* list = NULL;
    ast_list* i_ptr;
    ast_list* j_ptr;

    switch( ast->node_type ) {
        case AST_STMTS:
            list = ast->ast_list;
            while( list != NULL ) {
                connection_check( symtab, list->ast_node );
                list = list->next;
            }
            break;
        case AST_NET:
            connection_check( symtab, ast->ast_node );
            break;
        case AST_PARALLEL:
            connection_check( symtab, ast->op.left );
            connection_check( symtab, ast->op.right );
            break;
        case AST_SERIAL:
            connection_check( symtab, ast->op.left );
            connection_check( symtab, ast->op.right );
            // match ports of left.con_right with right.con_left
            i_ptr = ast->op.left->op.con_right;
            do {
                if (ast->op.left->node_type == AST_ID) {
                    node_id = ast->op.left->id;
                    i_ptr = (ast_list*)0;
                }
                else {
                    node_id = i_ptr->ast_node->id;
                    i_ptr = i_ptr->next;
                }
                j_ptr = ast->op.right->op.con_left;
                do {
                    if (ast->op.right->node_type == AST_ID) {
                        tmp_node_id = ast->op.right->id;
                        j_ptr = (ast_list*)0;
                    }
                    else {
                        tmp_node_id = j_ptr->ast_node->id;
                        j_ptr = j_ptr->next;
                    }
                    /* graph_add_edge(graph, node_id, tmp_node_id); */
                }
                while (j_ptr != 0);
            }
            while (i_ptr != 0);
            break;
        default:
            ;
    }
}

/******************************************************************************/
void id_check( symrec** symtab, ast_node* ast ) {
    ast_list* list = NULL;

    switch( ast->node_type ) {
        case AST_STMTS:
        case AST_CONNECTS:
            list = ast->ast_list;
            while( list != NULL ) {
                id_check( symtab, list->ast_node );
                list = list->next;
            }
            break;
        case AST_BOX:
            scope++;
            break;
        case AST_WRAP:
            scope++;
            utarray_push_back( scope_stack, &scope );
            id_check( symtab, ast->wrap.stmts );
            utarray_pop_back( scope_stack );
            break;
        case AST_NET:
            id_check( symtab, ast->ast_node );
            break;
        case AST_CONNECT:
            id_check( symtab, ast->connect.connects );
            break;
        case AST_SERIAL:
        case AST_PARALLEL:
            id_check( symtab, ast->op.left );
            id_check( symtab, ast->op.right );
            break;
        case AST_ID:
            symrec_get( symtab, ast->ast_id.name, ast->ast_id.line );
            break;
        default:
            ;
    }
}

/******************************************************************************/
void id_install( symrec** symtab, ast_node* ast, bool is_sync ) {
    ast_list* list = NULL;
    box_attr* b_attr = NULL;
    port_attr* p_attr = NULL;
    bool is_sync_temp = false;

    switch( ast->node_type ) {
        case AST_SYNC:
            sync_id++;
            is_sync_temp = true;
        case AST_PORTS:
        case AST_STMTS:
            list = ast->ast_list;
            while (list != NULL) {
                id_install( symtab, list->ast_node, is_sync_temp );
                list = list->next;
            }
            break;
        case AST_BOX:
            scope++;
            utarray_push_back( scope_stack, &scope );
            id_install( symtab, ast->box.ports, false );
            utarray_pop_back( scope_stack );
            b_attr = ( box_attr* )malloc( sizeof( box_attr ) );
            b_attr->state = ( ast->box.state == NULL ) ? true : false;
            /* b_attr->ports = head; */
            symrec_put( symtab, ast->box.id->ast_id.name,
                    *utarray_back( scope_stack ), VAL_BOX, ( void* )b_attr,
                    ast->box.id->ast_id.line );
            break;
        case AST_WRAP:
            symrec_put( symtab, ast->wrap.id->ast_id.name,
                    *utarray_back( scope_stack ), VAL_NET, NULL,
                    ast->wrap.id->ast_id.line );
            scope++;
            utarray_push_back( scope_stack, &scope );
            id_install( symtab, ast->wrap.ports, false );
            id_install( symtab, ast->wrap.stmts, false );
            utarray_pop_back( scope_stack );
            break;
        case AST_PORT:
            if( ast->port.collection != NULL )
                list = ast->port.collection->ast_list;
            p_attr = ( port_attr* )malloc( sizeof( port_attr ) );
            p_attr->mode = ast->port.mode->ast_node->ast_attr.val;
            p_attr->up = false;
            p_attr->down = false;
            p_attr->side = false;
            while( list != NULL ) {
                if( list->ast_node->ast_attr.val == VAL_UP )
                    p_attr->up = true;
                else if( list->ast_node->ast_attr.val == VAL_DOWN )
                    p_attr->down = true;
                else if( list->ast_node->ast_attr.val == VAL_SIDE )
                    p_attr->side = true;
                list = list->next;
            }
            if( is_sync ) {
                p_attr->decoupled = (ast->port.coupling == NULL) ? false : true;
                p_attr->sync_id = sync_id;
            }
            symrec_put( symtab, ast->port.id->ast_id.name,
                    *utarray_back( scope_stack ), VAL_PORT, p_attr,
                    ast->port.id->ast_id.line );

            break;
        default:
            ;
    }
}

/******************************************************************************/
symrec* symrec_get( symrec** symtab, char *name, int line ) {
    symrec* item;
    int* p = NULL;
    bool in_scope = false;
    HASH_FIND_STR( *symtab, name, item );
    /* iterate through all entries with the same name */
    while( !in_scope && item != NULL ) {
        /* check whether their scope matches with a scope on the stack */
        while( ( p = ( int* )utarray_prev( scope_stack, p ) ) != NULL ) {
            if( *p == item->scope ) {
                in_scope = true;
                /* printf( "found" ); */
                /* if( item->attr != NULL ) { */
                /*     if( ( ( struct box_attr* )item->attr )->state ) */
                /*         printf( " stateless" ); */
                /*     printf( " box" ); */
                /* } */
                /* else if( item->type == VAL_NET ) */
                /*     printf( " net" ); */
                /* printf( " %s in scope %d\n", item->name, item->scope ); */
                return item;
            }
        }
        item = item->next;
    }
    if( item == NULL ) {
        sprintf( error_msg, ERROR_UNDEFINED_ID, line, name );
        yyerror( error_msg );
    }
    return item;
}

/******************************************************************************/
symrec* symrec_put( symrec** symtab, char *name, int scope, int type, void* attr,
        int line ) {
    symrec* item;
    symrec* previous_item;
    symrec* new_item;
    symrec* res = NULL;
    bool is_identical = false;
    /* printf( "id_install" ); */
    /* if( type == VAL_BOX ) { */
    /*     if( ( ( struct box_attr* )attr )->state ) */
    /*         printf( " stateless" ); */
    /*     printf( " box" ); */
    /* } */
    /* else if( type == VAL_PORT || type == VAL_SPORT ) { */
    /*     if( ( ( struct port_attr* )attr )->up ) */
    /*         printf( " up" ); */
    /*     if( ( ( struct port_attr* )attr )->down ) */
    /*         printf( " down" ); */
    /*     if( ( ( struct port_attr* )attr )->side ) */
    /*         printf( " side" ); */
    /*     if( ( ( struct port_attr* )attr )->mode == VAL_IN ) */
    /*         printf( " in" ); */
    /*     else if( ( ( struct port_attr* )attr )->mode == VAL_OUT ) */
    /*         printf( " out" ); */
    /*     if( type == VAL_PORT ) printf( " port" ); */
    /*     /1* else if( type == VAL_SPORT ) { *1/ */
    /*     /1*     if( ( ( struct sport_attr* )attr )->decoupled ) *1/ */
    /*     /1*         printf( " decoupled" ); *1/ */
    /*     /1*     printf( " sync(%d) port", ( ( struct sport_attr* )attr )->sync_id ); *1/ */
    /*     /1* } *1/ */
    /* } */
    /* else if( type == VAL_NET ) */
    /*     printf( " net" ); */
    /* printf( " %s in scope %d\n", name, scope ); */
    HASH_FIND_STR( *symtab, name, item );
    previous_item = item;
    /* new key */
    if( item == NULL ) {
        item = ( symrec* )malloc( sizeof( symrec ) );
        item->scope = scope;
        item->type = type;
        item->name = ( char* )malloc( strlen( name ) + 1 );
        strcpy (item->name, name);
        item->attr = attr;
        HASH_ADD_KEYPTR( hh, *symtab, item->name, strlen(item->name), item );
        res = item;
    }
    /* hash exists */
    else {
        /* iterate through all entries with the same hash */
        do {
            /* name, scope, mode, or collections are the same -> error */
            if( strlen( item->name ) == strlen( name )
                && memcmp( item->name, name, strlen( name ) ) == 0
                && item->scope == scope
                && ( ( item->type == VAL_BOX || item->type == VAL_NET )
                    || ( ( type == VAL_PORT || type == VAL_SPORT )
                        && ( ( ( struct port_attr* )attr )->mode
                                == ( ( struct port_attr* )item->attr )->mode
                            && ( ( struct port_attr* )attr )->up
                                == ( ( struct port_attr* )item->attr )->up
                            && ( ( struct port_attr* )attr )->down
                                == ( ( struct port_attr* )item->attr )->down
                            && ( ( struct port_attr* )attr )->side
                                == ( ( struct port_attr* )item->attr )->side ) )
                    ) ) {
                is_identical = true;
                break;
            }
            previous_item = item;
            item = item->next;
        } while( item != NULL );

        if( !is_identical ) {
            new_item = ( symrec* )malloc( sizeof( symrec ) );
            new_item->scope = scope;
            new_item->type = type;
            new_item->name = ( char* )malloc( strlen( name ) + 1 );
            strcpy( new_item->name, name );
            new_item->attr = attr;
            previous_item->next = new_item;
            res = new_item;
        }
    }
    if( is_identical ) {
        sprintf( error_msg, ERROR_DUPLICATE_ID, line, name );
        yyerror( error_msg );
    }
    return res;
}
