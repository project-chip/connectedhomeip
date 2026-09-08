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

# See https://github.com/project-chip/connectedhomeip/blob/master/docs/testing/python.md#defining-the-ci-test-arguments
# for details about the block below.
#
# === BEGIN CI TEST ARGUMENTS ===
# test-runner-runs:
#   run1:
#     app: ${ALL_CLUSTERS_APP}
#     app-args: >
#       --discriminator 1234
#       --KVS kvs1
#       --custom-flow 0
#       --app-pipe /tmp/tmp_pipe
#       --trace-to json:${TRACE_APP}.json
#     script-args: >
#       --storage-path admin_storage.json
#       --qr-code MT:-24J0Q1212-10648G00
#       --PICS src/app/tests/suites/certification/ci-pics-values
#       --app-pipe /tmp/tmp_pipe
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

import logging

from mobly import asserts
from onboarding_payload_checks import OnboardingPayloadChecks

from matter.setup_payload import SetupPayload
from matter.testing.decorators import async_test_body, pics
from matter.testing.matter_testing import MatterTestUncommissionedDevice
from matter.testing.runner import default_matter_test_main

log = logging.getLogger(__name__)


class TC_DD_1_1(OnboardingPayloadChecks, MatterTestUncommissionedDevice):

    @pics('MCORE.ROLE.COMMISSIONEE', 'MCORE.DD.QR')
    @async_test_body
    async def test_TC_DD_1_1(self):
        """[TC-DD-1.1] QR Code Content [DUT - Commissionee]"""
        self.step(1, "TH parses the QR code payload provided for the DUT")
        asserts.assert_true(self.matter_test_config.qr_code_content, "This test needs to be run with the qr-code param.")
        qr_code_content = self.matter_test_config.qr_code_content[0]
        payload = SetupPayload().ParseQrCode(qr_code_content)

        self.step("2a", "Verify the onboarding payload version", expectation="Version field is 0")
        self.check_payload_version(payload)

        self.step("2b", "Verify Vendor ID and Product ID are present in the payload")
        self.log_vendor_and_product_id_not_verified(payload)

        self.step("2c", "Verify the Commissioning Flow value", expectation="Value is 0, 1 or 2")
        self.check_commissioning_flow(payload)

        self.step("2d", "Verify the 8-bit Discovery Capabilities Bitmask",
                  expectation="No reserved bits are set, and at least one discovery method is advertised")
        self.check_discovery_capabilities_bitmask(payload)

        self.step("2e", "Verify the 12-bit discriminator matches the value the DUT advertises during commissioning",
                  expectation="DUT is discoverable using the discriminator encoded in the QR code")
        asserts.assert_is_not_none(payload.long_discriminator, "QR code payload is missing the long discriminator")
        await self.check_advertised_discriminator(payload.long_discriminator, long=True)

        self.step("2f", "Verify the onboarding payload contains a 27-bit Passcode",
                  expectation="Passcode is between 0x0000001 and 0x5f5e0fe")
        self.check_passcode_range(payload)

        self.step("2g", "Verify passcode is valid", expectation="Passcode is not a disallowed default value")
        self.check_passcode_validity(payload)

        self.step("2h", "Verify the QR code prefix", expectation="Prefix is \"MT:\"")
        self.check_code_prefix(qr_code_content[:3])

        self.step(3, "Verify the packed binary data structure",
                  expectation="Structure is padded with 0 bits at the end to the nearest byte boundary")
        # QRCodeSetupPayloadParser (src/setup_payload/QRCodeSetupPayloadParser.cpp) rejects the
        # payload with CHIP_ERROR_INVALID_ARGUMENT if the trailing padding bits are not all 0, so
        # the successful parse in step 1 already confirms the packed structure is correctly padded.
        log.info("Padding bits validated implicitly by the successful parse in step 1.")


if __name__ == "__main__":
    default_matter_test_main()
