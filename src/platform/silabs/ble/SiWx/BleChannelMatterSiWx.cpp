/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
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

/**
 *    @file
 *      Matter BLE channel implementation for SiWx917.
 */

#include "BleChannelMatterSiWx.h"
#include <lib/core/CHIPError.h>

namespace chip {
namespace DeviceLayer {
namespace Internal {

CHIP_ERROR BleChannelMatterSiWx::Init()
{
    return CHIP_NO_ERROR;
}

CHIP_ERROR BleChannelMatterSiWx::Shutdown()
{
    return CHIP_NO_ERROR;
}

CHIP_ERROR BleChannelMatterSiWx::StartAdvertising(const BleAdvertisingParams & params)
{
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

CHIP_ERROR BleChannelMatterSiWx::StopAdvertising()
{
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

CHIP_ERROR BleChannelMatterSiWx::SetAdvertisingData(const uint8_t * data, size_t length)
{
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

CHIP_ERROR BleChannelMatterSiWx::CloseConnection(BleConnectionHandle conId)
{
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

CHIP_ERROR BleChannelMatterSiWx::UpdateConnectionParams(BleConnectionHandle conId,
                                                        const BleConnectionParams & params)
{
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

CHIP_ERROR BleChannelMatterSiWx::SendIndication(BleConnectionHandle conId, uint16_t charHandle,
                                                const uint8_t * data, size_t length)
{
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

uint16_t BleChannelMatterSiWx::GetMTU(BleConnectionHandle conId) const
{
    return 0;
}

void BleChannelMatterSiWx::SetEventCallback(BleEventCallback callback, void * context) {}

bool BleChannelMatterSiWx::CanHandleEvent(uint32_t eventId) const
{
    return false;
}

void BleChannelMatterSiWx::ParseEvent(void * platformEvent) {}

CHIP_ERROR BleChannelMatterSiWx::GetAddress(uint8_t * addr, size_t * length)
{
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

CHIP_ERROR BleChannelMatterSiWx::SetDeviceName(const char * name)
{
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

CHIP_ERROR BleChannelMatterSiWx::StartIndicationTimer(BleConnectionHandle conId, uint32_t timeoutMs,
                                                      void (*callback)(void *), void * context)
{
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

CHIP_ERROR BleChannelMatterSiWx::CancelIndicationTimer(BleConnectionHandle conId)
{
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
