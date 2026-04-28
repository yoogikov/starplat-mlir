module {
  "starplat.func"() <{argNames = ["g", "beta", "delta", "maxIter", "pageRank"], functionType = (!starplat.graph, !starplat.spfloat, !starplat.spfloat, !starplat.spint, !starplat.propNode<!starplat.spfloat, "g">) -> i64, sym_name = "ComputePageRank"}> ({
  ^bb0(%arg0: !starplat.graph, %arg1: !starplat.spfloat, %arg2: !starplat.spfloat, %arg3: !starplat.spint, %arg4: !starplat.propNode<!starplat.spfloat, "g">):
    %0 = "starplat.const"() <{sym_name = "const_0", sym_visibility = "public", value = 0 : i64}> : () -> i64
    "starplat.return"(%0) : (i64) -> ()
  }) : () -> ()
}
