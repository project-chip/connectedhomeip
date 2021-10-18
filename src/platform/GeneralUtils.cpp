/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2018 Nest Labs, Inc.
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

/**
 *    @file
 *          General utility functions available on all platforms.
 */

#include <lib/support/CodeUtils.h>
#include <platform/internal/CHIPDeviceLayerInternal.h>

namespace chip {
namespace DeviceLayer {

/**
 * Register a text error formatter for Device Layer errors.
 */
void RegisterDeviceLayerErrorFormatter()
{
    static ErrorFormatter sDeviceLayerErrorFormatter = { FormatDeviceLayerError, nullptr };

    RegisterErrorFormatter(&sDeviceLayerErrorFormatter);
}

/**
 * Given Device Layer error, returns a human-readable NULL-terminated C string
 * describing the error.
 *
 * @param[in] buf                   Buffer into which the error string will be placed.
 * @param[in] bufSize               Size of the supplied buffer in bytes.
 * @param[in] err                   The error to be described.
 *
 * @return true                     If a descriptions string was written into the supplied buffer.
 * @return false                    If the supplied error was not a Device Layer error.
 *
 */
bool FormatDeviceLayerError(char * buf, uint16_t bufSize, CHIP_ERROR err)
{
    const char * desc = nullptr;

    if (!err.IsPart(ChipError::SdkPart::kDevice))
    {
        return false;
    }

#if !CHIP_CONFIG_SHORT_ERROR_STR
    switch (err.AsInteger())
    {
    case CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND.AsInteger():
        desc = "Config not found";
        break;
    case CHIP_DEVICE_ERROR_NOT_SERVICE_PROVISIONED.AsInteger():
        desc = "Not service provisioned";
        break;
    }
#endif // !CHIP_CONFIG_SHORT_ERROR_STR

    FormatError(buf, bufSize, "Device Layer", err, desc);

    return true;
}

const char * CharacterizeIPv6Address(const chip::Inet::IPAddress & ipAddr)
{
    if (ipAddr.IsIPv6LinkLocal())
    {
        return "IPv6 link-local address";
    }

    if (ipAddr.IsIPv6ULA())
    {
        return "IPv6 unique local address";
    }

    if (ipAddr.IsIPv6GlobalUnicast())
    {
        return "IPv6 global unicast address";
    }

    return "IPv6 address";
}

} // namespace DeviceLayer
} // namespace chip
