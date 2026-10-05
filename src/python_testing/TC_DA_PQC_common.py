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

"""Shared code for the DUT-Commissioner PQC device attestation tests (TC-DA-1.11/1.13).

The DUT is a commissioner. Each matrix row starts the Linux example app as the TH
commissionee with the row's attestation configuration and has the DUT commission it.
The TH app log is the vendor-independent evidence channel: commissioning completion and
the CertificateChainRequest count per certificate type are observed from the TH side.

Common arguments (no CI test-arguments block: fixture JSONs are not checked in):

  --string-arg app_path:${ALL_CLUSTERS_APP}
  --string-arg pqc_dac_provider_ml_dsa_44:<provider json with dac/pai_cert_ml_dsa_44>
  --string-arg pqc_dac_provider_ml_dsa_65:<provider json with dac/pai_cert_ml_dsa_65>
  --bool-arg dut_supports_pqc_profiles:<true|false>
  --int-arg app_passcode:<passcode>  (default 20202021)

app_path is passed by the test runner (see .github/workflows/tests.yaml for how other
tests receive ${ALL_CLUSTERS_APP}); there is no built-in default. Generate the provider
fixtures and the matching paa-root-certs trust-store directory with
scripts/tools/gen_pqc_test_fixtures.sh; with dut_supports_pqc_profiles:true the SDK
commissioner needs that directory as --paa-trust-store-path.

The fallback rows use the app pipe command "SetPQCDeviceAttestationProfileReadMode" to
keep the PQCDA feature bit set while making PQCDeviceAttestationProfile reads fail.
"""

import asyncio
import json
import logging
import os
import threading

from mobly import asserts

from matter import ChipDeviceCtrl
from matter.setup_payload.setup_payload import SetupPayload
from matter.testing.apps import AppServerSubprocess
from matter.testing.matter_testing import MatterTestCommissioner

logger = logging.getLogger(__name__)

kDefaultAppPasscode = 20202021
kPaiChainRequestLogLine = "Certificate Chain request received for PAI"
kDacChainRequestLogLine = "Certificate Chain request received for DAC"
kCommissioningSuccessLogLine = "Commissioning completed successfully"
kServerReadyLogLine = "Server initialization complete"
kReadModeCommand = "SetPQCDeviceAttestationProfileReadMode"
# DER-encoded ML-DSA-44/65 signature algorithm OIDs (FIPS 204), used to check that a
# trust store actually contains the generated PQC roots.
kMlDsaSignatureOids = (bytes.fromhex('0609608648016503040311'), bytes.fromhex('0609608648016503040312'))


class PQCCommissioneeApp(AppServerSubprocess):
    """TH commissionee app with TH-side evidence counters on its output stream."""

    def __init__(self, app: str, storage_dir: str, discriminator: int, passcode: int, fifo_path: str,
                 extra_args: list[str]):
        super().__init__(app, storage_dir=storage_dir, discriminator=discriminator, passcode=passcode,
                         extra_args=['--app-pipe', fifo_path] + extra_args)
        self.fifo_path = fifo_path
        self._count_lock = threading.Lock()
        self._counts = {kPaiChainRequestLogLine: 0, kDacChainRequestLogLine: 0, kCommissioningSuccessLogLine: 0}
        prefix_cb = self.output_cb

        def tap(line: bytes, is_stderr: bool) -> bytes:
            text = line.decode('utf-8', errors='replace')
            with self._count_lock:
                for needle in self._counts:
                    if needle in text:
                        self._counts[needle] += 1
            return prefix_cb(line, is_stderr)

        self.output_cb = tap

    def count(self, needle: str) -> int:
        with self._count_lock:
            return self._counts[needle]


class PQCDACommissionerTestBase(MatterTestCommissioner):
    """Common setup and row runner for TC-DA-1.11 and TC-DA-1.13."""

    # Distinct per subclass so parallel-ish reruns never collide on discovery.
    kDiscriminatorBase = 0x700

    def setup_class(self):
        super().setup_class()

        params = self.matter_test_config.global_test_params
        self.app_path = params.get('app_path')
        self.provider_44 = params.get('pqc_dac_provider_ml_dsa_44')
        self.provider_65 = params.get('pqc_dac_provider_ml_dsa_65')
        self.dut_supports_pqc = params.get('dut_supports_pqc_profiles', not self.is_pics_sdk_ci_only)
        # The app's default passcode may change; allow overriding it per run.
        self.app_passcode = params.get('app_passcode', kDefaultAppPasscode)

        if self.app_path is None or not os.path.exists(self.app_path):
            asserts.fail("--string-arg app_path:<path> is required; the test runner provides it the same way "
                         "other tests receive ${ALL_CLUSTERS_APP} (see .github/workflows/tests.yaml)")

        for suffix, name, path in (('44', 'pqc_dac_provider_ml_dsa_44', self.provider_44),
                                   ('65', 'pqc_dac_provider_ml_dsa_65', self.provider_65)):
            if path is None or not os.path.exists(path):
                asserts.fail(f"--string-arg {name}:<provider json> is required: pre-condition 1 needs a "
                             "TestHarnessDACProvider JSON carrying that ML-DSA chain (see the module docstring)")
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
            # With the SDK controller as the DUT, the PQC rows validate the ML-DSA chains, so
            # the controller's trust store must hold both generated PQC roots (published by
            # the fixture generator in <fixture-dir>/paa-root-certs).
            store = self.matter_test_config.paa_trust_store_path
            store_certs = [entry.read_bytes() for entry in store.glob('*.der')] if store is not None else []
            if not all(any(oid in cert for cert in store_certs) for oid in kMlDsaSignatureOids):
                asserts.fail("dut_supports_pqc_profiles is true but --paa-trust-store-path is missing an "
                             "ML-DSA-44 or ML-DSA-65 PAA; point it at the fixture generator's paa-root-certs "
                             "directory")

    async def run_pqc_da_row(self, index: int, name: str, provider_path: str | None, pipe_mode: str | None,
                             expected_outcome: str) -> tuple[int, int]:
        """Run one matrix row; returns (pai_requests, dac_requests) observed by the TH.

        Starts the TH commissionee with the row's configuration, applies the simulated
        profile-read behavior through the app pipe when requested, has the DUT commission
        the TH (SDK controller in CI, operator prompt otherwise), and corroborates the
        reported outcome against the TH-side commissioning-complete log marker.
        """
        log_path = os.path.join(self.matter_test_config.logs_path, type(self).__name__)
        os.makedirs(log_path, exist_ok=True)
        fifo_path = os.path.abspath(os.path.join(log_path, f"{name}_fifo"))
        discriminator = self.kDiscriminatorBase + index
        manual_code = SetupPayload().GenerateManualPairingCode(passcode=self.app_passcode,
                                                               discriminator=discriminator)

        app = PQCCommissioneeApp(self.app_path, storage_dir=log_path, discriminator=discriminator,
                                 passcode=self.app_passcode, fifo_path=fifo_path,
                                 extra_args=['--dac_provider', provider_path] if provider_path else [])
        # start() terminates the process itself if the ready marker never appears.
        app.start(expected_output=kServerReadyLogLine, timeout=30)

        try:
            if pipe_mode is not None:
                # The FIFO is created in ApplicationInit, which is not strictly ordered
                # against the server-ready log line; wait for it before writing.
                for _ in range(100):
                    if os.path.exists(fifo_path):
                        break
                    await asyncio.sleep(0.1)
                else:
                    asserts.fail(f"Row {name}: the app never created {fifo_path}")
                # The command is handled asynchronously on the Matter thread; arm the
                # acknowledgement match before writing so commissioning cannot start
                # before the simulated behavior is active.
                app.arm_output_match(f"read mode set to {pipe_mode}")
                self.write_to_app_pipe({"Name": kReadModeCommand, "Mode": pipe_mode}, app_pipe=fifo_path)
                if not app.wait_for_output(timeout=10):
                    asserts.fail(f"Row {name}: the app never acknowledged read mode {pipe_mode}")

            if self.is_pics_sdk_ci_only:
                try:
                    await self.default_controller.CommissionOnNetwork(
                        nodeId=self.kDiscriminatorBase * 0x10 + index, setupPinCode=self.app_passcode,
                        filterType=ChipDeviceCtrl.DiscoveryFilterType.LONG_DISCRIMINATOR, filter=discriminator)
                    resp = 'Y'
                except Exception as commissioning_error:
                    logger.exception("Commissioning failed for row %s: %s", name, commissioning_error)
                    resp = 'N'
            else:
                resp = self.wait_for_user_input(
                    f"\nPlease commission the TH commissionee with the DUT commissioner:\n"
                    f"  Manual Pairing Code: '{manual_code}'\n"
                    f"  (discriminator 0x{discriminator:03X}, passcode {self.app_passcode})\n\n"
                    f"Expected: {expected_outcome}\n\n"
                    f"Input 'Y' if the DUT commissions the device successfully as described\n"
                    f"Input 'N' if commissioning fails or the DUT reports an attestation error\n")

            commissioning_success = resp is not None and resp.lower() == 'y'
        finally:
            # Bounded teardown: SIGTERM, then SIGKILL on timeout (see Subprocess.terminate).
            app.terminate()

        asserts.assert_true(commissioning_success,
                            f"Row {name}: expected '{expected_outcome}', but commissioning did not succeed")
        # Corroborate the reported outcome with TH-side evidence so a mistaken 'Y' (or a
        # commissioning call that returned without completing) cannot produce a false pass.
        asserts.assert_greater(app.count(kCommissioningSuccessLogLine), 0,
                               f"Row {name}: the TH app never logged '{kCommissioningSuccessLogLine}', so the "
                               "commissioning reported as successful never completed on the commissionee")

        pai_requests = app.count(kPaiChainRequestLogLine)
        dac_requests = app.count(kDacChainRequestLogLine)
        logger.info("Row %s: TH observed %d PAI and %d DAC CertificateChainRequest(s)", name, pai_requests,
                    dac_requests)
        asserts.assert_greater(pai_requests, 0, f"Row {name}: TH never received a PAI request")
        asserts.assert_greater(dac_requests, 0, f"Row {name}: TH never received a DAC request")
        return pai_requests, dac_requests

    def assert_segmented_retrieval(self, counts: tuple[int, int], name: str):
        """A PQC-capable DUT retrieves the ML-DSA-signed PAI and DAC in >1 segment each."""
        pai_requests, dac_requests = counts
        asserts.assert_greater(pai_requests, 1,
                               f"Row {name}: profile-selected retrieval must be segmented, but the TH saw a "
                               "single PAI request")
        asserts.assert_greater(dac_requests, 1,
                               f"Row {name}: profile-selected retrieval must be segmented, but the TH saw a "
                               "single DAC request")

    def assert_single_response_retrieval(self, counts: tuple[int, int], name: str):
        """Legacy and fallback rows are expected to retrieve each certificate in one response.

        A conformant DUT may retry a request after a transient failure, which is
        indistinguishable from segmentation by count alone, so extra requests are reported
        as a warning rather than a failure; correctness is anchored on the corroborated
        commissioning outcome and on the segmented assertions of the PQC rows.
        """
        pai_requests, dac_requests = counts
        if pai_requests > 1 or dac_requests > 1:
            logger.warning("Row %s: observed %d PAI / %d DAC requests where 1 was expected; possible DUT "
                           "retries after transient failures", name, pai_requests, dac_requests)
