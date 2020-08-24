/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

#ifndef GENERIC_CONFIGURATION_MANAGER_IMPL_IPP
#define GENERIC_CONFIGURATION_MANAGER_IMPL_IPP

#include <ble/CHIPBleServiceData.h>
#include <platform/internal/CHIPDeviceLayerInternal.h>
#include <platform/internal/GenericConfigurationManagerImpl.h>
#include <support/Base64.h>
#include <support/CHIPMem.h>
#include <support/CodeUtils.h>

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
#include <platform/ThreadStackManager.h>
#endif

#if CHIP_DEVICE_CONFIG_LOG_PROVISIONING_HASH
#include <crypto/CHIPCryptoPAL.h>
#endif

namespace chip {
namespace DeviceLayer {
namespace Internal {

template <class ImplClass>
CHIP_ERROR GenericConfigurationManagerImpl<ImplClass>::_Init()
{
    mFlags = 0;

    // Cache flags indicating whether the device is currently service provisioned, is a member of a fabric,
    // is paired to an account, and/or provisioned with operational credentials.
    SetFlag(mFlags, kFlag_IsServiceProvisioned, Impl()->ConfigValueExists(ImplClass::kConfigKey_ServiceConfig));
    SetFlag(mFlags, kFlag_IsMemberOfFabric, Impl()->ConfigValueExists(ImplClass::kConfigKey_FabricId));
    SetFlag(mFlags, kFlag_IsPairedToAccount, Impl()->ConfigValueExists(ImplClass::kConfigKey_PairedAccountId));
    SetFlag(mFlags, kFlag_OperationalDeviceCredentialsProvisioned,
            Impl()->ConfigValueExists(ImplClass::kConfigKey_OperationalDeviceCert));

    return CHIP_NO_ERROR;
}

template <class ImplClass>
CHIP_ERROR GenericConfigurationManagerImpl<ImplClass>::_ConfigureChipStack()
{
    CHIP_ERROR err = CHIP_NO_ERROR;

#if CHIP_CONFIG_ENABLE_FABRIC_STATE
    size_t pairingCodeLen;

    static char sPairingCodeBuf[ConfigurationManager::kMaxPairingCodeLength + 1];

    // Configure the CHIP FabricState object with the local node id.
    err = Impl()->_GetDeviceId(FabricState.LocalNodeId);
    SuccessOrExit(err);

    // Configure the FabricState object with the pairing code string, if present.
    err = Impl()->_GetPairingCode(sPairingCodeBuf, sizeof(sPairingCodeBuf), pairingCodeLen);
    if (err != CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND)
    {
        SuccessOrExit(err);
        FabricState.PairingCode = sPairingCodeBuf;
    }

    // If the device is a member of a CHIP fabric, configure the FabricState object with the fabric id.
    err = Impl()->_GetFabricId(FabricState.FabricId);
    if (err == CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND)
    {
        FabricState.FabricId = kFabricIdNotSpecified;
        err                  = CHIP_NO_ERROR;
    }
    SuccessOrExit(err);
#endif // CHIP_CONFIG_ENABLE_FABRIC_STATE

#if CHIP_PROGRESS_LOGGING

    Impl()->LogDeviceConfig();

#if CHIP_DEVICE_CONFIG_LOG_PROVISIONING_HASH
    {
        uint8_t provHash[chip::Crypto::kSHA256_Hash_Length];
        char provHashBase64[BASE64_ENCODED_LEN(sizeof(provHash)) + 1];
        err = Impl()->_ComputeProvisioningHash(provHash, sizeof(provHash));
        if (err == CHIP_NO_ERROR)
        {
            Base64Encode(provHash, sizeof(provHash), provHashBase64);
            provHashBase64[sizeof(provHashBase64) - 1] = '\0';
            ChipLogProgress(DeviceLayer, "CHIP Provisioning Hash: %s", provHashBase64);
        }
        else
        {
            ChipLogError(DeviceLayer, "Error generating CHIP Provisioning Hash: %s", chip::ErrorStr(err));
            err = CHIP_NO_ERROR;
        }
    }
#endif // CHIP_DEVICE_CONFIG_LOG_PROVISIONING_HASH

#endif // CHIP_PROGRESS_LOGGING

    SuccessOrExit(err);

exit:
    return err;
}

template <class ImplClass>
CHIP_ERROR GenericConfigurationManagerImpl<ImplClass>::_GetFirmwareRevision(char * buf, size_t bufSize, size_t & outLen)
{
#ifdef CHIP_DEVICE_CONFIG_DEVICE_FIRMWARE_REVISION
    if (CHIP_DEVICE_CONFIG_DEVICE_FIRMWARE_REVISION[0] != 0)
    {
        outLen = min(bufSize, sizeof(CHIP_DEVICE_CONFIG_DEVICE_FIRMWARE_REVISION) - 1);
        memcpy(buf, CHIP_DEVICE_CONFIG_DEVICE_FIRMWARE_REVISION, outLen);
        return CHIP_NO_ERROR;
    }
    else
#endif // CHIP_DEVICE_CONFIG_DEVICE_FIRMWARE_REVISION
    {
        outLen = 0;
        return CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND;
    }
}

template <class ImplClass>
CHIP_ERROR GenericConfigurationManagerImpl<ImplClass>::_GetFirmwareBuildTime(uint16_t & year, uint8_t & month, uint8_t & dayOfMonth,
                                                                             uint8_t & hour, uint8_t & minute, uint8_t & second)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    err = ParseCompilerDateStr(CHIP_DEVICE_CONFIG_FIRWMARE_BUILD_DATE, year, month, dayOfMonth);
    SuccessOrExit(err);

    err = Parse24HourTimeStr(CHIP_DEVICE_CONFIG_FIRMWARE_BUILD_TIME, hour, minute, second);
    SuccessOrExit(err);

exit:
    return err;
}

template <class ImplClass>
CHIP_ERROR GenericConfigurationManagerImpl<ImplClass>::_GetManufacturerDeviceId(uint64_t & deviceId)
{
    CHIP_ERROR err;

    err = Impl()->ReadConfigValue(ImplClass::kConfigKey_MfrDeviceId, deviceId);

#if CHIP_DEVICE_CONFIG_ENABLE_TEST_DEVICE_IDENTITY
    if (err == CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND)
    {
        deviceId = TestDeviceId;
        err      = CHIP_NO_ERROR;
    }
#endif

    return err;
}

template <class ImplClass>
CHIP_ERROR GenericConfigurationManagerImpl<ImplClass>::_StoreManufacturerDeviceId(uint64_t deviceId)
{
    return Impl()->WriteConfigValue(ImplClass::kConfigKey_MfrDeviceId, deviceId);
}

template <class ImplClass>
CHIP_ERROR GenericConfigurationManagerImpl<ImplClass>::_GetSerialNumber(char * buf, size_t bufSize, size_t & serialNumLen)
{
    CHIP_ERROR err;

    err = Impl()->ReadConfigValueStr(ImplClass::kConfigKey_SerialNum, buf, bufSize, serialNumLen);
#ifdef CHIP_DEVICE_CONFIG_USE_TEST_SERIAL_NUMBER
    if (CHIP_DEVICE_CONFIG_USE_TEST_SERIAL_NUMBER[0] != 0 && err == CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND)
    {
        VerifyOrExit(sizeof(CHIP_DEVICE_CONFIG_USE_TEST_SERIAL_NUMBER) <= bufSize, err = CHIP_ERROR_BUFFER_TOO_SMALL);
        memcpy(buf, CHIP_DEVICE_CONFIG_USE_TEST_SERIAL_NUMBER, sizeof(CHIP_DEVICE_CONFIG_USE_TEST_SERIAL_NUMBER));
        serialNumLen = sizeof(CHIP_DEVICE_CONFIG_USE_TEST_SERIAL_NUMBER) - 1;
        ChipLogProgress(DeviceLayer, "Serial Number not found; using default: %s", CHIP_DEVICE_CONFIG_USE_TEST_SERIAL_NUMBER);
        err = CHIP_NO_ERROR;
    }
#endif // CHIP_DEVICE_CONFIG_USE_TEST_SERIAL_NUMBER
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
inline CHIP_ERROR GenericConfigurationManagerImpl<ImplClass>::_GetProductRevision(uint16_t & productRev)
{
    CHIP_ERROR err;
    uint32_t val;

    err = Impl()->ReadConfigValue(ImplClass::kConfigKey_ProductRevision, val);
    if (err == CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND)
    {
        productRev = (uint16_t) CHIP_DEVICE_CONFIG_DEFAULT_DEVICE_PRODUCT_REVISION;
        err        = CHIP_NO_ERROR;
    }
    else
    {
        productRev = (uint16_t) val;
    }

    return err;
}

template <class ImplClass>
inline CHIP_ERROR GenericConfigurationManagerImpl<ImplClass>::_StoreProductRevision(uint16_t productRev)
{
    return Impl()->WriteConfigValue(ImplClass::kConfigKey_ProductRevision, (uint32_t) productRev);
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

    year = strtoul(dateStr, &parseEnd, 10);
    VerifyOrExit(parseEnd == dateStr + 4, err = CHIP_ERROR_INVALID_ARGUMENT);

    month = strtoul(dateStr + 5, &parseEnd, 10);
    VerifyOrExit(parseEnd == dateStr + 7, err = CHIP_ERROR_INVALID_ARGUMENT);

    dayOfMonth = strtoul(dateStr + 8, &parseEnd, 10);
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
CHIP_ERROR GenericConfigurationManagerImpl<ImplClass>::_GetManufacturerDeviceCertificate(uint8_t * buf, size_t bufSize,
                                                                                         size_t & certLen)
{
    CHIP_ERROR err;

    err = Impl()->ReadConfigValueBin(ImplClass::kConfigKey_MfrDeviceCert, buf, bufSize, certLen);

#if CHIP_DEVICE_CONFIG_ENABLE_TEST_DEVICE_IDENTITY

    if (err == CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND)
    {
        certLen = TestDeviceCertLength;
        VerifyOrExit(buf != NULL, err = CHIP_NO_ERROR);
        VerifyOrExit(TestDeviceCertLength <= bufSize, err = CHIP_ERROR_BUFFER_TOO_SMALL);
        ChipLogProgress(DeviceLayer, "Device certificate not found; using default");
        memcpy(buf, TestDeviceCert, TestDeviceCertLength);
        err = CHIP_NO_ERROR;
    }

#endif // CHIP_DEVICE_CONFIG_ENABLE_TEST_DEVICE_IDENTITY

    SuccessOrExit(err);

exit:
    return err;
}

template <class ImplClass>
CHIP_ERROR GenericConfigurationManagerImpl<ImplClass>::_StoreManufacturerDeviceCertificate(const uint8_t * cert, size_t certLen)
{
    return Impl()->WriteConfigValueBin(ImplClass::kConfigKey_MfrDeviceCert, cert, certLen);
}

template <class ImplClass>
CHIP_ERROR GenericConfigurationManagerImpl<ImplClass>::_GetManufacturerDeviceIntermediateCACerts(uint8_t * buf, size_t bufSize,
                                                                                                 size_t & certsLen)
{
    CHIP_ERROR err;

    err = Impl()->ReadConfigValueBin(ImplClass::kConfigKey_MfrDeviceICACerts, buf, bufSize, certsLen);

#if CHIP_DEVICE_CONFIG_ENABLE_TEST_DEVICE_IDENTITY

    if (err == CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND)
    {
        certsLen = TestDeviceIntermediateCACertLength;
        VerifyOrExit(buf != NULL, err = CHIP_NO_ERROR);
        VerifyOrExit(TestDeviceIntermediateCACertLength <= bufSize, err = CHIP_ERROR_BUFFER_TOO_SMALL);
        ChipLogProgress(DeviceLayer, "Device certificate not found; using default");
        memcpy(buf, TestDeviceIntermediateCACert, TestDeviceIntermediateCACertLength);
        err = CHIP_NO_ERROR;
    }

#endif // CHIP_DEVICE_CONFIG_ENABLE_TEST_DEVICE_IDENTITY

    SuccessOrExit(err);

exit:
    return err;
}

template <class ImplClass>
CHIP_ERROR GenericConfigurationManagerImpl<ImplClass>::_StoreManufacturerDeviceIntermediateCACerts(const uint8_t * certs,
                                                                                                   size_t certsLen)
{
    return Impl()->WriteConfigValueBin(ImplClass::kConfigKey_MfrDeviceICACerts, certs, certsLen);
}

template <class ImplClass>
CHIP_ERROR GenericConfigurationManagerImpl<ImplClass>::_GetManufacturerDevicePrivateKey(uint8_t * buf, size_t bufSize,
                                                                                        size_t & keyLen)
{
    CHIP_ERROR err;

    err = Impl()->ReadConfigValueBin(ImplClass::kConfigKey_MfrDevicePrivateKey, buf, bufSize, keyLen);

#if CHIP_DEVICE_CONFIG_ENABLE_TEST_DEVICE_IDENTITY

    if (err == CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND)
    {
        keyLen = TestDevicePrivateKeyLength;
        VerifyOrExit(buf != NULL, err = CHIP_NO_ERROR);
        VerifyOrExit(TestDevicePrivateKeyLength <= bufSize, err = CHIP_ERROR_BUFFER_TOO_SMALL);
        ChipLogProgress(DeviceLayer, "Device private key not found; using default");
        memcpy(buf, TestDevicePrivateKey, TestDevicePrivateKeyLength);
        err = CHIP_NO_ERROR;
    }

#endif // CHIP_DEVICE_CONFIG_ENABLE_TEST_DEVICE_IDENTITY

    SuccessOrExit(err);

exit:
    return err;
}

template <class ImplClass>
CHIP_ERROR GenericConfigurationManagerImpl<ImplClass>::_StoreManufacturerDevicePrivateKey(const uint8_t * key, size_t keyLen)
{
    return Impl()->WriteConfigValueBin(ImplClass::kConfigKey_MfrDevicePrivateKey, key, keyLen);
}

template <class ImplClass>
CHIP_ERROR GenericConfigurationManagerImpl<ImplClass>::_GetDeviceId(uint64_t & deviceId)
{
    CHIP_ERROR err;

#if CHIP_DEVICE_CONFIG_ENABLE_JUST_IN_TIME_PROVISIONING
    if (!UseManufacturerCredentialsAsOperational())
    {
        err = Impl()->ReadConfigValue(ImplClass::kConfigKey_OperationalDeviceId, deviceId);
    }
    else
#endif
    {
        err = Impl()->_GetManufacturerDeviceId(deviceId);
    }

    return err;
}

template <class ImplClass>
CHIP_ERROR GenericConfigurationManagerImpl<ImplClass>::_GetDeviceCertificate(uint8_t * buf, size_t bufSize, size_t & certLen)
{
    CHIP_ERROR err;

#if CHIP_DEVICE_CONFIG_ENABLE_JUST_IN_TIME_PROVISIONING
    if (!UseManufacturerCredentialsAsOperational())
    {
        err = Impl()->ReadConfigValueBin(ImplClass::kConfigKey_OperationalDeviceCert, buf, bufSize, certLen);
    }
    else
#endif
    {
        err = Impl()->_GetManufacturerDeviceCertificate(buf, bufSize, certLen);
    }

    return err;
}

template <class ImplClass>
CHIP_ERROR GenericConfigurationManagerImpl<ImplClass>::_GetDeviceIntermediateCACerts(uint8_t * buf, size_t bufSize,
                                                                                     size_t & certsLen)
{
    CHIP_ERROR err;

#if CHIP_DEVICE_CONFIG_ENABLE_JUST_IN_TIME_PROVISIONING
    if (!UseManufacturerCredentialsAsOperational())
    {
        err = Impl()->ReadConfigValueBin(ImplClass::kConfigKey_OperationalDeviceICACerts, buf, bufSize, certsLen);
    }
    else
#endif
    {
        err = Impl()->_GetManufacturerDeviceIntermediateCACerts(buf, bufSize, certsLen);
    }

    return err;
}

template <class ImplClass>
CHIP_ERROR GenericConfigurationManagerImpl<ImplClass>::_GetDevicePrivateKey(uint8_t * buf, size_t bufSize, size_t & keyLen)
{
    CHIP_ERROR err;

#if CHIP_DEVICE_CONFIG_ENABLE_JUST_IN_TIME_PROVISIONING
    if (!UseManufacturerCredentialsAsOperational())
    {
        err = Impl()->ReadConfigValueBin(ImplClass::kConfigKey_OperationalDevicePrivateKey, buf, bufSize, keyLen);
    }
    else
#endif
    {
        err = Impl()->_GetManufacturerDevicePrivateKey(buf, bufSize, keyLen);
    }

    return err;
}

#if CHIP_DEVICE_CONFIG_ENABLE_JUST_IN_TIME_PROVISIONING

template <class ImplClass>
CHIP_ERROR GenericConfigurationManagerImpl<ImplClass>::_StoreDeviceId(uint64_t deviceId)
{
    return Impl()->WriteConfigValue(ImplClass::kConfigKey_OperationalDeviceId, deviceId);
}

template <class ImplClass>
CHIP_ERROR GenericConfigurationManagerImpl<ImplClass>::_StoreDeviceCertificate(const uint8_t * cert, size_t certLen)
{
    return Impl()->WriteConfigValueBin(ImplClass::kConfigKey_OperationalDeviceCert, cert, certLen);
}

template <class ImplClass>
CHIP_ERROR GenericConfigurationManagerImpl<ImplClass>::_StoreDeviceIntermediateCACerts(const uint8_t * certs, size_t certsLen)
{
    return Impl()->WriteConfigValueBin(ImplClass::kConfigKey_OperationalDeviceICACerts, certs, certsLen);
}

template <class ImplClass>
CHIP_ERROR GenericConfigurationManagerImpl<ImplClass>::_StoreDevicePrivateKey(const uint8_t * key, size_t keyLen)
{
    return Impl()->WriteConfigValueBin(ImplClass::kConfigKey_OperationalDevicePrivateKey, key, keyLen);
}

template <class ImplClass>
CHIP_ERROR GenericConfigurationManagerImpl<ImplClass>::_ClearOperationalDeviceCredentials(void)
{
    Impl()->ClearConfigValue(ImplClass::kConfigKey_OperationalDeviceId);
    Impl()->ClearConfigValue(ImplClass::kConfigKey_OperationalDeviceCert);
    Impl()->ClearConfigValue(ImplClass::kConfigKey_OperationalDeviceICACerts);
    Impl()->ClearConfigValue(ImplClass::kConfigKey_OperationalDevicePrivateKey);

    ClearFlag(mFlags, kFlag_OperationalDeviceCredentialsProvisioned);

    return CHIP_NO_ERROR;
}

template <class ImplClass>
bool GenericConfigurationManagerImpl<ImplClass>::_OperationalDeviceCredentialsProvisioned()
{
    return ::chip::GetFlag(mFlags, kFlag_OperationalDeviceCredentialsProvisioned);
}

template <class ImplClass>
bool GenericConfigurationManagerImpl<ImplClass>::UseManufacturerCredentialsAsOperational()
{
    return ::chip::GetFlag(mFlags, kFlag_UseManufacturerCredentialsAsOperational);
}

template <class ImplClass>
void GenericConfigurationManagerImpl<ImplClass>::_UseManufacturerCredentialsAsOperational(bool val)
{
    SetFlag(mFlags, kFlag_UseManufacturerCredentialsAsOperational, val);
}

#endif // CHIP_DEVICE_CONFIG_ENABLE_JUST_IN_TIME_PROVISIONING

template <class ImplClass>
CHIP_ERROR GenericConfigurationManagerImpl<ImplClass>::_GetSetupPinCode(uint32_t & setupPinCode)
{
    CHIP_ERROR err;

    err = Impl()->ReadConfigValue(ImplClass::kConfigKey_SetupPinCode, setupPinCode);
#if defined(CHIP_DEVICE_CONFIG_USE_TEST_SETUP_PIN_CODE) && CHIP_DEVICE_CONFIG_USE_TEST_SETUP_PIN_CODE
    if (err == CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND)
    {
        setupPinCode = CHIP_DEVICE_CONFIG_USE_TEST_SETUP_PIN_CODE;
        ChipLogProgress(DeviceLayer, "Setup PIN code not found; using default: %09u", CHIP_DEVICE_CONFIG_USE_TEST_SETUP_PIN_CODE);
        err = CHIP_NO_ERROR;
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
CHIP_ERROR GenericConfigurationManagerImpl<ImplClass>::_GetSetupDiscriminator(uint32_t & setupDiscriminator)
{
    CHIP_ERROR err;

    err = Impl()->ReadConfigValue(ImplClass::kConfigKey_SetupDiscriminator, setupDiscriminator);
#if defined(CHIP_DEVICE_CONFIG_USE_TEST_SETUP_DISCRIMINATOR) && CHIP_DEVICE_CONFIG_USE_TEST_SETUP_DISCRIMINATOR
    if (err == CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND)
    {
        setupDiscriminator = CHIP_DEVICE_CONFIG_USE_TEST_SETUP_DISCRIMINATOR;
        ChipLogProgress(DeviceLayer, "Setup PIN discriminator not found; using default: %03x",
                        CHIP_DEVICE_CONFIG_USE_TEST_SETUP_DISCRIMINATOR);
        err = CHIP_NO_ERROR;
    }
#endif // defined(CHIP_DEVICE_CONFIG_USE_TEST_SETUP_DISCRIMINATOR) && CHIP_DEVICE_CONFIG_USE_TEST_SETUP_DISCRIMINATOR
    SuccessOrExit(err);

exit:
    return err;
}

template <class ImplClass>
CHIP_ERROR GenericConfigurationManagerImpl<ImplClass>::_StoreSetupDiscriminator(uint32_t setupDiscriminator)
{
    return Impl()->WriteConfigValue(ImplClass::kConfigKey_SetupDiscriminator, setupDiscriminator);
}

template <class ImplClass>
CHIP_ERROR GenericConfigurationManagerImpl<ImplClass>::_GetFabricId(uint64_t & fabricId)
{
    return Impl()->ReadConfigValue(ImplClass::kConfigKey_FabricId, fabricId);
}

template <class ImplClass>
CHIP_ERROR GenericConfigurationManagerImpl<ImplClass>::_StoreFabricId(uint64_t fabricId)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

#if CHIP_CONFIG_ENABLE_FABRIC_STATE
    if (fabricId != kFabricIdNotSpecified)
    {
        err = Impl()->WriteConfigValue(ImplClass::kConfigKey_FabricId, fabricId);
        SuccessOrExit(err);
        SetFlag(mFlags, kFlag_IsMemberOfFabric);
    }
    else
    {
        ClearFlag(mFlags, kFlag_IsMemberOfFabric);
        err = Impl()->ClearConfigValue(ImplClass::kConfigKey_FabricId);
        SuccessOrExit(err);
    }

exit:
#endif

    return err;
}

template <class ImplClass>
CHIP_ERROR GenericConfigurationManagerImpl<ImplClass>::_GetServiceId(uint64_t & serviceId)
{
    return Impl()->ReadConfigValue(ImplClass::kConfigKey_ServiceId, serviceId);
}

template <class ImplClass>
CHIP_ERROR GenericConfigurationManagerImpl<ImplClass>::_GetServiceConfig(uint8_t * buf, size_t bufSize, size_t & serviceConfigLen)
{
    return Impl()->ReadConfigValueBin(ImplClass::kConfigKey_ServiceConfig, buf, bufSize, serviceConfigLen);
}

template <class ImplClass>
CHIP_ERROR GenericConfigurationManagerImpl<ImplClass>::_StoreServiceConfig(const uint8_t * serviceConfig, size_t serviceConfigLen)
{
    return Impl()->WriteConfigValueBin(ImplClass::kConfigKey_ServiceConfig, serviceConfig, serviceConfigLen);
}

template <class ImplClass>
CHIP_ERROR GenericConfigurationManagerImpl<ImplClass>::_GetPairedAccountId(char * buf, size_t bufSize, size_t & accountIdLen)
{
    return Impl()->ReadConfigValueStr(ImplClass::kConfigKey_PairedAccountId, buf, bufSize, accountIdLen);
}

template <class ImplClass>
CHIP_ERROR GenericConfigurationManagerImpl<ImplClass>::_StorePairedAccountId(const char * accountId, size_t accountIdLen)
{
    CHIP_ERROR err;

    err = Impl()->WriteConfigValueStr(ImplClass::kConfigKey_PairedAccountId, accountId, accountIdLen);
    SuccessOrExit(err);

    SetFlag(mFlags, kFlag_IsPairedToAccount, (accountId != NULL && accountIdLen != 0));

exit:
    return err;
}

template <class ImplClass>
CHIP_ERROR GenericConfigurationManagerImpl<ImplClass>::_StoreServiceProvisioningData(uint64_t serviceId,
                                                                                     const uint8_t * serviceConfig,
                                                                                     size_t serviceConfigLen,
                                                                                     const char * accountId, size_t accountIdLen)
{
    CHIP_ERROR err;

    err = Impl()->WriteConfigValue(ImplClass::kConfigKey_ServiceId, serviceId);
    SuccessOrExit(err);

    err = _StoreServiceConfig(serviceConfig, serviceConfigLen);
    SuccessOrExit(err);

    err = _StorePairedAccountId(accountId, accountIdLen);
    SuccessOrExit(err);

    SetFlag(mFlags, kFlag_IsServiceProvisioned);
    SetFlag(mFlags, kFlag_IsPairedToAccount, (accountId != NULL && accountIdLen != 0));

exit:
    if (err != CHIP_NO_ERROR)
    {
        Impl()->ClearConfigValue(ImplClass::kConfigKey_ServiceId);
        Impl()->ClearConfigValue(ImplClass::kConfigKey_ServiceConfig);
        Impl()->ClearConfigValue(ImplClass::kConfigKey_PairedAccountId);
        ClearFlag(mFlags, kFlag_IsServiceProvisioned);
        ClearFlag(mFlags, kFlag_IsPairedToAccount);
    }
    return err;
}

template <class ImplClass>
CHIP_ERROR GenericConfigurationManagerImpl<ImplClass>::_ClearServiceProvisioningData()
{
    Impl()->ClearConfigValue(ImplClass::kConfigKey_ServiceId);
    Impl()->ClearConfigValue(ImplClass::kConfigKey_ServiceConfig);
    Impl()->ClearConfigValue(ImplClass::kConfigKey_PairedAccountId);

    // TODO: Move these behaviors out of configuration manager.

    // If necessary, post an event alerting other subsystems to the change in
    // the account pairing state.
    if (_IsPairedToAccount())
    {
        ChipDeviceEvent event;
        event.Type                                   = DeviceEventType::kAccountPairingChange;
        event.AccountPairingChange.IsPairedToAccount = false;
        PlatformMgr().PostEvent(&event);
    }

    // If necessary, post an event alerting other subsystems to the change in
    // the service provisioning state.
    if (_IsServiceProvisioned())
    {
        ChipDeviceEvent event;
        event.Type                                           = DeviceEventType::kServiceProvisioningChange;
        event.ServiceProvisioningChange.IsServiceProvisioned = false;
        event.ServiceProvisioningChange.ServiceConfigUpdated = false;
        PlatformMgr().PostEvent(&event);
    }

    ClearFlag(mFlags, kFlag_IsServiceProvisioned);
    ClearFlag(mFlags, kFlag_IsPairedToAccount);

    return CHIP_NO_ERROR;
}

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
CHIP_ERROR GenericConfigurationManagerImpl<ImplClass>::_GetQRCodeString(char * buf, size_t bufSize)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

template <class ImplClass>
CHIP_ERROR GenericConfigurationManagerImpl<ImplClass>::_GetWiFiAPSSID(char * buf, size_t bufSize)
{
    CHIP_ERROR err;

#ifdef CHIP_DEVICE_CONFIG_WIFI_AP_SSID_PREFIX

    uint8_t mac[6];

    VerifyOrExit(bufSize >= sizeof(CHIP_DEVICE_CONFIG_WIFI_AP_SSID_PREFIX) + 4, err = CHIP_ERROR_BUFFER_TOO_SMALL);

    err = Impl()->_GetPrimaryWiFiMACAddress(mac);
    SuccessOrExit(err);

    snprintf(buf, bufSize, "%s%02X%02X", CHIP_DEVICE_CONFIG_WIFI_AP_SSID_PREFIX, mac[4], mac[5]);
    buf[bufSize - 1] = 0;

#else // CHIP_DEVICE_CONFIG_WIFI_AP_SSID_PREFIX

    ExitNow(err = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND);

#endif // CHIP_DEVICE_CONFIG_WIFI_AP_SSID_PREFIX

exit:
    return err;
}

template <class ImplClass>
CHIP_ERROR
GenericConfigurationManagerImpl<ImplClass>::_GetBLEDeviceIdentificationInfo(Ble::ChipBLEDeviceIdentificationInfo & deviceIdInfo)
{
    CHIP_ERROR err;
    uint16_t id;
    uint32_t discriminator;

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

    // TODO: Update when CHIP service/fabric provision is implemented
#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
    deviceIdInfo.PairingStatus = ThreadStackMgr().IsThreadAttached()
        ? Ble::ChipBLEDeviceIdentificationInfo::kPairingStatus_Paired
        : Ble::ChipBLEDeviceIdentificationInfo::kPairingStatus_Unpaired;
#else
    deviceIdInfo.PairingStatus = Impl()->_IsPairedToAccount() ? Ble::ChipBLEDeviceIdentificationInfo::kPairingStatus_Paired
                                                              : Ble::ChipBLEDeviceIdentificationInfo::kPairingStatus_Unpaired;
#endif

exit:
    return err;
}

template <class ImplClass>
bool GenericConfigurationManagerImpl<ImplClass>::_IsServiceProvisioned()
{
    return ::chip::GetFlag(mFlags, kFlag_IsServiceProvisioned);
}

template <class ImplClass>
bool GenericConfigurationManagerImpl<ImplClass>::_IsMemberOfFabric()
{
    return ::chip::GetFlag(mFlags, kFlag_IsMemberOfFabric);
}

template <class ImplClass>
bool GenericConfigurationManagerImpl<ImplClass>::_IsPairedToAccount()
{
    return ::chip::GetFlag(mFlags, kFlag_IsPairedToAccount);
}

template <class ImplClass>
bool GenericConfigurationManagerImpl<ImplClass>::_IsFullyProvisioned()
{
    return
#if CHIP_DEVICE_CONFIG_ENABLE_WIFI_STATION
        ConnectivityMgr().IsWiFiStationProvisioned() &&
#endif
#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
        ConnectivityMgr().IsThreadProvisioned() &&
#endif
#if !CHIP_DEVICE_CONFIG_DISABLE_ACCOUNT_PAIRING
        Impl()->IsPairedToAccount() &&
#endif
#if CHIP_DEVICE_CONFIG_ENABLE_JUST_IN_TIME_PROVISIONING
        (!UseManufacturerCredentialsAsOperational() && _OperationalDeviceCredentialsProvisioned()) &&
#endif
        Impl()->IsMemberOfFabric();
}

template <class ImplClass>
CHIP_ERROR GenericConfigurationManagerImpl<ImplClass>::_ComputeProvisioningHash(uint8_t * hashBuf, size_t hashBufSize)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

#if CHIP_DEVICE_CONFIG_LOG_PROVISIONING_HASH
    using HashAlgo = chip::Crypto::Hash_SHA256_stream;

    HashAlgo hash;
    uint8_t * dataBuf = NULL;
    size_t dataBufSize;
    constexpr uint16_t kLenFieldLen = 4; // 4 hex characters

    VerifyOrExit(hashBufSize >= chip::Crypto::kSHA256_Hash_Length, err = CHIP_ERROR_BUFFER_TOO_SMALL);

    // Compute a hash of the device's provisioning data.  The generated hash value confirms to the form
    // described in the CHIP Chip: Factory Provisioning Specification.
    //
    // A CHIP provisioning hash is a SHA-256 hash of an ASCII string with the following format:
    //
    //     DDDDddddddddddddddddCCCCcccc…ccccIIIIiiii…iiiiKKKKkkkk…kkkkPPPPpppppp
    //
    // Where:
    //     dddddddddddddddd is the CHIP node id for the device, encoded as a string of 16 uppercase hex digits.
    //     cccc…cccc is the device CHIP certificate, in base-64 format.
    //     iiii…iiii is the device intermediate CA certificates, in base-64 format (if provisioned).
    //     kkkk…kkkk is the device private key, in base-64 format.
    //     pppppp is the device pairing code, as ASCII characters.
    //     DDDD is the length of the dddddddddddddddd field (the device id), represented as 4 uppercase hex digits.
    //         Because the device id is always the same size, this field is always '0010'.
    //     CCCC is the length of the cccc…cccc field (the device certificate), represented as 4 uppercase hex digits.
    //     IIII is the length of the iiii…iiii field (the device intermediate CA certificates), represented as 4 uppercase hex
    //     digits. KKKK is the length of the kkkk…kkkk field (the device private key), represented as 4 uppercase hex digits. PPPP
    //     is the length of the pppppp field (the device pairing code), represented as 4 uppercase hex digits.

    hash.Begin();

    // Hash the device id
    {
        uint64_t deviceId;
        constexpr uint16_t kDeviceIdLen = 16;           // 16 hex characters
        char inputBuf[kLenFieldLen + kDeviceIdLen + 1]; // +1 for terminator

        err = Impl()->_GetManufacturerDeviceId(deviceId);
        SuccessOrExit(err);

        snprintf(inputBuf, sizeof(inputBuf), "0010%016" PRIX64, deviceId);

        hash.AddData((uint8_t *) inputBuf, kLenFieldLen + kDeviceIdLen);
    }

    // Hash the device certificate
    {
        size_t certLen;

        // Determine the length of the device certificate.
        err = Impl()->_GetManufacturerDeviceCertificate((uint8_t *) NULL, 0, certLen);
        SuccessOrExit(err);

        // Create a temporary buffer to hold the certificate.  (This will also be used for
        // the private key).
        dataBufSize = certLen;
        dataBuf     = (uint8_t *) chip::Platform::MemoryAlloc(dataBufSize);
        VerifyOrExit(dataBuf != NULL, err = CHIP_ERROR_NO_MEMORY);

        // Read the certificate.
        err = Impl()->_GetManufacturerDeviceCertificate(dataBuf, certLen, certLen);
        SuccessOrExit(err);
    }

    // Hash the device intermediate CA certificates
    if (Impl()->ConfigValueExists(ImplClass::kConfigKey_MfrDeviceICACerts))
    {
        size_t certsLen;

        // Determine the length of the device intermediate CA certificates.
        err = Impl()->_GetManufacturerDeviceIntermediateCACerts((uint8_t *) NULL, 0, certsLen);
        SuccessOrExit(err);

        // Allocate larger buffer to hold the intermediate CA certificates.
        // (This will also be used for the private key).
        if (certsLen > dataBufSize)
        {
            chip::Platform::MemoryFree(dataBuf);

            dataBufSize = certsLen;
            dataBuf     = (uint8_t *) chip::Platform::MemoryAlloc(dataBufSize);
            VerifyOrExit(dataBuf != NULL, err = CHIP_ERROR_NO_MEMORY);
        }

        // Read the device intermediate CA certificates.
        err = Impl()->_GetManufacturerDeviceIntermediateCACerts(dataBuf, certsLen, certsLen);
        SuccessOrExit(err);
    }

    // Hash the device private key
    {
        size_t keyLen;

        // Determine the length of the device private key.
        err = Impl()->_GetManufacturerDevicePrivateKey((uint8_t *) NULL, 0, keyLen);
        SuccessOrExit(err);

        // Read the private key.  (Note that we presume the buffer allocated to hold the certificate
        // is big enough to hold the private key.  _GetDevicePrivateKey() will return an error in the
        // unlikely event that this is not the case.)
        err = Impl()->_GetManufacturerDevicePrivateKey(dataBuf, dataBufSize, keyLen);
        SuccessOrExit(err);
    }

    // Hash the device pairing code.  If the device does not have a pairing code, hash a zero-length value.
    {
        char pairingCode[ConfigurationManager::kMaxPairingCodeLength + 1]; // +1 for terminator
        char lenStr[kLenFieldLen + 1];                                     // +1 for terminator
        size_t pairingCodeLen;

        err = Impl()->_GetPairingCode(pairingCode, sizeof(pairingCode), pairingCodeLen);
        if (err == CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND)
        {
            pairingCodeLen = 0;
            err            = CHIP_NO_ERROR;
        }
        SuccessOrExit(err);

        snprintf(lenStr, sizeof(lenStr), "%04" PRIX16, (uint16_t) pairingCodeLen);

        hash.AddData((uint8_t *) lenStr, kLenFieldLen);
        hash.AddData((uint8_t *) pairingCode, pairingCodeLen);
    }

    hash.Finish(hashBuf);

exit:
    if (dataBuf != NULL)
    {
        chip::Crypto::ClearSecretData(dataBuf, dataBufSize);
        chip::Platform::MemoryFree(dataBuf);
    }
#endif // CHIP_DEVICE_CONFIG_LOG_PROVISIONING_HASH

    return err;
}

#if defined(DEBUG)
template <class ImplClass>
CHIP_ERROR GenericConfigurationManagerImpl<ImplClass>::_RunUnitTests()
{
    ChipLogProgress(DeviceLayer, "Running configuration unit test");
    Impl()->RunConfigUnitTest();

    return CHIP_NO_ERROR;
}
#endif

#if CHIP_PROGRESS_LOGGING

template <class ImplClass>
void GenericConfigurationManagerImpl<ImplClass>::LogDeviceConfig()
{
    CHIP_ERROR err;

    ChipLogProgress(DeviceLayer, "Device Configuration:");

#if CHIP_CONFIG_ENABLE_FABRIC_STATE
    ChipLogProgress(DeviceLayer, "  Device Id: %016" PRIX64, FabricState.LocalNodeId);
#endif

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

#if CHIP_CONFIG_ENABLE_FABRIC_STATE
    if (FabricState.FabricId != kFabricIdNotSpecified)
    {
        ChipLogProgress(DeviceLayer, "  Fabric Id: %016" PRIX64, FabricState.FabricId);
    }
    else
    {
        ChipLogProgress(DeviceLayer, "  Fabric Id: (none)");
    }

    ChipLogProgress(DeviceLayer, "  Pairing Code: %s", (FabricState.PairingCode != NULL) ? FabricState.PairingCode : "(none)");
#endif // CHIP_CONFIG_ENABLE_FABRIC_STATE
}

#endif // CHIP_PROGRESS_LOGGING

// Fully instantiate the generic implementation class in whatever compilation unit includes this file.
template class GenericConfigurationManagerImpl<ConfigurationManagerImpl>;

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip

#endif // GENERIC_CONFIGURATION_MANAGER_IMPL_IPP
