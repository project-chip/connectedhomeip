/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

#include <core/CHIPEncoding.h>
#include <platform/nrfconnect/ZephyrConfig.h>
#include <support/CodeUtils.h>

#include <support/logging/CHIPLogging.h>

namespace chip {
namespace DeviceLayer {
namespace Internal {

CHIP_ERROR ZephyrConfig::Init()
{
    ChipLogError(DeviceLayer, "%s: NOT IMPLEMENTED", __PRETTY_FUNCTION__);
    return CHIP_NO_ERROR;
}

CHIP_ERROR ZephyrConfig::ReadConfigValue(Key key, bool & val)
{
    ChipLogError(DeviceLayer, "%s: NOT IMPLEMENTED", __PRETTY_FUNCTION__);
    return CHIP_NO_ERROR;
}

CHIP_ERROR ZephyrConfig::ReadConfigValue(Key key, uint32_t & val)
{
    ChipLogError(DeviceLayer, "%s: NOT IMPLEMENTED", __PRETTY_FUNCTION__);
    return CHIP_NO_ERROR;
}

CHIP_ERROR ZephyrConfig::ReadConfigValue(Key key, uint64_t & val)
{
    ChipLogError(DeviceLayer, "%s: NOT IMPLEMENTED", __PRETTY_FUNCTION__);
    return CHIP_NO_ERROR;
}

CHIP_ERROR ZephyrConfig::ReadConfigValueStr(Key key, char * buf, size_t bufSize, size_t & outLen)
{
    ChipLogError(DeviceLayer, "%s: NOT IMPLEMENTED", __PRETTY_FUNCTION__);
    return CHIP_NO_ERROR;
}

CHIP_ERROR ZephyrConfig::ReadConfigValueBin(Key key, uint8_t * buf, size_t bufSize, size_t & outLen)
{
    ChipLogError(DeviceLayer, "%s: NOT IMPLEMENTED", __PRETTY_FUNCTION__);
    return CHIP_NO_ERROR;
}

CHIP_ERROR ZephyrConfig::WriteConfigValue(Key key, bool val)
{
    ChipLogError(DeviceLayer, "%s: NOT IMPLEMENTED", __PRETTY_FUNCTION__);
    return CHIP_NO_ERROR;
}

CHIP_ERROR ZephyrConfig::WriteConfigValue(Key key, uint32_t val)
{
    ChipLogError(DeviceLayer, "%s: NOT IMPLEMENTED", __PRETTY_FUNCTION__);
    return CHIP_NO_ERROR;
}

CHIP_ERROR ZephyrConfig::WriteConfigValue(Key key, uint64_t val)
{
    ChipLogError(DeviceLayer, "%s: NOT IMPLEMENTED", __PRETTY_FUNCTION__);
    return CHIP_NO_ERROR;
}

CHIP_ERROR ZephyrConfig::WriteConfigValueStr(Key key, const char * str)
{
    ChipLogError(DeviceLayer, "%s: NOT IMPLEMENTED", __PRETTY_FUNCTION__);
    return CHIP_NO_ERROR;
}

CHIP_ERROR ZephyrConfig::WriteConfigValueStr(Key key, const char * str, size_t strLen)
{
    ChipLogError(DeviceLayer, "%s: NOT IMPLEMENTED", __PRETTY_FUNCTION__);
    return CHIP_NO_ERROR;
}

CHIP_ERROR ZephyrConfig::WriteConfigValueBin(Key key, const uint8_t * data, size_t dataLen)
{
    ChipLogError(DeviceLayer, "%s: NOT IMPLEMENTED", __PRETTY_FUNCTION__);
    return CHIP_NO_ERROR;
}

CHIP_ERROR ZephyrConfig::ClearConfigValue(Key key)
{
    ChipLogError(DeviceLayer, "%s: NOT IMPLEMENTED", __PRETTY_FUNCTION__);
    return CHIP_NO_ERROR;
}

bool ZephyrConfig::ConfigValueExists(Key key)
{
    ChipLogError(DeviceLayer, "%s: NOT IMPLEMENTED", __PRETTY_FUNCTION__);
    return false;
}

CHIP_ERROR ZephyrConfig::EnsureNamespace(const char * ns)
{
    ChipLogError(DeviceLayer, "%s: NOT IMPLEMENTED", __PRETTY_FUNCTION__);
    return CHIP_NO_ERROR;
}

CHIP_ERROR ZephyrConfig::ClearNamespace(const char * ns)
{
    ChipLogError(DeviceLayer, "%s: NOT IMPLEMENTED", __PRETTY_FUNCTION__);
    return CHIP_NO_ERROR;
}

CHIP_ERROR ZephyrConfig::FactoryResetConfig(void)
{
    ChipLogError(DeviceLayer, "%s: NOT IMPLEMENTED", __PRETTY_FUNCTION__);
    return CHIP_NO_ERROR;
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
