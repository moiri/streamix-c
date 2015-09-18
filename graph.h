/* 
 * A plugin to draw the connection graph of the network in dot
 *
 * @file    congraph.h
 * @author  Simon Maurer
 *
 * */


#ifndef CONGRAPH_H
#define CONGRAPH_H

void initGraph ( FILE*, int );
void finishGraph ( FILE* );
void addNode ( FILE*, char*, char*, const char* );
void addEdge ( FILE*, char*, char* );

#endif /* CONGRAPH_H */
