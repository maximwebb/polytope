; ModuleID = 'output/test_opt.ll'
source_filename = "test.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@.str = private unnamed_addr constant [5 x i8] c"%d, \00", align 1
@.str.1 = private unnamed_addr constant [2 x i8] c"\0A\00", align 1

; Function Attrs: noinline nounwind uwtable
define dso_local i32 @main() #0 {
entry:
  %call = call noalias align 16 dereferenceable_or_null(64) i8* @malloc(i64 64) #5
  %0 = bitcast i8* %call to [2000 x i32]*
  br label %for.body

for.body:                                         ; preds = %for.inc6, %entry
  %i.02 = phi i32 [ 0, %entry ], [ %inc7, %for.inc6 ]
  br label %for.body3

for.body3:                                        ; preds = %for.body3, %for.body
  %j.01 = phi i32 [ 0, %for.body ], [ %inc, %for.body3 ]
  %mul = mul nsw i32 %j.01, 7
  %add = add nuw nsw i32 %i.02, %mul
  %rem = and i32 %add, 3
  %idxprom = zext i32 %i.02 to i64
  %idxprom4 = zext i32 %j.01 to i64
  %arrayidx5 = getelementptr inbounds [2000 x i32], [2000 x i32]* %0, i64 %idxprom, i64 %idxprom4
  store i32 %rem, i32* %arrayidx5, align 4
  %inc = add nuw nsw i32 %j.01, 1
  %cmp2 = icmp ult i32 %j.01, 3
  br i1 %cmp2, label %for.body3, label %for.inc6, !llvm.loop !4

for.inc6:                                         ; preds = %for.body3
  %inc7 = add nuw nsw i32 %i.02, 1
  %cmp = icmp ult i32 %i.02, 3
  br i1 %cmp, label %for.body, label %for.body12.preheader, !llvm.loop !6

for.body12.preheader:                             ; preds = %for.inc6
  br label %for.body12

for.body12:                                       ; preds = %for.inc29, %for.body12.preheader
  %p = phi i32 [ 1, %for.body12.preheader ], [ %p.inc, %for.inc29 ]
  %1 = sub i32 %p, 0
  %l1 = sub i32 %1, 1
  %2 = srem i32 %l1, 1
  %3 = call i32 @llvm.smin.i32(i32 %2, i32 1)
  %4 = sdiv i32 %l1, 1
  %5 = mul i32 0, %4
  %6 = add i32 %5, %3
  %7 = call i32 @llvm.smax.i32(i32 -2147483648, i32 %6)
  %l1.ceil = add i32 %7, 1
  %8 = sub i32 %p, 0
  %l3 = sub i32 %8, 3
  %9 = srem i32 %l3, 1
  %10 = call i32 @llvm.smin.i32(i32 %9, i32 1)
  %11 = sdiv i32 %l3, 1
  %12 = mul i32 0, %11
  %13 = add i32 %12, %10
  %14 = call i32 @llvm.smin.i32(i32 2147483647, i32 %13)
  %q.upper = add i32 %14, 3
  %15 = sdiv i32 %p, 1
  %16 = mul i32 0, %15
  %17 = sub i32 %16, %l1.ceil
  %offset = srem i32 %17, 1
  %q.lower = add i32 %l1.ceil, %offset
  br label %for.body16

for.body16:                                       ; preds = %for.body16, %for.body12
  %q = phi i32 [ %q.lower, %for.body12 ], [ %q.inc, %for.body16 ]
  %18 = mul i32 1, %q
  %19 = mul i32 0, %p
  %20 = sub i32 %19, %18
  %i.new = sdiv i32 %20, -1
  %21 = mul i32 1, %p
  %22 = mul i32 0, %q
  %23 = sub i32 %22, %21
  %j.new = sdiv i32 %23, -1
  %idxprom17 = zext i32 %i.new to i64
  %idxprom19 = zext i32 %j.new to i64
  %arrayidx20 = getelementptr inbounds [2000 x i32], [2000 x i32]* %0, i64 %idxprom17, i64 %idxprom19
  store i32 3, i32* %arrayidx20, align 4
  %idxprom21 = zext i32 %i.new to i64
  %sub = add nsw i32 %j.new, -1
  %idxprom23 = sext i32 %sub to i64
  %arrayidx24 = getelementptr inbounds [2000 x i32], [2000 x i32]* %0, i64 %idxprom21, i64 %idxprom23
  %24 = load i32, i32* %arrayidx24, align 4
  %add25 = add nsw i32 %24, 2
  store i32 %add25, i32* %arrayidx24, align 4
  %q.inc = add i32 %q, 1
  %25 = icmp sle i32 %q.inc, %q.upper
  br i1 %25, label %for.body16, label %for.inc29, !llvm.loop !7

for.inc29:                                        ; preds = %for.body16
  %p.inc = add i32 %p, 1
  %26 = icmp sle i32 %p.inc, 3
  br i1 %26, label %for.body12, label %for.body35.preheader

for.body35.preheader:                             ; preds = %for.inc29
  br label %for.body35

for.body35:                                       ; preds = %for.body35.preheader, %for.end47
  %i32.06 = phi i32 [ %inc50, %for.end47 ], [ 0, %for.body35.preheader ]
  br label %for.body39

for.body39:                                       ; preds = %for.body39, %for.body35
  %j36.05 = phi i32 [ 0, %for.body35 ], [ %inc46, %for.body39 ]
  %idxprom40 = zext i32 %i32.06 to i64
  %idxprom42 = zext i32 %j36.05 to i64
  %arrayidx43 = getelementptr inbounds [2000 x i32], [2000 x i32]* %0, i64 %idxprom40, i64 %idxprom42
  %27 = load i32, i32* %arrayidx43, align 4
  %call44 = call i32 (i8*, ...) @printf(i8* noundef nonnull dereferenceable(1) getelementptr inbounds ([5 x i8], [5 x i8]* @.str, i64 0, i64 0), i32 %27) #5
  %inc46 = add nuw nsw i32 %j36.05, 1
  %cmp38 = icmp ult i32 %j36.05, 3
  br i1 %cmp38, label %for.body39, label %for.end47, !llvm.loop !11

for.end47:                                        ; preds = %for.body39
  %putchar = call i32 @putchar(i32 10)
  %inc50 = add nuw nsw i32 %i32.06, 1
  %cmp34 = icmp ult i32 %i32.06, 3
  br i1 %cmp34, label %for.body35, label %for.end51, !llvm.loop !12

for.end51:                                        ; preds = %for.end47
  call void @free(i8* %call) #5
  ret i32 0
}

; Function Attrs: nounwind
declare dso_local noalias i8* @malloc(i64) #1

declare dso_local i32 @printf(i8*, ...) #2

; Function Attrs: nounwind
declare dso_local void @free(i8*) #1

; Function Attrs: nofree nounwind
declare noundef i32 @putchar(i32 noundef) #3

; Function Attrs: nofree nosync nounwind readnone speculatable willreturn
declare i32 @llvm.smin.i32(i32, i32) #4

; Function Attrs: nofree nosync nounwind readnone speculatable willreturn
declare i32 @llvm.smax.i32(i32, i32) #4

attributes #0 = { noinline nounwind uwtable "frame-pointer"="all" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { nounwind "frame-pointer"="all" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #2 = { "frame-pointer"="all" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #3 = { nofree nounwind }
attributes #4 = { nofree nosync nounwind readnone speculatable willreturn }
attributes #5 = { nounwind }

!llvm.module.flags = !{!0, !1, !2}
!llvm.ident = !{!3}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"uwtable", i32 1}
!2 = !{i32 7, !"frame-pointer", i32 2}
!3 = !{!"clang version 14.0.0 (https://github.com/llvm/llvm-project.git cb395f66ac3ce60427ca2b99580e716ac6dd551a)"}
!4 = distinct !{!4, !5}
!5 = !{!"llvm.loop.mustprogress"}
!6 = distinct !{!6, !5}
!7 = distinct !{!7, !8, !9, !10}
!8 = !{!"llvm.loop.parallel_accesses", i32 0}
!9 = !{!"llvm.mem.parallel_loop_access", i32 0}
!10 = !{!"llvm.loop.vectorize.enable", i32 0}
!11 = distinct !{!11, !5}
!12 = distinct !{!12, !5}
