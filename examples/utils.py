import random
import copy
from typing import List


def fill_arr(m: int, n=0) -> List[List[int]]:
    if n == 0:
        n = m
    arr = []
    for i in range(m):
        arr.append([])
        for j in range(n):
            arr[i].append([])
            arr[i][j] = random.randint(0, 10)
    return arr


def clone_arr(arr: List) -> List:
    return copy.deepcopy(arr)


def print_arr(arr) -> None:
    [print(row) for row in arr]
    print("")
