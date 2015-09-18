#include <stdlib.h> /* For malloc in symbol table */
#include <string.h> /* For strcmp in symbol table */
#include <stdio.h>
#include "ast.h"
#include "graph.h"
#include "defines.h"

int global_node_id = 0;
/*
 * Add a net identifier to the AST.
 *
 * @param: char* name:  name of the net
 * @return: ast_node*:
 *      a pointer to the location where the data was stored
 * */
ast_node* ast_add_id ( char* name ) {
    ast_node *ptr;
    ptr = (ast_node*) malloc(sizeof(ast_node));
    ptr->name = (char*) malloc(strlen(name)+1);
    strcpy (ptr->name, name);
    ptr->node_type = OP_ID;
    return ptr;
}

/*
 * Add a an operation to the symbol table.
 *
 * @param: char* name:      name of the net
 * @param: ast_node* left:  pointer to the left operand
 * @param: ast_node* right: pointer to the right operand
 * @return: ast_node*:
 *      a pointer to the location where the data was stored
 * */
ast_node* ast_add_op ( ast_node* left, ast_node* right, int node_type ) {
    ast_node *ptr;
    ptr = (ast_node*) malloc(sizeof(ast_node));
    ptr->op.left = left;
    ptr->op.right = right;
    ptr->node_type = node_type;
    return ptr;
}

/*
 * Draw a dot diagram of the AST
 *
 * @param: ast_node* start:  pointer to the root node of the AST
 * */
void draw_ast (ast_node* start) {
    FILE* ast_graph = fopen("ast.dot", "w");
    initGraph(ast_graph);
    draw_ast_step(ast_graph, start, 0);
    finishGraph(ast_graph);
}

int draw_ast_step (FILE* graph, ast_node* ptr, int node_id) {
    int child_node_id;
    char child_node_id_str[11];
    char node_id_str[11];
    char node_name[11];
    global_node_id++;
    node_id = global_node_id;
    sprintf(node_id_str, "id%d", node_id);
    if (ptr->node_type == OP_ID) {
        addNode(graph, node_id_str, ptr->name, SHAPE_BOX);
    }
    else if (ptr->node_type == OP_SERIAL || ptr->node_type == OP_PARALLEL) {
        if (ptr->node_type == OP_SERIAL)    strcpy(node_name, "serial");
        if (ptr->node_type == OP_PARALLEL)  strcpy(node_name, "parallel");
        addNode(graph, node_id_str, node_name, SHAPE_ELLIPSE);
        // continue on the left branch
        child_node_id = draw_ast_step(graph, ptr->op.left, node_id);
        sprintf(child_node_id_str, "id%d", child_node_id);
        addEdge(graph, node_id_str, child_node_id_str);
        // continue on the right branch
        child_node_id = draw_ast_step(graph, ptr->op.right, child_node_id);
        sprintf(child_node_id_str, "id%d", child_node_id);
        addEdge(graph, node_id_str, child_node_id_str);
    }
    return node_id;
}
