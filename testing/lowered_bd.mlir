module {
  llvm.func @get_num_nodes(!llvm.ptr) -> i32
  llvm.func @ComputePageRank(%arg0: !llvm.ptr, %arg1: !llvm.ptr, %arg2: !llvm.ptr, %arg3: !llvm.ptr, %arg4: !llvm.ptr, %arg5: !llvm.ptr, %arg6: i64, %arg7: i64, %arg8: i64) -> i64 {
    %0 = llvm.mlir.poison : !llvm.struct<(ptr, ptr, i64, array<1 x i64>, array<1 x i64>)>
    %1 = llvm.insertvalue %arg4, %0[0] : !llvm.struct<(ptr, ptr, i64, array<1 x i64>, array<1 x i64>)> 
    %2 = llvm.insertvalue %arg5, %1[1] : !llvm.struct<(ptr, ptr, i64, array<1 x i64>, array<1 x i64>)> 
    %3 = llvm.insertvalue %arg6, %2[2] : !llvm.struct<(ptr, ptr, i64, array<1 x i64>, array<1 x i64>)> 
    %4 = llvm.insertvalue %arg7, %3[3, 0] : !llvm.struct<(ptr, ptr, i64, array<1 x i64>, array<1 x i64>)> 
    %5 = llvm.insertvalue %arg8, %4[4, 0] : !llvm.struct<(ptr, ptr, i64, array<1 x i64>, array<1 x i64>)> 
    %6 = llvm.mlir.constant(1 : index) : i64
    %7 = llvm.mlir.constant(0 : index) : i64
    %8 = llvm.mlir.constant(1.000000e+00 : f64) : f64
    %9 = llvm.mlir.constant(0 : i64) : i64
    %10 = llvm.mlir.constant(0.000000e+00 : f64) : f64
    %11 = llvm.mlir.constant(1 : i64) : i64
    %12 = llvm.alloca %11 x !llvm.ptr : (i64) -> !llvm.ptr
    %13 = llvm.call @get_num_nodes(%arg0) : (!llvm.ptr) -> i32
    %14 = llvm.sext %13 : i32 to i64
    %15 = llvm.sitofp %14 : i64 to f64
    llvm.store %15, %12 : f64, !llvm.ptr
    %16 = llvm.call @get_num_nodes(%arg0) : (!llvm.ptr) -> i32
    %17 = llvm.sext %16 : i32 to i64
    %18 = llvm.mlir.constant(1 : index) : i64
    %19 = llvm.alloca %17 x f64 : (i64) -> !llvm.ptr
    %20 = llvm.mlir.poison : !llvm.struct<(ptr, ptr, i64, array<1 x i64>, array<1 x i64>)>
    %21 = llvm.insertvalue %19, %20[0] : !llvm.struct<(ptr, ptr, i64, array<1 x i64>, array<1 x i64>)> 
    %22 = llvm.insertvalue %19, %21[1] : !llvm.struct<(ptr, ptr, i64, array<1 x i64>, array<1 x i64>)> 
    %23 = llvm.mlir.constant(0 : index) : i64
    %24 = llvm.insertvalue %23, %22[2] : !llvm.struct<(ptr, ptr, i64, array<1 x i64>, array<1 x i64>)> 
    %25 = llvm.insertvalue %17, %24[3, 0] : !llvm.struct<(ptr, ptr, i64, array<1 x i64>, array<1 x i64>)> 
    %26 = llvm.insertvalue %18, %25[4, 0] : !llvm.struct<(ptr, ptr, i64, array<1 x i64>, array<1 x i64>)> 
    %27 = llvm.load %12 : !llvm.ptr -> f64
    %28 = llvm.fdiv %8, %27 : f64
    %29 = llvm.extractvalue %5[3, 0] : !llvm.struct<(ptr, ptr, i64, array<1 x i64>, array<1 x i64>)> 
    llvm.br ^bb1(%7 : i64)
  ^bb1(%30: i64):  // 2 preds: ^bb0, ^bb2
    %31 = llvm.icmp "slt" %30, %29 : i64
    llvm.cond_br %31, ^bb2, ^bb3
  ^bb2:  // pred: ^bb1
    %32 = llvm.extractvalue %5[1] : !llvm.struct<(ptr, ptr, i64, array<1 x i64>, array<1 x i64>)> 
    %33 = llvm.getelementptr inbounds|nuw %32[%30] : (!llvm.ptr, i64) -> !llvm.ptr, f64
    llvm.store %28, %33 : f64, !llvm.ptr
    %34 = llvm.add %30, %6 : i64
    llvm.br ^bb1(%34 : i64)
  ^bb3:  // pred: ^bb1
    llvm.br ^bb4(%7 : i64)
  ^bb4(%35: i64):  // 2 preds: ^bb3, ^bb5
    %36 = llvm.icmp "slt" %35, %17 : i64
    llvm.cond_br %36, ^bb5, ^bb6
  ^bb5:  // pred: ^bb4
    %37 = llvm.extractvalue %26[1] : !llvm.struct<(ptr, ptr, i64, array<1 x i64>, array<1 x i64>)> 
    %38 = llvm.getelementptr inbounds|nuw %37[%35] : (!llvm.ptr, i64) -> !llvm.ptr, f64
    llvm.store %10, %38 : f64, !llvm.ptr
    %39 = llvm.add %35, %6 : i64
    llvm.br ^bb4(%39 : i64)
  ^bb6:  // pred: ^bb4
    %40 = llvm.alloca %11 x !llvm.ptr : (i64) -> !llvm.ptr
    llvm.store %9, %40 : i64, !llvm.ptr
    %41 = llvm.alloca %11 x !llvm.ptr : (i64) -> !llvm.ptr
    llvm.store %10, %41 : f64, !llvm.ptr
    llvm.return %9 : i64
  }
}

