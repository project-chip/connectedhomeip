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

# This test requires an all-clusters-app. Specify with --string-arg th_server_app_path:<path_to_app>
#
# Example invocation:
#   python src/python_testing/test_testing/TestCommissioningStatusDetectionIntegration.py \
#     --string-arg th_server_app_path:./out/linux-x64-all-clusters-no-ble/chip-all-clusters-app
#
# See https://github.com/project-chip/connectedhomeip/blob/master/docs/testing/python.md#defining-the-ci-test-arguments
# for details about the block below.
#
# === BEGIN CI TEST ARGUMENTS ===
# test-runner-runs:
#   run1:
#     app: ${ALL_CLUSTERS_APP}
#     app-args: --discriminator 1234 --passcode 20202021
#     script-args: >
#       --storage-path admin_storage.json
#       --string-arg th_server_app_path:${ALL_CLUSTERS_APP}
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

"""
Integration tests for commissioning status detection functions.

These tests verify that the commissioning status detection functions work correctly
with a real Matter application server.
"""

import logging
import os
import sys
import tempfile
from pathlib import Path
from time import sleep

from mobly import asserts

from matter import ChipDeviceCtrl
from matter.testing.apps import AppServerSubprocess
from matter.testing.commissioning import (
    _is_device_operational_via_dnssd,
    is_commissioned,
)
from matter.testing.matter_testing import MatterBaseTest, async_test_body, default_matter_test_main

# Add python_testing directory to path so mdns_discovery module is available
PYTHON_TESTING_DIR = Path(__file__).parent.parent
if str(PYTHON_TESTING_DIR) not in sys.path:
    sys.path.insert(0, str(PYTHON_TESTING_DIR))


class TestCommissioningStatusDetectionIntegration(MatterBaseTest):
    """
    Integration tests for commissioning status detection with real Matter apps.

    Follows TC_SC_3_5 pattern for server management and commissioning.
    """

    def setup_class(self):
        super().setup_class()

        self.th_server = None
        self.storage = None

        # Use fixed values like TC_SC_3_5
        self.th_server_local_nodeid = 1111
        self.th_server_discriminator = 1234
        self.th_server_passcode = 20202021

        # Get the path to the test app
        self.th_server_app = self.user_params.get("th_server_app_path", None)
        if not self.th_server_app:
            asserts.fail(
                "This test requires an all-clusters-app. "
                "Specify app path with --string-arg th_server_app_path:<path_to_app>"
            )
        if not os.path.exists(self.th_server_app):
            asserts.fail(f"The path {self.th_server_app} does not exist")

    def teardown_class(self):
        if self.th_server is not None:
            self.th_server.terminate()
        if self.storage is not None:
            self.storage.cleanup()
        super().teardown_class()

    def start_th_server(self):
        """Start the TH server app (following TC_SC_3_5 pattern)."""
        if self.th_server is not None:
            self.th_server.terminate()
        if self.storage is not None:
            self.storage.cleanup()

        # Expire any existing sessions for this node to ensure we start fresh
        # This prevents the controller from trying to use a stale session from a previous test run
        if self.default_controller:
            self.default_controller.ExpireSessions(self.th_server_local_nodeid)

        # Create a temporary storage directory for KVS files
        self.storage = tempfile.TemporaryDirectory(prefix=self.__class__.__name__)
        logging.info("Temporary storage directory: %s", self.storage.name)

        self.th_server = AppServerSubprocess(
            self.th_server_app,
            storage_dir=self.storage.name,
            discriminator=self.th_server_discriminator,
            passcode=self.th_server_passcode
        )

        self.th_server.start(
            expected_output="Server initialization complete",
            timeout=60
        )
        logging.info("TH server started successfully")

    @async_test_body
    async def test_TC_COMMISSION_DETECT_1_1_factory_fresh_dnssd(self):
        """
        Test that DNS-SD check returns False for factory-fresh device.
        """
        logging.info("=== Test: Factory Fresh Device - DNS-SD Check ===")
        self.start_th_server()

        logging.info("Checking DNS-SD for factory-fresh device")
        is_operational = await _is_device_operational_via_dnssd(
            self.default_controller,
            self.th_server_local_nodeid
        )
        asserts.assert_false(
            is_operational,
            "Factory-fresh device should NOT be found operational via DNS-SD"
        )
        logging.info("PASS: Factory-fresh device not found operational via DNS-SD")

    @async_test_body
    async def test_TC_COMMISSION_DETECT_1_2_factory_fresh_is_commissioned(self):
        """
        Test that is_commissioned() returns False for factory-fresh device.
        """
        logging.info("=== Test: Factory Fresh Device - is_commissioned() ===")
        self.start_th_server()

        pase_params = {
            'discriminator': self.th_server_discriminator,
            'passcode': self.th_server_passcode
        }

        logging.info("Checking is_commissioned() on factory-fresh device")
        commissioned = await is_commissioned(
            self.default_controller,
            self.th_server_local_nodeid,
            pase_params=pase_params
        )
        asserts.assert_false(
            commissioned,
            "Factory-fresh device should report is_commissioned=False"
        )
        logging.info("PASS: Factory-fresh device reports is_commissioned=False")

    @async_test_body
    async def test_TC_COMMISSION_DETECT_1_3_commissioned_dnssd(self):
        """
        Test that DNS-SD finds a commissioned device as operational.
        """
        logging.info("=== Test: Commissioned Device - DNS-SD Check ===")
        self.start_th_server()

        # Commission device (same pattern as TC_SC_3_5)
        logging.info("Commissioning TH server to TH fabric")
        await self.default_controller.CommissionOnNetwork(
            nodeId=self.th_server_local_nodeid,
            setupPinCode=self.th_server_passcode,
            filterType=ChipDeviceCtrl.DiscoveryFilterType.LONG_DISCRIMINATOR,
            filter=self.th_server_discriminator
        )
        logging.info("Commissioning TH_SERVER complete")

        # Wait for mDNS advertisement to propagate (using time.sleep like TC_SC_3_5)
        sleep(2)

        logging.info("Checking DNS-SD for commissioned device")

        # Retry a few times for flaky mDNS
        is_operational = False
        for attempt in range(3):
            is_operational = await _is_device_operational_via_dnssd(
                self.default_controller,
                self.th_server_local_nodeid
            )
            if is_operational:
                break
            logging.info(f"DNS-SD check attempt {attempt + 1} returned False, retrying...")
            sleep(2)

        asserts.assert_true(
            is_operational,
            "Commissioned device SHOULD be found operational via DNS-SD"
        )
        logging.info("PASS: Commissioned device found operational via DNS-SD")


if __name__ == "__main__":
    default_matter_test_main()
