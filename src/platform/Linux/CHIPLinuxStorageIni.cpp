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
 *          using IniPP on Linux platform.
 *
 */

#include <fstream>
#include <string>
#include <unistd.h>

#include <platform/Linux/CHIPLinuxStorageIni.h>
#include <platform/internal/CHIPDeviceLayerInternal.h>
#include <support/Base64.h>
#include <support/CHIPMem.h>
#include <support/CodeUtils.h>
#include <support/logging/CHIPLogging.h>

namespace chip {
namespace DeviceLayer {
namespace Internal {

CHIP_ERROR ChipLinuxStorageIni::Init(void)
{
    return RemoveAll();
}

CHIP_ERROR ChipLinuxStorageIni::GetDefaultSection(std::map<std::string, std::string> & section)
{
    CHIP_ERROR retval = CHIP_NO_ERROR;

    auto it = mConfigStore.sections.find("DEFAULT");

    if (it != mConfigStore.sections.end())
    {
        section = mConfigStore.sections["DEFAULT"];
    }
    else
    {
        retval = CHIP_ERROR_KEY_NOT_FOUND;
    }

    return retval;
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
    CHIP_ERROR retval = CHIP_NO_ERROR;
    std::ofstream ofs;
    std::string tmpPath = configFile;

    tmpPath.append(".tmp");

    ofs.open(tmpPath, std::ofstream::out | std::ofstream::trunc);

    if (ofs.is_open())
    {
        ChipLogProgress(DeviceLayer, "writing settings to file (%s)", tmpPath.c_str());

        mConfigStore.generate(ofs);
        ofs.close();

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

CHIP_ERROR ChipLinuxStorageIni::GetUIntValue(const char * key, uint32_t & val)
{
    CHIP_ERROR retval = CHIP_NO_ERROR;
    std::map<std::string, std::string> section;

    retval = GetDefaultSection(section);

    if (retval == CHIP_NO_ERROR)
    {
        auto it = section.find(key);

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
    std::map<std::string, std::string> section;

    retval = GetDefaultSection(section);

    if (retval == CHIP_NO_ERROR)
    {
        auto it = section.find(key);

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
    std::map<std::string, std::string> section;

    retval = GetDefaultSection(section);

    if (retval == CHIP_NO_ERROR)
    {
        auto it = section.find(key);

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

CHIP_ERROR ChipLinuxStorageIni::GetBinaryBlobDataAndLengths(const char * key, char *& encodedData, size_t & encodedDataLen,
                                                            size_t & decodedDataLen)
{
    CHIP_ERROR retval            = CHIP_NO_ERROR;
    size_t encodedDataPaddingLen = 0;
    std::map<std::string, std::string> section;
    retval = GetDefaultSection(section);

    if (retval == CHIP_NO_ERROR)
    {
        auto it = section.find(key);

        if (it != section.end())
        {
            std::string value;

            // Compute the expectedDecodedLen
            if (inipp::extract(section[key], value))
            {
                size_t len = value.size();

                encodedData                 = (char *) chip::Platform::MemoryAlloc(len + 1);
                encodedDataLen              = value.copy(encodedData, len);
                encodedData[encodedDataLen] = '\0';

                // Check if encoded data was padded. Only "=" or "==" padding combinations are allowed.
                if ((encodedDataLen > 0) && (encodedData[encodedDataLen - 1] == '='))
                {
                    encodedDataPaddingLen++;
                    if ((encodedDataLen > 1) && (encodedData[encodedDataLen - 2] == '='))
                        encodedDataPaddingLen++;
                }

                decodedDataLen = ((encodedDataLen - encodedDataPaddingLen) * 3) / 4;
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

CHIP_ERROR ChipLinuxStorageIni::GetBinaryBlobValue(const char * key, uint8_t * decodedData, size_t bufSize, size_t & decodedDataLen)
{
    CHIP_ERROR retval  = CHIP_NO_ERROR;
    char * encodedData = NULL;
    size_t encodedDataLen;
    size_t expectedDecodedLen = 0;

    retval = GetBinaryBlobDataAndLengths(key, encodedData, encodedDataLen, expectedDecodedLen);

    // Check the size
    if (retval == CHIP_NO_ERROR)
    {
        if (expectedDecodedLen > bufSize)
        {
            decodedDataLen = expectedDecodedLen;
            retval         = CHIP_ERROR_BUFFER_TOO_SMALL;
        }
    }

    // Decode it
    if (retval == CHIP_NO_ERROR)
    {
        decodedDataLen = Base64Decode(encodedData, encodedDataLen, (uint8_t *) decodedData);
        if (decodedDataLen == UINT16_MAX || decodedDataLen > expectedDecodedLen)
        {
            retval = CHIP_ERROR_DECODE_FAILED;
        }

        if (encodedData)
        {
            chip::Platform::MemoryFree(encodedData);
        }
    }

    return retval;
}

bool ChipLinuxStorageIni::HasValue(const char * key)
{
    std::map<std::string, std::string> section;

    if (GetDefaultSection(section) != CHIP_NO_ERROR)
        return false;

    auto it = section.find(key);

    if (it != section.end())
    {
        return true;
    }
    else
    {
        return false;
    }
}

CHIP_ERROR ChipLinuxStorageIni::AddEntry(const char * key, const char * value)
{
    CHIP_ERROR retval = CHIP_NO_ERROR;

    if ((key != NULL) && (value != NULL))
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
