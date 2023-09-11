#
# SPDX-FileCopyrightText: 2021 Project CHIP Authors
#
# SPDX-License-Identifier: Apache-2.0
#
"""Subprocess utilities."""

import logging
import subprocess
from typing import List

from memdf.util.config import Config


def run_tool_pipe(config: Config, command: List[str]) -> subprocess.Popen:
    """Run a command."""
    if tool := config.getl(['tool', command[0]]):
        command[0] = tool
    logging.info('Execute: %s', ' '.join(command))
    return subprocess.Popen(command, stdout=subprocess.PIPE)
