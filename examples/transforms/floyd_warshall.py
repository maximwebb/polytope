from typing import *
from utils import clone_arr


def floyd_warshall(arr: List[List[float]]) -> List[List[float]]:
    N = len(arr)

    S = clone_arr(arr)

    for k in range(N):
        for i in range(N):
            for j in range(N):
                S[i][j] = min(S[i][j], S[i][k] + S[k][j])
    return S


def fast_floyd_warshall(arr: List[List[float]]) -> List[List[float]]:
    N = len(arr)

    S = clone_arr(arr)

    for k in range(N):
        for i in range(N):
            for j in range(N):
                pass
    return S