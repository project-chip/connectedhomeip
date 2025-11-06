#!/usr/bin/env -S python3 -B

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

import subprocess

from .runner import Executor, SubprocessInfo


class DarwinExecutor(Executor):
    def run(self, subproc: SubprocessInfo, stdin, stdout, stderr):
        # Try harder to avoid any stdout buffering in our tests
        wrapped = subproc.wrap_with('stdbuf', '-o0', '-i0')
        s = subprocess.Popen(wrapped.to_cmd(), stdin=stdin, stdout=stdout, stderr=stderr)
        return s
