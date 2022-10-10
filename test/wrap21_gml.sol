Creator "igraph version 0.7.1 StreamixC"
Version 1
graph
[
  directed 1
  node
  [
    id 0
    label "Prod2"
    func "prod2"
    static 0
    pure 0
    location 0
    tt 0
  ]
  node
  [
    id 1
    label "Ctrl"
    func "ctrl"
    static 0
    pure 0
    location 0
    tt 0
  ]
  node
  [
    id 2
    label "smx_rn"
    func "smx_rn"
    static 0
    pure 0
    location 0
    tt 0
  ]
  node
  [
    id 3
    label "smx_rn"
    func "smx_rn"
    static 0
    pure 0
    location 0
    tt 0
  ]
  node
  [
    id 4
    label "Prod1"
    func "prod1"
    static 0
    pure 0
    location 0
    tt 0
  ]
  node
  [
    id 5
    label "Sink"
    func "sink"
    static 0
    pure 0
    location 0
    tt 0
  ]
  node
  [
    id 6
    label "Db"
    func "db"
    static 0
    pure 0
    location 0
    tt 0
  ]
  node
  [
    id 7
    label "smx_rn"
    func "smx_rn"
    static 0
    pure 0
    location 0
    tt 0
  ]
  edge
  [
    source 1
    target 2
    label "trigger"
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
    dynsrc 0
    dyndst 0
  ]
  edge
  [
    source 3
    target 1
    label "event"
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
    dynsrc 0
    dyndst 0
  ]
  edge
  [
    source 0
    target 3
    label "event"
    nsrc "smx_null"
    ndst "smx_null"
    dsrc 1
    ddst 0
    len 1
    dts 0
    dtns 0
    sts 0
    stns 0
    type 0
    dynsrc 0
    dyndst 0
  ]
  edge
  [
    source 4
    target 5
    label "data"
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
    dynsrc 0
    dyndst 0
  ]
  edge
  [
    source 4
    target 3
    label "event"
    nsrc "smx_null"
    ndst "smx_null"
    dsrc 1
    ddst 0
    len 1
    dts 0
    dtns 0
    sts 0
    stns 0
    type 0
    dynsrc 0
    dyndst 0
  ]
  edge
  [
    source 2
    target 5
    label "trigger"
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
    dynsrc 0
    dyndst 0
  ]
  edge
  [
    source 7
    target 6
    label "events"
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
    dynsrc 0
    dyndst 0
  ]
  edge
  [
    source 2
    target 7
    label "trigger"
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
    dynsrc 0
    dyndst 0
  ]
  edge
  [
    source 3
    target 7
    label "event"
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
    dynsrc 0
    dyndst 0
  ]
]
