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
#include "ChipDeviceController-StorageDelegate.h"

#include <fstream>

#include <lib/support/CodeUtils.h>

// We use std::basic_string>char> to match inipp::Ini<char> in class definiation
using String   = std::basic_string<char>;
using Section  = std::map<String, String>;
using Sections = std::map<String, Section>;

using namespace ::chip;
using namespace ::chip::Controller;

constexpr const char kFilename[]           = "/tmp/chip_tool_config.ini";
constexpr const char kDefaultSectionName[] = "Default";

namespace chip {
namespace Controller {

CHIP_ERROR PythonPersistentStorageDelegate::Init()
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

void PythonPersistentStorageDelegate::SetStorageDelegate(PersistentStorageResultDelegate * delegate) {}

CHIP_ERROR PythonPersistentStorageDelegate::SyncGetKeyValue(const char * key, char * value, uint16_t & size)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    std::string iniValue;
    size_t iniValueLength = 0;

    const auto & section = mConfig.sections[kDefaultSectionName];
    auto it              = section.find(key);
    VerifyOrExit(it != section.end(), err = CHIP_ERROR_KEY_NOT_FOUND);

    VerifyOrExit(inipp::extract(it->second, iniValue), err = CHIP_ERROR_INVALID_ARGUMENT);

    iniValueLength = iniValue.size();
    VerifyOrExit(iniValueLength <= static_cast<size_t>(size) - 1, err = CHIP_ERROR_BUFFER_TOO_SMALL);

    iniValueLength        = iniValue.copy(value, iniValueLength);
    value[iniValueLength] = '\0';

exit:
    return err;
}

void PythonPersistentStorageDelegate::AsyncSetKeyValue(const char * key, const char * value)
{
    mConfig.sections[kDefaultSectionName][key] = std::string(value);
    CommitConfig();
}

void PythonPersistentStorageDelegate::AsyncDeleteKeyValue(const char * key)
{
    mConfig.sections[kDefaultSectionName].erase(key);
    CommitConfig();
}

CHIP_ERROR PythonPersistentStorageDelegate::CommitConfig()
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

} // namespace Controller
} // namespace chip
