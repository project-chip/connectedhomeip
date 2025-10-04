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


import logging

from mobly import asserts
from TC_SUTestBase import SoftwareUpdateBaseTest

import matter.clusters as Clusters
from matter.testing.event_attribute_reporting import AttributeSubscriptionHandler
from matter.testing.matter_testing import AttributeMatcher, TestStep, async_test_body, default_matter_test_main

logger = logging.getLogger(__name__)


class TC_SU_2_4(SoftwareUpdateBaseTest):
    "This test case verifies that the DUT behaves according to the spec when it is applying the software update."

    def desc_TC_SU_2_4(self) -> str:
        return "  [TC-SU-2.4] ApplyUpdateRequest Command from DUT to OTA-P"

    def pics_TC_SU_2_4(self):
        """Return the PICS definitions associated with this test."""
        pics = [
            "MCORE.OTA.Requestor",
        ]
        return pics

    def steps_TC_SU_2_4(self) -> list[TestStep]:
        steps = [
            TestStep(0, "Commissioning, already done", is_commissioning=True),
            TestStep(1, 'DUT sends a QueryImage command to the TH/OTA-P. TH/OTA-P sends a QueryImageResponse back to DUT. QueryStatus is set to "UpdateAvailable". Set ImageURI to the location where the image is located. After the DUT transfers the image, the DUT should send ApplyUpdateRequest to the OTA-P.', 'Verify that the request received on the OTA-P has the following mandatory fields.'
                     'UpdateToken - verify that it is same as the one sent in the QueryImageResponse.'
                     'NewVersion - verify that this is the same as the software version that was downloaded.')
        ]
        return steps

    @async_test_body
    async def teardown_test(self):
        self.current_provider_app_proc.terminate()
        return super().teardown_test()

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
        expected_software_version = 2
        ota_image_path = self.get_ota_image_path(version=expected_software_version)
        image_path = 'bdx://0000000000000141/' + ota_image_path
        await self.commission_provider(
            controller=controller,
            discriminator=provider_data['discriminator'],
            setup_pincode=provider_data['setup_pincode'],
            requestor_node_id=requestor_node_id,
            provider_node_id=provider_data['node_id'],
            version=expected_software_version,
            endpoint=0,
            log_file='/tmp/provider.log',
            extra_arguments=['--queryImageStatus', 'updateAvailable', '--imageUri', image_path]
        )
        update_state_attr_handler = AttributeSubscriptionHandler(
            expected_cluster=Clusters.OtaSoftwareUpdateRequestor,
            expected_attribute=Clusters.OtaSoftwareUpdateRequestor.Attributes.UpdateState
        )
        await update_state_attr_handler.start(dev_ctrl=controller, node_id=requestor_node_id, endpoint=0,
                                              fabric_filtered=False, min_interval_sec=0, max_interval_sec=5)
        await self.announce_ota_provider(controller, provider_data['node_id'], requestor_node_id)

        update_state_match = AttributeMatcher.from_callable(
            "Update state is applying",
            lambda report: report.value == Clusters.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum.kApplying)

        update_state_attr_handler.await_all_expected_report_matches([update_state_match], timeout_sec=60*6)
        # This value should be obtained from a response but for now is 32
        expected_token = '32'
        found_lines = self.current_provider_app_proc.read_from_logs(
            'Provider received ApplyUpdateRequest', regex=False, before=2, after=6)
        if len(found_lines) == 0:
            asserts.fail("No found lines while searching for the string 'Provider received ApplyUpdateRequest'")
        update_token_line = found_lines[0]['after'][0]
        handle_apply_request_line = found_lines[0]['after'][2]
        logger.info(f"Handle Apply Request {handle_apply_request_line} and token line {update_token_line}")
        version_string = f"version: {expected_software_version}"
        token_stirng = f"Update Token: {expected_token}"
        # Match for the expected strings
        asserts.assert_true(version_string in handle_apply_request_line, f"{version_string} is not in the expected line")
        asserts.assert_true(token_stirng in update_token_line, f"{expected_token} is not in the expected line")
        update_state_attr_handler.reset()
        await update_state_attr_handler.cancel()


if __name__ == "__main__":
    default_matter_test_main()
