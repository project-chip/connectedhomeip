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
        break;

    case kSecurePairing:
        return "SecurePairing";
        break;

    case kReadCommissioningInfo:
        return "ReadCommissioningInfo";
        break;

    case kCheckForMatchingFabric:
        return "CheckForMatchingFabric";
        break;

    case kArmFailsafe:
        return "ArmFailSafe";
        break;

    case kScanNetworks:
        return "ScanNetworks";
        break;

    case kConfigRegulatory:
        return "ConfigRegulatory";
        break;

    case kConfigureUTCTime:
        return "ConfigureUTCTime";
        break;

    case kConfigureTimeZone:
        return "ConfigureTimeZone";
        break;

    case kConfigureDSTOffset:
        return "ConfigureDSTOffset";
        break;

    case kConfigureDefaultNTP:
        return "ConfigureDefaultNTP";
        break;

    case kSendPAICertificateRequest:
        return "SendPAICertificateRequest";
        break;

    case kSendDACCertificateRequest:
        return "SendDACCertificateRequest";
        break;

    case kSendAttestationRequest:
        return "SendAttestationRequest";
        break;

    case kAttestationVerification:
        return "AttestationVerification";
        break;

    case kSendOpCertSigningRequest:
        return "SendOpCertSigningRequest";
        break;

    case kValidateCSR:
        return "ValidateCSR";
        break;

    case kGenerateNOCChain:
        return "GenerateNOCChain";
        break;

    case kSendTrustedRootCert:
        return "SendTrustedRootCert";
        break;

    case kSendNOC:
        return "SendNOC";
        break;

    case kConfigureTrustedTimeSource:
        return "ConfigureTrustedTimeSource";
        break;

    case kWiFiNetworkSetup:
        return "WiFiNetworkSetup";
        break;

    case kThreadNetworkSetup:
        return "ThreadNetworkSetup";
        break;

    case kFailsafeBeforeWiFiEnable:
        return "FailsafeBeforeWiFiEnable";
        break;

    case kFailsafeBeforeThreadEnable:
        return "FailsafeBeforeThreadEnable";
        break;

    case kWiFiNetworkEnable:
        return "WiFiNetworkEnable";
        break;

    case kThreadNetworkEnable:
        return "ThreadNetworkEnable";
        break;

    case kFindOperational:
        return "FindOperational";
        break;

    case kSendComplete:
        return "SendComplete";
        break;

    case kCleanup:
        return "Cleanup";
        break;

    case kNeedsNetworkCreds:
        return "NeedsNetworkCreds";
        break;

    default:
        return "???";
        break;
    }
}

} // namespace Controller
} // namespace chip
