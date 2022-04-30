import glob
import os
from typing import List

from benchmark import Benchmark
from compilation_strategy import ClangStrategy, OptClangStrategy, PolytopeStrategy, ClangO3Strategy
from example_generator import TestGenerator, RandomLinGenerator
from execution_strategy import CorrectnessTest, TimeTest


def main():
    clear()
    benchmark = Benchmark(
        TestGenerator(1000),
        # RandomLinGenerator(30),
        [OptClangStrategy(), PolytopeStrategy()],
        # [ClangStrategy(), OptClangStrategy(), PolytopeStrategy(), ClangO3Strategy()],
        [CorrectnessTest(), TimeTest(iterations=1000, names=["Clang+Opt", "Polytope"])],
        # [CorrectnessTest(), TimeTest(iterations=50, names=["Clang", "Clang+Opt", "Polytope", "Clang -O3"])],
        1
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
