/**
 * Convert streamix box descriptions into SIA graphs
 *
 * @file    smx2sia.h
 * @author  Simon Maurer
 *
 */

#ifndef SMX2SIA_H
#define SMX2SIA_H

#include "sia.h"
#include "igraph.h"
#include "symtab.h"

/**
 *
 */
void smx2sia( igraph_t*, sia_t** );

/**
 *
 */
void smx2sia_add_loops( igraph_t*, symrec_t* );

/**
 *
 */
void smx2sia_add_transition( igraph_t*, symrec_t*, int, int );

/**
 *
 */
sia_t* smx2sia_pure( symrec_list_t*, const char* );

/**
 *
 */
sia_t* smx2sia_state( symrec_list_t*, const char* );

#endif // SMX2SIA_H

