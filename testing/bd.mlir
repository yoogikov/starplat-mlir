module {
  "starplat.func"() <{argNames = ["g", "beta", "delta", "maxIter", "pageRank"], functionType = (!starplat.graph, !starplat.spfloat, !starplat.spfloat, !starplat.spint, !starplat.propNode<!starplat.spfloat, "g">) -> i64, sym_name = "ComputePageRank"}> ({
  ^bb0(%arg0: !starplat.graph, %arg1: !starplat.spfloat, %arg2: !starplat.spfloat, %arg3: !starplat.spint, %arg4: !starplat.propNode<!starplat.spfloat, "g">):
    %0 = "starplat.declare2"() <{sym_name = "numNodes", sym_visibility = "public"}> : () -> !starplat.spfloat
    %1 = "starplat.num_nodes"(%arg0) : (!starplat.graph) -> i64
    %2 = "starplat.cast"(%1) : (i64) -> !starplat.spfloat
    "starplat.assign"(%0, %2) : (!starplat.spfloat, !starplat.spfloat) -> ()
    %3 = "starplat.const"() <{sym_name = "const_0", sym_visibility = "public", value = 0 : i64}> : () -> i64
    "starplat.return"(%3) : (i64) -> ()
  }) : () -> ()
}
