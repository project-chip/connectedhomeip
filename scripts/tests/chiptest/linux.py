#
#    Copyright (c) 2026 Project CHIP Authors
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

"""
Handles linux-specific functionality for running test cases
"""

import os
from typing import IO, Any

from chiptest.runner import Executor, LogPipe, SubprocessInfo
from python_path import PythonPath

root_dir = os.path.dirname(
    os.path.dirname(
        os.path.dirname(os.path.dirname(os.path.abspath(__file__)))))

with PythonPath(os.path.join(root_dir, 'src/python_testing/matter_testing_infrastructure/matter'), relative_to=__file__):
    from testing.linux import (BluetoothMock, DBusTestSystemBus, IsolatedNetworkNamespace, ThreadBorderRouter, WpaSupplicantMock,
                               ensure_network_namespace_availability, ensure_private_state)

__all__ = [
    "ensure_network_namespace_availability",
    "ensure_private_state",
    "BluetoothMock",
    "DBusTestSystemBus",
    "IsolatedNetworkNamespace",
    "LinuxNamespacedExecutor",
    "ThreadBorderRouter",
    "WpaSupplicantMock"
]


class LinuxNamespacedExecutor(Executor):
    def __init__(self, ns: IsolatedNetworkNamespace):
        super().__init__()
        self.ns = ns

    def run(self, subproc: SubprocessInfo, stdin: IO[Any] | None = None, stdout: IO[Any] | LogPipe | None = None,
            stderr: IO[Any] | LogPipe | None = None):
        wrapped = subproc.wrap_with("ip", "netns", "exec", self.ns.netns_for_subprocess_kind(subproc.kind))
        return super().run(wrapped, stdin=stdin, stdout=stdout, stderr=stderr)
