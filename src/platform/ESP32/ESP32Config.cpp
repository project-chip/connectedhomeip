/*
 *
 *    Copyright (c) 2019-2020 Google LLC.
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
 *          Utilities for interacting with the the ESP32 "NVS" key-value store.
 */

#include <Weave/DeviceLayer/internal/WeaveDeviceLayerInternal.h>
#include <Weave/DeviceLayer/ESP32/ESP32Config.h>

#include "nvs_flash.h"
#include "nvs.h"

namespace nl {
namespace Weave {
namespace DeviceLayer {
namespace Internal {

// *** CAUTION ***: Changing the names or namespaces of these values will *break* existing devices.

// NVS namespaces used to store device configuration information.
const char ESP32Config::kConfigNamespace_WeaveFactory[]                    = "weave-factory";
const char ESP32Config::kConfigNamespace_WeaveConfig[]                     = "weave-config";
const char ESP32Config::kConfigNamespace_WeaveCounters[]                   = "weave-counters";

// Keys stored in the weave-factory namespace
const ESP32Config::Key ESP32Config::kConfigKey_SerialNum                   = { kConfigNamespace_WeaveFactory, "serial-num"         };
const ESP32Config::Key ESP32Config::kConfigKey_MfrDeviceId                 = { kConfigNamespace_WeaveFactory, "device-id"          };
const ESP32Config::Key ESP32Config::kConfigKey_MfrDeviceCert               = { kConfigNamespace_WeaveFactory, "device-cert"        };
const ESP32Config::Key ESP32Config::kConfigKey_MfrDeviceICACerts           = { kConfigNamespace_WeaveFactory, "device-ca-certs"    };
const ESP32Config::Key ESP32Config::kConfigKey_MfrDevicePrivateKey         = { kConfigNamespace_WeaveFactory, "device-key"         };
const ESP32Config::Key ESP32Config::kConfigKey_ProductRevision             = { kConfigNamespace_WeaveFactory, "product-rev"        };
const ESP32Config::Key ESP32Config::kConfigKey_ManufacturingDate           = { kConfigNamespace_WeaveFactory, "mfg-date"           };
const ESP32Config::Key ESP32Config::kConfigKey_PairingCode                 = { kConfigNamespace_WeaveFactory, "pairing-code"       };

// Keys stored in the weave-config namespace
const ESP32Config::Key ESP32Config::kConfigKey_FabricId                    = { kConfigNamespace_WeaveConfig,  "fabric-id"          };
const ESP32Config::Key ESP32Config::kConfigKey_ServiceConfig               = { kConfigNamespace_WeaveConfig,  "service-config"     };
const ESP32Config::Key ESP32Config::kConfigKey_PairedAccountId             = { kConfigNamespace_WeaveConfig,  "account-id"         };
const ESP32Config::Key ESP32Config::kConfigKey_ServiceId                   = { kConfigNamespace_WeaveConfig,  "service-id"         };
const ESP32Config::Key ESP32Config::kConfigKey_FabricSecret                = { kConfigNamespace_WeaveConfig,  "fabric-secret"      };
const ESP32Config::Key ESP32Config::kConfigKey_GroupKeyIndex               = { kConfigNamespace_WeaveConfig,  "group-key-index"    };
const ESP32Config::Key ESP32Config::kConfigKey_LastUsedEpochKeyId          = { kConfigNamespace_WeaveConfig,  "last-ek-id"         };
const ESP32Config::Key ESP32Config::kConfigKey_FailSafeArmed               = { kConfigNamespace_WeaveConfig,  "fail-safe-armed"    };
const ESP32Config::Key ESP32Config::kConfigKey_WiFiStationSecType          = { kConfigNamespace_WeaveConfig,  "sta-sec-type"       };
const ESP32Config::Key ESP32Config::kConfigKey_OperationalDeviceId         = { kConfigNamespace_WeaveConfig,  "op-device-id"       };
const ESP32Config::Key ESP32Config::kConfigKey_OperationalDeviceCert       = { kConfigNamespace_WeaveConfig,  "op-device-cert"     };
const ESP32Config::Key ESP32Config::kConfigKey_OperationalDeviceICACerts   = { kConfigNamespace_WeaveConfig,  "op-device-ca-certs" };
const ESP32Config::Key ESP32Config::kConfigKey_OperationalDevicePrivateKey = { kConfigNamespace_WeaveConfig,  "op-device-key"      };

// Prefix used for NVS keys that contain Weave group encryption keys.
const char ESP32Config::kGroupKeyNamePrefix[]                              = "gk-";


WEAVE_ERROR ESP32Config::ReadConfigValue(Key key, bool & val)
{
    WEAVE_ERROR err;
    nvs_handle handle;
    bool needClose = false;
    uint32_t intVal;

    err = nvs_open(key.Namespace, NVS_READONLY, &handle);
    SuccessOrExit(err);
    needClose = true;

    err = nvs_get_u32(handle, key.Name, &intVal);
    if (err == ESP_ERR_NVS_NOT_FOUND)
    {
        err = WEAVE_DEVICE_ERROR_CONFIG_NOT_FOUND;
    }
    SuccessOrExit(err);

    val = (intVal != 0);

exit:
    if (needClose)
    {
        nvs_close(handle);
    }
    return err;
}

WEAVE_ERROR ESP32Config::ReadConfigValue(Key key, uint32_t & val)
{
    WEAVE_ERROR err;
    nvs_handle handle;
    bool needClose = false;

    err = nvs_open(key.Namespace, NVS_READONLY, &handle);
    SuccessOrExit(err);
    needClose = true;

    err = nvs_get_u32(handle, key.Name, &val);
    if (err == ESP_ERR_NVS_NOT_FOUND)
    {
        err = WEAVE_DEVICE_ERROR_CONFIG_NOT_FOUND;
    }
    SuccessOrExit(err);

exit:
    if (needClose)
    {
        nvs_close(handle);
    }
    return err;
}

WEAVE_ERROR ESP32Config::ReadConfigValue(Key key, uint64_t & val)
{
    WEAVE_ERROR err;
    nvs_handle handle;
    bool needClose = false;

    err = nvs_open(key.Namespace, NVS_READONLY, &handle);
    SuccessOrExit(err);
    needClose = true;

    // Special case the MfrDeviceId value, optionally allowing it to be read as a blob containing
    // a 64-bit big-endian integer, instead of a u64 value.
    //
    // The ESP32 development environment provides a tool for pre-populating the NVS partition using
    // values from a CSV file.  This tool is convenient for provisioning devices during manufacturing.
    // However currently the tool does not support pre-populating u64 values such as MfrDeviceId.
    // Thus we allow MfrDeviceId to be stored as a blob instead.
    //
    if (key == kConfigKey_MfrDeviceId)
    {
        uint8_t deviceIdBytes[sizeof(uint64_t)];
        size_t deviceIdLen = sizeof(deviceIdBytes);
        err = nvs_get_blob(handle, key.Name, deviceIdBytes, &deviceIdLen);
        if (err == ESP_OK)
        {
            VerifyOrExit(deviceIdLen == sizeof(deviceIdBytes), err = ESP_ERR_NVS_INVALID_LENGTH);
            val = Encoding::BigEndian::Get64(deviceIdBytes);
            ExitNow();
        }
    }

    err = nvs_get_u64(handle, key.Name, &val);
    if (err == ESP_ERR_NVS_NOT_FOUND)
    {
        err = WEAVE_DEVICE_ERROR_CONFIG_NOT_FOUND;
    }
    SuccessOrExit(err);

exit:
    if (needClose)
    {
        nvs_close(handle);
    }
    return err;
}

WEAVE_ERROR ESP32Config::ReadConfigValueStr(Key key, char * buf, size_t bufSize, size_t & outLen)
{
    WEAVE_ERROR err;
    nvs_handle handle;
    bool needClose = false;

    err = nvs_open(key.Namespace, NVS_READONLY, &handle);
    SuccessOrExit(err);
    needClose = true;

    outLen = bufSize;
    err = nvs_get_str(handle, key.Name, buf, &outLen);
    if (err == ESP_ERR_NVS_NOT_FOUND)
    {
        outLen = 0;
        err = WEAVE_DEVICE_ERROR_CONFIG_NOT_FOUND;
    }
    else if (err == ESP_ERR_NVS_INVALID_LENGTH)
    {
        err = (buf == NULL) ? WEAVE_NO_ERROR : WEAVE_ERROR_BUFFER_TOO_SMALL;
    }
    SuccessOrExit(err);

    outLen -= 1; // Don't count trailing nul.

exit:
    if (needClose)
    {
        nvs_close(handle);
    }

    return err;
}

WEAVE_ERROR ESP32Config::ReadConfigValueBin(Key key, uint8_t * buf, size_t bufSize, size_t & outLen)
{
    WEAVE_ERROR err;
    nvs_handle handle;
    bool needClose = false;

    err = nvs_open(key.Namespace, NVS_READONLY, &handle);
    SuccessOrExit(err);
    needClose = true;

    outLen = bufSize;
    err = nvs_get_blob(handle, key.Name, buf, &outLen);
    if (err == ESP_ERR_NVS_NOT_FOUND)
    {
        outLen = 0;
        err = WEAVE_DEVICE_ERROR_CONFIG_NOT_FOUND;
    }
    else if (err == ESP_ERR_NVS_INVALID_LENGTH)
    {
        err = (buf == NULL) ? WEAVE_NO_ERROR : WEAVE_ERROR_BUFFER_TOO_SMALL;
    }
    SuccessOrExit(err);

exit:
    if (needClose)
    {
        nvs_close(handle);
    }

    return err;
}

WEAVE_ERROR ESP32Config::WriteConfigValue(Key key, bool val)
{
    WEAVE_ERROR err;
    nvs_handle handle;
    bool needClose = false;

    err = nvs_open(key.Namespace, NVS_READWRITE, &handle);
    SuccessOrExit(err);
    needClose = true;

    err = nvs_set_u32(handle, key.Name, val ? 1 : 0);
    SuccessOrExit(err);

    // Commit the value to the persistent store.
    err = nvs_commit(handle);
    SuccessOrExit(err);

    WeaveLogProgress(DeviceLayer, "NVS set: %s/%s = %s", key.Namespace, key.Name, val ? "true" : "false");

exit:
    if (needClose)
    {
        nvs_close(handle);
    }

    return err;
}

WEAVE_ERROR ESP32Config::WriteConfigValue(Key key, uint32_t val)
{
    WEAVE_ERROR err;
    nvs_handle handle;
    bool needClose = false;

    err = nvs_open(key.Namespace, NVS_READWRITE, &handle);
    SuccessOrExit(err);
    needClose = true;

    err = nvs_set_u32(handle, key.Name, val);
    SuccessOrExit(err);

    // Commit the value to the persistent store.
    err = nvs_commit(handle);
    SuccessOrExit(err);

    WeaveLogProgress(DeviceLayer, "NVS set: %s/%s = %" PRIu32 " (0x%" PRIX32 ")", key.Namespace, key.Name, val, val);

exit:
    if (needClose)
    {
        nvs_close(handle);
    }

    return err;
}

WEAVE_ERROR ESP32Config::WriteConfigValue(Key key, uint64_t val)
{
    WEAVE_ERROR err;
    nvs_handle handle;
    bool needClose = false;

    err = nvs_open(key.Namespace, NVS_READWRITE, &handle);
    SuccessOrExit(err);
    needClose = true;

    err = nvs_set_u64(handle, key.Name, val);
    SuccessOrExit(err);

    // Commit the value to the persistent store.
    err = nvs_commit(handle);
    SuccessOrExit(err);

    WeaveLogProgress(DeviceLayer, "NVS set: %s/%s = %" PRIu64 " (0x%" PRIX64 ")", key.Namespace, key.Name, val, val);

exit:
    if (needClose)
    {
        nvs_close(handle);
    }

    return err;
}

WEAVE_ERROR ESP32Config::WriteConfigValueStr(Key key, const char * str)
{
    WEAVE_ERROR err;
    nvs_handle handle;
    bool needClose = false;

    if (str != NULL)
    {
        err = nvs_open(key.Namespace, NVS_READWRITE, &handle);
        SuccessOrExit(err);
        needClose = true;

        err = nvs_set_str(handle, key.Name, str);
        SuccessOrExit(err);

        // Commit the value to the persistent store.
        err = nvs_commit(handle);
        SuccessOrExit(err);

        WeaveLogProgress(DeviceLayer, "NVS set: %s/%s = \"%s\"", key.Namespace, key.Name, str);
    }

    else
    {
        err = ClearConfigValue(key);
        SuccessOrExit(err);
    }

exit:
    if (needClose)
    {
        nvs_close(handle);
    }

    return err;
}

WEAVE_ERROR ESP32Config::WriteConfigValueStr(Key key, const char * str, size_t strLen)
{
    WEAVE_ERROR err;
    char * strCopy = NULL;

    if (str != NULL)
    {
        strCopy = strndup(str, strLen);
        VerifyOrExit(strCopy != NULL, err = WEAVE_ERROR_NO_MEMORY);
    }

    err = ESP32Config::WriteConfigValueStr(key, strCopy);

exit:
    if (strCopy != NULL)
    {
        free(strCopy);
    }
    return err;
}

WEAVE_ERROR ESP32Config::WriteConfigValueBin(Key key, const uint8_t * data, size_t dataLen)
{
    WEAVE_ERROR err;
    nvs_handle handle;
    bool needClose = false;

    if (data != NULL)
    {
        err = nvs_open(key.Namespace, NVS_READWRITE, &handle);
        SuccessOrExit(err);
        needClose = true;

        err = nvs_set_blob(handle, key.Name, data, dataLen);
        SuccessOrExit(err);

        // Commit the value to the persistent store.
        err = nvs_commit(handle);
        SuccessOrExit(err);

        WeaveLogProgress(DeviceLayer, "NVS set: %s/%s = (blob length %" PRId32 ")", key.Namespace, key.Name, dataLen);
    }

    else
    {
        err = ClearConfigValue(key);
        SuccessOrExit(err);
    }

exit:
    if (needClose)
    {
        nvs_close(handle);
    }

    return err;
}

WEAVE_ERROR ESP32Config::ClearConfigValue(Key key)
{
    WEAVE_ERROR err;
    nvs_handle handle;
    bool needClose = false;

    err = nvs_open(key.Namespace, NVS_READWRITE, &handle);
    SuccessOrExit(err);
    needClose = true;

    err = nvs_erase_key(handle, key.Name);
    if (err == ESP_ERR_NVS_NOT_FOUND)
    {
        ExitNow(err = WEAVE_NO_ERROR);
    }
    SuccessOrExit(err);

    // Commit the value to the persistent store.
    err = nvs_commit(handle);
    SuccessOrExit(err);

    WeaveLogProgress(DeviceLayer, "NVS erase: %s/%s", key.Namespace, key.Name);

exit:
    if (needClose)
    {
        nvs_close(handle);
    }

    return err;
}

bool ESP32Config::ConfigValueExists(Key key)
{
    WEAVE_ERROR err;
    nvs_handle handle;
    bool needClose = false;

    err = nvs_open(key.Namespace, NVS_READONLY, &handle);
    SuccessOrExit(err);
    needClose = true;

    // This code is a rather unfortunate consequence of the limitations
    // in the ESP NVS API.  As defined, there is no API for determining
    // whether a particular key exists.  Furthermore, calling one of the
    // nvs_get_* APIs will result in a ESP_ERR_NVS_NOT_FOUND in the case
    // where the key exists, but the requested data type does not match.
    // (This is true despite the existence of the ESP_ERR_NVS_TYPE_MISMATCH
    // error, which would seem to be the obvious correct response).
    //
    // Thus the solution is to exhaustively check for the key using
    // each possible value type.
    {
        uint8_t v;
        err = nvs_get_u8(handle, key.Name, &v);
    }
    if (err == ESP_ERR_NVS_NOT_FOUND)
    {
        int8_t v;
        err = nvs_get_i8(handle, key.Name, &v);
    }
    if (err == ESP_ERR_NVS_NOT_FOUND)
    {
        uint16_t v;
        err = nvs_get_u16(handle, key.Name, &v);
    }
    if (err == ESP_ERR_NVS_NOT_FOUND)
    {
        int16_t v;
        err = nvs_get_i16(handle, key.Name, &v);
    }
    if (err == ESP_ERR_NVS_NOT_FOUND)
    {
        uint32_t v;
        err = nvs_get_u32(handle, key.Name, &v);
    }
    if (err == ESP_ERR_NVS_NOT_FOUND)
    {
        int32_t v;
        err = nvs_get_i32(handle, key.Name, &v);
    }
    if (err == ESP_ERR_NVS_NOT_FOUND)
    {
        uint64_t v;
        err = nvs_get_u64(handle, key.Name, &v);
    }
    if (err == ESP_ERR_NVS_NOT_FOUND)
    {
        int64_t v;
        err = nvs_get_i64(handle, key.Name, &v);
    }
    if (err == ESP_ERR_NVS_NOT_FOUND)
    {
        size_t sz;
        err = nvs_get_str(handle, key.Name, NULL, &sz);
    }
    if (err == ESP_ERR_NVS_NOT_FOUND)
    {
        size_t sz;
        err = nvs_get_blob(handle, key.Name, NULL, &sz);
    }

    // In the case of blob and string, ESP_ERR_NVS_INVALID_LENGTH means
    // the key exists.
    if (err == ESP_ERR_NVS_INVALID_LENGTH)
    {
        err = ESP_OK;
    }

exit:
    if (needClose)
    {
        nvs_close(handle);
    }
    return err == ESP_OK;
}

WEAVE_ERROR ESP32Config::EnsureNamespace(const char * ns)
{
    WEAVE_ERROR err;
    nvs_handle handle;
    bool needClose = false;

    err = nvs_open(ns, NVS_READONLY, &handle);
    if (err == ESP_ERR_NVS_NOT_FOUND)
    {
        err = nvs_open(ns, NVS_READWRITE, &handle);
        SuccessOrExit(err);
        needClose = true;

        err = nvs_commit(handle);
        SuccessOrExit(err);
    }
    SuccessOrExit(err);
    needClose = true;

exit:
    if (needClose)
    {
        nvs_close(handle);
    }
    return err;
}

WEAVE_ERROR ESP32Config::ClearNamespace(const char * ns)
{
    WEAVE_ERROR err;
    nvs_handle handle;
    bool needClose = false;

    err = nvs_open(ns, NVS_READWRITE, &handle);
    SuccessOrExit(err);
    needClose = true;

    err = nvs_erase_all(handle);
    SuccessOrExit(err);

    err = nvs_commit(handle);
    SuccessOrExit(err);

exit:
    if (needClose)
    {
        nvs_close(handle);
    }
    return err;
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace Weave
} // namespace nl
