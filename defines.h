/* 
 * Global definitions
 *
 * @file    defines.h
 * @author  Simon Maurer
 *
 * */

#ifndef DEFINES_H
#define DEFINES_H

// constants
#define CONST_ERROR_LEN 256
#define CONST_SCOPE_LEN 4

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
#define VAL_COPY    4

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
#define SHAPE_CIRCLE    "circle"

#define COLOR_SIDE      "goldenrod3"
#define COLOR_PARALLEL  "chartreuse3"
#define COLOR_SERIAL    "cadetblue3"


// styles of graphs
#define STYLE_G_DEFAULT     0
#define STYLE_G_CON_NET     1
#define STYLE_G_CON_PORT    2
#define STYLE_G_AST         3

// styles of subgraphs
#define STYLE_SG_WRAPPER    10
#define STYLE_SG_PARALLEL   11
#define STYLE_SG_SERIAL     12

// styles of edges
#define STYLE_E_DEFAULT     20
#define STYLE_E_SIDE        21

// styles of nodes
#define STYLE_N_DEFAULT     31
#define STYLE_N_NET_BOX     32
#define STYLE_N_NET_CP      33
#define STYLE_N_AST_ATTR    34
#define STYLE_N_AST_NODE    35
#define STYLE_N_AST_ID      36

// flags taht are used to reorder the dot file
#define FLAG_STMTS      's'
#define FLAG_STMTS_END  'S'
#define FLAG_WRAP       'w'
#define FLAG_WRAP_END   'W'
#define FLAG_NET        'n'
#define FLAG_CONNECT    'c'

#define MAX_STACK_SIZE  100

// errors
#define ERR_WARNING "warning"
#define ERR_ERROR   "error"

#define ERROR_UNDEFINED_ID  "%s: use of undeclared identifier '%s'"
#define ERROR_DUPLICATE_ID  "%s: redefinition of '%s'"
#define ERROR_BAD_MODE      "%s: conflicting modes of ports '%s' in '%s'(%d) and '%s'(%d) (line %d)"
#define ERROR_NO_NET_CON    "%s: no port connection in serial combinition of '%s'(%d) and '%s'(%d)"
#define ERROR_NO_PORT_CON   "%s: port '%s' in '%s'(%d) is not connected"
#define ERROR_NO_PORT       "%s: no side port '%s' in '%s'(%d)"

#endif /* DEFINES_H */
