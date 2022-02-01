/*
 *
 *    Copyright (c) 2020-2022 Project CHIP Authors
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
#include <protocols/secure_channel/PASESession.h>

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
#include <platform/ThreadStackManager.h>
#endif

namespace chip {
namespace DeviceLayer {
namespace Internal {

template <class ConfigClass>
CHIP_ERROR GenericConfigurationManagerImpl<ConfigClass>::Init()
{
#if CHIP_ENABLE_ROTATING_DEVICE_ID
    mLifetimePersistedCounter.Init(CHIP_CONFIG_LIFETIIME_PERSISTED_COUNTER_KEY);
#endif

    return CHIP_NO_ERROR;
}

template <class ConfigClass>
CHIP_ERROR GenericConfigurationManagerImpl<ConfigClass>::GetVendorName(char * buf, size_t bufSize)
{
    ReturnErrorCodeIf(bufSize < sizeof(CHIP_DEVICE_CONFIG_DEVICE_VENDOR_NAME), CHIP_ERROR_BUFFER_TOO_SMALL);
    strcpy(buf, CHIP_DEVICE_CONFIG_DEVICE_VENDOR_NAME);
    return CHIP_NO_ERROR;
}

template <class ConfigClass>
CHIP_ERROR GenericConfigurationManagerImpl<ConfigClass>::GetProductName(char * buf, size_t bufSize)
{
    ReturnErrorCodeIf(bufSize < sizeof(CHIP_DEVICE_CONFIG_DEVICE_PRODUCT_NAME), CHIP_ERROR_BUFFER_TOO_SMALL);
    strcpy(buf, CHIP_DEVICE_CONFIG_DEVICE_PRODUCT_NAME);
    return CHIP_NO_ERROR;
}

template <class ConfigClass>
CHIP_ERROR GenericConfigurationManagerImpl<ConfigClass>::GetSoftwareVersionString(char * buf, size_t bufSize)
{
    ReturnErrorCodeIf(bufSize < sizeof(CHIP_DEVICE_CONFIG_DEVICE_SOFTWARE_VERSION_STRING), CHIP_ERROR_BUFFER_TOO_SMALL);
    strcpy(buf, CHIP_DEVICE_CONFIG_DEVICE_SOFTWARE_VERSION_STRING);
    return CHIP_NO_ERROR;
}

template <class ConfigClass>
CHIP_ERROR GenericConfigurationManagerImpl<ConfigClass>::GetSerialNumber(char * buf, size_t bufSize)
{
    CHIP_ERROR err;
    size_t serialNumLen = 0; // without counting null-terminator
    err                 = ReadConfigValueStr(ConfigClass::kConfigKey_SerialNum, buf, bufSize, serialNumLen);

#ifdef CHIP_DEVICE_CONFIG_TEST_SERIAL_NUMBER
    if (CHIP_DEVICE_CONFIG_TEST_SERIAL_NUMBER[0] != 0 && err == CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND)
    {
        ReturnErrorCodeIf(sizeof(CHIP_DEVICE_CONFIG_TEST_SERIAL_NUMBER) > bufSize, CHIP_ERROR_BUFFER_TOO_SMALL);
        memcpy(buf, CHIP_DEVICE_CONFIG_TEST_SERIAL_NUMBER, sizeof(CHIP_DEVICE_CONFIG_TEST_SERIAL_NUMBER));
        serialNumLen = sizeof(CHIP_DEVICE_CONFIG_TEST_SERIAL_NUMBER) - 1;
        err          = CHIP_NO_ERROR;
    }
#endif // CHIP_DEVICE_CONFIG_TEST_SERIAL_NUMBER

    ReturnErrorOnFailure(err);

    ReturnErrorCodeIf(serialNumLen >= bufSize, CHIP_ERROR_BUFFER_TOO_SMALL);
    ReturnErrorCodeIf(buf[serialNumLen] != 0, CHIP_ERROR_INVALID_STRING_LENGTH);

    return err;
}

template <class ConfigClass>
CHIP_ERROR GenericConfigurationManagerImpl<ConfigClass>::StoreSerialNumber(const char * serialNum, size_t serialNumLen)
{
    return WriteConfigValueStr(ConfigClass::kConfigKey_SerialNum, serialNum, serialNumLen);
}

template <class ConfigClass>
CHIP_ERROR GenericConfigurationManagerImpl<ConfigClass>::GetPrimaryWiFiMACAddress(uint8_t * buf)
{
    return CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND;
}

template <class ConfigClass>
CHIP_ERROR GenericConfigurationManagerImpl<ConfigClass>::StorePrimaryWiFiMACAddress(const uint8_t * buf)
{
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

template <class ConfigClass>
CHIP_ERROR GenericConfigurationManagerImpl<ConfigClass>::GetPrimaryMACAddress(MutableByteSpan buf)
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

template <class ConfigClass>
CHIP_ERROR GenericConfigurationManagerImpl<ConfigClass>::GetPrimary802154MACAddress(uint8_t * buf)
{
#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
    return ThreadStackManager().GetPrimary802154MACAddress(buf);
#else
    return CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND;
#endif // CHIP_DEVICE_CONFIG_ENABLE_THREAD
}

template <class ConfigClass>
CHIP_ERROR GenericConfigurationManagerImpl<ConfigClass>::StorePrimary802154MACAddress(const uint8_t * buf)
{
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

template <class ConfigClass>
inline CHIP_ERROR GenericConfigurationManagerImpl<ConfigClass>::GetHardwareVersionString(char * buf, size_t bufSize)
{
    ReturnErrorCodeIf(bufSize < sizeof(CHIP_DEVICE_CONFIG_DEFAULT_DEVICE_HARDWARE_VERSION_STRING), CHIP_ERROR_BUFFER_TOO_SMALL);
    strcpy(buf, CHIP_DEVICE_CONFIG_DEFAULT_DEVICE_HARDWARE_VERSION_STRING);
    return CHIP_NO_ERROR;
}

template <class ConfigClass>
inline CHIP_ERROR GenericConfigurationManagerImpl<ConfigClass>::GetHardwareVersion(uint16_t & hardwareVer)
{
    CHIP_ERROR err;
    uint32_t val;

    err = ReadConfigValue(ConfigClass::kConfigKey_HardwareVersion, val);
    if (err == CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND)
    {
        hardwareVer = static_cast<uint16_t>(CHIP_DEVICE_CONFIG_DEFAULT_DEVICE_HARDWARE_VERSION);
        err         = CHIP_NO_ERROR;
    }
    else
    {
        hardwareVer = static_cast<uint16_t>(val);
    }

    return err;
}

template <class ConfigClass>
inline CHIP_ERROR GenericConfigurationManagerImpl<ConfigClass>::StoreHardwareVersion(uint16_t hardwareVer)
{
    return WriteConfigValue(ConfigClass::kConfigKey_HardwareVersion, static_cast<uint32_t>(hardwareVer));
}

template <class ConfigClass>
CHIP_ERROR GenericConfigurationManagerImpl<ConfigClass>::GetManufacturingDate(uint16_t & year, uint8_t & month,
                                                                              uint8_t & dayOfMonth)
{
    CHIP_ERROR err;
    enum
    {
        kDateStringLength = 10 // YYYY-MM-DD
    };
    char dateStr[kDateStringLength + 1];
    size_t dateLen;
    char * parseEnd;

    err = ReadConfigValueStr(ConfigClass::kConfigKey_ManufacturingDate, dateStr, sizeof(dateStr), dateLen);
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

template <class ConfigClass>
CHIP_ERROR GenericConfigurationManagerImpl<ConfigClass>::StoreManufacturingDate(const char * mfgDate, size_t mfgDateLen)
{
    return WriteConfigValueStr(ConfigClass::kConfigKey_ManufacturingDate, mfgDate, mfgDateLen);
}

template <class ConfigClass>
void GenericConfigurationManagerImpl<ConfigClass>::InitiateFactoryReset()
{}

template <class ImplClass>
void GenericConfigurationManagerImpl<ImplClass>::NotifyOfAdvertisementStart()
{
#if CHIP_ENABLE_ROTATING_DEVICE_ID
    // Increment life time counter to protect against long-term tracking of rotating device ID.
    IncrementLifetimeCounter();
    // Inheriting classes should call this method so the lifetime counter is updated if necessary.
#endif
}

template <class ConfigClass>
CHIP_ERROR GenericConfigurationManagerImpl<ConfigClass>::GetSetupPinCode(uint32_t & setupPinCode)
{
    CHIP_ERROR err;

    err = ReadConfigValue(ConfigClass::kConfigKey_SetupPinCode, setupPinCode);
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

template <class ConfigClass>
CHIP_ERROR GenericConfigurationManagerImpl<ConfigClass>::StoreSetupPinCode(uint32_t setupPinCode)
{
    return WriteConfigValue(ConfigClass::kConfigKey_SetupPinCode, setupPinCode);
}

template <class ConfigClass>
CHIP_ERROR GenericConfigurationManagerImpl<ConfigClass>::GetSetupDiscriminator(uint16_t & setupDiscriminator)
{
    CHIP_ERROR err;
    uint32_t val;

    err = ReadConfigValue(ConfigClass::kConfigKey_SetupDiscriminator, val);
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

template <class ConfigClass>
CHIP_ERROR GenericConfigurationManagerImpl<ConfigClass>::StoreSetupDiscriminator(uint16_t setupDiscriminator)
{
    return WriteConfigValue(ConfigClass::kConfigKey_SetupDiscriminator, static_cast<uint32_t>(setupDiscriminator));
}

template <class ConfigClass>
CHIP_ERROR GenericConfigurationManagerImpl<ConfigClass>::GetSpake2pIterationCount(uint32_t & iterationCount)
{
    CHIP_ERROR err = ReadConfigValue(ConfigClass::kConfigKey_Spake2pIterationCount, iterationCount);

#if defined(CHIP_DEVICE_CONFIG_USE_TEST_SPAKE2P_ITERATION_COUNT) && CHIP_DEVICE_CONFIG_USE_TEST_SPAKE2P_ITERATION_COUNT
    if (err == CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND)
    {
        iterationCount = CHIP_DEVICE_CONFIG_USE_TEST_SPAKE2P_ITERATION_COUNT;
        err            = CHIP_NO_ERROR;
    }
#endif // defined(CHIP_DEVICE_CONFIG_USE_TEST_SPAKE2P_ITERATION_COUNT) && CHIP_DEVICE_CONFIG_USE_TEST_SPAKE2P_ITERATION_COUNT
    SuccessOrExit(err);

exit:
    return err;
}

template <class ConfigClass>
CHIP_ERROR GenericConfigurationManagerImpl<ConfigClass>::GetSpake2pSalt(uint8_t * buf, size_t bufSize, size_t & saltLen)
{
    CHIP_ERROR err                                                   = CHIP_NO_ERROR;
    char saltB64[BASE64_ENCODED_LEN(chip::kPBKDFMaximumSaltLen) + 1] = { 0 };
    size_t saltB64Len                                                = 0;

    err = ReadConfigValueStr(ConfigClass::kConfigKey_Spake2pSalt, saltB64, sizeof(saltB64), saltB64Len);
    err = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND;
#if defined(CHIP_DEVICE_CONFIG_USE_TEST_SPAKE2P_SALT)
    if (err == CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND)
    {
        saltB64Len = strlen(CHIP_DEVICE_CONFIG_USE_TEST_SPAKE2P_SALT);
        ReturnErrorCodeIf(saltB64Len > sizeof(saltB64), CHIP_ERROR_BUFFER_TOO_SMALL);
        memcpy(saltB64, CHIP_DEVICE_CONFIG_USE_TEST_SPAKE2P_SALT, saltB64Len);
        err = CHIP_NO_ERROR;
    }
#endif // defined(CHIP_DEVICE_CONFIG_USE_TEST_SPAKE2P_SALT)

    ReturnErrorOnFailure(err);
    saltLen = chip::Base64Decode32(saltB64, saltB64Len, reinterpret_cast<uint8_t *>(saltB64));
    ReturnErrorCodeIf(saltLen > bufSize, CHIP_ERROR_BUFFER_TOO_SMALL);
    memcpy(buf, saltB64, saltLen);

    return CHIP_NO_ERROR;
}

template <class ConfigClass>
CHIP_ERROR GenericConfigurationManagerImpl<ConfigClass>::GetSpake2pVerifier(uint8_t * buf, size_t bufSize, size_t & verifierLen)
{
    CHIP_ERROR err                                                                 = CHIP_NO_ERROR;
    char verifierB64[BASE64_ENCODED_LEN(chip::kSpake2pSerializedVerifierSize) + 1] = { 0 };
    size_t verifierB64Len                                                          = 0;

    err = ReadConfigValueStr(ConfigClass::kConfigKey_Spake2pVerifier, verifierB64, sizeof(verifierB64), verifierB64Len);
    err = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND;
#if defined(CHIP_DEVICE_CONFIG_USE_TEST_SPAKE2P_VERIFIER)
    if (err == CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND)
    {
        verifierB64Len = strlen(CHIP_DEVICE_CONFIG_USE_TEST_SPAKE2P_VERIFIER);
        ReturnErrorCodeIf(verifierB64Len > sizeof(verifierB64), CHIP_ERROR_BUFFER_TOO_SMALL);
        memcpy(verifierB64, CHIP_DEVICE_CONFIG_USE_TEST_SPAKE2P_VERIFIER, verifierB64Len);
        err = CHIP_NO_ERROR;
    }
#endif // defined(CHIP_DEVICE_CONFIG_USE_TEST_SPAKE2P_VERIFIER)

    ReturnErrorOnFailure(err);
    verifierLen = chip::Base64Decode32(verifierB64, verifierB64Len, reinterpret_cast<uint8_t *>(verifierB64));
    ReturnErrorCodeIf(verifierLen > bufSize, CHIP_ERROR_BUFFER_TOO_SMALL);
    memcpy(buf, verifierB64, verifierLen);

    return err;
}

template <class ConfigClass>
CHIP_ERROR GenericConfigurationManagerImpl<ConfigClass>::GetRegulatoryLocation(uint8_t & location)
{
    return GetLocationCapability(location);
}

template <class ConfigClass>
CHIP_ERROR GenericConfigurationManagerImpl<ConfigClass>::StoreRegulatoryLocation(uint8_t location)
{
    uint32_t value = location;
    return WriteConfigValue(ConfigClass::kConfigKey_RegulatoryLocation, value);
}

template <class ConfigClass>
CHIP_ERROR GenericConfigurationManagerImpl<ConfigClass>::GetCountryCode(char * buf, size_t bufSize, size_t & codeLen)
{
    return ReadConfigValueStr(ConfigClass::kConfigKey_CountryCode, buf, bufSize, codeLen);
}

template <class ConfigClass>
CHIP_ERROR GenericConfigurationManagerImpl<ConfigClass>::StoreCountryCode(const char * code, size_t codeLen)
{
    return WriteConfigValueStr(ConfigClass::kConfigKey_CountryCode, code, codeLen);
}

template <class ConfigClass>
CHIP_ERROR GenericConfigurationManagerImpl<ConfigClass>::GetBreadcrumb(uint64_t & breadcrumb)
{
    return ReadConfigValue(ConfigClass::kConfigKey_Breadcrumb, breadcrumb);
}

template <class ConfigClass>
CHIP_ERROR GenericConfigurationManagerImpl<ConfigClass>::StoreBreadcrumb(uint64_t breadcrumb)
{
    return WriteConfigValue(ConfigClass::kConfigKey_Breadcrumb, breadcrumb);
}

template <class ImplClass>
CHIP_ERROR GenericConfigurationManagerImpl<ImplClass>::GetRebootCount(uint32_t & rebootCount)
{
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

template <class ImplClass>
CHIP_ERROR GenericConfigurationManagerImpl<ImplClass>::StoreRebootCount(uint32_t rebootCount)
{
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

template <class ImplClass>
CHIP_ERROR GenericConfigurationManagerImpl<ImplClass>::GetTotalOperationalHours(uint32_t & totalOperationalHours)
{
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

template <class ImplClass>
CHIP_ERROR GenericConfigurationManagerImpl<ImplClass>::StoreTotalOperationalHours(uint32_t totalOperationalHours)
{
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

template <class ImplClass>
CHIP_ERROR GenericConfigurationManagerImpl<ImplClass>::GetBootReason(uint32_t & bootReason)
{
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

template <class ImplClass>
CHIP_ERROR GenericConfigurationManagerImpl<ImplClass>::StoreBootReason(uint32_t bootReason)
{
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

template <class ConfigClass>
CHIP_ERROR GenericConfigurationManagerImpl<ConfigClass>::GetNodeLabel(char * buf, size_t bufSize)
{
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

template <class ConfigClass>
CHIP_ERROR GenericConfigurationManagerImpl<ConfigClass>::StoreNodeLabel(const char * buf, size_t bufSize)
{
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

template <class ConfigClass>
CHIP_ERROR GenericConfigurationManagerImpl<ConfigClass>::GetPartNumber(char * buf, size_t bufSize)
{
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

template <class ConfigClass>
CHIP_ERROR GenericConfigurationManagerImpl<ConfigClass>::GetProductURL(char * buf, size_t bufSize)
{
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

template <class ConfigClass>
CHIP_ERROR GenericConfigurationManagerImpl<ConfigClass>::GetProductLabel(char * buf, size_t bufSize)
{
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

template <class ConfigClass>
CHIP_ERROR GenericConfigurationManagerImpl<ConfigClass>::GetLocalConfigDisabled(bool & disabled)
{
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

template <class ConfigClass>
CHIP_ERROR GenericConfigurationManagerImpl<ConfigClass>::GetReachable(bool & reachable)
{
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

template <class ConfigClass>
CHIP_ERROR GenericConfigurationManagerImpl<ConfigClass>::GetUniqueId(char * buf, size_t bufSize)
{
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

template <class ConfigClass>
CHIP_ERROR GenericConfigurationManagerImpl<ConfigClass>::GetLifetimeCounter(uint16_t & lifetimeCounter)
{
#if CHIP_ENABLE_ROTATING_DEVICE_ID
    lifetimeCounter = static_cast<uint16_t>(mLifetimePersistedCounter.GetValue());
    return CHIP_NO_ERROR;
#else
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
#endif
}

template <class ConfigClass>
CHIP_ERROR GenericConfigurationManagerImpl<ConfigClass>::IncrementLifetimeCounter()
{
#if CHIP_ENABLE_ROTATING_DEVICE_ID
    return mLifetimePersistedCounter.Advance();
#else
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
#endif
}

template <class ConfigClass>
CHIP_ERROR GenericConfigurationManagerImpl<ConfigClass>::GetFailSafeArmed(bool & val)
{
    return ReadConfigValue(ConfigClass::kConfigKey_FailSafeArmed, val);
}

template <class ConfigClass>
CHIP_ERROR GenericConfigurationManagerImpl<ConfigClass>::SetFailSafeArmed(bool val)
{
    return WriteConfigValue(ConfigClass::kConfigKey_FailSafeArmed, val);
}

template <class ConfigClass>
CHIP_ERROR
GenericConfigurationManagerImpl<ConfigClass>::GetBLEDeviceIdentificationInfo(Ble::ChipBLEDeviceIdentificationInfo & deviceIdInfo)
{
    CHIP_ERROR err;
    uint16_t id;
    uint16_t discriminator;

    deviceIdInfo.Init();

    err = GetVendorId(id);
    SuccessOrExit(err);
    deviceIdInfo.SetVendorId(id);

    err = GetProductId(id);
    SuccessOrExit(err);
    deviceIdInfo.SetProductId(id);

    err = GetSetupDiscriminator(discriminator);
    SuccessOrExit(err);
    deviceIdInfo.SetDeviceDiscriminator(discriminator);

#if CHIP_ENABLE_ADDITIONAL_DATA_ADVERTISING
    deviceIdInfo.SetAdditionalDataFlag(true);
#endif

exit:
    return err;
}

template <class ConfigClass>
bool GenericConfigurationManagerImpl<ConfigClass>::IsFullyProvisioned()
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

template <class ConfigClass>
bool GenericConfigurationManagerImpl<ConfigClass>::IsCommissionableDeviceTypeEnabled()
{
#if CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONABLE_DEVICE_TYPE
    return true;
#else
    return false;
#endif
}

template <class ConfigClass>
bool GenericConfigurationManagerImpl<ConfigClass>::IsCommissionableDeviceNameEnabled()
{
    return CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONABLE_DEVICE_NAME == 1;
}

template <class ConfigClass>
CHIP_ERROR GenericConfigurationManagerImpl<ConfigClass>::GetCommissionableDeviceName(char * buf, size_t bufSize)
{
    ReturnErrorCodeIf(bufSize < sizeof(CHIP_DEVICE_CONFIG_DEVICE_NAME), CHIP_ERROR_BUFFER_TOO_SMALL);
    strcpy(buf, CHIP_DEVICE_CONFIG_DEVICE_NAME);
    return CHIP_NO_ERROR;
}

template <class ConfigClass>
CHIP_ERROR GenericConfigurationManagerImpl<ConfigClass>::GetInitialPairingInstruction(char * buf, size_t bufSize)
{
    ReturnErrorCodeIf(bufSize < sizeof(CHIP_DEVICE_CONFIG_PAIRING_INITIAL_INSTRUCTION), CHIP_ERROR_BUFFER_TOO_SMALL);
    strcpy(buf, CHIP_DEVICE_CONFIG_PAIRING_INITIAL_INSTRUCTION);
    return CHIP_NO_ERROR;
}

template <class ConfigClass>
CHIP_ERROR GenericConfigurationManagerImpl<ConfigClass>::GetSecondaryPairingInstruction(char * buf, size_t bufSize)
{
    ReturnErrorCodeIf(bufSize < sizeof(CHIP_DEVICE_CONFIG_PAIRING_SECONDARY_INSTRUCTION), CHIP_ERROR_BUFFER_TOO_SMALL);
    strcpy(buf, CHIP_DEVICE_CONFIG_PAIRING_SECONDARY_INSTRUCTION);
    return CHIP_NO_ERROR;
}

template <class ConfigClass>
CHIP_ERROR GenericConfigurationManagerImpl<ConfigClass>::RunUnitTests()
{
#if !defined(NDEBUG)
    ChipLogProgress(DeviceLayer, "Running configuration unit test");
    RunConfigUnitTest();
#endif
    return CHIP_NO_ERROR;
}

template <class ConfigClass>
void GenericConfigurationManagerImpl<ConfigClass>::LogDeviceConfig()
{
    CHIP_ERROR err;

    ChipLogProgress(DeviceLayer, "Device Configuration:");

    {
        char serialNum[ConfigurationManager::kMaxSerialNumberLength + 1];
        err = GetSerialNumber(serialNum, sizeof(serialNum));
        ChipLogProgress(DeviceLayer, "  Serial Number: %s", (err == CHIP_NO_ERROR) ? serialNum : "(not set)");
    }

    {
        uint16_t vendorId;
        if (GetVendorId(vendorId) != CHIP_NO_ERROR)
        {
            vendorId = 0;
        }
        ChipLogProgress(DeviceLayer, "  Vendor Id: %" PRIu16 " (0x%" PRIX16 ")", vendorId, vendorId);
    }

    {
        uint16_t productId;
        if (GetProductId(productId) != CHIP_NO_ERROR)
        {
            productId = 0;
        }
        ChipLogProgress(DeviceLayer, "  Product Id: %" PRIu16 " (0x%" PRIX16 ")", productId, productId);
    }

    {
        uint16_t hardwareVer;
        if (GetHardwareVersion(hardwareVer) != CHIP_NO_ERROR)
        {
            hardwareVer = 0;
        }
        ChipLogProgress(DeviceLayer, "  Hardware Version: %" PRIu16, hardwareVer);
    }

    {
        uint32_t setupPINCode;
        if (GetSetupPinCode(setupPINCode) != CHIP_NO_ERROR)
        {
            setupPINCode = 0;
        }
        ChipLogProgress(DeviceLayer, "  Setup Pin Code: %" PRIu32 "", setupPINCode);
    }

    {
        uint16_t setupDiscriminator;
        if (GetSetupDiscriminator(setupDiscriminator) != CHIP_NO_ERROR)
        {
            setupDiscriminator = 0;
        }
        ChipLogProgress(DeviceLayer, "  Setup Discriminator: %" PRIu16 " (0x%" PRIX16 ")", setupDiscriminator, setupDiscriminator);
    }

    {
        uint16_t year;
        uint8_t month, dayOfMonth;
        err = GetManufacturingDate(year, month, dayOfMonth);
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
        if (GetDeviceTypeId(deviceType) != CHIP_NO_ERROR)
        {
            deviceType = 0;
        }
        ChipLogProgress(DeviceLayer, "  Device Type: %" PRIu16 " (0x%" PRIX16 ")", deviceType, deviceType);
    }
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip

#endif // GENERIC_CONFIGURATION_MANAGER_IMPL_CPP
