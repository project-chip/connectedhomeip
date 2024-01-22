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
import logging

import chip.clusters as Clusters
from matter_testing_support import MatterBaseTest, async_test_body, default_matter_test_main
from mobly import asserts

logger = logging.getLogger('PythonMatterControllerTEST')
logger.setLevel(logging.INFO)


class TC_ICDM_2_1(MatterBaseTest):
    async def read_icdm_attribute_expect_success(self, endpoint, attribute):
        cluster = Clusters.Objects.IcdManagement
        return await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attribute)

    def pics_TC_ICDM_2_1(self) -> list[str]:
        return ["ICDM.S"]

    @async_test_body
    async def test_TC_ICDM_2_1(self):

        if not self.check_pics("ICDM.S"):
            logger.info("Test skipped because PICS ICDM.S is not set")
            return

        endpoint = self.user_params.get("endpoint", 0)

        self.print_step(1, "Commissioning, already done")
        attributes = Clusters.IcdManagement.Attributes
        idleModeDuration = 0

        # Idle Mode Duration attribute test
        if (self.check_pics("ICDM.S.A0000")):
            self.print_step(2, "Read IdleModeDuration Attribute")

            idleModeDuration = await self.read_icdm_attribute_expect_success(endpoint=endpoint,
                                                                             attribute=attributes.IdleModeDuration)
            asserts.assert_greater_equal(idleModeDuration, 1, "IdleModeDuration attribute is smaller than minimum value (1).")
            asserts.assert_less_equal(idleModeDuration, 64800, "IdleModeDuration attribute is greater than maximum value (64800).")
        else:
            asserts.assert_true(False, "IdleModeDuration is a mandatory attribute and must be present in the PICS file")

        # Active Mode Duration attribute test
        if (self.check_pics("ICDM.S.A0001")):
            self.print_step(2, "Read ActiveModeDuration Attribute")

            idleModeDuration *= 1000  # Convert seconds to milliseconds
            activeModeDuration = await self.read_icdm_attribute_expect_success(endpoint=endpoint,
                                                                               attribute=attributes.ActiveModeDuration)
            asserts.assert_true(0 <= activeModeDuration <= 65535,
                                "ActiveModeDuration attribute does not fit in a uint16.")
            asserts.assert_less_equal(activeModeDuration, idleModeDuration,
                                      "ActiveModeDuration attribute is greater than the IdleModeDuration attrbiute.")
        else:
            asserts.assert_true(False, "ActiveModeDuration is a mandatory attribute and must be present in the PICS file")

        # Active Mode Threshold attribute test
        if (self.check_pics("ICDM.S.A0002")):
            self.print_step(2, "Read ActiveModeThreshold Attribute")

            activeModeThreshold = await self.read_icdm_attribute_expect_success(endpoint=endpoint,
                                                                                attribute=attributes.ActiveModeThreshold)
            asserts.assert_true(0 <= activeModeThreshold <= 65535,
                                "ActiveModeThreshold attribute does not fit in a uint16.")
        else:
            asserts.assert_true(False, "ActiveModeThreshold is a mandatory attribute and must be present in the PICS file")

        # RegisteredClients attribute test
        if (self.check_pics("ICDM.S.A0003")):
            self.print_step(2, "Read RegisteredClients Attribute")

            await self.read_icdm_attribute_expect_success(endpoint=endpoint,
                                                          attribute=attributes.RegisteredClients)

        # ICDCounter attribute test
        if (self.check_pics("ICDM.S.A0003")):
            self.print_step(2, "Read ICDCounter Attribute")

            ICDCounter = await self.read_icdm_attribute_expect_success(endpoint=endpoint,
                                                                       attribute=attributes.ICDCounter)
            asserts.assert_true(0 <= ICDCounter <= 4294967295,
                                "ICDCounter attribute does not fit in a uint32.")

        # ClientsSupportedPerFabric attribute test
        if (self.check_pics("ICDM.S.A0003")):
            self.print_step(2, "Read ClientsSupportedPerFabric Attribute")

            clientsSupportedPerFabric = await self.read_icdm_attribute_expect_success(endpoint=endpoint,
                                                                                      attribute=attributes.ClientsSupportedPerFabric)
            asserts.assert_true(0 <= clientsSupportedPerFabric <= 65535,
                                "ActiveModeThreshold ClientsSupportedPerFabric does not fit in a uint16.")
            asserts.assert_greater_equal(clientsSupportedPerFabric, 1,
                                         "ClientsSupportedPerFabric attribute is smaller than minimum value (1).")


if __name__ == "__main__":
    default_matter_test_main()
