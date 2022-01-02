; ModuleID = 'output/test_opt.ll'
source_filename = "test.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@j = dso_local global i32 0, align 4

; Function Attrs: noinline nounwind uwtable
define dso_local i32 @bar(i32 %N) #0 {
entry:
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.inc5, %entry
  %i.03 = phi i32 [ 0, %entry ], [ %inc6, %for.inc5 ]
  br label %for.body3

for.body3:                                        ; preds = %for.inc, %for.cond1.preheader
  %k.02 = phi i32 [ 3, %for.cond1.preheader ], [ %inc, %for.inc ]
  %0 = load i32, i32* @j, align 4
  %add = add nsw i32 %0, %i.03
  store i32 %add, i32* @j, align 4
  %cmp4 = icmp sgt i32 %N, 2
  br i1 %cmp4, label %if.then, label %for.inc

if.then:                                          ; preds = %for.body3
  %1 = load i32, i32* @j, align 4
  %sub = sub nsw i32 %1, %k.02
  store i32 %sub, i32* @j, align 4
  br label %for.inc

for.inc:                                          ; preds = %if.then, %for.body3
  %inc = add nuw nsw i32 %k.02, 1
  %exitcond = icmp ne i32 %inc, 9
  br i1 %exitcond, label %for.body3, label %for.inc5, !llvm.loop !4

for.inc5:                                         ; preds = %for.inc
  %inc6 = add nuw nsw i32 %i.03, 1
  %exitcond1 = icmp ne i32 %inc6, 6
  br i1 %exitcond1, label %for.cond1.preheader, label %for.end7, !llvm.loop !6

for.end7:                                         ; preds = %for.inc5
  %2 = load i32, i32* @j, align 4
  ret i32 %2
}

; Function Attrs: noinline nounwind uwtable
define dso_local i32 @main() #0 {
entry:
  %call = call i32 @bar(i32 10)
  %call1 = call i32 @bar(i32 10)
  ret i32 %call1
}

attributes #0 = { noinline nounwind uwtable "frame-pointer"="all" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }

!llvm.module.flags = !{!0, !1, !2}
!llvm.ident = !{!3}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"uwtable", i32 1}
!2 = !{i32 7, !"frame-pointer", i32 2}
!3 = !{!"clang version 14.0.0 (https://github.com/llvm/llvm-project.git 79932211f9126b56bf6cf3ae82c7581d55d91ae2)"}
!4 = distinct !{!4, !5}
!5 = !{!"llvm.loop.mustprogress"}
!6 = distinct !{!6, !5}
