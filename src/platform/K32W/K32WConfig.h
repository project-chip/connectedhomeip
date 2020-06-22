/*
 *
 *    Copyright (c) 2020 Google LLC.
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
 *          Utilities for accessing persisted device configuration on
 *          platforms based on the NXP K32W SDK.
 */

#ifndef K32W_CONFIG_H
#define K32W_CONFIG_H

#include <functional>

#include <Weave/DeviceLayer/internal/WeaveDeviceLayerInternal.h>

#include "PDM.h"
#include "MemManager.h"

namespace nl {
namespace Weave {
namespace DeviceLayer {
namespace Internal {

constexpr inline uint16_t K32WConfigKey(uint8_t weaveId, uint8_t pdmId)
{
    return static_cast<uint16_t>(weaveId) << 8 | pdmId;
}


/**
 *
 * This implementation uses the NXP Persistent Data Manager (PDM) library as the
 * underlying storage layer.
 *
 * NOTE: This class is designed to be mixed-in to the concrete subclass of the
 * GenericConfigurationManagerImpl<> template.  When used this way, the class
 * naturally provides implementations for the delegated members referenced by
 * the template class (e.g. the ReadConfigValue() method).
 */
class K32WConfig
{
public:

    // PDM ids used by the OpenWeave Device Layer
    static constexpr uint8_t kPDMId_WeaveFactory              = 0x01; /**< PDM id for settings containing persistent config values set at manufacturing time.
                                                                           *   Retained during factory reset. */
    static constexpr uint8_t kPDMId_WeaveConfig               = 0x02; /**< PDM id for settings containing dynamic config values set at runtime.
                                                                           *   Cleared during factory reset. */
    static constexpr uint8_t kPDMId_WeaveCounter              = 0x03; /**< PDM id for settings containing dynamic counter values set at runtime.
                                                                           *   Retained during factory reset. */

    using Key = uint32_t;

    // Key definitions for well-known configuration values.
    // Factory config keys
    static constexpr Key kConfigKey_SerialNum                   = K32WConfigKey(kPDMId_WeaveFactory, 0x00);
    static constexpr Key kConfigKey_MfrDeviceId                 = K32WConfigKey(kPDMId_WeaveFactory, 0x01);
    static constexpr Key kConfigKey_MfrDeviceCert               = K32WConfigKey(kPDMId_WeaveFactory, 0x02);
    static constexpr Key kConfigKey_MfrDevicePrivateKey         = K32WConfigKey(kPDMId_WeaveFactory, 0x03);
    static constexpr Key kConfigKey_ManufacturingDate           = K32WConfigKey(kPDMId_WeaveFactory, 0x04);
    static constexpr Key kConfigKey_PairingCode                 = K32WConfigKey(kPDMId_WeaveFactory, 0x05);
    static constexpr Key kConfigKey_MfrDeviceICACerts           = K32WConfigKey(kPDMId_WeaveFactory, 0x06);
    static constexpr Key kConfigKey_ProductRevision             = K32WConfigKey(kPDMId_WeaveFactory, 0x07);
	// Weave Config Keys
    static constexpr Key kConfigKey_FabricId                    = K32WConfigKey(kPDMId_WeaveConfig,  0x00);
    static constexpr Key kConfigKey_ServiceConfig               = K32WConfigKey(kPDMId_WeaveConfig,  0x01);
    static constexpr Key kConfigKey_PairedAccountId             = K32WConfigKey(kPDMId_WeaveConfig,  0x02);
    static constexpr Key kConfigKey_ServiceId                   = K32WConfigKey(kPDMId_WeaveConfig,  0x03);
    static constexpr Key kConfigKey_FabricSecret                = K32WConfigKey(kPDMId_WeaveConfig,  0x04);
    static constexpr Key kConfigKey_LastUsedEpochKeyId          = K32WConfigKey(kPDMId_WeaveConfig,  0x05);
    static constexpr Key kConfigKey_FailSafeArmed               = K32WConfigKey(kPDMId_WeaveConfig,  0x06);

    static constexpr Key kConfigKey_OperationalDeviceId         = K32WConfigKey(kPDMId_WeaveConfig,  0x07);
    static constexpr Key kConfigKey_OperationalDeviceCert       = K32WConfigKey(kPDMId_WeaveConfig,  0x08);
    static constexpr Key kConfigKey_OperationalDeviceICACerts   = K32WConfigKey(kPDMId_WeaveConfig,  0x09);
    static constexpr Key kConfigKey_OperationalDevicePrivateKey = K32WConfigKey(kPDMId_WeaveConfig,  0x0A);

    static constexpr Key kConfigKey_GroupKey                    = K32WConfigKey(kPDMId_WeaveConfig,  0x0B);
    static constexpr Key kConfigKey_GroupKey0 = K32WConfigKey(kPDMId_WeaveConfig, 0x0C);
    static constexpr Key kConfigKey_GroupKey1 = K32WConfigKey(kPDMId_WeaveConfig, 0x0D);
    static constexpr Key kConfigKey_GroupKey2 = K32WConfigKey(kPDMId_WeaveConfig, 0x0E);
    static constexpr Key kConfigKey_GroupKey3 = K32WConfigKey(kPDMId_WeaveConfig, 0x0F);
    static constexpr Key kConfigKey_GroupKey4 = K32WConfigKey(kPDMId_WeaveConfig, 0x10);
    static constexpr Key kConfigKey_GroupKey5 = K32WConfigKey(kPDMId_WeaveConfig, 0x11);
    static constexpr Key kConfigKey_GroupKey6 = K32WConfigKey(kPDMId_WeaveConfig, 0x12);
    static constexpr Key kConfigKey_GroupKey7 = K32WConfigKey(kPDMId_WeaveConfig, 0x13);
    static constexpr Key kConfigKey_GroupKey8 = K32WConfigKey(kPDMId_WeaveConfig, 0x14);
    static constexpr Key kConfigKey_GroupKey9 = K32WConfigKey(kPDMId_WeaveConfig, 0x15);
    static constexpr Key kConfigKey_GroupKey10 = K32WConfigKey(kPDMId_WeaveConfig, 0x16);
    static constexpr Key kConfigKey_GroupKey11 = K32WConfigKey(kPDMId_WeaveConfig, 0x17);
    static constexpr Key kConfigKey_GroupKey12 = K32WConfigKey(kPDMId_WeaveConfig, 0x18);
    static constexpr Key kConfigKey_GroupKey13 = K32WConfigKey(kPDMId_WeaveConfig, 0x19);
    static constexpr Key kConfigKey_GroupKey14 = K32WConfigKey(kPDMId_WeaveConfig, 0x1A);
    static constexpr Key kConfigKey_GroupKey15 = K32WConfigKey(kPDMId_WeaveConfig, 0x1B);

    static constexpr Key kConfigKey_GroupKeyBase = kConfigKey_GroupKey0;
    static constexpr Key kConfigKey_GroupKeyMax = K32WConfigKey(kPDMId_WeaveConfig, 0x1B);; // Allows 16 Group Keys to be created.

    // Set key id limits for each group.
    static constexpr Key kMinConfigKey_WeaveFactory = K32WConfigKey(kPDMId_WeaveFactory, 0x00);
    static constexpr Key kMaxConfigKey_WeaveFactory = K32WConfigKey(kPDMId_WeaveFactory, 0x07);
    static constexpr Key kMinConfigKey_WeaveConfig  = K32WConfigKey(kPDMId_WeaveConfig, 0x00);
    static constexpr Key kMaxConfigKey_WeaveConfig  = K32WConfigKey(kPDMId_WeaveConfig, 0x1A);
    static constexpr Key kMinConfigKey_WeaveCounter = K32WConfigKey(kPDMId_WeaveCounter, 0x00);
    static constexpr Key kMaxConfigKey_WeaveCounter = K32WConfigKey(kPDMId_WeaveCounter, 0x1F); // Allows 32 Counters to be created.

    static WEAVE_ERROR Init(void);

    // Configuration methods used by the GenericConfigurationManagerImpl<> template.
    static WEAVE_ERROR ReadConfigValue(Key key, bool & val);
    static WEAVE_ERROR ReadConfigValue(Key key, uint32_t & val);
    static WEAVE_ERROR ReadConfigValue(Key key, uint64_t & val);
    static WEAVE_ERROR ReadConfigValueStr(Key key, char * buf, size_t bufSize, size_t & outLen);
    static WEAVE_ERROR ReadConfigValueBin(Key key, uint8_t * buf, size_t bufSize, size_t & outLen);
    static WEAVE_ERROR ReadConfigValueCounter(uint8_t counterIdx, uint32_t &val);
    static WEAVE_ERROR WriteConfigValue(Key key, bool val);
    static WEAVE_ERROR WriteConfigValue(Key key, uint32_t val);
    static WEAVE_ERROR WriteConfigValue(Key key, uint64_t val);
    static WEAVE_ERROR WriteConfigValueStr(Key key, const char * str);
    static WEAVE_ERROR WriteConfigValueStr(Key key, const char * str, size_t strLen);
    static WEAVE_ERROR WriteConfigValueBin(Key key, const uint8_t * data, size_t dataLen);
    static WEAVE_ERROR WriteConfigValueCounter(uint8_t counterIdx, uint32_t val);
    static WEAVE_ERROR ClearConfigValue(Key key);
    static bool        ConfigValueExists(Key key);
    static WEAVE_ERROR FactoryResetConfig(void);
	static bool        ValidConfigKey(Key key);

    static void RunConfigUnitTest(void);

protected:
    using ForEachRecordFunct = std::function<WEAVE_ERROR(const Key &PDMIdKey, const size_t &length)>;
    static WEAVE_ERROR ForEachRecord(Key firstKey, Key lastKey, bool addNewRecord, ForEachRecordFunct funct);
    static constexpr uint8_t GetPDMId(uint32_t key);
    static constexpr uint8_t GetRecordKey(uint32_t key);

private:
    static WEAVE_ERROR MapPdmStatus(PDM_teStatus pdmStatus);
    static WEAVE_ERROR MapPdmInitStatus(int pdmStatus);
};

/**
 * Extract a PDM id from a Key value.
 */
inline constexpr uint8_t K32WConfig::GetPDMId(Key key)
{
    return static_cast<uint8_t>(key >> 8);
}

/**
 * Extract an NVM record key from a Key value.
 */
inline constexpr uint8_t K32WConfig::GetRecordKey(Key key)
{
    return static_cast<uint8_t>(key);
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace Weave
} // namespace nl

#endif // K32W_CONFIG_H
