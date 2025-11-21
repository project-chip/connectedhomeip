"""Multicast DNS Service Discovery for Python, v0.14-wmcbrine
    )
Copyright 2003 Paul Scott-Murphy, 2014 William McBrine

This module provides a framework for the use of DNS Service Discovery
using IP multicast.

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301
USA
"""

from __future__ import annotations

import logging
import sys
from typing import Any, ClassVar, cast

log = logging.getLogger(__name__.split(".", maxsplit=1)[0])
log.addHandler(logging.NullHandler())


def set_logger_level_if_unset() -> None:
    if log.level == logging.NOTSET:
        log.setLevel(logging.WARN)


set_logger_level_if_unset()


class QuietLogger:
    _seen_logs: ClassVar[dict[str, int | tuple]] = {}

    @classmethod
    def log_exception_warning(cls, *logger_data: Any) -> None:
        exc_info = sys.exc_info()
        exc_str = str(exc_info[1])
        if exc_str not in cls._seen_logs:
            # log at warning level the first time this is seen
            cls._seen_logs[exc_str] = exc_info
            logger = log.warning
        else:
            logger = log.debug
        logger(*(logger_data or ["Exception occurred"]), exc_info=True)

    @classmethod
    def log_exception_debug(cls, *logger_data: Any) -> None:
        log_exc_info = False
        exc_info = sys.exc_info()
        exc_str = str(exc_info[1])
        if exc_str not in cls._seen_logs:
            # log the trace only on the first time
            cls._seen_logs[exc_str] = exc_info
            log_exc_info = True
        log.debug(*(logger_data or ["Exception occurred"]), exc_info=log_exc_info)

    @classmethod
    def log_warning_once(cls, *args: Any) -> None:
        msg_str = args[0]
        if msg_str not in cls._seen_logs:
            cls._seen_logs[msg_str] = 0
            logger = log.warning
        else:
            logger = log.debug
        cls._seen_logs[msg_str] = cast(int, cls._seen_logs[msg_str]) + 1
        logger(*args)

    @classmethod
    def log_exception_once(cls, exc: Exception, *args: Any) -> None:
        msg_str = args[0]
        if msg_str not in cls._seen_logs:
            cls._seen_logs[msg_str] = 0
            logger = log.warning
        else:
            logger = log.debug
        cls._seen_logs[msg_str] = cast(int, cls._seen_logs[msg_str]) + 1
        logger(*args, exc_info=exc)
