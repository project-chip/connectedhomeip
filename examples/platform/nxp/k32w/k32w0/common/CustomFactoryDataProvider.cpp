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

CustomFactoryDataProvider::CustomFactoryDataProvider()
{
    // Custom ids should be from a range that does not overlap with the standard factory data range.
    static_assert((uint16_t) CustomFactoryIds::kCustomId1 >= (uint16_t) FactoryDataProvider::FactoryDataId::kMaxId);
}

CHIP_ERROR CustomFactoryDataProvider::ParseFunctionExample()
{
    uint8_t data_buf[kMaxLengthCustomId1];
    MutableByteSpan buffer(data_buf);
    memset(buffer.data(), 0, buffer.size());
    uint16_t userDataSize = 0;
    // A user can use FactoryDataProvider::SearchForId to read an id from internal
    // flash factory data section.
    auto * provider = static_cast<FactoryDataProvider *>(DeviceLayer::GetDeviceInstanceInfoProvider());
    ReturnErrorOnFailure((provider != nullptr) ? CHIP_NO_ERROR : CHIP_ERROR_INVALID_ADDRESS);
    ReturnErrorOnFailure(provider->SearchForId(CustomFactoryIds::kCustomId1, buffer.data(), buffer.size(), userDataSize));

    // Data should now be ready for custom parsing.

    return CHIP_NO_ERROR;
}

} // namespace DeviceLayer
} // namespace chip
