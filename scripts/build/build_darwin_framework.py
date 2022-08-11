#!/usr/bin/env -S python3 -B
# Copyright (c) 2022 Project Matter Authors
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
import argparse
import os
from subprocess import PIPE, Popen
import platform


def run_command(command):
    returncode = -1
    command_log = b''
    print("Running {}".format(command))
    with Popen(command, cwd=os.getcwd(), stdout=PIPE, stderr=PIPE) as process:
        for line in process.stdout:
            command_log += line

        for line in process.stderr:
            command_log += line

        process.wait()
        returncode = process.returncode

    with open(args.log_path, "wb") as f:
        f.write(command_log)
    return returncode


def build_darwin_framework(args):
    abs_path = os.path.abspath(args.out_path)
    if not os.path.exists(abs_path):
        os.mkdir(abs_path)

    command = [
        'xcodebuild',
        '-scheme',
        args.target,
        '-sdk',
        'macosx',
        '-project',
        args.project_path,
        '-derivedDataPath',
        abs_path,
        "PLATFORM_PREFERRED_ARCH={}".format(platform.machine())
    ]
    command_result = run_command(command)

    print("Build Framework Result: {}".format(command_result))
    exit(command_result)


if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        description="Build the Matter Darwin framework")
    parser.add_argument(
        "--project_path",
        default="src/darwin/Framework/Matter.xcodeproj",
        help="Set the project path",
        required=True,
    )
    parser.add_argument(
        "--out_path",
        default="/tmp/macos_framework_output",
        help="Output lpath for framework",
        required=True,
    )
    parser.add_argument("--target",
                        default="Matter",
                        help="Name of target to build",
                        required=True)
    parser.add_argument("--log_path",
                        help="Output log file destination",
                        required=True)

    args = parser.parse_args()
    build_darwin_framework(args)
