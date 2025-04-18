import enum
import logging
import os
import re
import subprocess
from dataclasses import dataclass
from typing import List, Optional

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


def extract_build_target_name(binary_path):
    """Extract build directory name (target name) from the path by finding directory right above 'chip_pw_fuzztest' """
    path_directories = binary_path.split(os.sep)
    build_target_name = ""

    for i, directory in enumerate(path_directories):
        if directory == "chip_pw_fuzztest" and i > 0:
            build_target_name = path_directories[i-1]
            break

    return build_target_name


def list_fuzz_test_binaries():
    """Lists all compiled fuzz tests in the 'out' directory"""
    build_dir = "out"
    if not os.path.isdir(build_dir):
        logging.error(f"Error: Build directory '{build_dir}' does not exist.")
        return []

    fuzz_tests = []
    for root, _, files in os.walk(build_dir):
        for file in files:
            if file.startswith("fuzz-") and os.access(os.path.join(root, file), os.X_OK) and "chip_pw_fuzztest" in root.split(os.sep):
                fuzz_tests.append(os.path.join(root, file))
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
            logging.info("No test cases found.")
            return
    except Exception as e:
        logging.error(f"Error executing {context.fuzz_test_binary_path}: {e}")
        return


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
        profraw_file = f"{build_profile_folder}/{context.coverage_output_base_name}.profraw"
        env["LLVM_PROFILE_FILE"] = profraw_file

    try:
        if context.run_mode == FuzzTestMode.UNIT_TEST_MODE:
            subprocess.run([context.fuzz_test_binary_path, ], env=env, check=True)
            logging.info("Fuzz Test Suite executed in Unit Test Mode.")

        elif context.run_mode == FuzzTestMode.CONTINUOUS_FUZZ_MODE:
            subprocess.run([context.fuzz_test_binary_path, f"--fuzz={context.selected_fuzz_test_case}"], env=env, check=True)

    # in FuzzTestMode.CONTINUOUS_FUZZ_MODE, the fuzzing will run indefinitely until stopped by the user
    except KeyboardInterrupt:
        logging.info("\n===============\nContinuous-Mode Fuzzing Stopped")

    except Exception as e:
        logging.error(f"Error running fuzz test: {e}")


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
        "inconsistent", "source"
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

    logging.info(f"Generating Coverage Report into PATH: {coverage_subfolder}/index.html\n...Please wait...")
    try:
        subprocess.run(cmd, check=True, stdout=subprocess.DEVNULL)
    except FileNotFoundError:
        logging.error("genhtml not found. Please install lcov to generate the HTML coverage report")
        return

    logging.info("Coverage report Generated.")


@click.command(add_help_option=False)
@click.option("--fuzz-test", help="Specific FuzzTest binary to run. If not provided, all available FuzzTest binaries in 'out' are listed.")
@click.option("--test-case", help="Optional: Specific test case to run in continuous mode.")
@click.option("--list-test-cases", is_flag=True, help="List available test cases for the given FuzzTest binary and exit.")
@click.option("--interactive", is_flag=True, help="Run Script in Interactive Mode (automatically lists FuzzTests, TestCases and allows choosing easily).")
@click.option('--help', is_flag=True, help="Show this message and exit.")
@click.option("--output", help="Optional directory for coverage report (auto-generated if not provided).")
def main(fuzz_test, test_case, list_test_cases, interactive, output, help):

    coloredlogs.install(
        level="INFO",
        fmt="%(message)s",
        level_styles={
            "debug": {"color": "cyan"},
            "info": {"color": "green"},
            "error": {"color": "red"},
        },
    )

    logging.info("\nThis Script:")
    logging.info("1. Runs Google FuzzTests in CONTINUOUS_FUZZ_MODE or UNIT_TEST_MODE")
    logging.info("2. Automatically generates HTML Coverage Report if FuzzTest is coverage-instrumented.\n")
    logging.info("WARNING: This Script is designed to work with FuzzTests built using build_examples.py")

    print("=" * 70 + "\n")

    if help or not fuzz_test:
        ctx = click.get_current_context()
        click.echo(ctx.get_help())
        logging.info("\nCoverage Report Generation requires: llvm-profdata, llvm-cov, and genhtml (part of lcov package)")

        print("\n" + "=" * 70 + "\n")

    fuzz_tests = list_fuzz_test_binaries()

    if not fuzz_tests:
        logging.error("No pigweed-based FuzzTests found in the 'out' directory.\n")
        logging.info("FuzzTests can be built using build_examples.py, for example:\n\n\tpython scripts/build/build_examples.py --target linux-x64-tests-clang-pw-fuzztest-coverage build \n")
        return

    if help or not fuzz_test:
        logging.info("\nAVAILABLE FUZZTEST BINARIES in 'out' directory (each Binary can have multiple FUZZ_TESTs/TestCases): \n")
        previous_build_target_dir = ""
        for test in fuzz_tests:
            build_target_dir = extract_build_target_name(test)
            if (build_target_dir != previous_build_target_dir):
                is_coverage_build = "-coverage" in build_target_dir
                if is_coverage_build:
                    logging.info("\n----------- Coverage-instrumented FuzzTests -----------\n")
                else:
                    logging.info("\n----------- FuzzTests without coverage-instrumentation -----------\n")
                previous_build_target_dir = build_target_dir
            print(f"   {test}")
        print("\n")
        return

    # --fuzz-test was passed to script
    if fuzz_test not in fuzz_tests:
        raise ValueError(f"Error: Fuzz test '{fuzz_test}' not found in 'out' directory ")

    context = FuzzTestContext(
        fuzz_test_binary_path=fuzz_test,
        fuzz_test_binary_name=fuzz_test.split(os.sep)[-1],
        build_target_name=extract_build_target_name(fuzz_test),
        is_coverage_instrumented="-coverage" in extract_build_target_name(fuzz_test)
    )

    if not context.is_coverage_instrumented:
        logging.error(
            f"\nFuzzTest Not coverage instrumented: No coverage report will be generated for: '{context.fuzz_test_binary_path}'\n")
        logging.error("for Coverage reports --> Build with Coverage by appending '-coverage' to target e.g.:\n\n\tpython scripts/build/build_examples.py --target linux-x64-tests-clang-pw-fuzztest-coverage build\n")

    test_cases = get_fuzz_test_cases(context)

    if list_test_cases and test_cases:
        logging.info(f"List of Test Cases for {context.fuzz_test_binary_name}: \n")
        for case in test_cases:
            print(case)
        print("\n")
        return

    if not test_cases:
        logging.info(f"No FUZZ_TESTs (TestCases) found in {fuzz_test}")
        return

    if not interactive and not test_case:
        logging.error("Please either choose TestCase to run or choose 'all' for unit test mode")
        return
    elif not interactive and test_case:
        if "all" in test_case:
            context.run_mode = FuzzTestMode.UNIT_TEST_MODE
        else:
            context.run_mode = FuzzTestMode.CONTINUOUS_FUZZ_MODE
            context.selected_fuzz_test_case = test_case
    else:
        print("=" * 70 + "\n")
        logging.info("AVAILABLE TEST CASES --> Choose a number to run in 'Continuous Mode', continues until interrupted:\n")
        for i, case in enumerate(test_cases, start=1):
            logging.info(f"\t{i}. {case}")
        logging.info("\nUNIT_TEST_MODE: Enter 0 to run all test cases in Unit Test Mode (just a few seconds of each FUZZ_TEST(testcase)\n")

        choice = click.prompt("Enter the number of the test case to run", type=int)
        if 1 <= choice <= len(test_cases):

            context.run_mode = FuzzTestMode.CONTINUOUS_FUZZ_MODE
            context.selected_fuzz_test_case = test_cases[choice - 1]

            # Use the FuzzTest (Test Case) Name  as the name for coverage output
          #  context.coverage_output_base_name = "{}".format(context.selected_fuzz_test_case.replace('.', "_"))

        elif choice == 0:

            context.run_mode = FuzzTestMode.UNIT_TEST_MODE

            # Use the FuzzTest Suite Name as the name for coverage output
      #      context.coverage_output_base_name = f"{test_cases[0].split('.')[0]}"
        else:
            logging.info("Invalid choice")
            return

    try:
        run_fuzz_test(context)
    finally:
        if context.is_coverage_instrumented:
            generate_coverage_report(context, output)
        else:
            logging.info("Skipping coverage report generation for non-instrumented build")


if __name__ == "__main__":
    main()
