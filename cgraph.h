/* 
 * Small library to create and access the connection graph
 *
 * @file    cgraph.h
 * @author  Simon Maurer
 *
 * */

#ifndef CGRAPH_H
#define CGRAPH_H

#include <igraph.h>

/**
 *
 */
void cgraph_connect( igraph_t*, igraph_vector_t*, igraph_vector_t* );

#endif // CGRAPH_H
