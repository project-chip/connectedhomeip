# Copyright (c) 2025 Project CHIP Authors
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

import contextlib
import logging
from typing import BinaryIO

from chiptest.concurrency.worker import WorkerProcess

from .runner import Executor, LogPipe, SubprocessInfo

log = logging.getLogger(__name__)


class DarwinExecutor(Executor):
    def run(self, subproc: SubprocessInfo, stdin: BinaryIO | None = None, stdout: BinaryIO | LogPipe | None = None,
            stderr: BinaryIO | LogPipe | None = None):
        # Try harder to avoid any stdout buffering in our tests
        wrapped = subproc.wrap_with('stdbuf', '-o0', '-i0')
        return super().run(wrapped, stdin, stdout, stderr)


class DarwinWorkerProcess(WorkerProcess):
    """Darwin implementation of the worker process."""

    def _platform_init(self, exit_stack: contextlib.ExitStack) -> DarwinExecutor:
        log.debug("Initializing Darwin test executor.")
        return exit_stack.enter_context(DarwinExecutor())
