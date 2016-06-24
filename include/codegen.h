#ifndef CODEGEN_H
#define CODEGEN_H

void cgen_box_create( int, int, int, const char* );
void cgen_box_destroy( int, int, int );
void cgen_box_run( int, int, int, const char* );
void cgen_box_wait_end( int, const char* );
void cgen_channel_create( int, int, int );
void cgen_channel_destroy( int, int, int );
void cgen_connect( int, int, int, int, const char*, const char* );
void cgen_function_end( int );
void cgen_header_c_file( const char* );
void cgen_ident( int );
void cgen_include_local( const char* );
void cgen_main_head();
int cgen_print( const char*, ... );
void cgen_program_init( int );
void cgen_program_cleanup( int );

#endif /* ifndef CODEGEN_H */
