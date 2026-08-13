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
import threading

from matter.testing.concurrency.context import TerminablePopen


class DBusTestSystemBus(TerminablePopen[str]):
    """Run a dbus-daemon in a subprocess as a test system bus."""

    SOCKET = pathlib.Path(f"/tmp/chip-dbus-{os.getpid()}")
    ADDRESS = f"unix:path={SOCKET}"

    def __init__(self) -> None:
        super().__init__(lambda: subprocess.Popen(["dbus-daemon", "--session", "--print-address", "--address", self.ADDRESS],
                                                  stdout=subprocess.PIPE, stderr=subprocess.DEVNULL, text=True))
        self._prev_system_bus_address: str | None = None

    def resource_start(self) -> subprocess.Popen[str]:
        process = super().resource_start()

        # Wait for the bus to start (it will print the address to stdout).
        started = threading.Event()

        def _wait_for_start() -> None:
            assert process.stdout is not None, "stdout should have been set to subprocess.PIPE"
            while True:
                if self.ADDRESS in process.stdout.readline():
                    started.set()
                    break

        threading.Thread(name="DBusTestSystemBusStdout", target=_wait_for_start, daemon=True).start()

        if not started.wait(self.RESOURCE_TIMEOUT_START_S):
            raise TimeoutError(f"DBus test system bus did not start within {self.RESOURCE_TIMEOUT_START_S} seconds")
        if process.returncode is not None and process.returncode != 0:
            raise RuntimeError(f"DBus test system bus process exited with code {process.returncode}")

        # Set the environment variable so that clients will use this bus.
        self._prev_system_bus_address = os.environ.get("DBUS_SYSTEM_BUS_ADDRESS")
        os.environ["DBUS_SYSTEM_BUS_ADDRESS"] = self.ADDRESS

        return process

    def resource_terminate(self) -> None:
        # Restore the environment variable so that clients will stop using this bus.
        if self._prev_system_bus_address is not None:
            os.environ["DBUS_SYSTEM_BUS_ADDRESS"] = self._prev_system_bus_address

        super().resource_terminate()
        self.SOCKET.unlink(missing_ok=True)
