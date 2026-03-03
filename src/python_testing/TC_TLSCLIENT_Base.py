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

from datetime import timedelta

import test_plan_support
from TC_TLS_Utils import TLSUtils

from matter.testing.matter_testing import MatterBaseTest
from matter.testing.runner import TestStep
from matter.utils import CommissioningBuildingBlocks


class TC_TLSCLIENT_Base(MatterBaseTest):
    """Base class for TC_TLSCLIENT test cases with common setup methods."""

    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)

    class TwoFabricData:
        def __init__(self, cr1_cmd: TLSUtils, cr2_cmd: TLSUtils):
            self.cr1_cmd = cr1_cmd
            self.cr2_cmd = cr2_cmd

    # This test creates the maximum number of endpoints, which can take some time
    @property
    def default_timeout(self) -> int:
        return timedelta(minutes=5).total_seconds()

    async def common_setup(self):
        self.step(1)
        endpoint = self.get_endpoint()
        cr1_cmd = TLSUtils(self, endpoint=endpoint)
        cr1 = self.default_controller

        self.step(2)
        # Establishing CR2 controller
        cr2_certificate_authority = self.certificate_authority_manager.NewCertificateAuthority()
        cr2_fabric_admin = cr2_certificate_authority.NewFabricAdmin(vendorId=0xFFF1, fabricId=cr1.fabricId + 1)
        cr2 = cr2_fabric_admin.NewController(nodeId=cr1.nodeId + 1)
        cr2_dut_node_id = self.dut_node_id+1

        self.step(3)
        _, noc_resp, _ = await CommissioningBuildingBlocks.AddNOCForNewFabricFromExisting(
            commissionerDevCtrl=cr1, newFabricDevCtrl=cr2,
            existingNodeId=self.dut_node_id, newNodeId=cr2_dut_node_id
        )
        fabric_index_cr2 = noc_resp.fabricIndex

        cr2_cmd = TLSUtils(self, endpoint=endpoint, dev_ctrl=cr2, node_id=cr2_dut_node_id, fabric_index=fabric_index_cr2)
        return self.TwoFabricData(cr1_cmd=cr1_cmd, cr2_cmd=cr2_cmd)

    def get_common_steps(self) -> list[TestStep]:
        return [
            TestStep(1, test_plan_support.commission_if_required('CR1'), is_commissioning=True),
            TestStep(2, test_plan_support.open_commissioning_window()),
            TestStep(3, test_plan_support.commission_from_existing('CR1', 'CR2')),
        ]
