/*
 *
 *    Copyright (c) 2021-2022 Project CHIP Authors
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

#include <CC32XXConfig.h>
#include <lib/core/CHIPEncoding.h>
#include <lib/support/CodeUtils.h>
#include <ti/drivers/net/wifi/simplelink.h>

extern "C" {
int FILE_write(char * pFilename, uint16_t length, uint8_t * pValue, uint32_t * pToken, uint32_t flags);
int FILE_read(int8_t * pFilename, uint16_t length, uint8_t * pValue, uint32_t token);
};

// need to define custom tokens to read/write files from the file system
#define KVS_TOKEN 0x13578642

// need between 4k and 8k bytes to store the LL when reading/writing it as a buffer
#define NV_BUFFER_SIZE 8192

uint16_t NVBufferLength = 0;

extern "C" void cc32xxLog(const char * aFormat, ...);

char listName[] = "/sys/matter/kvs.cfg";

namespace chip {
namespace DeviceLayer {
namespace Internal {

/**
 * This class is designed to be mixed-in to concrete implementation classes as a means to
 * provide access to configuration information to generic base classes.
 *
 * This class contains the definition of a LL entry -- calling the constructor creates a LL entry only, adding the entry to a LL is
 * handled in CC32XXKVSList
 */
class CC32XXKVSEntry
{
private:
    // KVS key
    char mKey[40] = "";

    // KVS values
    uint16_t mValueLen;
    uint8_t * mValue;

public:
    CC32XXKVSEntry * mPNext;

    CC32XXKVSEntry(char * key, const uint8_t * pBuf, uint16_t len)
    {
        Platform::CopyString(mKey, key);

        mValueLen = len;
        mValue    = new uint8_t[len];
        if (mValue)
        {
            memcpy(mValue, pBuf, len);
        }
        mPNext = NULL;
    }

    bool IsMatch(const char * key) { return (strcmp(mKey, key) == 0); }

    char * Key() { return mKey; }

    uint16_t Len() { return mValueLen; }

    uint8_t * Value() { return mValue; }

    CHIP_ERROR ReadVal(uint8_t * pBuff, size_t len)
    {
        CHIP_ERROR err = CHIP_ERROR_NOT_FOUND;
        if (mValueLen <= (uint16_t) len)
        {
            memcpy(pBuff, mValue, mValueLen);
            err = CHIP_NO_ERROR;
        }
        return err;
    }

    CHIP_ERROR UpdateVal(const uint8_t * pBuff, uint16_t len)
    {
        CHIP_ERROR err = CHIP_ERROR_INVALID_MESSAGE_LENGTH;
        if (len > 0)
        {
            if (mValue)
            {
                delete (mValue);
            }
            mValue    = new uint8_t[len];
            mValueLen = len;
            memcpy(mValue, pBuff, len);
            err = CHIP_NO_ERROR;
        }
        return err;
    }

    uint16_t DeleteVal(void)
    {
        delete mValue;
        return 0;
    }
};

/**
 * Linked List traversal operations for when it is in RAM, and operations to read and write from NV
 */
class CC32XXKVSList
{
private:
    CC32XXKVSEntry * mPHead;

public:
    CC32XXKVSList() { mPHead = NULL; }

    CC32XXKVSEntry * GetEntryByKey(const char * key)
    {
        CC32XXKVSEntry * pEntry = mPHead;
        while (pEntry)
        {
            if (pEntry->IsMatch(key))
            {
                return pEntry;
            }
            pEntry = pEntry->mPNext;
        }
        return NULL;
    }

    CHIP_ERROR AddEntryByKey(char * key, const uint8_t * pBuff, const uint16_t len)
    {
        CHIP_ERROR err;
        CC32XXKVSEntry * pEntry = GetEntryByKey(key);

        if (!pEntry)
        {
            CC32XXKVSEntry * pEntryNew = new CC32XXKVSEntry(key, pBuff, len);

            if (mPHead)
            {
                pEntryNew->mPNext = mPHead;
            }

            mPHead = pEntryNew;

            err = CHIP_NO_ERROR;
        }
        else
        {
            err = pEntry->UpdateVal(pBuff, len);
        }
        return err;
    }

    CHIP_ERROR DeleteEntryByKey(char * key)
    {
        CHIP_ERROR err = CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND;

        CC32XXKVSEntry * temp = mPHead;
        CC32XXKVSEntry * prev = NULL;

        const char * tempKey = temp->Key();

        if (strcmp(tempKey, key) == 0)
        {
            mPHead = temp->mPNext;
            temp->DeleteVal();
            delete temp;
            err = CHIP_NO_ERROR;
        }
        else
        {
            while (temp != NULL && strcmp(temp->Key(), key) != 0)
            {
                prev = temp;
                temp = temp->mPNext;
            }
            if (temp != NULL)
            {
                prev->mPNext = temp->mPNext;

                delete temp;
                err = CHIP_NO_ERROR;
            }
        }

        return err;
    }

    uint8_t * SerializeLinkedList(uint16_t * length)
    {
        uint8_t * list                = new uint8_t[8192];
        CC32XXKVSEntry * currentEntry = mPHead;
        uint16_t bufferLength         = 0;

        while (currentEntry != NULL)
        {
            // copy key length
            list[bufferLength] = (uint8_t) strlen(currentEntry->Key());
            bufferLength++;

            // copy key
            memcpy(list + bufferLength, currentEntry->Key(), strlen(currentEntry->Key()));
            bufferLength += (uint16_t) strlen(currentEntry->Key());

            // copy value length
            list[bufferLength]     = (uint8_t)(currentEntry->Len() & 0xFF);
            list[bufferLength + 1] = (uint8_t)((currentEntry->Len() & 0xFF00) >> 8);
            bufferLength           = bufferLength + 2;

            // copy value
            uint8_t * value = currentEntry->Value();
            memcpy(list + bufferLength, value, currentEntry->Len());
            bufferLength += currentEntry->Len();

            currentEntry = currentEntry->mPNext;
        }

        *length = bufferLength;

        return list;
    }

    void CreateLinkedListFromNV(uint8_t * list, uint16_t length)
    {
        uint16_t currentLength = 0;

        // check for end of LL
        while (currentLength < length)
        {
            // read in key length
            uint8_t keyLen = list[currentLength];
            currentLength++;

            // read in key

            char key[40] = { 0 };
            memcpy(key, list + currentLength, keyLen);
            currentLength += keyLen;

            // read in value length

            uint16_t valueLen = 0;
            valueLen          = (uint16_t)(list[currentLength] | list[currentLength + 1] << 8);
            currentLength += 2;

            // read in value

            uint8_t * value = new uint8_t[valueLen];
            memcpy(value, list + currentLength, valueLen);
            currentLength += valueLen;

            // add entry to LL

            AddEntryByKey(key, value, valueLen);

            // value is stored in the LL, we do not need the value variable above

            delete value;
        }
    }
};

// *** CAUTION ***: Changing the names or namespaces of these values will *break* existing devices.

// Keys stored in the Chip-factory namespace
const CC32XXConfig::Key CC32XXConfig::kConfigKey_SerialNum           = { "TI_kConfigKey_SerialNum" };
const CC32XXConfig::Key CC32XXConfig::kConfigKey_MfrDeviceId         = { "TI_kConfigKey_MfrDeviceId" };
const CC32XXConfig::Key CC32XXConfig::kConfigKey_MfrDeviceCert       = { "TI_kConfigKey_MfrDeviceCert" };
const CC32XXConfig::Key CC32XXConfig::kConfigKey_MfrDeviceICACerts   = { "TI_kConfigKey_MfrDeviceICACerts" };
const CC32XXConfig::Key CC32XXConfig::kConfigKey_MfrDevicePrivateKey = { "TI_kConfigKey_MfrDevicePrivateKey" };
const CC32XXConfig::Key CC32XXConfig::kConfigKey_HardwareVersion     = { "TI_kConfigKey_HardwareVersion" };
const CC32XXConfig::Key CC32XXConfig::kConfigKey_ManufacturingDate   = { "TI_kConfigKey_ManufacturingDate" };
const CC32XXConfig::Key CC32XXConfig::kConfigKey_SetupPinCode        = { "TI_kConfigKey_SetupPinCode" };
const CC32XXConfig::Key CC32XXConfig::kConfigKey_SetupDiscriminator  = { "TI_kConfigKey_SetupDiscriminator" };

// Keys stored in the Chip-config namespace
const CC32XXConfig::Key CC32XXConfig::kConfigKey_FabricId           = { "TI_kConfigKey_FabricId" };
const CC32XXConfig::Key CC32XXConfig::kConfigKey_ServiceConfig      = { "TI_kConfigKey_ServiceConfig" };
const CC32XXConfig::Key CC32XXConfig::kConfigKey_PairedAccountId    = { "TI_kConfigKey_PairedAccountId" };
const CC32XXConfig::Key CC32XXConfig::kConfigKey_ServiceId          = { "TI_kConfigKey_ServiceId" };
const CC32XXConfig::Key CC32XXConfig::kConfigKey_FabricSecret       = { "TI_kConfigKey_FabricSecret" };
const CC32XXConfig::Key CC32XXConfig::kConfigKey_GroupKeyIndex      = { "TI_kConfigKey_GroupKeyIndex" };
const CC32XXConfig::Key CC32XXConfig::kConfigKey_LastUsedEpochKeyId = { "TI_kConfigKey_LastUsedEpochKeyId" };
const CC32XXConfig::Key CC32XXConfig::kConfigKey_FailSafeArmed      = { "TI_kConfigKey_FailSafeArmed" };
const CC32XXConfig::Key CC32XXConfig::kConfigKey_WiFiStationSecType = { "TI_kConfigKey_WiFiStationSecType" };
const CC32XXConfig::Key CC32XXConfig::kConfigKey_RegulatoryLocation = { "TI_kConfigKey_RegulatoryLocation" };
const CC32XXConfig::Key CC32XXConfig::kConfigKey_CountryCode        = { "TI_kConfigKey_CountryCode" };
const CC32XXConfig::Key CC32XXConfig::kConfigKey_Breadcrumb         = { "TI_kConfigKey_Breadcrumb" };
const CC32XXConfig::Key CC32XXConfig::kConfigKey_UniqueId           = { "TI_kConfigKey_UniqueId" };

const CC32XXConfig::Key CC32XXConfig::kConfigKey_Spake2pIterationCount = { "TI_kConfigKey_Spake2pIterationCount" };
const CC32XXConfig::Key CC32XXConfig::kConfigKey_Spake2pSalt           = { "TI_kConfigKey_Spake2pSalt" };
const CC32XXConfig::Key CC32XXConfig::kConfigKey_Spake2pVerifier       = { "TI_kConfigKey_Spake2pVerifier" };

CC32XXKVSList * pList;

CHIP_ERROR CC32XXConfig::Init()
{
    cc32xxLog("[%s], KVS List created", __FUNCTION__);
    pList = new CC32XXKVSList();
    ReadKVSFromNV();
    return CHIP_NO_ERROR;
}

CHIP_ERROR CC32XXConfig::ReadConfigValue(Key key, bool & val)
{
    CHIP_ERROR ret;
    size_t ignore;
    uint8_t localVal;
    cc32xxLog("[%s] %s", __FUNCTION__, key.key);

    ret = ReadConfigValueBin(key, &localVal, sizeof(localVal), ignore);

    // reference CC32XXConfig::WriteConfigValue(Key key, bool val) for storage of boolean values
    val = (localVal != 0);

    return ret;
}

CHIP_ERROR CC32XXConfig::ReadConfigValue(Key key, uint32_t & val)
{
    size_t ignore;
    return ReadConfigValueBin(key, (uint8_t *) &val, sizeof(val), ignore);
}

CHIP_ERROR CC32XXConfig::ReadConfigValue(Key key, uint64_t & val)
{
    size_t ignore;
    return ReadConfigValueBin(key, (uint8_t *) &val, sizeof(val), ignore);
}

CHIP_ERROR CC32XXConfig::ReadConfigValueStr(Key key, char * buf, size_t bufSize, size_t & outLen)
{
    return ReadConfigValueBin(key, (uint8_t *) buf, bufSize, outLen);
}

CHIP_ERROR CC32XXConfig::ReadConfigValueBin(Key key, uint8_t * buf, size_t bufSize, size_t & outLen)
{
    cc32xxLog("[%s] %s", __FUNCTION__, key.key);

    CC32XXKVSEntry * pEntry = pList->GetEntryByKey(key.key);

    VerifyOrReturnError(pEntry != nullptr, CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND);

    pEntry->ReadVal(buf, bufSize);
    outLen = pEntry->Len();
    return CHIP_NO_ERROR;
}

CHIP_ERROR CC32XXConfig::WriteConfigValue(Key key, bool val)
{
    uint8_t localVal = val ? 1 : 0;
    return WriteConfigValueBin(key, (const uint8_t *) &localVal, sizeof(localVal));
}

CHIP_ERROR CC32XXConfig::WriteConfigValue(Key key, uint32_t val)
{
    return WriteConfigValueBin(key, (const uint8_t *) &val, sizeof(val));
}

CHIP_ERROR CC32XXConfig::WriteConfigValue(Key key, uint64_t val)
{
    return WriteConfigValueBin(key, (const uint8_t *) &val, sizeof(val));
}

CHIP_ERROR CC32XXConfig::WriteConfigValueStr(Key key, const char * str)
{
    size_t strLen = strlen(str);
    return WriteConfigValueBin(key, (const uint8_t *) str, strLen);
}
CHIP_ERROR CC32XXConfig::WriteConfigValueStr(Key key, const char * str, size_t strLen)
{
    return WriteConfigValueBin(key, (const uint8_t *) str, strLen);
}

CHIP_ERROR CC32XXConfig::WriteConfigValueBin(Key key, const uint8_t * data, size_t dataLen)
{
    cc32xxLog("[%s]", __FUNCTION__);

    CHIP_ERROR err = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND;
    err            = pList->AddEntryByKey(key.key, data, (uint16_t) dataLen);
    return err;
}

CHIP_ERROR CC32XXConfig::ClearConfigValue(Key key)
{
    cc32xxLog("[%s] %s", __FUNCTION__, key.key);

    CHIP_ERROR err = CHIP_NO_ERROR;
    pList->DeleteEntryByKey(key.key);
    return err;
}

bool CC32XXConfig::ConfigValueExists(Key key)
{
    cc32xxLog("[%s] %s", __FUNCTION__, key.key);

    bool ret                = false;
    CC32XXKVSEntry * pEntry = pList->GetEntryByKey(key.key);
    if (pEntry)
        ret = true;
    return ret;
}

CHIP_ERROR CC32XXConfig::FactoryResetConfig()
{
    cc32xxLog("[%s] ", __FUNCTION__);

    while (true)
        ;
    CHIP_ERROR err = CHIP_NO_ERROR;
    return err;
}

void CC32XXConfig::RunConfigUnitTest()
{
    cc32xxLog("[%s] ", __FUNCTION__);

    // Run common unit test.
    ::chip::DeviceLayer::Internal::RunConfigUnitTest<CC32XXConfig>();
}

CHIP_ERROR CC32XXConfig::ClearKVS(const char * key)
{
    CHIP_ERROR err     = CHIP_NO_ERROR;
    char keyBuffer[40] = "";
    memcpy(keyBuffer, key, strlen(key));
    err = pList->DeleteEntryByKey(keyBuffer);
    cc32xxLog("[%s] key %s", __FUNCTION__, key);
    return err;
}

CHIP_ERROR CC32XXConfig::WriteKVS(const char * key, const void * value, size_t value_size)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    cc32xxLog("[%s] Key is %s value size is %d ", __FUNCTION__, key, value_size);
    // Write key value pair as LL entry in RAM buffer
    char keyBuffer[40] = "";
    memcpy(keyBuffer, key, strlen(key));
    pList->AddEntryByKey(keyBuffer, (uint8_t *) value, value_size);

    return err;
}

CHIP_ERROR CC32XXConfig::ReadKVS(const char * key, void * value, size_t value_size, size_t * read_bytes_size, size_t offset_bytes)
{
    CHIP_ERROR err         = CHIP_NO_ERROR;
    CC32XXKVSEntry * entry = pList->GetEntryByKey(key);
    // if (!entry)
    // {
    //     err = CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND;
    //     return err;
    // }
    VerifyOrReturnError(entry != nullptr, CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND);
    uint8_t * entryValue = entry->Value();
    uint16_t valueLen    = entry->Len();

    size_t readLen;

    if ((offset_bytes + value_size) > valueLen)
    {
        // trying to read up to the end of the element
        readLen = valueLen - offset_bytes;
    }
    else
    {
        readLen = value_size;
    }

    memcpy(value, entryValue + offset_bytes, readLen);

    if (read_bytes_size)
    {
        *read_bytes_size = readLen;
    }

    cc32xxLog("[%s] key %s, read %d bytes", __FUNCTION__, key, readLen);
    return err;
}

CHIP_ERROR CC32XXConfig::WriteKVSToNV()
{
    uint8_t * list = pList->SerializeLinkedList(&NVBufferLength);

    uint32_t token = KVS_TOKEN;

    uint32_t fileSystemFlags = SL_FS_CREATE_STATIC_TOKEN | SL_FS_CREATE_VENDOR_TOKEN;

    int ret = FILE_write(listName, NVBufferLength, list, &token, fileSystemFlags);
    if (ret < 0)
    {
        cc32xxLog("could not write in Linked List to NV, error %d", ret);
        return CHIP_ERROR_PERSISTED_STORAGE_FAILED;
    }

    else
    {
        return CHIP_NO_ERROR;
    }
    // return error
}

CHIP_ERROR CC32XXConfig::ReadKVSFromNV()
{
    int rc;
    uint16_t bufferLength;

    uint8_t * list = new uint8_t[NV_BUFFER_SIZE];
    rc             = FILE_read((int8_t *) listName, NV_BUFFER_SIZE, list, KVS_TOKEN);
    if (rc > 0)
    {
        bufferLength = rc;
        pList->CreateLinkedListFromNV(list, bufferLength);
        cc32xxLog("read in KVS Linked List from NV");
        return CHIP_NO_ERROR;
    }
    else
    {
        cc32xxLog("could not read in Linked List from NV, error %d", rc);
        return CHIP_ERROR_PERSISTED_STORAGE_FAILED;
    }
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
