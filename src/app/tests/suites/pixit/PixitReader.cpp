/**
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include "PixitReader.h"

#include <lib/support/CodeUtils.h>

#include <fstream>
#include <sstream>

namespace {
static inline void trim(std::string & str)
{
    size_t start = str.find_first_not_of(" \t");
    size_t end   = str.find_last_not_of(" \t");
    if (start != std::string::npos && end != std::string::npos && start <= end)
    {
        str = str.substr(start, end - start + 1);
    }
    else
    {
        str = "";
    }
}
} // namespace

CHIP_ERROR PixitReader::UpdateFromFile(std::string filepath, PixitMap & map)
{
    std::ifstream f(filepath);
    if (!f.is_open())
    {
        ChipLogError(chipTool, "Error reading: %s", filepath.c_str());
        return CHIP_ERROR_INVALID_FILE_IDENTIFIER;
    }
    std::string line;
    std::string key;
    std::string value;
    uint16_t lineNumber = 0;
    while (std::getline(f, line))
    {
        if (line.empty())
        {
            continue;
        }
        size_t start = line.find_first_not_of(" \t");
        if (line.at(start) == '#')
        {
            continue;
        }

        std::stringstream ss(line);

        std::getline(ss, key, '=');
        trim(key);
        if (key.empty())
        {
            ChipLogError(chipTool, "Missing PIXIT key at line %u", lineNumber + 1);
            return CHIP_ERROR_READ_FAILED;
        }
        std::transform(key.begin(), key.end(), key.begin(), [](unsigned char c) { return std::toupper(c); });
        if (!PixitTypeTransformer::Exists(key))
        {
            ChipLogError(chipTool, "PIXIT code %s not found in list of PIXIT types", key.c_str());
            return CHIP_ERROR_INVALID_ARGUMENT;
        }

        std::getline(ss, value);
        trim(value);
        if (value.empty())
        {
            ChipLogError(chipTool, "Missing PIXIT value at line %u", lineNumber + 1);
            return CHIP_ERROR_READ_FAILED;
        }

        map[key] = PixitTypeTransformer::TransformToType(key, value);

        lineNumber++;
    }

    return CHIP_NO_ERROR;
}

PixitReader::PixitMap PixitReader::PopulateDefaults()
{
    PixitMap map = {
        { "PIXIT.UNITTEST.UINT8.1", PixitTypeTransformer::PixitValueHolder::Create<uint8_t>(static_cast<uint8_t>(1)) },
        { "PIXIT.UNITTEST.UINT8.2", PixitTypeTransformer::PixitValueHolder::Create<uint8_t>(static_cast<uint8_t>(2)) },
        { "PIXIT.UNITTEST.UINT16.1", PixitTypeTransformer::PixitValueHolder::Create<uint16_t>(static_cast<uint16_t>(1)) },
        { "PIXIT.UNITTEST.UINT16.2", PixitTypeTransformer::PixitValueHolder::Create<uint16_t>(static_cast<uint16_t>(2)) },
        { "PIXIT.UNITTEST.UINT32.1", PixitTypeTransformer::PixitValueHolder::Create<uint32_t>(static_cast<uint32_t>(1)) },
        { "PIXIT.UNITTEST.UINT32.2", PixitTypeTransformer::PixitValueHolder::Create<uint32_t>(static_cast<uint32_t>(2)) },
        { "PIXIT.UNITTEST.UINT64.1", PixitTypeTransformer::PixitValueHolder::Create<uint64_t>(static_cast<uint64_t>(1)) },
        { "PIXIT.UNITTEST.UINT64.2", PixitTypeTransformer::PixitValueHolder::Create<uint64_t>(static_cast<uint64_t>(2)) },
        { "PIXIT.UNITTEST.INT8.1", PixitTypeTransformer::PixitValueHolder::Create<int8_t>(static_cast<int8_t>(1)) },
        { "PIXIT.UNITTEST.INT8.2", PixitTypeTransformer::PixitValueHolder::Create<int8_t>(static_cast<int8_t>(2)) },
        { "PIXIT.UNITTEST.INT16.1", PixitTypeTransformer::PixitValueHolder::Create<int16_t>(static_cast<int16_t>(1)) },
        { "PIXIT.UNITTEST.INT16.2", PixitTypeTransformer::PixitValueHolder::Create<int16_t>(static_cast<int16_t>(2)) },
        { "PIXIT.UNITTEST.INT32.1", PixitTypeTransformer::PixitValueHolder::Create<int32_t>(static_cast<int32_t>(1)) },
        { "PIXIT.UNITTEST.INT32.2", PixitTypeTransformer::PixitValueHolder::Create<int32_t>(static_cast<int32_t>(2)) },
        { "PIXIT.UNITTEST.INT64.1", PixitTypeTransformer::PixitValueHolder::Create<int64_t>(static_cast<int64_t>(1)) },
        { "PIXIT.UNITTEST.INT64.2", PixitTypeTransformer::PixitValueHolder::Create<int64_t>(static_cast<int64_t>(2)) },
        { "PIXIT.UNITTEST.STRING.1", PixitTypeTransformer::PixitValueHolder::Create<std::string>("str1") },
        { "PIXIT.UNITTEST.STRING.2", PixitTypeTransformer::PixitValueHolder::Create<std::string>("str2") },
        { "PIXIT.UNITTEST.SIZET.1", PixitTypeTransformer::PixitValueHolder::Create<size_t>(static_cast<size_t>(1)) },
        { "PIXIT.UNITTEST.SIZET.2", PixitTypeTransformer::PixitValueHolder::Create<size_t>(static_cast<size_t>(2)) },
        { "PIXIT.UNITTEST.BOOL.T", PixitTypeTransformer::PixitValueHolder::Create<bool>(true) },
        { "PIXIT.UNITTEST.BOOL.F", PixitTypeTransformer::PixitValueHolder::Create<bool>(false) },

    };

    return map;
}
