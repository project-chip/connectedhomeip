#include "static-supported-modes-manager.h"
#include "ESP32Config.h"
#include <app/util/debug-printing.h>
#include <app/util/ember-print.h>

using namespace chip;
using namespace chip::app::Clusters;
using namespace chip::DeviceLayer::Internal;
using namespace chip::app::Clusters::ModeSelect;
using chip::Protocols::InteractionModel::Status;

using ModeOptionStructType = Structs::ModeOptionStruct::Type;
using SemanticTag          = Structs::SemanticTagStruct::Type;
template <typename T>
using List = app::DataModel::List<T>;

StaticSupportedModesManager::ModeOptionStructType * StaticSupportedModesManager::modeOptionStructList = nullptr;
StaticSupportedModesManager::SemanticTag * StaticSupportedModesManager::semanticTags                  = nullptr;

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

    for (int index = 0; index < supportedModeCount; index++)
    {
        Structs::ModeOptionStruct::Type option;
        uint32_t supportedModeMode = 0;
        uint32_t semanticTagCount  = 0;
        size_t outLen              = 0;

        memset(keyBuf, 0, sizeof(char) * ESP32Config::kMaxConfigKeyNameLength);
        VerifyOrReturnValue(ESP32Config::KeyAllocator::SupportedModesLabel(keyBuf, sizeof(keyBuf), endpointId, index) ==
                                CHIP_NO_ERROR,
                            ModeOptionsProvider(nullptr, nullptr), CleanUp());
        ESP32Config::Key labelKey(ESP32Config::kConfigNamespace_ChipFactory, keyBuf);
        VerifyOrReturnValue(ESP32Config::ReadConfigValueStr(labelKey, nullptr, 0, outLen) == CHIP_NO_ERROR,
                            ModeOptionsProvider(nullptr, nullptr), CleanUp());

        char * modeLabel = new char[outLen + 1];
        VerifyOrReturnValue(ESP32Config::ReadConfigValueStr(labelKey, modeLabel, outLen + 1, outLen) == CHIP_NO_ERROR,
                            ModeOptionsProvider(nullptr, nullptr), CleanUp());

        memset(keyBuf, 0, sizeof(char) * ESP32Config::kMaxConfigKeyNameLength);
        VerifyOrReturnValue(ESP32Config::KeyAllocator::SupportedModesValue(keyBuf, sizeof(keyBuf), endpointId, index) ==
                                CHIP_NO_ERROR,
                            ModeOptionsProvider(nullptr, nullptr), CleanUp());
        ESP32Config::Key modeKey(ESP32Config::kConfigNamespace_ChipFactory, keyBuf);
        VerifyOrReturnValue(ESP32Config::ReadConfigValue(labelKey, supportedModeMode) == CHIP_NO_ERROR,
                            ModeOptionsProvider(nullptr, nullptr), CleanUp());

        memset(keyBuf, 0, sizeof(char) * ESP32Config::kMaxConfigKeyNameLength);
        VerifyOrReturnValue(ESP32Config::KeyAllocator::SemanticTagsCount(keyBuf, sizeof(keyBuf), endpointId, index) ==
                                CHIP_NO_ERROR,
                            ModeOptionsProvider(nullptr, nullptr), CleanUp());
        ESP32Config::Key stCountKey(ESP32Config::kConfigNamespace_ChipFactory, keyBuf);
        VerifyOrReturnValue(ESP32Config::ReadConfigValue(stCountKey, semanticTagCount) == CHIP_NO_ERROR,
                            ModeOptionsProvider(nullptr, nullptr), CleanUp());

        semanticTags = new SemanticTag[semanticTagCount];
        if (semanticTags == nullptr)
        {
            CleanUp();
            return ModeOptionsProvider(nullptr, nullptr);
        }
        for (auto stIndex = 0; stIndex < semanticTagCount; stIndex++)
        {

            uint32_t semanticTagValue   = 0;
            uint32_t semanticTagMfgCode = 0;
            SemanticTag tag;

            memset(keyBuf, 0, sizeof(char) * ESP32Config::kMaxConfigKeyNameLength);
            VerifyOrReturnValue(ESP32Config::KeyAllocator::SemanticTagValue(keyBuf, sizeof(keyBuf), endpointId, index, stIndex) ==
                                    CHIP_NO_ERROR,
                                ModeOptionsProvider(nullptr, nullptr), CleanUp());
            ESP32Config::Key stValueKey(ESP32Config::kConfigNamespace_ChipFactory, keyBuf);
            VerifyOrReturnValue(ESP32Config::ReadConfigValue(stValueKey, semanticTagValue) == CHIP_NO_ERROR,
                                ModeOptionsProvider(nullptr, nullptr), CleanUp());

            memset(keyBuf, 0, sizeof(char) * ESP32Config::kMaxConfigKeyNameLength);
            VerifyOrReturnValue(ESP32Config::KeyAllocator::SemanticTagMfgCode(keyBuf, sizeof(keyBuf), endpointId, index, stIndex) ==
                                    CHIP_NO_ERROR,
                                ModeOptionsProvider(nullptr, nullptr), CleanUp());
            ESP32Config::Key stMfgCodeKey(ESP32Config::kConfigNamespace_ChipFactory, keyBuf);
            VerifyOrReturnValue(ESP32Config::ReadConfigValue(stMfgCodeKey, semanticTagMfgCode) == CHIP_NO_ERROR,
                                ModeOptionsProvider(nullptr, nullptr), CleanUp());

            tag.value             = static_cast<uint16_t>(semanticTagValue);
            tag.mfgCode           = static_cast<chip::VendorId>(semanticTagMfgCode);
            semanticTags[stIndex] = tag;
        }

        option.label        = chip::CharSpan::fromCharString(modeLabel);
        option.mode         = static_cast<uint8_t>(supportedModeMode);
        option.semanticTags = DataModel::List<const SemanticTag>(semanticTags, semanticTagCount);

        modeOptionStructList[index] = option;
    }
    epModeOptionsProviderList[endpointId] = ModeOptionsProvider(modeOptionStructList, modeOptionStructList + supportedModeCount);

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
    emberAfPrintln(EMBER_AF_PRINT_DEBUG, "Cannot find the mode %u", mode);
    return Status::InvalidCommand;
}

const ModeSelect::SupportedModesManager * ModeSelect::getSupportedModesManager()
{
    return &StaticSupportedModesManager::instance;
}

void StaticSupportedModesManager::FreeSupportedModes() const
{
    for (int i = 0; i < FIXED_ENDPOINT_COUNT; i++)
    {
        if (epModeOptionsProviderList[i].begin() != nullptr)
        {
            auto * begin = epModeOptionsProviderList[i].begin();
            auto * end   = epModeOptionsProviderList[i].end();
            for (auto * it = begin; it != end; ++it)
            {
                auto & modeOption = *it;
                delete modeOption.label.data();
                delete[] modeOption.semanticTags.data();
            }
            delete[] begin;
        }
        epModeOptionsProviderList[i] = ModeOptionsProvider();
    }
}

void StaticSupportedModesManager::CleanUp() const
{
    ChipLogError(Zcl, "Supported mode data is in incorrect format");
    FreeSupportedModes();
}
