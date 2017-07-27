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
 * @param g             pointer to the streamix graph
 * @param smx_symbs     pointer to the SIA symbol table
 * @param desc_symbs    pointer to the user provided SIA symbol table
 */
void smx2sia( igraph_t* g, sia_t** smx_symbs, sia_t** desc_symbs );

/**
 * @brief Add a transition to the SIA
 *
 * @param g         pointer to the SIA graph
 * @param port      pointer to the port defining the action
 * @param id_src    source id of the transition
 * @param id_dst    destination id of the transition
 * @param vid       unique id ( vertex id of the dependency graph)
 */
void smx2sia_add_transition( igraph_t* g, virt_port_t* port, int id_src,
        int id_dst, int vid );

/**
 * @brief Set the streamix name of a SIA
 *
 * The streamix SIA name is composed out of the box name and the box
 * implmentation name: <box_name><infix><box_implementation_name>
 *
 * @param sia       pointer to the sia where the name is set
 * @param box_name  name of the box
 * @param impl_name name of the box implementation
 * @param id        unique id ( vertex id of the dependency graph)
 */
void smx2sia_set_name_box( sia_t* sia, const char* box_name,
        const char* impl_name, int id );

/**
 * @brief Generate a SIA for a stateful box
 *
 * The order of the ports is assumed to be the order in which data is read from
 * ports and written to them. The generated SIA is deterministic and a circle
 * graph.
 *
 * @param ports_rec pointer to the list of ports of a net
 * @param vid               unique id ( vertex id of the dependency graph)
 * @return sia_t*           a new SIA structure
 */
sia_t* smx2sia_state( virt_port_list_t* ports_rec, int vid );

/**
 * @brief destroy all sia structures and its corresponding sub structures
 *
 * @param sias          pointer to the list of sias
 * @param desc_symbols  pointer to the user symbol table of sias
 * @param smx_symbols   pointer to the streamix symbol table of sias
 */
void smx2sia_sias_destroy( sias_t* sias, sia_t** desc_symbols,
        sia_t** smx_symbols );

/**
 * @brief Write out the graph files of the sias
 *
 * @param symbols   pointer to the symbol table of sias
 * @param out_path  output path where the files will be stored
 * @param format    format string, either 'gml' or 'graphml'
 */
void smx2sia_sias_write( sia_t** symbols, const char* out_path,
        const char* format );

/**
 * @brief Check and update user defined SIAs
 *
 * If a user defined SIA has actions that do not match the signature an error
 * is produced. A user defined SIA is altered as follows:
 *  - decoupled ports are added as self loops to each vertex
 *  - the SIA name is changed to <box_name><infix><box_implementation_name>
 *  - the action names are changed to <port_name><infix><edge_id>
 *
 * @param g         pointer to the user defined SIA graph
 * @param ports_rec pointer to the signature of the box
 * @param vid       unique id ( vertex id of the dependency graph)
 */
void smx2sia_update( igraph_t* g, virt_port_list_t* ports_rec, int vid );
#endif // SMX2SIA_H

