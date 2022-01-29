from transforms import basic
from transforms.hnf import hnf
from utils import *


def main():
    A = [[4, 7],
         [2, 6]]
    hnf(A)


    a = fill_arr(4)
    print_arr(a)
    b = clone_arr(a)
    a = basic.func2(a)
    b = basic.fast_func2(b)
    print_arr([a, b])
    print_arr(b)


if __name__ == '__main__':
    main()
