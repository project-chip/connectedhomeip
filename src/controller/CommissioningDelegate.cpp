/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
 *    All rights reserved.
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

#include <controller/CommissioningDelegate.h>

namespace chip {
namespace Controller {

const char * StageToString(CommissioningStage stage)
{
    switch (stage)
    {
    case kError:
        return "Error";

    case kSecurePairing:
        return "SecurePairing";

    case kReadCommissioningInfo:
        return "ReadCommissioningInfo";

    case kReadCommissioningInfo2:
        return "ReadCommissioningInfo2";

    case kArmFailsafe:
        return "ArmFailSafe";

    case kScanNetworks:
        return "ScanNetworks";

    case kConfigRegulatory:
        return "ConfigRegulatory";

    case kConfigureUTCTime:
        return "ConfigureUTCTime";

    case kConfigureTimeZone:
        return "ConfigureTimeZone";

    case kConfigureDSTOffset:
        return "ConfigureDSTOffset";

    case kConfigureDefaultNTP:
        return "ConfigureDefaultNTP";

    case kSendPAICertificateRequest:
        return "SendPAICertificateRequest";

    case kSendDACCertificateRequest:
        return "SendDACCertificateRequest";

    case kSendAttestationRequest:
        return "SendAttestationRequest";

    case kAttestationVerification:
        return "AttestationVerification";

    case kAttestationRevocationCheck:
        return "AttestationRevocationCheck";

    case kSendOpCertSigningRequest:
        return "SendOpCertSigningRequest";

    case kValidateCSR:
        return "ValidateCSR";

    case kGenerateNOCChain:
        return "GenerateNOCChain";

    case kSendTrustedRootCert:
        return "SendTrustedRootCert";

    case kSendNOC:
        return "SendNOC";

    case kConfigureTrustedTimeSource:
        return "ConfigureTrustedTimeSource";

    case kICDGetRegistrationInfo:
        return "ICDGetRegistrationInfo";

    case kICDRegistration:
        return "ICDRegistration";

    case kWiFiNetworkSetup:
        return "WiFiNetworkSetup";

    case kThreadNetworkSetup:
        return "ThreadNetworkSetup";

    case kFailsafeBeforeWiFiEnable:
        return "FailsafeBeforeWiFiEnable";

    case kFailsafeBeforeThreadEnable:
        return "FailsafeBeforeThreadEnable";

    case kWiFiNetworkEnable:
        return "WiFiNetworkEnable";

    case kThreadNetworkEnable:
        return "ThreadNetworkEnable";

    case kEvictPreviousCaseSessions:
        return "kEvictPreviousCaseSessions";

    case kFindOperationalForStayActive:
        return "kFindOperationalForStayActive";

    case kFindOperationalForCommissioningComplete:
        return "kFindOperationalForCommissioningComplete";

    case kICDSendStayActive:
        return "ICDSendStayActive";

    case kSendComplete:
        return "SendComplete";

    case kCleanup:
        return "Cleanup";

    case kNeedsNetworkCreds:
        return "NeedsNetworkCreds";

    case kPrimaryOperationalNetworkFailed:
        return "PrimaryOperationalNetworkFailed";

    case kRemoveWiFiNetworkConfig:
        return "RemoveWiFiNetworkConfig";

    case kRemoveThreadNetworkConfig:
        return "RemoveThreadNetworkConfig";

    default:
        return "???";
    }
}

#if MATTER_TRACING_ENABLED
const char * MetricKeyForCommissioningStage(CommissioningStage stage)
{
    switch (stage)
    {
    case kError:
        return "core_commissioning_stage_error";

    case kSecurePairing:
        return "core_commissioning_stage_secure_pairing";

    case kReadCommissioningInfo:
        return "core_commissioning_stage_read_commissioning_info";

    case kReadCommissioningInfo2:
        return "core_commissioning_stage_read_commissioning_info2";

    case kArmFailsafe:
        return "core_commissioning_stage_arm_failsafe";

    case kScanNetworks:
        return "core_commissioning_stage_scan_networks";

    case kConfigRegulatory:
        return "core_commissioning_stage_config_regulatory";

    case kConfigureUTCTime:
        return "core_commissioning_stage_configure_utc_time";

    case kConfigureTimeZone:
        return "core_commissioning_stage_configure_timezone";

    case kConfigureDSTOffset:
        return "core_commissioning_stage_configure_dst_offset";

    case kConfigureDefaultNTP:
        return "core_commissioning_stage_configure_default_ntp";

    case kSendPAICertificateRequest:
        return "core_commissioning_stage_send_pai_certificate_request";

    case kSendDACCertificateRequest:
        return "core_commissioning_stage_send_dac_certificate_request";

    case kSendAttestationRequest:
        return "core_commissioning_stage_send_attestation_request";

    case kAttestationVerification:
        return "core_commissioning_stage_attestation_verification";

    case kSendOpCertSigningRequest:
        return "core_commissioning_stage_opcert_signing_request";

    case kValidateCSR:
        return "core_commissioning_stage_validate_csr";

    case kGenerateNOCChain:
        return "core_commissioning_stage_generate_noc_chain";

    case kSendTrustedRootCert:
        return "core_commissioning_stage_send_trusted_root_cert";

    case kSendNOC:
        return "core_commissioning_stage_send_noc";

    case kConfigureTrustedTimeSource:
        return "core_commissioning_stage_configure_trusted_time_source";

    case kICDGetRegistrationInfo:
        return "core_commissioning_stage_icd_get_registration_info";

    case kICDRegistration:
        return "core_commissioning_stage_icd_registration";

    case kWiFiNetworkSetup:
        return "core_commissioning_stage_wifi_network_setup";

    case kThreadNetworkSetup:
        return "core_commissioning_stage_thread_network_setup";

    case kFailsafeBeforeWiFiEnable:
        return "core_commissioning_stage_failsafe_before_wifi_enable";

    case kFailsafeBeforeThreadEnable:
        return "core_commissioning_stage_failsafe_before_thread_enable";

    case kWiFiNetworkEnable:
        return "core_commissioning_stage_wifi_network_enable";

    case kThreadNetworkEnable:
        return "core_commissioning_stage_thread_network_enable";

    case kEvictPreviousCaseSessions:
        return "core_commissioning_stage_evict_previous_case_sessions";

    case kFindOperationalForStayActive:
        return "core_commissioning_stage_find_operational_for_stay_active";

    case kFindOperationalForCommissioningComplete:
        return "core_commissioning_stage_find_operational_for_commissioning_complete";

    case kICDSendStayActive:
        return "core_commissioning_stage_icd_send_stay_active";

    case kSendComplete:
        return "core_commissioning_stage_send_complete";

    case kCleanup:
        return "core_commissioning_stage_cleanup";

    case kNeedsNetworkCreds:
        return "core_commissioning_stage_need_network_creds";

    default:
        return "core_commissioning_stage_unknown";
    }
}
#endif

} // namespace Controller
} // namespace chip
