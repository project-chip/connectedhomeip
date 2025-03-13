import argparse
import csv
import glob
import os
import re
import subprocess
from datetime import datetime

# Constants for default values
DEFAULT_TARGETS = [
    "linux-x64-air-purifier-no-ble",
    "linux-x64-air-quality-sensor-no-ble",
    "linux-x64-all-clusters-minimal-no-ble",
    "linux-x64-all-clusters-no-ble-clang-boringssl",
    "linux-x64-bridge-no-ble-clang-boringssl",
    "linux-x64-contact-sensor-no-ble",
    "linux-x64-dishwasher-no-ble",
    "linux-x64-energy-management-no-ble-clang-boringssl",
    "linux-x64-light-data-model-no-unique-id-no-ble",
    "linux-x64-light-no-ble",
    "linux-x64-lit-icd-no-ble",
    "linux-x64-lock-no-ble-clang-boringssl",
    "linux-x64-microwave-oven-no-ble-clang-boringssl",
    "linux-x64-network-manager-ipv6only-no-ble-clang-boringssl",
    "linux-x64-ota-provider-no-ble",
    "linux-x64-ota-provider-no-ble-clang-boringssl",
    "linux-x64-ota-requestor-no-ble",
    "linux-x64-refrigerator-no-ble",
    "linux-x64-rvc-no-ble",
    "linux-x64-rvc-no-ble-clang-boringssl",
    "linux-x64-thermostat-no-ble",
    "linux-x64-tv-app-no-ble-clang-boringssl",
    "linux-x64-tv-casting-app-no-ble",
    "linux-x64-water-leak-detector-no-ble"
]
DEFAULT_TESTS = ["TC_DeviceBasicComposition", "TC_DeviceConformance"]
TMP_RESULTS_DIR = "/tmp/conformance_report"
OUT_DIR = "./out"
TEST_COMMAND = "scripts/run_in_python_env.sh out/python_env './scripts/tests/run_python_test.py --app {} --factory-reset --app-args \"--trace-to json:log\" --script src/python_testing/{}.py --script-args \"--qr-code MT:-24J0AFN00KA0648G00\"'"
BUILD_COMMAND = "python3 scripts/build/build_examples.py --ninja-jobs {} --target {} build"
NINJA_JOBS = max(os.cpu_count() - 2, 1)  # Limit # of jobs to avoid using too much CPU and RAM


def find_executables(dirs):
    """
    Look for the first executable file in a list of directories.

    dirs is a list of directories for each sample app built. We assume there's only
    a single executable in each directory, so once we find one we put it on the list to return.

    This is just to avoid maintaining a list with the names of all executables for each target.
    For example, for a list of directories:

    dirs = [out/linux-x64-lock-no-ble-clang-boringssl,
                out/linux-x64-network-manager-ipv6only-no-ble-clang-boringssl]

    find_executables(dirs) will return:

    [out/linux-x64-lock-no-ble-clang-boringssl/chip-lock-app,
    out/linux-x64-network-manager-ipv6only-no-ble-clang-boringssl/matter-network-manager-app]

    Args:
      dirs: A list of directories to search.

    Returns:
      A list of paths to the first executable found in each directory.
    """
    executables = []
    for dir in dirs:
        if not os.path.isdir(dir):
            continue
        for filename in os.listdir(dir):
            filepath = os.path.join(dir, filename)
            if os.path.isfile(filepath) and os.access(filepath, os.X_OK):
                executables.append(filepath)
                break  # Move to the next directory
    return executables


def parse_test_logs(test_log_paths):
    """
    Analyze test output files and return a dictionary summarizing each test.

    Args:
      test_log_paths: A list of paths to test output files.

    Returns:
      A dictionary where keys are test names and values are lists of results. 
      Each result is a list: [app_target_name, test_pass_fail, error_count, test_cmd, error_summary].
    """

    all_tests_results = {}
    for test_log_path in test_log_paths:
        print(f"  Parsing {test_log_path}")
        try:
            with open(test_log_path, "r") as f:
                output_lines = f.readlines()
        except FileNotFoundError:
            print(f"Result file not found {test_log_path}. Skipping...")
            continue

        app_target_name = ""
        test_pass_fail = ""
        failures = []
        test_name_from_log = ""
        test_cmd = ""

        # Use a for loop with enumerate for easier line processing
        for i, line in enumerate(output_lines):
            if not test_name_from_log and "INFO Executing " in line:
                test_name_from_log = line.split("INFO Executing ")[1].split()[0].split(".")[0]
                all_tests_results.setdefault(test_name_from_log, [])

            if not app_target_name and " --app " in line:
                app_target_name = os.path.basename(line.split(" --app ")[1].split()[0])

            if not test_pass_fail and "Final result: " in line:
                test_pass_fail = line.split("Final result: ")[1].split()[0]

            if not test_cmd and "run_python_test.py" in line:
                test_cmd = "run_python_test.py " + line.split("run_python_test.py")[1]

            if "Problem: ProblemSeverity.ERROR" in line:
                try:
                    error_details = "\n".join(
                        ["  " + re.sub(r"^\[.*?\]\[.*?\]\[.*?\]", "", error_line).strip() for error_line in output_lines[i:i+8]]
                    )
                    failures.append(error_details + "\n")
                except IndexError:
                    print("IndexError: End of file reached unexpectedly.")
                    break

        if not all([app_target_name, test_pass_fail, test_name_from_log]):
            print("Invalid test output file, couldn't parse it. Skipping...")
            continue

        if test_pass_fail == "FAIL" and not failures:
            last_log_lines = output_lines[-100:] if len(output_lines) > 100 else output_lines
            failures.append("Test didn't complete. Possible timeout or crash. Last log lines:")
            failures.append("\n".join(last_log_lines))
            test_pass_fail = "INVALID"

        print(f"\t{app_target_name}\t{test_pass_fail}\t{len(failures)} errors.")
        all_tests_results[test_name_from_log].append(
            [app_target_name, test_pass_fail, len(failures), test_cmd, "\n".join(failures)]
        )

    return all_tests_results


def run_tests(tests, executable_paths, tmp_results_dir, skip_testing):
    """
    Runs tests against executable files.

    Args:
      tests: A list of test names to run.
      executable_paths: A list of paths to executable files.
      tmp_results_dir: Directory to store test output.
      skip_testing: Flag to skip test execution.
    """
    for test_name in tests:
        for executable_path in executable_paths:
            app_name = os.path.basename(executable_path)
            if skip_testing:
                print(f"Testing {app_name} ...skipped")
                continue

            print(f"Testing {app_name} against {test_name}...")
            try:
                command = TEST_COMMAND.format(executable_path, test_name)
                test_output_path = os.path.join(tmp_results_dir, f"{test_name}_{app_name}.txt")
                with open(test_output_path, "wb") as f:
                    result = subprocess.run(command, shell=True, capture_output=False,
                                            text=False, stdout=f, stderr=subprocess.STDOUT)
                    result.check_returncode()  # Raise an exception if the command returned a non-zero exit code
                    print(f"    - Test PASSED. Logs written to {test_output_path}")
            except subprocess.CalledProcessError as e:
                print(f"    - Test FAILED. Logs written to {test_output_path}: {e}")
            except Exception as e:
                print(f"Error running test for {app_name}: {e}")


def build_targets(targets, skip_building):
    """
    Builds targets using the specified build command.

    Args:
      targets: A list of targets to build.
      skip_building: Flag to skip building.
    """
    for target in targets:
        if skip_building:
            print(f"Building: {target} ...skipped")
            continue

        command = BUILD_COMMAND.format(NINJA_JOBS, target)
        try:
            print(f"Building: {target}")
            print(command)
            result = subprocess.run(command, shell=True, capture_output=False, text=True)
            result.check_returncode()  # Raise CalledProcessError if build fails
        except subprocess.CalledProcessError as e:
            print(f"Error building {target}:")
            print(f"  Return code: {e.returncode}")
            print(f"  stdout: {e.stdout}")
            print(f"  stderr: {e.stderr}")
        except Exception as e:
            print(f"Error building {target}: {e}")


def generate_csv_summaries(all_tests_results_dict, out_dir):
    """
    Generates CSV summaries of test results.

    Args:
      all_tests_results_dict: Dictionary of test results.
      out_dir: Directory to save CSV files.
    """
    for test_name, test_results in all_tests_results_dict.items():
        csv_filename = os.path.join(out_dir, f"{test_name}_summary.csv")
        with open(csv_filename, 'w', newline='') as f:
            writer = csv.writer(f)
            test_passed_count = sum(1 for result in test_results if result[1] == "PASS")
            writer.writerow([f"{test_name} ({test_passed_count} / {len(test_results)}) examples passed"])
            writer.writerow(["Application", "Result", "Errors", "Test Command", "Error Summary"])
            writer.writerows(test_results)

        print("CSV test summary saved to " + csv_filename)


def csv_to_html_report(csv_file_paths, html_page_title, html_out_dir, sha):
    """
    Generates an HTML report from CSV files.

    Args:
      csv_file_paths: List of paths to CSV files.
      html_page_title: Title of the HTML report.
      html_out_dir: Directory to save the HTML report.
      sha: SHA commit hash for the report.
    """
    now = datetime.now().strftime("%d/%m/%Y %H:%M:%S")
    html_report = f"""
    <!DOCTYPE html>
    <html>
    <head>
    <style>
    h1 {{
      font-family: Tahoma, Geneva, sans-serif;
      font-size: 32px; color: #333;
      text-align: center;
    }}
    h2 {{
      font-family: Tahoma, Geneva, sans-serif;
      font-size: 22px; color: #333;
      text-align: center;
    }}
    h4 {{
      font-family: Tahoma, Geneva, sans-serif;
      font-size: 14px; color: #333;
      text-align: left;
    }}
    table {{
      border-collapse: collapse;
      font-family: Tahoma, Geneva, sans-serif;
      margin-left: auto;
      margin-right: auto;
      width: 80%;
    }}
    table td {{
      padding: 15px;
    }}
    td[value="FAIL"] {{
      color: red;
    }}
    td[value="PASS"] {{
      color: green;
    }}
    th {{
      background-color: #54585d;
      color: #ffffff;
      font-weight: bold;
      font-size: 15px;
      border: 1px solid #54585d;
    }}
    table tbody td {{
      color: #636363;
      border: 1px solid #dddfe1;
    }}
    table tbody tr {{
      background-color: #f9fafb;
    }}
    table tbody tr:nth-child(odd) {{
      background-color: #ffffff;
    }}
    </style>
    <title>{html_page_title}</title>
    </head>
    <h1>{html_page_title}</h1>
    <hr>
    <h4>Generated on {now}<br>SHA: {sha}</h4> 
    <hr>
    <body>
    """

    for csv_file_path in csv_file_paths:
        with open(csv_file_path, 'r') as csv_file:
            reader = csv.reader(csv_file)
            table_title = next(reader)[0]
            headers = next(reader)
            data = list(reader)

        html_table = f"<h2>{table_title}</h2><table>"
        html_table += "<tr>" + "".join(f"<th>{header}</th>" for header in headers) + "</tr>"
        for row in data:
            html_table += "<tr>"
            for cell in row:
                if len(cell) > 100:
                    html_table += "<td><details><summary>Show/Hide</summary>" + cell.replace('\n', '<br>') + "</details></td>"
                elif cell in ("PASS", "FAIL"):
                    html_table += f"<td value='{cell}'>{cell}</td>"
                else:
                    html_table += "<td>" + cell.replace('\n', '<br>') + "</td>"
            html_table += "</tr>"
        html_table += "</table>"
        html_report += html_table

    html_report += """
    </body>
    </html>
    """

    html_file = os.path.join(html_out_dir, "conformance_report.html")
    print("Saving HTML report to " + html_file)
    with open(html_file, "w") as f:
        f.write(html_report)


def get_git_revision_hash() -> str:
    return subprocess.check_output(['git', 'rev-parse', 'HEAD']).decode('ascii').strip()


if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        description="Build examples, run conformance tests and generate a report with the results."
    )
    parser.add_argument(
        "--test-name",
        help="Override the default tests with a specific test name."
    )
    parser.add_argument(
        "--target-name",
        help="Override the default targets with a specific target name."
    )
    parser.add_argument(
        "--skip-building",
        help="Skip building the target(s).",
        action="store_true"
    )
    parser.add_argument(
        "--skip-testing",
        help="Skip testing the target(s).",
        action="store_true"
    )
    parser.add_argument(
        "--skip-html",
        help="Skip generating the HTML report.",
        action="store_true"
    )
    parser.add_argument(
        "--html-out-dir",
        help="Specify the directory to save the HTML report.",
        default=TMP_RESULTS_DIR
    )

    args = parser.parse_args()

    targets = [args.target_name] if args.target_name else DEFAULT_TARGETS
    target_out_dirs = [os.path.join(OUT_DIR, target) for target in targets]
    tests = [args.test_name] if args.test_name else DEFAULT_TESTS

    os.makedirs(TMP_RESULTS_DIR, exist_ok=True)

    build_targets(targets, args.skip_building)
    executable_paths = find_executables(target_out_dirs)
    run_tests(tests, executable_paths, TMP_RESULTS_DIR, args.skip_testing)

    print(f"Parsing all test output logs in {TMP_RESULTS_DIR}...")
    test_logs = glob.glob(os.path.join(TMP_RESULTS_DIR, "*.txt"))
    aggregated_results_dict = parse_test_logs(test_logs)
    generate_csv_summaries(aggregated_results_dict, TMP_RESULTS_DIR)
    csv_summaries_paths = glob.glob(os.path.join(TMP_RESULTS_DIR, "*.csv"))

    if not args.skip_html:
        os.makedirs(args.html_out_dir, exist_ok=True)
        csv_to_html_report(
            csv_summaries_paths,
            "Matter SDK Example Conformance Report",
            args.html_out_dir,
            get_git_revision_hash()
        )
