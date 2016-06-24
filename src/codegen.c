#include "codegen.h"
#include <stdio.h>

/******************************************************************************/
void cgen_box_create( int scope, int id, const char* name )
{
    printf( "void* box_%d_%d = SMX_BOX_CREATE( %s );\n", id, scope,
            name );
}

/******************************************************************************/
void cgen_channel_create( int scope, int id )
{
    printf( "void* ch_%d_%d = SMX_CHANNEL_CREATE();\n", id, scope );
}

/******************************************************************************/
void cgen_connect( int scope, int id_ch, int id_box, const char* box_name,
        const char* ch_name )
{
    printf( "SMX_CONNECT( box_%d_%d, %s, ch_%d_%d, %s );\n", id_box, scope,
            box_name, id_ch, scope, ch_name );
}
