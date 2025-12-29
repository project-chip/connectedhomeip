#
#    Copyright (c) 2025 Project CHIP Authors
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
# test-runner-runs:
#   run1:
#     app: ${ALL_CLUSTERS_APP}
#     factory-reset: true
#     quiet: true
#     app-args: --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json
#     script-args: >
#       --storage-path admin_storage.json
#       --commissioning-method on-network
#       --discriminator 1234
#       --passcode 20202021
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
# === END CI TEST ARGUMENTS ===

from mobly import asserts
import random
import logging

from copy import deepcopy
from matter import ChipDeviceCtrl
import matter.clusters as Clusters
from matter.clusters import ClusterObjects as ClusterObjects
from matter.exceptions import ChipStackError
from matter.interaction_model import Status
from matter.testing.event_attribute_reporting import AttributeSubscriptionHandler, EventSubscriptionHandler
from matter.testing.matter_testing import MatterBaseTest, async_test_body, default_matter_test_main

log = logging.getLogger(__name__)


class TC_IDM_8_1_step_5_repro(MatterBaseTest):

    @async_test_body
    async def teardown_test(self):
        self.th1_events.cancel()
        self.th2_events.cancel()
        th2_fabric_idx = await self.read_single_attribute_check_success(cluster=Clusters.OperationalCredentials, attribute=Clusters.OperationalCredentials.Attributes.CurrentFabricIndex, dev_ctrl=self.th2, node_id=self.th2_dut_node_id)
        cmd = Clusters.OperationalCredentials.Commands.RemoveFabric(fabricIndex=th2_fabric_idx)
        await self.send_single_cmd(cmd=cmd, dev_ctrl=self.default_controller)
        super().teardown_test()

    @async_test_body
    async def test_TC_IDM_8_1_step_5_repro(self):
        self.original_acl: list = await self.read_single_attribute_check_success(cluster=Clusters.AccessControl, attribute=Clusters.AccessControl.Attributes.Acl)
        new = deepcopy(self.original_acl)
        # append some dummy value for a non-existent node
        new.append(Clusters.AccessControl.Structs.AccessControlEntryStruct(privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kAdminister,
                                                                           authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kCase,
                                                                           subjects=[3],
                                                                           targets=[]))

        await self.default_controller.WriteAttribute(nodeId=self.dut_node_id, attributes=[(0, Clusters.AccessControl.Attributes.Acl(value=new))])

        log.info('')
        log.info('--------------------------------------------------------------')
        log.info('About to wait for a subscription report on fabric 1')
        log.info(f'If you want to double check this in the logs, check for a non-empty report from node ID {self.dut_node_id}')
        self.th1_events.wait_for_event_report(Clusters.AccessControl.Events.AccessControlEntryChanged)
        log.info('Event was received on matching fabric (fabric 1)')
        log.info('--------------------------------------------------------------')
        log.info('')
        log.info('--------------------------------------------------------------')
        log.info('About to wait for a subscription report on fabric 2. No report should be generated, this will wait for 5 seconds')
        log.info(
            f'If you want to double check this in the logs, no data reports from node ID {self.th2_dut_node_id} should appear, empty reports are acceptable')
        self.th2_events.wait_for_event_expect_no_report(timeout_sec=5)
        log.info('Done waiting, no reports received')
        log.info('--------------------------------------------------------------')
        log.info('')

    @async_test_body
    async def setup_test(self):
        super().setup_test()
        # Create a new certificate authority and fabric admin for TH1
        th2_certificate_authority = self.certificate_authority_manager.NewCertificateAuthority()
        th2_fabric_admin = th2_certificate_authority.NewFabricAdmin(vendorId=0xFFF1, fabricId=random.randint(1, 100000))
        self.th2 = th2_fabric_admin.NewController(nodeId=1, useTestCommissioner=True)

        self.th2_dut_node_id = 1
        params = await self.open_commissioning_window()
        await self.th2.CommissionOnNetwork(
            nodeId=self.th2_dut_node_id, setupPinCode=params.commissioningParameters.setupPinCode,
            filterType=ChipDeviceCtrl.DiscoveryFilterType.LONG_DISCRIMINATOR, filter=params.randomDiscriminator)

        self.th1_events = EventSubscriptionHandler(expected_cluster=Clusters.AccessControl,
                                                   expected_event_id=Clusters.AccessControl.Events.AccessControlEntryChanged.event_id)
        await self.th1_events.start(self.default_controller, self.dut_node_id, 0)

        self.th2_events = EventSubscriptionHandler(expected_cluster=Clusters.AccessControl,
                                                   expected_event_id=Clusters.AccessControl.Events.AccessControlEntryChanged.event_id)
        await self.th2_events.start(self.th2, self.th2_dut_node_id, 0)


if __name__ == "__main__":
    default_matter_test_main()
