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
import re
import tempfile
from dataclasses import dataclass
from sys import stderr, stdout
from typing import BinaryIO, Optional, Union

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
                 passcode: int, port: int = 5540, extra_args: list[str] = [], kvs_path: Optional[str] = None, f_stdout: BinaryIO = stdout.buffer, f_stderr: BinaryIO = stderr.buffer):
        # Create a temporary KVS file and keep the descriptor to avoid leaks.

        if kvs_path is not None:
            self.kvs_fd = None
            kvs_path = kvs_path
        else:
            self.kvs_fd, kvs_path = tempfile.mkstemp(dir=storage_dir, prefix="kvs-app-")
        try:
            # Build the command list
            command = [app]
            if extra_args:
                command.extend(extra_args)

            command.extend([
                "--KVS", kvs_path,
                '--secured-device-port', str(port),
                "--discriminator", str(discriminator),
                "--passcode", str(passcode)
            ])

            # Start the server application
            super().__init__(*command,  # Pass the constructed command list
                             output_cb=lambda line, is_stderr: self.PREFIX + line, f_stdout=f_stdout, f_stderr=f_stderr)
        except Exception:
            # Do not leak KVS file descriptor on failure
            if self.kvs_fd is not None:
                os.close(self.kvs_fd)
                raise

    def __del__(self):
        # Do not leak KVS file descriptor.
        if hasattr(self, "kvs_fd"):
            try:
                if self.kvs_fd is not None:
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

    DEFAULT_ADMIN_NODE_ID = 112233
    log_file = ""
    err_log_file = ""

    # Prefix for log messages from the OTA provider application.
    PREFIX = b"[OTA-PROVIDER]"
    log_file = ""
    err_log_file = ""

    def __init__(self, app: str, storage_dir: str, discriminator: int,
                 passcode: int, ota_source: Union[OtaImagePath, ImageListPath],
                 port: int = 5541, extra_args: list[str] = [], kvs_path: Optional[str] = None, log_file: Union[str, BinaryIO] = stdout.buffer, err_log_file: Union[str, BinaryIO] = stderr.buffer):
        """Initialize the OTA Provider subprocess.

        Args:
            app(str): Path to the chip-ota-provider-app executable
            storage_dir(str): Directory for persistent storage
            discriminator(int): Discriminator for commissioning
            passcode(int): Passcode for commissioning
            port(int): UDP port for secure connections (default: 5541)
            ota_source(OtaImagePath,ImageListPath): Either OtaImagePath or ImageListPath specifying the OTA image source
            extra_args(list): Additional command line arguments
            kvs_path(str): Str of the path for the kvs path, if not will use temp file.
            log_file(str,BinaryIO): Path to create the BinaryIO logger for stdoutput, if not use the default stdout.buffer.
            err_log_file(str,BinaryIO): Path to create the BinaryIO logger for stderr, if not use the default stderr.buffer.
        """
        # Create the BinaryIO fp allow to use
        if isinstance(log_file, str):
            f_stdout = open(log_file, 'ab')
            self.log_file = log_file

        if isinstance(err_log_file, str):
            f_stderr = open(err_log_file, 'ab')
            self.err_log_file = err_log_file

        # Build OTA-specific arguments using the ota_source property
        combined_extra_args = ota_source.ota_args + extra_args

        # Initialize with the combined arguments
        super().__init__(app=app, storage_dir=storage_dir, discriminator=discriminator,
                         passcode=passcode, port=port, extra_args=combined_extra_args, kvs_path=kvs_path, f_stdout=f_stdout, f_stderr=f_stderr)

    def kill(self):
        self.p.send_signal(signal.SIGKILL)

    def get_pid(self) -> int:
        return self.p.pid

    def read_from_logs(self, pattern: str, before: int = 4, after: int = 4) -> list[dict]:
        """Search for a string a return the matches. 

        Args:
            pattern (str): _description_
            before (int, optional): Number of lines before the found line. Defaults to 4.
            after (int, optional): Number of lines after the found line. Defaults to 4.

        Raises:
            FileNotFoundError: _description_

        Returns:
            list[dict]: List with a dict of the info retrieved.
        """
        if not os.path.exists(self.log_file):
            raise FileNotFoundError

        # read all lines at the moment
        all_lines = None
        with open(self.log_file, 'rb') as fp:
            all_lines = fp.readlines()

        found_lines = []

        for index, line in enumerate(all_lines):
            n_line = line.decode("utf-8", 'replace')
            if pattern in n_line:
                before_lines = all_lines[(index-before):index]
                after_lines = all_lines[index+1:(index+after+1)]
                match = {
                    'before': list(map(lambda x: x.decode("utf-8"), before_lines)),
                    'match': n_line,
                    'line': index,
                    'after': list(map(lambda x: x.decode("utf-8"), after_lines))
                }
                found_lines.append(match)

        return found_lines
