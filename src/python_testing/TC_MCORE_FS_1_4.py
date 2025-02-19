#
#    Copyright (c) 2024 Project CHIP Authors
#    All rights reserved.
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

# This test requires a TH_SERVER_NO_UID application that returns UnsupportedAttribute
# when reading UniqueID from BasicInformation Cluster. Please specify the app
# location with --string-arg th_server_no_uid_app_path:<path_to_app>

# See https://github.com/project-chip/connectedhomeip/blob/master/docs/testing/python.md#defining-the-ci-test-arguments
# for details about the block below.
#
# === BEGIN CI TEST ARGUMENTS ===
# test-runner-runs:
#   run1:
#     app: examples/fabric-admin/scripts/fabric-sync-app.py
#     app-args: --app-admin=${FABRIC_ADMIN_APP} --app-bridge=${FABRIC_BRIDGE_APP} --discriminator=1234
#     app-ready-pattern: "Successfully opened pairing window on the device"
#     app-stdin-pipe: dut-fsa-stdin
#     script-args: >
#       --PICS src/app/tests/suites/certification/ci-pics-values
#       --storage-path admin_storage.json
#       --commissioning-method on-network
#       --discriminator 1234 --passcode 20202021
#       --string-arg th_fsa_app_path:examples/fabric-admin/scripts/fabric-sync-app.py th_fsa_admin_path:${FABRIC_ADMIN_APP} th_fsa_bridge_path:${FABRIC_BRIDGE_APP} th_server_no_uid_app_path:${LIGHTING_APP_NO_UNIQUE_ID} dut_fsa_stdin_pipe:dut-fsa-stdin
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#     factory-reset: true
#     quiet: true
#   run2:
#     app: ${FABRIC_SYNC_APP}
#     app-args: --discriminator=1234
#     app-stdin-pipe: dut-fsa-stdin
#     script-args: >
#       --PICS src/app/tests/suites/certification/ci-pics-values
#       --storage-path admin_storage.json
#       --commissioning-method on-network
#       --discriminator 1234 --passcode 20202021
#       --bool-arg unified_fabric_sync_app:true
#       --string-arg th_fsa_app_path:examples/fabric-admin/scripts/fabric-sync-app.py
#       --string-arg th_fsa_admin_path:${FABRIC_ADMIN_APP}
#       --string-arg th_fsa_bridge_path:${FABRIC_BRIDGE_APP}
#       --string-arg th_server_no_uid_app_path:${LIGHTING_APP_NO_UNIQUE_ID}
#       --string-arg dut_fsa_stdin_pipe:dut-fsa-stdin
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

import asyncio
import logging
import os
import random
import tempfile

import chip.clusters as Clusters
from chip import ChipDeviceCtrl
from chip.interaction_model import Status
from chip.testing.apps import AppServerSubprocess
from chip.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main, type_matches
from chip.testing.tasks import Subprocess
from mobly import asserts


class FabricSyncApp(Subprocess):

    def __init__(self, fabric_sync_app_path: str, fabric_admin_app_path: str,
                 fabric_bridge_app_path: str, storage_dir: str, paa_trust_store_path: str = None,
                 fabric_name: str = None, node_id: int = None, vendor_id: int = None,
                 bridge_discriminator: int = None, bridge_passcode: int = None,
                 bridge_port: int = 5540):
        args = [
            f"--app-admin={fabric_admin_app_path}",
            f"--app-bridge={fabric_bridge_app_path}",
            # Override default ports, so it will be possible to run
            # our TH_FSA alongside the DUT_FSA during CI testing.
            "--app-admin-rpc-port=44000",
            "--app-bridge-rpc-port=44001",
            # Keep the storage directory in a temporary location.
            f"--storage-dir={storage_dir}",
        ]
        if paa_trust_store_path is not None:
            args.append(f"--paa-trust-store-path={paa_trust_store_path}")
        if fabric_name is not None:
            args.append(f"--commissioner-name={fabric_name}")
        if node_id is not None:
            args.append(f"--commissioner-node-id={node_id}")
        if vendor_id is not None:
            args.append(f"--commissioner-vendor-id={vendor_id}")
        if bridge_port is not None:
            args.append(f"--secured-device-port={bridge_port}")
        if bridge_discriminator is not None:
            args.append(f"--discriminator={bridge_discriminator}")
        if bridge_passcode is not None:
            args.append(f"--passcode={bridge_passcode}")
        # Start the FSA application with dedicated storage and RPC ports.
        super().__init__(fabric_sync_app_path, *args)

    def start(self):
        # Start process and block until it prints the expected output.
        super().start(expected_output="Successfully opened pairing window on the device")

    def commission_on_network(self, node_id: int, setup_pin_code: int, filter_type=None, filter=None):
        self.send(f"pairing onnetwork {node_id} {setup_pin_code}")


class TC_MCORE_FS_1_4(MatterBaseTest):

    @property
    def default_timeout(self) -> int:
        # This test has some manual steps, so we need a longer timeout.
        return 200

    def setup_class(self):
        super().setup_class()

        self.th_fsa_controller = None
        self.th_server = None
        self.storage = None

        # Get the path to the TH_FSA (fabric-admin and fabric-bridge) app from the user params.
        th_fsa_app_path = self.user_params.get("th_fsa_app_path")
        if not th_fsa_app_path:
            asserts.fail("This test requires a TH_FSA app. Specify app path with --string-arg th_fsa_app_path:<path_to_app>")
        if not os.path.exists(th_fsa_app_path):
            asserts.fail(f"The path {th_fsa_app_path} does not exist")
        th_fsa_admin_path = self.user_params.get("th_fsa_admin_path")
        if not th_fsa_admin_path:
            asserts.fail("This test requires a TH_FSA_ADMIN app. Specify app path with --string-arg th_fsa_admin_path:<path_to_app>")
        if not os.path.exists(th_fsa_admin_path):
            asserts.fail(f"The path {th_fsa_admin_path} does not exist")
        th_fsa_bridge_path = self.user_params.get("th_fsa_bridge_path")
        if not th_fsa_bridge_path:
            asserts.fail("This test requires a TH_FSA_BRIDGE app. Specify app path with --string-arg th_fsa_bridge_path:<path_to_app>")
        if not os.path.exists(th_fsa_bridge_path):
            asserts.fail(f"The path {th_fsa_bridge_path} does not exist")

        # Get the path to the TH_SERVER_NO_UID app from the user params.
        th_server_no_uid_app = self.user_params.get("th_server_no_uid_app_path", None)
        if not th_server_no_uid_app:
            asserts.fail("This test requires a TH_SERVER_NO_UID app. Specify app path with --string-arg th_server_no_uid_app_path:<path_to_app>")
        if not os.path.exists(th_server_no_uid_app):
            asserts.fail(f"The path {th_server_no_uid_app} does not exist")

        # Create a temporary storage directory for keeping KVS files.
        self.storage = tempfile.TemporaryDirectory(prefix=self.__class__.__name__)
        logging.info("Temporary storage directory: %s", self.storage.name)

        self.th_fsa_bridge_address = "::1"
        self.th_fsa_bridge_port = 5543
        # Random discriminator between 0 and MAX - 1. The one-less is to save
        # a room for the TH_SERVER_NO_UID discriminator.
        self.th_fsa_bridge_discriminator = random.randint(0, 4094)
        self.th_fsa_bridge_passcode = 20202021

        self.th_fsa_controller = FabricSyncApp(
            th_fsa_app_path,
            th_fsa_admin_path,
            th_fsa_bridge_path,
            storage_dir=self.storage.name,
            paa_trust_store_path=self.matter_test_config.paa_trust_store_path,
            bridge_port=self.th_fsa_bridge_port,
            bridge_discriminator=self.th_fsa_bridge_discriminator,
            bridge_passcode=self.th_fsa_bridge_passcode,
            vendor_id=0xFFF1)
        self.th_fsa_controller.start()

        if self.is_pics_sdk_ci_only:
            # Get the named pipe path for the DUT_FSA app input from the user params.
            dut_fsa_stdin_pipe = self.user_params.get("dut_fsa_stdin_pipe")
            if not dut_fsa_stdin_pipe:
                asserts.fail("CI setup requires --string-arg dut_fsa_stdin_pipe:<path_to_pipe>")
            self.dut_fsa_stdin = open(dut_fsa_stdin_pipe, "w")

        self.th_server_port = 5544
        self.th_server_discriminator = self.th_fsa_bridge_discriminator + 1
        self.th_server_passcode = 20202022

        # Start the TH_SERVER_NO_UID app.
        self.th_server = AppServerSubprocess(
            th_server_no_uid_app,
            storage_dir=self.storage.name,
            port=self.th_server_port,
            discriminator=self.th_server_discriminator,
            passcode=self.th_server_passcode)
        self.th_server.start(
            expected_output="Server initialization complete",
            timeout=30)

    def teardown_class(self):
        if self.th_fsa_controller is not None:
            self.th_fsa_controller.terminate()
        if self.th_server is not None:
            self.th_server.terminate()
        if self.storage is not None:
            self.storage.cleanup()
        super().teardown_class()

    def steps_TC_MCORE_FS_1_4(self) -> list[TestStep]:
        return [
            TestStep(0, "Commission DUT if not done", is_commissioning=True),
            TestStep(1, "TH commissions TH_SERVER_NO_UID to TH's fabric.",
                     "TH verifies that the TH_SERVER_NO_UID does not provide a UniqueID."),
            TestStep(2, "TH instructs TH_FSA to commission TH_SERVER_NO_UID to TH_FSA's fabric."),
            TestStep(3, "TH instructs TH_FSA to open up commissioning window on it's aggregator."),
            TestStep(4, "Follow manufacturer provided instructions to have DUT_FSA commission TH_FSA's aggregator."),
            TestStep(5, "Follow manufacturer provided instructions to enable DUT_FSA to synchronize TH_SERVER_NO_UID"
                     " from TH_FSA onto DUT_FSA's fabric. TH to provide endpoint saved from step 2 in user prompt."),
            TestStep(6, "DUT_FSA synchronizes TH_SERVER_NO_UID onto DUT_FSA's fabric and copies the UniqueID presented"
                     " by TH_FSA's Bridged Device Basic Information Cluster."),
        ]

    @async_test_body
    async def test_TC_MCORE_FS_1_4(self):

        # Commissioning - done
        self.step(0)

        self.step(1)

        th_server_th_node_id = 1

        await self.default_controller.CommissionOnNetwork(
            nodeId=th_server_th_node_id,
            setupPinCode=self.th_server_passcode,
            filterType=ChipDeviceCtrl.DiscoveryFilterType.LONG_DISCRIMINATOR,
            filter=self.th_server_discriminator,
        )

        await self.read_single_attribute_expect_error(
            cluster=Clusters.BasicInformation,
            attribute=Clusters.BasicInformation.Attributes.UniqueID,
            node_id=th_server_th_node_id,
            error=Status.UnsupportedAttribute,
        )

        self.step(2)

        th_fsa_bridge_th_node_id = 2
        # Commissioning TH_FSA_BRIDGE to TH fabric.
        await self.default_controller.CommissionOnNetwork(
            nodeId=th_fsa_bridge_th_node_id,
            setupPinCode=self.th_fsa_bridge_passcode,
            filterType=ChipDeviceCtrl.DiscoveryFilterType.LONG_DISCRIMINATOR,
            filter=self.th_fsa_bridge_discriminator,
        )

        # Get the list of endpoints on the TH_FSA_BRIDGE before adding the TH_SERVER_NO_UID.
        th_fsa_bridge_endpoints = set(await self.read_single_attribute_check_success(
            cluster=Clusters.Descriptor,
            attribute=Clusters.Descriptor.Attributes.PartsList,
            node_id=th_fsa_bridge_th_node_id,
            endpoint=0,
        ))

        discriminator = random.randint(0, 4095)
        # Open commissioning window on TH_SERVER_NO_UID.
        params = await self.default_controller.OpenCommissioningWindow(
            nodeid=th_server_th_node_id,
            option=self.default_controller.CommissioningWindowPasscode.kTokenWithRandomPin,
            discriminator=discriminator,
            iteration=10000,
            timeout=600)

        th_server_th_fsa_node_id = 3
        # Commissioning TH_SERVER_NO_UID to TH_FSA.
        self.th_fsa_controller.commission_on_network(
            node_id=th_server_th_fsa_node_id,
            setup_pin_code=params.setupPinCode,
            filter_type=ChipDeviceCtrl.DiscoveryFilterType.LONG_DISCRIMINATOR,
            filter=discriminator,
        )

        get_dynamic_endpoint_retries = 60
        th_fsa_bridge_endpoints_new = set(th_fsa_bridge_endpoints)
        # Try to get the dynamic endpoint number for the TH_SERVER_NO_UID on the TH_FSA_BRIDGE.
        while th_fsa_bridge_endpoints_new == th_fsa_bridge_endpoints and get_dynamic_endpoint_retries > 0:
            await asyncio.sleep(0.5)
            get_dynamic_endpoint_retries -= 1
            # Get the list of endpoints on the TH_FSA_BRIDGE.
            th_fsa_bridge_endpoints_new.update(await self.read_single_attribute_check_success(
                cluster=Clusters.Descriptor,
                attribute=Clusters.Descriptor.Attributes.PartsList,
                node_id=th_fsa_bridge_th_node_id,
                endpoint=0,
            ))

        # Get the endpoint number for just added TH_SERVER_NO_UID.
        logging.info("Endpoints on TH_FSA_BRIDGE: old=%s, new=%s", th_fsa_bridge_endpoints, th_fsa_bridge_endpoints_new)
        asserts.assert_true(th_fsa_bridge_endpoints_new.issuperset(th_fsa_bridge_endpoints),
                            "Expected only new endpoints to be added")
        unique_endpoints_set = th_fsa_bridge_endpoints_new - th_fsa_bridge_endpoints
        asserts.assert_equal(len(unique_endpoints_set), 1, "Expected only one new endpoint")
        th_fsa_bridge_th_server_endpoint = list(unique_endpoints_set)[0]

        # Verify that TH_FSA created a UniqueID for TH_SERVER_NO_UID.
        th_fsa_bridge_th_server_unique_id = await self.read_single_attribute_check_success(
            cluster=Clusters.BridgedDeviceBasicInformation,
            attribute=Clusters.BridgedDeviceBasicInformation.Attributes.UniqueID,
            node_id=th_fsa_bridge_th_node_id,
            endpoint=th_fsa_bridge_th_server_endpoint)
        asserts.assert_true(type_matches(th_fsa_bridge_th_server_unique_id, str), "UniqueID should be a string")
        asserts.assert_true(th_fsa_bridge_th_server_unique_id, "UniqueID should not be an empty string")
        logging.info("UniqueID generated for TH_SERVER_NO_UID: %s", th_fsa_bridge_th_server_unique_id)

        self.step(3)

        discriminator = random.randint(0, 4095)
        # Open commissioning window on TH_FSA_BRIDGE.
        params = await self.default_controller.OpenCommissioningWindow(
            nodeid=th_fsa_bridge_th_node_id,
            option=self.default_controller.CommissioningWindowPasscode.kTokenWithRandomPin,
            discriminator=discriminator,
            iteration=10000,
            timeout=600)

        self.step(4)

        # Commissioning TH_FSA_BRIDGE to DUT_FSA fabric.
        if not self.is_pics_sdk_ci_only:
            self.wait_for_user_input(
                f"Commission TH_FSA's aggregator on DUT using manufacturer specified mechanism.\n"
                f"Use the following parameters:\n"
                f"- discriminator: {discriminator}\n"
                f"- setupPinCode: {params.setupPinCode}\n"
                f"- setupQRCode: {params.setupQRCode}\n"
                f"- setupManualCode: {params.setupManualCode}\n"
                f"If using FabricSync Admin, you may type:\n"
                f">>> fabricsync add-bridge <desired_node_id> {params.setupPinCode} <th_host_ip> {self.th_fsa_bridge_port}")
        else:
            if self.user_params.get("unified_fabric_sync_app"):
                self.dut_fsa_stdin.write(
                    f"app add-bridge 10 {params.setupPinCode} {self.th_fsa_bridge_address} {self.th_fsa_bridge_port}\n")
            else:
                self.dut_fsa_stdin.write(
                    f"fabricsync add-bridge 10 {params.setupPinCode} {self.th_fsa_bridge_address} {self.th_fsa_bridge_port}\n")
            self.dut_fsa_stdin.flush()
            # Wait for the commissioning to complete.
            await asyncio.sleep(5)

        self.step(5)

        # Get the list of endpoints on the DUT_FSA_BRIDGE before synchronization.
        dut_fsa_bridge_endpoints = set(await self.read_single_attribute_check_success(
            cluster=Clusters.Descriptor,
            attribute=Clusters.Descriptor.Attributes.PartsList,
            node_id=self.dut_node_id,
            endpoint=0,
        ))

        # Synchronize TH_SERVER_NO_UID from TH_FSA to DUT_FSA fabric.
        if not self.is_pics_sdk_ci_only:
            self.wait_for_user_input(
                f"Synchronize endpoint from TH_FSA's aggregator to DUT using manufacturer specified mechanism.\n"
                f"Use the following parameters:\n"
                f"- endpointID: {th_fsa_bridge_th_server_endpoint}\n"
                f"If using FabricSync Admin, you may type:\n"
                f">>> fabricsync sync-device {th_fsa_bridge_th_server_endpoint}")
        else:
            if self.user_params.get("unified_fabric_sync_app"):
                self.dut_fsa_stdin.write(f"app sync-device {th_fsa_bridge_th_server_endpoint}\n")
            else:
                self.dut_fsa_stdin.write(f"fabricsync sync-device {th_fsa_bridge_th_server_endpoint}\n")
            self.dut_fsa_stdin.flush()
            # Wait for the synchronization to complete.
            await asyncio.sleep(5)

        self.step(6)

        # Get the list of endpoints on the DUT_FSA_BRIDGE after synchronization
        dut_fsa_bridge_endpoints_new = set(await self.read_single_attribute_check_success(
            cluster=Clusters.Descriptor,
            attribute=Clusters.Descriptor.Attributes.PartsList,
            node_id=self.dut_node_id,
            endpoint=0,
        ))

        # Get the endpoint number for just synced TH_SERVER_NO_UID.
        logging.info("Endpoints on DUT_FSA_BRIDGE: old=%s, new=%s", dut_fsa_bridge_endpoints, dut_fsa_bridge_endpoints_new)
        asserts.assert_true(dut_fsa_bridge_endpoints_new.issuperset(dut_fsa_bridge_endpoints),
                            "Expected only new endpoints to be added")
        unique_endpoints_set = dut_fsa_bridge_endpoints_new - dut_fsa_bridge_endpoints
        asserts.assert_equal(len(unique_endpoints_set), 1, "Expected exactly one new endpoint on DUT_FSA")
        dut_fsa_bridge_th_server_endpoint = list(unique_endpoints_set)[0]

        # Verify that DUT_FSA copied the TH_SERVER_NO_UID UniqueID from TH_FSA.
        dut_fsa_bridge_th_server_unique_id = await self.read_single_attribute_check_success(
            cluster=Clusters.BridgedDeviceBasicInformation,
            attribute=Clusters.BridgedDeviceBasicInformation.Attributes.UniqueID,
            endpoint=dut_fsa_bridge_th_server_endpoint)
        asserts.assert_true(type_matches(dut_fsa_bridge_th_server_unique_id, str), "UniqueID should be a string")
        asserts.assert_true(dut_fsa_bridge_th_server_unique_id, "UniqueID should not be an empty string")
        logging.info("UniqueID for TH_SERVER_NO_UID on DUT_FSA: %s", th_fsa_bridge_th_server_unique_id)

        # Make sure that the UniqueID on the DUT_FSA_BRIDGE is the same as the one on the DUT_FSA_BRIDGE.
        asserts.assert_equal(dut_fsa_bridge_th_server_unique_id, th_fsa_bridge_th_server_unique_id,
                             "UniqueID on DUT_FSA and TH_FSA should be the same")


if __name__ == "__main__":
    default_matter_test_main()
