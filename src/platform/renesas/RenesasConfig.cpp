/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2019 Nest Labs, Inc.
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
 * @file
 * Utilities for accessing persisted device configuration.
 */
/* this file behaves like a config.h, comes first */
#include <platform/internal/CHIPDeviceLayerInternal.h>

namespace chip {
namespace DeviceLayer {
namespace Internal {

CHIP_ERROR RenesasConfig::Init()
{
    return CHIP_NO_ERROR;
}

template <typename T>
CHIP_ERROR RenesasConfig::ReadConfigValue(Key key, T & val)
{
    size_t read_count;
    ReturnErrorOnFailure(ReadConfigValueBin(key, &val, sizeof(val), read_count));
    VerifyOrReturnError(sizeof(val) == read_count, CHIP_ERROR_PERSISTED_STORAGE_FAILED);
    return CHIP_NO_ERROR;
}

CHIP_ERROR RenesasConfig::ReadConfigValueStr(Key key, char * buf, size_t bufSize, size_t & outLen)
{
    return ReadConfigValueBin(key, buf, bufSize, outLen);
}

CHIP_ERROR RenesasConfig::ReadConfigValueBin(Key key, uint8_t * buf, size_t bufSize, size_t & outLen)
{
    return ReadConfigValueBin(key, static_cast<void *>(buf), bufSize, outLen);
}

CHIP_ERROR RenesasConfig::ReadConfigValueBin(Key key, void * buf, size_t bufSize, size_t & outLen)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

template CHIP_ERROR RenesasConfig::ReadConfigValue(Key key, bool & val);
template CHIP_ERROR RenesasConfig::ReadConfigValue(Key key, uint32_t & val);
template CHIP_ERROR RenesasConfig::ReadConfigValue(Key key, uint64_t & val);

template <typename T>
CHIP_ERROR RenesasConfig::WriteConfigValue(Key key, T val)
{
    return WriteConfigValueBin(key, &val, sizeof(val));
}

template CHIP_ERROR RenesasConfig::WriteConfigValue(Key key, bool val);
template CHIP_ERROR RenesasConfig::WriteConfigValue(Key key, uint32_t val);
template CHIP_ERROR RenesasConfig::WriteConfigValue(Key key, uint64_t val);

CHIP_ERROR RenesasConfig::WriteConfigValueStr(Key key, const char * str)
{
    return WriteConfigValueStr(key, str, (str != NULL) ? strlen(str) : 0);
}

CHIP_ERROR RenesasConfig::WriteConfigValueStr(Key key, const char * str, size_t strLen)
{
    return WriteConfigValueBin(key, str, strLen);
}

CHIP_ERROR RenesasConfig::WriteConfigValueBin(Key key, const uint8_t * data, size_t dataLen)
{
    return WriteConfigValueBin(key, static_cast<const void *>(data), dataLen);
}

CHIP_ERROR RenesasConfig::WriteConfigValueBin(Key key, const void * data, size_t dataLen)
{
    /* Skip writing because the write API reports error result for zero length data. */
    if (dataLen == 0)
        return CHIP_NO_ERROR;

    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR RenesasConfig::ClearConfigValue(Key key)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

bool RenesasConfig::ConfigValueExists(Key key)
{
    uint8_t val;
    return ChipError::IsSuccess(ReadConfigValue(key, val));
}

CHIP_ERROR RenesasConfig::FactoryResetConfig(void)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

void RenesasConfig::RunConfigUnitTest(void) {}


} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
