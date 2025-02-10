#!/usr/bin/env python3

# Copyright (c) 2024 Project CHIP Authors
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import configparser
import enum
import fnmatch
import glob
import logging
import multiprocessing
import os
import platform
import shlex
import stat
import subprocess
import sys
import textwrap
import time
from dataclasses import dataclass
from typing import List, Optional

import alive_progress
import click
import colorama
import coloredlogs
import tabulate
import yaml


def _get_native_machine_target():
    """
    Returns the build prefix for applications, such as 'linux-x64'.
    """
    current_system_info = platform.uname()
    arch = current_system_info.machine
    if arch == "x86_64":
        arch = "x64"
    elif arch == "i386" or arch == "i686":
        arch = "x86"
    elif arch in ("aarch64", "aarch64_be", "armv8b", "armv8l"):
        arch = "arm64"

    return f"{current_system_info.system.lower()}-{arch}"


_CONFIG_PATH = "out/local_py.ini"


def get_coverage_default(coverage: Optional[bool]) -> bool:
    if coverage is not None:
        return coverage
    config = configparser.ConfigParser()
    try:
        config.read(_CONFIG_PATH)
        return config["OPTIONS"].getboolean("coverage")
    except Exception:
        return False


def _get_variants(coverage: Optional[bool]):
    """
    compute the build variant suffixes for the given options
    """
    variants = ["no-ble", "clang", "boringssl"]

    config = configparser.ConfigParser()
    config["OPTIONS"] = {}
    try:
        config.read(_CONFIG_PATH)
        logging.info("Defaults read from '%s'", _CONFIG_PATH)
    except Exception:
        config["OPTIONS"]["coverage"] = "true"

    if coverage is None:
        # Coverage is NOT passed in as an explicit flag, so try to
        # resume it from whatever last `build` flag was used
        coverage = config["OPTIONS"].getboolean("coverage")
        logging.info(
            "Coverage setting not provided via command line. Will use: %s", coverage
        )

    if coverage:
        variants.append("coverage")
        config["OPTIONS"]["coverage"] = "true"
    else:
        config["OPTIONS"]["coverage"] = "false"

    if not os.path.exists("./out"):
        os.mkdir("./out")
    with open(_CONFIG_PATH, "w") as f:
        config.write(f)

    return "-".join(variants)


@dataclass
class ApplicationTarget:
    key: str  # key for test_env running in python
    target: str  # target name for build_examples (and directory in out)
    binary: str  # elf binary to run after it is built


def _get_targets(coverage: Optional[bool]) -> list[ApplicationTarget]:
    target_prefix = _get_native_machine_target()
    suffix = _get_variants(coverage)

    targets = []

    targets.append(
        ApplicationTarget(
            key="CHIP_TOOL",
            target=f"{target_prefix}-chip-tool-{suffix}",
            binary="chip-tool",
        )
    )
    targets.append(
        ApplicationTarget(
            key="ALL_CLUSTERS_APP",
            target=f"{target_prefix}-all-clusters-{suffix}",
            binary="chip-all-clusters-app",
        )
    )
    targets.append(
        ApplicationTarget(
            key="CHIP_LOCK_APP",
            target=f"{target_prefix}-lock-{suffix}",
            binary="chip-lock-app",
        )
    )
    targets.append(
        ApplicationTarget(
            key="ENERGY_MANAGEMENT_APP",
            target=f"{target_prefix}-energy-management-{suffix}",
            binary="chip-energy-management-app",
        )
    )
    targets.append(
        ApplicationTarget(
            key="LIT_ICD_APP",
            target=f"{target_prefix}-lit-icd-{suffix}",
            binary="lit-icd-app",
        )
    )
    targets.append(
        ApplicationTarget(
            key="CHIP_MICROWAVE_OVEN_APP",
            target=f"{target_prefix}-microwave-oven-{suffix}",
            binary="chip-microwave-oven-app",
        )
    )
    targets.append(
        ApplicationTarget(
            key="CHIP_RVC_APP",
            target=f"{target_prefix}-rvc-{suffix}",
            binary="chip-rvc-app",
        )
    )
    targets.append(
        ApplicationTarget(
            key="NETWORK_MANAGEMENT_APP",
            target=f"{target_prefix}-network-manager-ipv6only-{suffix}",
            binary="matter-network-manager-app",
        )
    )
    targets.append(
        ApplicationTarget(
            key="FABRIC_ADMIN_APP",
            target=f"{target_prefix}-fabric-admin-no-wifi-rpc-ipv6only-{suffix}",
            binary="fabric-admin",
        )
    )
    targets.append(
        ApplicationTarget(
            key="FABRIC_BRIDGE_APP",
            target=f"{target_prefix}-fabric-bridge-no-wifi-rpc-ipv6only-{suffix}",
            binary="fabric-bridge-app",
        )
    )
    targets.append(
        ApplicationTarget(
            key="FABRIC_SYNC_APP",
            target=f"{target_prefix}-fabric-sync-no-wifi-ipv6only-{suffix}",
            binary="fabric-sync",
        )
    )
    targets.append(
        ApplicationTarget(
            key="LIGHTING_APP_NO_UNIQUE_ID",
            target=f"{target_prefix}-light-data-model-no-unique-id-ipv6only-no-wifi-{suffix}",
            binary="chip-lighting-app",
        )
    )

    # These are needed for chip tool tests
    targets.append(
        ApplicationTarget(
            key="OTA_PROVIDER_APP",
            target=f"{target_prefix}-ota-provider-{suffix}",
            binary="chip-ota-provider-app",
        )
    )
    targets.append(
        ApplicationTarget(
            key="OTA_REQUESTOR_APP",
            target=f"{target_prefix}-ota-requestor-{suffix}",
            binary="chip-ota-requestor-app",
        )
    )
    targets.append(
        ApplicationTarget(
            key="TV_APP",
            target=f"{target_prefix}-tv-app-{suffix}",
            binary="chip-tv-app",
        )
    )
    targets.append(
        ApplicationTarget(
            key="BRIDGE_APP",
            target=f"{target_prefix}-bridge-{suffix}",
            binary="chip-bridge-app",
        )
    )
    targets.append(
        ApplicationTarget(
            key="TERMS_AND_CONDITIONS_APP",
            target=f"{target_prefix}-terms-and-conditions-{suffix}",
            binary="chip-terms-and-conditions-app",
        )
    )

    return targets


class BinaryRunner(enum.Enum):
    """
    Enumeration describing a wrapper runner for an application. Useful for debugging
    failures (i.e. running under memory validators or replayability for failures).
    """

    NONE = enum.auto()
    RR = enum.auto()
    VALGRIND = enum.auto()
    COVERAGE = enum.auto()

    def execute_str(self, path: str):
        if self == BinaryRunner.NONE:
            return path
        elif self == BinaryRunner.RR:
            return f"exec rr record {path}"
        elif self == BinaryRunner.VALGRIND:
            return f"exec valgrind {path}"
        elif self == BinaryRunner.COVERAGE:
            # Expected path is like "out/<target>/<binary>"
            #
            # We output up to 10K of separate profiles that will be merged as a
            # final step.
            rawname = os.path.basename(path[: path.rindex("/")] + "-run-%10000m.profraw")
            return f'export LLVM_PROFILE_FILE="out/profiling/{rawname}"; exec {path}'


__RUNNERS__ = {
    "none": BinaryRunner.NONE,
    "rr": BinaryRunner.RR,
    "valgrind": BinaryRunner.VALGRIND,
}

__LOG_LEVELS__ = {
    "debug": logging.DEBUG,
    "info": logging.INFO,
    "warn": logging.WARN,
    "fatal": logging.FATAL,
}


@dataclass
class ExecutionTimeInfo:
    """
    Contains information about duration that a script took to run
    """

    script: str
    duration_sec: float
    status: str


# Top level command, groups all other commands for the purpose of having
# common command line arguments.
@click.group()
@click.option(
    "--log-level",
    default="INFO",
    type=click.Choice(list(__LOG_LEVELS__.keys()), case_sensitive=False),
    help="Determines the verbosity of script output",
)
def cli(log_level):
    """
    Helper script design to make running tests localy simpler. Handles
    application/prerequisite builds and execution of tests.

    The binary is designed to be run in the root checkout and will
    compile things in `out/` and execute tests locally.

    These are examples for running "Python tests"

    \b
    local.py build                              # builds python and applications
    local.py python-tests                       # Runs ALL python tests
    local.py python-tests --test-filter TC_FAN  # Runs all *FAN* tests

    \b
    local.py build-apps                         # Re-build applications (if only those changed)
    local.py build-python                       # Re-build python module only
    """
    coloredlogs.install(
        level=__LOG_LEVELS__[log_level], fmt="%(asctime)s %(levelname)-7s %(message)s"
    )


def _with_activate(build_cmd: List[str], output_path=None) -> List[str]:
    """
    Given a bash command list, will generate a new command suitable for subprocess
    with an execution of `scripts/activate.sh` prepended to it
    """
    cmd = shlex.join(build_cmd)
    if output_path:
        cmd = cmd + f" >{output_path}"

    return [
        "bash",
        "-c",
        ";".join(["set -e", "source scripts/activate.sh", cmd])
    ]


def _do_build_python():
    """
    Builds a python virtual environment into `out/venv`
    """
    logging.info("Building python packages in out/venv ...")
    subprocess.run(
        ["./scripts/build_python.sh", "--install_virtual_env", "out/venv"], check=True
    )


def _do_build_apps(coverage: Optional[bool], ccache: bool):
    """
    Builds example python apps suitable for running all python_tests.

    This builds a LOT of apps (significant storage usage).
    """
    logging.info("Building example apps...")

    targets = [t.target for t in _get_targets(coverage)]

    cmd = ["./scripts/build/build_examples.py"]
    for target in targets:
        cmd.append("--target")
        cmd.append(target)

    if ccache:
        cmd.append("--pw-command-launcher=ccache")

    cmd.append("build")

    subprocess.run(_with_activate(cmd), check=True)


def _do_build_basic_apps(coverage: Optional[bool]):
    """
    Builds a minimal subset of test applications, specifically
    all-clusters and chip-tool only, for basic tests.
    """
    logging.info("Building example apps...")

    all_targets = dict([(t.key, t) for t in _get_targets(coverage)])
    targets = [
        all_targets["CHIP_TOOL"].target,
        all_targets["ALL_CLUSTERS_APP"].target,
    ]

    cmd = ["./scripts/build/build_examples.py"]
    for target in targets:
        cmd.append("--target")
        cmd.append(target)
    cmd.append("build")

    subprocess.run(_with_activate(cmd), check=True)


@cli.command()
@click.option("--coverage/--no-coverage", default=None)
def build_basic_apps(coverage):
    """Builds chip-tool and all-clusters app."""
    _do_build_basic_apps(coverage)


@cli.command()
def build_python():
    """
    Builds a python environment in out/pyenv.

    Generally used together with `python-tests`.
    To re-build the python environment use `build-apps`.
    To re-build both python and apps, use `build`
    """
    _do_build_python()


@cli.command()
@click.option("--coverage/--no-coverage", default=None)
@click.option("--ccache/--no-ccache", default=False)
def build_apps(coverage, ccache):
    """
    Builds MANY apps used by python-tests.

    Generally used together with `python-tests`.
    To re-build the python environment use `build-python`.
    To re-build both python and apps, use `build`
    """
    _do_build_apps(coverage, ccache)


@cli.command()
@click.option("--coverage/--no-coverage", default=None)
@click.option("--ccache/--no-ccache", default=False)
def build(coverage, ccache):
    """
    Builds both python and apps (same as build-python + build-apps)

    Generally used together with `python-tests`.
    """
    _do_build_python()
    _do_build_apps(coverage, ccache)


def _maybe_with_runner(script_name: str, path: str, runner: BinaryRunner):
    """
    Constructs a "real" path to execute, which may be replacing the input
    path with a wrapper script that executes things like valgrind or rr.
    """
    if runner == BinaryRunner.NONE:
        return path

    # create a separate runner script based on the app
    if not os.path.exists("out/runners"):
        os.mkdir("out/runners")

    script_name = f"out/runners/{script_name}.sh"
    with open(script_name, "wt") as f:
        f.write(
            textwrap.dedent(
                f"""\
                #!/usr/bin/env bash
                {runner.execute_str(path)} "$@"
                """
            )
        )
    st = os.stat(script_name)
    os.chmod(script_name, st.st_mode | stat.S_IEXEC)

    return script_name


def _add_target_to_cmd(cmd, flag, path, runner):
    """
    Handles the `--target` argument (or similar) to a command list.

    Specifically it figures out how to convert `path` into either itself or
    execution via a `runner` script.

    cmd will get "<flag> <executable>" appended to it, where executable
    is either the input path or a wrapper script to execute via the given
    input runner.
    """
    cmd.append(flag)
    cmd.append(_maybe_with_runner(flag[2:].replace("-", "_"), path, runner))


@dataclass
class GlobFilter:
    pattern: str

    def matches(self, txt: str) -> bool:
        return fnmatch.fnmatch(txt, self.pattern)


@dataclass
class FilterList:
    filters: list[GlobFilter]

    def any_matches(self, txt: str) -> bool:
        return any([f.matches(txt) for f in self.filters])


def _parse_filters(entry: str) -> FilterList:
    if not entry:
        entry = "*.*"

    if "," in entry:
        entry_list = entry.split(",")
    else:
        entry_list = [entry]

    filters = []
    for f in entry_list:
        if not f.startswith("*"):
            f = "*" + f
        if not f.endswith("*"):
            f = f + "*"
        filters.append(GlobFilter(pattern=f))

    return FilterList(filters=filters)


@dataclass
class RawProfile:
    raw_profile_paths: list[str]
    binary_path: str


def _raw_profile_to_info(profile: RawProfile):
    path = profile.raw_profile_paths[0]

    # Merge all per-app profiles into a single large profile
    data_path = path[:path.find("-run-")] + ".profdata"
    cmd = [
        "llvm-profdata",
        "merge",
        "-sparse",
    ]
    cmd.extend(["-o", data_path])
    cmd.extend(profile.raw_profile_paths)

    p = subprocess.run(_with_activate(cmd), check=True, capture_output=True)

    # Export to something lcov likes
    cmd = [
        "llvm-cov",
        "export",
        "-format=lcov",
        "--instr-profile",
        data_path,
        profile.binary_path
    ]

    # for -ignore-filename-regex
    ignore_paths = [
        "third_party/boringssl/.*",
        "third_party/perfetto/.*",
        "third_party/jsoncpp/.*",
        "third_party/editline/.*",
        "third_party/initpp/.*",
        "third_party/libwebsockets/.*",
        "third_party/pigweed/.*",
        "third_party/nanopb/.*",
        "third_party/nl.*",
        "/usr/include/.*",
        "/usr/lib/.*",
    ]

    for p in ignore_paths:
        cmd.append("-ignore-filename-regex")
        cmd.append(p)

    info_path = path.replace(".profraw", ".info")
    subprocess.run(_with_activate(cmd, output_path=info_path), check=True)
    logging.info("Generated %s", info_path)

    # !!!!! HACK ALERT !!!!!
    #
    # The paths for our examples are generally including CHIP as
    # examples/<name>/third_party/connectedhomeip/....
    # So we will replace every occurence of these to remove the extra indirection into third_party
    #
    # Generally we will replace every path (Shown as SF:...) with the corresponding item
    #
    # We assume that the info lines fit in RAM
    lines = []
    with open(info_path, 'rt') as f:
        for line in f.readlines():
            line = line.rstrip()
            if line.startswith("SF:"):
                # This is a source file line: "SF:..."
                path = line[3:]
                line = f"SF:{os.path.realpath(path)}\n"
            lines.append(line)

    # re-write it.
    with open(info_path, 'wt') as f:
        f.write("\n".join(lines))

    return info_path


@cli.command()
@click.option(
    "--flat",
    default=False,
    is_flag=True,
    show_default=True,
    help="Use a flat (1-directory) layout rather than hierarchical.",
)
def gen_coverage(flat):
    """
    Generate coverage from tests run with "--coverage"
    """
    # This assumes default.profraw exists, so it tries to
    # generate coverage out of it
    #
    # Each target gets its own profile

    raw_profiles = []
    for t in _get_targets(coverage=True):
        glob_str = os.path.join("./out", "profiling", f"{t.target}*.profraw")
        app_profiles = []
        for path in glob.glob(glob_str):
            app_profiles.append(path)

        if app_profiles:
            raw_profiles.append(RawProfile(
                raw_profile_paths=app_profiles,
                binary_path=os.path.join("./out", t.target, t.binary)
            ))
        else:
            logging.warning("No profiles for %s", t.target)

    with multiprocessing.Pool() as p:
        trace_files = p.map(_raw_profile_to_info, raw_profiles)

    if not trace_files:
        logging.error(
            "Could not find any trace files. Did you run tests with coverage enabled?"
        )
        return

    cmd = ["lcov"]
    for t in trace_files:
        cmd.append("--add-tracefile")
        cmd.append(t)

    errors_to_ignore = [
        "inconsistent", "range", "corrupt", "category"
    ]

    cmd.append("--output-file")
    cmd.append("out/profiling/merged.info")
    for e in errors_to_ignore:
        cmd.append("--ignore-errors")
        cmd.append(e)

    if os.path.exists("out/profiling/merged.info"):
        os.unlink("out/profiling/merged.info")

    subprocess.run(cmd, check=True)

    logging.info("Generating HTML...")
    cmd = ["genhtml"]
    for e in errors_to_ignore:
        cmd.append("--ignore-errors")
        cmd.append(e)

    cmd.append("--flat" if flat else "--hierarchical")
    cmd.append("--output-directory")
    cmd.append("out/coverage")
    cmd.append("out/profiling/merged.info")

    subprocess.run(cmd, check=True)
    logging.info("Coverage HTML should be available in out/coverage/index.html")


@cli.command()
@click.option(
    "--test-filter",
    default="*",
    show_default=True,
    help="Run only tests that match the given glob filter(s). Comma separated list of filters",
)
@click.option(
    "--skip",
    default="",
    show_default=True,
    help="Skip the tests matching the given glob. Comma separated list of filters. Empty for no skipping.",
)
@click.option(
    "--from-filter",
    default=None,
    help="Start running from the test matching the given glob pattern (including the test that matches).",
)
@click.option(
    "--from-skip-filter",
    default=None,
    help="Start from the first test matching the given glob, but skip the matching element (start right after).",
)
@click.option(
    "--dry-run",
    default=False,
    is_flag=True,
    show_default=True,
    help="Don't actually execute the tests, just print out the command that would be run.",
)
@click.option(
    "--no-show-timings",
    default=False,
    is_flag=True,
    help="At the end of the execution, show how many seconds each test took.",
)
@click.option(
    "--keep-going",
    default=False,
    is_flag=True,
    show_default=True,
    help="Keep going on errors. Will report all failed tests at the end.",
)
@click.option(
    "--fail-log-dir",
    default=None,
    help="Save failure logs into the specified directory instead of logging (as logging can be noisy/slow)",
    type=click.Path(file_okay=False, dir_okay=True),
)
@click.option("--coverage/--no-coverage", default=None)
@click.option(
    "--runner",
    default="none",
    type=click.Choice(list(__RUNNERS__.keys()), case_sensitive=False),
    help="Determines the verbosity of script output",
)
def python_tests(
    test_filter,
    skip,
    from_filter,
    from_skip_filter,
    dry_run,
    no_show_timings,
    runner,
    keep_going,
    coverage,
    fail_log_dir,
):
    """
    Run python tests via `run_python_test.py`

    Constructs the run yaml in `out/test_env.yaml`. Assumes that binaries
    were built already, generally with `build` (or separate `build-python` and `build-apps`).
    """
    runner = __RUNNERS__[runner]

    # make sure we are fully aware if running with or without coverage
    coverage = get_coverage_default(coverage)
    if coverage:
        if runner != BinaryRunner.NONE:
            logging.error("Runner for coverage is implict")
            sys.exit(1)

        # wrap around so we get a good LLVM_PROFILE_FILE
        runner = BinaryRunner.COVERAGE

    def as_runner(path):
        return _maybe_with_runner(os.path.basename(path), path, runner)

    # create an env file
    with open("./out/test_env.yaml", "wt") as f:
        for target in _get_targets(coverage):
            run_path = as_runner(f"out/{target.target}/{target.binary}")
            f.write(f"{target.key}: {run_path}\n")
        f.write("TRACE_APP: out/trace_data/app-{SCRIPT_BASE_NAME}\n")
        f.write("TRACE_TEST_JSON: out/trace_data/test-{SCRIPT_BASE_NAME}\n")
        f.write("TRACE_TEST_PERFETTO: out/trace_data/test-{SCRIPT_BASE_NAME}\n")

    if not test_filter:
        test_filter = "*"
    test_filter = _parse_filters(test_filter)

    if skip:
        print("SKIP IS %r" % skip)
        skip = _parse_filters(skip)

    if from_filter:
        if not from_filter.startswith("*"):
            from_filter = "*" + from_filter
        if not from_filter.endswith("*"):
            from_filter = from_filter + "*"

    if from_skip_filter:
        if not from_skip_filter.startswith("*"):
            from_skip_filter = "*" + from_skip_filter
        if not from_skip_filter.endswith("*"):
            from_skip_filter = from_skip_filter + "*"

    # This MUST be available or perfetto dies. This is VERY annoying to debug
    if not os.path.exists("out/trace_data"):
        os.mkdir("out/trace_data")

    if fail_log_dir and not os.path.exists(fail_log_dir):
        os.mkdir(fail_log_dir)

    metadata = yaml.full_load(open("src/python_testing/test_metadata.yaml"))
    excluded_patterns = set([item["name"] for item in metadata["not_automated"]])

    # NOTE: for slow tests. we add logs to not get impatient
    slow_test_duration = dict(
        [(item["name"], item["duration"]) for item in metadata["slow_tests"]]
    )

    if not os.path.isdir("src/python_testing"):
        raise Exception(
            "Script meant to be run from the CHIP checkout root (src/python_testing must exist)."
        )

    test_scripts = []
    for file in glob.glob(os.path.join("src/python_testing/", "*.py")):
        if os.path.basename(file) in excluded_patterns:
            continue
        test_scripts.append(file)
    test_scripts.append("src/controller/python/test/test_scripts/mobile-device-test.py")
    test_scripts.sort()  # order consistent

    execution_times = []
    failed_tests = []
    try:
        to_run = []
        for script in [t for t in test_scripts if test_filter.any_matches(t)]:
            if from_filter:
                if not fnmatch.fnmatch(script, from_filter):
                    logging.info("From-filter SKIP %s", script)
                    continue
                from_filter = None

            if from_skip_filter:
                if fnmatch.fnmatch(script, from_skip_filter):
                    from_skip_filter = None
                logging.info("From-skip-filter SKIP %s", script)
                continue
            if skip:
                if skip.any_matches(script):
                    logging.info("EXPLICIT SKIP %s", script)
                    continue

            to_run.append(script)

        with alive_progress.alive_bar(len(to_run), title="Running tests") as bar:
            for script in to_run:
                bar.text(script)
                cmd = [
                    "scripts/run_in_python_env.sh",
                    "out/venv",
                    f"./scripts/tests/run_python_test.py --load-from-env out/test_env.yaml --script {script}",
                ]

                if dry_run:
                    print(shlex.join(cmd))
                    continue

                base_name = os.path.basename(script)
                if base_name in slow_test_duration:
                    logging.warning(
                        "SLOW test '%s' is executing (expect to take around %s). Be patient...",
                        base_name,
                        slow_test_duration[base_name],
                    )

                tstart = time.time()
                result = subprocess.run(cmd, capture_output=True)
                tend = time.time()

                if result.returncode != 0:
                    logging.error("Test failed: %s", script)
                    if fail_log_dir:
                        out_name = os.path.join(fail_log_dir, f"{base_name}.out.log")
                        err_name = os.path.join(fail_log_dir, f"{base_name}.err.log")

                        logging.error("STDOUT IN %s", out_name)
                        logging.error("STDERR IN %s", err_name)

                        with open(out_name, "wb") as f:
                            f.write(result.stdout)
                        with open(err_name, "wb") as f:
                            f.write(result.stderr)

                    else:
                        logging.info("STDOUT:\n%s", result.stdout.decode("utf8"))
                        logging.warning("STDERR:\n%s", result.stderr.decode("utf8"))
                    if not keep_going:
                        sys.exit(1)
                    failed_tests.append(script)

                time_info = ExecutionTimeInfo(
                    script=base_name,
                    duration_sec=(tend - tstart),
                    status=(
                        "PASS"
                        if result.returncode == 0
                        else colorama.Fore.RED + "FAILURE" + colorama.Fore.RESET
                    ),
                )
                execution_times.append(time_info)

                if time_info.duration_sec > 20 and base_name not in slow_test_duration:
                    logging.warning(
                        "%s finished in %0.2f seconds",
                        time_info.script,
                        time_info.duration_sec,
                    )
                bar()
    finally:
        if failed_tests and keep_going:
            logging.error("FAILED TESTS:")
            for name in failed_tests:
                logging.error("  %s", name)

        if execution_times and not no_show_timings:
            execution_times.sort(
                key=lambda v: (0 if v.status == "PASS" else 1, v.duration_sec),
            )
            print(
                tabulate.tabulate(
                    execution_times, headers=["Script", "Duration(sec)", "Status"]
                )
            )

        if failed_tests:
            # Propagate the final failure
            sys.exit(1)


def _do_build_fabric_sync_apps(coverage: Optional[bool]):
    """
    Build applications used for fabric sync tests
    """
    target_prefix = _get_native_machine_target()
    suffix = _get_variants(coverage)
    targets = [
        f"{target_prefix}-fabric-admin-no-wifi-rpc-ipv6only-{suffix}",
        f"{target_prefix}-fabric-bridge-no-wifi-rpc-ipv6only-{suffix}",
        f"{target_prefix}-all-clusters-{suffix}",
    ]

    build_cmd = ["./scripts/build/build_examples.py"]
    for target in targets:
        build_cmd.append("--target")
        build_cmd.append(target)
    build_cmd.append("build")

    subprocess.run(_with_activate(build_cmd))


@cli.command()
@click.option("--coverage/--no-coverage", default=None)
def build_fabric_sync_apps(coverage):
    """
    Build fabric synchronizatio applications.
    """
    _do_build_fabric_sync_apps(coverage)


@cli.command()
@click.option("--coverage/--no-coverage", default=None)
def build_fabric_sync(coverage):
    """
    Builds both python environment and fabric sync applications
    """
    # fabric sync interfaces with python for convenience, so do that
    _do_build_python()
    _do_build_fabric_sync_apps(coverage)


@cli.command()
@click.option("--asan", is_flag=True, default=False, show_default=True)
def build_casting_apps(asan):
    """
    Builds Applications used for tv casting tests
    """
    tv_args = []
    casting_args = []

    casting_args.append("chip_casting_simplified=true")

    tv_args.append('chip_crypto="boringssl"')
    casting_args.append('chip_crypto="boringssl"')

    if asan:
        tv_args.append("is_asan=true is_clang=true")
        casting_args.append("is_asan=true is_clang=true")

    tv_args = " ".join(tv_args)
    casting_args = " ".join(casting_args)

    if tv_args:
        tv_args = f" '{tv_args}'"
    if casting_args:
        casting_args = f" '{casting_args}'"

    cmd = ";".join(
        [
            "set -e",
            "source scripts/activate.sh",
            f"./scripts/examples/gn_build_example.sh examples/tv-app/linux/ out/tv-app{tv_args}",
            f"./scripts/examples/gn_build_example.sh  examples/tv-casting-app/linux/ out/tv-casting-app{casting_args}",
        ]
    )
    subprocess.run(["bash", "-c", cmd], check=True)


@cli.command()
@click.option("--test", type=click.Choice(["basic", "passcode"]), default="basic")
@click.option("--log-directory", default=None)
@click.option(
    "--tv-app",
    type=str,
    default="out/tv-app/chip-tv-app",
)
@click.option(
    "--tv-casting-app",
    type=str,
    default="out/tv-casting-app/chip-tv-casting-app",
)
@click.option(
    "--runner",
    default="none",
    type=click.Choice(list(__RUNNERS__.keys()), case_sensitive=False),
    help="Determines the verbosity of script output",
)
def casting_test(test, log_directory, tv_app, tv_casting_app, runner):
    """
    Runs the tv casting tests.

    Generally used after `build-casting-apps`.
    """
    runner = __RUNNERS__[runner]

    script = "python3 scripts/tests/run_tv_casting_test.py"

    script += f" --tv-app-rel-path '{_maybe_with_runner('tv_app', tv_app, runner)}'"
    script += f" --tv-casting-app-rel-path '{_maybe_with_runner('casting_app', tv_casting_app, runner)}'"

    if test == "passcode":
        script += " --commissioner-generated-passcode true"

    if log_directory:
        script += f" --log-directory '{log_directory}'"

    cmd = ";".join(["set -e", "source scripts/activate.sh", script])
    subprocess.run(["bash", "-c", cmd], check=True)


@cli.command()
@click.option("--target", default=None)
@click.option("--target-glob", default=None)
@click.option("--include-tags", default=None)
@click.option("--expected-failures", default=None)
@click.option("--coverage/--no-coverage", default=None)
@click.option(
    "--keep-going",
    default=False,
    is_flag=True,
    show_default=True,
    help="Keep going on errors. Will report all failed tests at the end.",
)
@click.option(
    "--runner",
    default="none",
    type=click.Choice(list(__RUNNERS__.keys()), case_sensitive=False),
    help="Determines the verbosity of script output",
)
def chip_tool_tests(
    target, target_glob, include_tags, expected_failures, coverage, keep_going, runner
):
    """
    Run integration tests using chip-tool.

    Assumes `build-apps` was used to build applications, although build-basic-apps will be
    sufficient for all-clusters tests.
    """

    # This likely should be run in docker to not allow breaking things
    # run as:
    #
    # docker run --rm -it -v ~/devel/connectedhomeip:/workspace --privileged ghcr.io/project-chip/chip-build-vscode:64
    runner = __RUNNERS__[runner]

    # make sure we are fully aware if running with or without coverage
    coverage = get_coverage_default(coverage)
    if coverage:
        if runner != BinaryRunner.NONE:
            logging.error("Runner for coverage is implict")
            sys.exit(1)

        # wrap around so we get a good LLVM_PROFILE_FILE
        runner = BinaryRunner.COVERAGE

    cmd = [
        "./scripts/tests/run_test_suite.py",
        "--runner",
        "chip_tool_python",
    ]

    cmd.extend(["--exclude-tags", "MANUAL"])
    cmd.extend(["--exclude-tags", "IN_DEVELOPMENT"])
    cmd.extend(["--exclude-tags", "FLAKY"])
    cmd.extend(["--exclude-tags", "EXTRA_SLOW"])
    cmd.extend(["--exclude-tags", "PURPOSEFUL_FAILURE"])

    paths = dict(
        [(t.key, f"./out/{t.target}/{t.binary}") for t in _get_targets(coverage)]
    )

    if runner == BinaryRunner.COVERAGE:
        # when running with coveage, chip-tool also is covered
        cmd.extend(["--chip-tool", _maybe_with_runner("chip-tool", paths["CHIP_TOOL"], runner)])
    else:
        cmd.extend(["--chip-tool", paths["CHIP_TOOL"]])

    if target is not None:
        cmd.extend(["--target", target])

    if include_tags is not None:
        cmd.extend(["--include-tags", include_tags])

    if target_glob is not None:
        cmd.extend(["--target-glob", target_glob])

    cmd.append("run")
    cmd.extend(["--iterations", "1"])

    # NOTE: we allow all runs here except extra slow
    #       This means our timeout is quite large
    cmd.extend(["--test-timeout-seconds", "300"])

    if expected_failures is not None:
        cmd.extend(["--expected-failures", expected_failures])
        keep_going = True  # if we expect failures, we have to keep going

    if keep_going:
        cmd.append("--keep-going")

    target_flags = [
        ("--all-clusters-app", "ALL_CLUSTERS_APP"),
        ("--lock-app", "CHIP_LOCK_APP"),
        ("--ota-provider-app", "OTA_PROVIDER_APP"),
        ("--ota-requestor-app", "OTA_REQUESTOR_APP"),
        ("--tv-app", "TV_APP"),
        ("--bridge-app", "BRIDGE_APP"),
        ("--lit-icd-app", "LIT_ICD_APP"),
        ("--microwave-oven-app", "CHIP_MICROWAVE_OVEN_APP"),
        ("--rvc-app", "CHIP_RVC_APP"),
    ]

    for flag, path_key in target_flags:
        _add_target_to_cmd(cmd, flag, paths[path_key], runner)

    subprocess.run(_with_activate(cmd), check=True)


if __name__ == "__main__":
    cli()
