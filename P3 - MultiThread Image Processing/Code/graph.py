# import matplotlib.pyplot as plt
# import os

# def read_execution_time(filename):
#     with open(filename, "r") as file:
#         for line in file:
#             if line.startswith("Execution Time:"):
#                 return float(line.split(":")[1].strip()[:-3])
#     return None

# prefix = "../Assets/Results/"
# serial_time = read_execution_time(prefix + "serial.txt")

# speedups = []
# thread_counts = sorted(set(range(1, 9)))

# for i in thread_counts:
#     parallel_time = read_execution_time(prefix + f"parallel_{i}.txt")
#     speedup = serial_time / parallel_time
#     speedups.append(speedup)

# best_thread = thread_counts[speedups.index(max(speedups))]
# best_time = serial_time / max(speedups)

# plt.plot(thread_counts, speedups, marker="o")
# plt.title(f"Speedup of parallel execution\nSerial Time: {serial_time}s\nBest Thread: {best_thread}\nBest Time: {best_time}s")
# plt.xlabel("Number of threads")
# plt.ylabel("Speedup")
# plt.grid(True)

# plt.savefig("../Assets/Pictures/graph.png")
# plt.show()
import os
import subprocess
import fileinput
from tqdm import tqdm
import numpy as np
import matplotlib.pyplot as plt

pool_hpp_path = "./parallel/src/pool.hpp"
speedup_path = "./speedup.sh"

thread_counts = sorted(set(range(1, 9)).difference({4})) + [4]
num_runs = 5
prefix = "../Assets/Results/"


def read_execution_time(filename):
    with open(filename, "r") as file:
        for line in file:
            if line.startswith("Execution Time:"):
                return float(line.split(":")[1].strip()[:-3])
    return None


if __name__ == "__main__":
    import sys

    times = [_ for _ in range(9)]
    serial_time = 0
    for i in tqdm(range(num_runs), colour="blue"):
        subprocess.run([f"python ./threads.py {sys.argv[1]}"], shell=True)

        serial_time += read_execution_time(prefix + "serial.txt")
        for thread_count in thread_counts:
            times[thread_count] += read_execution_time(
                prefix + f"parallel_{thread_count}.txt"
            )
        print("---------------------------------------------------------------------------")

    serial_time /= num_runs
    times = [t / num_runs for t in times]

    speedups = []
    for i in range(1, 9):
        speedup = serial_time / times[i]
        speedups.append(speedup)

    best_thread = speedups.index(max(speedups))
    
    best_time = serial_time / max(speedups)

    plt.plot(range(1, 9), speedups, marker="o")
    plt.title(
        f"Speedup of parallel execution\nSerial Time: {serial_time}s\nBest Thread: {best_thread}\nBest Time: {best_time}s"
    )
    plt.xlabel("Number of threads")
    plt.ylabel("Speedup")
    plt.grid(True)

    plt.savefig("../Assets/Pictures/graph.png")
    plt.show()
