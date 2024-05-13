/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#pragma once

#include <cstddef>

#include <lib/core/CHIPError.h>

namespace chip {
namespace DeviceLayer {
namespace PersistedStorage {
namespace Internal {
namespace AppPreference {

CHIP_ERROR CheckData(const char * key);
CHIP_ERROR GetData(const char * key, void * data, size_t dataSize, size_t * getDataSize, size_t offset);
CHIP_ERROR SaveData(const char * key, const void * data, size_t size);
CHIP_ERROR RemoveData(const char * key);

} // namespace AppPreference
} // namespace Internal
} // namespace PersistedStorage
} // namespace DeviceLayer
} // namespace chip
