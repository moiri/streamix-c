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
 * @brief   Checkes whether port names match
 *
 * @param p1    pointer the a port of a virtual net
 * @param p2    pointer the a port of a virtual net
 * @return      true if ports can connect, false if not
 */
bool are_port_names_ok( virt_port_t*, virt_port_t* );

/**
 * @brief   Checkes whether port classes for normal connections match
 *
 * @param p1        pointer the a port of a virtual net
 * @param p2        pointer the a port of a virtual net
 * @param directed  flag to indicate wheter the direction needs to be taken
 *                  into account
 * @return          true if ports can connect, false if not
 */
bool are_port_classes_ok( virt_port_t*, virt_port_t*, bool );

/**
 * @brief   Checkes port classes for copy synchronizer connections
 *
 * @param p1    pointer the a port of a virtual net
 * @param p2    pointer the a port of a virtual net
 * @param cpp   flag to indicate wheter copy synchronizer connections in
 *              parallel operators are checked
 * @return      true if ports can connect, false if not
 */
bool are_port_cp_classes_ok( virt_port_t*, virt_port_t*, bool );

/**
 * @brief    Checkes whether port modes match
 *
 * @param p1    pointer the a port of a virtual net
 * @param p2    pointer the a port of a virtual net
 * @param equal if set to true, the function checks wheter modes are equal
 *              if set to false, the function checks wheter modes are differnt
 * @return      true if ports can connect, false if not
 */
bool are_port_modes_ok( virt_port_t*, virt_port_t*, bool );

/**
 * @brief    check port connections of two ports
 *
 * Check the connection of two ports from virtual nets and connect them.
 * The dependancy graph is updated.
 *
 * @param ports_l   pointer to the port of a virtual net of the left operator
 * @param ports_r   pointer to the port of a virtual net of the right operator
 * @param g         pointer to a initialized igraph object
 * @return          true if connection was ok, false if no connection
 */
bool check_connection( virt_port_t*, virt_port_t*, igraph_t* g, bool );

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
 * @param parallel  flag to indicate wheter cp syncs of paralle combinations are
 *                  checker (true) or side ports in serial combinations (false)
 */
void check_connection_cp( virt_net_t*, virt_port_t*, virt_port_t*, igraph_t*,
        bool, bool );

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
void check_connection_missing( virt_net_t*, virt_net_t*, igraph_t* );

/**
 * @brief    Check port connections of two nets and connect them
 *
 * @param v_net1    pointer to the virtual net of the left operator
 * @param v_net2    pointer to the virtual net of the right operator
 * @param g         pointer to a initialized igraph object
 */
void check_connections( virt_net_t*, virt_net_t*, igraph_t* );

/**
 * @brief   Connect copy synchronizers of two nets
 *
 * Establish copy synchronizer connections between two virtual nets. This
 * includes side ports as well as regular ports.
 *
 * @param v_net1    pointer to the virtual net
 * @param parallel  flag to indicate wheter copy synchronizer connections
 *                  in parallel operators are checked
 * @param g         pointer to a initialized igraph object
 */
void check_connections_cp( virt_net_t*, bool, igraph_t*, bool );

/**
 *
 */
virt_net_t* check_connections_wrap( virt_port_list_t*, virt_port_list_t* );

/**
 * @brief    Check the context of all identifiers in the program
 *
 * @param ast       pointer to the root ast node
 * @param symtab    pointer to the symbol table
 * @param g         pointer to an initialized igraph object
 */
void check_context( ast_node_t*, symrec_t**, igraph_t* );

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
void* check_context_ast( symrec_t**, UT_array*, ast_node_t* );

/**
 * @brief   Check whether types of a prototype and a net match
 *
 * @param r_ports   port list from a symbol record
 * @param v_net     a virtual net containing the port list from a net
 * @param name      name of the symbol
 * @return          true if prototype matches with net definition
 *                  false if there is no match
 */
bool check_prototype( symrec_list_t*, virt_net_t*, char* );

/**
 * @brief   check if all ports are connected
 *
 * @param v_net pointer to a virtual net
 */
void check_open_ports( virt_net_t* );

/**
 * @brief   check whether synchronizers have only outgoing or incoming ports
 *
 * @param g     pointer to the flattened dependancy graph
 * @param id    id of the node to check
 * @return      true if cp is valid, false otherwise
 */
bool check_single_mode_cp( igraph_t*, int );

/**
 * @brief   Connect two instances by a pirt of matching ports
 *
 * The dependancy graph is updated, port classes and states are set
 * according to the connection semantics
 *
 * @param port_l        pointer to a virtual port of the left instance
 * @param port_r        pointer to a virtual port of the right instance
 * @oaram g             pointer to the dependancy graph to update
 * @param connect_sync  if true, port state of synch-ports is updated.
 *                      this must be true when a serial connection is
 *                      performed
 */
void connect_ports( virt_port_t*, virt_port_t*, igraph_t*, bool );

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
 * @return      pointer to the merged copy synchronizer
 */
instrec_t* cpsync_merge( virt_port_t*, virt_port_t*, igraph_t* );

/**
 * @brief   update ports of a virtual net after merging two copy synchrpnizers
 *
 * @param port_l    pointer to the left virtual port
 * @param port_r    pointer to the right virtual port
 * @param cp_sync   pointer to the copy synchronizer instance
 * @param g         pointer to the dependency graph
 */
void cpsync_merge_ports( virt_port_t*, virt_port_t*, instrec_t*, igraph_t* );

/**
 * @brief   Replace cp syncs with only two pors by an edge
 *
 * @param g     pointer to the dependancy graph
 * @param id    id of the synchronizer to check
 * @return      true if the sync was replaced, false if not
 */
bool cpsync_reduce( igraph_t*, int );

/**
 * @brief   Print debug information of a port of a port record list
 *
 * @param port  pointer to the port record
 * @param name  name of the net instance to port belongs to
 */
void debug_print_rport( symrec_t*, char* );

/**
 * @brief   Print debug information of all ports in a port record list
 *
 * @param rports    pointer to the port record list
 * @param name      name of the net instance to port belongs to
 */
void debug_print_rports( symrec_list_t*, char* );

/**
 * @brief   Check whether each list has the same number of elements
 *
 * @param r_ports   port list from a symbol record
 * @param v_ports   port list from a virtual net
 * @return          true if port count matches, false if not
 */
bool do_port_cnts_match( symrec_list_t*, virt_port_list_t* );

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
bool do_port_attrs_match( symrec_list_t*, virt_port_list_t* );

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
 * @return              pointer to a virtual net with a port list and connection
 *                      vectors
 */
virt_net_t* install_nets( symrec_t**, UT_array*, ast_node_t*, igraph_t* );

/**
 * @brief   checks wheter two instances are connected
 *
 * @param rec1  a pointer to an instance record
 * @param rec2  a pointer to an instance record
 * @param g     pointer to an initialized igraph object
 * @return      true if there is a connection
 *              false if the instances are not connected
 */
bool is_connected( instrec_t*, instrec_t*, igraph_t* );

/**
 * @brief   perform post proecessing operations on the graph
 *
 * @param g pointer to the dependancy graph
 */
void post_process( igraph_t* );

#endif // CONTEXT_H
