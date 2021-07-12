/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

/* this file behaves like a config.h, comes first */
#include "chip_porting.h"
#include <core/CHIPEncoding.h>
#include <platform/Ameba/AmebaConfig.h>
#include <platform/internal/CHIPDeviceLayerInternal.h>
#include <support/CHIPMem.h>
#include <support/CHIPMemString.h>
#include <support/CodeUtils.h>
#include <support/logging/CHIPLogging.h>

enum
{
    kPrefsTypeBoolean = 1,
    kPrefsTypeInteger = 2,
    kPrefsTypeString  = 3,
    kPrefsTypeBuffer  = 4
};

namespace chip {
namespace DeviceLayer {
namespace Internal {

// *** CAUTION ***: Changing the names or namespaces of these values will *break* existing devices.

// NVS namespaces used to store device configuration information.
const char AmebaConfig::kConfigNamespace_ChipFactory[]  = "chip-factory";
const char AmebaConfig::kConfigNamespace_ChipConfig[]   = "chip-config";
const char AmebaConfig::kConfigNamespace_ChipCounters[] = "chip-counters";

// Keys stored in the chip-factory namespace
const AmebaConfig::Key AmebaConfig::kConfigKey_SerialNum           = { kConfigNamespace_ChipFactory, "serial-num" };
const AmebaConfig::Key AmebaConfig::kConfigKey_MfrDeviceId         = { kConfigNamespace_ChipFactory, "device-id" };
const AmebaConfig::Key AmebaConfig::kConfigKey_MfrDeviceCert       = { kConfigNamespace_ChipFactory, "device-cert" };
const AmebaConfig::Key AmebaConfig::kConfigKey_MfrDeviceICACerts   = { kConfigNamespace_ChipFactory, "device-ca-certs" };
const AmebaConfig::Key AmebaConfig::kConfigKey_MfrDevicePrivateKey = { kConfigNamespace_ChipFactory, "device-key" };
const AmebaConfig::Key AmebaConfig::kConfigKey_ProductRevision     = { kConfigNamespace_ChipFactory, "product-rev" };
const AmebaConfig::Key AmebaConfig::kConfigKey_ManufacturingDate   = { kConfigNamespace_ChipFactory, "mfg-date" };
const AmebaConfig::Key AmebaConfig::kConfigKey_SetupPinCode        = { kConfigNamespace_ChipFactory, "pin-code" };
const AmebaConfig::Key AmebaConfig::kConfigKey_SetupDiscriminator  = { kConfigNamespace_ChipFactory, "discriminator" };

// Keys stored in the chip-config namespace
const AmebaConfig::Key AmebaConfig::kConfigKey_FabricId                    = { kConfigNamespace_ChipConfig, "fabric-id" };
const AmebaConfig::Key AmebaConfig::kConfigKey_ServiceConfig               = { kConfigNamespace_ChipConfig, "service-config" };
const AmebaConfig::Key AmebaConfig::kConfigKey_PairedAccountId             = { kConfigNamespace_ChipConfig, "account-id" };
const AmebaConfig::Key AmebaConfig::kConfigKey_ServiceId                   = { kConfigNamespace_ChipConfig, "service-id" };
const AmebaConfig::Key AmebaConfig::kConfigKey_GroupKeyIndex               = { kConfigNamespace_ChipConfig, "group-key-index" };
const AmebaConfig::Key AmebaConfig::kConfigKey_LastUsedEpochKeyId          = { kConfigNamespace_ChipConfig, "last-ek-id" };
const AmebaConfig::Key AmebaConfig::kConfigKey_FailSafeArmed               = { kConfigNamespace_ChipConfig, "fail-safe-armed" };
const AmebaConfig::Key AmebaConfig::kConfigKey_WiFiStationSecType          = { kConfigNamespace_ChipConfig, "sta-sec-type" };
const AmebaConfig::Key AmebaConfig::kConfigKey_OperationalDeviceId         = { kConfigNamespace_ChipConfig, "op-device-id" };
const AmebaConfig::Key AmebaConfig::kConfigKey_OperationalDeviceCert       = { kConfigNamespace_ChipConfig, "op-device-cert" };
const AmebaConfig::Key AmebaConfig::kConfigKey_OperationalDeviceICACerts   = { kConfigNamespace_ChipConfig, "op-device-ca-certs" };
const AmebaConfig::Key AmebaConfig::kConfigKey_OperationalDevicePrivateKey = { kConfigNamespace_ChipConfig, "op-device-key" };
const AmebaConfig::Key AmebaConfig::kConfigKey_RegulatoryLocation          = { kConfigNamespace_ChipConfig, "regulatory-location" };
const AmebaConfig::Key AmebaConfig::kConfigKey_CountryCode                 = { kConfigNamespace_ChipConfig, "country-code" };
const AmebaConfig::Key AmebaConfig::kConfigKey_Breadcrumb                  = { kConfigNamespace_ChipConfig, "breadcrumb" };

CHIP_ERROR AmebaConfig::ReadConfigValue(Key key, bool & val)
{
    uint32_t intVal;

    char * _namespace = (char *) malloc(strlen(key.Namespace) + 1);
    char * _name      = (char *) malloc(strlen(key.Name) + 1);

    if (_namespace == NULL || _name == NULL)
    {
        free(_namespace);
        free(_name);
        return CHIP_ERROR_NO_MEMORY;
    }
    strcpy(_namespace, key.Namespace);
    strcpy(_name, key.Name);

    getPref_u32(_namespace, _name, kPrefsTypeBoolean, &intVal);

    val = (intVal != 0);

    free(_namespace);
    free(_name);
    return CHIP_NO_ERROR;
}

CHIP_ERROR AmebaConfig::ReadConfigValue(Key key, uint32_t & val)
{
    char * _namespace = (char *) malloc(strlen(key.Namespace) + 1);
    char * _name      = (char *) malloc(strlen(key.Name) + 1);

    if (_namespace == NULL || _name == NULL)
    {
        free(_namespace);
        free(_name);
        return CHIP_ERROR_NO_MEMORY;
    }
    strcpy(_namespace, key.Namespace);
    strcpy(_name, key.Name);

    getPref_u32(_namespace, _name, kPrefsTypeInteger, &val);

    free(_namespace);
    free(_name);
    return CHIP_NO_ERROR;
}

CHIP_ERROR AmebaConfig::ReadConfigValue(Key key, uint64_t & val)
{
    // TODO
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR AmebaConfig::ReadConfigValueStr(Key key, char * buf, size_t bufSize, size_t & outLen)
{
    int32_t ret       = 0;
    char * _namespace = (char *) malloc(strlen(key.Namespace) + 1);
    char * _name      = (char *) malloc(strlen(key.Name) + 1);

    if (_namespace == NULL || _name == NULL)
    {
        free(_namespace);
        free(_name);
        return CHIP_ERROR_NO_MEMORY;
    }
    strcpy(_namespace, key.Namespace);
    strcpy(_namespace, key.Name);

    ret = getPref_str(_namespace, _name, kPrefsTypeString, buf, &outLen);

    free(_namespace);
    free(_name);
    if (ret == 0)
    {
        return CHIP_NO_ERROR;
    }
    else
    {
        outLen = 0;
        return CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND;
    }
}

CHIP_ERROR AmebaConfig::ReadConfigValueBin(Key key, uint8_t * buf, size_t bufSize, size_t & outLen)
{
    // TODO
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR AmebaConfig::WriteConfigValue(Key key, bool val)
{
    int32_t success;
    uint8_t value;

    char * _namespace = (char *) malloc(strlen(key.Namespace) + 1);
    char * _name      = (char *) malloc(strlen(key.Name) + 1);

    if (_namespace == NULL || _name == NULL)
    {
        free(_namespace);
        free(_name);
        return CHIP_ERROR_NO_MEMORY;
    }
    strcpy(_namespace, key.Namespace);
    strcpy(_name, key.Name);

    if (val == 1)
        value = 1;
    else
        value = 0;
    success = setPref(_namespace, _name, kPrefsTypeBoolean, &value, 1);
    if (!success)
        printf("setPref: %s/%s = %s failed\n", _namespace, _name, value ? "true" : "false");

    free(_namespace);
    free(_name);
    return CHIP_NO_ERROR;
}

CHIP_ERROR AmebaConfig::WriteConfigValue(Key key, uint32_t val)
{
    int32_t success;

    char * _namespace = (char *) malloc(strlen(key.Namespace) + 1);
    char * _name      = (char *) malloc(strlen(key.Name) + 1);

    if (_namespace == NULL || _name == NULL)
    {
        free(_namespace);
        free(_name);
        return CHIP_ERROR_NO_MEMORY;
    }
    strcpy(_namespace, key.Namespace);
    strcpy(_name, key.Name);

    success = setPref(_namespace, _name, kPrefsTypeInteger, (uint8_t *) &val, sizeof(uint32_t));
    if (!success)
        printf("setPref: %s/%s = %d(0x%x) failed\n", _namespace, _name, val, val);

    free(_namespace);
    free(_name);
    return CHIP_NO_ERROR;
}

CHIP_ERROR AmebaConfig::WriteConfigValue(Key key, uint64_t val)
{
    int32_t success;

    char * _namespace = (char *) malloc(strlen(key.Namespace) + 1);
    char * _name      = (char *) malloc(strlen(key.Name) + 1);

    if (_namespace == NULL || _name == NULL)
    {
        free(_namespace);
        free(_name);
        return CHIP_ERROR_NO_MEMORY;
    }
    strcpy(_namespace, key.Namespace);
    strcpy(_name, key.Name);

    success = setPref(_namespace, _name, kPrefsTypeInteger, (uint8_t *) &val, sizeof(uint64_t));
    if (!success)
        printf("setPref: %s/%s = %d(0x%x) failed\n", _namespace, _name, val, val);

    free(_namespace);
    free(_name);
    return CHIP_NO_ERROR;
}

CHIP_ERROR AmebaConfig::WriteConfigValueStr(Key key, const char * str)
{
    int32_t success;

    char * _namespace = (char *) malloc(strlen(key.Namespace) + 1);
    char * _name      = (char *) malloc(strlen(key.Name) + 1);
    char * _str       = (char *) malloc(strlen(str) + 1);

    if (_namespace == NULL || _name == NULL || _str == NULL)
    {
        free(_namespace);
        free(_name);
        free(_str);
        return CHIP_ERROR_NO_MEMORY;
    }
    strcpy(_namespace, key.Namespace);
    strcpy(_name, key.Name);
    strcpy(_str, str);

    success = setPref(_namespace, _name, kPrefsTypeString, (uint8_t *) _str, strlen(_str) + 1);
    if (!success)
        printf("setPref: %s/%s = %s failed\n", _namespace, _name, _str);

    free(_namespace);
    free(_name);
    free(_str);
    return CHIP_NO_ERROR;
}

CHIP_ERROR AmebaConfig::WriteConfigValueStr(Key key, const char * str, size_t strLen)
{
    CHIP_ERROR err;
    chip::Platform::ScopedMemoryBuffer<char> strCopy;

    if (str != NULL)
    {
        strCopy.Calloc(strLen + 1);
        VerifyOrExit(strCopy, err = CHIP_ERROR_NO_MEMORY);
        strncpy(strCopy.Get(), str, strLen);
    }
    err = AmebaConfig::WriteConfigValueStr(key, strCopy.Get());
exit:
    return err;
}

CHIP_ERROR AmebaConfig::WriteConfigValueBin(Key key, const uint8_t * data, size_t dataLen)
{
    // TODO
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR AmebaConfig::ClearConfigValue(Key key)
{
    // TODO
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

bool AmebaConfig::ConfigValueExists(Key key)
{
    // TODO
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR AmebaConfig::EnsureNamespace(const char * ns)
{
    int32_t ret = -1;

    char * temp = (char *) malloc(strlen(ns) + 1);
    if (temp == NULL)
    {
        free(temp);
        return CHIP_ERROR_NO_MEMORY;
    }
    strcpy(temp, ns);

    ret = initPref(temp);
    if (ret != 0)
    {
        printf("dct_register_module failed\n");
    }
    free(temp);
    return CHIP_NO_ERROR;
}

CHIP_ERROR AmebaConfig::ClearNamespace(const char * ns)
{
    int32_t ret = -1;
    char * temp = (char *) malloc(strlen(ns) + 1);
    if (temp == NULL)
    {
        free(temp);
        return CHIP_ERROR_NO_MEMORY;
    }
    strcpy(temp, ns);

    ret = clearPref(temp);
    if (ret != 0)
    {
        printf("ClearNamespace failed\n");
    }
    free(temp);
    return CHIP_NO_ERROR;
}

void AmebaConfig::RunConfigUnitTest() {}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
