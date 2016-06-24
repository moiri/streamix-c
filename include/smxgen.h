/* 
 * Generates the Streamix C code
 *
 * @file    smxgen.h
 * @author  Simon Maurer
 *
 * */

#ifndef SMXGEN_H
#define SMXGEN_H

#include "insttab.h"

void smxgen_main( inst_net** );
void smxgen_network_create( inst_net**, int );
void smxgen_network_destroy( inst_net**, int );
void smxgen_network_run( inst_net**, int );
void smxgen_network_wait_end( inst_net**, int );

#endif /* ifndef SMXGEN_H */
