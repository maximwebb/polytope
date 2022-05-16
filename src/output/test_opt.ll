; ModuleID = 'test.ll'
source_filename = "test.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@.str = private unnamed_addr constant [5 x i8] c"%d, \00", align 1
@.str.1 = private unnamed_addr constant [2 x i8] c"\0A\00", align 1

; Function Attrs: noinline nounwind uwtable
define dso_local i32 @main() #0 {
entry:
  %call = call noalias align 16 dereferenceable_or_null(64) i8* @malloc(i64 64) #4
  %0 = bitcast i8* %call to [2000 x i32]*
  br label %for.body

for.body:                                         ; preds = %entry, %for.inc6
  %i.02 = phi i32 [ 0, %entry ], [ %inc7, %for.inc6 ]
  br label %for.body3

for.body3:                                        ; preds = %for.body, %for.body3
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
  br i1 %cmp, label %for.body, label %for.body12, !llvm.loop !6

for.body12:                                       ; preds = %for.inc6, %for.inc29
  %i9.04 = phi i32 [ %inc30, %for.inc29 ], [ 1, %for.inc6 ]
  br label %for.body16

for.body16:                                       ; preds = %for.body12, %for.body16
  %j13.03 = phi i32 [ 1, %for.body12 ], [ %inc27, %for.body16 ]
  %idxprom17 = zext i32 %i9.04 to i64
  %idxprom19 = zext i32 %j13.03 to i64
  %arrayidx20 = getelementptr inbounds [2000 x i32], [2000 x i32]* %0, i64 %idxprom17, i64 %idxprom19
  store i32 3, i32* %arrayidx20, align 4
  %idxprom21 = zext i32 %i9.04 to i64
  %sub = add nsw i32 %j13.03, -1
  %idxprom23 = sext i32 %sub to i64
  %arrayidx24 = getelementptr inbounds [2000 x i32], [2000 x i32]* %0, i64 %idxprom21, i64 %idxprom23
  %1 = load i32, i32* %arrayidx24, align 4
  %add25 = add nsw i32 %1, 2
  store i32 %add25, i32* %arrayidx24, align 4
  %inc27 = add nuw nsw i32 %j13.03, 1
  %cmp15 = icmp ult i32 %j13.03, 3
  br i1 %cmp15, label %for.body16, label %for.inc29, !llvm.loop !7

for.inc29:                                        ; preds = %for.body16
  %inc30 = add nuw nsw i32 %i9.04, 1
  %cmp11 = icmp ult i32 %i9.04, 3
  br i1 %cmp11, label %for.body12, label %for.body35, !llvm.loop !8

for.body35:                                       ; preds = %for.inc29, %for.end47
  %i32.06 = phi i32 [ %inc50, %for.end47 ], [ 0, %for.inc29 ]
  br label %for.body39

for.body39:                                       ; preds = %for.body35, %for.body39
  %j36.05 = phi i32 [ 0, %for.body35 ], [ %inc46, %for.body39 ]
  %idxprom40 = zext i32 %i32.06 to i64
  %idxprom42 = zext i32 %j36.05 to i64
  %arrayidx43 = getelementptr inbounds [2000 x i32], [2000 x i32]* %0, i64 %idxprom40, i64 %idxprom42
  %2 = load i32, i32* %arrayidx43, align 4
  %call44 = call i32 (i8*, ...) @printf(i8* noundef nonnull dereferenceable(1) getelementptr inbounds ([5 x i8], [5 x i8]* @.str, i64 0, i64 0), i32 %2) #4
  %inc46 = add nuw nsw i32 %j36.05, 1
  %cmp38 = icmp ult i32 %j36.05, 3
  br i1 %cmp38, label %for.body39, label %for.end47, !llvm.loop !9

for.end47:                                        ; preds = %for.body39
  %putchar = call i32 @putchar(i32 10)
  %inc50 = add nuw nsw i32 %i32.06, 1
  %cmp34 = icmp ult i32 %i32.06, 3
  br i1 %cmp34, label %for.body35, label %for.end51, !llvm.loop !10

for.end51:                                        ; preds = %for.end47
  call void @free(i8* %call) #4
  ret i32 0
}

; Function Attrs: nounwind
declare dso_local noalias i8* @malloc(i64) #1

declare dso_local i32 @printf(i8*, ...) #2

; Function Attrs: nounwind
declare dso_local void @free(i8*) #1

; Function Attrs: nofree nounwind
declare noundef i32 @putchar(i32 noundef) #3

attributes #0 = { noinline nounwind uwtable "frame-pointer"="all" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { nounwind "frame-pointer"="all" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #2 = { "frame-pointer"="all" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #3 = { nofree nounwind }
attributes #4 = { nounwind }

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
