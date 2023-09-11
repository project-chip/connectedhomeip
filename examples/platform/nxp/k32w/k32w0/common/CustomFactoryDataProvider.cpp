/*
 *
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
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
    static_assert(CustomFactoryIds::kCustomMaxId == FactoryDataProvider::kNumberOfIds,
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
    // A user can use FactoryDataProvider::SearchForId to read an id from internal
    // flash factory data section.
    ReturnErrorOnFailure(SearchForId(CustomFactoryIds::kCustomId1, buffer.data(), buffer.size(), userDataSize));

    // Data should now be ready for custom parsing.

    return CHIP_NO_ERROR;
}

} // namespace DeviceLayer
} // namespace chip
