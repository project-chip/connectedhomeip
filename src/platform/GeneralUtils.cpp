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

#include <platform/internal/CHIPDeviceLayerInternal.h>
#include <support/CodeUtils.h>

namespace chip {
namespace DeviceLayer {

CHIP_ERROR ParseCompilerDateStr(const char * dateStr, uint16_t & year, uint8_t & month, uint8_t & dayOfMonth)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    char monthStr[4];
    const char * p;
    char * endptr;

    static const char months[] = "JanFebMarAprMayJunJulAugSepOctNovDec";

    VerifyOrExit(strlen(dateStr) == 11, err = CHIP_ERROR_INVALID_ARGUMENT);

    memcpy(monthStr, dateStr, 3);
    monthStr[3] = 0;

    p = strstr(months, monthStr);
    VerifyOrExit(p != NULL, err = CHIP_ERROR_INVALID_ARGUMENT);

    month = ((p - months) / 3) + 1;

    dayOfMonth = strtoul(dateStr + 4, &endptr, 10);
    VerifyOrExit(endptr == dateStr + 6, err = CHIP_ERROR_INVALID_ARGUMENT);

    year = strtoul(dateStr + 7, &endptr, 10);
    VerifyOrExit(endptr == dateStr + 11, err = CHIP_ERROR_INVALID_ARGUMENT);

exit:
    return err;
}

CHIP_ERROR Parse24HourTimeStr(const char * timeStr, uint8_t & hour, uint8_t & minute, uint8_t & second)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    char * p;

    VerifyOrExit(strlen(timeStr) == 8, err = CHIP_ERROR_INVALID_ARGUMENT);

    hour = strtoul(timeStr, &p, 10);
    VerifyOrExit(p == timeStr + 2, err = CHIP_ERROR_INVALID_ARGUMENT);

    minute = strtoul(timeStr + 3, &p, 10);
    VerifyOrExit(p == timeStr + 5, err = CHIP_ERROR_INVALID_ARGUMENT);

    second = strtoul(timeStr + 6, &p, 10);
    VerifyOrExit(p == timeStr + 8, err = CHIP_ERROR_INVALID_ARGUMENT);

exit:
    return err;
}

/**
 * Register a text error formatter for Device Layer errors.
 */
void RegisterDeviceLayerErrorFormatter(void)
{
    static ErrorFormatter sDeviceLayerErrorFormatter = { FormatDeviceLayerError, NULL };

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
bool FormatDeviceLayerError(char * buf, uint16_t bufSize, int32_t err)
{
    const char * desc = NULL;

    if (err < CHIP_DEVICE_ERROR_MIN || err > CHIP_DEVICE_ERROR_MAX)
    {
        return false;
    }

#if !CHIP_CONFIG_SHORT_ERROR_STR
    switch (err)
    {
    case CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND:
        desc = "Config not found";
        break;
    case CHIP_DEVICE_ERROR_NOT_SERVICE_PROVISIONED:
        desc = "Not service provisioned";
        break;
    }
#endif // !CHIP_CONFIG_SHORT_ERROR_STR

    FormatError(buf, bufSize, "Device Layer", err, desc);

    return true;
}

const char * CharacterizeIPv6Address(const IPAddress & ipAddr)
{
    if (ipAddr.IsIPv6LinkLocal())
    {
        return "IPv6 link-local address";
    }
    else if (ipAddr.IsIPv6ULA())
    {
        return "IPv6 unique local address";
    }
    else if ((ntohl(ipAddr.Addr[0]) & 0xE0000000U) == 0x20000000U)
    {
        return "IPv6 global unicast address";
    }
    return "IPv6 address";
}

} // namespace DeviceLayer
} // namespace chip
