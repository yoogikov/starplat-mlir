; ModuleID = 'LLVMDialectModule'
source_filename = "LLVMDialectModule"

declare i32 @get_num_nodes(ptr)

define i64 @ComputePageRank(ptr %0, ptr %1, ptr %2, ptr %3, ptr %4, ptr %5, i64 %6, i64 %7, i64 %8) {
  %10 = insertvalue { ptr, ptr, i64, [1 x i64], [1 x i64] } poison, ptr %4, 0
  %11 = insertvalue { ptr, ptr, i64, [1 x i64], [1 x i64] } %10, ptr %5, 1
  %12 = insertvalue { ptr, ptr, i64, [1 x i64], [1 x i64] } %11, i64 %6, 2
  %13 = insertvalue { ptr, ptr, i64, [1 x i64], [1 x i64] } %12, i64 %7, 3, 0
  %14 = insertvalue { ptr, ptr, i64, [1 x i64], [1 x i64] } %13, i64 %8, 4, 0
  %15 = alloca ptr, i64 1, align 8
  %16 = call i32 @get_num_nodes(ptr %0)
  %17 = sext i32 %16 to i64
  %18 = sitofp i64 %17 to double
  store double %18, ptr %15, align 8
  %19 = call i32 @get_num_nodes(ptr %0)
  %20 = sext i32 %19 to i64
  %21 = alloca double, i64 %20, align 8
  %22 = insertvalue { ptr, ptr, i64, [1 x i64], [1 x i64] } poison, ptr %21, 0
  %23 = insertvalue { ptr, ptr, i64, [1 x i64], [1 x i64] } %22, ptr %21, 1
  %24 = insertvalue { ptr, ptr, i64, [1 x i64], [1 x i64] } %23, i64 0, 2
  %25 = insertvalue { ptr, ptr, i64, [1 x i64], [1 x i64] } %24, i64 %20, 3, 0
  %26 = insertvalue { ptr, ptr, i64, [1 x i64], [1 x i64] } %25, i64 1, 4, 0
  %27 = load double, ptr %15, align 8
  %28 = fdiv double 1.000000e+00, %27
  %29 = extractvalue { ptr, ptr, i64, [1 x i64], [1 x i64] } %14, 3, 0
  br label %30

30:                                               ; preds = %33, %9
  %31 = phi i64 [ %36, %33 ], [ 0, %9 ]
  %32 = icmp slt i64 %31, %29
  br i1 %32, label %33, label %37

33:                                               ; preds = %30
  %34 = extractvalue { ptr, ptr, i64, [1 x i64], [1 x i64] } %14, 1
  %35 = getelementptr inbounds nuw double, ptr %34, i64 %31
  store double %28, ptr %35, align 8
  %36 = add i64 %31, 1
  br label %30

37:                                               ; preds = %30
  br label %38

38:                                               ; preds = %41, %37
  %39 = phi i64 [ %44, %41 ], [ 0, %37 ]
  %40 = icmp slt i64 %39, %20
  br i1 %40, label %41, label %45

41:                                               ; preds = %38
  %42 = extractvalue { ptr, ptr, i64, [1 x i64], [1 x i64] } %26, 1
  %43 = getelementptr inbounds nuw double, ptr %42, i64 %39
  store double 0.000000e+00, ptr %43, align 8
  %44 = add i64 %39, 1
  br label %38

45:                                               ; preds = %38
  %46 = alloca ptr, i64 1, align 8
  store i64 0, ptr %46, align 4
  %47 = alloca ptr, i64 1, align 8
  store double 0.000000e+00, ptr %47, align 8
  ret i64 0
}

!llvm.module.flags = !{!0}

!0 = !{i32 2, !"Debug Info Version", i32 3}
