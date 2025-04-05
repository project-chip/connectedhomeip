#!/usr/bin/env -S python3 -B
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
#    This script is intended for QA use only and does not automatically
#    verify the results of the commissioning process. Please use with caution.
#
# [TC-DA-1.9] Device Attestation Revocation [DUT-Commissioner]
#
#  This test is about commissioning a device configured with a revoked DAC and/or PAI
#  and checking if the commissioner warns about the device attestation error.
#  The test case will be successful if the commissioner warns about the device
#  attestation error and the device is not commissioned.

# See https://github.com/project-chip/connectedhomeip/blob/master/docs/testing/python.md#defining-the-ci-test-arguments
# for details about the block below.
#
# === BEGIN CI TEST ARGUMENTS ===
# test-runner-runs:
#   run1:
#     script-args: >
#       --storage-path admin_storage.json
#       --string-arg app_path:out/linux-x64-all-clusters-ipv6only-no-ble-no-wifi-tsan-clang-test/chip-all-clusters-app
#       --string-arg dac_provider_base_path:credentials/test/revoked-attestation-certificates/dac-provider-test-vectors
#       --string-arg revocation_set_base_path:credentials/test/revoked-attestation-certificates/dac-provider-test-vectors/revocation-sets
#       --string-arg app_log_path:/tmp/TC_DA_1_9
#       --bool-arg is_ci:true
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

import os
import signal
import subprocess

from chip.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from mobly import asserts


class TC_DA_1_9(MatterBaseTest):
    def setup_class(self):
        super().setup_class()
        self.dac_provider_base_path = self.matter_test_config.global_test_params.get('dac_provider_base_path')
        self.revocation_set_base_path = self.matter_test_config.global_test_params.get('revocation_set_base_path')
        self.app_path = self.matter_test_config.global_test_params.get('app_path')
        self.app_log_path = self.matter_test_config.global_test_params.get('app_log_path')
        self.is_ci = self.matter_test_config.global_test_params.get('is_ci')

    def desc_TC_DA_1_9(self) -> str:
        return "[TC-DA-1.9] Device Attestation Revocation [DUT-Commissioner]"

    def pics_TC_DA_1_9(self) -> list[str]:
        pics = [
            "MCORE.ROLE.COMMISSIONER",
        ]
        return pics

    def steps_TC_DA_1_9(self) -> list[TestStep]:
        return [
            TestStep(1, "Test commissioning with revoked DAC",
                     "(DUT)Commissioner warns about commissioning the non-genuine device, Or Commissioning fails with device attestation error 302"),
            TestStep(2, "Test commissioning with revoked PAI",
                     "(DUT)Commissioner warns about commissioning the non-genuine device, Or Commissioning fails with device attestation error 202"),
            TestStep(3, "Test commissioning with both DAC and PAI revoked",
                     "(DUT)Commissioner warns about commissioning the non-genuine device, Or Commissioning fails with device attestation error 208"),
            TestStep(4, "Test commissioning with revoked DAC using delegated CRL signer",
                     "(DUT)Commissioner warns about commissioning the non-genuine device, Or Commissioning fails with device attestation error 302"),
            TestStep(5, "Test commissioning with revoked PAI using delegated CRL signer",
                     "(DUT)Commissioner warns about commissioning the non-genuine device, Or Commissioning fails with device attestation error 202"),
            TestStep(6, "Test commissioning with both DAC and PAI revoked using delegated CRL signer",
                     "(DUT)Commissioner warns about commissioning the non-genuine device, Or Commissioning fails with device attestation error 208"),
            TestStep(7, "Test commissioning with valid DAC and PAI",
                     "Commissioning succeeds without any attestation errors"),
        ]

    @async_test_body
    async def test_TC_DA_1_9(self):
        test_vectors = [
            {
                'name': 'tc_dac_revoked',
                'dac_provider': 'revoked-dac-01.json',
                'revocation_set': 'revocation-set.json',
                'expects_commissioning_success': False,
                'expects_att_err': 302
            },
            {
                'name': 'tc_pai_revoked',
                'dac_provider': 'revoked-pai.json',
                'revocation_set': 'revocation-set.json',
                'expects_commissioning_success': False,
                'expects_att_err': 202
            },
            {
                'name': 'tc_dac_and_pai_revoked',
                'dac_provider': 'revoked-dac-and-pai.json',
                'revocation_set': 'revocation-set.json',
                'expects_commissioning_success': False,
                'expects_att_err': 208
            },
            {
                'name': 'tc_dac_revoked_using_delegated_crl_signer',
                'dac_provider': 'indirect-revoked-dac-01.json',
                'revocation_set': 'revocation-set.json',
                'expects_commissioning_success': False,
                'expects_att_err': 302
            },
            {
                'name': 'tc_pai_revoked_using_delegated_crl_signer',
                'dac_provider': 'indirect-revoked-pai-03.json',
                'revocation_set': 'revocation-set.json',
                'expects_commissioning_success': False,
                'expects_att_err': 202
            },
            {
                'name': 'tc_dac_and_pai_revoked_using_delegated_crl_signer',
                'dac_provider': 'indirect-revoked-dac-01-pai-03.json',
                'revocation_set': 'revocation-set.json',
                'expects_commissioning_success': False,
                'expects_att_err': 208
            },
            {
                'name': 'tc_dac_and_pai_valid',
                'dac_provider': None,
                'revocation_set': 'revocation-set.json',
                'expects_commissioning_success': True,
                'expects_att_err': None
            }
        ]

        for idx, test_case in enumerate(test_vectors):
            self.step(idx + 1)

            # Clean up any existing KVS files
            subprocess.call("rm -f /tmp/tmpkvs*", shell=True)

            # Create log files for this test case
            os.makedirs(self.app_log_path, exist_ok=True)
            app_log_file_name = f"{self.app_log_path}/{test_case['name']}_app.log"

            with open(app_log_file_name, 'w') as app_log_file:
                # Start the all-clusters-app with appropriate DAC provider
                app_args = '--trace_decode 1 --KVS /tmp/tmpkvs'
                if test_case['dac_provider']:
                    dac_provider_path = os.path.join(self.dac_provider_base_path, test_case['dac_provider'])
                    app_args += f' --dac_provider {dac_provider_path}'

                app_cmd = f"{self.app_path} {app_args}"

                # Run the all-clusters-app in background
                app_process = subprocess.Popen(app_cmd.split(), stdout=app_log_file, stderr=app_log_file)

                # Prompt user with instructions
                prompt_msg = (
                    f"\nPlease commission the DUT with:\n"
                    f"  QR Code: 'MT:-24J0AFN00KA0648G00'\n"
                    f"  Revocation Set: {os.path.join(self.revocation_set_base_path, test_case['revocation_set'])}\n\n"
                    f"Input 'Y' if DUT successfully commissions without any warnings\n"
                    f"Input 'N' if commissioner warns about commissioning the non-genuine device, "
                    f"Or Commissioning fails with device attestation error {test_case['expects_att_err']}\n"
                )

                # TODO: run the chip-tool and commission the TH, parse the output and check if it matches the expected results
                if self.is_ci:
                    resp = 'Y' if test_case['expects_commissioning_success'] else 'N'
                else:
                    resp = self.wait_for_user_input(prompt_msg)

                commissioning_success = resp.lower() == 'y'

                # Verify results
                asserts.assert_equal(
                    commissioning_success,
                    test_case['expects_commissioning_success'],
                    f"Commissioning {'succeeded' if commissioning_success else 'failed'} when it should have {'succeeded' if test_case['expects_commissioning_success'] else 'failed'}"
                )

                app_process.send_signal(signal.SIGINT.value)
                app_process.wait()


if __name__ == "__main__":
    default_matter_test_main()
