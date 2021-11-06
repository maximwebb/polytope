from typing import *
from utils import *


# List must be square
def func(arr: List[List[float]]) -> List[List[float]]:
    a = clone_arr(arr)
    N = len(arr)
    for i in range(1, N):
        for j in range(1, min(i+2, N)):
            a[i][j] = a[i-1][j] + a[i][j-1]
    return a


def fast_func(arr: List[List[float]]) -> List[List[float]]:
    a = clone_arr(arr)
    N = len(arr)
    for i in range(2, 2 * N + 1):
        for j in range(max(1, i + 1 - N), min((i+1)//2 + 1, N)):
            a[i-j][j] = a[i-j-1][j] + a[i-j][j-1]
    return a


def func2(arr: List[List[float]]) -> List[List[float]]:
    a = clone_arr(arr)
    N = len(arr)
    for i in range(N):
        for j in range(2, N):
            a[i][j] = a[i][j-1] + a[i][j-2]
    return a


def fast_func2(arr: List[List[float]]) -> List[List[float]]:
    a = clone_arr(arr)
    N = len(arr)
    for i in range(2, N):
        for j in range(N):
            a[j][i] = a[j][i-1] + a[j][i-2]
    return a