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

from termcolor import colored

import config

_CONFIG_LEVEL = config.log_level

_FORMAT_PRE_FSTRING = "%(asctime)s %(levelname)s {%(module)s} [%(funcName)s] "
_FORMAT_PRE = colored(_FORMAT_PRE_FSTRING, "blue") if config.enable_color else _FORMAT_PRE_FSTRING
_FORMAT_POST = "%(message)s"
_FORMAT_NO_COLOR = _FORMAT_PRE_FSTRING+_FORMAT_POST

FORMATS = {
    logging.DEBUG: _FORMAT_PRE + colored(_FORMAT_POST, "blue"),
    logging.INFO: _FORMAT_PRE + colored(_FORMAT_POST, "green"),
    logging.WARNING: _FORMAT_PRE + colored(_FORMAT_POST, "yellow"),
    logging.ERROR: _FORMAT_PRE + colored(_FORMAT_POST, "red", attrs=["bold"]),
    logging.CRITICAL: _FORMAT_PRE + colored(_FORMAT_POST, "red",  "on_yellow", attrs=["bold"]),
}


class LoggingFormatter(logging.Formatter):

    def format(self, record):
        log_fmt = FORMATS.get(record.levelno) if config.enable_color else _FORMAT_NO_COLOR
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
    to_print = f"{border}{i_border}{to_print}{i_border}{border}"
    if config.enable_color:
        to_print = colored(to_print, "magenta")
    print(to_print)


def print_and_write(to_print: str, file: TextIO) -> None:
    if config.enable_color:
        print(colored(to_print, "green"))
    else:
        print(to_print)
    file.write(to_print)


def add_border(to_print: str) -> str:
    return '\n' + '*' * len(to_print) + '\n' + to_print
