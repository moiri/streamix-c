# streamix-c
Compiler for the coordination language Streamix

## Installation

    make
    sudo make install

Requires
 - [`flex`](https://github.com/westes/flex) for lexing the Streamix code
 - [`bison`](https://www.gnu.org/software/bison/) for parsing the Streamix code
 - [`igraph`](http://igraph.org/c/) to build depedency graphs (make sure GraphML is enabled)

## Run Testcases

    make test

Requires
 - [`graphviz`](http://www.graphviz.org/) to plot results with the `dot` application
 - [`gs`](https://www.ghostscript.com/index.html) to combine seperate pdf files
 - ([`valgrind`](http://valgrind.org/) for memory checks)
