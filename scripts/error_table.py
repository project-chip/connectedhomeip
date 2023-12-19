#!/usr/bin/env python

#
#    Copyright (c) 2022 Project CHIP Authors
#    All rights reserved.
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

#
# Execute as `python scripts/error_table.py > docs/ERROR_CODES.md` from root of repos
#
# This script uses heuristics scraping of the headers to generate nice tables
#

import re
from dataclasses import dataclass
from enum import IntEnum
from operator import attrgetter
from pathlib import Path

from tabulate import tabulate


@dataclass
class ErrorCode:
    code: int
    name: str
    description: str


@dataclass
class ErrorDescriptor:
    section: str
    code_range: int
    # `macro_regex` needs to have `code` and `name` named groups.
    macro_regex: str
    include_description: bool = False


class CommentState(IntEnum):
    WAIT_START_COMMENT = 0
    ACCUMULATE_COMMENT = 1


class ErrorCodeLoader:
    def __init__(self) -> None:
        self.reset()

    def reset(self):
        self._comment_state = CommentState.WAIT_START_COMMENT
        self._last_comment = []
        self._error_codes: list[ErrorCode] = []

    def _process_comment_extract(self, line):
        if self._comment_state == CommentState.WAIT_START_COMMENT:
            if "/**" in line:
                self._last_comment = []
                self._comment_state = CommentState.ACCUMULATE_COMMENT
        elif self._comment_state == CommentState.ACCUMULATE_COMMENT:
            if "*/" in line:
                self._comment_state = CommentState.WAIT_START_COMMENT
            else:
                self._last_comment.append(line)

    def _process_error_extract(self, descriptor: ErrorDescriptor, line: str):
        match = re.search(descriptor.macro_regex, line)
        if match is None:
            return

        last_comment = "".join(self._last_comment).replace("   ", " ").replace("  ", " ").replace("*", "").replace(".", "")
        last_comment = last_comment.split("@brief")[-1].strip()

        code = int(match.group("code"), 0)
        code = descriptor.code_range | code
        name = match.group("name")

        description = last_comment if descriptor.include_description else ""
        self._error_codes.append(ErrorCode(code=code, name=name, description=description))

    def load_error_header(self, filename: Path, descriptor: ErrorDescriptor) -> list[ErrorCode]:
        self.reset()

        lines = filename.read_text().split("\n")
        for line in lines:
            line = line.strip()
            self._process_comment_extract(line)
            self._process_error_extract(descriptor, line)

        return self._error_codes


def get_section_title(section: str) -> tuple[str, str]:
    markdown_title = f"{section} errors"
    anchor_name = f'#{markdown_title.lower().replace(" ","-").replace(".","-")}'

    return markdown_title, anchor_name


def dump_table(header_path: Path, descriptor: ErrorDescriptor):
    loader = ErrorCodeLoader()
    codes_for_section = loader.load_error_header(header_path, descriptor)

    markdown_title, _ = get_section_title(descriptor.section)
    print(f"## {markdown_title}")
    print()

    headers = ["Decimal", "Hex", "Name"]

    if descriptor.include_description:
        headers.append("Description")

    data = []

    for code in sorted(codes_for_section, key=attrgetter("code")):
        line = [code.code, "0x%02X" % code.code, f"`{code.name}`"]
        if descriptor.include_description:
            line.append(code.description)
        data.append(line)

    print(tabulate(data, headers, tablefmt="github", numalign="left"))
    print()


def main():
    descriptors = {
        "src/lib/core/CHIPError.h": ErrorDescriptor(section="SDK Core", code_range=0x000, macro_regex=r"^#define\s+(?P<name>[_A-Z0-9]+)\s+CHIP(_CORE)?_ERROR[(](?P<code>(0x[a-fA-F0-9]+)|\d+)[)]"),
        "src/inet/InetError.h": ErrorDescriptor(section="SDK Inet Layer", code_range=0x100, macro_regex=r"^#define\s+(?P<name>[_A-Z0-9]+)\s+CHIP_INET_ERROR[(](?P<code>(0x[a-fA-F0-9]+)|\d+)[)]"),
        "src/include/platform/CHIPDeviceError.h": ErrorDescriptor(section="SDK Device Layer", code_range=0x200, macro_regex=r"^#define\s+(?P<name>[_A-Z0-9]+)\s+CHIP_DEVICE_ERROR[(](?P<code>(0x[a-fA-F0-9]+)|\d+)[)]"),
        "src/lib/asn1/ASN1Error.h": ErrorDescriptor(section="ASN1 Layer", code_range=0x300, macro_regex=r"^#define\s+(?P<name>[_A-Z0-9]+)\s+CHIP_ASN1_ERROR[(](?P<code>(0x[a-fA-F0-9]+)|\d+)[)]"),
        "src/ble/BleError.h": ErrorDescriptor(section="BLE Layer", code_range=0x400, macro_regex=r"^#define\s+(?P<name>[_A-Z0-9]+)\s+CHIP_BLE_ERROR[(](?P<code>(0x[a-fA-F0-9]+)|\d+)[)]"),
        "src/protocols/interaction_model/StatusCodeList.h": ErrorDescriptor(section="IM Global errors", code_range=0x500, macro_regex=r"^CHIP_IM_STATUS_CODE[(][A-Za-z0-9_]+\s*,\s*(?P<name>[A-Z0-9_]+)\s*,\s*(?P<code>(0x[a-fA-F0-9]+)|\d+)[)]"),
    }

    print("# Matter SDK `CHIP_ERROR` enums values")
    print()
    print("This file was **AUTOMATICALLY** generated by")
    print("`python scripts/error_table.py > docs/ERROR_CODES.md`. DO NOT EDIT BY HAND!")
    print()
    print("## Table of contents")

    for descriptor in descriptors.values():
        markdown_title, anchor_name = get_section_title(descriptor.section)
        print(f"-   [{markdown_title}: range `0x{descriptor.code_range:03X}..0x{descriptor.code_range | 0xFF:03X}`]({anchor_name})")
    print()

    for filename, descriptor in descriptors.items():
        dump_table(Path(filename), descriptor)


if __name__ == "__main__":
    main()
