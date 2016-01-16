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
    graph_fix_dot( TEMP_DOT_PATH, N_CON_DOT_PATH );
    graph_fix_dot( TEMP_DOT_PATH, P_CON_DOT_PATH );
#endif // DOT_CON
}

/******************************************************************************/
void connection_check( instrec** insttab, ast_node* ast ) {
    ast_list* i_ptr;
    ast_list* j_ptr;
    ast_node* op_left;
    ast_node* op_right;
    // match ports of left.con_right with right.con_left
    i_ptr = ast->op.left->op.con_right;
    do {
        if (ast->op.left->node_type == AST_ID) {
            // left operator is an ID
            op_left = ast->op.left;
            i_ptr = ( ast_list* )0;   // stop condition of outer loop
        }
        else {
            // left operator is a net (opeartion)
            op_left = i_ptr->ast_node;
            i_ptr = i_ptr->next;
        }
        j_ptr = ast->op.right->op.con_left;
        do {
            if (ast->op.right->node_type == AST_ID) {
                // right operator is an ID
                op_right = ast->op.right;
                j_ptr = ( ast_list* )0;
            }
            else {
                // right operator is a net (opeartion)
                op_right = j_ptr->ast_node;
                j_ptr = j_ptr->next;
            }
            /* printf( "'%s' conncets with '%s'\n", op_left->ast_id.name, */
            /*         op_right->ast_id.name ); */
#ifdef DOT_CON
            graph_add_edge( __n_con_graph, op_left->id, op_right->id, NULL,
                    false );
#endif // DOT_CON
            connection_check_port( insttab, op_left->id, op_right->id, false );
        }
        while (j_ptr != 0);
    }
    while (i_ptr != 0);
}

/******************************************************************************/
void connection_check_port( instrec** insttab, int id_left, int id_right,
        bool side ) {
#ifdef DOT_CON
    int id_node_start, id_node_end;
#endif // DOT_CON
    instrec* op_left = instrec_get( insttab, id_left );
    if ( op_left == NULL ) return;
    /* printf("get instance %s(%d)\n", op_left->net->name, op_left->id); */
    instrec* op_right = instrec_get( insttab, id_right );
    if ( op_right == NULL ) return;
    /* printf("get instance %s(%d)\n", op_right->net->name, op_right->id); */
    symrec_list* ports_left;
    symrec_list* ports_right;
    port_attr* p_attr_left;
    port_attr* p_attr_right;
    // check whether ports can connect
    ports_left = op_left->ports;
    while ( ports_left != NULL ) {
        /* printf( " %s.%s\n", op_left->net->name, ports_left->rec->name ); */
        p_attr_left = ( struct port_attr* )ports_left->rec->attr;
        ports_right = op_right->ports;
        while (ports_right != NULL ) {
            /* printf( " %s.%s\n", op_right->net->name, ports_right->rec->name ); */
            p_attr_right = ( struct port_attr* )ports_right->rec->attr;
            if( // ports are not yet connected
                    !ports_left->is_connected && !ports_right->is_connected
                // ports have the same name
                    && ( strcmp( ports_left->rec->name,
                            ports_right->rec->name ) == 0 )
                // ports are of opposite mode
                    && ( p_attr_left->mode != p_attr_right->mode )
                // the left port is in DS and the right is in US
                    && ( ( ( ( ( p_attr_left->collection == VAL_DOWN )
                                && ( p_attr_right->collection == VAL_UP ) )
                    // or they are both in no collection
                            || ( ( p_attr_left->collection == VAL_NONE )
                                && ( p_attr_right->collection == VAL_NONE ) ) )
                    // and we are not considering side ports
                        && !side )
                    // or we are considering side ports and both potrs are SP
                        || ( side && ( p_attr_left->collection == VAL_SIDE
                                && p_attr_right->collection == VAL_SIDE ) ) )
            ) {
                /* printf( " %s.%s connects with %s.%s\n", */
                /*         op_left->net->name, ports_left->rec->name, */
                /*         op_right->net->name, ports_right->rec->name ); */
                if( side ) printf( "side\n" );
                ports_left->is_connected = true;
                ports_right->is_connected = true;
#ifdef DOT_CON
                id_node_start = id_left;
                id_node_end = id_right;
                if( p_attr_left->mode == VAL_IN ) {
                    id_node_start = id_right;
                    id_node_end = id_left;
                }
                graph_add_edge( __p_con_graph, id_node_start, id_node_end,
                        ports_left->rec->name, side );
#endif // DOT_CON
            }
            ports_right = ports_right->next;
        }
        ports_left = ports_left->next;
    }
}

/******************************************************************************/
void connection_check_sport( instrec** insttab, ast_node* ast,
        int connect_cnt ) {
    ast_list* list = ast->connect.connects->ast_list;
    int id1, id2;
    if( connect_cnt <= 1 ) {
        // ERROR: too few connecting nets
        sprintf( __error_msg, ERROR_UNDEFINED_ID, ast->connect.id->ast_id.line,
                ast->connect.id->ast_id.name );
        yyerror( __error_msg );
        return;
    }
    else if( connect_cnt == 2 ) {
        // two nets to connect by a sideport
        // this is the wrong id! I need to get the id of the instances in
        // this scope! This could be more than the count indicates...
        id1 = list->ast_node->id;
        list = list->next;
        id2 = list->ast_node->id;
        // net1 and net2 are to be connected by side ports
        connection_check_port( insttab, id1, id2, true );
    }
    else {
        // multiple nets to connect by a sideport with the use of a
        // copy synchronizer
        while( list != NULL ) {
            instrec* net = instrec_get( insttab, list->ast_node->id );
            if ( net == NULL ) continue;
            while( net->ports != NULL ) {

            }

            list = list->next;
        }
    }
}

/******************************************************************************/
void* id_check( symrec** symtab, instrec** insttab, ast_node* ast ) {
    ast_list* list = NULL;
    symrec* rec = NULL;
    int connect_cnt = 0;
    void* res = NULL;

    switch( ast->node_type ) {
        case AST_CONNECT:
            connect_cnt = *( int* )id_check( symtab, insttab,
                    ast->connect.connects );
            /* printf( "connection_cnt:%d\n", connect_cnt ); */
            /* if( connect_cnt > 2 ) { */
            /*     // create a copy synchronizer */
            /*     rec = ( symrec* )malloc( sizeof( symrec ) ); */
            /*     rec->scope = *utarray_back( __scope_stack ); */
            /*     rec->type = VAL_COPY; */
            /*     rec->name = NULL; */
            /*     rec->attr = NULL; */
            /*     instrec_put( insttab, ast->id, rec ); */
            /* } */
            connection_check_sport( insttab, ast, connect_cnt );
            break;
        case AST_CONNECTS:
            list = ast->ast_list;
            while( list != NULL ) {
                id_check( symtab, insttab, list->ast_node );
                connect_cnt++;
                list = list->next;
            }
            res = (void*)&connect_cnt;
            break;
        case AST_STMTS:
#ifdef DOT_CON
            graph_add_divider ( __n_con_graph, *utarray_back( __scope_stack ),
                    FLAG_STMTS );
            graph_add_divider ( __p_con_graph, *utarray_back( __scope_stack ),
                    FLAG_STMTS );
            graph_init( __n_con_graph, STYLE_N_CON_GRAPH );
            graph_init( __p_con_graph, STYLE_P_CON_GRAPH );
#endif // DOT_CON
            list = ast->ast_list;
            while( list != NULL ) {
                id_check( symtab, insttab, list->ast_node );
                list = list->next;
            }
#ifdef DOT_CON
            graph_add_divider ( __n_con_graph, *utarray_back( __scope_stack ),
                    FLAG_STMTS_END );
            graph_add_divider ( __p_con_graph, *utarray_back( __scope_stack ),
                    FLAG_STMTS_END );
            graph_finish( __n_con_graph );
            graph_finish( __p_con_graph );
#endif // DOT_CON
            break;
        case AST_BOX:
            __scope++;
            break;
        case AST_WRAP:
            __scope++;
            utarray_push_back( __scope_stack, &__scope );
#ifdef DOT_CON
            graph_add_divider ( __n_con_graph, *utarray_back( __scope_stack ),
                    FLAG_WRAP );
            graph_add_divider ( __p_con_graph, *utarray_back( __scope_stack ),
                    FLAG_WRAP );
            graph_init_subgraph( __n_con_graph, ast->wrap.id->ast_id.name,
                    STYLE_WRAPPER );
            graph_init_subgraph( __p_con_graph, ast->wrap.id->ast_id.name,
                    STYLE_WRAPPER );
#endif // DOT_CON
            id_check( symtab, insttab, ast->wrap.stmts );
#ifdef DOT_CON
            graph_add_divider ( __n_con_graph, *utarray_back( __scope_stack ),
                    FLAG_WRAP_END );
            graph_add_divider ( __p_con_graph, *utarray_back( __scope_stack ),
                    FLAG_WRAP_END );
            graph_finish_subgraph( __n_con_graph );
            graph_finish_subgraph( __p_con_graph );
#endif // DOT_CON
            utarray_pop_back( __scope_stack );
            break;
        case AST_NET:
            id_check( symtab, insttab, ast->ast_node );
            break;
        case AST_PARALLEL:
#if defined(DOT_CON) && defined(DOT_STRUCT)
            graph_add_divider ( __n_con_graph, *utarray_back( __scope_stack ),
                    FLAG_NET );
            graph_add_divider ( __p_con_graph, *utarray_back( __scope_stack ),
                    FLAG_NET );
            graph_init_subgraph( __n_con_graph, "", STYLE_PARALLEL );
            graph_init_subgraph( __p_con_graph, "", STYLE_PARALLEL );
#endif // DOT_CON && DOT_STRUCT
            id_check( symtab, insttab, ast->op.left );
            id_check( symtab, insttab, ast->op.right );
#if defined(DOT_CON) && defined(DOT_STRUCT)
            graph_add_divider ( __n_con_graph, *utarray_back( __scope_stack ),
                    FLAG_NET );
            graph_add_divider ( __p_con_graph, *utarray_back( __scope_stack ),
                    FLAG_NET );
            graph_finish_subgraph( __n_con_graph );
            graph_finish_subgraph( __p_con_graph );
#endif // DOT_CON && DOT_STRUCT
            break;
        case AST_SERIAL:
#if defined(DOT_CON) && defined(DOT_STRUCT)
            graph_add_divider ( __n_con_graph, *utarray_back( __scope_stack ),
                    FLAG_NET );
            graph_add_divider ( __p_con_graph, *utarray_back( __scope_stack ),
                    FLAG_NET );
            graph_init_subgraph( __n_con_graph, "", STYLE_SERIAL );
            graph_init_subgraph( __p_con_graph, "", STYLE_SERIAL );
#endif // DOT_CON && DOT_STRUCT
            id_check( symtab, insttab, ast->op.left );
            id_check( symtab, insttab, ast->op.right );
#if defined(DOT_CON) && defined(DOT_STRUCT)
            graph_add_divider ( __n_con_graph, *utarray_back( __scope_stack ),
                    FLAG_NET );
            graph_add_divider ( __p_con_graph, *utarray_back( __scope_stack ),
                    FLAG_NET );
            graph_finish_subgraph( __n_con_graph );
            graph_finish_subgraph( __p_con_graph );
#endif // DOT_CON && DOT_STRUCT
            connection_check( insttab, ast );
            break;
        case AST_ID:
            // check the context of the symbol
            rec = symrec_get( symtab, ast->ast_id.name, ast->ast_id.line );
            // add a net symbol to the instance table
            if( ast->ast_id.type == ID_NET && rec != NULL ) {
                /* printf( "put instance %s(%d)\n", ast->ast_id.name, ast->id ); */
                instrec_put( insttab, ast->id, rec );
#ifdef DOT_CON
                graph_add_divider ( __n_con_graph,
                        *utarray_back( __scope_stack ), FLAG_NET );
                graph_add_divider ( __p_con_graph,
                        *utarray_back( __scope_stack ), FLAG_NET );
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
    return res;
}

/******************************************************************************/
void* id_install( symrec** symtab, ast_node* ast, bool is_sync ) {
    ast_list* list = NULL;
    net_attr* b_attr = NULL;
    port_attr* p_attr = NULL;
    void* res = NULL;
    symrec_list* ptr = NULL;
    symrec_list* port_list = NULL;
    bool set_sync = false;

    switch( ast->node_type ) {
        case AST_SYNC:
            __sync_id++;
            set_sync = true;
        case AST_PORTS:
            list = ast->ast_list;
            while (list != NULL) {
                res = id_install( symtab, list->ast_node, set_sync );
                ptr = ( struct symrec_list* )res;
                while( ( ( struct symrec_list* ) res)->next != NULL )
                    res = ( ( struct symrec_list* )res )->next;
                ( ( struct symrec_list* )res )->next = port_list;
                port_list = ptr;
                list = list->next;
            }
            res = ( void* )ptr;   // return pointer to the port list
            break;
        case AST_STMTS:
            list = ast->ast_list;
            while (list != NULL) {
                id_install( symtab, list->ast_node, set_sync );
                list = list->next;
            }
            break;
        case AST_BOX:
            __scope++;
            utarray_push_back( __scope_stack, &__scope );
            port_list = ( struct symrec_list* )id_install( symtab,
                    ast->box.ports, set_sync );
            utarray_pop_back( __scope_stack );
            // prepare symbol attributes and install symbol
            b_attr = ( net_attr* )malloc( sizeof( net_attr ) );
            b_attr->state = ( ast->box.state == NULL ) ? true : false;
            b_attr->ports = port_list;
            symrec_put( symtab, ast->box.id->ast_id.name,
                    *utarray_back( __scope_stack ), ast->box.id->ast_id.type,
                    ( void* )b_attr, ast->box.id->ast_id.line );
            break;
        case AST_WRAP:
            __scope++;
            utarray_push_back( __scope_stack, &__scope );
            id_install( symtab, ast->wrap.stmts, set_sync );
            port_list = ( struct symrec_list* )id_install( symtab,
                    ast->wrap.ports, set_sync );
            utarray_pop_back( __scope_stack );
            // prepare symbol attributes and install symbol
            b_attr = ( net_attr* )malloc( sizeof( net_attr ) );
            b_attr->state = false;
            b_attr->ports = port_list;
            symrec_put( symtab, ast->wrap.id->ast_id.name,
                    *utarray_back( __scope_stack ), ast->wrap.id->ast_id.type,
                    ( void* )b_attr, ast->wrap.id->ast_id.line );
            break;
        case AST_PORT:
            // prepare symbol attributes
            if( ast->port.collection != NULL )
                list = ast->port.collection->ast_list;
            // insert one port for each collection attribute but only one if no
            // collection is set.
            do {
                p_attr = ( port_attr* )malloc( sizeof( port_attr ) );
                p_attr->mode = ast->port.mode->ast_node->ast_attr.val;
                // add sync attributes if port is a sync port
                if( is_sync ) {
                    p_attr->decoupled =
                        (ast->port.coupling == NULL) ? false : true;
                    p_attr->sync_id = __sync_id;
                }
                // set collection
                if ( list == NULL )
                    p_attr->collection = VAL_NONE;
                else {
                    p_attr->collection = list->ast_node->ast_attr.val;
                    list = list->next;
                }
                // install symbol and return pointer to the symbol record
                res = ( void* )symrec_put( symtab,
                        ast->port.id->ast_id.name,
                        *utarray_back( __scope_stack ),
                        ast->port.id->ast_id.type, p_attr,
                        ast->port.id->ast_id.line );
                ptr = ( symrec_list* )malloc( sizeof( symrec_list ) );
                ptr->rec = ( struct symrec* )res;
                ptr->next = port_list;
                port_list = ptr;
            }
            while( list != NULL );
            res = ( void* )ptr;   // return pointer to the port list
            break;
        default:
            ;
    }
    port_list = NULL;
    ptr = NULL;
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
    symrec_list* inst_ports;
    symrec_list* sym_ports;
    symrec_list* port_list = NULL;
    item = ( instrec* )malloc( sizeof( instrec ) );
    item->id = id;
    item->net = rec;
    // copy portlist from symtab to insttab
    sym_ports = ( ( struct net_attr* )rec->attr )->ports;
    while( sym_ports != NULL  ) {
        inst_ports = ( struct symrec_list* )malloc( sizeof( symrec_list ) );
        inst_ports->rec = sym_ports->rec;
        inst_ports->next = port_list;
        port_list = inst_ports;
        sym_ports = sym_ports->next;
    }
    item->ports = port_list;
    HASH_ADD_INT( *insttab, id, item );
    return item;
}

/******************************************************************************/
symrec* symrec_get( symrec** symtab, char *name, int line ) {
    int* p = NULL;
    symrec* item = NULL;
    char key[ strlen( name ) + 5 ];

    /* check whether their scope matches with a scope on the stack */
    while( ( p = ( int* )utarray_prev( __scope_stack, p ) ) != NULL ) {
        // generate key
        sprintf( key, "%s%d", name, *p );
        HASH_FIND_STR( *symtab, key, item );
        // iterate through all entries with the same key to handle collisions
        while( item != NULL ) {
            if( strlen( item->name ) == strlen( name )
                && memcmp( item->name, name, strlen( name ) ) == 0
                && item->scope == *p ) {
                /* printf( "found" ); */
                /* if( item->attr != NULL ) { */
                /*     if( ( ( struct net_attr* )item->attr )->state ) */
                /*         printf( " stateless" ); */
                /*     printf( " box" ); */
                /* } */
                /* else if( item->type == VAL_NET ) */
                /*     printf( " net" ); */
                /* printf( " %s in scope %d\n", item->name, item->scope ); */
                break; // found a match
            }
            item = item->next;
        }
        if( item != NULL ) break; // found a match
    }
    if( item == NULL ) {
        sprintf( __error_msg, ERROR_UNDEFINED_ID, line, name );
        yyerror( __error_msg );
    }
    return item;
}

/******************************************************************************/
symrec* symrec_put( symrec** symtab, char *name, int scope, int type,
        void* attr, int line ) {
    symrec* item = NULL;
    symrec* new_item = NULL;
    symrec* previous_item = NULL;
    symrec* res = NULL;
    bool is_identical = false;
    char key[ strlen( name ) + 5 ];

    // generate key
    sprintf( key, "%s%d", name, scope );
    // create new iten structure
    new_item = ( symrec* )malloc( sizeof( symrec ) );
    new_item->scope = scope;
    new_item->type = type;
    new_item->key = ( char* )malloc( strlen( key ) + 1 );
    strcpy( new_item->key, key );
    new_item->attr = attr;
    new_item->name = ( char* )malloc( strlen( name ) + 1 );
    strcpy( new_item->name, name );
    new_item->attr = attr;
    // check wheter key already exists
    HASH_FIND_STR( *symtab, key, item );
    // the key is new
    if( item == NULL ) {
        HASH_ADD_KEYPTR( hh, *symtab, new_item->key, strlen( key ), new_item );
        res = new_item;
    }
    /* hash exists */
    else {
        // iterate through all entries with the same hash to handle collisions
        // and catch ports with the same name and scope but a with different
        // collections
        do {
            /* name, scope, mode, or collections are the same -> error */
            if( strlen( item->name ) == strlen( name )
                && memcmp( item->name, name, strlen( name ) ) == 0
                && item->scope == scope
                && ( ( item->type == VAL_BOX || item->type == VAL_NET )
                    || ( ( type == VAL_PORT || type == VAL_SPORT )
                        && ( ( ( struct port_attr* )attr )->mode
                            == ( ( struct port_attr* )item->attr )->mode
                        && ( ( struct port_attr* )attr )->collection
                            == ( ( struct port_attr* )item->attr )->collection )
                       )
                    ) ) {
                is_identical = true;
                break;
            }
            previous_item = item;
            item = item->next;
        }
        while( item != NULL );

        if( !is_identical ) {
            previous_item->next = new_item;
            res = new_item;
        }
    }
    if( is_identical ) {
        free( new_item->name );
        free( new_item->key );
        free( new_item );
        sprintf( __error_msg, ERROR_DUPLICATE_ID, line, name );
        yyerror( __error_msg );
        res = NULL;
    }
    return res;
}
