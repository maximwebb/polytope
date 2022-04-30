import os
import subprocess
from abc import abstractmethod, ABC

# Nasty
OPT_PATH = "../llvm-project/llvm/build/bin/opt"
POLY_PATH = "../polytope-pass/cmake-build-debug/libpolytope-pass.so"


class ICompilationStrategy(ABC):
    @abstractmethod
    def compile(self, file: str) -> str:
        pass


class ClangStrategy(ICompilationStrategy):
    def compile(self, file: str) -> str:
        file_name = f"./bin/{file.split('/')[-1].split('.')[0]}_clang"
        subprocess.run(["clang", "-O0", file, "-o", file_name])
        return file_name


class ClangO3Strategy(ICompilationStrategy):
    def compile(self, file: str) -> str:
        file_name = f"./bin/{file.split('/')[-1].split('.')[0]}_clang_o3"
        subprocess.run(["clang", file, "-O3", "-o", file_name])
        return file_name


# Lowers to LLVM IR
class OptStrategy(ICompilationStrategy):
    def compile(self, file: str) -> str:
        file_name = f"./bin/{file.split('/')[-1].split('.')[0]}_opt.ll"
        subprocess.run(
            ["clang", "-emit-llvm", "-fno-discard-value-names", "-O0", "-Xclang", "-disable-O0-optnone", file, "-S",
             "-o", file_name]
        )
        process = subprocess.run(
            [OPT_PATH, "-S", "-passes", "mem2reg,loop-rotate,simplifycfg,instcombine,loop-vectorize", file_name, "-o", file_name]
        )

        return file_name


class OptClangStrategy(ICompilationStrategy):
    def __init__(self):
        self.__clang = ClangStrategy()
        self.__opt = OptStrategy()

    def compile(self, file: str) -> str:
        ir = self.__opt.compile(file)
        res = self.__clang.compile(ir)
        os.remove(ir)
        return res


class PolytopeStrategy(ICompilationStrategy):
    def __init__(self):
        self.__clang = ClangStrategy()
        self.__opt = OptStrategy()

    def compile(self, file: str) -> str:
        ir = self.__opt.compile(file)
        file_name = f"./bin/{ir.split('/')[-1].split('.')[0]}_poly.ll"
        subprocess.run(
            [OPT_PATH, "-S", "-load-pass-plugin", POLY_PATH, "-passes", "polytope", ir, "-o", file_name]
        )
        subprocess.run(
            [OPT_PATH, "-S", "-passes", "simplifycfg,instcombine", file_name, "-o", file_name]
        )
        res = self.__clang.compile(file_name)
        # os.remove(ir)
        # os.remove(file_name)
        return res
