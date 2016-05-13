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

void cgraph_update( igraph_t*, int, int, int, int, igraph_t* );

/**
 * adds edges to a graph according to the connection semantics of streamix
 *
 * @param igraph_t*:        the graph where the edges are added
 * @param igraph_vector_t*: vector of the left connecting nets
 * @param igraph_vector_t*: vector of the right connecting nets
 */
void dgraph_connect_full_ptr( igraph_t*, igraph_vector_ptr_t*,
        igraph_vector_ptr_t* );
void dgraph_connect_1( igraph_t*, int, int, int, int );
void dgraph_disconnect_full( igraph_t*, igraph_vector_t*, igraph_vector_t* );
int dgraph_merge_vertice_1( igraph_t*, int, int );

#endif // NGRAPH_H
