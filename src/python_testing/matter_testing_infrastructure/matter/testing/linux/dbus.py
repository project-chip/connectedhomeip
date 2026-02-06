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


class DBusTestSystemBus(subprocess.Popen[bytes]):
    """Run a dbus-daemon in a subprocess as a test system bus."""

    SOCKET = pathlib.Path(f"/tmp/chip-dbus-{os.getpid()}")
    ADDRESS = f"unix:path={SOCKET}"

    def __init__(self):
        super().__init__(["dbus-daemon", "--session", "--print-address", "--address", self.ADDRESS],
                         stdout=subprocess.PIPE, stderr=subprocess.DEVNULL)
        os.environ["DBUS_SYSTEM_BUS_ADDRESS"] = self.ADDRESS
        # Wait for the bus to start (it will print the address to stdout).
        assert self.stdout is not None, "stdout should have been set to subprocess.PIPE"
        self.stdout.readline()

    def terminate(self):
        super().terminate()
        self.SOCKET.unlink(True)
        self.wait()
