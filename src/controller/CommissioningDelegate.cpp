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
        return "core_cm_stg_error";

    case kSecurePairing:
        return "core_cm_stg_secure_pairing";

    case kReadCommissioningInfo:
        return "core_cm_stg_rd_comm_info";

    case kReadCommissioningInfo2:
        return "core_cm_stg_rd_comm_info2";

    case kArmFailsafe:
        return "core_cm_stg_afs";

    case kScanNetworks:
        return "core_cm_stg_scn_nws";

    case kConfigRegulatory:
        return "core_cm_stg_cfg_reg";

    case kConfigureUTCTime:
        return "core_cm_stg_cfg_utc";

    case kConfigureTimeZone:
        return "core_cm_stg_cfg_tmz";

    case kConfigureDSTOffset:
        return "core_cm_stg_cfg_dst_off";

    case kConfigureDefaultNTP:
        return "core_cm_stg_cfg_ntp";

    case kSendPAICertificateRequest:
        return "core_cm_stg_csr_req";

    case kSendDACCertificateRequest:
        return "core_cm_stg_dac_req";

    case kSendAttestationRequest:
        return "core_cm_stg_att_req";

    case kAttestationVerification:
        return "core_cm_stg_att_ver";

    case kSendOpCertSigningRequest:
        return "core_cm_stg_opcrt_csr";

    case kValidateCSR:
        return "core_cm_stg_val_csr";

    case kGenerateNOCChain:
        return "core_cm_stg_gen_noc";

    case kSendTrustedRootCert:
        return "core_cm_stg_trust_rca";

    case kSendNOC:
        return "core_cm_stg_snd_noc";

    case kConfigureTrustedTimeSource:
        return "core_cm_stg_cfg_tms";

    case kICDGetRegistrationInfo:
        return "core_cm_stg_icd_reg_info";

    case kICDRegistration:
        return "core_cm_stg_icd_reg";

    case kWiFiNetworkSetup:
        return "core_cm_stg_wifi_nw_cred";

    case kThreadNetworkSetup:
        return "core_cm_stg_thrd_nw_cred";

    case kFailsafeBeforeWiFiEnable:
        return "core_cm_stg_afs_bf_wifi";

    case kFailsafeBeforeThreadEnable:
        return "core_cm_stg_afs_bf_thrd";

    case kWiFiNetworkEnable:
        return "core_cm_stg_wifi_enbl";

    case kThreadNetworkEnable:
        return "core_cm_stg_thrd_enbl";

    case kEvictPreviousCaseSessions:
        return "core_cm_stg_evict_case";

    case kFindOperationalForStayActive:
        return "core_cm_stg_op_stay_active";

    case kFindOperationalForCommissioningComplete:
        return "core_cm_stg_op_cm_comp";

    case kICDSendStayActive:
        return "core_cm_stg_icd_stay_act";

    case kSendComplete:
        return "core_cm_stg_snd_comp";

    case kCleanup:
        return "core_cm_stg_cleanup";

    case kNeedsNetworkCreds:
        return "core_cm_stg_need_nw_creds";

    default:
        return "core_cm_stg_unknown";
    }
}
#endif

} // namespace Controller
} // namespace chip
