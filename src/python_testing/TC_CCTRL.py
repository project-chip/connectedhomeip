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

import ipaddress
import logging
import os
import signal
import subprocess
import uuid
import random
import pathlib
import time

import chip.clusters as Clusters
import chip.exceptions
from matter_testing_support import MatterBaseTest, default_matter_test_main, per_endpoint_test, has_cluster, async_test_body
from mobly import asserts
from chip.interaction_model import InteractionModelError, Status

# isort: off

from chip import ChipDeviceCtrl  # Needed before chip.FabricAdmin
import chip.CertificateAuthority
from chip.ChipDeviceCtrl import CommissioningParameters

# isort: on


class TC_CCTRL(MatterBaseTest):

    @async_test_body
    async def setup_class(self):
        super().setup_class()
        # TODO: This needs to come from an arg and needs to be something available on the TH
        # TODO: confirm whether we can open processes like this on the TH
        app = os.path.join(pathlib.Path(__file__).resolve().parent, '..','..','out', 'linux-x64-all-clusters-no-ble', 'chip-all-clusters-app')

        self.kvs = f'kvs_{str(uuid.uuid4())}'
        self.port = 5543
        discriminator = random.randint(0, 4095)
        discriminator = 3840
        passcode = 20202021
        app_args = f'--secured-device-port {self.port} --discriminator {discriminator} --passcode {passcode} --KVS {self.kvs}'
        cmd = f'{app} {app_args}'
        # TODO: Determine if we want these logs cooked or pushed to somewhere else
        logging.info("Starting TH_SERVER")
        self.app_process = subprocess.Popen(cmd, bufsize=0, shell=True)
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
        logging.warning("Stopping app with SIGTERM")
        self.app_process.send_signal(signal.SIGTERM.value)
        test_app_exit_code = self.app_process.wait()
        # TODO: Use timeout, if term doesn't work, try SIGINT

        os.remove(self.kvs)
        super().teardown_class()

    @per_endpoint_test(has_cluster(Clusters.CommissionerControl))
    async def test_TC_CCTRL_3_1(self):
        self.is_ci = self.check_pics('PICS_SDK_CI_ONLY')

        #self.step(1)
        th_server_fabrics = await self.read_single_attribute_check_success(cluster=Clusters.OperationalCredentials, attribute=Clusters.OperationalCredentials.Attributes.Fabrics, dev_ctrl=self.TH_server_controller, node_id=self.server_nodeid, endpoint=0)
        #self.step(2)
        th_server_vid = await self.read_single_attribute_check_success(cluster=Clusters.BasicInformation, attribute=Clusters.BasicInformation.Attributes.VendorID, dev_ctrl=self.TH_server_controller, node_id=self.server_nodeid, endpoint=0)
        #self.step(3)
        th_server_pid = await self.read_single_attribute_check_success(cluster=Clusters.BasicInformation, attribute=Clusters.BasicInformation.Attributes.ProductID, dev_ctrl=self.TH_server_controller, node_id=self.server_nodeid, endpoint=0)

        #self.step(4)
        event_path = [(self.matter_test_config.endpoint, Clusters.CommissionerControl.Events.CommissioningRequestResult, 1)]
        events = await self.default_controller.ReadEvent(nodeid=self.dut_node_id, events=event_path)

        #self.step(5)
        ipaddr = ipaddress.IPv6Address('::1')
        cmd = Clusters.CommissionerControl.Commands.CommissionNode(requestId=1, responseTimeoutSeconds=30, ipAddress=ipaddr.packed, port=self.port)
        try:
            await self.send_single_cmd(cmd)
            asserts.fail("Unexpected success on CommissionNode")
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.Failure, "Incorrect error returned")

        #self.step(6)
        params = await self.openCommissioningWindow(dev_ctrl=self.default_controller, node_id=self.dut_node_id)
        pase_nodeid = self.dut_node_id + 1
        await self.default_controller.FindOrEstablishPASESession(setupCode=params.commissioningParameters.setupQRCode, nodeid=pase_nodeid)
        try:
            await self.send_single_cmd(cmd=cmd, node_id=pase_nodeid)
            asserts.fail("Unexpected success on CommissionNode")
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.UnsupportedAccess, "Incorrect error returned")

        # self.step(7)
        good_request_id = 0x1234567887654321
        cmd = Clusters.CommissionerControl.Commands.RequestCommissioningApproval(requestId=good_request_id, vendorId=th_server_vid, productId=th_server_pid)
        try:
            await self.send_single_cmd(cmd=cmd, node_id=pase_nodeid)
            asserts.fail("Unexpected success on RequestCommissioningApproval over PASE")
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.UnsupportedAccess, "Incorrect error returned")

        #self.step(8)
        if not events:
            new_event = await self.default_controller.ReadEvent(nodeid=self.dut_node_id, events=event_path)
        else:
            event_nums = [e.Header.EventNumber for e in events]
            new_event = await self.default_controller.ReadEvent(nodeid=self.dut_node_id, events=event_path, eventNumberFilter=max(event_nums)+1)
        asserts.assert_equal(new_event, [], "Unexpected event")


        #self.step(9)
        bad_vid = 0x6006
        cmd = Clusters.CommissionerControl.Commands.RequestCommissioningApproval(requestId=good_request_id, vendorId=bad_vid, productId=th_server_pid)
        # If no exception is raised, this is success
        await self.send_single_cmd(cmd)

        #self.step(10)
        if not self.is_ci:
            self.wait_for_use_input("Approve Commissioning approval request using manufacturer specified mechanism")

        #self.step(11)
        if not events:
            new_event = await self.default_controller.ReadEvent(nodeid=self.dut_node_id, events=event_path)
        else:
            event_nums = [e.Header.EventNumber for e in events]
            new_event = await self.default_controller.ReadEvent(nodeid=self.dut_node_id, events=event_path, eventNumberFilter=max(event_nums)+1)
        asserts.assert_equal(len(new_event), 1, "Unexpected event list len")
        asserts.assert_equal(new_event[0].Data.statusCode, 0, "Unexpected status code")
        asserts.assert_equal(new_event[0].Data.clientNodeId, self.matter_test_config.controller_node_id, "Unexpected client node id")
        asserts.assert_equal(new_event[0].Data.requestId, good_request_id, "Unexpected request ID")

        #self.step(12)
        bad_request_id = 0x1234567887654322
        cmd = Clusters.CommissionerControl.Commands.CommissionNode(requestId=bad_request_id, responseTimeoutSeconds=30)
        try:
            await self.send_single_cmd(cmd=cmd)
            asserts.fail("Unexpected success on CommissionNode")
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.Failure, "Incorrect error returned")

        #self.step(13)
        cmd = Clusters.CommissionerControl.Commands.CommissionNode(requestId=good_request_id, responseTimeoutSeconds=29)
        try:
            await self.send_single_cmd(cmd=cmd)
            asserts.fail("Unexpected success on CommissionNode")
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.Failure, "Incorrect error returned")

        #self.step(14)
        cmd = Clusters.CommissionerControl.Commands.CommissionNode(requestId=good_request_id, responseTimeoutSeconds=121)
        try:
            await self.send_single_cmd(cmd=cmd)
            asserts.fail("Unexpected success on CommissionNode")
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.Failure, "Incorrect error returned")

        #self.step(15)
        cmd = Clusters.CommissionerControl.Commands.CommissionNode(requestId=good_request_id, responseTimeoutSeconds=30)
        resp = await self.send_single_cmd(cmd)
        asserts.assert_equal(type(resp), Clusters.CommissionerControl.Commands.ReverseOpenCommissioningWindow, "Incorrect response type")

        #self.step(16)
        #TODO: we do not have a direct line to OpenEnhancedCommissioningWindow. Need to add plumbing
        # For now, open with anything so the below command is ok
        await self.openCommissioningWindow(dev_ctrl=self.TH_server_controller, node_id=self.server_nodeid)

        #self.step(17)
        logging.info("Test now waits for 30 seconds")
        #time.sleep(30)

        #self.step(18)
        print(f'server node id {self.server_nodeid}')
        th_server_fabrics_new = await self.read_single_attribute_check_success(cluster=Clusters.OperationalCredentials, attribute=Clusters.OperationalCredentials.Attributes.Fabrics, dev_ctrl=self.TH_server_controller, node_id=self.server_nodeid, endpoint=0)
        asserts.assert_equal(len(th_server_fabrics), len(th_server_fabrics_new), "Unexpected number of fabrics on TH_SERVER")

        #self.step(19)
        cmd = Clusters.AdministratorCommissioning.Commands.RevokeCommissioning()
        await self.send_single_cmd(cmd, dev_ctrl=self.TH_server_controller, node_id=self.server_nodeid, timedRequestTimeoutMs=5000, endpoint=0)

        #self.step(20)
        good_request_id = 0x1234567812345678
        cmd = Clusters.CommissionerControl.Commands.RequestCommissioningApproval(requestId=good_request_id, vendorId=th_server_vid, productId=th_server_pid, label="Test Ecosystem")
        self.send_single_cmd(cmd)

        #self.step(21)
        if not self.is_ci:
            self.wait_for_use_input("Approve Commissioning approval request using manufacturer specified mechanism")

        #self.step(22)
        events = new_event
        event_nums = [e.Header.EventNumber for e in events]
        new_event = await self.default_controller.ReadEvent(nodeid=self.dut_node_id, events=event_path, eventNumberFilter=max(event_nums)+1)
        asserts.assert_equal(len(new_event), 1, "Unexpected event list len")
        asserts.assert_equal(new_event[0].Data.statusCode, 0, "Unexpected status code")
        asserts.assert_equal(new_event[0].Data.clientNodeId, self.matter_test_config.controller_node_id, "Unexpected client node id")
        asserts.assert_equal(new_event[0].Data.requestId, good_request_id, "Unexpected request ID")



if __name__ == "__main__":
    default_matter_test_main()