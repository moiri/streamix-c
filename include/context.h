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
 * @brief   append id from instances to a initialised vector
 *
 * This function either appends the instance id to the vector or if the
 * instance has child instances it collects all ids of the chlidren.
 *
 * @param rec   source instance record
 * @param id    an initialised vector where the ids are appended
 */
void append_inst_ids( inst_rec_t*, igraph_vector_t* );

/**
 * @brief   Checkes whether port names match
 *
 * The port classes (collections) are taken into account to check the name
 * matching
 *
 * @param p1    pointer the a port of a virtual net
 * @param p2    pointer the a port of a virtual net
 * @param cpp   flag to indicate wheter copy synchronizer connections in
 *              parallel operators are checked
 * @param cps   flag to indicate wheter copy synchronizer connections in
 *              serial operators are checked
 * @return      true if ports can connect, false if not
 */
bool are_port_names_ok( virt_port_t*, virt_port_t*, bool, bool );

/**
 * @brief    Checkes whether port modes match
 *
 * @param p1    pointer the a port of a virtual net
 * @param p2    pointer the a port of a virtual net
 * @return      true if ports can connect, false if not
 */
bool are_port_modes_ok( virt_port_t*, virt_port_t* );

/**
 * @brief    check port connections of two ports
 *
 * Check the connection of two ports from virtual nets and connect them.
 * The dependancy graph is updated.
 *
 * @param net       pointer to the instance of a net
 * @param ports_l   pointer to the port of a virtual net of the left operator
 * @param ports_r   pointer to the port of a virtual net of the right operator
 * @param g         pointer to a initialized igraph object
 * @return          true if connection was ok, false if no connection
 */
bool check_connection( inst_net_t*, virt_port_t*, virt_port_t*, igraph_t* g );

/**
 * @brief    check port connections of two nets
 *
 * Check the connection of two virtual nets and connect them. Connecting ports
 * are removed from the virtual nets.
 *
 * @param net       pointer to the instance of a net
 * @param v_net1    pointer to the virtual net of the left operator
 * @param v_net2    pointer to the virtual net of the right operator
 * @param g         pointer to a initialized igraph object
 */
void check_connections( inst_net_t*, virt_net_t*, virt_net_t*, igraph_t* );

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
 * @brief    Check the context of all identifiers in the program
 *
 * @param ast   pointer to the root ast node
 * @param nets  pointer to the instance table of nets (scopes)
 * @param g     pointer to a initialized igraph object
 */
void check_context( ast_node_t*, inst_net_t**, igraph_t* );

/**
 * @brief    Step wise context checker
 *
 * Check the context of all identifiers in the program by iterating through the
 * AST. This function is recursive.
 *
 * @param symtab        pointer to the symbol table
 * @param nets          pointer to the instance table of nets (scopes)
 * @param scope_stack   pointer to the scope stack
 * @param ast           pointer to the ast node
 * @param g             pointer to a initialized igraph object
 * @return              pointer to a symrec net attribute (cast to void*)
 */
void* check_context_ast( symrec_t**, inst_net_t**, UT_array*, ast_node_t*,
        igraph_t* );

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
 * @brief   Connect two ports of copy synchronizers
 *
 * Establish copy synchronizer connections between two ports of two virtual nets.
 * This includes side ports as well as regular ports. The dependancy graph is
 * updated.
 *
 * @param net   pointer to the instance a of net
 * @param port1 pointer to the port of a virtual net of the left operator
 * @param port2 pointer to the port of a virtual net of the right operator
 * @param g     pointer to a initialized igraph object
 */
void cpsync_connect( inst_net_t*, virt_port_t*, virt_port_t*, igraph_t* );

/**
 * @brief   Connect copy synchronizers of two nets
 *
 * Establish copy synchronizer connections between two virtual nets. This
 * includes side ports as well as regular ports.
 *
 * @param net       pointer to the instance of a net
 * @param v_net1    pointer to the virtual net of the left operator
 * @param v_net2    pointer to the virtual net of the right operator
 * @param parallel  flag to indicate wheter copy synchronizer connections
 *                  in parallel operators are checked
 * @param g         pointer to a initialized igraph object
 */
void cpsync_connects( inst_net_t*, virt_net_t*, virt_net_t*, bool, igraph_t* );

/**
 * @brief   Merge two copy sunchronizer
 *
 * @param net   pointer to the instance of a net
 * @param port1 pointer to the port of a virtual net
 * @param port2 pointer to the port of a virtual net
 * @param g     pointer to a initialized igraph object
 * @return      pointer to the merged copy synchronizer
 */
inst_rec_t* cpsync_merge( inst_net_t*, virt_port_t*, virt_port_t*, igraph_t* );

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
 * @brief   Print debug information of a port of a virtual net
 *
 * @param port  pointer to the port of a virtual net
 */
void debug_print_vport( virt_port_t* );

/**
 * @brief   Print debug information of all ports in a virtual net
 *
 * @param v_net pointer to the virtual net
 */
void debug_print_vports( virt_net_t* );

/**
 * @brief   Check whether each list has the same number of elements
 *
 * @param r_ports   port list from a symbol record
 * @param v_ports   port list from a virtual net
 * @return          true if port count matches, false if not
 */
bool do_port_cnts_match( symrec_list_t*, virt_port_t* );

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
bool do_port_attrs_match( symrec_list_t*, virt_port_t* );

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
 * @param net           pointer to the instance table of nets (scopes)
 * @param scope_stack   pointer to the scope stack
 * @param ast           pointer to the ast node
 * @param g             pointer to a initialized igraph object
 * @param g_con         pointer to a initialized igraph object (connection graph)
 * @return              pointer to a virtual net with a port list and connection
 *                      vectors
 */
virt_net_t* install_nets( symrec_t**, inst_net_t*, UT_array*, ast_node_t*,
        igraph_t*, igraph_t* );

/**
 * @brief   checks wheter two instances are connected
 *
 * @param rec1  a pointer to an instance record
 * @param rec2  a pointer to an instance record
 * @param g     pointer to an initialized igraph object
 * @return      true if there is a connection
 *              false if the instances are not connected
 */
bool is_connected( inst_rec_t*, inst_rec_t*, igraph_t* );

#endif // CONTEXT_H
