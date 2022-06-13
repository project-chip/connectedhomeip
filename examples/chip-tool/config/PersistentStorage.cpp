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
#include "PersistentStorage.h"

#include <lib/core/CHIPEncoding.h>
#include <lib/support/Base64.h>
#include <protocols/secure_channel/PASESession.h>

#include <fstream>
#include <memory>

using String   = std::basic_string<char>;
using Section  = std::map<String, String>;
using Sections = std::map<String, Section>;

using namespace ::chip;
using namespace ::chip::Controller;
using namespace ::chip::Logging;

constexpr const char kDefaultSectionName[]  = "Default";
constexpr const char kPortKey[]             = "ListenPort";
constexpr const char kLoggingKey[]          = "LoggingLevel";
constexpr const char kLocalNodeIdKey[]      = "LocalNodeId";
constexpr const char kCommissionerCATsKey[] = "CommissionerCATs";
constexpr LogCategory kDefaultLoggingLevel  = kLogCategory_Automation;

std::string GetFilename(const char * name)
{
    if (name == nullptr)
    {
        return "/tmp/chip_tool_config.ini";
    }
    return "/tmp/chip_tool_config." + std::string(name) + ".ini";
}

namespace {

std::string StringToBase64(const std::string & value)
{
    std::unique_ptr<char[]> buffer(new char[BASE64_ENCODED_LEN(value.length())]);

    uint32_t len =
        chip::Base64Encode32(reinterpret_cast<const uint8_t *>(value.data()), static_cast<uint32_t>(value.length()), buffer.get());
    if (len == UINT32_MAX)
    {
        return "";
    }

    return std::string(buffer.get(), len);
}

std::string Base64ToString(const std::string & b64Value)
{
    std::unique_ptr<uint8_t[]> buffer(new uint8_t[BASE64_MAX_DECODED_LEN(b64Value.length())]);

    uint32_t len = chip::Base64Decode32(b64Value.data(), static_cast<uint32_t>(b64Value.length()), buffer.get());
    if (len == UINT32_MAX)
    {
        return "";
    }

    return std::string(reinterpret_cast<const char *>(buffer.get()), len);
}

} // namespace

CHIP_ERROR PersistentStorage::Init(const char * name)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    std::ifstream ifs;
    ifs.open(GetFilename(name), std::ifstream::in);
    if (!ifs.good())
    {
        CommitConfig(name);
        ifs.open(GetFilename(name), std::ifstream::in);
    }
    VerifyOrExit(ifs.is_open(), err = CHIP_ERROR_OPEN_FAILED);

    mName = name;
    mConfig.parse(ifs);
    ifs.close();
exit:
    return err;
}

CHIP_ERROR PersistentStorage::SyncGetKeyValue(const char * key, void * value, uint16_t & size)
{
    std::string iniValue;

    auto section = mConfig.sections[kDefaultSectionName];
    auto it      = section.find(key);
    ReturnErrorCodeIf(it == section.end(), CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND);

    ReturnErrorCodeIf(!inipp::extract(section[key], iniValue), CHIP_ERROR_INVALID_ARGUMENT);

    iniValue = Base64ToString(iniValue);

    uint16_t dataSize = static_cast<uint16_t>(iniValue.size());
    if (dataSize > size)
    {
        size = dataSize;
        return CHIP_ERROR_BUFFER_TOO_SMALL;
    }

    size = dataSize;
    memcpy(value, iniValue.data(), dataSize);

    return CHIP_NO_ERROR;
}

CHIP_ERROR PersistentStorage::SyncSetKeyValue(const char * key, const void * value, uint16_t size)
{
    auto section = mConfig.sections[kDefaultSectionName];
    section[key] = StringToBase64(std::string(static_cast<const char *>(value), size));

    mConfig.sections[kDefaultSectionName] = section;
    return CommitConfig(mName);
}

CHIP_ERROR PersistentStorage::SyncDeleteKeyValue(const char * key)
{
    auto section = mConfig.sections[kDefaultSectionName];
    auto it      = section.find(key);
    ReturnErrorCodeIf(it == section.end(), CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND);

    section.erase(key);

    mConfig.sections[kDefaultSectionName] = section;
    return CommitConfig(mName);
}

CHIP_ERROR PersistentStorage::SyncClearAll()
{
    ChipLogProgress(chipTool, "Clearing %s storage", kDefaultSectionName);
    auto section = mConfig.sections[kDefaultSectionName];
    section.clear();
    mConfig.sections[kDefaultSectionName] = section;
    return CommitConfig(mName);
}

CHIP_ERROR PersistentStorage::CommitConfig(const char * name)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    std::ofstream ofs;
    std::string tmpPath = GetFilename(name) + ".tmp";
    ofs.open(tmpPath, std::ofstream::out | std::ofstream::trunc);
    VerifyOrExit(ofs.good(), err = CHIP_ERROR_WRITE_FAILED);

    mConfig.generate(ofs);
    ofs.close();
    VerifyOrExit(ofs.good(), err = CHIP_ERROR_WRITE_FAILED);

    VerifyOrExit(rename(tmpPath.c_str(), GetFilename(name).c_str()) == 0, err = CHIP_ERROR_WRITE_FAILED);

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
