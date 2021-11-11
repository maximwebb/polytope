from transforms import basic
from utils import *


def main():
    a = fill_arr(4)
    print_arr(a)
    b = clone_arr(a)
    a = basic.func2(a)
    b = basic.fast_func2(b)
    print_arr([a, b])
    print_arr(b)


if __name__ == '__main__':
    main()
