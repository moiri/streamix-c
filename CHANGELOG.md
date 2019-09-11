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

# `v0.3.0`

The initial release after completing the [dissertation](https://uhra.herts.ac.uk/handle/2299/21094).
