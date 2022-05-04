import glob
import os
from typing import List

from benchmark import Benchmark
from compilation_strategy import ClangStrategy, OptClangStrategy, PolytopeStrategy, ClangO3Strategy
from example_generator import TestGenerator, RandomLinGenerator, SelectedExampleGenerator, RepeatedExampleGenerator
from execution_strategy import CorrectnessTest, TimeTest, BarChart, LineGraph


def main():
    clear()
    test_names = sorted(["LCS", "Dither", "ArrReduce", "TransClos", "GaussJordan", "MatMul"])
    benchmark = Benchmark(
        # TestGenerator(1000),
        # RandomLinGenerator(1000),
        RepeatedExampleGenerator("./examples/arr_red.c", 1000),
        [OptClangStrategy(), PolytopeStrategy()],
        # [ClangStrategy(), OptClangStrategy(), PolytopeStrategy(), ClangO3Strategy()],
        # [BarChart(iterations=30, compile_names=["Clang+Opt", "Tope", "Clang+Polly"], test_names=test_names,
        #           normalise=True)],
        [LineGraph(iterations=10, compile_names=["Clang+Opt", "Tope"])],
        # [CorrectnessTest(),
        # TimeTest(iterations=50, names=["Clang", "Clang+Opt", "Polytope", "Clang -O3"])],
        50,
        True
    )
    benchmark.run()


def create(examples: List[str], name="example"):
    for (i, program) in enumerate(examples):
        f = open(f"./dump/{name}_{i}.c", 'w')
        f.write(program)
        f.close()


def clear():
    dump_files = glob.glob("./dump/*")
    bin_files = glob.glob("./bin/*")
    for f in dump_files + bin_files:
        os.remove(f)


if __name__ == "__main__":
    main()
