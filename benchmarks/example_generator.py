from abc import abstractmethod, ABC
import random
from typing import List, Tuple


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
    return 0;
}}
"""
            )

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
