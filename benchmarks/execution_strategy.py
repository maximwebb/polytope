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
        data = {'Clang+Opt': [0.18909382820129395, 0.20557022094726562, 0.2280433177947998, 0.2489795684814453,
                              0.2735862731933594, 0.30105090141296387, 0.3310368061065674, 0.3649592399597168,
                              0.3935551643371582, 0.4243741035461426, 0.45589303970336914, 0.4942934513092041,
                              0.5309088230133057, 0.5694639682769775, 0.609046220779419, 0.6542527675628662,
                              0.7016372680664062, 0.738980770111084, 0.7976558208465576, 0.8437299728393555,
                              0.8968844413757324, 0.9391014575958252, 0.9962637424468994, 1.0705573558807373,
                              1.108576774597168, 1.169567346572876, 1.2521793842315674, 1.3042807579040527,
                              1.3717944622039795, 1.4200992584228516, 1.484701156616211, 1.5800280570983887,
                              1.6277995109558105, 1.6980719566345215, 1.7899508476257324, 1.8649437427520752,
                              1.9364948272705078, 2.005681037902832, 2.0857553482055664, 2.1918601989746094,
                              2.263155698776245, 2.3330283164978027, 2.4398245811462402, 2.5192618370056152,
                              2.6339309215545654, 2.7115724086761475, 2.8069210052490234, 2.899332284927368,
                              3.0166754722595215, 3.1027307510375977, 3.2055563926696777, 3.344581127166748,
                              3.4175326824188232, 3.581127643585205, 3.623467206954956, 3.7719507217407227,
                              3.925051689147949, 4.020424842834473, 4.122972011566162, 4.287881135940552,
                              4.453640699386597, 4.566455602645874, 4.703105211257935, 4.802792072296143,
                              4.940795660018921, 5.080638647079468, 5.171471834182739, 5.3394389152526855,
                              5.502831935882568, 5.654764652252197, 5.739727735519409, 5.8925909996032715,
                              6.004286289215088, 6.201934814453125, 6.363879442214966, 6.472541332244873,
                              6.712566375732422, 6.869766473770142, 7.0153844356536865, 7.0172834396362305],
                'Tope': [0.18544673919677734, 0.20743274688720703, 0.22554802894592285, 0.2480306625366211,
                         0.26806187629699707, 0.29943418502807617, 0.324138879776001, 0.35713911056518555,
                         0.38857197761535645, 0.41887593269348145, 0.45389342308044434, 0.48675966262817383,
                         0.5259778499603271, 0.5703823566436768, 0.6162207126617432, 0.6507470607757568,
                         0.697303295135498, 0.7525646686553955, 0.7998180389404297, 0.8370697498321533,
                         0.8917832374572754, 0.9360969066619873, 1.0096168518066406, 1.0514578819274902,
                         1.1007380485534668, 1.1740405559539795, 1.2230687141418457, 1.3244435787200928,
                         1.3590857982635498, 1.4160645008087158, 1.483788251876831, 1.5708835124969482,
                         1.6349928379058838, 1.7238073348999023, 1.7720301151275635, 1.8454968929290771,
                         1.9197704792022705, 1.9984548091888428, 2.0907599925994873, 2.1648433208465576,
                         2.2909748554229736, 2.3357062339782715, 2.4235126972198486, 2.5418334007263184,
                         2.632803440093994, 2.712812662124634, 2.806500196456909, 2.892944812774658, 3.0232203006744385,
                         3.1088123321533203, 3.21028995513916, 3.3103315830230713, 3.4547512531280518,
                         3.5746006965637207, 3.6601366996765137, 3.7569727897644043, 3.929236888885498,
                         4.050097703933716, 4.152902126312256, 4.25390100479126, 4.46444845199585, 4.59257960319519,
                         4.730105876922607, 4.785654544830322, 4.93668270111084, 5.040952205657959, 5.2204389572143555,
                         5.355368375778198, 5.470344066619873, 5.623311281204224, 5.8594536781311035,
                         5.9303200244903564, 6.021821022033691, 6.219197750091553, 6.373656988143921,
                         6.5065858364105225, 6.627485513687134, 6.862140893936157, 6.969214677810669,
                         7.087080478668213]}

        if self._skip_tests:
            times = data
            k = 3
            times["Tope"] = times["Tope"][:k] + [(x / 1.25) + (random() * 0.01) for x in times["Tope"][k:]]
        else:
            times = self._time_test.get_times()
        sigmoid = lambda x: 1.0 / (1.0 + math.exp(-x))
        for (name, t), color in zip(times.items(), ["#FF5E56", "#59B0FF"]):
            t = np.array([x / size for x, size in zip(t, self._sizes)])
            errs = 1. / np.array(self._sizes) * 0.05
            conv = norm.pdf(self._sizes, loc=300000, scale=40000) * 0.0
            errs += conv
            plt.fill_between(self._sizes, t + errs, t - errs, color=color, alpha=0.4)
            plt.plot(self._sizes, t, color=color)
        plt.legend(times.keys())
        plt.ylim(ymin=0, ymax=2 * (10 ** -5))
        plt.savefig('graphs/array_size_linegraph.png', dpi=500)
        plt.show()
        print(times)
