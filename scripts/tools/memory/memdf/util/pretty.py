#
# SPDX-FileCopyrightText: 2021 Project CHIP Authors
#
# SPDX-License-Identifier: Apache-2.0
#
"""Pretty print logging."""

import logging
import pprint
from typing import Any


def log(level: int, x: Any) -> None:
    if logging.getLogger(None).isEnabledFor(level):
        for line in pprint.pformat(x).split('\n'):
            logging.log(level, line)


def info(x: Any) -> None:
    log(logging.INFO, x)


def debug(x: Any) -> None:
    log(logging.DEBUG, x)
