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
# dut_supports_pqc_profiles selects the matrix column: when true, rows 2 and 3
# additionally require segmented chain retrieval in the TH app log (the "using
# MlDsa44/65" half of the row outcome); when false (the SDK-CI default: standard CI
# controller builds use a crypto backend without ML-DSA, so negotiation falls back),
# commissioning success over the legacy fallback is the row result. Run the true column
# with a controller built with chip_crypto="openssl" against OpenSSL 3.5+.

from TC_DA_PQC_common import PQCDACommissionerTestBase

from matter.testing.decorators import async_test_body, pics
from matter.testing.runner import default_matter_test_main


class TC_DA_1_13(PQCDACommissionerTestBase):
    """[TC-DA-1.13] ML-DSA Device Attestation Request Validation Scenario [DUT-Commissioner]"""

    kDiscriminatorBase = 0x710

    @pics("MCORE.ROLE.COMMISSIONER")
    @async_test_body
    async def test_TC_DA_1_13(self):
        self.step(1, "Start TH as a legacy-only commissionee (PQCDA not set) and commission it from the DUT.",
                  expectation="DUT commissions the legacy-only device successfully.")
        await self.run_pqc_da_row(1, 'legacy_only_device', provider_path=None, pipe_mode=None,
                                  expected_outcome='DUT commissions the legacy-only device successfully')

        self.step(2, "Start TH advertising EcdsaMatterLegacy + MlDsa44 for PAA/PAI and commission it from the "
                     "DUT.",
                  expectation="DUT commissions the PQC-capable device successfully using MlDsa44.")
        expected = ('DUT commissions the device successfully using MlDsa44' if self.dut_supports_pqc
                    else 'DUT commissions the device successfully (legacy fallback for a legacy-only DUT)')
        counts = await self.run_pqc_da_row(2, 'pqc_ml_dsa_44', provider_path=self.provider_44, pipe_mode=None,
                                           expected_outcome=expected)
        if self.dut_supports_pqc:
            self.assert_segmented_retrieval(counts, 'pqc_ml_dsa_44')

        self.step(3, "Start TH advertising EcdsaMatterLegacy + MlDsa65 for PAA/PAI and commission it from the "
                     "DUT.",
                  expectation="DUT commissions the PQC-capable device successfully using MlDsa65.")
        expected = ('DUT commissions the device successfully using MlDsa65' if self.dut_supports_pqc
                    else 'DUT commissions the device successfully (legacy fallback for a legacy-only DUT)')
        counts = await self.run_pqc_da_row(3, 'pqc_ml_dsa_65', provider_path=self.provider_65, pipe_mode=None,
                                           expected_outcome=expected)
        if self.dut_supports_pqc:
            self.assert_segmented_retrieval(counts, 'pqc_ml_dsa_65')

        self.step(4, "Start TH with PQCDA set but PQCDeviceAttestationProfile absent or unreadable, and "
                     "commission it from the DUT.",
                  expectation="DUT falls back to legacy device attestation and still commissions successfully.")
        counts = await self.run_pqc_da_row(4, 'profile_attribute_unreadable_fallback',
                                           provider_path=self.provider_65, pipe_mode='UnsupportedAttribute',
                                           expected_outcome='DUT falls back to legacy device attestation and '
                                                            'commissions successfully')
        self.assert_single_response_retrieval(counts, 'profile_attribute_unreadable_fallback')


if __name__ == "__main__":
    default_matter_test_main()
