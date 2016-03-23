#include "cgraph.h"
#include <stdio.h>

/******************************************************************************/
void cgraph_connect( igraph_t* g, igraph_vector_t* v1, igraph_vector_t* v2 )
{
    igraph_vector_t edges;
    int i, j;
    igraph_vector_init( &edges, 0 );
    for( i=0; i<igraph_vector_size( v1 ); i++ ) {
        for( j=0; j<igraph_vector_size( v2 ); j++ ) {
            igraph_vector_push_back( &edges, VECTOR( *v1 )[i] );
            igraph_vector_push_back( &edges, VECTOR( *v2 )[j] );
        }
    }
    igraph_add_edges( g, &edges, 0 );
    igraph_vector_destroy( &edges );
}

/******************************************************************************/
void cgraph_init( ast_node* ast )
{
    ast_list* list = NULL;
    if( ast == NULL ) return;

    switch( ast->node_type ) {
        case AST_STMTS:
            list = ast->ast_list;
            while( list != NULL ) {
                cgraph_init( list->ast_node );
                list = list->next;
            }
            break;
        case AST_NET_DEF:
            cgraph_init( ast->ast_assign.op );
            break;
        case AST_WRAP:
            cgraph_init( ast->wrap.stmts );
            break;
        case AST_NET:
            // create new graph and store pointer in the net ast node
            igraph_empty( &ast->ast_net.g, 0, false );
            // initialize the connection vectors
            ast->ast_net.con = ( net_con* )malloc( sizeof( net_con ) );
            igraph_vector_init( &ast->ast_net.con->left, 0 );
            igraph_vector_init( &ast->ast_net.con->right, 0 );
            cgraph_spawn( &ast->ast_net.g, ast->ast_net.net, ast->ast_net.con );
            /* igraph_write_graph_dot( &ast->ast_net.g, stdout ); */
            break;
        default:
            ;
    }
}

/******************************************************************************/
void cgraph_spawn( igraph_t* g, ast_node* ast, net_con* con )
{
    net_con* lcon = ( net_con* )0;
    if( ast == NULL ) return;

    switch( ast->node_type ) {
        case AST_PARALLEL:
        case AST_SERIAL:
            lcon = ( net_con* )malloc( sizeof( net_con ) );
            // traverse left side
            cgraph_spawn( g, ast->op.left, con );
            // save connection vectors in a temporary var
            igraph_vector_copy( &lcon->left, &con->left );
            igraph_vector_copy( &lcon->right, &con->right );
            igraph_vector_clear( &con->left );
            igraph_vector_clear( &con->right );
            // traverse right side with again empty vectors
            cgraph_spawn( g, ast->op.right, con );
            // connect tcon->right (left.con->right) with con->left
            // (right.con->left)
            if( ast->node_type == AST_SERIAL ) {
                cgraph_connect( g, &lcon->right, &con->left );
            }
            // update final connection vectors
            if( ast->node_type == AST_PARALLEL ) {
                // cleft = ( right.cleft, left.cleft )
                igraph_vector_append( &con->left, &lcon->left );
                // cright = ( right.cright, left.cright )
                igraph_vector_append( &con->right, &lcon->right );
            }
            else if( ast->node_type == AST_SERIAL ) {
                // cleft = ( left.cleft )
                igraph_vector_update( &con->left, &lcon->left );
                // cright = ( right.cright ) is already set in con->right
            }

            // free memory from temp connection var
            igraph_vector_destroy( &lcon->left );
            igraph_vector_destroy( &lcon->right );
            free( lcon );
            break;
        case AST_ID:
            // the vertice id is the number of vertices before adding a new one
            igraph_vector_push_back( &con->left, igraph_vcount( g ) );
            igraph_vector_push_back( &con->right, igraph_vcount( g ) );
            // add new vertex to graph
            igraph_add_vertices( g, 1, NULL );
            break;
        default:
            ;
    }
}
