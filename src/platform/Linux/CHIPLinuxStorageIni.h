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
 *          Provides an implementation of the Configuration key-value store interface
 *          using IniPP.
 *
 */

#pragma once

#include <inipp/inipp.h>
#include <lib/support/ScopedBuffer.h>
#include <platform/PersistedStorage.h>

#include <map>
#include <mutex>
#include <string>

namespace chip {
namespace DeviceLayer {
namespace Internal {

class ChipLinuxStorageIni
{
public:
    CHIP_ERROR Init();
    CHIP_ERROR AddConfig(const std::string & configFile);
    CHIP_ERROR CommitConfig(const std::string & configFile);
    CHIP_ERROR GetUInt16Value(const char * key, uint16_t & val);
    CHIP_ERROR GetUIntValue(const char * key, uint32_t & val);
    CHIP_ERROR GetUInt64Value(const char * key, uint64_t & val);
    CHIP_ERROR GetStringValue(const char * key, char * buf, size_t bufSize, size_t & outLen);
    CHIP_ERROR GetBinaryBlobValue(const char * key, uint8_t * decodedData, size_t bufSize, size_t & decodedDataLen);
    bool HasValue(const char * key);

protected:
    CHIP_ERROR AddEntry(const char * key, const char * value);
    CHIP_ERROR RemoveEntry(const char * key);
    CHIP_ERROR RemoveAll();

private:
    CHIP_ERROR GetDefaultSection(std::map<std::string, std::string> & section);
    CHIP_ERROR GetBinaryBlobDataAndLengths(const char * key, chip::Platform::ScopedMemoryBuffer<char> & encodedData,
                                           size_t & encodedDataLen, size_t & decodedDataLen);
    std::mutex mLock;
    inipp::Ini<char> mConfigStore;
};

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
