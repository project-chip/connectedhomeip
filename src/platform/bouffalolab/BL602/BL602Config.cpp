/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
 *    Copyright (c) 2019-2020 Google LLC.
 *    Copyright (c) 2019 Nest Labs, Inc.
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
 *          Utilities for interacting with the the BL602 Easyflash module.
 */
/* this file behaves like a config.h, comes first */
#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <platform/bouffalolab/BL602/BL602Config.h>

#include <lib/core/CHIPEncoding.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CHIPMemString.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>

#include <blog.h>
#include <easyflash.h>
#include <utils_log.h>

// 3R: easyflash errno mapping to the CHIP errno

namespace chip {
namespace DeviceLayer {
namespace Internal {

const BL602Config::Key BL602Config::kConfigKey_SerialNum             = { "serial-num" };
const BL602Config::Key BL602Config::kConfigKey_MfrDeviceId           = { "device-id" };
const BL602Config::Key BL602Config::kConfigKey_MfrDeviceCert         = { "device-cert" };
const BL602Config::Key BL602Config::kConfigKey_MfrDeviceICACerts     = { "device-ca-certs" };
const BL602Config::Key BL602Config::kConfigKey_MfrDevicePrivateKey   = { "device-key" };
const BL602Config::Key BL602Config::kConfigKey_HardwareVersion       = { "hardware-ver" };
const BL602Config::Key BL602Config::kConfigKey_ManufacturingDate     = { "mfg-date" };
const BL602Config::Key BL602Config::kConfigKey_SetupPinCode          = { "pin-code" };
const BL602Config::Key BL602Config::kConfigKey_SetupDiscriminator    = { "discriminator" };
const BL602Config::Key BL602Config::kConfigKey_Spake2pIterationCount = { "iteration-count" };
const BL602Config::Key BL602Config::kConfigKey_Spake2pSalt           = { "salt" };
const BL602Config::Key BL602Config::kConfigKey_Spake2pVerifier       = { "verifier" };

// Keys stored in the chip-config namespace
const BL602Config::Key BL602Config::kConfigKey_ServiceConfig               = { "service-config" };
const BL602Config::Key BL602Config::kConfigKey_PairedAccountId             = { "account-id" };
const BL602Config::Key BL602Config::kConfigKey_ServiceId                   = { "service-id" };
const BL602Config::Key BL602Config::kConfigKey_LastUsedEpochKeyId          = { "last-ek-id" };
const BL602Config::Key BL602Config::kConfigKey_FailSafeArmed               = { "fail-safe-armed" };
const BL602Config::Key BL602Config::kConfigKey_WiFiStationSecType          = { "sta-sec-type" };
const BL602Config::Key BL602Config::kConfigKey_OperationalDeviceId         = { "op-device-id" };
const BL602Config::Key BL602Config::kConfigKey_OperationalDeviceCert       = { "op-device-cert" };
const BL602Config::Key BL602Config::kConfigKey_OperationalDeviceICACerts   = { "op-device-ca-certs" };
const BL602Config::Key BL602Config::kConfigKey_OperationalDevicePrivateKey = { "op-device-key" };
const BL602Config::Key BL602Config::kConfigKey_RegulatoryLocation          = { "regulatory-location" };
const BL602Config::Key BL602Config::kConfigKey_CountryCode                 = { "country-code" };
const BL602Config::Key BL602Config::kConfigKey_UniqueId                    = { "unique-id" };

const BL602Config::Key BL602Config::kCounterKey_RebootCount           = { "reboot-count" };
const BL602Config::Key BL602Config::kCounterKey_TotalOperationalHours = { "total-hours" };

CHIP_ERROR BL602Config::Init()
{
    return CHIP_NO_ERROR;
}

CHIP_ERROR BL602Config::ReadConfigValue(Key key, bool & val)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    bool tmpVal;
    size_t ret, valLen;

    ret = ef_get_env_blob(key.name, &tmpVal, sizeof(tmpVal), &valLen);
    if (ret <= 0)
    {
        err = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND;
    }
    SuccessOrExit(err);

    val = tmpVal;

exit:
    return err;
}

CHIP_ERROR BL602Config::ReadConfigValue(Key key, uint32_t & val)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    uint32_t tmpVal;
    size_t ret, valLen;

    ret = ef_get_env_blob(key.name, &tmpVal, sizeof(tmpVal), &valLen);
    if (ret <= 0)
    {
        err = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND;
    }
    SuccessOrExit(err);

    val = tmpVal;

exit:
    return err;
}

CHIP_ERROR BL602Config::ReadConfigValue(Key key, uint64_t & val)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    uint64_t tmpVal;
    size_t ret, valLen;

    ret = ef_get_env_blob(key.name, &tmpVal, sizeof(tmpVal), &valLen);
    if (ret <= 0)
    {
        err = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND;
    }
    SuccessOrExit(err);

    val = tmpVal;
exit:
    return err;
}

CHIP_ERROR BL602Config::ReadConfigValueStr(Key key, char * buf, size_t bufSize, size_t & outLen)
{
    CHIP_ERROR err       = CHIP_NO_ERROR;
    char tmpVal[bufSize] = { 0 };
    size_t ret;

    ret = ef_get_env_blob(key.name, tmpVal, sizeof(tmpVal) - 1, NULL);
    if ('\0' == tmpVal[0])
    {
        err = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND;
    }
    SuccessOrExit(err);

    outLen = ret;
    strncpy(buf, tmpVal, outLen);

exit:
    return err;
}

CHIP_ERROR BL602Config::ReadConfigValueBin(Key key, uint8_t * buf, size_t bufSize, size_t & outLen)
{
    CHIP_ERROR err       = CHIP_NO_ERROR;
    char tmpVal[bufSize] = { 0 };
    size_t ret;
    size_t savedLen = 0;

    ret = ef_get_env_blob(key.name, tmpVal, sizeof(tmpVal), &savedLen);
    if (0 == savedLen)
    {
        err = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND;
    }
    SuccessOrExit(err);

    outLen = ret;
    memcpy(buf, tmpVal, outLen);

exit:
    return err;
}

CHIP_ERROR BL602Config::WriteConfigValue(Key key, bool val)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    EfErrCode ret = ef_set_env_blob(key.name, &val, sizeof(val));
    if (ret != EF_NO_ERR)
    {
        err = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND;
    }
    SuccessOrExit(err);

    // ChipLogProgress(DeviceLayer, "Easyflash set: %s = %s", key.name, val ? "true" : "false");
exit:
    return err;
}

CHIP_ERROR BL602Config::WriteConfigValue(Key key, uint32_t val)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    EfErrCode ret = ef_set_env_blob(key.name, &val, sizeof(val));
    if (ret != EF_NO_ERR)
    {
        err = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND;
    }
    SuccessOrExit(err);

    // ChipLogProgress(DeviceLayer, "Easyflash set: %s = %" PRIu32 " (0x%" PRIX32 ")", key.name, val, val);

exit:
    return err;
}

CHIP_ERROR BL602Config::WriteConfigValue(Key key, uint64_t val)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    EfErrCode ret = ef_set_env_blob(key.name, &val, sizeof(val));
    if (ret != EF_NO_ERR)
    {
        log_error("WriteConfigValue() failed. key: %s, ret: %d\r\n", key.name, ret);
        err = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND;
    }
    SuccessOrExit(err);

    // ChipLogProgress(DeviceLayer, "Easyflash set: %s = %" PRIu64 " (0x%" PRIX64 ")", key.name, val, val);

exit:
    return err;
}

CHIP_ERROR BL602Config::WriteConfigValueStr(Key key, const char * str)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    if (str != NULL)
    {
        EfErrCode ret = ef_set_env(key.name, str);
        if (ret != EF_NO_ERR)
        {
            err = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND;
        }
        SuccessOrExit(err);

        // ChipLogProgress(DeviceLayer, "Easyflash set: %s = \"%s\"", key.name, str);
    }
    else
    {
        err = ClearConfigValue(key);
        SuccessOrExit(err);
    }

exit:
    return err;
}

CHIP_ERROR BL602Config::WriteConfigValueStr(Key key, const char * str, size_t strLen)
{
    CHIP_ERROR err;
    chip::Platform::ScopedMemoryBuffer<char> strCopy;

    if (str != NULL)
    {
        strCopy.Calloc(strLen + 1);
        VerifyOrExit(strCopy, err = CHIP_ERROR_NO_MEMORY);
        strncpy(strCopy.Get(), str, strLen);
    }
    err = BL602Config::WriteConfigValueStr(key, strCopy.Get());

exit:
    return err;
}

CHIP_ERROR BL602Config::WriteConfigValueBin(Key key, const uint8_t * data, size_t dataLen)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    if (data != NULL)
    {
        EfErrCode ret = ef_set_env_blob(key.name, data, dataLen);
        if (ret != EF_NO_ERR)
        {
            err = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND;
        }
        SuccessOrExit(err);

        // ChipLogProgress(DeviceLayer, "Easyflash set: /%s = (blob length %" PRId32 ")", key.name, (unsigned long )dataLen);
    }
    else
    {
        err = ClearConfigValue(key);
        SuccessOrExit(err);
    }

exit:
    return err;
}

CHIP_ERROR BL602Config::WriteWifiInfo(const char * ssid, const char * passwd)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

#if 0
    if (ssid != NULL && passwd != NULL)
    {
        EfErrCode ret = ef_set_env_blob(kBLConfigKey_wifissid, ssid, strlen(ssid));
        if (ret != EF_NO_ERR)
        {
            err = CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND;
        }

        ef_set_env_blob(kBLConfigKey_wifipassword, passwd, strlen(passwd));
        if (ret != EF_NO_ERR)
        {
            err = CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND;
        }
        SuccessOrExit(err);

    }
    else
    {
        //err = ClearConfigValue(ssid);
        //err = ClearConfigValue(passwd);
        SuccessOrExit(err);
    }

exit:
#endif

    return err;
}

CHIP_ERROR BL602Config::ClearConfigValue(Key key)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    EfErrCode ret  = ef_del_env(key.name);
    if (ret != EF_NO_ERR)
    {
        err = CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND;
    }

    SuccessOrExit(err);

    ChipLogProgress(DeviceLayer, "Easyflash erase: %s", key.name);

exit:
    return err;
}

bool BL602Config::ConfigValueExists(Key key)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    env_node_obj node;
    bool result = ef_get_env_obj(key.name, &node);
    if (!result)
    {
        err = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND;
    }

    return err == CHIP_NO_ERROR;
}

void BL602Config::RunConfigUnitTest() {}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
