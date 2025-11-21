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
# TODO: Modify this
# See https://github.com/project-chip/connectedhomeip/blob/master/docs/testing/python.md#defining-the-ci-test-arguments
# for details about the block below.
#
# === BEGIN CI TEST ARGUMENTS ===
# test-runner-runs:
#   run1:
#     app: ${LIGHTING_APP_NO_UNIQUE_ID}
#     app-args: --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json
#     script-args: >
#       --storage-path admin_storage.json
#       --commissioning-method on-network
#       --discriminator 1234
#       --passcode 20202021
#       --PICS src/app/tests/suites/certification/ci-pics-values
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#       --endpoint 1
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

import logging
from typing import List

from mobly import asserts

import matter.clusters as Clusters
from matter.interaction_model import Status
from matter.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main

logger = logging.getLogger(__name__)


class TC_GCAST_2_2(MatterBaseTest):
    def desc_TC_GCAST_2_2(self):
        return "[TC-GCAST-2.2] JoinGroup as Listener or Sender with DUT as Server - PROVISIONAL"

    def steps_TC_G_2_2(self):
        return [
            TestStep(1, "Commission DUT to TH (can be skipped if done in a preceding test)", is_commissioning=True),
            TestStep(2, "TH removes any existing group and KeyID on the DUT"),
            TestStep(3, "TH subscribes to Membership attribute with min interval 0s and max interval 30s"),
            TestStep(4, "If GCAST.S.F00(LN) feature is not supported on the cluster Skip to step 12"),
            TestStep(5, "Attempt to join a Group G1 with a new key using JoinGroup (GroupID=G1, Endpoints=[EP1], KeyID=K1, Key=InputKey1)"),
            TestStep(6, "TH awaits subscription report of new Membership within max interval for G1"),
            TestStep(7, "If DUT only support one non-root and non-aggregator endpoint, skip to step 8"),
            TestStep(8, "Attempt to add EP2 to Group G1 using JoinGroup (G1,[EP2],K1,Key omitted)"),
            TestStep(9, "TH awaits subscription report of updated Membership for G1 with EP2"),
            TestStep(10, "Attempt to join a Group G2 with existing K1 and UseAuxiliaryACL=True"),
            TestStep(11, "TH awaits subscription report of new Membership for G2"),
            TestStep(12, "Attempt to join Group G2 using a new Key and providing a grace period for K1"),
            TestStep(13, "TH awaits subscription report showing KeyID=K2, ExpiringKeyID=K1 for G2"),
            TestStep(14, "TH Wait GracePeriod(GP) + tolerance (+10%)"),
            TestStep(15, "TH awaits subscription report showing KeyID=K2, ExpiringKeyID absent for G2"),
            TestStep(16, "Attempt to join Group G3 using a new Key but providing an existing KeyID"),
            TestStep(17, "Attempt to join Group G3 using a new KeyId but without providing the Inputkey"),
            TestStep(18, "Attempt to join Group G3 with invalid endpoint"),
            TestStep(19, "If GCAST.S.F01(SD) feature is supported Skip this step, else attempt to join Group 3 with an empty endpoints list"),
            TestStep(20, "If DUT has more than 20 endpoints, attempt to join Group 3 with 21 endpoints"),
            TestStep(21, "If GCAST.S.F01(SD) feature is not supported Skip to step 27"),
            TestStep(22, "Attempt to join Group G4 as Sender (no endpoints) and a new key"),
            TestStep(23, "TH awaits subscription report of new Membership for G4 as Sender"),
            TestStep(24, "Attempt to join Group G5 as Sender using an existing KeyId"),
            TestStep(25, "TH awaits subscription report of new Membership for G5 as Sender"),
            TestStep(26, "Attempt to join Group G5 using a new Key but providing an existing KeyID"),
            TestStep(27, "Attempt to join Group G5 using a new Key but without a InputKey"),
            TestStep(28, "Attempt to join Group G5 using a new Key and providing a grace period for K4"),
            TestStep(29, "TH awaits subscription report showing KeyID=K5 and ExpiringKeyID=K4 for G5"),
            TestStep(30, "TH Wait GracePeriod(GP) + tolerance (+10%)"),
            TestStep(31, "TH awaits subscription report showing KeyID=K5 and ExpiringKeyID absent for G5"),
            TestStep(32, "If GCAST.S.F00(LN) feature is supported Skip this step, else attempt to add endpoints to Group 5"),
            TestStep(33, "If GCAST.S.F00(LN) feature is supported Skip this step, else attempt to add endpoints to Group 5 with UseAuxiliaryACL=True"),
            TestStep(34, "TH sends command JoinGroup with invalid GroupID=0"),
            TestStep(35, "TH sends command JoinGroup with Key length !=16 (e.g. 15 bytes)"),
            TestStep(36, "TH sends command JoinGroup with GracePeriod > 86400 (e.g. 86401)"),
        ]

    @async_test_body
    async def test_TC_GCAST_2_2(self):
        endpoint = 0
        cluster = Clusters.Objects.Groupcast
        membership_attribute = Clusters.Groupcast.Attributes.Membership
        max_membership_count_attribute = Clusters.Groupcast.Attributes.MaxMembershipCount

        # PICS: GCAST.S, GCAST.S.F00 (LN), GCAST.S.C00.Rsp(JoinGroup)

        self.step(1)
        # Read feature map
        self.step(2)
        
        self.step(3)
        
        self.step(4)
        
        self.step(5)
        
        self.step(6)
        
        self.step(7)
        
        self.step(8)
        
        self.step(9)
        
        self.step(10)
        
        self.step(11)
        
        self.step(12)
        
        self.step(13)
        
        self.step(14)
        
        self.step(15)
        
        self.step(16)
        
        self.step(17)
        
        self.step(18)
        
        self.step(19)
        
        self.step(20)
        
        self.step(21)
        
        self.step(22)
        
        self.step(23)
        
        self.step(24)
        
        self.step(25)
        
        self.step(26)
        
        self.step(27)
        
        self.step(28)
        
        self.step(29)
        
        self.step(30)
        
        self.step(31)
        
        self.step(32)
        
        self.step(33)
        
        self.step(34)
        
        self.step(35)
        
        self.step(36)

if __name__ == "__main__":
    default_matter_test_main()
