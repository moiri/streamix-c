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

#ifdef DOT_AST
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
#endif // DOT_AST

/**
 * Add a devider to the file. This is used to later construct the
 * correct dot file.
 *
 * @param FILE*: file pointer to the dot file
 * @param int:   actual scope
 * */
void graph_add_divider ( FILE*, int, const char );

/**
 * Add an edge to the graph
 *
 * @param FILE*:    file pointer to the dot file
 * @param int:      id of the start node
 * @param int:      id of the end node
 * @param char*:    name of the edge
 * @param bool:     flag to indicate wheter it is a side port
 * */
void graph_add_edge ( FILE*, int, int, char*, bool );

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
 * Closes the subgraph by adding closing brackets
 *
 * @param FILE*:        file pointer to the dot file
 * */
void graph_finish_subgraph ( FILE* );

/**
 * The dot instructions are put into a file following the ast structure.
 * Because of the nesting of wrappers the order of the instructions is not
 * correct. However, the insructions are prepended with labels that allow
 * to reorder the file in order to correctly compile. This functions performs
 * this reordering.
 *
 * @param char*:    path to a temporary file
 * @param char*:    path to the dot file to be reordered
 * */
void graph_fix_dot( char*, char* );

/**
 * Initializes the file with the dot header to draw a graph
 *
 * @param FILE*:    file pointer to the dot file
 * @param int:      style of the graph (STYLE_DEFAULT, STYLE_CON_GRAPH)
 * */
void graph_init ( FILE*, int );

/**
 * Initializes a subgraph with a lable composed out of a name and a scope
 *
 * @param FILE*:    file pointer to the dot file
 * @param char*:    name of the net that is represented as a subgraph
 * @param int:      scope of the net that is represented as a subgraph
 * */
void graph_init_subgraph ( FILE*, char*, int );

#endif /* GRAPH_H */
