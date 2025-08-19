# Copyright (c) 2025 Project CHIP Authors
# Licensed under the Apache License, Version 2.0

# === BEGIN CI TEST ARGUMENTS ===
# test-runner-runs:
#   run1:
#     app: ${TV_APP}
#     app-args: --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json
#     script-args: >
#       --storage-path admin_storage.json
#       --commissioning-method on-network
#       --discriminator 1234
#       --passcode 20202021
#       --endpoint 1
# === END CI TEST ARGUMENTS ===

import logging

from mobly import asserts

import matter.clusters as Clusters
from matter.testing.matter_asserts import assert_valid_uint16
from matter.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main


class TC_APPLAUNCHER_3_5(MatterBaseTest):
    def desc_TC_APPLAUNCHER_3_5(self) -> str:
        return "[TC_APPLAUNCHER-3.5] AppLauncher CatalogList attribute validation"

    def pics_TC_APPLAUNCHER_3_5(self):
        return [
            "APPLAUNCHER.S",
        ]

    def steps_TC_APPLAUNCHER_3_5(self):
        return [
            TestStep("precondition", "DUT commissioned and preconditions", is_commissioning=True),
            TestStep(1, "TH reads CatalogList attribute from the DUT for the list of supported application catalogs")
        ]

    @async_test_body
    async def test_TC_APPLAUNCHER_3_5(self):
        self.endpoint = self.get_endpoint()
        cluster = Clusters.ApplicationLauncher
        attributes = cluster.Attributes

        # Precondition: Commissioning
        self.step("precondition")

        # Step 1: Read CatalogList attribute
        self.step(1)
        catalog_list = await self.read_single_attribute_check_success(
            cluster=cluster,
            attribute=attributes.CatalogList
        )

        # Confirm it's a list
        asserts.assert_is_instance(
            catalog_list, list,
            f"CatalogList is not a list. Received type: {type(catalog_list)}"
        )

        # Validate that CatalogList is present and not empty
        asserts.assert_greater(len(catalog_list), 0, "CatalogList attribute is empty or not present")

        logging.info(f"CatalogList: {catalog_list}")

        # Validate each entry is a valid CSA-issued Vendor ID (uint16)
        for vendor_id in catalog_list:
            assert_valid_uint16(vendor_id, f"Invalid Vendor ID in CatalogList: {vendor_id}")


if __name__ == "__main__":
    default_matter_test_main()
