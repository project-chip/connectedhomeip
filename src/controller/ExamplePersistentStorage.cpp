/*
 *   Copyright (c) 2020-2022 Project CHIP Authors
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
#include "ExamplePersistentStorage.h"

#include <lib/core/CHIPEncoding.h>
#include <lib/support/IniEscaping.h>

#include <fstream>
#include <map>
#include <memory>
#include <string>

using String   = std::basic_string<char>;
using Section  = std::map<String, String>;
using Sections = std::map<String, Section>;

using namespace ::chip;
using namespace ::chip::Controller;
using namespace ::chip::IniEscaping;
using namespace ::chip::Logging;

constexpr char kDefaultSectionName[]       = "Default";
constexpr char kPortKey[]                  = "ListenPort";
constexpr char kLoggingKey[]               = "LoggingLevel";
constexpr char kLocalNodeIdKey[]           = "LocalNodeId";
constexpr char kCommissionerCATsKey[]      = "CommissionerCATs";
constexpr LogCategory kDefaultLoggingLevel = kLogCategory_Automation;

const char * GetUsedDirectory(const char * directory)
{
    const char * dir = directory;

    if (dir == nullptr)
    {
        dir = getenv("TMPDIR");
    }

    if (dir == nullptr)
    {
        dir = "/tmp";
    }

    return dir;
}

std::string GetFilename(const char * directory, const char * name)
{
    const char * dir = GetUsedDirectory(directory);

    if (name == nullptr)
    {
        return std::string(dir) + "/chip_tool_config.ini";
    }

    return std::string(dir) + "/chip_tool_config." + std::string(name) + ".ini";
}

CHIP_ERROR PersistentStorage::Init(const char * name, const char * directory)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    std::ifstream ifs;
    ifs.open(GetFilename(directory, name), std::ifstream::in);
    if (!ifs.good())
    {
        CommitConfig(directory, name);
        ifs.open(GetFilename(directory, name), std::ifstream::in);
    }
    VerifyOrExit(ifs.is_open(), err = CHIP_ERROR_OPEN_FAILED);

    mName      = name;
    mDirectory = directory;
    mConfig.clear();
    mConfig.parse(ifs);
    ifs.close();

    // To audit the contents at init, uncomment the following:
#if 0
    DumpKeys();
#endif

exit:
    return err;
}

CHIP_ERROR PersistentStorage::SyncGetKeyValue(const char * key, void * value, uint16_t & size)
{
    std::string iniValue;

    ReturnErrorCodeIf(((value == nullptr) && (size != 0)), CHIP_ERROR_INVALID_ARGUMENT);

    auto section = mConfig.sections[kDefaultSectionName];

    ReturnErrorCodeIf(!SyncDoesKeyExist(key), CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND);

    std::string escapedKey = EscapeKey(key);
    ReturnErrorCodeIf(!inipp::extract(section[escapedKey], iniValue), CHIP_ERROR_INVALID_ARGUMENT);

    iniValue = Base64ToString(iniValue);

    uint16_t dataSize = static_cast<uint16_t>(iniValue.size());
    ReturnErrorCodeIf(size == 0 && dataSize == 0, CHIP_NO_ERROR);
    ReturnErrorCodeIf(value == nullptr, CHIP_ERROR_BUFFER_TOO_SMALL);

    uint16_t sizeToCopy = std::min(size, dataSize);

    memcpy(value, iniValue.data(), sizeToCopy);
    size = sizeToCopy;
    return size < dataSize ? CHIP_ERROR_BUFFER_TOO_SMALL : CHIP_NO_ERROR;
}

CHIP_ERROR PersistentStorage::SyncSetKeyValue(const char * key, const void * value, uint16_t size)
{
    ReturnErrorCodeIf((value == nullptr) && (size != 0), CHIP_ERROR_INVALID_ARGUMENT);

    auto section = mConfig.sections[kDefaultSectionName];

    std::string escapedKey = EscapeKey(key);
    if (value == nullptr)
    {
        section[escapedKey] = "";
    }
    else
    {
        section[escapedKey] = StringToBase64(std::string(static_cast<const char *>(value), size));
    }

    mConfig.sections[kDefaultSectionName] = section;
    return CommitConfig(mDirectory, mName);
}

CHIP_ERROR PersistentStorage::SyncDeleteKeyValue(const char * key)
{
    auto section = mConfig.sections[kDefaultSectionName];

    ReturnErrorCodeIf(!SyncDoesKeyExist(key), CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND);

    std::string escapedKey = EscapeKey(key);
    section.erase(escapedKey);

    mConfig.sections[kDefaultSectionName] = section;
    return CommitConfig(mDirectory, mName);
}

bool PersistentStorage::SyncDoesKeyExist(const char * key)
{
    std::string escapedKey = EscapeKey(key);
    auto section           = mConfig.sections[kDefaultSectionName];
    auto it                = section.find(escapedKey);
    return (it != section.end());
}

void PersistentStorage::DumpKeys() const
{
#if CHIP_PROGRESS_LOGGING
    for (const auto & section : mConfig.sections)
    {
        const std::string & sectionName = section.first;
        const auto & sectionContent     = section.second;

        ChipLogProgress(chipTool, "[%s]", sectionName.c_str());
        for (const auto & entry : sectionContent)
        {
            const std::string & keyName = entry.first;
            ChipLogProgress(chipTool, "  => %s", UnescapeKey(keyName).c_str());
        }
    }
#endif // CHIP_PROGRESS_LOGGING
}

CHIP_ERROR PersistentStorage::SyncClearAll()
{
    ChipLogProgress(chipTool, "Clearing %s storage", kDefaultSectionName);
    auto section = mConfig.sections[kDefaultSectionName];
    section.clear();
    mConfig.sections[kDefaultSectionName] = section;
    return CommitConfig(mDirectory, mName);
}

const char * PersistentStorage::GetDirectory() const
{
    return GetUsedDirectory(mDirectory);
}

CHIP_ERROR PersistentStorage::CommitConfig(const char * directory, const char * name)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    std::ofstream ofs;
    std::string tmpPath = GetFilename(directory, name) + ".tmp";
    ofs.open(tmpPath, std::ofstream::out | std::ofstream::trunc);
    VerifyOrExit(ofs.good(), err = CHIP_ERROR_WRITE_FAILED);

    mConfig.generate(ofs);
    ofs.close();
    VerifyOrExit(ofs.good(), err = CHIP_ERROR_WRITE_FAILED);

    VerifyOrExit(rename(tmpPath.c_str(), GetFilename(directory, name).c_str()) == 0, err = CHIP_ERROR_WRITE_FAILED);

exit:
    return err;
}

uint16_t PersistentStorage::GetListenPort()
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    // By default chip-tool listens on an ephemeral port.
    uint16_t chipListenPort = 0;

    char value[6];
    uint16_t size = static_cast<uint16_t>(sizeof(value));
    err           = SyncGetKeyValue(kPortKey, value, size);
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
    err           = SyncGetKeyValue(kLoggingKey, value, size);
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
        else if (strcasecmp(value, "automation") == 0)
        {
            chipLogLevel = kLogCategory_Automation;
        }
    }

    return chipLogLevel;
}

NodeId PersistentStorage::GetLocalNodeId()
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    uint64_t nodeId;
    uint16_t size = static_cast<uint16_t>(sizeof(nodeId));
    err           = SyncGetKeyValue(kLocalNodeIdKey, &nodeId, size);
    if (err == CHIP_NO_ERROR)
    {
        return static_cast<NodeId>(Encoding::LittleEndian::HostSwap64(nodeId));
    }

    return kTestControllerNodeId;
}

CHIP_ERROR PersistentStorage::SetLocalNodeId(NodeId value)
{
    uint64_t nodeId = Encoding::LittleEndian::HostSwap64(value);
    return SyncSetKeyValue(kLocalNodeIdKey, &nodeId, sizeof(nodeId));
}

CATValues PersistentStorage::GetCommissionerCATs()
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    CATValues cats;
    chip::CATValues::Serialized serializedCATs;
    uint16_t size = chip::CATValues::kSerializedLength;
    err           = SyncGetKeyValue(kCommissionerCATsKey, serializedCATs, size);
    if (err == CHIP_NO_ERROR && size == chip::CATValues::kSerializedLength)
    {
        err = cats.Deserialize(serializedCATs);
        if (err == CHIP_NO_ERROR)
        {
            return cats;
        }
    }
    return chip::kUndefinedCATs;
}

CHIP_ERROR PersistentStorage::SetCommissionerCATs(const CATValues & cats)
{
    chip::CATValues::Serialized serializedCATs;
    ReturnErrorOnFailure(cats.Serialize(serializedCATs));

    return SyncSetKeyValue(kCommissionerCATsKey, serializedCATs, sizeof(serializedCATs));
}
