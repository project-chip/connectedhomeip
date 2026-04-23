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

import shutil
from typing import IO, Any

from .runner import Executor, LogPipe, SubprocessInfo

# Older Homebrew coreutils installed 'stdbuf' directly on $PATH; newer versions only expose it as 'gstdbuf',
# with the unprefixed name installed only in libexec/gnubin (not on $PATH by default). Use whichever is available.
_STDBUF = shutil.which('stdbuf') or shutil.which('gstdbuf')


class DarwinExecutor(Executor):
    def run(self, subproc: SubprocessInfo, stdin: IO[Any] | None = None, stdout: IO[Any] | LogPipe | None = None, stderr: IO[Any] | LogPipe | None = None):
        # Try harder to avoid any stdout buffering in our tests
        wrapped = subproc.wrap_with(_STDBUF, '-o0', '-i0') if _STDBUF else subproc
        return super().run(wrapped, stdin, stdout, stderr)
