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
#     app: ${OTA_REQUESTOR_APP}
#     app-args: >
#       --discriminator 123
#       --passcode 2123
#       --secured-device-port 5540
#       --KVS /tmp/chip_kvs_requestor
#       --autoApplyImage
#       --trace-to json:${TRACE_APP}.json
#     script-args: >
#       --storage-path admin_storage.json
#       --commissioning-method on-network
#       --discriminator 123
#       --passcode 2123
#       --endpoint 0
#       --nodeId 123
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

import asyncio
import logging
from subprocess import run
from time import sleep

from mobly import asserts
from TC_SUBase import SoftwareUpdateBaseTest

import matter.clusters as Clusters
from matter import ChipDeviceCtrl
from matter.clusters.Types import NullValue
from matter.testing.event_attribute_reporting import EventSubscriptionHandler
from matter.testing.matter_testing import TestStep, async_test_body, default_matter_test_main

logger = logging.getLogger(__name__)


class TC_SU_2_5(SoftwareUpdateBaseTest):
    "This test case verifies that the DUT behaves according to the spec when it is applying the software update."

    def desc_TC_SU_2_5(self) -> str:
        return "  [TC-SU-2.4] ApplyUpdateRequest Command from DUT to OTA-P"

    def pics_TC_SU_2_5(self):
        """Return the PICS definitions associated with this test."""
        pics = [
            "MCORE.OTA.Requestor",
        ]
        return pics

    def steps_TC_SU_2_5(self) -> list[TestStep]:
        steps = [
            TestStep(0, "Commissioning, already done", is_commissioning=True),
            TestStep(1, 'DUT sends a QueryImage command to the TH/OTA-P. TH/OTA-P sends a QueryImageResponse back to DUT. QueryStatus is set to "UpdateAvailable". Set ImageURI to the location where the image is located. After the DUT transfers the image, the DUT should send ApplyUpdateRequest to the OTA-P.', 'Verify that the request received on the OTA-P has the following mandatory fields.'
                     'UpdateToken - verify that it is same as the one sent in the QueryImageResponse.'
                     'NewVersion - verify that this is the same as the software version that was downloaded.'),
        ]
        return steps

    @async_test_body
    async def test_TC_SU_2_4(self):

        self.step(0)
        controller = self.default_controller
        requestor_node_id = self.dut_node_id  # 123 with discriminator 123

        provider_data = {
            "node_id": 321,
            "discriminator": 321,
            "setup_pincode": 2321
        }

        self.step(1)
        self.commission_provider(
            controller=controller,
            requestor_node_id=requestor_node_id,
            provider_node_id=provider_data['node_id'],
            version=2,
            endpoint=0,
            log_file='/tmp/provider.log',
            extra_arguments=['--queryImageStatus updateAvailable']
        )
        await self.announce_ota_provider(controller, provider_data['node_id'], requestor_node_id)
        found_lines = self.current_provider_app_proc.read_log('ApplyUpdateRequest', regex=False, before=2, after=6)
        if len(found_lines) == 0:
            asserts.fail("No found lines while searching for the string 'ApplyUpdateRequest'")


if __name__ == "__main__":
    default_matter_test_main()
