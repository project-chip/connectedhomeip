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

#include <string>

namespace chip {
namespace DeviceLayer {

static const std::string factoryroot  = "/tmp";
static const std::string configroot   = "/tmp";
static const std::string countersroot = "/tmp";
static const std::string kvsroot      = "/tmp";

std::string DefaultFilesystemStorageLocationProviderImpl::GetFactoryDataLocation() const
{
    return factoryroot;
}

std::string DefaultFilesystemStorageLocationProviderImpl::GetConfigDataLocation() const
{
    return configroot;
}

std::string DefaultFilesystemStorageLocationProviderImpl::GetCountersDataLocation() const
{
    return countersroot;
}

std::string DefaultFilesystemStorageLocationProviderImpl::GetKVSDataLocation() const
{
    return kvsroot;
}

DefaultFilesystemStorageLocationProviderImpl & DefaultFilesystemStorageLocationProviderImpl::GetDefaultInstance()
{
    static DefaultFilesystemStorageLocationProviderImpl sInstance;
    return sInstance;
}

} // namespace DeviceLayer
} // namespace chip
