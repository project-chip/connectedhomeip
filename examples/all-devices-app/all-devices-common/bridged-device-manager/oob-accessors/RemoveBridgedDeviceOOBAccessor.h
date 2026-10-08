/*
 *    Copyright (c) 2026 Project CHIP Authors
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

#include <bridged-device-manager/BridgedDeviceManager.h>
#include <lib/core/TLV.h>
#include <lib/support/logging/CHIPLogging.h>
#include <oob-accessors/OOBAccessor.h>

#include <optional>

namespace chip::app {

/**
 * Usage:
 *    Tag(2): BridgedDeviceManager::DeviceInterfaceId deviceInterfaceId (uint16_t)
 *       The deviceInterfaceId of the bridged device to be removed.
 *
 * Note: Here Tag(2) is used for the deviceInterfaceId, because Tag(1) is commonly used for endpointId.
 */

class RemoveBridgedDeviceOOBAccessor : public OOBAccessor
{
public:
    explicit RemoveBridgedDeviceOOBAccessor(BridgedDeviceManager & bridgedDeviceManager);
    std::optional<CHIP_ERROR> HandleAction(CharSpan action, ByteSpan tlvData) override;

private:
    BridgedDeviceManager & mBridgedDeviceManager;
};

} // namespace chip::app
