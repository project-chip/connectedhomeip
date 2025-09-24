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
import subprocess
from dataclasses import dataclass
from typing import Optional, Union

import matter.clusters as Clusters
from matter.ChipDeviceCtrl import ChipDeviceController
from matter.clusters.Types import NullValue
from matter.interaction_model import Status
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
                 passcode: int, port: int = 5540, extra_args: list[str] = []):
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
                             output_cb=lambda line, is_stderr: self.PREFIX + line)
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

    # DEFAULT_ADMIN_NODE_ID = 112233

    # Prefix for log messages from the OTA provider application.
    PREFIX = b"[OTA-PROVIDER]"

    def __init__(
        self,
        ota_file: str,
        discriminator: int,
        passcode: int,
        secured_device_port: int,
        queue: str = None,
        timeout: int = None,
        override_image_uri: str = None,
        log_file_path: str = "provider.log",
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
        """

        self.log_file_path = log_file_path

        # Path to provider binary and hardcoded KVS
        path_to_app = "./out/debug/chip-ota-provider-app"
        provider_kvs_path = "/tmp/chip_kvs_provider"

        # Build argument list exactly as before
        args = [
            f"--filepath={ota_file}" if isinstance(ota_file, str) else ota_file.path,
            f"--discriminator={discriminator}",
            f"--passcode={passcode}",
            f"--secured-device-port={secured_device_port}",
            f"--KVS={provider_kvs_path}",
        ]
        if queue:
            args += ["-q", queue]
        if timeout:
            args += ["-t", str(timeout)]
        if override_image_uri:
            args += ["-i", override_image_uri]

        # Clear the log file before starting
        open(log_file_path, "w").close()

        # Save args for use in the base constructor
        self._extra_args = args

        super().__init__(
            app=path_to_app,
            storage_dir="/tmp",
            discriminator=discriminator,
            passcode=passcode,
            port=secured_device_port,
            extra_args=args,
        )

    def _process_output(self, line: bytes, is_stderr: bool) -> bytes:
        """Write logs only to file, return empty bytes to avoid console output."""
        with open(self.log_file_path, "ab") as f:
            f.write(line)
            f.flush()
        return b""  # must return bytes, not None

    def start(self, expected_output: str = None, timeout: int = 30):
        """Override start to attach log processing callback."""
        self.output_cb = self._process_output
        super().start(expected_output=expected_output, timeout=timeout)

    # # ---------------- ACL METHODS ---------------- #

    # async def write_acl(self, controller, node_id: int, acl: list):
    #     """
    #     Writes the Access Control List (ACL) to the DUT device using the specified controller.

    #     Args:
    #         controller: The Matter controller (e.g., th1, th4) that will perform the write operation.
    #         acl (list): List of AccessControlEntryStruct objects defining the ACL permissions to write.
    #         node_id:

    #     Raises:
    #         AssertionError: If writing the ACL attribute fails (status is not Status.Success).
    #     """
    #     acl_attribute = Clusters.AccessControl.Attributes.Acl(acl)
    #     result = await controller.WriteAttribute(
    #         nodeid=node_id,
    #         attributes=[(0, acl_attribute)]
    #     )
    #     if result[0].Status != Status.Success:
    #         raise RuntimeError(f"ACL write failed for node {node_id}: {result[0].Status}")
    #     return True

    # def create_acl_entry(self, dev_ctrl: ChipDeviceController, provider_node_id: int, requestor_node_id: Optional[int] = None):
    #     """Create ACL entries to allow OTA requestors to access the provider.

    #     Args:
    #         dev_ctrl: Device controller for sending commands
    #         provider_node_id: Node ID of the OTA provider
    #         requestor_node_id: Optional specific requestor node ID for targeted access

    #     Returns:
    #         Result of the ACL write operation
    #     """
    #     # Standard ACL entry for OTA Provider cluster
    #     admin_node_id = dev_ctrl.nodeId if hasattr(dev_ctrl, 'nodeId') else self.DEFAULT_ADMIN_NODE_ID
    #     requestor_subjects = [requestor_node_id] if requestor_node_id else NullValue

    #     # Create ACL entries using proper struct constructors
    #     acl_entries = [
    #         # Admin entry
    #         Clusters.AccessControl.Structs.AccessControlEntryStruct(  # type: ignore
    #             privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kAdminister,  # type: ignore
    #             authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kCase,  # type: ignore
    #             subjects=[admin_node_id],  # type: ignore
    #             targets=NullValue
    #         ),
    #         # Operate entry
    #         Clusters.AccessControl.Structs.AccessControlEntryStruct(  # type: ignore
    #             privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kOperate,  # type: ignore
    #             authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kCase,  # type: ignore
    #             subjects=requestor_subjects,  # type: ignore
    #             targets=[
    #                 Clusters.AccessControl.Structs.AccessControlTargetStruct(  # type: ignore
    #                     cluster=Clusters.OtaSoftwareUpdateProvider.id,  # type: ignore
    #                     endpoint=NullValue,
    #                     deviceType=NullValue
    #                 )
    #             ],
    #         )
    #     ]

    #     # Create the attribute descriptor for the ACL attribute
    #     acl_attribute = Clusters.AccessControl.Attributes.Acl(acl_entries)

    #     return dev_ctrl.WriteAttribute(
    #         nodeid=provider_node_id,
    #         attributes=[(0, acl_attribute)]
    #     )

    # async def set_acl_for_requestor(
    #         self,
    #         controller,
    #         requestor_node: int,
    #         provider_node: int,
    #         fabric_index: int,
    #         original_requestor_acls: list
    # ):
    #     """
    #     Read existing ACLs on Requestor, add minimal ACL for Provider, and write back.
    #     """

    #     # Add minimal ACL for Provider
    #     acl_operate_provider = Clusters.AccessControl.Structs.AccessControlEntryStruct(
    #         fabricIndex=fabric_index,
    #         privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kOperate,
    #         authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kCase,
    #         subjects=[provider_node],
    #         targets=[Clusters.AccessControl.Structs.AccessControlTargetStruct(
    #             endpoint=0,
    #             cluster=Clusters.OtaSoftwareUpdateRequestor.id
    #         )]
    #     )

    #     # Combine existing + new ACLs
    #     combined_acls = original_requestor_acls + [acl_operate_provider]
    #     await self.write_acl(controller, requestor_node, combined_acls)

    #     return original_requestor_acls

    # async def set_acl_for_provider(
    #         self, controller,
    #         provider_node: int,
    #         requestor_node: int,
    #         fabric_index: int,
    #         original_provider_acls: list
    # ):
    #     """
    #     Read existing ACLs on Provider, add minimal ACL for Requestor, and write back.
    #     """

    #     # Add minimal ACL for Requestor
    #     acl_operate_requestor = Clusters.AccessControl.Structs.AccessControlEntryStruct(
    #         fabricIndex=fabric_index,
    #         privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kOperate,
    #         authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kCase,
    #         subjects=[requestor_node],
    #         targets=[Clusters.AccessControl.Structs.AccessControlTargetStruct(
    #             endpoint=0,
    #             cluster=Clusters.OtaSoftwareUpdateProvider.id
    #         )]
    #     )

    #     # Combine existing + new ACLs
    #     combined_acls = original_provider_acls + [acl_operate_requestor]
    #     await self.write_acl(controller, provider_node, combined_acls)

    #     return original_provider_acls

    # async def set_ota_acls_for_provider(
    #         self, controller,
    #         requestor_node: int,
    #         provider_node: int,
    #         fabric_index: int,
    #         original_requestor_acls: list,
    #         original_provider_acls: list
    # ):
    #     """
    #     Set ACLs both ways and preserve originals.
    #     """
    #     original_requestor_acls = await self.set_acl_for_requestor(controller, requestor_node, provider_node, fabric_index, original_requestor_acls)
    #     original_provider_acls = await self.set_acl_for_provider(controller, provider_node, requestor_node, fabric_index, original_provider_acls)
    #     return original_requestor_acls, original_provider_acls


class ACLHandler:
    """
    Utility class to handle Access Control List (ACL) operations for OTA Provider and Requestor.
    """

    DEFAULT_ADMIN_NODE_ID = 112233

    def __init__(self, controller):
        """
        Initialize the ACL handler.

        Args:
            controller: The controller used to perform ACL write operations.
        """
        self.controller = controller

    async def write_acl(self, node_id: int, acl: list):
        """
        Writes the Access Control List (ACL) to the DUT device using the specified controller.

        Args:
            node_id: Node ID of the target device (provider or requestor).
            acl (list): List of AccessControlEntryStruct objects defining ACL permissions.

        Raises:
            AssertionError: If writing the ACL attribute fails (status is not Status.Success).
        """
        acl_attribute = Clusters.AccessControl.Attributes.Acl(acl)
        result = await self.controller.WriteAttribute(
            nodeid=node_id,
            attributes=[(0, acl_attribute)]
        )
        if result[0].Status != Status.Success:
            raise RuntimeError(f"ACL write failed for node {node_id}: {result[0].Status}")
        return True

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

    async def set_acl_for_requestor(
            self,
            requestor_node: int,
            provider_node: int,
            fabric_index: int,
            original_requestor_acls: list
    ):
        """
        Read existing ACLs on Requestor, add minimal ACL for Provider, and write back.

        Args:
            requestor_node: Node ID of the Requestor device.
            provider_node: Node ID of the Provider device.
            fabric_index: Fabric index for the ACL entry.
            original_requestor_acls: Existing ACLs for the Requestor.

        Returns:
            The original requestor ACLs (unchanged).
        """

        # Add minimal ACL for Provider
        acl_operate_provider = Clusters.AccessControl.Structs.AccessControlEntryStruct(
            fabricIndex=fabric_index,
            privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kOperate,
            authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kCase,
            subjects=[provider_node],
            targets=[Clusters.AccessControl.Structs.AccessControlTargetStruct(
                endpoint=0,
                cluster=Clusters.OtaSoftwareUpdateRequestor.id
            )]
        )

        # Combine existing + new ACLs
        combined_acls = original_requestor_acls + [acl_operate_provider]
        await self.write_acl(requestor_node, combined_acls)

    async def set_acl_for_provider(
            self,
            provider_node: int,
            requestor_node: int,
            fabric_index: int,
            original_provider_acls: list
    ):
        """
        Read existing ACLs on Provider, add minimal ACL for Requestor, and write back.

        Args:
            provider_node: Node ID of the Provider device.
            requestor_node: Node ID of the Requestor device.
            fabric_index: Fabric index for the ACL entry.
            original_provider_acls: Existing ACLs for the Provider.

        Returns:
            The original provider ACLs (unchanged).
        """

        # Add minimal ACL for Requestor
        acl_operate_requestor = Clusters.AccessControl.Structs.AccessControlEntryStruct(
            fabricIndex=fabric_index,
            privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kOperate,
            authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kCase,
            subjects=[requestor_node],
            targets=[Clusters.AccessControl.Structs.AccessControlTargetStruct(
                endpoint=0,
                cluster=Clusters.OtaSoftwareUpdateProvider.id
            )]
        )

        # Combine existing + new ACLs
        combined_acls = original_provider_acls + [acl_operate_requestor]
        await self.write_acl(provider_node, combined_acls)

    async def set_ota_acls(
            self,
            requestor_node: int,
            provider_node: int,
            fabric_index: int,
            original_requestor_acls: list,
            original_provider_acls: list
    ):
        """
        Set ACLs both ways and preserve originals.

        Args:
            requestor_node: Node ID of the Requestor.
            provider_node: Node ID of the Provider.
            fabric_index: Fabric index for ACL entries.
            original_requestor_acls: Existing ACLs on Requestor.
            original_provider_acls: Existing ACLs on Provider.

        Returns:
            original_requestor_acls, original_provider_acls
        """
        await self.set_acl_for_requestor(requestor_node, provider_node, fabric_index, original_requestor_acls)
        await self.set_acl_for_provider(provider_node, requestor_node, fabric_index, original_provider_acls)
