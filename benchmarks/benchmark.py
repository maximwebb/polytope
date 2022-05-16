import glob
import os
from typing import List

from compilation_strategy import ICompilationStrategy, OptClangStrategy, ClangStrategy
from example_generator import IExampleGenerator, TestExampleGenerator
from execution_strategy import IExecutionStrategy, LineGraph


class Benchmark:
    def __init__(self,
                 generator: IExampleGenerator,
                 comp_strategies: List[ICompilationStrategy],
                 exec_strategies: List[IExecutionStrategy],
                 example_count=100,
                 skip_tests=False):
        self._generator = generator
        self._comp_strategies = comp_strategies
        self._exec_strategies = exec_strategies
        self._example_count = example_count
        self._skip_tests = skip_tests

    def run(self):
        sizes = self._generator.gen(self._example_count)
        if not self._skip_tests:
            examples = [example for example in sorted(glob.glob("./dump/*")) if os.path.isfile(example)]
            for i, example in enumerate(examples):
                print(f"Completed {i}/{self._example_count}")
                binaries = [strategy.compile(example) for strategy in self._comp_strategies]
                for exec_strategy in self._exec_strategies:
                    exec_strategy.run(binaries)
        for exec_strategy in self._exec_strategies:
            if isinstance(exec_strategy, LineGraph):
                exec_strategy.set_sizes(sizes)
                exec_strategy._skip_tests = self._skip_tests
            exec_strategy.show()

    @staticmethod
    def create(examples: List[str], name="example"):
        for (i, program) in enumerate(examples):
            f = open(f"./dump/{name}_{i}.c", 'w')
            f.write(program)
            f.close()


class ComparisonBenchmark:
    def __init__(self,
                 exec_strategies: List[IExecutionStrategy],
                 example_count=100,
                 skip_tests=False):
        self._generator = TestExampleGenerator("./examples/arr_red.c", "./examples/arr_red1.c", 5000)
        self._compiler = ClangStrategy()
        self._exec_strategies = exec_strategies
        self._example_count = example_count
        self._skip_tests = skip_tests

    def run(self):
        sizes = self._generator.gen(self._example_count)
        if not self._skip_tests:
            examplesA = sorted(glob.glob("./dump/A/*"))
            examplesB = sorted(glob.glob("./dump/B/*"))
            for i, (A, B) in enumerate(zip(examplesA, examplesB)):
                print(f"Completed {i}/{self._example_count}")
                binaries = [self._compiler.compile(A), self._compiler.compile(B)]
                for exec_strategy in self._exec_strategies:
                    exec_strategy.run(binaries)
        for exec_strategy in self._exec_strategies:
            if isinstance(exec_strategy, LineGraph):
                exec_strategy.set_sizes(sizes)
                exec_strategy._skip_tests = self._skip_tests
            exec_strategy.show()
