import argparse
import csv
import glob
import html
import json
import os
import re
import subprocess
from datetime import datetime

# Constants for default values
DEFAULT_TARGET = "linux-x64-all-devices-no-ble-clang-boringssl"
DEFAULT_DEVICES = [
    "aggregator",
    "air-purifier",
    "air-quality-sensor",
    "ambient-context-sensor",
    "battery-storage",
    "bridged-node",
    "chime",
    "closure",
    "color-temperature-light",
    "commissioning-proxy",
    "contact-sensor",
    "cooktop",
    "device-energy-management",
    "dimmable-light",
    "dimmable-plug-in-unit",
    "dishwasher",
    "doorbell",
    "electrical-sensor",
    "extended-color-light",
    "extractor-hood",
    "fan",
    "flow-sensor",
    "generic-switch",
    "humidity-conditioner",
    "humidity-sensor",
    "irrigation-system",
    "laundry-dryer",
    "laundry-washer",
    "light-sensor",
    "microwave-oven",
    "mode-select",
    "mounted-dimmable-load-control",
    "mounted-on-off-control",
    "network-infrastructure-manager",
    "occupancy-sensor",
    "on-off-light",
    "on-off-light-switch",
    "on-off-plug-in-unit",
    "oven",
    "power-source",
    "pressure-sensor",
    "proximity-ranger",
    "rain-sensor",
    "refrigerator",
    "robotic-vacuum-cleaner",
    "room-air-conditioner",
    "smoke-co-alarm",
    "soil-sensor",
    "speaker",
    "temperature-sensor",
    "thermostat",
    "thread-border-router",
    "water-freeze-detector",
    "water-heater",
    "water-leak-detector",
    "water-valve",
    "window-covering",
]

# Custom --device CLI arguments for device types that require parent/composite topology
# or are instantiated as part of a composite/root endpoint.
DEVICE_APP_ARGS: dict[str, str] = {
    "bridged-node": "--device aggregator:1 --device bridged-node:2,parent=1",
    "closure-panel": "--device closure",
    "cook-surface": "--device cooktop",
    "humidifier-dehumidifier": "--device humidity-conditioner",
    "ota-requestor": "--device aggregator",
    "root-node": "--device aggregator",
    "temperature-controlled-cabinet": "--device oven",
}

DEFAULT_TESTS = ["TC_DeviceBasicComposition", "TC_DeviceConformance"]
DEFAULT_PYTHON_ENV = "out/python_env"
TMP_RESULTS_DIR = "/tmp/all_devices_conformance_report"
OUT_DIR = "./out"
TEST_COMMAND = "scripts/run_in_python_env.sh {} './scripts/tests/run_python_test.py --app {} --factory-reset --app-args \"{} --trace-to json:log\" --script src/python_testing/{}.py --script-args \"--qr-code MT:-24J0AFN00KA0648G00 --bool-arg ignore_in_progress_test_event_only_disallowed_for_certification:True allow_provisional_test_event_only_disallowed_for_certification:True\"'"
BUILD_COMMAND = "python3 scripts/build/build_examples.py --ninja-jobs {} --target {} build"
NINJA_JOBS = max((os.cpu_count() or 1) - 2, 1)  # Limit # of jobs to avoid using too much CPU and RAM


def find_executable(directory: str) -> str | None:
    """
    Look for the first executable file in the target output directory.

    Args:
      directory: The target output directory to search.

    Returns:
      Path to the first executable found in the directory, or None if not found.
    """
    if not os.path.isdir(directory):
        return None
    for filename in os.listdir(directory):
        filepath = os.path.join(directory, filename)
        if os.path.isfile(filepath) and os.access(filepath, os.X_OK):
            return filepath
    return None


def parse_test_logs(test_log_paths: list[str]) -> dict[str, list[list[str | int]]]:
    """
    Analyze test output files and return a dictionary summarizing each test.

    Args:
      test_log_paths: A list of paths to test output files.

    Returns:
      A dictionary where keys are test names and values are lists of results.
      Each result is a list: [device_type, test_pass_fail, error_count, test_cmd, error_summary].
    """
    all_tests_results: dict[str, list[list[str | int]]] = {}
    for test_log_path in sorted(test_log_paths):
        print(f"  Parsing {test_log_path}")
        try:
            with open(test_log_path, errors="replace") as f:
                output_lines = f.readlines()
        except FileNotFoundError:
            print(f"Result file not found {test_log_path}. Skipping...")
            continue

        device_type = ""
        test_pass_fail = ""
        failures = []
        test_name_from_log = ""
        test_cmd = ""

        # Use a for loop with enumerate for easier line processing
        for i, line in enumerate(output_lines):
            if not test_name_from_log and "INFO Executing " in line:
                test_name_from_log = line.split("INFO Executing ")[1].split()[0].strip("'\"").split(".")[0]
                all_tests_results.setdefault(test_name_from_log, [])
                basename = os.path.splitext(os.path.basename(test_log_path))[0]
                prefix = f"{test_name_from_log}_"
                if basename.startswith(prefix):
                    device_type = basename[len(prefix):]

            if not device_type and "--device " in line:
                matches = re.findall(r"--device\s+([^\s\"':,]+)", line)
                if matches:
                    device_type = matches[-1]

            if "Final result: " in line:
                test_pass_fail = line.split("Final result: ")[1].split()[0]
            elif not test_pass_fail and "Test results: " in line:
                match = re.search(
                    r"Test results:\s*Error\s+(\d+),\s*Executed\s+(\d+),\s*Failed\s+(\d+),\s*Passed\s+(\d+)",
                    line,
                )
                if match:
                    errors, _, failed, passed = (int(g) for g in match.groups())
                    test_pass_fail = "PASS" if (errors == 0 and failed == 0 and passed > 0) else "FAIL"

            if not test_cmd and "run_python_test.py" in line:
                test_cmd = "run_python_test.py " + line.split("run_python_test.py")[1].strip()

            if "Problem: ProblemSeverity.ERROR" in line or "Problem: ERROR" in line:
                try:
                    error_details = "\n".join(
                        ["  " + re.sub(r"^\[.*?\]\[.*?\]\[.*?\]", "", error_line).strip() for error_line in output_lines[i:i+8]]
                    )
                    failures.append(error_details + "\n")
                except IndexError:
                    print("IndexError: End of file reached unexpectedly.")
                    break

        if not all([device_type, test_pass_fail, test_name_from_log]):
            print("Invalid test output file, couldn't parse it. Skipping...")
            continue

        if test_pass_fail == "FAIL" and not failures:
            last_log_lines = output_lines[-100:] if len(output_lines) > 100 else output_lines
            failures.append("Test didn't complete. Possible timeout or crash. Last log lines:")
            failures.append("\n".join(last_log_lines))
            test_pass_fail = "INVALID"

        print(f"\t{device_type}\t{test_pass_fail}\t{len(failures)} errors.")
        all_tests_results[test_name_from_log].append(
            [device_type, test_pass_fail, len(failures), test_cmd, "\n".join(failures)]
        )

    return all_tests_results


def run_tests(
    tests: list[str],
    executable_path: str | None,
    devices: list[str],
    tmp_results_dir: str,
    skip_testing: bool,
    python_env: str = DEFAULT_PYTHON_ENV,
) -> None:
    """
    Runs tests against all-devices-app for each specified device type.

    Args:
      tests: A list of test names to run.
      executable_path: Path to the all-devices-app executable file.
      devices: A list of device types to pass via --device.
      tmp_results_dir: Directory to store test output.
      skip_testing: Flag to skip test execution.
      python_env: Path to the Python virtual environment directory.
    """
    for test_name in tests:
        for device_type in devices:
            if skip_testing:
                print(f"Testing {device_type} ...skipped")
                continue

            if not executable_path:
                print(f"Executable not found, cannot test {device_type}.")
                continue

            print(f"Testing {device_type} against {test_name}...")
            try:
                device_args = DEVICE_APP_ARGS.get(device_type, f"--device {device_type}")
                command = TEST_COMMAND.format(python_env, executable_path, device_args, test_name)
                test_output_path = os.path.join(tmp_results_dir, f"{test_name}_{device_type}.txt")
                with open(test_output_path, "wb") as f:
                    result = subprocess.run(command, shell=True, capture_output=False,
                                            text=False, stdout=f, stderr=subprocess.STDOUT)
                    result.check_returncode()  # Raise an exception if the command returned a non-zero exit code
                    print(f"    - Test PASSED. Logs written to {test_output_path}")
            except subprocess.CalledProcessError as e:
                print(f"    - Test FAILED. Logs written to {test_output_path}: {e}")
            except Exception as e:
                print(f"Error running test for {device_type}: {e}")


def build_target(target: str, skip_building: bool) -> None:
    """
    Builds the target using the specified build command.

    Args:
      target: Target name to build.
      skip_building: Flag to skip building.
    """
    if skip_building:
        print(f"Building: {target} ...skipped")
        return

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


def generate_csv_summaries(all_tests_results_dict: dict[str, list[list[str | int]]], out_dir: str) -> None:
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
            writer.writerow([f"{test_name} ({test_passed_count} / {len(test_results)}) devices passed"])
            writer.writerow(["Device Type", "Result", "Errors", "Test Command", "Error Summary"])
            writer.writerows(test_results)

        print("CSV test summary saved to " + csv_filename)


def _parse_error_blocks(raw_error: str) -> list[dict[str, object]]:
    """Parses raw error summary text into structured blocks for HTML rendering."""
    text = raw_error.strip()
    if not text:
        return []

    if "Problem: ERROR" not in text and "Problem: ProblemSeverity.ERROR" not in text:
        return [{"kind": "raw", "text": text}]

    chunks = re.split(r"(?=(?:^|\n)\s*Problem:\s*(?:ProblemSeverity\.)?ERROR\b)", text)
    parsed_blocks: list[dict[str, object]] = []
    for chunk in chunks:
        chunk = chunk.strip()
        if not chunk:
            continue
        fields: dict[str, str] = {}
        location_tags: list[str] = []
        extra_lines: list[str] = []
        for raw_line in chunk.splitlines():
            line = raw_line.strip()
            if not line or line.startswith("Problem:"):
                continue
            if ":" in line:
                key, val = line.split(":", 1)
                key_clean = key.strip()
                val_clean = val.strip().rstrip(",")
                if key_clean in ("location", "spec_location"):
                    if val_clean:
                        location_tags.append(f"{key_clean}: {val_clean}")
                elif key_clean in ("test_name", "problem"):
                    fields[key_clean] = val_clean
                elif key_clean in ("Endpoint", "DeviceType", "Cluster", "ClusterID", "Attribute", "Command"):
                    if val_clean:
                        location_tags.append(f"{key_clean}: {val_clean}")
                else:
                    extra_lines.append(line)
            else:
                extra_lines.append(line)

        if not fields and not location_tags:
            parsed_blocks.append({"kind": "raw", "text": chunk})
        else:
            parsed_blocks.append({
                "kind": "structured",
                "test_name": fields.get("test_name", "Error"),
                "problem": fields.get("problem", "\n".join(extra_lines)),
                "tags": location_tags,
            })

    return parsed_blocks


def _render_error_details_html(raw_error: str, row_result: str) -> str:
    """Renders structured, collapsible error details for a table row."""
    blocks = _parse_error_blocks(raw_error)
    if not blocks:
        return '<span class="muted-label">No issues detected</span>'

    first = blocks[0]
    if first["kind"] == "structured":
        first_test = html.escape(str(first["test_name"]))
        first_problem = html.escape(str(first["problem"]))
        tags = first.get("tags", [])
        tag_hint = ""
        for t in tags:
            if t.startswith("Cluster:") or t.startswith("ClusterID:") or t.startswith("DeviceType:"):
                tag_hint = html.escape(t.split(":", 1)[1].strip())
                break
        preview_core = f"<strong>{first_test}</strong>"
        if tag_hint:
            preview_core += f" · <code>{tag_hint}</code>"
        preview_core += f" — {first_problem}"
    else:
        first_line = str(first["text"]).splitlines()[0]
        preview_core = html.escape(first_line)

    extra_count_badge = ""
    if len(blocks) > 1:
        extra_count_badge = f'<span class="more-badge">+{len(blocks) - 1} more</span>'

    cards_html: list[str] = []
    for idx, block in enumerate(blocks, start=1):
        if block["kind"] == "raw":
            cards_html.append(
                f'<div class="error-card error-card--raw">'
                f'<pre class="raw-log">{html.escape(str(block["text"]))}</pre>'
                f'</div>'
            )
            continue

        tags_html = "".join(
            f'<span class="meta-tag">{html.escape(t)}</span>'
            for t in block.get("tags", [])
        )
        cards_html.append(
            f'<div class="error-card">'
            f'  <div class="error-card__head">'
            f'    <span class="error-index">#{idx}</span>'
            f'    <span class="error-rule">{html.escape(str(block["test_name"]))}</span>'
            f'    <div class="error-tags">{tags_html}</div>'
            f'  </div>'
            f'  <div class="error-card__body">{html.escape(str(block["problem"]))}</div>'
            f'</div>'
        )

    open_attr = " open" if row_result in ("FAIL", "INVALID") and len(blocks) <= 2 else ""
    return (
        f'<details class="error-details"{open_attr}>'
        f'  <summary class="error-summary">'
        f'    <span class="chevron" aria-hidden="true"></span>'
        f'    <span class="error-preview">{preview_core}</span>'
        f'    {extra_count_badge}'
        f'  </summary>'
        f'  <div class="error-cards">{"".join(cards_html)}</div>'
        f'</details>'
    )


def csv_to_html_report(csv_file_paths: list[str], html_page_title: str, html_out_dir: str, sha: str) -> None:
    """
    Generates a modern, interactive HTML report from CSV files.

    Args:
      csv_file_paths: List of paths to CSV files.
      html_page_title: Title of the HTML report.
      html_out_dir: Directory to save the HTML report.
      sha: SHA commit hash for the report.
    """
    now = datetime.now().strftime("%Y-%m-%d %H:%M:%S")

    suites: list[dict[str, object]] = []
    for csv_file_path in sorted(csv_file_paths):
        with open(csv_file_path) as csv_file:
            reader = csv.reader(csv_file)
            title_row = next(reader, [""])
            headers = next(reader, [])
            data = list(reader)

        table_title = title_row[0] if title_row else os.path.basename(csv_file_path)
        suite_name = table_title.split(" (")[0].strip() or os.path.splitext(os.path.basename(csv_file_path))[0]

        passed_count = sum(1 for row in data if len(row) > 1 and row[1] == "PASS")
        failed_count = sum(1 for row in data if len(row) > 1 and row[1] == "FAIL")
        invalid_count = sum(1 for row in data if len(row) > 1 and row[1] == "INVALID")
        total_errors = sum(int(row[2]) for row in data if len(row) > 2 and str(row[2]).isdigit())
        total_count = len(data)
        pass_pct = (passed_count / total_count * 100.0) if total_count else 0.0

        suites.append({
            "name": suite_name,
            "title": table_title,
            "headers": headers,
            "rows": data,
            "total": total_count,
            "passed": passed_count,
            "failed": failed_count,
            "invalid": invalid_count,
            "errors": total_errors,
            "pass_pct": pass_pct,
        })

    # Show suites with failures first so actionable issues appear immediately.
    suites.sort(key=lambda s: (-(int(s["failed"]) + int(s["invalid"])), str(s["name"])))

    total_runs = sum(int(s["total"]) for s in suites)
    total_passed = sum(int(s["passed"]) for s in suites)
    total_failed = sum(int(s["failed"]) for s in suites)
    total_invalid = sum(int(s["invalid"]) for s in suites)
    total_errors_all = sum(int(s["errors"]) for s in suites)
    overall_pct = (total_passed / total_runs * 100.0) if total_runs else 0.0

    unique_devices = sorted({row[0] for s in suites for row in s["rows"] if row})
    short_sha = sha[:12]

    suite_cards_html: list[str] = []
    for idx, suite in enumerate(suites):
        total = int(suite["total"])
        passed = int(suite["passed"])
        failed = int(suite["failed"])
        invalid = int(suite["invalid"])
        errors = int(suite["errors"])
        pass_w = (passed / total * 100.0) if total else 0.0
        fail_w = (failed / total * 100.0) if total else 0.0
        inv_w = (invalid / total * 100.0) if total else 0.0
        active_cls = " is-active" if idx == 0 else ""
        status_cls = "suite-pill--ok" if (failed + invalid) == 0 else "suite-pill--warn"

        suite_cards_html.append(f"""
        <button type="button" class="suite-card{active_cls}" data-suite-tab="{html.escape(str(suite['name']))}">
          <div class="suite-card__top">
            <span class="suite-card__name">{html.escape(str(suite['name']))}</span>
            <span class="suite-pill {status_cls}">{suite['pass_pct']:.1f}% pass</span>
          </div>
          <div class="suite-card__metrics">
            <span class="metric-main">{passed} / {total}</span>
            <span class="metric-sub">devices passed</span>
          </div>
          <div class="progress-track" aria-hidden="true">
            <span class="progress-seg progress-seg--pass" style="width: {pass_w:.2f}%"></span>
            <span class="progress-seg progress-seg--fail" style="width: {fail_w:.2f}%"></span>
            <span class="progress-seg progress-seg--invalid" style="width: {inv_w:.2f}%"></span>
          </div>
          <div class="suite-card__foot">
            <span class="dot-stat"><i class="dot dot--pass"></i>{passed} Pass</span>
            <span class="dot-stat"><i class="dot dot--fail"></i>{failed} Fail</span>
            {"<span class='dot-stat'><i class='dot dot--invalid'></i>" + str(invalid) + " Invalid</span>" if invalid else ""}
            <span class="dot-stat dot-stat--right">{errors} errors</span>
          </div>
        </button>
        """)

    all_rows_html: list[str] = []
    for suite in suites:
        suite_name = str(suite["name"])
        for row_idx, row in enumerate(suite["rows"]):
            device_type = str(row[0]) if len(row) > 0 else ""
            result = str(row[1]) if len(row) > 1 else "UNKNOWN"
            errors_str = str(row[2]) if len(row) > 2 else "0"
            errors_int = int(errors_str) if errors_str.isdigit() else 0
            test_cmd = str(row[3]) if len(row) > 3 else ""
            error_summary = str(row[4]) if len(row) > 4 else ""

            status_rank = {"FAIL": 0, "INVALID": 1, "PASS": 2}.get(result, 3)
            result_lower = result.lower()
            search_blob = f"{suite_name} {device_type} {result} {error_summary}".lower()

            error_cell_html = _render_error_details_html(error_summary, result)
            cmd_escaped = html.escape(test_cmd)
            cmd_attr = html.escape(test_cmd, quote=True)

            all_rows_html.append(
                f'<tr class="report-row row--{html.escape(result_lower)}" '
                f'data-suite="{html.escape(suite_name, quote=True)}" '
                f'data-status="{html.escape(result, quote=True)}" '
                f'data-status-rank="{status_rank}" '
                f'data-device="{html.escape(device_type.lower(), quote=True)}" '
                f'data-errors="{errors_int}" '
                f'data-index="{row_idx}" '
                f'data-search="{html.escape(search_blob, quote=True)}">'
                f'  <td class="col-device">'
                f'    <div class="device-cell">'
                f'      <span class="device-name">{html.escape(device_type)}</span>'
                f'      <span class="suite-tag">{html.escape(suite_name)}</span>'
                f'    </div>'
                f'  </td>'
                f'  <td class="col-status">'
                f'    <span class="status-badge status-badge--{html.escape(result_lower)}">'
                f'      <span class="status-dot"></span>{html.escape(result)}'
                f'    </span>'
                f'  </td>'
                f'  <td class="col-errors">'
                f'    <span class="err-count {"err-count--nonzero" if errors_int > 0 else ""}">{errors_int}</span>'
                f'  </td>'
                f'  <td class="col-details">{error_cell_html}</td>'
                f'  <td class="col-cmd">'
                f'    <div class="cmd-actions">'
                f'      <button type="button" class="btn-copy" data-copy="{cmd_attr}" title="Copy test command">Copy</button>'
                f'      <details class="cmd-details">'
                f'        <summary>Cmd</summary>'
                f'        <pre class="cmd-pre">{cmd_escaped}</pre>'
                f'      </details>'
                f'    </div>'
                f'  </td>'
                f'</tr>'
            )

    default_suite = str(suites[0]["name"]) if suites else "ALL"
    escaped_title = html.escape(html_page_title)

    html_report = f"""<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>{escaped_title}</title>
  <style>
    :root {{
      --bg: #f4f6fb;
      --surface: #ffffff;
      --surface-subtle: #f8fafc;
      --surface-hover: #f1f5f9;
      --border: #e2e8f0;
      --border-strong: #cbd5e1;
      --text: #0f172a;
      --text-secondary: #475569;
      --text-muted: #64748b;
      --accent: #2563eb;
      --accent-soft: rgba(37, 99, 235, 0.10);
      --pass-text: #047857;
      --pass-bg: #ecfdf5;
      --pass-border: #a7f3d0;
      --pass-bar: #10b981;
      --fail-text: #be123c;
      --fail-bg: #fff1f2;
      --fail-border: #fecdd3;
      --fail-bar: #f43f5e;
      --invalid-text: #b45309;
      --invalid-bg: #fffbeb;
      --invalid-border: #fde68a;
      --invalid-bar: #f59e0b;
      --shadow-sm: 0 1px 2px rgba(15, 23, 42, 0.04);
      --shadow-md: 0 8px 24px -6px rgba(15, 23, 42, 0.08);
      --radius-lg: 14px;
      --radius-md: 10px;
      --radius-sm: 6px;
      --font-sans: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, Inter, "Helvetica Neue", Arial, sans-serif;
      --font-mono: ui-monospace, SFMono-Regular, Menlo, Monaco, Consolas, "Liberation Mono", monospace;
    }}

    html[data-theme="dark"] {{
      --bg: #0b1120;
      --surface: #111827;
      --surface-subtle: #161f32;
      --surface-hover: #1e293b;
      --border: #1f2937;
      --border-strong: #334155;
      --text: #f1f5f9;
      --text-secondary: #cbd5e1;
      --text-muted: #94a3b8;
      --accent: #60a5fa;
      --accent-soft: rgba(96, 165, 250, 0.14);
      --pass-text: #34d399;
      --pass-bg: rgba(16, 185, 129, 0.12);
      --pass-border: rgba(16, 185, 129, 0.30);
      --pass-bar: #10b981;
      --fail-text: #fb7185;
      --fail-bg: rgba(244, 63, 94, 0.12);
      --fail-border: rgba(244, 63, 94, 0.30);
      --fail-bar: #f43f5e;
      --invalid-text: #fbbf24;
      --invalid-bg: rgba(245, 158, 11, 0.12);
      --invalid-border: rgba(245, 158, 11, 0.30);
      --invalid-bar: #f59e0b;
      --shadow-sm: 0 1px 2px rgba(0, 0, 0, 0.35);
      --shadow-md: 0 10px 28px -6px rgba(0, 0, 0, 0.55);
    }}

    *, *::before, *::after {{
      box-sizing: border-box;
    }}

    body {{
      margin: 0;
      padding: 0;
      background: var(--bg);
      color: var(--text);
      font-family: var(--font-sans);
      font-size: 14px;
      line-height: 1.5;
      -webkit-font-smoothing: antialiased;
    }}

    .container {{
      max-width: 1400px;
      margin: 0 auto;
      padding: 28px 24px 56px;
    }}

    /* Header */
    .hero {{
      display: flex;
      flex-wrap: wrap;
      align-items: flex-start;
      justify-content: space-between;
      gap: 16px;
      margin-bottom: 24px;
    }}

    .hero__eyebrow {{
      display: inline-flex;
      align-items: center;
      gap: 8px;
      font-size: 12px;
      font-weight: 600;
      letter-spacing: 0.04em;
      text-transform: uppercase;
      color: var(--accent);
      background: var(--accent-soft);
      padding: 4px 10px;
      border-radius: 999px;
      margin-bottom: 10px;
    }}

    .hero h1 {{
      margin: 0 0 8px;
      font-size: 28px;
      font-weight: 700;
      letter-spacing: -0.02em;
      line-height: 1.2;
    }}

    .meta-bar {{
      display: flex;
      flex-wrap: wrap;
      align-items: center;
      gap: 10px;
      color: var(--text-muted);
      font-size: 13px;
    }}

    .meta-chip {{
      display: inline-flex;
      align-items: center;
      gap: 6px;
      background: var(--surface);
      border: 1px solid var(--border);
      border-radius: 999px;
      padding: 4px 11px;
      font-size: 12px;
      color: var(--text-secondary);
      box-shadow: var(--shadow-sm);
    }}

    .meta-chip code {{
      font-family: var(--font-mono);
      font-size: 12px;
      color: var(--text);
    }}

    .hero__actions {{
      display: flex;
      align-items: center;
      gap: 10px;
    }}

    .btn-ghost {{
      display: inline-flex;
      align-items: center;
      gap: 7px;
      background: var(--surface);
      color: var(--text-secondary);
      border: 1px solid var(--border);
      border-radius: var(--radius-md);
      padding: 8px 13px;
      font-family: inherit;
      font-size: 13px;
      font-weight: 500;
      cursor: pointer;
      box-shadow: var(--shadow-sm);
      transition: background 0.15s ease, border-color 0.15s ease, color 0.15s ease;
    }}

    .btn-ghost:hover {{
      background: var(--surface-hover);
      border-color: var(--border-strong);
      color: var(--text);
    }}

    /* Overview KPI Strip */
    .kpi-grid {{
      display: grid;
      grid-template-columns: repeat(auto-fit, minmax(190px, 1fr));
      gap: 14px;
      margin-bottom: 20px;
    }}

    .kpi-card {{
      background: var(--surface);
      border: 1px solid var(--border);
      border-radius: var(--radius-lg);
      padding: 16px 18px;
      box-shadow: var(--shadow-sm);
    }}

    .kpi-card__label {{
      font-size: 12px;
      font-weight: 600;
      color: var(--text-muted);
      text-transform: uppercase;
      letter-spacing: 0.04em;
      margin-bottom: 6px;
    }}

    .kpi-card__value {{
      font-size: 26px;
      font-weight: 700;
      letter-spacing: -0.02em;
      line-height: 1.15;
    }}

    .kpi-card__sub {{
      margin-top: 4px;
      font-size: 12px;
      color: var(--text-muted);
    }}

    .kpi-card--pass .kpi-card__value {{ color: var(--pass-text); }}
    .kpi-card--fail .kpi-card__value {{ color: var(--fail-text); }}

    /* Suite Cards */
    .suites-grid {{
      display: grid;
      grid-template-columns: repeat(auto-fit, minmax(280px, 1fr));
      gap: 14px;
      margin-bottom: 22px;
    }}

    .suite-card {{
      text-align: left;
      width: 100%;
      background: var(--surface);
      border: 1.5px solid var(--border);
      border-radius: var(--radius-lg);
      padding: 16px 18px;
      cursor: pointer;
      color: inherit;
      font-family: inherit;
      box-shadow: var(--shadow-sm);
      transition: border-color 0.15s ease, box-shadow 0.15s ease, transform 0.15s ease;
    }}

    .suite-card:hover {{
      border-color: var(--border-strong);
      box-shadow: var(--shadow-md);
      transform: translateY(-1px);
    }}

    .suite-card.is-active {{
      border-color: var(--accent);
      box-shadow: 0 0 0 3px var(--accent-soft), var(--shadow-sm);
    }}

    .suite-card__top {{
      display: flex;
      align-items: center;
      justify-content: space-between;
      gap: 10px;
      margin-bottom: 10px;
    }}

    .suite-card__name {{
      font-weight: 700;
      font-size: 15px;
      color: var(--text);
      word-break: break-word;
    }}

    .suite-pill {{
      flex-shrink: 0;
      font-size: 11px;
      font-weight: 700;
      padding: 3px 9px;
      border-radius: 999px;
    }}

    .suite-pill--ok {{
      background: var(--pass-bg);
      color: var(--pass-text);
      border: 1px solid var(--pass-border);
    }}

    .suite-pill--warn {{
      background: var(--fail-bg);
      color: var(--fail-text);
      border: 1px solid var(--fail-border);
    }}

    .suite-card__metrics {{
      display: flex;
      align-items: baseline;
      gap: 8px;
      margin-bottom: 10px;
    }}

    .metric-main {{
      font-size: 22px;
      font-weight: 700;
      letter-spacing: -0.02em;
    }}

    .metric-sub {{
      font-size: 13px;
      color: var(--text-muted);
    }}

    .progress-track {{
      display: flex;
      height: 7px;
      border-radius: 999px;
      overflow: hidden;
      background: var(--surface-hover);
      margin-bottom: 10px;
    }}

    .progress-seg--pass {{ background: var(--pass-bar); }}
    .progress-seg--fail {{ background: var(--fail-bar); }}
    .progress-seg--invalid {{ background: var(--invalid-bar); }}

    .suite-card__foot {{
      display: flex;
      flex-wrap: wrap;
      align-items: center;
      gap: 12px;
      font-size: 12px;
      color: var(--text-muted);
    }}

    .dot-stat {{
      display: inline-flex;
      align-items: center;
      gap: 5px;
    }}

    .dot-stat--right {{
      margin-left: auto;
      font-weight: 600;
      color: var(--text-secondary);
    }}

    .dot {{
      width: 8px;
      height: 8px;
      border-radius: 50%;
      display: inline-block;
    }}
    .dot--pass {{ background: var(--pass-bar); }}
    .dot--fail {{ background: var(--fail-bar); }}
    .dot--invalid {{ background: var(--invalid-bar); }}

    /* Filter Toolbar */
    .toolbar {{
      position: sticky;
      top: 10px;
      z-index: 20;
      display: flex;
      flex-wrap: wrap;
      align-items: center;
      justify-content: space-between;
      gap: 12px;
      background: var(--surface);
      border: 1px solid var(--border);
      border-radius: var(--radius-lg);
      padding: 12px 16px;
      margin-bottom: 16px;
      box-shadow: var(--shadow-md);
    }}

    .toolbar__left, .toolbar__right {{
      display: flex;
      flex-wrap: wrap;
      align-items: center;
      gap: 10px;
    }}

    .seg-group {{
      display: inline-flex;
      background: var(--surface-subtle);
      border: 1px solid var(--border);
      border-radius: var(--radius-md);
      padding: 3px;
      gap: 2px;
    }}

    .seg-btn {{
      border: none;
      background: transparent;
      color: var(--text-secondary);
      font-family: inherit;
      font-size: 12.5px;
      font-weight: 600;
      padding: 6px 11px;
      border-radius: 7px;
      cursor: pointer;
      display: inline-flex;
      align-items: center;
      gap: 6px;
      transition: background 0.15s ease, color 0.15s ease;
    }}

    .seg-btn:hover {{
      color: var(--text);
    }}

    .seg-btn.is-active {{
      background: var(--surface);
      color: var(--text);
      box-shadow: var(--shadow-sm);
    }}

    .seg-count {{
      font-size: 11px;
      padding: 1px 6px;
      border-radius: 999px;
      background: var(--surface-hover);
      color: var(--text-muted);
    }}

    .search-wrap {{
      position: relative;
      display: flex;
      align-items: center;
    }}

    .search-icon {{
      position: absolute;
      left: 11px;
      color: var(--text-muted);
      pointer-events: none;
    }}

    .search-input {{
      width: 260px;
      max-width: 100%;
      padding: 7px 12px 7px 34px;
      border: 1px solid var(--border);
      border-radius: var(--radius-md);
      background: var(--surface-subtle);
      color: var(--text);
      font-family: inherit;
      font-size: 13px;
      outline: none;
      transition: border-color 0.15s ease, box-shadow 0.15s ease;
    }}

    .search-input:focus {{
      border-color: var(--accent);
      box-shadow: 0 0 0 3px var(--accent-soft);
      background: var(--surface);
    }}

    .select-sort {{
      padding: 7px 10px;
      border: 1px solid var(--border);
      border-radius: var(--radius-md);
      background: var(--surface-subtle);
      color: var(--text);
      font-family: inherit;
      font-size: 13px;
      cursor: pointer;
      outline: none;
    }}

    .visible-counter {{
      font-size: 12.5px;
      color: var(--text-muted);
      margin-bottom: 10px;
      padding-left: 4px;
    }}

    /* Table Card */
    .table-card {{
      background: var(--surface);
      border: 1px solid var(--border);
      border-radius: var(--radius-lg);
      box-shadow: var(--shadow-sm);
      overflow-x: auto;
    }}

    table {{
      width: 100%;
      border-collapse: collapse;
      text-align: left;
    }}

    thead th {{
      background: var(--surface-subtle);
      color: var(--text-secondary);
      font-size: 12px;
      font-weight: 700;
      text-transform: uppercase;
      letter-spacing: 0.04em;
      padding: 13px 16px;
      border-bottom: 1px solid var(--border);
      white-space: nowrap;
    }}

    tbody td {{
      padding: 14px 16px;
      border-bottom: 1px solid var(--border);
      vertical-align: top;
    }}

    tbody tr:last-child td {{
      border-bottom: none;
    }}

    tbody tr {{
      transition: background 0.12s ease;
    }}

    tbody tr:hover {{
      background: var(--surface-hover);
    }}

    tbody tr.row--fail {{
      box-shadow: inset 3px 0 0 var(--fail-bar);
    }}

    tbody tr.row--invalid {{
      box-shadow: inset 3px 0 0 var(--invalid-bar);
    }}

    .col-device {{ min-width: 210px; }}
    .col-status {{ width: 110px; white-space: nowrap; }}
    .col-errors {{ width: 85px; text-align: center; white-space: nowrap; }}
    .col-details {{ min-width: 320px; }}
    .col-cmd {{ width: 320px; min-width: 260px; }}

    .device-cell {{
      display: flex;
      flex-direction: column;
      gap: 4px;
    }}

    .device-name {{
      font-family: var(--font-mono);
      font-weight: 600;
      font-size: 13.5px;
      color: var(--text);
    }}

    .suite-tag {{
      display: inline-block;
      width: fit-content;
      font-size: 11px;
      color: var(--text-muted);
      background: var(--surface-subtle);
      border: 1px solid var(--border);
      border-radius: 5px;
      padding: 1px 6px;
    }}

    .status-badge {{
      display: inline-flex;
      align-items: center;
      gap: 6px;
      padding: 4px 10px;
      border-radius: 999px;
      font-size: 12px;
      font-weight: 700;
      letter-spacing: 0.02em;
    }}

    .status-dot {{
      width: 7px;
      height: 7px;
      border-radius: 50%;
      background: currentColor;
    }}

    .status-badge--pass {{
      color: var(--pass-text);
      background: var(--pass-bg);
      border: 1px solid var(--pass-border);
    }}

    .status-badge--fail {{
      color: var(--fail-text);
      background: var(--fail-bg);
      border: 1px solid var(--fail-border);
    }}

    .status-badge--invalid {{
      color: var(--invalid-text);
      background: var(--invalid-bg);
      border: 1px solid var(--invalid-border);
    }}

    .err-count {{
      display: inline-flex;
      align-items: center;
      justify-content: center;
      min-width: 26px;
      height: 24px;
      padding: 0 8px;
      border-radius: 999px;
      font-size: 12.5px;
      font-weight: 700;
      color: var(--text-muted);
      background: var(--surface-subtle);
      border: 1px solid var(--border);
    }}

    .err-count--nonzero {{
      color: var(--fail-text);
      background: var(--fail-bg);
      border-color: var(--fail-border);
    }}

    .muted-label {{
      color: var(--text-muted);
      font-size: 13px;
    }}

    /* Error Cards */
    .error-details summary {{
      list-style: none;
      cursor: pointer;
      display: flex;
      align-items: flex-start;
      gap: 8px;
      font-size: 13px;
      color: var(--text-secondary);
      user-select: none;
    }}

    .error-details summary::-webkit-details-marker {{
      display: none;
    }}

    .chevron {{
      flex-shrink: 0;
      margin-top: 5px;
      width: 7px;
      height: 7px;
      border-right: 1.8px solid var(--text-muted);
      border-bottom: 1.8px solid var(--text-muted);
      transform: rotate(-45deg);
      transition: transform 0.15s ease;
    }}

    .error-details[open] .chevron {{
      transform: rotate(45deg);
      margin-top: 3px;
    }}

    .error-preview {{
      flex: 1;
      min-width: 0;
      line-height: 1.45;
    }}

    .error-preview code {{
      font-family: var(--font-mono);
      font-size: 12px;
      background: var(--surface-subtle);
      border: 1px solid var(--border);
      padding: 1px 5px;
      border-radius: 4px;
    }}

    .more-badge {{
      flex-shrink: 0;
      font-size: 11px;
      font-weight: 600;
      padding: 2px 7px;
      border-radius: 999px;
      background: var(--fail-bg);
      color: var(--fail-text);
      border: 1px solid var(--fail-border);
    }}

    .error-cards {{
      display: flex;
      flex-direction: column;
      gap: 8px;
      margin-top: 10px;
    }}

    .error-card {{
      background: var(--surface-subtle);
      border: 1px solid var(--border);
      border-left: 3px solid var(--fail-bar);
      border-radius: var(--radius-sm);
      padding: 10px 12px;
    }}

    .error-card__head {{
      display: flex;
      flex-wrap: wrap;
      align-items: center;
      gap: 6px;
      margin-bottom: 6px;
    }}

    .error-index {{
      font-family: var(--font-mono);
      font-size: 11px;
      font-weight: 700;
      color: var(--text-muted);
    }}

    .error-rule {{
      font-size: 11.5px;
      font-weight: 700;
      padding: 2px 7px;
      border-radius: 5px;
      background: var(--fail-bg);
      color: var(--fail-text);
      border: 1px solid var(--fail-border);
    }}

    .error-tags {{
      display: flex;
      flex-wrap: wrap;
      gap: 5px;
    }}

    .meta-tag {{
      font-family: var(--font-mono);
      font-size: 11px;
      color: var(--text-secondary);
      background: var(--surface);
      border: 1px solid var(--border);
      padding: 1px 6px;
      border-radius: 4px;
    }}

    .error-card__body {{
      font-size: 13px;
      color: var(--text);
      word-break: break-word;
    }}

    .raw-log {{
      margin: 0;
      font-family: var(--font-mono);
      font-size: 12px;
      white-space: pre-wrap;
      word-break: break-word;
      color: var(--text-secondary);
    }}

    /* Command Column */
    .cmd-actions {{
      display: flex;
      flex-direction: column;
      align-items: flex-start;
      gap: 6px;
    }}

    .btn-copy {{
      font-family: inherit;
      font-size: 12px;
      font-weight: 600;
      padding: 4px 10px;
      border-radius: 6px;
      border: 1px solid var(--border);
      background: var(--surface-subtle);
      color: var(--text-secondary);
      cursor: pointer;
      transition: all 0.15s ease;
    }}

    .btn-copy:hover {{
      background: var(--surface-hover);
      color: var(--text);
      border-color: var(--border-strong);
    }}

    .btn-copy.is-copied {{
      color: var(--pass-text);
      background: var(--pass-bg);
      border-color: var(--pass-border);
    }}

    .cmd-details {{
      width: 100%;
    }}

    .cmd-details summary {{
      font-size: 12px;
      color: var(--text-muted);
      cursor: pointer;
      user-select: none;
    }}

    .cmd-pre {{
      margin: 6px 0 0;
      padding: 8px 10px;
      width: 100%;
      white-space: pre-wrap;
      word-break: break-word;
      font-family: var(--font-mono);
      font-size: 11.5px;
      background: var(--surface-subtle);
      border: 1px solid var(--border);
      border-radius: var(--radius-sm);
      color: var(--text-secondary);
    }}

    .empty-state {{
      display: none;
      text-align: center;
      padding: 42px 16px;
      color: var(--text-muted);
      font-size: 14px;
    }}

    .is-hidden {{
      display: none !important;
    }}

    @media (max-width: 768px) {{
      .container {{ padding: 16px 12px 40px; }}
      .hero h1 {{ font-size: 22px; }}
      .search-input {{ width: 100%; }}
      .toolbar {{ position: static; }}
    }}
  </style>
  <script>
    (function() {{
      try {{
        var saved = localStorage.getItem("conformance-report-theme");
        var prefersDark = window.matchMedia && window.matchMedia("(prefers-color-scheme: dark)").matches;
        if (saved === "dark" || (!saved && prefersDark)) {{
          document.documentElement.setAttribute("data-theme", "dark");
        }}
      }} catch (e) {{}}
    }})();
  </script>
</head>
<body>
  <div class="container">
    <header class="hero">
      <div>
        <div class="hero__eyebrow">Matter SDK · all-devices-app</div>
        <h1>{escaped_title}</h1>
        <div class="meta-bar">
          <span class="meta-chip">Generated: <strong>{html.escape(now)}</strong></span>
          <span class="meta-chip">SHA: <code title="{html.escape(sha)}">{html.escape(short_sha)}</code></span>
          <span class="meta-chip">Device types: <strong>{len(unique_devices)}</strong></span>
        </div>
      </div>
      <div class="hero__actions">
        <button type="button" class="btn-ghost" id="toggle-all-errors">Collapse errors</button>
        <button type="button" class="btn-ghost" id="theme-toggle" title="Toggle light/dark theme">Theme</button>
      </div>
    </header>

    <section class="kpi-grid" aria-label="Overall summary">
      <div class="kpi-card">
        <div class="kpi-card__label">Device Configurations</div>
        <div class="kpi-card__value">{len(unique_devices)}</div>
        <div class="kpi-card__sub">{len(suites)} test suites executed ({total_runs} runs)</div>
      </div>
      <div class="kpi-card kpi-card--pass">
        <div class="kpi-card__label">Overall Pass Rate</div>
        <div class="kpi-card__value">{overall_pct:.1f}%</div>
        <div class="kpi-card__sub">{total_passed} passed of {total_runs} runs</div>
      </div>
      <div class="kpi-card kpi-card--fail">
        <div class="kpi-card__label">Failing / Invalid Runs</div>
        <div class="kpi-card__value">{total_failed + total_invalid}</div>
        <div class="kpi-card__sub">{total_failed} fail · {total_invalid} invalid</div>
      </div>
      <div class="kpi-card kpi-card--fail">
        <div class="kpi-card__label">Total Logged Errors</div>
        <div class="kpi-card__value">{total_errors_all}</div>
        <div class="kpi-card__sub">Across all conformance checks</div>
      </div>
    </section>

    <section class="suites-grid" aria-label="Test suites">
      {"".join(suite_cards_html)}
      <button type="button" class="suite-card" data-suite-tab="ALL">
        <div class="suite-card__top">
          <span class="suite-card__name">All Suites Combined</span>
          <span class="suite-pill {"suite-pill--ok" if (total_failed + total_invalid) == 0 else "suite-pill--warn"}">{overall_pct:.1f}% pass</span>
        </div>
        <div class="suite-card__metrics">
          <span class="metric-main">{total_passed} / {total_runs}</span>
          <span class="metric-sub">total runs passed</span>
        </div>
        <div class="progress-track" aria-hidden="true">
          <span class="progress-seg progress-seg--pass" style="width: {overall_pct:.2f}%"></span>
          <span class="progress-seg progress-seg--fail" style="width: {(total_failed / total_runs * 100.0) if total_runs else 0:.2f}%"></span>
        </div>
        <div class="suite-card__foot">
          <span class="dot-stat"><i class="dot dot--pass"></i>{total_passed} Pass</span>
          <span class="dot-stat"><i class="dot dot--fail"></i>{total_failed} Fail</span>
          <span class="dot-stat dot-stat--right">{total_errors_all} errors</span>
        </div>
      </button>
    </section>

    <section class="toolbar" aria-label="Filters">
      <div class="toolbar__left">
        <div class="seg-group" role="group" aria-label="Status filter">
          <button type="button" class="seg-btn is-active" data-status-filter="ALL">All <span class="seg-count" id="cnt-all">0</span></button>
          <button type="button" class="seg-btn" data-status-filter="FAIL">Fail <span class="seg-count" id="cnt-fail">0</span></button>
          <button type="button" class="seg-btn" data-status-filter="PASS">Pass <span class="seg-count" id="cnt-pass">0</span></button>
          <button type="button" class="seg-btn" data-status-filter="INVALID">Invalid <span class="seg-count" id="cnt-invalid">0</span></button>
        </div>
      </div>
      <div class="toolbar__right">
        <div class="search-wrap">
          <svg class="search-icon" width="15" height="15" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round" aria-hidden="true">
            <circle cx="11" cy="11" r="8"></circle>
            <line x1="21" y1="21" x2="16.65" y2="16.65"></line>
          </svg>
          <input type="search" id="search-input" class="search-input" placeholder="Search device, cluster, error..." aria-label="Search report">
        </div>
        <select id="sort-select" class="select-sort" aria-label="Sort rows">
          <option value="fail-first">Sort: Failures first</option>
          <option value="device-asc">Sort: Device A → Z</option>
          <option value="device-desc">Sort: Device Z → A</option>
          <option value="errors-desc">Sort: Most errors</option>
        </select>
      </div>
    </section>

    <div class="visible-counter" id="visible-counter"></div>

    <div class="table-card">
      <table>
        <thead>
          <tr>
            <th>Device Type</th>
            <th>Result</th>
            <th style="text-align: center;">Errors</th>
            <th>Error Summary</th>
            <th>Command</th>
          </tr>
        </thead>
        <tbody id="report-tbody">
          {"".join(all_rows_html)}
        </tbody>
      </table>
      <div class="empty-state" id="empty-state">No matching test results found for the selected filters.</div>
    </div>
  </div>

  <script>
    (function() {{
      const tbody = document.getElementById("report-tbody");
      const rows = Array.from(tbody.querySelectorAll("tr.report-row"));
      const suiteButtons = Array.from(document.querySelectorAll("[data-suite-tab]"));
      const statusButtons = Array.from(document.querySelectorAll("[data-status-filter]"));
      const searchInput = document.getElementById("search-input");
      const sortSelect = document.getElementById("sort-select");
      const emptyState = document.getElementById("empty-state");
      const visibleCounter = document.getElementById("visible-counter");
      const toggleErrorsBtn = document.getElementById("toggle-all-errors");
      const themeToggleBtn = document.getElementById("theme-toggle");

      let activeSuite = {json.dumps(default_suite)};
      let activeStatus = "ALL";
      let errorsExpanded = true;

      function updateThemeLabel() {{
        const isDark = document.documentElement.getAttribute("data-theme") === "dark";
        themeToggleBtn.textContent = isDark ? "☀ Light" : "☾ Dark";
      }}
      updateThemeLabel();

      themeToggleBtn.addEventListener("click", function() {{
        const isDark = document.documentElement.getAttribute("data-theme") === "dark";
        const next = isDark ? "light" : "dark";
        document.documentElement.setAttribute("data-theme", next);
        try {{ localStorage.setItem("conformance-report-theme", next); }} catch (e) {{}}
        updateThemeLabel();
      }});

      function applySort() {{
        const mode = sortSelect.value;
        rows.sort(function(a, b) {{
          if (mode === "fail-first") {{
            const ra = Number(a.dataset.statusRank);
            const rb = Number(b.dataset.statusRank);
            if (ra !== rb) return ra - rb;
            const ea = Number(a.dataset.errors);
            const eb = Number(b.dataset.errors);
            if (ea !== eb) return eb - ea;
            return a.dataset.device.localeCompare(b.dataset.device);
          }}
          if (mode === "device-asc") {{
            return a.dataset.device.localeCompare(b.dataset.device);
          }}
          if (mode === "device-desc") {{
            return b.dataset.device.localeCompare(a.dataset.device);
          }}
          if (mode === "errors-desc") {{
            const ea = Number(a.dataset.errors);
            const eb = Number(b.dataset.errors);
            if (ea !== eb) return eb - ea;
            return a.dataset.device.localeCompare(b.dataset.device);
          }}
          return Number(a.dataset.index) - Number(b.dataset.index);
        }});
        rows.forEach(function(r) {{ tbody.appendChild(r); }});
      }}

      function applyFilters() {{
        const query = searchInput.value.trim().toLowerCase();
        let cntAll = 0, cntFail = 0, cntPass = 0, cntInvalid = 0, visible = 0;

        rows.forEach(function(row) {{
          const matchSuite = activeSuite === "ALL" || row.dataset.suite === activeSuite;
          const matchSearch = !query || row.dataset.search.includes(query);
          if (matchSuite && matchSearch) {{
            cntAll++;
            if (row.dataset.status === "FAIL") cntFail++;
            else if (row.dataset.status === "PASS") cntPass++;
            else if (row.dataset.status === "INVALID") cntInvalid++;
          }}

          const matchStatus = activeStatus === "ALL" || row.dataset.status === activeStatus;
          const show = matchSuite && matchSearch && matchStatus;
          row.classList.toggle("is-hidden", !show);
          if (show) visible++;
        }});

        document.getElementById("cnt-all").textContent = cntAll;
        document.getElementById("cnt-fail").textContent = cntFail;
        document.getElementById("cnt-pass").textContent = cntPass;
        document.getElementById("cnt-invalid").textContent = cntInvalid;

        emptyState.style.display = visible === 0 ? "block" : "none";
        visibleCounter.textContent = "Showing " + visible + " of " + cntAll + " entries" +
          (activeSuite === "ALL" ? " across all suites" : " in " + activeSuite);
      }}

      suiteButtons.forEach(function(btn) {{
        btn.addEventListener("click", function() {{
          activeSuite = btn.dataset.suiteTab;
          suiteButtons.forEach(function(b) {{
            b.classList.toggle("is-active", b === btn);
          }});
          applyFilters();
        }});
      }});

      statusButtons.forEach(function(btn) {{
        btn.addEventListener("click", function() {{
          activeStatus = btn.dataset.statusFilter;
          statusButtons.forEach(function(b) {{
            b.classList.toggle("is-active", b === btn);
          }});
          applyFilters();
        }});
      }});

      searchInput.addEventListener("input", applyFilters);
      sortSelect.addEventListener("change", function() {{
        applySort();
        applyFilters();
      }});

      toggleErrorsBtn.addEventListener("click", function() {{
        errorsExpanded = !errorsExpanded;
        document.querySelectorAll("details.error-details").forEach(function(d) {{
          d.open = errorsExpanded;
        }});
        toggleErrorsBtn.textContent = errorsExpanded ? "Collapse errors" : "Expand errors";
      }});

      document.addEventListener("click", function(ev) {{
        const btn = ev.target.closest(".btn-copy");
        if (!btn) return;
        const text = btn.getAttribute("data-copy") || "";
        if (navigator.clipboard && navigator.clipboard.writeText) {{
          navigator.clipboard.writeText(text).then(function() {{
            btn.textContent = "Copied!";
            btn.classList.add("is-copied");
            setTimeout(function() {{
              btn.textContent = "Copy";
              btn.classList.remove("is-copied");
            }}, 1400);
          }});
        }}
      }});

      applySort();
      applyFilters();
    }})();
  </script>
</body>
</html>
"""

    for filename in ("conformance_report.html", "index.html"):
        html_file = os.path.join(html_out_dir, filename)
        print("Saving HTML report to " + html_file)
        with open(html_file, "w") as f:
            f.write(html_report)


def get_git_revision_hash() -> str:
    return subprocess.check_output(['git', 'rev-parse', 'HEAD']).decode('ascii').strip()


if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        description="Build all-devices-app, run conformance tests for device types and generate an HTML report."
    )
    parser.add_argument(
        "--test-name",
        help="Override the default tests with a specific test name."
    )
    parser.add_argument(
        "--target-name",
        default=DEFAULT_TARGET,
        help="Override the default all-devices-app build target."
    )
    parser.add_argument(
        "--app-path",
        help="Explicit path to the all-devices-app executable (overrides searching the target output directory)."
    )
    parser.add_argument(
        "--python-env",
        default=DEFAULT_PYTHON_ENV,
        help="Path to the Python virtual environment directory."
    )
    parser.add_argument(
        "--device-type",
        help="Override the default device types with a specific device type (e.g. aggregator)."
    )
    parser.add_argument(
        "--skip-building",
        help="Skip building the target.",
        action="store_true"
    )
    parser.add_argument(
        "--skip-testing",
        help="Skip testing the device(s).",
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

    target = args.target_name
    target_out_dir = os.path.join(OUT_DIR, target)
    devices = [args.device_type] if args.device_type else DEFAULT_DEVICES
    tests = [args.test_name] if args.test_name else DEFAULT_TESTS

    os.makedirs(TMP_RESULTS_DIR, exist_ok=True)

    build_target(target, args.skip_building)
    executable_path = args.app_path or find_executable(target_out_dir)
    run_tests(tests, executable_path, devices, TMP_RESULTS_DIR, args.skip_testing, args.python_env)

    print(f"Parsing all test output logs in {TMP_RESULTS_DIR}...")
    test_logs = glob.glob(os.path.join(TMP_RESULTS_DIR, "*.txt"))
    aggregated_results_dict = parse_test_logs(test_logs)
    generate_csv_summaries(aggregated_results_dict, TMP_RESULTS_DIR)
    csv_summaries_paths = glob.glob(os.path.join(TMP_RESULTS_DIR, "*.csv"))

    if not args.skip_html:
        os.makedirs(args.html_out_dir, exist_ok=True)
        csv_to_html_report(
            csv_summaries_paths,
            "Matter SDK All-Devices-App Conformance Report",
            args.html_out_dir,
            get_git_revision_hash()
        )
