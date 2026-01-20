#    Copyright (c) 2026 Project CHIP Authors
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
#       --endpoint 0
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#       --PICS src/app/tests/suites/certification/ci-pics-values
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

import datetime
import logging
from urllib.parse import urlparse

from mobly import asserts

import matter.clusters as Clusters
from matter.interaction_model import Status
from matter.testing.decorators import has_cluster, run_if_endpoint_matches
from matter.testing.matter_asserts import (assert_is_string, assert_string_length, assert_valid_bool, assert_valid_uint16,
                                           assert_valid_uint32)
from matter.testing.matter_testing import MatterBaseTest
from matter.testing.runner import TestStep, default_matter_test_main

log = logging.getLogger(__name__)


class TC_BINFO_2_1(MatterBaseTest):
    def pics_TC_BINFO_2_1(self) -> list[str]:
        return ["BINFO.S"]

    def steps_TC_BINFO_2_1(self) -> list[TestStep]:
        return [
            TestStep(1, "Commissioning, TH Saves the CountryCode Information", is_commissioning=True),
            TestStep(2, "TH reads DataModelRevision from the DUT"),
            TestStep(3, "TH writes DataModelRevision", "Write fails with UNSUPPORTED_WRITE"),
            TestStep(4, "TH reads DataModelRevision from the DUT"),
            TestStep(5, "TH reads VendorName from the DUT"),
            TestStep(6, "TH writes VendorName", "Write fails with UNSUPPORTED_WRITE"),
            TestStep(7, "TH reads VendorName from the DUT"),
            TestStep(8, "TH reads VendorID from the DUT"),
            TestStep(9, "TH writes VendorID", "Write fails with UNSUPPORTED_WRITE"),
            TestStep(10, "TH reads VendorID from the DUT"),
            TestStep(11, "TH reads ProductName from the DUT"),
            TestStep(12, "TH writes ProductName", "Write fails with UNSUPPORTED_WRITE"),
            TestStep(13, "TH reads ProductName from the DUT"),
            TestStep(14, "TH reads ProductID from the DUT"),
            TestStep(15, "TH writes ProductID", "Write fails with UNSUPPORTED_WRITE"),
            TestStep(16, "TH reads ProductID from the DUT"),
            TestStep(17, "TH reads NodeLabel from the DUT"),
            TestStep(18, "TH writes NodeLabel", "Verify success"),
            TestStep(19, "TH reads NodeLabel and verify matches written value"),
            TestStep("20a", "TH reads Location from the DUT"),
            TestStep("20b", "TH writes Location to a different country code"),
            TestStep("20c", "TH reads Location and verify matches written value"),
            TestStep(21, "TH reads HardwareVersion from the DUT"),
            TestStep(22, "TH writes HardwareVersion", "Write fails with UNSUPPORTED_WRITE"),
            TestStep(23, "TH reads HardwareVersion from the DUT"),
            TestStep(24, "TH reads HardwareVersionString from the DUT"),
            TestStep(25, "TH writes HardwareVersionString", "Write fails with UNSUPPORTED_WRITE"),
            TestStep(26, "TH reads HardwareVersionString from the DUT"),
            TestStep(27, "TH reads SoftwareVersion from the DUT"),
            TestStep(28, "TH writes SoftwareVersion", "Write fails with UNSUPPORTED_WRITE"),
            TestStep(29, "TH reads SoftwareVersion from the DUT"),
            TestStep(30, "TH reads SoftwareVersionString from the DUT"),
            TestStep(31, "TH writes SoftwareVersionString", "Write fails with UNSUPPORTED_WRITE"),
            TestStep(32, "TH reads SoftwareVersionString from the DUT"),
            TestStep(33, "TH reads ManufacturingDate from the DUT"),
            TestStep(34, "TH writes ManufacturingDate", "Write fails with UNSUPPORTED_WRITE"),
            TestStep(35, "TH reads ManufacturingDate from the DUT"),
            TestStep(36, "TH reads PartNumber from the DUT"),
            TestStep(37, "TH writes PartNumber", "Write fails with UNSUPPORTED_WRITE"),
            TestStep(38, "TH reads PartNumber from the DUT"),
            TestStep(39, "TH reads ProductURL from the DUT"),
            TestStep(40, "TH writes ProductURL", "Write fails with UNSUPPORTED_WRITE"),
            TestStep(41, "TH reads ProductURL from the DUT"),
            TestStep(42, "TH reads ProductLabel from the DUT"),
            TestStep(43, "TH writes ProductLabel", "Write fails with UNSUPPORTED_WRITE"),
            TestStep(44, "TH reads ProductLabel from the DUT"),
            TestStep(45, "TH reads SerialNumber from the DUT"),
            TestStep(46, "TH writes SerialNumber", "Write fails with UNSUPPORTED_WRITE"),
            TestStep(47, "TH reads SerialNumber from the DUT"),
            TestStep(48, "TH reads LocalConfigDisabled from the DUT"),
            TestStep(49, "TH sets LocalConfigDisabled to True"),
            TestStep(50, "TH reads LocalConfigDisabled from the DUT"),
            TestStep(51, "TH reads Reachable from the DUT"),
            TestStep("52a", "TH writes Reachable to false", "Write fails with UNSUPPORTED_WRITE"),
            TestStep("52b", "TH reads Reachable from the DUT"),
            TestStep(53, "TH reads UniqueID from the DUT"),
            TestStep(54, "TH writes UniqueID", "Write fails with UNSUPPORTED_WRITE"),
            TestStep(55, "TH reads UniqueID from the DUT"),
            TestStep(56, "TH reads CapabilityMinima from the DUT and checks fields"),
            TestStep(57, "TH writes CapabilityMinima (CaseSessionsPerFabric)", "Write fails with UNSUPPORTED_WRITE"),
            TestStep(58, "TH writes CapabilityMinima (SimultaneousInvocationsSupported)", "Write fails with UNSUPPORTED_WRITE"),
            TestStep(59, "TH reads CapabilityMinima from the DUT"),
            TestStep(60, "TH reads ProductAppearance from the DUT"),
            TestStep(61, "TH writes ProductAppearance", "Write fails with UNSUPPORTED_WRITE"),
            TestStep(62, "TH reads ProductAppearance from the DUT"),
            TestStep(63, "TH reads SpecificationVersion from the DUT"),
            TestStep(64, "TH writes SpecificationVersion", "Write fails with UNSUPPORTED_WRITE"),
            TestStep(65, "TH reads SpecificationVersion from the DUT"),
            TestStep(66, "TH reads MaxPathsPerInvoke from the DUT"),
            TestStep(67, "TH writes MaxPathsPerInvoke", "Write fails with UNSUPPORTED_WRITE"),
            TestStep(68, "TH reads MaxPathsPerInvoke from the DUT"),
            TestStep(69, "TH reads ConfigurationVersion from the DUT"),
            TestStep(70, "TH writes ConfigurationVersion", "Write fails with UNSUPPORTED_WRITE"),
            TestStep(71, "TH reads ConfigurationVersion from the DUT")
        ]

    # Helper to check unsupported writes
    async def verify_unsupported_write(self, attribute, value, endpoint):
        status = await self.write_single_attribute(
            attribute_value=value,
            endpoint_id=endpoint,
            expect_success=False
        )
        asserts.assert_equal(status, Status.UnsupportedWrite, f"Expected UNSUPPORTED_WRITE for {attribute}")

    @run_if_endpoint_matches(has_cluster(Clusters.BasicInformation))
    async def test_TC_BINFO_2_1(self):
        # We assume endpoint 0 for Basic Information cluster
        self.endpoint = self.get_endpoint()
        cluster = Clusters.BasicInformation
        attributes = cluster.Attributes

        # Step 1: Commissioning
        self.step(1)
        log.info("Commissioning successful")

        # Step 2: Read DataModelRevision
        self.step(2)
        val = await self.read_single_attribute_check_success(cluster=cluster, attribute=attributes.DataModelRevision, endpoint=self.endpoint)
        log.info(f"Read DataModelRevision: {val}")
        assert_valid_uint16(val, "DataModelRevision")
        asserts.assert_equal(val, 19, "DataModelRevision should be 19")

        # Step 3: Write DataModelRevision
        self.step(3)
        val_to_write = attributes.DataModelRevision(4388)
        log.info(f"Attempting to write DataModelRevision to {val_to_write} (expect failure)")
        await self.verify_unsupported_write(attributes.DataModelRevision, val_to_write, self.endpoint)
        log.info("Verified UNSUPPORTED_WRITE for DataModelRevision")

        # Step 4: Check if value changed
        self.step(4)
        val_after = await self.read_single_attribute_check_success(cluster=cluster, attribute=attributes.DataModelRevision, endpoint=self.endpoint)
        log.info(f"Read DataModelRevision again: {val_after}")
        asserts.assert_equal(val, val_after, "DataModelRevision changed after failed write")
        log.info("Verified DataModelRevision verification successful")

        # Step 5: Read VendorName
        self.step(5)
        vendor_name = await self.read_single_attribute_check_success(cluster=cluster, attribute=attributes.VendorName, endpoint=self.endpoint)
        log.info(f"Read VendorName: {vendor_name}")
        assert_is_string(vendor_name, "VendorName should be a string")
        assert_string_length(vendor_name, max_length=32, description="VendorName max length is 32")

        # Step 6: Write VendorName
        self.step(6)
        val_to_write = attributes.VendorName("sample")
        log.info(f"Attempting to write VendorName to {val_to_write} (expect failure)")
        await self.verify_unsupported_write(attributes.VendorName, val_to_write, self.endpoint)
        log.info("Verified UNSUPPORTED_WRITE for VendorName")

        # Step 7: Verify VendorName unchanged
        self.step(7)
        val = await self.read_single_attribute_check_success(cluster=cluster, attribute=attributes.VendorName, endpoint=self.endpoint)
        log.info(f"Read VendorName again: {val}")
        asserts.assert_equal(val, vendor_name, "VendorName changed after failed write")
        log.info("Verified VendorName unchanged verification successful")

        # Step 8: VendorID
        self.step(8)
        vendor_id = await self.read_single_attribute_check_success(cluster=cluster, attribute=attributes.VendorID, endpoint=self.endpoint)
        log.info(f"Read VendorID: {vendor_id}")
        asserts.assert_greater_equal(vendor_id, 1, "VendorID can't be less than 1")
        asserts.assert_less_equal(vendor_id, 65521, "VendorID can't be greater than 65521")

        # Step 9: Write VendorID
        self.step(9)
        val_to_write = attributes.VendorID(17)
        log.info(f"Attempting to write VendorID to {val_to_write} (expect failure)")
        await self.verify_unsupported_write(attributes.VendorID, val_to_write, self.endpoint)
        log.info("Verified UNSUPPORTED_WRITE for VendorID")

        # Step 10: Verify VendorID unchanged
        self.step(10)
        val = await self.read_single_attribute_check_success(cluster=cluster, attribute=attributes.VendorID, endpoint=self.endpoint)
        log.info(f"Read VendorID again: {val}")
        asserts.assert_equal(val, vendor_id, "VendorID changed after failed write")
        log.info("Verified VendorID unchanged verification successful")

        # Step 11: ProductName
        self.step(11)
        product_name = await self.read_single_attribute_check_success(cluster=cluster, attribute=attributes.ProductName, endpoint=self.endpoint)
        log.info(f"Read ProductName: {product_name}")
        assert_is_string(product_name, "ProductName")
        assert_string_length(product_name, max_length=32, description="ProductName")

        # Step 12: Write ProductName
        self.step(12)
        val_to_write = attributes.ProductName("newproduct")
        log.info(f"Attempting to write ProductName to {val_to_write} (expect failure)")
        await self.verify_unsupported_write(attributes.ProductName, val_to_write, self.endpoint)
        log.info("Verified UNSUPPORTED_WRITE for ProductName")

        # Step 13: Verify ProductName unchanged
        self.step(13)
        val = await self.read_single_attribute_check_success(cluster=cluster, attribute=attributes.ProductName, endpoint=self.endpoint)
        log.info(f"Read ProductName again: {val}")
        asserts.assert_equal(val, product_name, "ProductName changed after failed write")
        log.info("Verified ProductName unchanged verification successful")

        # Step 14: ProductID
        self.step(14)
        product_id = await self.read_single_attribute_check_success(cluster=cluster, attribute=attributes.ProductID, endpoint=self.endpoint)
        log.info(f"Read ProductID: {product_id}")
        assert_valid_uint16(product_id, "ProductID should be an int16u")
        asserts.assert_greater_equal(product_id, 1, "ProductID can't be less than 1")
        asserts.assert_less_equal(product_id, 65534, "ProductID can't be greater than 65534")

        # Step 15: Write ProductID
        self.step(15)
        val_to_write = attributes.ProductID(17697)
        log.info(f"Attempting to write ProductID to {val_to_write} (expect failure)")
        await self.verify_unsupported_write(attributes.ProductID, val_to_write, self.endpoint)
        log.info("Verified UNSUPPORTED_WRITE for ProductID")

        # Step 16: Verify ProductID unchanged
        self.step(16)
        val = await self.read_single_attribute_check_success(cluster=cluster, attribute=attributes.ProductID, endpoint=self.endpoint)
        log.info(f"Read ProductID again: {val}")
        asserts.assert_equal(val, product_id, "ProductID changed after failed write")
        log.info("Verified ProductID unchanged verification successful")

        # Step 17: NodeLabel
        self.step(17)
        node_label = await self.read_single_attribute_check_success(cluster=cluster, attribute=attributes.NodeLabel, endpoint=self.endpoint)
        log.info(f"Read NodeLabel: {node_label}")
        assert_is_string(node_label, "NodeLabel")
        assert_string_length(node_label, max_length=32, description="NodeLabel")

        # Step 18: Write NodeLabel (Should SUCCEED)
        self.step(18)
        log.info("Attempting to write NodeLabel to 'newnode'")
        await self.write_single_attribute(attributes.NodeLabel("newnode"), endpoint_id=self.endpoint)

        # Step 19: Verify NodeLabel unchanged
        self.step(19)
        new_label = await self.read_single_attribute_check_success(cluster=cluster, attribute=attributes.NodeLabel, endpoint=self.endpoint)
        log.info(f"Read NodeLabel again: {new_label}")
        asserts.assert_equal(new_label, "newnode", "NodeLabel update failed")
        log.info("Verified NodeLabel update successful")

        # Step 20a: Location
        self.step("20a")
        location = await self.read_single_attribute_check_success(cluster=cluster, attribute=attributes.Location, endpoint=self.endpoint)
        log.info(f"Read Location: {location}")
        asserts.assert_equal(len(location), 2, "Location should be a 2-character Country Code")
        # Note: YAML Step 1 traces show "XX" being used during commissioning.
        # Ideally we would match this with the value sent during commissioning.

        # Step 20b: Write Location (Should SUCCEED)
        self.step("20b")
        log.info("Attempting to write Location to 'in'")
        await self.write_single_attribute(attributes.Location("in"), endpoint_id=self.endpoint)

        # Step 20c
        self.step("20c")
        new_loc = await self.read_single_attribute_check_success(cluster=cluster, attribute=attributes.Location, endpoint=self.endpoint)
        log.info(f"Read Location again: {new_loc}")
        asserts.assert_equal(new_loc, "in", "Location update failed")
        log.info("Verified Location update successful")

        # Step 21: HardwareVersion
        self.step(21)
        hw_ver = await self.read_single_attribute_check_success(cluster=cluster, attribute=attributes.HardwareVersion, endpoint=self.endpoint)
        log.info(f"Read HardwareVersion: {hw_ver}")
        assert_valid_uint16(hw_ver, "HardwareVersion should be an int16u")
        asserts.assert_greater_equal(hw_ver, 0, "HardwareVersion can't be less than 0")
        asserts.assert_less_equal(hw_ver, 65534, "HardwareVersion can't be greater than 65534")

        # Step 22: Write HardwareVersion
        self.step(22)
        val_to_write = attributes.HardwareVersion(17713)
        log.info(f"Attempting to write HardwareVersion to {val_to_write} (expect failure)")
        await self.verify_unsupported_write(attributes.HardwareVersion, val_to_write, self.endpoint)
        log.info("Verified UNSUPPORTED_WRITE for HardwareVersion")

        # Step 23
        self.step(23)
        val = await self.read_single_attribute_check_success(cluster=cluster, attribute=attributes.HardwareVersion, endpoint=self.endpoint)
        log.info(f"Read HardwareVersion again: {val}")
        asserts.assert_equal(val, hw_ver, "HardwareVersion changed after failed write")
        log.info("Verified HardwareVersion unchanged verification successful")

        # Step 24: HardwareVersionString
        self.step(24)
        hw_ver_str = await self.read_single_attribute_check_success(cluster=cluster, attribute=attributes.HardwareVersionString, endpoint=self.endpoint)
        log.info(f"Read HardwareVersionString: {hw_ver_str}")
        assert_is_string(hw_ver_str, "HardwareVersionString")
        assert_string_length(hw_ver_str, min_length=1, description="HardwareVersionString")
        assert_string_length(hw_ver_str, max_length=64, description="HardwareVersionString")

        # Step 25
        self.step(25)
        val_to_write = attributes.HardwareVersionString("newhard")
        log.info(f"Attempting to write HardwareVersionString to {val_to_write} (expect failure)")
        await self.verify_unsupported_write(attributes.HardwareVersionString, val_to_write, self.endpoint)
        log.info("Verified UNSUPPORTED_WRITE for HardwareVersionString")

        # Step 26: Verify HardwareVersionString unchanged
        self.step(26)
        val = await self.read_single_attribute_check_success(cluster=cluster, attribute=attributes.HardwareVersionString, endpoint=self.endpoint)
        log.info(f"Read HardwareVersionString again: {val}")
        asserts.assert_equal(val, hw_ver_str, "HardwareVersionString changed after failed write")
        log.info("Verified HardwareVersionString unchanged verification successful")

        # Step 27: SoftwareVersion
        self.step(27)
        sw_ver = await self.read_single_attribute_check_success(cluster=cluster, attribute=attributes.SoftwareVersion, endpoint=self.endpoint)
        log.info(f"Read SoftwareVersion: {sw_ver}")
        asserts.assert_greater_equal(sw_ver, 0, "SoftwareVersion can't be less than 0")
        asserts.assert_less_equal(sw_ver, 4294967294, "SoftwareVersion can't be greater than 4294967294")

        # Step 28: Write SoftwareVersion
        self.step(28)
        val_to_write = attributes.SoftwareVersion(33299)
        log.info(f"Attempting to write SoftwareVersion to {val_to_write} (expect failure)")
        await self.verify_unsupported_write(attributes.SoftwareVersion, val_to_write, self.endpoint)
        log.info("Verified UNSUPPORTED_WRITE for SoftwareVersion")

        # Step 29: Verify SoftwareVersion unchanged
        self.step(29)
        val = await self.read_single_attribute_check_success(cluster=cluster, attribute=attributes.SoftwareVersion, endpoint=self.endpoint)
        log.info(f"Read SoftwareVersion again: {val}")
        asserts.assert_equal(val, sw_ver, "SoftwareVersion changed after failed write")
        log.info("Verified SoftwareVersion unchanged verification successful")

        # Step 30: SoftwareVersionString
        self.step(30)
        sw_ver_str = await self.read_single_attribute_check_success(cluster=cluster, attribute=attributes.SoftwareVersionString, endpoint=self.endpoint)
        log.info(f"Read SoftwareVersionString: {sw_ver_str}")
        assert_is_string(sw_ver_str, "SoftwareVersionString")
        assert_string_length(sw_ver_str, min_length=1, description="SoftwareVersionString")
        assert_string_length(sw_ver_str, max_length=64, description="SoftwareVersionString")

        # Step 31: Write SoftwareVersionString
        self.step(31)
        val_to_write = attributes.SoftwareVersionString("1.0")
        log.info(f"Attempting to write SoftwareVersionString to {val_to_write} (expect failure)")
        await self.verify_unsupported_write(attributes.SoftwareVersionString, val_to_write, self.endpoint)
        log.info("Verified UNSUPPORTED_WRITE for SoftwareVersionString")

        # Step 32: Verify SoftwareVersionString unchanged
        self.step(32)
        val = await self.read_single_attribute_check_success(cluster=cluster, attribute=attributes.SoftwareVersionString, endpoint=self.endpoint)
        log.info(f"Read SoftwareVersionString again: {val}")
        asserts.assert_equal(val, sw_ver_str, "SoftwareVersionString changed after failed write")
        log.info("Verified SoftwareVersionString unchanged verification successful")

        # Step 33: ManufacturingDate
        self.step(33)
        mfg_date = await self.read_single_attribute_check_success(cluster=cluster, attribute=attributes.ManufacturingDate, endpoint=self.endpoint)
        log.info(f"Read ManufacturingDate: {mfg_date}")
        assert_is_string(mfg_date, "ManufacturingDate")
        assert_string_length(mfg_date, min_length=8, description="ManufacturingDate")
        assert_string_length(mfg_date, max_length=16, description="ManufacturingDate")
        try:
            datetime.datetime.strptime(mfg_date[:8], "%Y%m%d")
        except ValueError:
            asserts.fail(f"First 8 characters of ManufacturingDate '{mfg_date}' do not match the ISO 8601 format: YYYYMMDD")

        # Step 34: Write ManufacturingDate
        self.step(34)
        val_to_write = attributes.ManufacturingDate("20210814789452IN")
        log.info(f"Attempting to write ManufacturingDate to {val_to_write} (expect failure)")
        await self.verify_unsupported_write(attributes.ManufacturingDate, val_to_write, self.endpoint)
        log.info("Verified UNSUPPORTED_WRITE for ManufacturingDate")

        # Step 35: Verify ManufacturingDate unchanged
        self.step(35)
        val = await self.read_single_attribute_check_success(cluster=cluster, attribute=attributes.ManufacturingDate, endpoint=self.endpoint)
        log.info(f"Read ManufacturingDate again: {val}")
        asserts.assert_equal(val, mfg_date, "ManufacturingDate changed after failed write")
        log.info("Verified ManufacturingDate unchanged verification successful")

        # Step 36: PartNumber
        self.step(36)
        part_num = await self.read_single_attribute_check_success(cluster=cluster, attribute=attributes.PartNumber, endpoint=self.endpoint)
        log.info(f"Read PartNumber: {part_num}")
        assert_is_string(part_num, "PartNumber")
        assert_string_length(part_num, max_length=32, description="PartNumber")

        # Step 37: Write PartNumber
        self.step(37)
        val_to_write = attributes.PartNumber("newpart")
        log.info(f"Attempting to write PartNumber to {val_to_write} (expect failure)")
        await self.verify_unsupported_write(attributes.PartNumber, val_to_write, self.endpoint)
        log.info("Verified UNSUPPORTED_WRITE for PartNumber")

        # Step 38: Verify PartNumber unchanged
        self.step(38)
        val = await self.read_single_attribute_check_success(cluster=cluster, attribute=attributes.PartNumber, endpoint=self.endpoint)
        log.info(f"Read PartNumber again: {val}")
        asserts.assert_equal(val, part_num, "PartNumber changed after failed write")
        log.info("Verified PartNumber unchanged verification successful")

        # Step 39: ProductURL
        self.step(39)
        prod_url = await self.read_single_attribute_check_success(cluster=cluster, attribute=attributes.ProductURL, endpoint=self.endpoint)
        log.info(f"Read ProductURL: {prod_url}")
        assert_is_string(prod_url, "ProductURL should be a string")
        assert_string_length(prod_url, max_length=256, description="ProductURL max length is 256")

        # Verify RFC 3986 syntax and web page link if not empty
        if prod_url:
            try:
                result = urlparse(prod_url)
                if not all([result.scheme, result.netloc]):
                    asserts.fail(f"ProductURL '{prod_url}' is not a valid URL according to RFC 3986 (missing scheme or netloc)")
                if result.scheme not in ['http', 'https']:
                    asserts.fail(f"ProductURL '{prod_url}' should point to a web page (http/https)")
            except ValueError:
                asserts.fail(f"ProductURL '{prod_url}' could not be parsed as a URL")

        # Step 40: Write ProductURL
        self.step(40)
        val_to_write = attributes.ProductURL("https://www.example.com")
        log.info(f"Attempting to write ProductURL to {val_to_write} (expect failure)")
        await self.verify_unsupported_write(attributes.ProductURL, val_to_write, self.endpoint)
        log.info("Verified UNSUPPORTED_WRITE for ProductURL")

        # Step 41
        self.step(41)
        val = await self.read_single_attribute_check_success(cluster=cluster, attribute=attributes.ProductURL, endpoint=self.endpoint)
        log.info(f"Read ProductURL again: {val}")
        asserts.assert_equal(val, prod_url, "ProductURL changed after failed write")
        log.info("Verified ProductURL unchanged verification successful")

        # Step 42: ProductLabel
        self.step(42)
        prod_label = await self.read_single_attribute_check_success(cluster=cluster, attribute=attributes.ProductLabel, endpoint=self.endpoint)
        log.info(f"Read ProductLabel: {prod_label}")
        assert_is_string(prod_label, "ProductLabel")
        assert_string_length(prod_label, max_length=64, description="ProductLabel")
        # Verify that the ProductLabel does not contain the VendorName
        if vendor_name in prod_label:
            asserts.fail(f"ProductLabel '{prod_label}' must not contain VendorName '{vendor_name}'")

        # Step 43: Write ProductLabel
        self.step(43)
        val_to_write = attributes.ProductLabel("newproductlabel")
        log.info(f"Attempting to write ProductLabel to {val_to_write} (expect failure)")
        await self.verify_unsupported_write(attributes.ProductLabel, val_to_write, self.endpoint)
        log.info("Verified UNSUPPORTED_WRITE for ProductLabel")

        # Step 44
        self.step(44)
        val = await self.read_single_attribute_check_success(cluster=cluster, attribute=attributes.ProductLabel, endpoint=self.endpoint)
        log.info(f"Read ProductLabel again: {val}")
        asserts.assert_equal(val, prod_label, "ProductLabel changed after failed write")
        log.info("Verified ProductLabel unchanged verification successful")

        # Step 45: SerialNumber
        self.step(45)
        serial_num = await self.read_single_attribute_check_success(cluster=cluster, attribute=attributes.SerialNumber, endpoint=self.endpoint)
        log.info(f"Read SerialNumber: {serial_num}")
        assert_is_string(serial_num, "SerialNumber")
        assert_string_length(serial_num, max_length=32, description="SerialNumber")

        # Step 46: Write SerialNumber
        self.step(46)
        val_to_write = attributes.SerialNumber("newser")
        log.info(f"Attempting to write SerialNumber to {val_to_write} (expect failure)")
        await self.verify_unsupported_write(attributes.SerialNumber, val_to_write, self.endpoint)
        log.info("Verified UNSUPPORTED_WRITE for SerialNumber")

        # Step 47: Verify SerialNumber unchanged
        self.step(47)
        val = await self.read_single_attribute_check_success(cluster=cluster, attribute=attributes.SerialNumber, endpoint=self.endpoint)
        log.info(f"Read SerialNumber again: {val}")
        asserts.assert_equal(val, serial_num, "SerialNumber changed after failed write")
        log.info("Verified SerialNumber unchanged verification successful")

        # Step 48: LocalConfigDisabled
        self.step(48)
        log.info("Reading LocalConfigDisabled")
        local_config_disabled = await self.read_single_attribute_check_success(cluster=cluster, attribute=attributes.LocalConfigDisabled, endpoint=self.endpoint)
        log.info(f"Read LocalConfigDisabled: {local_config_disabled}")
        assert_valid_bool(local_config_disabled, "LocalConfigDisabled")

        # Step 49: Write LocalConfigDisabled (Should SUCCEED)
        self.step(49)
        log.info("Attempting to write LocalConfigDisabled to True")
        await self.write_single_attribute(attributes.LocalConfigDisabled(True), endpoint_id=self.endpoint)

        # Step 50: Verify LocalConfigDisabled
        self.step(50)
        val = await self.read_single_attribute_check_success(cluster=cluster, attribute=attributes.LocalConfigDisabled, endpoint=self.endpoint)
        log.info(f"Read LocalConfigDisabled: {val}")
        asserts.assert_equal(val, True, "LocalConfigDisabled should be True")
        log.info("Verified LocalConfigDisabled update successful")

        # Step 51: Reachable
        self.step(51)
        if self.check_pics("BINFO.S.A0011"):
            reachable = await self.read_single_attribute_check_success(cluster=cluster, attribute=attributes.Reachable, endpoint=self.endpoint)
            log.info(f"Read Reachable: {reachable}")
            assert_valid_bool(reachable, "Reachable")

            # Step 52a: Write Reachable = False (Expect UNSUPPORTED_WRITE)
            self.step("52a")
            val_to_write = attributes.Reachable(False)
            log.info(f"Attempting to write Reachable to {val_to_write} (expect failure)")
            await self.verify_unsupported_write(attributes.Reachable, val_to_write, self.endpoint)
            log.info("Verified UNSUPPORTED_WRITE for Reachable")

            # Step 52b: Verify Reachable unchanged
            self.step("52b")
            reachable_after = await self.read_single_attribute_check_success(cluster=cluster, attribute=attributes.Reachable, endpoint=self.endpoint)
            asserts.assert_equal(reachable, reachable_after, "Reachable should remain unchanged")
            log.info("Verified Reachable unchanged verification successful")
        else:
            log.info("Step 51 & 52: Skipping Reachable (BINFO.S.A0011) - Optional/PICS disabled")
            self.step("52a")
            self.step("52b")

        # Step 53: UniqueID
        self.step(53)
        if self.check_pics("BINFO.S.A0012"):
            uid = await self.read_single_attribute_check_success(cluster=cluster, attribute=attributes.UniqueID, endpoint=self.endpoint)
            log.info(f"Read UniqueID: {uid}")
            assert_is_string(uid, "UniqueID")
            assert_string_length(uid, max_length=32, description="UniqueID")

            # Step 54: Write UniqueID
            self.step(54)
            val_to_write = attributes.UniqueID("newid")
            log.info(f"Attempting to write UniqueID to {val_to_write} (expect failure)")
            await self.verify_unsupported_write(attributes.UniqueID, val_to_write, self.endpoint)
            log.info("Verified UNSUPPORTED_WRITE for UniqueID")

            # Step 55: Verify UniqueID unchanged
            self.step(55)
            val = await self.read_single_attribute_check_success(cluster=cluster, attribute=attributes.UniqueID, endpoint=self.endpoint)
            log.info(f"Read UniqueID again: {val}")
            asserts.assert_equal(val, uid, "UniqueID changed after failed write")
            log.info("Verified UniqueID unchanged verification successful")
        else:
            log.info("Step 53-55: Skipping UniqueID (BINFO.S.A0012) - Optional/PICS disabled")
            self.step(54)
            self.step(55)

        # Step 56: CapabilityMinima - CRITICAL VALIDATION STEP
        self.step(56)
        log.info("Reading and Validating CapabilityMinima")
        caps = await self.read_single_attribute_check_success(cluster=cluster, attribute=attributes.CapabilityMinima, endpoint=self.endpoint)
        log.info(f"Read CapabilityMinima: {caps}")

        # CaseSessionsPerFabric: min 3
        asserts.assert_true(hasattr(caps, "caseSessionsPerFabric"),
                            "caseSessionsPerFabric is not an attribute in CapabilityMinimaStruct")
        log.info(f"Checking CaseSessionsPerFabric: {caps.caseSessionsPerFabric} >= 3")
        asserts.assert_greater_equal(caps.caseSessionsPerFabric, 3, "CaseSessionsPerFabric >= 3")

        # SubscriptionsPerFabric: min 3
        asserts.assert_true(hasattr(caps, "subscriptionsPerFabric"),
                            "subscriptionsPerFabric is not an attribute in CapabilityMinimaStruct")
        log.info(f"Checking SubscriptionsPerFabric: {caps.subscriptionsPerFabric} >= 3")
        asserts.assert_greater_equal(caps.subscriptionsPerFabric, 3, "SubscriptionsPerFabric >= 3")

        # SimultaneousInvocationsSupported: min 1 (New field)
        asserts.assert_true(hasattr(caps, "simultaneousInvocationsSupported"),
                            "simultaneousInvocationsSupported is not an attribute in CapabilityMinimaStruct")
        log.info(f"Checking SimultaneousInvocationsSupported: {caps.simultaneousInvocationsSupported} >= 1")
        asserts.assert_greater_equal(caps.simultaneousInvocationsSupported, 1, "SimultaneousInvocationsSupported >= 1")

        # SimultaneousWritesSupported: min 1 (New field)
        asserts.assert_true(hasattr(caps, "simultaneousWritesSupported"),
                            "simultaneousWritesSupported is not an attribute in CapabilityMinimaStruct")
        log.info(f"Checking SimultaneousWritesSupported: {caps.simultaneousWritesSupported} >= 1")
        asserts.assert_greater_equal(caps.simultaneousWritesSupported, 1, "SimultaneousWritesSupported >= 1")

        # ReadPathsSupported: min 9 (New field)
        asserts.assert_true(hasattr(caps, "readPathsSupported"), "readPathsSupported is not an attribute in CapabilityMinimaStruct")
        log.info(f"Checking ReadPathsSupported: {caps.readPathsSupported} >= 9")
        asserts.assert_greater_equal(caps.readPathsSupported, 9, "ReadPathsSupported >= 9")

        # SubscribePathsSupported: min 3 (New field)
        asserts.assert_true(hasattr(caps, "subscribePathsSupported"),
                            "subscribePathsSupported is not an attribute in CapabilityMinimaStruct")
        log.info(f"Checking SubscribePathsSupported: {caps.subscribePathsSupported} >= 3")
        asserts.assert_greater_equal(caps.subscribePathsSupported, 3, "SubscribePathsSupported >= 3")

        # Step 57: Write CapabilityMinima (Existing fields)
        self.step(57)
        test_val = attributes.CapabilityMinima(cluster.Structs.CapabilityMinimaStruct(
            caseSessionsPerFabric=4, subscriptionsPerFabric=4))
        log.info(f"Attempting to write CapabilityMinima to {test_val} (expect failure)")
        await self.verify_unsupported_write(attributes.CapabilityMinima, test_val, self.endpoint)
        log.info("Verified UNSUPPORTED_WRITE for CapabilityMinima")

        # Step 58: Write CapabilityMinima (New fields)
        self.step(58)
        test_val_new = attributes.CapabilityMinima(cluster.Structs.CapabilityMinimaStruct(
            caseSessionsPerFabric=3, subscriptionsPerFabric=3, simultaneousInvocationsSupported=1, simultaneousWritesSupported=1))
        log.info(f"Attempting to write CapabilityMinima to {test_val_new} (expect failure)")
        await self.verify_unsupported_write(attributes.CapabilityMinima, test_val_new, self.endpoint)
        log.info("Verified UNSUPPORTED_WRITE for CapabilityMinima")

        # Step 59
        self.step(59)
        val = await self.read_single_attribute_check_success(cluster=cluster, attribute=attributes.CapabilityMinima, endpoint=self.endpoint)
        log.info(f"Read CapabilityMinima again: {val}")
        asserts.assert_equal(val, caps, "CapabilityMinima changed after failed write")
        log.info("Verified CapabilityMinima unchanged verification successful")

        # Step 60: ProductAppearance
        self.step(60)
        pa = await self.read_single_attribute_check_success(cluster=cluster, attribute=attributes.ProductAppearance, endpoint=self.endpoint)
        log.info(f"Read ProductAppearance: {pa}")

        # Step 61: Write ProductAppearance
        self.step(61)
        # Need to construct a valid ProductAppearanceStruct to attempt write
        # assuming arbitrary values for test
        pa_val = attributes.ProductAppearance(cluster.Structs.ProductAppearanceStruct(finish=3, primaryColor=4))
        log.info(f"Attempting to write ProductAppearance to {pa_val} (expect failure)")
        await self.verify_unsupported_write(attributes.ProductAppearance, pa_val, self.endpoint)
        log.info("Verified UNSUPPORTED_WRITE for ProductAppearance")

        # Step 62
        self.step(62)
        val = await self.read_single_attribute_check_success(cluster=cluster, attribute=attributes.ProductAppearance, endpoint=self.endpoint)
        log.info(f"Read ProductAppearance again: {val}")
        asserts.assert_equal(val, pa, "ProductAppearance changed after failed write")
        log.info("Verified ProductAppearance unchanged verification successful")

        # Step 63: SpecificationVersion
        self.step(63)
        spec_ver = await self.read_single_attribute_check_success(cluster=cluster, attribute=attributes.SpecificationVersion, endpoint=self.endpoint)
        log.info(f"Read SpecificationVersion: {spec_ver}")

        # Verify valid values
        allowed_versions = [0x01040000, 0x01040100, 0x01040200, 0x01050000]
        if spec_ver not in allowed_versions:
            asserts.fail(
                f"SpecificationVersion {spec_ver:#010x} is not one of the allowed values: {[hex(v) for v in allowed_versions]}")

        # Verify lower 8 bits are zero
        if spec_ver & 0xFF != 0:
            asserts.fail(f"SpecificationVersion {spec_ver:#010x} must have lower 8 bits set to zero")

        # Step 64
        self.step(64)
        val_to_write = attributes.SpecificationVersion(0x0103AAF1)
        log.info(f"Attempting to write SpecificationVersion to {val_to_write} (expect failure)")
        await self.verify_unsupported_write(attributes.SpecificationVersion, val_to_write, self.endpoint)
        log.info("Verified UNSUPPORTED_WRITE for SpecificationVersion")

        # Step 65
        self.step(65)
        val = await self.read_single_attribute_check_success(cluster=cluster, attribute=attributes.SpecificationVersion, endpoint=self.endpoint)
        log.info(f"Read SpecificationVersion again: {val}")
        asserts.assert_equal(val, spec_ver, "SpecificationVersion changed after failed write")
        log.info("Verified SpecificationVersion unchanged verification successful")

        # Step 66: MaxPathsPerInvoke
        self.step(66)
        mpi = await self.read_single_attribute_check_success(cluster=cluster, attribute=attributes.MaxPathsPerInvoke, endpoint=self.endpoint)
        log.info(f"Read MaxPathsPerInvoke: {mpi}")
        asserts.assert_greater_equal(mpi, 1, "MaxPathsPerInvoke min 1")
        asserts.assert_less_equal(mpi, 65535, "MaxPathsPerInvoke max 65535")

        # Step 67
        self.step(67)
        val_to_write = attributes.MaxPathsPerInvoke(12345)
        log.info(f"Attempting to write MaxPathsPerInvoke to {val_to_write} (expect failure)")
        await self.verify_unsupported_write(attributes.MaxPathsPerInvoke, val_to_write, self.endpoint)
        log.info("Verified UNSUPPORTED_WRITE for MaxPathsPerInvoke")

        # Step 68
        self.step(68)
        val = await self.read_single_attribute_check_success(cluster=cluster, attribute=attributes.MaxPathsPerInvoke, endpoint=self.endpoint)
        log.info(f"Read MaxPathsPerInvoke again: {val}")
        asserts.assert_equal(val, mpi, "MaxPathsPerInvoke changed after failed write")
        log.info("Verified MaxPathsPerInvoke unchanged verification successful")

        # Step 69: ConfigurationVersion
        self.step(69)
        config_version = await self.read_single_attribute_check_success(cluster=cluster, attribute=attributes.ConfigurationVersion, endpoint=self.endpoint)
        log.info(f"Read ConfigurationVersion: {config_version}")
        assert_valid_uint32(config_version, "ConfigurationVersion")
        asserts.assert_greater_equal(config_version, 1, "ConfigurationVersion min 1")

        # Step 70
        self.step(70)
        val_to_write = attributes.ConfigurationVersion(4388)
        log.info(f"Attempting to write ConfigurationVersion to {val_to_write} (expect failure)")
        await self.verify_unsupported_write(attributes.ConfigurationVersion, val_to_write, self.endpoint)
        log.info("Verified UNSUPPORTED_WRITE for ConfigurationVersion")

        # Step 71
        self.step(71)
        val = await self.read_single_attribute_check_success(cluster=cluster, attribute=attributes.ConfigurationVersion, endpoint=self.endpoint)
        log.info(f"Read ConfigurationVersion again: {val}")
        asserts.assert_equal(val, config_version, "ConfigurationVersion changed after failed write")
        log.info("Verified ConfigurationVersion unchanged verification successful")


if __name__ == "__main__":
    default_matter_test_main()
