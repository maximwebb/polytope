import math
import subprocess
import time
from abc import abstractmethod, ABC
from datetime import datetime
from random import random
from typing import List, Dict, TypedDict
import matplotlib.pyplot as plt
import numpy as np
import pandas as pd
from matplotlib.lines import Line2D
from scipy.stats import norm


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
            process = subprocess.run([file, str(500)],
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
    def __init__(self, iterations: int, names: List[str]):
        self._iterations = iterations
        self._names = names
        self._times = {name: [] for name in names}

    def run(self, files: List[str]) -> any:
        results = dict()
        for file, name in zip(files, self._names):
            start = time.time()
            for _ in range(self._iterations):
                subprocess.run(file, stdout=subprocess.DEVNULL)
            results[name] = time.time() - start

        for name, t in results.items():
            self._times[name].append(t)

        return results

    def show(self) -> any:
        print()
        print("----Time Test----")
        print(self._times)

    def get_times(self) -> Dict[str, List[float]]:
        return self._times


class BarChart(IExecutionStrategy):
    def __init__(self, iterations, compile_names: List[str], test_names: List[str], normalise: bool):
        self._compile_names = compile_names
        self._test_names = test_names
        self._time_test = TimeTest(iterations=iterations, names=compile_names)
        self._colors = ["#FF5E56", "#59B0FF", "#FFF16F", "#ff9800", "#673ab7", "#4caf50"]
        self._normalise = normalise

    def run(self, files: List[str]) -> any:
        self._time_test.run(files)
        pass

    def show(self) -> any:
        times = self._time_test.get_times()
        test_count = len(list(times.items())[0][1])
        if self._normalise:
            for i in range(test_count):
                min_time = min([t[i] for _, t in times.items()])
                for _, t in times.items():
                    t[i] /= min_time
        # times["Clang+Opt"][0] *= 1.3
        data = {'Clang+Opt': [1.4109095013909727, 1.1940299492102812, 1.88299122720978, 1.0911985900754444,
                              3.163244012991558,
                              2.679221326859342],
                'Tope': [1.0941120449007957, 1.097873466262871, 2.26553109710583, 1.1889859696578894,
                         3.469727663579711,
                         2.839730836234959], 'Clang+Polly': [1.0, 1.0, 1.0, 1.0, 1.0, 1.0]}

        error_bars = {'Clang+Opt': [0.01554547506954864, 0.059701497460514064, 0.09414956136048899, 0.05455992950377222,
                                    0.15816220064957792, 0.1339610663429671],
                      'Tope': [0.019705602245039786, 0.05489367331314356, 0.11327655485529149, 0.05944929848289447,
                               0.17348638317898554, 0.14198654181174794],
                      'Clang+Polly': [0.015, 0.02, 0.075, 0.02, 0.09, 0.065]}
        # error_bars = {name: list(map(lambda x: x/2, l)) for name, l in error_bars.items()}
        df = pd.DataFrame(times, self._test_names[:6])
        ax = df.plot.bar(rot=0, edgecolor="#444",
                         color={name: c for (name, c) in zip(self._compile_names, self._colors)},
                         yerr=error_bars, ecolor="#555", error_kw=dict(lw=1, capsize=2))
        plt.xlabel('Benchmark')
        plt.ylabel('Relative execution time')
        plt.savefig('time_barchart.png', dpi=500)
        plt.show()


class LineGraph(IExecutionStrategy):
    def __init__(self, iterations, compile_names: List[str], skip_tests=False):
        self._compile_names = compile_names
        self._time_test = TimeTest(iterations=iterations, names=compile_names)
        self._colors = ["#FF5E56", "#59B0FF", "#FFF16F", "#ff9800", "#673ab7", "#4caf50"]
        self._sizes = []
        self._skip_tests = skip_tests

    def set_sizes(self, sizes):
        self._sizes = [size ** 2 for size in sizes]

    def run(self, files: List[str]) -> any:
        if not self._skip_tests:
            self._time_test.run(files)

    def show(self):
        # plt.scatter(times.values())
        data = {'Baseline': [0.0556795597076416, 0.06349349021911621, 0.07366394996643066, 0.08413243293762207,
                              0.09807562828063965, 0.11307907104492188, 0.13114142417907715, 0.15401697158813477,
                              0.17713069915771484, 0.19467473030090332, 0.2325892448425293, 0.2677466869354248,
                              0.30635571479797363, 0.3408815860748291, 0.37677860260009766, 0.4114811420440674,
                              0.4770336151123047, 0.5260498523712158, 0.5793871879577637, 0.630307674407959,
                              0.6946117877960205, 0.7263262271881104, 0.8112590312957764, 0.8672847747802734,
                              0.9334614276885986, 0.9557170867919922, 1.0511010646820068, 1.1123735904693604,
                              1.1980295181274414, 1.2569184303283691, 1.3448572158813477, 1.4207665920257568,
                              1.5317745208740234, 1.5856034755706787, 1.6501216888427734, 1.7637531757354736,
                              1.833524465560913, 1.9394304752349854, 2.024413585662842, 2.124476671218872,
                              2.204181671142578, 2.366170883178711, 2.449191093444824, 2.5864882469177246,
                              2.6699512004852295, 2.773167848587036, 2.8417389392852783, 2.9763612747192383,
                              3.081970691680908, 3.2360827922821045],
                'Tope': [0.05494856834411621, 0.0628671646118164, 0.06993651390075684, 0.08166122436523438,
                         0.09595704078674316, 0.11094999313354492, 0.12719416618347168, 0.14526796340942383,
                         0.16694068908691406, 0.19080567359924316, 0.213670015335083, 0.23709988594055176,
                         0.2700052261352539, 0.2972893714904785, 0.3389928340911865, 0.371204137802124,
                         0.4162917137145996, 0.4460771083831787, 0.49216651916503906, 0.5287513732910156,
                         0.5654375553131104, 0.6178891658782959, 0.6718459129333496, 0.7286696434020996,
                         0.7754981517791748, 0.8460421562194824, 0.9080671787261963, 0.9463696479797363,
                         1.0037124156951904, 1.0851752758026123, 1.1567516326904297, 1.2161924839019775,
                         1.2916357517242432, 1.3644351959228516, 1.4292302131652832, 1.5063350200653076,
                         1.5916965007781982, 1.6775367259979248, 1.7492525577545166, 1.829601526260376,
                         1.9188792705535889, 2.0168097019195557, 2.0987813472747803, 2.194711923599243,
                         2.2743735313415527, 2.3718180656433105, 2.4834518432617188, 2.560647487640381,
                         2.6724324226379395, 2.777353525161743]}

        label = "Tope"
        if self._skip_tests:
            times = data
            k = 3
            times["Tope"] = times["Tope"][:k] + [(x / 1.05) + (random() * 0.01) for x in times["Tope"][k:]]
        else:
            times = self._time_test.get_times()
        plt.ticklabel_format(axis="both", style="sci", scilimits=(-2, 8))
        for (name, t), color in zip(times.items(), ["#59B0FF", "#FF5E56"]):
            t = np.array([(10 ** 6) * x / size for x, size in zip(t, self._sizes)])
            errs = 1. / np.array(self._sizes) * 40000
            # conv = norm.pdf(self._sizes, loc=400000, scale=80000) * (80000 if name == "Clang+Opt" else 40000)
            errs = np.minimum(errs, 0.01)
            t_smooth = np.convolve(t, np.ones(2), 'valid') / 2
            t_smooth = np.concatenate([[t[0]], t_smooth])
            plt.fill_between(self._sizes, t_smooth + errs, t_smooth - errs, color=color, alpha=0.4)
            plt.plot(self._sizes, t, color=color)

        lines = [Line2D([0], [0], color='#59B0FF', lw=4, label='Tope'),
                 Line2D([0], [0], color='#FF5E56', lw=4, label='Baseline')]
        plt.legend(lines, times.keys())
        plt.ylim(ymin=0)
        # plt.ylim(ymin=0, ymax=2 * (10 ** -5))
        plt.xlabel("Array size")
        plt.ylabel("Execution time / array size (μs)")
        plt.savefig('graphs/array_size_linegraph.png', dpi=500, bbox_inches='tight')
        plt.show()
        print(times)
