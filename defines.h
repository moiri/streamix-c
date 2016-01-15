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
#define VAL_NONE    0
#define VAL_UP      1
#define VAL_DOWN    2
#define VAL_SIDE    3

#define VAL_IN      0
#define VAL_OUT     1

#define VAL_BOX     0
#define VAL_NET     1
#define VAL_PORT    2
#define VAL_SPORT   3

#define VAL_DECOUPLED   0

#define VAL_STATELESS   0

// drawing
#define AST_DOT_PATH    "dot/ast_graph.dot"
#define N_CON_DOT_PATH  "dot/net_connection_graph.dot"
#define P_CON_DOT_PATH  "dot/port_connection_graph.dot"
#define TEMP_DOT_PATH   "dot/graph.dot.tmp"
#define DOT_PATTERN     "// ===>"

#define SHAPE_BOX       "box"
#define SHAPE_ELLIPSE   "ellipse"
#define SHAPE_OCTAGON   "octagon"

#define STYLE_DEFAULT       0
#define STYLE_N_CON_GRAPH   1
#define STYLE_P_CON_GRAPH   2
#define STYLE_WRAPPER       3
#define STYLE_PARALLEL      4
#define STYLE_SERIAL        5

#define FLAG_STMTS      's'
#define FLAG_STMTS_END  'S'
#define FLAG_WRAP       'w'
#define FLAG_WRAP_END   'W'
#define FLAG_NET        'n'

#define MAX_STACK_SIZE  100

// errors
#define ERROR_UNDEFINED_ID  "%d: error: use of undeclared identifier '%s'"
#define ERROR_DUPLICATE_ID  "%d: error: redefinition of '%s'"

#endif /* DEFINES_H */
