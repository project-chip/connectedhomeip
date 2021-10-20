/*
 *
 *    Copyright (c) 2020-2021 Project CHIP Authors
 *    Copyright (c) 2019-2020 Google LLC.
 *    Copyright (c) 2018 Nest Labs, Inc.
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
 *          Contains non-inline method definitions for the
 *          GenericConfigurationManagerImpl<> template.
 */

#ifndef GENERIC_CONFIGURATION_MANAGER_IMPL_CPP
#define GENERIC_CONFIGURATION_MANAGER_IMPL_CPP

#include <ble/CHIPBleServiceData.h>
#include <inttypes.h>
#include <lib/core/CHIPConfig.h>
#include <lib/support/Base64.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/ScopedBuffer.h>
#include <platform/internal/CHIPDeviceLayerInternal.h>
#include <platform/internal/GenericConfigurationManagerImpl.h>

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
#include <platform/ThreadStackManager.h>
#endif

namespace chip {
namespace DeviceLayer {
namespace Internal {

template <class ImplClass>
CHIP_ERROR GenericConfigurationManagerImpl<ImplClass>::_Init()
{
#if CHIP_ENABLE_ROTATING_DEVICE_ID
    mLifetimePersistedCounter.Init(CHIP_CONFIG_LIFETIIME_PERSISTED_COUNTER_KEY);
#endif

    return CHIP_NO_ERROR;
}

template <class ImplClass>
CHIP_ERROR GenericConfigurationManagerImpl<ImplClass>::_GetVendorName(char * buf, size_t bufSize)
{
    ReturnErrorCodeIf(bufSize < sizeof(CHIP_DEVICE_CONFIG_DEVICE_VENDOR_NAME), CHIP_ERROR_BUFFER_TOO_SMALL);
    strcpy(buf, CHIP_DEVICE_CONFIG_DEVICE_VENDOR_NAME);
    return CHIP_NO_ERROR;
}

template <class ImplClass>
CHIP_ERROR GenericConfigurationManagerImpl<ImplClass>::_GetProductName(char * buf, size_t bufSize)
{
    ReturnErrorCodeIf(bufSize < sizeof(CHIP_DEVICE_CONFIG_DEVICE_PRODUCT_NAME), CHIP_ERROR_BUFFER_TOO_SMALL);
    strcpy(buf, CHIP_DEVICE_CONFIG_DEVICE_PRODUCT_NAME);
    return CHIP_NO_ERROR;
}

template <class ImplClass>
CHIP_ERROR GenericConfigurationManagerImpl<ImplClass>::_GetFirmwareRevisionString(char * buf, size_t bufSize)
{
    ReturnErrorCodeIf(bufSize < sizeof(CHIP_DEVICE_CONFIG_DEVICE_FIRMWARE_REVISION_STRING), CHIP_ERROR_BUFFER_TOO_SMALL);
    strcpy(buf, CHIP_DEVICE_CONFIG_DEVICE_FIRMWARE_REVISION_STRING);
    return CHIP_NO_ERROR;
}

template <class ImplClass>
CHIP_ERROR GenericConfigurationManagerImpl<ImplClass>::_GetSerialNumber(char * buf, size_t bufSize, size_t & serialNumLen)
{
    CHIP_ERROR err;
    err = Impl()->ReadConfigValueStr(ImplClass::kConfigKey_SerialNum, buf, bufSize, serialNumLen);
#ifdef CHIP_DEVICE_CONFIG_TEST_SERIAL_NUMBER
    if (CHIP_DEVICE_CONFIG_TEST_SERIAL_NUMBER[0] != 0 && err == CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND)
    {
        VerifyOrExit(sizeof(CHIP_DEVICE_CONFIG_TEST_SERIAL_NUMBER) <= bufSize, err = CHIP_ERROR_BUFFER_TOO_SMALL);
        memcpy(buf, CHIP_DEVICE_CONFIG_TEST_SERIAL_NUMBER, sizeof(CHIP_DEVICE_CONFIG_TEST_SERIAL_NUMBER));
        serialNumLen = sizeof(CHIP_DEVICE_CONFIG_TEST_SERIAL_NUMBER) - 1;
        err          = CHIP_NO_ERROR;
    }
#endif // CHIP_DEVICE_CONFIG_TEST_SERIAL_NUMBER
    SuccessOrExit(err);

exit:
    return err;
}

template <class ImplClass>
CHIP_ERROR GenericConfigurationManagerImpl<ImplClass>::_StoreSerialNumber(const char * serialNum, size_t serialNumLen)
{
    return Impl()->WriteConfigValueStr(ImplClass::kConfigKey_SerialNum, serialNum, serialNumLen);
}

template <class ImplClass>
CHIP_ERROR GenericConfigurationManagerImpl<ImplClass>::_GetPrimaryWiFiMACAddress(uint8_t * buf)
{
    return CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND;
}

template <class ImplClass>
CHIP_ERROR GenericConfigurationManagerImpl<ImplClass>::_StorePrimaryWiFiMACAddress(const uint8_t * buf)
{
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

template <class ImplClass>
CHIP_ERROR GenericConfigurationManagerImpl<ImplClass>::_GetPrimaryMACAddress(MutableByteSpan buf)
{
    if (buf.size() != ConfigurationManager::kPrimaryMACAddressLength)
        return CHIP_ERROR_INVALID_ARGUMENT;

    memset(buf.data(), 0, buf.size());

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
    if (chip::DeviceLayer::ThreadStackMgr().GetPrimary802154MACAddress(buf.data()) == CHIP_NO_ERROR)
    {
        ChipLogDetail(DeviceLayer, "Using Thread extended MAC for hostname.");
        return CHIP_NO_ERROR;
    }
#else
    if (DeviceLayer::ConfigurationMgr().GetPrimaryWiFiMACAddress(buf.data()) == CHIP_NO_ERROR)
    {
        ChipLogDetail(DeviceLayer, "Using wifi MAC for hostname");
        return CHIP_NO_ERROR;
    }
#endif

    ChipLogError(DeviceLayer, "MAC is not known, using a default.");
    uint8_t temp[ConfigurationManager::kMaxMACAddressLength] = { 0xEE, 0xAA, 0xBA, 0xDA, 0xBA, 0xD0, 0xDD, 0xCA };
    memcpy(buf.data(), temp, buf.size());
    return CHIP_NO_ERROR;
}

template <class ImplClass>
CHIP_ERROR GenericConfigurationManagerImpl<ImplClass>::_GetPrimary802154MACAddress(uint8_t * buf)
{
#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
    return ThreadStackManager().GetPrimary802154MACAddress(buf);
#else
    return CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND;
#endif // CHIP_DEVICE_CONFIG_ENABLE_THREAD
}

template <class ImplClass>
CHIP_ERROR GenericConfigurationManagerImpl<ImplClass>::_StorePrimary802154MACAddress(const uint8_t * buf)
{
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

template <class ImplClass>
inline CHIP_ERROR GenericConfigurationManagerImpl<ImplClass>::_GetProductRevisionString(char * buf, size_t bufSize)
{
    ReturnErrorCodeIf(bufSize < sizeof(CHIP_DEVICE_CONFIG_DEFAULT_DEVICE_PRODUCT_REVISION_STRING), CHIP_ERROR_BUFFER_TOO_SMALL);
    strcpy(buf, CHIP_DEVICE_CONFIG_DEFAULT_DEVICE_PRODUCT_REVISION_STRING);
    return CHIP_NO_ERROR;
}

template <class ImplClass>
inline CHIP_ERROR GenericConfigurationManagerImpl<ImplClass>::_GetProductRevision(uint16_t & productRev)
{
    CHIP_ERROR err;
    uint32_t val;

    err = Impl()->ReadConfigValue(ImplClass::kConfigKey_ProductRevision, val);
    if (err == CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND)
    {
        productRev = static_cast<uint16_t>(CHIP_DEVICE_CONFIG_DEFAULT_DEVICE_PRODUCT_REVISION);
        err        = CHIP_NO_ERROR;
    }
    else
    {
        productRev = static_cast<uint16_t>(val);
    }

    return err;
}

template <class ImplClass>
inline CHIP_ERROR GenericConfigurationManagerImpl<ImplClass>::_StoreProductRevision(uint16_t productRev)
{
    return Impl()->WriteConfigValue(ImplClass::kConfigKey_ProductRevision, static_cast<uint32_t>(productRev));
}

template <class ImplClass>
CHIP_ERROR GenericConfigurationManagerImpl<ImplClass>::_GetManufacturingDate(uint16_t & year, uint8_t & month, uint8_t & dayOfMonth)
{
    CHIP_ERROR err;
    enum
    {
        kDateStringLength = 10 // YYYY-MM-DD
    };
    char dateStr[kDateStringLength + 1];
    size_t dateLen;
    char * parseEnd;

    err = Impl()->ReadConfigValueStr(ImplClass::kConfigKey_ManufacturingDate, dateStr, sizeof(dateStr), dateLen);
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
    dayOfMonth = static_cast<uint8_t>(strtoul(dateStr + 8, &parseEnd, 10));
    VerifyOrExit(parseEnd == dateStr + 10, err = CHIP_ERROR_INVALID_ARGUMENT);

exit:
    if (err != CHIP_NO_ERROR && err != CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND)
    {
        ChipLogError(DeviceLayer, "Invalid manufacturing date: %s", dateStr);
    }
    return err;
}

template <class ImplClass>
CHIP_ERROR GenericConfigurationManagerImpl<ImplClass>::_StoreManufacturingDate(const char * mfgDate, size_t mfgDateLen)
{
    return Impl()->WriteConfigValueStr(ImplClass::kConfigKey_ManufacturingDate, mfgDate, mfgDateLen);
}

template <class ImplClass>
CHIP_ERROR GenericConfigurationManagerImpl<ImplClass>::_GetSetupPinCode(uint32_t & setupPinCode)
{
    CHIP_ERROR err;

    err = Impl()->ReadConfigValue(ImplClass::kConfigKey_SetupPinCode, setupPinCode);
#if defined(CHIP_DEVICE_CONFIG_USE_TEST_SETUP_PIN_CODE) && CHIP_DEVICE_CONFIG_USE_TEST_SETUP_PIN_CODE
    if (err == CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND)
    {
        setupPinCode = CHIP_DEVICE_CONFIG_USE_TEST_SETUP_PIN_CODE;
        err          = CHIP_NO_ERROR;
    }
#endif // defined(CHIP_DEVICE_CONFIG_USE_TEST_SETUP_PIN_CODE) && CHIP_DEVICE_CONFIG_USE_TEST_SETUP_PIN_CODE
    SuccessOrExit(err);

exit:
    return err;
}

template <class ImplClass>
CHIP_ERROR GenericConfigurationManagerImpl<ImplClass>::_StoreSetupPinCode(uint32_t setupPinCode)
{
    return Impl()->WriteConfigValue(ImplClass::kConfigKey_SetupPinCode, setupPinCode);
}

template <class ImplClass>
CHIP_ERROR GenericConfigurationManagerImpl<ImplClass>::_GetSetupDiscriminator(uint16_t & setupDiscriminator)
{
    CHIP_ERROR err;
    uint32_t val;

    err = Impl()->ReadConfigValue(ImplClass::kConfigKey_SetupDiscriminator, val);
#if defined(CHIP_DEVICE_CONFIG_USE_TEST_SETUP_DISCRIMINATOR) && CHIP_DEVICE_CONFIG_USE_TEST_SETUP_DISCRIMINATOR
    if (err == CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND)
    {
        val = CHIP_DEVICE_CONFIG_USE_TEST_SETUP_DISCRIMINATOR;
        err = CHIP_NO_ERROR;
    }
#endif // defined(CHIP_DEVICE_CONFIG_USE_TEST_SETUP_DISCRIMINATOR) && CHIP_DEVICE_CONFIG_USE_TEST_SETUP_DISCRIMINATOR
    SuccessOrExit(err);

    setupDiscriminator = static_cast<uint16_t>(val);

exit:
    return err;
}

template <class ImplClass>
CHIP_ERROR GenericConfigurationManagerImpl<ImplClass>::_StoreSetupDiscriminator(uint16_t setupDiscriminator)
{
    return Impl()->WriteConfigValue(ImplClass::kConfigKey_SetupDiscriminator, static_cast<uint32_t>(setupDiscriminator));
}

template <class ImplClass>
CHIP_ERROR GenericConfigurationManagerImpl<ImplClass>::_GetRegulatoryLocation(uint32_t & location)
{
    return Impl()->ReadConfigValue(ImplClass::kConfigKey_RegulatoryLocation, location);
}

template <class ImplClass>
CHIP_ERROR GenericConfigurationManagerImpl<ImplClass>::_StoreRegulatoryLocation(uint32_t location)
{
    return Impl()->WriteConfigValue(ImplClass::kConfigKey_RegulatoryLocation, location);
}

template <class ImplClass>
CHIP_ERROR GenericConfigurationManagerImpl<ImplClass>::_GetCountryCode(char * buf, size_t bufSize, size_t & codeLen)
{
    return Impl()->ReadConfigValueStr(ImplClass::kConfigKey_CountryCode, buf, bufSize, codeLen);
}

template <class ImplClass>
CHIP_ERROR GenericConfigurationManagerImpl<ImplClass>::_StoreCountryCode(const char * code, size_t codeLen)
{
    return Impl()->WriteConfigValueStr(ImplClass::kConfigKey_CountryCode, code, codeLen);
}

template <class ImplClass>
CHIP_ERROR GenericConfigurationManagerImpl<ImplClass>::_GetBreadcrumb(uint64_t & breadcrumb)
{
    return Impl()->ReadConfigValue(ImplClass::kConfigKey_Breadcrumb, breadcrumb);
}

template <class ImplClass>
CHIP_ERROR GenericConfigurationManagerImpl<ImplClass>::_StoreBreadcrumb(uint64_t breadcrumb)
{
    return Impl()->WriteConfigValue(ImplClass::kConfigKey_Breadcrumb, breadcrumb);
}

#if CHIP_ENABLE_ROTATING_DEVICE_ID
template <class ImplClass>
CHIP_ERROR GenericConfigurationManagerImpl<ImplClass>::_GetLifetimeCounter(uint16_t & lifetimeCounter)
{
    lifetimeCounter = static_cast<uint16_t>(mLifetimePersistedCounter.GetValue());
    return CHIP_NO_ERROR;
}

template <class ImplClass>
CHIP_ERROR GenericConfigurationManagerImpl<ImplClass>::_IncrementLifetimeCounter()
{
    return mLifetimePersistedCounter.Advance();
}
#endif

template <class ImplClass>
CHIP_ERROR GenericConfigurationManagerImpl<ImplClass>::_GetFailSafeArmed(bool & val)
{
    return Impl()->ReadConfigValue(ImplClass::kConfigKey_FailSafeArmed, val);
}

template <class ImplClass>
CHIP_ERROR GenericConfigurationManagerImpl<ImplClass>::_SetFailSafeArmed(bool val)
{
    return Impl()->WriteConfigValue(ImplClass::kConfigKey_FailSafeArmed, val);
}

template <class ImplClass>
CHIP_ERROR
GenericConfigurationManagerImpl<ImplClass>::_GetBLEDeviceIdentificationInfo(Ble::ChipBLEDeviceIdentificationInfo & deviceIdInfo)
{
    CHIP_ERROR err;
    uint16_t id;
    uint16_t discriminator;

    deviceIdInfo.Init();

    err = Impl()->_GetVendorId(id);
    SuccessOrExit(err);
    deviceIdInfo.SetVendorId(id);

    err = Impl()->_GetProductId(id);
    SuccessOrExit(err);
    deviceIdInfo.SetProductId(id);

    err = Impl()->_GetSetupDiscriminator(discriminator);
    SuccessOrExit(err);
    deviceIdInfo.SetDeviceDiscriminator(discriminator);

exit:
    return err;
}

template <class ImplClass>
bool GenericConfigurationManagerImpl<ImplClass>::_IsFullyProvisioned()
{
#if CHIP_BYPASS_RENDEZVOUS
    return true;
#else // CHIP_BYPASS_RENDEZVOUS

    return
#if CHIP_DEVICE_CONFIG_ENABLE_WIFI_STATION
        ConnectivityMgr().IsWiFiStationProvisioned() &&
#endif
#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
        ConnectivityMgr().IsThreadProvisioned() &&
#endif
        true;
#endif // CHIP_BYPASS_RENDEZVOUS
}

template <class ImplClass>
bool GenericConfigurationManagerImpl<ImplClass>::_IsCommissionableDeviceTypeEnabled()
{
    return CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONABLE_DEVICE_TYPE == 1;
}

template <class ImplClass>
bool GenericConfigurationManagerImpl<ImplClass>::_IsCommissionableDeviceNameEnabled()
{
    return CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONABLE_DEVICE_NAME == 1;
}

template <class ImplClass>
CHIP_ERROR GenericConfigurationManagerImpl<ImplClass>::_GetDeviceName(char * buf, size_t bufSize)
{
    ReturnErrorCodeIf(bufSize < sizeof(CHIP_DEVICE_CONFIG_DEVICE_NAME), CHIP_ERROR_BUFFER_TOO_SMALL);
    strcpy(buf, CHIP_DEVICE_CONFIG_DEVICE_NAME);
    return CHIP_NO_ERROR;
}

template <class ImplClass>
CHIP_ERROR GenericConfigurationManagerImpl<ImplClass>::_GetInitialPairingInstruction(char * buf, size_t bufSize)
{
    ReturnErrorCodeIf(bufSize < sizeof(CHIP_DEVICE_CONFIG_PAIRING_INITIAL_INSTRUCTION), CHIP_ERROR_BUFFER_TOO_SMALL);
    strcpy(buf, CHIP_DEVICE_CONFIG_PAIRING_INITIAL_INSTRUCTION);
    return CHIP_NO_ERROR;
}

template <class ImplClass>
CHIP_ERROR GenericConfigurationManagerImpl<ImplClass>::_GetSecondaryPairingInstruction(char * buf, size_t bufSize)
{
    ReturnErrorCodeIf(bufSize < sizeof(CHIP_DEVICE_CONFIG_PAIRING_SECONDARY_INSTRUCTION), CHIP_ERROR_BUFFER_TOO_SMALL);
    strcpy(buf, CHIP_DEVICE_CONFIG_PAIRING_SECONDARY_INSTRUCTION);
    return CHIP_NO_ERROR;
}

#if !defined(NDEBUG)
template <class ImplClass>
CHIP_ERROR GenericConfigurationManagerImpl<ImplClass>::_RunUnitTests()
{
    ChipLogProgress(DeviceLayer, "Running configuration unit test");
    Impl()->RunConfigUnitTest();

    return CHIP_NO_ERROR;
}
#endif

template <class ImplClass>
void GenericConfigurationManagerImpl<ImplClass>::_LogDeviceConfig()
{
    CHIP_ERROR err;

    ChipLogProgress(DeviceLayer, "Device Configuration:");

    {
        char serialNum[ConfigurationManager::kMaxSerialNumberLength + 1];
        size_t serialNumLen;
        err = Impl()->_GetSerialNumber(serialNum, sizeof(serialNum), serialNumLen);
        ChipLogProgress(DeviceLayer, "  Serial Number: %s", (err == CHIP_NO_ERROR) ? serialNum : "(not set)");
    }

    {
        uint16_t vendorId;
        if (Impl()->_GetVendorId(vendorId) != CHIP_NO_ERROR)
        {
            vendorId = 0;
        }
        ChipLogProgress(DeviceLayer, "  Vendor Id: %" PRIu16 " (0x%" PRIX16 ")", vendorId, vendorId);
    }

    {
        uint16_t productId;
        if (Impl()->_GetProductId(productId) != CHIP_NO_ERROR)
        {
            productId = 0;
        }
        ChipLogProgress(DeviceLayer, "  Product Id: %" PRIu16 " (0x%" PRIX16 ")", productId, productId);
    }

    {
        uint16_t productRev;
        if (Impl()->_GetProductRevision(productRev) != CHIP_NO_ERROR)
        {
            productRev = 0;
        }
        ChipLogProgress(DeviceLayer, "  Product Revision: %" PRIu16, productRev);
    }

    {
        uint32_t setupPINCode;
        if (Impl()->_GetSetupPinCode(setupPINCode) != CHIP_NO_ERROR)
        {
            setupPINCode = 0;
        }
        ChipLogProgress(DeviceLayer, "  Setup Pin Code: %" PRIu32 "", setupPINCode);
    }

    {
        uint16_t setupDiscriminator;
        if (Impl()->_GetSetupDiscriminator(setupDiscriminator) != CHIP_NO_ERROR)
        {
            setupDiscriminator = 0;
        }
        ChipLogProgress(DeviceLayer, "  Setup Discriminator: %" PRIu16 " (0x%" PRIX16 ")", setupDiscriminator, setupDiscriminator);
    }

    {
        uint16_t year;
        uint8_t month, dayOfMonth;
        err = Impl()->_GetManufacturingDate(year, month, dayOfMonth);
        if (err == CHIP_NO_ERROR)
        {
            ChipLogProgress(DeviceLayer, "  Manufacturing Date: %04" PRIu16 "/%02" PRIu8 "/%02" PRIu8, year, month, dayOfMonth);
        }
        else
        {
            ChipLogProgress(DeviceLayer, "  Manufacturing Date: (not set)");
        }
    }

    {
        uint16_t deviceType;
        if (Impl()->_GetDeviceType(deviceType) != CHIP_NO_ERROR)
        {
            deviceType = 0;
        }
        ChipLogProgress(DeviceLayer, "  Device Type: %" PRIu16 " (0x%" PRIX16 ")", deviceType, deviceType);
    }
}

// Fully instantiate the generic implementation class in whatever compilation unit includes this file.
template class GenericConfigurationManagerImpl<ConfigurationManagerImpl>;

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip

#endif // GENERIC_CONFIGURATION_MANAGER_IMPL_CPP
