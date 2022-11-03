/*
 * SPDX-FileCopyrightText: (c) 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "ChipDeviceController-ScriptDevicePairingDelegate.h"

#include <controller/CHIPDeviceController.h>

namespace chip {
namespace Controller {

class ScriptPairingDeviceDiscoveryDelegate : public DeviceDiscoveryDelegate
{
public:
    void Init(NodeId nodeId, uint32_t setupPasscode, CommissioningParameters commissioningParams,
              ScriptDevicePairingDelegate * pairingDelegate, DeviceCommissioner * activeDeviceCommissioner)
    {
        mNodeId                   = nodeId;
        mSetupPasscode            = setupPasscode;
        mParams                   = commissioningParams;
        mPairingDelegate          = pairingDelegate;
        mActiveDeviceCommissioner = activeDeviceCommissioner;
    }
    void OnDiscoveredDevice(const Dnssd::DiscoveredNodeData & nodeData);

private:
    ScriptDevicePairingDelegate * mPairingDelegate;
    DeviceCommissioner * mActiveDeviceCommissioner = nullptr;

    CommissioningParameters mParams;
    NodeId mNodeId;
    uint32_t mSetupPasscode;
};

} // namespace Controller
} // namespace chip
