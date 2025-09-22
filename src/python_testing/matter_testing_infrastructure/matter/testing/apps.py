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
from sys import stdout
from typing import BinaryIO, Optional, Union

import matter.clusters as Clusters
from matter.ChipDeviceCtrl import ChipDeviceController
from matter.clusters.Types import NullValue
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
                 passcode: int, port: int = 5540, extra_args: list[str] = [], f_stdout: BinaryIO = stdout.buffer, f_stderr: BinaryIO = stdout.buffer):
        # Create a temporary KVS file and keep the descriptor to avoid leaks.
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
            os.close(self.kvs_fd)
            raise

    def __del__(self):
        # Do not leak KVS file descriptor.
        if hasattr(self, "kvs_fd"):
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

    DEFAULT_ADMIN_NODE_ID = 112233

    # Prefix for log messages from the OTA provider application.
    PREFIX = b"[OTA-PROVIDER]"

    def __init__(self, app: str, storage_dir: str, discriminator: int,
                 passcode: int, ota_source: Union[OtaImagePath, ImageListPath],
                 port: int = 5541, extra_args: list[str] = [], f_stdout: BinaryIO = stdout.buffer, f_stderr: BinaryIO = stdout.buffer):
        """Initialize the OTA Provider subprocess.

        Args:
            app: Path to the chip-ota-provider-app executable
            storage_dir: Directory for persistent storage
            discriminator: Discriminator for commissioning
            passcode: Passcode for commissioning
            port: UDP port for secure connections (default: 5541)
            ota_source: Either OtaImagePath or ImageListPath specifying the OTA image source
            extra_args: Additional command line arguments
        """

        # Build OTA-specific arguments using the ota_source property
        combined_extra_args = ota_source.ota_args + extra_args
        print(combined_extra_args)

        # Initialize with the combined arguments
        super().__init__(app=app, storage_dir=storage_dir, discriminator=discriminator,
                         passcode=passcode, port=port, extra_args=combined_extra_args, f_stdout=f_stdout, f_stderr=f_stderr)

    def kill(self):
        self.p.send_signal(signal.SIGKILL)

    def get_pid(self):
        self.p.pid

    def create_acl_entry(self, dev_ctrl: ChipDeviceController, provider_node_id: int, requestor_node_id: Optional[int] = None):
        """Create ACL entries to allow OTA requestors to access the provider.

        Args:
            dev_ctrl: Device controller for sending commands
            provider_node_id: Node ID of the OTA provider
            requestor_node_id: Optional specific requestor node ID for targeted access

        Returns:
            Result of the ACL write operation
        """
        # Standard ACL entry for OTA Provider cluster
        admin_node_id = dev_ctrl.nodeId if hasattr(dev_ctrl, 'nodeId') else self.DEFAULT_ADMIN_NODE_ID
        requestor_subjects = [requestor_node_id] if requestor_node_id else NullValue

        # Create ACL entries using proper struct constructors
        acl_entries = [
            # Admin entry
            Clusters.AccessControl.Structs.AccessControlEntryStruct(  # type: ignore
                privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kAdminister,  # type: ignore
                authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kCase,  # type: ignore
                subjects=[admin_node_id],  # type: ignore
                targets=NullValue
            ),
            # Operate entry
            Clusters.AccessControl.Structs.AccessControlEntryStruct(  # type: ignore
                privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kOperate,  # type: ignore
                authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kCase,  # type: ignore
                subjects=requestor_subjects,  # type: ignore
                targets=[
                    Clusters.AccessControl.Structs.AccessControlTargetStruct(  # type: ignore
                        cluster=Clusters.OtaSoftwareUpdateProvider.id,  # type: ignore
                        endpoint=NullValue,
                        deviceType=NullValue
                    )
                ],
            )
        ]

        # Create the attribute descriptor for the ACL attribute
        acl_attribute = Clusters.AccessControl.Attributes.Acl(acl_entries)

        return dev_ctrl.WriteAttribute(
            nodeid=provider_node_id,
            attributes=[(0, acl_attribute)]
        )
