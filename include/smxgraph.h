/**
 * Small library to handle the dependancy graph according to the semantics of
 * streamix
 *
 * @file    smxgraph.h
 * @author  Simon Maurer
 *
 */

#ifndef NGRAPH_H
#define NGRAPH_H

#include <igraph.h>
#include "vnet.h"
#include "symtab.h"

int dgraph_find_bp_port( igraph_vector_ptr_t*, const char* );
/**
 *
 * @param g
 * @param sps_src   symrec port list of the net prototype
 * @param vps_net   virt port list of the network equations in the wrapper
 */
virt_port_list_t* dgraph_merge_port_net( igraph_t*, symrec_list_t*,
        virt_port_list_t* );
virt_port_list_t* dgraph_merge_port_wrap( igraph_t*, symrec_list_t*,
        virt_port_list_t* );

/**
 * @brief   Append a graph to another graph
 *
 * Append all elements to another graph. A deep or a shallow copy of the graph
 * attributes can be done
 *
 * @param g     a pointer to the destination garph object
 * @param g_tpl a pointer to the source graph object
 * @param deep  if true a deep copy of the attributes is done
 *              if false a shallow copy of the attributes is done
 */
void dgraph_append( igraph_t*, igraph_t*, bool );

/**
 * @brief   Add an edge to the dependancy graph
 *
 * @param g         pointer to the dependancy graph
 * @param p_src     pointer to the source port
 * @param p_dest    pointer to the destination port
 * @param name      name of the edge to add
 * @return          id of the new edge
 */
int dgraph_edge_add( igraph_t*, virt_port_t*, virt_port_t*, const char* );

/**
 * @brief   destroy the vertex attributes of a graph
 *
 * @param g graph where the attributes will be destroyed
 */
void dgraph_destroy_attr( igraph_t* );
void dgraph_destroy_attr_v( igraph_t*, const char* );
void dgraph_destroy_attr_e( igraph_t*, const char* );

/**
 * @brief   Flatten a hierarchical graph
 *
 * Flatten a hierarchical graph by creating new instances of each occurence
 * and reconnect them in the lowest herarchical level. This is arecursive
 * function
 *
 * @param g_new An initialised empty graph object. This will hold the flattend
 *              graph
 * @param g     The graph to be flattened. Use a deep copy of a net graph here
 */
void dgraph_flatten( igraph_t*, igraph_t* );

/**
 * @brief   Helper function to flatten the graph
 *
 * This function replaces a net instance with its child graph and connects
 * the corresponding ports
 *
 * @param g_new     resulting graph
 * @param g_child   graph of the net instance
 * @param net_id    id of the net instance
 */
void dgraph_flatten_net( igraph_t*, igraph_t*, symrec_t*, instrec_t* );

/**
 * @brief   Search an equivalent port in a similar virtual net
 *
 * @param g         the graph where the port template resides
 * @param g_new     the graph where the target port resides
 * @param id_edge   edge id of the port template in the initial graph
 * @param id_inst   vertex id of the port template in the initial graph
 * @param attr      PORT_ATTR_PSRC or PORT_ATTR_PDST
 * @return          a pointer to the equivalent port or NULL
 */
virt_port_t* dgraph_port_search_neighbour( igraph_t*, igraph_t*, int, int,
        const char* );

/**
 * @brief   Find a port in a child graph of a net
 *
 * @param g     child graph of a net
 * @param port  port template to be found
 */
virt_port_t* dgraph_port_search_child( igraph_t*, virt_port_t*, bool );
virt_port_t* dgraph_port_search_wrap( virt_net_t*, virt_port_t* );

/**
 * @brief   Add a vertex to a graph
 *
 * @param g     pointer to the graph where the vertex will be added
 * @param name  name of the vertex to be added
 * @return      id of the added vertex
 */
int dgraph_vertex_add( igraph_t*, const char* );

/**
 * @brief   Add attributes to a vertex
 *
 * @param g     graph where the vertex resides
 * @param int   id of the vertex
 * @param func  pointer to the implementation name or NULL
 * @param symb  pointer to the symbol or NULL
 * @param inst  pointer to the instance or NULL
 * @param v_net pointer to the virtual net or NULL
 * @param g_net pointer to teh graph or NULL
 */
void dgraph_vertex_add_attr( igraph_t*, int, const char*, symrec_t*, instrec_t*,
        virt_net_t*, igraph_t* );

/**
 * @brief   Create a box instance and add it to the graph
 *
 * @param g     graph where the vertex will be added
 * @param symb  pointer to the box symbol
 * @param line  the line number of the instance
 * @return      the virtual net of the box instance
 */
virt_net_t* dgraph_vertex_add_box( igraph_t*, symrec_t*, int );

/**
 * @brief   Create a net instance and add it to the graph
 *
 * @param g     graph where the vertex will be added
 * @param symb  pointer to the net symbol
 * @param line  the line number of the instance
 * @return      the virtual net of the net instance
 */
virt_net_t* dgraph_vertex_add_net( igraph_t*, symrec_t*, int );

/**
 * @brief   Create a synchronizer instance and add it to the graph
 *
 * @param g     graph where the vertex will be added
 * @param port  pointer to a virtual port
 * @return      the virtual net of the synchronizer instance
 */
virt_net_t* dgraph_vertex_add_sync( igraph_t*, virt_port_t* );

/**
 * @brief   Create a wrapper instance and add it to the graph
 *
 * @param g     graph where the vertex will be added
 * @param symb  pointer to the wrapper symbol
 * @param line  the line number of the instance
 * @return      the virtual net of the wrapper instance
 */
virt_net_t* dgraph_vertex_add_wrap( igraph_t*, symrec_t*, int );

/**
 * @brief   Copy the vertex of a graph to another graph
 *
 * @param g_src     pointer to the sorce graph
 * @param g_dest    pointer to the destination graph
 * @param id        id of the vertec to copy
 * @param deep      if true a deep copy of the attributes is done
 *                  if false a shallow copy of the attributes is done
 */
instrec_t* dgraph_vertex_copy( igraph_t*, igraph_t*, int, bool );

/**
 * @brief   destroy the instance and the virtual net attributes of a vertex
 *
 * @param g     graph object containing the vertex
 * @param id    id of the vertex
 * @param deep  if true destroy all ports of the virtual net
 *              if false only destroy the port list structure
 */
void dgraph_vertex_destroy_attr( igraph_t*, int, bool );

/**
 * @brief   Merge two vertices into one
 *
 * @param g     pointer to the dependancy graph
 * @param id1   id of a vertex
 * @param id2   id of a vertex
 * @return      id of the removed vertex
 */
int dgraph_vertex_merge( igraph_t*, int, int );

/**
 * @brief   decrement the instance id of all vertices in a graph
 *
 * @param g         pointer to the graph
 * @param id_start  for this and all following vertices the id of the
 *                  corresponding instance is decremented by 1
 */
void dgraph_vertex_update_ids( igraph_t*, int );

/**
 * @brief   Convert a vector of instance pointers to a vector of its ids
 *
 * @param vptr  vector to be converted
 * @param v     pointer to an initialised vector of size 0. This vector will be
 *              resized and will hold the ids of teh instances
 */
void dgraph_vptr_to_v( igraph_vector_ptr_t*, igraph_vector_t* );

#endif // NGRAPH_H
