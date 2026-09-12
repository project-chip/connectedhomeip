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

# [TC-DA-1.11] Validate CertificateChainRequest with Multiple Signature Algorithms [DUT-Commissioner]
#
# The DUT is a commissioner. For each row of the certificate matrix, this script starts the
# Linux example app as the TH commissionee with the row's attestation configuration, has the
# DUT commission it, and then checks the retrieval mode from the TH side: a PQC profile makes
# the ML-DSA-signed PAI (and the DAC it signs) larger than one 600-byte segment, so a DUT that
# negotiated a PQC profile shows up in the TH app log as repeated CertificateChainRequest
# invocations for the same certificate type, while legacy and fallback rows complete in a
# single request per certificate.
#
# Arguments (no CI block yet; no PQC provider fixtures are checked in):
#
#   --string-arg app_path:<path to chip-all-clusters-app>
#   --string-arg pqc_dac_provider_ml_dsa_44:<provider json with dac/pai_cert_ml_dsa_44>
#   --string-arg pqc_dac_provider_ml_dsa_65:<provider json with dac/pai_cert_ml_dsa_65>
#   --bool-arg dut_supports_pqc_profiles:<true|false>
#   --int-arg app_passcode:<passcode>  (default 20202021)
#
# dut_supports_pqc_profiles states whether the DUT commissioner implements PQC profile
# negotiation (matrix column "DUT Supported Profiles"). When false, every row must fall back
# to legacy retrieval and still commission successfully, which is exactly the matrix row for
# a DUT supporting only EcdsaMatterLegacy. It defaults to false for SDK-CI runs (the SDK
# commissioner's PQC negotiation is landing separately) and true otherwise. A PQC-capable
# DUT also needs the generated ML-DSA PAA roots in its trust store: the fixture generator
# publishes them in <fixture-dir>/paa-root-certs, which the SDK commissioner takes as
# --paa-trust-store-path.
#
# The fallback row uses the app pipe command "SetPQCDeviceAttestationProfileReadMode" to keep PQCDA set
# while making PQCDeviceAttestationProfile reads fail (test setup condition 2).

import asyncio
import contextlib
import json
import logging
import os
import signal

from mobly import asserts

from matter import ChipDeviceCtrl
from matter.setup_payload.setup_payload import SetupPayload
from matter.testing.decorators import async_test_body
from matter.testing.matter_testing import MatterTestCommissioner
from matter.testing.runner import TestStep, default_matter_test_main

logger = logging.getLogger(__name__)

kDefaultAppPasscode = 20202021
kCommissioningSuccessLogLine = "Commissioning completed successfully"
kPaiChainRequestLogLine = "Certificate Chain request received for PAI"
kDacChainRequestLogLine = "Certificate Chain request received for DAC"


def count_occurrences_in_log(log_file_name: str, needle: str) -> int:
    """Count TH app log lines containing needle; the log is TH-side evidence of DUT behavior."""
    try:
        with open(log_file_name, errors="replace") as log_file:
            return sum(needle in line for line in log_file)
    except OSError:
        return 0


class TC_DA_1_11(MatterTestCommissioner):
    """Validate CertificateChainRequest with Multiple Signature Algorithms [DUT-Commissioner]."""

    def setup_class(self):
        super().setup_class()

        self.app_path = self.matter_test_config.global_test_params.get('app_path')
        self.provider_44 = self.matter_test_config.global_test_params.get('pqc_dac_provider_ml_dsa_44')
        self.provider_65 = self.matter_test_config.global_test_params.get('pqc_dac_provider_ml_dsa_65')
        self.dut_supports_pqc = self.matter_test_config.global_test_params.get(
            'dut_supports_pqc_profiles', not self.is_pics_sdk_ci_only)
        # The app's default passcode may change; allow overriding it per run.
        self.app_passcode = self.matter_test_config.global_test_params.get('app_passcode', kDefaultAppPasscode)

        if self.app_path is None and self.is_pics_sdk_ci_only:
            root_dir = os.path.abspath(os.path.join(os.path.dirname(__file__), '../..'))
            self.app_path = os.path.join(
                root_dir, "objdir-clone/linux-x64-all-clusters-ipv6only-no-ble-no-wifi-tsan-clang-test/chip-all-clusters-app")

        if self.app_path is None or not os.path.exists(self.app_path):
            asserts.fail("--string-arg app_path:<app_path> is required: the TH commissionee is the Linux example app")

        for suffix, name, path in (('44', 'pqc_dac_provider_ml_dsa_44', self.provider_44),
                                   ('65', 'pqc_dac_provider_ml_dsa_65', self.provider_65)):
            if path is None or not os.path.exists(path):
                asserts.fail(f"--string-arg {name}:<provider json> is required: pre-condition 1 needs a "
                             "TestHarnessDACProvider JSON carrying that ML-DSA chain (see the header of this file)")
            # A malformed or miskeyed fixture would make the app keep its default legacy
            # credentials without setting PQCDA, so the PQC rows would silently degrade to
            # testing legacy fallback only. Validate the fixture up front.
            try:
                with open(path) as provider_file:
                    provider = json.load(provider_file)
                for key in (f'pai_cert_ml_dsa_{suffix}', f'dac_cert_ml_dsa_{suffix}'):
                    if not bytes.fromhex(provider[key]):
                        raise ValueError(f"{key} is empty")
            except (OSError, ValueError, KeyError, TypeError) as fixture_error:
                asserts.fail(f"{name} is not a usable PQC provider fixture ({fixture_error!r}); "
                             "regenerate it with scripts/tools/gen_pqc_test_fixtures.sh")

        if self.dut_supports_pqc and self.is_pics_sdk_ci_only:
            # With the SDK controller as the DUT, rows 2/3 validate the ML-DSA chains, so the
            # controller's trust store must hold the generated PQC roots (the fixture
            # generator publishes them in <fixture-dir>/paa-root-certs).
            kMlDsaSignatureOids = (bytes.fromhex('0609608648016503040311'), bytes.fromhex('0609608648016503040312'))
            store = self.matter_test_config.paa_trust_store_path
            store_certs = [entry.read_bytes() for entry in store.glob('*.der')] if store is not None else []
            has_all_pqc_roots = all(any(oid in cert for cert in store_certs) for oid in kMlDsaSignatureOids)
            if not has_all_pqc_roots:
                asserts.fail("dut_supports_pqc_profiles is true but --paa-trust-store-path is missing an ML-DSA-44 "
                             "or ML-DSA-65 PAA; point it at the fixture generator's paa-root-certs directory")

    def desc_TC_DA_1_11(self) -> str:
        return "[TC-DA-1.11] Validate CertificateChainRequest with Multiple Signature Algorithms [DUT-Commissioner]"

    def pics_TC_DA_1_11(self) -> list[str]:
        return ["MCORE.ROLE.COMMISSIONER"]

    def steps_TC_DA_1_11(self) -> list[TestStep]:
        return [
            TestStep(1, "Start TH as a legacy-only commissionee (PQCDA not set) and commission it from the DUT.",
                     "DUT falls back to legacy and retrieves the full DAC and PAI without segmented follow-up "
                     "requests."),
            TestStep(2, "Start TH advertising EcdsaMatterLegacy + MlDsa44 and commission it from the DUT.",
                     "A DUT supporting MlDsa44 selects it and issues segmented retrieval requests; a DUT supporting "
                     "only EcdsaMatterLegacy retrieves the legacy certificates in single responses."),
            TestStep(3, "Start TH advertising EcdsaMatterLegacy + MlDsa65 and commission it from the DUT.",
                     "A DUT supporting MlDsa65 selects it and issues segmented retrieval requests; a DUT supporting "
                     "only EcdsaMatterLegacy retrieves the legacy certificates in single responses."),
            TestStep(4, "Start TH with PQCDA set but PQCDeviceAttestationProfile unreadable, and commission it from "
                        "the DUT.",
                     "DUT falls back to legacy behavior and successfully retrieves a usable legacy DAC/PAI pair."),
        ]

    @async_test_body
    async def test_TC_DA_1_11(self):
        # 'expect_segmented' is what a PQC-capable DUT must do for the row; with
        # dut_supports_pqc_profiles false every row degrades to single-response legacy
        # retrieval, which is the matrix row for a legacy-only DUT.
        test_vectors = [
            {
                'name': 'pqcda_not_set',
                'app_args': [],
                'discriminator': 0x720,
                'expect_segmented': False,
            },
            {
                'name': 'advertises_ml_dsa_44',
                'app_args': ['--dac_provider', self.provider_44],
                'discriminator': 0x721,
                'expect_segmented': self.dut_supports_pqc,
            },
            {
                'name': 'advertises_ml_dsa_65',
                'app_args': ['--dac_provider', self.provider_65],
                'discriminator': 0x722,
                'expect_segmented': self.dut_supports_pqc,
            },
            {
                'name': 'profile_attribute_unreadable_fallback',
                'app_args': ['--dac_provider', self.provider_65],
                'pipe_mode': 'Failure',
                'discriminator': 0x723,
                'expect_segmented': False,
            },
        ]

        log_path = os.path.join(self.matter_test_config.logs_path, 'TC_DA_1_11')
        os.makedirs(log_path, exist_ok=True)

        for idx, test_case in enumerate(test_vectors):
            self.step(idx + 1)

            # The KVS path must be absolute: on Darwin the app resolves relative KVS paths
            # into the user documents directory, where stale state from a previous row would
            # leave the app already commissioned and therefore not discoverable.
            kvs_path = os.path.abspath(os.path.join(log_path, f"{test_case['name']}_kvs"))
            if os.path.exists(kvs_path):
                os.remove(kvs_path)

            app_log_file_name = os.path.join(log_path, f"{test_case['name']}_app.log")
            discriminator = test_case['discriminator']
            manual_code = SetupPayload().GenerateManualPairingCode(passcode=self.app_passcode, discriminator=discriminator)

            pipe_mode = test_case.get('pipe_mode')
            pipe_path = os.path.abspath(os.path.join(log_path, f"{test_case['name']}_fifo"))

            with open(app_log_file_name, 'w') as app_log_file:
                app_args = ['--KVS', kvs_path, '--discriminator', str(discriminator),
                            '--passcode', str(self.app_passcode)] + test_case['app_args']
                if pipe_mode is not None:
                    app_args += ['--app-pipe', pipe_path]
                app_process = await asyncio.create_subprocess_exec(self.app_path, *app_args,
                                                                   stdout=app_log_file, stderr=app_log_file)

                # The commissionee must not outlive the row, even if the pipe setup fails,
                # the prompt hits EOF, or commissioning raises something unexpected.
                try:
                    if pipe_mode is not None:
                        # The app creates the pipe during startup; wait for it, then set the
                        # simulated PQCDeviceAttestationProfile read behavior.
                        for _ in range(100):
                            if os.path.exists(pipe_path):
                                break
                            await asyncio.sleep(0.1)
                        else:
                            asserts.fail(f"Row {test_case['name']}: the app never created {pipe_path}; see "
                                         f"{app_log_file_name}")
                        self.write_to_app_pipe({"Name": "SetPQCDeviceAttestationProfileReadMode", "Mode": pipe_mode},
                                               app_pipe=pipe_path)
                        # The command is handled asynchronously on the Matter thread; wait for the
                        # app's acknowledgement so commissioning cannot start before the simulated
                        # behavior is active.
                        mode_ack = f"read mode set to {pipe_mode}"
                        for _ in range(100):
                            if count_occurrences_in_log(app_log_file_name, mode_ack) > 0:
                                break
                            await asyncio.sleep(0.1)
                        else:
                            asserts.fail(f"Row {test_case['name']}: the app never acknowledged read mode "
                                         f"{pipe_mode}; see {app_log_file_name}")

                    prompt_msg = (
                        f"\nPlease commission the TH commissionee with the DUT commissioner:\n"
                        f"  Manual Pairing Code: '{manual_code}'\n"
                        f"  (discriminator 0x{discriminator:03X}, passcode {self.app_passcode})\n\n"
                        f"Input 'Y' if the DUT commissions the device successfully\n"
                        f"Input 'N' if commissioning fails\n"
                    )

                    if self.is_pics_sdk_ci_only:
                        try:
                            await self.default_controller.CommissionOnNetwork(
                                nodeId=0x11100 + idx, setupPinCode=self.app_passcode,
                                filterType=ChipDeviceCtrl.DiscoveryFilterType.LONG_DISCRIMINATOR,
                                filter=discriminator)
                            resp = 'Y'
                        except Exception as commissioning_error:
                            logger.exception("Commissioning failed for row %s: %s", test_case['name'], commissioning_error)
                            resp = 'N'
                    else:
                        resp = self.wait_for_user_input(prompt_msg)

                    commissioning_success = resp is not None and resp.lower() == 'y'
                finally:
                    with contextlib.suppress(ProcessLookupError):
                        app_process.send_signal(signal.SIGTERM)
                    await app_process.wait()

            # Corroborate the reported outcome with TH-side evidence so a mistaken 'Y' (or a
            # commissioning call that returned without completing) cannot produce a false pass.
            asserts.assert_greater(
                count_occurrences_in_log(app_log_file_name, kCommissioningSuccessLogLine), 0,
                f"Row {test_case['name']}: the TH app log has no '{kCommissioningSuccessLogLine}' entry, so the "
                "commissioning reported as successful never completed on the commissionee")
            asserts.assert_true(commissioning_success,
                                f"Row {test_case['name']}: the DUT must commission the TH successfully")

            pai_requests = count_occurrences_in_log(app_log_file_name, kPaiChainRequestLogLine)
            dac_requests = count_occurrences_in_log(app_log_file_name, kDacChainRequestLogLine)
            logger.info("Row %s: TH observed %d PAI and %d DAC CertificateChainRequest(s)",
                        test_case['name'], pai_requests, dac_requests)

            # The fixture was validated in setup_class, so a provider-load error here means
            # the app fell back to its default legacy credentials and never set PQCDA.
            if '--dac_provider' in test_case['app_args']:
                for provider_error in ("Error opening json file", "Error parsing json file"):
                    asserts.assert_equal(
                        count_occurrences_in_log(app_log_file_name, provider_error), 0,
                        f"Row {test_case['name']}: the app failed to load the DAC provider fixture; "
                        "the row would otherwise only exercise the default legacy chain")

            if test_case['expect_segmented']:
                # An ML-DSA signature makes the PAI (and the DAC it signs) larger than one
                # 600-byte segment, so profile-selected retrieval must arrive as more than one
                # request per certificate type.
                asserts.assert_greater(pai_requests, 1,
                                       f"Row {test_case['name']}: the DUT advertised-profile retrieval must be "
                                       "segmented, but the TH saw a single PAI request")
                asserts.assert_greater(dac_requests, 1,
                                       f"Row {test_case['name']}: the DUT advertised-profile retrieval must be "
                                       "segmented, but the TH saw a single DAC request")
            else:
                # Legacy and fallback rows require each certificate in one response, with no
                # segmented follow-up requests.
                asserts.assert_equal(pai_requests, 1,
                                     f"Row {test_case['name']}: expected the full PAI in a single response, but the "
                                     f"TH saw {pai_requests} PAI request(s)")
                asserts.assert_equal(dac_requests, 1,
                                     f"Row {test_case['name']}: expected the full DAC in a single response, but the "
                                     f"TH saw {dac_requests} DAC request(s)")


if __name__ == "__main__":
    default_matter_test_main()
