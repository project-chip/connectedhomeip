#
#    Copyright (c) 2023 Project CHIP Authors
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

import queue
import time

import chip.clusters as Clusters
from chip import ChipDeviceCtrl
from chip.clusters.Types import NullValue
from matter_testing_support import MatterBaseTest, SimpleEventCallback, async_test_body, default_matter_test_main, type_matches
from mobly import asserts


class TC_TIMESYNC_2_13(MatterBaseTest):
    def wait_for_trusted_time_souce_event(self, timeout):
        try:
            ret = self.q.get(block=True, timeout=timeout)
            asserts.assert_true(type_matches(received_value=ret.Data,
                                desired_type=Clusters.TimeSynchronization.Events.MissingTrustedTimeSource), "Incorrect type received for event")
        except queue.Empty:
            asserts.fail("Did not receive MissingTrustedTimeSouce event")

    def pics_TC_TIMESYNC_2_13(self) -> list[str]:
        return ["TIMESYNC.S.F01"]

    @async_test_body
    async def test_TC_TIMESYNC_2_13(self):

        self.endpoint = 0

        self.print_step(0, "Commissioning, already done")

        self.print_step(1, "TH1 opens a commissioning window")
        params = await self.default_controller.OpenCommissioningWindow(
            nodeid=self.dut_node_id, timeout=600, iteration=10000, discriminator=1234, option=1)

        self.print_step(2, "Commission to TH2")
        new_certificate_authority = self.certificate_authority_manager.NewCertificateAuthority()
        new_fabric_admin = new_certificate_authority.NewFabricAdmin(vendorId=0xFFF1, fabricId=2)
        TH2 = new_fabric_admin.NewController(nodeId=112233)

        await TH2.CommissionOnNetwork(
            nodeId=self.dut_node_id, setupPinCode=params.setupPinCode,
            filterType=ChipDeviceCtrl.DiscoveryFilterType.LONG_DISCRIMINATOR, filter=1234)

        self.print_step(3, "TH2 reads the current fabric")
        th2_fabric_idx = await self.read_single_attribute_check_success(
            dev_ctrl=TH2, cluster=Clusters.OperationalCredentials, attribute=Clusters.OperationalCredentials.Attributes.CurrentFabricIndex)

        self.print_step(4, "TH2 sends the SetTrustedTimeSource command to the DUT with its nodeID")
        tts = Clusters.TimeSynchronization.Structs.FabricScopedTrustedTimeSourceStruct(nodeID=TH2.nodeId, endpoint=0)
        await self.send_single_cmd(dev_ctrl=TH2, cmd=Clusters.TimeSynchronization.Commands.SetTrustedTimeSource(trustedTimeSource=tts))

        self.print_step(5, "TH1 subscribeds to the MissingTrustedTimeSource event")
        event = Clusters.TimeSynchronization.Events.MissingTrustedTimeSource
        self.q = queue.Queue()
        cb = SimpleEventCallback("MissingTrustedTimeSource", event.cluster_id, event.event_id, self.q)
        urgent = 1
        subscription = await self.default_controller.ReadEvent(nodeid=self.dut_node_id, events=[(self.endpoint, event, urgent)], reportInterval=[1, 3])
        subscription.SetEventUpdateCallback(callback=cb)

        self.print_step(6, "TH1 removes the TH2 fabric")
        await self.send_single_cmd(cmd=Clusters.OperationalCredentials.Commands.RemoveFabric(fabricIndex=th2_fabric_idx))

        self.print_step(7, "TH1 waits for the MissingTrustedTimeSource event with a timeout of 5 seconds")
        self.wait_for_trusted_time_souce_event(5)

        self.print_step(8, "TH1 sends a SetTrusteTimeSource command")
        tts = Clusters.TimeSynchronization.Structs.FabricScopedTrustedTimeSourceStruct(
            nodeID=self.default_controller.nodeId, endpoint=0)
        await self.send_single_cmd(cmd=Clusters.TimeSynchronization.Commands.SetTrustedTimeSource(trustedTimeSource=tts))

        self.print_step(9, "TH1 waits 5 seconds")
        time.sleep(5)

        self.print_step(10, "TH1 sends the SetTrustedTimeSource command with TrustedTimeSource set to NULL")
        await self.send_single_cmd(cmd=Clusters.TimeSynchronization.Commands.SetTrustedTimeSource(NullValue))

        self.print_step(11, "TH1 waits for the MissingTrustedTimeSource event with a timeout of 5 seconds")
        self.wait_for_trusted_time_souce_event(5)


if __name__ == "__main__":
    default_matter_test_main()
