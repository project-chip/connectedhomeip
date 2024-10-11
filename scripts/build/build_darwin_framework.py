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
import glob
import os
import platform
from subprocess import PIPE, Popen


def get_file_from_pigweed(name):
    CHIP_ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..'))
    PIGWEED = os.path.join(CHIP_ROOT, ".environment/cipd/packages/pigweed")

    pattern = os.path.join(PIGWEED, '**', name)
    for filename in glob.glob(pattern, recursive=True):
        if os.path.isfile(filename):
            return filename


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

    if returncode != 0:
        # command_log is binary, so decoding as utf-8 might technically fail.  We don't want
        # to throw on that.
        try:
            print("Failure log: {}".format(command_log.decode()))
        except Exception:
            pass

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
        args.target_sdk,
        '-project',
        args.project_path,
        '-derivedDataPath',
        abs_path,
        "ARCHS={}".format(args.target_arch),
    ]

    if args.target_sdk != "macosx":
        command += [
            # Build Matter.framework as a static library
            "SUPPORTS_TEXT_BASED_API=NO",
            "MACH_O_TYPE=staticlib",
            # Change visibility flags such that both darwin-framework-tool and Matter.framework
            # are built with the same flags.
            "GCC_INLINES_ARE_PRIVATE_EXTERN=NO",
            "GCC_SYMBOLS_PRIVATE_EXTERN=NO",
        ]

    options = {
        'CHIP_INET_CONFIG_ENABLE_IPV4': args.ipv4,
        'CHIP_IS_ASAN': args.asan,
        'CHIP_IS_BLE': args.ble,
        'CHIP_IS_CLANG': args.clang,
        'CHIP_ENABLE_ENCODING_SENTINEL_ENUM_VALUES': args.enable_encoding_sentinel_enum_values
    }
    for option in options:
        command += ["{}={}".format(option, "YES" if options[option] else "NO")]

    defines = 'GCC_PREPROCESSOR_DEFINITIONS=${inherited} MTR_NO_AVAILABILITY=1'

    command += [defines]

    cflags = ["${inherited}"]
    ldflags = ["${inherited}"]

    if args.clang:
        command += [
            "CC={}".format(get_file_from_pigweed("clang")),
            "CXX={}".format(get_file_from_pigweed("clang++")),
            "COMPILER_INDEX_STORE_ENABLE=NO",
            "CLANG_ENABLE_MODULES=NO",
        ]

        ldflags += [
            "-nostdlib++",
            get_file_from_pigweed("libc++.a"),
        ]

    if args.asan:
        flags = ["-fsanitize=address", "-fno-omit-frame-pointer"]
        cflags += flags
        ldflags += flags

        if args.clang:
            ldflags += [
                get_file_from_pigweed("libclang_rt.asan_osx_dynamic.dylib")
            ]

    if args.enable_encoding_sentinel_enum_values:
        cflags += ["-DCHIP_CONFIG_IM_ENABLE_ENCODING_SENTINEL_ENUM_VALUES=1"]

    if args.compdb:
        cflags += ["-gen-cdb-fragment-path ", abs_path + '/compdb']

    command += ["OTHER_CFLAGS=" + ' '.join(cflags), "OTHER_LDFLAGS=" + ' '.join(ldflags)]
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
    parser.add_argument("--target_sdk",
                        default="macosx",
                        help="Set the target sdk",
                        required=False,
                        )
    parser.add_argument("--target_arch",
                        default=platform.machine(),
                        help="Set the target architecture",
                        required=False,
                        )
    parser.add_argument("--log_path",
                        help="Output log file destination",
                        required=True)
    parser.add_argument('--ipv4', action=argparse.BooleanOptionalAction)
    parser.add_argument('--asan', action=argparse.BooleanOptionalAction)
    parser.add_argument('--ble', action=argparse.BooleanOptionalAction)
    parser.add_argument('--clang', action=argparse.BooleanOptionalAction)
    parser.add_argument('--enable-encoding-sentinel-enum-values', action=argparse.BooleanOptionalAction)
    parser.add_argument('--compdb', action=argparse.BooleanOptionalAction)

    args = parser.parse_args()
    build_darwin_framework(args)
