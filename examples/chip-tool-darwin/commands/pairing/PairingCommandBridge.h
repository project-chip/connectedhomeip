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
#import <CHIP/CHIPClustersObjc.h>
#import <CHIP/CHIPDevicePairingDelegate.h>

enum class PairingMode
{
    None,
    QRCode,
    ManualCode,
    Ethernet
};

class PairingCommandBridge : public CHIPCommandBridge
{
public:
    PairingCommandBridge(const char * commandName, PairingMode mode) : CHIPCommandBridge(commandName), mPairingMode(mode)
    {
        AddArgument("node-id", 0, UINT64_MAX, &mNodeId);

        switch (mode)
        {
        case PairingMode::None:
            break;
        case PairingMode::QRCode:
            AddArgument("payload", &mOnboardingPayload);
            break;
        case PairingMode::ManualCode:
            AddArgument("payload", &mOnboardingPayload);
            break;
        case PairingMode::Ethernet:
            AddArgument("setup-pin-code", 0, 134217727, &mSetupPINCode);
            AddArgument("discriminator", 0, 4096, &mDiscriminator);
            AddArgument("device-remote-ip", &ipAddress);
            AddArgument("device-remote-port", 0, UINT16_MAX, &mRemotePort);
            break;
        }
    }

    /////////// CHIPCommandBridge Interface /////////
    CHIP_ERROR RunCommand() override;
    chip::System::Clock::Timeout GetWaitDuration() const override { return chip::System::Clock::Seconds16(120); }

private:
    void PairWithCode(NSError * __autoreleasing * error);
    void PairWithIPAddress(NSError * __autoreleasing * error);
    void Unpair(NSError * __autoreleasing * error);
    void SetUpPairingDelegate();

    const PairingMode mPairingMode;
    chip::NodeId mNodeId;
    uint16_t mRemotePort;
    uint16_t mDiscriminator;
    uint32_t mSetupPINCode;
    char * mOnboardingPayload;
    char * ipAddress;
};
