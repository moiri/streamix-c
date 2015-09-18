#include <string.h> /* For strcmp in symbol table */
#include <stdio.h>
#include "graph.h"

void initGraph (FILE* graph) {
    fprintf(graph, "digraph Net {\n");
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
