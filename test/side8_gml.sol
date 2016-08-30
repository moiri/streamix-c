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
    label "smx_cp"
    func "null"
  ]
  node
  [
    id 4
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
    source 2
    target 1
    label "s1"
  ]
  edge
  [
    source 0
    target 2
    label "s1"
  ]
  edge
  [
    source 1
    target 3
    label "s2"
  ]
  edge
  [
    source 3
    target 0
    label "s2"
  ]
  edge
  [
    source 1
    target 4
    label "b"
  ]
  edge
  [
    source 2
    target 4
    label "s1"
  ]
  edge
  [
    source 4
    target 3
    label "s2"
  ]
]
