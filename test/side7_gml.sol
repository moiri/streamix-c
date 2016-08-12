Creator "igraph version 0.7.1 StreamixC"
Version 1
graph
[
  directed 1
  node
  [
    id 0
    label "A"
    func "A"
  ]
  node
  [
    id 1
    label "B"
    func "B"
  ]
  node
  [
    id 2
    label "smx_cp"
    func "null"
  ]
  node
  [
    id 3
    label "C"
    func "B"
  ]
  edge
  [
    source 0
    target 1
    label "a"
  ]
  edge
  [
    source 1
    target 2
    label "s"
  ]
  edge
  [
    source 2
    target 0
    label "s"
  ]
  edge
  [
    source 1
    target 3
    label "b"
  ]
  edge
  [
    source 3
    target 2
    label "s"
  ]
]
