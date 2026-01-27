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

from datetime import datetime

import pycountry
import validators
from mobly import asserts

from matter.clusters.ClusterObjects import Cluster
from matter.testing.conformance import ConformanceException
from matter.testing.matter_testing import MatterBaseTest, TestStep
from matter.testing.spec_parsing import dm_from_spec_version


class BasicInformationAttributesVerificationBase(MatterBaseTest):
    def steps(self) -> list[TestStep]:
        return [
            TestStep(0, "DUT commissioned if not already done", is_commissioning=True),
            TestStep(1, "TH reads DataModelRevision from the DUT.",
                     "Verify that the value is DataModelRevision of current matter version 1.5 which is value 19"),
            TestStep(2, "TH reads VendorName from the DUT.", "Verify that the VendorName returns a string with max 32 bytes"),
            TestStep(3, "TH reads VendorID from the DUT.", "Verify value is in the range of 0x0001 to 0xFFF4"),
            TestStep(4, "TH reads ProductName from the DUT.", "Verify it is a string with max length of 32 bytes."),
            TestStep(5, "TH reads ProductID from the DUT.", "Verify value is in the inclusive range of 1 to 65534"),
            TestStep(6, "TH reads NodeLabel from the DUT", "Verify it is of type string, and length is less than or equal to 32 bytes"),
            TestStep(7, "TH reads Location from the DUT.",
                     "Verify it is of type string, length is less than or equal to 2 characters, and verify that the Location is a valid CountryCode value per ISO 3166-1 alpha-2"),
            TestStep(8, "TH reads HardwareVersion from the DUT.", "Verify that the value is in range of 0 to 65534"),
            TestStep(9, "TH reads HardwareVersionString from the DUT.",
                     "Verify it is of type string, and value length in the range of 1 to 64 bytes"),
            TestStep(10, "TH reads SoftwareVersion from the DUT", "Verify that the value is the range of 0 to 4294967294"),
            TestStep(11, "TH reads SoftwareVersionString from the DUT.",
                     "Verify it is of type string, has a length of 1 to 64 bytes"),
            TestStep(12, "TH reads ManufacturingDate from the DUT",
                     "Verify it is of type string, length in the range of 8 to 16 bytes, and the first 8 characters specify date according to YYYYMMDD, additionally the date should not be in the future and not before the first Matter release date (2022-10-04)."),
            TestStep(13, "TH reads PartNumber from the DUT", "Verify it is of type string, and has a max length of 32 bytes"),
            TestStep(14, "TH reads ProductURL from the DUT.",
                     "Verify it is of type string, less than or equal to 256 ASCII characters, and follows the syntax rules specified in RFC 3986"),
            TestStep(15, "TH reads ProductLabel from the DUT.",
                     "Verify it is of type string, less than or equal to 64 bytes, and does not include the name of the vendor as defined within the VendorName attribute"),
            TestStep(16, "TH reads SerialNumber from the DUT.", "Verify it is of type string, and has a max length of 32 bytes"),
            TestStep(17, "TH reads LocalConfigDisabled from the DUT.", "Verify it is of type boolean, and is set to false"),
            TestStep(18, "TH reads Reachable from the DUT.", "Verify it is of type boolean, and is set to true"),
            TestStep(19, "TH reads UniqueID from the DUT.",
                     "Verify it is of type string, and verify that the value is not identical to SerialNumber attribute if SerialNumber attribute is supported"),
            TestStep(20, "TH reads CapabilityMinima attribute from the DUT",
                     "Verify that the CaseSessionsPerFabric is in the range of 3 to 65535, and SubscriptionsPerFabric is in the inclusive range of 3 to 65535"),
            TestStep(21, "TH reads ProductAppearance from the DUT.", "Verify it is of type ProductAppearanceStruct."),
            TestStep(22, "TH reads SpecificationVersion from the DUT.",
                     "Value should be set to a valid Major, Minor, and Dot version with the lower 8 bits set to zero."),
            TestStep(23, "TH reads MaxPathsPerInvoke from the DUT.", "Verify that the value is in the range of 1 to 65535."),
        ]

    def pics(self, cluster_pics) -> list[str]:
        return [f"{cluster_pics}.S"]

    async def implementation(self, cluster: Cluster):
        self.endpoint = self.get_endpoint()
        self.step(0)  # commissioning already done

        self.step(1)
        if hasattr(cluster.Attributes, 'DataModelRevision') and await self.attribute_guard(endpoint=self.endpoint, attribute=cluster.Attributes.DataModelRevision):
            ret1 = await self.read_single_attribute_check_success(cluster=cluster, attribute=cluster.Attributes.DataModelRevision)
            asserts.assert_equal(ret1, 19, "DataModelRevision should be 19")
        elif not hasattr(cluster.Attributes, 'DataModelRevision'):
            self.mark_current_step_skipped()

        # Step 2: VendorName
        self.step(2)
        if await self.attribute_guard(endpoint=self.endpoint, attribute=cluster.Attributes.VendorName):
            vendor_name = await self.read_single_attribute_check_success(cluster=cluster, attribute=cluster.Attributes.VendorName)
            asserts.assert_true(isinstance(vendor_name, str), "VendorName should be a string")
            asserts.assert_less_equal(len(vendor_name), 32, "VendorName should be a string with max 32 bytes")

        # Step 3: VendorID
        self.step(3)
        if await self.attribute_guard(endpoint=self.endpoint, attribute=cluster.Attributes.VendorID):
            ret3 = await self.read_single_attribute_check_success(cluster=cluster, attribute=cluster.Attributes.VendorID)
            asserts.assert_greater_equal(ret3, 0x0001, "VendorID should be greater than or equal to 0x0001")
            # 0xFFF1-0xFFF4 are reserved for test vendors, so we allow up to 0xFFF4
            asserts.assert_less_equal(ret3, 0xFFF4, "VendorID should be less than or equal to 0xFFF4")

        # Step 4: ProductName
        self.step(4)
        if await self.attribute_guard(endpoint=self.endpoint, attribute=cluster.Attributes.ProductName):
            ret4 = await self.read_single_attribute_check_success(cluster=cluster, attribute=cluster.Attributes.ProductName)
            asserts.assert_equal(len(ret4) <= 32, True, "ProductName should be a string with max 32 bytes")

        # Step 5: ProductID
        self.step(5)
        if await self.attribute_guard(endpoint=self.endpoint, attribute=cluster.Attributes.ProductID):
            ret5 = await self.read_single_attribute_check_success(cluster=cluster, attribute=cluster.Attributes.ProductID)
            asserts.assert_greater_equal(ret5, 1, "ProductID should be greater than or equal to 1")
            asserts.assert_less_equal(ret5, 65534, "ProductID should be less than or equal to 65534")

        # Step 6: NodeLabel
        self.step(6)
        if await self.attribute_guard(endpoint=self.endpoint, attribute=cluster.Attributes.NodeLabel):
            ret6 = await self.read_single_attribute_check_success(cluster=cluster, attribute=cluster.Attributes.NodeLabel)
            asserts.assert_equal(len(ret6) <= 32, True, "NodeLabel should be a string with max 32 bytes")

        # Step 7: Location
        self.step(7)
        if hasattr(cluster.Attributes, 'Location') and await self.attribute_guard(endpoint=self.endpoint, attribute=cluster.Attributes.Location):
            ret7 = await self.read_single_attribute_check_success(cluster=cluster, attribute=cluster.Attributes.Location)
            asserts.assert_true(isinstance(ret7, str), "Location should be a string")
            asserts.assert_less_equal(len(ret7), 2, "Location should have max 2 characters")
            # Location should be a valid ISO 3166-1 alpha-2 country code or empty string "XX"
            if ret7 != "XX":
                # Use pycountry to validate the country code
                country = pycountry.countries.get(alpha_2=ret7)
                asserts.assert_is_not_none(country, f"Location '{ret7}' should be a valid ISO 3166-1 alpha-2 country code")
        elif not hasattr(cluster.Attributes, 'Location'):
            self.mark_current_step_skipped()

        # Step 8: HardwareVersion
        self.step(8)
        if await self.attribute_guard(endpoint=self.endpoint, attribute=cluster.Attributes.HardwareVersion):
            ret8 = await self.read_single_attribute_check_success(cluster=cluster, attribute=cluster.Attributes.HardwareVersion)
            asserts.assert_greater_equal(ret8, 0, "HardwareVersion should be greater than or equal to 0")
            asserts.assert_less_equal(ret8, 65534, "HardwareVersion should be less than or equal to 65534")

        # Step 9: HardwareVersionString
        self.step(9)
        if await self.attribute_guard(endpoint=self.endpoint, attribute=cluster.Attributes.HardwareVersionString):
            ret9 = await self.read_single_attribute_check_success(cluster=cluster, attribute=cluster.Attributes.HardwareVersionString)
            asserts.assert_true(isinstance(ret9, str), "HardwareVersionString should be a string")
            asserts.assert_equal(len(ret9) <= 64, True, "HardwareVersionString should be a string with max 64 bytes")

        # Step 10: SoftwareVersion
        self.step(10)
        if await self.attribute_guard(endpoint=self.endpoint, attribute=cluster.Attributes.SoftwareVersion):
            ret10 = await self.read_single_attribute_check_success(cluster=cluster, attribute=cluster.Attributes.SoftwareVersion)
            asserts.assert_greater_equal(ret10, 0, "SoftwareVersion should be greater than or equal to 0")
            asserts.assert_less_equal(ret10, 4294967294, "SoftwareVersion should be less than or equal to 4294967294")

        # Step 11: SoftwareVersionString
        self.step(11)
        if await self.attribute_guard(endpoint=self.endpoint, attribute=cluster.Attributes.SoftwareVersionString):
            ret11 = await self.read_single_attribute_check_success(cluster=cluster, attribute=cluster.Attributes.SoftwareVersionString)
            asserts.assert_true(isinstance(ret11, str), "SoftwareVersionString should be a string")
            asserts.assert_equal(len(ret11) <= 64, True, "SoftwareVersionString should be a string with max 64 bytes")

        # Step 12: ManufacturingDate
        self.step(12)
        if await self.attribute_guard(endpoint=self.endpoint, attribute=cluster.Attributes.ManufacturingDate):
            ret12 = await self.read_single_attribute_check_success(cluster=cluster, attribute=cluster.Attributes.ManufacturingDate)
            asserts.assert_true(isinstance(ret12, str), "Manufacturing Date should be a string")
            asserts.assert_greater_equal(len(ret12), 8, "ManufacturingDate should have at least 8 characters")
            asserts.assert_less_equal(len(ret12), 16, "ManufacturingDate should have max 16 characters")
            # Validate the first 8 characters are a valid date in YYYYMMDD format
            date_str = ret12[:8]
            try:
                parsed_date = datetime.strptime(date_str, "%Y%m%d")

                # Verify it's not a future date
                asserts.assert_less_equal(parsed_date, datetime.now(), "ManufacturingDate should not be in the future")

                # Verify it's also not before the first Matter release (Matter 1.0 released October 4, 2022)
                # Allow 2020-01-01 as a valid date (SDK default value)
                first_matter_release = datetime(2022, 10, 4)
                sdk_default_date = datetime(2020, 1, 1)
                is_ci = self.check_pics("PICS_SDK_CI_ONLY")
                if is_ci:
                    is_valid_date = parsed_date >= first_matter_release or parsed_date == sdk_default_date
                    asserts.assert_true(
                        is_valid_date,
                        "ManufacturingDate should not be before the first Matter release date (2022-10-04), except for SDK default value (2020-01-01)")
                else:
                    asserts.assert_greater_equal(
                        parsed_date, first_matter_release,
                        "ManufacturingDate should not be before the first Matter release date (2022-10-04)")
            except ValueError:
                asserts.fail(f"ManufacturingDate '{date_str}' is not a valid date in YYYYMMDD format")

        # Step 13: PartNumber
        self.step(13)
        if await self.attribute_guard(endpoint=self.endpoint, attribute=cluster.Attributes.PartNumber):
            ret13 = await self.read_single_attribute_check_success(cluster=cluster, attribute=cluster.Attributes.PartNumber)
            asserts.assert_true(isinstance(ret13, str), "PartNumber should be a string")
            asserts.assert_equal(len(ret13) <= 32, True, "PartNumber should be a string with max 32 bytes")

        # Step 14: ProductURL
        self.step(14)
        if await self.attribute_guard(endpoint=self.endpoint, attribute=cluster.Attributes.ProductURL):
            ret14 = await self.read_single_attribute_check_success(cluster=cluster, attribute=cluster.Attributes.ProductURL)
            asserts.assert_true(isinstance(ret14, str), "ProductURL should be a string")
            asserts.assert_less_equal(len(ret14), 256, "ProductURL should be a string with max 256 bytes")
            # Only validate URL format if the string is not empty
            if len(ret14) > 0:
                asserts.assert_true(validators.url(ret14), f"ProductURL '{ret14}' should be a valid URL per RFC 3986")

        # Step 15: ProductLabel
        self.step(15)
        if await self.attribute_guard(endpoint=self.endpoint, attribute=cluster.Attributes.ProductLabel):
            ret15 = await self.read_single_attribute_check_success(cluster=cluster, attribute=cluster.Attributes.ProductLabel)
            asserts.assert_true(isinstance(ret15, str), "ProductLabel should be a string")
            asserts.assert_equal(len(ret15) <= 64, True, "ProductLabel should be a string with max 64 bytes")
            if vendor_name:
                asserts.assert_not_in(
                    vendor_name, ret15, "ProductLabel should not include the name of the vendor as defined within the VendorName attribute")

        # Step 16: SerialNumber
        self.step(16)
        if await self.attribute_guard(endpoint=self.endpoint, attribute=cluster.Attributes.SerialNumber):
            serial_number = await self.read_single_attribute_check_success(cluster=cluster, attribute=cluster.Attributes.SerialNumber)
            asserts.assert_true(isinstance(serial_number, str), "SerialNumber should be a string")
            asserts.assert_less_equal(len(serial_number), 32, "SerialNumber should be a string with max 32 bytes")

        # Step 17: LocalConfigDisabled
        self.step(17)
        if hasattr(cluster.Attributes, 'LocalConfigDisabled') and await self.attribute_guard(endpoint=self.endpoint, attribute=cluster.Attributes.LocalConfigDisabled):
            ret17 = await self.read_single_attribute_check_success(cluster=cluster, attribute=cluster.Attributes.LocalConfigDisabled)
            asserts.assert_true(isinstance(ret17, bool), "LocalConfigDisabled should be a boolean")
            asserts.assert_equal(ret17, False, "LocalConfigDisabled should be set to false")
        elif not hasattr(cluster.Attributes, 'LocalConfigDisabled'):
            self.mark_current_step_skipped()

        # Step 18: Reachable
        self.step(18)
        if await self.attribute_guard(endpoint=self.endpoint, attribute=cluster.Attributes.Reachable):
            ret18 = await self.read_single_attribute_check_success(cluster=cluster, attribute=cluster.Attributes.Reachable)
            asserts.assert_true(isinstance(ret18, bool), "Reachable should be a boolean")
            asserts.assert_equal(ret18, True, "Reachable should be set to true")

        # Step 19: UniqueID
        self.step(19)
        if await self.attribute_guard(endpoint=self.endpoint, attribute=cluster.Attributes.UniqueID):
            ret19 = await self.read_single_attribute_check_success(cluster=cluster, attribute=cluster.Attributes.UniqueID)
            asserts.assert_true(isinstance(ret19, str), "UniqueID should be a string")
            if await self.attribute_guard(endpoint=self.endpoint, attribute=cluster.Attributes.SerialNumber):
                asserts.assert_not_equal(
                    ret19, serial_number, "UniqueID should not be identical to SerialNumber attribute if SerialNumber attribute is supported")

        # Step 20: CapabilityMinima
        self.step(20)
        if hasattr(cluster.Attributes, 'CapabilityMinima') and await self.attribute_guard(endpoint=self.endpoint, attribute=cluster.Attributes.CapabilityMinima):
            capability_minima = await self.read_single_attribute_check_success(cluster=cluster, attribute=cluster.Attributes.CapabilityMinima)
            asserts.assert_greater_equal(capability_minima.caseSessionsPerFabric, 3,
                                         "CaseSessionsPerFabric should be greater than or equal to 3")
            asserts.assert_less_equal(capability_minima.caseSessionsPerFabric, 65535,
                                      "CaseSessionsPerFabric should be less than or equal to 65535")
            asserts.assert_greater_equal(capability_minima.subscriptionsPerFabric, 3,
                                         "SubscriptionsPerFabric should be greater than or equal to 3")
            asserts.assert_less_equal(capability_minima.subscriptionsPerFabric, 65535,
                                      "SubscriptionsPerFabric should be less than or equal to 65535")
        elif not hasattr(cluster.Attributes, 'CapabilityMinima'):
            self.mark_current_step_skipped()

        # Step 21: ProductAppearance
        self.step(21)
        if await self.attribute_guard(endpoint=self.endpoint, attribute=cluster.Attributes.ProductAppearance):
            ret21 = await self.read_single_attribute_check_success(cluster=cluster, attribute=cluster.Attributes.ProductAppearance)
            asserts.assert_true(isinstance(ret21, cluster.Structs.ProductAppearanceStruct),
                                "ProductAppearance should be a ProductAppearanceStruct")

        # Step 22: SpecificationVersion
        self.step(22)
        if hasattr(cluster.Attributes, 'SpecificationVersion') and await self.attribute_guard(endpoint=self.endpoint, attribute=cluster.Attributes.SpecificationVersion):
            ret22 = await self.read_single_attribute_check_success(cluster=cluster, attribute=cluster.Attributes.SpecificationVersion)
            try:
                dm_from_spec_version(ret22)
            except ConformanceException:
                asserts.fail(f'Unknown SpecificationVersion {ret22:08X}')
        elif not hasattr(cluster.Attributes, 'SpecificationVersion'):
            self.mark_current_step_skipped()

        # Step 23: MaxPathsPerInvoke
        self.step(23)
        if hasattr(cluster.Attributes, 'MaxPathsPerInvoke') and await self.attribute_guard(endpoint=self.endpoint, attribute=cluster.Attributes.MaxPathsPerInvoke):
            ret23 = await self.read_single_attribute_check_success(cluster=cluster, attribute=cluster.Attributes.MaxPathsPerInvoke)
            asserts.assert_greater_equal(ret23, 1, "MaxPathsPerInvoke should be greater than or equal to 1")
            asserts.assert_less_equal(ret23, 65535, "MaxPathsPerInvoke should be less than or equal to 65535")
        elif not hasattr(cluster.Attributes, 'MaxPathsPerInvoke'):
            self.mark_current_step_skipped()
