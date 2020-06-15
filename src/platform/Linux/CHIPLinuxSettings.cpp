/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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
 *         This file implements a class for managing client application
 *         user-editable settings.
 *
 */

#include <errno.h>
#include <libgen.h>
#include <string>
#include <fstream>
#include <unistd.h>

#include <platform/Linux/CHIPLinuxSettings.h>
#include <platform/internal/CHIPDeviceLayerInternal.h>
#include <support/Base64.h>
#include <support/CodeUtils.h>
#include <support/logging/CHIPLogging.h>

namespace chip {
namespace DeviceLayer {
namespace Internal {

ChipLinuxSettings::ChipLinuxSettings()
{
    mDirty    = false;
    mWritable = true;
}

ChipLinuxSettings::~ChipLinuxSettings()
{
}

CHIP_ERROR ChipLinuxSettings::Init(const char * configFile)
{
    CHIP_ERROR retval = CHIP_NO_ERROR;
    std::ifstream ifs;

    if (retval == CHIP_NO_ERROR)
    {
        mConfigPath.assign(configFile);
        retval = ChipLinuxConfig::Init();
    }

    ifs.open (configFile, std::ifstream::in);

    if (retval == CHIP_NO_ERROR && !ifs.good())
    {
        mDirty = true;
        retval = Commit();
        mDirty = false;
    }

    if (retval == CHIP_NO_ERROR)
    {
        retval = ChipLinuxConfig::AddConfig(configFile);
    }

    return retval;
}

CHIP_ERROR ChipLinuxSettings::ReadValue(const char * key, bool & val)
{
    CHIP_ERROR retval = CHIP_NO_ERROR;
    uint32_t result;

    mLock.lock();

    retval = ChipLinuxConfig::GetUIntValue(key, result);
    val    = (result == 0 ? false : true);

    mLock.unlock();

    return retval;
}

CHIP_ERROR ChipLinuxSettings::ReadValue(const char * key, uint32_t & val)
{
    CHIP_ERROR retval = CHIP_NO_ERROR;

    mLock.lock();

    retval = ChipLinuxConfig::GetUIntValue(key, val);

    mLock.unlock();

    return retval;
}

CHIP_ERROR ChipLinuxSettings::ReadValue(const char * key, uint64_t & val)
{
    CHIP_ERROR retval = CHIP_NO_ERROR;

    mLock.lock();

    retval = ChipLinuxConfig::GetUInt64Value(key, val);

    mLock.unlock();

    return retval;
}

CHIP_ERROR ChipLinuxSettings::ReadValueStr(const char * key, char * buf, size_t bufSize, size_t & outLen)
{
    CHIP_ERROR retval = CHIP_NO_ERROR;

    mLock.lock();

    retval = ChipLinuxConfig::GetBinaryBlobValue(key, (uint8_t *) buf, bufSize, outLen);

    mLock.unlock();

    return retval;
}

CHIP_ERROR ChipLinuxSettings::ReadValueBin(const char * key, uint8_t * buf, size_t bufSize, size_t & outLen)
{
    CHIP_ERROR retval = CHIP_NO_ERROR;

    mLock.lock();

    retval = ChipLinuxConfig::GetBinaryBlobValue(key, buf, bufSize, outLen);

    mLock.unlock();

    return retval;
}

CHIP_ERROR ChipLinuxSettings::WriteValue(const char * key, bool val)
{
    CHIP_ERROR retval = CHIP_NO_ERROR;

    if (!mWritable)
    {
        return CHIP_ERROR_BAD_REQUEST;
    }

    if (val)
    {
        retval = WriteValue(key, (uint32_t) 1);
    }
    else
    {
        retval = WriteValue(key, (uint32_t) 0);
    }

    return retval;
}

CHIP_ERROR ChipLinuxSettings::WriteValue(const char * key, uint32_t val)
{
    char buf[32];

    if (!mWritable)
    {
        return CHIP_ERROR_BAD_REQUEST;
    }

    snprintf(buf, sizeof(buf), "%d", val);

    return WriteValueStr(key, buf);
}

CHIP_ERROR ChipLinuxSettings::WriteValue(const char * key, uint64_t val)
{
    char buf[64];

    if (!mWritable)
    {
        return CHIP_ERROR_BAD_REQUEST;
    }

    snprintf(buf, sizeof(buf), "%llu", val);

    return WriteValueStr(key, buf);
}

CHIP_ERROR ChipLinuxSettings::WriteValueStr(const char * key, const char * val)
{
    CHIP_ERROR retval = CHIP_NO_ERROR;

    if (!mWritable)
    {
        return CHIP_ERROR_BAD_REQUEST;
    }

    mLock.lock();

    retval = ChipLinuxConfig::AddEntry(key, val);

    mDirty = true;

    mLock.unlock();

    return retval;
}

CHIP_ERROR ChipLinuxSettings::WriteValueBin(const char * key, const uint8_t * data, size_t dataLen)
{
    static const size_t kMaxBlobSize = 5 * 1024;

    CHIP_ERROR retval         = CHIP_NO_ERROR;
    char * encodedData        = NULL;
    size_t encodedDataLen     = 0;
    size_t expectedEncodedLen = ((dataLen + 3) * 4) / 3;

    if (!mWritable)
    {
        return CHIP_ERROR_BAD_REQUEST;
    }

    // We only support encoding blobs up to 5kb
    if (dataLen > kMaxBlobSize)
    {
        retval = CHIP_ERROR_INVALID_ARGUMENT;
    }

    // Compute our expectedEncodedLen
    // Allocate just enough space for the encoded data, and the NULL terminator
    if (retval == CHIP_NO_ERROR)
    {
        encodedData = (char *) malloc(expectedEncodedLen + 1);
        if (encodedData == NULL)
        {
            retval = CHIP_ERROR_NO_MEMORY;
        }
    }

    // Encode it
    if (retval == CHIP_NO_ERROR)
    {
        encodedDataLen              = Base64Encode(data, dataLen, encodedData);
        encodedData[encodedDataLen] = 0;
    }

    // Store it
    if (retval == CHIP_NO_ERROR)
    {
        WriteValueStr(key, (const char *) encodedData);
    }

    // Free memory
    if (encodedData)
    {
        free(encodedData);
    }

    return retval;
}

CHIP_ERROR ChipLinuxSettings::ClearValue(const char * key)
{
    CHIP_ERROR retval = CHIP_NO_ERROR;

    mLock.lock();

    retval = ChipLinuxConfig::RemoveEntry(key);

    if (retval == CHIP_NO_ERROR)
    {
        mDirty = true;
    }
    else
    {
        retval = CHIP_ERROR_KEY_NOT_FOUND;
    }

    mLock.unlock();

    return retval;
}

CHIP_ERROR ChipLinuxSettings::ClearAll(void)
{
    CHIP_ERROR retval = CHIP_NO_ERROR;

    mLock.lock();

    retval = ChipLinuxConfig::RemoveAll();

    if (retval == CHIP_NO_ERROR)
    {
        retval = Commit();
    }
    else
    {
        retval = CHIP_ERROR_PERSISTED_STORAGE_FAIL;
    }

    mLock.unlock();

    return retval;
}

bool ChipLinuxSettings::HasValue(const char * key)
{
    bool retval;

    mLock.lock();

    retval = ChipLinuxConfig::HasValue(key);

    mLock.unlock();

    return retval;
}

void ChipLinuxSettings::SetWritability(bool writable)
{
    mWritable = writable;
}

CHIP_ERROR ChipLinuxSettings::Commit(void)
{
    CHIP_ERROR retval = CHIP_NO_ERROR;
    const char * path = mConfigPath.c_str();

    if (mDirty && mWritable && path != NULL && strcmp(path, ""))
    {
        mLock.lock();

        retval = ChipLinuxConfig::CommitConfig(path);

        mLock.unlock();
    }
    else
    {
        retval = CHIP_ERROR_PERSISTED_STORAGE_FAIL;
    }

    return retval;
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
