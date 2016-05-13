/* 
 * Checks the context of symbols and connections
 *
 * @file    symtab.h
 * @author  Simon Maurer
 *
 * */


#ifndef CONTEXT_H
#define CONTEXT_H

#include <stdbool.h>
#include <igraph.h>
#include "ast.h"
#include "insttab.h"
#include "symtab.h"
#include "utarray.h"

/**
 * Check the connection of two virtual nets and connect them. Connecting ports
 * are removed from the virtual nets. The dependancy graph is updated.
 *
 * @param inst_net**:   pointer to the instance table of nets (scopes)
 * @param virt_net*:    pointer to the virtual net of the left operator
 * @param virt_net*:    pointer to the virtual net of the right operator
 * @param igraph_t*:    pointer to a temporary graph indicating the required
 *                      connections between nets
 */
void check_connection( inst_net*, virt_net*, virt_net*, igraph_t* );

/**
 * Check the context of all identifiers in the program
 *
 * @param ast_node*:    pointer to the root ast node
 * */
void check_context( ast_node* );

/**
 * Check the context of all identifiers in the program by iterating through the
 * AST. This function is recursive.
 *
 * @param symrec**:     pointer to the symbol table
 * @param inst_net**:   pointer to the instance table of nets (scopes)
 * @param UT_array**:   pointer to the scope stack
 * @param ast_node*:    pointer to the ast node
 * @param bool:         flag to help identify synchronized ports
 * @return void*:       a generic pointer to pass information down the
 *                      recursive function
 * */
void* check_context_ast( symrec**, inst_net**, UT_array*, ast_node*, bool );

/**
 * Establish copy synchronizer connections between two virtual nets. This
 * includes side ports as well as regular ports The dependancy graph is
 * updated.
 *
 * @param inst_net**:   pointer to the instance table of nets (scopes)
 * @param virt_net*:    pointer to the virtual net of the left operator
 * @param virt_net*:    pointer to the virtual net of the right operator
 */
void cpsync_connect( inst_net*, virt_net*, virt_net* );

/**
 * Merge two copy sunchronizer
 *
 * @param inst_net**:   pointer to the instance table of nets (scopes)
 * @param virt_net*:    pointer to the port of a virtual net
 * @param virt_net*:    pointer to the port of a virtual net
 * @return inst_rec*:   pointer to the merged copy synchronizer
 */
inst_rec* cpsync_merge( inst_net*, virt_ports*, virt_ports* );

/**
 * Print debug information of a port of a virtual net
 *
 * @param virt_net*:    pointer to the port of a virtual net
 */
void debug_print_port( virt_ports* );

/**
 * Print debug information of all ports in a virtual net
 *
 * @param virt_net*:    pointer to the virtual net
 */
void debug_print_ports( virt_net* );

/**
 * This function performs the following tasks
 * - check whether the given identificator is in the symbol table
 * - add instances to the instance table
 * - add instances to the dependency graph
 * This is a recursive function.
 *
 * @param symrec**:     pointer to the symbol table
 * @param inst_net**:   pointer to the instance table of nets (scopes)
 * @param UT_array**:   pointer to the scope stack
 * @param ast_node*:    pointer to the ast node
 * @return virt_net*:   pointer to a virtual net with a port list
 *                      and connection vectors
 * */
virt_net* install_nets( symrec**, inst_net*, UT_array*, ast_node* );

#endif // CONTEXT_H
