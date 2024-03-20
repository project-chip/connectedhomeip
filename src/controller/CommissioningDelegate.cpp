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

} // namespace Controller
} // namespace chip
