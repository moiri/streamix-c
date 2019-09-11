# The Streamix compiler `smxc`

Compiler for the coordination language Streamix.
It takes a file containing Streamix syntax and produces a dependency graph in either the `graphml` (default) or `gml` format.

## Installation

A Debian package is distributed with the repository. Use the command

    sudo apt install __path_to_deb__

where `__path_to_deb__` defines the path to the local Debian package.

## Building

To build the compiler from scratch run the following commands:

    git clone --recursive https://github.com/moiri/streamix-c.git git-smxc
    make
    sudo make install

Note that the following libraries are required to compile `smxc` successfully:

### [`flex`](https://github.com/westes/flex)
This is used for lexing the Streamix code. To install on an apt-based Linux system type

    sudo apt update
    sudo apt install flex

### [`bison`](https://www.gnu.org/software/bison/)
This is used for parsing the Streamix code. To install on an apt-based Linux system type

    sudo apt update
    sudo apt install bison


### [`igraph`](http://igraph.org/c/)
This is used to build dependency graphs.

    sudo apt update
    sudo apt install libigraph0-dev


## Run Testcases

    make test

Requires
 - [`graphviz`](http://www.graphviz.org/) to plot results with the `dot` application
 - [`gs`](https://www.ghostscript.com/index.html) to combine separate pdf files
 - ([`valgrind`](http://valgrind.org/) for memory checks)

## Usage

    ./smxc [OPTION...] FILE

    Options:
      -h            This message
      -v            Version
      -o 'path'     Path to store the generated file
      -f 'format'   Format of the graph either 'gml' or 'graphml'

