module {
  "starplat.func"() <{argNames = ["g", "dist", "weight", "src"], functionType = (!starplat.graph, !starplat.propNode<i64, "g">, !starplat.propEdge<i64, "g">, !starplat.node) -> i64, sym_name = "Compute_SSSP"}> ({
  ^bb0(%arg0: !starplat.graph, %arg1: !starplat.propNode<i64, "g">, %arg2: !starplat.propEdge<i64, "g">, %arg3: !starplat.node):
    %0 = "starplat.declare"(%arg0) <{sym_name = "modified_nxt", sym_visibility = "public"}> : (!starplat.graph) -> !starplat.propNode<i1, "g">
    %1 = "starplat.declare"(%arg0) <{sym_name = "modified", sym_visibility = "public"}> : (!starplat.graph) -> !starplat.propNode<i1, "g">
    %2 = "starplat.const"() <{sym_name = "False", sym_visibility = "public", value = "False"}> : () -> i1
    %3 = "starplat.const"() <{sym_name = "INF", sym_visibility = "public", value = "INF"}> : () -> i64
    "starplat.attachNodeProperty"(%arg0, %1, %2, %0, %2, %arg1, %3) : (!starplat.graph, !starplat.propNode<i1, "g">, i1, !starplat.propNode<i1, "g">, i1, !starplat.propNode<i64, "g">, i64) -> ()
    %4 = "starplat.const"() <{sym_name = "0", sym_visibility = "public", value = "0"}> : () -> i64
    "starplat.setNodeProperty"(%arg1, %arg3, %arg1, %4) : (!starplat.propNode<i64, "g">, !starplat.node, !starplat.propNode<i64, "g">, i64) -> ()
    %5 = "starplat.const"() <{sym_name = "True", sym_visibility = "public", value = "True"}> : () -> i1
    "starplat.setNodeProperty"(%arg0, %arg3, %1, %5) : (!starplat.graph, !starplat.node, !starplat.propNode<i1, "g">, i1) -> ()
    %6 = "starplat.declare2"() <{sym_name = "finished", sym_visibility = "public"}> : () -> !starplat.spint
    "starplat.assign"(%6, %2) : (!starplat.spint, i1) -> ()
    "starplat.fixedPointUntil"(%6, %1) <{sym_name = "FixedPnt", terminationCondition = ["NOT"]}> ({
      "starplat.forallnodes"(%arg0) <{sym_name = "loopa"}> ({
      ^bb0(%arg4: !starplat.node):
        %8 = "starplat.getNodeProperty"(%arg4, %1) <{propertyAttr = "modified"}> : (!starplat.node, !starplat.propNode<i1, "g">) -> i1
        %9 = "starplat.cmp"(%8, %5) <{op = "=="}> : (i1, i1) -> i1
        "starplat.spif"(%9) ({
          "starplat.forallneighbours"(%arg0, %arg4) <{sym_name = "loopa"}> ({
          ^bb0(%arg5: !starplat.node):
            %10 = "starplat.declare2"() <{sym_name = "e", sym_visibility = "public"}> : () -> !starplat.edge
            %11 = "starplat.getedge"(%arg0, %arg4, %arg5) : (!starplat.graph, !starplat.node, !starplat.node) -> !starplat.edge
            "starplat.assign"(%10, %11) : (!starplat.edge, !starplat.edge) -> ()
            %12 = "starplat.getNodeProperty"(%arg5, %arg1) <{propertyAttr = "dist"}> : (!starplat.node, !starplat.propNode<i64, "g">) -> i64
            %13 = "starplat.getNodeProperty"(%arg4, %arg1) <{propertyAttr = "dist"}> : (!starplat.node, !starplat.propNode<i64, "g">) -> i64
            %14 = "starplat.getEdgeProperty"(%10, %arg2) <{property = "weight"}> : (!starplat.edge, !starplat.propEdge<i64, "g">) -> i64
            %15 = "starplat.add"(%13, %14) : (i64, i64) -> i64
            "starplat.min"(%arg5, %arg1, %arg5, %0, %12, %15, %5) : (!starplat.node, !starplat.propNode<i64, "g">, !starplat.node, !starplat.propNode<i1, "g">, i64, i64, i1) -> ()
            "starplat.end"() : () -> ()
          }) : (!starplat.graph, !starplat.node) -> ()
          "starplat.end"() : () -> ()
        }) : (i1) -> ()
        "starplat.end"() : () -> ()
      }) : (!starplat.graph) -> ()
      "starplat.store"(%1, %0) : (!starplat.propNode<i1, "g">, !starplat.propNode<i1, "g">) -> ()
      "starplat.attachNodeProperty"(%arg0, %0, %2) : (!starplat.graph, !starplat.propNode<i1, "g">, i1) -> ()
      "starplat.end"() : () -> ()
    }) {sym_visibility = "nested"} : (!starplat.spint, !starplat.propNode<i1, "g">) -> ()
    %7 = "starplat.const"() <{sym_name = "const_0", sym_visibility = "public", value = 0 : i64}> : () -> i64
    "starplat.return"(%7) : (i64) -> ()
  }) : () -> ()
}
