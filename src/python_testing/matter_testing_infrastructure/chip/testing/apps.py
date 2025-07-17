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
from typing import Optional

import chip.clusters as Clusters
from chip.testing.tasks import Subprocess


class AppServerSubprocess(Subprocess):
    """Wrapper class for starting an application server in a subprocess."""

    # Prefix for log messages from the application server.
    PREFIX = b"[SERVER]"

    def __init__(self, app: str, storage_dir: str, discriminator: int,
                 passcode: int, port: int = 5540, extra_args: list[str] = []):
        self.kvs_fd, kvs_path = tempfile.mkstemp(dir=storage_dir, prefix="kvs-app-")

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
                         output_cb=lambda line, is_stderr: self.PREFIX + line)

    def __del__(self):
        # Do not leak KVS file descriptor.
        os.close(self.kvs_fd)


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
                 passcode: int, port: int = 5540, ota_image_path: Optional[str] = None,
                 image_list_path: Optional[str] = None, extra_args: list[str] = []):
        """Initialize the OTA Provider subprocess.

        Args:
            app: Path to the chip-ota-provider-app executable
            storage_dir: Directory for persistent storage
            discriminator: Discriminator for commissioning
            passcode: Passcode for commissioning
            port: UDP port for secure connections
            ota_image_path: Path to a file containing an OTA image (cannot be used with image_list_path)
            image_list_path: Path to a file containing a list of OTA images (cannot be used with ota_image_path)
            extra_args: Additional command line arguments
        """

        if ota_image_path and image_list_path:
            raise ValueError("Cannot specify both ota_image_path and image_list_path")

        if not ota_image_path and not image_list_path:
            raise ValueError("Must specify either ota_image_path or image_list_path")

        # Build OTA-specific arguments
        ota_args = []
        if ota_image_path:
            ota_args.extend(["--filepath", ota_image_path])
        elif image_list_path:
            ota_args.extend(["--otaImageList", image_list_path])

        combined_extra_args = ota_args + extra_args

        # Initialize with the combined arguments
        super().__init__(app=app, storage_dir=storage_dir, discriminator=discriminator,
                         passcode=passcode, port=port, extra_args=combined_extra_args)

    def create_acl_entry(self, dev_ctrl, provider_node_id: int, requestor_node_id: Optional[int] = None):
        """Create ACL entries to allow OTA requestors to access the provider.

        Args:
            dev_ctrl: Device controller for sending commands
            provider_node_id: Node ID of the OTA provider
            requestor_node_id: Optional specific requestor node ID for targeted access

        Returns:
            Result of the ACL write operation
        """
        # Standard ACL entry for OTA Provider cluster
        acl_entries = [
            {
                "fabricIndex": 1,
                "privilege": Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kAdminister,
                "authMode": Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kCase,
                "subjects": [dev_ctrl.nodeId] if hasattr(dev_ctrl, 'nodeId') else [self.DEFAULT_ADMIN_NODE_ID],
                "targets": None
            },
            {
                "fabricIndex": 1,
                "privilege": Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kOperate,
                "authMode": Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kCase,
                "subjects": [requestor_node_id] if requestor_node_id else None,
                "targets": [{"cluster": Clusters.OtaSoftwareUpdateProvider.id, "endpoint": None, "deviceType": None}]
            }
        ]

        return dev_ctrl.WriteAttribute(
            nodeid=provider_node_id,
            attributes=[(0, Clusters.AccessControl.id, Clusters.AccessControl.Attributes.Acl.attribute_id, acl_entries)]
        )

    def __del__(self):
        # Override to safely handle kvs_fd that might not exist if constructor failed
        if hasattr(self, 'kvs_fd') and self.kvs_fd is not None:
            try:
                os.close(self.kvs_fd)
            except (OSError, AttributeError):
                pass  # Ignore errors during cleanup
