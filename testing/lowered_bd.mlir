module {
  llvm.func @get_edge_weight(!llvm.ptr) -> i64
  llvm.func @get_edge(!llvm.ptr, i32, i32) -> !llvm.ptr
  llvm.func @get_neighbours(!llvm.ptr, i32) -> !llvm.ptr
  llvm.func @get_num_neighbours(!llvm.ptr, i32) -> i32
  llvm.func @get_num_nodes(!llvm.ptr) -> i32
  omp.declare_reduction @__scf_reduction : i8 init {
  ^bb0(%arg0: i8):
    %0 = llvm.mlir.constant(0 : i8) : i8
    omp.yield(%0 : i8)
  } combiner {
  ^bb0(%arg0: i8, %arg1: i8):
    %0 = llvm.or %arg0, %arg1 : i8
    omp.yield(%0 : i8)
  } atomic {
  ^bb0(%arg0: !llvm.ptr, %arg1: !llvm.ptr):
    %0 = llvm.load %arg1 : !llvm.ptr -> i8
    %1 = llvm.atomicrmw _or %arg0, %0 monotonic : !llvm.ptr, i8
    omp.yield
  }
  llvm.func @Compute_SSSP(%arg0: !llvm.ptr, %arg1: !llvm.ptr, %arg2: !llvm.ptr, %arg3: i64, %arg4: i64, %arg5: i64, %arg6: !llvm.ptr, %arg7: !llvm.ptr, %arg8: i64, %arg9: i64, %arg10: i64, %arg11: !llvm.ptr) -> i64 {
    %0 = llvm.mlir.poison : !llvm.struct<(ptr, ptr, i64, array<1 x i64>, array<1 x i64>)>
    %1 = llvm.insertvalue %arg1, %0[0] : !llvm.struct<(ptr, ptr, i64, array<1 x i64>, array<1 x i64>)> 
    %2 = llvm.insertvalue %arg2, %1[1] : !llvm.struct<(ptr, ptr, i64, array<1 x i64>, array<1 x i64>)> 
    %3 = llvm.insertvalue %arg3, %2[2] : !llvm.struct<(ptr, ptr, i64, array<1 x i64>, array<1 x i64>)> 
    %4 = llvm.insertvalue %arg4, %3[3, 0] : !llvm.struct<(ptr, ptr, i64, array<1 x i64>, array<1 x i64>)> 
    %5 = llvm.insertvalue %arg5, %4[4, 0] : !llvm.struct<(ptr, ptr, i64, array<1 x i64>, array<1 x i64>)> 
    %6 = llvm.mlir.constant(1 : i8) : i8
    %7 = llvm.mlir.constant(0 : i8) : i8
    %8 = llvm.mlir.constant(1 : index) : i64
    %9 = llvm.mlir.constant(0 : index) : i64
    %10 = llvm.mlir.constant(1 : i32) : i32
    %11 = llvm.mlir.constant(1 : i64) : i64
    %12 = llvm.mlir.constant(0 : i64) : i64
    %13 = llvm.mlir.constant(2147483647 : i64) : i64
    %14 = llvm.mlir.constant(false) : i1
    %15 = llvm.call @get_num_nodes(%arg0) : (!llvm.ptr) -> i32
    %16 = llvm.sext %15 : i32 to i64
    %17 = llvm.mlir.constant(1 : index) : i64
    %18 = llvm.alloca %16 x i8 : (i64) -> !llvm.ptr
    %19 = llvm.mlir.poison : !llvm.struct<(ptr, ptr, i64, array<1 x i64>, array<1 x i64>)>
    %20 = llvm.insertvalue %18, %19[0] : !llvm.struct<(ptr, ptr, i64, array<1 x i64>, array<1 x i64>)> 
    %21 = llvm.insertvalue %18, %20[1] : !llvm.struct<(ptr, ptr, i64, array<1 x i64>, array<1 x i64>)> 
    %22 = llvm.mlir.constant(0 : index) : i64
    %23 = llvm.insertvalue %22, %21[2] : !llvm.struct<(ptr, ptr, i64, array<1 x i64>, array<1 x i64>)> 
    %24 = llvm.insertvalue %16, %23[3, 0] : !llvm.struct<(ptr, ptr, i64, array<1 x i64>, array<1 x i64>)> 
    %25 = llvm.insertvalue %17, %24[4, 0] : !llvm.struct<(ptr, ptr, i64, array<1 x i64>, array<1 x i64>)> 
    %26 = llvm.call @get_num_nodes(%arg0) : (!llvm.ptr) -> i32
    %27 = llvm.sext %26 : i32 to i64
    %28 = llvm.mlir.constant(1 : index) : i64
    %29 = llvm.alloca %27 x i8 : (i64) -> !llvm.ptr
    %30 = llvm.mlir.poison : !llvm.struct<(ptr, ptr, i64, array<1 x i64>, array<1 x i64>)>
    %31 = llvm.insertvalue %29, %30[0] : !llvm.struct<(ptr, ptr, i64, array<1 x i64>, array<1 x i64>)> 
    %32 = llvm.insertvalue %29, %31[1] : !llvm.struct<(ptr, ptr, i64, array<1 x i64>, array<1 x i64>)> 
    %33 = llvm.mlir.constant(0 : index) : i64
    %34 = llvm.insertvalue %33, %32[2] : !llvm.struct<(ptr, ptr, i64, array<1 x i64>, array<1 x i64>)> 
    %35 = llvm.insertvalue %27, %34[3, 0] : !llvm.struct<(ptr, ptr, i64, array<1 x i64>, array<1 x i64>)> 
    %36 = llvm.insertvalue %28, %35[4, 0] : !llvm.struct<(ptr, ptr, i64, array<1 x i64>, array<1 x i64>)> 
    llvm.br ^bb1(%9 : i64)
  ^bb1(%37: i64):  // 2 preds: ^bb0, ^bb2
    %38 = llvm.icmp "slt" %37, %27 : i64
    llvm.cond_br %38, ^bb2, ^bb3
  ^bb2:  // pred: ^bb1
    %39 = llvm.extractvalue %36[1] : !llvm.struct<(ptr, ptr, i64, array<1 x i64>, array<1 x i64>)> 
    %40 = llvm.getelementptr inbounds|nuw %39[%37] : (!llvm.ptr, i64) -> !llvm.ptr, i8
    llvm.store %7, %40 : i8, !llvm.ptr
    %41 = llvm.add %37, %8 : i64
    llvm.br ^bb1(%41 : i64)
  ^bb3:  // pred: ^bb1
    llvm.br ^bb4(%9 : i64)
  ^bb4(%42: i64):  // 2 preds: ^bb3, ^bb5
    %43 = llvm.icmp "slt" %42, %16 : i64
    llvm.cond_br %43, ^bb5, ^bb6
  ^bb5:  // pred: ^bb4
    %44 = llvm.extractvalue %25[1] : !llvm.struct<(ptr, ptr, i64, array<1 x i64>, array<1 x i64>)> 
    %45 = llvm.getelementptr inbounds|nuw %44[%42] : (!llvm.ptr, i64) -> !llvm.ptr, i8
    llvm.store %7, %45 : i8, !llvm.ptr
    %46 = llvm.add %42, %8 : i64
    llvm.br ^bb4(%46 : i64)
  ^bb6:  // pred: ^bb4
    %47 = llvm.extractvalue %5[3, 0] : !llvm.struct<(ptr, ptr, i64, array<1 x i64>, array<1 x i64>)> 
    llvm.br ^bb7(%9 : i64)
  ^bb7(%48: i64):  // 2 preds: ^bb6, ^bb8
    %49 = llvm.icmp "slt" %48, %47 : i64
    llvm.cond_br %49, ^bb8, ^bb9
  ^bb8:  // pred: ^bb7
    %50 = llvm.extractvalue %5[1] : !llvm.struct<(ptr, ptr, i64, array<1 x i64>, array<1 x i64>)> 
    %51 = llvm.getelementptr inbounds|nuw %50[%48] : (!llvm.ptr, i64) -> !llvm.ptr, i64
    llvm.store %13, %51 : i64, !llvm.ptr
    %52 = llvm.add %48, %8 : i64
    llvm.br ^bb7(%52 : i64)
  ^bb9:  // pred: ^bb7
    %53 = llvm.load %arg11 : !llvm.ptr -> i32
    %54 = llvm.sext %53 : i32 to i64
    %55 = llvm.extractvalue %5[1] : !llvm.struct<(ptr, ptr, i64, array<1 x i64>, array<1 x i64>)> 
    %56 = llvm.getelementptr inbounds|nuw %55[%54] : (!llvm.ptr, i64) -> !llvm.ptr, i64
    llvm.store %12, %56 : i64, !llvm.ptr
    %57 = llvm.load %arg11 : !llvm.ptr -> i32
    %58 = llvm.sext %57 : i32 to i64
    %59 = llvm.extractvalue %36[1] : !llvm.struct<(ptr, ptr, i64, array<1 x i64>, array<1 x i64>)> 
    %60 = llvm.getelementptr inbounds|nuw %59[%58] : (!llvm.ptr, i64) -> !llvm.ptr, i8
    llvm.store %6, %60 : i8, !llvm.ptr
    %61 = llvm.alloca %11 x !llvm.ptr : (i64) -> !llvm.ptr
    llvm.store %14, %61 : i1, !llvm.ptr
    llvm.br ^bb10
  ^bb10:  // 2 preds: ^bb9, ^bb13
    %62 = llvm.call @get_num_nodes(%arg0) : (!llvm.ptr) -> i32
    %63 = llvm.sext %62 : i32 to i64
    %64 = llvm.mlir.constant(1 : i64) : i64
    omp.parallel {
      omp.wsloop {
        omp.loop_nest (%arg12) : i64 = (%9) to (%63) step (%8) {
          %87 = llvm.intr.stacksave : !llvm.ptr
          llvm.br ^bb1
        ^bb1:  // pred: ^bb0
          %88 = llvm.trunc %arg12 : i64 to i32
          %89 = llvm.alloca %10 x i32 : (i32) -> !llvm.ptr
          llvm.store %88, %89 : i32, !llvm.ptr
          %90 = llvm.load %89 : !llvm.ptr -> i32
          %91 = llvm.sext %90 : i32 to i64
          %92 = llvm.extractvalue %36[1] : !llvm.struct<(ptr, ptr, i64, array<1 x i64>, array<1 x i64>)> 
          %93 = llvm.getelementptr inbounds|nuw %92[%91] : (!llvm.ptr, i64) -> !llvm.ptr, i8
          %94 = llvm.load %93 : !llvm.ptr -> i8
          %95 = llvm.icmp "ne" %94, %7 : i8
          llvm.cond_br %95, ^bb2, ^bb8
        ^bb2:  // pred: ^bb1
          %96 = llvm.load %89 : !llvm.ptr -> i32
          %97 = llvm.call @get_num_neighbours(%arg0, %96) : (!llvm.ptr, i32) -> i32
          %98 = llvm.sext %97 : i32 to i64
          %99 = llvm.call @get_neighbours(%arg0, %96) : (!llvm.ptr, i32) -> !llvm.ptr
          llvm.br ^bb3(%9 : i64)
        ^bb3(%100: i64):  // 2 preds: ^bb2, ^bb6
          %101 = llvm.icmp "slt" %100, %98 : i64
          llvm.cond_br %101, ^bb4, ^bb7
        ^bb4:  // pred: ^bb3
          %102 = llvm.getelementptr %99[%100] : (!llvm.ptr, i64) -> !llvm.ptr, i32
          %103 = llvm.load %102 : !llvm.ptr -> i32
          %104 = llvm.alloca %10 x i32 : (i32) -> !llvm.ptr
          llvm.store %103, %104 : i32, !llvm.ptr
          %105 = llvm.alloca %11 x !llvm.ptr : (i64) -> !llvm.ptr
          %106 = llvm.load %89 : !llvm.ptr -> i32
          %107 = llvm.load %104 : !llvm.ptr -> i32
          %108 = llvm.call @get_edge(%arg0, %106, %107) : (!llvm.ptr, i32, i32) -> !llvm.ptr
          llvm.store %108, %105 : !llvm.ptr, !llvm.ptr
          %109 = llvm.load %104 : !llvm.ptr -> i32
          %110 = llvm.sext %109 : i32 to i64
          %111 = llvm.extractvalue %5[1] : !llvm.struct<(ptr, ptr, i64, array<1 x i64>, array<1 x i64>)> 
          %112 = llvm.getelementptr inbounds|nuw %111[%110] : (!llvm.ptr, i64) -> !llvm.ptr, i64
          %113 = llvm.load %112 : !llvm.ptr -> i64
          %114 = llvm.load %89 : !llvm.ptr -> i32
          %115 = llvm.sext %114 : i32 to i64
          %116 = llvm.extractvalue %5[1] : !llvm.struct<(ptr, ptr, i64, array<1 x i64>, array<1 x i64>)> 
          %117 = llvm.getelementptr inbounds|nuw %116[%115] : (!llvm.ptr, i64) -> !llvm.ptr, i64
          %118 = llvm.load %117 : !llvm.ptr -> i64
          %119 = llvm.load %105 : !llvm.ptr -> !llvm.ptr
          %120 = llvm.call @get_edge_weight(%119) : (!llvm.ptr) -> i64
          %121 = llvm.add %118, %120 : i64
          %122 = llvm.load %104 : !llvm.ptr -> i32
          %123 = llvm.load %104 : !llvm.ptr -> i32
          %124 = llvm.sext %122 : i32 to i64
          %125 = llvm.sext %123 : i32 to i64
          %126 = llvm.intr.smin(%113, %121) : (i64, i64) -> i64
          %127 = llvm.extractvalue %5[1] : !llvm.struct<(ptr, ptr, i64, array<1 x i64>, array<1 x i64>)> 
          %128 = llvm.getelementptr %127[%124] : (!llvm.ptr, i64) -> !llvm.ptr, i64
          %129 = llvm.atomicrmw min %128, %126 acq_rel : !llvm.ptr, i64
          %130 = llvm.icmp "ne" %129, %126 : i64
          llvm.cond_br %130, ^bb5, ^bb6
        ^bb5:  // pred: ^bb4
          %131 = llvm.extractvalue %25[1] : !llvm.struct<(ptr, ptr, i64, array<1 x i64>, array<1 x i64>)> 
          %132 = llvm.getelementptr %131[%125] : (!llvm.ptr, i64) -> !llvm.ptr, i8
          %133 = llvm.atomicrmw xchg %132, %6 acq_rel : !llvm.ptr, i8
          llvm.br ^bb6
        ^bb6:  // 2 preds: ^bb4, ^bb5
          %134 = llvm.add %100, %8 : i64
          llvm.br ^bb3(%134 : i64)
        ^bb7:  // pred: ^bb3
          llvm.br ^bb8
        ^bb8:  // 2 preds: ^bb1, ^bb7
          llvm.intr.stackrestore %87 : !llvm.ptr
          llvm.br ^bb9
        ^bb9:  // pred: ^bb8
          omp.yield
        }
      }
      omp.terminator
    }
    %65 = llvm.mlir.constant(1 : index) : i64
    %66 = llvm.extractvalue %25[3, 0] : !llvm.struct<(ptr, ptr, i64, array<1 x i64>, array<1 x i64>)> 
    %67 = llvm.mul %65, %66 : i64
    %68 = llvm.mlir.zero : !llvm.ptr
    %69 = llvm.getelementptr %68[1] : (!llvm.ptr) -> !llvm.ptr, i8
    %70 = llvm.ptrtoint %69 : !llvm.ptr to i64
    %71 = llvm.mul %67, %70 : i64
    %72 = llvm.extractvalue %25[1] : !llvm.struct<(ptr, ptr, i64, array<1 x i64>, array<1 x i64>)> 
    %73 = llvm.extractvalue %25[2] : !llvm.struct<(ptr, ptr, i64, array<1 x i64>, array<1 x i64>)> 
    %74 = llvm.getelementptr %72[%73] : (!llvm.ptr, i64) -> !llvm.ptr, i8
    %75 = llvm.extractvalue %36[1] : !llvm.struct<(ptr, ptr, i64, array<1 x i64>, array<1 x i64>)> 
    %76 = llvm.extractvalue %36[2] : !llvm.struct<(ptr, ptr, i64, array<1 x i64>, array<1 x i64>)> 
    %77 = llvm.getelementptr %75[%76] : (!llvm.ptr, i64) -> !llvm.ptr, i8
    "llvm.intr.memcpy"(%77, %74, %71) <{isVolatile = false}> : (!llvm.ptr, !llvm.ptr, i64) -> ()
    llvm.br ^bb11(%9 : i64)
  ^bb11(%78: i64):  // 2 preds: ^bb10, ^bb12
    %79 = llvm.icmp "slt" %78, %16 : i64
    llvm.cond_br %79, ^bb12, ^bb13
  ^bb12:  // pred: ^bb11
    %80 = llvm.extractvalue %25[1] : !llvm.struct<(ptr, ptr, i64, array<1 x i64>, array<1 x i64>)> 
    %81 = llvm.getelementptr inbounds|nuw %80[%78] : (!llvm.ptr, i64) -> !llvm.ptr, i8
    llvm.store %7, %81 : i8, !llvm.ptr
    %82 = llvm.add %78, %8 : i64
    llvm.br ^bb11(%82 : i64)
  ^bb13:  // pred: ^bb11
    %83 = llvm.mlir.constant(1 : i64) : i64
    %84 = llvm.alloca %83 x i8 : (i64) -> !llvm.ptr
    llvm.store %7, %84 : i8, !llvm.ptr
    omp.parallel {
      omp.wsloop reduction(@__scf_reduction %84 -> %arg12 : !llvm.ptr) {
        omp.loop_nest (%arg13) : i64 = (%9) to (%27) step (%8) {
          %87 = llvm.intr.stacksave : !llvm.ptr
          llvm.br ^bb1
        ^bb1:  // pred: ^bb0
          %88 = llvm.extractvalue %36[1] : !llvm.struct<(ptr, ptr, i64, array<1 x i64>, array<1 x i64>)> 
          %89 = llvm.getelementptr inbounds|nuw %88[%arg13] : (!llvm.ptr, i64) -> !llvm.ptr, i8
          %90 = llvm.load %89 : !llvm.ptr -> i8
          %91 = llvm.load %arg12 : !llvm.ptr -> i8
          %92 = llvm.or %91, %90 : i8
          llvm.store %92, %arg12 : i8, !llvm.ptr
          llvm.intr.stackrestore %87 : !llvm.ptr
          llvm.br ^bb2
        ^bb2:  // pred: ^bb1
          omp.yield
        }
      }
      omp.terminator
    }
    %85 = llvm.load %84 : !llvm.ptr -> i8
    %86 = llvm.icmp "ne" %85, %7 : i8
    llvm.cond_br %86, ^bb10, ^bb14
  ^bb14:  // pred: ^bb13
    llvm.return %12 : i64
  }
}

