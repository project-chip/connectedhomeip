/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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

#include "static-supported-modes-manager.h"
#include <platform/ESP32/ESP32Config.h>

using namespace chip;
using namespace chip::app::Clusters;
using namespace chip::DeviceLayer::Internal;
using namespace chip::app::Clusters::ModeSelect;
using chip::Protocols::InteractionModel::Status;

using ModeOptionStructType = Structs::ModeOptionStruct::Type;
using ModeTagType          = Structs::ModeTagStruct::Type;

template <typename T>
using List = app::DataModel::List<T>;

const StaticSupportedModesManager StaticSupportedModesManager::instance = StaticSupportedModesManager();

SupportedModesManager::ModeOptionsProvider StaticSupportedModesManager::epModeOptionsProviderList[FIXED_ENDPOINT_COUNT];

void StaticSupportedModesManager::InitEndpointArray()
{
    for (int i = 0; i < FIXED_ENDPOINT_COUNT; i++)
    {
        epModeOptionsProviderList[i] = ModeOptionsProvider();
    }
}

SupportedModesManager::ModeOptionsProvider StaticSupportedModesManager::getModeOptionsProvider(EndpointId endpointId) const
{
    if (epModeOptionsProviderList[endpointId].begin() != nullptr && epModeOptionsProviderList[endpointId].end() != nullptr)
    {
        return ModeOptionsProvider(epModeOptionsProviderList[endpointId].begin(), epModeOptionsProviderList[endpointId].end());
    }

    ModeOptionStructType * modeOptionStructList = nullptr;
    ModeTagType * modeTags                      = nullptr;

    char keyBuf[ESP32Config::kMaxConfigKeyNameLength];
    uint32_t supportedModeCount = 0;

    VerifyOrReturnValue(ESP32Config::KeyAllocator::SupportedModesCount(keyBuf, sizeof(keyBuf), endpointId) == CHIP_NO_ERROR,
                        ModeOptionsProvider(nullptr, nullptr));
    ESP32Config::Key countKey(ESP32Config::kConfigNamespace_ChipFactory, keyBuf);
    VerifyOrReturnValue(ESP32Config::ReadConfigValue(countKey, supportedModeCount) == CHIP_NO_ERROR,
                        ModeOptionsProvider(nullptr, nullptr));

    modeOptionStructList = new ModeOptionStructType[supportedModeCount];
    if (modeOptionStructList == nullptr)
    {
        return ModeOptionsProvider(nullptr, nullptr);
    }

    epModeOptionsProviderList[endpointId] = ModeOptionsProvider(modeOptionStructList, modeOptionStructList + supportedModeCount);

    for (int index = 0; index < supportedModeCount; index++)
    {
        Structs::ModeOptionStruct::Type option;
        uint32_t supportedModeMode = 0;
        uint32_t modeTagCount      = 0;
        size_t outLen              = 0;

        memset(keyBuf, 0, sizeof(char) * ESP32Config::kMaxConfigKeyNameLength);
        VerifyOrReturnValue(ESP32Config::KeyAllocator::SupportedModesLabel(keyBuf, sizeof(keyBuf), endpointId, index) ==
                                CHIP_NO_ERROR,
                            ModeOptionsProvider(nullptr, nullptr), CleanUp(endpointId));
        ESP32Config::Key labelKey(ESP32Config::kConfigNamespace_ChipFactory, keyBuf);
        VerifyOrReturnValue(ESP32Config::ReadConfigValueStr(labelKey, nullptr, 0, outLen) == CHIP_NO_ERROR,
                            ModeOptionsProvider(nullptr, nullptr), CleanUp(endpointId));

        char * modeLabel = new char[outLen + 1];
        if (modeLabel == nullptr)
        {
            CleanUp(endpointId);
            return ModeOptionsProvider(nullptr, nullptr);
        }

        VerifyOrReturnValue(ESP32Config::ReadConfigValueStr(labelKey, modeLabel, outLen + 1, outLen) == CHIP_NO_ERROR,
                            ModeOptionsProvider(nullptr, nullptr), CleanUp(endpointId));

        memset(keyBuf, 0, sizeof(char) * ESP32Config::kMaxConfigKeyNameLength);
        VerifyOrReturnValue(ESP32Config::KeyAllocator::SupportedModesValue(keyBuf, sizeof(keyBuf), endpointId, index) ==
                                CHIP_NO_ERROR,
                            ModeOptionsProvider(nullptr, nullptr), CleanUp(endpointId));
        ESP32Config::Key modeKey(ESP32Config::kConfigNamespace_ChipFactory, keyBuf);
        VerifyOrReturnValue(ESP32Config::ReadConfigValue(labelKey, supportedModeMode) == CHIP_NO_ERROR,
                            ModeOptionsProvider(nullptr, nullptr), CleanUp(endpointId));

        memset(keyBuf, 0, sizeof(char) * ESP32Config::kMaxConfigKeyNameLength);
        VerifyOrReturnValue(ESP32Config::KeyAllocator::ModeTagsCount(keyBuf, sizeof(keyBuf), endpointId, index) == CHIP_NO_ERROR,
                            ModeOptionsProvider(nullptr, nullptr), CleanUp(endpointId));
        ESP32Config::Key stCountKey(ESP32Config::kConfigNamespace_ChipFactory, keyBuf);
        VerifyOrReturnValue(ESP32Config::ReadConfigValue(stCountKey, modeTagCount) == CHIP_NO_ERROR,
                            ModeOptionsProvider(nullptr, nullptr), CleanUp(endpointId));

        modeTags = new ModeTagType[modeTagCount];
        if (modeTags == nullptr)
        {
            CleanUp(endpointId);
            return ModeOptionsProvider(nullptr, nullptr);
        }
        for (auto stIndex = 0; stIndex < modeTagCount; stIndex++)
        {

            uint32_t modeTagValue   = 0;
            uint32_t modeTagMfgCode = 0;
            ModeTagType tag;

            memset(keyBuf, 0, sizeof(char) * ESP32Config::kMaxConfigKeyNameLength);
            VerifyOrReturnValue(ESP32Config::KeyAllocator::ModeTagValue(keyBuf, sizeof(keyBuf), endpointId, index, stIndex) ==
                                    CHIP_NO_ERROR,
                                ModeOptionsProvider(nullptr, nullptr), CleanUp(endpointId));
            ESP32Config::Key stValueKey(ESP32Config::kConfigNamespace_ChipFactory, keyBuf);
            VerifyOrReturnValue(ESP32Config::ReadConfigValue(stValueKey, modeTagValue) == CHIP_NO_ERROR,
                                ModeOptionsProvider(nullptr, nullptr), CleanUp(endpointId));

            memset(keyBuf, 0, sizeof(char) * ESP32Config::kMaxConfigKeyNameLength);
            VerifyOrReturnValue(ESP32Config::KeyAllocator::ModeTagMfgCode(keyBuf, sizeof(keyBuf), endpointId, index, stIndex) ==
                                    CHIP_NO_ERROR,
                                ModeOptionsProvider(nullptr, nullptr), CleanUp(endpointId));
            ESP32Config::Key stMfgCodeKey(ESP32Config::kConfigNamespace_ChipFactory, keyBuf);
            VerifyOrReturnValue(ESP32Config::ReadConfigValue(stMfgCodeKey, modeTagMfgCode) == CHIP_NO_ERROR,
                                ModeOptionsProvider(nullptr, nullptr), CleanUp(endpointId));

            tag.value = static_cast<uint16_t>(modeTagValue);
            //            tag.mfgCode       = static_cast<chip::VendorId>(modeTagMfgCode);
            modeTags[stIndex] = tag;
        }

        option.label    = chip::CharSpan::fromCharString(modeLabel);
        option.mode     = static_cast<uint8_t>(supportedModeMode);
        option.modeTags = DataModel::List<const ModeTagType>(modeTags, modeTagCount);

        modeOptionStructList[index] = option;
    }

    return ModeOptionsProvider(modeOptionStructList, modeOptionStructList + supportedModeCount);
}

Status StaticSupportedModesManager::getModeOptionByMode(unsigned short endpointId, unsigned char mode,
                                                        const ModeOptionStructType ** dataPtr) const
{
    auto modeOptionsProvider = this->getModeOptionsProvider(endpointId);
    if (modeOptionsProvider.begin() == nullptr)
    {
        return Status::UnsupportedCluster;
    }
    auto * begin = modeOptionsProvider.begin();
    auto * end   = modeOptionsProvider.end();

    for (auto * it = begin; it != end; ++it)
    {
        auto & modeOption = *it;
        if (modeOption.mode == mode)
        {
            *dataPtr = &modeOption;
            return Status::Success;
        }
    }
    ChipLogProgress(Zcl, "Cannot find the mode %u", mode);
    return Status::InvalidCommand;
}

const ModeSelect::SupportedModesManager * ModeSelect::getSupportedModesManager()
{
    return &StaticSupportedModesManager::instance;
}

void StaticSupportedModesManager::FreeSupportedModes(EndpointId endpointId) const
{
    if (epModeOptionsProviderList[endpointId].begin() != nullptr)
    {
        auto * begin = epModeOptionsProviderList[endpointId].begin();
        auto * end   = epModeOptionsProviderList[endpointId].end();
        for (auto * it = begin; it != end; ++it)
        {
            auto & modeOption = *it;
            delete[] modeOption.label.data();
            delete[] modeOption.modeTags.data();
        }
        delete[] begin;
    }
    epModeOptionsProviderList[endpointId] = ModeOptionsProvider();
}

void StaticSupportedModesManager::CleanUp(EndpointId endpointId) const
{
    ChipLogError(Zcl, "Supported mode data is in incorrect format");
    FreeSupportedModes(endpointId);
}
