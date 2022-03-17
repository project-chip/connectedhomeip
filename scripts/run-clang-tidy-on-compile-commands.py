#!/usr/bin/env python
#
# Runs clang-tidy on files based on a `compile_commands.json` file
#

"""
Run clang-tidy in parallel on compile databases.

Example run:

# This prepares the build. NOTE this is `build` not `gen` because the build
# steps generates required header files (this can be simplified if needed
# to invoke ninja to compile only generated files if needed)

./scripts/build/build_examples.py --target linux-x64-chip-tool-clang build

# Actually running clang-tidy to check status

./scripts/run-clang-tidy-on-compile-commands.py check

"""

import build
import click
import coloredlogs
import glob
import json
import logging
import multiprocessing
import os
import re
import shlex
import subprocess
import sys
import threading
import traceback
import queue


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

    def __init__(self, json_entry):
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

        if command.startswith("clang++ ") or command.startswith("clang "):
            self.valid = True
            self.clang_arguments = shlex.split(command[command.find(" "):])
        else:
            logging.warning(
                "Cannot tidy %s - not a clang compile command", self.file)
            return

    @property
    def full_path(self):
        return os.path.abspath(os.path.join(self.directory, self.file))

    def ExportFixesTo(self, f: str):
        self.tidy_arguments.append("--export-fixes")
        self.tidy_arguments.append(f)

    def Check(self):
        logging.debug("Running tidy on %s from %s", self.file, self.directory)
        try:
            proc = subprocess.Popen(
                ["clang-tidy", self.file, "--"] + self.clang_arguments,
                cwd=self.directory,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
            )
            output, err = proc.communicate()
            if output:
                logging.info("TIDY %s: %s", self.file, output.decode("utf-8"))

            if err:
                logging.warning("TIDY %s: %s", self.file, err.decode("utf-8"))

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
        except:
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


class ClangTidyRunner:
    """Handles running clang-tidy"""

    def __init__(self):
        self.entries = []
        self.state = TidyState()

    def AddDatabase(self, compile_commands_json):
        database = json.load(open(compile_commands_json))

        for entry in database:
            item = ClangTidyEntry(entry)
            if not item.valid:
                continue

            self.entries.append(item)

    def ExportFixesTo(self, f):
        # use absolute path since running things will change working directories
        f = os.path.abspath(f)
        for e in self.entries:
            e.ExportFixesTo(f)

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

        logging.info("Successfully processed %d paths", self.state.successes)
        logging.info("Failed to process %d paths", self.state.failures)
        if self.state.failures:
            for name in self.state.failed_files:
                logging.warning("Failure reported for %s", name)

            sys.exit(1)


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
    help="Where to export fixes to apply. TODO(fix apply not yet implemented).",
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

    context.obj = ClangTidyRunner()

    for name in compile_database:
      context.obj.AddDatabase(name)

    if file_include_regex:
        r = re.compile(file_include_regex)
        context.obj.FilterEntries(lambda e: r.search(e.file))

    if file_exclude_regex:
        r = re.compile(file_exclude_regex)
        context.obj.FilterEntries(lambda e: not r.search(e.file))

    if export_fixes:
        context.obj.ExportFixesTo(export_fixes)

    for e in context.obj.entries:
        logging.info("Will tidy %s", e.full_path)


@main.command("check", help="Run clang-tidy check")
@click.pass_context
def cmd_check(context):
    context.obj.Check()


if __name__ == "__main__":
    main()
