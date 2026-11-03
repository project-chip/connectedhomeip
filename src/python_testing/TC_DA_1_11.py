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

# See TC_DA_PQC_common.py for the required arguments and fixture generation.
#
# dut_supports_pqc_profiles selects the matrix column: when true, the PQC rows must show
# segmented chain retrieval in the TH app log; when false (the SDK-CI default: standard
# CI controller builds use a crypto backend without ML-DSA, so negotiation falls back),
# every row must fall back to legacy retrieval and still commission successfully, which
# is the matrix row for a DUT supporting only EcdsaMatterLegacy. Run the true column
# with a controller built with chip_crypto="openssl" against OpenSSL 3.5+.

from TC_DA_PQC_common import PQCDACommissionerTestBase

from matter.testing.decorators import async_test_body, pics
from matter.testing.runner import default_matter_test_main


class TC_DA_1_11(PQCDACommissionerTestBase):
    """[TC-DA-1.11] Validate CertificateChainRequest with Multiple Signature Algorithms [DUT-Commissioner]"""

    kDiscriminatorBase = 0x720

    @pics("MCORE.ROLE.COMMISSIONER")
    @async_test_body
    async def test_TC_DA_1_11(self):
        self.step(1, "Start TH as a legacy-only commissionee (PQCDA not set) and commission it from the DUT.",
                  expectation="DUT falls back to legacy and retrieves the full DAC and PAI without segmented follow-up "
                  "requests.")
        counts = await self.run_pqc_da_row(1, 'pqcda_not_set', provider_path=None, pipe_mode=None,
                                           expected_outcome='DUT commissions the legacy-only device successfully')
        self.assert_single_response_retrieval(counts, 'pqcda_not_set')

        self.step(2, "Start TH advertising EcdsaMatterLegacy + MlDsa44 and commission it from the DUT.",
                  expectation="A DUT supporting MlDsa44 selects it and issues segmented retrieval requests; a DUT supporting "
                  "only EcdsaMatterLegacy retrieves the legacy certificates in single responses.")
        counts = await self.run_pqc_da_row(2, 'advertises_ml_dsa_44', provider_path=self.provider_44,
                                           pipe_mode=None,
                                           expected_outcome='DUT commissions the device successfully')
        if self.dut_supports_pqc:
            self.assert_segmented_retrieval(counts, 'advertises_ml_dsa_44')
        else:
            self.assert_single_response_retrieval(counts, 'advertises_ml_dsa_44')

        self.step(3, "Start TH advertising EcdsaMatterLegacy + MlDsa65 and commission it from the DUT.",
                  expectation="A DUT supporting MlDsa65 selects it and issues segmented retrieval requests; a DUT supporting "
                  "only EcdsaMatterLegacy retrieves the legacy certificates in single responses.")
        counts = await self.run_pqc_da_row(3, 'advertises_ml_dsa_65', provider_path=self.provider_65,
                                           pipe_mode=None,
                                           expected_outcome='DUT commissions the device successfully')
        if self.dut_supports_pqc:
            self.assert_segmented_retrieval(counts, 'advertises_ml_dsa_65')
        else:
            self.assert_single_response_retrieval(counts, 'advertises_ml_dsa_65')

        self.step(4, "Start TH with PQCDA set but PQCDeviceAttestationProfile unreadable, and commission it "
                     "from the DUT.",
                  expectation="DUT falls back to legacy behavior and successfully retrieves a usable legacy DAC/PAI pair.")
        counts = await self.run_pqc_da_row(4, 'profile_attribute_unreadable_fallback',
                                           provider_path=self.provider_65, pipe_mode='Failure',
                                           expected_outcome='DUT falls back to legacy device attestation and '
                                                            'commissions successfully')
        self.assert_single_response_retrieval(counts, 'profile_attribute_unreadable_fallback')


if __name__ == "__main__":
    default_matter_test_main()
