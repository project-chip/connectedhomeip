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
#       --PICS src/app/tests/suites/certification/ci-pics-values
#       --string-arg app_path:${ALL_CLUSTERS_APP}
#       --string-arg dac_provider_base_path:credentials/test/revoked-attestation-certificates/dac-provider-test-vectors
#       --string-arg revocation_set_base_path:credentials/test/revoked-attestation-certificates/revocation-sets
#       --string-arg app_log_path:/tmp/TC_DA_1_9
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

import os
import signal
import subprocess

from chip import ChipDeviceCtrl
from chip.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from mobly import asserts


class TC_DA_1_9(MatterBaseTest):
    def setup_class(self):
        super().setup_class()

        self.app_path = self.matter_test_config.global_test_params.get('app_path')
        self.dac_provider_base_path = self.matter_test_config.global_test_params.get('dac_provider_base_path')
        self.revocation_set_base_path = self.matter_test_config.global_test_params.get('revocation_set_base_path')

        # Set ROOT_DIR and default app path
        # if we are running in CI, we expect the app to be in the SDK root
        # else we expect the app is running in chip-cert-bin docker image
        # same for dac_provider_base_path and revocation_set_base_path
        if self.is_pics_sdk_ci_only:
            ROOT_DIR = os.path.abspath(os.path.join(os.path.dirname(__file__), '../..'))
            DEFAULT_APP_PATH = os.path.join(
                ROOT_DIR, "objdir-clone/linux-x64-all-clusters-ipv6only-no-ble-no-wifi-tsan-clang-test/chip-all-clusters-app")
        else:
            ROOT_DIR = "/root"
            DEFAULT_APP_PATH = os.path.join(ROOT_DIR, "apps/chip-all-clusters-app")

        if self.app_path is None:
            self.app_path = DEFAULT_APP_PATH

        if self.dac_provider_base_path is None:
            self.dac_provider_base_path = os.path.join(
                ROOT_DIR, "credentials/test/revoked-attestation-certificates/dac-provider-test-vectors")

        if self.revocation_set_base_path is None:
            self.revocation_set_base_path = os.path.join(
                ROOT_DIR, "credentials/test/revoked-attestation-certificates/revocation-sets")

        # Check if paths that we expect to exist do exist
        if self.app_path is None or not os.path.exists(self.app_path):
            asserts.fail("--string-arg app_path:<app_path> is required for this test, please provide the path to the app (eg: all-clusters-app)")

        if self.dac_provider_base_path is None or not os.path.exists(self.dac_provider_base_path):
            asserts.fail("--string-arg dac_provider_base_path:<dac_provider_base_path> is required for this test, please provide the path to the dac provider test vectors, it can be found in Matter SDK at: credentials/test/revoked-attestation-certificates/dac-provider-test-vectors")

        if self.revocation_set_base_path is None or not os.path.exists(self.revocation_set_base_path):
            asserts.fail("--string-arg revocation_set_base_path:<revocation_set_base_path> is required for this test, please provide the path to the revocation set, it can be found in Matter SDK at: credentials/test/revoked-attestation-certificates/revocation-sets")

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
                     "(DUT)Commissioner warns about commissioning the non-genuine device, Or Commissioning fails with device appropriate attestation error"),
            TestStep(2, "Test commissioning with revoked PAI",
                     "(DUT)Commissioner warns about commissioning the non-genuine device, Or Commissioning fails with device appropriate attestation error"),
            TestStep(3, "Test commissioning with both DAC and PAI revoked",
                     "(DUT)Commissioner warns about commissioning the non-genuine device, Or Commissioning fails with device appropriate attestation error"),
            TestStep(4, "Test commissioning with revoked DAC using delegated CRL signer",
                     "(DUT)Commissioner warns about commissioning the non-genuine device, Or Commissioning fails with device appropriate attestation error"),
            TestStep(5, "Test commissioning with revoked PAI using delegated CRL signer",
                     "(DUT)Commissioner warns about commissioning the non-genuine device, Or Commissioning fails with device appropriate attestation error"),
            TestStep(6, "Test commissioning with both DAC and PAI revoked using delegated CRL signer",
                     "(DUT)Commissioner warns about commissioning the non-genuine device, Or Commissioning fails with device appropriate attestation error"),
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
                'manual_pairing_code': '14970112338',
                'discriminator': 0x700,
            },
            {
                'name': 'tc_pai_revoked',
                'dac_provider': 'revoked-pai.json',
                'revocation_set': 'revocation-set.json',
                'expects_commissioning_success': False,
                'manual_pairing_code': '20054912334',
                'discriminator': 0x800,
            },
            {
                'name': 'tc_dac_and_pai_revoked',
                'dac_provider': 'revoked-dac-and-pai.json',
                'revocation_set': 'revocation-set.json',
                'expects_commissioning_success': False,
                'manual_pairing_code': '21693312337',
                'discriminator': 0x900,
            },
            {
                'name': 'tc_dac_revoked_using_delegated_crl_signer',
                'dac_provider': 'indirect-revoked-dac-01.json',
                'revocation_set': 'revocation-set.json',
                'expects_commissioning_success': False,
                'manual_pairing_code': '23331712339',
                'discriminator': 0xA00,
            },
            {
                'name': 'tc_pai_revoked_using_delegated_crl_signer',
                'dac_provider': 'indirect-revoked-pai-03.json',
                'revocation_set': 'revocation-set.json',
                'expects_commissioning_success': False,
                'manual_pairing_code': '24970112330',
                'discriminator': 0xB00,
            },
            {
                'name': 'tc_dac_and_pai_revoked_using_delegated_crl_signer',
                'dac_provider': 'indirect-revoked-dac-01-pai-03.json',
                'revocation_set': 'revocation-set.json',
                'expects_commissioning_success': False,
                'manual_pairing_code': '30054912331',
                'discriminator': 0xC00,
            },
            {
                'name': 'tc_dac_and_pai_valid',
                'dac_provider': None,
                'revocation_set': 'revocation-set.json',
                'expects_commissioning_success': True,
                'manual_pairing_code': '31693312339',
                'discriminator': 0xD00,
            }
        ]

        for idx, test_case in enumerate(test_vectors):
            self.step(idx + 1)

            # Clean up any existing KVS files
            subprocess.call("rm -f all-clusters-kvs*", shell=True)

            # Create log files for this test case
            log_path = os.path.join(self.matter_test_config.logs_path, 'TC_DA_1_9')
            os.makedirs(log_path, exist_ok=True)
            app_log_file_name = os.path.join(log_path, f"{test_case['name']}_app.log")

            with open(app_log_file_name, 'w') as app_log_file:
                # Start the all-clusters-app with appropriate DAC provider
                app_args = '--trace_decode 1 --KVS all-clusters-kvs'
                if test_case['dac_provider']:
                    dac_provider_path = os.path.join(self.dac_provider_base_path, test_case['dac_provider'])
                    app_args += f' --dac_provider {dac_provider_path}'

                if test_case['discriminator']:
                    discriminator = test_case['discriminator']
                    app_args += f' --discriminator {discriminator}'

                app_cmd = f"{self.app_path} {app_args}"

                # Run the all-clusters-app in background
                app_process = subprocess.Popen(app_cmd.split(), stdout=app_log_file, stderr=app_log_file)

                revocation_set = os.path.join(self.revocation_set_base_path, test_case['revocation_set'])
                # Prompt user with instructions
                prompt_msg = (
                    f"\nPlease commission the DUT with:\n"
                    f"  Manual Pairing Code: '{test_case['manual_pairing_code']}'\n"
                    f"  Revocation Set: {revocation_set}\n\n"
                    f"Input 'Y' if DUT successfully commissions without any warnings\n"
                    f"Input 'N' if commissioner warns about commissioning the non-genuine device, "
                    f"Or Commissioning fails with device appropriate attestation error\n"
                )

                if self.is_pics_sdk_ci_only:
                    try:
                        self.default_controller.SetDACRevocationSetPath(revocation_set)
                        await self.default_controller.CommissionWithCode(
                            setupPayload=test_case['manual_pairing_code'],
                            nodeid=1,
                            discoveryType=ChipDeviceCtrl.DiscoveryType.DISCOVERY_NETWORK_ONLY,
                        )
                        resp = 'Y'
                    except Exception:
                        resp = 'N'
                else:
                    resp = self.wait_for_user_input(prompt_msg)

                commissioning_success = resp.lower() == 'y'

                app_process.send_signal(signal.SIGTERM.value)
                app_process.wait()

                # Verify results
                asserts.assert_equal(
                    commissioning_success,
                    test_case['expects_commissioning_success'],
                    f"Commissioning {'succeeded' if commissioning_success else 'failed'} when it should have {'succeeded' if test_case['expects_commissioning_success'] else 'failed'}"
                )


if __name__ == "__main__":
    default_matter_test_main()
