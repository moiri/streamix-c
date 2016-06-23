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

void smxgen_box( int, int, const char* );
void smxgen_channel( int, int );
void smxgen_connect( int, int, int, const char*, const char* );
void smxgen_network( inst_net** );

#endif /* ifndef SMXGEN_H */
