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
 *      Matter BLE channel implementation for EFR32.
 */

#include "BleChannelMatterEfr32.h"
#include <lib/core/CHIPError.h>

namespace chip {
namespace DeviceLayer {
namespace Internal {

CHIP_ERROR BleChannelMatterEfr32::Init()
{
    return CHIP_NO_ERROR;
}

CHIP_ERROR BleChannelMatterEfr32::Shutdown()
{
    return CHIP_NO_ERROR;
}

CHIP_ERROR BleChannelMatterEfr32::StartAdvertising(const BleAdvertisingParams & params)
{
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

CHIP_ERROR BleChannelMatterEfr32::StopAdvertising()
{
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

CHIP_ERROR BleChannelMatterEfr32::SetAdvertisingData(const uint8_t * data, size_t length)
{
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

CHIP_ERROR BleChannelMatterEfr32::CloseConnection(BleConnectionHandle conId)
{
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

CHIP_ERROR BleChannelMatterEfr32::UpdateConnectionParams(BleConnectionHandle conId, const BleConnectionParams & params)
{
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

CHIP_ERROR BleChannelMatterEfr32::SendIndication(BleConnectionHandle conId, uint16_t charHandle, const uint8_t * data,
                                                 size_t length)
{
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

uint16_t BleChannelMatterEfr32::GetMTU(BleConnectionHandle conId) const
{
    return 0;
}

void BleChannelMatterEfr32::SetEventCallback(BleEventCallback callback, void * context) {}

bool BleChannelMatterEfr32::CanHandleEvent(uint32_t eventId) const
{
    return false;
}

void BleChannelMatterEfr32::ParseEvent(void * platformEvent) {}

CHIP_ERROR BleChannelMatterEfr32::GetAddress(uint8_t * addr, size_t * length)
{
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

CHIP_ERROR BleChannelMatterEfr32::SetDeviceName(const char * name)
{
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

CHIP_ERROR BleChannelMatterEfr32::StartIndicationTimer(BleConnectionHandle conId, uint32_t timeoutMs, void (*callback)(void *),
                                                       void * context)
{
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

CHIP_ERROR BleChannelMatterEfr32::CancelIndicationTimer(BleConnectionHandle conId)
{
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
