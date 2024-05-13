#
# Copyright (c) 2021 Project CHIP Authors
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
