/*
 *
 *    Copyright (c) 2020-2022 Project CHIP Authors
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
 *         user-editable settings on Linux platform.
 *
 */

#include <errno.h>
#include <fstream>
#include <inttypes.h>
#include <libgen.h>
#include <string>
#include <unistd.h>

#include <lib/support/Base64.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/ScopedBuffer.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/Linux/CHIPLinuxStorage.h>
#include <platform/internal/CHIPDeviceLayerInternal.h>

namespace chip {
namespace DeviceLayer {
namespace Internal {

ChipLinuxStorage::ChipLinuxStorage()
{
    mDirty = false;
}

ChipLinuxStorage::~ChipLinuxStorage() {}

CHIP_ERROR ChipLinuxStorage::Init(const char * configFile)
{
    CHIP_ERROR retval = CHIP_NO_ERROR;

    if (mInitialized)
    {
        ChipLogError(DeviceLayer, "ChipLinuxStorage::Init: Attempt to re-initialize with KVS config file: %s, IGNORING.",
                     StringOrNullMarker(configFile));
        return CHIP_NO_ERROR;
    }

    ChipLogDetail(DeviceLayer, "ChipLinuxStorage::Init: Using KVS config file: %s", StringOrNullMarker(configFile));

    mConfigPath.assign(configFile);
    retval = ChipLinuxStorageIni::Init();

    if (retval == CHIP_NO_ERROR)
    {
        std::ifstream ifs;

        ifs.open(configFile, std::ifstream::in);

        // Create default setting file if not exist.
        if (!ifs.good())
        {
            mDirty = true;
            retval = Commit();
            mDirty = false;
        }
    }

    if (retval == CHIP_NO_ERROR)
    {
        retval = ChipLinuxStorageIni::AddConfig(mConfigPath);
    }

    mInitialized = true;

    return retval;
}

CHIP_ERROR ChipLinuxStorage::ReadValue(const char * key, bool & val)
{
    CHIP_ERROR retval = CHIP_NO_ERROR;
    uint32_t result;

    mLock.lock();

    retval = ChipLinuxStorageIni::GetUIntValue(key, result);
    val    = (result != 0);

    mLock.unlock();

    return retval;
}

CHIP_ERROR ChipLinuxStorage::ReadValue(const char * key, uint16_t & val)
{
    CHIP_ERROR retval = CHIP_NO_ERROR;

    mLock.lock();

    retval = ChipLinuxStorageIni::GetUInt16Value(key, val);

    mLock.unlock();

    return retval;
}

CHIP_ERROR ChipLinuxStorage::ReadValue(const char * key, uint32_t & val)
{
    CHIP_ERROR retval = CHIP_NO_ERROR;

    mLock.lock();

    retval = ChipLinuxStorageIni::GetUIntValue(key, val);

    mLock.unlock();

    return retval;
}

CHIP_ERROR ChipLinuxStorage::ReadValue(const char * key, uint64_t & val)
{
    CHIP_ERROR retval = CHIP_NO_ERROR;

    mLock.lock();

    retval = ChipLinuxStorageIni::GetUInt64Value(key, val);

    mLock.unlock();

    return retval;
}

CHIP_ERROR ChipLinuxStorage::ReadValueStr(const char * key, char * buf, size_t bufSize, size_t & outLen)
{
    CHIP_ERROR retval = CHIP_NO_ERROR;

    mLock.lock();

    retval = ChipLinuxStorageIni::GetStringValue(key, buf, bufSize, outLen);

    mLock.unlock();

    return retval;
}

CHIP_ERROR ChipLinuxStorage::ReadValueBin(const char * key, uint8_t * buf, size_t bufSize, size_t & outLen)
{
    CHIP_ERROR retval = CHIP_NO_ERROR;

    mLock.lock();

    retval = ChipLinuxStorageIni::GetBinaryBlobValue(key, buf, bufSize, outLen);

    mLock.unlock();

    return retval;
}

CHIP_ERROR ChipLinuxStorage::WriteValue(const char * key, bool val)
{
    CHIP_ERROR retval = CHIP_NO_ERROR;

    if (val)
    {
        retval = WriteValue(key, static_cast<uint32_t>(1));
    }
    else
    {
        retval = WriteValue(key, static_cast<uint32_t>(0));
    }

    return retval;
}

CHIP_ERROR ChipLinuxStorage::WriteValue(const char * key, uint16_t val)
{
    char buf[16];

    snprintf(buf, sizeof(buf), "%u", val);

    return WriteValueStr(key, buf);
}

CHIP_ERROR ChipLinuxStorage::WriteValue(const char * key, uint32_t val)
{
    char buf[32];

    snprintf(buf, sizeof(buf), "%d", val);

    return WriteValueStr(key, buf);
}

CHIP_ERROR ChipLinuxStorage::WriteValue(const char * key, uint64_t val)
{
    char buf[64];

    snprintf(buf, sizeof(buf), "%" PRIu64, val);

    return WriteValueStr(key, buf);
}

CHIP_ERROR ChipLinuxStorage::WriteValueStr(const char * key, const char * val)
{
    CHIP_ERROR retval = CHIP_NO_ERROR;

    mLock.lock();

    retval = ChipLinuxStorageIni::AddEntry(key, val);

    mDirty = true;

    mLock.unlock();

    return retval;
}

CHIP_ERROR ChipLinuxStorage::WriteValueBin(const char * key, const uint8_t * data, size_t dataLen)
{
    static const size_t kMaxBlobSize = 10 * 1024;

    CHIP_ERROR retval = CHIP_NO_ERROR;
    chip::Platform::ScopedMemoryBuffer<char> encodedData;
    size_t encodedDataLen     = 0;
    size_t expectedEncodedLen = ((dataLen + 3) * 4) / 3;

    // We only support encoding blobs up to 10kb
    if (dataLen > kMaxBlobSize)
    {
        retval = CHIP_ERROR_INVALID_ARGUMENT;
    }

    // Compute our expectedEncodedLen
    // Allocate just enough space for the encoded data, and the NULL terminator
    if (retval == CHIP_NO_ERROR)
    {
        if (!encodedData.Alloc(expectedEncodedLen + 1))
        {
            retval = CHIP_ERROR_NO_MEMORY;
        }
    }

    // Encode it
    if (retval == CHIP_NO_ERROR)
    {
        // We tested above that dataLen is no more than kMaxBlobSize.
        static_assert(kMaxBlobSize < UINT16_MAX, "dataLen won't fit");
        encodedDataLen              = Base64Encode(data, static_cast<uint16_t>(dataLen), encodedData.Get());
        encodedData[encodedDataLen] = 0;
    }

    // Store it
    if (retval == CHIP_NO_ERROR)
    {
        retval = WriteValueStr(key, encodedData.Get());
    }

    return retval;
}

CHIP_ERROR ChipLinuxStorage::ClearValue(const char * key)
{
    CHIP_ERROR retval = CHIP_NO_ERROR;

    mLock.lock();

    retval = ChipLinuxStorageIni::RemoveEntry(key);

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

CHIP_ERROR ChipLinuxStorage::ClearAll()
{
    CHIP_ERROR retval = CHIP_NO_ERROR;

    mLock.lock();

    retval = ChipLinuxStorageIni::RemoveAll();

    mLock.unlock();

    if (retval == CHIP_NO_ERROR)
    {
        mDirty = true;
        retval = Commit();
    }
    else
    {
        retval = CHIP_ERROR_WRITE_FAILED;
    }

    return retval;
}

bool ChipLinuxStorage::HasValue(const char * key)
{
    bool retval;

    mLock.lock();

    retval = ChipLinuxStorageIni::HasValue(key);

    mLock.unlock();

    return retval;
}

CHIP_ERROR ChipLinuxStorage::Commit()
{
    CHIP_ERROR retval = CHIP_NO_ERROR;

    if (mDirty && !mConfigPath.empty())
    {
        mLock.lock();

        retval = ChipLinuxStorageIni::CommitConfig(mConfigPath);

        mLock.unlock();
    }
    else
    {
        retval = CHIP_ERROR_WRITE_FAILED;
    }

    return retval;
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
