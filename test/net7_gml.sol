Creator "igraph version 0.7.1 StreamixC"
Version 1
graph
[
  directed 1
  node
  [
    id 0
    label "A"
    func "funcA"
  ]
  node
  [
    id 1
    label "A"
    func "funcA"
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
    label "B"
    func "funcB"
  ]
  node
  [
    id 4
    label "B"
    func "funcB"
  ]
  node
  [
    id 5
    label "smx_cp"
    func "null"
  ]
  node
  [
    id 6
    label "C"
    func "funcC"
  ]
  node
  [
    id 7
    label "C"
    func "funcC"
  ]
  edge
  [
    source 1
    target 2
    label "a"
  ]
  edge
  [
    source 0
    target 2
    label "a"
  ]
  edge
  [
    source 2
    target 4
    label "a"
  ]
  edge
  [
    source 2
    target 3
    label "a"
  ]
  edge
  [
    source 4
    target 5
    label "b"
  ]
  edge
  [
    source 3
    target 5
    label "b"
  ]
  edge
  [
    source 5
    target 7
    label "b"
  ]
  edge
  [
    source 5
    target 6
    label "b"
  ]
]
