# streamix-c
Compiler for the coordination language Streamix

Requires
 - [`flex`](https://github.com/westes/flex) for lexing the Streamix code
 - [`bison`](https://www.gnu.org/software/bison/) for parsing the Streamix code
 - [`igraph`](http://igraph.org/c/) to build depedency graphs
 - [`graphviz`](http://www.graphviz.org/) to plot results with the `dot` application
 - [`gs`](https://www.ghostscript.com/index.html) to combine seperate pdf files
 - ([`valgrind`](http://valgrind.org/) for memory checks)
