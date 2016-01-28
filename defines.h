/* 
 * Global definitions
 *
 * @file    defines.h
 * @author  Simon Maurer
 *
 * */

#ifndef DEFINES_H
#define DEFINES_H

/* #define DEBUG */
/* #define DEBUG_INST */
/* #define DEBUG_SYMB */
/* #define DEBUG_CONNECT */
/* #define DEBUG_LINK */
/* #define DEBUG_SERIAL */
/* #define DEBUG_PORT */
#define DEBUG_GRAPH

// constants
#define CONST_ERROR_LEN 256
#define CONST_SCOPE_LEN 9

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

#define VAL_THIS    "this"

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
#define COLOR_LINK      "deepskyblue4"


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
#define STYLE_E_PORT        21
#define STYLE_E_SPORT       22
#define STYLE_E_SPORT_IN    23
#define STYLE_E_SPORT_OUT   24
#define STYLE_E_SPORT_BI    25
#define STYLE_E_LPORT       26

// styles of nodes
#define STYLE_N_DEFAULT     31
#define STYLE_N_NET_BOX     32
#define STYLE_N_NET_CP      33
#define STYLE_N_NET_CPS     34
#define STYLE_N_NET_CPL     35
#define STYLE_N_NET_INVIS   36
#define STYLE_N_AST_ATTR    37
#define STYLE_N_AST_NODE    38
#define STYLE_N_AST_ID      39

// flags that are used to reorder the dot file
#define FLAG_STMTS      's'
#define FLAG_STMTS_END  'S'
#define FLAG_WRAP       'w'
#define FLAG_WRAP_PRE   'p'
#define FLAG_WRAP_END   'W'
#define FLAG_NET        'n'
#define FLAG_CONNECT    'c'

#define TYPE_CONNECT    1
#define TYPE_LINK       2
#define TYPE_SERIAL     3
#define TYPE_LS         4

#define MAX_STACK_SIZE  100

// errors
#define ERR_WARNING "warning"
#define ERR_ERROR   "error"

#define ERROR_UNDEF_ID      "%s: use of undeclared identifier '%s'"
#define ERROR_DUPLICATE_ID  "%s: redefinition of '%s'"
#define ERROR_BAD_MODE      "%s: conflicting modes of ports '%s' in '%s'(%d) and '%s'(%d) (line %d)"
#define ERROR_BAD_MODE_SIDE "%s: conflicting modes of side port '%s' in '%s'(%d)"
#define ERROR_NO_NET_CON    "%s: no port connection in serial combinition '%s(%d).%s(%d)'"
#define ERROR_NO_PORT_CON   "%s: port '%s' in '%s'(%s, %d) is not connected"
#define ERROR_UNDEF_PORT    "%s: use of undeclared port '%s' in '%s'(%d)"

#endif /* DEFINES_H */
