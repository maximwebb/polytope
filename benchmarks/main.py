import glob
import os
from typing import List

from compilation_strategy import ClangStrategy, OptClangStrategy, ClangO3Strategy
from example_generator import RandomLinGenerator
from execution_strategy import CorrectnessTest, TimeTest


def main():
    clear()
    rand_lin = RandomLinGenerator(size=1000)
    clang = ClangStrategy()
    opt_clang = OptClangStrategy()
    o3_clang = ClangO3Strategy()
    correctness_test = CorrectnessTest()
    names = ["clang", "opt clang", "O3 clang"]
    time_test = TimeTest(iterations=10, names=names)

    # Prepare examples
    create(rand_lin.gen(5))
    examples = glob.glob("./dump/*")

    # Initialise results
    correct = True
    time_results = {name: 0.0 for name in names}

    # Run tests
    for i, example in enumerate(examples):
        print(f"Completed {i}/{len(examples)}")
        binary1 = clang.compile(example)
        binary2 = opt_clang.compile(example)
        binary3 = o3_clang.compile(example)
        correct &= correctness_test.run([binary1, binary2, binary3])
        res = time_test.run([binary1, binary2, binary3])
        for name, time in res.items():
            time_results[name] += time

    # Display results
    if correct:
        print("Correctness test passed")
    else:
        print("Correctness test failed")

    print("---Time test results---")
    print(time_results)


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
