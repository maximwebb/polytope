from typing import *

from transforms.hnf import hnf
from utils import *


# List must be square
def func(arr: List[List[float]]) -> List[List[float]]:
    a = clone_arr(arr)
    N = len(arr)
    for i in range(1, N):
        for j in range(1, min(i + 2, N)):
            a[i][j] = a[i - 1][j] + a[i][j - 1]
    return a


def fast_func(arr: List[List[float]]) -> List[List[float]]:
    a = clone_arr(arr)
    N = len(arr)
    for i in range(2, 2 * N + 1):
        for j in range(max(1, i + 1 - N), min((i + 1) // 2 + 1, N)):
            a[i - j][j] = a[i - j - 1][j] + a[i - j][j - 1]
    return a


def func1(arr: List[List[float]]) -> List[List[float]]:
    a = clone_arr(arr)
    N = len(arr)
    for i in range(1, N):
        for j in range(1, N):
            a[i][j] = a[i - 1][j] + a[i][j - 1]
    return a


def fast_func1(arr: List[List[float]]) -> List[List[float]]:
    a = clone_arr(arr)
    N = len(arr)
    for i in range(2, 2 * N + 1):
        for j in range(max(1, i - N + 1), min(N, i)):
            i_new = i - j
            a[i - j][j] = a[i - j - 1][j] + a[i - j][j - 1]
    return a


def func2(arr: List[List[float]]) -> List[List[float]]:
    a = clone_arr(arr)
    N = len(arr)
    for i in range(N):
        for j in range(2, N):
            a[i][j] = a[i][j - 1] + a[i][j - 2]
    return a


def fast_func2(arr: List[List[float]]) -> List[List[float]]:
    a = clone_arr(arr)
    N = len(arr)
    for i in range(2, N):
        for j in range(N):
            a[j][i] = a[j][i - 1] + a[j][i - 2]
    return a


a1 = 4
a2 = 6
b1 = 15
b2 = 19
# a1 = 12
# a2 = 4
# b1 = 31
# b2 = 39


def func3(arr: List[List[float]]) -> List[List[float]]:
    a = clone_arr(arr)
    N = len(arr)
    for i in range(1, b1):
        for j in range(1, b2):
            a[i][j] = a[i - 1][j] + a[i][j - 1]
    return a


def fast_func3(arr: List[List[float]]) -> List[List[float]]:
    a = clone_arr(arr)
    N = len(arr)
    for i in range(2, 2 * b2 + 1):
        for j in range(max(1, i - b1 + 1), min(b2, i)):
            i_new = i - j
            j_new = j
            a[i_new][j_new] = a[i_new - 1][j_new] + a[i_new][j_new - 1]
    return a


def func4(arr: List[List[float]]) -> List[List[float]]:
    a = clone_arr(arr)
    N = len(arr)
    s = set()
    for i in range(a1, b1):
        for j in range(a2, b2):
            s.add((i, j))
            a[i][j] = a[i - 1][j] + a[i][j - 1]
    return a


# Wikipedia example
def fast_func4(arr: List[List[float]]) -> List[List[float]]:
    a = clone_arr(arr)
    N = len(arr)
    s = set()
    for i in range(a1 + a2, b1 + b2 - 1):
        for j in range(max(a2, i - b1 + 1), min(b2, i - a1 + 1)):
            i_new = i - j
            j_new = j
            s.add((i_new, j_new))
            a[i_new][j_new] = a[i_new - 1][j_new] + a[i_new][j_new - 1]
    return a


def func5(arr: List[List[float]]) -> List[List[float]]:
    a = clone_arr(arr)
    N = len(arr)
    s = set()
    for i in range(a1, b1):
        for j in range(a2, b2):
            s.add((i, j))
            a[i][j] = a[i - 1][j] + a[i][j - 1] + a[i + 1][j - 1]
    return s


def fast_func5(arr: List[List[float]]) -> List[List[float]]:
    a = clone_arr(arr)
    s = set()
    for i in range(a1, b1):
        for j in range(2 * a2 + i, 2 * b2 + i - 1, 2):
            i_new = i
            j_new = (j - i) // 2
            s.add((i_new, j_new))
            a[i_new][j_new] = a[i_new - 1][j_new] + a[i_new][j_new - 1] + a[i_new + 1][j_new - 1]
    return s


def func6(arr: List[List[float]]) -> List[List[float]]:
    a = clone_arr(arr)
    s = set()
    for i in range(a1, b1):
        for j in range(a2, b2):
            s.add((i, j))
            a[i][j] = a[i - 1][j] + a[i][j - 1] + a[i + 1][j - 1]
    return s


def fast_func6(arr: List[List[float]]) -> List[List[float]]:
    a = clone_arr(arr)
    s = set()
    # T = (1 r)
    #     (0 t)
    r = 2
    t = 1
    for i in range(a1 + r * a2, b1 + r * (b2 - 1)):
        for j in range(max(t*a2, - t*((b1-1 - i) // r)),
                       min(t * (b2 - 1), t * (b2 - 1) - t * ((a1 + r * (b2 - 1) - i + 1) // r)) + 1, t):
                       # min(t*(b2-1) + 1, - t*((a1-1 - i) // r)), t):
            i_new = (t * i - r * j) // t
            j_new = j // t
            s.add((i_new, j_new))
            a[i_new][j_new] = a[i_new - 1][j_new] + a[i_new][j_new - 1] + a[i_new + 1][j_new - 1]
    return s


def fast_func7(arr: List[List[float]]) -> List[List[float]]:
    a = clone_arr(arr)
    s = set()
    # T = (p r)
    #     (q t)
    p = 3
    q = 3
    r = 2
    t = 4
    det = p * t - r * q
    H = hnf([[p, r], [q, t]])
    for i in range(p * a1 + r * a2, p * (b1 - 1) + r * (b2 - 1) + 1, H[0][0]):
        l1 = i - p*(b1-1) - r*a2
        l1_ceil = max(q*(l1//p) + (l1 % p > 0), t*(l1//r) + (l1 % r > 0)) + q*(b1-1) + t*a2
        l3 = i - p*a1 - r*(b2-1)
        offset = (H[1][0] * (i//H[0][0]) - l1_ceil) % H[1][1]
        for j in range(l1_ceil + offset,
                       min(q*(l3//p) + (l3 % p > 0), t*(l3//r) + (l3 % r > 0)) + q*a1 + t*(b2-1) + 1, H[1][1]):
            i_new = (t * i - r * j) // det
            j_new = (p * j - q * i) // det
            s.add((i_new, j_new))
            a[i_new][j_new] = a[i_new - 1][j_new] + a[i_new][j_new - 1] + a[i_new + 1][j_new - 1]
    return s
