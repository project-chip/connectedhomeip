#!/usr/bin/env python
#
# Copyright (c) 2022 Project CHIP Authors
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
#
"""SS CGS generator

This script generates SS CGS and HDF files.

"""

import argparse
import io
import os
import pathlib
import sys

from collections import OrderedDict

THREAD_FACTORY_KEY_BASE = 0x2000
MATTER_FACTORY_KEY_BASE = 0x2100


def main():
    args = parse_args()

    configs = gen_thread_factory_config()

    gen_ss_hdf(args.ss_hdf, configs)
    gen_ss_cgs(args.ss_cgs, args.ss_hdf, configs)
    gen_empty_cgs(args.empty_cgs, args.ss_hdf, configs)


def parse_args():
    parser = argparse.ArgumentParser()
    parser.add_argument("--ss_hdf", required=True, type=pathlib.Path)
    parser.add_argument("--ss_cgs", required=True, type=pathlib.Path)
    parser.add_argument("--empty_cgs", required=True, type=pathlib.Path)
    return parser.parse_args()


def gen_thread_factory_config() -> OrderedDict:
    configs = OrderedDict()
    configs["ExtendedAddress"] = {"key": THREAD_FACTORY_KEY_BASE, "value": os.urandom(8)}
    return configs


def gen_ss_hdf(path: pathlib.Path, configs: OrderedDict):
    with open(str(path), mode="w") as hdf:
        hdf.write("""\
SECTION "Config Data Entries" 0
{
""")
        for name, config in configs.items():
            if "value" in config:
                write_hdf_command(hdf, name, config["key"])

        hdf.write("}")


def write_hdf_command(hdf: io.TextIOBase, name: str, key: int):
    hdf.write(f"""\
    COMMAND "{name}" {key:#x}
    {{
        PARAM "Data"
            uint8[0xff00] omit_pad_bytes;
    }}
""")


def gen_ss_cgs(cgs_path: pathlib.Path, hdf_path: pathlib.Path, configs: OrderedDict):
    with open(str(cgs_path), mode="w") as cgs:
        write_cgs_header(cgs, hdf_path)

        for name, config in configs.items():
            if "value" in config:
                write_cgs_data(cgs, name, config["value"])


def gen_empty_cgs(cgs_path: pathlib.Path, hdf_path: pathlib.Path, configs: OrderedDict):
    with open(str(cgs_path), mode="w") as cgs:
        write_cgs_header(cgs, hdf_path)

        for name, config in configs.items():
            if "value" in config:
                write_cgs_data(cgs, name)
                break


def write_cgs_header(cgs: io.TextIOBase, hdf_path: pathlib.Path):
    cgs.write(f"""\
# DO NOT EDIT MANUALLY! FW2 VERSION INCLUDESSCRC32 INCLUDEDSCRC32
DEFINITION <{hdf_path}>
""")


def write_cgs_data(cgs: io.TextIOBase, name: str, data: bytes = b""):
    cgs.write(f"""\
ENTRY "{name}"
{{
    "Data" =
    COMMENTED_BYTES
    {{
        <hex>
""")

    indent = "        "
    for i in range(0, len(data), 16):
        cgs.write(indent + data[i:i+16].hex(' ') + "\n")

    cgs.write("""\
    } END_COMMENTED_BYTES
}
""")


if __name__ == "__main__":
    main()
