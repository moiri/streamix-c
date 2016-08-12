Creator "igraph version 0.7.1 StreamixC"
Version 1
graph
[
  directed 1
  node
  [
    id 0
    label "X"
    func "funcX"
  ]
  node
  [
    id 1
    label "Z"
    func "funcZ"
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
    label "A"
    func "funcA"
  ]
  node
  [
    id 4
    label "B"
    func "funcB"
  ]
  edge
  [
    source 1
    target 2
    label "x"
  ]
  edge
  [
    source 3
    target 4
    label "b"
  ]
  edge
  [
    source 2
    target 4
    label "x"
  ]
  edge
  [
    source 2
    target 3
    label "x"
  ]
  edge
  [
    source 4
    target 1
    label "c"
  ]
  edge
  [
    source 0
    target 3
    label "a"
  ]
]
