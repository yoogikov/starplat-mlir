; ModuleID = 'LLVMDialectModule'
source_filename = "LLVMDialectModule"

%struct.ident_t = type { i32, i32, i32, i32, ptr }

@0 = private unnamed_addr constant [23 x i8] c";unknown;unknown;0;0;;\00", align 1
@1 = private unnamed_addr constant %struct.ident_t { i32 0, i32 2, i32 0, i32 22, ptr @0 }, align 8
@2 = private unnamed_addr constant %struct.ident_t { i32 0, i32 66, i32 0, i32 22, ptr @0 }, align 8
@3 = private unnamed_addr constant %struct.ident_t { i32 0, i32 18, i32 0, i32 22, ptr @0 }, align 8
@.gomp_critical_user_.reduction.var = common global [8 x i32] zeroinitializer, align 8

declare i64 @get_edge_weight(ptr)

declare ptr @get_edge(ptr, i32, i32)

declare ptr @get_neighbours(ptr, i32)

declare i32 @get_num_neighbours(ptr, i32)

declare i32 @get_num_nodes(ptr)

define i64 @Compute_SSSP(ptr %0, ptr %1, ptr %2, i64 %3, i64 %4, i64 %5, ptr %6, ptr %7, i64 %8, i64 %9, i64 %10, ptr %11) {
  %structArg62 = alloca { ptr, ptr, ptr }, align 8
  %structArg = alloca { ptr, ptr, ptr, ptr, ptr }, align 8
  %.reloaded59 = alloca i64, align 8
  %.reloaded60 = alloca { ptr, ptr, i64, [1 x i64], [1 x i64] }, align 8
  %.reloaded = alloca i64, align 8
  %.reloaded16 = alloca { ptr, ptr, i64, [1 x i64], [1 x i64] }, align 8
  %.reloaded17 = alloca { ptr, ptr, i64, [1 x i64], [1 x i64] }, align 8
  %.reloaded18 = alloca { ptr, ptr, i64, [1 x i64], [1 x i64] }, align 8
  %13 = insertvalue { ptr, ptr, i64, [1 x i64], [1 x i64] } poison, ptr %1, 0
  %14 = insertvalue { ptr, ptr, i64, [1 x i64], [1 x i64] } %13, ptr %2, 1
  %15 = insertvalue { ptr, ptr, i64, [1 x i64], [1 x i64] } %14, i64 %3, 2
  %16 = insertvalue { ptr, ptr, i64, [1 x i64], [1 x i64] } %15, i64 %4, 3, 0
  %17 = insertvalue { ptr, ptr, i64, [1 x i64], [1 x i64] } %16, i64 %5, 4, 0
  %18 = call i32 @get_num_nodes(ptr %0)
  %19 = sext i32 %18 to i64
  %20 = alloca i8, i64 %19, align 1
  %21 = insertvalue { ptr, ptr, i64, [1 x i64], [1 x i64] } poison, ptr %20, 0
  %22 = insertvalue { ptr, ptr, i64, [1 x i64], [1 x i64] } %21, ptr %20, 1
  %23 = insertvalue { ptr, ptr, i64, [1 x i64], [1 x i64] } %22, i64 0, 2
  %24 = insertvalue { ptr, ptr, i64, [1 x i64], [1 x i64] } %23, i64 %19, 3, 0
  %25 = insertvalue { ptr, ptr, i64, [1 x i64], [1 x i64] } %24, i64 1, 4, 0
  %26 = call i32 @get_num_nodes(ptr %0)
  %27 = sext i32 %26 to i64
  %28 = alloca i8, i64 %27, align 1
  %29 = insertvalue { ptr, ptr, i64, [1 x i64], [1 x i64] } poison, ptr %28, 0
  %30 = insertvalue { ptr, ptr, i64, [1 x i64], [1 x i64] } %29, ptr %28, 1
  %31 = insertvalue { ptr, ptr, i64, [1 x i64], [1 x i64] } %30, i64 0, 2
  %32 = insertvalue { ptr, ptr, i64, [1 x i64], [1 x i64] } %31, i64 %27, 3, 0
  %33 = insertvalue { ptr, ptr, i64, [1 x i64], [1 x i64] } %32, i64 1, 4, 0
  br label %34

34:                                               ; preds = %37, %12
  %35 = phi i64 [ %40, %37 ], [ 0, %12 ]
  %36 = icmp slt i64 %35, %27
  br i1 %36, label %37, label %41

37:                                               ; preds = %34
  %38 = extractvalue { ptr, ptr, i64, [1 x i64], [1 x i64] } %33, 1
  %39 = getelementptr inbounds nuw i8, ptr %38, i64 %35
  store i8 0, ptr %39, align 1
  %40 = add i64 %35, 1
  br label %34

41:                                               ; preds = %34
  br label %42

42:                                               ; preds = %45, %41
  %43 = phi i64 [ %48, %45 ], [ 0, %41 ]
  %44 = icmp slt i64 %43, %19
  br i1 %44, label %45, label %49

45:                                               ; preds = %42
  %46 = extractvalue { ptr, ptr, i64, [1 x i64], [1 x i64] } %25, 1
  %47 = getelementptr inbounds nuw i8, ptr %46, i64 %43
  store i8 0, ptr %47, align 1
  %48 = add i64 %43, 1
  br label %42

49:                                               ; preds = %42
  %50 = extractvalue { ptr, ptr, i64, [1 x i64], [1 x i64] } %17, 3, 0
  br label %51

51:                                               ; preds = %54, %49
  %52 = phi i64 [ %57, %54 ], [ 0, %49 ]
  %53 = icmp slt i64 %52, %50
  br i1 %53, label %54, label %58

54:                                               ; preds = %51
  %55 = extractvalue { ptr, ptr, i64, [1 x i64], [1 x i64] } %17, 1
  %56 = getelementptr inbounds nuw i64, ptr %55, i64 %52
  store i64 2147483647, ptr %56, align 4
  %57 = add i64 %52, 1
  br label %51

58:                                               ; preds = %51
  %59 = load i32, ptr %11, align 4
  %60 = sext i32 %59 to i64
  %61 = extractvalue { ptr, ptr, i64, [1 x i64], [1 x i64] } %17, 1
  %62 = getelementptr inbounds nuw i64, ptr %61, i64 %60
  store i64 0, ptr %62, align 4
  %63 = load i32, ptr %11, align 4
  %64 = sext i32 %63 to i64
  %65 = extractvalue { ptr, ptr, i64, [1 x i64], [1 x i64] } %33, 1
  %66 = getelementptr inbounds nuw i8, ptr %65, i64 %64
  store i8 1, ptr %66, align 1
  %67 = alloca ptr, i64 1, align 8
  store i1 false, ptr %67, align 1
  br label %68

68:                                               ; preds = %omp.par.exit25, %58
  %69 = call i32 @get_num_nodes(ptr %0)
  %70 = sext i32 %69 to i64
  %omp_global_thread_num = call i32 @__kmpc_global_thread_num(ptr @1)
  store i64 %70, ptr %.reloaded, align 4
  store { ptr, ptr, i64, [1 x i64], [1 x i64] } %33, ptr %.reloaded16, align 8
  store { ptr, ptr, i64, [1 x i64], [1 x i64] } %17, ptr %.reloaded17, align 8
  store { ptr, ptr, i64, [1 x i64], [1 x i64] } %25, ptr %.reloaded18, align 8
  br label %omp_parallel

omp_parallel:                                     ; preds = %68
  %gep_.reloaded = getelementptr { ptr, ptr, ptr, ptr, ptr }, ptr %structArg, i32 0, i32 0
  store ptr %.reloaded, ptr %gep_.reloaded, align 8
  %gep_.reloaded16 = getelementptr { ptr, ptr, ptr, ptr, ptr }, ptr %structArg, i32 0, i32 1
  store ptr %.reloaded16, ptr %gep_.reloaded16, align 8
  %gep_.reloaded17 = getelementptr { ptr, ptr, ptr, ptr, ptr }, ptr %structArg, i32 0, i32 2
  store ptr %.reloaded17, ptr %gep_.reloaded17, align 8
  %gep_.reloaded18 = getelementptr { ptr, ptr, ptr, ptr, ptr }, ptr %structArg, i32 0, i32 3
  store ptr %.reloaded18, ptr %gep_.reloaded18, align 8
  %gep_ = getelementptr { ptr, ptr, ptr, ptr, ptr }, ptr %structArg, i32 0, i32 4
  store ptr %0, ptr %gep_, align 8
  call void (ptr, i32, ptr, ...) @__kmpc_fork_call(ptr @1, i32 1, ptr @Compute_SSSP..omp_par, ptr %structArg)
  br label %omp.par.exit

omp.par.exit:                                     ; preds = %omp_parallel
  %71 = extractvalue { ptr, ptr, i64, [1 x i64], [1 x i64] } %25, 3, 0
  %72 = mul i64 1, %71
  %73 = mul i64 %72, 1
  %74 = extractvalue { ptr, ptr, i64, [1 x i64], [1 x i64] } %25, 1
  %75 = extractvalue { ptr, ptr, i64, [1 x i64], [1 x i64] } %25, 2
  %76 = getelementptr i8, ptr %74, i64 %75
  %77 = extractvalue { ptr, ptr, i64, [1 x i64], [1 x i64] } %33, 1
  %78 = extractvalue { ptr, ptr, i64, [1 x i64], [1 x i64] } %33, 2
  %79 = getelementptr i8, ptr %77, i64 %78
  call void @llvm.memcpy.p0.p0.i64(ptr %79, ptr %76, i64 %73, i1 false)
  br label %80

80:                                               ; preds = %83, %omp.par.exit
  %81 = phi i64 [ %86, %83 ], [ 0, %omp.par.exit ]
  %82 = icmp slt i64 %81, %19
  br i1 %82, label %83, label %87

83:                                               ; preds = %80
  %84 = extractvalue { ptr, ptr, i64, [1 x i64], [1 x i64] } %25, 1
  %85 = getelementptr inbounds nuw i8, ptr %84, i64 %81
  store i8 0, ptr %85, align 1
  %86 = add i64 %81, 1
  br label %80

87:                                               ; preds = %80
  %88 = alloca i8, i64 1, align 1
  store i8 0, ptr %88, align 1
  %omp_global_thread_num19 = call i32 @__kmpc_global_thread_num(ptr @1)
  store i64 %27, ptr %.reloaded59, align 4
  store { ptr, ptr, i64, [1 x i64], [1 x i64] } %33, ptr %.reloaded60, align 8
  br label %omp_parallel64

omp_parallel64:                                   ; preds = %87
  %gep_.reloaded59 = getelementptr { ptr, ptr, ptr }, ptr %structArg62, i32 0, i32 0
  store ptr %.reloaded59, ptr %gep_.reloaded59, align 8
  %gep_.reloaded60 = getelementptr { ptr, ptr, ptr }, ptr %structArg62, i32 0, i32 1
  store ptr %.reloaded60, ptr %gep_.reloaded60, align 8
  %gep_63 = getelementptr { ptr, ptr, ptr }, ptr %structArg62, i32 0, i32 2
  store ptr %88, ptr %gep_63, align 8
  call void (ptr, i32, ptr, ...) @__kmpc_fork_call(ptr @1, i32 1, ptr @Compute_SSSP..omp_par.1, ptr %structArg62)
  br label %omp.par.exit25

omp.par.exit25:                                   ; preds = %omp_parallel64
  %89 = load i8, ptr %88, align 1
  %90 = icmp ne i8 %89, 0
  br i1 %90, label %68, label %91

91:                                               ; preds = %omp.par.exit25
  ret i64 0
}

; Function Attrs: nounwind
define internal void @Compute_SSSP..omp_par.1(ptr noalias %tid.addr20, ptr noalias %zero.addr21, ptr %0) #0 {
omp.par.entry22:
  %gep_.reloaded59 = getelementptr { ptr, ptr, ptr }, ptr %0, i32 0, i32 0
  %loadgep_.reloaded59 = load ptr, ptr %gep_.reloaded59, align 8, !align !1
  %gep_.reloaded60 = getelementptr { ptr, ptr, ptr }, ptr %0, i32 0, i32 1
  %loadgep_.reloaded60 = load ptr, ptr %gep_.reloaded60, align 8, !align !1
  %gep_ = getelementptr { ptr, ptr, ptr }, ptr %0, i32 0, i32 2
  %loadgep_ = load ptr, ptr %gep_, align 8, !align !2
  %p.lastiter51 = alloca i32, align 4
  %p.lowerbound52 = alloca i64, align 8
  %p.upperbound53 = alloca i64, align 8
  %p.stride54 = alloca i64, align 8
  %tid.addr.local26 = alloca i32, align 4
  %1 = load i32, ptr %tid.addr20, align 4
  store i32 %1, ptr %tid.addr.local26, align 4
  %tid27 = load i32, ptr %tid.addr.local26, align 4
  %2 = alloca i8, align 1
  %red.array = alloca [1 x ptr], align 8
  %3 = load i64, ptr %loadgep_.reloaded59, align 4
  %4 = load { ptr, ptr, i64, [1 x i64], [1 x i64] }, ptr %loadgep_.reloaded60, align 8
  br label %omp.region.after_alloca33

omp.region.after_alloca33:                        ; preds = %omp.par.entry22
  br label %omp.region.after_alloca30

omp.region.after_alloca30:                        ; preds = %omp.region.after_alloca33
  br label %omp.par.region23

omp.par.region23:                                 ; preds = %omp.region.after_alloca30
  br label %omp.par.region32

omp.par.region32:                                 ; preds = %omp.par.region23
  br label %omp.reduction.init

omp.reduction.init:                               ; preds = %omp.par.region32
  %omp_orig = load i8, ptr %loadgep_, align 1
  store i8 0, ptr %2, align 1
  br label %omp.wsloop.region35

omp.wsloop.region35:                              ; preds = %omp.reduction.init
  %5 = select i1 false, i64 %3, i64 0
  %6 = select i1 false, i64 0, i64 %3
  %7 = sub nsw i64 %6, %5
  %8 = icmp sle i64 %6, %5
  %9 = sub i64 %7, 1
  %10 = udiv i64 %9, 1
  %11 = add i64 %10, 1
  %12 = icmp ule i64 %7, 1
  %13 = select i1 %12, i64 1, i64 %11
  %omp_loop.tripcount36 = select i1 %8, i64 0, i64 %13
  br label %omp_loop.preheader37

omp_loop.preheader37:                             ; preds = %omp.wsloop.region35
  store i64 0, ptr %p.lowerbound52, align 4
  %14 = sub i64 %omp_loop.tripcount36, 1
  store i64 %14, ptr %p.upperbound53, align 4
  store i64 1, ptr %p.stride54, align 4
  %omp_global_thread_num55 = call i32 @__kmpc_global_thread_num(ptr @1)
  call void @__kmpc_for_static_init_8u(ptr @1, i32 %omp_global_thread_num55, i32 34, ptr %p.lastiter51, ptr %p.lowerbound52, ptr %p.upperbound53, ptr %p.stride54, i64 1, i64 0)
  %15 = load i64, ptr %p.lowerbound52, align 4
  %16 = load i64, ptr %p.upperbound53, align 4
  %17 = sub i64 %16, %15
  %18 = add i64 %17, 1
  br label %omp_loop.header38

omp_loop.header38:                                ; preds = %omp_loop.inc41, %omp_loop.preheader37
  %omp_loop.iv44 = phi i64 [ 0, %omp_loop.preheader37 ], [ %omp_loop.next46, %omp_loop.inc41 ]
  br label %omp_loop.cond39

omp_loop.cond39:                                  ; preds = %omp_loop.header38
  %omp_loop.cmp45 = icmp ult i64 %omp_loop.iv44, %18
  br i1 %omp_loop.cmp45, label %omp_loop.body40, label %omp_loop.exit42

omp_loop.exit42:                                  ; preds = %omp_loop.cond39
  call void @__kmpc_for_static_fini(ptr @1, i32 %omp_global_thread_num55)
  %omp_global_thread_num56 = call i32 @__kmpc_global_thread_num(ptr @1)
  call void @__kmpc_barrier(ptr @2, i32 %omp_global_thread_num56)
  br label %omp_loop.after43

omp_loop.after43:                                 ; preds = %omp_loop.exit42
  br label %omp.region.cont34

omp.region.cont34:                                ; preds = %omp_loop.after43
  %red.array.elem.0 = getelementptr inbounds [1 x ptr], ptr %red.array, i64 0, i64 0
  store ptr %2, ptr %red.array.elem.0, align 8
  %omp_global_thread_num57 = call i32 @__kmpc_global_thread_num(ptr @3)
  %reduce = call i32 @__kmpc_reduce(ptr @3, i32 %omp_global_thread_num57, i32 1, i64 8, ptr %red.array, ptr @.omp.reduction.func, ptr @.gomp_critical_user_.reduction.var)
  switch i32 %reduce, label %reduce.finalize [
    i32 1, label %reduce.switch.nonatomic
    i32 2, label %reduce.switch.atomic
  ]

reduce.switch.atomic:                             ; preds = %omp.region.cont34
  %19 = load i8, ptr %2, align 1
  %20 = atomicrmw or ptr %loadgep_, i8 %19 monotonic, align 1
  br label %reduce.finalize

reduce.switch.nonatomic:                          ; preds = %omp.region.cont34
  %red.value.0 = load i8, ptr %loadgep_, align 1
  %red.private.value.0 = load i8, ptr %2, align 1
  %21 = or i8 %red.value.0, %red.private.value.0
  store i8 %21, ptr %loadgep_, align 1
  call void @__kmpc_end_reduce(ptr @3, i32 %omp_global_thread_num57, ptr @.gomp_critical_user_.reduction.var)
  br label %reduce.finalize

reduce.finalize:                                  ; preds = %reduce.switch.atomic, %reduce.switch.nonatomic, %omp.region.cont34
  %omp_global_thread_num58 = call i32 @__kmpc_global_thread_num(ptr @1)
  call void @__kmpc_barrier(ptr @2, i32 %omp_global_thread_num58)
  br label %omp.region.cont31

omp.region.cont31:                                ; preds = %reduce.finalize
  br label %omp.par.pre_finalize24

omp.par.pre_finalize24:                           ; preds = %omp.region.cont31
  br label %.fini61

.fini61:                                          ; preds = %omp.par.pre_finalize24
  br label %omp.par.exit25.exitStub

omp_loop.body40:                                  ; preds = %omp_loop.cond39
  %22 = add i64 %omp_loop.iv44, %15
  %23 = mul i64 %22, 1
  %24 = add i64 %23, 0
  br label %omp.loop_nest.region48

omp.loop_nest.region48:                           ; preds = %omp_loop.body40
  %25 = call ptr @llvm.stacksave.p0()
  br label %omp.loop_nest.region49

omp.loop_nest.region49:                           ; preds = %omp.loop_nest.region48
  %26 = extractvalue { ptr, ptr, i64, [1 x i64], [1 x i64] } %4, 1
  %27 = getelementptr inbounds nuw i8, ptr %26, i64 %24
  %28 = load i8, ptr %27, align 1
  %29 = load i8, ptr %2, align 1
  %30 = or i8 %29, %28
  store i8 %30, ptr %2, align 1
  call void @llvm.stackrestore.p0(ptr %25)
  br label %omp.loop_nest.region50

omp.loop_nest.region50:                           ; preds = %omp.loop_nest.region49
  br label %omp.region.cont47

omp.region.cont47:                                ; preds = %omp.loop_nest.region50
  br label %omp_loop.inc41

omp_loop.inc41:                                   ; preds = %omp.region.cont47
  %omp_loop.next46 = add nuw i64 %omp_loop.iv44, 1
  br label %omp_loop.header38

omp.par.exit25.exitStub:                          ; preds = %.fini61
  ret void
}

; Function Attrs: nounwind
define internal void @Compute_SSSP..omp_par(ptr noalias %tid.addr, ptr noalias %zero.addr, ptr %0) #0 {
omp.par.entry:
  %gep_.reloaded = getelementptr { ptr, ptr, ptr, ptr, ptr }, ptr %0, i32 0, i32 0
  %loadgep_.reloaded = load ptr, ptr %gep_.reloaded, align 8, !align !1
  %gep_.reloaded16 = getelementptr { ptr, ptr, ptr, ptr, ptr }, ptr %0, i32 0, i32 1
  %loadgep_.reloaded16 = load ptr, ptr %gep_.reloaded16, align 8, !align !1
  %gep_.reloaded17 = getelementptr { ptr, ptr, ptr, ptr, ptr }, ptr %0, i32 0, i32 2
  %loadgep_.reloaded17 = load ptr, ptr %gep_.reloaded17, align 8, !align !1
  %gep_.reloaded18 = getelementptr { ptr, ptr, ptr, ptr, ptr }, ptr %0, i32 0, i32 3
  %loadgep_.reloaded18 = load ptr, ptr %gep_.reloaded18, align 8, !align !1
  %gep_ = getelementptr { ptr, ptr, ptr, ptr, ptr }, ptr %0, i32 0, i32 4
  %loadgep_ = load ptr, ptr %gep_, align 8, !align !2
  %p.lastiter = alloca i32, align 4
  %p.lowerbound = alloca i64, align 8
  %p.upperbound = alloca i64, align 8
  %p.stride = alloca i64, align 8
  %tid.addr.local = alloca i32, align 4
  %1 = load i32, ptr %tid.addr, align 4
  store i32 %1, ptr %tid.addr.local, align 4
  %tid = load i32, ptr %tid.addr.local, align 4
  %2 = load i64, ptr %loadgep_.reloaded, align 4
  %3 = load { ptr, ptr, i64, [1 x i64], [1 x i64] }, ptr %loadgep_.reloaded16, align 8
  %4 = load { ptr, ptr, i64, [1 x i64], [1 x i64] }, ptr %loadgep_.reloaded17, align 8
  %5 = load { ptr, ptr, i64, [1 x i64], [1 x i64] }, ptr %loadgep_.reloaded18, align 8
  br label %omp.region.after_alloca2

omp.region.after_alloca2:                         ; preds = %omp.par.entry
  br label %omp.region.after_alloca

omp.region.after_alloca:                          ; preds = %omp.region.after_alloca2
  br label %omp.par.region

omp.par.region:                                   ; preds = %omp.region.after_alloca
  br label %omp.par.region1

omp.par.region1:                                  ; preds = %omp.par.region
  br label %omp.wsloop.region

omp.wsloop.region:                                ; preds = %omp.par.region1
  %6 = select i1 false, i64 %2, i64 0
  %7 = select i1 false, i64 0, i64 %2
  %8 = sub nsw i64 %7, %6
  %9 = icmp sle i64 %7, %6
  %10 = sub i64 %8, 1
  %11 = udiv i64 %10, 1
  %12 = add i64 %11, 1
  %13 = icmp ule i64 %8, 1
  %14 = select i1 %13, i64 1, i64 %12
  %omp_loop.tripcount = select i1 %9, i64 0, i64 %14
  br label %omp_loop.preheader

omp_loop.preheader:                               ; preds = %omp.wsloop.region
  store i64 0, ptr %p.lowerbound, align 4
  %15 = sub i64 %omp_loop.tripcount, 1
  store i64 %15, ptr %p.upperbound, align 4
  store i64 1, ptr %p.stride, align 4
  %omp_global_thread_num14 = call i32 @__kmpc_global_thread_num(ptr @1)
  call void @__kmpc_for_static_init_8u(ptr @1, i32 %omp_global_thread_num14, i32 34, ptr %p.lastiter, ptr %p.lowerbound, ptr %p.upperbound, ptr %p.stride, i64 1, i64 0)
  %16 = load i64, ptr %p.lowerbound, align 4
  %17 = load i64, ptr %p.upperbound, align 4
  %18 = sub i64 %17, %16
  %19 = add i64 %18, 1
  br label %omp_loop.header

omp_loop.header:                                  ; preds = %omp_loop.inc, %omp_loop.preheader
  %omp_loop.iv = phi i64 [ 0, %omp_loop.preheader ], [ %omp_loop.next, %omp_loop.inc ]
  br label %omp_loop.cond

omp_loop.cond:                                    ; preds = %omp_loop.header
  %omp_loop.cmp = icmp ult i64 %omp_loop.iv, %19
  br i1 %omp_loop.cmp, label %omp_loop.body, label %omp_loop.exit

omp_loop.exit:                                    ; preds = %omp_loop.cond
  call void @__kmpc_for_static_fini(ptr @1, i32 %omp_global_thread_num14)
  %omp_global_thread_num15 = call i32 @__kmpc_global_thread_num(ptr @1)
  call void @__kmpc_barrier(ptr @2, i32 %omp_global_thread_num15)
  br label %omp_loop.after

omp_loop.after:                                   ; preds = %omp_loop.exit
  br label %omp.region.cont3

omp.region.cont3:                                 ; preds = %omp_loop.after
  br label %omp.region.cont

omp.region.cont:                                  ; preds = %omp.region.cont3
  br label %omp.par.pre_finalize

omp.par.pre_finalize:                             ; preds = %omp.region.cont
  br label %.fini

.fini:                                            ; preds = %omp.par.pre_finalize
  br label %omp.par.exit.exitStub

omp_loop.body:                                    ; preds = %omp_loop.cond
  %20 = add i64 %omp_loop.iv, %16
  %21 = mul i64 %20, 1
  %22 = add i64 %21, 0
  br label %omp.loop_nest.region

omp.loop_nest.region:                             ; preds = %omp_loop.body
  %23 = call ptr @llvm.stacksave.p0()
  br label %omp.loop_nest.region5

omp.loop_nest.region5:                            ; preds = %omp.loop_nest.region
  %24 = trunc i64 %22 to i32
  %25 = alloca i32, align 4
  store i32 %24, ptr %25, align 4
  %26 = load i32, ptr %25, align 4
  %27 = sext i32 %26 to i64
  %28 = extractvalue { ptr, ptr, i64, [1 x i64], [1 x i64] } %3, 1
  %29 = getelementptr inbounds nuw i8, ptr %28, i64 %27
  %30 = load i8, ptr %29, align 1
  %31 = icmp ne i8 %30, 0
  br i1 %31, label %omp.loop_nest.region6, label %omp.loop_nest.region12

omp.loop_nest.region12:                           ; preds = %omp.loop_nest.region11, %omp.loop_nest.region5
  call void @llvm.stackrestore.p0(ptr %23)
  br label %omp.loop_nest.region13

omp.loop_nest.region13:                           ; preds = %omp.loop_nest.region12
  br label %omp.region.cont4

omp.region.cont4:                                 ; preds = %omp.loop_nest.region13
  br label %omp_loop.inc

omp_loop.inc:                                     ; preds = %omp.region.cont4
  %omp_loop.next = add nuw i64 %omp_loop.iv, 1
  br label %omp_loop.header

omp.loop_nest.region6:                            ; preds = %omp.loop_nest.region5
  %32 = load i32, ptr %25, align 4
  %33 = call i32 @get_num_neighbours(ptr %loadgep_, i32 %32)
  %34 = sext i32 %33 to i64
  %35 = call ptr @get_neighbours(ptr %loadgep_, i32 %32)
  br label %omp.loop_nest.region7

omp.loop_nest.region7:                            ; preds = %omp.loop_nest.region10, %omp.loop_nest.region6
  %36 = phi i64 [ %67, %omp.loop_nest.region10 ], [ 0, %omp.loop_nest.region6 ]
  %37 = icmp slt i64 %36, %34
  br i1 %37, label %omp.loop_nest.region8, label %omp.loop_nest.region11

omp.loop_nest.region11:                           ; preds = %omp.loop_nest.region7
  br label %omp.loop_nest.region12

omp.loop_nest.region8:                            ; preds = %omp.loop_nest.region7
  %38 = getelementptr i32, ptr %35, i64 %36
  %39 = load i32, ptr %38, align 4
  %40 = alloca i32, align 4
  store i32 %39, ptr %40, align 4
  %41 = alloca ptr, i64 1, align 8
  %42 = load i32, ptr %25, align 4
  %43 = load i32, ptr %40, align 4
  %44 = call ptr @get_edge(ptr %loadgep_, i32 %42, i32 %43)
  store ptr %44, ptr %41, align 8
  %45 = load i32, ptr %40, align 4
  %46 = sext i32 %45 to i64
  %47 = extractvalue { ptr, ptr, i64, [1 x i64], [1 x i64] } %4, 1
  %48 = getelementptr inbounds nuw i64, ptr %47, i64 %46
  %49 = load i64, ptr %48, align 4
  %50 = load i32, ptr %25, align 4
  %51 = sext i32 %50 to i64
  %52 = extractvalue { ptr, ptr, i64, [1 x i64], [1 x i64] } %4, 1
  %53 = getelementptr inbounds nuw i64, ptr %52, i64 %51
  %54 = load i64, ptr %53, align 4
  %55 = load ptr, ptr %41, align 8
  %56 = call i64 @get_edge_weight(ptr %55)
  %57 = add i64 %54, %56
  %58 = load i32, ptr %40, align 4
  %59 = load i32, ptr %40, align 4
  %60 = sext i32 %58 to i64
  %61 = sext i32 %59 to i64
  %62 = call i64 @llvm.smin.i64(i64 %49, i64 %57)
  %63 = extractvalue { ptr, ptr, i64, [1 x i64], [1 x i64] } %4, 1
  %64 = getelementptr i64, ptr %63, i64 %60
  %65 = atomicrmw min ptr %64, i64 %62 acq_rel, align 8
  %66 = icmp ne i64 %65, %62
  br i1 %66, label %omp.loop_nest.region9, label %omp.loop_nest.region10

omp.loop_nest.region10:                           ; preds = %omp.loop_nest.region9, %omp.loop_nest.region8
  %67 = add i64 %36, 1
  br label %omp.loop_nest.region7

omp.loop_nest.region9:                            ; preds = %omp.loop_nest.region8
  %68 = extractvalue { ptr, ptr, i64, [1 x i64], [1 x i64] } %5, 1
  %69 = getelementptr i8, ptr %68, i64 %61
  %70 = atomicrmw xchg ptr %69, i8 1 acq_rel, align 1
  br label %omp.loop_nest.region10

omp.par.exit.exitStub:                            ; preds = %.fini
  ret void
}

; Function Attrs: nounwind
declare i32 @__kmpc_global_thread_num(ptr) #0

; Function Attrs: nocallback nofree nosync nounwind willreturn
declare ptr @llvm.stacksave.p0() #1

; Function Attrs: nocallback nofree nosync nounwind willreturn
declare void @llvm.stackrestore.p0(ptr) #1

; Function Attrs: nocallback nocreateundeforpoison nofree nosync nounwind speculatable willreturn memory(none)
declare i64 @llvm.smin.i64(i64, i64) #2

; Function Attrs: nounwind
declare void @__kmpc_for_static_init_8u(ptr, i32, i32, ptr, ptr, ptr, ptr, i64, i64) #0

; Function Attrs: nounwind
declare void @__kmpc_for_static_fini(ptr, i32) #0

; Function Attrs: convergent nounwind
declare void @__kmpc_barrier(ptr, i32) #3

; Function Attrs: nocallback nofree nounwind willreturn memory(argmem: readwrite)
declare void @llvm.memcpy.p0.p0.i64(ptr noalias writeonly captures(none), ptr noalias readonly captures(none), i64, i1 immarg) #4

define internal void @.omp.reduction.func(ptr %0, ptr %1) {
  %3 = getelementptr inbounds [1 x ptr], ptr %0, i64 0, i64 0
  %4 = load ptr, ptr %3, align 8
  %5 = load i8, ptr %4, align 1
  %6 = getelementptr inbounds [1 x ptr], ptr %1, i64 0, i64 0
  %7 = load ptr, ptr %6, align 8
  %8 = load i8, ptr %7, align 1
  %9 = or i8 %5, %8
  store i8 %9, ptr %4, align 1
  ret void
}

; Function Attrs: convergent nounwind
declare i32 @__kmpc_reduce(ptr, i32, i32, i64, ptr, ptr, ptr) #3

; Function Attrs: convergent nounwind
declare void @__kmpc_end_reduce(ptr, i32, ptr) #3

; Function Attrs: nounwind
declare !callback !3 void @__kmpc_fork_call(ptr, i32, ptr, ...) #0

attributes #0 = { nounwind }
attributes #1 = { nocallback nofree nosync nounwind willreturn }
attributes #2 = { nocallback nocreateundeforpoison nofree nosync nounwind speculatable willreturn memory(none) }
attributes #3 = { convergent nounwind }
attributes #4 = { nocallback nofree nounwind willreturn memory(argmem: readwrite) }

!llvm.module.flags = !{!0}

!0 = !{i32 2, !"Debug Info Version", i32 3}
!1 = !{i64 8}
!2 = !{i64 1}
!3 = !{!4}
!4 = !{i64 2, i64 -1, i64 -1, i1 true}
