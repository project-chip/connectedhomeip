/*
 *   Copyright (c) 2022 Project CHIP Authors
 *   All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */

#pragma once
#include "../common/CHIPCommandBridge.h"
#import <Matter/Matter.h>

enum class PairingMode
{
    Unpair,
    Code,
    Ble,
    AlreadyDiscoveredByIndex,
};

enum class CommissioningType
{
    None,           // establish PASE only
    WithoutNetwork, // commission but don't configure network
    WithWiFi,       // commission and configure WiFi
    WithThread,     // commission and configure Thread
};

class PairingCommandBridge : public CHIPCommandBridge
{
public:
    PairingCommandBridge(const char * commandName, PairingMode mode, CommissioningType commissioningType) :
        CHIPCommandBridge(commandName), mPairingMode(mode), mCommissioningType(commissioningType)
    {
        AddArgument("node-id", 0, UINT64_MAX, &mNodeId);
        switch (commissioningType)
        {
        case CommissioningType::None:
        case CommissioningType::WithoutNetwork:
            break;
        case CommissioningType::WithWiFi:
            AddArgument("ssid", &mSSID);
            AddArgument("password", &mPassword);
            break;
        case CommissioningType::WithThread:
            AddArgument("operationalDataset", &mOperationalDataset);
            break;
        }

        switch (mode)
        {
        case PairingMode::Unpair:
            break;
        case PairingMode::Code:
            AddArgument("payload", &mOnboardingPayload);
            AddArgument("dcl-hostname", &mDCLHostName,
                        "Hostname of the DCL server to fetch information from. Defaults to 'on.dcl.csa-iot.org'.");
            AddArgument("dcl-port", 0, UINT16_MAX, &mDCLPort, "Port number for connecting to the DCL server. Defaults to '443'.");
            AddArgument("use-dcl", 0, 1, &mUseDCL, "Use DCL to fetch onboarding information");
            break;
        case PairingMode::Ble:
            AddArgument("setup-pin-code", 0, 134217727, &mSetupPINCode);
            AddArgument("discriminator", 0, 4096, &mDiscriminator);
            break;
        case PairingMode::AlreadyDiscoveredByIndex:
            AddArgument("payload", &mOnboardingPayload);
            AddArgument("index", 0, UINT16_MAX, &mIndex);
            AddArgument("dcl-hostname", &mDCLHostName,
                        "Hostname of the DCL server to fetch information from. Defaults to 'on.dcl.csa-iot.org'.");
            AddArgument("dcl-port", 0, UINT16_MAX, &mDCLPort, "Port number for connecting to the DCL server. Defaults to '443'.");
            AddArgument("use-dcl", 0, 1, &mUseDCL, "Use DCL to fetch onboarding information");
            break;
        }

        if (commissioningType != CommissioningType::None)
        {
            AddArgument("country-code", &mCountryCode,
                        "Country code to use to set the Basic Information cluster's Location attribute");
            AddArgument("use-device-attestation-delegate", 0, 1, &mUseDeviceAttestationDelegate,
                        "If true, use a device attestation delegate that always wants to be notified about attestation results.  "
                        "Defaults to false.");
            AddArgument("device-attestation-failsafe-time", 0, UINT16_MAX, &mDeviceAttestationFailsafeTime,
                        "If set, the time to extend the failsafe to before calling the device attestation delegate");
        }
    }

    /////////// CHIPCommandBridge Interface /////////
    CHIP_ERROR RunCommand() override;
    chip::System::Clock::Timeout GetWaitDuration() const override { return chip::System::Clock::Seconds16(120); }

private:
    void PairWithCode(NSError * __autoreleasing * error);
    void PairWithIndex(NSError * __autoreleasing * error);
    void PairWithPayload(NSError * __autoreleasing * error);
    void Unpair();
    void SetUpDeviceControllerDelegate(NSError * __autoreleasing * error);
    void MaybeDisplayTermsAndConditions(MTRCommissioningParameters * params, NSError * __autoreleasing * error);

    const PairingMode mPairingMode;
    const CommissioningType mCommissioningType;
    chip::ByteSpan mOperationalDataset;
    chip::ByteSpan mSSID;
    chip::ByteSpan mPassword;
    chip::NodeId mNodeId;
    uint16_t mDiscriminator;
    uint32_t mSetupPINCode;
    uint16_t mIndex;
    char * mOnboardingPayload;
    chip::Optional<bool> mUseDeviceAttestationDelegate;
    chip::Optional<uint16_t> mDeviceAttestationFailsafeTime;
    chip::Optional<char *> mCountryCode;
    chip::Optional<char *> mDCLHostName;
    chip::Optional<uint16_t> mDCLPort;
    chip::Optional<bool> mUseDCL;
};
