# `v0.1.0`

### Bug Fixes

### Changes
 - instead of compiling igraph manually use the already prepared deb package

### New Features
 - mark nets that are framed by temporal firewals with a graph attribute
 - introduce the keyword `coupled` which allows to overwrite the default
   decoupling of side output ports and make such a port blocking.
 - introduce the keyword `extern` which allows to mark boxes which are defined
   externally. For boxes with the keyword `extern` no signature will be
   generated, however, an h-file will be included in the main file with the
   name of the box.

# `diss_final`

The initial release after completing the [dissertation](https://uhra.herts.ac.uk/handle/2299/21094).
