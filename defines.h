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
#define VAL_WRAP    1

// AST node_types
#define AST_SERIAL      0
#define AST_PARALLEL    1
#define AST_ID          2
#define AST_NET         3
#define AST_BOX         4
#define AST_WRAP        5
#define AST_STMT        6
#define AST_PORT        7
#define AST_STMTS       8

// drawing
#define AST_DOT_PATH    "dot/ast_graph.dot"
#define CON_DOT_PATH    "dot/connection_graph.dot"

#define LABEL_SERIAL    "serial"
#define LABEL_PARALLEL  "parallel"
#define LABEL_NET       "net"
#define LABEL_BOX       "box"
#define LABEL_WRAP      "wrapper"
#define LABEL_PORT      "port"

#define SHAPE_BOX       "box"
#define SHAPE_ELLIPSE   "ellipse"

#define STYLE_DEFAULT   0
#define STYLE_CON_GRAPH 1

#define MAX_STACK_SIZE  100

// errors
#define WARNING_STACK_OVERFLOW  "warning: Stack overflow, increase MAX_STACK_SIZE"
#define ERROR_UNDEFINED_ID      "%d: error: %s is an undeclared identifier"
#define ERROR_DUPLICATE_ID      "%d: error: box %s is already defined"
#define ERROR_DUPLICATE_PORT    "%d: error: port %s is already defined in this scope"

#endif /* DEFINES_H */
