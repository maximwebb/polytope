import unittest
import utils
import transforms.basic as basic
import transforms.lcs as lcs


class Basic(unittest.TestCase):
    def test_example(self):
        self.assertEqual(True, True)

    def test_basic_is_correct(self):
        data1 = utils.fill_arr(100)
        data2 = utils.clone_arr(data1)

        res1 = basic.func(data1)
        res2 = basic.fast_func(data2)

        self.assertEqual(res1, res2)

    def test_basic_2_is_correct(self):
        data1 = utils.fill_arr(100)
        data2 = utils.clone_arr(data1)

        res1 = basic.func2(data1)
        res2 = basic.fast_func2(data2)

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


if __name__ == '__main__':
    unittest.main()
