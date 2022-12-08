/*
 *
 *    Copyright (c) 2020-2022 Project CHIP Authors
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
 *    @file
 *          Utilities for accessing persisted device configuration on
 *          platforms based on the MediaTek SDK.
 */

#pragma once

#include <functional>

#include <platform/internal/CHIPDeviceLayerInternal.h>

#include "nvdm.h"

namespace chip {
namespace DeviceLayer {
namespace Internal {

/**
 *
 * This implementation uses the MT793X NVDM flash data storage library
 * as the underlying storage layer.
 *
 * NOTE: This class is designed to be mixed-in to the concrete subclass of the
 * GenericConfigurationManagerImpl<> template.  When used this way, the class
 * naturally provides implementations for the delegated members referenced by
 * the template class (e.g. the ReadConfigValue() method).
 */

class MT793XConfig
{
public:
public:
    typedef struct
    {
        const char * Namespace;
        const char * Name;
    } Key;

    // NVM3 key base offsets used by the CHIP Device Layer.
    static constexpr char * kConfigNamespace_ChipFactory  = (char *) "chip-factory";
    static constexpr char * kConfigNamespace_ChipConfig   = (char *) "chip-config";
    static constexpr char * kConfigNamespace_ChipCounters = (char *) "chip-counters";

    // Factory config keys
    static const Key kConfigKey_SerialNum;
    static const Key kConfigKey_UniqueId;
    static const Key kConfigKey_MfrDeviceId;
    static const Key kConfigKey_MfrDeviceCert;
    static const Key kConfigKey_MfrDevicePrivateKey;
    static const Key kConfigKey_ManufacturingDate;
    static const Key kConfigKey_SetupPinCode;
    static const Key kConfigKey_MfrDeviceICACerts;
    static const Key kConfigKey_SetupDiscriminator;
    static const Key kConfigKey_Spake2pIterationCount;
    static const Key kConfigKey_Spake2pSalt;
    static const Key kConfigKey_Spake2pVerifier;

    // CHIP Config Keys
    static const Key kConfigKey_FabricId;
    static const Key kConfigKey_ServiceConfig;
    static const Key kConfigKey_PairedAccountId;
    static const Key kConfigKey_ServiceId;
    static const Key kConfigKey_FabricSecret;
    static const Key kConfigKey_LastUsedEpochKeyId;
    static const Key kConfigKey_FailSafeArmed;
    static const Key kConfigKey_GroupKey;
    static const Key kConfigKey_HardwareVersion;
    static const Key kConfigKey_RegulatoryLocation;
    static const Key kConfigKey_CountryCode;
    static const Key kConfigKey_Breadcrumb;
    static const Key kConfigKey_WiFiSSID;
    static const Key kConfigKey_WiFiPSK;
    static const Key kConfigKey_WiFiSEC;
    static const Key kConfigKey_GroupKeyBase;
    static const Key kConfigKey_GroupKeyMax;

    static const Key kConfigKey_LockUser;
    static const Key kConfigKey_Credential;
    static const Key kConfigKey_LockUserName;
    static const Key kConfigKey_CredentialData;
    static const Key kConfigKey_UserCredentials;
    static const Key kConfigKey_WeekDaySchedules;
    static const Key kConfigKey_YearDaySchedules;
    static const Key kConfigKey_HolidaySchedules;
    // CHIP Counter Keys
    static const Key kConfigKey_BootCount;
    static const Key kConfigKey_TotalOperationalHours;

    static CHIP_ERROR Init(void);

    // Configuration methods used by the GenericConfigurationManagerImpl<> template.
    static CHIP_ERROR ReadConfigValue(Key key, bool & val);
    static CHIP_ERROR ReadConfigValue(Key key, uint32_t & val);
    static CHIP_ERROR ReadConfigValue(Key key, uint64_t & val);
    static CHIP_ERROR ReadConfigValueStr(Key key, char * buf, size_t bufSize, size_t & outLen);
    static CHIP_ERROR ReadConfigValueBin(Key key, uint8_t * buf, size_t bufSize, size_t & outLen);
    static CHIP_ERROR ReadConfigValueCounter(uint8_t counterIdx, uint32_t & val);
    static CHIP_ERROR WriteConfigValue(Key key, bool val);
    static CHIP_ERROR WriteConfigValue(Key key, uint32_t val);
    static CHIP_ERROR WriteConfigValue(Key key, uint64_t val);
    static CHIP_ERROR WriteConfigValueStr(Key key, const char * str);
    static CHIP_ERROR WriteConfigValueStr(Key key, const char * str, size_t strLen);
    static CHIP_ERROR WriteConfigValueBin(Key key, const uint8_t * data, size_t dataLen);
    static CHIP_ERROR WriteConfigValueCounter(uint8_t counterIdx, uint32_t val);
    static CHIP_ERROR ClearConfigValue(Key key);
    static bool ConfigValueExists(Key key);
    static CHIP_ERROR FactoryResetConfig(void);

    static void RunConfigUnitTest(void);

private:
    static CHIP_ERROR MapNvdmStatus(nvdm_status_t nvdm_status);
    static void OnExit(void);
};

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
