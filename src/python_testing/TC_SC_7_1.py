#
#    Copyright (c) 2022 Project CHIP Authors
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
# test-runner-runs: run1
# test-runner-run/run1/app: ${ALL_CLUSTERS_APP}
# test-runner-run/run1/factoryreset: True
# test-runner-run/run1/quiet: True
# test-runner-run/run1/app-args: --discriminator 2222 --KVS kvs1 --trace-to json:${TRACE_APP}.json
# test-runner-run/run1/script-args: --storage-path admin_storage.json --bool-arg post_cert_test:true  --qr-code MT:-24J0KCZ16750648G00 --trace-to json:${TRACE_TEST_JSON}.json --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
# === END CI TEST ARGUMENTS ===

# Note that in the CI we are using the post-cert test as we can only start one app from the current script.
# This should still be fine as this test has unit tests for other conditions. See test_TC_SC_7_1.py
import logging

import chip.clusters as Clusters
from matter_testing_support import MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from mobly import asserts


def _trusted_root_test_step(dut_num: int) -> TestStep:
    read_trusted_roots_over_pase = f'TH establishes a PASE session to DUT{dut_num} using the provided setup code and reads the TrustedRootCertificates attribute from the operational credentials cluster over PASE'
    return TestStep(dut_num, read_trusted_roots_over_pase, "List should be empty as the DUT should be in factory reset ")


class TC_SC_7_1(MatterBaseTest):
    ''' TC-SC-7.1

        This test requires two instances of the DUT with the same PID/VID to confirm that the individual
        devices are provisioned with different discriminators and PAKE salts in the same product line.

        This test MUST be run on a factory reset device, over PASE, with no commissioned fabrics.
    '''

    def __init__(self, *args):
        super().__init__(*args)
        self.post_cert_test = False

    def setup_class(self):
        super().setup_class()
        self.post_cert_test = self.user_params.get("post_cert_test", False)

    def expected_number_of_DUTs(self) -> int:
        return 1 if self.post_cert_test else 2

    def steps_TC_SC_7_1(self):
        if self.post_cert_test:
            return [_trusted_root_test_step(1),
                    TestStep(2, "TH extracts the discriminator from the provided setup code", "Ensure the code is not the default")]

        return [_trusted_root_test_step(1),
                _trusted_root_test_step(2),
                TestStep(3, "TH compares the discriminators from the provided setup codes", "Discriminators do not match")]

    # TODO: Need a pics or something to limit this to devices that have a factory-provided matter setup code (as opposed to a field upgradable device / device with a custom commissioning where this test won't apply)

    @async_test_body
    async def test_TC_SC_7_1(self):
        # For now, this test is WAY easier if we just ask for the setup code instead of discriminator / passcode
        asserts.assert_false(self.matter_test_config.discriminators,
                             "This test needs to be run with either the QR or manual setup code. The QR code is preferred.")

        if len(self.matter_test_config.qr_code_content + self.matter_test_config.manual_code) != self.expected_number_of_DUTs():
            if self.post_cert_test:
                msg = "The post_cert_test flag is only for use post-certification. When using this flag, specify a single discriminator, manual-code or qr-code-content"
            else:
                msg = "This test requires two devices for use at certification. Specify two device discriminators or QR codes ex. --discriminator 1234 5678"
            asserts.fail(msg)

        # Make sure these are no fabrics on the device so we know we're looking at the factory discriminator. This also ensures that the provided codes are correct.
        for i, setup_code in enumerate(self.matter_test_config.qr_code_content + self.matter_test_config.manual_code):
            self.step(i+1)
            await self.default_controller.FindOrEstablishPASESession(setupCode=setup_code, nodeid=i+1)
            root_certs = await self.read_single_attribute_check_success(node_id=i+1, cluster=Clusters.OperationalCredentials, attribute=Clusters.OperationalCredentials.Attributes.TrustedRootCertificates, endpoint=0)
            asserts.assert_equal(
                root_certs, [], "Root certificates found on device. Device must be factory reset before running this test.")

        self.step(i+2)
        setup_payload_info = self.get_setup_payload_info()
        if self.post_cert_test:
            # For post-cert, we're testing against the defaults
            # TODO: Does it even make sense to test against a manual code in post-cert? It's such a small space, collisions are likely. Should we restrict post-cert to QR? What if one isn't provided?
            asserts.assert_not_equal(setup_payload_info[0].filter_value, 3840, "Device is using the default discriminator")
        else:
            if setup_payload_info[0].filter_value == setup_payload_info[1].filter_value and self.matter_test_config.manual_code is not None:
                logging.warn("The two provided discriminators are the same. Note that this CAN occur by chance, especially when using manual codes with the short discriminator. Consider using a QR code, or a different device if you believe the DUTs have individually provisioned")
            asserts.assert_not_equal(
                setup_payload_info[0].filter_value, setup_payload_info[1].filter_value, "Devices are using the same discriminator values")

        # TODO: add test for PAKE salt. This needs to be plumbed through starting from HandlePBKDFParamResponse.
        # Will handle in a separate follow up as the plumbing here is aggressive and through some of the crypto layers.
        # TODO: Other unit-specific values?


if __name__ == "__main__":
    default_matter_test_main()
