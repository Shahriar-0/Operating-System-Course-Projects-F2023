import os
import subprocess
import fileinput
from tqdm import tqdm

pool_hpp_path = "./parallel/src/pool.hpp"
speedup_path = "./speedup.sh"

thread_counts = sorted(set(range(1, 9)).difference({4})) + [4]


def modify_scripts(thread_count, input_number):
    with fileinput.FileInput(pool_hpp_path, inplace=True) as file:
        for line in file:
            if "THREAD_COUNT" in line:
                line = f"constexpr int THREAD_COUNT = {thread_count};\n"
            print(line, end="")

    with fileinput.FileInput(speedup_path, inplace=True) as file:
        for line in file:
            if line.startswith("threads="):
                line = f"threads={thread_count}\n"
            elif line.startswith("inputMain="):
                line = f'inputMain="../Assets/Pictures/input{input_number}.bmp"\n'
            print(line, end="")

    subprocess.run([speedup_path, "runparallel"])


if __name__ == "__main__":
    import sys

    subprocess.run([speedup_path, "clean"])
    print("\033[92m" + "Running serial" + "\033[0m")
    subprocess.run([speedup_path, "runserial"])
    print("\033[92m" + "Done running serial" + "\033[0m")
    input_number = sys.argv[1]
    print("\033[92m" + "Running parallel" + "\033[0m")
    for thread_count in tqdm(thread_counts):
        print("\r\033[92m" + f"Running parallel with {thread_count} threads" + "\033[0m", end="")
        modify_scripts(thread_count, input_number)
    print("\r\033[92m" + "Done running parallel" + "\033[0m")

