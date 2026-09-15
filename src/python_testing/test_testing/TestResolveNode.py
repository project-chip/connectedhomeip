#
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

# This test requires an all-clusters-app. Specify with --string-arg th_server_app_path:<path_to_app>
#
# Example invocation:
#   python src/python_testing/test_testing/TestResolveNode.py --no-wildcard-subscription \
#     --string-arg th_server_app_path:./out/linux-x64-all-clusters-no-ble/chip-all-clusters-app
#
# See https://github.com/project-chip/connectedhomeip/blob/master/docs/testing/python.md#defining-the-ci-test-arguments
# for details about the block below.
#
# === BEGIN CI TEST ARGUMENTS ===
# test-runner-runs:
#   run1:
#     script-args: >
#       --storage-path admin_storage.json
#       --no-wildcard-subscription
#       --string-arg th_server_app_path:${ALL_CLUSTERS_APP}
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

"""
Integration tests for the controller's ResolveNode query.

ResolveNode answers "is this node advertising on my fabric" through the controller's own
DNS-SD resolver without opening a session. It is a prior, not proof: the answer can come
from the resolver's cache for a while after a device goes away. A CASE session is what
settles it. The tests below pin down exactly that.

Tests are numbered because they build on each other's DNS-SD state within one process:
the factory-fresh check has to run before any device has advertised on this fabric. The
node ID is random per run so a cached advertisement from an earlier run cannot match.
"""

import asyncio
import logging
import os
import random
import tempfile

from mobly import asserts

import matter.clusters as Clusters
from matter import ChipDeviceCtrl
from matter.exceptions import ChipStackError
from matter.testing.apps import AppServerSubprocess
from matter.testing.decorators import async_test_body
from matter.testing.matter_testing import MatterTestCommissioner
from matter.testing.runner import default_matter_test_main

LOGGER = logging.getLogger(__name__)

RESOLVE_TIMEOUT_MS = 2000


class TestResolveNode(MatterTestCommissioner):

    def setup_class(self):
        super().setup_class()

        self.th_server = None
        self.storage = None
        self.th_server_local_nodeid = random.randint(0x1000, 0xFFFF_FFFF)
        LOGGER.info("TH server node ID for this run: 0x%X", self.th_server_local_nodeid)
        self.th_server_discriminator = 1234
        self.th_server_passcode = 20202021

        self.th_server_app = self.user_params.get("th_server_app_path", None)
        if not self.th_server_app:
            asserts.fail(
                "This test requires an all-clusters-app. "
                "Specify app path with --string-arg th_server_app_path:<path_to_app>"
            )
        if not os.path.exists(self.th_server_app):
            asserts.fail(f"The path {self.th_server_app} does not exist")

    def teardown_class(self):
        self.stop_th_server()
        super().teardown_class()

    def start_th_server(self):
        """Start a factory-fresh TH server app on a temporary KVS."""
        self.stop_th_server()

        # Drop any session left over from a previous test so the controller starts clean.
        self.default_controller.ExpireSessions(self.th_server_local_nodeid)

        self.storage = tempfile.TemporaryDirectory(prefix=self.__class__.__name__)
        self.th_server = AppServerSubprocess(
            self.th_server_app,
            storage_dir=self.storage.name,
            discriminator=self.th_server_discriminator,
            passcode=self.th_server_passcode
        )
        self.th_server.start(expected_output="Server initialization complete", timeout=60)

    def stop_th_server(self):
        if self.th_server is not None:
            self.th_server.terminate()
            self.th_server = None
        if self.storage is not None:
            self.storage.cleanup()
            self.storage = None

    async def commission_th_server(self):
        await self.default_controller.CommissionOnNetwork(
            nodeId=self.th_server_local_nodeid,
            setupPinCode=self.th_server_passcode,
            filterType=ChipDeviceCtrl.DiscoveryFilterType.LONG_DISCRIMINATOR,
            filter=self.th_server_discriminator
        )

    @async_test_body
    async def test_1_factory_fresh_device_does_not_resolve(self):
        """A device that is not on our fabric does not resolve."""
        self.start_th_server()

        resolved = await self.default_controller.ResolveNode(self.th_server_local_nodeid, RESOLVE_TIMEOUT_MS)
        asserts.assert_is_none(resolved, "Factory-fresh device must not resolve on our fabric")

    @async_test_body
    async def test_2_commissioned_device_resolves_and_has_record(self):
        """After commissioning the node resolves on our fabric, concurrently and consistently."""
        self.start_th_server()
        await self.commission_th_server()

        # A read establishes CASE so the session's address is known for the parity check below.
        await self.read_single_attribute_check_success(
            cluster=Clusters.BasicInformation, attribute=Clusters.BasicInformation.Attributes.VendorID,
            node_id=self.th_server_local_nodeid, endpoint=0)

        # Two lookups for the same node at once must both complete with the same answer.
        resolved, resolved_again = await asyncio.gather(
            self.default_controller.ResolveNode(self.th_server_local_nodeid, RESOLVE_TIMEOUT_MS),
            self.default_controller.ResolveNode(self.th_server_local_nodeid, RESOLVE_TIMEOUT_MS))
        asserts.assert_is_not_none(resolved, "Commissioned device must resolve on our fabric")
        asserts.assert_equal(resolved, resolved_again, "Concurrent lookups must agree")
        address, port = resolved
        asserts.assert_true(len(address) > 0, "Resolved address must not be empty")
        asserts.assert_greater(port, 0, "Resolved port must be set")
        asserts.assert_equal(
            self.default_controller.GetAddressAndPort(self.th_server_local_nodeid), (address, port),
            "Resolve must report the same address and port the session used")

    @async_test_body
    async def test_3_stopped_device_may_resolve_from_cache_but_case_fails(self):
        """A stopped device may still resolve from the DNS-SD cache; a session attempt is what fails."""
        self.start_th_server()
        await self.commission_th_server()
        await self.read_single_attribute_check_success(
            cluster=Clusters.BasicInformation, attribute=Clusters.BasicInformation.Attributes.VendorID,
            node_id=self.th_server_local_nodeid, endpoint=0)

        self.stop_th_server()
        self.default_controller.ExpireSessions(self.th_server_local_nodeid)

        # Cache dependent, so not asserted either way; logged for the record.
        resolved = await self.default_controller.ResolveNode(self.th_server_local_nodeid, RESOLVE_TIMEOUT_MS)
        LOGGER.info("Resolve after stop (cache dependent): %s", resolved)
        with asserts.assert_raises((ChipStackError, asyncio.TimeoutError), "CASE to a stopped device must fail"):
            await self.default_controller.GetConnectedDevice(
                self.th_server_local_nodeid, allowPASE=False, timeoutMs=RESOLVE_TIMEOUT_MS)

    @async_test_body
    async def test_4_invalid_inputs_are_rejected(self):
        """Non-operational node IDs and non-positive timeouts are rejected before any lookup starts."""
        with asserts.assert_raises(ChipStackError, "Node ID 0 is not operational"):
            await self.default_controller.ResolveNode(0, RESOLVE_TIMEOUT_MS)
        with asserts.assert_raises(ValueError, "A zero timeout is rejected"):
            await self.default_controller.ResolveNode(self.th_server_local_nodeid, 0)


if __name__ == "__main__":
    default_matter_test_main()
