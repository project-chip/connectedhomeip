#!/usr/bin/env -S python3 -B

# Copyright (c) 2025 Project CHIP Authors
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
import logging
import os
import sys

import click
import coloredlogs
import jinja2

# Supported log levels, mapping string values required for argument
# parsing into logging constants
__LOG_LEVELS__ = {
    "debug": logging.DEBUG,
    "info": logging.INFO,
    "warn": logging.WARN,
    "fatal": logging.FATAL,
}

# This repesents the code that is generated according to
# https://clang.llvm.org/docs/SourceBasedCodeCoverage.html#using-the-profiling-runtime-without-static-initializers
#
# So that the output of coverage is customized and does not go to `default.profraw`
_CPP_TEMPLATE = """\
extern "C" {

int __llvm_profile_runtime = 0;
void __llvm_profile_initialize_file();
void __llvm_profile_write_file();
void __llvm_profile_set_filename(const char *);

} // extern "C"

struct StaticInitLLVMProfile {
    StaticInitLLVMProfile() {
        __llvm_profile_set_filename("{{raw_profile_filename}}");
    }
    ~StaticInitLLVMProfile() {
        __llvm_profile_write_file();
    }
} gInitLLVMProfilingPaths;
"""


@click.command()
@click.option(
    "--log-level",
    default="INFO",
    type=click.Choice([k for k in __LOG_LEVELS__.keys()], case_sensitive=False),
    help="Determines the verbosity of script output.",
)
@click.option(
    "--no-log-timestamps",
    default=False,
    is_flag=True,
    help="Skip timestaps in log output",
)
@click.option(
    "--output",
    help="What file to output when runnning under clang profiling",
)
@click.option(
    "--raw-profile-filename",
    help="Filename to use for output",
)
def main(log_level, no_log_timestamps, output, raw_profile_filename):
    log_fmt = "%(asctime)s %(levelname)-7s %(message)s"
    if no_log_timestamps:
        log_fmt = "%(levelname)-7s %(message)s"
    coloredlogs.install(level=__LOG_LEVELS__[log_level], fmt=log_fmt)

    expected_output = jinja2.Template(_CPP_TEMPLATE).render(raw_profile_filename=raw_profile_filename)
    if os.path.exists(output):
        with open(output, 'rt') as f:
            if f.read() == expected_output:
                logging.info("File %s is already as expected. Will not re-write", output)
                sys.exit(0)

    logging.info("Writing output to %s (profile name: %s)", output, raw_profile_filename)
    with open(output, "wt") as f:
        f.write(expected_output)

    logging.debug("Writing completed")


if __name__ == "__main__":
    main(auto_envvar_prefix="CHIP")
