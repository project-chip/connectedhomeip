/*
 *   Copyright (c) 2020 Project CHIP Authors
 *   All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */
#include "PersistentStorage.h"

#include <fstream>

using String   = std::basic_string<char>;
using Section  = std::map<String, String>;
using Sections = std::map<String, Section>;

using namespace ::chip;
using namespace ::chip::Controller;
using namespace ::chip::Logging;

constexpr const char kFilename[]           = "/tmp/chip_tool_config.ini";
constexpr const char kDefaultSectionName[] = "Default";
constexpr const char kPortKey[]            = "ListenPort";
constexpr const char kLoggingKey[]         = "LoggingLevel";
constexpr LogCategory kDefaultLoggingLevel = kLogCategory_Detail;

CHIP_ERROR PersistentStorage::Init()
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    std::ifstream ifs;
    ifs.open(kFilename, std::ifstream::in);
    if (!ifs.good())
    {
        CommitConfig();
        ifs.open(kFilename, std::ifstream::in);
    }
    VerifyOrExit(ifs.is_open(), err = CHIP_ERROR_OPEN_FAILED);

    mConfig.parse(ifs);
    ifs.close();

exit:
    return err;
}

void PersistentStorage::SetDelegate(PersistentStorageResultDelegate * delegate) {}

void PersistentStorage::GetKeyValue(const char * key) {}

CHIP_ERROR PersistentStorage::GetKeyValue(const char * key, char * value, uint16_t & size)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    std::string iniValue;
    size_t iniValueLength = 0;

    auto section = mConfig.sections[kDefaultSectionName];
    auto it      = section.find(key);
    VerifyOrExit(it != section.end(), err = CHIP_ERROR_KEY_NOT_FOUND);

    VerifyOrExit(inipp::extract(section[key], iniValue), err = CHIP_ERROR_INVALID_ARGUMENT);

    iniValueLength = iniValue.size();
    VerifyOrExit(iniValueLength <= static_cast<size_t>(size) - 1, err = CHIP_ERROR_BUFFER_TOO_SMALL);

    iniValueLength        = iniValue.copy(value, iniValueLength);
    value[iniValueLength] = '\0';

exit:
    return err;
}

void PersistentStorage::SetKeyValue(const char * key, const char * value)
{
    auto section = mConfig.sections[kDefaultSectionName];
    section[key] = std::string(value);

    mConfig.sections[kDefaultSectionName] = section;
    CommitConfig();
}

void PersistentStorage::DeleteKeyValue(const char * key)
{
    auto section = mConfig.sections[kDefaultSectionName];
    section.erase(key);

    mConfig.sections[kDefaultSectionName] = section;
    CommitConfig();
}

CHIP_ERROR PersistentStorage::CommitConfig()
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    std::ofstream ofs;
    std::string tmpPath = kFilename;
    tmpPath.append(".tmp");
    ofs.open(tmpPath, std::ofstream::out | std::ofstream::trunc);
    VerifyOrExit(ofs.good(), err = CHIP_ERROR_WRITE_FAILED);

    mConfig.generate(ofs);
    ofs.close();
    VerifyOrExit(ofs.good(), err = CHIP_ERROR_WRITE_FAILED);

    VerifyOrExit(rename(tmpPath.c_str(), kFilename) == 0, err = CHIP_ERROR_WRITE_FAILED);

exit:
    return err;
}

uint16_t PersistentStorage::GetListenPort()
{
    CHIP_ERROR err          = CHIP_NO_ERROR;
    uint16_t chipListenPort = CHIP_PORT;

    char value[6];
    uint16_t size = static_cast<uint16_t>(sizeof(value));
    err           = GetKeyValue(kPortKey, value, size);
    if (CHIP_NO_ERROR == err)
    {
        uint16_t tmpValue;
        std::stringstream ss(value);
        ss >> tmpValue;
        if (!ss.fail() && ss.eof())
        {
            chipListenPort = tmpValue;
        }
    }

    return chipListenPort;
}

LogCategory PersistentStorage::GetLoggingLevel()
{
    CHIP_ERROR err           = CHIP_NO_ERROR;
    LogCategory chipLogLevel = kDefaultLoggingLevel;

    char value[9];
    uint16_t size = static_cast<uint16_t>(sizeof(value));
    err           = GetKeyValue(kLoggingKey, value, size);
    if (CHIP_NO_ERROR == err)
    {
        if (strcasecmp(value, "none") == 0)
        {
            chipLogLevel = kLogCategory_None;
        }
        else if (strcasecmp(value, "error") == 0)
        {
            chipLogLevel = kLogCategory_Error;
        }
        else if (strcasecmp(value, "progress") == 0)
        {
            chipLogLevel = kLogCategory_Progress;
        }
        else if (strcasecmp(value, "detail") == 0)
        {
            chipLogLevel = kLogCategory_Detail;
        }
        else if (strcasecmp(value, "retain") == 0)
        {
            chipLogLevel = kLogCategory_Retain;
        }
    }

    return chipLogLevel;
}
