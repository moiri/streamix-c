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
#include "vnet.h"

/**
 * @brief Generate SIAs for all boxes
 *
 * For each box where not already a SIA definition exists, a new SIA is
 * generated. Each SIA is named as <box_name><infix><box_implementation_name>.
 * Actions are named as <port_name><infix><edge_id> to avoid name conflicts.
 * The port mode defines the action mode: MODE_IN -> '?' and MODE_OUT -> '!'.
 *
 * I there exists alreadt a SIA definition for a box, the definition is checked
 * and altered according to the box declarationr.
 *
 * @param igraph_t*     pointer to the streamix graph
 * @param sia_t**       pointer to the SIA symbol table
 * @param sia_t**       pointer to the user provided SIA symbol table
 */
void smx2sia( igraph_t*, sia_t**, sia_t** );

/**
 * @brief Add a transition to the SIA
 *
 * @param igraph_t*     pointer to the SIA graph
 * @param virt_port_t*  pointer to the port defining the action
 * @param int           source id of the transition
 * @param int           destination id of the transition
 * @param int           unique id ( vertex id of the dependency graph)
 */
void smx2sia_add_transition( igraph_t*, virt_port_t*, int, int, int );

/**
 *
 */
sia_t* smx2sia_pure( symrec_list_t*, const char* );

/**
 * @brief Set the streamix name of a SIA
 *
 * The streamix SIA name is composed out of the box name and the box
 * implmentation name: <box_name><infix><box_implementation_name>
 *
 * @param sia_t*        pointer to the sia where the name is set
 * @param const char*   name of the box
 * @param const char*   name of the box implementation
 * @param int           unique id ( vertex id of the dependency graph)
 */
void smx2sia_set_name_box( sia_t*, const char*, const char*, int );

/**
 * @brief Generate a SIA for a stateful box
 *
 * The order of the ports is assumed to be the order in which data is read from
 * ports and written to them. The generated SIA is deterministic and a circle
 * graph.
 *
 * @param virt_port_list_t* pointer to the list of ports of a net
 * @param int               unique id ( vertex id of the dependency graph)
 * @return sia_t*           a new SIA structure
 */
sia_t* smx2sia_state( virt_port_list_t*, int );

/**
 * @brief destroy all sia structures and its corresponding sub structures
 *
 * @param sias_t*   pointer to the list of sias
 * @param sia_t**   pointer to the user symbol table of sias
 * @param sia_t**   pointer to the streamix symbol table of sias
 */
void smx2sia_sias_destroy( sias_t*, sia_t**, sia_t** );

/**
 * @brief Write out the graph files of the sias
 *
 * @param sia_t**       pointer to the symbol table of sias
 * @param const char*   output path where the files will be stored
 * @param const char*   format string, either 'gml' or 'graphml'
 */
void smx2sia_sias_write( sia_t**, const char*, const char* );

/**
 * @brief Check and update user defined SIAs
 *
 * If a user defined SIA has actions that do not match the signature an error
 * is produced. A user defined SIA is altered as follows:
 *  - decoupled ports are added as self loops to each vertex
 *  - the SIA name is changed to <box_name><infix><box_implementation_name>
 *  - the action names are changed to <port_name><infix><edge_id>
 *
 * @param igraph_t*         pointer to the user defined SIA graph
 * @param virt_port_list_t* pointer to the signature of the box
 * @param int               unique id ( vertex id of the dependency graph)
 */
void smx2sia_update( igraph_t*, virt_port_list_t*, int );
#endif // SMX2SIA_H

