#!/usr/bin/env python

#
#    Copyright (c) 2024 Project CHIP Authors
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
# Execute as `python scripts/gen_chip_error.py` from root of repos
#
# This script uses heuristics scraping of the headers to generate ChipError.java for Android platform.
#

import re
from dataclasses import dataclass
from enum import IntEnum
from operator import attrgetter
from pathlib import Path


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


CHIP_ERROR_FILE = "src/platform/android/java/chip/platform/ChipError.java"
IM_STATUS_FILE = "src/controller/java/src/chip/devicecontroller/model/Status.java"


def dump_top_comments(file):
    file.write(r'''/*
 *   Copyright (c) 2024 Project CHIP Authors
 *   All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */
''')


def get_section_title(section: str) -> tuple[str, str]:
    markdown_title = f"{section} errors"
    anchor_name = f'#{markdown_title.lower().replace(" ","-").replace(".","-")}'

    return markdown_title, anchor_name


def dump_chip_error_by_descriptor(chip_error_file, header_path: Path, descriptor: ErrorDescriptor, is_last: bool):
    loader = ErrorCodeLoader()
    codes_for_section = loader.load_error_header(header_path, descriptor)

    title, _ = get_section_title(descriptor.section)
    chip_error_file.write("\n")
    chip_error_file.write("    /**\n")
    chip_error_file.write(f"     * {title}\n")
    chip_error_file.write("     *\n")
    chip_error_file.write(f"     * These errors are defined in {header_path}\n")
    chip_error_file.write("     */\n")

    for code in sorted(codes_for_section, key=attrgetter("code")):
        if is_last and code == codes_for_section[-1]:
            chip_error_file.write(f"    {code.name}({code.code}L);\n")
        else:
            chip_error_file.write(f"    {code.name}({code.code}L),\n")


def dump_chip_error(chip_error_file, descriptors: dict[str, ErrorDescriptor]):
    dump_top_comments(chip_error_file)
    chip_error_file.write(r'''
package chip.platform;

public enum ChipError {
''')

    for i, (filename, descriptor) in enumerate(descriptors.items()):
        dump_chip_error_by_descriptor(chip_error_file, Path(filename), descriptor, i == len(descriptors) - 1)

    chip_error_file.write(r'''
    private final long value;

    ChipError(long value) {
        this.value = value;
    }

    public long getValue() {
        return value;
    }

    public static ChipError value(long value) {
        for (ChipError error : values()) {
            if (error.value == value) {
                return error;
            }
        }
        throw new IllegalArgumentException("Invalid value: " + value);
    }

    public String toString() {
        return toString(value);
    }

    public static native String toString(long errorCode);
}
''')


def dump_im_status(im_status_file, header_path: Path, descriptor: ErrorDescriptor):
    dump_top_comments(im_status_file)
    im_status_file.write(r'''
package chip.devicecontroller.model;

import java.util.Locale;
import java.util.Optional;

/** Class for Interaction Model Status * */
public final class Status {
  public enum Code {
''')

    loader = ErrorCodeLoader()
    codes_for_section = loader.load_error_header(header_path, descriptor)

    for code in sorted(codes_for_section, key=attrgetter("code")):
        hex_code = '0x{:02x}'.format(code.code)
        if code == codes_for_section[-1]:
            im_status_file.write(f"    {code.name}({hex_code});\n")
        else:
            im_status_file.write(f"    {code.name}({hex_code}),\n")

    im_status_file.write(r'''
    private int id = 0;

    Code(int id) {
      this.id = id;
    }

    public int getId() {
      return id;
    }

    public static Code fromId(int id) {
      for (Code type : values()) {
        if (type.getId() == id) {
          return type;
        }
      }
      return null;
    }
  }

  private Code status = Code.Success;
  private Optional<Integer> clusterStatus;

  private Status(int status, Optional<Integer> clusterStatus) {
    this.status = Code.fromId(status);
    this.clusterStatus = clusterStatus;
  }

  // Getters
  public Code getStatus() {
    return status;
  }

  public Optional<Integer> getClusterStatus() {
    return clusterStatus;
  }

  public String toString() {
    return String.format(
        Locale.ENGLISH,
        "status %s, clusterStatus %s",
        status.name(),
        clusterStatus.isPresent() ? String.valueOf(clusterStatus.get()) : "None");
  }

  public static Status newInstance(int status) {
    return new Status(status, Optional.empty());
  }

  public static Status newInstance(int status, Integer clusterStatus) {
    return new Status(status, Optional.ofNullable(clusterStatus));
  }

  public static Status newInstance(int status, Optional<Integer> clusterStatus) {
    return new Status(status, clusterStatus);
  }
}
''')


def main():
    with open(CHIP_ERROR_FILE, "w") as chip_error_file:
        descriptors = {
            "src/lib/core/CHIPError.h": ErrorDescriptor(section="SDK Core", code_range=0x000, macro_regex=r"^#define\s+(?P<name>[_A-Z0-9]+)\s+CHIP(_CORE)?_ERROR[(](?P<code>(0x[a-fA-F0-9]+)|\d+)[)]", include_description=True),
            "src/inet/InetError.h": ErrorDescriptor(section="SDK Inet Layer", code_range=0x100, macro_regex=r"^#define\s+(?P<name>[_A-Z0-9]+)\s+CHIP_INET_ERROR[(](?P<code>(0x[a-fA-F0-9]+)|\d+)[)]", include_description=True),
            "src/include/platform/CHIPDeviceError.h": ErrorDescriptor(section="SDK Device Layer", code_range=0x200, macro_regex=r"^#define\s+(?P<name>[_A-Z0-9]+)\s+CHIP_DEVICE_ERROR[(](?P<code>(0x[a-fA-F0-9]+)|\d+)[)]", include_description=True),
            "src/lib/asn1/ASN1Error.h": ErrorDescriptor(section="ASN1 Layer", code_range=0x300, macro_regex=r"^#define\s+(?P<name>[_A-Z0-9]+)\s+CHIP_ASN1_ERROR[(](?P<code>(0x[a-fA-F0-9]+)|\d+)[)]", include_description=True),
            "src/ble/BleError.h": ErrorDescriptor(section="BLE Layer", code_range=0x400, macro_regex=r"^#define\s+(?P<name>[_A-Z0-9]+)\s+CHIP_BLE_ERROR[(](?P<code>(0x[a-fA-F0-9]+)|\d+)[)]", include_description=True),
        }
        dump_chip_error(chip_error_file, descriptors)

    with open(IM_STATUS_FILE, "w") as im_status_file:
        header_path = Path("src/protocols/interaction_model/StatusCodeList.h")
        descriptor = ErrorDescriptor(section="IM Global", code_range=0x000, macro_regex=r"^CHIP_IM_STATUS_CODE[(](?P<name>[A-Za-z0-9_]+)\s*,\s*(?P<spec_name>[A-Za-z0-9_]+)\s*,\s*(?P<code>(0x[a-fA-F0-9]+)|\d+)[)]")
        dump_im_status(im_status_file, header_path, descriptor)


if __name__ == "__main__":
    main()
