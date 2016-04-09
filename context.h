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

void check_connection( inst_net*, virt_net*, virt_net* );
void check_connection_cp( inst_net*, virt_net*, virt_net* );

/**
 * Check the context of all identifiers in the program
 *
 * @param ast_node*:    pointer to the root ast node
 * */
void check_context( ast_node* );

/**
 * This function performs the following tasks:
 * - check whether the given identificator in a net is in the symbol table
 * - add net instances to the instance table
 * - add net instances to the connection graph
 * This is a recursive function.
 *
 * @param symrec**:     pointer to the symbol table
 * @param inst_net**:   pointer to the net instance table
 * @param UT_array**:   pointer to the scope stack
 * @param ast_node*:    pointer to the root ast node
 * */
void ___check_nets( symrec**, inst_net**, UT_array*, ast_node* );

/**
 * This function performs the following tasks (callee of check_nets):
 * - check whether the given identificator is in the symbol table
 * - add instances to the instance table
 * - add instances to the connection graph
 * This is a recursive function.
 *
 * @param symrec**:     pointer to the symbol table
 * @param symrec**:     pointer to the instance table
 * @param UT_array**:   pointer to the scope stack
 * @param ast_node*:    pointer to the root ast node
 * @param net_con*:     pointer to the structure holding the two vectors
 *                      indicating the left and right connections of the net
 * @param igraph_t*:    pointer to the connection graph
 * */
virt_net* install_nets( symrec**, inst_net*, UT_array*, ast_node* );

/**
 *
 * @param inst_net**:   pointer to the net instance table
 */
void check_instances( inst_net** );
void check_nets( inst_rec**, igraph_t* g );

/**
 * put symbol names into the symbol table. this includes collision and scope
 * handling. This is a recursive funtion.
 *
 * @param symrec**:     pointer to the symbol table
 * @param UT_array**:   pointer to the scope stack
 * @param ast_node*:    pointer to the ast node
 * @param bool:         flag indicating wheter the ports are synchronized
 * @return void*:
 *      the return value is used to propagate back the information of which
 *      ports belong to which net
 * */
void* install_ids( symrec**, UT_array*, ast_node*, bool );
void* check_context_ast( symrec**, inst_net**, UT_array*, ast_node*, bool );

symrec_list* get_port_list_net( inst_rec**, igraph_vector_t*, int, symrec_list* );

#endif // CONTEXT_H
