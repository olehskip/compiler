import sys
import os
import subprocess
import difflib
import shutil
from datetime import datetime


def run_program(program, *args):
    try:
        result = subprocess.run(
            [program, *args], capture_output=True, text=True, check=True
        )
        return result.stdout
    except subprocess.CalledProcessError as e:
        # print(f"Error running {program} on {filepath}: {e}")
        print(f"Can't run {program} with args {args}, error = {e}")
        return None


def compile_chicken(program, output_folder):
    os.mkdir(output_folder)
    output_file = os.path.join(output_folder, "output")
    run_program("chicken-csc", program, "-o", output_file)
    stdout = run_program(output_file)
    if stdout is not None:
        with open(os.path.join(output_folder, "stdout.txt"), "w") as f:
            f.write(stdout)
    return stdout


def compile_mine(program, output_folder):
    # os.mkdir(output_folder)
    output_file = os.path.join(output_folder, "output")
    if run_program("./build/compiler_output", program, output_folder) is None:
        return None
    run_program(
        "nasm",
        "-f",
        "elf64",
        os.path.join(output_folder, "output.nasm"),
        "-o",
        os.path.join(output_folder, "output.o"),
    )
    run_program(
        "ld",
        os.path.join(output_folder, "output.o"),
        "./build/src/std/CMakeFiles/std.dir/std.asm.o",
        "-o",
        os.path.join(output_folder, "output"),
    )
    stdout = run_program(output_file)
    if stdout is not None:
        with open(os.path.join(output_folder, "stdout.txt"), "w") as f:
            f.write(stdout)
    return stdout


def compare_outputs(current_folder, filename, output_folder):
    filepath = os.path.join(current_folder, filename)
    print(f"Processing file: {filepath}")

    output1 = compile_mine(filepath, os.path.join(output_folder, f"{filename}-mine"))
    output2 = compile_chicken(
        filepath, os.path.join(output_folder, f"{filename}-chicken")
    )

    if output1 is None:
        print("Mine failed")
    if output2 is None:
        print("Chicken failed")
    if output1 is not None and output2 is not None:
        if output1 == output2:
            print("Outputs match")
            return True
        else:
            print("Outputs differ")
            print("Diff:")
            diff = difflib.unified_diff(
                output1.splitlines(), output2.splitlines(), lineterm=""
            )
            print("\n".join(diff))
            return False
    return False


def compare_outputs_recursively(directory):
    output_folder = "./output/" + datetime.now().strftime("%d-%m-%Y_%H:%M:%S:%f")
    os.makedirs(output_folder)
    for current_folder, _, filenames in os.walk(directory, followlinks=True):
        for filename in filenames:
            is_same = compare_outputs(current_folder, filename, output_folder)
            print("------------------------")


if __name__ == "__main__":
    compare_outputs_recursively("./tests/scheme_programs")
