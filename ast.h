/* 
 * A simple AST plugin
 *
 * @file    ast.h
 * @author  Simon Maurer
 *
 * */

#ifndef AST_H
#define AST_H

struct ast_node {
    int node_type;
    union {
        char* name;
        struct {
            struct ast_node* left;
            struct ast_node* right;
        } op;
    };
};
typedef struct ast_node ast_node;

ast_node* ast_add_id (char*);
ast_node* ast_add_op (ast_node*, ast_node*, int);
void draw_ast_graph (ast_node*);
int draw_ast_step (FILE*, ast_node*);
void draw_connection_graph (ast_node*);
void draw_connection_step (FILE*, ast_node*);
int write_stack (int*, int, int*, int);
int read_stack (int*, int, int*, int);

#endif /* AST_H */
