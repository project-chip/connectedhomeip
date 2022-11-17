/*
 *    Copyright (c) 2022 Project CHIP Authors
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

#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <platform/bouffalolab/BL702/bl702Config.h>

#include <lib/core/CHIPEncoding.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CHIPMemString.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>

// #include <blog.h>
#include <easyflash.h>

namespace chip {
namespace DeviceLayer {
namespace Internal {

// // Prefix used for Easyflash keys that contain Chip group encryption keys.
CHIP_ERROR BL702Config::Init()
{
    return CHIP_NO_ERROR;
}

CHIP_ERROR BL702Config::ReadConfigValue(const char * key, uint8_t * val, size_t size, size_t & readsize)
{
    env_node_obj node;

    ef_port_env_lock();

    if (true == ef_get_env_obj(key, &node))
    {

        readsize = 0;
        if (val && size)
        {
            if (size > node.value_len)
            {
                readsize = node.value_len;
            }
            else
            {
                readsize = size;
            }
        }

        ef_port_read(node.addr.value, (uint32_t *) val, readsize);

        ef_port_env_unlock();

        return CHIP_NO_ERROR;
    }

    ef_port_env_unlock();

    return CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND;
}

CHIP_ERROR BL702Config::ReadConfigValue(const char * key, bool & val)
{
    size_t readlen = 0;
    return ReadConfigValue(key, (uint8_t *) &val, 1, readlen);
}

CHIP_ERROR BL702Config::ReadConfigValue(const char * key, uint32_t & val)
{
    size_t readlen = 0;
    return ReadConfigValue(key, (uint8_t *) &val, sizeof(val), readlen);
}

CHIP_ERROR BL702Config::ReadConfigValue(const char * key, uint64_t & val)
{
    size_t readlen = 0;
    return ReadConfigValue(key, (uint8_t *) &val, sizeof(val), readlen);
}

CHIP_ERROR BL702Config::ReadConfigValueStr(const char * key, char * buf, size_t bufSize, size_t & outLen)
{
    size_t readlen = 0;
    if (CHIP_NO_ERROR == ReadConfigValue(key, (uint8_t *) buf, bufSize, readlen))
    {
        outLen = readlen;
        if (readlen && readlen < bufSize)
        {
            buf[readlen] = '\0';
        }

        return CHIP_NO_ERROR;
    }

    return CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND;
}

CHIP_ERROR BL702Config::ReadConfigValueBin(const char * key, uint8_t * buf, size_t bufSize, size_t & outLen)
{
    size_t readlen = 0;
    if (CHIP_NO_ERROR == ReadConfigValue(key, (uint8_t *) buf, bufSize, readlen))
    {
        outLen = readlen;
        return CHIP_NO_ERROR;
    }

    return CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND;
}

CHIP_ERROR BL702Config::WriteConfigValue(const char * key, uint8_t * val, size_t size)
{
    EfErrCode ret = EF_NO_ERR;

    ef_port_env_lock();

    if (size && val)
    {
        ret = ef_set_env_blob(key, val, size);
    }

    ef_port_env_unlock();

    if (ret == EF_NO_ERR)
    {
        return CHIP_NO_ERROR;
    }
    else
    {
        return CHIP_ERROR_PERSISTED_STORAGE_FAILED;
    }
}

CHIP_ERROR BL702Config::WriteConfigValue(const char * key, bool val)
{
    return WriteConfigValue(key, (uint8_t *) &val, sizeof(val));
}

CHIP_ERROR BL702Config::WriteConfigValue(const char * key, uint32_t val)
{
    return WriteConfigValue(key, (uint8_t *) &val, sizeof(val));
}

CHIP_ERROR BL702Config::WriteConfigValue(const char * key, uint64_t val)
{
    return WriteConfigValue(key, (uint8_t *) &val, sizeof(val));
}

CHIP_ERROR BL702Config::WriteConfigValueStr(const char * key, const char * str)
{
    return WriteConfigValue(key, (uint8_t *) str, strlen(str) + 1);
}

CHIP_ERROR BL702Config::WriteConfigValueStr(const char * key, const char * str, size_t strLen)
{
    return WriteConfigValue(key, (uint8_t *) str, strLen);
}

CHIP_ERROR BL702Config::WriteConfigValueBin(const char * key, const uint8_t * data, size_t dataLen)
{
    return WriteConfigValue(key, (uint8_t *) data, dataLen);
}

CHIP_ERROR BL702Config::ClearConfigValue(const char * key)
{
    return EF_NO_ERR == ef_del_env(key) ? CHIP_NO_ERROR : CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND;
}

CHIP_ERROR BL702Config::FactoryResetConfig(void)
{
    // Only reset config section information

    ef_env_set_default();

    return CHIP_NO_ERROR;
}

void BL702Config::RunConfigUnitTest() {}

bool BL702Config::ConfigValueExists(const char * key)
{
    env_node_obj node;

    return ef_get_env_obj(key, &node);
}

CHIP_ERROR BL702Config::ReadKVS(const char * key, void * value, size_t value_size, size_t * read_bytes_size, size_t offset_bytes)
{
    size_t read_len = 0, datalen;
    env_node_obj node;

    ef_port_env_lock();

    if (true == ef_get_env_obj(key, &node))
    {
        if (offset_bytes > node.value_len)
        {
            ef_port_env_unlock();
            return CHIP_ERROR_INVALID_ARGUMENT;
        }

        datalen = node.value_len - offset_bytes;
        if (value_size && value)
        {

            if (value_size > datalen)
            {
                read_len = datalen;
            }
            else
            {
                read_len = value_size;
            }

            ef_port_read(node.addr.value + offset_bytes, (uint32_t *) value, read_len);

            if (read_bytes_size)
            {
                *read_bytes_size = read_len;
            }
        }
        else
        {
            if (read_bytes_size)
            {
                *read_bytes_size = 0;
            }
        }

        ef_port_env_unlock();

        return CHIP_NO_ERROR;
    }

    ef_port_env_unlock();

    return CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND;
}

CHIP_ERROR BL702Config::WriteKVS(const char * key, const void * value, size_t value_size)
{
    EfErrCode ret = EF_NO_ERR;

    ef_port_env_lock();

    if (value && value_size)
    {
        ret = ef_set_env_blob(key, value, value_size);
    }

    ef_port_env_unlock();

    if (ret == EF_NO_ERR)
    {
        return CHIP_NO_ERROR;
    }
    else
    {
        return CHIP_ERROR_PERSISTED_STORAGE_FAILED;
    }
}

CHIP_ERROR BL702Config::ClearKVS(const char * key)
{
    ef_port_env_lock();

    ef_del_env(key);

    ef_port_env_unlock();

    return CHIP_NO_ERROR;
}

CHIP_ERROR BL702Config::WriteWiFiInfo(const char * ssid, const char * passwd)
{
    char *pssid = (char *) ssid, *ppass = (char *) passwd;

    ef_port_env_lock();

    if (EF_NO_ERR == ef_set_env_blob(kBLConfigKey_wifissid, pssid, strlen(pssid)) &&
        EF_NO_ERR == ef_set_env_blob(kBLConfigKey_wifipassword, ppass, strlen(ppass)))
    {

        ef_port_env_unlock();

        return CHIP_NO_ERROR;
    }

    ef_port_env_unlock();

    return CHIP_ERROR_PERSISTED_STORAGE_FAILED;
}

CHIP_ERROR BL702Config::ReadWiFiInfo(const char * ssid, uint32_t ssid_size, const char * passwd, uint32_t passwd_size)
{
    size_t saved_value_len = 0;
    char *pssid = (char *) ssid, *ppass = (char *) passwd;

    memset((void *) ssid, 0, ssid_size);
    memset((void *) passwd, 0, passwd_size);

    ef_port_env_lock();

    ef_get_env_blob(kBLConfigKey_wifissid, (void *) pssid, ssid_size, &saved_value_len);
    if (saved_value_len != 0)
    {
        saved_value_len = 0;
        ef_get_env_blob(kBLConfigKey_wifipassword, (void *) ppass, passwd_size, &saved_value_len);
    }

    ef_port_env_unlock();

    return saved_value_len ? CHIP_NO_ERROR : CHIP_ERROR_PERSISTED_STORAGE_FAILED;
}

CHIP_ERROR BL702Config::ClearWiFiInfo(void)
{
    ef_port_env_lock();
    if (EF_NO_ERR == ef_del_env(kBLConfigKey_wifissid) && EF_NO_ERR == ef_del_env(kBLConfigKey_wifipassword))
    {
        ef_port_env_unlock();

        return CHIP_NO_ERROR;
    }
    ef_port_env_unlock();

    return CHIP_ERROR_PERSISTED_STORAGE_FAILED;
}

bool BL702Config::isWiFiInfoSaved()
{
    env_node_obj node;

    char tssid[64], tpasswd[64];
    ReadWiFiInfo(tssid, sizeof(tssid), tpasswd, sizeof(tpasswd));

    return ef_get_env_obj(kBLConfigKey_wifissid, &node) && ef_get_env_obj(kBLConfigKey_wifipassword, &node);
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
