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
from matter_testing_support import MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from mobly import asserts
from enum import Flag, auto

logger = logging.getLogger(__name__)

kRootEndpointId = 0


class UserActiveModeTriggerHintBitmap(Flag):
    SettingsMenu = Auto()
    CustomInstruction = Auto()
    DeviceManual = Auto()
    ActuateSensor = Auto()
    ActuateSensorSeconds = Auto()
    ActuateSensorTimes = Auto()
    ActuateSensorLightsBlink = Auto()
    ResetButton = Auto()
    ResetButtonLightsBlink = Auto()
    ResetButtonSeconds = Auto()
    ResetButtonTimes = Auto()
    SetupButton = Auto()
    SetupButtonSeconds = Auto()
    SetupButtonLightsBlink = Auto()
    SetupButtonTimes = Auto()
    AppDefinedButton = Auto()


class OperatingModeEnum(IntEnum):
    SIT = 0
    LIT = 1


class TC_ICDM_2_1(MatterBaseTest):

    #
    # Class Helper functions
    #

    async def _read_icdm_attribute_expect_success(self, attribute):
        cluster = Clusters.Objects.IcdManagement
        return await self.read_single_attribute_check_success(endpoint=kRootEndpointId, cluster=cluster, attribute=attribute)

    async def _wildcard_cluster_read(self):
        cluster = Clusters.IcdManagement
        return await self.default_controller.ReadAttribute(self.dut_node_id, [(kRootEndpointId, cluster)])

    #
    # Test Harness Helpers
    #

    def desc_TC_ICDM_2_1(self) -> str:
        """Returns a description of this test"""
        return "[TC_ICDM_2_1]  Attributes with DUT as Server"

    def steps_TC_ICDM_2_1(self) -> list[TestStep]:
        steps = [
            TestStep(1, "Commissioning, already done", is_commissioning=True),
            TestStep(2, "Read ActiveModeThreshold attribute"),
            TestStep(3, "Read ActiveModeDuration attribute"),
            TestStep(4, "Read IdleModeDuration attribute"),
            TestStep(5, "Read ClientsSupportedPerFabric attribute"),
            TestStep(6, "Read RegisteredClients attribute"),
            TestStep(7, "Read ICDCounter attribute"),
            TestStep(8, "Read UserActiveModeTriggerHint attribute"),
            TestStep(9, "Read UserActiveModeTriggerInstruction attribute"),
            TestStep(10, "Read OperatingMode attribute"),

        ]
        return steps

    def pics_TC_ICDM_2_1(self) -> list[str]:
        """ This function returns a list of PICS for this test case that must be True for the test to be run"""
        pics = [
            "ICDM.S",
        ]
        return pics

    #
    # ICDM 2.1 Test Body
    #

    @async_test_body
    async def test_TC_ICDM_2_1(self):

        cluster = Clusters.Objects.IcdManagement
        attribute = cluster.Attributes

        # Commissioning
        self.step(1)

        # Wildcard Read cluster attributes
        icdmAttributes = await self._wildcard_cluster_read()

        # Validate ActiveModeThreshold
        self.step(2)
        if self.check_pics("ICDM.S.A0002"):
            activeModeThreshold = icdmAttributes[kRootEndpointId][cluster][attribute.ActiveModeThreshold]
            # Verify ActiveModeThreshold is not bigger than uint32
            asserts.assert_true(0 <= activeModeThreshold <= 65535,
                                "ActiveModeThreshold attribute does not fit in a uint16.")
        else:
            asserts.assert_true(
                False, "ActiveModeThreshold is a mandatory attribute and must be present in the PICS file")

        # Validate ActiveModeDuration
        self.step(3)
        if self.check_pics("ICDM.S.A0001"):
            activeModeDuration = icdmAttributes[kRootEndpointId][cluster][attribute.ActiveModeDuration]
            # Verify ActiveModeDuration is not bigger than uint32
            asserts.assert_true(0 <= activeModeDuration <= 4294967295,
                                "ActiveModeDuration attribute does not fit in a uint32")
        else:
            asserts.assert_true(
                False, "ActiveModeDuration is a mandatory attribute and must be present in the PICS file")

        # Validate IdleModeDuration
        self.step(4)
        if self.check_pics("ICDM.S.A0001"):
            idleModeDuration = icdmAttributes[kRootEndpointId][cluster][attribute.IdleModeDuration]
            # Verify IdleModeDuration is not bigger than uint32
            asserts.assert_greater_equal(
                idleModeDuration, 1, "IdleModeDuration attribute is smaller than minimum value (1).")
            asserts.assert_less_equal(
                idleModeDuration, 64800, "IdleModeDuration attribute is greater than maximum value (64800).")
            asserts.assert_greater_equal(idleModeDuration * 1000, activeModeDuration,
                                         "ActiveModeDuration attribute is greater than the IdleModeDuration attrbiute.")
        else:
            asserts.assert_true(
                False, "IdleModeDuration is a mandatory attribute and must be present in the PICS file")

        # Validate ClientsSupportedPerFabric
        self.step(5)
        if self.pics_guard(self.check_pics("ICDM.S.A0005")):
            clientsSupportedPerFabric = icdmAttributes[kRootEndpointId][
                cluster][attribute.ClientsSupportedPerFabric]

            # Verify ClientsSupportedPerFabric is not bigger than uint32
            asserts.assert_greater_equal(
                clientsSupportedPerFabric, 1, "ClientsSupportedPerFabric attribute is smaller than minimum value (1).")

            asserts.assert_less_equal(activeModeThreshold, 65535,
                                      "ClientsSupportedPerFabric attribute does not fit in a uint16.")

        # Validate RegisteredClients
        self.step(6)
        if self.pics_guard(self.check_pics("ICDM.S.A0005")):
            registeredClients = icdmAttributes[kRootEndpointId][cluster][attribute.RegisteredClients]

            asserts.assert_true(isinstance(
                registeredClients, list), "RegisteredClients is not a list.")

        # Validate ICDCounter
        self.step(7)
        if self.pics_guard(self.check_pics("ICDM.S.A0005")):

            icdCounter = icdmAttributes[kRootEndpointId][cluster][attribute.ICDCounter]
            # Verify ICDCounter is not bigger than uint32
            asserts.assert_true(0 <= icdCounter <= 4294967295,
                                "ActiveModeDuration attribute does not fit in a uint32")

        # Validate UserActiveModeTriggerHint
        self.step(8)
        userActiveModeTriggerHint = icdmAttributes[kRootEndpointId][cluster][attribute.UserActiveModeTriggerHint]

        # Valdate UserActiveModeTriggerInstruction
        self.step(9)

        # Validate OperatingMode
        self.step(10)

        # Idle Mode Duration attribute test
        # if (self.check_pics("ICDM.S.A0000")):
        #     self.print_step(2, "Read IdleModeDuration Attribute")

        #     idleModeDuration = await self.read_icdm_attribute_expect_success(
        #         attribute=attributes.IdleModeDuration)
        #     asserts.assert_greater_equal(
        #         idleModeDuration, 1, "IdleModeDuration attribute is smaller than minimum value (1).")
        #     asserts.assert_less_equal(
        #         idleModeDuration, 64800, "IdleModeDuration attribute is greater than maximum value (64800).")
        # else:
        #     asserts.assert_true(
        #         False, "IdleModeDuration is a mandatory attribute and must be present in the PICS file")

        # # Active Mode Duration attribute test
        # if (self.check_pics("ICDM.S.A0001")):
        #     self.print_step(2, "Read ActiveModeDuration Attribute")

        #     idleModeDuration *= 1000  # Convert seconds to milliseconds
        #     activeModeDuration = await self.read_icdm_attribute_expect_success(endpoint=endpoint,
        #                                                                        attribute=attributes.ActiveModeDuration)
        #     asserts.assert_true(0 <= activeModeDuration <= 65535,
        #                         "ActiveModeDuration attribute does not fit in a uint16.")
        #     asserts.assert_less_equal(activeModeDuration, idleModeDuration,
        #                               "ActiveModeDuration attribute is greater than the IdleModeDuration attrbiute.")
        # else:
        #     asserts.assert_true(
        #         False, "ActiveModeDuration is a mandatory attribute and must be present in the PICS file")

        # # Active Mode Threshold attribute test
        # if (self.check_pics("ICDM.S.A0002")):
        #     self.print_step(2, "Read ActiveModeThreshold Attribute")

        #     activeModeThreshold = await self.read_icdm_attribute_expect_success(endpoint=endpoint,
        #                                                                         attribute=attributes.ActiveModeThreshold)
        #     asserts.assert_true(0 <= activeModeThreshold <= 65535,
        #                         "ActiveModeThreshold attribute does not fit in a uint16.")
        # else:
        #     asserts.assert_true(
        #         False, "ActiveModeThreshold is a mandatory attribute and must be present in the PICS file")

        # # RegisteredClients attribute test
        # if (self.check_pics("ICDM.S.A0003")):
        #     self.print_step(2, "Read RegisteredClients Attribute")

        #     await self.read_icdm_attribute_expect_success(endpoint=endpoint,
        #                                                   attribute=attributes.RegisteredClients)

        # # ICDCounter attribute test
        # if (self.check_pics("ICDM.S.A0003")):
        #     self.print_step(2, "Read ICDCounter Attribute")

        #     ICDCounter = await self.read_icdm_attribute_expect_success(endpoint=endpoint,
        #                                                                attribute=attributes.ICDCounter)
        #     asserts.assert_true(0 <= ICDCounter <= 4294967295,
        #                         "ICDCounter attribute does not fit in a uint32.")

        # # ClientsSupportedPerFabric attribute test
        # if (self.check_pics("ICDM.S.A0003")):
        #     self.print_step(2, "Read ClientsSupportedPerFabric Attribute")

        #     clientsSupportedPerFabric = await self.read_icdm_attribute_expect_success(endpoint=endpoint,
        #                                                                               attribute=attributes.ClientsSupportedPerFabric)
        #     asserts.assert_true(0 <= clientsSupportedPerFabric <= 65535,
        #                         "ActiveModeThreshold ClientsSupportedPerFabric does not fit in a uint16.")
        #     asserts.assert_greater_equal(clientsSupportedPerFabric, 1,
        #                                  "ClientsSupportedPerFabric attribute is smaller than minimum value (1).")


if __name__ == "__main__":
    default_matter_test_main()
