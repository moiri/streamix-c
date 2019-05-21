/**
 * Graph label definitions
 *
 * @file    graph_defines.h
 * @author  Simon Maurer
 *
 */

#ifndef GRAPH_DEFINES_H
#define GRAPH_DEFINES_H

#define TEXT_CH     "smx_ch"
#define TEXT_CP     "smx_rn"
#define TEXT_TF     "smx_tf"
#define TEXT_NULL   "smx_null"

#define GG_NAME      "name"
#define GG_SIA       "sia"

#define GV_GRAPH    "igraph"
#define GV_IMPL     "func"
#define GV_LABEL    "label"
#define GV_PURE     "pure"
#define GV_SIA      "sia"
#define GV_STATIC   "static"
#define GV_SYMB     "symb"
#define GV_VNET     "vnet"
#define GV_TT       "tt"

#define GE_DSRC     "dsrc"
#define GE_DDST     "ddst"
#define GE_LABEL    "label"
#define GE_LEN      "len"
#define GE_NSRC     "nsrc"
#define GE_NDST     "ndst"
#define GE_PSRC     "psrc"
#define GE_PDST     "pdst"
#define GE_SIA      "sia"
#define GE_STS      "sts"
#define GE_STNS     "stns"
#define GE_DTS      "dts"
#define GE_DTNS     "dtns"
#define GE_TYPE     "type"

enum rate_type_e {
    TIME_NONE,
    TIME_TT,
    TIME_TB,
};

#endif /* GRAPH_DEFINES_H */
