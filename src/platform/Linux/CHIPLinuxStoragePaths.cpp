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
 *         This file implements functions for managing KVS storage paths on Linux platform.
 *
 */

#include <platform/Linux/CHIPLinuxStoragePaths.h>

namespace chip {
namespace DeviceLayer {

namespace {
// Helper function to resolve a path by appending default filename if needed
std::string ResolvePath(const std::string & path, const std::string & defaultFilename)
{
    std::filesystem::path p(path.empty() ? CHIP_DEFAULT_BASE_DIR : path);
    std::string pathStr = p.string();
    bool isDirectory    = pathStr.empty() || pathStr.back() == '/';
    bool hasExtension   = !p.extension().empty();
    if (isDirectory || !hasExtension)
    {
        p /= defaultFilename;
    }
    return p.string();
}
} // namespace

std::string ChipLinuxStoragePaths::GetFactoryFilePath() const
{
    return ResolvePath(factoryFile, CHIP_DEFAULT_FACTORY_FILENAME);
}

std::string ChipLinuxStoragePaths::GetConfigFilePath() const
{
    return ResolvePath(configFile, CHIP_DEFAULT_CONFIG_FILENAME);
}

std::string ChipLinuxStoragePaths::GetCountersFilePath() const
{
    return ResolvePath(countersFile, CHIP_DEFAULT_COUNTERS_FILENAME);
}

ChipLinuxStoragePaths & GetStoragePaths()
{
    static ChipLinuxStoragePaths sInstance;
    return sInstance;
}

void SetStoragePaths(const ChipLinuxStoragePaths & paths)
{
    GetStoragePaths() = paths;
}

} // namespace DeviceLayer
} // namespace chip
