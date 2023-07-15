#!/usr/bin/env python
#
# Runs clang-tidy on files based on a `compile_commands.json` file
#

"""
Run clang-tidy in parallel on compile databases.

Example run:

# This prepares the build. NOTE this is `build` not `gen` because the build
# step generates required header files (this can be simplified if needed
# to invoke ninja to compile only generated files if needed)

./scripts/build/build_examples.py --target linux-x64-chip-tool-clang build

# Actually running clang-tidy to check status

./scripts/run-clang-tidy-on-compile-commands.py check

# Run and output a fix yaml

./scripts/run-clang-tidy-on-compile-commands.py --export-fixes out/fixes.yaml check

# Apply the fixes
clang-apply-replacements out/fixes.yaml

"""

import glob
import json
import logging
import multiprocessing
import os
import queue
import re
import shlex
import subprocess
import sys
import tempfile
import threading
import traceback

import click
import coloredlogs
import yaml


class TidyResult:
    def __init__(self, path: str, ok: bool):
        self.path = path
        self.ok = ok

    def __repr__(self):
        if self.ok:
            status = "OK"
        else:
            status = "FAIL"

        return "%s(%s)" % (status, self.path)

    def __str__(self):
        return self.__repr__()


class ClangTidyEntry:
    """Represents a single entry for running clang-tidy based
    on a compile_commands.json item.
    """

    def __init__(self, json_entry, gcc_sysroot=None):
        # Entries in compile_commands:
        #    - "directory": location to run the compile
        #    - "file": a relative path to directory
        #    - "command": full compilation command

        self.directory = json_entry["directory"]
        self.file = json_entry["file"]
        self.valid = False
        self.clang_arguments = []
        self.tidy_arguments = []

        command = json_entry["command"]

        command_items = shlex.split(command)
        compiler = os.path.basename(command_items[0])

        # Allow gcc/g++ invocations to also be tidied - arguments should be
        # compatible and on darwin gcc/g++ is actually a symlink to clang
        if compiler in ['clang++', 'clang', 'gcc', 'g++']:
            self.valid = True
            self.clang_arguments = command_items[1:]
        else:
            logging.warning(
                "Cannot tidy %s - not a clang compile command", self.file)
            return

        if compiler in ['gcc', 'g++'] and gcc_sysroot:
            self.clang_arguments.insert(0, '--sysroot='+gcc_sysroot)

    @property
    def full_path(self):
        return os.path.abspath(os.path.join(self.directory, self.file))

    def ExportFixesTo(self, f: str):
        self.tidy_arguments.append("--export-fixes")
        self.tidy_arguments.append(f)

    def SetChecks(self, checks: str):
        self.tidy_arguments.append("--checks")
        self.tidy_arguments.append(checks)

    def Check(self):
        logging.debug("Running tidy on %s from %s", self.file, self.directory)
        try:
            cmd = ["clang-tidy", self.file] + \
                self.tidy_arguments + ["--"] + self.clang_arguments
            logging.debug("Executing: %r" % cmd)

            proc = subprocess.Popen(
                cmd,
                cwd=self.directory,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
            )
            output, err = proc.communicate()
            if output:
                # Output generally contains validation data. Print it out as-is
                logging.info("TIDY %s: %s", self.file, output.decode("utf-8"))

            if err:
                # Most (all?) of our files do contain errors in system-headers so lines like these
                # are expected:
                #
                # ```
                # 59 warnings generated.
                # Suppressed 59 warnings (59 in non-user code).
                # Use -header-filter=.* to display errors from all non-system headers.
                # Use -system-headers to display errors from system headers as well.
                # ```
                #
                # The list below ignores those expected output lines.
                skip_strings = [
                    "warnings generated",
                    "in non-user code",
                    "Use -header-filter=.* to display errors from all non-system headers.",
                    "Use -system-headers to display errors from system headers as well.",
                ]

                for line in err.decode('utf-8').split('\n'):
                    line = line.strip()

                    if any(map(lambda s: s in line, skip_strings)):
                        continue

                    if not line:
                        continue  # no empty lines

                    logging.warning('TIDY %s: %s', self.file, line)

            if proc.returncode != 0:
                if proc.returncode < 0:
                    logging.error(
                        "Failed %s with signal %d", self.file, -proc.returncode
                    )
                else:
                    logging.warning(
                        "Tidy %s ended with code %d", self.file, proc.returncode
                    )
                return TidyResult(self.full_path, False)
        except Exception:
            traceback.print_exc()
            return TidyResult(self.full_path, False)

        return TidyResult(self.full_path, True)


class TidyState:
    def __init__(self):
        self.successes = 0
        self.failures = 0
        self.lock = threading.Lock()
        self.failed_files = []

    def Success(self):
        with self.lock:
            self.successes += 1

    def Failure(self, path: str):
        with self.lock:
            self.failures += 1
            self.failed_files.append(path)
            logging.error("Failed to process %s", path)


def find_darwin_gcc_sysroot():
    for line in subprocess.check_output('xcodebuild -sdk -version'.split()).decode('utf8').split('\n'):
        if not line.startswith('Path: '):
            continue
        path = line[line.find(': ')+2:]
        if '/MacOSX.platform/' not in path:
            continue
        logging.info("Found %s" % path)
        return path

    # A hard-coded value that works on default installations
    logging.warning("Using default platform sdk path. This may be incorrect.")
    return '/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk'


class ClangTidyRunner:
    """Handles running clang-tidy"""

    def __init__(self):
        self.entries = []
        self.state = TidyState()
        self.fixes_file = None
        self.fixes_temporary_file_dir = None
        self.gcc_sysroot = None
        self.file_names_to_check = set()

        if sys.platform == 'darwin':
            # Darwin gcc invocation will auto select a system root, however clang requires an explicit path since
            # we are using the built-in pigweed clang-tidy.
            logging.info(
                'Searching for a MacOS system root for gcc invocations...')
            self.gcc_sysroot = find_darwin_gcc_sysroot()
            logging.info('  Chose: %s' % self.gcc_sysroot)

    def AddDatabase(self, compile_commands_json):
        database = json.load(open(compile_commands_json))

        for entry in database:
            item = ClangTidyEntry(entry, self.gcc_sysroot)
            if not item.valid:
                continue

            if item.file in self.file_names_to_check:
                logging.info('Ignoring additional request for checking %s', item.file)
                continue

            self.file_names_to_check.add(item.file)
            self.entries.append(item)

    def Cleanup(self):
        if self.fixes_temporary_file_dir:
            all_diagnostics = []

            # When running over several files, fixes may be applied to the same
            # file over and over again, like 'append override' can result in the
            # same override being appended multiple times.
            already_seen = set()
            for name in glob.iglob(
                os.path.join(self.fixes_temporary_file_dir.name, "*.yaml")
            ):
                content = yaml.safe_load(open(name, "r"))
                if not content:
                    continue
                diagnostics = content.get("Diagnostics", [])

                # Allow all diagnostics for distinct paths to be applied
                # at once but never again for future paths
                for d in diagnostics:
                    if d['DiagnosticMessage']['FilePath'] not in already_seen:
                        all_diagnostics.append(d)

                # in the future assume these files were already processed
                for d in diagnostics:
                    already_seen.add(d['DiagnosticMessage']['FilePath'])

            if all_diagnostics:
                with open(self.fixes_file, "w") as out:
                    yaml.safe_dump(
                        {"MainSourceFile": "", "Diagnostics": all_diagnostics}, out
                    )
            else:
                open(self.fixes_file, "w").close()

            logging.info(
                "Cleaning up directory: %r", self.fixes_temporary_file_dir.name
            )
            self.fixes_temporary_file_dir.cleanup()
            self.fixes_temporary_file_dir = None

    def ExportFixesTo(self, f):
        # use absolute path since running things will change working directories
        self.fixes_file = os.path.abspath(f)
        self.fixes_temporary_file_dir = tempfile.TemporaryDirectory(
            prefix="tidy-", suffix="-fixes"
        )

        logging.info(
            "Storing temporary fix files into %s", self.fixes_temporary_file_dir.name
        )
        for idx, e in enumerate(self.entries):
            e.ExportFixesTo(
                os.path.join(
                    self.fixes_temporary_file_dir.name, "fixes%d.yaml" % (
                        idx + 1,)
                )
            )

    def SetChecks(self, checks: str):
        for e in self.entries:
            e.SetChecks(checks)

    def FilterEntries(self, f):
        for e in self.entries:
            if not f(e):
                logging.info("Skipping %s in %s", e.file, e.directory)
        self.entries = [e for e in self.entries if f(e)]

    def CheckThread(self, task_queue):
        while True:
            entry = task_queue.get()
            status = entry.Check()

            if status.ok:
                self.state.Success()
            else:
                self.state.Failure(status.path)

            task_queue.task_done()

    def Check(self):
        count = multiprocessing.cpu_count()
        task_queue = queue.Queue(count)

        for _ in range(count):
            t = threading.Thread(target=self.CheckThread, args=(task_queue,))
            t.daemon = True
            t.start()

        for e in self.entries:
            task_queue.put(e)
        task_queue.join()

        logging.info("Successfully processed %d path(s)", self.state.successes)
        if self.state.failures:
            logging.warning("Failed to process %d path(s)", self.state.failures)
            logging.warning("The following paths failed clang-tidy checks:")
            for name in self.state.failed_files:
                logging.warning("  - %s", name)

        return self.state.failures == 0


# Supported log levels, mapping string values required for argument
# parsing into logging constants
__LOG_LEVELS__ = {
    "debug": logging.DEBUG,
    "info": logging.INFO,
    "warn": logging.WARN,
    "fatal": logging.FATAL,
}


@click.group(chain=True)
@click.option(
    "--compile-database",
    default=[],
    multiple=True,
    help="Path to `compile_commands.json` to use for executing clang-tidy.",
)
@click.option(
    "--file-include-regex",
    default="/(src|examples)/",
    help="regular expression to apply to the file paths for running.",
)
@click.option(
    "--file-exclude-regex",
    # NOTE: if trying '/third_party/' note that a lot of sources are routed through
    # paths like `../../examples/chip-tool/third_party/connectedhomeip/src/`
    default="/(repo|zzz_generated)/",
    help="Regular expression to apply to the file paths for running. Skip overrides includes.",
)
@click.option(
    "--log-level",
    default="INFO",
    type=click.Choice(__LOG_LEVELS__.keys(), case_sensitive=False),
    help="Determines the verbosity of script output.",
)
@click.option(
    "--no-log-timestamps",
    default=False,
    is_flag=True,
    help="Skip timestaps in log output",
)
@click.option(
    "--export-fixes",
    default=None,
    type=click.Path(),
    help="Where to export fixes to apply.",
)
@click.option(
    "--checks",
    default=None,
    type=str,
    help="Checks to run (passed in to clang-tidy). If not set the .clang-tidy file is used.",
)
@click.pass_context
def main(
    context,
    compile_database,
    file_include_regex,
    file_exclude_regex,
    log_level,
    no_log_timestamps,
    export_fixes,
    checks,
):
    log_fmt = "%(asctime)s %(levelname)-7s %(message)s"
    if no_log_timestamps:
        log_fmt = "%(levelname)-7s %(message)s"
    coloredlogs.install(level=__LOG_LEVELS__[log_level], fmt=log_fmt)

    if not compile_database:
        logging.warning(
            "Compilation database file not provided. Searching for first item in ./out"
        )
        compile_database = next(
            glob.iglob("./out/**/compile_commands.json", recursive=True)
        )
        if not compile_database:
            raise Exception("Could not find `compile_commands.json` in ./out")
        logging.info("Will use %s for compile", compile_database)
        compile_database = [compile_database]

    context.obj = runner = ClangTidyRunner()

    @context.call_on_close
    def cleanup():
        runner.Cleanup()

    for name in compile_database:
        runner.AddDatabase(name)

    if file_include_regex:
        r = re.compile(file_include_regex)
        runner.FilterEntries(lambda e: r.search(e.file))

    if file_exclude_regex:
        r = re.compile(file_exclude_regex)
        runner.FilterEntries(lambda e: not r.search(e.file))

    if export_fixes:
        runner.ExportFixesTo(export_fixes)

    if checks:
        runner.SetChecks(checks)

    for e in context.obj.entries:
        logging.info("Will tidy %s", e.full_path)


@main.command("check", help="Run clang-tidy check")
@click.pass_context
def cmd_check(context):
    if not context.obj.Check():
        sys.exit(1)


@main.command("fix", help="Run check followd by fix")
@click.pass_context
def cmd_fix(context):
    runner = context.obj
    with tempfile.TemporaryDirectory(prefix="tidy-apply-fixes") as tmpdir:
        if not runner.fixes_file:
            runner.ExportFixesTo(os.path.join(tmpdir, "fixes.tmp"))

        runner.Check()
        runner.Cleanup()

        if runner.state.failures:
            fixes_yaml = os.path.join(tmpdir, "fixes.yaml")
            with open(fixes_yaml, "w") as out:
                out.write(open(runner.fixes_file, "r").read())

            logging.info("Applying fixes in %s", tmpdir)
            subprocess.check_call(["clang-apply-replacements", tmpdir])
        else:
            logging.info("No failures detected, no fixes to apply.")


if __name__ == "__main__":
    main(auto_envvar_prefix='CHIP')
