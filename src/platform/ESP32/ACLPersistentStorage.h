/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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
#pragma once

#include <lib/core/CHIPPersistentStorageDelegate.h>
#include <limits>
#include <platform/ESP32/ESP32Config.h>

class ACLPersistentStorage : public chip::PersistentStorageDelegate
{
public:
    CHIP_ERROR SyncGetKeyValue(const char * key, void * buffer, uint16_t & size) override
    {
        size_t outSize                                       = 0;
        chip::DeviceLayer::Internal::ESP32Config::Key getKey = {
            chip::DeviceLayer::Internal::ESP32Config::kConfigNamespace_ChipConfig, key
        };

        CHIP_ERROR err =
            chip::DeviceLayer::Internal::ESP32Config::ReadConfigValueBin(getKey, static_cast<uint8_t *>(buffer), size, outSize);
        if (err == CHIP_NO_ERROR)
        {
            if (outSize > std::numeric_limits<uint16_t>::max())
            {
                err = CHIP_ERROR_BUFFER_TOO_SMALL;
            }
            else
            {
                size = static_cast<uint16_t>(outSize);
            }
        }
        return err;
    }

    CHIP_ERROR SyncSetKeyValue(const char * key, const void * value, uint16_t size) override
    {
        chip::DeviceLayer::Internal::ESP32Config::Key setKey = {
            chip::DeviceLayer::Internal::ESP32Config::kConfigNamespace_ChipConfig, key
        };
        return chip::DeviceLayer::Internal::ESP32Config::WriteConfigValueBin(setKey, static_cast<const uint8_t *>(value),
                                                                             static_cast<size_t>(size));
    }

    CHIP_ERROR SyncDeleteKeyValue(const char * key) override
    {
        chip::DeviceLayer::Internal::ESP32Config::Key delKey = {
            chip::DeviceLayer::Internal::ESP32Config::kConfigNamespace_ChipConfig, key
        };
        return chip::DeviceLayer::Internal::ESP32Config::ClearConfigValue(delKey);
    }
};
