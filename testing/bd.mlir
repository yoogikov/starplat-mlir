module {
  "starplat.func"() <{argNames = ["g", "beta", "delta", "maxIter", "pageRank"], functionType = (!starplat.graph, !starplat.spfloat, !starplat.spfloat, !starplat.spint, !starplat.propNode<f64, "g">) -> i64, sym_name = "ComputePageRank"}> ({
  ^bb0(%arg0: !starplat.graph, %arg1: !starplat.spfloat, %arg2: !starplat.spfloat, %arg3: !starplat.spint, %arg4: !starplat.propNode<f64, "g">):
    %0 = "starplat.declare2"() <{sym_name = "numNodes", sym_visibility = "public"}> : () -> !starplat.spfloat
    %1 = "starplat.num_nodes"(%arg0) : (!starplat.graph) -> i64
    %2 = "starplat.cast"(%1) : (i64) -> f64
    "starplat.assign"(%0, %2) : (!starplat.spfloat, f64) -> ()
    %3 = "starplat.declare"(%arg0) <{sym_name = "pageRankNext", sym_visibility = "public"}> : (!starplat.graph) -> !starplat.propNode<f64, "g">
    %4 = "starplat.const"() <{sym_name = "const_0", sym_visibility = "private", value = 1 : i64}> : () -> i64
    %5 = "starplat.div"(%4, %0) : (i64, !starplat.spfloat) -> f64
    %6 = "starplat.const"() <{sym_name = "const_1", sym_visibility = "private", value = 0.000000e+00 : f64}> : () -> f64
    "starplat.attachNodeProperty"(%arg0, %arg4, %5, %3, %6) : (!starplat.graph, !starplat.propNode<f64, "g">, f64, !starplat.propNode<f64, "g">, f64) -> ()
    %7 = "starplat.declare2"() <{sym_name = "iterCount", sym_visibility = "public"}> : () -> !starplat.spint
    %8 = "starplat.const"() <{sym_name = "const_2", sym_visibility = "private", value = 0 : i64}> : () -> i64
    "starplat.assign"(%7, %8) : (!starplat.spint, i64) -> ()
    %9 = "starplat.declare2"() <{sym_name = "diff", sym_visibility = "public"}> : () -> !starplat.spfloat
    %10 = "starplat.const"() <{sym_name = "const_3", sym_visibility = "private", value = 0.000000e+00 : f64}> : () -> f64
    "starplat.assign"(%9, %10) : (!starplat.spfloat, f64) -> ()
    "starplat.dowhile"() <{sym_name = "DoWhile"}> ({
      %12 = "starplat.const"() <{sym_name = "inc_iterCount", sym_visibility = "private", value = 1 : i64}> : () -> i64
      "starplat.incassign"(%7, %12) : (!starplat.spint, i64) -> ()
      %13 = "starplat.cmp"(%9, %arg1) <{op = ">"}> : (!starplat.spfloat, !starplat.spfloat) -> i1
      %14 = "starplat.cmp"(%7, %arg3) <{op = "<"}> : (!starplat.spint, !starplat.spint) -> i1
      %15 = "starplat.and"(%13, %14) : (i1, i1) -> i1
      "starplat.dowhile_cond"(%15) : (i1) -> ()
    }) : () -> ()
    %11 = "starplat.const"() <{sym_name = "const_4", sym_visibility = "public", value = 0 : i64}> : () -> i64
    "starplat.return"(%11) : (i64) -> ()
  }) : () -> ()
}
