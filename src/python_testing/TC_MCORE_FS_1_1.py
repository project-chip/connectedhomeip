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

import logging
import os
import random
import signal
import subprocess
import time
import uuid

import chip.clusters as Clusters
from chip import ChipDeviceCtrl
from matter_testing_support import MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from mobly import asserts


class TC_MCORE_FS_1_1(MatterBaseTest):

    @async_test_body
    async def setup_class(self):
        super().setup_class()
        # TODO: confirm whether we can open processes like this on the TH
        app = self.matter_test_config.user_params.get("th_server_app_path", None)
        if not app:
            asserts.fail('This test requires a TH_SERVER app. Specify app path with --string-arg th_server_app_path:<path_to_app>')

        self.kvs = f'kvs_{str(uuid.uuid4())}'
        self.port = 5543
        discriminator = random.randint(0, 4095)
        passcode = 20202021
        app_args = f'--secured-device-port {self.port} --discriminator {discriminator} --passcode {passcode} --KVS {self.kvs}'
        cmd = f'{app} {app_args}'
        # TODO: Determine if we want these logs cooked or pushed to somewhere else
        logging.info("Starting application to acts mock a server portion of TH_FSA")
        self.app_process = subprocess.Popen(cmd, bufsize=0, shell=True)
        logging.info("Started application to acts mock a server portion of TH_FSA")
        time.sleep(3)

        logging.info("Commissioning from separate fabric")
        # Create a second controller on a new fabric to communicate to the server
        new_certificate_authority = self.certificate_authority_manager.NewCertificateAuthority()
        new_fabric_admin = new_certificate_authority.NewFabricAdmin(vendorId=0xFFF1, fabricId=2)
        paa_path = str(self.matter_test_config.paa_trust_store_path)
        self.TH_server_controller = new_fabric_admin.NewController(nodeId=112233, paaTrustStorePath=paa_path)
        self.server_nodeid = 1111
        await self.TH_server_controller.CommissionOnNetwork(nodeId=self.server_nodeid, setupPinCode=passcode, filterType=ChipDeviceCtrl.DiscoveryFilterType.LONG_DISCRIMINATOR, filter=discriminator)
        logging.info("Commissioning TH_SERVER complete")

    def teardown_class(self):
        logging.warning("Stopping app with SIGTERM")
        self.app_process.send_signal(signal.SIGTERM.value)
        self.app_process.wait()

        os.remove(self.kvs)
        super().teardown_class()

    def steps_TC_MCORE_FS_1_1(self) -> list[TestStep]:
        steps = [TestStep(1, "Enable Fabric Synchronization on DUT_FSA using the manufacturer specified mechanism.", is_commissioning=True),
                 TestStep(2, "Commission DUT_FSA onto TH_FSA fabric."),
                 TestStep(3, "Reverse Commision Commission TH_FSAs onto DUT_FSA fabric."),
                 TestStep("3a", "TH_FSA sends RequestCommissioningApproval"),
                 TestStep("3b", "TH_FSA sends CommissionNode"),
                 TestStep("3c", "DUT_FSA commissions TH_FSA")]
        return steps

    @async_test_body
    async def test_TC_MCORE_FS_1_1(self):
        self.is_ci = self.check_pics('PICS_SDK_CI_ONLY')
        # TODO this value should either be determined or passed in from command line
        dut_commissioning_control_endpoint = 0
        self.step(1)
        self.step(2)
        self.step(3)
        th_fsa_server_fabrics = await self.read_single_attribute_check_success(cluster=Clusters.OperationalCredentials, attribute=Clusters.OperationalCredentials.Attributes.Fabrics, dev_ctrl=self.TH_server_controller, node_id=self.server_nodeid, endpoint=0)
        th_fsa_server_vid = await self.read_single_attribute_check_success(cluster=Clusters.BasicInformation, attribute=Clusters.BasicInformation.Attributes.VendorID, dev_ctrl=self.TH_server_controller, node_id=self.server_nodeid, endpoint=0)
        th_fsa_server_pid = await self.read_single_attribute_check_success(cluster=Clusters.BasicInformation, attribute=Clusters.BasicInformation.Attributes.ProductID, dev_ctrl=self.TH_server_controller, node_id=self.server_nodeid, endpoint=0)

        event_path = [(dut_commissioning_control_endpoint, Clusters.CommissionerControl.Events.CommissioningRequestResult, 1)]
        events = await self.default_controller.ReadEvent(nodeid=self.dut_node_id, events=event_path)

        self.step("3a")
        good_request_id = 0x1234567812345678
        cmd = Clusters.CommissionerControl.Commands.RequestCommissioningApproval(
            requestId=good_request_id, vendorId=th_fsa_server_vid, productId=th_fsa_server_pid, label="Test Ecosystem")
        await self.send_single_cmd(cmd, endpoint=dut_commissioning_control_endpoint)

        if not self.is_ci:
            self.wait_for_use_input("Approve Commissioning approval request on DUT using manufacturer specified mechanism")

        if not events:
            new_event = await self.default_controller.ReadEvent(nodeid=self.dut_node_id, events=event_path)
        else:
            event_nums = [e.Header.EventNumber for e in events]
            new_event = await self.default_controller.ReadEvent(nodeid=self.dut_node_id, events=event_path, eventNumberFilter=max(event_nums)+1)

        asserts.assert_equal(len(new_event), 1, "Unexpected event list len")
        asserts.assert_equal(new_event[0].Data.statusCode, 0, "Unexpected status code")
        asserts.assert_equal(new_event[0].Data.clientNodeId,
                             self.matter_test_config.controller_node_id, "Unexpected client node id")
        asserts.assert_equal(new_event[0].Data.requestId, good_request_id, "Unexpected request ID")

        self.step("3b")
        cmd = Clusters.CommissionerControl.Commands.CommissionNode(requestId=good_request_id, responseTimeoutSeconds=30)
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
        if not self.is_ci:
            time.sleep(30)

        th_fsa_server_fabrics_new = await self.read_single_attribute_check_success(cluster=Clusters.OperationalCredentials, attribute=Clusters.OperationalCredentials.Attributes.Fabrics, dev_ctrl=self.TH_server_controller, node_id=self.server_nodeid, endpoint=0)
        # TODO: this should be mocked too.
        if not self.is_ci:
            asserts.assert_equal(len(th_fsa_server_fabrics) + 1, len(th_fsa_server_fabrics_new),
                                 "Unexpected number of fabrics on TH_SERVER")


if __name__ == "__main__":
    default_matter_test_main()
