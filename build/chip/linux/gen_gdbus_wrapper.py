#!/usr/bin/env python

# Copyright (c) 2020 Project CHIP Authors
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
import subprocess
import sys


def main(argv):
    parser = argparse.ArgumentParser(description=("Generate dbus bindings."))

    parser.add_argument("--input_file",
                        required=True,
                        help="The dbus service definition XML file.")

    parser.add_argument(
        "--output_c",
        help="The source file to generate containing the GDBus proxy implementation"
    )

    parser.add_argument(
        "--output_h",
        help="The header file to generate containing the GDBus proxy interface"
    )

    parser.add_argument("--c-namespace",
                        default=None,
                        help="Prefix APIs with C namespace")

    parser.add_argument("--interface-prefix",
                        default=None,
                        help="Add interface prefix")

    parser.add_argument("--c-generate-object-manager",
                        default=False, action='store_true',
                        help="Generate object manager")

    options = parser.parse_args(argv)

    extra_args = []
    if options.c_namespace:
        extra_args += ["--c-namespace", options.c_namespace]

    if options.interface_prefix:
        extra_args += ["--interface-prefix", options.interface_prefix]

    if options.c_generate_object_manager:
        extra_args += ["--c-generate-object-manager"]

    if options.output_c:
        gdbus_args = ["gdbus-codegen", "--body", "--output", options.output_c
                      ] + extra_args + [options.input_file]
        subprocess.check_call(gdbus_args)
        sed_args = ["sed", "-i",
                    r"s/config\.h/BuildConfig.h/g", options.output_c]
        if sys.platform == "darwin":
            sed_args = ["sed", "-i", "",
                        r"s/config\.h/BuildConfig.h/g", options.output_c]
        subprocess.check_call(sed_args)

    if options.output_h:
        gdbus_args = [
            "gdbus-codegen", "--header", "--output", options.output_h
        ] + extra_args + [options.input_file]
        subprocess.check_call(gdbus_args)

    return 0


if __name__ == '__main__':
    sys.exit(main(sys.argv[1:]))
