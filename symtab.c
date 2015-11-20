#include <stdlib.h> /* For malloc in symbol table */
#include <stdio.h>
#include "symtab.h"
#include "utarray.h"
#include "defines.h"
#ifdef DOT_CON
#include "graph.h"
#endif // DOT_CON

/* handle errors with the bison error function */
extern void yyerror ( const char* );
char __error_msg[255];

/* global variables */
int         __scope = 0;    // global scope counter
int         __sync_id = 0;  // used to assemble ports to sync groups
UT_array*   __scope_stack;  // stack to handle the scope
#ifdef DOT_CON
FILE*       __n_con_graph;  // file handler for the net connection graph
FILE*       __p_con_graph;  // file handler for the port connection graph
extern int  __node_id;
#endif // DOT_CON


/******************************************************************************/
void context_check( ast_node* ast ) {
    instrec*    insttab = NULL;       // hash table to store the instances
    symrec*     symtab = NULL;        // hash table to store the symbols

#ifdef DOT_CON
    __n_con_graph = fopen( N_CON_DOT_PATH, "w" );
    __p_con_graph = fopen( P_CON_DOT_PATH, "w" );
#endif // DOT_CON

    __scope = 0;
    utarray_new( __scope_stack, &ut_int_icd );
    utarray_push_back( __scope_stack, &__scope );
    id_install( &symtab, ast, false );
    __scope = 0;
    id_check( &symtab, &insttab, ast );
#ifdef DOT_CON
    fclose( __n_con_graph );
    fclose( __p_con_graph );
#endif // DOT_CON
}

/******************************************************************************/
void connection_check( symrec** symtab, ast_node* ast ) {
    ast_list* i_ptr;
    ast_list* j_ptr;
    ast_node* op1;
    ast_node* op2;
    // match ports of left.con_right with right.con_left
    i_ptr = ast->op.left->op.con_right;
    do {
        if (ast->op.left->node_type == AST_ID) {
            // left operator is an ID
            op1 = ast->op.left;
            i_ptr = ( ast_list* )0;   // stop condition of outer loop
        }
        else {
            // left operator is a net (opeartion)
            op1 = i_ptr->ast_node;
            i_ptr = i_ptr->next;
        }
        j_ptr = ast->op.right->op.con_left;
        do {
            if (ast->op.right->node_type == AST_ID) {
                // right operator is an ID
                op2 = ast->op.right;
                j_ptr = ( ast_list* )0;
            }
            else {
                // right operator is a net (opeartion)
                op2 = j_ptr->ast_node;
                j_ptr = j_ptr->next;
            }
            /* printf( "'%s' conncets with '%s'\n", op1->ast_id.name, */
            /*         op2->ast_id.name ); */
#ifdef DOT_CON
            graph_add_edge( __n_con_graph, op1->id, op2->id );
#endif // DOT_CON
            connection_check_port( symtab, op1, op2 );
        }
        while (j_ptr != 0);
    }
    while (i_ptr != 0);
}

/******************************************************************************/
void connection_check_port( symrec** symtab, ast_node* ast_op1,
        ast_node* ast_op2 ) {
    symrec* op1 = symrec_get( symtab, ast_op1->ast_id.name,
            ast_op1->ast_id.line );
    symrec* op2 = symrec_get( symtab, ast_op2->ast_id.name,
            ast_op1->ast_id.line );
    symrec_list* ports1;
    symrec_list* ports2;
    // check whether ports can connect
    ports1 = ( ( struct box_attr* )( op1->attr ) )->ports;
    while ( ports1 != NULL ) {
        /* printf( " %s.%s\n", op1->name, ports1->rec->name ); */
        ports2 = ( ( struct box_attr* )( op2->attr ) )->ports;
        while (ports2 != NULL ) {
            /* printf( " %s.%s\n", op2->name, ports2->rec->name ); */
            // ports connect if
            // 1. they have the same name
            // 2. they are of opposite mode
            if( ( strcmp( ports1->rec->name, ports2->rec->name ) == 0 )
                && ( ( ( struct port_attr* )ports1->rec->attr )->mode !=
                    ( ( struct port_attr* )ports2->rec->attr )->mode ) ) {
                /* printf( " %s.%s connects with %s.%s\n", */
                /*         op1->name, ports1->rec->name, */
                /*         op2->name, ports2->rec->name ); */
#ifdef DOT_CON
                graph_add_edge( __p_con_graph, ast_op1->id, ast_op2->id );
#endif // DOT_CON
            }

            ports2 = ports2->next;
        }
        ports1 = ports1->next;
    }
}

/******************************************************************************/
void id_check( symrec** symtab, instrec** insttab, ast_node* ast ) {
    ast_list* list = NULL;
    symrec* rec = NULL;

    switch( ast->node_type ) {
        case AST_CONNECTS:
        case AST_STMTS:
            list = ast->ast_list;
            while( list != NULL ) {
                id_check( symtab, insttab, list->ast_node );
                list = list->next;
            }
            break;
        case AST_BOX:
            __scope++;
            break;
        case AST_WRAP:
            __scope++;
            utarray_push_back( __scope_stack, &__scope );
            id_check( symtab, insttab, ast->wrap.stmts );
            utarray_pop_back( __scope_stack );
            break;
        case AST_NET:
#ifdef DOT_CON
            graph_init( __n_con_graph, STYLE_CON_GRAPH );
            graph_init( __p_con_graph, STYLE_CON_GRAPH );
#endif // DOT_CON
            id_check( symtab, insttab, ast->ast_node );
#ifdef DOT_CON
            graph_finish( __n_con_graph );
            graph_finish( __p_con_graph );
#endif // DOT_CON
            break;
        case AST_CONNECT:
            id_check( symtab, insttab, ast->connect.connects );
            break;
        case AST_PARALLEL:
            id_check( symtab, insttab, ast->op.left );
            id_check( symtab, insttab, ast->op.right );
            break;
        case AST_SERIAL:
            id_check( symtab, insttab, ast->op.left );
            id_check( symtab, insttab, ast->op.right );
            connection_check( symtab, ast );
            break;
        case AST_ID:
            // check the context of the symbol
            rec = symrec_get( symtab, ast->ast_id.name, ast->ast_id.line );
            // add the symbol to the instance table
            if( ast->ast_id.type == ID_NET ) {
                instrec_put( insttab, ast->id, rec );
#ifdef DOT_CON
                graph_add_node( __n_con_graph, ast->id, ast->ast_id.name,
                        SHAPE_BOX );
                graph_add_node( __p_con_graph, ast->id, ast->ast_id.name,
                        SHAPE_BOX );
#endif // DOT_CON
            }
            break;
        default:
            ;
    }
}

/******************************************************************************/
void* id_install( symrec** symtab, ast_node* ast, bool is_sync ) {
    ast_list* list = NULL;
    box_attr* b_attr = NULL;
    port_attr* p_attr = NULL;
    bool is_sync_temp = false;
    void* res = NULL;
    symrec* port = NULL;
    symrec_list* ptr = NULL;
    symrec_list* port_list = NULL;

    switch( ast->node_type ) {
        case AST_SYNC:
            __sync_id++;
            is_sync_temp = true;
        case AST_PORTS:
            list = ast->ast_list;
            while (list != NULL) {
                port = ( struct symrec* )id_install( symtab, list->ast_node,
                        is_sync_temp );
                list = list->next;
                ptr = (symrec_list*) malloc(sizeof(symrec_list));
                ptr->rec = port;
                ptr->next = port_list;
                port_list = ptr;
            }
            res = ( void* )port_list;   // return pointer to the port list
            break;
        case AST_STMTS:
            list = ast->ast_list;
            while (list != NULL) {
                id_install( symtab, list->ast_node, is_sync_temp );
                list = list->next;
            }
            break;
        case AST_BOX:
            __scope++;
            utarray_push_back( __scope_stack, &__scope );
            port_list = ( struct symrec_list* )id_install( symtab,
                    ast->box.ports, false );
            utarray_pop_back( __scope_stack );
            // prepare symbol attributes and install symbol
            b_attr = ( box_attr* )malloc( sizeof( box_attr ) );
            b_attr->state = ( ast->box.state == NULL ) ? true : false;
            b_attr->ports = port_list;
            symrec_put( symtab, ast->box.id->ast_id.name,
                    *utarray_back( __scope_stack ), VAL_BOX, ( void* )b_attr,
                    ast->box.id->ast_id.line );
            break;
        case AST_WRAP:
            __scope++;
            utarray_push_back( __scope_stack, &__scope );
            id_install( symtab, ast->wrap.stmts, false );
            port_list = ( struct symrec_list* )id_install( symtab,
                    ast->wrap.ports, false );
            utarray_pop_back( __scope_stack );
            // prepare symbol attributes and install symbol
            b_attr = ( box_attr* )malloc( sizeof( box_attr ) );
            b_attr->state = false;
            b_attr->ports = port_list;
            symrec_put( symtab, ast->wrap.id->ast_id.name,
                    *utarray_back( __scope_stack ), VAL_NET, ( void* )b_attr,
                    ast->wrap.id->ast_id.line );
            break;
        case AST_PORT:
            // prepare symbol attributes
            if( ast->port.collection != NULL )
                list = ast->port.collection->ast_list;
            p_attr = ( port_attr* )malloc( sizeof( port_attr ) );
            p_attr->mode = ast->port.mode->ast_node->ast_attr.val;
            p_attr->up = false;
            p_attr->down = false;
            p_attr->side = false;
            while( list != NULL ) { // set port collections
                if( list->ast_node->ast_attr.val == VAL_UP )
                    p_attr->up = true;
                else if( list->ast_node->ast_attr.val == VAL_DOWN )
                    p_attr->down = true;
                else if( list->ast_node->ast_attr.val == VAL_SIDE )
                    p_attr->side = true;
                list = list->next;
            }
            if( is_sync ) { // add sync attributes if port is a sync port
                p_attr->decoupled = (ast->port.coupling == NULL) ? false : true;
                p_attr->sync_id = __sync_id;
            }
            // install symbol and return pointer to the symbol record
            res = ( void* )symrec_put( symtab,
                    ast->port.id->ast_id.name,
                    *utarray_back( __scope_stack ), VAL_PORT, p_attr,
                    ast->port.id->ast_id.line );

            break;
        default:
            ;
    }
    return res;
}

/******************************************************************************/
instrec* instrec_get( instrec** insttab, int id ) {
    // no collision handling is needed, IDs are unique
    instrec* item;
    HASH_FIND_INT( *insttab, &id, item );
    return item;
}

/******************************************************************************/
instrec* instrec_put( instrec** insttab, int id, symrec* rec ) {
    // no collision handling is needed, IDs are unique
    instrec* item;
    item = ( instrec* )malloc( sizeof( instrec ) );
    item->id = id;
    item->rec = rec;
    HASH_ADD_INT( *insttab, id, item );
    return item;
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
        while( ( p = ( int* )utarray_prev( __scope_stack, p ) ) != NULL ) {
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
        sprintf( __error_msg, ERROR_UNDEFINED_ID, line, name );
        yyerror( __error_msg );
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
        sprintf( __error_msg, ERROR_DUPLICATE_ID, line, name );
        yyerror( __error_msg );
    }
    return res;
}
