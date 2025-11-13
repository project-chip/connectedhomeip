#!/usr/bin/env -S python3 -B
#
#    Copyright (c) 2025 Project CHIP Authors
#
#    Licensed under the Apache License, Version 2.0 (the "License");
#    you may not use this file except in compliance with the License.
#    You may obtain a copy of the License at
#
#        http://www.apache.org/licenses/LICENSE-2.0
#
#    Unless required by applicable law or agreed to in writing, software
#    distributed under the License is distributed on an "AS IS" BASIS,
#    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#    See the License for the specific language governing permissions and
#    limitations under the License.
#
# /// script
# requires-python = ">=3.11"
# dependencies = [
#     "click",
#     "coloredlogs",
# ]
# ///
#
# Parse a compile_commands.json and extract relevant compiler arguments to allow
# file compilation for godbolt tests. Use to test out code and generate
# assembly.
#
# Example usages (via uv run script to auto-create an env - see https://github.com/astral-sh/uv)
#
#  - Get the flags corresponding to a file matching the regex `basic-information` (first match)
#     uv run --script ./compile_flags_from_compile_commands.py -c sme_path/compile_commands.json flags basic-information
#
#  - Get the flags corresponding to a file matching the regex `basic-information` (first match) and save it to the clipboard
#     uv run --script ./compile_flags_from_compile_commands.py      \
#        -c some_path/compile_commands.json flags basic-information \
#        | wl-copy
#
#  - List all files in compile_commands
#     uv run --script ./compile_flags_from_compile_commands.py -c some_path/compile_commands.json files
#
#  - List all files in compile_commands that match an expression
#     uv run --script ./compile_flags_from_compile_commands.py -c some_path/compile_commands.json files --matching basic-info
#
import json
import logging
import os
import re
import shlex
from typing import Any, List, Optional

import click
import coloredlogs

# Supported log levels, mapping string values required for argument
# parsing into logging constants
__LOG_LEVELS__ = {
    "debug": logging.DEBUG,
    "info": logging.INFO,
    "warn": logging.WARNING,
    "fatal": logging.FATAL,
}


class CompileCommand:
    def __init__(self, dir, file, command):
        args = shlex.split(command)

        self.dir = dir
        self.file = file
        self.compiler = args[0]
        self.args = []

        args = iter(args[1:])
        for arg in args:
            # Command contains some things we want to strip out:
            #  -c, -o for compile/output, MF and MMD for dependency generation
            if arg in {"-c", "-o", "-MF"}:
                next(args, None)  # skip next argument
                continue
            if arg in {"-MMD"}:
                continue

            # convert relative paths in absolute paths for incldues to absolute
            # so if '-I' is not absolute, adjust
            prefixes = ["-I", "-isystem"]
            for p in prefixes:
                if not arg.startswith(p):
                    continue
                if arg.startswith(p + "/"):
                    continue
                path = arg[len(p):]  # full path
                path = os.path.abspath(os.path.join(self.dir, path))
                arg = f"{p}{path}"
                break

            # for defines, we have defines like `-DMBEDTLS_CONFIG_FILE="efr32-chip-mbedtls-config.h"`
            # This does not parse well for godbolt and we have to add extra quotes so that this becomes
            # `-DMBEDTLS_CONFIG_FILE='"efr32-chip-mbedtls-config.h"'`
            if arg.startswith("-D") and "=" in arg and arg.endswith('"'):
                arg = arg.replace('="', "='\"") + "'"

            self.args.append(arg)

    def print_flags(self):
        for flag in self.args:
            print(flag)

    def __str__(self):
        prefix = "\n      "  # have all args on newlines with an indent
        return f"[{self.dir}: {self.file}] -> {prefix}{prefix.join(self.args)}"


class ParsedCommands:
    def __init__(self, path: str):
        logging.info("Processing json %s", path)

        with open(path, "r") as f:
            self.json_data = json.load(f)

        # data is a list of entries. We sort them since this as a sideffect places `/src/` before
        # '/third_party/' and when multi-matching things, this makes for better best-guess matches
        self.json_data.sort(key=lambda a: a["file"])

        logging.info("Done. Loaded %d entries", len(self.json_data))

    def all_matching(self, reg_expr: str) -> List[Any]:
        r = re.compile(reg_expr)

        # JSON data is a list of entries:
        # {
        #    "file": the relative path to compile
        #    "directory": the build directory for this file
        #    "command": the build command (generally a arm-none-eabi-g++)
        # }
        return [e for e in self.json_data if re.search(r, e["file"])]

    def find(self, reg_expr: str) -> Optional[CompileCommand]:
        """
        Returns the FIRST match of reg_expr, processed as a CompileCommmand
        """
        matches = self.all_matching(reg_expr)

        if not matches:
            return None

        if len(matches) > 1:
            logging.warning(
                "Multiple matches found: %d matches for %s", len(matches), reg_expr
            )
            for m in matches:
                logging.warning("  match: %s", m["file"])

        entry = matches[0]
        return CompileCommand(entry["directory"], entry["file"], entry["command"])


@click.group()
@click.option(
    "--log-level",
    default="FATAL",  # default on purpose so that wl-copy/xclip just works
    show_default=True,
    type=click.Choice(list(__LOG_LEVELS__.keys()), case_sensitive=False),
    help="Determines the verbosity of script output.",
)
@click.option(
    "-c",
    "--compile-commands",
    required=True,
    type=click.Path(exists=True, file_okay=True, dir_okay=False, readable=True),
    help="Path to the compile_commands.json file.",
)
@click.pass_context
def main(ctx, log_level, compile_commands):
    log_fmt = "%(asctime)s %(levelname)-7s %(message)s"
    coloredlogs.install(level=__LOG_LEVELS__[log_level], fmt=log_fmt)
    ctx.obj = ParsedCommands(compile_commands)


@main.command()
@click.argument("file_expr")
@click.pass_obj
def flags(compile_commands, file_expr):
    f = compile_commands.find(file_expr)
    if f is None:
        raise Exception(f"Failed to find file matching {file_expr}")
    logging.info("Compiler settings for %s", f.file)
    logging.debug("File data: %s", f)
    f.print_flags()


@main.command()
@click.argument("file_expr", required=False)
@click.pass_obj
def files(compile_commands, file_expr):
    if file_expr:
        entries = compile_commands.all_matching(file_expr)
    else:
        entries = compile_commands.json_data

    for entry in entries:
        print(entry["file"])


if __name__ == "__main__":
    main(auto_envvar_prefix="CHIP")
