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

/* itemID and subID are limited to 10 bits, even though their types are uint16_t */

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

/* Internal for the KVS interface. */
const CC13X2_26X2Config::Key CC13X2_26X2Config::kConfigKey_KVS_key   = { { .systemID = kCC13X2_26X2ChipFactory_Sysid,
                                                                         .itemID   = 0x0021 } };
const CC13X2_26X2Config::Key CC13X2_26X2Config::kConfigKey_KVS_value = { { .systemID = kCC13X2_26X2ChipFactory_Sysid,
                                                                           .itemID   = 0x0020 } };

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

/* Iterate through the key range to find a key that matches. */
static uint8_t FindKVSSubID(const char * key, uint16_t & subID)
{
    char key_scratch[32]; // 32 characters seems large enough for a key
    NVINTF_nvProxy_t nvProxy = { 0 };
    uint8_t status           = NVINTF_SUCCESS;

    nvProxy.sysid  = CC13X2_26X2Config::kConfigKey_KVS_key.nvID.systemID;
    nvProxy.itemid = CC13X2_26X2Config::kConfigKey_KVS_key.nvID.itemID;
    nvProxy.buffer = key_scratch;
    nvProxy.len    = sizeof(key_scratch);
    nvProxy.flag   = NVINTF_DOSTART | NVINTF_DOITMID | NVINTF_DOREAD;

    intptr_t lock_key = sNvoctpFps.lockNV();
    do
    {
        memset(key_scratch, 0, sizeof(key_scratch));
        status = sNvoctpFps.doNext(&nvProxy);
        if (NVINTF_SUCCESS != status)
        {
            break;
        }
        if (0 == strcmp(key, (char *) nvProxy.buffer))
        {
            subID = nvProxy.subid;
            break;
        }
    } while (NVINTF_SUCCESS == status);

    sNvoctpFps.unlockNV(lock_key);
    return status;
}

CHIP_ERROR CC13X2_26X2Config::ReadKVS(const char * key, void * value, size_t value_size, size_t * read_bytes_size,
                                      size_t offset_bytes)
{
    CHIP_ERROR err           = CHIP_NO_ERROR;
    NVINTF_itemID_t val_item = CC13X2_26X2Config::kConfigKey_KVS_value.nvID;
    uint16_t subID;
    size_t len;

    VerifyOrExit(FindKVSSubID(key, subID) == NVINTF_SUCCESS, err = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND);

    val_item.subID = subID;

    len = sNvoctpFps.getItemLen(val_item);
    VerifyOrExit(len > 0, err = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND); // key not found

    VerifyOrExit(sNvoctpFps.readItem(val_item, (uint16_t) offset_bytes, (uint16_t) value_size, value) == NVINTF_SUCCESS,
                 err = CHIP_ERROR_PERSISTED_STORAGE_FAILED);

    if (read_bytes_size)
    {
        if (len - offset_bytes > value_size)
        {
            *read_bytes_size = value_size;
        }
        else
        {
            *read_bytes_size = len - offset_bytes;
        }
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

CHIP_ERROR CC13X2_26X2Config::WriteKVS(const char * key, const void * value, size_t value_size)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    uint16_t subID;

    if (FindKVSSubID(key, subID) == NVINTF_SUCCESS)
    {
        NVINTF_itemID_t val_item = CC13X2_26X2Config::kConfigKey_KVS_value.nvID;
        // key already exists, update value
        val_item.subID = subID;
        VerifyOrExit(sNvoctpFps.updateItem(val_item, (uint16_t) value_size, (void *) value) == NVINTF_SUCCESS,
                     err = CHIP_ERROR_PERSISTED_STORAGE_FAILED);
    }
    else
    {
        // key does not exist, likely case
        intptr_t lock_key = sNvoctpFps.lockNV();

        NVINTF_itemID_t key_item = CC13X2_26X2Config::kConfigKey_KVS_key.nvID;
        NVINTF_itemID_t val_item = CC13X2_26X2Config::kConfigKey_KVS_value.nvID;

        /* Iterate through the subID range to find an unused subID in the
         * keyspace.  SubID is a 10 bit value, reference
         * `<simplelink_sdk>/source/ti/common/nv/nvocmp.c:MVOCMP_MAXSUBID`.
         */
        for (uint16_t i = 0; i < 0x3FF; i++)
        {
            key_item.subID = i;
            if (sNvoctpFps.getItemLen(key_item) == 0U)
            {
                val_item.subID = i;
                break;
            }
        }
        // write they key item
        if (sNvoctpFps.writeItem(key_item, (uint16_t) strlen(key), (void *) key) == NVINTF_SUCCESS)
        {
            if (sNvoctpFps.writeItem(val_item, (uint16_t) value_size, (void *) value) != NVINTF_SUCCESS)
            {
                // try to delete the key item
                sNvoctpFps.deleteItem(key_item);
                err = CHIP_ERROR_PERSISTED_STORAGE_FAILED;
            }
        }
        else
        {
            err = CHIP_ERROR_PERSISTED_STORAGE_FAILED;
        }
        sNvoctpFps.unlockNV(lock_key);
    }
exit:
    return err;
}

CHIP_ERROR CC13X2_26X2Config::WriteConfigValueBin(Key key, const uint8_t * data, size_t dataLen)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    VerifyOrExit(sNvoctpFps.writeItem(key.nvID, (uint16_t) dataLen, (void *) data) == NVINTF_SUCCESS,
                 err = CHIP_ERROR_PERSISTED_STORAGE_FAILED);
exit:
    return err;
}

CHIP_ERROR CC13X2_26X2Config::ClearKVS(const char * key)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    uint16_t subID;
    NVINTF_itemID_t key_item = CC13X2_26X2Config::kConfigKey_KVS_key.nvID;
    NVINTF_itemID_t val_item = CC13X2_26X2Config::kConfigKey_KVS_value.nvID;

    if (FindKVSSubID(key, subID) == NVINTF_SUCCESS)
    {
        key_item.subID = subID;
        val_item.subID = subID;
        // delete the value item
        if (sNvoctpFps.deleteItem(val_item) != NVINTF_SUCCESS)
        {
            err = CHIP_ERROR_PERSISTED_STORAGE_FAILED;
        }
        // delete the key item
        if (sNvoctpFps.deleteItem(key_item) != NVINTF_SUCCESS)
        {
            err = CHIP_ERROR_PERSISTED_STORAGE_FAILED;
        }
    }

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
