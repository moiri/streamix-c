/**
 * Checks the context of symbols and connections
 *
 * @file    context.h
 * @author  Simon Maurer
 *
 */

#ifndef CONTEXT_H
#define CONTEXT_H

#include <stdbool.h>
#include <igraph.h>
#include "ast.h"
#include "vnet.h"
#include "insttab.h"
#include "symtab.h"
#include "utarray.h"

/**
 * Spawns a profiler routing node and connects it to the profiler backend.
 *
 * @param g     pointer to the dependency graph
 */
void add_profiler_connections( igraph_t* g );

/**
 * @brief    check port connections of two ports
 *
 * Check the connection of two ports from virtual nets and connect them.
 * The dependancy graph is updated.
 *
 * @param ports_l   pointer to the port of a virtual net of the left operator
 * @param ports_r   pointer to the port of a virtual net of the right operator
 * @param g         pointer to a initialized igraph object
 * @param directed  if true port_l is assumed to be left, port_r is assumed to
 *                  be right. If false no assumption is taken
 * @param ignore_class  if true, the port classes are ignored
 * @param mode_equal    if true, port modes have to be equal
 *                      if false, port modes have to be different
 * @return          true if connection was ok, false if no connection
 */
bool check_connection( virt_port_t* ports_l, virt_port_t* ports_r, igraph_t* g,
        bool directed, bool ignore_class, bool mode_equal );

/**
 * @brief   Connect two ports of copy synchronizers
 *
 * Establish copy synchronizer connections between two ports of two virtual nets.
 * This includes side ports as well as regular ports. The dependancy graph is
 * updated.
 *
 * @param net       pointer to the virtuel net
 * @param port1     pointer to the port of a virtual net of the left operator
 * @param port2     pointer to the port of a virtual net of the right operator
 * @param g         pointer to a the dependancy graph to be updated
 * @param parallel  flag to indicate wheter cp syncs of parallel combinations
 *                  are checked (AST_PARALLEL/AST_PARALLEL_DET) or side ports
 *                  in serial combinations (0)
 * @param is_tt     true if the net is time-triggered, false otherwise
 */
void check_connection_cp( virt_net_t* net, virt_port_t* port1,
        virt_port_t* port2, igraph_t* g, node_type_t parallel, bool is_tt );

/**
 * @brief    Report missing connections
 *
 * Reports errors if two nets are not connected but should be according to
 * Streamix operator semantics
 *
 * @param v_net_l   pointer to the virtual net of the left operand
 * @param v_net_r   pointer to the virtual net of the right operand
 * @param g         pointer to a initialized igraph object
 */
void check_connection_missing( virt_net_t* v_net_l, virt_net_t* v_net_r,
        igraph_t* g );

/**
 * Check whether a port is declared as open in a connections of two ports.
 *
 * @param ports_l   pointer to the port of a virtual net of the left operator
 * @param ports_r   pointer to the port of a virtual net of the right operator
 * @return          true if connection was ok, false if no connection
 */
bool check_connection_on_open_ports( virt_port_t* port_l, virt_port_t* port_r );

/**
 * @brief    Check port connections of two nets and connect them
 *
 * @param v_net1    pointer to the virtual net of the left operator
 * @param v_net2    pointer to the virtual net of the right operator
 * @param g         pointer to a initialized igraph object
 */
void check_connections( virt_net_t* v_net1, virt_net_t* v_net2, igraph_t* g );

/**
 * @brief   Connect copy synchronizers of two nets
 *
 * Establish copy synchronizer connections between two virtual nets. This
 * includes side ports as well as regular ports.
 *
 * @param v_net1    pointer to the virtual net
 * @param g         pointer to a initialized igraph object
 * @param parallel  flag to indicate wheter copy synchronizer connections
 *                  in parallel operators are checked
 * @param is_tt     true if the net is time-triggered, false otherwise
 */
void check_connections_cp( virt_net_t* v_net1, igraph_t* g,
        node_type_t parallel, bool is_tt );

/**
 * @brief Check for open connections
 *
 * If connections are open after a locality binding serial connection
 * (operator '.') and have a class asigned that should have connected, throw
 * an error message
 *
 * @param vnet_l    pointer to the vnet of the left operator
 * @param vnet_r    pointer to the vnet of the right operator
 */
void check_connections_open( virt_net_t* vnet_l, virt_net_t* vnet_r );

/**
 * @brief Connect self loops of a box or wrapper
 *
 * @param g     pointer to the net graph
 * @param v_net pointer to the virtual net where self loops must be connected
 */
void check_connections_self( igraph_t* g, virt_net_t* v_net );

/**
 * @brief    Check the context of all identifiers in the program
 *
 * @param ast       pointer to the root ast node
 * @param symtab    pointer to the symbol table
 * @param g         pointer to an initialized igraph object
 */
void check_context( ast_node_t* ast, symrec_t** symtab, igraph_t* g );

/**
 * @brief    Step wise context checker
 *
 * Check the context of all identifiers in the program by iterating through the
 * AST. This function is recursive.
 *
 * @param symtab        pointer to the symbol table
 * @param scope_stack   pointer to the scope stack
 * @param ast           pointer to the ast node
 * @return              pointer to a symrec net attribute (cast to void*)
 */
void* check_context_ast( symrec_t** symtab, UT_array* scope_stack,
        ast_node_t* ast );

/**
 * @brief   check if a net has at least one triggering input
 *
 * @param ports pointer to a symbol port list
 */
void check_ports_decoupled( symrec_list_t* ports );

/**
 * @brief   check if all ports are connected
 *
 * @param v_net pointer to a virtual net
 */
void check_ports_open( virt_net_t* v_net );

/**
 * @brief   Check whether types of a prototype and a net match
 *
 * @param r_ports   port list from a symbol record
 * @param v_net     a virtual net containing the port list from a net
 * @param name      name of the symbol
 * @return          true if prototype matches with net definition
 *                  false if there is no match
 */
bool check_prototype( symrec_list_t* r_ports, virt_net_t* v_net, char* name );

/**
 * @brief   check whether synchronizers have only outgoing or incoming ports
 *
 * @param g     pointer to the flattened dependancy graph
 * @param id    id of the node to check
 * @return      true if cp is valid, false otherwise
 */
bool check_single_mode_cp( igraph_t* g, int id );

/**
 * @brief   Connect two instances by a pirt of matching ports
 *
 * The dependancy graph is updated, port classes and states are set
 * according to the connection semantics
 *
 * @param port_l        pointer to a virtual port of the left instance
 * @param port_r        pointer to a virtual port of the right instance
 * @param g             pointer to the dependancy graph to update
 * @param connect_sync  if true, port state of synch-ports is updated.
 *                      this must be true when a serial connection is
 *                      performed
 */
void connect_ports( virt_port_t* port_l, virt_port_t* port_r, igraph_t* g,
        bool connect_sync );

/**
 * @brief   Merge two copy synchronizer
 *
 * This merges two copy synchronizer by merging two graph vertices and
 * consucutevly updating alle changes ids. It also removes the obsolete
 * instance of the cp sync.
 *
 * @param port1 pointer to the port of a virtual net
 * @param port2 pointer to the port of a virtual net
 * @param g     pointer to a initialized igraph object
 */
void cpsync_merge( virt_port_t* port1, virt_port_t* port2, igraph_t* g );

/**
 * @brief   update ports of a virtual net after merging two copy synchrpnizers
 *
 * @param port_l    pointer to the left virtual port
 * @param port_r    pointer to the right virtual port
 * @param cp_sync   pointer to the copy synchronizer instance
 * @param g         pointer to the dependency graph
 */
void cpsync_merge_ports( virt_port_t* port_l, virt_port_t* port_r,
        instrec_t* cp_sync, igraph_t* g );

/**
 * @brief   Replace cp syncs with only two pors by an edge
 *
 * @param g     pointer to the dependancy graph
 * @param id    id of the synchronizer to check
 * @return      true if the sync was replaced, false if not
 */
bool cpsync_reduce( igraph_t* g, int id );

/**
 * @brief   Check whether each list has the same number of elements
 *
 * @param r_ports   port list from a symbol record
 * @param v_ports   port list from a virtual net
 * @return          true if port count matches, false if not
 */
bool do_port_cnts_match( symrec_list_t* r_ports, virt_port_list_t* v_ports );

/**
 * @brief   Check whether port attributes from two port lists match
 *
 * Check whether each element on one list has a matching element in the
 * other list
 *
 * @param r_ports   port list from a symbol record
 * @param v_ports   port list from a net
 * @return          true if port attributes match, false if not
 */
bool do_port_attrs_match( symrec_list_t* r_ports, virt_port_list_t* v_ports );

/**
 * @brief   Install instances to the instance table and the graph
 *
 * This function performs the following tasks
 * - check whether the given identificator is in the symbol table
 * - add instances to the instance table
 * - add instances to the dependency graph
 * This is a recursive function.
 *
 * @param symtab        pointer to the symbol table
 * @param scope_stack   pointer to the scope stack
 * @param ast           pointer to the ast node
 * @param g             pointer to an initialized igraph object
 * @param is_tt         true if the net is time-triggered, false otherwise
 * @return              pointer to a virtual net with a port list and connection
 *                      vectors
 */
virt_net_t* install_nets( symrec_t** symtab, UT_array* scope_stack,
        ast_node_t* ast, igraph_t* g, bool is_tt );

/**
 * @brief   checks wheter two instances are connected
 *
 * @param rec1  a pointer to an instance record
 * @param rec2  a pointer to an instance record
 * @param g     pointer to an initialized igraph object
 * @return      true if there is a connection
 *              false if the instances are not connected
 */
bool is_connected( instrec_t* rec1, instrec_t* rec2, igraph_t* g );

/**
 * @brief   perform post proecessing operations on the graph
 *
 * @param g pointer to the dependancy graph
 */
void post_process( igraph_t* g );

#endif // CONTEXT_H
