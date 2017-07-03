/**
 * A plugin to draw dot graphs from the AST:
 *  - the AST itself
 *  - the connection graph of the network
 *
 * @file    smxdot.c
 * @author  Simon Maurer
 *
 */

#include "defines.h"
#include "smxdot.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef DOT_AST
char* node_label[] =
{
    "assign",
    "box decl",
    "alt name",
    "link",
    "net",
    "net prototype",
    "parallel",
    "parallel det",
    "port decl",
    "ports",
    "program",
    "serial",
    "serial prop",
    "stmts",
    "sync",
    "tt",
    "wrapper decl"
};
char* mode_label[] = {
    "in",
    "out"
};
char* class_label[] = {
    PORT_CLASS_NONE_STR,
    PORT_CLASS_UP_STR,
    PORT_CLASS_DOWN_STR,
    PORT_CLASS_SIDE_STR
};
char* nattr_label[] = {
    "decoupled",
    "pure",
    "static"
};
char** attr_label[] = {
    mode_label,
    class_label,
    nattr_label
};

/******************************************************************************/
void draw_ast_graph( ast_node_t* start )
{
    FILE* ast_graph = fopen( AST_DOT_PATH, "w" );
    graph_init( ast_graph, STYLE_G_DEFAULT );
    draw_ast_graph_step( ast_graph, start );
    graph_finish( ast_graph );
    fclose( ast_graph );
}

/******************************************************************************/
void draw_ast_graph_step( FILE* graph, ast_node_t* ptr )
{
    ast_list_t* ast_list_ptr;
    char label[10];

    if( ptr == NULL ) return;

    switch( ptr->type ) {
        // reached a leaf node of the AST -> add box or octagon to drawing
        case AST_ATTR:
            if( ptr->attr->type == ATTR_INT )
                sprintf( label, "%d", ptr->attr->val );
            else
                sprintf( label, "%s",
                        attr_label[ ptr->attr->type ][ ptr->attr->val ] );
            graph_add_node( graph, ptr->id, label, STYLE_N_AST_ATTR );
            break;
        case AST_ID:
            graph_add_node( graph, ptr->id, ptr->symbol->name, STYLE_N_AST_ID );
            break;
        case AST_PROGRAM:
            graph_add_node( graph, ptr->id, node_label[ ptr->type ],
                    STYLE_N_AST_NODE );
            if( ptr->program->stmts != NULL ) {
                draw_ast_graph_step( graph, ptr->program->stmts );
                graph_add_edge( graph, ptr->id, ptr->program->stmts->id, NULL,
                        STYLE_E_DEFAULT );
            }
            draw_ast_graph_step( graph, ptr->program->net );
            graph_add_edge( graph, ptr->id, ptr->program->net->id, NULL,
                    STYLE_E_DEFAULT );
            break;
        // draw a list-node with its children
        case AST_LINKS:
        case AST_STMTS:
        case AST_PORTS:
        case AST_INT_PORTS:
        case AST_SYNCS:
            graph_add_node( graph, ptr->id, node_label[ptr->type],
                    STYLE_N_AST_NODE );
            // iterate through all elements of the list
            ast_list_ptr = ptr->list;
            while( ast_list_ptr != 0 ) {
                draw_ast_graph_step( graph, ast_list_ptr->node );
                graph_add_edge( graph, ptr->id, ast_list_ptr->node->id,
                        NULL, STYLE_E_DEFAULT );
                ast_list_ptr = ast_list_ptr->next;
            }
            break;
        // draw assignments
        case AST_ASSIGN:
            graph_add_node( graph, ptr->id, node_label[ ptr->type ],
                    STYLE_N_AST_NODE );
            // draw the id
            draw_ast_graph_step( graph, ptr->assign->id );
            graph_add_edge( graph, ptr->id, ptr->assign->id->id, NULL,
                    STYLE_E_DEFAULT );
            // draw the step
            draw_ast_graph_step( graph, ptr->assign->op );
            graph_add_edge( graph, ptr->id, ptr->assign->op->id, NULL,
                    STYLE_E_DEFAULT );
            break;
        // draw operators
        case AST_SERIAL:
        case AST_PARALLEL:
            graph_add_node( graph, ptr->id, node_label[ ptr->type ],
                    STYLE_N_AST_NODE );
            // continue on the left branch
            draw_ast_graph_step( graph, ptr->op->left );
            graph_add_edge( graph, ptr->id, ptr->op->left->id, NULL,
                    STYLE_E_DEFAULT );
            // continue on the right branch
            draw_ast_graph_step( graph, ptr->op->right );
            graph_add_edge( graph, ptr->id, ptr->op->right->id, NULL,
                    STYLE_E_DEFAULT );
            break;
        // draw simple nodes
        case AST_NET:
            graph_add_node( graph, ptr->id, node_label[ ptr->type ],
                    STYLE_N_AST_NODE );
            draw_ast_graph_step( graph, ptr->network->net );
            graph_add_edge( graph, ptr->id, ptr->network->net->id, NULL,
                    STYLE_E_DEFAULT );
            break;
        // draw special nodes
        case AST_BOX:
            graph_add_node(graph, ptr->id, node_label[ ptr->type ],
                    STYLE_N_AST_NODE );
            // id implementation
            draw_ast_graph_step( graph, ptr->box->impl );
            graph_add_edge( graph, ptr->id, ptr->box->impl->id, NULL,
                    STYLE_E_DEFAULT );
            // port list
            if( ptr->box->ports != NULL ) {
                draw_ast_graph_step( graph, ptr->box->ports );
                graph_add_edge( graph, ptr->id, ptr->box->ports->id, NULL,
                        STYLE_E_DEFAULT );
            }
            // state
            if( ptr->box->attr_pure != NULL ) {
                draw_ast_graph_step( graph, ptr->box->attr_pure );
                graph_add_edge( graph, ptr->id, ptr->box->attr_pure->id, NULL,
                        STYLE_E_DEFAULT );
            }
            break;
        case AST_NET_PROTO:
            graph_add_node( graph, ptr->id, node_label[ ptr->type ],
                    STYLE_N_AST_NODE );
            // id
            draw_ast_graph_step( graph, ptr->proto->id );
            graph_add_edge( graph, ptr->id, ptr->proto->id->id, NULL,
                    STYLE_E_DEFAULT );
            // port list
            if( ptr->proto->ports != NULL ) {
                draw_ast_graph_step( graph, ptr->proto->ports );
                graph_add_edge( graph, ptr->id, ptr->proto->ports->id, NULL,
                        STYLE_E_DEFAULT );
            }
            break;
        case AST_WRAP:
            graph_add_node( graph, ptr->id, node_label[ ptr->type ],
                    STYLE_N_AST_NODE );
            // id
            draw_ast_graph_step( graph, ptr->wrap->id );
            graph_add_edge( graph, ptr->id, ptr->wrap->id->id, NULL,
                    STYLE_E_DEFAULT );
            // wrapper port list
            if( ptr->wrap->ports_wrap != NULL ) {
                draw_ast_graph_step( graph, ptr->wrap->ports_wrap );
                graph_add_edge( graph, ptr->id, ptr->wrap->ports_wrap->id, NULL,
                        STYLE_E_DEFAULT );
            }
            // net port list
            if( ptr->wrap->ports_net != NULL ) {
                draw_ast_graph_step( graph, ptr->wrap->ports_net );
                graph_add_edge( graph, ptr->id, ptr->wrap->ports_net->id, NULL,
                        STYLE_E_DEFAULT );
            }
            // stmt list
            if( ptr->wrap->stmts != NULL ) {
                draw_ast_graph_step( graph, ptr->wrap->stmts );
                graph_add_edge( graph, ptr->id, ptr->wrap->stmts->id, NULL,
                        STYLE_E_DEFAULT );
            }
            // static
            if( ptr->wrap->attr_static != NULL ) {
                draw_ast_graph_step( graph, ptr->wrap->attr_static );
                graph_add_edge( graph, ptr->id, ptr->wrap->attr_static->id,
                        NULL, STYLE_E_DEFAULT );
            }
            break;
        case AST_PORT:
            graph_add_node( graph, ptr->id, node_label[ ptr->type ],
                    STYLE_N_AST_NODE );
            // id
            if( ptr->port->id != NULL ) {
                draw_ast_graph_step( graph, ptr->port->id );
                graph_add_edge( graph, ptr->id, ptr->port->id->id, NULL,
                        STYLE_E_DEFAULT );
            }
            // internal id
            if( ptr->port->int_id != NULL ) {
                draw_ast_graph_step( graph, ptr->port->int_id );
                graph_add_edge( graph, ptr->id, ptr->port->int_id->id, NULL,
                        STYLE_E_DEFAULT );
            }
            // mode
            if( ptr->port->mode != NULL ) {
                draw_ast_graph_step( graph, ptr->port->mode );
                graph_add_edge( graph, ptr->id, ptr->port->mode->id, NULL,
                        STYLE_E_DEFAULT );
            }
            // collection
            if( ptr->port->collection != NULL ) {
                draw_ast_graph_step( graph, ptr->port->collection );
                graph_add_edge( graph, ptr->id, ptr->port->collection->id, NULL,
                        STYLE_E_DEFAULT );
            }
            // coupling
            if( ptr->port->coupling != NULL ) {
                draw_ast_graph_step( graph, ptr->port->coupling );
                graph_add_edge( graph, ptr->id, ptr->port->coupling->id, NULL,
                        STYLE_E_DEFAULT );
            }
            // ch_len
            if( ptr->port->ch_len != NULL ) {
                draw_ast_graph_step( graph, ptr->port->ch_len );
                graph_add_edge( graph, ptr->id, ptr->port->ch_len->id, NULL,
                        STYLE_E_DEFAULT );
            }
            break;
        case AST_TT:
            graph_add_node( graph, ptr->id, node_label[ ptr->type ],
                    STYLE_N_AST_NODE );
            draw_ast_graph_step( graph, ptr->tt->op );
            graph_add_edge( graph, ptr->id, ptr->tt->op->id, NULL,
                    STYLE_E_DEFAULT );
            // frequ
            draw_ast_graph_step( graph, ptr->tt->freq );
            graph_add_edge( graph, ptr->id, ptr->tt->freq->id, NULL,
                    STYLE_E_DEFAULT );
            break;
        default:
            ;
    }
}
#endif // DOT_AST

/******************************************************************************/
void graph_add_divider( FILE* graph, int scope, const char flag )
{
    fprintf( graph, "%s%c%d\n", DOT_PATTERN, flag, scope );
}

/******************************************************************************/
void graph_add_edge( FILE* graph, int start, int end, char* label, int style )
{
    switch( style ) {
        case STYLE_E_DEFAULT:
            fprintf (graph, "\tid%d->id%d", start, end );
            break;
        case STYLE_E_PORT:
            fprintf( graph, "\tid%d->id%d", start, end );
#ifdef DOT_EDGE_LABEL
            fprintf( graph, "[xlabel=\"%s\", fontsize=10]", label );
#endif // DOT_EDGE_LABEL
            break;
        case STYLE_E_SPORT:
        case STYLE_E_SPORT_IN:
        case STYLE_E_SPORT_OUT:
        case STYLE_E_SPORT_BI:
            fprintf( graph, "\tid%d", start );
            if( ( style == STYLE_E_SPORT_IN )
                    || ( style == STYLE_E_SPORT_BI ) )
                fprintf( graph, ":s" );
            fprintf( graph, "->id%d", end );
            if( ( style == STYLE_E_SPORT_OUT )
                    || ( style == STYLE_E_SPORT_BI ) )
                fprintf( graph, ":s" );
            fprintf( graph, "[color=%s", COLOR_SIDE );
            /* fprintf( graph, ", constraint=false" ); */
#ifdef DOT_EDGE_LABEL
            fprintf( graph, ", xlabel=\"%s\", fontsize=10", label );
#endif // DOT_EDGE_LABEL
            fprintf( graph, "]" );
            break;
        case STYLE_E_LPORT:
        case STYLE_E_LSPORT:
            fprintf( graph, "\tid%d->id%d", start, end );
            if( style == STYLE_E_LPORT )
                fprintf( graph, "[color=%s", COLOR_LINK );
            else if( style == STYLE_E_LSPORT )
                fprintf( graph, "[color=%s", COLOR_SLINK );
            /* fprintf( graph, ", constraint=false" ); */
#ifdef DOT_EDGE_LABEL
            fprintf( graph, ", xlabel=\"%s\", fontsize=10", label );
#endif // DOT_EDGE_LABEL
            fprintf( graph, "]" );
            break;
        default:
            ;
    }
    fprintf( graph, ";\n" );
}

/******************************************************************************/
void graph_add_node( FILE* graph, int id, char* name, int style )
{
    switch( style ) {
        case STYLE_N_DEFAULT:
            fprintf( graph, "\tid%d [label=\"%s\"", id, name );
            break;
        case STYLE_N_NET_CPLS:
        case STYLE_N_NET_CPS:
        case STYLE_N_NET_CPL:
        case STYLE_N_NET_CP:
            fprintf( graph, "\tid%d [label=\"%s\"", id, name );
            fprintf( graph, ", width=0.3, fixedsize=true, margin=0, shape=%s",
                    SHAPE_CIRCLE );
            if( style == STYLE_N_NET_CPS )
                fprintf( graph, ", color=%s", COLOR_SIDE );
            else if( style == STYLE_N_NET_CPL )
                fprintf( graph, ", color=%s", COLOR_LINK );
            else if( style == STYLE_N_NET_CPLS )
                fprintf( graph, ", color=%s", COLOR_SLINK );
            break;
        case STYLE_N_AST_ATTR:
            fprintf( graph, "\tid%d [label=\"%s\"", id, name );
            fprintf( graph, ", shape=%s", SHAPE_OCTAGON );
            break;
        case STYLE_N_AST_NODE:
            fprintf( graph, "\tid%d [label=\"%s\"", id, name );
            fprintf( graph, ", shape=%s", SHAPE_ELLIPSE );
            break;
        case STYLE_N_NET_BOX:
        case STYLE_N_NET_WRAP:
            fprintf( graph, "\tid%d [label=<%s<SUB>%d</SUB>>", id, name, id );
            fprintf( graph, ", shape=%s", SHAPE_BOX );
            if( style == STYLE_N_NET_WRAP )
                fprintf( graph, ", color=%s", COLOR_N_WRAP );
            else if( style == STYLE_N_NET_BOX )
                fprintf( graph, ", color=%s", COLOR_N_BOX );
            break;
        case STYLE_N_NET_INVIS:
            fprintf( graph, "\tid%d [label=\"\", fixedsize=\"false\"", id );
            fprintf( graph, ", width=0, height=0, shape=none" );
            break;
        case STYLE_N_AST_ID:
            fprintf( graph, "\tid%d [label=\"%s\"", id, name );
            fprintf( graph, ", shape=%s", SHAPE_BOX );
            break;
        default:
            ;
    }
    fprintf( graph, "];\n" );
}

/******************************************************************************/
void graph_add_rank( FILE* graph, int id1, int id2 )
{
    fprintf( graph, "\t{ rank=same; id%d; id%d; }\n", id1, id2 );
}

/******************************************************************************/
void graph_finish( FILE* graph )
{
    fprintf( graph, "}\n" );
}

/******************************************************************************/
void graph_finish_subgraph( FILE* graph )
{
    fprintf( graph, "\t}\n" );
}

#ifdef DOT_CON
/******************************************************************************/
void graph_fix_dot( char* t_path, char* r_path )
{
#if defined(DEBUG) || defined(DEBUG_GRAPH)
    char d_path[40];
    FILE* d_graph;
#endif // DEBUG
    FILE* r_graph;
    FILE* t_graph;
    char str[512];
    char* ptr = NULL;
    char flag;
    char next_flag = FLAG_STMTS; // first flag will always be STMTS
    int scope = 0;
    int last_scope = -1;
    int iteration_cnt = 0;
    bool copy = false;
    bool done = false;
    char pattern[ sizeof( DOT_PATTERN ) + 10 ] = DOT_PATTERN;
    t_graph = fopen( t_path, "w" );
    while( !done ) {
        r_graph = fopen( r_path, "r" );
        while( fgets( str, 512, r_graph ) ) {
            ptr = strstr( str, pattern );
            if( ptr != NULL ) {
                // found an occurrence of the pattern
                copy = false;
                ptr += sizeof( DOT_PATTERN ) - 1;
                flag = *ptr;
                ptr++;
                scope = atoi(ptr);
                switch( flag ) {
                    case FLAG_STMTS:
                        if( scope <= last_scope ) continue;
                        if( flag != next_flag ) continue;
                        last_scope = scope;
                        next_flag = FLAG_WRAP_PRE;
                        copy = true;
                        break;
                    case FLAG_WRAP:
                        if( scope != last_scope ) continue;
                        if( flag != next_flag ) continue;
#ifdef DOT_SYNC_FIRST
                        next_flag = FLAG_CONNECT;
#else // DOT_SYNC_FIRST
                        next_flag = FLAG_NET;
#endif // DOT_SYNC_FIRST
                        copy = true;
                        break;
                    case FLAG_WRAP_END:
                    case FLAG_WRAP_PRE:
                    case FLAG_STMTS_END:
                    case FLAG_NET:
                    case FLAG_CONNECT:
                        if( scope != last_scope ) continue;
                        if( flag != next_flag ) continue;
                        copy = true;
                        break;
                    default:
                        ;
                }
                /* printf( "flag: %c, scope: %d\n", flag, scope ); */
            }
            else if( copy ) {
                fprintf( t_graph, "%s", str );
            }
        }
        switch( next_flag ) {
            case FLAG_STMTS:
                // no new stmts -> we are done
                done = true;
                break;
            case FLAG_WRAP_PRE:
                next_flag = FLAG_WRAP;
                break;
            case FLAG_WRAP:
                // no wrapper in this scope -> copy nets
                // because the wrap appears BEFORE the stmts we need to iterate
                // twice to make sure to get the wrap statements
                if( iteration_cnt > 0 )
#ifdef DOT_SYNC_FIRST
                    next_flag = FLAG_CONNECT;
#else // DOT_SYNC_FIRST
                    next_flag = FLAG_NET;
#endif // DOT_SYNC_FIRST
                iteration_cnt++;
                break;
            case FLAG_NET:
                // add side port connections
                iteration_cnt = 0;
#ifdef DOT_SYNC_FIRST
                next_flag = FLAG_WRAP_END;
#else // DOT_SYNC_FIRST
                next_flag = FLAG_CONNECT;
#endif // DOT_SYNC_FIRST
                break;
            case FLAG_CONNECT:
                // need to close wrapper
#ifdef DOT_SYNC_FIRST
                next_flag = FLAG_NET;
#else // DOT_SYNC_FIRST
                next_flag = FLAG_WRAP_END;
#endif // DOT_SYNC_FIRST
                break;
            case FLAG_WRAP_END:
                // need to close stmts
                next_flag = FLAG_STMTS_END;
                break;
            case FLAG_STMTS_END:
                // this scope is done -> go to the next one
                next_flag = FLAG_STMTS;
            default:
                ;
        }
        fclose( r_graph );
    }
    fclose( t_graph );
#if defined(DEBUG) || defined(DEBUG_GRAPH)
    // save the original file in order to debug
    sprintf( d_path, "%s.dbg", r_path );
    r_graph = fopen( r_path, "r" );
    d_graph = fopen( d_path, "w" );
    while( fgets( str, 512, r_graph ) ) {
        fprintf( d_graph, "%s", str );
    }
    fclose( r_graph );
    fclose( d_graph );
#endif // DEBUG
    // copy the fixed dot file back to the original file
    t_graph = fopen( t_path, "r" );
    r_graph = fopen( r_path, "w" );
    while( fgets( str, 512, t_graph ) ) {
        fprintf( r_graph, "%s", str );
    }
}
#endif // DOT_CON

/******************************************************************************/
void graph_init( FILE* graph, int style )
{
    fprintf( graph, "digraph {\n" );
    switch( style ) {
        case STYLE_G_DEFAULT:
            break;
        case STYLE_G_CON_NET:
            fprintf( graph, "\trankdir=LR;\n" );
            fprintf( graph, "\tedge [dir=none];\n" );
            break;
        case STYLE_G_CON_PORT:
            fprintf( graph, "\trankdir=LR;\n" );
#ifdef DOT_EDGE_LABEL
            fprintf( graph, "\tranksep=0.75;\n" );
            /* fprintf( graph, "\tnodesep=0.75;\n" ); */
#endif // DOT_EDGE_LABEL
            // caused sideprts and copy synchrniyers to behave strange
            /* fprintf( graph, "\tsplines=false;\n" ); */
            break;
        default:
            ;
    }
}

/******************************************************************************/
void graph_init_subgraph( FILE* graph, char* name, int id, int style )
{
    switch( style ) {
        case STYLE_SG_PARALLEL:
            fprintf( graph, "\tsubgraph clusterP%d {\n", id );
            fprintf( graph, "\tlabel=\"%s\";\n", name );
            fprintf( graph, "\tstyle=dotted;\n" );
#ifdef DOT_COLOR
            fprintf( graph, "\tcolor=%s;\n", COLOR_PARALLEL );
#else
            fprintf( graph, "\tcolor=invis;\n" );
#endif // DOT_CON
            break;
        case STYLE_SG_SERIAL:
            fprintf( graph, "\tsubgraph clusterS%d {\n", id );
            fprintf( graph, "\tlabel=\"%s\";\n", name );
            fprintf( graph, "\tstyle=dashed;\n" );
#ifdef DOT_COLOR
            fprintf( graph, "\tcolor=%s;\n", COLOR_SERIAL );
#else
            fprintf( graph, "\tcolor=invis;\n" );
#endif // DOT_CON
            break;
        case STYLE_SG_WRAPPER:
            fprintf( graph, "\tsubgraph clusterW%d {\n", id );
            fprintf( graph, "\tcolor=%s;\n", COLOR_WRAP );
            fprintf( graph, "\tlabel=\"%s\";\n", name );
            break;
        default:
            ;
    }
}
