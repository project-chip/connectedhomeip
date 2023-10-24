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

import multiprocessing
import os
import time
from multiprocessing import Process
from pathlib import Path

import log

logger = log.get_logger(__file__)


def create_file_timestamp() -> str:
    """Conventional file timestamp suffix"""
    return time.strftime("%Y%m%d_%H%M%S")


def create_standard_log_name(name: str, ext: str, parent: str = "") -> str:
    """Returns the name argument wrapped as a standard log name"""
    ts = create_file_timestamp()
    return os.path.join(parent, f'idt_{ts}_{name}.{ext}')


def safe_mkdir(dir_name: str) -> None:
    Path(dir_name).mkdir(parents=True, exist_ok=True)


def multiproc_size_observer(file_name: str) -> None:
    """
    Warn if a file is not growing over time
    Intended to be run as multiproc target
    """
    time.sleep(15)  # Give some time for the file to show up
    last_size = 0
    while True:
        if not os.path.exists(file_name):
            logger.error(f"Streamed file not on disk yet, this may be normal at the start of execution! {file_name}")
        else:
            new_size = os.path.getsize(file_name)
            if not new_size > last_size:
                logger.critical(f"Streamed file is not growing; check your connection! {file_name}")
            last_size = new_size
        time.sleep(10)


def get_observer_proc(file_name: str) -> Process:
    return multiprocessing.Process(target=multiproc_size_observer, args=(file_name,))
