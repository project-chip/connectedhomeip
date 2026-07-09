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
 *         This file defines structures for managing KVS file paths on Linux platform.
 *
 */

#pragma once

#include <filesystem>
#include <optional>
#include <string>

#ifndef CHIP_DEFAULT_BASE_DIR
#define CHIP_DEFAULT_BASE_DIR "/tmp"
#endif

#ifndef CHIP_DEFAULT_DATA_FILENAME
#define CHIP_DEFAULT_DATA_FILENAME "chip_kvs"
#endif

#ifndef CHIP_DEFAULT_FACTORY_FILENAME
#define CHIP_DEFAULT_FACTORY_FILENAME "chip_factory.ini"
#endif

#ifndef CHIP_DEFAULT_CONFIG_FILENAME
#define CHIP_DEFAULT_CONFIG_FILENAME "chip_config.ini"
#endif

#ifndef CHIP_DEFAULT_COUNTERS_FILENAME
#define CHIP_DEFAULT_COUNTERS_FILENAME "chip_counters.ini"
#endif

namespace chip {
namespace DeviceLayer {

/**
 * @brief Class for managing KVS file paths on Linux platform
 *
 * This class encapsulates path management logic for KVS storage files,
 * providing default filename resolution when directory paths are provided.
 */
class ChipLinuxStoragePaths
{
public:
    ChipLinuxStoragePaths() = default;

    // Setter methods for configuration
    void SetKVSDataFile(const std::string & path);
    void SetLegacyKVSFile(const char * path);
    void SetFactoryFile(const std::string & path);
    void SetConfigFile(const std::string & path);
    void SetCountersFile(const std::string & path);

    // Getter methods - return full resolved paths
    std::string GetKVSDataFilePath() const;
    const char * GetLegacyKVSFile() const;
    std::string GetFactoryFilePath() const;
    std::string GetConfigFilePath() const;
    std::string GetCountersFilePath() const;

    // Check if legacy KVS file is set
    bool HasLegacyKVSFile() const;

private:
    // Helper function to resolve a path by appending default filename if needed
    static std::string ResolvePath(const std::string & path, const std::string & defaultFilename);

    std::string mKVSDataFile;
    std::string mLegacyKVSFile; // Empty string means not set (equivalent to nullptr)
    std::string mFactoryFile;
    std::string mConfigFile;
    std::string mCountersFile;
};

/**
 * @brief Get the ChipLinuxStoragePaths singleton
 *
 * @return ChipLinuxStoragePaths& Reference to the storage paths structure
 */
ChipLinuxStoragePaths & GetStoragePaths();

/**
 * @brief Set the ChipLinuxStoragePaths
 *
 * @param paths The storage paths to use
 */
void SetStoragePaths(const ChipLinuxStoragePaths & paths);

} // namespace DeviceLayer
} // namespace chip
