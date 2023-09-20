#
#    Copyright (c) 2023 Project CHIP Authors
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

import time
from pathlib import Path
from typing import TextIO


def add_border(to_print: str) -> str:
    """Add star borders to important strings"""
    return '\n' + '*' * len(to_print) + '\n' + to_print


def create_file_timestamp() -> str:
    """Conventional file timestamp suffix"""
    return time.strftime("%Y%m%d_%H%M%S")


def create_standard_log_name(name: str, ext: str) -> str:
    """Returns the name argument wrapped as a standard log name"""
    ts = create_file_timestamp()
    return f'idt_{ts}_{name}.{ext}'


def safe_mkdir(dir_name: str) -> None:
    Path(dir_name).mkdir(parents=True, exist_ok=True)


def print_and_write(to_print: str, file: TextIO) -> None:
    print(to_print)
    file.write(to_print)


def border_print(to_print: str, important: bool = False) -> None:
    len_borders = 64
    border = f"\n{'_' * len_borders}\n"
    i_border = f"\n{'!' * len_borders}\n" if important else ""
    print(f"{border}{i_border}{to_print}{i_border}{border}")
