#include "insttab.h"
#include <string.h>
#include <stdio.h>

int main( ) {
    inst_net* nets = NULL;
    inst_rec* recs_id = NULL;
    inst_rec* recs_name = NULL;
    inst_net* res1;
    inst_rec* res2;
    int error_cnt = 0;
    int scope = 5;
    int id = 10;
    char name[] = "test";

    inst_net_put( &nets, scope, &recs_name, &recs_id );
    inst_rec_put( &recs_name, &recs_id, name, id, NULL );

    res1 = inst_net_get( &nets, scope );
    if( res1 == NULL ) {
        printf( "error: no nets in scope %d found\n", scope );
        error_cnt++;
    }
    else if( res1->scope != scope ) {
        printf( "error: expected scope=%d but got scope=%d\n", scope,
                res1->scope );
        error_cnt++;
    }

    res2 = inst_rec_get_id( res1->recs_id, id );
    if( res2 == NULL ) {
        printf( "error: no record found with id '%d'\n", id );
        error_cnt++;
    }
    else {
        if( res2->id != id ) {
            printf( "error: expected id=%d but got id=%d\n", id, res2->id );
            error_cnt++;
        }
        if( strcmp( res2->name, name ) != 0 ) {
            printf( "error: expected name=%s but got name=%s\n", name,
                    res2->name );
            error_cnt++;
        }
    }
    res2 = inst_rec_get_name( res1->recs_name, name );
    if( res2 == NULL ) {
        printf( "error: no record found with name '%s'\n", name );
        error_cnt++;
    }
    else {
        if( res2->id != id ) {
            printf( "error: expected id=%d but got id=%d\n", id, res2->id );
            error_cnt++;
        }
        if( strcmp( res2->name, name ) != 0 ) {
            printf( "error: expected name=%s but got name=%s\n", name,
                    res2->name );
            error_cnt++;
        }
    }

    if( error_cnt == 0 ) printf( "success!\n" );

    return 0;
}
