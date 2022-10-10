# `v0.5.3` (latest)

### New Features

 - introduce the keyword `intern` which allows to mark boxes which are defined
   in the RTS. For boxes with the keyword `intern` no signature will be
   generated and no dependencies will be fetched.

### Changes

 - link to the library `libsmxutils` in order to use shared graph definitions.


-------------------
# `v0.5.2`

### New Features

 - add long versions of the program arguments
 - add program arguments to set different rt priorities
 - add the new net operator `rt` to allow to run a non-tt net with rt
   priorities


-------------------
# `v0.5.1`

### Bug Fixes

 - fix scope handling (cast required otherwise char is assumed)
 - fix segfault when defining unused wrapper ports (#20)
 - fix routing node connection of two bi-directional ports

### New Features

 - allow port decoupling in a wrapper which enables to decouple ports on
   routing nodes
 - add compiler option to set the minimal channel length


-------------------
# `v0.5.0`

### Bug Fixes

 - don't throw an error on unconnected `open` ports.
 - ignore open ports in prototype checking
 - don't print error when two matching ports are open, just don't connect (#19)

### Changes

 - change the installation path from `/opt/smx/bin` to `/usr/bin`

### New Features

 - introduce the keyword `dynamic` which allows to mark that a port is generated
   dynamically and has no name association through macros.
 - reduce routing nodes in post process
 - when merging routing nodes, distinguish between pre and post connect
 - print '/' in front of open ports


-------------------
# `v0.4.1`

### Changes

 - disable error when all inputs are decoupled

### Bug Fixes

 - fix a bug when merging more than two internal ports in a wrapper
 - Fix merging of routing nodes (cp_sync)
 - fix dependencies
 - fix testcase: add warning messages when changing ch length


-------------------
# `v0.4.0`

### Bug Fixes

 - fix a bug where nets were not connecting properly (#15)
 - produce a warning instead of an error when a bypass operator does not
   connect to its immediate neighbor (#18)
 - produce a warning when the channel length is automatically reduced due to a
   TT net (#17)

### Changes

 - instead of compiling igraph manually use the already prepared deb package

### New Features

 - mark nets that are framed by temporal firewalls with a graph attribute
 - introduce the keyword `coupled` which allows to overwrite the default
   decoupling of side output ports and make such a port blocking.
 - introduce the keyword `extern` which allows to mark boxes which are defined
   externally. For boxes with the keyword `extern` no signature will be
   generated, however, an h-file will be included in the main file with the
   name of the box.
 - introduce the keyword `open` which allows to mark that a port was
   intentionally left open.


-------------------
# `v0.3.0`

The initial release after completing the [dissertation](https://uhra.herts.ac.uk/handle/2299/21094).
