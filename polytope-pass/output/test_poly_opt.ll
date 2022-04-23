; ModuleID = 'test_opt.ll'
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
  %A = alloca [3 x [4 x i32]], align 16
  %0 = bitcast [3 x [4 x i32]]* %A to i8*
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* noundef nonnull align 16 dereferenceable(48) %0, i8* noundef nonnull align 16 dereferenceable(48) bitcast ([3 x [4 x i32]]* @__const.arr.A to i8*), i64 48, i1 false)
  %cmp4 = icmp sgt i32 %N, 1
  br i1 %cmp4, label %for.body, label %for.end18

for.body:                                         ; preds = %for.inc16, %entry
  %i.05 = phi i32 [ %inc17, %for.inc16 ], [ 1, %entry ]
  %cmp22 = icmp sgt i32 %N, 0
  br i1 %cmp22, label %for.body3, label %for.inc16

for.body3:                                        ; preds = %for.body3, %for.body
  %k.03 = phi i32 [ %inc, %for.body3 ], [ 1, %for.body ]
  %sub = add nsw i32 %i.05, -1
  %idxprom = sext i32 %sub to i64
  %idxprom4 = zext i32 %k.03 to i64
  %arrayidx5 = getelementptr inbounds [3 x [4 x i32]], [3 x [4 x i32]]* %A, i64 0, i64 %idxprom, i64 %idxprom4
  %1 = load i32, i32* %arrayidx5, align 4
  %idxprom6 = zext i32 %i.05 to i64
  %sub8 = add nsw i32 %k.03, -1
  %idxprom9 = sext i32 %sub8 to i64
  %arrayidx10 = getelementptr inbounds [3 x [4 x i32]], [3 x [4 x i32]]* %A, i64 0, i64 %idxprom6, i64 %idxprom9
  %2 = load i32, i32* %arrayidx10, align 4
  %add11 = add nsw i32 %1, %2
  %idxprom12 = zext i32 %i.05 to i64
  %idxprom14 = zext i32 %k.03 to i64
  %arrayidx15 = getelementptr inbounds [3 x [4 x i32]], [3 x [4 x i32]]* %A, i64 0, i64 %idxprom12, i64 %idxprom14
  store i32 %add11, i32* %arrayidx15, align 4
  %inc = add nuw nsw i32 %k.03, 1
  %cmp2 = icmp slt i32 %k.03, %N
  br i1 %cmp2, label %for.body3, label %for.inc16, !llvm.loop !4

for.inc16:                                        ; preds = %for.body3, %for.body
  %inc17 = add nuw nsw i32 %i.05, 1
  %cmp = icmp slt i32 %inc17, %N
  br i1 %cmp, label %for.body, label %for.end18, !llvm.loop !6

for.end18:                                        ; preds = %for.inc16, %entry
  br label %for.body22

for.body22:                                       ; preds = %for.end33, %for.end18
  %i19.07 = phi i32 [ 1, %for.end18 ], [ %inc36, %for.end33 ]
  br label %for.body26

for.body26:                                       ; preds = %for.body26, %for.body22
  %k23.06 = phi i32 [ 1, %for.body22 ], [ %inc32, %for.body26 ]
  %idxprom27 = zext i32 %i19.07 to i64
  %idxprom29 = zext i32 %k23.06 to i64
  %arrayidx30 = getelementptr inbounds [3 x [4 x i32]], [3 x [4 x i32]]* %A, i64 0, i64 %idxprom27, i64 %idxprom29
  %3 = load i32, i32* %arrayidx30, align 4
  %call = call i32 (i8*, ...) @printf(i8* noundef nonnull dereferenceable(1) getelementptr inbounds ([5 x i8], [5 x i8]* @.str, i64 0, i64 0), i32 %3) #4
  %inc32 = add nuw nsw i32 %k23.06, 1
  %cmp25 = icmp ult i32 %k23.06, 3
  br i1 %cmp25, label %for.body26, label %for.end33, !llvm.loop !7

for.end33:                                        ; preds = %for.body26
  %putchar = call i32 @putchar(i32 10)
  %inc36 = add nuw nsw i32 %i19.07, 1
  %cmp21 = icmp ult i32 %i19.07, 2
  br i1 %cmp21, label %for.body22, label %for.end37, !llvm.loop !8

for.end37:                                        ; preds = %for.end33
  %arrayidx39 = getelementptr inbounds [3 x [4 x i32]], [3 x [4 x i32]]* %A, i64 0, i64 2, i64 2
  %4 = load i32, i32* %arrayidx39, align 8
  ret i32 %4
}

; Function Attrs: argmemonly nofree nounwind willreturn
declare void @llvm.memcpy.p0i8.p0i8.i64(i8* noalias nocapture writeonly, i8* noalias nocapture readonly, i64, i1 immarg) #1

declare dso_local i32 @printf(i8*, ...) #2

; Function Attrs: noinline nounwind uwtable
define dso_local i32 @main() #0 {
entry:
  %call = call i32 @arr(i32 3)
  ret i32 %call
}

; Function Attrs: nofree nounwind
declare noundef i32 @putchar(i32 noundef) #3

attributes #0 = { noinline nounwind uwtable "frame-pointer"="all" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { argmemonly nofree nounwind willreturn }
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
