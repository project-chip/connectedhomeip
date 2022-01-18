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

#include <controller/AutoCommissioner.h>

#include <controller/CHIPDeviceController.h>

namespace chip {
namespace Controller {

CHIP_ERROR AutoCommissioner::SetCommissioningParameters(const CommissioningParameters & params)
{
    mParams = params;
    if (params.HasThreadOperationalDataset())
    {
        ByteSpan dataset = params.GetThreadOperationalDataset().Value();
        if (dataset.size() > CommissioningParameters::kMaxThreadDatasetLen)
        {
            return CHIP_ERROR_INVALID_ARGUMENT;
        }
        memcpy(mThreadOperationalDataset, dataset.data(), dataset.size());
        mParams.SetThreadOperationalDataset(ByteSpan(mThreadOperationalDataset, dataset.size()));
    }
    if (params.HasWiFiCredentials())
    {
        WiFiCredentials creds = params.GetWiFiCredentials().Value();
        if (creds.ssid.size() > CommissioningParameters::kMaxSsidLen ||
            creds.credentials.size() > CommissioningParameters::kMaxCredentialsLen)
        {
            return CHIP_ERROR_INVALID_ARGUMENT;
        }
        memcpy(mSsid, creds.ssid.data(), creds.ssid.size());
        memcpy(mCredentials, creds.credentials.data(), creds.credentials.size());
        mParams.SetWiFiCredentials(
            WiFiCredentials(ByteSpan(mSsid, creds.ssid.size()), ByteSpan(mCredentials, creds.credentials.size())));
    }
    return CHIP_NO_ERROR;
}

CommissioningStage AutoCommissioner::GetNextCommissioningStage(CommissioningStage currentStage, CHIP_ERROR lastErr)
{
    if (lastErr != CHIP_NO_ERROR)
    {
        return CommissioningStage::kCleanup;
    }
    switch (currentStage)
    {
    case CommissioningStage::kSecurePairing:
        return CommissioningStage::kArmFailsafe;
    case CommissioningStage::kArmFailsafe:
        return CommissioningStage::kConfigRegulatory;
    case CommissioningStage::kConfigRegulatory:
        return CommissioningStage::kDeviceAttestation;
    case CommissioningStage::kDeviceAttestation:
        // TODO(cecille): device attestation casues operational cert provisioinging to happen, This should be a separate stage.
        // For thread and wifi, this should go to network setup then enable. For on-network we can skip right to finding the
        // operational network because the provisioning of certificates will trigger the device to start operational advertising.
        if (mParams.HasWiFiCredentials())
        {
            return CommissioningStage::kWiFiNetworkSetup;
        }
        else if (mParams.HasThreadOperationalDataset())
        {
            return CommissioningStage::kThreadNetworkSetup;
        }
        else
        {
#if CHIP_DEVICE_CONFIG_ENABLE_DNSSD
            return CommissioningStage::kFindOperational;
#else
            return CommissioningStage::kSendComplete;
#endif
        }
    case CommissioningStage::kWiFiNetworkSetup:
        if (mParams.HasThreadOperationalDataset())
        {
            return CommissioningStage::kThreadNetworkSetup;
        }
        else
        {
            return CommissioningStage::kWiFiNetworkEnable;
        }
    case CommissioningStage::kThreadNetworkSetup:
        if (mParams.HasWiFiCredentials())
        {
            return CommissioningStage::kWiFiNetworkEnable;
        }
        else
        {
            return CommissioningStage::kThreadNetworkEnable;
        }

    case CommissioningStage::kWiFiNetworkEnable:
        if (mParams.HasThreadOperationalDataset())
        {
            return CommissioningStage::kThreadNetworkEnable;
        }
        else
        {
            return CommissioningStage::kFindOperational;
        }
    case CommissioningStage::kThreadNetworkEnable:
        return CommissioningStage::kFindOperational;
    case CommissioningStage::kFindOperational:
        return CommissioningStage::kSendComplete;
    case CommissioningStage::kSendComplete:
        return CommissioningStage::kCleanup;

    // Currently unimplemented.
    case CommissioningStage::kConfigACL:
    case CommissioningStage::kCheckCertificates:
        return CommissioningStage::kError;
    // Neither of these have a next stage so return kError;
    case CommissioningStage::kCleanup:
    case CommissioningStage::kError:
        return CommissioningStage::kError;
    }
    return CommissioningStage::kError;
}

void AutoCommissioner::StartCommissioning(CommissioneeDeviceProxy * proxy)
{
    // TODO: check that there is no commissioning in progress currently.
    mCommissioneeDeviceProxy = proxy;
    mCommissioner->PerformCommissioningStep(mCommissioneeDeviceProxy, CommissioningStage::kArmFailsafe, mParams, this);
}

void AutoCommissioner::CommissioningStepFinished(CHIP_ERROR err, CommissioningDelegate::CommissioningReport report)
{
    if (report.stageCompleted == CommissioningStage::kFindOperational)
    {
        mOperationalDeviceProxy = report.OperationalNodeFoundData.operationalProxy;
    }
    CommissioningStage nextStage = GetNextCommissioningStage(report.stageCompleted, err);
    DeviceProxy * proxy          = mCommissioneeDeviceProxy;
    if (nextStage == CommissioningStage::kSendComplete ||
        (nextStage == CommissioningStage::kCleanup && mOperationalDeviceProxy != nullptr))
    {
        proxy = mOperationalDeviceProxy;
    }

    if (proxy == nullptr)
    {
        ChipLogError(Controller, "Invalid device for commissioning");
        return;
    }
    mParams.SetCompletionStatus(err);
    mCommissioner->PerformCommissioningStep(proxy, nextStage, mParams, this);
}

} // namespace Controller
} // namespace chip
