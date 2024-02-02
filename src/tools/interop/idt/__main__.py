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

import os
import sys

import psutil
from idt import InteropDebuggingTool
from utils.host import current_platform
from utils.log import get_logger
from utils.shell import Bash

logger = get_logger(__file__)
dirty_cleanup = True

if __name__ == "__main__":
    try:
        current_platform.verify_py_version()
        InteropDebuggingTool()
        dirty_cleanup = False
    finally:
        logger.info("Checking for stray child processes")
        psutil_proc = psutil.Process(os.getpid())
        found = False
        for child_proc in psutil_proc.children(recursive=True):
            found = True
            command_short = Bash("").get_current_command_for_pid(child_proc.pid)
            command_full = Bash("").get_current_command_for_pid_full(child_proc.pid)
            logger.error(f"PID: {child_proc.pid} \nCOMMAND: {command_short}\n{command_full}")
        if found:
            logger.error("Stray processes detected, you might want to clean these up!")
        else:
            logger.info("No stray processes detected!")
        if dirty_cleanup and not sys.argv[len(sys.argv)-1] == "-h":
            logger.critical("Crash detected! Clean up any stray processes listed above!!!")
