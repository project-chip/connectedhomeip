#!/usr/bin/env -S python3 -B

# Copyright (c) 2025 Project CHIP Authors
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import enum
import glob
import logging
import os
import re
import shutil
import subprocess
import sys
from dataclasses import dataclass
from typing import Optional

import click
import coloredlogs  # type: ignore

profile_output_folder = "out/profiling_fuzztest"
coverage_report_output_folder = "out/coverage_fuzztest"


class FuzzTestMode(enum.Enum):
    UNIT_TEST_MODE = enum.auto()
    CONTINUOUS_FUZZ_MODE = enum.auto()


@dataclass
class FuzzTestContext:
    fuzz_test_binary_path: str
    fuzz_test_binary_name: str
    build_target_name: str
    is_coverage_instrumented: bool
    selected_fuzz_test_case: Optional[str] = None
    coverage_output_base_name: Optional[str] = None
    run_mode: Optional[FuzzTestMode] = None


FUZZTEST_TOOLCHAIN_MARKER_DIR = "chip_pw_fuzztest"
FUZZTEST_BIN_PATTERN = f"{FUZZTEST_TOOLCHAIN_MARKER_DIR}/tests/*"


def get_build_target_from_fuzztest_path(binary_path):
    """Extract build directory name (target name) from the given binary path, by using FUZZTEST_TOOLCHAIN_MARKER_DIR as a reference point.

    This assumes the binary was built using the Pigweed FuzzTest toolchain, which GN places in a dedicated subdirectory with the toolchain name 'chip_pw_fuzztest' (as it is a secondary toolchain).
    The build target is always the parent directory of 'chip_pw_fuzztest'.
 """
    path_directories = binary_path.split(os.sep)

    for parent, child in zip(path_directories, path_directories[1:]):
        if child == FUZZTEST_TOOLCHAIN_MARKER_DIR:
            return parent

    raise ValueError(
        f"Could not deduce build target name: '{FUZZTEST_TOOLCHAIN_MARKER_DIR}' is not found in binary path: '{binary_path}'")


def list_fuzz_test_binaries():
    """Lists all compiled fuzz tests in the 'out' directory"""
    build_dir = "out"
    if not os.path.isdir(build_dir):
        logging.error(f"Error: Build directory '{build_dir}' does not exist.")
        return []

    fuzz_tests = []
    for file in glob.glob(f"{build_dir}/**/{FUZZTEST_BIN_PATTERN}", recursive=True):
        if os.access(file, os.X_OK):
            fuzz_tests.append(file)
    return fuzz_tests


def get_fuzz_test_cases(context):
    """Executes the fuzz test binary (i.e. Fuzz Test Suite) with --list_fuzz_tests to print the list of all available FUZZ_TESTs (i.e. Test Cases)"""
    try:
        logging.debug(f"\nfuzz_test_path = {context.fuzz_test_binary_path}\n")

        # workaround: we add this to supress creation of "default.profraw" which we don't need at this stage
        env = os.environ.copy()
        env["LLVM_PROFILE_FILE"] = os.devnull

        result = subprocess.run([context.fuzz_test_binary_path, "--list_fuzz_tests"], env=env, capture_output=True, text=True)
        output = result.stdout
        if output:
            return re.findall(r'test:\s*(\S+)', output)

        else:
            logging.info("No FUZZ_TESTs (TestCases) found in {context.fuzz_test_binary_path}")
            raise ValueError(f"FuzzTest Binary outputted the following error: \n{result.stderr}\n")

    except Exception as e:
        raise Exception(f"Error executing {context.fuzz_test_binary_path}: {e}")


def check_if_coverage_tools_detected():
    missing = []
    for tool in ["llvm-profdata", "llvm-cov", "genhtml"]:
        if shutil.which(tool) is None:
            missing.append(tool)

    if missing:
        raise Exception("Following required coverage packages not found: " + ", ".join(missing) +
                        "\nPlease either install them or source the correct environment")


def run_fuzz_test(context):
    """Runs the fuzz test and generates an LLVM profile file if binary is coverage instrumented"""

    # Create build-specific profile folder
    build_profile_folder = f"{profile_output_folder}/{context.build_target_name}"
    os.makedirs(build_profile_folder, exist_ok=True)

    if context.run_mode == FuzzTestMode.CONTINUOUS_FUZZ_MODE:
        # Use the FuzzTest (Test Case) Name  as the name for coverage output
        context.coverage_output_base_name = "{}".format(context.selected_fuzz_test_case.replace('.', "_"))
    elif context.run_mode == FuzzTestMode.UNIT_TEST_MODE:
        # Use the FuzzTest Binary Name  as the name for coverage output
        context.coverage_output_base_name = "{}".format(context.fuzz_test_binary_name)

    env = os.environ.copy()
    if context.is_coverage_instrumented:
        check_if_coverage_tools_detected()
        profraw_file = f"{build_profile_folder}/{context.coverage_output_base_name}.profraw"
        env["LLVM_PROFILE_FILE"] = profraw_file

    try:
        if context.run_mode == FuzzTestMode.UNIT_TEST_MODE:
            subprocess.run([context.fuzz_test_binary_path, ], env=env, check=True)
            logging.info("Fuzz Test Suite executed in Unit Test Mode.\n")
        elif context.run_mode == FuzzTestMode.CONTINUOUS_FUZZ_MODE:
            cmd_args = [context.fuzz_test_binary_path, f"--fuzz={context.selected_fuzz_test_case}"]
            # Use Popen instead of run() so we can always terminate cleanly and avoid profraw file issues
            process = subprocess.Popen(cmd_args, env=env)
            return_code = process.wait()
            if return_code != 0:
                raise subprocess.CalledProcessError(process.returncode, cmd_args)

    except KeyboardInterrupt:
        logging.info("\nFuzzing Interrupted by the user \n")
        if context.run_mode == FuzzTestMode.CONTINUOUS_FUZZ_MODE:
            process.terminate()
            process.wait()

    except Exception as e:
        raise ValueError(f"Error running fuzz test: {e}")


def generate_coverage_report(context, output_dir_arg):
    """Generates an HTML coverage report."""

    build_profile_folder = f"{profile_output_folder}/{context.build_target_name}"
    build_coverage_folder = f"{coverage_report_output_folder}/{context.build_target_name}"

    # Create build-specific directories
    os.makedirs(build_profile_folder, exist_ok=True)
    os.makedirs(build_coverage_folder, exist_ok=True)

    if not output_dir_arg:
        coverage_subfolder = f"{build_coverage_folder}/{context.coverage_output_base_name}"
    else:
        coverage_subfolder = output_dir_arg

    profraw_file = f"{build_profile_folder}/{context.coverage_output_base_name}.profraw"
    profdata_file = f"{build_profile_folder}/{context.coverage_output_base_name}.profdata"
    lcov_trace_file = f"{build_profile_folder}/{context.coverage_output_base_name}.info"

    if not os.path.exists(profraw_file):
        logging.error(f"Profile raw file not found: {profraw_file}")
        return False

    # Step1 Merge the profile data
    subprocess.run(["llvm-profdata", "merge", "-sparse", profraw_file, "-o", profdata_file], check=True)
    logging.debug(f"Profile data merged into {profdata_file}")

    # Step2 Exports coverage data into lcov trace file format.
    cmd = [
        "llvm-cov",
        "export",
        "-format=lcov",
        "--instr-profile",
        profdata_file,
        context.fuzz_test_binary_path
    ]

    # for -ignore-filename-regex
    ignore_paths = [
        "third_party/.*",
        "/usr/include/.*",
        "/usr/lib/.*",

    ]
    for p in ignore_paths:
        cmd.append("-ignore-filename-regex")
        cmd.append(p)

    with open(lcov_trace_file, "w") as file:
        subprocess.run(cmd, stdout=file, stderr=file)

    logging.debug("Data exported into lcov trace format")

    # Step3 Generate the coverage report
    cmd = ["genhtml"]

    errors_to_ignore = [
        "inconsistent", "source", "unmapped"
    ]
    for e in errors_to_ignore:
        cmd.append("--ignore-errors")
        cmd.append(e)

    flat = False
    cmd.append("--flat" if flat else "--hierarchical")
    cmd.append("--synthesize-missing")
    cmd.append("--output-directory")
    cmd.append(f"{coverage_subfolder}")
    cmd.append(f"{lcov_trace_file}")

    logging.info(f"Generating Coverage Report into: {coverage_subfolder}/index.html\n...Please wait...\n")
    try:
        subprocess.run(cmd, check=True, stdout=subprocess.DEVNULL)
    except FileNotFoundError:
        logging.error("genhtml not found. Please install lcov to generate the HTML coverage report")
        return

    logging.info("Coverage report Generated.")


def run_script_in_interactive_mode():

    fuzz_tests = list_fuzz_test_binaries()
    if not fuzz_tests:
        logging.error("No pigweed-based FuzzTests found in the 'out' directory.\n")
        logging.info("FuzzTests can be built using build_examples.py, for example:\n\n\tpython scripts/build/build_examples.py --target linux-x64-tests-clang-pw-fuzztest-coverage build \n")
        raise ValueError

    # ==== Choose FuzzTest Binary ====
    while True:
        print("=" * 70 + "\n")
        logging.info("INTERACTIVE MODE: Choose a FuzzTest Binary to Run:\n")
        for i, fuzz_test in enumerate(fuzz_tests, start=1):
            logging.info(f"\t{i}. {fuzz_test}")

        fuzz_choice = click.prompt("Enter the number of the fuzz test binary to run", type=int)

        if 1 <= fuzz_choice <= len(fuzz_tests):
            break
        else:
            logging.error("Invalid choice for fuzz test binary. Please try again")

    selected_fuzz = fuzz_tests[fuzz_choice - 1]
    context = FuzzTestContext(
        fuzz_test_binary_path=selected_fuzz,
        fuzz_test_binary_name=selected_fuzz.split(os.sep)[-1],
        build_target_name=get_build_target_from_fuzztest_path(selected_fuzz),
        is_coverage_instrumented="-coverage" in get_build_target_from_fuzztest_path(selected_fuzz)
    )

    if not context.is_coverage_instrumented:
        logging.error(
            f"\nFuzzTest Not coverage instrumented: No coverage report will be generated for: '{context.fuzz_test_binary_path}'\n")
        logging.error("for Coverage reports --> Build with Coverage by appending '-coverage' to target e.g.:\n\n\tpython scripts/build/build_examples.py --target linux-x64-tests-clang-pw-fuzztest-coverage build\n")
        logging.info("Continuing...")

    test_cases = get_fuzz_test_cases(context)

    # ==== Choose Test Case (FUZZ_TEST) ====
    while True:
        print("=" * 70 + "\n")
        logging.info("AVAILABLE TEST CASES --> Choose a number to run in 'Continuous Mode', continues until interrupted:\n")
        for i, case in enumerate(test_cases, start=1):
            logging.info(f"\t{i}. {case}")
        logging.info("\nUNIT_TEST_MODE: Enter 0 to run all test cases in Unit Test Mode (just a few seconds of each FUZZ_TEST(testcase)\n")

        choice = click.prompt("Enter the number of the test case to run", type=int)

        if 1 <= choice <= len(test_cases):
            context.run_mode = FuzzTestMode.CONTINUOUS_FUZZ_MODE
            context.selected_fuzz_test_case = test_cases[choice - 1]
            break

        elif choice == 0:
            context.run_mode = FuzzTestMode.UNIT_TEST_MODE
            break

        else:
            logging.info("Invalid choice. Please try again")

    return context


def run_script_in_normal_mode(fuzz_test, test_case, list_test_cases, help):
    fuzz_tests = list_fuzz_test_binaries()

    if help or not fuzz_test:
        logging.info("\nAVAILABLE FUZZTEST BINARIES in 'out' directory (each Binary can have multiple FUZZ_TESTs/TestCases): \n")
        previous_build_target_dir = ""
        for test in fuzz_tests:
            build_target_dir = get_build_target_from_fuzztest_path(test)
            if build_target_dir != previous_build_target_dir:
                is_coverage_build = "-coverage" in build_target_dir
                if is_coverage_build:
                    logging.info("\n----------- Coverage-instrumented FuzzTests -----------\n")
                else:
                    logging.info("\n----------- FuzzTests without coverage-instrumentation -----------\n")
                previous_build_target_dir = build_target_dir
            print(f"   {test}")
        print("\n")
        sys.exit(0)

    # initialise fuzz test context
    context = FuzzTestContext(
        fuzz_test_binary_path=fuzz_test,
        fuzz_test_binary_name=fuzz_test.split(os.sep)[-1],
        build_target_name=get_build_target_from_fuzztest_path(fuzz_test),
        is_coverage_instrumented="-coverage" in get_build_target_from_fuzztest_path(fuzz_test)
    )

    if not context.is_coverage_instrumented:
        logging.error(
            f"\nFuzzTest Not coverage instrumented: No coverage report will be generated for: '{context.fuzz_test_binary_path}'\n")
        logging.error("for Coverage reports --> Build with Coverage by appending '-coverage' to target e.g.:\n\n\tpython scripts/build/build_examples.py --target linux-x64-tests-clang-pw-fuzztest-coverage build\n")
        logging.info("Continuing...")

    test_cases = get_fuzz_test_cases(context)

    if (list_test_cases or not test_case) and test_cases:
        logging.info(f"\nList of Testcases (i.e. FUZZ_TESTs) for {context.fuzz_test_binary_name}: \n")
        for case in test_cases:
            print(f"  {case}")
        print("\n")
        if not test_case:
            raise ValueError(
                "Please use for: \n  1. CONTINUOUS_FUZZ_MODE: Use --test-case to choose a specific Testcase, run until interrupted  \n  2. UNIT_TEST_MODE: '--test-case all' to run all Testcases for a few seconds")
        sys.exit(0)

    if not test_cases:
        raise ValueError(f"No FUZZ_TESTs (TestCases) found in {fuzz_test}")

    if test_case.strip().lower() == "all":
        context.run_mode = FuzzTestMode.UNIT_TEST_MODE
    elif test_case in test_cases:
        context.run_mode = FuzzTestMode.CONTINUOUS_FUZZ_MODE
        context.selected_fuzz_test_case = test_case
    else:
        raise ValueError(f"Test case '{test_case}' not found in the list of test cases for {context.fuzz_test_binary_name} ")

    return context


@click.command(add_help_option=False)
@click.option("--fuzz-test", help="Specific FuzzTest binary to run. If not provided, all available FuzzTest binaries in 'out' are listed.")
@click.option("--test-case", help="Specific test case to run in continuous mode OR add '--test-case all' to run all Testcases for a few seconds.")
@click.option("--list-test-cases", is_flag=True, help="List available test cases for the given FuzzTest binary and exit.")
@click.option("--interactive", is_flag=True, help="Run Script in Interactive Mode (automatically lists FuzzTests, TestCases and allows choosing easily).")
@click.option('--help', is_flag=True, help="Show this message and exit.")
@click.option("--output", help="Optional directory for coverage report (auto-generated if not provided).")
def main(fuzz_test, test_case, list_test_cases, interactive, output, help):

    coloredlogs.install(
        level="DEBUG",
        fmt="%(message)s",
        level_styles={
            "debug": {"color": "cyan"},
            "info": {"color": "green"},
            "error": {"color": "red"},
        },
    )

    if help or not fuzz_test:
        logging.info("\nThis Script:")
        logging.info("1. Runs Google FuzzTests in CONTINUOUS_FUZZ_MODE or UNIT_TEST_MODE")
        logging.info("2. Automatically generates HTML Coverage Report if FuzzTest is coverage-instrumented.\n")
        logging.info("WARNING: This Script is designed to work with FuzzTests built using build_examples.py")
        print("=" * 70 + "\n")
        ctx = click.get_current_context()
        click.echo(ctx.get_help())
        logging.info("\nCoverage Report Generation requires: llvm-profdata, llvm-cov, and genhtml (part of lcov package)")
        print("\n" + "=" * 70 + "\n")

    # ==== Run Script in Interactive or non-interactive mode ====
    try:
        if interactive:
            context = run_script_in_interactive_mode()
        else:
            context = run_script_in_normal_mode(fuzz_test, test_case, list_test_cases, help)

    except Exception as e:
        logging.error(e)
        logging.error("\nPlease Try Again.")
        sys.exit(0)

    # ==== Run FuzzTest and Generate Coverage Report ====
    should_generate_coverage = False
    try:
        run_fuzz_test(context)
        # Unit Test Mode
        should_generate_coverage = True
    except KeyboardInterrupt:
        # Continuous Fuzzing Mode Stoppped by User
        should_generate_coverage = True
    except ValueError as e:
        logging.error(e)

    if should_generate_coverage and context.is_coverage_instrumented:
        generate_coverage_report(context, output)
    else:
        logging.info("Skipping coverage report generation")


if __name__ == "__main__":
    main()
