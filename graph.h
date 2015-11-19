/* 
 * A plugin to draw dot graphs from the AST:
 *  - the AST itself
 *  - teh connection graph of the network
 *
 * @file    graph.h
 * @author  Simon Maurer
 *
 * */

#ifndef GRAPH_H
#define GRAPH_H

#include <stdio.h>
#include "ast.h"

/**
 * Draw a dot diagram of the AST
 *
 * @param ast_node*:  pointer to the root node of the AST
 * */
void draw_ast_graph ( ast_node* );

/**
 * Recursive function to draw AST nodes
 *
 * @param FILE*:        file pointer to the dot file
 * @param ast_node*:    pointer to the current ast node
 * */
void draw_ast_graph_step ( FILE*, ast_node* );

/**
 * Add an edge to the graph
 *
 * @param FILE*:    file pointer to the dot file
 * @param int:      id of the start node
 * @param int:      id of the end node
 * */
void graph_add_edge ( FILE*, int, int );

/**
 * Add a inode to the graph
 *
 * @param FILE*:        file pointer to the dot file
 * @param int:          id of the node
 * @param char*:        name of the node
 * @param const char*:  shape of the node (SHAPE_BOX, SHAPE_ELLIPSE)
 * */
void graph_add_node ( FILE*, int, char*, const char* );

/**
 * Adds final bracket to the dot file
 *
 * @param FILE*:        file pointer to the dot file
 * */
void graph_finish ( FILE* );

/**
 * Initializes the file with the dot header to draw a graph
 *
 * @param FILE*:    file pointer to the dot file
 * @param int:      style of the graph (STYLE_DEFAULT, STYLE_CON_GRAPH)
 * */
void graph_init ( FILE*, int );

#endif /* GRAPH_H */
