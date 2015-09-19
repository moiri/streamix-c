#include <stdlib.h> /* For malloc in symbol table */
#include <string.h> /* For strcmp in symbol table */
#include <stdio.h>
#include "ast.h"
#include "graph.h"
#include "defines.h"

int __node_id_ast = 0;
int __node_id_con = 0;
int __l_stack_con[MAX_STACK_SIZE];
int __l_stack_con_cnt = 0;
int __r_stack_con[MAX_STACK_SIZE];
int __r_stack_con_cnt = 0;

/*
 * Add a net identifier to the AST.
 *
 * @param: char* name:  name of the net
 * @return: ast_node*:
 *      a pointer to the location where the data was stored
 * */
ast_node* ast_add_id ( char* name ) {
    ast_node *ptr;
    ptr = (ast_node*) malloc(sizeof(ast_node));
    ptr->name = (char*) malloc(strlen(name)+1);
    strcpy (ptr->name, name);
    ptr->node_type = OP_ID;
    return ptr;
}

/*
 * Add a an operation to the symbol table.
 *
 * @param: char* name:      name of the net
 * @param: ast_node* left:  pointer to the left operand
 * @param: ast_node* right: pointer to the right operand
 * @param: int node_type:   OP_ID, OP_SERIAL, OP_PARALLEL
 * @return: ast_node*:
 *      a pointer to the location where the data was stored
 * */
ast_node* ast_add_op ( ast_node* left, ast_node* right, int node_type ) {
    ast_node *ptr;
    ptr = (ast_node*) malloc(sizeof(ast_node));
    ptr->op.left = left;
    ptr->op.right = right;
    ptr->node_type = node_type;
    return ptr;
}

/*
 * Draw a dot diagram of the AST
 *
 * @param: ast_node* start:  pointer to the root node of the AST
 * */
void draw_ast_graph (ast_node* start) {
    FILE* ast_graph = fopen(AST_DOT_PATH, "w");
    initGraph(ast_graph, STYLE_DEFAULT);
    draw_ast_step(ast_graph, start);
    finishGraph(ast_graph);
}

/*
 * Recursive function to draw AST nodes
 *
 * @param: FILE* graph:     file pointer to the dot file
 * @param: ast_node* ptr:   pointer to the current ast node
 * @return: int node_id:    id of the current node
 * */
int draw_ast_step (FILE* graph, ast_node* ptr) {
    int node_id;
    int child_node_id;
    char child_node_id_str[11];
    char node_id_str[11];
    char node_name[11];
    __node_id_ast++;
    node_id = __node_id_ast;
    sprintf(node_id_str, "id%d", node_id);
    if (ptr->node_type == OP_ID) {
        addNode(graph, node_id_str, ptr->name, SHAPE_BOX);
    }
    else if (ptr->node_type == OP_SERIAL || ptr->node_type == OP_PARALLEL) {
        if (ptr->node_type == OP_SERIAL)    strcpy(node_name, "serial");
        if (ptr->node_type == OP_PARALLEL)  strcpy(node_name, "parallel");
        addNode(graph, node_id_str, node_name, SHAPE_ELLIPSE);
        // continue on the left branch
        child_node_id = draw_ast_step(graph, ptr->op.left);
        sprintf(child_node_id_str, "id%d", child_node_id);
        addEdge(graph, node_id_str, child_node_id_str);
        // continue on the right branch
        child_node_id = draw_ast_step(graph, ptr->op.right);
        sprintf(child_node_id_str, "id%d", child_node_id);
        addEdge(graph, node_id_str, child_node_id_str);
    }
    return node_id;
}

/*
 * Draw a dot diagram of the connection graph
 *
 * @param: ast_node* start:  pointer to the root node of the AST
 * */
void draw_connection_graph (ast_node* start) {
    FILE* con_graph = fopen(CON_DOT_PATH, "w");
    initGraph(con_graph, EDGE_UNDIRECTED);
    draw_connection_step(con_graph, start);
    finishGraph(con_graph);
}

/*
 * Recursive function to draw the connection graph
 *
 * @param: FILE* graph:     file pointer to the dot file
 * @param: ast_node* ptr:   pointer to the current ast node
 * */
void draw_connection_step (FILE* graph, ast_node* ptr) {
    int lo[MAX_STACK_SIZE];
    int lo_cnt = 0;
    int li[MAX_STACK_SIZE];
    int li_cnt = 0;
    int ri[MAX_STACK_SIZE];
    int ri_cnt = 0;
    int ro[MAX_STACK_SIZE];
    int ro_cnt = 0;
    char node_id_str[11];
    char tmp_node_id_str[11];

    if (ptr->node_type == OP_ID) {
        // reached a leaf node of the AST -> add box to drawing
        __node_id_con++;
        sprintf(node_id_str, "id%d", __node_id_con);
        addNode(graph, node_id_str, ptr->name, SHAPE_BOX);

        // write stack
        __l_stack_con_cnt = write_stack (__l_stack_con, __l_stack_con_cnt,
                &__node_id_con, 1);
        __r_stack_con_cnt = write_stack (__r_stack_con, __r_stack_con_cnt,
                &__node_id_con, 1);
        /* printf("box: %d / %d\n", __l_stack_con[__l_stack_con_cnt-1], __r_stack_con[__r_stack_con_cnt-1]); */
    }
    else if (ptr->node_type == OP_SERIAL || ptr->node_type == OP_PARALLEL) {
        // continue on the left branch
        draw_connection_step(graph, ptr->op.left);
        if (ptr->node_type == OP_SERIAL) {
            /* printf("READ STACK SERIAL LEFT OP\n"); */
            // l -> lo
            lo_cnt = read_stack(__l_stack_con, __l_stack_con_cnt, lo, lo_cnt);
            __l_stack_con_cnt = 0;
            // r -> li
            li_cnt = read_stack(__r_stack_con, __r_stack_con_cnt, li, li_cnt);
            __r_stack_con_cnt = 0;
        }
        else if (ptr->node_type == OP_PARALLEL) {
            /* printf("READ STACK PARALLEL LEFT OP\n"); */
            // l -> lo
            lo_cnt = read_stack(__l_stack_con, __l_stack_con_cnt, lo, lo_cnt);
            // l -> ri
            ri_cnt = read_stack(__l_stack_con, __l_stack_con_cnt, ri, ri_cnt);
            __l_stack_con_cnt = 0;
            // r -> li
            li_cnt = read_stack(__r_stack_con, __r_stack_con_cnt, li, li_cnt);
            // r -> ro
            ro_cnt = read_stack(__r_stack_con, __r_stack_con_cnt, ro, ro_cnt);
            __r_stack_con_cnt = 0;
        }
        // continue on the right branch
        draw_connection_step(graph, ptr->op.right);
        if (ptr->node_type == OP_SERIAL) {
            /* printf("READ STACK SERIAL RIGHT OP\n"); */
            // l -> ri
            ri_cnt = read_stack(__l_stack_con, __l_stack_con_cnt, ri, ri_cnt);
            __l_stack_con_cnt = 0;
            // r -> ro
            ro_cnt = read_stack(__r_stack_con, __r_stack_con_cnt, ro, ro_cnt);
            __r_stack_con_cnt = 0;

            // draw inner connections
            /* printf("DRAW EDGE\n"); */
            for (int i = 0; i < li_cnt; i++) {
                /* printf(" drawing li id%d (%d)\n", li[i], i); */
                sprintf(node_id_str, "id%d", li[i]);
                for (int j = 0; j < ri_cnt; j++) {
                    /* printf(" drawing ri id%d (%d)\n", ri[j], j); */
                    sprintf(tmp_node_id_str, "id%d", ri[j]);
                    addEdge(graph, node_id_str, tmp_node_id_str);
                }
            }
        }
        else if (ptr->node_type == OP_PARALLEL) {
            /* printf("READ STACK PARALLEL RIGHT OP\n"); */
            // l -> lo
            lo_cnt = read_stack(__l_stack_con, __l_stack_con_cnt, lo, lo_cnt);
            // l -> ri
            ri_cnt = read_stack(__l_stack_con, __l_stack_con_cnt, ri, ri_cnt);
            __l_stack_con_cnt = 0;
            // r -> li
            li_cnt = read_stack(__r_stack_con, __r_stack_con_cnt, li, li_cnt);
            // r -> ro
            ro_cnt = read_stack(__r_stack_con, __r_stack_con_cnt, ro, ro_cnt);
            __r_stack_con_cnt = 0;
        }

        // write to stack
        // lo -> l
        __l_stack_con_cnt = write_stack(__l_stack_con, __l_stack_con_cnt, lo, lo_cnt);
        lo_cnt = 0;
        // ro -> r
        __r_stack_con_cnt = write_stack(__r_stack_con, __r_stack_con_cnt, ro, ro_cnt);
        ro_cnt = 0;
    }
}

/*
 * Write to a stack
 *
 * @param int* stack:       pointer to the stack array
 * @param int stack_pos:    position of the stack pointer
 * @param int* src:         pointer to the source array
 * @param int src_len       length of the source array
 * @return int stack_pos:   new position of the stack pointer
 * */
int write_stack (int* stack, int stack_pos, int* src, int src_len) {
    int val;
    while (src_len > 0) {
        src_len--;
        val = src[src_len];
        stack[stack_pos] = val;
        stack_pos++;
        /* printf(" write to stack %d\n", stack[pos-1]); */
        if (stack_pos >= MAX_STACK_SIZE) {
            printf("%s\n", WARNING_STACK_OVERFLOW);
            stack_pos = MAX_STACK_SIZE - 1;
            break;
        }
    }
    return stack_pos;
}

/*
 * read from a stack
 *
 * @param int* stack:       pointer to the stack array
 * @param int stack_len:    length of the stack array
 * @param int* dest:        pointer to the destiantion array
 * @param int dest_pos      position of the dest array pointer
 * @return int dest_pos:    new position of dest array pointer
 * */
int read_stack (int* stack, int stack_len, int* dest, int dest_pos) {
    int val;
    while (stack_len > 0) {
        stack_len--;
        val = stack[stack_len];
        dest[dest_pos] = val;
        dest_pos++;
        if (dest_pos >= MAX_STACK_SIZE) {
            printf("%s\n", WARNING_STACK_OVERFLOW);
            dest_pos = MAX_STACK_SIZE - 1;
            break;
        }
    }
    return dest_pos;
}
