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

#pragma once

#include <platform/internal/CHIPDeviceLayerInternal.h>
#include <platform/internal/testing/ConfigUnitTest.h>

#include <string.h>

namespace chip {
namespace DeviceLayer {
namespace Internal {

/**
 * Provides functions and definitions for accessing device configuration information on the ESP32.
 *
 * This class is designed to be mixed-in to concrete implementation classes as a means to
 * provide access to configuration information to generic base classes.
 */
class MbedConfig
{
public:
    using Key = const char *;

    // NVS prefix used to store device configuration information.
    static const char kConfigNamespace_ChipFactory[];
    static const char kConfigNamespace_ChipConfig[];
    static const char kConfigNamespace_ChipCounters[];

    // Key definitions for well-known keys.
    static const Key kConfigKey_SerialNum;
    static const Key kConfigKey_MfrDeviceId;
    static const Key kConfigKey_MfrDeviceCert;
    static const Key kConfigKey_MfrDeviceICACerts;
    static const Key kConfigKey_MfrDevicePrivateKey;
    static const Key kConfigKey_ProductRevision;
    static const Key kConfigKey_ManufacturingDate;
    static const Key kConfigKey_SetupPinCode;
    static const Key kConfigKey_FabricId;
    static const Key kConfigKey_ServiceConfig;
    static const Key kConfigKey_PairedAccountId;
    static const Key kConfigKey_ServiceId;
    static const Key kConfigKey_FabricSecret;
    static const Key kConfigKey_GroupKeyIndex;
    static const Key kConfigKey_LastUsedEpochKeyId;
    static const Key kConfigKey_FailSafeArmed;
    static const Key kConfigKey_WiFiStationSecType;
    static const Key kConfigKey_OperationalDeviceId;
    static const Key kConfigKey_OperationalDeviceCert;
    static const Key kConfigKey_OperationalDeviceICACerts;
    static const Key kConfigKey_OperationalDevicePrivateKey;
    static const Key kConfigKey_SetupDiscriminator;
    static const Key kConfigKey_RegulatoryLocation;
    static const Key kConfigKey_CountryCode;
    static const Key kConfigKey_Breadcrumb;

    // Config value accessors.
    static CHIP_ERROR ReadConfigValue(Key key, bool & val);
    static CHIP_ERROR ReadConfigValue(Key key, uint32_t & val);
    static CHIP_ERROR ReadConfigValue(Key key, uint64_t & val);
    static CHIP_ERROR ReadConfigValueStr(Key key, char * buf, size_t bufSize, size_t & outLen);
    static CHIP_ERROR ReadConfigValueBin(Key key, uint8_t * buf, size_t bufSize, size_t & outLen);
    static CHIP_ERROR WriteConfigValue(Key key, bool val);
    static CHIP_ERROR WriteConfigValue(Key key, uint32_t val);
    static CHIP_ERROR WriteConfigValue(Key key, uint64_t val);
    static CHIP_ERROR WriteConfigValueStr(Key key, const char * str);
    static CHIP_ERROR WriteConfigValueStr(Key key, const char * str, size_t strLen);
    static CHIP_ERROR WriteConfigValueBin(Key key, const uint8_t * data, size_t dataLen);
    static CHIP_ERROR ClearConfigValue(Key key);
    static bool ConfigValueExists(Key key);
    static CHIP_ERROR FactoryResetConfig();

    // NVS Namespace helper functions.
    static CHIP_ERROR ConstructCounterKey(Key id, char * buf, size_t bufSize);
    static CHIP_ERROR ReadCounter(Key counterId, uint32_t & value);
    static CHIP_ERROR WriteCounter(Key counterId, uint32_t value);
    static CHIP_ERROR ClearNamespace(const char * ns);

    static void RunConfigUnitTest(void);
};

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
