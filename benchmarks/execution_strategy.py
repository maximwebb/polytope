import subprocess
import time
from abc import abstractmethod, ABC
from datetime import datetime
from typing import List, Dict, TypedDict


class IExecutionStrategy(ABC):
    @abstractmethod
    def run(self, files: List[str]) -> any:
        pass

    @abstractmethod
    def show(self) -> any:
        pass


class CorrectnessTest(IExecutionStrategy):
    def __init__(self):
        self._correct_count = 0
        self._count = 0

    def run(self, files: List[str]) -> bool:
        self._count += 1
        res = self.__run(files)
        if res:
            self._correct_count += 1
        return res

    def show(self) -> any:
        print()
        print("----Correctness Test----")
        if self._correct_count == self._count:
            print("PASSED")
        else:
            print("FAILED")
        print(f"{self._correct_count}/{self._count} passed")

    @staticmethod
    def __run(files: List[str]) -> bool:
        outputs = []
        for file in files:
            process = subprocess.run(file,
                                     capture_output=True,
                                     text=True)
            stdout = process.stdout
            stderr = process.stderr
            if stderr != '':
                log_file = f"correctness_{datetime.now().strftime('%H:%M:%S')}.txt"
                print(f"Error running {file} - printing to {log_file}...")
                with open(f"./logs/{log_file}", 'w') as f:
                    f.write(stderr)
                return False
            outputs.append(stdout)

        for i in range(len(outputs) - 1):
            if outputs[i] != outputs[i + 1]:
                log_file = f"correctness_{datetime.now().strftime('%H:%M:%S')}.txt"
                print(f"{files[i]} and {files[i + 1]} do not match - printing to {log_file}...")
                with open(f"./logs/{log_file}", 'w') as f:
                    f.write(outputs[i])
                    f.write("\n")
                    f.write(outputs[i + 1])
                return False
        return True


# Assumes CorrectnessTest has been run beforehand.
class TimeTest(IExecutionStrategy):
    def __init__(self, iterations, names: List[str]):
        self._iterations = iterations
        self._names = names
        self._times = {name: 0.0 for name in names}

    def run(self, files: List[str]) -> Dict[str, float]:
        results = dict()
        for file, name in zip(files, self._names):
            start = time.time()
            for _ in range(self._iterations):
                subprocess.run(file, stdout=subprocess.DEVNULL)
            results[name] = time.time() - start

        for name, t in results.items():
            self._times[name] += t

        return results

    def show(self) -> any:
        print()
        print("----Time Test----")
        print(self._times)
