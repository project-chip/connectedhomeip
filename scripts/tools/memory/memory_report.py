import argparse
import csv
import os
import subprocess
from dataclasses import dataclass
from datetime import datetime

import memdf.collect
import memdf.report
import memdf.select
import memdf.util
import pandas
from memdf import Config, DFs, SectionDF

# Constants for output directories
TMP_RESULTS_DIR = "/tmp/memory_report"
OUT_DIR = "./out"


@dataclass
class TargetInfo:
    name: str
    config_file: str
    executable_path: str


# Configuration file mapping
config_files_dict = {
    "linux-": "scripts/tools/memory/platform/linux.cfg",
    "efr32-": "scripts/tools/memory/platform/efr32.cfg",
    "esp32-": "scripts/tools/memory/platform/esp32.cfg",
    "telink-": "scripts/tools/memory/platform/telink.cfg",
    "tizen-": "scripts/tools/memory/platform/tizen.cfg",
}


def find_built_targets(config_files_dict: dict[str, str], out_dir: str) -> list[TargetInfo]:
    """Finds built targets and their associated configuration files.

    Args:
        config_files_dict: Dictionary mapping prefixes to config file paths.
        out_dir: Directory to search for built targets.

    Returns:
        A list of TargetInfo objects.
    """
    print(f"Searching targets built in {out_dir}")
    targets = []
    if not os.path.isdir(out_dir):
        print(f"Warning: Output directory '{out_dir}' does not exist.")
        return targets

    for dir_name in sorted(os.listdir(out_dir)):
        dir_path = os.path.join(out_dir, dir_name)
        if not os.path.isdir(dir_path):
            continue

        for prefix, config_file in config_files_dict.items():
            if dir_name.startswith(prefix):
                executable_path = find_executable(dir_path)
                if executable_path:
                    targets.append(
                        TargetInfo(name=dir_name, config_file=config_file, executable_path=executable_path)
                    )
                break  # Move to the next directory

    print(f"Found {len(targets)} built targets!")
    return targets


def find_executable(directory: str) -> str | None:
    """Finds the first executable file in a directory.

    Args:
        directory: The directory to search.

    Returns:
        The path to the executable, or None if no executable is found.
    """
    if not os.path.isdir(directory):
        return None

    for filename in os.listdir(directory):
        if filename.endswith((".sh", ".py")):  # More robust exclusion
            continue
        filepath = os.path.join(directory, filename)
        if os.path.isfile(filepath) and os.access(filepath, os.X_OK):
            return filepath
    return None


def calculate_flash_ram(target_info: TargetInfo) -> tuple[int, int, str] | None:
    """Calculates the flash and RAM usage of a binary.

    Args:
        target_info:  TargetInfo object

    Returns:
        A tuple containing (flash usage, RAM usage, details string), or None on error.
    """
    try:
        config_desc = {
            **memdf.util.config.CONFIG,
            **memdf.collect.CONFIG,
            **memdf.select.CONFIG,
            **memdf.report.OUTPUT_CONFIG,
        }
        config = Config().init(config_desc)
        config.parse(['', '--config-file', target_info.config_file])

        collected: DFs = memdf.collect.collect_files(config, [target_info.executable_path])
        sections = collected[SectionDF.name]
        section_summary = sections[['section', 'size']].sort_values(by='section')
        section_summary.attrs['name'] = "section"

        region_summary = memdf.select.groupby(config, collected['section'], 'region')
        region_summary.attrs['name'] = "region"

        flash = region_summary[region_summary['region'] == 'FLASH']['size'].iloc[0]
        ram = region_summary[region_summary['region'] == 'RAM']['size'].iloc[0]
        details = str(section_summary)

        return (flash, ram, details)

    except (KeyError, IndexError, FileNotFoundError) as e:
        print(f"Error processing {target_info.name}: {e}")
        return None
    except Exception as e:
        print(f"An unexpected error occurred while processing {target_info.name}: {e}")
        return None


def generate_csv_report(results: dict[str, tuple[int, int, str]], csv_filename: str):
    """Generates a CSV report of the memory usage results.

    Args:
        results: A dictionary mapping target names to (flash, ram, details) tuples.
        csv_filename: The output CSV filename.
    """
    try:
        with open(csv_filename, 'w', newline='') as f:  # Use newline='' for correct CSV handling
            writer = csv.writer(f)
            writer.writerow(["Application", "FLASH (bytes)", "RAM (bytes)", "Details"])
            for app, (flash, ram, details) in results.items():
                writer.writerow([app, flash, ram, details])
        print(f"CSV Memory summary saved to {csv_filename}")
    except Exception as e:
        print(f"Error generating CSV report: {e}")


def generate_html_report(csv_file_path: str, html_page_title: str, html_table_title: str, html_out_dir: str, sha: str):
    """Generates an HTML report from a CSV file, inlined."""
    try:
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
        <body>
        <h1>{html_page_title}</h1>
        <hr>
        <h4>Generated on: {now}<br>SHA: {sha}</h4>
        <hr>
        """

        table = pandas.read_csv(csv_file_path)
        table['Details'] = "<details><summary>Show/Hide</summary>" + \
            table['Details'].str.replace('\n', '<br>') + "</details>"

        html_table = f"<h2>{html_table_title}</h2>"
        html_table += table.to_html(escape=False)
        html_report += html_table
        html_report += """
        </body>
        </html>
        """

        html_file = os.path.join(html_out_dir, "memory_report.html")
        print(f"Saving HTML report to {html_file}")
        with open(html_file, "w") as f:
            f.write(html_report)

    except FileNotFoundError:
        print(f"Error: Could not find {csv_file_path}")
    except Exception as e:
        print(f"Error generating HTML report: {e}")


def get_git_revision_hash() -> str:
    """Gets the current Git revision hash."""
    try:
        return subprocess.check_output(['git', 'rev-parse', 'HEAD']).decode('ascii').strip()
    except subprocess.CalledProcessError:
        return "N/A"  # Not a git repository, or git not available


def main():
    """Main function to parse arguments and run the memory analysis."""
    parser = argparse.ArgumentParser(
        description="Calculate FLASH and RAM usage on example apps and generate a report."
    )
    parser.add_argument(
        "--out-dir",
        help="Override the default ./out directory to search for built targets.",
        default=OUT_DIR
    )
    parser.add_argument(
        "--html-out-dir",
        help="Specify the directory to save the HTML report.",
        default=TMP_RESULTS_DIR
    )

    args = parser.parse_args()

    targets = find_built_targets(config_files_dict, args.out_dir)

    results = {}
    print("APP\tFLASH\tRAM")  # header for text output.
    for target in targets:
        result = calculate_flash_ram(target)
        if result:
            flash, ram, details = result
            results[target.name] = (flash, ram, details)
            print(f"{target.name}\t{flash}\t{ram}")

    os.makedirs(args.html_out_dir, exist_ok=True)
    csv_filename = os.path.join(args.html_out_dir, "flash_ram.csv")
    generate_csv_report(results, csv_filename)
    generate_html_report(csv_filename, "Matter SDK Memory Usage Report",
                         "Example Apps Memory Usage", args.html_out_dir, get_git_revision_hash())


if __name__ == "__main__":
    main()
