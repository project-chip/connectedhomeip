# Copyright (c) 2025 Project CHIP Authors
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

import subprocess
import logging
import os
import signal
import tempfile
from dataclasses import dataclass
from typing import Optional

from mobly import asserts

import matter.clusters as Clusters
from matter.ChipDeviceCtrl import ChipDeviceController, DiscoveryFilterType
from matter.clusters.Types import NullValue
from matter.interaction_model import Status
from matter.testing.tasks import Subprocess

# Create a logger
logger = logging.getLogger(__name__)


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
                 passcode: int, port: int = 5540, extra_args: list[str] = [], kvs_path: str = None):
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
        queue: str = None,
        timeout: int = None,
        override_image_uri: str = None,
        log_file_path: str = "provider.log",
        app_path: str = None,
        kvs_path: str = None,
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
        open(log_file_path, "w").close()

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
        with open(self.log_file_path, "ab") as f:
            f.write(line)
            f.flush()
        return b""  # must return bytes, not None

    def start(self, expected_output: str = None, timeout: int = 30):
        """Override start to attach log processing callback."""
        self.output_cb = self._process_output
        super().start(expected_output=expected_output, timeout=timeout)

    def terminate(self):
        """Override terminate to ensure log file is closed."""
        super().terminate()
        # No explicit file close needed since we open/close on each write.


class ACLHandler:
    """
    Utility class to handle Access Control List (ACL) operations for OTA Provider and Requestor.
    """

    DEFAULT_ADMIN_NODE_ID = 112233

    def __init__(self, controller: ChipDeviceController):
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


class OTAHelper:
    """Helper to handle OTA Provider setup, commissioning, and ACLs."""

    def __init__(self, log_file_path: str, app_path: str, kvs_path: str, acl_handler: 'ACLHandler'):
        """
        Args:
            log_file_path: Log file path for provider logs.
            app_path: Path to chip-ota-provider-app binary.
            kvs_path: Optional KVS path to reuse across steps.
            acl_handler: Instance of ACLHandler to configure ACLs.
        """
        self.log_file_path = log_file_path
        self.app_path = app_path
        self.kvs_path = kvs_path
        self.acl_handler = acl_handler

    async def read_single_attribute(self, controller, node_id, endpoint, attribute, fabric_filtered=False):
        """
        Reads a single attribute from a node using the controller directly.

        Args:
            controller: The controller to use for reading the attribute
            node_id: Node ID of the device
            endpoint: Endpoint of the cluster
            attribute: Attribute to read
            fabric_filtered: Whether to filter by fabric ID (optional)

        Returns:
            The value of the attribute read
        """
        result = await controller.ReadAttribute(node_id, [(endpoint, attribute)], fabricFiltered=fabric_filtered)
        data = result[endpoint]
        return list(data.values())[0][attribute]

    async def read_single_attribute_check_success(self, controller, node_id, endpoint, attribute):
        """
        Reads a single attribute and raises an error if it fails.

        Args:
            controller: The controller to use for reading the attribute
            node_id: Node ID of the device
            endpoint: Endpoint of the cluster
            attribute: Attribute to read

        Returns:
            The value of the attribute if successful
        """
        value = await self.read_single_attribute(controller, node_id, endpoint, attribute)
        if value is None:
            raise RuntimeError(f"Attribute read failed: {attribute}")
        return value

    async def add_single_ota_provider(self, controller, requestor_node_id: int, provider_node_id: int):
        """
        Adds a single OTA provider to the Requestor's DefaultOTAProviders attribute
        only if no provider is currently registered. If a provider already exists,
        the function does nothing.

        Args:
            controller: The controller to use for reading and writing attributes.
            requestor_node_id (int): Node ID of the Requestor device.
            provider_node_id (int): Node ID of the OTA Provider to add.

        Returns:
            None
        """
        # Read existing DefaultOTAProviders on the Requestor
        current_providers = await self.read_single_attribute_check_success(
            controller=controller,
            node_id=requestor_node_id,
            endpoint=0,
            attribute=Clusters.OtaSoftwareUpdateRequestor.Attributes.DefaultOTAProviders
        )
        logger.info(f'Prerequisite #5.0 - Current DefaultOTAProviders on Requestor: {current_providers}')

        # If there is already a provider, skip adding
        if current_providers:
            logger.info(f'Skipping add: Requestor already has a provider registered ({current_providers})')
            return

        # Create a ProviderLocation for the new provider
        provider_location = Clusters.OtaSoftwareUpdateRequestor.Structs.ProviderLocation(
            providerNodeID=provider_node_id,
            endpoint=0,
            fabricIndex=controller.fabricId
        )
        logger.info(f'Prerequisite #5.0 - ProviderLocation to add: {provider_location}')

        # Combine with existing providers (preserving previous ones)
        updated_providers = current_providers + [provider_location]

        # Write the updated DefaultOTAProviders list back to the Requestor
        attr = Clusters.OtaSoftwareUpdateRequestor.Attributes.DefaultOTAProviders(value=updated_providers)
        resp = await controller.WriteAttribute(
            attributes=[(0, attr)],
            nodeid=requestor_node_id
        )
        logger.info(f'Prerequisite #5.0 - Write DefaultOTAProviders response: {resp}')
        asserts.assert_equal(resp[0].Status, Status.Success, "Failed to write DefaultOTAProviders attribute")

    async def clear_ota_providers(self, controller, requestor_node_id: int):
        """
        Clears the DefaultOTAProviders attribute on the Requestor, leaving it empty.

        Args:
            controller: The controller to use for writing attributes.
            requestor_node_id (int): Node ID of the Requestor device.

        Returns:
            None
        """
        # Set DefaultOTAProviders to empty list
        attr_clear = Clusters.OtaSoftwareUpdateRequestor.Attributes.DefaultOTAProviders(value=[])
        resp = await controller.WriteAttribute(
            attributes=[(0, attr_clear)],
            nodeid=requestor_node_id
        )
        logger.info('Cleanup - DefaultOTAProviders cleared')

        assert resp[0].Status == Status.Success, "Failed to clear DefaultOTAProviders"

    async def setup_provider(
        self,
        controller: ChipDeviceController,
        fabric_id,
        requestor_node_id,
        provider_node_id,
        provider_discriminator,
        provider_setup_pin_code,
        provider_port,
        provider_ota_file,
        provider_wait_for,
        provider_queue,
        provider_timeout,
        provider_override_image_uri
    ):
        """
        Set up an OTA Provider Launch,  commissioning, and ACLs.

        Steps:
            1. Launch the OTA Provider process with given parameters.
            2. Commission the Provider onto the specified fabric.
            3. Configure ACLs on both Requestor and Provider to allow OTA cluster interactions.
            4. Add the Provider to the Requestor's DefaultOTAProviders attribute if none exists.

        Args:
            controller: Controller instance for commissioning and ACL configuration.
            fabric_id: Fabric index to associate with the Provider.
            requestor_node_id: Node ID of the OTA Requestor (DUT).
            provider_node_id: Node ID of the OTA Provider.
            provider_discriminator: Discriminator used for Provider discovery.
            provider_setup_pin_code: Setup PIN code for commissioning.
            provider_port: Port number for Provider process.
            provider_ota_file: Path to OTA image file served by Provider.
            provider_wait_for: Regex to wait for specific Provider log output.
            provider_queue: Queue used by the Provider.
            provider_timeout: Timeout for Provider.
            provider_override_image_uri: ImageURI for the OTA file.

        Returns:
            provider_proc: Process handle of the launched Provider.
        """

        logger.info(f"""Prerequisite #1.0 - Provider info:
            NodeID: {provider_node_id},
            discriminator: {provider_discriminator},
            setupPinCode: {provider_setup_pin_code},
            port: {provider_port},
            ota_file: {provider_ota_file}""")

        # Step 1: Launch Provider process
        provider_proc = OTAProviderSubprocess(
            ota_file=provider_ota_file,
            discriminator=provider_discriminator,
            passcode=provider_setup_pin_code,
            secured_device_port=provider_port,
            queue=provider_queue,
            timeout=provider_timeout,
            override_image_uri=provider_override_image_uri,
            log_file_path=self.log_file_path,
            app_path=self.app_path,
            kvs_path=self.kvs_path,
        )
        provider_proc.start(expected_output=provider_wait_for, timeout=300)
        logger.info(f"Prerequisite #2.0 - Launched Provider PID {provider_proc.p.pid}")

        # Step 2: Commission the Provider
        resp = await controller.CommissionOnNetwork(
            nodeId=provider_node_id,
            setupPinCode=provider_setup_pin_code,
            filterType=DiscoveryFilterType.LONG_DISCRIMINATOR,
            filter=provider_discriminator
        )
        logger.info(f'Prerequisite #3 - Provider Commissioning response: {resp}')

        # Step 3: Configure ACLs
        original_requestor_acls = await self.read_single_attribute(
            controller=controller,
            node_id=requestor_node_id,
            endpoint=0,
            attribute=Clusters.AccessControl.Attributes.Acl,
        )
        original_provider_acls = await self.read_single_attribute(
            controller=controller,
            node_id=provider_node_id,
            endpoint=0,
            attribute=Clusters.AccessControl.Attributes.Acl,
        )

        await self.acl_handler.set_ota_acls(
            requestor_node=requestor_node_id,
            provider_node=provider_node_id,
            fabric_index=fabric_id,
            original_requestor_acls=original_requestor_acls,
            original_provider_acls=original_provider_acls,
        )
        logger.info("Prerequisite #4.0 - Configure ACLs on Requestor and Provider")

        # Prerequisite #5.0 - Add OTA Provider to the Requestor
        await self.add_single_ota_provider(
            controller=controller,
            requestor_node_id=requestor_node_id,
            provider_node_id=provider_node_id,
        )
        logger.info("Prerequisite #5.0 - Added Provider to Requestor(DUT) DefaultOTAProviders")

        return provider_proc

    async def cleanup_provider(
        self,
        controller: ChipDeviceController,
        requestor_node_id,
        provider_node_id,
        provider_proc,
        original_requestor_acls,
        original_provider_acls
    ):
        """
        Cleanly shuts down the Provider process.
        Restores ACLs, expires sessions, stops the provider process, and deletes temporary KVS files.

        Args:
            controller: Controller object.
            requestor_node_id: Node ID of the requestor.
            provider_node_id: Node ID of the provider.
            provider_proc: Process handle of the provider to stop.
            original_requestor_acls: Original ACLs for the requestor.
            original_provider_acls: Original ACLs for the provider.

        Returns:
            None
        """
        # Clean Provider ACL
        await self.acl_handler.write_acl(provider_node_id, original_provider_acls)

        # Expire sessions
        controller.ExpireSessions(provider_node_id)

        # Clean Requestor ACL
        await self.acl_handler.write_acl(requestor_node_id, original_requestor_acls)

        # Kill Provider process
        provider_proc.terminate()

        # Delete KVS files
        subprocess.run("rm -rf /tmp/chip_kvs /tmp/chip_kvs-shm /tmp/chip_kvs-wal", shell=True)
        subprocess.run("rm -rf /tmp/chip_kvs_provider*", shell=True)
