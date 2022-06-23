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
 *          Provides an implementation of the Configuration key-value store object
 *          using IniPP on webOS platform.
 *
 */

#include <fstream>
#include <string>
#include <unistd.h>

#include <lib/support/Base64.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/internal/CHIPDeviceLayerInternal.h>
#include <platform/webos/CHIPWebOSStorageIni.h>

namespace chip {
namespace DeviceLayer {
namespace Internal {

CHIP_ERROR ChipLinuxStorageIni::Init()
{
    return RemoveAll();
}

CHIP_ERROR ChipLinuxStorageIni::AddConfig(const std::string & configFile)
{
    CHIP_ERROR retval = CHIP_NO_ERROR;
    std::ifstream ifs;

    ifs.open(configFile, std::ifstream::in);

    if (ifs.is_open())
    {
        mConfigStore.parse(ifs);
        ifs.close();
    }
    else
    {
        ChipLogError(DeviceLayer, "Failed to open config file: %s", configFile.c_str());
        retval = CHIP_ERROR_OPEN_FAILED;
    }

    return retval;
}

// Updating a file atomically and durably on Linux requires:
// 1. Writing to a temporary file
// 2. Sync'ing the temp file to commit updated data
// 3. Using rename() to overwrite the existing file
CHIP_ERROR ChipLinuxStorageIni::CommitConfig(const std::string & configFile)
{
    CHIP_ERROR retval   = CHIP_NO_ERROR;
    std::string tmpPath = configFile + "-XXXXXX";

    int fd = mkstemp(&tmpPath[0]);
    if (fd != -1)
    {
        std::ofstream ofs;

        ChipLogProgress(DeviceLayer, "writing settings to file (%s)", tmpPath.c_str());

        ofs.open(tmpPath, std::ofstream::out | std::ofstream::trunc);
        mConfigStore.generate(ofs);

        close(fd);

        if (rename(tmpPath.c_str(), configFile.c_str()) == 0)
        {
            ChipLogError(DeviceLayer, "renamed tmp file to file (%s)", configFile.c_str());
        }
        else
        {
            ChipLogError(DeviceLayer, "failed to rename (%s), %s (%d)", tmpPath.c_str(), strerror(errno), errno);
            retval = CHIP_ERROR_WRITE_FAILED;
        }
    }
    else
    {
        ChipLogError(DeviceLayer, "failed to open file (%s) for writing", tmpPath.c_str());
        retval = CHIP_ERROR_OPEN_FAILED;
    }

    return retval;
}

CHIP_ERROR ChipLinuxStorageIni::GetUInt16Value(const char * key, uint16_t & val)
{
    CHIP_ERROR retval = CHIP_NO_ERROR;

    auto isConfigFind = mConfigStore.sections.find("DEFAULT");

    if (isConfigFind != mConfigStore.sections.end())
    {
        std::map<std::string, std::string> & section = mConfigStore.sections["DEFAULT"];
        auto it                                      = section.find(key);

        if (it != section.end())
        {
            if (!inipp::extract(section[key], val))
            {
                retval = CHIP_ERROR_INVALID_ARGUMENT;
            }
        }
        else
        {
            retval = CHIP_ERROR_KEY_NOT_FOUND;
        }
    }

    return retval;
}

CHIP_ERROR ChipLinuxStorageIni::GetUIntValue(const char * key, uint32_t & val)
{
    CHIP_ERROR retval = CHIP_NO_ERROR;
    auto isConfigFind = mConfigStore.sections.find("DEFAULT");

    if (isConfigFind != mConfigStore.sections.end())
    {
        std::map<std::string, std::string> & section = mConfigStore.sections["DEFAULT"];
        auto it                                      = section.find(key);

        if (it != section.end())
        {
            if (!inipp::extract(section[key], val))
            {
                retval = CHIP_ERROR_INVALID_ARGUMENT;
            }
        }
        else
        {
            retval = CHIP_ERROR_KEY_NOT_FOUND;
        }
    }

    return retval;
}

CHIP_ERROR ChipLinuxStorageIni::GetUInt64Value(const char * key, uint64_t & val)
{
    CHIP_ERROR retval = CHIP_NO_ERROR;
    auto isConfigFind = mConfigStore.sections.find("DEFAULT");

    if (isConfigFind != mConfigStore.sections.end())
    {
        std::map<std::string, std::string> & section = mConfigStore.sections["DEFAULT"];
        auto it                                      = section.find(key);

        if (it != section.end())
        {
            if (!inipp::extract(section[key], val))
            {
                retval = CHIP_ERROR_INVALID_ARGUMENT;
            }
        }
        else
        {
            retval = CHIP_ERROR_KEY_NOT_FOUND;
        }
    }

    return retval;
}

CHIP_ERROR ChipLinuxStorageIni::GetStringValue(const char * key, char * buf, size_t bufSize, size_t & outLen)
{
    CHIP_ERROR retval = CHIP_NO_ERROR;
    auto isConfigFind = mConfigStore.sections.find("DEFAULT");

    if (isConfigFind != mConfigStore.sections.end())
    {
        std::map<std::string, std::string> & section = mConfigStore.sections["DEFAULT"];
        auto it                                      = section.find(key);

        if (it != section.end())
        {
            std::string value;
            if (inipp::extract(section[key], value))
            {
                size_t len = value.size();

                if (len > bufSize - 1)
                {
                    outLen = len;
                    retval = CHIP_ERROR_BUFFER_TOO_SMALL;
                }
                else
                {
                    outLen      = value.copy(buf, len);
                    buf[outLen] = '\0';
                }
            }
            else
            {
                retval = CHIP_ERROR_INVALID_ARGUMENT;
            }
        }
        else
        {
            retval = CHIP_ERROR_KEY_NOT_FOUND;
        }
    }

    return retval;
}

CHIP_ERROR ChipLinuxStorageIni::GetBinaryBlobDataAndLengths(const char * key,
                                                            chip::Platform::ScopedMemoryBuffer<char> & encodedData,
                                                            size_t & encodedDataLen, size_t & decodedDataLen)
{
    size_t encodedDataPaddingLen = 0;
    auto isConfigFind            = mConfigStore.sections.find("DEFAULT");

    if (isConfigFind == mConfigStore.sections.end())
    {
        return CHIP_ERROR_KEY_NOT_FOUND;
    }
    std::map<std::string, std::string> & section = mConfigStore.sections["DEFAULT"];

    auto it = section.find(key);
    if (it == section.end())
    {
        return CHIP_ERROR_KEY_NOT_FOUND;
    }

    std::string value;

    // Compute the expectedDecodedLen
    if (!inipp::extract(section[key], value))
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    size_t len = value.size();
    if (!encodedData.Alloc(len + 1))
    {
        return CHIP_ERROR_NO_MEMORY;
    }
    encodedDataLen              = value.copy(encodedData.Get(), len);
    encodedData[encodedDataLen] = '\0';

    // Check if encoded data was padded. Only "=" or "==" padding combinations are allowed.
    if ((encodedDataLen > 0) && (encodedData[encodedDataLen - 1] == '='))
    {
        encodedDataPaddingLen++;
        if ((encodedDataLen > 1) && (encodedData[encodedDataLen - 2] == '='))
            encodedDataPaddingLen++;
    }

    decodedDataLen = ((encodedDataLen - encodedDataPaddingLen) * 3) / 4;

    return CHIP_NO_ERROR;
}

CHIP_ERROR ChipLinuxStorageIni::GetBinaryBlobValue(const char * key, uint8_t * decodedData, size_t bufSize, size_t & decodedDataLen)
{
    CHIP_ERROR retval = CHIP_NO_ERROR;
    chip::Platform::ScopedMemoryBuffer<char> encodedData;
    size_t encodedDataLen;
    size_t expectedDecodedLen = 0;

    retval = GetBinaryBlobDataAndLengths(key, encodedData, encodedDataLen, expectedDecodedLen);

    // Check the size
    if (retval != CHIP_NO_ERROR)
    {
        return retval;
    }

    if (expectedDecodedLen > bufSize)
    {
        decodedDataLen = expectedDecodedLen;
        return CHIP_ERROR_BUFFER_TOO_SMALL;
    }

    if (encodedDataLen > UINT16_MAX)
    {
        // We can't even pass this length into Base64Decode.
        return CHIP_ERROR_DECODE_FAILED;
    }

    // Decode it
    // Cast is safe because we checked encodedDataLen above.
    decodedDataLen = Base64Decode(encodedData.Get(), static_cast<uint16_t>(encodedDataLen), decodedData);
    if (decodedDataLen == UINT16_MAX || decodedDataLen > expectedDecodedLen)
    {
        return CHIP_ERROR_DECODE_FAILED;
    }

    return CHIP_NO_ERROR;
}

bool ChipLinuxStorageIni::HasValue(const char * key)
{
    auto isConfigFind = mConfigStore.sections.find("DEFAULT");

    if (isConfigFind == mConfigStore.sections.end())
        return false;

    std::map<std::string, std::string> & section = mConfigStore.sections["DEFAULT"];
    auto it                                      = section.find(key);

    return it != section.end();
}

CHIP_ERROR ChipLinuxStorageIni::AddEntry(const char * key, const char * value)
{
    CHIP_ERROR retval = CHIP_NO_ERROR;

    if ((key != nullptr) && (value != nullptr))
    {
        std::map<std::string, std::string> & section = mConfigStore.sections["DEFAULT"];
        section[key]                                 = std::string(value);
    }
    else
    {
        ChipLogError(DeviceLayer, "Invalid input argument, failed to add entry");
        retval = CHIP_ERROR_INVALID_ARGUMENT;
    }

    return retval;
}

CHIP_ERROR ChipLinuxStorageIni::RemoveEntry(const char * key)
{
    CHIP_ERROR retval = CHIP_NO_ERROR;

    std::map<std::string, std::string> & section = mConfigStore.sections["DEFAULT"];

    auto it = section.find(key);

    if (it != section.end())
    {
        section.erase(it);
    }
    else
    {
        retval = CHIP_ERROR_KEY_NOT_FOUND;
    }

    return retval;
}

CHIP_ERROR ChipLinuxStorageIni::RemoveAll()
{
    mConfigStore.clear();

    return CHIP_NO_ERROR;
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
