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

#include <filesystem>

#include <platform/Linux/CHIPLinuxStoragePaths.h>

namespace chip {
namespace DeviceLayer {

// Constructor implementation
ChipLinuxStoragePaths::ChipLinuxStoragePaths(const std::string & baseDir) : mBaseDir(baseDir) {}

// Setter implementations
void ChipLinuxStoragePaths::SetBaseDir(const std::string & baseDir)
{
    mBaseDir = baseDir;
}

std::string ChipLinuxStoragePaths::GetBaseDir() const
{
    return mBaseDir;
}

void ChipLinuxStoragePaths::SetKVSDataFile(const std::string & path)
{
    mKVSDataFile = path;
}

void ChipLinuxStoragePaths::SetFactoryFile(const std::string & path)
{
    mFactoryFile = path;
}

void ChipLinuxStoragePaths::SetConfigFile(const std::string & path)
{
    mConfigFile = path;
}

void ChipLinuxStoragePaths::SetCountersFile(const std::string & path)
{
    mCountersFile = path;
}

// Helper function to resolve a path using base directory and default filename
std::string ChipLinuxStoragePaths::ResolvePath(const std::string & path, const std::string & defaultFilename) const
{
    // If path is explicitly set and non-empty, use it directly
    if (!path.empty())
    {
        return path;
    }

    // Otherwise, use base directory + default filename
    std::string baseDir = mBaseDir.empty() ? CHIP_DEFAULT_BASE_DIR : mBaseDir;
    std::filesystem::path p(baseDir);
    // Always append default filename when path is empty (user didn't specify explicit file)
    p /= defaultFilename;
    return p.string();
}

// Getter implementations
std::string ChipLinuxStoragePaths::GetKVSDataFilePath() const
{
    // If an explicit KVS data file was set, use it directly.
    if (!mKVSDataFile.empty())
    {
        return mKVSDataFile;
    }
    // Otherwise fall back to the compile-time default (CHIP_CONFIG_KVS_PATH),
    // preserving backward compatibility with apps that rely on it.
#ifdef CHIP_CONFIG_KVS_PATH
    return CHIP_CONFIG_KVS_PATH;
#else
    return ResolvePath(mKVSDataFile, CHIP_DEFAULT_DATA_FILENAME);
#endif
}

std::string ChipLinuxStoragePaths::GetFactoryFilePath() const
{
    return ResolvePath(mFactoryFile, CHIP_DEFAULT_FACTORY_FILENAME);
}

std::string ChipLinuxStoragePaths::GetConfigFilePath() const
{
    return ResolvePath(mConfigFile, CHIP_DEFAULT_CONFIG_FILENAME);
}

std::string ChipLinuxStoragePaths::GetCountersFilePath() const
{
    return ResolvePath(mCountersFile, CHIP_DEFAULT_COUNTERS_FILENAME);
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
