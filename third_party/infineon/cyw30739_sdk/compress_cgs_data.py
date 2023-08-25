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
"""CGS data entry compressor

This script compresses "Data" entries into "Compressed data" entries
from the input CGS file to the output CGS file with the given LZSS tool.

"Data" entries that are smaller than 512 bytes would be skipped.

"""

import argparse
import io
import pathlib
import re
import subprocess
import sys
from collections import namedtuple

Item = namedtuple('Item', 'line type nested_data')
Entry = namedtuple('Entry', 'items name remark')
CommentedBytes = namedtuple('CommentedBytes', 'items data')


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--output", required=True, type=pathlib.Path)
    parser.add_argument("--input", required=True, type=pathlib.Path)
    parser.add_argument("--lzss_tool", required=True, type=pathlib.Path)

    option = parser.parse_args()

    cgs = parse_cgs(option.input)
    compress_cgs(option, cgs)
    export_cgs(option.output, cgs)


def parse_cgs(file_name):
    cgs = []

    with open(file_name, mode="r") as file:
        for line in file:
            if len(line) == 0:
                break

            striped_line = line.strip()
            item_type = None
            nested_data = None

            if striped_line.startswith('ENTRY'):
                item_type = "entry"
                nested_data = parse_entry(file, line)
            elif striped_line.startswith('#') or striped_line.startswith('//'):
                item_type = "comment"
            elif striped_line.startswith('DEFINITION'):
                item_type = "definition"

            cgs.append(Item(line, item_type, nested_data))

    return cgs


def parse_entry(file: io.TextIOBase, line):

    # compile the regex for extracting name and remark of the entry.
    data_re = re.compile(r"""
      ^\s*               # Allow leading spaces
      ENTRY\s+"([^"]+)"  # Parse the entry name
      (?:                # Optional non-capturing part begin
        \s*=\s*          # Allow leading and trailing spaces
        "([^"]+)"        # Parse the entry remark
      )?                 # Optional non-capturing part end
      \s*$               # Allow trailing spaces
    """, re.VERBOSE)

    items = []
    name = None
    remark = None

    while len(line) != 0:
        striped_line = line.strip()
        item_type = None
        nested_data = None

        if striped_line.startswith('ENTRY'):
            item_type = "entry"
            match = data_re.match(striped_line)
            if match:
                name = match[1]
                remark = match[2]
        elif striped_line.startswith('COMMENTED_BYTES'):
            item_type = "commented_bytes"
            nested_data = parse_commented_bytes(file, line)
        elif striped_line.startswith('{'):
            item_type = "start"
        elif striped_line.startswith('}'):
            item_type = "end"

        items.append(Item(line, item_type, nested_data))
        if item_type == "end":
            break

        line = file.readline()

    return Entry(items, name, remark)


def parse_commented_bytes(file: io.TextIOBase, line):
    items = []
    data = bytearray()

    while len(line) != 0:
        striped_line = line.strip()
        name = None

        if striped_line.startswith('COMMENTED_BYTES'):
            name = "commented_bytes"
        elif striped_line.startswith('<hex>'):
            name = "hex"
        elif striped_line.startswith('{'):
            name = "start"
        elif striped_line.startswith('}'):
            name = "end"
        else:
            name = "data"
            data += bytearray.fromhex(striped_line)

        items.append(Item(line, name, None))
        if name == "end":
            break

        line = file.readline()

    return CommentedBytes(items, data)


def compress_cgs(option, cgs):
    for item in cgs:
        if item.nested_data is None:
            continue
        entry = compress_entry(option, item.nested_data)
        if entry is None:
            continue

        cgs.insert(cgs.index(item), Item(item.line, item.type, entry))
        cgs.remove(item)


def compress_entry(option, entry):
    if entry.name != 'Data':
        return None

    for item in entry.items:
        if item.nested_data is None:
            continue
        commented_bytes = compress_commented_bytes(option, item.nested_data)
        if commented_bytes is None:
            return None

        entry.items.insert(entry.items.index(item), Item(item.line, item.type, commented_bytes))
        entry.items.remove(item)

    return Entry(entry.items, 'Compressed data', entry.remark)


def compress_commented_bytes(option, commented_bytes):
    if len(commented_bytes.data) < 512:
        return None
    return CommentedBytes(commented_bytes.items, compress_data(option, commented_bytes.data))


def compress_data(option, data):
    raw_file = str(option.input.with_suffix('.tmp'))
    compressed_file = str(option.input.with_suffix('.tmp.lz'))

    with open(raw_file, mode="wb") as binary:
        binary.write(data)

    subprocess.run([option.lzss_tool, "e", raw_file,
                   compressed_file, ], check=True)

    with open(compressed_file, mode="rb") as binary:
        return binary.read()


def export_cgs(file_name, cgs):
    with open(file_name, mode="w") as file:
        for item in cgs:
            if item.nested_data is None:
                file.write(item.line)
            else:
                export_entry(file, item.nested_data)


def export_entry(file: io.TextIOBase, entry):
    for item in entry.items:
        if item.type == 'entry':
            file.write('ENTRY "{}"'.format(entry.name))
            if entry.remark:
                file.write(' = "{}"'.format(entry.remark))
            file.write('\n')
        elif item.nested_data is None:
            file.write(item.line)
        else:
            export_commented_bytes(file, item.nested_data, entry.name == 'Compressed data')


def export_commented_bytes(file: io.TextIOBase, commented_bytes, is_compressed_data):
    for item in commented_bytes.items:
        if is_compressed_data and item.type == 'data':
            continue

        file.write(item.line)

        if is_compressed_data and item.type == 'hex':
            indent = item.line.rstrip()[:-len('<hex>')]

            for i in range(0, len(commented_bytes.data), 16):
                file.write(indent + commented_bytes.data[i:i + 16].hex(' ') + '\n')


if __name__ == "__main__":
    sys.exit(main())
