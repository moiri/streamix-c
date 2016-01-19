#include "defines.h"
#include "graph.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef DOT_AST
char* node_label[] = {
    "box decl",
    "collection",
    "connect",
    "connecting",
    "coupling",
    "internal ID",
    "mode",
    "net",
    "parallel",
    "port decl",
    "ports",
    "serial",
    "*",
    "state",
    "stmt",
    "stmts",
    "sync",
    "net decl"
};
char* mode_label[] = {
    "in",
    "out"
};
char* class_label[] = {
    "up",
    "down",
    "side"
};
char* coupling_label[] = {
    "decoupled"
};
char* state_label[] = {
    "stateless"
};
char** attr_label[] = {
    mode_label,
    class_label,
    coupling_label,
    state_label
};

/******************************************************************************/
void draw_ast_graph (ast_node* start) {
    FILE* ast_graph = fopen(AST_DOT_PATH, "w");
    graph_init(ast_graph, STYLE_DEFAULT);
    draw_ast_graph_step(ast_graph, start);
    graph_finish(ast_graph);
    fclose(ast_graph);
}

/******************************************************************************/
void draw_ast_graph_step (FILE* graph, ast_node* ptr) {
    ast_list* ast_list_ptr;

    switch (ptr->node_type) {
        // reached a leaf node of the AST -> add box or octagon to drawing
        case AST_ATTR:
            graph_add_node(
                    graph,
                    ptr->id,
                    attr_label[ptr->ast_attr.attr_type][ptr->ast_attr.val],
                    SHAPE_OCTAGON
            );
            break;
        case AST_ID:
            graph_add_node(graph, ptr->id, ptr->ast_id.name, SHAPE_BOX);
            break;
        case AST_STAR:
            graph_add_node(graph, ptr->id, node_label[ptr->node_type], SHAPE_BOX);
            break;
        // draw a list-node with its children
        case AST_COLLECT:
        case AST_CONNECTS:
        case AST_STMTS:
        case AST_PORTS:
        case AST_SYNC:
            graph_add_node(graph, ptr->id, node_label[ptr->node_type], SHAPE_ELLIPSE);
            // iterate through all elements of the list
            ast_list_ptr = ptr->ast_list;
            do {
                draw_ast_graph_step( graph, ast_list_ptr->ast_node );
                graph_add_edge( graph, ptr->id, ast_list_ptr->ast_node->id, NULL,
                        false );
                ast_list_ptr = ast_list_ptr->next;
            }
            while (ast_list_ptr != 0);
            break;
        // draw operators
        case AST_SERIAL:
        case AST_PARALLEL:
            graph_add_node(graph, ptr->id, node_label[ptr->node_type],
                    SHAPE_ELLIPSE);
            // continue on the left branch
            draw_ast_graph_step( graph, ptr->op.left );
            graph_add_edge( graph, ptr->id, ptr->op.left->id, NULL, false );
            // continue on the right branch
            draw_ast_graph_step( graph, ptr->op.right );
            graph_add_edge( graph, ptr->id, ptr->op.right->id, NULL, false );
            break;
        // draw simple nodes
        case AST_COUPLING:
        case AST_STATE:
        case AST_NET:
        case AST_INT_ID:
        case AST_MODE:
            graph_add_node( graph, ptr->id, node_label[ptr->node_type],
                    SHAPE_ELLIPSE );
            draw_ast_graph_step( graph, ptr->ast_node );
            graph_add_edge( graph, ptr->id, ptr->ast_node->id, NULL, false );
            break;
        // draw special nodes
        case AST_CONNECT:
            graph_add_node( graph, ptr->id, node_label[ ptr->node_type ],
                    SHAPE_ELLIPSE );
            // id
            draw_ast_graph_step( graph, ptr->connect.id );
            graph_add_edge( graph, ptr->id, ptr->connect.id->id, NULL, false );
            // connecting nets
            if (ptr->connect.connects != 0) {
                draw_ast_graph_step( graph, ptr->connect.connects );
                graph_add_edge( graph, ptr->id, ptr->connect.connects->id, NULL,
                        false);
            }
            break;
        case AST_BOX:
            graph_add_node(graph, ptr->id, node_label[ptr->node_type],
                    SHAPE_ELLIPSE);
            // id
            draw_ast_graph_step( graph, ptr->box.id );
            graph_add_edge( graph, ptr->id, ptr->box.id->id, NULL, false );
            // port list
            if (ptr->box.ports != 0) {
                draw_ast_graph_step( graph, ptr->box.ports );
                graph_add_edge( graph, ptr->id, ptr->box.ports->id, NULL,
                        false );
            }
            // state
            if (ptr->box.state != 0) {
                draw_ast_graph_step( graph, ptr->box.state );
                graph_add_edge( graph, ptr->id, ptr->box.state->id, NULL,
                        false );
            }
            break;
        case AST_WRAP:
            graph_add_node( graph, ptr->id, node_label[ ptr->node_type ],
                    SHAPE_ELLIPSE );
            // id
            draw_ast_graph_step( graph, ptr->wrap.id );
            graph_add_edge( graph, ptr->id, ptr->wrap.id->id, NULL, false );
            // port list
            if (ptr->wrap.ports != 0) {
                draw_ast_graph_step( graph, ptr->wrap.ports );
                graph_add_edge( graph, ptr->id, ptr->wrap.ports->id, NULL,
                        false );
            }
            // stmt list
            if (ptr->wrap.stmts != 0) {
                draw_ast_graph_step( graph, ptr->wrap.stmts );
                graph_add_edge( graph, ptr->id, ptr->wrap.stmts->id, NULL,
                        false );
            }
            break;
        case AST_PORT:
            graph_add_node( graph, ptr->id, node_label[ptr->node_type],
                    SHAPE_ELLIPSE );
            // id
            draw_ast_graph_step( graph, ptr->port.id );
            graph_add_edge( graph, ptr->id, ptr->port.id->id, NULL, false );
            // internal id
            if (ptr->port.int_id != 0) {
                draw_ast_graph_step( graph, ptr->port.int_id );
                graph_add_edge( graph, ptr->id, ptr->port.int_id->id, NULL,
                        false );
            }
            // mode
            draw_ast_graph_step( graph, ptr->port.mode );
            graph_add_edge( graph, ptr->id, ptr->port.mode->id, NULL, false );
            // collection
            if (ptr->port.collection != 0) {
                draw_ast_graph_step( graph, ptr->port.collection );
                graph_add_edge( graph, ptr->id, ptr->port.collection->id, NULL,
                        false);
            }
            // coupling
            if (ptr->port.coupling != 0) {
                draw_ast_graph_step( graph, ptr->port.coupling );
                graph_add_edge( graph, ptr->id, ptr->port.coupling->id, NULL,
                        false );
            }
            break;
        default:
            ;
    }
}
#endif // DOT_AST

/******************************************************************************/
void graph_add_divider ( FILE* graph, int scope, const char flag ) {
    fprintf(graph, "%s%c%d\n", DOT_PATTERN, flag, scope );
}

/******************************************************************************/
void graph_add_edge ( FILE* graph, int start, int end, char* label, bool side ) {
    fprintf(graph, "\tid%d->id%d", start, end);
#ifdef DOT_EDGE_LABEL
    if( ( label != NULL ) && side )
        fprintf( graph, "[xlabel=\"%s\", fontsize=10, color=%s]", label,
                COLOR_SIDE );
    else if( ( label != NULL ) && !side )
        fprintf( graph, "[xlabel=\"%s\", fontsize=10]", label );
#else
    if( side )
        fprintf( graph, "[color=%s]", COLOR_SIDE );
#endif // DOT_EDGE_LABEL
    fprintf( graph, ";\n" );
}

/******************************************************************************/
void graph_add_node ( FILE* graph, int id, char* name, const char* shape ) {
    fprintf(graph, "\tid%d [label=\"%s\", shape=%s];\n", id, name, shape);
}

/******************************************************************************/
void graph_finish (FILE* graph) {
    fprintf(graph, "}\n");
}

/******************************************************************************/
void graph_finish_subgraph (FILE* graph) {
    fprintf(graph, "\t}\n");
}

/******************************************************************************/
void graph_fix_dot( char* t_path, char* r_path ) {
    FILE* r_graph;
    FILE* t_graph;
    char str[512];
    char* ptr = NULL;
    char flag;
    char next_flag = FLAG_STMTS; // first flag will always be STMTS
    int scope = 0;
    int last_scope = -1;
    int iteration_cnt = 0;
    bool copy = false;
    bool done = false;
    char pattern[ sizeof( DOT_PATTERN ) + 10 ] = DOT_PATTERN;
    t_graph = fopen( t_path, "w" );
    while( !done ) {
        r_graph = fopen( r_path, "r" );
        while( fgets( str, 512, r_graph ) ) {
            ptr = strstr( str, pattern );
            if( ptr != NULL ) {
                // found an occurrence of the pattern
                copy = false;
                ptr += sizeof( DOT_PATTERN ) - 1;
                flag = *ptr;
                ptr++;
                scope = atoi(ptr);
                switch( flag ) {
                    case FLAG_STMTS:
                        if( scope <= last_scope ) continue;
                        if( flag != next_flag ) continue;
                        last_scope = scope;
                        next_flag = FLAG_WRAP;
                        copy = true;
                        break;
                    case FLAG_WRAP:
                        if( scope != last_scope ) continue;
                        if( flag != next_flag ) continue;
#ifdef DOT_SYNC_FIRST
                        next_flag = FLAG_CONNECT;
#else // DOT_SYNC_FIRST
                        next_flag = FLAG_NET;
#endif // DOT_SYNC_FIRST
                        copy = true;
                        break;
                    case FLAG_WRAP_END:
                    case FLAG_STMTS_END:
                    case FLAG_NET:
                    case FLAG_CONNECT:
                        if( scope != last_scope ) continue;
                        if( flag != next_flag ) continue;
                        copy = true;
                        break;
                    default:
                        ;
                }
                /* printf( "flag: %c, scope: %d\n", flag, scope ); */
            }
            else if( copy ) {
                fprintf( t_graph, "%s", str );
            }
        }
        switch( next_flag ) {
            case FLAG_STMTS:
                // no new stmts -> we are done
                done = true;
                break;
            case FLAG_WRAP:
                // no wrapper in this scope -> copy nets
                // because the wrap appears BEFORE the stmts we need to iterate
                // twice to make sure to get the wrap statements
#ifdef DOT_SYNC_FIRST
                if( iteration_cnt > 0 ) next_flag = FLAG_CONNECT;
#else // DOT_SYNC_FIRST
                if( iteration_cnt > 0 ) next_flag = FLAG_NET;
#endif // DOT_SYNC_FIRST
                iteration_cnt++;
                break;
            case FLAG_NET:
                // add side port connections
                iteration_cnt = 0;
#ifdef DOT_SYNC_FIRST
                next_flag = FLAG_WRAP_END;
#else // DOT_SYNC_FIRST
                next_flag = FLAG_CONNECT;
#endif // DOT_SYNC_FIRST
                break;
            case FLAG_CONNECT:
                // need to close wrapper
#ifdef DOT_SYNC_FIRST
                next_flag = FLAG_NET;
#else // DOT_SYNC_FIRST
                next_flag = FLAG_WRAP_END;
#endif // DOT_SYNC_FIRST
                break;
            case FLAG_WRAP_END:
                // need to close stmts
                next_flag = FLAG_STMTS_END;
                break;
            case FLAG_STMTS_END:
                // this scope is done -> go to the next one
                next_flag = FLAG_STMTS;
            default:
                ;
        }
        fclose( r_graph );
    }
    fclose( t_graph );
    // copy the fixed dot file back to the original file
    t_graph = fopen( t_path, "r" );
    r_graph = fopen( r_path, "w" );
    while( fgets( str, 512, t_graph ) ) {
        fprintf( r_graph, "%s", str );
    }
}

/******************************************************************************/
void graph_init( FILE* graph, int style ) {
    fprintf(graph, "digraph {\n");
    switch (style) {
        case STYLE_N_CON_GRAPH:
            fprintf(graph, "\tedge [dir=none];\n");
            fprintf(graph, "\trankdir=LR;\n");
            break;
        case STYLE_P_CON_GRAPH:
            fprintf(graph, "\trankdir=LR;\n");
#ifdef DOT_EDGE_LABEL
            fprintf(graph, "\tranksep=0.75;\n");
#endif // DOT_EDGE_LABEL
            // caused sideprts and copy synchrniyers to behave strange
            /* fprintf(graph, "\tsplines=false;\n"); */
            break;
        default:
            ;
    }
}

/******************************************************************************/
void graph_init_subgraph( FILE* graph, char* name, int style ) {
    switch (style) {
        case STYLE_PARALLEL:
            fprintf( graph, "\tsubgraph clusterP {\n" );
            fprintf( graph, "\tlabel=\"%s\";\n", name );
            fprintf( graph, "\tstyle=dotted;\n" );
#ifdef DOT_COLOR
            fprintf( graph, "\tcolor=%s;\n", COLOR_PARALLEL );
#else
            fprintf( graph, "\tcolor=invis;\n" );
#endif // DOT_CON
            break;
        case STYLE_SERIAL:
            fprintf( graph, "\tsubgraph clusterS {\n" );
            fprintf( graph, "\tlabel=\"%s\";\n", name );
            fprintf( graph, "\tstyle=dashed;\n" );
#ifdef DOT_COLOR
            fprintf( graph, "\tcolor=%s;\n", COLOR_SERIAL );
#else
            fprintf( graph, "\tcolor=invis;\n" );
#endif // DOT_CON
            break;
        case STYLE_WRAPPER:
            fprintf( graph, "\tsubgraph clusterW {\n" );
            fprintf( graph, "\tlabel=\"%s\";\n", name );
            break;
        default:
            ;
    }
}
