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
    label "DS"
    func "DS"
  ]
  node
  [
    id 3
    label "RS"
    func "RS"
  ]
  node
  [
    id 4
    label "B"
    func "B"
  ]
  node
  [
    id 5
    label "MB"
    func "MB"
  ]
  node
  [
    id 6
    label "ABS"
    func "ABS"
  ]
  node
  [
    id 7
    label "DC"
    func "DC"
  ]
  node
  [
    id 8
    label "CPA"
    func "CPA"
  ]
  node
  [
    id 9
    label "smx_cp"
    func "null"
  ]
  node
  [
    id 10
    label "DS"
    func "DS"
  ]
  node
  [
    id 11
    label "RS"
    func "RS"
  ]
  node
  [
    id 12
    label "B"
    func "B"
  ]
  node
  [
    id 13
    label "MB"
    func "MB"
  ]
  node
  [
    id 14
    label "ABS"
    func "ABS"
  ]
  node
  [
    id 15
    label "DC"
    func "DC"
  ]
  node
  [
    id 16
    label "CPA"
    func "CPA"
  ]
  node
  [
    id 17
    label "smx_cp"
    func "null"
  ]
  node
  [
    id 18
    label "DS"
    func "DS"
  ]
  node
  [
    id 19
    label "RS"
    func "RS"
  ]
  node
  [
    id 20
    label "B"
    func "B"
  ]
  node
  [
    id 21
    label "MB"
    func "MB"
  ]
  node
  [
    id 22
    label "ABS"
    func "ABS"
  ]
  node
  [
    id 23
    label "DC"
    func "DC"
  ]
  node
  [
    id 24
    label "CPA"
    func "CPA"
  ]
  node
  [
    id 25
    label "smx_cp"
    func "null"
  ]
  edge
  [
    source 6
    target 5
    label "block"
  ]
  edge
  [
    source 5
    target 6
    label "break_cmd"
  ]
  edge
  [
    source 3
    target 6
    label "speed"
  ]
  edge
  [
    source 6
    target 4
    label "break_abs"
  ]
  edge
  [
    source 7
    target 6
    label "dc"
  ]
  edge
  [
    source 6
    target 7
    label "abs"
  ]
  edge
  [
    source 8
    target 7
    label "cpa"
  ]
  edge
  [
    source 7
    target 8
    label "dc"
  ]
  edge
  [
    source 9
    target 8
    label "dist"
  ]
  edge
  [
    source 9
    target 7
    label "dist"
  ]
  edge
  [
    source 2
    target 9
    label "dist"
  ]
  edge
  [
    source 14
    target 13
    label "block"
  ]
  edge
  [
    source 13
    target 14
    label "break_cmd"
  ]
  edge
  [
    source 11
    target 14
    label "speed"
  ]
  edge
  [
    source 14
    target 12
    label "break_abs"
  ]
  edge
  [
    source 15
    target 14
    label "dc"
  ]
  edge
  [
    source 14
    target 15
    label "abs"
  ]
  edge
  [
    source 16
    target 15
    label "cpa"
  ]
  edge
  [
    source 15
    target 16
    label "dc"
  ]
  edge
  [
    source 17
    target 16
    label "dist"
  ]
  edge
  [
    source 17
    target 15
    label "dist"
  ]
  edge
  [
    source 10
    target 17
    label "dist"
  ]
  edge
  [
    source 22
    target 21
    label "block"
  ]
  edge
  [
    source 21
    target 22
    label "break_cmd"
  ]
  edge
  [
    source 19
    target 22
    label "speed"
  ]
  edge
  [
    source 22
    target 20
    label "break_abs"
  ]
  edge
  [
    source 23
    target 22
    label "dc"
  ]
  edge
  [
    source 22
    target 23
    label "abs"
  ]
  edge
  [
    source 24
    target 23
    label "cpa"
  ]
  edge
  [
    source 23
    target 24
    label "dc"
  ]
  edge
  [
    source 25
    target 24
    label "dist"
  ]
  edge
  [
    source 25
    target 23
    label "dist"
  ]
  edge
  [
    source 18
    target 25
    label "dist"
  ]
  edge
  [
    source 8
    target 0
    label "car_prev"
  ]
  edge
  [
    source 0
    target 8
    label "car_next"
  ]
  edge
  [
    source 16
    target 8
    label "car_prev"
  ]
  edge
  [
    source 8
    target 16
    label "car_next"
  ]
  edge
  [
    source 24
    target 16
    label "car_prev"
  ]
  edge
  [
    source 16
    target 24
    label "car_next"
  ]
  edge
  [
    source 1
    target 24
    label "car_prev"
  ]
  edge
  [
    source 24
    target 1
    label "car_next"
  ]
]
