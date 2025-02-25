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
#     app-args: --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json
#     script-args: >
#       --storage-path admin_storage.json
#       --commissioning-method on-network
#       --discriminator 1234
#       --passcode 20202021
#       --endpoint 1
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

import logging
from typing import Any, Optional, Union

import chip.clusters as Clusters
from chip.clusters import ClusterObjects as ClusterObjects
from chip.testing import matter_asserts
from chip.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from mobly import asserts

logger = logging.getLogger(__name__)


class TC_CC_2_1(MatterBaseTest):

    def TC_CC_2_1(self) -> str:
        return "25.2.1. [TC-CC-2.1] Attributes with server as DUT"

    def pics_TC_CC_2_1(self):
        """Return PICS definitions asscociated with this test."""
        pics = [
            "CC.S"
        ]
        return pics

    def steps_TC_CC_2_1(self) -> list[TestStep]:
        """Execute the test steps."""
        steps = [
            TestStep(1, 'Wait for the commissioned device to be retrieved', is_commissioning=True),
            TestStep(2, 'TH reads from the DUT the (0x0000) CurrentHue attribute'),
            TestStep(3, 'TH reads from the DUT the (0x0001) CurrentSaturation attribute'),
            TestStep(4, 'TH reads from the DUT the (0x0002) RemainingTime attribute'),
            TestStep(5, 'TH reads from the DUT the (0x0003) CurrentX attribute'),
            TestStep(6, 'TH reads from the DUT the (0x0004) CurrentY attribute'),
            TestStep(7, 'TH reads from the DUT the (0x0005) DriftCompensation attribute'),
            TestStep(8, 'TH reads from the DUT the (0x0006) CompensationText attribute'),
            TestStep(9, 'TH reads from the DUT the (0x0007) ColorTemperatureMireds attribute'),
            TestStep(10, 'TH reads from the DUT the (0x0008) ColorMode attribute'),
            TestStep(11, 'TH reads from the DUT the (0x000f) Options attribute'),
            TestStep(12, 'TH reads from the DUT the (0x4000) EnhancedCurrentHue attribute'),
            TestStep(13, 'TH reads from the DUT the (0x4001) EnhancedColorMode attribute'),
            TestStep(14, 'TH reads from the DUT the (0x4002) ColorLoopActive attribute'),
            TestStep(15, 'TH reads from the DUT the (0x4003) ColorLoopDirection attribute'),
            TestStep(16, 'TH reads from the DUT the (0x4004) ColorLoopTime attribute'),
            TestStep(17, 'TH reads from the DUT the (0x4005) ColorLoopStartEnhancedHue attribute'),
            TestStep(18, 'TH reads from the DUT the (0x4006) ColorLoopStoredEnhancedHue attributeT'),
            TestStep('18.a', 'Saving for comparision in step 19 reads FeatureMap attribute from DUT'),
            TestStep(19, 'TH reads from the DUT the (0x400a) ColorCapabilities attribute'),
            TestStep(20, 'TH reads from the DUT the (0x400b) ColorTempPhysicalMinMireds attribute'),
            TestStep(21, 'TH reads from the DUT the (0x400c) ColorTempPhysicalMaxMireds attribute'),
            TestStep(22, 'TH reads from the DUT the (0x400d) CoupleColorTempToLevelMinMireds attribute'),
            TestStep(23, 'TH reads from the DUT the (0x4010) StartUpColorTemperatureMireds attribute'),
            TestStep(24, 'TH reads from the DUT the (0x0010) NumberOfPrimaries attribute'),
            TestStep(
                25, 'TH reads Primary1X attribute from DUT and Verify that the DUT response contains an uint16 [Min:0 Max:0xfeff] if NumberOfPrimaries is 1 or more'),
            TestStep(
                26, 'TH reads Primary1Y attribute from DUT and Verify that the DUT response contains an uint16 [Min:0 Max:0xfeff] if NumberOfPrimaries is 1 or more'),
            TestStep(27, 'TH reads Primary1Intensity attribute from DUT and Verify that the DUT response contains an uint8 if NumberOfPrimaries is 1 or more'),
            TestStep(
                28, 'TH reads Primary2X attribute from DUT and Verify that the DUT response contains an uint16 [Min:0 Max:0xfeff] if NumberOfPrimaries is 2 or more'),
            TestStep(
                29, 'TH reads Primary2Y attribute from DUT and Verify that the DUT response contains an uint16 [Min:0 Max:0xfeff] if NumberOfPrimaries is 2 or more'),
            TestStep(30, 'TH reads Primary2Intensity attribute from DUT and Verify that the DUT response contains an uint8 if NumberOfPrimaries is 2 or more'),
            TestStep(
                31, 'TH reads Primary3X attribute from DUT and Verify that the DUT response contains an uint16 [Min:0 Max:0xfeff] if NumberOfPrimaries is 3 or more'),
            TestStep(
                32, 'TH reads Primary3Y attribute from DUT and Verify that the DUT response contains an uint16 [Min:0 Max:0xfeff] if NumberOfPrimaries is 3 or more'),
            TestStep(33, 'TH reads Primary3Intensity attribute from DUT and Verify that the DUT response contains an uint8 if NumberOfPrimaries is 3 or more'),
            TestStep(
                34, 'TH reads Primary4X attribute from DUT Verify that the DUT response contains an uint16 [Min:0 Max:0xfeff] if NumberOfPrimaries is 4 or more'),
            TestStep(
                35, 'TH reads Primary4Y attribute from DUT and Verify that the DUT response contains an uint16 [Min:0 Max:0xfeff] if NumberOfPrimaries is 4 or more'),
            TestStep(36, 'TH reads Primary4Intensity attribute from DUT and Verify that the DUT response contains an uint8 if NumberOfPrimaries is 4 or more'),
            TestStep(
                37, 'TH reads Primary5X attribute from DUT and Verify that the DUT response contains an uint16 [Min:0 Max:0xfeff] if NumberOfPrimaries is 5 or more'),
            TestStep(
                38, 'TH reads Primary5Y attribute from DUT and Verify that the DUT response contains an uint16 [Min:0 Max:0xfeff] if NumberOfPrimaries is 5 or more'),
            TestStep(39, 'TH reads Primary5Intensity attribute from DUT and Verify that the DUT response contains an uint8 if NumberOfPrimaries is 5 or more'),
            TestStep(
                40, 'TH reads Primary6X attribute from DUT and Verify that the DUT response contains an uint16 [Min:0 Max:0xfeff] if NumberOfPrimaries is 6 or more'),
            TestStep(
                41, 'TH reads Primary6Y attribute from DUT and Verify that the DUT response contains an uint16 [Min:0 Max:0xfeff] if NumberOfPrimaries is 6 or more'),
            TestStep(42, 'TH reads Primary6Intensity attribute from DUT and Verify that the DUT response contains an uint8 if NumberOfPrimaries is 6 or more'),
            TestStep(43, 'TH reads from the DUT the (0x0030) WhitePointX attribute'),
            TestStep(44, 'TH reads from the DUT the (0x0031) WhitePointY attribute'),
            TestStep(45, 'TH reads from the DUT the (0x0032) ColorPointRX attribute'),
            TestStep(46, 'TH reads from the DUT the (0x0033) ColorPointRY attribute'),
            TestStep(47, 'TH reads from the DUT the (0x0034) ColorPointRIntensity attribute'),
            TestStep(48, 'TH reads from the DUT the (0x0036) ColorPointGX attribute'),
            TestStep(49, 'TH reads from the DUT the (0x0037) ColorPointGY attribute'),
            TestStep(50, 'TH reads from the DUT the (0x0038) ColorPointGIntensity attribute'),
            TestStep(51, 'TH reads from the DUT the (0x003a) ColorPointBX attribute'),
            TestStep(52, 'TH reads from the DUT the (0x003b) ColorPointBY attribute'),
            TestStep(53, 'TH reads from the DUT the (0x003c) ColorPointBIntensity attribute')
        ]

        return steps

    async def _guard_and_read_attribute(self, attribute: str) -> Union[bool, Any]:
        """Check using guard and reads the attribute value.
        Args:
            attribute (str): Attribute name from the cluster.
        Returns:
            Union[bool, Any]: Return the guard_status and value from the attribute if reached.
        """
        tmp_attr = None
        attr_val = None
        try:
            tmp_attr = getattr(Clusters.ColorControl.Attributes, attribute)
        except AttributeError as e:
            asserts.fail(f"Error retrieving the attribute {str(e)}")

        logger.info(f"Trying guard on {attribute}")
        guard_status = self.attribute_guard(self.endpoint, tmp_attr)
        if guard_status:
            attr_val = await self.read_single_attribute_check_success(
                endpoint=self.endpoint,
                cluster=self.cluster,
                attribute=tmp_attr
            )
            logger.info(f"Value for attribute {attribute} is {attr_val}")
        return guard_status, attr_val

    async def _verify_attribute(self, attribute: str, data_type: str, enum_range: Optional[list] = None, min_len: Optional[int] = None, max_len: Optional[int] = None, skip_verification: bool = False):
        """Verify the attribute exists and value is the specific type of value.

        Args:
            attribute (str): Name of the attribute we want to retrieve
            data_type (str): type of data we want to validate against (uint8,uint16,uint32,string,enum)
            enum_range (list, optional): Range that the enum may have. 0-2 or 0-5. Defaults to Optional[list].
            min_len (int, optional): If present verify the low range of the attribute.
            max_len (int, optional): If present verify the high range of the attribute.
        """
        # If attribute_guard return false it set the current step as skipped so we can finish the step here.
        guard_status, attr_val = await self._guard_and_read_attribute(attribute=attribute)
        if not guard_status:
            return

        if not skip_verification:
            logger.info(f"Vefirying the attribute {attribute} should be {data_type}")

        if skip_verification:
            logger.info(f"Skipping verification for attribute {attribute}, current value is : {attr_val}")
            return attr_val

        logger.info(f"Current value for {attribute} is {attr_val}")
        if data_type == self.UINT8:
            logger.info("Checkng is uint8")
            matter_asserts.assert_valid_uint8(attr_val, "Is not uint8")
        elif data_type == self.UINT16:
            logger.info("Checkng is uint16")
            matter_asserts.assert_valid_uint16(attr_val, "Is not uint16")
        elif data_type == self.UINT32:
            logger.info("Checkng is uint32")
            matter_asserts.assert_valid_uint32(attr_val, "Is not uint32")
        elif data_type == self.ENUM:
            if len(enum_range) >= 0:
                logger.info(f"Checking for enum with range {enum_range}")
                asserts.assert_in(attr_val, enum_range, f"Value is not in range for enum with range {enum_range}")
            else:
                asserts.fail("Range list is empty")
        elif data_type == self.STRING and isinstance(attr_val, str):
            if max_len > 0:
                logger.info(f"Validating string with a max len of {max_len}")
                asserts.assert_true((len(attr_val) <= max_len), "String len is out of range.")
            else:
                asserts.fail("FAILED")
        else:
            asserts.fail("Validation not possible as not data type provided.")

        # if we land at this point it mean validation had passed
        # check if string has uint and verify is we need to compare against a min or max value
        if 'uint' in data_type and (max_len is not None or min_len is not None):
            if isinstance(max_len, int):
                logger.info(f"Max len defined validation max range for uint {max_len}")
                asserts.assert_true((attr_val <= max_len), f"Attribute {attribute} is out of range (max): {max_len}")
            if isinstance(min_len, int):
                logger.info(f"Min len defined validation max range for uint {max_len}")
                asserts.assert_true((attr_val >= min_len), f"Attribute {attribute} is out of range (min): {min_len}")

        return attr_val

    def _verify_for_numberofprimaries_value(self, numberofprimaries_value: Optional[int] = None, numberofprimaries_condition: int = 0) -> bool:
        """Verify the numbrofprimaries attribute value against the condition. Return the status to skip tests.
        Args:
            numberofprimaries_value (Optional[int], optional): _description_. Defaults to None.
            numberofprimaries_condition (int, optional): _description_. Defaults to 0.
        Returns:
            bool: Return the status of skipping verification.
        """
        skip_tests = False
        logger.info(
            f"Verifying the value of numberofprimaries {numberofprimaries_value} is greater than {numberofprimaries_condition} ?")
        # is not equal or greater than
        if numberofprimaries_value < numberofprimaries_condition:
            skip_tests = True

        return skip_tests

    def _verify_first_4bits(self, numa, numb):
        # get the first 4 bits and compare them
        tmp_a = numa & (2**4-1)
        tmp_b = numb & (2**4-1)
        logger.info("Verifying if first 4 bits are equal.")
        logger.info(f"Num a : {bin(tmp_a)}")
        logger.info(f"Num b : {bin(tmp_b)}")
        asserts.assert_equal(tmp_a, tmp_b, "Numbers are not equal in bit after bit comparsion")

    @async_test_body
    async def test_TC_CC_2_1(self):
        self.UINT8 = "uint8"
        self.UINT16 = "uint16"
        self.UINT32 = 'uint32'
        self.STRING = "string"
        self.ENUM = "enum"
        self.cluster = Clusters.ColorControl
        self.endpoint = self.get_endpoint(1)

        self.step(1)

        self.step(2)
        await self._verify_attribute("CurrentHue", self.UINT8)

        self.step(3)
        await self._verify_attribute("CurrentSaturation", self.UINT8)

        self.step(4)
        await self._verify_attribute("RemainingTime", self.UINT16)

        self.step(5)
        await self._verify_attribute("CurrentX", self.UINT16, max_len=0xfeff)

        self.step(6)
        await self._verify_attribute("CurrentY", self.UINT16, max_len=0xfeff)

        self.step(7)
        await self._verify_attribute("DriftCompensation", self.ENUM, enum_range=range(0, 5))

        self.step(8)
        await self._verify_attribute("CompensationText", self.STRING, max_len=254)

        self.step(9)
        await self._verify_attribute("ColorTemperatureMireds", self.UINT16, max_len=0xfeff)

        self.step(10)
        await self._verify_attribute("ColorMode", self.ENUM, enum_range=range(0, 3))

        self.step(11)
        await self._verify_attribute("Options", self.UINT8, max_len=4)

        self.step(12)
        await self._verify_attribute("EnhancedCurrentHue", self.UINT16)

        self.step(13)
        await self._verify_attribute("EnhancedColorMode", self.ENUM, enum_range=range(0, 4))

        self.step(14)
        await self._verify_attribute("ColorLoopActive", self.UINT8)

        self.step(15)
        await self._verify_attribute("ColorLoopDirection", self.UINT8)

        self.step(16)
        await self._verify_attribute("ColorLoopTime", self.UINT16)

        self.step(17)
        await self._verify_attribute("ColorLoopStartEnhancedHue", self.UINT16)

        self.step(18)
        await self._verify_attribute("ColorLoopStoredEnhancedHue", self.UINT16)

        self.step("18.a")
        # read and save FeatureMap attribute
        guard_status, feature_map_value = await self._guard_and_read_attribute("FeatureMap")

        self.step(19)
        color_capabilities_value = await self._verify_attribute("ColorCapabilities", self.UINT16, max_len=0x001f)
        # verify the first 4 bits of colorcapabilities are the same on FeatureMap
        self._verify_first_4bits(feature_map_value, color_capabilities_value)

        self.step(20)
        await self._verify_attribute("ColorTempPhysicalMinMireds", self.UINT16, max_len=0xfeff)

        self.step(21)
        await self._verify_attribute("ColorTempPhysicalMaxMireds", self.UINT16, max_len=0xfeff)

        self.step(22)
        await self._verify_attribute("CoupleColorTempToLevelMinMireds", self.UINT16, max_len=0xfeff)

        self.step(23)
        guard_status, sctmr_val = await self._guard_and_read_attribute("StartUpColorTemperatureMireds")
        if guard_status:
            asserts.assert_true(sctmr_val is None or (sctmr_val >= 1) and (sctmr_val <= 65279), "Value is out of range.")

        self.step(24)
        numberofprimaries_value = await self._verify_attribute("NumberOfPrimaries", self.UINT16, max_len=6)

        # verify for numberofprimaries section
        skip_steps_verifyp1 = self._verify_for_numberofprimaries_value(numberofprimaries_value, 1)
        self.step(25)
        await self._verify_attribute("Primary1X", self.UINT16, max_len=0xfeff, skip_verification=skip_steps_verifyp1)
        self.step(26)
        await self._verify_attribute("Primary1Y", self.UINT16, max_len=0xfeff, skip_verification=skip_steps_verifyp1)
        self.step(27)
        await self._verify_attribute("Primary1Intensity", self.UINT16, max_len=0xfeff, skip_verification=skip_steps_verifyp1)

        skip_steps_verifyp2 = self._verify_for_numberofprimaries_value(numberofprimaries_value, 2)
        self.step(28)
        await self._verify_attribute("Primary2X", self.UINT16, max_len=0xfeff, skip_verification=skip_steps_verifyp2)
        self.step(29)
        await self._verify_attribute("Primary2Y", self.UINT16, max_len=0xfeff, skip_verification=skip_steps_verifyp2)
        self.step(30)
        await self._verify_attribute("Primary2Intensity", self.UINT16, max_len=0xfeff, skip_verification=skip_steps_verifyp2)

        skip_steps_verifyp3 = self._verify_for_numberofprimaries_value(numberofprimaries_value, 3)
        self.step(31)
        await self._verify_attribute("Primary3X", self.UINT16, max_len=0xfeff, skip_verification=skip_steps_verifyp3)
        self.step(32)
        await self._verify_attribute("Primary3Y", self.UINT16, max_len=0xfeff, skip_verification=skip_steps_verifyp3)
        self.step(33)
        await self._verify_attribute("Primary3Intensity", self.UINT16, max_len=0xfeff, skip_verification=skip_steps_verifyp3)

        skip_steps_verifyp4 = self._verify_for_numberofprimaries_value(numberofprimaries_value, 4)
        self.step(34)
        await self._verify_attribute("Primary4X", self.UINT16, max_len=0xfeff, skip_verification=skip_steps_verifyp4)
        self.step(35)
        await self._verify_attribute("Primary4Y", self.UINT16, max_len=0xfeff, skip_verification=skip_steps_verifyp4)
        self.step(36)
        await self._verify_attribute("Primary4Intensity", self.UINT16, max_len=0xfeff, skip_verification=skip_steps_verifyp4)

        skip_steps_verifyp5 = self._verify_for_numberofprimaries_value(numberofprimaries_value, 5)
        self.step(37)
        await self._verify_attribute("Primary5X", self.UINT16, max_len=0xfeff, skip_verification=skip_steps_verifyp5)
        self.step(38)
        await self._verify_attribute("Primary5Y", self.UINT16, max_len=0xfeff, skip_verification=skip_steps_verifyp5)
        self.step(39)
        await self._verify_attribute("Primary5Intensity", self.UINT16, max_len=0xfeff, skip_verification=skip_steps_verifyp5)

        skip_steps_verifyp6 = self._verify_for_numberofprimaries_value(numberofprimaries_value, 6)
        self.step(40)
        await self._verify_attribute("Primary6X", self.UINT16, max_len=0xfeff, skip_verification=skip_steps_verifyp6)
        self.step(41)
        await self._verify_attribute("Primary6Y", self.UINT16, max_len=0xfeff, skip_verification=skip_steps_verifyp6)
        self.step(42)
        await self._verify_attribute("Primary6Intensity", self.UINT16, max_len=0xfeff, skip_verification=skip_steps_verifyp6)
        # no more check numberofprimaries at this point

        self.step(43)
        await self._verify_attribute("WhitePointX", self.UINT16, max_len=0xfeff)

        self.step(44)
        await self._verify_attribute("WhitePointY", self.UINT16, max_len=0xfeff)

        self.step(45)
        await self._verify_attribute("ColorPointRX", self.UINT16, max_len=0xfeff)

        self.step(46)
        await self._verify_attribute("ColorPointRY", self.UINT16, max_len=0xfeff)

        self.step(47)
        await self._verify_attribute("ColorPointRIntensity", self.UINT8)

        self.step(48)
        await self._verify_attribute("ColorPointGX", self.UINT16, max_len=0xfeff)

        self.step(49)
        await self._verify_attribute("ColorPointGY", self.UINT16, max_len=0xfeff)

        self.step(50)
        await self._verify_attribute("ColorPointGIntensity", self.UINT8)

        self.step(51)
        await self._verify_attribute("ColorPointBX", self.UINT16, max_len=0xfeff)

        self.step(52)
        await self._verify_attribute("ColorPointBY", self.UINT16, max_len=0xfeff)

        self.step(53)
        await self._verify_attribute("ColorPointBIntensity", self.UINT8)


if __name__ == "__main__":
    default_matter_test_main()
