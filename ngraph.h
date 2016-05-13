/* 
 * Small library to handle the connection graph according to the semantics of
 * streamix
 *
 * @file    cgraph.h
 * @author  Simon Maurer
 *
 * */

#ifndef NGRAPH_H
#define NGRAPH_H

#include <igraph.h>

/**
 * Adds edges to a connection graph according to the connection semantics of
 * streamix
 *
 * @param igraph_t*:             the graph where the edges are added
 * @param igraph_vector_ptr_t*:  vector of pointers to ids of the left operand
 * @param igraph_vector_ptr_t*:  vector of pointers to ids of the right operand
 */
void cgraph_connect_full_ptr( igraph_t*, igraph_vector_ptr_t*,
        igraph_vector_ptr_t* );

/**
 * Removes edges from a connection graph according to the connection semantics
 * of streamix
 *
 * @param igraph_t*:        the graph from which the edges are removed
 * @param igraph_vector_t*: vector of ids of the left operands
 * @param igraph_vector_t*: vector of ids of the right operands
 */
void cgraph_disconnect_full( igraph_t*, igraph_vector_t*, igraph_vector_t* );

/**
 * Update the connection graph by removing edges that have been added to the
 * dependancy graph
 *
 * @param igraph_t*:    pointer to the connection graph
 * @param int:          id of the left operand
 * @param int:          id of the right operand
 * @param int:          type of the left operand
 * @param int:          type of the right operand
 * @param igraph_t*:    pointer to the dependancy graph
 */
void cgraph_update( igraph_t*, int, int, int, int, igraph_t* );

/**
 * Adds a directed connection between to nets to the dependancy graph
 *
 * @param igraph_t*:    pointer to the dependancy graph
 * @param int:          id of the left operand
 * @param int:          id of the right operand
 * @param int:          mode of the left operand
 * @param int:          mode of the right operand
 */
void dgraph_connect_1( igraph_t*, int, int, int, int );

/**
 * Merge to vertices into one
 *
 * @param igraph_t*:    pointer to the dependancy graph
 * @param int:          id of a vertex
 * @param int:          id of a vertex
 * @return int:         id of the removed vertex
 */
int dgraph_merge_vertice_1( igraph_t*, int, int );

#endif // NGRAPH_H
