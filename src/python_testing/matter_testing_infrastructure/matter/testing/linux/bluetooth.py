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

import logging
import subprocess
import threading

from matter.testing.concurrency.context import TerminableResource

log = logging.getLogger(__name__)


class BluetoothMock(TerminableResource):
    """Run a BlueZ mock server in a subprocess."""

    # The MAC addresses of the virtual Bluetooth adapters.
    ADAPTERS = ["00:00:00:11:11:11", "00:00:00:22:22:22"]

    def _forward_stderr(self, process: subprocess.Popen[str], event: threading.Event) -> None:
        assert process.stderr is not None, "stderr should have been set to subprocess.PIPE"
        adapters_to_init = set(enumerate(self.ADAPTERS))
        for line in process.stderr:
            for index, adapter in adapters_to_init.copy():
                if f"adapter[{index}][{adapter}]" in line:
                    adapters_to_init.discard((index, adapter))
                    break
            if not adapters_to_init:
                event.set()
            log.debug(line.strip())

    def __init__(self) -> None:
        self._process: subprocess.Popen[str] | None = None

    def resource_start(self) -> None:
        adapters = [f"--adapter={mac}" for mac in self.ADAPTERS]
        self._process = subprocess.Popen(["bluezoo", "--auto-enable"] + adapters, stderr=subprocess.PIPE, text=True)

        event = threading.Event()
        threading.Thread(name="BluetoothMockStderr", target=self._forward_stderr, args=(self._process, event), daemon=True).start()

        # Wait for the adapters to be ready.
        event.wait()

    def resource_terminate(self) -> None:
        if self._process is not None:
            try:
                self._process.terminate()
                self._process.wait()
            finally:
                if self._process.stderr is not None:
                    self._process.stderr.close()
