/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2020 Google LLC.
 *    Copyright (c) 2018 Nest Labs, Inc.
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
/* this file behaves like a config.h, comes first */
#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <platform/nxp/k32w/k32w0/K32W0Config.h>

#include <lib/core/CHIPEncoding.h>
#include <platform/internal/testing/ConfigUnitTest.h>

#include "FreeRTOS.h"

namespace chip {
namespace DeviceLayer {
namespace Internal {

RamStorage K32WConfig::sFactoryStorage{ kNvmId_Factory };
RamStorage K32WConfig::sConfigStorage{ kNvmId_Config };
RamStorage K32WConfig::sCounterStorage{ kNvmId_Counter };

const K32WConfig::Key K32WConfig::kConfigKey_SerialNum{ &K32WConfig::sFactoryStorage, kKeyId_Factory, 0x00 };
const K32WConfig::Key K32WConfig::kConfigKey_MfrDeviceId{ &K32WConfig::sFactoryStorage, kKeyId_Factory, 0x01 };
const K32WConfig::Key K32WConfig::kConfigKey_MfrDeviceCert{ &K32WConfig::sFactoryStorage, kKeyId_Factory, 0x02 };
const K32WConfig::Key K32WConfig::kConfigKey_MfrDevicePrivateKey{ &K32WConfig::sFactoryStorage, kKeyId_Factory, 0x03 };
const K32WConfig::Key K32WConfig::kConfigKey_ManufacturingDate{ &K32WConfig::sFactoryStorage, kKeyId_Factory, 0x04 };
const K32WConfig::Key K32WConfig::kConfigKey_SetupPinCode{ &K32WConfig::sFactoryStorage, kKeyId_Factory, 0x05 };
const K32WConfig::Key K32WConfig::kConfigKey_MfrDeviceICACerts{ &K32WConfig::sFactoryStorage, kKeyId_Factory, 0x06 };
const K32WConfig::Key K32WConfig::kConfigKey_HardwareVersion{ &K32WConfig::sFactoryStorage, kKeyId_Factory, 0x07 };
const K32WConfig::Key K32WConfig::kConfigKey_SetupDiscriminator{ &K32WConfig::sFactoryStorage, kKeyId_Factory, 0x08 };
const K32WConfig::Key K32WConfig::kConfigKey_Spake2pIterationCount{ &K32WConfig::sFactoryStorage, kKeyId_Factory, 0x09 };
const K32WConfig::Key K32WConfig::kConfigKey_Spake2pSalt{ &K32WConfig::sFactoryStorage, kKeyId_Factory, 0x0A };
const K32WConfig::Key K32WConfig::kConfigKey_Spake2pVerifier{ &K32WConfig::sFactoryStorage, kKeyId_Factory, 0x0B };

const K32WConfig::Key K32WConfig::kConfigKey_ServiceConfig{ &K32WConfig::sConfigStorage, kKeyId_Config, 0x01 };
const K32WConfig::Key K32WConfig::kConfigKey_PairedAccountId{ &K32WConfig::sConfigStorage, kKeyId_Config, 0x02 };
const K32WConfig::Key K32WConfig::kConfigKey_ServiceId{ &K32WConfig::sConfigStorage, kKeyId_Config, 0x03 };
const K32WConfig::Key K32WConfig::kConfigKey_LastUsedEpochKeyId{ &K32WConfig::sConfigStorage, kKeyId_Config, 0x05 };
const K32WConfig::Key K32WConfig::kConfigKey_FailSafeArmed{ &K32WConfig::sConfigStorage, kKeyId_Config, 0x06 };
const K32WConfig::Key K32WConfig::kConfigKey_RegulatoryLocation{ &K32WConfig::sConfigStorage, kKeyId_Config, 0x07 };
const K32WConfig::Key K32WConfig::kConfigKey_CountryCode{ &K32WConfig::sConfigStorage, kKeyId_Config, 0x08 };
const K32WConfig::Key K32WConfig::kConfigKey_UniqueId{ &K32WConfig::sConfigStorage, kKeyId_Config, 0x0A };
const K32WConfig::Key K32WConfig::kConfigKey_SoftwareVersion{ &K32WConfig::sConfigStorage, kKeyId_Config, 0x0B };
const K32WConfig::Key K32WConfig::kConfigKey_SoftwareUpdateCompleted{ &K32WConfig::sConfigStorage, kKeyId_Config, 0x0C };

const K32WConfig::Key K32WConfig::kCounterKey_RebootCount{ &K32WConfig::sCounterStorage, kKeyId_Counter, 0x00 };
const K32WConfig::Key K32WConfig::kCounterKey_UpTime{ &K32WConfig::sCounterStorage, kKeyId_Counter, 0x01 };
const K32WConfig::Key K32WConfig::kCounterKey_TotalOperationalHours{ &K32WConfig::sCounterStorage, kKeyId_Counter, 0x02 };
const K32WConfig::Key K32WConfig::kCounterKey_BootReason{ &K32WConfig::sCounterStorage, kKeyId_Counter, 0x03 };

CHIP_ERROR K32WConfig::Init()
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    err = sFactoryStorage.Init(RamStorage::kRamBufferInitialSize);
    SuccessOrExit(err);
    err = sConfigStorage.Init(RamStorage::kRamBufferInitialSize);
    SuccessOrExit(err);
    err = sCounterStorage.Init(RamStorage::kRamBufferInitialSize);
    SuccessOrExit(err);

exit:
    if (err != CHIP_NO_ERROR)
    {
        sFactoryStorage.FreeBuffer();
        sConfigStorage.FreeBuffer();
        sCounterStorage.FreeBuffer();
    }
    return err;
}

CHIP_ERROR K32WConfig::ReadConfigValueStr(Key key, char * buf, size_t bufSize, size_t & outLen)
{
    CHIP_ERROR err;
    uint16_t sizeToRead = bufSize;

    err = key.Read((uint8_t *) buf, sizeToRead);
    SuccessOrExit(err);

    outLen = sizeToRead;
exit:
    return err;
}

CHIP_ERROR K32WConfig::ReadConfigValueBin(Key key, uint8_t * buf, size_t bufSize, size_t & outLen)
{
    return ReadConfigValueStr(key, (char *) buf, bufSize, outLen);
}

CHIP_ERROR K32WConfig::ReadConfigValueCounter(uint8_t counterIdx, uint32_t & val)
{
    Key key{ &sCounterStorage, kKeyId_Counter, counterIdx };
    return ReadConfigValue(key, val);
}

CHIP_ERROR K32WConfig::WriteConfigValueStr(Key key, const char * str)
{
    return WriteConfigValueStr(key, str, (str != NULL) ? strlen(str) : 0);
}

CHIP_ERROR K32WConfig::WriteConfigValueStr(Key key, const char * str, size_t strLen)
{
    return key.Write((uint8_t *) str, strLen);
}

CHIP_ERROR K32WConfig::WriteConfigValueBin(Key key, const uint8_t * data, size_t dataLen)
{
    return WriteConfigValueStr(key, (char *) data, dataLen);
}

CHIP_ERROR K32WConfig::WriteConfigValueCounter(uint8_t counterIdx, uint32_t val)
{
    Key key{ &sCounterStorage, kKeyId_Counter, counterIdx };
    return WriteConfigValue(key, val);
}

CHIP_ERROR K32WConfig::ClearConfigValue(Key key)
{
    return key.Delete();
}

bool K32WConfig::ConfigValueExists(Key key)
{
    CHIP_ERROR err;
    uint16_t sizeToRead;
    err = key.Read(NULL, sizeToRead);

    return (err == CHIP_NO_ERROR && sizeToRead != 0);
}

CHIP_ERROR K32WConfig::FactoryResetConfig(void)
{
    sConfigStorage.OnFactoryReset();
    sCounterStorage.OnFactoryReset();

    return CHIP_NO_ERROR;
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
