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

import logging
from typing import TextIO

import config

_CONFIG_LEVEL = config.log_level

blue = "\x1b[34;1m"
cyan = "\x1b[36;1m"
green = "\x1b[32;1m"
grey = "\x1b[38;20m"
magenta = "\x1b[35;1m"
yellow = "\x1b[33;20m"
red_black_background = "\x1b[31;40;20m"
bold_red_yellow_background = "\x1b[31;43;1m"

reset = "\x1b[0m"

format_pre = "%(asctime)s %(levelname)s {%(module)s} [%(funcName)s] " + reset
format_pre = cyan + format_pre if config.enable_color else format_pre
format_post = "%(message)s" + reset
format_no_color = format_pre+format_post

FORMATS = {
    logging.DEBUG: format_pre + blue + format_post,
    logging.INFO: format_pre + green + format_post,
    logging.WARNING: format_pre + yellow + format_post,
    logging.ERROR: format_pre + red_black_background + format_post,
    logging.CRITICAL: format_pre + bold_red_yellow_background + format_post
}


class LoggingFormatter(logging.Formatter):

    def format(self, record):
        log_fmt = FORMATS.get(record.levelno) if config.enable_color else format_no_color
        formatter = logging.Formatter(log_fmt)
        return formatter.format(record)


def get_logger(logger_name) -> logging.Logger:
    logger = logging.getLogger(logger_name)
    logger.setLevel(_CONFIG_LEVEL)
    ch = logging.StreamHandler()
    ch.setLevel(_CONFIG_LEVEL)
    ch.setFormatter(LoggingFormatter())
    logger.addHandler(ch)
    logger.propagate = False
    return logger


def border_print(to_print: str, important: bool = False) -> None:
    len_borders = len(to_print)
    border = f"\n{'_' * len_borders}\n"
    i_border = f"\n{'!' * len_borders}\n" if important else ""
    if config.enable_color:
        print(magenta, end="")
    print(f"{border}{i_border}{to_print}{i_border}{border}")
    if config.enable_color:
        print(reset, end="")


def print_and_write(to_print: str, file: TextIO) -> None:
    if config.enable_color:
        print(green, end="")
    print(f"{to_print}")
    if config.enable_color:
        print(reset, end="")
    file.write(to_print)


def add_border(to_print: str) -> str:
    return '\n' + '*' * len(to_print) + '\n' + to_print
