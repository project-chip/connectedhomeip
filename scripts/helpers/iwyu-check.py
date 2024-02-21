#!/usr/bin/env -S python3 -B

# Copyright (c) 2021 Project CHIP Authors
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

import contextlib
import glob
import json
import logging
import os
import re
import shutil
import subprocess
import sys
from pathlib import Path

import click
import coloredlogs

# Supported log levels, mapping string values required for argument
# parsing into logging constants
__LOG_LEVELS__ = {
    'debug': logging.DEBUG,
    'info': logging.INFO,
    'warn': logging.WARN,
    'fatal': logging.FATAL,
}

root_dir = os.path.dirname(os.path.realpath(__file__))
proj_root_dir = os.path.join(Path(root_dir).parent.parent)


def find_program(names):
    for name in names:
        found = shutil.which(name)
        if found is not None:
            return found


@click.command()
@click.option(
    '--log-level',
    default='INFO',
    type=click.Choice(__LOG_LEVELS__.keys(), case_sensitive=False),
    help='Determines the verbosity of script output.')
@click.option(
    '--no-log-timestamps',
    default=False,
    is_flag=True,
    help='Skip timestamps in log output')
@click.option(
    '--compile-commands-glob',
    show_default=True,
    default=os.path.join(proj_root_dir, "out", "debug", "compile_commands*.json"),
    help='Set global pattern for compile_commands.json files'
)
@click.option(
    '--scanning-destination',
    show_default=True,
    default=os.path.join(proj_root_dir, "src", "platform"),
    help='Set scanning destination file(s) or directory /ies in project'
)
@click.option(
    '--mapping-file-dir',
    help='Set mapping file directory /ies manually. File should have name iwyu.imp'
)
@click.option(
    '--iwyu-args',
    show_default=True,
    default="-Xiwyu --no_fwd_decls",
    help='Set custom arg(s) for include what you use'
)
@click.option(
    '--clang-args',
    default="",
    help='Set custom arg(s) for clang'
)
def main(compile_commands_glob, scanning_destination, mapping_file_dir,
         iwyu_args, clang_args, log_level, no_log_timestamps):
    # Ensures somewhat pretty logging of what is going on
    log_fmt = '%(asctime)s %(levelname)-7s %(message)s'
    if no_log_timestamps:
        log_fmt = '%(levelname)-7s %(message)s'
    coloredlogs.install(level=__LOG_LEVELS__[log_level], fmt=log_fmt)

    # checking if a program IWYU exists
    iwyu = find_program(('iwyu_tool', 'iwyu_tool.py'))

    if iwyu is None:
        logging.error("Can't find IWYU")
        sys.exit(1)

    # For iterating how many files had problems with includes
    warning_in_files = 0

    platform = ""
    compile_commands_glob = glob.glob(compile_commands_glob)

    if not compile_commands_glob:
        logging.error("Can't find compile_commands.json file(s)")
        sys.exit(1)

    for compile_commands in compile_commands_glob:

        compile_commands_path = os.path.dirname(compile_commands)
        compile_commands_file = os.path.join(compile_commands_path, "compile_commands.json")
        logging.debug("Copy compile command file %s to %s", compile_commands, compile_commands_file)

        with contextlib.suppress(shutil.SameFileError):
            shutil.copyfile(compile_commands, compile_commands_file)

        # Prase json file for find target name
        with open(compile_commands, 'r') as json_data:
            json_data = json.load(json_data)

        for key in json_data:
            find_re = re.search(r'^.*/src/platform*?\/(.*)/.*$', key['file'])
            if find_re is not None:
                platform = find_re.group(1)
                break
        if not platform:
            logging.error("Can't find platform")
            sys.exit(1)

        if not mapping_file_dir:
            mapping_file_dir = os.path.join(root_dir, "platforms", platform)

        # Platform specific clang arguments, as some platform
        # may needed some hacks for clang compiler for work properly.
        platform_clang_args = []

        if platform == "Tizen":
            platform_clang_args = [
                "--target=arm-linux-gnueabi",
                "-I$TIZEN_SDK_TOOLCHAIN/arm-tizen-linux-gnueabi/include/c++/9.2.0",
                "-I$TIZEN_SDK_TOOLCHAIN/arm-tizen-linux-gnueabi/include/c++/9.2.0/arm-tizen-linux-gnueabi",
                "-I$TIZEN_SDK_TOOLCHAIN/lib/gcc/arm-tizen-linux-gnueabi/9.2.0/include",
            ]

        # TODO: Add another platform for easy scanning
        # Actually works scanning for platform: tizen, darwin, linux other not tested yet.

        command_arr = [
            iwyu,
            "-p", compile_commands_path, scanning_destination,
            "--", iwyu_args,
            "-Xiwyu", "--mapping_file=" + mapping_file_dir + "/iwyu.imp",
        ] + platform_clang_args + [clang_args]

        logging.info("Used compile commands: %s", compile_commands)
        logging.info("Scanning includes for platform: %s", platform)
        logging.info("Scanning destination: %s", scanning_destination)

        logging.debug("Command: %s", " ".join(command_arr))
        status = subprocess.Popen(" ".join(command_arr),
                                  shell=True,
                                  text=True,
                                  stdout=subprocess.PIPE,
                                  stderr=subprocess.STDOUT)

        logging.info("============== IWYU output start  ================")

        logger = logging.info
        for line in status.stdout:
            line = line.rstrip()

            if re.match(r"^warning:.*$", line):
                logger = logging.warning
            elif re.match(r"^.*([A-Za-z0-9]+(/[A-Za-z0-9]+)+)\.cpp should [a-zA-Z]+ these lines:$", line):
                logger = logging.warning
            elif re.match(r"^.*([A-Za-z0-9]+(/[A-Za-z0-9]+)+)\.[a-zA-Z]+ has correct #includes/fwd-decls\)$", line):
                logger = logging.info
            elif re.match(r"^The full include-list for .*$", line):
                logger = logging.warning
                warning_in_files += 1

            logger("%s", line)

        logging.info("============== IWYU output end  ================")

    if warning_in_files:
        logging.error("Number of files with include issues: %d", warning_in_files)
        sys.exit(2)
    else:
        logging.info("Every include looks good!")


if __name__ == '__main__':
    main(auto_envvar_prefix='CHIP')
