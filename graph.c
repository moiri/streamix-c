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
    char node_name[50];
    ast_list* ast_list_ptr;
    sprintf(node_id_str, "id%d", ptr->id);
    switch (ptr->node_type) {
        // reached a leaf node of the AST -> add box or octagon to drawing
        case AST_ATTR:
            sprintf(node_name, "%s", ptr->name);
            graph_add_node(graph, node_id_str, node_name, SHAPE_OCTAGON);
            break;
        case AST_ID:
            sprintf(node_name, "%s", ptr->name);
            graph_add_node(graph, node_id_str, node_name, SHAPE_BOX);
            break;
        case AST_STAR:
            sprintf(node_name, "%s", LABEL_STAR);
            graph_add_node(graph, node_id_str, node_name, SHAPE_BOX);
            break;
        // draw a list-node with its children
        case AST_CONNECTS:
            graph_add_node(graph, node_id_str, LABEL_CONNECTS, SHAPE_ELLIPSE);
        case AST_STMTS:
            if (ptr->node_type == AST_STMTS)
                graph_add_node(graph, node_id_str, LABEL_STMTS, SHAPE_ELLIPSE);
        case AST_PORTS:
            if (ptr->node_type == AST_PORTS)
                graph_add_node(graph, node_id_str, LABEL_PORTS, SHAPE_ELLIPSE);
        case AST_SYNC:
            if (ptr->node_type == AST_SYNC)
                graph_add_node(graph, node_id_str, LABEL_SYNC, SHAPE_ELLIPSE);
            ast_list_ptr = ptr->ast_list;
            do {
                draw_ast_graph_step(graph, ast_list_ptr->ast_node);
                sprintf(child_node_id_str, "id%d", ast_list_ptr->ast_node->id);
                graph_add_edge(graph, node_id_str, child_node_id_str);
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
            graph_add_node(graph, node_id_str, node_name, SHAPE_ELLIPSE);
            // continue on the left branch
            draw_ast_graph_step(graph, ptr->op.left);
            sprintf(child_node_id_str, "id%d", ptr->op.left->id);
            graph_add_edge(graph, node_id_str, child_node_id_str);
            // continue on the right branch
            draw_ast_graph_step(graph, ptr->op.right);
            sprintf(child_node_id_str, "id%d", ptr->op.right->id);
            graph_add_edge(graph, node_id_str, child_node_id_str);
            break;
        // draw simple nodes
        case AST_NET:
            graph_add_node(graph, node_id_str, LABEL_NET, SHAPE_ELLIPSE);
            draw_ast_graph_step(graph, ptr->ast_node);
            sprintf(child_node_id_str, "id%d", ptr->ast_node->id);
            graph_add_edge(graph, node_id_str, child_node_id_str);
            break;
        // draw special nodes
        case AST_CONNECT:
            graph_add_node(graph, node_id_str, LABEL_CONNECT, SHAPE_ELLIPSE);
            draw_ast_graph_step(graph, ptr->connect.id);
            sprintf(child_node_id_str, "id%d", ptr->connect.id->id);
            graph_add_edge(graph, node_id_str, child_node_id_str);
            draw_ast_graph_step(graph, ptr->connect.connects);
            sprintf(child_node_id_str, "id%d", ptr->connect.connects->id);
            graph_add_edge(graph, node_id_str, child_node_id_str);
            break;
        case AST_BOX:
            graph_add_node(graph, node_id_str, LABEL_BOX, SHAPE_ELLIPSE);
            draw_ast_graph_step(graph, ptr->box.id);
            sprintf(child_node_id_str, "id%d", ptr->box.id->id);
            graph_add_edge(graph, node_id_str, child_node_id_str);
            draw_ast_graph_step(graph, ptr->box.ports);
            sprintf(child_node_id_str, "id%d", ptr->box.ports->id);
            graph_add_edge(graph, node_id_str, child_node_id_str);
            break;
        case AST_WRAP:
            graph_add_node(graph, node_id_str, LABEL_WRAP, SHAPE_ELLIPSE);
            draw_ast_graph_step(graph, ptr->wrap.id);
            sprintf(child_node_id_str, "id%d", ptr->wrap.id->id);
            graph_add_edge(graph, node_id_str, child_node_id_str);
            draw_ast_graph_step(graph, ptr->wrap.ports);
            sprintf(child_node_id_str, "id%d", ptr->wrap.ports->id);
            graph_add_edge(graph, node_id_str, child_node_id_str);
            draw_ast_graph_step(graph, ptr->wrap.stmts);
            sprintf(child_node_id_str, "id%d", ptr->wrap.stmts->id);
            graph_add_edge(graph, node_id_str, child_node_id_str);
            break;
        case AST_PORT:
            graph_add_node(graph, node_id_str, LABEL_PORT, SHAPE_ELLIPSE);
            draw_ast_graph_step(graph, ptr->port.id);
            sprintf(child_node_id_str, "id%d", ptr->port.id->id);
            graph_add_edge(graph, node_id_str, child_node_id_str);
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
    char node_id_str[11];
    char tmp_node_id_str[11];
    ast_list* i_ptr;
    ast_list* j_ptr;

    if (ptr->node_type == AST_ID) {
        // reached a leaf node of the AST -> add box to drawing
        sprintf(node_id_str, "id%d", ptr->id);
        graph_add_node(graph, node_id_str, ptr->name, SHAPE_BOX);
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
                    sprintf(node_id_str, "id%d", ptr->op.left->id);
                    i_ptr = (ast_list*)0;
                }
                else {
                    sprintf(node_id_str, "id%d", i_ptr->ast_node->id);
                    i_ptr = i_ptr->next;
                }
                j_ptr = ptr->op.right->op.con_left;
                do {
                    if (ptr->op.right->node_type == AST_ID) {
                        sprintf(tmp_node_id_str, "id%d", ptr->op.right->id);
                        j_ptr = (ast_list*)0;
                    }
                    else {
                        sprintf(tmp_node_id_str, "id%d", j_ptr->ast_node->id);
                        j_ptr = j_ptr->next;
                    }
                    graph_add_edge(graph, node_id_str, tmp_node_id_str);
                }
                while (j_ptr != 0);
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
