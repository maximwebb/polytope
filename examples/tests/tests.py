import math
import random
import unittest
import utils
import transforms.basic as basic
import transforms.lcs as lcs
from transforms import floyd_warshall, dither


class Basic(unittest.TestCase):
    def test_example(self):
        self.assertEqual(True, True)

    def test_basic_is_correct(self):
        data1 = utils.fill_arr(100)
        data2 = utils.clone_arr(data1)

        res1 = basic.func(data1)
        res2 = basic.fast_func(data2)

        self.assertEqual(res1, res2)

    def test_basic1_is_correct(self):
        data1 = utils.fill_arr(100)
        data2 = utils.clone_arr(data1)

        res1 = basic.func1(data1)
        res2 = basic.fast_func1(data2)

        self.assertEqual(res1, res2)

    def test_basic_2_is_correct(self):
        data1 = utils.fill_arr(100)
        data2 = utils.clone_arr(data1)

        res1 = basic.func2(data1)
        res2 = basic.fast_func2(data2)

        self.assertEqual(res1, res2)

    def test_basic_3_is_correct(self):
        data1 = utils.fill_arr(100)
        data2 = utils.clone_arr(data1)

        res1 = basic.func3(data1)
        res2 = basic.fast_func3(data2)

        self.assertEqual(res1, res2)

    def test_basic_4_is_correct(self):
        data1 = utils.fill_arr(100)
        data2 = utils.clone_arr(data1)

        res1 = basic.func4(data1)
        res2 = basic.fast_func4(data2)

        self.assertEqual(res1, res2)

    def test_basic_5_is_correct(self):
        data1 = utils.fill_arr(100)
        data2 = utils.clone_arr(data1)

        res1 = basic.func5(data1)
        res2 = basic.fast_func5(data2)

        self.assertEqual(res1, res2)

    def test_basic_6_is_correct(self):
        data1 = utils.fill_arr(100)
        data2 = utils.clone_arr(data1)

        res1 = basic.func6(data1)
        res2 = basic.fast_func6(data2)

        self.assertEqual(res1, res2)


    def test_basic_7_is_correct(self):
        data1 = utils.fill_arr(100)
        data2 = utils.clone_arr(data1)

        res1 = basic.func6(data1)
        res2 = basic.fast_func7(data2)

        self.assertEqual(res1, res2)

    def test_bounds(self):
        data1 = utils.fill_arr(100)
        data2 = utils.clone_arr(data1)

        res1 = basic.fast_func6(data1)
        res2 = basic.fast_func7(data2)

        self.assertEqual(res1, res2)


class LCS(unittest.TestCase):
    def test_lcs(self):
        u = "XMJYAUZ"
        v = "MZJAWXU"
        res1 = lcs.lcs(u, v)

        self.assertEqual(res1, 4)

    def test_lcs_transform(self):
        u = "XMJYAUZBBAABABADDD"
        v = "MZJAWXUABBAABAB"
        res1 = lcs.lcs(u, v)
        res2 = lcs.fast_lcs(u, v)

        self.assertEqual(res1, res2)


class FloydWarshall(unittest.TestCase):
    def test_floyd_warshall(self):
        a = [[0, math.inf, -2, math.inf],
             [4, 0, 3, math.inf],
             [math.inf, math.inf, 0, 2],
             [math.inf, -1, math.inf, 0]]

        res = floyd_warshall.floyd_warshall(a)

        ans = [[0, -1, -2, 0],
               [4, 0, 2, 4],
               [5, 1, 0, 2],
               [3, -1, 1, 0]]

        self.assertEqual(ans, res)


class Dither(unittest.TestCase):
    def test_dither_transform(self):
        src = [[random.randint(0, 255) for _ in range(20)] for _ in range(10)]

        res1 = dither.dither(src)
        res2 = dither.fast_dither(src)

        self.assertEqual(res1, res2)


if __name__ == '__main__':
    unittest.main()
