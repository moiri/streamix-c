#include "defines.h"
#include "graph.h"

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
                graph_add_edge( graph, ptr->id, ast_list_ptr->ast_node->id, NULL );
                ast_list_ptr = ast_list_ptr->next;
            }
            while (ast_list_ptr != 0);
            break;
        // draw operators
        case AST_SERIAL:
        case AST_PARALLEL:
            graph_add_node(graph, ptr->id, node_label[ptr->node_type], SHAPE_ELLIPSE);
            // continue on the left branch
            draw_ast_graph_step( graph, ptr->op.left );
            graph_add_edge( graph, ptr->id, ptr->op.left->id, NULL );
            // continue on the right branch
            draw_ast_graph_step( graph, ptr->op.right );
            graph_add_edge( graph, ptr->id, ptr->op.right->id, NULL );
            break;
        // draw simple nodes
        case AST_COUPLING:
        case AST_STATE:
        case AST_NET:
        case AST_INT_ID:
        case AST_MODE:
            graph_add_node( graph, ptr->id, node_label[ptr->node_type], SHAPE_ELLIPSE );
            draw_ast_graph_step( graph, ptr->ast_node );
            graph_add_edge( graph, ptr->id, ptr->ast_node->id, NULL );
            break;
        // draw special nodes
        case AST_CONNECT:
            graph_add_node( graph, ptr->id, node_label[ ptr->node_type ], SHAPE_ELLIPSE );
            // id
            draw_ast_graph_step( graph, ptr->connect.id );
            graph_add_edge( graph, ptr->id, ptr->connect.id->id, NULL );
            // connecting nets
            if (ptr->connect.connects != 0) {
                draw_ast_graph_step( graph, ptr->connect.connects );
                graph_add_edge( graph, ptr->id, ptr->connect.connects->id, NULL );
            }
            break;
        case AST_BOX:
            graph_add_node(graph, ptr->id, node_label[ptr->node_type], SHAPE_ELLIPSE);
            // id
            draw_ast_graph_step( graph, ptr->box.id );
            graph_add_edge( graph, ptr->id, ptr->box.id->id, NULL );
            // port list
            if (ptr->box.ports != 0) {
                draw_ast_graph_step( graph, ptr->box.ports );
                graph_add_edge( graph, ptr->id, ptr->box.ports->id, NULL );
            }
            // state
            if (ptr->box.state != 0) {
                draw_ast_graph_step( graph, ptr->box.state );
                graph_add_edge( graph, ptr->id, ptr->box.state->id, NULL );
            }
            break;
        case AST_WRAP:
            graph_add_node( graph, ptr->id, node_label[ ptr->node_type ], SHAPE_ELLIPSE );
            // id
            draw_ast_graph_step( graph, ptr->wrap.id );
            graph_add_edge( graph, ptr->id, ptr->wrap.id->id, NULL );
            // port list
            if (ptr->wrap.ports != 0) {
                draw_ast_graph_step( graph, ptr->wrap.ports );
                graph_add_edge( graph, ptr->id, ptr->wrap.ports->id, NULL );
            }
            // stmt list
            if (ptr->wrap.stmts != 0) {
                draw_ast_graph_step( graph, ptr->wrap.stmts );
                graph_add_edge( graph, ptr->id, ptr->wrap.stmts->id, NULL );
            }
            break;
        case AST_PORT:
            graph_add_node( graph, ptr->id, node_label[ptr->node_type], SHAPE_ELLIPSE );
            // id
            draw_ast_graph_step( graph, ptr->port.id );
            graph_add_edge( graph, ptr->id, ptr->port.id->id, NULL );
            // internal id
            if (ptr->port.int_id != 0) {
                draw_ast_graph_step( graph, ptr->port.int_id );
                graph_add_edge( graph, ptr->id, ptr->port.int_id->id, NULL );
            }
            // mode
            draw_ast_graph_step( graph, ptr->port.mode );
            graph_add_edge( graph, ptr->id, ptr->port.mode->id, NULL );
            // collection
            if (ptr->port.collection != 0) {
                draw_ast_graph_step( graph, ptr->port.collection );
                graph_add_edge( graph, ptr->id, ptr->port.collection->id, NULL );
            }
            // coupling
            if (ptr->port.coupling != 0) {
                draw_ast_graph_step( graph, ptr->port.coupling );
                graph_add_edge( graph, ptr->id, ptr->port.coupling->id, NULL );
            }
            break;
        default:
            ;
    }
}
#endif // DOT_AST

/******************************************************************************/
void graph_add_edge ( FILE* graph, int start, int end, char* label ) {
    fprintf(graph, "\tid%d->id%d", start, end);
    if( label != NULL )
        fprintf( graph, "[label=\"%s\"]", label );
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
void graph_init (FILE* graph, int style) {
    fprintf(graph, "digraph {\n");
    switch (style) {
        case STYLE_N_CON_GRAPH:
            fprintf(graph, "\tedge [dir=none]\n");
        case STYLE_P_CON_GRAPH:
            fprintf(graph, "\trankdir=LR\n");
            break;
        default:
            ;
    }
}
