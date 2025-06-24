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
#     app: ${LIGHTING_APP_NO_UNIQUE_ID}
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
from enum import Enum
from typing import Optional

import chip.clusters as Clusters
from chip.clusters import Attribute
from chip.clusters import ClusterObjects as ClusterObjects
from chip.clusters.Types import NullValue
from chip.testing import matter_asserts
from chip.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main, has_attribute
from mobly import asserts

logger = logging.getLogger(__name__)


class ValueTypesEnum(Enum):
    """Data types to validate in this case."""
    UINT8 = 1
    UINT16 = 2
    UINT32 = 3
    STRING = 4
    ENUM = 5


class TC_CC_2_1(MatterBaseTest):

    def desc_TC_CC_2_1(self) -> str:
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
            TestStep(24, 'TH reads from the DUT the (0x0010) NumberOfPrimaries attribute and verify that the DUT response contains an uint8 and value SHALL be in range from 0..6. NumberOfPrimaries attribute is consistent with the PrimaryN<X,Y,Intensity> attributes provided'),
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

    async def verify_primary_index(self, primary_index: int) -> bool:
        # Read all PrimaryN<X,Y,Intensity> attributes available in the cluster
        instance_attribute_names = [attr for attr in self.attributes.__dict__.keys(
        ) if attr.startswith('Primary')]
        # Attributes to search
        primary_list = ["Primary{primary}X", "Primary{primary}Y", "Primary{primary}Intensity"]
        # Validate the expected attribute is found.
        for primary in primary_list:
            attribute_str = primary.format(primary=primary_index)
            await self._populate_wildcard()
            has_attr_func = has_attribute(attribute=getattr(self.attributes, attribute_str))
            has_attr_status = has_attr_func(self.stored_global_wildcard, self.endpoint)
            if attribute_str not in instance_attribute_names or not has_attr_status:
                return False
        return True

    async def _get_totalnumberofprimaries(self, primaries: int) -> int:
        """Read the attributes which area Primaries<n>X,Y,Intensity attributes and return the total found and available in the current cluster.
        Returns:
            numberofprimaries(int) : Number of Primary attributes found.
        """
        numberofprimaries = 0
        for pindex in range(1, primaries+1):
            if await self.verify_primary_index(pindex):
                numberofprimaries += 1
            else:
                # Don't break to count all
                logger.warning(f"PrimaryN<X,Y,Intensity> with index {pindex} was not found in the cluster.")
        return numberofprimaries

    async def _verify_attribute(self, attribute: Attribute, data_type: ValueTypesEnum, enum_range: Optional[list] = None, min_len: Optional[int] = None, max_len: Optional[int] = None, nullable: bool = False):
        """Verify the attribute exists and value is the specific type of value.

        Args:
            attribute (Attribute): Name of the attribute we want to retrieve
            data_type (ValueTypesEnum): type of data we want to validate against (uint8,uint16,uint32,string,enum)
            enum_range (list, optional): Range that the enum may have. 0-2 or 0-5. Defaults to Optional[list].
            min_len (int, optional): If present verify the low range of the attribute.
            max_len (int, optional): If present verify the high range of the attribute.
        """
        # If attribute_guard return false it set the current step as skipped so we can finish the step here.
        logger.info(f"Verifying the attribute :  {attribute}")
        # Verify if the attribute is implemented in the current cluster.
        if await self.attribute_guard(endpoint=self.endpoint, attribute=attribute):
            # it is so retrieve the value to check the type.
            attr_val = await self.read_single_attribute_check_success(cluster=self.cluster, attribute=attribute, endpoint=self.endpoint)
            logger.info(f"Current value for {attribute} is {attr_val}")
            if nullable and attr_val is NullValue:
                logger.info("Value is NULL (ok)")
                return attr_val
            if data_type == ValueTypesEnum.UINT8:
                logger.info("Checking is uint8")
                matter_asserts.assert_valid_uint8(attr_val, "Is not uint8")
            elif data_type == ValueTypesEnum.UINT16:
                logger.info("Checking is uint16")
                matter_asserts.assert_valid_uint16(attr_val, "Is not uint16")
            elif data_type == ValueTypesEnum.UINT32:
                logger.info("Checking is uint32")
                matter_asserts.assert_valid_uint32(attr_val, "Is not uint32")
            elif data_type == ValueTypesEnum.ENUM:
                if len(enum_range) >= 0:
                    logger.info(f"Checking for enum with range {enum_range}")
                    asserts.assert_in(attr_val, enum_range, f"Value is not in range for enum with range {enum_range}")
                else:
                    asserts.fail("Range list is empty")
            elif data_type == ValueTypesEnum.STRING and isinstance(attr_val, str):
                if max_len > 0:
                    logger.info(f"Validating string with a max len of {max_len}")
                    asserts.assert_true((len(attr_val) <= max_len), "String len is out of range.")
                else:
                    asserts.fail("Invalid String range provided.")
            else:
                asserts.fail("Validation not possible as not data type provided.")

            # If we land at this point it mean validation had passed
            # Check if string has uint and verify is we need to compare against a min or max value_verify_first_4bits
            if 'uint' in data_type.name.lower() and (max_len is not None or min_len is not None):
                if isinstance(min_len, int):
                    logger.info(f"Min len defined validation max range for uint {min_len}")
                    asserts.assert_true((attr_val >= min_len),
                                        f"Attribute {attribute} with value {attr_val} is out of range (min): {min_len}")
                if isinstance(max_len, int):
                    logger.info(f"Max len defined validation max range for uint {max_len}")
                    asserts.assert_true((attr_val <= max_len),
                                        f"Attribute {attribute} with value {attr_val} is out of range (max): {max_len}")
            return attr_val

    def _verify_lower_4bits(self, numa, numb):
        # Get the lowest 4 bits and compare them
        tmp_a = numa & (2**4-1)
        tmp_b = numb & (2**4-1)
        logger.info("Verifying if lower 4 bits are equal.")
        logger.info(f"Num a : {bin(tmp_a)}")
        logger.info(f"Num b : {bin(tmp_b)}")
        asserts.assert_equal(tmp_a, tmp_b, "Lower 4 bits of values are not equal")

    @async_test_body
    async def test_TC_CC_2_1(self):
        self.cluster = Clusters.ColorControl
        self.endpoint = self.get_endpoint(1)
        self.attributes = Clusters.ColorControl.Attributes

        self.step(1)

        self.step(2)
        await self._verify_attribute(self.attributes.CurrentHue, ValueTypesEnum.UINT8)

        self.step(3)
        await self._verify_attribute(self.attributes.CurrentSaturation, ValueTypesEnum.UINT8)

        self.step(4)
        await self._verify_attribute(self.attributes.RemainingTime, ValueTypesEnum.UINT16)

        self.step(5)
        await self._verify_attribute(self.attributes.CurrentX, ValueTypesEnum.UINT16, max_len=65279)

        self.step(6)
        await self._verify_attribute(self.attributes.CurrentY, ValueTypesEnum.UINT16, max_len=65279)

        self.step(7)
        await self._verify_attribute(self.attributes.DriftCompensation, ValueTypesEnum.ENUM, enum_range=range(0, 5))

        self.step(8)
        await self._verify_attribute(self.attributes.CompensationText, ValueTypesEnum.STRING, max_len=254)

        self.step(9)
        await self._verify_attribute(self.attributes.ColorTemperatureMireds, ValueTypesEnum.UINT16, min_len=1, max_len=65279)

        self.step(10)
        await self._verify_attribute(self.attributes.ColorMode, ValueTypesEnum.ENUM, enum_range=range(0, 3))

        self.step(11)
        await self._verify_attribute(self.attributes.Options, ValueTypesEnum.UINT8, max_len=4)

        self.step(12)
        await self._verify_attribute(self.attributes.EnhancedCurrentHue, ValueTypesEnum.UINT16)

        self.step(13)
        await self._verify_attribute(self.attributes.EnhancedColorMode, ValueTypesEnum.ENUM, enum_range=range(0, 4))

        self.step(14)
        await self._verify_attribute(self.attributes.ColorLoopActive, ValueTypesEnum.UINT8)

        self.step(15)
        await self._verify_attribute(self.attributes.ColorLoopDirection, ValueTypesEnum.UINT8)

        self.step(16)
        await self._verify_attribute(self.attributes.ColorLoopTime, ValueTypesEnum.UINT16)

        self.step(17)
        await self._verify_attribute(self.attributes.ColorLoopStartEnhancedHue, ValueTypesEnum.UINT16)

        self.step(18)
        await self._verify_attribute(self.attributes.ColorLoopStoredEnhancedHue, ValueTypesEnum.UINT16)

        self.step("18.a")
        # Read and save FeatureMap attribute
        feature_map_value = await self.read_single_attribute_check_success(cluster=self.cluster, endpoint=self.endpoint, attribute=self.attributes.FeatureMap)

        self.step(19)
        color_capabilities_value = await self._verify_attribute(self.attributes.ColorCapabilities, ValueTypesEnum.UINT16, max_len=0x001f)
        # Verify the lower 4 bits of colorcapabilities are the same on FeatureMap
        self._verify_lower_4bits(feature_map_value, color_capabilities_value)

        self.step(20)
        colortempphysicalminmireds_val = await self._verify_attribute(self.attributes.ColorTempPhysicalMinMireds, ValueTypesEnum.UINT16, min_len=1, max_len=65279)

        self.step(21)
        colortempphysicalmaxmireds_val = await self._verify_attribute(self.attributes.ColorTempPhysicalMaxMireds, ValueTypesEnum.UINT16, min_len=1, max_len=65279)

        self.step(22)
        await self._verify_attribute(self.attributes.CoupleColorTempToLevelMinMireds, ValueTypesEnum.UINT16, min_len=colortempphysicalminmireds_val, max_len=colortempphysicalmaxmireds_val)

        self.step(23)
        # Manual check
        if await self.attribute_guard(endpoint=self.endpoint, attribute=self.attributes.StartUpColorTemperatureMireds):
            sctmr_val = await self.read_single_attribute_check_success(cluster=self.cluster, endpoint=self.endpoint, attribute=self.attributes.StartUpColorTemperatureMireds)
            asserts.assert_true(sctmr_val is NullValue or ((sctmr_val >= 1) and (sctmr_val <= 65279)), "Value is out of range.")

        # NumberOfPrimaries will be used to verify Primary<n>[X|Y|Intensity]
        # Number of primaries cant be 0, it should be greater or equal than 1.
        # Issue: #9103 to address remove 0 in NumberoOfPrimaries.
        # After this is resolved, check not 0.

        self.step(24)
        # Read NumberOfPrimaries from the cluster.
        number_of_primaries_value = await self._verify_attribute(self.attributes.NumberOfPrimaries, ValueTypesEnum.UINT8, min_len=0, max_len=6, nullable=True)
        if number_of_primaries_value is NullValue or number_of_primaries_value == 0:
            logger.info("NumberOfPrimaries is Null or 0 - skipping steps 25 through 42.")
            for i in range(25, 43):
                self.skip_step(i)
        else:
            primariesfound = await self._get_totalnumberofprimaries(primaries=number_of_primaries_value)
            logger.info(f"Fetched Primaries attributes {primariesfound}")
            asserts.assert_equal(number_of_primaries_value, primariesfound,
                                 "NumberOfPrimaries does not match with the Primaries attributes found in the cluster.")
            # Verify for NumberOfPrimaries section
            # We are at step 24 before all the number of primaries checks
            current_step = 24
            # Range is defined from 1-6
            for primariesindex in range(1, 7):
                logger.info(
                    f"Skip if the test index {primariesindex} is graeter than NumberOfPrimaries {number_of_primaries_value} ?")
                if primariesindex > number_of_primaries_value:
                    # Skip the 3 steps
                    logger.info(f"Skipping for NumberOfPrimaries {primariesindex}")
                    for i in range(1, 4):
                        current_step += 1
                        self.skip_step(current_step)
                    continue
                # Get the attributes to check then perform guard (at _verify_attribute)
                current_step += 1
                self.step(current_step)
                await self._verify_attribute(getattr(self.attributes, f"Primary{primariesindex}X"), ValueTypesEnum.UINT16, max_len=65279)
                current_step += 1
                self.step(current_step)
                await self._verify_attribute(getattr(self.attributes, f"Primary{primariesindex}Y"), ValueTypesEnum.UINT16, max_len=65279)
                current_step += 1
                self.step(current_step)
                await self._verify_attribute(getattr(self.attributes, f"Primary{primariesindex}Intensity"), ValueTypesEnum.UINT8, nullable=True)

        # No more check numberofprimaries at this point
        self.step(43)
        await self._verify_attribute(self.attributes.WhitePointX, ValueTypesEnum.UINT16, max_len=65279)

        self.step(44)
        await self._verify_attribute(self.attributes.WhitePointY, ValueTypesEnum.UINT16, max_len=65279)

        self.step(45)
        await self._verify_attribute(self.attributes.ColorPointRX, ValueTypesEnum.UINT16, max_len=65279)

        self.step(46)
        await self._verify_attribute(self.attributes.ColorPointRY, ValueTypesEnum.UINT16, max_len=65279)

        self.step(47)
        await self._verify_attribute(self.attributes.ColorPointRIntensity, ValueTypesEnum.UINT8, nullable=True)

        self.step(48)
        await self._verify_attribute(self.attributes.ColorPointGX, ValueTypesEnum.UINT16, max_len=65279)

        self.step(49)
        await self._verify_attribute(self.attributes.ColorPointGY, ValueTypesEnum.UINT16, max_len=65279)

        self.step(50)
        await self._verify_attribute(self.attributes.ColorPointGIntensity, ValueTypesEnum.UINT8, nullable=True)

        self.step(51)
        await self._verify_attribute(self.attributes.ColorPointBX, ValueTypesEnum.UINT16, max_len=65279)

        self.step(52)
        await self._verify_attribute(self.attributes.ColorPointBY, ValueTypesEnum.UINT16, max_len=65279)

        self.step(53)
        await self._verify_attribute(self.attributes.ColorPointBIntensity, ValueTypesEnum.UINT8, nullable=True)


if __name__ == "__main__":
    default_matter_test_main()
