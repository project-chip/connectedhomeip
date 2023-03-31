/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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
 *          platforms based on the Silicon Labs SDK.
 */
/* this file behaves like a config.h, comes first */

#include "config.h"
#include "assert.h"
#include <string.h>
#include <nvm3.h>
#include <nvm3_default.h>
#include <nvm3_hal_flash.h>
#include <platform/silabs/SilabsConfig.h>

using Key = chip::DeviceLayer::Internal::SilabsConfig::Key;
using namespace chip::DeviceLayer::Internal;


namespace chip {
namespace DeviceLayer {
namespace Internal {
namespace Config {

CHIP_ERROR Init()
{
    return nvm3_open(nvm3_defaultHandle, nvm3_defaultInit);
}

void Finish()
{
    nvm3_close(nvm3_defaultHandle);
}

CHIP_ERROR Read(Key key, bool & val)
{
    CHIP_ERROR err = 0;
    uint32_t objectType;
    size_t dataLen;
    bool tmpVal = 0;

    ASSERT(IsValid(key), err = -1; goto exit, "Invalid key.");

    // Get nvm3 object info.
    err = nvm3_getObjectInfo(nvm3_defaultHandle, key, &objectType, &dataLen);
    ASSERT(!err, goto exit, "NVM3 error.");

    // Read nvm3 bytes into tmp.
    err = nvm3_readData(nvm3_defaultHandle, key, &tmpVal, dataLen);
    ASSERT(!err, goto exit, "NVM3 error.");
    val = tmpVal;

exit:
    return err;
}

CHIP_ERROR Read(Key key, uint16_t & val, uint16_t def_value)
{
    CHIP_ERROR err = 0;
    uint32_t objectType;
    size_t dataLen;
    uint16_t tmpVal = 0;

    ASSERT(IsValid(key), err = -1; val = def_value; goto exit, "Invalid key.");

    // Get nvm3 object info.
    err = nvm3_getObjectInfo(nvm3_defaultHandle, key, &objectType, &dataLen);
    ASSERT(!err, val = def_value; goto exit, "NVM3 error.");

    // Read nvm3 bytes into tmp.
    err = nvm3_readData(nvm3_defaultHandle, key, &tmpVal, dataLen);
    ASSERT(!err, val = def_value; goto exit, "NVM3 error.");
    val = tmpVal;

exit:
    return err;
}

CHIP_ERROR Read(Key key, uint32_t & val, uint32_t def_value)
{
    CHIP_ERROR err = 0;
    uint32_t objectType;
    size_t dataLen;
    uint32_t tmpVal = 0;

    ASSERT(IsValid(key), err = -1; val = def_value; goto exit, "Invalid key.");

    // Get nvm3 object info.
    err = nvm3_getObjectInfo(nvm3_defaultHandle, key, &objectType, &dataLen);
    ASSERT(!err, val = def_value; goto exit, "NVM3 error.");

    // Read nvm3 bytes into tmp.
    err = nvm3_readData(nvm3_defaultHandle, key, &tmpVal, dataLen);
    ASSERT(!err, val = def_value; goto exit, "NVM3 error.");
    val = tmpVal;

exit:
    return err;
}

CHIP_ERROR Read(Key key, uint64_t & val)
{
    CHIP_ERROR err = 0;
    uint32_t objectType;
    size_t dataLen;
    uint64_t tmpVal = 0;

    ASSERT(IsValid(key), err = -1; goto exit, "Invalid key.");

    // Get nvm3 object info.
    err = nvm3_getObjectInfo(nvm3_defaultHandle, key, &objectType, &dataLen);
    ASSERT(!err, goto exit, "NVM3 error.");

    // Read nvm3 bytes into tmp.
    err = nvm3_readData(nvm3_defaultHandle, key, &tmpVal, dataLen);
    ASSERT(!err, goto exit, "NVM3 error.");
    val = tmpVal;

exit:
    return err;
}

CHIP_ERROR ReadStr(Key key, char * buf, size_t bufSize, size_t & outLen)
{
    CHIP_ERROR err = 0;
    uint32_t objectType;
    size_t dataLen;
    outLen = 0;

    ASSERT(IsValid(key), err = -1; goto exit, "Invalid key.");

    // Get nvm3 object info.
    err = nvm3_getObjectInfo(nvm3_defaultHandle, key, &objectType, &dataLen);
    ASSERT(!err, goto exit, "NVM3 error.");
    ASSERT(dataLen > 0, return -2, "Invalid length.");

    if (buf != NULL)
    {
        // Read nvm3 bytes directly into the output buffer- check buffer is
        // long enough to take the string (nvm3 string does not include the
        // terminator char).
        ASSERT(bufSize > dataLen, return -3, "Buffer too small.");

        err = nvm3_readData(nvm3_defaultHandle, key, buf, dataLen);
        ASSERT(!err, goto exit, "NVM3 error.");

        outLen      = ((dataLen == 1) && (buf[0] == 0)) ? 0 : dataLen;
        buf[outLen] = 0; // Add the terminator char.
    }
    else
    {
        if (dataLen > 1)
        {
            outLen = dataLen;
        }
        else
        {
            // Read the first byte of the nvm3 string into a tmp var.
            char firstByte;
            err = nvm3_readData(nvm3_defaultHandle, key, &firstByte, 1);
            ASSERT(!err, goto exit, "NVM3 error.");

            outLen = (firstByte == 0) ? 0 : dataLen;
        }
    }

exit:
    return err;
}

CHIP_ERROR ReadBin(Key key, uint8_t * buf, size_t bufSize, size_t & outLen)
{
    CHIP_ERROR err = 0;
    uint32_t objectType;
    size_t dataLen;

    outLen = 0;
    ASSERT(IsValid(key), err = -1; goto exit, "Invalid key.");

    // Get nvm3 object info.
    err = nvm3_getObjectInfo(nvm3_defaultHandle, key, &objectType, &dataLen);
    ASSERT(!err, goto exit, "NVM3 error.");
    ASSERT(dataLen > 0, return -2, "Invalid length.");

    if (buf != NULL)
    {
        // Read nvm3 bytes directly into output buffer- check buffer is long
        // enough to take the data.
        ASSERT(bufSize >= dataLen, return -3, "Buffer too small.");

        err = nvm3_readData(nvm3_defaultHandle, key, buf, dataLen);
        ASSERT(!err, goto exit, "NVM3 error.");

        outLen = dataLen;
    }

exit:
    return err;
}

CHIP_ERROR ReadCounter(uint8_t counterIdx, uint32_t & val)
{
    CHIP_ERROR err = 0;
    uint32_t tmpVal = 0;
    Key key         = SilabsConfig::kMinConfigKey_MatterCounter + counterIdx;

    ASSERT(IsValid(key), err = -1; goto exit, "Invalid key.");

    // Read bytes into tmp.
    err = nvm3_readCounter(nvm3_defaultHandle, key, &tmpVal);
    ASSERT(!err, goto exit, "NVM3 error.");
    val = tmpVal;

exit:
    return err;
}

CHIP_ERROR Write(Key key, bool val)
{
    CHIP_ERROR err = 0;

    ASSERT(IsValid(key), err = -1; goto exit, "Invalid key.");

    err = nvm3_writeData(nvm3_defaultHandle, key, &val, sizeof(val));
    ASSERT(!err, goto exit, "NVM3 error.");

exit:
    return err;
}

CHIP_ERROR Write(Key key, uint16_t val)
{
    CHIP_ERROR err = 0;

    ASSERT(IsValid(key), err = -1; goto exit, "Invalid key.");

    err = nvm3_writeData(nvm3_defaultHandle, key, &val, sizeof(val));
    ASSERT(!err, goto exit, "NVM3 error.");

exit:
    return err;
}

CHIP_ERROR Write(Key key, uint32_t val)
{
    CHIP_ERROR err = 0;

    ASSERT(IsValid(key), err = -1; goto exit, "Invalid key.");

    err = nvm3_writeData(nvm3_defaultHandle, key, &val, sizeof(val));
    ASSERT(!err, goto exit, "NVM3 error.");

exit:
    return err;
}

CHIP_ERROR Write(Key key, uint64_t val)
{
    CHIP_ERROR err = 0;

    ASSERT(IsValid(key), err = -1; goto exit, "Invalid key.");

    err = nvm3_writeData(nvm3_defaultHandle, key, &val, sizeof(val));
    ASSERT(!err, goto exit, "NVM3 error.");

exit:
    return err;
}

CHIP_ERROR WriteStr(Key key, const char * str)
{
    return WriteStr(key, str, (str != NULL) ? strlen(str) : 0);
}

CHIP_ERROR WriteStr(Key key, const char * str, size_t strLen)
{
    CHIP_ERROR err = 0;

    ASSERT(IsValid(key), err = -1; goto exit, "Invalid key.");

    if (str != NULL)
    {
        // Write the string to nvm3 without the terminator char (apart from
        // empty strings where only the terminator char is stored in nvm3).
        err = nvm3_writeData(nvm3_defaultHandle, key, str, (strLen > 0) ? strLen : 1);
        ASSERT(!err, goto exit, "NVM3 error.");
    }

exit:
    return err;
}

CHIP_ERROR WriteBin(Key key, const uint8_t * data, size_t dataLen)
{
    CHIP_ERROR err = 0;

    ASSERT(IsValid(key), err = -1; goto exit, "Invalid key.");

    // Only write NULL pointer if the given size is 0, since in that case, nothing is read at the pointer
    if ((data != NULL) || (dataLen == 0))
    {
        // Write the binary data to nvm3.
        err = nvm3_writeData(nvm3_defaultHandle, key, data, dataLen);
        ASSERT(!err, goto exit, "NVM3 error.");
    }

exit:
    return err;
}

CHIP_ERROR WriteCounter(uint8_t counterIdx, uint32_t val)
{
    CHIP_ERROR err = 0;
    Key key = SilabsConfig::kMinConfigKey_MatterCounter + counterIdx;

    ASSERT(IsValid(key), err = -1; goto exit, "Invalid key.");

    err = nvm3_writeCounter(nvm3_defaultHandle, key, val);
    ASSERT(!err, goto exit, "NVM3 error.");

exit:
    return err;
}

CHIP_ERROR Clear(Key key)
{
    CHIP_ERROR err = 0;

    // Delete the nvm3 object with the given key id.
    err = nvm3_deleteObject(nvm3_defaultHandle, key);
    ASSERT(!err, goto exit, "NVM3 error.");

exit:
    return err;
}

bool Exists(Key key)
{
    uint32_t objectType;
    size_t dataLen;

    // Find object with key id.
    CHIP_ERROR err = nvm3_getObjectInfo(nvm3_defaultHandle, key, &objectType, &dataLen);
    return (err == CHIP_NO_ERROR);
}

bool Exists(Key key, size_t & dataLen)
{
    uint32_t objectType;
    size_t dLen;

    // Find object with key id.
    CHIP_ERROR err = nvm3_getObjectInfo(nvm3_defaultHandle, key, &objectType, &dLen);
    if (err == CHIP_NO_ERROR)
    {
        dataLen = dLen;
    }
    return (err == CHIP_NO_ERROR);
}

bool IsValid(Key key)
{
    // Returns true if the key is in the Matter nvm3 reserved key range.
    // Additional check validates that the user consciously defined the expected key range
    return (key >= kMatterNvm3KeyLoLimit) &&
           (key <= kMatterNvm3KeyHiLimit) &&
           (key >= SilabsConfig::kMinConfigKey_MatterFactory) &&
           (key <= SilabsConfig::kMaxConfigKey_MatterKvs);
}

void Repack(void)
{
    // Repack nvm3 flash if nvm3 space < headroom threshold.
    // Note- checking periodically during idle periods should prevent
    // forced repack events on any write operation.
    nvm3_repack(nvm3_defaultHandle);
}

} // namespace Config
} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
