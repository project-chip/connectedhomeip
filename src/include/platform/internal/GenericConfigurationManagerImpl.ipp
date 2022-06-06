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
#include <crypto/CHIPCryptoPAL.h>
#include <crypto/RandUtils.h>
#include <inttypes.h>
#include <lib/core/CHIPConfig.h>
#include <lib/support/Base64.h>
#include <lib/support/BytesToHex.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/ScopedBuffer.h>
#include <platform/CommissionableDataProvider.h>
#include <platform/DeviceControlServer.h>
#include <platform/DeviceInstanceInfoProvider.h>
#include <platform/BuildTime.h>
#include <platform/internal/CHIPDeviceLayerInternal.h>
#include <platform/internal/GenericConfigurationManagerImpl.h>

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
#include <platform/ThreadStackManager.h>
#endif

// TODO : may be we can make it configurable
#define BLE_ADVERTISEMENT_VERSION 0

namespace chip {
namespace DeviceLayer {
namespace Internal {

static Optional<System::Clock::Seconds32> sFirmwareBuildChipEpochTime;

#if CHIP_USE_TRANSITIONAL_DEVICE_INSTANCE_INFO_PROVIDER
template <class ConfigClass>
class LegacyDeviceInstanceInfoProvider : public DeviceInstanceInfoProvider
{

public:
    // GenericConfigurationManagerImpl will own a LegacyDeviceInstanceInfoProvider which
    // *refers back to that GenericConfigurationManagerImpl*, due to how CRTP-based
    // storage APIs are defined. This is a bit unclean, but only applicable to the
    // transition path when `CHIP_USE_TRANSITIONAL_DEVICE_INSTANCE_INFO_PROVIDER` is true.
    // This circular dependency is NOT needed by DeviceInstanceInfoProvider, but required
    // to keep legacy code running.
    LegacyDeviceInstanceInfoProvider(GenericConfigurationManagerImpl<ConfigClass> & configManager) :
        mGenericConfigManager(configManager)
    {}

    CHIP_ERROR GetSerialNumber(char * buf, size_t bufSize) override;
    CHIP_ERROR GetManufacturingDate(uint16_t & year, uint8_t & month, uint8_t & day) override;
    CHIP_ERROR GetHardwareVersion(uint16_t & hardwareVersion) override;
    CHIP_ERROR GetHardwareVersionString(char * buf, size_t bufSize) override;
    CHIP_ERROR GetRotatingDeviceIdUniqueId(MutableByteSpan & uniqueIdSpan) override;

private:
    GenericConfigurationManagerImpl<ConfigClass> & mGenericConfigManager;
};

template <class ConfigClass>
CHIP_ERROR LegacyDeviceInstanceInfoProvider<ConfigClass>::GetSerialNumber(char * buf, size_t bufSize)
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
    }
#endif // CHIP_DEVICE_CONFIG_TEST_SERIAL_NUMBER
    ReturnErrorOnFailure(err);

    ReturnErrorCodeIf(serialNumLen >= bufSize, CHIP_ERROR_BUFFER_TOO_SMALL);
    ReturnErrorCodeIf(buf[serialNumLen] != 0, CHIP_ERROR_INVALID_STRING_LENGTH);

    return err;
}

template <class ConfigClass>
CHIP_ERROR LegacyDeviceInstanceInfoProvider<ConfigClass>::GetManufacturingDate(uint16_t & year, uint8_t & month, uint8_t & day)
{
#if CHIP_DEVICE_LAYER_TARGET_FAKE
    return CHIP_ERROR_NOT_IMPLEMENTED;
#else

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
#endif
}

template <class ConfigClass>
CHIP_ERROR LegacyDeviceInstanceInfoProvider<ConfigClass>::GetHardwareVersion(uint16_t & hardwareVersion)
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
CHIP_ERROR LegacyDeviceInstanceInfoProvider<ConfigClass>::GetHardwareVersionString(char * buf, size_t bufSize)
{
#if CHIP_DEVICE_LAYER_TARGET_ANDROID
    CHIP_ERROR err;
    size_t hardwareVersionLen = 0; // without counting null-terminator
    err = ConfigClass::ReadConfigValueStr(ConfigClass::kConfigKey_HardwareVersionString, buf, bufSize, hardwareVersionLen);
    if (err == CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND)
    {
        ReturnErrorCodeIf(bufSize < sizeof(CHIP_DEVICE_CONFIG_DEFAULT_DEVICE_HARDWARE_VERSION_STRING), CHIP_ERROR_BUFFER_TOO_SMALL);
        strcpy(buf, CHIP_DEVICE_CONFIG_DEFAULT_DEVICE_HARDWARE_VERSION_STRING);
    }

    return err;
#else
    ReturnErrorCodeIf(bufSize < sizeof(CHIP_DEVICE_CONFIG_DEFAULT_DEVICE_HARDWARE_VERSION_STRING), CHIP_ERROR_BUFFER_TOO_SMALL);
    strcpy(buf, CHIP_DEVICE_CONFIG_DEFAULT_DEVICE_HARDWARE_VERSION_STRING);
    return CHIP_NO_ERROR;
#endif
}

template <class ConfigClass>
CHIP_ERROR LegacyDeviceInstanceInfoProvider<ConfigClass>::GetRotatingDeviceIdUniqueId(MutableByteSpan & uniqueIdSpan)
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
#endif // CHIP_USE_TRANSITIONAL_DEVICE_INSTANCE_INFO_PROVIDER

#if CHIP_USE_TRANSITIONAL_COMMISSIONABLE_DATA_PROVIDER

// Legacy version of CommissionableDataProvider used for a grace period
// to a transition where all ConfigurationManager customers move to
// provide their own impl of CommissionableDataProvider interface.

template <class ConfigClass>
class LegacyTemporaryCommissionableDataProvider : public CommissionableDataProvider
{
public:
    // GenericConfigurationManagerImpl will own a LegacyTemporaryCommissionableDataProvider which
    // *refers back to that GenericConfigurationManagerImpl*, due to how CRTP-based
    // storage APIs are defined. This is a bit unclean, but only applicable to the
    // transition path when `CHIP_USE_TRANSITIONAL_COMMISSIONABLE_DATA_PROVIDER` is true.
    // This circular dependency is NOT needed by CommissionableDataProvider, but required
    // to keep legacy code running.
    LegacyTemporaryCommissionableDataProvider(GenericConfigurationManagerImpl<ConfigClass> & configManager) :
        mGenericConfigManager(configManager)
    {}

    CHIP_ERROR GetSetupDiscriminator(uint16_t & setupDiscriminator) override;
    CHIP_ERROR SetSetupDiscriminator(uint16_t setupDiscriminator) override;
    CHIP_ERROR GetSpake2pIterationCount(uint32_t & iterationCount) override;
    CHIP_ERROR GetSpake2pSalt(MutableByteSpan & saltBuf) override;
    CHIP_ERROR GetSpake2pVerifier(MutableByteSpan & verifierBuf, size_t & outVerifierLen) override;
    CHIP_ERROR GetSetupPasscode(uint32_t & setupPasscode) override;
    CHIP_ERROR SetSetupPasscode(uint32_t setupPasscode) override;

private:
    GenericConfigurationManagerImpl<ConfigClass> & mGenericConfigManager;
};

template <class ConfigClass>
CHIP_ERROR LegacyTemporaryCommissionableDataProvider<ConfigClass>::GetSetupPasscode(uint32_t & setupPasscode)
{
    CHIP_ERROR err;

    err = mGenericConfigManager.ReadConfigValue(ConfigClass::kConfigKey_SetupPinCode, setupPasscode);
#if defined(CHIP_DEVICE_CONFIG_USE_TEST_SETUP_PIN_CODE) && CHIP_DEVICE_CONFIG_USE_TEST_SETUP_PIN_CODE
    if (err == CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND)
    {
        setupPasscode = CHIP_DEVICE_CONFIG_USE_TEST_SETUP_PIN_CODE;
        err           = CHIP_NO_ERROR;
    }
#endif // defined(CHIP_DEVICE_CONFIG_USE_TEST_SETUP_PIN_CODE) && CHIP_DEVICE_CONFIG_USE_TEST_SETUP_PIN_CODE
    SuccessOrExit(err);

exit:
    return err;
}

template <class ConfigClass>
CHIP_ERROR LegacyTemporaryCommissionableDataProvider<ConfigClass>::SetSetupPasscode(uint32_t setupPasscode)
{
    return mGenericConfigManager.WriteConfigValue(ConfigClass::kConfigKey_SetupPinCode, setupPasscode);
}

template <class ConfigClass>
CHIP_ERROR LegacyTemporaryCommissionableDataProvider<ConfigClass>::GetSetupDiscriminator(uint16_t & setupDiscriminator)
{
    CHIP_ERROR err;
    uint32_t val;

    err = mGenericConfigManager.ReadConfigValue(ConfigClass::kConfigKey_SetupDiscriminator, val);
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
CHIP_ERROR LegacyTemporaryCommissionableDataProvider<ConfigClass>::SetSetupDiscriminator(uint16_t setupDiscriminator)
{
    return mGenericConfigManager.WriteConfigValue(ConfigClass::kConfigKey_SetupDiscriminator,
                                                  static_cast<uint32_t>(setupDiscriminator));
}

template <class ConfigClass>
CHIP_ERROR LegacyTemporaryCommissionableDataProvider<ConfigClass>::GetSpake2pIterationCount(uint32_t & iterationCount)
{
    CHIP_ERROR err = mGenericConfigManager.ReadConfigValue(ConfigClass::kConfigKey_Spake2pIterationCount, iterationCount);

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
CHIP_ERROR LegacyTemporaryCommissionableDataProvider<ConfigClass>::GetSpake2pSalt(MutableByteSpan & saltBuf)
{
    static constexpr size_t kSpake2pSalt_MaxBase64Len = BASE64_ENCODED_LEN(chip::Crypto::kSpake2p_Max_PBKDF_Salt_Length) + 1;

    CHIP_ERROR err                          = CHIP_NO_ERROR;
    char saltB64[kSpake2pSalt_MaxBase64Len] = { 0 };
    size_t saltB64Len                       = 0;

    err = mGenericConfigManager.ReadConfigValueStr(ConfigClass::kConfigKey_Spake2pSalt, saltB64, sizeof(saltB64), saltB64Len);

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
    size_t saltLen = chip::Base64Decode32(saltB64, saltB64Len, reinterpret_cast<uint8_t *>(saltB64));

    ReturnErrorCodeIf(saltLen > saltBuf.size(), CHIP_ERROR_BUFFER_TOO_SMALL);
    memcpy(saltBuf.data(), saltB64, saltLen);
    saltBuf.reduce_size(saltLen);

    return CHIP_NO_ERROR;
}

template <class ConfigClass>
CHIP_ERROR LegacyTemporaryCommissionableDataProvider<ConfigClass>::GetSpake2pVerifier(MutableByteSpan & verifierBuf,
                                                                                      size_t & verifierLen)
{
    static constexpr size_t kSpake2pSerializedVerifier_MaxBase64Len =
        BASE64_ENCODED_LEN(chip::Crypto::kSpake2p_VerifierSerialized_Length) + 1;

    CHIP_ERROR err                                            = CHIP_NO_ERROR;
    char verifierB64[kSpake2pSerializedVerifier_MaxBase64Len] = { 0 };
    size_t verifierB64Len                                     = 0;

    err = mGenericConfigManager.ReadConfigValueStr(ConfigClass::kConfigKey_Spake2pVerifier, verifierB64, sizeof(verifierB64),
                                                   verifierB64Len);

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
    ReturnErrorCodeIf(verifierLen > verifierBuf.size(), CHIP_ERROR_BUFFER_TOO_SMALL);
    memcpy(verifierBuf.data(), verifierB64, verifierLen);
    verifierBuf.reduce_size(verifierLen);

    return err;
}

#endif // CHIP_USE_TRANSITIONAL_COMMISSIONABLE_DATA_PROVIDER

template <class ConfigClass>
CHIP_ERROR GenericConfigurationManagerImpl<ConfigClass>::Init()
{
    CHIP_ERROR err = CHIP_NO_ERROR;

#if CHIP_ENABLE_ROTATING_DEVICE_ID && defined(CHIP_DEVICE_CONFIG_ROTATING_DEVICE_ID_UNIQUE_ID)
    mLifetimePersistedCounter.Init(CHIP_CONFIG_LIFETIIME_PERSISTED_COUNTER_KEY);
#endif

#if CHIP_USE_TRANSITIONAL_DEVICE_INSTANCE_INFO_PROVIDER
    // Using a temporary singleton here because the overall GenericConfigurationManagerImpl is
    // a singleton. This is TEMPORARY code to set the table for clients to set their own
    // implementation properly, without loss of functionality for legacy in the meantime.
    static LegacyDeviceInstanceInfoProvider<ConfigClass> sLegacyDeviceInstanceInfoProvider(*this);

    SetDeviceInstanceInfoProvider(&sLegacyDeviceInstanceInfoProvider);
#endif

#if CHIP_USE_TRANSITIONAL_COMMISSIONABLE_DATA_PROVIDER
    // Using a temporary singleton here because the overall GenericConfigurationManagerImpl is
    // a singleton. This is TEMPORARY code to set the table for clients to set their own
    // implementation properly, without loss of functionality for legacy in the meantime.
    static LegacyTemporaryCommissionableDataProvider<ConfigClass> sLegacyTemporaryCommissionableDataProvider(*this);

    SetCommissionableDataProvider(&sLegacyTemporaryCommissionableDataProvider);
#endif

    char uniqueId[kMaxUniqueIDLength + 1];

    // Generate Unique ID only if it is not present in the storage.
    if (GetUniqueId(uniqueId, sizeof(uniqueId)) != CHIP_NO_ERROR)
    {
        ReturnErrorOnFailure(GenerateUniqueId(uniqueId, sizeof(uniqueId)));
        ReturnErrorOnFailure(StoreUniqueId(uniqueId, strlen(uniqueId)));
    }

    bool failSafeArmed = false;

    // If the fail-safe was armed when the device last shutdown, initiate cleanup based on the pending Fail Safe Context with
    // which the fail-safe timer was armed.
    if (GetFailSafeArmed(failSafeArmed) == CHIP_NO_ERROR && failSafeArmed)
    {
        FabricIndex fabricIndex;
        bool addNocCommandInvoked;
        bool updateNocCommandInvoked;

        ChipLogProgress(DeviceLayer, "Detected fail-safe armed on reboot");

        err = FailSafeContext::LoadFromStorage(fabricIndex, addNocCommandInvoked, updateNocCommandInvoked);
        if (err == CHIP_NO_ERROR)
        {
            DeviceControlServer::DeviceControlSvr().GetFailSafeContext().ScheduleFailSafeCleanup(fabricIndex, addNocCommandInvoked,
                                                                                                 updateNocCommandInvoked);
        }
        else
        {
            // This should not happen, but we should not fail system init based on it!
            ChipLogError(DeviceLayer, "Failed to load fail-safe context from storage (err= %" CHIP_ERROR_FORMAT "), cleaning-up!",
                         err.Format());
            (void) SetFailSafeArmed(false);
            err = CHIP_NO_ERROR;
        }
    }

    return err;
}

template <class ConfigClass>
CHIP_ERROR GenericConfigurationManagerImpl<ConfigClass>::GetVendorId(uint16_t & vendorId)
{
    vendorId = static_cast<uint16_t>(CHIP_DEVICE_CONFIG_DEVICE_VENDOR_ID);
    return CHIP_NO_ERROR;
}

template <class ConfigClass>
CHIP_ERROR GenericConfigurationManagerImpl<ConfigClass>::GetProductId(uint16_t & productId)
{
    productId = static_cast<uint16_t>(CHIP_DEVICE_CONFIG_DEVICE_PRODUCT_ID);
    return CHIP_NO_ERROR;
}

template <class ConfigClass>
CHIP_ERROR GenericConfigurationManagerImpl<ConfigClass>::GetSoftwareVersion(uint32_t & softwareVer)
{
    softwareVer = static_cast<uint32_t>(CHIP_DEVICE_CONFIG_DEVICE_SOFTWARE_VERSION);
    return CHIP_NO_ERROR;
}

template <class ConfigClass>
inline CHIP_ERROR GenericConfigurationManagerImpl<ConfigClass>::StoreSoftwareVersion(uint32_t softwareVer)
{
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

template <class ConfigClass>
CHIP_ERROR GenericConfigurationManagerImpl<ConfigClass>::GetFirmwareBuildChipEpochTime(System::Clock::Seconds32 & chipEpochTime)
{
    // If the setter was called and we have a value in memory, return this.
    if (sFirmwareBuildChipEpochTime.HasValue())
    {
        chipEpochTime = sFirmwareBuildChipEpochTime.Value();
        return CHIP_NO_ERROR;
    }
    // Else, attempt to read the hard-coded values.
    VerifyOrReturnError(!BUILD_DATE_IS_BAD(CHIP_DEVICE_CONFIG_FIRMWARE_BUILD_DATE), CHIP_ERROR_INTERNAL);
    VerifyOrReturnError(!BUILD_TIME_IS_BAD(CHIP_DEVICE_CONFIG_FIRMWARE_BUILD_TIME), CHIP_ERROR_INTERNAL);
    const char * date = CHIP_DEVICE_CONFIG_FIRMWARE_BUILD_DATE;
    const char * time = CHIP_DEVICE_CONFIG_FIRMWARE_BUILD_TIME;
    uint32_t seconds;
    auto good = CalendarToChipEpochTime(COMPUTE_BUILD_YEAR(date), COMPUTE_BUILD_MONTH(date), COMPUTE_BUILD_DAY(date), COMPUTE_BUILD_HOUR(time), COMPUTE_BUILD_MIN(time), COMPUTE_BUILD_SEC(time), seconds);
    if (good)
    {
        chipEpochTime = chip::System::Clock::Seconds32(seconds);
    }
    return good ? CHIP_NO_ERROR : CHIP_ERROR_INVALID_ARGUMENT;
}

template <class ConfigClass>
CHIP_ERROR GenericConfigurationManagerImpl<ConfigClass>::SetFirmwareBuildChipEpochTime(System::Clock::Seconds32 chipEpochTime)
{
    // The setter is sticky in that once the hard-coded time is overriden, it
    // will be for the lifetime of the configuration manager singleton.
    // However, this is not persistent across boots.
    //
    // Implementations that can't use the hard-coded time for whatever reason
    // should set this at each init.
    sFirmwareBuildChipEpochTime.SetValue(chipEpochTime);
    return CHIP_NO_ERROR;
}

template <class ConfigClass>
CHIP_ERROR GenericConfigurationManagerImpl<ConfigClass>::GetDeviceTypeId(uint32_t & deviceType)
{
    deviceType = static_cast<uint32_t>(CHIP_DEVICE_CONFIG_DEVICE_TYPE);
    return CHIP_NO_ERROR;
}

template <class ConfigClass>
CHIP_ERROR GenericConfigurationManagerImpl<ConfigClass>::GetInitialPairingHint(uint16_t & pairingHint)
{
    pairingHint = static_cast<uint16_t>(CHIP_DEVICE_CONFIG_PAIRING_INITIAL_HINT);
    return CHIP_NO_ERROR;
}

template <class ConfigClass>
CHIP_ERROR GenericConfigurationManagerImpl<ConfigClass>::GetSecondaryPairingHint(uint16_t & pairingHint)
{
    pairingHint = static_cast<uint16_t>(CHIP_DEVICE_CONFIG_PAIRING_SECONDARY_HINT);
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
#endif

    if (chip::DeviceLayer::ConfigurationMgr().GetPrimaryWiFiMACAddress(buf.data()) == CHIP_NO_ERROR)
    {
        ChipLogDetail(DeviceLayer, "Using wifi MAC for hostname");
        return CHIP_NO_ERROR;
    }

    return CHIP_ERROR_NOT_FOUND;
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
inline CHIP_ERROR GenericConfigurationManagerImpl<ConfigClass>::StoreHardwareVersion(uint16_t hardwareVer)
{
    return WriteConfigValue(ConfigClass::kConfigKey_HardwareVersion, static_cast<uint32_t>(hardwareVer));
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
#if CHIP_ENABLE_ROTATING_DEVICE_ID && defined(CHIP_DEVICE_CONFIG_ROTATING_DEVICE_ID_UNIQUE_ID)
    // Increment life time counter to protect against long-term tracking of rotating device ID.
    IncrementLifetimeCounter();
    // Inheriting classes should call this method so the lifetime counter is updated if necessary.
#endif
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
CHIP_ERROR GenericConfigurationManagerImpl<ConfigClass>::GetUniqueId(char * buf, size_t bufSize)
{
    CHIP_ERROR err;
    size_t uniqueIdLen = 0; // without counting null-terminator
    err                = ReadConfigValueStr(ConfigClass::kConfigKey_UniqueId, buf, bufSize, uniqueIdLen);

    ReturnErrorOnFailure(err);

    ReturnErrorCodeIf(uniqueIdLen >= bufSize, CHIP_ERROR_BUFFER_TOO_SMALL);
    ReturnErrorCodeIf(buf[uniqueIdLen] != 0, CHIP_ERROR_INVALID_STRING_LENGTH);

    return err;
}

template <class ConfigClass>
CHIP_ERROR GenericConfigurationManagerImpl<ConfigClass>::StoreUniqueId(const char * uniqueId, size_t uniqueIdLen)
{
    return WriteConfigValueStr(ConfigClass::kConfigKey_UniqueId, uniqueId, uniqueIdLen);
}

template <class ConfigClass>
CHIP_ERROR GenericConfigurationManagerImpl<ConfigClass>::GenerateUniqueId(char * buf, size_t bufSize)
{
    uint64_t randomUniqueId = Crypto::GetRandU64();
    return Encoding::BytesToUppercaseHexString(reinterpret_cast<uint8_t *>(&randomUniqueId), sizeof(uint64_t), buf, bufSize);
}

#if CHIP_ENABLE_ROTATING_DEVICE_ID && defined(CHIP_DEVICE_CONFIG_ROTATING_DEVICE_ID_UNIQUE_ID)
template <class ConfigClass>
CHIP_ERROR GenericConfigurationManagerImpl<ConfigClass>::GetLifetimeCounter(uint16_t & lifetimeCounter)
{
    lifetimeCounter = static_cast<uint16_t>(mLifetimePersistedCounter.GetValue());
    return CHIP_NO_ERROR;
}

template <class ConfigClass>
CHIP_ERROR GenericConfigurationManagerImpl<ConfigClass>::IncrementLifetimeCounter()
{
    return mLifetimePersistedCounter.Advance();
}

template <class ConfigClass>
CHIP_ERROR GenericConfigurationManagerImpl<ConfigClass>::SetRotatingDeviceIdUniqueId(const ByteSpan & uniqueIdSpan)
{
    ReturnErrorCodeIf(uniqueIdSpan.size() != kRotatingDeviceIDUniqueIDLength, CHIP_ERROR_BUFFER_TOO_SMALL);
    memcpy(mRotatingDeviceIdUniqueId, uniqueIdSpan.data(), kRotatingDeviceIDUniqueIDLength);
    return CHIP_NO_ERROR;
}

#endif // CHIP_ENABLE_ROTATING_DEVICE_ID

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

    err = GetCommissionableDataProvider()->GetSetupDiscriminator(discriminator);
    SuccessOrExit(err);
    deviceIdInfo.SetDeviceDiscriminator(discriminator);

    deviceIdInfo.SetAdvertisementVersion(BLE_ADVERTISEMENT_VERSION);

#if CHIP_ENABLE_ADDITIONAL_DATA_ADVERTISING
    deviceIdInfo.SetAdditionalDataFlag(true);
#endif

exit:
    return err;
}

template <class ConfigClass>
bool GenericConfigurationManagerImpl<ConfigClass>::IsFullyProvisioned()
{
    return
#if CHIP_DEVICE_CONFIG_ENABLE_WIFI_STATION
        ConnectivityMgr().IsWiFiStationProvisioned() &&
#endif
#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
        ConnectivityMgr().IsThreadProvisioned() &&
#endif
        true;
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

    DeviceInstanceInfoProvider * deviceInstanceInfoProvider = GetDeviceInstanceInfoProvider();

    {
        char serialNum[ConfigurationManager::kMaxSerialNumberLength + 1];
        err = deviceInstanceInfoProvider->GetSerialNumber(serialNum, sizeof(serialNum));
        ChipLogProgress(DeviceLayer, "  Serial Number: %s", (err == CHIP_NO_ERROR) ? serialNum : "(not set)");
    }

    {
        uint16_t vendorId;
        if (GetVendorId(vendorId) != CHIP_NO_ERROR)
        {
            vendorId = 0;
        }
        ChipLogProgress(DeviceLayer, "  Vendor Id: %u (0x%X)", vendorId, vendorId);
    }

    {
        uint16_t productId;
        if (GetProductId(productId) != CHIP_NO_ERROR)
        {
            productId = 0;
        }
        ChipLogProgress(DeviceLayer, "  Product Id: %u (0x%X)", productId, productId);
    }

    {
        uint16_t hardwareVer;
        if (deviceInstanceInfoProvider->GetHardwareVersion(hardwareVer) != CHIP_NO_ERROR)
        {
            hardwareVer = 0;
        }
        ChipLogProgress(DeviceLayer, "  Hardware Version: %u", hardwareVer);
    }

    CommissionableDataProvider * cdp = GetCommissionableDataProvider();

    {
        uint32_t setupPasscode;
        if ((cdp == nullptr) || (cdp->GetSetupPasscode(setupPasscode) != CHIP_NO_ERROR))
        {
            setupPasscode = 0;
        }
        ChipLogProgress(DeviceLayer, "  Setup Pin Code (0 for UNKNOWN/ERROR): %" PRIu32 "", setupPasscode);
    }

    {
        uint16_t setupDiscriminator;
        if ((cdp == nullptr) || (cdp->GetSetupDiscriminator(setupDiscriminator) != CHIP_NO_ERROR))
        {
            setupDiscriminator = 0xFFFF;
        }
        ChipLogProgress(DeviceLayer, "  Setup Discriminator (0xFFFF for UNKNOWN/ERROR): %u (0x%X)", setupDiscriminator,
                        setupDiscriminator);
    }

    {
        uint16_t year;
        uint8_t month, dayOfMonth;
        err = deviceInstanceInfoProvider->GetManufacturingDate(year, month, dayOfMonth);
        if (err == CHIP_NO_ERROR)
        {
            ChipLogProgress(DeviceLayer, "  Manufacturing Date: %04u/%02u/%02u", year, month, dayOfMonth);
        }
        else
        {
            ChipLogProgress(DeviceLayer, "  Manufacturing Date: (not set)");
        }
    }

    {
        uint32_t deviceType;
        if (GetDeviceTypeId(deviceType) != CHIP_NO_ERROR)
        {
            deviceType = 0;
        }
        ChipLogProgress(DeviceLayer, "  Device Type: %" PRIu32 " (0x%" PRIX32 ")", deviceType, deviceType);
    }
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip

#endif // GENERIC_CONFIGURATION_MANAGER_IMPL_CPP
