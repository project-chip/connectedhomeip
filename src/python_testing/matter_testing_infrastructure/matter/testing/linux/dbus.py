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

import os
import pathlib
import subprocess

from matter.testing.concurrency.context import TerminableResource


class DBusTestSystemBus(TerminableResource):
    """Run a dbus-daemon in a subprocess as a test system bus."""

    SOCKET = pathlib.Path(f"/tmp/chip-dbus-{os.getpid()}")
    ADDRESS = f"unix:path={SOCKET}"

    def __init__(self) -> None:
        self._process: subprocess.Popen[bytes] | None = None

    def resource_start(self) -> None:
        self._process = subprocess.Popen(["dbus-daemon", "--session", "--print-address", "--address", self.ADDRESS],
                                         stdout=subprocess.PIPE, stderr=subprocess.DEVNULL)
        os.environ["DBUS_SYSTEM_BUS_ADDRESS"] = self.ADDRESS

        # Wait for the bus to start (it will print the address to stdout).
        assert self._process.stdout is not None, "stdout should have been set to subprocess.PIPE"
        self._process.stdout.readline()

    def resource_terminate(self) -> None:
        if self._process is not None:
            try:
                self._process.terminate()
                self._process.wait()
            finally:
                if self._process.stdout is not None:
                    self._process.stdout.close()

        self.SOCKET.unlink(missing_ok=True)
