from typing import *
from utils import clone_arr


def dither(s) -> List[List[int]]:
    src = clone_arr(s)
    dst = dst = [[0 for _ in range(20)] for _ in range(10)]
    w = len(src)
    h = len(src[0])

    for j in range(h):
        for i in range(w):
            v = src[i][j]
            if i > 0:
                v -= (dst[i-1][j] - src[i-1][j])/2
            if j > 0:
                v -= (dst[i][j-1] - src[i][j-1])/4
                if i < w-1:
                    v -= (dst[i+1][j-1] - src[i+1][j-1])/4
            dst[i][j] = 0 if v < 128 else 255
            src[i][j] = 0 if v < 0 else v if v < 255 else 255
    return dst


# (i, j) maps to (i, 2j + i)
def fast_dither(s) -> List[List[int]]:
    src = clone_arr(s)
    dst = dst = [[0 for _ in range(20)] for _ in range(10)]
    w = len(src)
    h = len(src[0])

    for y in range(2 * h + w):
        for x in range(max(y % 2, y-(2*h)+2), min(y, w-1)+1, 2):
            i = x
            j = (y-x)//2
            v = src[i][j]
            if i > 0:
                v -= (dst[i-1][j] - src[i-1][j]) / 2
            if j > 0:
                v -= (dst[i][j-1] - src[i][j-1]) / 4
                if i < w - 1:
                    v -= (dst[i+1][j-1] - src[i+1][j-1]) / 4
            dst[i][j] = 0 if v < 128 else 255
            src[i][j] = 0 if v < 0 else v if v < 255 else 255
    return dst
