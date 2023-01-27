/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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

#include "CustomFactoryDataProvider.h"

namespace chip {
namespace DeviceLayer {

static constexpr size_t kMaxLengthCustomId1 = 10;
static constexpr size_t kMaxLengthCustomId2 = 50;
static constexpr size_t kMaxLengthCustomId3 = 100;

CustomFactoryDataProvider & CustomFactoryDataProvider::GetDefaultInstance()
{
    static CustomFactoryDataProvider sInstance;
    return sInstance;
}

CHIP_ERROR CustomFactoryDataProvider::SetCustomIds()
{
    static_assert(CustomFactoryIds::kCustomMaxId == K32W0FactoryDataProvider::kNumberOfIds,
                  "kNumberOfIds is computed based on CHIP_DEVICE_CONFIG_CUSTOM_PROVIDER_NUMBER_IDS.");
    maxLengths[CustomFactoryIds::kCustomId1] = kMaxLengthCustomId1;
    maxLengths[CustomFactoryIds::kCustomId2] = kMaxLengthCustomId2;
    maxLengths[CustomFactoryIds::kCustomId3] = kMaxLengthCustomId3;

    return CHIP_NO_ERROR;
}

CHIP_ERROR CustomFactoryDataProvider::ParseFunctionExample()
{
    uint8_t data_buf[kMaxLengthCustomId1];
    MutableByteSpan buffer(data_buf);
    memset(buffer.data(), 0, buffer.size());
    uint16_t userDataSize = 0;
    // A user can use K32W0FactoryDataProvider::SearchForId to read an id from internal
    // flash factory data section.
    ReturnErrorOnFailure(SearchForId(CustomFactoryIds::kCustomId1, buffer.data(), buffer.size(), userDataSize));

    // Data should now be ready for custom parsing.

    return CHIP_NO_ERROR;
}

} // namespace DeviceLayer
} // namespace chip
