/*
 *
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "ChipDeviceController-ScriptDevicePairingDelegate.h"

#include <controller/CHIPDeviceController.h>
#include <platform/CHIPDeviceLayer.h>
#include <system/SystemClock.h>

namespace chip {
namespace Controller {

class ScriptPairingDeviceDiscoveryDelegate : public DeviceDiscoveryDelegate
{
public:
    CHIP_ERROR Init(NodeId nodeId, uint32_t setupPasscode, CommissioningParameters commissioningParams,
                    ScriptDevicePairingDelegate * pairingDelegate, DeviceCommissioner * activeDeviceCommissioner,
                    uint32_t discoveryTimeoutMsec)
    {
        mNodeId                   = nodeId;
        mSetupPasscode            = setupPasscode;
        mParams                   = commissioningParams;
        mPairingDelegate          = pairingDelegate;
        mActiveDeviceCommissioner = activeDeviceCommissioner;
        VerifyOrReturnError(mActiveDeviceCommissioner != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
        mActiveDeviceCommissioner->RegisterDeviceDiscoveryDelegate(this);
        return chip::DeviceLayer::SystemLayer().StartTimer(System::Clock::Milliseconds32(discoveryTimeoutMsec), OnDiscoveredTimeout,
                                                           this);
    }
    void OnDiscoveredDevice(const Dnssd::DiscoveredNodeData & nodeData);

private:
    static void OnDiscoveredTimeout(System::Layer * layer, void * context)
    {
        ChipLogError(Controller, "Mdns discovery timed out");
        auto * self = static_cast<ScriptPairingDeviceDiscoveryDelegate *>(context);

        // Stop Mdns discovery.
        self->mActiveDeviceCommissioner->RegisterDeviceDiscoveryDelegate(nullptr);

        if (self->mPairingDelegate != nullptr)
            self->mPairingDelegate->OnPairingComplete(CHIP_ERROR_TIMEOUT);
    }

    ScriptDevicePairingDelegate * mPairingDelegate;
    DeviceCommissioner * mActiveDeviceCommissioner = nullptr;

    CommissioningParameters mParams;
    NodeId mNodeId;
    uint32_t mSetupPasscode;
};

} // namespace Controller
} // namespace chip
