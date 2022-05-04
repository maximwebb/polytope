import glob
import re
from abc import abstractmethod, ABC
import random
from typing import List, Tuple
import numpy as np


class IExampleGenerator(ABC):
    @abstractmethod
    def gen(self, n):
        pass


class RandomLinGenerator(IExampleGenerator):
    def __init__(self, size: int):
        self._size = size

    def gen(self, n) -> List[str]:
        random.seed(4)
        examples = []
        for i in range(n):
            write = (self.get_rand_affine(), self.get_rand_affine())
            num_reads = random.randint(1, 4)
            reads = [(self.get_rand_affine(), self.get_rand_affine()) for _ in range(num_reads)]
            examples.append(
                f"""#include <stdio.h>
#include <stdlib.h>
#define N {self._size}
#define M {self._size}
#define seed 7


int main() {{
    int (*A)[M] = malloc(sizeof(int[N][M]));

    for (int i = 0; i < N; i++) {{
        for (int j = 0; j < N; j++) {{
            A[i][j] = (i + j*seed) % 4;
        }}
    }}

    for (int i = 3; i < N/4 - 1; ++i) {{
        for (int j = 3; j < N/4 - 1; ++j) {{
            {self.stringify(write)} = ({" + ".join([self.stringify(read) for read in reads])}) % 17;
        }}
    }}

    for (int i = 0; i < N; ++i) {{
        for (int j = 0; j < N; ++j) {{
            printf("%d, ", A[i][j]);
        }}
        printf("\\n");
    }}
    free(A);
    return 0;
}}
"""
            )

        for (i, program) in enumerate(examples):
            f = open(f"./dump/example_{i}.c", 'w')
            f.write(program)
            f.close()

        return examples

    @staticmethod
    def get_rand_affine(r=2) -> List[int]:
        res = [random.randint(0, r), random.randint(0, r)]
        if res[0] == 0 and res[1] == 0:
            res.append(random.randint(0, r))
        else:
            res.append(random.randint(-r, r))
        return res

    @staticmethod
    def stringify(access: Tuple[List[int], List[int]]) -> str:
        access_str = []
        for affine in access:
            s = "["
            if affine[0] == 1:
                s += "i"
            elif affine[0] > 1:
                s += f"{affine[0]}*i"

            if affine[0] >= 1 and affine[1] >= 1:
                s += "+"

            if affine[1] == 1:
                s += "j"
            elif affine[1] > 1:
                s += f"{affine[1]}*j"

            if affine[2] >= 1 and (affine[0] >= 1 or affine[1] >= 1):
                s += "+"

            if affine[2] != 0 or (affine[0] == 0 and affine[1] == 0):
                s += f"{affine[2]}"

            s += "]"
            access_str.append(s)

        return "A" + "".join(access_str)


class TestGenerator(IExampleGenerator):
    def __init__(self, size: int):
        self._size = size

    def gen(self, n) -> List[str]:
        random.seed(4)
        examples = [f"""#include <stdio.h>
#include <stdlib.h>
#define N {self._size}
#define M {self._size}
#define seed 7


int main() {{
    int (*A)[M] = malloc(sizeof(int[N][M]));

    for (int i = 0; i < N; i++) {{
        for (int j = 0; j < N; j++) {{
            A[i][j] = (i + j*seed) % 4;
        }}
    }}

    for (int i = 1; i < N; ++i) {{
        for (int j = 1; j < N; ++j) {{
            A[i][j] = A[i][j-1];
        }}
    }}
    
    for (int i = 0; i < 4; ++i) {{
        for (int j = 0; j < 5; ++j) {{
            printf("%d, ", A[i][j]);
        }}
        printf("\\n");
    }}
    
    free(A);
    return 0;
}}
"""]
        for (i, program) in enumerate(examples):
            f = open(f"./dump/example_{i}.c", 'w')
            f.write(program)
            f.close()

        return examples


class SelectedExampleGenerator(IExampleGenerator):
    def __init__(self, size: int):
        self._size = size

    def gen(self, n=6):
        examples = sorted(glob.glob("./examples/*"))
        res = []
        for example in examples[:n]:
            file_name = f"./dump/{example.split('/')[-1]}"
            with open(example, "r") as f1, open(file_name, "w") as f2:
                text = f1.read()
                new_text = re.sub("#define (N|M).*", f"#define \\1 {self._size}", text)
                new_text = re.sub("#define (n|m).*", f"#define \\1 {self._size//3}", new_text)
                f2.write(new_text)
        return res


class RepeatedExampleGenerator(IExampleGenerator):
    def __init__(self, path: str, max_size: int):
        self._path = path
        self._max_size = max_size

    def gen(self, n=6):
        with open(self._path, "r") as f1:
            text = f1.read()
            sizes = [int(x) for x in np.linspace(100, self._max_size, n)]
            for i, size in enumerate(sizes):
                new_text = re.sub("#define (N|M).*", f"#define \\1 {size}", text)
                file_name = f"./dump/{chr(65+i)}_{self._path.split('/')[-1].split('.')[0]}_{size}.c"
                with open(file_name, "w") as f2:
                    f2.write(new_text)
        return sizes