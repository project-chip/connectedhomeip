/**
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
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
