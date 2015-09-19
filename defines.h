/* 
 * Global definitions
 *
 * @file    defines.h
 * @author  Simon Maurer
 *
 * */

#ifndef DEFINES_H
#define DEFINES_H

#define VAL_UP      0
#define VAL_DOWN    1
#define VAL_SIDE    2

#define VAL_IN      0
#define VAL_OUT     1

#define VAL_BOX     0
#define VAL_WRAP    1

#define OP_SERIAL   0
#define OP_PARALLEL 1
#define OP_ID       2

#define SHAPE_BOX       "box"
#define SHAPE_ELLIPSE   "ellipse"

#define STYLE_DEFAULT   0
#define EDGE_UNDIRECTED 1

#define AST_DOT_PATH    "dot/ast_graph.dot"
#define CON_DOT_PATH    "dot/connection_graph.dot"

#define MAX_STACK_SIZE  100

#define WARNING_STACK_OVERFLOW "WARNING: Stack overflow, increase MAX_STACK_SIZE"

#endif /* DEFINES_H */
