Creator "igraph version 0.7.1 StreamixC"
Version 1
graph
[
  directed 1
  node
  [
    id 0
    label "A"
    func "a"
    static 0
    pure 0
    extern 0
    tt 0
  ]
  node
  [
    id 1
    label "B"
    func "b"
    static 0
    pure 0
    extern 0
    tt 0
  ]
  node
  [
    id 2
    label "Profiler"
    func "smx_mongo"
    static 0
    pure 0
    extern 0
    tt 0
  ]
  node
  [
    id 3
    label "smx_profiler"
    func "smx_profiler"
    static 0
    pure 0
    extern 0
    tt 0
  ]
  edge
  [
    source 0
    target 1
    label "x"
    nsrc "smx_null"
    ndst "smx_null"
    dsrc 0
    ddst 0
    len 1
    dts 0
    dtns 0
    sts 0
    stns 0
    type 0
  ]
  edge
  [
    source 3
    target 2
    label "data"
    nsrc "profiler"
    ndst "smx_null"
    dsrc 0
    ddst 0
    len 10
    dts 0
    dtns 0
    sts 0
    stns 0
    type 0
  ]
]
