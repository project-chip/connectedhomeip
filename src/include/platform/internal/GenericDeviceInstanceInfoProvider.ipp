/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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

#include <platform/internal/GenericDeviceInstanceInfoProvider.h>

namespace chip {
namespace DeviceLayer {
namespace Internal {

template <class ConfigClass>
CHIP_ERROR GenericDeviceInstanceInfoProvider<ConfigClass>::GetVendorId(uint16_t & vendorId)
{
    vendorId = static_cast<uint16_t>(CHIP_DEVICE_CONFIG_DEVICE_VENDOR_ID);
    return CHIP_NO_ERROR;
}

template <class ConfigClass>
CHIP_ERROR GenericDeviceInstanceInfoProvider<ConfigClass>::GetProductId(uint16_t & productId)
{
    productId = static_cast<uint16_t>(CHIP_DEVICE_CONFIG_DEVICE_PRODUCT_ID);
    return CHIP_NO_ERROR;
}

template <class ConfigClass>
CHIP_ERROR GenericDeviceInstanceInfoProvider<ConfigClass>::GetVendorName(char * buf, size_t bufSize)
{
    ReturnErrorCodeIf(bufSize < sizeof(CHIP_DEVICE_CONFIG_DEVICE_VENDOR_NAME), CHIP_ERROR_BUFFER_TOO_SMALL);
    strcpy(buf, CHIP_DEVICE_CONFIG_DEVICE_VENDOR_NAME);
    return CHIP_NO_ERROR;
}

template <class ConfigClass>
CHIP_ERROR GenericDeviceInstanceInfoProvider<ConfigClass>::GetProductName(char * buf, size_t bufSize)
{
    ReturnErrorCodeIf(bufSize < sizeof(CHIP_DEVICE_CONFIG_DEVICE_PRODUCT_NAME), CHIP_ERROR_BUFFER_TOO_SMALL);
    strcpy(buf, CHIP_DEVICE_CONFIG_DEVICE_PRODUCT_NAME);

    return CHIP_NO_ERROR;
}

template <class ConfigClass>
CHIP_ERROR GenericDeviceInstanceInfoProvider<ConfigClass>::GetSerialNumber(char * buf, size_t bufSize)
{
    ChipError err       = CHIP_NO_ERROR;
    size_t serialNumLen = 0; // without counting null-terminator

    err = mGenericConfigManager.ReadConfigValueStr(ConfigClass::kConfigKey_SerialNum, buf, bufSize, serialNumLen);

#ifdef CHIP_DEVICE_CONFIG_TEST_SERIAL_NUMBER
    if (CHIP_DEVICE_CONFIG_TEST_SERIAL_NUMBER[0] != 0 && err == CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND)
    {
        ReturnErrorCodeIf(sizeof(CHIP_DEVICE_CONFIG_TEST_SERIAL_NUMBER) > bufSize, CHIP_ERROR_BUFFER_TOO_SMALL);
        memcpy(buf, CHIP_DEVICE_CONFIG_TEST_SERIAL_NUMBER, sizeof(CHIP_DEVICE_CONFIG_TEST_SERIAL_NUMBER));
        serialNumLen = sizeof(CHIP_DEVICE_CONFIG_TEST_SERIAL_NUMBER) - 1;
        err = CHIP_NO_ERROR;
    }
#endif // CHIP_DEVICE_CONFIG_TEST_SERIAL_NUMBER
    ReturnErrorOnFailure(err);

    ReturnErrorCodeIf(serialNumLen >= bufSize, CHIP_ERROR_BUFFER_TOO_SMALL);
    ReturnErrorCodeIf(buf[serialNumLen] != 0, CHIP_ERROR_INVALID_STRING_LENGTH);

    return err;
}

template <class ConfigClass>
CHIP_ERROR GenericDeviceInstanceInfoProvider<ConfigClass>::GetManufacturingDate(uint16_t & year, uint8_t & month, uint8_t & day)
{
    CHIP_ERROR err;
    enum
    {
        kDateStringLength = 10 // YYYY-MM-DD
    };
    char dateStr[kDateStringLength + 1];
    size_t dateLen;
    char * parseEnd;

    err = mGenericConfigManager.ReadConfigValueStr(ConfigClass::kConfigKey_ManufacturingDate, dateStr, sizeof(dateStr), dateLen);
    SuccessOrExit(err);

    VerifyOrExit(dateLen == kDateStringLength, err = CHIP_ERROR_INVALID_ARGUMENT);

    // Cast does not lose information, because we then check that we only parsed
    // 4 digits, so our number can't be bigger than 9999.
    year = static_cast<uint16_t>(strtoul(dateStr, &parseEnd, 10));
    VerifyOrExit(parseEnd == dateStr + 4, err = CHIP_ERROR_INVALID_ARGUMENT);

    // Cast does not lose information, because we then check that we only parsed
    // 2 digits, so our number can't be bigger than 99.
    month = static_cast<uint8_t>(strtoul(dateStr + 5, &parseEnd, 10));
    VerifyOrExit(parseEnd == dateStr + 7, err = CHIP_ERROR_INVALID_ARGUMENT);

    // Cast does not lose information, because we then check that we only parsed
    // 2 digits, so our number can't be bigger than 99.
    day = static_cast<uint8_t>(strtoul(dateStr + 8, &parseEnd, 10));
    VerifyOrExit(parseEnd == dateStr + 10, err = CHIP_ERROR_INVALID_ARGUMENT);

exit:
    if (err != CHIP_NO_ERROR && err != CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND)
    {
        ChipLogError(DeviceLayer, "Invalid manufacturing date: %s", dateStr);
    }
    return err;
}

template <class ConfigClass>
CHIP_ERROR GenericDeviceInstanceInfoProvider<ConfigClass>::GetHardwareVersion(uint16_t & hardwareVersion)
{
    ChipError err   = CHIP_NO_ERROR;
    uint32_t valInt = 0;

    err = mGenericConfigManager.ReadConfigValue(ConfigClass::kConfigKey_HardwareVersion, valInt);
    if (err == CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND)
    {
        hardwareVersion = static_cast<uint16_t>(CHIP_DEVICE_CONFIG_DEFAULT_DEVICE_HARDWARE_VERSION);
        err             = CHIP_NO_ERROR;
    }
    else
    {
        hardwareVersion = static_cast<uint16_t>(valInt);
    }

    return err;
}

template <class ConfigClass>
CHIP_ERROR GenericDeviceInstanceInfoProvider<ConfigClass>::GetHardwareVersionString(char * buf, size_t bufSize)
{
    ReturnErrorCodeIf(bufSize < sizeof(CHIP_DEVICE_CONFIG_DEFAULT_DEVICE_HARDWARE_VERSION_STRING), CHIP_ERROR_BUFFER_TOO_SMALL);
    strcpy(buf, CHIP_DEVICE_CONFIG_DEFAULT_DEVICE_HARDWARE_VERSION_STRING);
    return CHIP_NO_ERROR;
}

template <class ConfigClass>
CHIP_ERROR GenericDeviceInstanceInfoProvider<ConfigClass>::GetRotatingDeviceIdUniqueId(MutableByteSpan & uniqueIdSpan)
{
    ChipError err = CHIP_ERROR_WRONG_KEY_TYPE;
#if CHIP_ENABLE_ROTATING_DEVICE_ID && defined(CHIP_DEVICE_CONFIG_ROTATING_DEVICE_ID_UNIQUE_ID)
    static_assert(ConfigurationManager::kRotatingDeviceIDUniqueIDLength >= ConfigurationManager::kMinRotatingDeviceIDUniqueIDLength,
                  "Length of unique ID for rotating device ID is smaller than minimum.");
    constexpr uint8_t uniqueId[] = CHIP_DEVICE_CONFIG_ROTATING_DEVICE_ID_UNIQUE_ID;

    ReturnErrorCodeIf(sizeof(uniqueId) > uniqueIdSpan.size(), CHIP_ERROR_BUFFER_TOO_SMALL);
    ReturnErrorCodeIf(sizeof(uniqueId) != ConfigurationManager::kRotatingDeviceIDUniqueIDLength, CHIP_ERROR_BUFFER_TOO_SMALL);
    memcpy(uniqueIdSpan.data(), uniqueId, sizeof(uniqueId));
    uniqueIdSpan.reduce_size(sizeof(uniqueId));
    return CHIP_NO_ERROR;
#endif
    return err;
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
