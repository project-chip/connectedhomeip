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
 *          Utilities for interacting with multiple file partitions and maps
 *          key-value config calls to the correct partition.
 */

#include <platform/internal/CHIPDeviceLayerInternal.h>
#include <platform/internal/testing/ConfigUnitTest.h>

#include <core/CHIPEncoding.h>
#include <platform/cc13x2_26x2/CC13X2_26X2Config.h>
#include <support/CodeUtils.h>

#include <ti/common/nv/nvintf.h>
#include <ti/common/nv/nvocmp.h>

namespace chip {
namespace DeviceLayer {
namespace Internal {

// *** CAUTION ***: Changing the names or namespaces of these values will *break* existing devices.

// Keys stored in the Chip-factory namespace
const CC13X2_26X2Config::Key CC13X2_26X2Config::kConfigKey_SerialNum           = { { kCC13X2_26X2ChipConfig_Sysid, 0x0001 } };
const CC13X2_26X2Config::Key CC13X2_26X2Config::kConfigKey_MfrDeviceId         = { { kCC13X2_26X2ChipConfig_Sysid, 0x0002 } };
const CC13X2_26X2Config::Key CC13X2_26X2Config::kConfigKey_MfrDeviceCert       = { { kCC13X2_26X2ChipConfig_Sysid, 0x0003 } };
const CC13X2_26X2Config::Key CC13X2_26X2Config::kConfigKey_MfrDeviceICACerts   = { { kCC13X2_26X2ChipConfig_Sysid, 0x0004 } };
const CC13X2_26X2Config::Key CC13X2_26X2Config::kConfigKey_MfrDevicePrivateKey = { { kCC13X2_26X2ChipConfig_Sysid, 0x0005 } };
const CC13X2_26X2Config::Key CC13X2_26X2Config::kConfigKey_ProductRevision     = { { kCC13X2_26X2ChipConfig_Sysid, 0x0006 } };
const CC13X2_26X2Config::Key CC13X2_26X2Config::kConfigKey_ManufacturingDate   = { { kCC13X2_26X2ChipConfig_Sysid, 0x0007 } };
const CC13X2_26X2Config::Key CC13X2_26X2Config::kConfigKey_SetupPinCode        = { { kCC13X2_26X2ChipConfig_Sysid, 0x0008 } };
const CC13X2_26X2Config::Key CC13X2_26X2Config::kConfigKey_SetupDiscriminator  = { { kCC13X2_26X2ChipConfig_Sysid, 0x0009 } };

// Keys stored in the Chip-config namespace
const CC13X2_26X2Config::Key CC13X2_26X2Config::kConfigKey_FabricId              = { { kCC13X2_26X2ChipFactory_Sysid, 0x0011 } };
const CC13X2_26X2Config::Key CC13X2_26X2Config::kConfigKey_ServiceConfig         = { { kCC13X2_26X2ChipFactory_Sysid, 0x0012 } };
const CC13X2_26X2Config::Key CC13X2_26X2Config::kConfigKey_PairedAccountId       = { { kCC13X2_26X2ChipFactory_Sysid, 0x0013 } };
const CC13X2_26X2Config::Key CC13X2_26X2Config::kConfigKey_ServiceId             = { { kCC13X2_26X2ChipFactory_Sysid, 0x0014 } };
const CC13X2_26X2Config::Key CC13X2_26X2Config::kConfigKey_FabricSecret          = { { kCC13X2_26X2ChipFactory_Sysid, 0x0015 } };
const CC13X2_26X2Config::Key CC13X2_26X2Config::kConfigKey_GroupKeyIndex         = { { kCC13X2_26X2ChipFactory_Sysid, 0x0016 } };
const CC13X2_26X2Config::Key CC13X2_26X2Config::kConfigKey_LastUsedEpochKeyId    = { { kCC13X2_26X2ChipFactory_Sysid, 0x0017 } };
const CC13X2_26X2Config::Key CC13X2_26X2Config::kConfigKey_FailSafeArmed         = { { kCC13X2_26X2ChipFactory_Sysid, 0x0018 } };
const CC13X2_26X2Config::Key CC13X2_26X2Config::kConfigKey_WiFiStationSecType    = { { kCC13X2_26X2ChipFactory_Sysid, 0x0019 } };
const CC13X2_26X2Config::Key CC13X2_26X2Config::kConfigKey_OperationalDeviceId   = { { kCC13X2_26X2ChipFactory_Sysid, 0x001a } };
const CC13X2_26X2Config::Key CC13X2_26X2Config::kConfigKey_OperationalDeviceCert = { { kCC13X2_26X2ChipFactory_Sysid, 0x001b } };
const CC13X2_26X2Config::Key CC13X2_26X2Config::kConfigKey_OperationalDeviceICACerts   = { { kCC13X2_26X2ChipFactory_Sysid,
                                                                                           0x001c } };
const CC13X2_26X2Config::Key CC13X2_26X2Config::kConfigKey_OperationalDevicePrivateKey = { { kCC13X2_26X2ChipFactory_Sysid,
                                                                                             0x001d } };

/* Static local variables */
static NVINTF_nvFuncts_t sNvoctpFps = { 0 };

CHIP_ERROR CC13X2_26X2Config::Init()
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    /* Load NVOCMP function pointers, extended API */
    NVOCMP_loadApiPtrsExt(&sNvoctpFps);

    /* Initialize NVOCMP */
    sNvoctpFps.initNV(NULL);

    return err;
}

CHIP_ERROR CC13X2_26X2Config::ReadConfigValue(Key key, bool & val)
{
    CHIP_ERROR ret;
    size_t ignore;
    uint8_t localVal;

    ret = ReadConfigValueBin(key, &localVal, sizeof(localVal), ignore);

    // reference CC13X2_26X2Config::WriteConfigValue(Key key, bool val) for storage of boolean values
    val = (localVal != 0);

    return ret;
}

CHIP_ERROR CC13X2_26X2Config::ReadConfigValue(Key key, uint32_t & val)
{
    size_t ignore;

    return ReadConfigValueBin(key, (uint8_t *) &val, sizeof(val), ignore);
}

CHIP_ERROR CC13X2_26X2Config::ReadConfigValue(Key key, uint64_t & val)
{
    size_t ignore;

    return ReadConfigValueBin(key, (uint8_t *) &val, sizeof(val), ignore);
}

CHIP_ERROR CC13X2_26X2Config::ReadConfigValueStr(Key key, char * buf, size_t bufSize, size_t & outLen)
{
    return ReadConfigValueBin(key, (uint8_t *) buf, bufSize, outLen);
}

CHIP_ERROR CC13X2_26X2Config::ReadConfigValueBin(Key key, uint8_t * buf, size_t bufSize, size_t & outLen)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    size_t len;

    len = sNvoctpFps.getItemLen(key.nvID);
    VerifyOrExit(len > 0, err = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND); // key not found
    VerifyOrExit(len <= bufSize, err = CHIP_ERROR_BUFFER_TOO_SMALL);

    VerifyOrExit(sNvoctpFps.readItem(key.nvID, 0, (uint16_t) len, buf) == NVINTF_SUCCESS,
                 err = CHIP_ERROR_PERSISTED_STORAGE_FAILED);

    if (outLen)
    {
        outLen = len;
    }

exit:
    return err;
}

CHIP_ERROR CC13X2_26X2Config::WriteConfigValue(Key key, bool val)
{
    uint8_t localVal = val ? 1 : 0;
    return WriteConfigValueBin(key, (const uint8_t *) &localVal, sizeof(localVal));
}

CHIP_ERROR CC13X2_26X2Config::WriteConfigValue(Key key, uint32_t val)
{
    return WriteConfigValueBin(key, (const uint8_t *) &val, sizeof(val));
}

CHIP_ERROR CC13X2_26X2Config::WriteConfigValue(Key key, uint64_t val)
{
    return WriteConfigValueBin(key, (const uint8_t *) &val, sizeof(val));
}

CHIP_ERROR CC13X2_26X2Config::WriteConfigValueStr(Key key, const char * str)
{
    size_t strLen = strlen(str);
    return WriteConfigValueBin(key, (const uint8_t *) str, strLen);
}
CHIP_ERROR CC13X2_26X2Config::WriteConfigValueStr(Key key, const char * str, size_t strLen)
{
    return WriteConfigValueBin(key, (const uint8_t *) str, strLen);
}

CHIP_ERROR CC13X2_26X2Config::WriteConfigValueBin(Key key, const uint8_t * data, size_t dataLen)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    VerifyOrExit(sNvoctpFps.writeItem(key.nvID, (uint16_t) dataLen, (void *) data) == NVINTF_SUCCESS,
                 err = CHIP_ERROR_PERSISTED_STORAGE_FAILED);
exit:
    return err;
}

CHIP_ERROR CC13X2_26X2Config::ClearConfigValue(Key key)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    VerifyOrExit(sNvoctpFps.deleteItem(key.nvID) == NVINTF_SUCCESS, err = CHIP_ERROR_PERSISTED_STORAGE_FAILED);
exit:
    return err;
}

bool CC13X2_26X2Config::ConfigValueExists(Key key)
{
    /* 0 is an invalid length for an item, getting a length of 0 means there is no item */
    return (0 != sNvoctpFps.getItemLen(key.nvID));
}

CHIP_ERROR CC13X2_26X2Config::FactoryResetConfig(void)
{
    CHIP_ERROR err           = CHIP_NO_ERROR;
    NVINTF_nvProxy_t nvProxy = { 0 };
    uint8_t status           = NVINTF_SUCCESS;

    intptr_t key = sNvoctpFps.lockNV();

    /* Setup doNext call */
    nvProxy.sysid = kCC13X2_26X2ChipConfig_Sysid;
    nvProxy.flag  = NVINTF_DOSTART | NVINTF_DOSYSID | NVINTF_DODELETE;

    /* Lock and wipe all items with sysid TIOP */
    do
    {
        status = sNvoctpFps.doNext(&nvProxy);
    } while (NVINTF_SUCCESS == status);
    /* check we ran out of elements */
    VerifyOrExit(status != NVINTF_NOTFOUND, err = CHIP_ERROR_PERSISTED_STORAGE_FAILED);

    /* Setup doNext call. Sysid is the same here, but it doesn't necessarily have to be. Matching POSIX impl */
    nvProxy.sysid = kCC13X2_26X2ChipFactory_Sysid;
    nvProxy.flag  = NVINTF_DOSTART | NVINTF_DOSYSID | NVINTF_DODELETE;

    /* Lock and wipe all items with sysid TIOP */
    do
    {
        status = sNvoctpFps.doNext(&nvProxy);
    } while (NVINTF_SUCCESS == status);
    /* check we ran out of elements */
    VerifyOrExit(status != NVINTF_NOTFOUND, err = CHIP_ERROR_PERSISTED_STORAGE_FAILED);

    /* Setup doNext call. Sysid is the same here, but it doesn't necessarily have to be. Matching POSIX impl */
    nvProxy.sysid = kCC13X2_26X2ChipCounters_Sysid;
    nvProxy.flag  = NVINTF_DOSTART | NVINTF_DOSYSID | NVINTF_DODELETE;

    /* Lock and wipe all items with sysid TIOP */
    do
    {
        status = sNvoctpFps.doNext(&nvProxy);
    } while (NVINTF_SUCCESS == status);
    /* check we ran out of elements */
    VerifyOrExit(status != NVINTF_NOTFOUND, err = CHIP_ERROR_PERSISTED_STORAGE_FAILED);

exit:
    sNvoctpFps.unlockNV(key);

    if (err == CHIP_NO_ERROR)
    {
        /* force compaction */
        sNvoctpFps.compactNV(0);
    }
    return err;
}

void CC13X2_26X2Config::RunConfigUnitTest()
{
    // Run common unit test.
    ::chip::DeviceLayer::Internal::RunConfigUnitTest<CC13X2_26X2Config>();
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
