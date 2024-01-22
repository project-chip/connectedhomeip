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

#include <easyflash.h>
#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <platform/bouffalolab/common/BLConfig.h>

namespace chip {
namespace DeviceLayer {
namespace Internal {

void BLConfig::Init(void)
{
    env_node_obj node;

    easyflash_init();
    ef_load_env_cache();

    if (ef_get_env_obj(kBLKey_factoryResetFlag, &node))
    {
        ef_print_env_cb([](env_node_obj_t env, void * arg1, void * arg2) {
            if (ENV_WRITE == env->status)
            {
                env->name[env->name_len] = '\0';
                if (strncmp(kBLKey_factoryResetFlag, env->name, sizeof(env->name) - 1))
                {
                    /** delete all key=value except factory reset flag */
                    ef_del_and_save_env(env->name);
                }
            }

            return false;
        });

        ef_del_and_save_env(kBLKey_factoryResetFlag);
        ef_env_set_default();
        ef_load_env_cache();
    }
}

CHIP_ERROR BLConfig::ReadConfigValue(const char * key, uint8_t * val, size_t size, size_t & readsize)
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

CHIP_ERROR BLConfig::ReadConfigValue(const char * key, bool & val)
{
    size_t readlen = 0;
    return ReadConfigValue(key, (uint8_t *) &val, 1, readlen);
}

CHIP_ERROR BLConfig::ReadConfigValue(const char * key, uint32_t & val)
{
    size_t readlen = 0;
    return ReadConfigValue(key, (uint8_t *) &val, sizeof(val), readlen);
}

CHIP_ERROR BLConfig::ReadConfigValue(const char * key, uint64_t & val)
{
    size_t readlen = 0;
    return ReadConfigValue(key, (uint8_t *) &val, sizeof(val), readlen);
}

CHIP_ERROR BLConfig::ReadConfigValueStr(const char * key, char * buf, size_t bufSize, size_t & outLen)
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

CHIP_ERROR BLConfig::ReadConfigValueBin(const char * key, uint8_t * buf, size_t bufSize, size_t & outLen)
{
    size_t readlen = 0;
    if (CHIP_NO_ERROR == ReadConfigValue(key, (uint8_t *) buf, bufSize, readlen))
    {
        outLen = readlen;
        return CHIP_NO_ERROR;
    }

    return CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND;
}

CHIP_ERROR BLConfig::WriteConfigValue(const char * key, uint8_t * val, size_t size)
{
    EfErrCode ret = EF_NO_ERR;

    ef_port_env_lock();

    if (size)
    {
        if (val)
        {
            ret = ef_set_env_blob(key, val, size);
        }
        else
        {
            ret = EF_ENV_ARG_ERR;
        }
    }
    else
    {
        uint32_t value_null = 0;
        ret                 = ef_set_env_blob(key, &value_null, size);
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

CHIP_ERROR BLConfig::WriteConfigValue(const char * key, bool val)
{
    return WriteConfigValue(key, (uint8_t *) &val, sizeof(val));
}

CHIP_ERROR BLConfig::WriteConfigValue(const char * key, uint32_t val)
{
    return WriteConfigValue(key, (uint8_t *) &val, sizeof(val));
}

CHIP_ERROR BLConfig::WriteConfigValue(const char * key, uint64_t val)
{
    return WriteConfigValue(key, (uint8_t *) &val, sizeof(val));
}

CHIP_ERROR BLConfig::WriteConfigValueStr(const char * key, const char * str)
{
    return WriteConfigValue(key, (uint8_t *) str, strlen(str) + 1);
}

CHIP_ERROR BLConfig::WriteConfigValueStr(const char * key, const char * str, size_t strLen)
{
    return WriteConfigValue(key, (uint8_t *) str, strLen);
}

CHIP_ERROR BLConfig::WriteConfigValueBin(const char * key, const uint8_t * data, size_t dataLen)
{
    return WriteConfigValue(key, (uint8_t *) data, dataLen);
}

CHIP_ERROR BLConfig::ClearConfigValue(const char * key)
{
    return EF_NO_ERR == ef_del_env(key) ? CHIP_NO_ERROR : CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND;
}

CHIP_ERROR BLConfig::FactoryResetConfig(void)
{
    /** set __factory_reset_pending here, let do factory reset operation during startup  */
    ef_set_and_save_env(kBLKey_factoryResetFlag, "pending");

    return CHIP_NO_ERROR;
}

void BLConfig::RunConfigUnitTest() {}

bool BLConfig::ConfigValueExists(const char * key)
{
    env_node_obj node;

    return ef_get_env_obj(key, &node);
}

CHIP_ERROR BLConfig::ReadKVS(const char * key, void * value, size_t value_size, size_t * read_bytes_size, size_t offset_bytes)
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

CHIP_ERROR BLConfig::WriteKVS(const char * key, const void * value, size_t value_size)
{
    EfErrCode ret = EF_NO_ERR;

    ef_port_env_lock();

    if (value_size)
    {
        if (value)
        {
            ret = ef_set_env_blob(key, value, value_size);
        }
        else
        {
            ret = EF_ENV_ARG_ERR;
        }
    }
    else
    {
        uint32_t value_null = 0;
        ret                 = ef_set_env_blob(key, &value_null, value_size);
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

CHIP_ERROR BLConfig::ClearKVS(const char * key)
{
    ef_port_env_lock();

    ef_del_env(key);

    ef_port_env_unlock();

    return CHIP_NO_ERROR;
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
