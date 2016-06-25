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
 * Convert a vector pointing to integer values into a vector of integer values
 *
 * @param igraph_vector_ptr_t*  vector to be converted
 * @param igraph_vector_t*      pointer to an initialised vector of size 0.
 *                              This vector will be resized and will hold the
 *                              integer values pointed to by the from vector
 */
void dgraph_vptr_to_v( igraph_vector_ptr_t*, igraph_vector_t* );

/**
 * Adds a directed connection between to nets to the dependancy graph
 *
 * @param igraph_t*:    pointer to the dependancy graph
 * @param int:          id of the left operand
 * @param int:          id of the right operand
 * @param int:          mode of the left operand
 * @param int:          mode of the right operand
 * @param const char*   name of the edge
 */
void dgraph_connect_1( igraph_t*, int, int, int, int, const char* );

/**
 * Merge two vertices into one
 *
 * @param igraph_t*:    pointer to the dependancy graph
 * @param int:          id of a vertex
 * @param int:          id of a vertex
 * @return int:         id of the removed vertex
 */
int dgraph_merge_vertice_1( igraph_t*, int, int );

#endif // NGRAPH_H
