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

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string>
#include <regex>

#include <lfs.h>
extern "C" {
#include <lfs_port.h>
}

#include <platform/internal/CHIPDeviceLayerInternal.h>
#include <platform/bouffalolab/common/BLConfig.h>

#ifndef BLCONFIG_LFS_NAMESPACE
#define BLCONFIG_LFS_NAMESPACE "/_blcfg_"
#endif

#ifndef BLCONFIG_SLASH
#define BLCONFIG_SLASH "-"
#endif

namespace chip {
namespace DeviceLayer {
namespace Internal {

static lfs_t      * blconfig_lfs = nullptr;

static inline std::string  blcfg_convert_key(std::string key, std::string lfs_ns = std::string("")) 
{
    return lfs_ns + std::string("/") + regex_replace(key, std::regex("/"), std::string(BLCONFIG_SLASH));
}

static CHIP_ERROR blcfg_do_factory_reset(void) 
{
    CHIP_ERROR      err = CHIP_NO_ERROR;
    int             ret;
    struct lfs_info stat;
    lfs_file_t      file;
    lfs_dir_t       dir = {};
    std::string     factory_reset_key = blcfg_convert_key(std::string(BLConfig::kBLKey_factoryResetFlag));

    blconfig_lfs->cfg->lock(blconfig_lfs->cfg);

    ret = lfs_stat(blconfig_lfs, const_cast<char*>(factory_reset_key.c_str()), &stat);

    if (LFS_ERR_OK == ret) {
        err = CHIP_ERROR_PERSISTED_STORAGE_FAILED;

        do {
            ret = lfs_file_open(blconfig_lfs, &file, const_cast<char*>(factory_reset_key.c_str()), LFS_O_RDONLY);
            VerifyOrExit(ret == LFS_ERR_OK, err = CHIP_ERROR_PERSISTED_STORAGE_FAILED);
            lfs_file_close(blconfig_lfs, &file);

            ret = lfs_dir_open(blconfig_lfs, &dir, BLCONFIG_LFS_NAMESPACE);
            VerifyOrExit(ret == LFS_ERR_OK, err = CHIP_ERROR_PERSISTED_STORAGE_FAILED);

            while (1) {
                ret = lfs_dir_read(blconfig_lfs, &dir, &stat);
                if (ret <= 0) {
                    break;
                }

                if (stat.type != LFS_TYPE_REG) {
                    continue;
                }
                std::string delete_key = blcfg_convert_key(std::string(stat.name), std::string(BLCONFIG_LFS_NAMESPACE));

                ret = lfs_remove(blconfig_lfs, const_cast<char*>(delete_key.c_str())); 
                if (ret != LFS_ERR_OK) {
                    break;
                }
            }

            lfs_dir_close(blconfig_lfs, &dir);

            ret = lfs_remove(blconfig_lfs, const_cast<char*>(factory_reset_key.c_str()));
            if (ret != LFS_ERR_OK) {
                break;
            }

            err = CHIP_NO_ERROR;
        } while(0);
    }

exit: 
    blconfig_lfs->cfg->unlock(blconfig_lfs->cfg);

    return err;
}

void BLConfig::Init(void)
{
    CHIP_ERROR      err = CHIP_NO_ERROR;
    int             ret;
    struct lfs_info stat;
    
    blconfig_lfs = lfs_xip_init();
    VerifyOrExit(blconfig_lfs != NULL, err = CHIP_ERROR_PERSISTED_STORAGE_FAILED);

    /* init namespace */
    ret = lfs_stat(blconfig_lfs, BLCONFIG_LFS_NAMESPACE, &stat);
    if (ret != LFS_ERR_OK) {
        ret = lfs_mkdir(blconfig_lfs, BLCONFIG_LFS_NAMESPACE);
        VerifyOrExit(ret == LFS_ERR_OK, err = CHIP_ERROR_PERSISTED_STORAGE_FAILED);

        ret = lfs_stat(blconfig_lfs, BLCONFIG_LFS_NAMESPACE, &stat);
    }
    VerifyOrExit(ret == LFS_ERR_OK && stat.type == LFS_TYPE_DIR, err = CHIP_ERROR_PERSISTED_STORAGE_FAILED);

    err = blcfg_do_factory_reset();
exit:
    configASSERT(err == CHIP_NO_ERROR);
}

CHIP_ERROR BLConfig::ReadConfigValue(const char * key, uint8_t * val, size_t size, size_t & readsize)
{
    CHIP_ERROR   err = CHIP_NO_ERROR;
    int          ret = LFS_ERR_OK;
    lfs_file_t   file;
    std::string  read_key = blcfg_convert_key(std::string(key), std::string(BLCONFIG_LFS_NAMESPACE));

    blconfig_lfs->cfg->lock(blconfig_lfs->cfg);

    ret = lfs_file_open(blconfig_lfs, &file, const_cast<char*>(read_key.c_str()), LFS_O_RDONLY);
    VerifyOrExit(ret == LFS_ERR_OK, err = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND);

    if (val && size) {
        ret = lfs_file_read(blconfig_lfs, &file, val, size);
        readsize = ret;
    }
    lfs_file_close(blconfig_lfs, &file);

exit:
    blconfig_lfs->cfg->unlock(blconfig_lfs->cfg);

    return err;
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
    int          ret = LFS_ERR_OK;
    CHIP_ERROR   err = CHIP_NO_ERROR;
    lfs_file_t   file;
    std::string  write_key = blcfg_convert_key(std::string(key), std::string(BLCONFIG_LFS_NAMESPACE));

    blconfig_lfs->cfg->lock(blconfig_lfs->cfg);

    ret = lfs_file_open(blconfig_lfs, &file, const_cast<char*>(write_key.c_str()), LFS_O_CREAT | LFS_O_RDWR);
    VerifyOrExit(ret == LFS_ERR_OK, err = CHIP_ERROR_PERSISTED_STORAGE_FAILED);

    lfs_file_write(blconfig_lfs, &file, val, size);
    lfs_file_close(blconfig_lfs, &file);

exit: 
    blconfig_lfs->cfg->unlock(blconfig_lfs->cfg);

    return err;
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
    std::string  delete_key = blcfg_convert_key(std::string(key), std::string(BLCONFIG_LFS_NAMESPACE));

    int ret = lfs_remove(blconfig_lfs, const_cast<char*>(delete_key.c_str()));

    return (ret >= LFS_ERR_OK || ret == LFS_ERR_NOENT) ? CHIP_NO_ERROR: CHIP_ERROR_PERSISTED_STORAGE_FAILED;
}

CHIP_ERROR BLConfig::FactoryResetConfig(void)
{
    int          ret = LFS_ERR_OK;
    lfs_file_t   file;
    std::string  reset_key = blcfg_convert_key(std::string(kBLKey_factoryResetFlag));
    const char   reset_key_value[] = "pending";

    blconfig_lfs->cfg->lock(blconfig_lfs->cfg);

    ret = lfs_file_open(blconfig_lfs, &file, const_cast<char*>(reset_key.c_str()), LFS_O_CREAT | LFS_O_RDWR);
    if (ret != LFS_ERR_OK) {
        blconfig_lfs->cfg->unlock(blconfig_lfs->cfg);
        return CHIP_ERROR_PERSISTED_STORAGE_FAILED;
    }

    lfs_file_write(blconfig_lfs, &file, reset_key_value, sizeof(reset_key_value));
    lfs_file_close(blconfig_lfs, &file);

    blconfig_lfs->cfg->unlock(blconfig_lfs->cfg);

    return blcfg_do_factory_reset();
}

void BLConfig::RunConfigUnitTest() {}

bool BLConfig::ConfigValueExists(const char * key)
{
    std::string     exist_key = blcfg_convert_key(std::string(key), std::string(BLCONFIG_LFS_NAMESPACE));
    struct lfs_info stat;

    return lfs_stat(blconfig_lfs, const_cast<char*>(exist_key.c_str()), &stat) == LFS_ERR_OK;
}

CHIP_ERROR BLConfig::ReadKVS(const char * key, void * value, size_t value_size, size_t * read_bytes_size, size_t offset_bytes)
{
    CHIP_ERROR   err = CHIP_NO_ERROR;
    int          ret = LFS_ERR_OK;
    lfs_file_t   file;
    std::string  read_key = blcfg_convert_key(std::string(key), std::string(BLCONFIG_LFS_NAMESPACE));

    if (read_bytes_size) {
        * read_bytes_size = 0;
    }

    blconfig_lfs->cfg->lock(blconfig_lfs->cfg);

    ret = lfs_file_open(blconfig_lfs, &file, const_cast<char*>(read_key.c_str()), LFS_O_RDONLY);
    VerifyOrExit(ret == LFS_ERR_OK, err = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND);

    if (value && value_size) {
        do {
            ret = 0;
            if (offset_bytes > 0 && lfs_file_seek(blconfig_lfs, &file, offset_bytes, 0) < 0) {
                err = CHIP_ERROR_PERSISTED_STORAGE_FAILED;
                break;
            }
            ret = lfs_file_read(blconfig_lfs, &file, value, value_size);
            if (ret > 0) {
                * read_bytes_size = ret;
            }
        } while (0);
    }

    lfs_file_close(blconfig_lfs, &file);
exit:
    blconfig_lfs->cfg->unlock(blconfig_lfs->cfg);

    return err;
}

CHIP_ERROR BLConfig::WriteKVS(const char * key, const void * value, size_t value_size)
{
    return WriteConfigValueBin(key, (const uint8_t *)value, value_size);
}

CHIP_ERROR BLConfig::ClearKVS(const char * key)
{
    return ClearConfigValue(key);
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
