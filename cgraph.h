/* 
 * Small library to handle the connection graph according to the semantics of
 * streamix
 *
 * @file    cgraph.h
 * @author  Simon Maurer
 *
 * */

#ifndef CGRAPH_H
#define CGRAPH_H

#include <igraph.h>

/**
 * adds edges to a graph according to the connection semantics of streamix
 *
 * @param igraph_t*:        the graph where the edges are added
 * @param igraph_vector_t*: vector of the left connecting nets
 * @param igraph_vector_t*: vector of the right connecting nets
 */
void cgraph_connect_full_ptr( igraph_t*, igraph_vector_ptr_t*,
        igraph_vector_ptr_t* );
void cgraph_connect_1( igraph_t*, int, int, int, int );
void cgraph_disconnect_full( igraph_t*, igraph_vector_t*, igraph_vector_t* );
int cgraph_merge_vertice_1( igraph_t*, int, int );

#endif // CGRAPH_H
