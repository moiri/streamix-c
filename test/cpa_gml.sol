Creator "igraph version 0.7.1 StreamixC"
Version 1
graph
[
  directed 1
  node
  [
    id 0
    label "CarS"
    func "CarS"
  ]
  node
  [
    id 1
    label "CarE"
    func "CarE"
  ]
  node
  [
    id 2
    label "DC"
    func "DC"
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
    label "CPA"
    func "CPA"
  ]
  node
  [
    id 5
    label "ABS"
    func "ABS"
  ]
  node
  [
    id 6
    label "RS"
    func "RS"
  ]
  node
  [
    id 7
    label "B"
    func "B"
  ]
  node
  [
    id 8
    label "MB"
    func "MB"
  ]
  node
  [
    id 9
    label "DS"
    func "DS"
  ]
  node
  [
    id 10
    label "DC"
    func "DC"
  ]
  node
  [
    id 11
    label "smx_cp"
    func "null"
  ]
  node
  [
    id 12
    label "CPA"
    func "CPA"
  ]
  node
  [
    id 13
    label "ABS"
    func "ABS"
  ]
  node
  [
    id 14
    label "RS"
    func "RS"
  ]
  node
  [
    id 15
    label "B"
    func "B"
  ]
  node
  [
    id 16
    label "MB"
    func "MB"
  ]
  node
  [
    id 17
    label "DS"
    func "DS"
  ]
  node
  [
    id 18
    label "DC"
    func "DC"
  ]
  node
  [
    id 19
    label "smx_cp"
    func "null"
  ]
  node
  [
    id 20
    label "CPA"
    func "CPA"
  ]
  node
  [
    id 21
    label "ABS"
    func "ABS"
  ]
  node
  [
    id 22
    label "RS"
    func "RS"
  ]
  node
  [
    id 23
    label "B"
    func "B"
  ]
  node
  [
    id 24
    label "MB"
    func "MB"
  ]
  node
  [
    id 25
    label "DS"
    func "DS"
  ]
  edge
  [
    source 3
    target 2
    label "dist"
  ]
  edge
  [
    source 4
    target 2
    label "cpa"
  ]
  edge
  [
    source 2
    target 4
    label "dc"
  ]
  edge
  [
    source 3
    target 4
    label "dist"
  ]
  edge
  [
    source 6
    target 5
    label "speed"
  ]
  edge
  [
    source 8
    target 5
    label "break_cmd"
  ]
  edge
  [
    source 5
    target 8
    label "block"
  ]
  edge
  [
    source 5
    target 7
    label "break_abs"
  ]
  edge
  [
    source 5
    target 2
    label "abs"
  ]
  edge
  [
    source 9
    target 3
    label "dist"
  ]
  edge
  [
    source 2
    target 5
    label "dc"
  ]
  edge
  [
    source 11
    target 10
    label "dist"
  ]
  edge
  [
    source 12
    target 10
    label "cpa"
  ]
  edge
  [
    source 10
    target 12
    label "dc"
  ]
  edge
  [
    source 11
    target 12
    label "dist"
  ]
  edge
  [
    source 14
    target 13
    label "speed"
  ]
  edge
  [
    source 16
    target 13
    label "break_cmd"
  ]
  edge
  [
    source 13
    target 16
    label "block"
  ]
  edge
  [
    source 13
    target 15
    label "break_abs"
  ]
  edge
  [
    source 13
    target 10
    label "abs"
  ]
  edge
  [
    source 17
    target 11
    label "dist"
  ]
  edge
  [
    source 10
    target 13
    label "dc"
  ]
  edge
  [
    source 19
    target 18
    label "dist"
  ]
  edge
  [
    source 20
    target 18
    label "cpa"
  ]
  edge
  [
    source 18
    target 20
    label "dc"
  ]
  edge
  [
    source 19
    target 20
    label "dist"
  ]
  edge
  [
    source 22
    target 21
    label "speed"
  ]
  edge
  [
    source 24
    target 21
    label "break_cmd"
  ]
  edge
  [
    source 21
    target 24
    label "block"
  ]
  edge
  [
    source 21
    target 23
    label "break_abs"
  ]
  edge
  [
    source 21
    target 18
    label "abs"
  ]
  edge
  [
    source 25
    target 19
    label "dist"
  ]
  edge
  [
    source 18
    target 21
    label "dc"
  ]
  edge
  [
    source 4
    target 12
    label "car_next"
  ]
  edge
  [
    source 12
    target 4
    label "car_prev"
  ]
  edge
  [
    source 4
    target 0
    label "car_prev"
  ]
  edge
  [
    source 0
    target 4
    label "car_next"
  ]
  edge
  [
    source 12
    target 20
    label "car_next"
  ]
  edge
  [
    source 20
    target 12
    label "car_prev"
  ]
  edge
  [
    source 20
    target 1
    label "car_next"
  ]
  edge
  [
    source 1
    target 20
    label "car_prev"
  ]
]
