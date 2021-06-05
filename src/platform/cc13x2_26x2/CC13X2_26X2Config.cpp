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
const CC13X2_26X2Config::Key CC13X2_26X2Config::kConfigKey_SerialNum           = { { .systemID = kCC13X2_26X2ChipConfig_Sysid,
                                                                           .itemID   = 0x0001 } };
const CC13X2_26X2Config::Key CC13X2_26X2Config::kConfigKey_MfrDeviceId         = { { .systemID = kCC13X2_26X2ChipConfig_Sysid,
                                                                             .itemID   = 0x0002 } };
const CC13X2_26X2Config::Key CC13X2_26X2Config::kConfigKey_MfrDeviceCert       = { { .systemID = kCC13X2_26X2ChipConfig_Sysid,
                                                                               .itemID   = 0x0003 } };
const CC13X2_26X2Config::Key CC13X2_26X2Config::kConfigKey_MfrDeviceICACerts   = { { .systemID = kCC13X2_26X2ChipConfig_Sysid,
                                                                                   .itemID   = 0x0004 } };
const CC13X2_26X2Config::Key CC13X2_26X2Config::kConfigKey_MfrDevicePrivateKey = { { .systemID = kCC13X2_26X2ChipConfig_Sysid,
                                                                                     .itemID   = 0x0005 } };
const CC13X2_26X2Config::Key CC13X2_26X2Config::kConfigKey_ProductRevision     = { { .systemID = kCC13X2_26X2ChipConfig_Sysid,
                                                                                 .itemID   = 0x0006 } };
const CC13X2_26X2Config::Key CC13X2_26X2Config::kConfigKey_ManufacturingDate   = { { .systemID = kCC13X2_26X2ChipConfig_Sysid,
                                                                                   .itemID   = 0x0007 } };
const CC13X2_26X2Config::Key CC13X2_26X2Config::kConfigKey_SetupPinCode        = { { .systemID = kCC13X2_26X2ChipConfig_Sysid,
                                                                              .itemID   = 0x0008 } };
const CC13X2_26X2Config::Key CC13X2_26X2Config::kConfigKey_SetupDiscriminator  = { { .systemID = kCC13X2_26X2ChipConfig_Sysid,
                                                                                    .itemID   = 0x0009 } };

// Keys stored in the Chip-config namespace
const CC13X2_26X2Config::Key CC13X2_26X2Config::kConfigKey_FabricId              = { { .systemID = kCC13X2_26X2ChipFactory_Sysid,
                                                                          .itemID   = 0x0011 } };
const CC13X2_26X2Config::Key CC13X2_26X2Config::kConfigKey_ServiceConfig         = { { .systemID = kCC13X2_26X2ChipFactory_Sysid,
                                                                               .itemID   = 0x0012 } };
const CC13X2_26X2Config::Key CC13X2_26X2Config::kConfigKey_PairedAccountId       = { { .systemID = kCC13X2_26X2ChipFactory_Sysid,
                                                                                 .itemID   = 0x0013 } };
const CC13X2_26X2Config::Key CC13X2_26X2Config::kConfigKey_ServiceId             = { { .systemID = kCC13X2_26X2ChipFactory_Sysid,
                                                                           .itemID   = 0x0014 } };
const CC13X2_26X2Config::Key CC13X2_26X2Config::kConfigKey_FabricSecret          = { { .systemID = kCC13X2_26X2ChipFactory_Sysid,
                                                                              .itemID   = 0x0015 } };
const CC13X2_26X2Config::Key CC13X2_26X2Config::kConfigKey_GroupKeyIndex         = { { .systemID = kCC13X2_26X2ChipFactory_Sysid,
                                                                               .itemID   = 0x0016 } };
const CC13X2_26X2Config::Key CC13X2_26X2Config::kConfigKey_LastUsedEpochKeyId    = { { .systemID = kCC13X2_26X2ChipFactory_Sysid,
                                                                                    .itemID   = 0x0017 } };
const CC13X2_26X2Config::Key CC13X2_26X2Config::kConfigKey_FailSafeArmed         = { { .systemID = kCC13X2_26X2ChipFactory_Sysid,
                                                                               .itemID   = 0x0018 } };
const CC13X2_26X2Config::Key CC13X2_26X2Config::kConfigKey_WiFiStationSecType    = { { .systemID = kCC13X2_26X2ChipFactory_Sysid,
                                                                                    .itemID   = 0x0019 } };
const CC13X2_26X2Config::Key CC13X2_26X2Config::kConfigKey_OperationalDeviceId   = { { .systemID = kCC13X2_26X2ChipFactory_Sysid,
                                                                                     .itemID   = 0x001a } };
const CC13X2_26X2Config::Key CC13X2_26X2Config::kConfigKey_OperationalDeviceCert = { { .systemID = kCC13X2_26X2ChipFactory_Sysid,
                                                                                       .itemID   = 0x001b } };
const CC13X2_26X2Config::Key CC13X2_26X2Config::kConfigKey_OperationalDeviceICACerts = {
    { .systemID = kCC13X2_26X2ChipFactory_Sysid, .itemID = 0x001c }
};
const CC13X2_26X2Config::Key CC13X2_26X2Config::kConfigKey_OperationalDevicePrivateKey = {
    { .systemID = kCC13X2_26X2ChipFactory_Sysid, .itemID = 0x001d }
};

const CC13X2_26X2Config::Key CC13X2_26X2Config::kConfigKey_RegulatoryLocation = { { .systemID = kCC13X2_26X2ChipFactory_Sysid,
                                                                                    .itemID   = 0x001e } };
const CC13X2_26X2Config::Key CC13X2_26X2Config::kConfigKey_CountryCode        = { { .systemID = kCC13X2_26X2ChipFactory_Sysid,
                                                                             .itemID   = 0x001f } };
const CC13X2_26X2Config::Key CC13X2_26X2Config::kConfigKey_Breadcrumb         = { { .systemID = kCC13X2_26X2ChipFactory_Sysid,
                                                                            .itemID   = 0x0020 } };

/* Internal for the KVS interface */
const CC13X2_26X2Config::Key CC13X2_26X2Config::kConfigKey_KVS = { { .systemID = kCC13X2_26X2ChipFactory_Sysid,
                                                                     .itemID   = 0x0021 } };

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

static CHIP_ERROR ReadBin(CC13X2_26X2Config::Key key, void * buf, size_t bufSize, size_t * outLen, size_t offset)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    size_t len;

    len = sNvoctpFps.getItemLen(key.nvID);
    VerifyOrExit(len > 0, err = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND); // key not found
    VerifyOrExit(len - offset <= bufSize, err = CHIP_ERROR_BUFFER_TOO_SMALL);

    VerifyOrExit(sNvoctpFps.readItem(key.nvID, (uint16_t) offset, (uint16_t) len, buf) == NVINTF_SUCCESS,
                 err = CHIP_ERROR_PERSISTED_STORAGE_FAILED);

    if (outLen)
    {
        *outLen = len;
    }

exit:
    return err;
}

CHIP_ERROR CC13X2_26X2Config::ReadKVS(uint16_t key, void * value, size_t value_size, size_t * read_bytes_size, size_t offset_bytes)
{
    Key nv_key = CC13X2_26X2Config::kConfigKey_KVS;

    nv_key.nvID.subID = key;
    return ReadBin(nv_key, value, value_size, read_bytes_size, offset_bytes);
}

CHIP_ERROR CC13X2_26X2Config::ReadConfigValueBin(Key key, uint8_t * buf, size_t bufSize, size_t & outLen)
{
    return ReadBin(key, (void *) buf, bufSize, &outLen, 0);
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

CHIP_ERROR CC13X2_26X2Config::WriteKVS(uint16_t key, const void * value, size_t value_size)
{
    Key nv_key = CC13X2_26X2Config::kConfigKey_KVS;

    nv_key.nvID.subID = key;
    return WriteConfigValueBin(nv_key, (const uint8_t *) value, value_size);
}

CHIP_ERROR CC13X2_26X2Config::WriteConfigValueBin(Key key, const uint8_t * data, size_t dataLen)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    VerifyOrExit(sNvoctpFps.writeItem(key.nvID, (uint16_t) dataLen, (void *) data) == NVINTF_SUCCESS,
                 err = CHIP_ERROR_PERSISTED_STORAGE_FAILED);
exit:
    return err;
}

CHIP_ERROR CC13X2_26X2Config::ClearKVS(uint16_t key)
{
    Key nv_key = CC13X2_26X2Config::kConfigKey_KVS;

    nv_key.nvID.subID = key;
    return ClearConfigValue(nv_key);
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
