/**
 * Global definitions
 *
 * @file    defines.h
 * @author  Simon Maurer
 *
 */

#ifndef DEFINES_H
#define DEFINES_H

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
#define CONST_ERROR_LEN 256
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
    PARSE_ATTR_STATELESS,
    PARSE_ATTR_STATIC,
    PARSE_ATTR_BOX,
    PARSE_ATTR_WRAP,
    PARSE_ATTR_NET
};

#define TEXT_CP      "smx_cp"
#define TEXT_NULL    "null"

#define INST_ATTR_LABEL     "label"
#define INST_ATTR_FUNC      "func"
#define INST_ATTR_SYMB      "symb"
#define INST_ATTR_VNET      "vnet"
#define INST_ATTR_GRAPH     "igraph"
#define INST_ATTR_STATIC    "static"
#define INST_ATTR_PURE      "pure"

#define PORT_ATTR_LABEL "label"
#define PORT_ATTR_PSRC  "psrc"
#define PORT_ATTR_PDST  "pdst"
#define PORT_ATTR_DSRC  "dsrc"
#define PORT_ATTR_DDST  "ddst"

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

// errors
#define ERR_WARNING "warning"
#define ERR_ERROR   "error"

#define ERROR_UNDEF_ID      "%s: use of undeclared identifier '%s'"
#define ERROR_DUPLICATE_ID  "%s: redefinition of '%s'"
#define ERROR_BAD_MODE      "%s: conflicting modes of ports '%s' in '%s'(%d) and '%s'(%d) (line %d)"
#define ERROR_BAD_MODE_SIDE "%s: conflicting modes of side port '%s' in '%s'(%d)"
#define ERROR_NO_NET_CON    "%s: no port connection in serial combinition '%s(%d).%s(%d)'"
#define ERROR_NO_PORT_CON   "%s: port '%s' in '%s'(%d) is not connected"
#define ERROR_UNDEF_PORT    "%s: use of undeclared port '%s' in '%s'(%d)"
#define ERROR_UNDEF_NET     "%s: undefined reference to net '%s'"
#define ERROR_TYPE_CONFLICT "%s: conflicting types for '%s'"
#define ERROR_SMODE_CP      "%s: single mode in synchroniser '%s'(%d)"

#endif /* DEFINES_H */
