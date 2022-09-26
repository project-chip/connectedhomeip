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

#include "PICSBooleanReader.h"
#include "PICSNormalizer.h"

#include <lib/support/CodeUtils.h>

#include <fstream>
#include <sstream>

std::map<std::string, bool> PICSBooleanReader::Read(std::string filepath)
{
    std::ifstream f(filepath);
    VerifyOrDieWithMsg(f.is_open(), chipTool, "Error reading: %s", filepath.c_str());

    std::map<std::string, bool> PICS;
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

        std::stringstream ss(line);

        std::getline(ss, key, '=');
        VerifyOrDieWithMsg(!key.empty(), chipTool, "Missing PICS key at line %u", lineNumber + 1);
        key = PICSNormalizer::Normalize(key);

        std::getline(ss, value);
        if (value == "0")
        {
            PICS[key] = false;
        }
        else if (value == "1")
        {
            PICS[key] = true;
        }
        else
        {
            ChipLogError(chipTool, "%s: PICS value should be either '0' or '1', got '%s'", key.c_str(), value.c_str());
            chipDie();
        }

        lineNumber++;
    }

    return PICS;
}
