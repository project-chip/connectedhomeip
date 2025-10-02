# Copyright (c) 2024 Project CHIP Authors
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

import os
import signal
import tempfile
from dataclasses import dataclass
from pathlib import Path
from typing import Any, Optional, Pattern

from matter.testing.tasks import Subprocess


@dataclass
class OtaImagePath:
    """Represents a path to a single OTA image file."""
    path: str

    @property
    def ota_args(self) -> list[str]:
        """Return the command line arguments for this OTA image path."""
        return ["--filepath", self.path]


@dataclass
class ImageListPath:
    """Represents a path to a file containing a list of OTA images."""
    path: str

    @property
    def ota_args(self) -> list[str]:
        """Return the command line arguments for this image list path."""
        return ["--otaImageList", self.path]


class AppServerSubprocess(Subprocess):
    """Wrapper class for starting an application server in a subprocess."""

    # Prefix for log messages from the application server.
    PREFIX = b"[SERVER]"

    def __init__(self, app: str, storage_dir: str, discriminator: int,
                 passcode: int, port: int = 5540, extra_args: list[str] = [], kvs_path: Optional[str] = None):
        # Create a temporary KVS file and keep the descriptor to avoid leaks.
        # self.kvs_fd, kvs_path = tempfile.mkstemp(dir=storage_dir, prefix="kvs-app-")

        if kvs_path:
            self.kvs_fd = None
            final_kvs_path = kvs_path
        else:
            self.kvs_fd, final_kvs_path = tempfile.mkstemp(dir=storage_dir, prefix="kvs-app-")

        try:
            # Build the command list
            command = [app]
            if extra_args:
                command.extend(extra_args)

            command.extend([
                "--KVS", final_kvs_path,
                '--secured-device-port', str(port),
                "--discriminator", str(discriminator),
                "--passcode", str(passcode)
            ])

            # Start the server application
            super().__init__(*command,
                             output_cb=lambda line, is_stderr: self.PREFIX + line)
        except Exception:
            # Do not leak KVS file descriptor on failure
            if self.kvs_fd is not None:
                os.close(self.kvs_fd)
            raise

    def __del__(self):
        # Do not leak KVS file descriptor.
        if hasattr(self, "kvs_fd") and self.kvs_fd is not None:
            try:
                os.close(self.kvs_fd)
            except OSError:
                pass


class IcdAppServerSubprocess(AppServerSubprocess):
    """Wrapper class for starting an ICD application server in a subprocess."""

    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.paused = False

    def pause(self, check_state: bool = True):
        if check_state and self.paused:
            raise ValueError("ICD TH Server unexpectedly is already paused")
        if not self.paused:
            # Stop (halt) the ICD server process by sending a SIGTOP signal.
            self.p.send_signal(signal.SIGSTOP)
            self.paused = True

    def resume(self, check_state: bool = True):
        if check_state and not self.paused:
            raise ValueError("ICD TH Server unexpectedly is already running")
        if self.paused:
            # Resume (continue) the ICD server process by sending a SIGCONT signal.
            self.p.send_signal(signal.SIGCONT)
            self.paused = False

    def terminate(self):
        # Make sure the ICD server process is not paused before terminating it.
        self.resume(check_state=False)
        super().terminate()


class JFControllerSubprocess(Subprocess):
    """Wrapper class for starting a controller in a subprocess."""

    # Prefix for log messages from the application server.
    PREFIX = b"[JF-CTRL]"

    def __init__(self, app: str, rpc_server_port: int, storage_dir: str,
                 vendor_id: int, extra_args: list[str] = []):

        # Build the command list
        command = [app]
        if extra_args:
            command.extend(extra_args)

        command.extend([
            "--rpc-server-port", str(rpc_server_port),
            "--storage-directory", storage_dir,
            "--commissioner-vendor-id", str(vendor_id)
        ])

        # Start the server application
        super().__init__(*command,  # Pass the constructed command list
                         output_cb=lambda line, is_stderr: self.PREFIX + line)


class OTAProviderSubprocess(AppServerSubprocess):
    """Wrapper class for starting an OTA Provider application server in a subprocess."""

    # Prefix for log messages from the OTA provider application.
    PREFIX = b"[OTA-PROVIDER]"

    def __init__(
        self,
        ota_file: str,
        discriminator: int,
        passcode: int,
        secured_device_port: int,
        queue: Optional[str] = None,
        timeout: Optional[int] = None,
        override_image_uri: Optional[str] = None,
        log_file_path: Optional[str] = "provider.log",
        app_path: Optional[str] = None,
        kvs_path: Optional[str] = None,
    ):
        """
        Initialize OTA Provider with hardcoded KVS path, log file, and extra args.

        Args:
            ota_file: Path to OTA image file (string or OtaImagePath).
            discriminator: Commissioning discriminator.
            passcode: Setup PIN code.
            secured_device_port: Port for provider process.
            queue: Optional queue name.
            timeout: Optional timeout in seconds.
            override_image_uri: Optional ImageURI override.
            log_file_path: File to store logs in real-time.
            app_path: Required path to the chip-ota-provider-app binary.
                      Must be provided explicitly (debug/release path may vary).
                      example: ./out/debug/chip-ota-provider-app
            kvs_path: Optional path for KVS storage. If provided, passed with --KVS.
        """

        self.log_file_path = log_file_path

        if not app_path:
            raise ValueError("app_path must be provided for OTAProviderSubprocess")

        # Build argument list for provider launch
        args = [f"--filepath={ota_file}" if isinstance(ota_file, str) else ota_file.path]
        if queue:
            args += ["-q", queue]
        if timeout:
            args += ["-t", str(timeout)]
        if override_image_uri:
            args += ["-i", override_image_uri]

         # Add KVS path only if explicitly provided
        if kvs_path:
            args += [f"--KVS={kvs_path}"]

        # Clear the log file before starting
        assert self.log_file_path is not None, "log_file_path must not be None"
        Path(self.log_file_path).write_text("")

        # Save args for use in the base constructor
        self._extra_args = args

        # Initialize parent AppServerSubprocess with final arguments
        super().__init__(
            app=app_path,
            storage_dir="/tmp",
            discriminator=discriminator,
            passcode=passcode,
            port=secured_device_port,
            extra_args=args,
            kvs_path=kvs_path
        )

    def _process_output(self, line: bytes, is_stderr: bool) -> bytes:
        """Write logs only to file, return empty bytes to avoid console output."""
        assert self.log_file_path is not None, "log_file_path must not be None"
        fpath = Path(self.log_file_path)
        fpath.parent.mkdir(parents=True, exist_ok=True)
        with fpath.open("ab") as f:
            f.write(line)
            f.flush()
        return b""  # must return bytes, not None

    def _output_adapter(self, line: bytes) -> bytes:
        return self._process_output(line, False)

    def start(self, expected_output: str | Pattern[Any] | None = None, timeout: float | None = 30):
        """Override start to attach log processing callback."""
        self.output_cb = self._process_output
        super().start(expected_output=expected_output, timeout=timeout)
