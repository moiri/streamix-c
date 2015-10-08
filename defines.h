/* 
 * Global definitions
 *
 * @file    defines.h
 * @author  Simon Maurer
 *
 * */

#ifndef DEFINES_H
#define DEFINES_H

// scanner values
#define VAL_UP      0
#define VAL_DOWN    1
#define VAL_SIDE    2

#define VAL_IN      0
#define VAL_OUT     1

#define VAL_BOX     0
#define VAL_NET     1

#define VAL_DECOUPLED 0

#define VAL_STATELESS 0

// drawing
#define AST_DOT_PATH    "dot/ast_graph.dot"
#define CON_DOT_PATH    "dot/connection_graph.dot"

#define SHAPE_BOX       "box"
#define SHAPE_ELLIPSE   "ellipse"
#define SHAPE_OCTAGON   "octagon"

#define STYLE_DEFAULT   0
#define STYLE_CON_GRAPH 1

#define MAX_STACK_SIZE  100

// errors
#define WARNING_STACK_OVERFLOW  "warning: Stack overflow, increase MAX_STACK_SIZE"
#define ERROR_UNDEFINED_ID      "%d: error: %s is an undeclared identifier"
#define ERROR_DUPLICATE_ID      "%d: error: box %s is already defined"
#define ERROR_DUPLICATE_PORT    "%d: error: port %s is already defined in this scope"

#endif /* DEFINES_H */
