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
 * Check the context of all identifiers in the program
 *
 * @param ast_node*:    pointer to the root ast node
 * */
void check_context( ast_node* );

/**
 * check whether the given identificator is in the symbol table.
 * This is a recursive function.
 *
 * @param symrec**:     pointer to the symbol table
 * @param symrec**:     pointer to the instance table
 * @param UT_array**:   pointer to the scope stack
 * @param ast_node*:    pointer to the root ast node
 * */
void check_ids( symrec**, inst_net**, UT_array*, ast_node* );
void check_ids_net( symrec**, inst_rec**, inst_rec**, UT_array*, ast_node*,
        net_con*, igraph_t* );

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

#endif // CONTEXT_H
