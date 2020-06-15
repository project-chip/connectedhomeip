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
 *          Provides an implementation of the Configuration Storage object
 *          using IniPP on Linux platform.
 *
 */

#include <fstream>
#include <string>
#include <unistd.h>

#include <platform/Linux/CHIPLinuxConfig.h>
#include <platform/internal/CHIPDeviceLayerInternal.h>
#include <support/Base64.h>
#include <support/CodeUtils.h>
#include <support/logging/CHIPLogging.h>

namespace chip {
namespace DeviceLayer {
namespace Internal {

CHIP_ERROR ChipLinuxConfig::Init(void)
{
    return RemoveAll();
}

CHIP_ERROR ChipLinuxConfig::GetDefaultSession(std::map<std::string, std::string> & section)
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

CHIP_ERROR ChipLinuxConfig::AddConfig(const char * configFile)
{
    CHIP_ERROR retval = CHIP_NO_ERROR;
    std::ifstream ifs;

    ifs.open(configFile, std::ifstream::in);

    if (ifs.is_open())
    {
        mConfigStore.parse(ifs);
    }
    else
    {
        ChipLogError(DeviceLayer, "Failed to open config file: %s", configFile);
        retval = CHIP_ERROR_PERSISTED_STORAGE_FAIL;
    }

    return retval;
}

CHIP_ERROR ChipLinuxConfig::CommitConfig(const char * configFile)
{
    CHIP_ERROR retval = CHIP_NO_ERROR;
    std::ofstream ofs;

    ofs.open(configFile, std::ofstream::out | std::ofstream::trunc);

    if (ofs.is_open())
    {
        mConfigStore.generate(ofs);
    }
    else
    {
        ChipLogError(DeviceLayer, "Failed to open config file: %s", configFile);
        retval = CHIP_ERROR_PERSISTED_STORAGE_FAIL;
    }

    return retval;
}

CHIP_ERROR ChipLinuxConfig::GetUIntValue(const char * key, uint32_t & val)
{
    CHIP_ERROR retval = CHIP_NO_ERROR;
    std::map<std::string, std::string> section;

    retval = GetDefaultSession(section);

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

CHIP_ERROR ChipLinuxConfig::GetUInt64Value(const char * key, uint64_t & val)
{
    CHIP_ERROR retval = CHIP_NO_ERROR;
    std::map<std::string, std::string> section;

    retval = GetDefaultSession(section);

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

CHIP_ERROR ChipLinuxConfig::GetBinaryBlobDataAndLengths(const char * key, char *& encodedData, size_t & encodedDataLen,
                                                        size_t & decodedDataLen)
{
    CHIP_ERROR retval            = CHIP_NO_ERROR;
    size_t encodedDataPaddingLen = 0;
    std::map<std::string, std::string> section;
    retval = GetDefaultSession(section);

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

                encodedData                 = (char *) malloc(len + 1);
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

CHIP_ERROR ChipLinuxConfig::GetBinaryBlobValue(const char * key, uint8_t * decodedData, size_t bufSize, size_t & decodedDataLen)
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
            retval = CHIP_ERROR_BUFFER_TOO_SMALL;
        }
    }

    // Decode it
    if (retval == CHIP_NO_ERROR)
    {
        decodedDataLen = Base64Decode(encodedData, encodedDataLen, (uint8_t *) decodedData);
        if (decodedDataLen == UINT16_MAX || decodedDataLen > expectedDecodedLen)
        {
            retval = CHIP_ERROR_NOT_IMPLEMENTED;
        }

        if (encodedData)
        {
            free(encodedData);
        }
    }

    return retval;
}

bool ChipLinuxConfig::HasValue(const char * key)
{
    std::map<std::string, std::string> section;

    if (GetDefaultSession(section) != CHIP_NO_ERROR)
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

CHIP_ERROR ChipLinuxConfig::AddEntry(const char * key, const char * value)
{
    CHIP_ERROR retval = CHIP_NO_ERROR;

    if ((key != NULL) && (value != NULL))
    {
        std::map<std::string, std::string> & section = mConfigStore.sections["DEFAULT"];
        section[key] = std::string(value);
    }
    else
    {
        ChipLogError(DeviceLayer, "Invalid input argument, failed to add entry");
        retval = CHIP_ERROR_INVALID_ARGUMENT;
    }

    return retval;
}

CHIP_ERROR ChipLinuxConfig::RemoveEntry(const char * key)
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

CHIP_ERROR ChipLinuxConfig::RemoveAll()
{
    mConfigStore.clear();

    return CHIP_NO_ERROR;
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
