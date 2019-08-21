/**
 * Global definitions
 *
 * @file    defines.h
 * @author  Simon Maurer
 *
 */

#ifndef DEFINES_H
#define DEFINES_H

#include "graph_defines.h"

#ifndef MAKE_TEST
/* #define DEBUG */
/* #define DEBUG_INST */
/* #define DEBUG_SYMB */
/* #define DEBUG_VNET */
/* #define DEBUG_CONNECT */
/* #define DEBUG_CONNECT_GRAPH */
/* #define DEBUG_CONNECT_MISSING */
/* #define DEBUG_CONNECT_WRAP */
/* #define DEBUG_PROTO */
/* #define DEBUG_NET_DOT */
/* #define DEBUG_NET_GML */
/* #define DEBUG_FLATTEN_GRAPH */
/* #define DEBUG_SEARCH_PORT */
/* #define DEBUG_SEARCH_PORT_WRAP */
/* #define DEBUG_SEARCH_PORT_CHILD */
/* #define DEBUG_LINK_DOT */
#endif

// constants
#define CONST_SCOPE_LEN 9
#define CONST_ID_LEN 9

/**
 * @brief   Port collections
 */
enum port_class_e
{
    PORT_CLASS_NONE,
    PORT_CLASS_UP,
    PORT_CLASS_DOWN,
    PORT_CLASS_SIDE
};

/**
 * @brief   Port modes
 */
enum port_mode_e
{
    PORT_MODE_IN,
    PORT_MODE_OUT,
    PORT_MODE_BI
};

/**
 * @brief   all kind of attributes used in the scanner
 */
enum parse_attr_e
{
    PARSE_ATTR_DECOUPLED,
    PARSE_ATTR_COUPLED,
    PARSE_ATTR_STATELESS,
    PARSE_ATTR_STATIC,
    PARSE_ATTR_EXTERN,
    PARSE_ATTR_OPEN,
    PARSE_ATTR_BOX,
    PARSE_ATTR_WRAP,
    PARSE_ATTR_NET
};

#define SIA_BOX_INFIX   "_"
#define SIA_PORT_INFIX  "_"

#define PORT_CLASS_NONE_STR "none"
#define PORT_CLASS_SIDE_STR "side"
#define PORT_CLASS_UP_STR   "up"
#define PORT_CLASS_DOWN_STR "down"

// drawing
#define DOT_FOLDER      "dot"
#define AST_DOT_PATH    DOT_FOLDER "/ast_graph.dot"
#define N_CON_DOT_PATH  DOT_FOLDER "/net_connection_graph.dot"
#define P_CON_DOT_PATH  DOT_FOLDER "/port_connection_graph.dot"
#define TEMP_DOT_PATH   DOT_FOLDER "/graph.dot.tmp"
#define DOT_PATTERN     "// ===>"

#define SHAPE_BOX       "box"
#define SHAPE_ELLIPSE   "ellipse"
#define SHAPE_OCTAGON   "octagon"
#define SHAPE_CIRCLE    "circle"

#define COLOR_SIDE      "goldenrod3"
#define COLOR_PARALLEL  "chartreuse3"
#define COLOR_SERIAL    "cadetblue3"
#define COLOR_LINK      "deepskyblue4"
#define COLOR_SLINK     COLOR_SIDE      // a link that is a side port
#define COLOR_N_BOX     "black"         // net with a box declaration
#define COLOR_N_WRAP    COLOR_LINK      // net with a net declaration
#define COLOR_WRAP      COLOR_LINK      // net declaration


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
#define STYLE_E_LSPORT      27

// styles of nodes
#define STYLE_N_DEFAULT     30
#define STYLE_N_NET_BOX     31
#define STYLE_N_NET_WRAP    32
#define STYLE_N_NET_CP      33
#define STYLE_N_NET_CPS     34
#define STYLE_N_NET_CPL     35
#define STYLE_N_NET_CPLS    36
#define STYLE_N_NET_INVIS   37
#define STYLE_N_AST_ATTR    38
#define STYLE_N_AST_NODE    39
#define STYLE_N_AST_ID      40

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

#endif /* DEFINES_H */
