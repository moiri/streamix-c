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
#include "ast.h"

/**
 *
 */
void cgraph_connect( igraph_t*, igraph_vector_t*, igraph_vector_t* );

/**
 *
 */
void cgraph_init( ast_node* );

/**
 *
 */
void cgraph_spawn( igraph_t*, ast_node*, net_con* );

#endif // CGRAPH_H
