import sys
import os
import subprocess
import difflib
import shutil

def run_program(program, *args):
    try:
        result = subprocess.run([program, *args], capture_output=True, text=True, check=True)
        return result.stdout
    except subprocess.CalledProcessError as e:
        # print(f"Error running {program} on {filepath}: {e}")
        print(f"Can't run {program} with args {args}, error = {e}")
        return None

def compile_chicken(program):
    OUTPUT_FOLDER = "./output"
    try:
        shutil.rmtree(OUTPUT_FOLDER)
    except:
        pass
    os.mkdir(OUTPUT_FOLDER)
    run_program("chicken-csc", program, "-o", "./output/output")
    return run_program("./output/output")

def compile_mine(program):
    OUTPUT_FOLDER = "./output"
    try:
        shutil.rmtree(OUTPUT_FOLDER)
    except:
        pass
    run_program("./build/compiler_output", program, OUTPUT_FOLDER)
    run_program("nasm", "-f", "elf64", "./output/output.nasm", "-o", "./output/output.o")
    run_program("ld", "./output/output.o", "./build/src/std/CMakeFiles/std.dir/std.asm.o", "-o", "./output/output")
    return run_program("./output/output")

def compare_outputs(directory):
    for filename in os.listdir(directory):
        filepath = os.path.join(directory, filename)
        if os.path.isfile(filepath):
            print(f"Processing file: {filepath}")
            
            output1 = compile_mine(filepath)
            output2 = compile_chicken(filepath)
            
            if output1 is None or output2 is None:
                continue
            
            if output1 == output2:
                print("Outputs match")
            else:
                print("Outputs differ")
                print("Diff:")
                diff = difflib.unified_diff(output1.splitlines(), output2.splitlines(), lineterm='')
                print('\n'.join(diff))
                break 
            print("------------------------")

if __name__ == "__main__":
    # compile_mine("./examples/begin.scheme")
    # compile_chicken("./examples/begin.scheme")
    # if len(sys.argv) != 4:
    #     print(f"Usage: {sys.argv[0]} <program1> <program2> <directory>")
    #     sys.exit(1)
    # 
    # program1, program2, directory = sys.argv[1], sys.argv[2], sys.argv[3]
    compare_outputs("examples")
