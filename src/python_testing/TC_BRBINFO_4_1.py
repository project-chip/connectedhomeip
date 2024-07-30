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
# === BEGIN CI TEST ARGUMENTS ===
# test-runner-runs: run1
# test-runner-run/run1/app: ${ALL_CLUSTERS_APP}
# test-runner-run/run1/factoryreset: True
# test-runner-run/run1/quiet: True
# test-runner-run/run1/app-args: --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json
# test-runner-run/run1/script-args: --storage-path admin_storage.json --commissioning-method on-network --discriminator 1234 --passcode 20202021 --PICS src/app/tests/suites/certification/ci-pics-values --trace-to json:${TRACE_TEST_JSON}.json --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
# === END CI TEST ARGUMENTS ===

import logging
import os
import random
import re
import queue
import pathlib
import uuid
import subprocess
import time
import signal

import chip.clusters as Clusters
from matter_testing_support import MatterBaseTest, TestStep, async_test_body, default_matter_test_main, SimpleEventCallback
from mobly import asserts

from chip import ChipDeviceCtrl  # Needed before chip.FabricAdmin
import chip.CertificateAuthority
from chip.ChipDeviceCtrl import CommissioningParameters

logger = logging.getLogger(__name__)
kRootEndpointId = 0
kICDBridgedEndpointId = 2
kMaxUserActiveModeBitmap = 0x1FFFF
kMaxUserActiveModeTriggerInstructionByteLength = 128

#uat = cluster.Bitmaps.UserActiveModeTriggerBitmap
#modes = cluster.Enums.OperatingModeEnum
#features = cluster.Bitmaps.Feature

class TC_BRBINFO_4_1(MatterBaseTest):

    #
    # Class Helper functions
    #

    async def _read_icdm_attribute_expect_success(self, attribute):
        cluster = Clusters.Objects.IcdManagement
        return await self.read_single_attribute_check_success(endpoint=kRootEndpointId, cluster=cluster, attribute=attribute)

    async def _read_brbinfo_attribute_expect_success(self, attribute):
        cluster = Clusters.Objects.BridgedDeviceBasicInformation
        return await self.read_single_attribute_check_success(endpoint=kRootEndpointId, cluster=cluster, attribute=attribute)

    async def _read_basicinfo_attribute_expect_success(self, attribute):
        cluster = Clusters.Objects.BasicInformation
        return await self.read_single_attribute_check_success(endpoint=kRootEndpointId, cluster=cluster, attribute=attribute)

    #
    # Test Harness Helpers
    #

    def desc_TC_BRBINFO_4_1(self) -> str:
        """Returns a description of this test"""
        return "[TC_BRBINFO_4_1] Verification of KeepActive Command [DUT-Server]"

    def steps_TC_BRBINFO_4_1(self) -> list[TestStep]:
        steps = [
            TestStep(1, "Commissioning of the DUT, already done", is_commissioning=True),
            TestStep(2, "Commissioning of the ICD, already done", is_commissioning=True),
            TestStep(3, "TH reads from the ICD the IDLE_MODE_DURATION, ACTIVE_MODE_DURATION, ACTIVE_MODE_THRESHOLD attributes"),
            TestStep(4, "Simple KeepActive command w/ subscription. ActiveChanged event received by TH contains PromisedActiveDuration"),
            TestStep(5, "Multiple KeepActive commands. ActiveChanged event received by TH contains a PromisedActiveDuration"),
            TestStep(6, "Simple KeepActive command w/ check-in. ActiveChanged event received by TH contains PromisedActiveDuration"),
            TestStep(7, "KeepActive not returned after 60 minutes of offline ICD"),
        ]
        return steps


    def pics_TC_BRBINFO_4_1(self) -> list[str]:
        """ This function returns a list of PICS for this test case that must be True for the test to be run"""
        pics = [
            "BRBINFO.C.80",
        ]
        return pics

    def _ask_for_vendor_commissioniong_ux_operation(self, discriminator, setupPinCode):
        self.wait_for_user_input(
            prompt_msg=f"Using the DUT vendor's provided interface, commission the ICD device using the following parameters:\n"
                        f"- discriminator: {discriminator}\n"
                        f"- setupPinCode: {setupPinCode}\n"
                        f"If using FabricSync Admin, you may type:\n"
                        f">>> pairing onnetwork 111 {setupPinCode}")

    @async_test_body
    async def setup_class(self):
        super().setup_class()
        # TODO: This needs to come from an arg and needs to be something available on the TH
        # TODO: confirm whether we can open processes like this on the TH
        app = os.path.join(pathlib.Path(__file__).resolve().parent, '..','..','out', 'linux-x64-lit-icd', 'lit-icd-app')

        self.kvs = f'kvs_{str(uuid.uuid4())}'
        self.port = 5543
        discriminator = random.randint(0, 4095)
        discriminator = 3850
        passcode = 20202021
        app_args = f'--secured-device-port {self.port} --discriminator {discriminator} --passcode {passcode} --KVS {self.kvs} ' + \
          '--secured-device-port 5545'
        cmd = f'{app} {app_args}'

        logging.info("Starting ICD Server App")
        self.app_process = subprocess.Popen(cmd, bufsize=0, shell=True)
        logging.info("ICD started")
        time.sleep(3)

        logging.info("Commissioning of ICD to fabric one (TH)")
        #new_certificate_authority = self.certificate_authority_manager.NewCertificateAuthority()
        #new_fabric_admin = new_certificate_authority.NewFabricAdmin(vendorId=0xFFF1, fabricId=2)
        #paa_path = str(self.matter_test_config.paa_trust_store_path)
        #print(f"paa_path = {paa_path}  ------------------------------------------------")
        #self.TH_server_controller = new_fabric_admin.NewController(nodeId=112233, paaTrustStorePath=paa_path)
        self.server_nodeid = 1111

        await self.default_controller.CommissionOnNetwork(nodeId=self.server_nodeid, setupPinCode=passcode, filterType=ChipDeviceCtrl.DiscoveryFilterType.LONG_DISCRIMINATOR, filter=discriminator)
        # await self.TH_server_controller.CommissionOnNetwork(nodeId=self.server_nodeid, setupPinCode=passcode, filterType=ChipDeviceCtrl.DiscoveryFilterType.LONG_DISCRIMINATOR, filter=discriminator)
        logging.info("Commissioning complete (1/2)")

        logging.info("Commissioning of ICD to fabric two (DUT)")
        params = await self.openCommissioningWindow(dev_ctrl=self.default_controller, node_id=self.server_nodeid)


        self._ask_for_vendor_commissioniong_ux_operation(params.randomDiscriminator, params.commissioningParameters.setupPinCode)

        # cmd = Clusters.CommissionerControl.Commands.CommissionNode(requestId=1, responseTimeoutSeconds=30, ipAddress=ipaddr.packed, port=self.port)


    def teardown_class(self):
        logging.warning("Stopping app with SIGTERM")
        self.app_process.send_signal(signal.SIGTERM.value)
        test_app_exit_code = self.app_process.wait()
        os.remove(self.kvs)
        super().teardown_class()

    #
    # BRBINFO 4.1 Test Body
    #

    @async_test_body
    async def test_TC_BRBINFO_4_1(self):

        brbinfoCluster = Clusters.Objects.BridgedDeviceBasicInformation
        basicinfoCluster = Clusters.Objects.BasicInformation


        brbinfoAttributes = brbinfoCluster.Attributes
        basicinfoAttributes = basicinfoCluster.Attributes

        self.endpoint = 0

        # Check commisisoned DUT
        self.step(1)
        # Confirms commissioning of DUT as it reads basic info from bridge device
        featureMap = await self._read_basicinfo_attribute_expect_success(
            basicinfoAttributes.FeatureMap)

        self.step(2)
        # Confirms commissioning of DUT as it reads basic info from bridge device
        featureMap = await self._read_basicinfo_attribute_expect_success(
            basicinfoAttributes.FeatureMap)

        self.step(3)
        ## ??

        # dummy values
        idle_mode_duration = 10000
        active_mode_duration = 200

        self.step(4)
        event = brbinfoCluster.Events.ActiveChanged
        self.q = queue.Queue()
        urgent = 1
        cb = SimpleEventCallback("KeepActive", event.cluster_id, event.event_id, self.q)
        subscription = await self.default_controller.ReadEvent(nodeid=self.dut_node_id, events=[(self.endpoint, event, urgent)], reportInterval=[1, 3])
        subscription.SetEventUpdateCallback(callback=cb)

        self.step(5)


        self.step(6)


        self.step(7)

if __name__ == "__main__":
    default_matter_test_main()
