/**
 * A plugin to draw dot graphs from the AST:
 *  - the AST itself
 *  - the connection graph of the network
 *
 * @file    smxdot.h
 * @author  Simon Maurer
 *
 */

#ifndef DOT_H
#define DOT_H

#include <stdio.h>
#include "ast.h"

#ifdef DOT_AST
/**
 * @brief   Draw a dot diagram of the AST
 *
 * @param start   pointer to the root node of the AST
 */
void draw_ast_graph( ast_node_t* );

/**
 * @breif   Recursive function to draw AST nodes
 *
 * @param graph     file pointer to the dot file
 * @param ptr       pointer to the current ast node
 */
void draw_ast_graph_step( FILE*, ast_node_t* );
#endif // DOT_AST

/**
 * Add a devider to the file. This is used to later construct the
 * correct dot file.
 *
 * @param graph     file pointer to the dot file
 * @param scope     actual scope
 * @param flag      an indicator to tag the divider
 */
void graph_add_divider( FILE*, int, const char );

/**
 * @brief   Add an edge to the graph
 *
 * @param graph     file pointer to the dot file
 * @param start     id of the start node
 * @param end       id of the end node
 * @param label     name of the edge
 * @param style     style of the edge
 */
void graph_add_edge( FILE*, int, int, char*, int );

/**
 * @brief   Add a node to the graph
 *
 * @param graph     file pointer to the dot file
 * @param id        id of the node
 * @param name      name of the node
 * @param style     style of the node
 */
void graph_add_node( FILE*, int, char*, int );

/**
 * @brief   Create a rank of two nodes
 *
 * @param graph     file pointer to the dot file
 * @param id1       id of a node
 * @param id2       id of a node
 */
void graph_add_rank( FILE*, int, int );

/**
 * @brief   Adds final bracket to the dot file
 *
 * @param graph     file pointer to the dot file
 */
void graph_finish( FILE* );

/**
 * @brief   Closes the subgraph by adding closing brackets
 *
 * @param graph     file pointer to the dot file
 */
void graph_finish_subgraph( FILE* );

#ifdef DOT_CON
/**
 * @brief   Use tagged dividers to create valid dot file
 *
 * The dot instructions are put into a file following the ast structure.
 * Because of the nesting of wrappers the order of the instructions is not
 * correct. However, the insructions are prepended with labels that allow
 * to reorder the file in order to correctly compile. This functions performs
 * this reordering.
 *
 * @param t_path    path to a temporary file
 * @param r_path    path to the dot file to be reordered
 */
void graph_fix_dot( char*, char* );
#endif // DOT_CON

/**
 * @brief   Initializes the file with the dot header to draw a graph
 *
 * @param graph     file pointer to the dot file
 * @param style     style of the graph (STYLE_DEFAULT, STYLE_CON_GRAPH)
 */
void graph_init( FILE*, int );

/**
 * Initializes a subgraph with a lable composed out of a name and a scope
 *
 * @param graph     file pointer to the dot file
 * @param name      name of the net that is represented as a subgraph
 * @param id        id of the node
 * @param style     scope of the net that is represented as a subgraph
 */
void graph_init_subgraph( FILE*, char*, int, int );

#endif /* DOT_H */
