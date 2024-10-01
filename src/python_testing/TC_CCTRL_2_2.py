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

# See https://github.com/project-chip/connectedhomeip/blob/master/docs/testing/python.md#defining-the-ci-test-arguments
# for details about the block below.
#
# TODO: Skip CI for now, we don't have any way to run this. Needs setup. See test_TC_CCTRL.py

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
from chip.interaction_model import InteractionModelError, Status
from matter_testing_support import (MatterBaseTest, TestStep, async_test_body, default_matter_test_main, has_cluster,
                                    run_if_endpoint_matches)
from mobly import asserts


class TC_CCTRL_2_2(MatterBaseTest):

    @async_test_body
    async def setup_class(self):
        super().setup_class()
        self.app_process = None
        app = self.user_params.get("th_server_app_path", None)
        if not app:
            asserts.fail('This test requires a TH_SERVER app. Specify app path with --string-arg th_server_app_path:<path_to_app>')

        self.kvs = f'kvs_{str(uuid.uuid4())}'
        self.port = 5543
        discriminator = random.randint(0, 4095)
        passcode = 20202021
        cmd = [app]
        cmd.extend(['--secured-device-port', str(5543)])
        cmd.extend(['--discriminator', str(discriminator)])
        cmd.extend(['--passcode', str(passcode)])
        cmd.extend(['--KVS', self.kvs])
        # TODO: Determine if we want these logs cooked or pushed to somewhere else
        logging.info("Starting TH_SERVER")
        self.app_process = subprocess.Popen(cmd)
        logging.info("TH_SERVER started")
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
        # In case the th_server_app_path does not exist, then we failed the test
        # and there is nothing to remove
        if self.app_process is not None:
            logging.warning("Stopping app with SIGTERM")
            self.app_process.send_signal(signal.SIGTERM.value)
            self.app_process.wait()

            if os.path.exists(self.kvs):
                os.remove(self.kvs)

        super().teardown_class()

    def steps_TC_CCTRL_2_2(self) -> list[TestStep]:
        steps = [TestStep(1, "Get number of fabrics from TH_SERVER", is_commissioning=True),
                 TestStep(2, "Reading Attribute VendorId from TH_SERVER"),
                 TestStep(3, "Reading Attribute ProductId from TH_SERVER"),
                 TestStep(4, "Reading Event CommissioningRequestResult from DUT"),
                 TestStep(5, "Send CommissionNode command to DUT with CASE session"),
                 TestStep(6, "Send OpenCommissioningWindow command on Administrator Commissioning Cluster to DUT with CASE session"),
                 TestStep(7, "Send CommissionNode command to DUT with PASE session"),
                 TestStep(8, "Send RequestCommissioningApproval command to DUT with PASE session"),
                 TestStep(9, "Send RevokeCommissioning command on Administrator Commissioning Cluster to DUT with CASE session"),
                 TestStep(10, "Reading Event CommissioningRequestResult from DUT, confirm no new events"),
                 TestStep(11, "Send RequestCommissioningApproval command to DUT with CASE session with incorrect vendorID"),
                 TestStep(12, "(Manual Step) Approve Commissioning Approval Request on DUT using method indicated by the manufacturer"),
                 TestStep(13, "Reading Event CommissioningRequestResult from DUT, confirm one new event"),
                 TestStep(14, "Send CommissionNode command to DUT with CASE session, with invalid RequestId"),
                 TestStep(15, "Send CommissionNode command to DUT with CASE session, with invalid ResponseTimeoutSeconds too low"),
                 TestStep(16, "Send CommissionNode command to DUT with CASE session, with invalid ResponseTimeoutSeconds too high"),
                 TestStep(17, "Send CommissionNode command to DUT with CASE session, with valid parameters"),
                 TestStep(18, "Send OpenCommissioningWindow command on Administrator Commissioning Cluster sent to TH_SERVER"),
                 TestStep(19, "Wait for DUT to fail commissioning TH_SERVER, 30 seconds"),
                 TestStep(20, "Get number of fabrics from TH_SERVER"),
                 TestStep(21, "Send RevokeCommissioning command on Administrator Commissioning Cluster sent to TH_SERVER"),
                 TestStep(22, "Send RequestCommissioningApproval command to DUT with CASE session with correct vendorID"),
                 TestStep(23, "(Manual Step) Approve Commissioning Approval Request on DUT using method indicated by the manufacturer"),
                 TestStep(24, "Reading Event CommissioningRequestResult from DUT, confirm one new event"),
                 TestStep(25, "Send CommissionNode command to DUT with CASE session, with valid parameters"),
                 TestStep(26, "Send OpenCommissioningWindow command on Administrator Commissioning Cluster sent to TH_SERVER"),
                 TestStep(27, "Wait for DUT to successfully commission TH_SERVER, 30 seconds"),
                 TestStep(28, "Get number of fabrics from TH_SERVER, verify DUT successfully commissioned TH_SERVER")]

        return steps

    # This test has some manual steps and also multiple sleeps for up to 30 seconds. Test typically runs
    # under 2 mins, so 4 minutes is more than enough.
    @property
    def default_timeout(self) -> int:
        return 4*60

    @run_if_endpoint_matches(has_cluster(Clusters.CommissionerControl))
    async def test_TC_CCTRL_2_2(self):
        self.step(1)
        th_server_fabrics = await self.read_single_attribute_check_success(cluster=Clusters.OperationalCredentials, attribute=Clusters.OperationalCredentials.Attributes.Fabrics, dev_ctrl=self.TH_server_controller, node_id=self.server_nodeid, endpoint=0, fabric_filtered=False)
        self.step(2)
        th_server_vid = await self.read_single_attribute_check_success(cluster=Clusters.BasicInformation, attribute=Clusters.BasicInformation.Attributes.VendorID, dev_ctrl=self.TH_server_controller, node_id=self.server_nodeid, endpoint=0)
        self.step(3)
        th_server_pid = await self.read_single_attribute_check_success(cluster=Clusters.BasicInformation, attribute=Clusters.BasicInformation.Attributes.ProductID, dev_ctrl=self.TH_server_controller, node_id=self.server_nodeid, endpoint=0)

        self.step(4)
        event_path = [(self.matter_test_config.endpoint, Clusters.CommissionerControl.Events.CommissioningRequestResult, 1)]
        events = await self.default_controller.ReadEvent(nodeid=self.dut_node_id, events=event_path)

        self.step(5)
        cmd = Clusters.CommissionerControl.Commands.CommissionNode(requestID=1, responseTimeoutSeconds=30)
        try:
            await self.send_single_cmd(cmd)
            asserts.fail("Unexpected success on CommissionNode")
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.Failure, "Incorrect error returned")

        self.step(6)
        params = await self.openCommissioningWindow(dev_ctrl=self.default_controller, node_id=self.dut_node_id)
        self.step(7)
        pase_nodeid = self.dut_node_id + 1
        await self.default_controller.FindOrEstablishPASESession(setupCode=params.commissioningParameters.setupQRCode, nodeid=pase_nodeid)
        try:
            await self.send_single_cmd(cmd=cmd, node_id=pase_nodeid)
            asserts.fail("Unexpected success on CommissionNode")
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.UnsupportedAccess, "Incorrect error returned")

        self.step(8)
        good_request_id = 0x1234567887654321
        cmd = Clusters.CommissionerControl.Commands.RequestCommissioningApproval(
            requestID=good_request_id, vendorID=th_server_vid, productID=th_server_pid)
        try:
            await self.send_single_cmd(cmd=cmd, node_id=pase_nodeid)
            asserts.fail("Unexpected success on RequestCommissioningApproval over PASE")
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.UnsupportedAccess, "Incorrect error returned")

        self.step(9)
        cmd = Clusters.AdministratorCommissioning.Commands.RevokeCommissioning()
        # If no exception is raised, this is success
        await self.send_single_cmd(cmd, timedRequestTimeoutMs=5000)

        self.step(10)
        if not events:
            new_event = await self.default_controller.ReadEvent(nodeid=self.dut_node_id, events=event_path)
        else:
            event_nums = [e.Header.EventNumber for e in events]
            new_event = await self.default_controller.ReadEvent(nodeid=self.dut_node_id, events=event_path, eventNumberFilter=max(event_nums)+1)
        asserts.assert_equal(new_event, [], "Unexpected event")

        self.step(11)
        # There should be nothing on the TH that uses VID 0x6006 as this is a real vendor ID.
        not_th_server_vid = 0x6006
        asserts.assert_not_equal(not_th_server_vid, th_server_vid, "Test implementation assumption incorrect")
        cmd = Clusters.CommissionerControl.Commands.RequestCommissioningApproval(
            requestID=good_request_id, vendorID=not_th_server_vid, productID=th_server_pid)
        # If no exception is raised, this is success
        await self.send_single_cmd(cmd)

        self.step(12)
        if not self.is_pics_sdk_ci_only:
            self.wait_for_user_input("Approve Commissioning approval request using manufacturer specified mechanism")

        self.step(13)
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

        self.step(14)
        bad_request_id = 0x1234567887654322
        cmd = Clusters.CommissionerControl.Commands.CommissionNode(requestID=bad_request_id, responseTimeoutSeconds=30)
        try:
            await self.send_single_cmd(cmd=cmd)
            asserts.fail("Unexpected success on CommissionNode")
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.Failure, "Incorrect error returned")

        self.step(15)
        cmd = Clusters.CommissionerControl.Commands.CommissionNode(requestID=good_request_id, responseTimeoutSeconds=29)
        try:
            await self.send_single_cmd(cmd=cmd)
            asserts.fail("Unexpected success on CommissionNode")
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.ConstraintError, "Incorrect error returned")

        self.step(16)
        cmd = Clusters.CommissionerControl.Commands.CommissionNode(requestID=good_request_id, responseTimeoutSeconds=121)
        try:
            await self.send_single_cmd(cmd=cmd)
            asserts.fail("Unexpected success on CommissionNode")
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.ConstraintError, "Incorrect error returned")

        self.step(17)
        cmd = Clusters.CommissionerControl.Commands.CommissionNode(requestID=good_request_id, responseTimeoutSeconds=30)
        resp: Clusters.CommissionerControl.Commands.ReverseOpenCommissioningWindow = await self.send_single_cmd(cmd)
        asserts.assert_equal(type(resp), Clusters.CommissionerControl.Commands.ReverseOpenCommissioningWindow,
                             "Incorrect response type")

        self.step(18)
        # min commissioning timeout is 3*60 seconds, so use that even though the command said 30.
        cmd = Clusters.AdministratorCommissioning.Commands.OpenCommissioningWindow(commissioningTimeout=3*60,
                                                                                   PAKEPasscodeVerifier=resp.PAKEPasscodeVerifier,
                                                                                   discriminator=resp.discriminator,
                                                                                   iterations=resp.iterations, salt=resp.salt)
        await self.send_single_cmd(cmd, dev_ctrl=self.TH_server_controller, node_id=self.server_nodeid, endpoint=0, timedRequestTimeoutMs=5000)

        self.step(19)
        logging.info("Test now waits for 30 seconds")
        if not self.is_pics_sdk_ci_only:
            time.sleep(30)

        self.step(20)
        print(f'server node id {self.server_nodeid}')
        th_server_fabrics_new = await self.read_single_attribute_check_success(cluster=Clusters.OperationalCredentials, attribute=Clusters.OperationalCredentials.Attributes.Fabrics, dev_ctrl=self.TH_server_controller, node_id=self.server_nodeid, endpoint=0, fabric_filtered=False)
        asserts.assert_equal(len(th_server_fabrics), len(th_server_fabrics_new), "Unexpected number of fabrics on TH_SERVER")

        self.step(21)
        cmd = Clusters.AdministratorCommissioning.Commands.RevokeCommissioning()
        await self.send_single_cmd(cmd, dev_ctrl=self.TH_server_controller, node_id=self.server_nodeid, timedRequestTimeoutMs=5000, endpoint=0)

        self.step(22)
        good_request_id = 0x1234567812345678
        cmd = Clusters.CommissionerControl.Commands.RequestCommissioningApproval(
            requestID=good_request_id, vendorID=th_server_vid, productID=th_server_pid, label="Test Ecosystem")
        await self.send_single_cmd(cmd)

        self.step(23)
        if not self.is_pics_sdk_ci_only:
            self.wait_for_user_input("Approve Commissioning approval request using manufacturer specified mechanism")

        self.step(24)
        events = new_event
        event_nums = [e.Header.EventNumber for e in events]
        new_event = await self.default_controller.ReadEvent(nodeid=self.dut_node_id, events=event_path, eventNumberFilter=max(event_nums)+1)
        asserts.assert_equal(len(new_event), 1, "Unexpected event list len")
        asserts.assert_equal(new_event[0].Data.statusCode, 0, "Unexpected status code")
        asserts.assert_equal(new_event[0].Data.clientNodeID,
                             self.matter_test_config.controller_node_id, "Unexpected client node id")
        asserts.assert_equal(new_event[0].Data.requestID, good_request_id, "Unexpected request ID")

        self.step(25)
        cmd = Clusters.CommissionerControl.Commands.CommissionNode(requestID=good_request_id, responseTimeoutSeconds=30)
        resp = await self.send_single_cmd(cmd)
        asserts.assert_equal(type(resp), Clusters.CommissionerControl.Commands.ReverseOpenCommissioningWindow,
                             "Incorrect response type")

        self.step(26)
        # min commissioning timeout is 3*60 seconds, so use that even though the command said 30.
        cmd = Clusters.AdministratorCommissioning.Commands.OpenCommissioningWindow(commissioningTimeout=3*60,
                                                                                   PAKEPasscodeVerifier=resp.PAKEPasscodeVerifier,
                                                                                   discriminator=resp.discriminator,
                                                                                   iterations=resp.iterations, salt=resp.salt)
        await self.send_single_cmd(cmd, dev_ctrl=self.TH_server_controller, node_id=self.server_nodeid, endpoint=0, timedRequestTimeoutMs=5000)

        self.step(27)
        if not self.is_pics_sdk_ci_only:
            time.sleep(30)

        self.step(28)
        th_server_fabrics_new = await self.read_single_attribute_check_success(cluster=Clusters.OperationalCredentials, attribute=Clusters.OperationalCredentials.Attributes.Fabrics, dev_ctrl=self.TH_server_controller, node_id=self.server_nodeid, endpoint=0, fabric_filtered=False)
        # TODO: this should be mocked too.
        if not self.is_pics_sdk_ci_only:
            asserts.assert_equal(len(th_server_fabrics) + 1, len(th_server_fabrics_new),
                                 "Unexpected number of fabrics on TH_SERVER")


if __name__ == "__main__":
    default_matter_test_main()
