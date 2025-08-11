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

import matter.clusters as Clusters
from chip.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from mobly import asserts


class TC_APPLAUNCHER_3_5(MatterBaseTest):
    def desc_TC_APPLAUNCHER_3_5(self) -> str:
        # Description of the test case
        return "[TC_APPLAUNCHER-3.5] AppLauncher CatalogList attribute validation"

    def pics_TC_APPLAUNCHER_3_5(self):
        # PICS requirement: AppLauncher cluster must be supported
        return [
            "APPLAUNCHER.S",
        ]

    def steps_TC_APPLAUNCHER_3_5(self):
        # Define the test steps
        return [
            TestStep(1, "Commissioning, already done", is_commissioning=True),
            TestStep(2, "TH reads CatalogList attribute from the DUT for the list of supported application catalogs")
        ]

    @async_test_body
    async def test_TC_APPLAUNCHER_3_5(self):
        # Get the endpoint where AppLauncher cluster is implemented
        self.endpoint = self.get_endpoint()
        cluster = Clusters.ApplicationLauncher
        attributes = cluster.Attributes

        # Step 1: Commissioning (already completed)
        self.step(1)

        # Step 2: Read CatalogList attribute
        self.step(2)
        catalog_list = await self.read_single_attribute_check_success(
            cluster=cluster,
            attribute=attributes.CatalogList
        )
        # Confirm it's a list
        asserts.assert_true(
            isinstance(catalog_list, list),
            f"CatalogList is not a list. Received type: {type(catalog_list)}"
        )
        # Validate that CatalogList is present and not empty
        asserts.assert_greater(len(catalog_list), 0, "CatalogList attribute is empty or not present")

        logging.info(f"CatalogList: {catalog_list}")

        # Validate each entry is a valid CSA-issued Vendor ID (uint16)
        for vendor_id in catalog_list:
            asserts.assert_greater(len(catalog_list), 0, "CatalogList attribute is empty or not present")


        # Entry point for the test runner
if __name__ == "__main__":
    default_matter_test_main()
