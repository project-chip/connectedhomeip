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

# This test requires a TH_SERVER application. Please specify with --string-arg th_server_app_path:<path_to_app>

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
#       --discriminator 1234
#       --passcode 20202021
#       --string-arg th_server_app_path:${ALL_CLUSTERS_APP}
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
#       --discriminator 1234
#       --passcode 20202021
#       --string-arg th_server_app_path:${ALL_CLUSTERS_APP}
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

import logging
import os
import random
import tempfile
import time

import chip.clusters as Clusters
from chip import ChipDeviceCtrl
from chip.testing.apps import AppServerSubprocess
from chip.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from mobly import asserts

_DEVICE_TYPE_AGGREGATOR = 0x000E


class TC_MCORE_FS_1_1(MatterBaseTest):

    @async_test_body
    async def setup_class(self):
        super().setup_class()

        self.th_server = None
        self.storage = None

        th_server_app = self.user_params.get("th_server_app_path", None)
        if not th_server_app:
            asserts.fail("This test requires a TH_SERVER app. Specify app path with --string-arg th_server_app_path:<path_to_app>")
        if not os.path.exists(th_server_app):
            asserts.fail(f"The path {th_server_app} does not exist")

        # Create a temporary storage directory for keeping KVS files.
        self.storage = tempfile.TemporaryDirectory(prefix=self.__class__.__name__)
        logging.info("Temporary storage directory: %s", self.storage.name)

        self.th_server_port = 5543
        self.th_server_discriminator = random.randint(0, 4095)
        self.th_server_passcode = 20202021

        # Start the TH_SERVER app.
        self.th_server = AppServerSubprocess(
            th_server_app,
            storage_dir=self.storage.name,
            port=self.th_server_port,
            discriminator=self.th_server_discriminator,
            passcode=self.th_server_passcode)
        self.th_server.start(
            expected_output="Server initialization complete",
            timeout=30)

        logging.info("Commissioning from separate fabric")
        # Create a second controller on a new fabric to communicate to the server
        new_certificate_authority = self.certificate_authority_manager.NewCertificateAuthority()
        new_fabric_admin = new_certificate_authority.NewFabricAdmin(vendorId=0xFFF1, fabricId=2)
        paa_path = str(self.matter_test_config.paa_trust_store_path)
        self.TH_server_controller = new_fabric_admin.NewController(nodeId=112233, paaTrustStorePath=paa_path)
        self.server_nodeid = 1111
        await self.TH_server_controller.CommissionOnNetwork(
            nodeId=self.server_nodeid,
            setupPinCode=self.th_server_passcode,
            filterType=ChipDeviceCtrl.DiscoveryFilterType.LONG_DISCRIMINATOR,
            filter=self.th_server_discriminator)
        logging.info("Commissioning TH_SERVER complete")

    def teardown_class(self):
        if self.th_server is not None:
            self.th_server.terminate()
        if self.storage is not None:
            self.storage.cleanup()
        super().teardown_class()

    def steps_TC_MCORE_FS_1_1(self) -> list[TestStep]:
        return [
            TestStep(1, "Enable Fabric Synchronization on DUT_FSA using the manufacturer specified mechanism.", is_commissioning=True),
            TestStep(2, "Commission DUT_FSA onto TH_FSA fabric."),
            TestStep(3, "Reverse Commission TH_FSAs onto DUT_FSA fabric."),
            TestStep("3a", "TH_FSA sends RequestCommissioningApproval"),
            TestStep("3b", "TH_FSA sends CommissionNode"),
            TestStep("3c", "DUT_FSA commissions TH_FSA"),
        ]

    # This test has some manual steps and one sleep for up to 30 seconds. Test typically
    # runs under 1 mins, so 3 minutes is more than enough.
    @property
    def default_timeout(self) -> int:
        return 3*60

    @async_test_body
    async def test_TC_MCORE_FS_1_1(self):
        dut_commissioning_control_endpoint = 0

        # Get the list of endpoints on the DUT_FSA_BRIDGE before adding the TH_SERVER_NO_UID.
        dut_fsa_bridge_endpoints = set(await self.read_single_attribute_check_success(
            cluster=Clusters.Descriptor,
            attribute=Clusters.Descriptor.Attributes.PartsList,
            node_id=self.dut_node_id,
            endpoint=0,
        ))

        # Iterate through the endpoints on the DUT_FSA_BRIDGE
        for endpoint in dut_fsa_bridge_endpoints:
            # Read the DeviceTypeList attribute for the current endpoint
            device_type_list = await self.read_single_attribute_check_success(
                cluster=Clusters.Descriptor,
                attribute=Clusters.Descriptor.Attributes.DeviceTypeList,
                node_id=self.dut_node_id,
                endpoint=endpoint
            )

            # Check if any of the device types is an AGGREGATOR
            if any(device_type.deviceType == _DEVICE_TYPE_AGGREGATOR for device_type in device_type_list):
                dut_commissioning_control_endpoint = endpoint
                logging.info(f"Aggregator endpoint found: {dut_commissioning_control_endpoint}")
                break

        asserts.assert_not_equal(dut_commissioning_control_endpoint, 0, "Invalid aggregator endpoint. Cannot proceed with test.")

        self.step(1)
        self.step(2)
        self.step(3)
        th_fsa_server_fabrics = await self.read_single_attribute_check_success(cluster=Clusters.OperationalCredentials, attribute=Clusters.OperationalCredentials.Attributes.Fabrics, dev_ctrl=self.TH_server_controller, node_id=self.server_nodeid, endpoint=0, fabric_filtered=False)
        th_fsa_server_vid = await self.read_single_attribute_check_success(cluster=Clusters.BasicInformation, attribute=Clusters.BasicInformation.Attributes.VendorID, dev_ctrl=self.TH_server_controller, node_id=self.server_nodeid, endpoint=0)
        th_fsa_server_pid = await self.read_single_attribute_check_success(cluster=Clusters.BasicInformation, attribute=Clusters.BasicInformation.Attributes.ProductID, dev_ctrl=self.TH_server_controller, node_id=self.server_nodeid, endpoint=0)

        event_path = [(dut_commissioning_control_endpoint, Clusters.CommissionerControl.Events.CommissioningRequestResult, 1)]
        events = await self.default_controller.ReadEvent(nodeid=self.dut_node_id, events=event_path)

        self.step("3a")
        good_request_id = 0x1234567812345678
        cmd = Clusters.CommissionerControl.Commands.RequestCommissioningApproval(
            requestID=good_request_id, vendorID=th_fsa_server_vid, productID=th_fsa_server_pid, label="Test Ecosystem")
        await self.send_single_cmd(cmd, endpoint=dut_commissioning_control_endpoint)

        if not self.is_pics_sdk_ci_only:
            self.wait_for_user_input("Approve Commissioning approval request on DUT using manufacturer specified mechanism")

        if not events:
            new_event = await self.default_controller.ReadEvent(nodeid=self.dut_node_id, events=event_path)
        else:
            event_nums = [e.Header.EventNumber for e in events]
            new_event = await self.default_controller.ReadEvent(nodeid=self.dut_node_id, events=event_path, eventNumberFilter=max(event_nums)+1)

        asserts.assert_equal(len(new_event), 1, "Unexpected event list len")
        asserts.assert_equal(new_event[0].Data.statusCode, 0, "Unexpected status code")
        asserts.assert_equal(new_event[0].Data.clientNodeID,
                             self.matter_test_config.controller_node_id, "Unexpected client node id")
        asserts.assert_equal(new_event[0].Data.requestID, good_request_id, "Unexpected request ID")

        self.step("3b")
        cmd = Clusters.CommissionerControl.Commands.CommissionNode(requestID=good_request_id, responseTimeoutSeconds=30)
        resp = await self.send_single_cmd(cmd, endpoint=dut_commissioning_control_endpoint)
        asserts.assert_equal(type(resp), Clusters.CommissionerControl.Commands.ReverseOpenCommissioningWindow,
                             "Incorrect response type")

        # min commissioning timeout is 3*60 seconds, so use that even though the command said 30.
        cmd = Clusters.AdministratorCommissioning.Commands.OpenCommissioningWindow(commissioningTimeout=3*60,
                                                                                   PAKEPasscodeVerifier=resp.PAKEPasscodeVerifier,
                                                                                   discriminator=resp.discriminator,
                                                                                   iterations=resp.iterations, salt=resp.salt)
        await self.send_single_cmd(cmd, dev_ctrl=self.TH_server_controller, node_id=self.server_nodeid, endpoint=0, timedRequestTimeoutMs=5000)

        self.step("3c")
        max_wait_time_sec = 30
        start_time = time.time()
        elapsed = 0
        time_remaining = max_wait_time_sec
        previous_number_th_server_fabrics = len(th_fsa_server_fabrics)

        th_fsa_server_fabrics_new = None
        while time_remaining > 0:
            time.sleep(2)
            th_fsa_server_fabrics_new = await self.read_single_attribute_check_success(cluster=Clusters.OperationalCredentials, attribute=Clusters.OperationalCredentials.Attributes.Fabrics, dev_ctrl=self.TH_server_controller, node_id=self.server_nodeid, endpoint=0, fabric_filtered=False)
            if previous_number_th_server_fabrics != len(th_fsa_server_fabrics_new):
                break
            elapsed = time.time() - start_time
            time_remaining = max_wait_time_sec - elapsed

        asserts.assert_not_equal(th_fsa_server_fabrics_new, None, "Failed to read Fabrics attribute")
        asserts.assert_equal(previous_number_th_server_fabrics + 1, len(th_fsa_server_fabrics_new),
                             "Unexpected number of fabrics on TH_SERVER")


if __name__ == "__main__":
    default_matter_test_main()
