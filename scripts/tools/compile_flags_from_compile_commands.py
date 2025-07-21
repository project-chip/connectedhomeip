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
# requires-python = ">=3.13"
# dependencies = [
#     "click",
#     "coloredlogs",
# ]
# ///
#
# Parse a compile_commands.json and extract relevant compiler arguments to allow
# file compilation for godbolt tests. Use to test out code and generate
# assembly.
import json
import logging
import shlex
import os
import re
from typing import Optional


import coloredlogs
import click

# Supported log levels, mapping string values required for argument
# parsing into logging constants
__LOG_LEVELS__ = {
    "debug": logging.DEBUG,
    "info": logging.INFO,
    "warn": logging.WARN,
    "fatal": logging.FATAL,
}


class CompileCommand:
    def __init__(self, dir, file, command):
        args = shlex.split(command)

        self.dir = dir
        self.file = file
        self.compiler = args[0]
        self.args = []

        index = 1  # start from 1 to remove compiler path
        while index < len(args):
            arg = args[index]
            # Command contains some things we want to strip out:
            #  -c, -o for compile/output, MF and MMD for dependency generation
            if arg in {"-c", "-o", "-MF"}:
                index += 1  # skip this and next argument
            elif arg not in {"-MMD"}:

                # convert relative paths in absolute paths for incldues to absolute
                # so if '-I' is not absolute, adjust
                prefixes = ["-I", "-isystem"]
                for p in prefixes:
                    if not arg.startswith(p):
                        continue
                    if arg.startswith(p + "/"):
                        continue
                    path = arg[len(p):] # full path
                    path = os.path.abspath(os.path.join(self.dir, path))
                    arg = f"{p}{path}"
                    break

                # for defines, we have defines like `-DMBEDTLS_CONFIG_FILE="efr32-chip-mbedtls-config.h"`
                # This does not parse well for godbolt and we have to add extra quotes so that this becomes
                # `-DMBEDTLS_CONFIG_FILE='"efr32-chip-mbedtls-config.h"'`
                if arg.startswith("-D") and "=" in arg and arg.endswith('"'):
                    arg = arg.replace('="', '=\'"') + "'"

                self.args.append(arg)
            index += 1

    def print_flags(self):
        for flag in self.args:
            print(flag)

    def __str__(self):
        return f"[{self.dir}: {self.file}] -> \n      {'\n      '.join(self.args)}"


class ParsedCommands:
    def __init__(self, path: str):
        logging.info("Processing json %s", path)
        self.json_data = json.load(open(path, "r"))
        logging.info("Done. Loaded %d entries", len(self.json_data))


    def find(self, expr: str) -> Optional[CompileCommand]:
        r = re.compile(expr)
        matches = []

        # JSON data is a list of entries:
        # {
        #    "file": the relative path to compile
        #    "directory": the build directory for this file
        #    "command": the build command (generally a arm-none-eabi-g++)
        # }
        for v in self.json_data:
            for _ in re.finditer(r, v["file"]):
                # if at least one match, return it
                matches.append(v)
                break
        if not matches:
            return None
        if len(matches) > 1:
            logging.warning(
                "Multiple matches found: %d matches for %s", len(matches), expr
            )
            for m in matches:
                logging.warning("  match: %s", m["file"])
        entry = matches[0]
        return CompileCommand(entry["directory"], entry["file"], entry["command"])



@click.group()
@click.option(
    "--log-level",
    default="FATAL",
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


if __name__ == "__main__":
    main(auto_envvar_prefix="CHIP")
