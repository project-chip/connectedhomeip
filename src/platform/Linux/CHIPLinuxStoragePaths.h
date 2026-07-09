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
 * @brief Structure holding KVS file paths for Linux platform
 *
 * This structure is populated from command-line arguments in examples/platform/linux/Options.cpp
 * and used by PosixConfig to initialize storage namespaces.
 */
struct ChipLinuxStoragePaths
{
    std::string kvsDataFile;  // Full path to KVS data file (default: /tmp/chip_kvs)
    std::string factoryFile;  // Full path to factory config file (default: /tmp/chip_factory.ini)
    std::string configFile;   // Full path to config file (default: /tmp/chip_config.ini)
    std::string countersFile; // Full path to counters file (default: /tmp/chip_counters.ini)

    // Legacy KVS file path (deprecated, use kvsDataFile instead)
    const char * legacyKvsFile = nullptr;
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
