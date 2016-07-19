#include "insttab.h"
#include <string.h>
#include <stdio.h>

int main( ) {
    inst_net_t* nets = NULL;
    inst_net_t* res1;
    inst_rec_t* res2;
    int error_cnt = 0;
    int scope = 5;
    int id = 10;
    int id_alt = 11;
    int id_replace = 20;
    char name[] = "test";

    inst_net_put( &nets, scope );

    // add net and check wheter it was added
    res1 = inst_net_get( &nets, scope );
    if( res1 == NULL ) {
        printf( "error: no nets in scope %d found\n", scope );
        return 0;
    }
    else if( res1->scope != scope ) {
        printf( "error: expected scope=%d but got scope=%d\n", scope,
                res1->scope );
    }
    else printf( "success: add net\n" );

    // add rec and check if we can get it back
    inst_rec_put( &res1->nodes, name, id, 0, INSTREC_NET, NULL );
    res2 = inst_rec_get( &res1->nodes, id );
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
    if( error_cnt == 0 ) {
        printf( "success: add rec name=%s, id=%d\n", name, id );
        printf( "success: get rec by id=%d\n", id );
    }
    error_cnt = 0;

    // add another rec with the same name as the previous record and check if we
    // can get it back
    inst_rec_put( &res1->nodes, name, id_alt, 0, INSTREC_NET, NULL );
    res2 = inst_rec_get( &res1->nodes, id_alt );
    if( res2 == NULL ) {
        printf( "error: no record found with id '%d'\n", id_alt );
        error_cnt++;
    }
    else {
        if( res2->id != id_alt ) {
            printf( "error: expected id=%d but got id=%d\n", id_alt, res2->id );
            error_cnt++;
        }
        if( strcmp( res2->name, name ) != 0 ) {
            printf( "error: expected name=%s but got name=%s\n", name,
                    res2->name );
            error_cnt++;
        }
    }
    if( error_cnt == 0 ) {
        printf( "success: add rec collision, name=%s, id=%d\n", name, id_alt );
        printf( "success: get rec by id=%d\n", id_alt );
    }
    error_cnt = 0;

    // remove element that was added second
    inst_rec_del( &res1->nodes, res2 );
    res2 = inst_rec_get( &res1->nodes, id_alt );
    if( res2 != NULL ) {
        printf( "error: element name=%s, id=%d was not deleted\n", res2->name,
                res2->id );
        error_cnt++;
    }
    if( error_cnt == 0 )
        printf( "success: del rec, name=%s, id=%d\n", name, id_alt );

    // replace id with id_replace
    inst_rec_replace_id( &res1->nodes, id, id_replace );
    res2 = inst_rec_get( &res1->nodes, id );
    if( res2 != NULL ) {
        printf( "error: element name=%s, id=%d was not removed (replaced)\n",
                res2->name, res2->id );
        error_cnt++;
    }
    res2 = inst_rec_get( &res1->nodes, id_replace );
    if( res2 == NULL ) {
        printf( "error: element id=%d was not added (replaced)\n", id_replace );
        error_cnt++;
    }
    if( error_cnt == 0 )
        printf( "success: alt rec of id=%d to id=%d (name=%s)\n", id,
                res2->id, res2->name );

    return 0;
}
