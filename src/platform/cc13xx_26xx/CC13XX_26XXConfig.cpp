/*
 *
 *    Copyright (c) 2020-2022 Project CHIP Authors
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

#include <lib/core/CHIPEncoding.h>
#include <lib/core/CHIPPersistentStorageDelegate.h>
#include <lib/support/CodeUtils.h>
#include <platform/cc13xx_26xx/CC13XX_26XXConfig.h>

#include <ti/common/nv/nvintf.h>
#include <ti/common/nv/nvocmp.h>

namespace chip {
namespace DeviceLayer {
namespace Internal {

// *** CAUTION ***: Changing the names or namespaces of these values will *break* existing devices.

/* itemID and subID are limited to 10 bits, even though their types are uint16_t */

// Keys stored in the Chip-factory namespace

const CC13XX_26XXConfig::Key CC13XX_26XXConfig::kConfigKey_SerialNum = {
    { .systemID = kCC13XX_26XXMatter_SysID, .itemID = kCC13XX_26XXMatter_ItemID_ChipFactory, .subID = 0x0001 }
};
const CC13XX_26XXConfig::Key CC13XX_26XXConfig::kConfigKey_MfrDeviceId = {
    { .systemID = kCC13XX_26XXMatter_SysID, .itemID = kCC13XX_26XXMatter_ItemID_ChipFactory, .subID = 0x0002 }
};
const CC13XX_26XXConfig::Key CC13XX_26XXConfig::kConfigKey_MfrDeviceCert = {
    { .systemID = kCC13XX_26XXMatter_SysID, .itemID = kCC13XX_26XXMatter_ItemID_ChipFactory, .subID = 0x0003 }
};
const CC13XX_26XXConfig::Key CC13XX_26XXConfig::kConfigKey_MfrDeviceICACerts = {
    { .systemID = kCC13XX_26XXMatter_SysID, .itemID = kCC13XX_26XXMatter_ItemID_ChipFactory, .subID = 0x0004 }
};
const CC13XX_26XXConfig::Key CC13XX_26XXConfig::kConfigKey_MfrDevicePrivateKey = {
    { .systemID = kCC13XX_26XXMatter_SysID, .itemID = kCC13XX_26XXMatter_ItemID_ChipFactory, .subID = 0x0005 }
};
const CC13XX_26XXConfig::Key CC13XX_26XXConfig::kConfigKey_HardwareVersion = {
    { .systemID = kCC13XX_26XXMatter_SysID, .itemID = kCC13XX_26XXMatter_ItemID_ChipFactory, .subID = 0x0006 }
};
const CC13XX_26XXConfig::Key CC13XX_26XXConfig::kConfigKey_ManufacturingDate = {
    { .systemID = kCC13XX_26XXMatter_SysID, .itemID = kCC13XX_26XXMatter_ItemID_ChipFactory, .subID = 0x0007 }
};
const CC13XX_26XXConfig::Key CC13XX_26XXConfig::kConfigKey_SetupPinCode = {
    { .systemID = kCC13XX_26XXMatter_SysID, .itemID = kCC13XX_26XXMatter_ItemID_ChipFactory, .subID = 0x0008 }
};
const CC13XX_26XXConfig::Key CC13XX_26XXConfig::kConfigKey_SetupDiscriminator = {
    { .systemID = kCC13XX_26XXMatter_SysID, .itemID = kCC13XX_26XXMatter_ItemID_ChipFactory, .subID = 0x0009 }
};
const CC13XX_26XXConfig::Key CC13XX_26XXConfig::kConfigKey_Spake2pIterationCount = {
    { .systemID = kCC13XX_26XXMatter_SysID, .itemID = kCC13XX_26XXMatter_ItemID_ChipFactory, .subID = 0x000a }
};
const CC13XX_26XXConfig::Key CC13XX_26XXConfig::kConfigKey_Spake2pSalt = {
    { .systemID = kCC13XX_26XXMatter_SysID, .itemID = kCC13XX_26XXMatter_ItemID_ChipFactory, .subID = 0x000b }
};
const CC13XX_26XXConfig::Key CC13XX_26XXConfig::kConfigKey_Spake2pVerifier = {
    { .systemID = kCC13XX_26XXMatter_SysID, .itemID = kCC13XX_26XXMatter_ItemID_ChipFactory, .subID = 0x000c }
};
const CC13XX_26XXConfig::Key CC13XX_26XXConfig::kConfigKey_LifeTimeCounter = {
    { .systemID = kCC13XX_26XXMatter_SysID, .itemID = kCC13XX_26XXMatter_ItemID_ChipFactory, .subID = 0x0010 }
};

// Keys stored in the Chip-counters namespace
const CC13XX_26XXConfig::Key CC13XX_26XXConfig::kConfigKey_BootCount = {
    { .systemID = kCC13XX_26XXMatter_SysID, .itemID = kCC13XX_26XXMatter_ItemID_ChipCounters, .subID = 0x000d }
};
const CC13XX_26XXConfig::Key CC13XX_26XXConfig::kConfigKey_TotalOperationalHours = {
    { .systemID = kCC13XX_26XXMatter_SysID, .itemID = kCC13XX_26XXMatter_ItemID_ChipCounters, .subID = 0x000f }
};

// Keys stored in the Chip-config namespace
const CC13XX_26XXConfig::Key CC13XX_26XXConfig::kConfigKey_ServiceConfig = {
    { .systemID = kCC13XX_26XXMatter_SysID, .itemID = kCC13XX_26XXMatter_ItemID_ChipConfig, .subID = 0x0012 }
};
const CC13XX_26XXConfig::Key CC13XX_26XXConfig::kConfigKey_PairedAccountId = {
    { .systemID = kCC13XX_26XXMatter_SysID, .itemID = kCC13XX_26XXMatter_ItemID_ChipConfig, .subID = 0x0013 }
};
const CC13XX_26XXConfig::Key CC13XX_26XXConfig::kConfigKey_ServiceId = {
    { .systemID = kCC13XX_26XXMatter_SysID, .itemID = kCC13XX_26XXMatter_ItemID_ChipConfig, .subID = 0x0014 }
};
const CC13XX_26XXConfig::Key CC13XX_26XXConfig::kConfigKey_LastUsedEpochKeyId = {
    { .systemID = kCC13XX_26XXMatter_SysID, .itemID = kCC13XX_26XXMatter_ItemID_ChipConfig, .subID = 0x00017 }
};
const CC13XX_26XXConfig::Key CC13XX_26XXConfig::kConfigKey_FailSafeArmed = {
    { .systemID = kCC13XX_26XXMatter_SysID, .itemID = kCC13XX_26XXMatter_ItemID_ChipConfig, .subID = 0x00018 }
};
const CC13XX_26XXConfig::Key CC13XX_26XXConfig::kConfigKey_WiFiStationSecType = {
    { .systemID = kCC13XX_26XXMatter_SysID, .itemID = kCC13XX_26XXMatter_ItemID_ChipConfig, .subID = 0x00019 }
};
const CC13XX_26XXConfig::Key CC13XX_26XXConfig::kConfigKey_RegulatoryLocation = {
    { .systemID = kCC13XX_26XXMatter_SysID, .itemID = kCC13XX_26XXMatter_ItemID_ChipConfig, .subID = 0x0001a }
};
const CC13XX_26XXConfig::Key CC13XX_26XXConfig::kConfigKey_CountryCode = {
    { .systemID = kCC13XX_26XXMatter_SysID, .itemID = kCC13XX_26XXMatter_ItemID_ChipConfig, .subID = 0x0001b }
};
// itemID 0x001c is unused (used to be breadcrumb).
const CC13XX_26XXConfig::Key CC13XX_26XXConfig::kConfigKey_UniqueId = {
    { .systemID = kCC13XX_26XXMatter_SysID, .itemID = kCC13XX_26XXMatter_ItemID_ChipConfig, .subID = 0x0001d }
};

/* Internal for the KVS interface. */
const CC13XX_26XXConfig::Key CC13XX_26XXConfig::kConfigKey_KVS_key   = { { .systemID = kCC13XX_26XXMatter_SysID,
                                                                           .itemID   = kCC13XX_26XXMatter_ItemID_ChipKVS_key } };
const CC13XX_26XXConfig::Key CC13XX_26XXConfig::kConfigKey_KVS_value = { { .systemID = kCC13XX_26XXMatter_SysID,
                                                                           .itemID   = kCC13XX_26XXMatter_ItemID_ChipKVS_value } };

/* Static local variables */
static NVINTF_nvFuncts_t sNvoctpFps = { 0 };

CHIP_ERROR CC13XX_26XXConfig::Init()
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    /* Load NVOCMP function pointers, extended API */
    NVOCMP_loadApiPtrsExt(&sNvoctpFps);

    /* Initialize NVOCMP */
    sNvoctpFps.initNV(NULL);

    return err;
}

CHIP_ERROR CC13XX_26XXConfig::ReadConfigValue(Key key, bool & val)
{
    CHIP_ERROR ret;
    size_t ignore;
    uint8_t localVal;

    ret = ReadConfigValueBin(key, &localVal, sizeof(localVal), ignore);

    // reference CC13XX_26XXConfig::WriteConfigValue(Key key, bool val) for storage of boolean values
    val = (localVal != 0);

    return ret;
}

CHIP_ERROR CC13XX_26XXConfig::ReadConfigValue(Key key, uint32_t & val)
{
    size_t ignore;

    return ReadConfigValueBin(key, (uint8_t *) &val, sizeof(val), ignore);
}

CHIP_ERROR CC13XX_26XXConfig::ReadConfigValue(Key key, uint64_t & val)
{
    size_t ignore;

    return ReadConfigValueBin(key, (uint8_t *) &val, sizeof(val), ignore);
}

CHIP_ERROR CC13XX_26XXConfig::ReadConfigValueStr(Key key, char * buf, size_t bufSize, size_t & outLen)
{
    return ReadConfigValueBin(key, (uint8_t *) buf, bufSize, outLen);
}

CHIP_ERROR CC13XX_26XXConfig::ReadConfigValueBin(Key key, uint8_t * buf, size_t bufSize, size_t & outLen)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    size_t len;

    len = sNvoctpFps.getItemLen(key.nvID);
    VerifyOrExit(len > 0, err = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND); // key not found
    VerifyOrExit(len <= bufSize, err = CHIP_ERROR_BUFFER_TOO_SMALL);

    VerifyOrExit(sNvoctpFps.readItem(key.nvID, 0, (uint16_t) len, buf) == NVINTF_SUCCESS,
                 err = CHIP_ERROR_PERSISTED_STORAGE_FAILED);

    outLen = len;

exit:
    return err;
}

static uint8_t FindKVSSubID(const char * key, uint16_t & subID)
{
    char key_scratch[PersistentStorageDelegate::kKeyLengthMax + 1];
    NVINTF_nvProxy_t nvProxy = { 0 };
    uint8_t status           = NVINTF_SUCCESS;
    // Store first valid sub ID that has already been found in NV to prevent re-scanning the same page
    uint16_t firstSubID = 0xffff;

    nvProxy.sysid  = CC13XX_26XXConfig::kConfigKey_KVS_key.nvID.systemID;
    nvProxy.itemid = CC13XX_26XXConfig::kConfigKey_KVS_key.nvID.itemID;
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
        if (firstSubID == 0xFFFF)
        {
            firstSubID = nvProxy.subid;
        }
        else if (firstSubID == nvProxy.subid)
        {
            // Scanned all of NV with no new subID found.
            status = NVINTF_NOTFOUND;
            break;
        }

    } while (NVINTF_SUCCESS == status);

    sNvoctpFps.unlockNV(lock_key);
    return status;
}

CHIP_ERROR CC13XX_26XXConfig::ReadKVS(const char * key, void * value, size_t value_size, size_t * read_bytes_size,
                                      size_t offset_bytes)
{
    CHIP_ERROR err           = CHIP_NO_ERROR;
    NVINTF_itemID_t val_item = CC13XX_26XXConfig::kConfigKey_KVS_value.nvID;
    uint16_t subID;
    size_t len;
    uint16_t read_len;

    VerifyOrExit(FindKVSSubID(key, subID) == NVINTF_SUCCESS, err = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND);

    val_item.subID = subID;

    len = sNvoctpFps.getItemLen(val_item);

    if (value_size >= (len - offset_bytes))
    {
        // reading to end of element
        read_len = len - offset_bytes;
    }
    else
    {
        read_len = value_size;
        err      = CHIP_ERROR_BUFFER_TOO_SMALL;
    }

    if (read_len > 0)
    {
        VerifyOrExit(sNvoctpFps.readItem(val_item, (uint16_t) offset_bytes, read_len, value) == NVINTF_SUCCESS,
                     err = CHIP_ERROR_PERSISTED_STORAGE_FAILED);
    }

    if (read_bytes_size)
    {
        *read_bytes_size = read_len;
    }

exit:
    return err;
}

CHIP_ERROR CC13XX_26XXConfig::WriteConfigValue(Key key, bool val)
{
    uint8_t localVal = val ? 1 : 0;
    return WriteConfigValueBin(key, (const uint8_t *) &localVal, sizeof(localVal));
}

CHIP_ERROR CC13XX_26XXConfig::WriteConfigValue(Key key, uint32_t val)
{
    return WriteConfigValueBin(key, (const uint8_t *) &val, sizeof(val));
}

CHIP_ERROR CC13XX_26XXConfig::WriteConfigValue(Key key, uint64_t val)
{
    return WriteConfigValueBin(key, (const uint8_t *) &val, sizeof(val));
}

CHIP_ERROR CC13XX_26XXConfig::WriteConfigValueStr(Key key, const char * str)
{
    size_t strLen = strlen(str);
    return WriteConfigValueBin(key, (const uint8_t *) str, strLen);
}
CHIP_ERROR CC13XX_26XXConfig::WriteConfigValueStr(Key key, const char * str, size_t strLen)
{
    return WriteConfigValueBin(key, (const uint8_t *) str, strLen);
}

CHIP_ERROR CC13XX_26XXConfig::WriteKVS(const char * key, const void * value, size_t value_size)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    uint16_t subID;

    NVINTF_itemID_t key_item = CC13XX_26XXConfig::kConfigKey_KVS_key.nvID;
    NVINTF_itemID_t val_item = CC13XX_26XXConfig::kConfigKey_KVS_value.nvID;

    if (FindKVSSubID(key, subID) != NVINTF_SUCCESS)
    {
        // key item not found, find an empty subID
        intptr_t lock_key = sNvoctpFps.lockNV();

        /* Iterate through the subID range to find an unused subID in the
         * keyspace.  SubID is a 10 bit value, reference
         * `<simplelink_sdk>/source/ti/common/nv/nvocmp.c:MVOCMP_MAXSUBID`.
         */
        for (uint16_t i = 0; i < 0x3FF; i++)
        {
            key_item.subID = i;
            if (sNvoctpFps.getItemLen(key_item) == 0U)
            {
                subID = i;
                break;
            }
        }
        sNvoctpFps.unlockNV(lock_key);

        // write they key item
        VerifyOrExit(sNvoctpFps.writeItem(key_item, (uint16_t) strlen(key), (void *) key) == NVINTF_SUCCESS,
                     err = CHIP_ERROR_PERSISTED_STORAGE_FAILED);
    }

    key_item.subID = subID;
    val_item.subID = subID;

    if (value_size == 0U)
    {
        // delete the value item if it exists
        int8_t ret = sNvoctpFps.deleteItem(val_item);
        if (ret != NVINTF_SUCCESS && ret != NVINTF_NOTFOUND)
        {
            err = CHIP_ERROR_PERSISTED_STORAGE_FAILED;
        }
    }
    else
    {
        if (sNvoctpFps.writeItem(val_item, (uint16_t) value_size, (void *) value) != NVINTF_SUCCESS)
        {
            // try to delete the key item
            sNvoctpFps.deleteItem(key_item);
            err = CHIP_ERROR_PERSISTED_STORAGE_FAILED;
        }
    }

exit:
    return err;
}

CHIP_ERROR CC13XX_26XXConfig::WriteConfigValueBin(Key key, const uint8_t * data, size_t dataLen)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    VerifyOrExit(sNvoctpFps.writeItem(key.nvID, (uint16_t) dataLen, (void *) data) == NVINTF_SUCCESS,
                 err = CHIP_ERROR_PERSISTED_STORAGE_FAILED);
exit:
    return err;
}

CHIP_ERROR CC13XX_26XXConfig::ClearKVS(const char * key)
{
    CHIP_ERROR err = CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND;
    uint16_t subID;
    NVINTF_itemID_t key_item = CC13XX_26XXConfig::kConfigKey_KVS_key.nvID;
    NVINTF_itemID_t val_item = CC13XX_26XXConfig::kConfigKey_KVS_value.nvID;

    if (FindKVSSubID(key, subID) == NVINTF_SUCCESS)
    {
        int8_t ret;

        key_item.subID = subID;
        val_item.subID = subID;
        // delete the value item if it exists
        ret = sNvoctpFps.deleteItem(val_item);
        if (ret != NVINTF_SUCCESS && ret != NVINTF_NOTFOUND)
        {
            err = CHIP_ERROR_PERSISTED_STORAGE_FAILED;
        }
        // delete the key item if it exists
        ret = sNvoctpFps.deleteItem(key_item);
        if (ret != NVINTF_SUCCESS && ret != NVINTF_NOTFOUND)
        {
            err = CHIP_ERROR_PERSISTED_STORAGE_FAILED;
        }

        err = CHIP_NO_ERROR;
    }

    return err;
}

CHIP_ERROR CC13XX_26XXConfig::ClearConfigValue(Key key)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    VerifyOrExit(sNvoctpFps.deleteItem(key.nvID) == NVINTF_SUCCESS, err = CHIP_ERROR_PERSISTED_STORAGE_FAILED);
exit:
    return err;
}

bool CC13XX_26XXConfig::ConfigValueExists(Key key)
{
    /* 0 is an invalid length for an item, getting a length of 0 means there is no item */
    return (0 != sNvoctpFps.getItemLen(key.nvID));
}

CHIP_ERROR CC13XX_26XXConfig::FactoryResetConfig(void)
{
    CHIP_ERROR err           = CHIP_NO_ERROR;
    NVINTF_nvProxy_t nvProxy = { 0 };
    uint8_t status           = NVINTF_SUCCESS;

    // Delete items with the config, counter, kvs_key and kvs_value itemIDs. Items with the factory
    // itemIDs are not deleted.

    intptr_t key = sNvoctpFps.lockNV();

    /* Setup doNext call */
    nvProxy.sysid  = kCC13XX_26XXMatter_SysID;
    nvProxy.itemid = kCC13XX_26XXMatter_ItemID_ChipConfig;
    nvProxy.flag   = NVINTF_DOSTART | NVINTF_DOITMID | NVINTF_DODELETE;

    /* Lock and wipe all items with config itemid */
    do
    {
        status = sNvoctpFps.doNext(&nvProxy);
    } while (NVINTF_SUCCESS == status);
    /* check we ran out of elements */
    VerifyOrExit(status == NVINTF_NOTFOUND, err = CHIP_ERROR_PERSISTED_STORAGE_FAILED);

    /* Setup doNext call */
    nvProxy.sysid  = kCC13XX_26XXMatter_SysID;
    nvProxy.itemid = kCC13XX_26XXMatter_ItemID_ChipCounters;
    nvProxy.flag   = NVINTF_DOSTART | NVINTF_DOITMID | NVINTF_DODELETE;

    /* Lock and wipe all items with counters itemid */
    do
    {
        status = sNvoctpFps.doNext(&nvProxy);
    } while (NVINTF_SUCCESS == status);
    /* check we ran out of elements */
    VerifyOrExit(status == NVINTF_NOTFOUND, err = CHIP_ERROR_PERSISTED_STORAGE_FAILED);

    /* Setup doNext call */
    nvProxy.sysid  = kCC13XX_26XXMatter_SysID;
    nvProxy.itemid = kCC13XX_26XXMatter_ItemID_ChipKVS_key;
    nvProxy.flag   = NVINTF_DOSTART | NVINTF_DOITMID | NVINTF_DODELETE;

    /* Lock and wipe all items with kvs_key itemid */
    do
    {
        status = sNvoctpFps.doNext(&nvProxy);
    } while (NVINTF_SUCCESS == status);
    /* check we ran out of elements */
    VerifyOrExit(status == NVINTF_NOTFOUND, err = CHIP_ERROR_PERSISTED_STORAGE_FAILED);

    /* Setup doNext call */
    nvProxy.sysid  = kCC13XX_26XXMatter_SysID;
    nvProxy.itemid = kCC13XX_26XXMatter_ItemID_ChipKVS_value;
    nvProxy.flag   = NVINTF_DOSTART | NVINTF_DOITMID | NVINTF_DODELETE;

    /* Lock and wipe all items with key_value itemid */
    do
    {
        status = sNvoctpFps.doNext(&nvProxy);
    } while (NVINTF_SUCCESS == status);
    /* check we ran out of elements */
    VerifyOrExit(status == NVINTF_NOTFOUND, err = CHIP_ERROR_PERSISTED_STORAGE_FAILED);

exit:
    sNvoctpFps.unlockNV(key);

    if (err == CHIP_NO_ERROR)
    {
        /* force compaction */
        sNvoctpFps.compactNV(0);
    }
    return err;
}

void CC13XX_26XXConfig::RunConfigUnitTest()
{
    // Run common unit test.
    ::chip::DeviceLayer::Internal::RunConfigUnitTest<CC13XX_26XXConfig>();
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
