; ModuleID = 'test.c'
source_filename = "test.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@.str = private unnamed_addr constant [5 x i8] c"%d, \00", align 1
@.str.1 = private unnamed_addr constant [2 x i8] c"\0A\00", align 1

; Function Attrs: noinline nounwind uwtable
define dso_local i32 @main() #0 {
entry:
  %retval = alloca i32, align 4
  %A = alloca [2000 x i32]*, align 8
  %i = alloca i32, align 4
  %j = alloca i32, align 4
  %i9 = alloca i32, align 4
  %j13 = alloca i32, align 4
  %i32 = alloca i32, align 4
  %j36 = alloca i32, align 4
  store i32 0, i32* %retval, align 4
  %call = call noalias align 16 i8* @malloc(i64 64) #3
  %0 = bitcast i8* %call to [2000 x i32]*
  store [2000 x i32]* %0, [2000 x i32]** %A, align 8
  store i32 0, i32* %i, align 4
  br label %for.cond

for.cond:                                         ; preds = %for.inc6, %entry
  %1 = load i32, i32* %i, align 4
  %cmp = icmp slt i32 %1, 4
  br i1 %cmp, label %for.body, label %for.end8

for.body:                                         ; preds = %for.cond
  store i32 0, i32* %j, align 4
  br label %for.cond1

for.cond1:                                        ; preds = %for.inc, %for.body
  %2 = load i32, i32* %j, align 4
  %cmp2 = icmp slt i32 %2, 4
  br i1 %cmp2, label %for.body3, label %for.end

for.body3:                                        ; preds = %for.cond1
  %3 = load i32, i32* %i, align 4
  %4 = load i32, i32* %j, align 4
  %mul = mul nsw i32 %4, 7
  %add = add nsw i32 %3, %mul
  %rem = srem i32 %add, 4
  %5 = load [2000 x i32]*, [2000 x i32]** %A, align 8
  %6 = load i32, i32* %i, align 4
  %idxprom = sext i32 %6 to i64
  %arrayidx = getelementptr inbounds [2000 x i32], [2000 x i32]* %5, i64 %idxprom
  %7 = load i32, i32* %j, align 4
  %idxprom4 = sext i32 %7 to i64
  %arrayidx5 = getelementptr inbounds [2000 x i32], [2000 x i32]* %arrayidx, i64 0, i64 %idxprom4
  store i32 %rem, i32* %arrayidx5, align 4
  br label %for.inc

for.inc:                                          ; preds = %for.body3
  %8 = load i32, i32* %j, align 4
  %inc = add nsw i32 %8, 1
  store i32 %inc, i32* %j, align 4
  br label %for.cond1, !llvm.loop !4

for.end:                                          ; preds = %for.cond1
  br label %for.inc6

for.inc6:                                         ; preds = %for.end
  %9 = load i32, i32* %i, align 4
  %inc7 = add nsw i32 %9, 1
  store i32 %inc7, i32* %i, align 4
  br label %for.cond, !llvm.loop !6

for.end8:                                         ; preds = %for.cond
  store i32 1, i32* %i9, align 4
  br label %for.cond10

for.cond10:                                       ; preds = %for.inc29, %for.end8
  %10 = load i32, i32* %i9, align 4
  %cmp11 = icmp slt i32 %10, 4
  br i1 %cmp11, label %for.body12, label %for.end31

for.body12:                                       ; preds = %for.cond10
  store i32 1, i32* %j13, align 4
  br label %for.cond14

for.cond14:                                       ; preds = %for.inc26, %for.body12
  %11 = load i32, i32* %j13, align 4
  %cmp15 = icmp slt i32 %11, 4
  br i1 %cmp15, label %for.body16, label %for.end28

for.body16:                                       ; preds = %for.cond14
  %12 = load [2000 x i32]*, [2000 x i32]** %A, align 8
  %13 = load i32, i32* %i9, align 4
  %idxprom17 = sext i32 %13 to i64
  %arrayidx18 = getelementptr inbounds [2000 x i32], [2000 x i32]* %12, i64 %idxprom17
  %14 = load i32, i32* %j13, align 4
  %idxprom19 = sext i32 %14 to i64
  %arrayidx20 = getelementptr inbounds [2000 x i32], [2000 x i32]* %arrayidx18, i64 0, i64 %idxprom19
  store i32 3, i32* %arrayidx20, align 4
  %15 = load [2000 x i32]*, [2000 x i32]** %A, align 8
  %16 = load i32, i32* %i9, align 4
  %idxprom21 = sext i32 %16 to i64
  %arrayidx22 = getelementptr inbounds [2000 x i32], [2000 x i32]* %15, i64 %idxprom21
  %17 = load i32, i32* %j13, align 4
  %sub = sub nsw i32 %17, 1
  %idxprom23 = sext i32 %sub to i64
  %arrayidx24 = getelementptr inbounds [2000 x i32], [2000 x i32]* %arrayidx22, i64 0, i64 %idxprom23
  %18 = load i32, i32* %arrayidx24, align 4
  %add25 = add nsw i32 %18, 2
  store i32 %add25, i32* %arrayidx24, align 4
  br label %for.inc26

for.inc26:                                        ; preds = %for.body16
  %19 = load i32, i32* %j13, align 4
  %inc27 = add nsw i32 %19, 1
  store i32 %inc27, i32* %j13, align 4
  br label %for.cond14, !llvm.loop !7

for.end28:                                        ; preds = %for.cond14
  br label %for.inc29

for.inc29:                                        ; preds = %for.end28
  %20 = load i32, i32* %i9, align 4
  %inc30 = add nsw i32 %20, 1
  store i32 %inc30, i32* %i9, align 4
  br label %for.cond10, !llvm.loop !8

for.end31:                                        ; preds = %for.cond10
  store i32 0, i32* %i32, align 4
  br label %for.cond33

for.cond33:                                       ; preds = %for.inc49, %for.end31
  %21 = load i32, i32* %i32, align 4
  %cmp34 = icmp slt i32 %21, 4
  br i1 %cmp34, label %for.body35, label %for.end51

for.body35:                                       ; preds = %for.cond33
  store i32 0, i32* %j36, align 4
  br label %for.cond37

for.cond37:                                       ; preds = %for.inc45, %for.body35
  %22 = load i32, i32* %j36, align 4
  %cmp38 = icmp slt i32 %22, 4
  br i1 %cmp38, label %for.body39, label %for.end47

for.body39:                                       ; preds = %for.cond37
  %23 = load [2000 x i32]*, [2000 x i32]** %A, align 8
  %24 = load i32, i32* %i32, align 4
  %idxprom40 = sext i32 %24 to i64
  %arrayidx41 = getelementptr inbounds [2000 x i32], [2000 x i32]* %23, i64 %idxprom40
  %25 = load i32, i32* %j36, align 4
  %idxprom42 = sext i32 %25 to i64
  %arrayidx43 = getelementptr inbounds [2000 x i32], [2000 x i32]* %arrayidx41, i64 0, i64 %idxprom42
  %26 = load i32, i32* %arrayidx43, align 4
  %call44 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([5 x i8], [5 x i8]* @.str, i64 0, i64 0), i32 %26)
  br label %for.inc45

for.inc45:                                        ; preds = %for.body39
  %27 = load i32, i32* %j36, align 4
  %inc46 = add nsw i32 %27, 1
  store i32 %inc46, i32* %j36, align 4
  br label %for.cond37, !llvm.loop !9

for.end47:                                        ; preds = %for.cond37
  %call48 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([2 x i8], [2 x i8]* @.str.1, i64 0, i64 0))
  br label %for.inc49

for.inc49:                                        ; preds = %for.end47
  %28 = load i32, i32* %i32, align 4
  %inc50 = add nsw i32 %28, 1
  store i32 %inc50, i32* %i32, align 4
  br label %for.cond33, !llvm.loop !10

for.end51:                                        ; preds = %for.cond33
  %29 = load [2000 x i32]*, [2000 x i32]** %A, align 8
  %30 = bitcast [2000 x i32]* %29 to i8*
  call void @free(i8* %30) #3
  ret i32 0
}

; Function Attrs: nounwind
declare dso_local noalias i8* @malloc(i64) #1

declare dso_local i32 @printf(i8*, ...) #2

; Function Attrs: nounwind
declare dso_local void @free(i8*) #1

attributes #0 = { noinline nounwind uwtable "frame-pointer"="all" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { nounwind "frame-pointer"="all" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #2 = { "frame-pointer"="all" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #3 = { nounwind }

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
!9 = distinct !{!9, !5}
!10 = distinct !{!10, !5}
