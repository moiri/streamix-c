#include "defines.h"
#include "graph.h"

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
    char node_name[50];
    ast_list* ast_list_ptr;

    switch (ptr->node_type) {
        // reached a leaf node of the AST -> add box or octagon to drawing
        case AST_ATTR:
            graph_add_node(
                    graph,
                    ptr->id,
                    attr_label[ptr->attr.attr_type][ptr->attr.val],
                    SHAPE_OCTAGON
            );
            break;
        case AST_ID:
            graph_add_node(graph, ptr->id, ptr->name, SHAPE_BOX);
            break;
        case AST_STAR:
            graph_add_node(graph, ptr->id, LABEL_STAR, SHAPE_BOX);
            break;
        // draw a list-node with its children
        case AST_COLLECT:
            graph_add_node(graph, ptr->id, LABEL_COLLECT, SHAPE_ELLIPSE);
        case AST_CONNECTS:
            if (ptr->node_type == AST_CONNECTS)
                graph_add_node(graph, ptr->id, LABEL_CONNECTS, SHAPE_ELLIPSE);
        case AST_STMTS:
            if (ptr->node_type == AST_STMTS)
                graph_add_node(graph, ptr->id, LABEL_STMTS, SHAPE_ELLIPSE);
        case AST_PORTS:
            if (ptr->node_type == AST_PORTS)
                graph_add_node(graph, ptr->id, LABEL_PORTS, SHAPE_ELLIPSE);
        case AST_SYNC:
            if (ptr->node_type == AST_SYNC)
                graph_add_node(graph, ptr->id, LABEL_SYNC, SHAPE_ELLIPSE);
            // iterate through all elements of the list
            ast_list_ptr = ptr->ast_list;
            do {
                draw_ast_graph_step(graph, ast_list_ptr->ast_node);
                graph_add_edge(graph, ptr->id, ast_list_ptr->ast_node->id);
                ast_list_ptr = ast_list_ptr->next;
            }
            while (ast_list_ptr != 0);
            break;
        // draw operators
        case AST_SERIAL:
            sprintf(node_name, LABEL_SERIAL);
        case AST_PARALLEL:
            if (ptr->node_type == AST_PARALLEL)
                sprintf(node_name, LABEL_PARALLEL);
            graph_add_node(graph, ptr->id, node_name, SHAPE_ELLIPSE);
            // continue on the left branch
            draw_ast_graph_step(graph, ptr->op.left);
            graph_add_edge(graph, ptr->id, ptr->op.left->id);
            // continue on the right branch
            draw_ast_graph_step(graph, ptr->op.right);
            graph_add_edge(graph, ptr->id, ptr->op.right->id);
            break;
        // draw simple nodes
        case AST_COUPLING:
            sprintf(node_name, LABEL_COUPLING);
        case AST_STATE:
            if (ptr->node_type == AST_STATE)
                sprintf(node_name, LABEL_STATE);
        case AST_NET:
            if (ptr->node_type == AST_NET)
                sprintf(node_name, LABEL_NET);
        case AST_INT_ID:
            if (ptr->node_type == AST_INT_ID)
                sprintf(node_name, LABEL_INT_ID);
        case AST_MODE:
            if (ptr->node_type == AST_MODE)
                sprintf(node_name, LABEL_MODE);
            graph_add_node(graph, ptr->id, node_name, SHAPE_ELLIPSE);
            draw_ast_graph_step(graph, ptr->ast_node);
            graph_add_edge(graph, ptr->id, ptr->ast_node->id);
            break;
        // draw special nodes
        case AST_CONNECT:
            graph_add_node(graph, ptr->id, LABEL_CONNECT, SHAPE_ELLIPSE);
            // id
            draw_ast_graph_step(graph, ptr->connect.id);
            graph_add_edge(graph, ptr->id, ptr->connect.id->id);
            // connecting nets
            if (ptr->connect.connects != 0) {
                draw_ast_graph_step(graph, ptr->connect.connects);
                graph_add_edge(graph, ptr->id, ptr->connect.connects->id);
            }
            break;
        case AST_BOX:
            graph_add_node(graph, ptr->id, LABEL_BOX, SHAPE_ELLIPSE);
            // id
            draw_ast_graph_step(graph, ptr->box.id);
            graph_add_edge(graph, ptr->id, ptr->box.id->id);
            // port list
            if (ptr->box.ports != 0) {
                draw_ast_graph_step(graph, ptr->box.ports);
                graph_add_edge(graph, ptr->id, ptr->box.ports->id);
            }
            // state
            if (ptr->box.state != 0) {
                draw_ast_graph_step(graph, ptr->box.state);
                graph_add_edge(graph, ptr->id, ptr->box.state->id);
            }
            break;
        case AST_WRAP:
            graph_add_node(graph, ptr->id, LABEL_WRAP, SHAPE_ELLIPSE);
            // id
            draw_ast_graph_step(graph, ptr->wrap.id);
            graph_add_edge(graph, ptr->id, ptr->wrap.id->id);
            // port list
            if (ptr->wrap.ports != 0) {
                draw_ast_graph_step(graph, ptr->wrap.ports);
                graph_add_edge(graph, ptr->id, ptr->wrap.ports->id);
            }
            // stmt list
            if (ptr->wrap.stmts != 0) {
                draw_ast_graph_step(graph, ptr->wrap.stmts);
                graph_add_edge(graph, ptr->id, ptr->wrap.stmts->id);
            }
            break;
        case AST_PORT:
            graph_add_node(graph, ptr->id, LABEL_PORT, SHAPE_ELLIPSE);
            // id
            draw_ast_graph_step(graph, ptr->port.id);
            graph_add_edge(graph, ptr->id, ptr->port.id->id);
            // internal id
            if (ptr->port.int_id != 0) {
                draw_ast_graph_step(graph, ptr->port.int_id);
                graph_add_edge(graph, ptr->id, ptr->port.int_id->id);
            }
            // mode
            draw_ast_graph_step(graph, ptr->port.mode);
            graph_add_edge(graph, ptr->id, ptr->port.mode->id);
            // collection
            if (ptr->port.collection != 0) {
                draw_ast_graph_step(graph, ptr->port.collection);
                graph_add_edge(graph, ptr->id, ptr->port.collection->id);
            }
            // coupling
            if (ptr->port.coupling != 0) {
                draw_ast_graph_step(graph, ptr->port.coupling);
                graph_add_edge(graph, ptr->id, ptr->port.coupling->id);
            }
            break;
        default:
            ;
    }
}

/******************************************************************************/
void draw_connection_graph (FILE* con_graph, ast_node* start) {
    graph_init(con_graph, STYLE_CON_GRAPH);
    draw_connection_graph_step(con_graph, start);
    graph_finish(con_graph);
}

/******************************************************************************/
void draw_connection_graph_step (FILE* graph, ast_node* ptr) {
    int node_id, tmp_node_id;
    ast_list* i_ptr;
    ast_list* j_ptr;

    if (ptr->node_type == AST_ID) {
        // reached a leaf node of the AST -> add box to drawing
        graph_add_node(graph, ptr->id, ptr->name, SHAPE_BOX);
    }
    else {
        // reached an operand -> follow lefat and right branch
        draw_connection_graph_step(graph, ptr->op.left);
        draw_connection_graph_step(graph, ptr->op.right);

        if (ptr->node_type == AST_SERIAL) {
            // serial operand -> draw all conenctions at this stage
            i_ptr = ptr->op.left->op.con_right;
            do {
                if (ptr->op.left->node_type == AST_ID) {
                    node_id = ptr->op.left->id;
                    i_ptr = (ast_list*)0;
                }
                else {
                    node_id = i_ptr->ast_node->id;
                    i_ptr = i_ptr->next;
                }
                j_ptr = ptr->op.right->op.con_left;
                do {
                    if (ptr->op.right->node_type == AST_ID) {
                        tmp_node_id = ptr->op.right->id;
                        j_ptr = (ast_list*)0;
                    }
                    else {
                        tmp_node_id = j_ptr->ast_node->id;
                        j_ptr = j_ptr->next;
                    }
                    graph_add_edge(graph, node_id, tmp_node_id);
                }
                while (j_ptr != 0);
            }
            while (i_ptr != 0);
        }
    }
}

/******************************************************************************/
void graph_add_edge ( FILE* graph, int start, int end) {
    fprintf(graph, "\tid%d->id%d;\n", start, end);
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
        case STYLE_CON_GRAPH:
            fprintf(graph, "\tedge [dir=none]\n");
            fprintf(graph, "\trankdir=LR\n");
            break;
        default:
            ;
    }
}
