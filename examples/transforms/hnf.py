from typing import List

from utils import clone_arr


def negate_col(M: List[List[int]], col: int):
    for row in M:
        row[col] = -row[col]


def swap_cols(M: List[List[int]], i: int, j: int):
    if i == j:
        return
    for row in M:
        tmp = row[i]
        row[i] = row[j]
        row[j] = tmp


def hnf(M: List[List[int]]) -> List[List[int]]:
    A = clone_arr(M)
    I = [[1,0],[0,1]]
    if len(A) != len(A[0]):
        raise Exception("Non-square matrix")
    N = len(A)
    i = 0
    while i < N:
        if all([x == 0 for x in A[i][i+1:]]):
            if A[i][i] < 0:
                negate_col(A, i)
                negate_col(I, i)
            i += 1
            continue
        # Choose smallest absolute element in row
        pivot = 100000
        pivot_index = 0
        for j, x in enumerate(A[i]):
            if abs(x) < pivot and x != 0:
                pivot_index = j
                pivot = x
        swap_cols(A, i, pivot_index)
        swap_cols(I, i, pivot_index)
        for j in range(i+1, N):
            q = A[i][j]//pivot
            for k in range(N):
                A[k][j] -= q * A[k][i]
                I[k][j] -= q * I[k][i]

    for i, row in enumerate(A):
        for j in range(i):
            # slow way to ensure all elements are positive, also doesn't work for n > 2
            while row[j] < 0:
                row[j] += A[i][i]
    return A
