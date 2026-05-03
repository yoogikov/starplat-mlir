; ModuleID = 'LLVMDialectModule'
source_filename = "LLVMDialectModule"

declare i32 @get_num_out_neighbours(ptr, i32)

declare ptr @get_in_neighbours(ptr, i32)

declare i32 @get_num_in_neighbours(ptr, i32)

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
  br label %48

48:                                               ; preds = %132, %45
  %49 = phi i1 [ %148, %132 ], [ true, %45 ]
  br i1 %49, label %50, label %149

50:                                               ; preds = %48
  %51 = load double, ptr %47, align 8
  %52 = call i32 @get_num_nodes(ptr %0)
  %53 = sext i32 %52 to i64
  br label %54

54:                                               ; preds = %124, %50
  %55 = phi i64 [ %131, %124 ], [ 0, %50 ]
  %56 = phi double [ %130, %124 ], [ %51, %50 ]
  %57 = icmp slt i64 %55, %53
  br i1 %57, label %58, label %132

58:                                               ; preds = %54
  %59 = trunc i64 %55 to i32
  %60 = alloca i32, align 4
  store i32 %59, ptr %60, align 4
  %61 = alloca ptr, i64 1, align 8
  store double 0.000000e+00, ptr %61, align 8
  %62 = load i32, ptr %60, align 4
  %63 = call i32 @get_num_in_neighbours(ptr %0, i32 %62)
  %64 = sext i32 %63 to i64
  %65 = call ptr @get_in_neighbours(ptr %0, i32 %62)
  br label %66

66:                                               ; preds = %69, %58
  %67 = phi i64 [ %85, %69 ], [ 0, %58 ]
  %68 = icmp slt i64 %67, %64
  br i1 %68, label %69, label %86

69:                                               ; preds = %66
  %70 = getelementptr i32, ptr %65, i64 %67
  %71 = load i32, ptr %70, align 4
  %72 = alloca i32, align 4
  store i32 %71, ptr %72, align 4
  %73 = load i32, ptr %72, align 4
  %74 = sext i32 %73 to i64
  %75 = extractvalue { ptr, ptr, i64, [1 x i64], [1 x i64] } %14, 1
  %76 = getelementptr inbounds nuw double, ptr %75, i64 %74
  %77 = load double, ptr %76, align 8
  %78 = load i32, ptr %72, align 4
  %79 = call i32 @get_num_out_neighbours(ptr %0, i32 %78)
  %80 = sext i32 %79 to i64
  %81 = sitofp i64 %80 to double
  %82 = fdiv double %77, %81
  %83 = load double, ptr %61, align 8
  %84 = fadd double %83, %82
  store double %84, ptr %61, align 8
  %85 = add i64 %67, 1
  br label %66

86:                                               ; preds = %66
  %87 = alloca ptr, i64 1, align 8
  %88 = load double, ptr %2, align 8
  %89 = fsub double 1.000000e+00, %88
  %90 = load double, ptr %15, align 8
  %91 = fdiv double %89, %90
  %92 = load double, ptr %2, align 8
  %93 = load double, ptr %61, align 8
  %94 = fmul double %92, %93
  %95 = fadd double %91, %94
  store double %95, ptr %87, align 8
  %96 = load i32, ptr %60, align 4
  %97 = sext i32 %96 to i64
  %98 = extractvalue { ptr, ptr, i64, [1 x i64], [1 x i64] } %14, 1
  %99 = getelementptr inbounds nuw double, ptr %98, i64 %97
  %100 = load double, ptr %99, align 8
  %101 = load double, ptr %87, align 8
  %102 = fsub double %101, %100
  %103 = fcmp oge double %102, 0.000000e+00
  br i1 %103, label %104, label %113

104:                                              ; preds = %86
  %105 = load i32, ptr %60, align 4
  %106 = sext i32 %105 to i64
  %107 = extractvalue { ptr, ptr, i64, [1 x i64], [1 x i64] } %14, 1
  %108 = getelementptr inbounds nuw double, ptr %107, i64 %106
  %109 = load double, ptr %108, align 8
  %110 = load double, ptr %87, align 8
  %111 = fsub double %110, %109
  %112 = fadd double %111, 0.000000e+00
  br label %122

113:                                              ; preds = %86
  %114 = load i32, ptr %60, align 4
  %115 = sext i32 %114 to i64
  %116 = extractvalue { ptr, ptr, i64, [1 x i64], [1 x i64] } %14, 1
  %117 = getelementptr inbounds nuw double, ptr %116, i64 %115
  %118 = load double, ptr %117, align 8
  %119 = load double, ptr %87, align 8
  %120 = fsub double %118, %119
  %121 = fadd double %120, 0.000000e+00
  br label %122

122:                                              ; preds = %104, %113
  %123 = phi double [ %121, %113 ], [ %112, %104 ]
  br label %124

124:                                              ; preds = %122
  %125 = load i32, ptr %60, align 4
  %126 = sext i32 %125 to i64
  %127 = load double, ptr %87, align 8
  %128 = extractvalue { ptr, ptr, i64, [1 x i64], [1 x i64] } %26, 1
  %129 = getelementptr inbounds nuw double, ptr %128, i64 %126
  store double %127, ptr %129, align 8
  %130 = fadd double %56, %123
  %131 = add i64 %55, 1
  br label %54

132:                                              ; preds = %54
  store double %56, ptr %47, align 8
  %133 = extractvalue { ptr, ptr, i64, [1 x i64], [1 x i64] } %26, 3, 0
  %134 = mul i64 1, %133
  %135 = mul i64 %134, 8
  %136 = extractvalue { ptr, ptr, i64, [1 x i64], [1 x i64] } %26, 1
  %137 = extractvalue { ptr, ptr, i64, [1 x i64], [1 x i64] } %26, 2
  %138 = getelementptr double, ptr %136, i64 %137
  %139 = extractvalue { ptr, ptr, i64, [1 x i64], [1 x i64] } %14, 1
  %140 = extractvalue { ptr, ptr, i64, [1 x i64], [1 x i64] } %14, 2
  %141 = getelementptr double, ptr %139, i64 %140
  call void @llvm.memcpy.p0.p0.i64(ptr %141, ptr %138, i64 %135, i1 false)
  %142 = load double, ptr %47, align 8
  %143 = load double, ptr %1, align 8
  %144 = fcmp ogt double %142, %143
  %145 = load i64, ptr %46, align 4
  %146 = load i64, ptr %3, align 4
  %147 = icmp slt i64 %145, %146
  %148 = and i1 %144, %147
  br label %48

149:                                              ; preds = %48
  ret i64 0
}

; Function Attrs: nocallback nofree nounwind willreturn memory(argmem: readwrite)
declare void @llvm.memcpy.p0.p0.i64(ptr noalias writeonly captures(none), ptr noalias readonly captures(none), i64, i1 immarg) #0

attributes #0 = { nocallback nofree nounwind willreturn memory(argmem: readwrite) }

!llvm.module.flags = !{!0}

!0 = !{i32 2, !"Debug Info Version", i32 3}
