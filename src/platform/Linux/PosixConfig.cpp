/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

#include <platform/internal/CHIPDeviceLayerInternal.h>
#include <platform/Linux/PosixConfig.h>
#include <support/CodeUtils.h>
#include <core/CHIPEncoding.h>

namespace chip {
namespace DeviceLayer {
namespace Internal {

CHIP_ERROR PosixConfig::Init()
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    return err;
}

CHIP_ERROR PosixConfig::ReadConfigValue(Key key, bool & val)
{
    // TODO(#738)
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR PosixConfig::ReadConfigValue(Key key, uint32_t & val)
{
    // TODO(#738)
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR PosixConfig::ReadConfigValue(Key key, uint64_t & val)
{
    // TODO(#738)
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR PosixConfig::ReadConfigValueStr(Key key, char * buf, size_t bufSize, size_t & outLen)
{
    // TODO(#738)
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR PosixConfig::ReadConfigValueBin(Key key, uint8_t * buf, size_t bufSize, size_t & outLen)
{
    // TODO(#738)
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR PosixConfig::WriteConfigValue(Key key, bool val)
{
    // TODO(#738)
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR PosixConfig::WriteConfigValue(Key key, uint32_t val)
{
    // TODO(#738)
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR PosixConfig::WriteConfigValue(Key key, uint64_t val)
{
    // TODO(#738)
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR PosixConfig::WriteConfigValueStr(Key key, const char * str)
{
    // TODO(#738)
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR PosixConfig::WriteConfigValueStr(Key key, const char * str, size_t strLen)
{
    // TODO(#738)
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR PosixConfig::WriteConfigValueBin(Key key, const uint8_t * data, size_t dataLen)
{
    // TODO(#738)
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR PosixConfig::ClearConfigValue(Key key)
{
    // TODO(#738)
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

bool PosixConfig::ConfigValueExists(Key key)
{
    // TODO(#738)
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR PosixConfig::EnsureNamespace(const char * ns)
{
    // TODO(#738)
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR PosixConfig::ClearNamespace(const char * ns)
{
    // TODO(#738)
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR PosixConfig::FactoryResetConfig(void)
{
    // TODO(#738)
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
