/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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

#include "FilesystemStorageLocationProviderImpl.h"
#include "CHIPLinuxStorage.h"

#include <string>

namespace chip {
namespace DeviceLayer {

const char * DefaultFilesystemStorageLocationProviderImpl::LegacyKVS() const
{
    return nullptr;
}

// Note: These methods return DIRECTORY paths, not full file paths.
// The file name is appended by the caller (PosixConfig.cpp).
// Default directory for all files is CHIP_DEFAULT_BASE_DIR (no subdirectories).

std::string DefaultFilesystemStorageLocationProviderImpl::GetFactoryDataLocation() const
{
    return CHIP_DEFAULT_BASE_DIR;
}

std::string DefaultFilesystemStorageLocationProviderImpl::GetConfigDataLocation() const
{
    return CHIP_DEFAULT_BASE_DIR;
}

std::string DefaultFilesystemStorageLocationProviderImpl::GetCountersDataLocation() const
{
    return CHIP_DEFAULT_BASE_DIR;
}

std::string DefaultFilesystemStorageLocationProviderImpl::GetKVSDataLocation() const
{
    return CHIP_DEFAULT_BASE_DIR;
}

DefaultFilesystemStorageLocationProviderImpl & DefaultFilesystemStorageLocationProviderImpl::GetDefaultInstance()
{
    static DefaultFilesystemStorageLocationProviderImpl sInstance;
    return sInstance;
}

} // namespace DeviceLayer
} // namespace chip
