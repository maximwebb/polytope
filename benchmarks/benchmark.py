import glob
from typing import List

from compilation_strategy import ICompilationStrategy
from example_generator import IExampleGenerator
from execution_strategy import IExecutionStrategy


class Benchmark:
    def __init__(self,
                 generator: IExampleGenerator,
                 comp_strategies: List[ICompilationStrategy],
                 exec_strategies: List[IExecutionStrategy],
                 example_count=100):
        self._generator = generator
        self._comp_strategies = comp_strategies
        self._exec_strategies = exec_strategies
        self._example_count = example_count

    def run(self):
        self.create(self._generator.gen(self._example_count))
        examples = glob.glob("./dump/*")
        for i, example in enumerate(examples):
            print(f"Completed {i}/{self._example_count}")
            binaries = [strategy.compile(example) for strategy in self._comp_strategies]
            for exec_strategy in self._exec_strategies:
                exec_strategy.run(binaries)
        for exec_strategy in self._exec_strategies:
            exec_strategy.show()

    @staticmethod
    def create(examples: List[str], name="example"):
        for (i, program) in enumerate(examples):
            f = open(f"./dump/{name}_{i}.c", 'w')
            f.write(program)
            f.close()
