#include <string.h> /* For strcmp in symbol table */
#include <stdio.h>
#include "graph.h"
#include "defines.h"

void initGraph (FILE* graph, int style) {
    fprintf(graph, "digraph Net {\n");
    switch (style) {
        case EDGE_UNDIRECTED:
            fprintf(graph, "\tedge [dir=none]\n");
            break;
        default:
            ;
    }
}
void finishGraph (FILE* graph) {
    fprintf(graph, "}");
    fclose(graph);
}
void addNode ( FILE* graph, char* id, char* name, const char* shape ) {
    fprintf(graph, "\t%s [label=\"%s\", shape=%s];\n", id, name, shape);
}
void addEdge ( FILE* graph, char* start, char* end) {
    fprintf(graph, "\t%s->%s;\n", start, end);
}
