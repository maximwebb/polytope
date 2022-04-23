; ModuleID = 'test.c'
source_filename = "test.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@j = dso_local global i32 0, align 4
@__const.arr.A = private unnamed_addr constant [3 x [4 x i32]] [[4 x i32] [i32 1, i32 3, i32 5, i32 8], [4 x i32] [i32 4, i32 2, i32 2, i32 3], [4 x i32] [i32 5, i32 3, i32 1, i32 7]], align 16
@.str = private unnamed_addr constant [5 x i8] c"%d, \00", align 1
@.str.1 = private unnamed_addr constant [2 x i8] c"\0A\00", align 1

; Function Attrs: noinline nounwind uwtable
define dso_local i32 @arr(i32 %N) #0 {
entry:
  %N.addr = alloca i32, align 4
  %A = alloca [3 x [4 x i32]], align 16
  %i = alloca i32, align 4
  %k = alloca i32, align 4
  %i19 = alloca i32, align 4
  %k23 = alloca i32, align 4
  store i32 %N, i32* %N.addr, align 4
  %0 = bitcast [3 x [4 x i32]]* %A to i8*
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* align 16 %0, i8* align 16 bitcast ([3 x [4 x i32]]* @__const.arr.A to i8*), i64 48, i1 false)
  store i32 1, i32* %i, align 4
  br label %for.cond

for.cond:                                         ; preds = %for.inc16, %entry
  %1 = load i32, i32* %i, align 4
  %2 = load i32, i32* %N.addr, align 4
  %cmp = icmp slt i32 %1, %2
  br i1 %cmp, label %for.body, label %for.end18

for.body:                                         ; preds = %for.cond
  store i32 1, i32* %k, align 4
  br label %for.cond1

for.cond1:                                        ; preds = %for.inc, %for.body
  %3 = load i32, i32* %k, align 4
  %4 = load i32, i32* %N.addr, align 4
  %add = add nsw i32 %4, 1
  %cmp2 = icmp slt i32 %3, %add
  br i1 %cmp2, label %for.body3, label %for.end

for.body3:                                        ; preds = %for.cond1
  %5 = load i32, i32* %i, align 4
  %sub = sub nsw i32 %5, 1
  %idxprom = sext i32 %sub to i64
  %arrayidx = getelementptr inbounds [3 x [4 x i32]], [3 x [4 x i32]]* %A, i64 0, i64 %idxprom
  %6 = load i32, i32* %k, align 4
  %idxprom4 = sext i32 %6 to i64
  %arrayidx5 = getelementptr inbounds [4 x i32], [4 x i32]* %arrayidx, i64 0, i64 %idxprom4
  %7 = load i32, i32* %arrayidx5, align 4
  %8 = load i32, i32* %i, align 4
  %idxprom6 = sext i32 %8 to i64
  %arrayidx7 = getelementptr inbounds [3 x [4 x i32]], [3 x [4 x i32]]* %A, i64 0, i64 %idxprom6
  %9 = load i32, i32* %k, align 4
  %sub8 = sub nsw i32 %9, 1
  %idxprom9 = sext i32 %sub8 to i64
  %arrayidx10 = getelementptr inbounds [4 x i32], [4 x i32]* %arrayidx7, i64 0, i64 %idxprom9
  %10 = load i32, i32* %arrayidx10, align 4
  %add11 = add nsw i32 %7, %10
  %11 = load i32, i32* %i, align 4
  %idxprom12 = sext i32 %11 to i64
  %arrayidx13 = getelementptr inbounds [3 x [4 x i32]], [3 x [4 x i32]]* %A, i64 0, i64 %idxprom12
  %12 = load i32, i32* %k, align 4
  %idxprom14 = sext i32 %12 to i64
  %arrayidx15 = getelementptr inbounds [4 x i32], [4 x i32]* %arrayidx13, i64 0, i64 %idxprom14
  store i32 %add11, i32* %arrayidx15, align 4
  br label %for.inc

for.inc:                                          ; preds = %for.body3
  %13 = load i32, i32* %k, align 4
  %inc = add nsw i32 %13, 1
  store i32 %inc, i32* %k, align 4
  br label %for.cond1, !llvm.loop !4

for.end:                                          ; preds = %for.cond1
  br label %for.inc16

for.inc16:                                        ; preds = %for.end
  %14 = load i32, i32* %i, align 4
  %inc17 = add nsw i32 %14, 1
  store i32 %inc17, i32* %i, align 4
  br label %for.cond, !llvm.loop !6

for.end18:                                        ; preds = %for.cond
  store i32 1, i32* %i19, align 4
  br label %for.cond20

for.cond20:                                       ; preds = %for.inc35, %for.end18
  %15 = load i32, i32* %i19, align 4
  %cmp21 = icmp slt i32 %15, 3
  br i1 %cmp21, label %for.body22, label %for.end37

for.body22:                                       ; preds = %for.cond20
  store i32 1, i32* %k23, align 4
  br label %for.cond24

for.cond24:                                       ; preds = %for.inc31, %for.body22
  %16 = load i32, i32* %k23, align 4
  %cmp25 = icmp slt i32 %16, 4
  br i1 %cmp25, label %for.body26, label %for.end33

for.body26:                                       ; preds = %for.cond24
  %17 = load i32, i32* %i19, align 4
  %idxprom27 = sext i32 %17 to i64
  %arrayidx28 = getelementptr inbounds [3 x [4 x i32]], [3 x [4 x i32]]* %A, i64 0, i64 %idxprom27
  %18 = load i32, i32* %k23, align 4
  %idxprom29 = sext i32 %18 to i64
  %arrayidx30 = getelementptr inbounds [4 x i32], [4 x i32]* %arrayidx28, i64 0, i64 %idxprom29
  %19 = load i32, i32* %arrayidx30, align 4
  %call = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([5 x i8], [5 x i8]* @.str, i64 0, i64 0), i32 %19)
  br label %for.inc31

for.inc31:                                        ; preds = %for.body26
  %20 = load i32, i32* %k23, align 4
  %inc32 = add nsw i32 %20, 1
  store i32 %inc32, i32* %k23, align 4
  br label %for.cond24, !llvm.loop !7

for.end33:                                        ; preds = %for.cond24
  %call34 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([2 x i8], [2 x i8]* @.str.1, i64 0, i64 0))
  br label %for.inc35

for.inc35:                                        ; preds = %for.end33
  %21 = load i32, i32* %i19, align 4
  %inc36 = add nsw i32 %21, 1
  store i32 %inc36, i32* %i19, align 4
  br label %for.cond20, !llvm.loop !8

for.end37:                                        ; preds = %for.cond20
  %arrayidx38 = getelementptr inbounds [3 x [4 x i32]], [3 x [4 x i32]]* %A, i64 0, i64 2
  %arrayidx39 = getelementptr inbounds [4 x i32], [4 x i32]* %arrayidx38, i64 0, i64 2
  %22 = load i32, i32* %arrayidx39, align 8
  ret i32 %22
}

; Function Attrs: argmemonly nofree nounwind willreturn
declare void @llvm.memcpy.p0i8.p0i8.i64(i8* noalias nocapture writeonly, i8* noalias nocapture readonly, i64, i1 immarg) #1

declare dso_local i32 @printf(i8*, ...) #2

; Function Attrs: noinline nounwind uwtable
define dso_local i32 @main() #0 {
entry:
  %retval = alloca i32, align 4
  %N = alloca i32, align 4
  store i32 0, i32* %retval, align 4
  store i32 3, i32* %N, align 4
  %0 = load i32, i32* %N, align 4
  %call = call i32 @arr(i32 %0)
  ret i32 %call
}

attributes #0 = { noinline nounwind uwtable "frame-pointer"="all" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { argmemonly nofree nounwind willreturn }
attributes #2 = { "frame-pointer"="all" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }

!llvm.module.flags = !{!0, !1, !2}
!llvm.ident = !{!3}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"uwtable", i32 1}
!2 = !{i32 7, !"frame-pointer", i32 2}
!3 = !{!"clang version 14.0.0 (https://github.com/llvm/llvm-project.git cb395f66ac3ce60427ca2b99580e716ac6dd551a)"}
!4 = distinct !{!4, !5}
!5 = !{!"llvm.loop.mustprogress"}
!6 = distinct !{!6, !5}
!7 = distinct !{!7, !5}
!8 = distinct !{!8, !5}
