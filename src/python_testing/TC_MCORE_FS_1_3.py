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
#     app-args: --app-admin=${FABRIC_ADMIN_APP} --app-bridge=${FABRIC_BRIDGE_APP} --stdin-pipe=dut-fsa-stdin --discriminator=1234
#     app-ready-pattern: "Successfully opened pairing window on the device"
#     script-args: >
#       --PICS src/app/tests/suites/certification/ci-pics-values
#       --storage-path admin_storage.json
#       --commissioning-method on-network
#       --discriminator 1234
#       --passcode 20202021
#       --string-arg th_server_no_uid_app_path:${LIGHTING_APP_NO_UNIQUE_ID} dut_fsa_stdin_pipe:dut-fsa-stdin
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#     factoryreset: true
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
from matter_testing_support import MatterBaseTest, TestStep, async_test_body, default_matter_test_main, type_matches
from mobly import asserts
from TC_MCORE_FS_1_1 import AppServer


class TC_MCORE_FS_1_3(MatterBaseTest):

    @property
    def default_timeout(self) -> int:
        # This test has some manual steps, so we need a longer timeout.
        return 200

    def setup_class(self):
        super().setup_class()

        self.th_server = None
        self.storage = None

        # Get the path to the TH_SERVER_NO_UID app from the user params.
        th_server_app = self.user_params.get("th_server_no_uid_app_path", None)
        if not th_server_app:
            asserts.fail("This test requires a TH_SERVER_NO_UID app. Specify app path with --string-arg th_server_no_uid_app_path:<path_to_app>")
        if not os.path.exists(th_server_app):
            asserts.fail(f"The path {th_server_app} does not exist")

        # Create a temporary storage directory for keeping KVS files.
        self.storage = tempfile.TemporaryDirectory(prefix=self.__class__.__name__)
        logging.info("Temporary storage directory: %s", self.storage.name)

        if self.is_pics_sdk_ci_only:
            # Get the named pipe path for the DUT_FSA app input from the user params.
            dut_fsa_stdin_pipe = self.user_params.get("dut_fsa_stdin_pipe")
            if not dut_fsa_stdin_pipe:
                asserts.fail("CI setup requires --string-arg dut_fsa_stdin_pipe:<path_to_pipe>")
            self.dut_fsa_stdin = open(dut_fsa_stdin_pipe, "w")

        self.th_server_port = 5544
        self.th_server_discriminator = random.randint(0, 4095)
        self.th_server_passcode = 20202021

        # Start the TH_SERVER_NO_UID app.
        self.th_server = AppServer(
            th_server_app,
            storage_dir=self.storage.name,
            port=self.th_server_port,
            discriminator=self.th_server_discriminator,
            passcode=self.th_server_passcode)
        self.th_server.start()

    def teardown_class(self):
        if self.th_server is not None:
            self.th_server.terminate()
        if self.storage is not None:
            self.storage.cleanup()
        super().teardown_class()

    def steps_TC_MCORE_FS_1_3(self) -> list[TestStep]:
        return [
            TestStep(0, "Commission DUT if not done", is_commissioning=True),
            TestStep(1, "TH commissions TH_SERVER_NO_UID to TH's fabric"),
            TestStep(2, "DUT_FSA commissions TH_SERVER_NO_UID to DUT_FSA's fabric and generates a UniqueID.",
                     "TH verifies a value is visible for the UniqueID from the DUT_FSA's Bridged Device Basic Information Cluster."),
        ]

    @async_test_body
    async def test_TC_MCORE_FS_1_3(self):

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

        # Get the list of endpoints on the DUT_FSA_BRIDGE before adding the TH_SERVER_NO_UID.
        dut_fsa_bridge_endpoints = set(await self.read_single_attribute_check_success(
            cluster=Clusters.Descriptor,
            attribute=Clusters.Descriptor.Attributes.PartsList,
            node_id=self.dut_node_id,
            endpoint=0,
        ))

        # Open commissioning window on TH_SERVER_NO_UID.
        discriminator = random.randint(0, 4095)
        params = await self.default_controller.OpenCommissioningWindow(
            nodeid=th_server_th_node_id,
            option=self.default_controller.CommissioningWindowPasscode.kTokenWithRandomPin,
            discriminator=discriminator,
            iteration=10000,
            timeout=600)

        # Commissioning TH_SERVER_NO_UID to DUT_FSA fabric.
        if not self.is_pics_sdk_ci_only:
            self.wait_for_user_input(
                f"Commission TH_SERVER_NO_UID on DUT using manufacturer specified mechanism.\n"
                f"Use the following parameters:\n"
                f"- discriminator: {discriminator}\n"
                f"- setupPinCode: {params.setupPinCode}\n"
                f"- setupQRCode: {params.setupQRCode}\n"
                f"- setupManualCode: {params.setupManualCode}\n"
                f"If using FabricSync Admin, you may type:\n"
                f">>> pairing onnetwork <desired_node_id> {params.setupPinCode}")
        else:
            self.dut_fsa_stdin.write(
                f"pairing onnetwork 10 {params.setupPinCode}\n")
            self.dut_fsa_stdin.flush()
            # Wait for the commissioning to complete.
            await asyncio.sleep(5)

        # Wait for the device to appear on the DUT_FSA_BRIDGE.
        await asyncio.sleep(1)

        # Get the list of endpoints on the DUT_FSA_BRIDGE after adding the TH_SERVER_NO_UID.
        dut_fsa_bridge_endpoints_new = set(await self.read_single_attribute_check_success(
            cluster=Clusters.Descriptor,
            attribute=Clusters.Descriptor.Attributes.PartsList,
            node_id=self.dut_node_id,
            endpoint=0,
        ))

        # Get the endpoint number for just added TH_SERVER_NO_UID.
        logging.info("Endpoints on DUT_FSA_BRIDGE: old=%s, new=%s", dut_fsa_bridge_endpoints, dut_fsa_bridge_endpoints_new)
        asserts.assert_true(dut_fsa_bridge_endpoints_new.issuperset(dut_fsa_bridge_endpoints),
                            "Expected only new endpoints to be added")
        unique_endpoints_set = dut_fsa_bridge_endpoints_new - dut_fsa_bridge_endpoints
        asserts.assert_equal(len(unique_endpoints_set), 1, "Expected only one new endpoint on DUT_FSA")
        dut_fsa_bridge_th_server_endpoint = list(unique_endpoints_set)[0]

        dut_fsa_bridge_th_server_unique_id = await self.read_single_attribute_check_success(
            cluster=Clusters.BridgedDeviceBasicInformation,
            attribute=Clusters.BridgedDeviceBasicInformation.Attributes.UniqueID,
            endpoint=dut_fsa_bridge_th_server_endpoint)
        asserts.assert_true(type_matches(dut_fsa_bridge_th_server_unique_id, str), "UniqueID should be a string")
        asserts.assert_true(dut_fsa_bridge_th_server_unique_id, "UniqueID should not be an empty string")
        logging.info("UniqueID generated for TH_SERVER_NO_UID: %s", dut_fsa_bridge_th_server_unique_id)


if __name__ == "__main__":
    default_matter_test_main()
