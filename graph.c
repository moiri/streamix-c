#include "defines.h"
#include "graph.h"

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
    char child_node_id_str[11];
    char node_id_str[11];
    char node_name[11];
    sprintf(node_id_str, "id%d", ptr->id);
    if (ptr->node_type == OP_ID) {
        // reached a leaf node of the AST -> add box to drawing
        graph_add_node(graph, node_id_str, ptr->name, SHAPE_BOX);
    }
    else if (ptr->node_type == OP_SERIAL || ptr->node_type == OP_PARALLEL) {
        if (ptr->node_type == OP_SERIAL)    sprintf(node_name, "serial");
        if (ptr->node_type == OP_PARALLEL)  sprintf(node_name, "parallel");
        graph_add_node(graph, node_id_str, node_name, SHAPE_ELLIPSE);
        // continue on the left branch
        draw_ast_graph_step(graph, ptr->op.left);
        sprintf(child_node_id_str, "id%d", ptr->op.left->id);
        graph_add_edge(graph, node_id_str, child_node_id_str);
        // continue on the right branch
        draw_ast_graph_step(graph, ptr->op.right);
        sprintf(child_node_id_str, "id%d", ptr->op.right->id);
        graph_add_edge(graph, node_id_str, child_node_id_str);
    }
}

/******************************************************************************/
void draw_connection_graph (ast_node* start) {
    FILE* con_graph = fopen(CON_DOT_PATH, "w");
    graph_init(con_graph, STYLE_CON_GRAPH);
    draw_connection_graph_step(con_graph, start);
    graph_finish(con_graph);
    fclose(con_graph);
}

/******************************************************************************/
void draw_connection_graph_step (FILE* graph, ast_node* ptr) {
    char node_id_str[11];
    char tmp_node_id_str[11];
    con_list* i_ptr;
    con_list* j_ptr;

    if (ptr->node_type == OP_ID) {
        // reached a leaf node of the AST -> add box to drawing
        sprintf(node_id_str, "id%d", ptr->id);
        graph_add_node(graph, node_id_str, ptr->name, SHAPE_BOX);
    }
    else {
        // reached an operand -> follow lefat and right branch
        draw_connection_graph_step(graph, ptr->op.left);
        draw_connection_graph_step(graph, ptr->op.right);

        if (ptr->node_type == OP_SERIAL) {
            // serial operand -> draw all conenctions at this stage
            i_ptr = ptr->op.left->connect.right;
            do {
                sprintf(node_id_str, "id%d", i_ptr->ast_node->id);
                j_ptr = ptr->op.right->connect.left;
                do {
                    sprintf(tmp_node_id_str, "id%d", j_ptr->ast_node->id);
                    graph_add_edge(graph, node_id_str, tmp_node_id_str);
                    j_ptr = j_ptr->next;
                }
                while (j_ptr != 0);
                i_ptr = i_ptr->next;
            }
            while (i_ptr != 0);
        }
    }
}

/******************************************************************************/
void graph_add_edge ( FILE* graph, char* start, char* end) {
    fprintf(graph, "\t%s->%s;\n", start, end);
}

/******************************************************************************/
void graph_add_node ( FILE* graph, char* id, char* name, const char* shape ) {
    fprintf(graph, "\t%s [label=\"%s\", shape=%s];\n", id, name, shape);
}

/******************************************************************************/
void graph_finish (FILE* graph) {
    fprintf(graph, "}");
}

/******************************************************************************/
void graph_init (FILE* graph, int style) {
    fprintf(graph, "digraph Net {\n");
    switch (style) {
        case STYLE_CON_GRAPH:
            fprintf(graph, "\tedge [dir=none]\n");
            fprintf(graph, "\trankdir=LR\n");
            break;
        default:
            ;
    }
}
