/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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
