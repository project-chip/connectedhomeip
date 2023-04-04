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
using List                                                                                = app::DataModel::List<T>;

StaticSupportedModesManager::ModeLabel* StaticSupportedModesManager::modeLabelList = nullptr;
StaticSupportedModesManager::ModeOptionStructType* StaticSupportedModesManager::modeOptionStruct = nullptr;
StaticSupportedModesManager::SemanticTag* StaticSupportedModesManager::semanticTags = nullptr;

const StaticSupportedModesManager StaticSupportedModesManager::instance = StaticSupportedModesManager();

ModeOptionStructType *StaticSupportedModesManager::endpointArray[FIXED_ENDPOINT_COUNT][2];

void StaticSupportedModesManager::InitEndpointArray()
{
	for(int i=0; i<FIXED_ENDPOINT_COUNT; i++) {
		endpointArray[i][0] = nullptr;
		endpointArray[i][1] = nullptr;
	}
}

SupportedModesManager::ModeOptionsProvider StaticSupportedModesManager::getModeOptionsProvider(EndpointId endpointId) const
{
	if(endpointArray[endpointId][0] != nullptr && endpointArray[endpointId][1] != nullptr) {
		return ModeOptionsProvider(endpointArray[endpointId][0], endpointArray[endpointId][1]);
	}

    char keyBuf[ESP32Config::kMaxConfigKeyNameLength];
    uint32_t supportedModeCount = 0;

    VerifyOrReturnValue(ESP32Config::KeyAllocator::SupportedModesCount(keyBuf, sizeof(keyBuf), endpointId) == CHIP_NO_ERROR,
                        ModeOptionsProvider(nullptr, nullptr));
    ESP32Config::Key countKey(ESP32Config::kConfigNamespace_ChipFactory, keyBuf);
    VerifyOrReturnValue(ESP32Config::ReadConfigValue(countKey, supportedModeCount) == CHIP_NO_ERROR,
                        ModeOptionsProvider(nullptr, nullptr));

	modeLabelList = new ModeLabel[supportedModeCount];
    modeOptionStruct = new ModeOptionStructType[supportedModeCount];

    for (int index = 0; index < supportedModeCount; index++)
    {
        Structs::ModeOptionStruct::Type option;
        uint32_t supportedModeMode = 0;
        uint32_t semanticTagCount  = 0;
        size_t outLen              = 0;

        memset(modeLabelList[index].supportedModeLabel, 0, sizeof(modeLabelList[index].supportedModeLabel));
        memset(keyBuf, 0, sizeof(char) * ESP32Config::kMaxConfigKeyNameLength);
        VerifyOrReturnValue(ESP32Config::KeyAllocator::SupportedModesLabel(keyBuf, sizeof(keyBuf), endpointId, index) ==
                                CHIP_NO_ERROR,
                            ModeOptionsProvider(nullptr, nullptr));
        ESP32Config::Key labelKey(ESP32Config::kConfigNamespace_ChipFactory, keyBuf);
        VerifyOrReturnValue(ESP32Config::ReadConfigValueStr(labelKey, modeLabelList[index].supportedModeLabel, sizeof(modeLabelList[index].supportedModeLabel), outLen) ==
                                CHIP_NO_ERROR,
                            ModeOptionsProvider(nullptr, nullptr));

        memset(keyBuf, 0, sizeof(char) * ESP32Config::kMaxConfigKeyNameLength);
        VerifyOrReturnValue(ESP32Config::KeyAllocator::SupportedModesValue(keyBuf, sizeof(keyBuf), endpointId, index) ==
                                CHIP_NO_ERROR,
                            ModeOptionsProvider(nullptr, nullptr));
        ESP32Config::Key modeKey(ESP32Config::kConfigNamespace_ChipFactory, keyBuf);
        VerifyOrReturnValue(ESP32Config::ReadConfigValue(labelKey, supportedModeMode) == CHIP_NO_ERROR,
                            ModeOptionsProvider(nullptr, nullptr));

        memset(keyBuf, 0, sizeof(char) * ESP32Config::kMaxConfigKeyNameLength);
        VerifyOrReturnValue(ESP32Config::KeyAllocator::SemanticTagsCount(keyBuf, sizeof(keyBuf), endpointId, index) ==
                                CHIP_NO_ERROR,
                            ModeOptionsProvider(nullptr, nullptr));
        ESP32Config::Key stCountKey(ESP32Config::kConfigNamespace_ChipFactory, keyBuf);
        VerifyOrReturnValue(ESP32Config::ReadConfigValue(stCountKey, semanticTagCount) == CHIP_NO_ERROR,
                            ModeOptionsProvider(nullptr, nullptr));

        semanticTags = new SemanticTag[semanticTagCount];
        for (auto stIndex = 0; stIndex < semanticTagCount; stIndex++)
        {

            uint32_t semanticTagValue   = 0;
            uint32_t semanticTagMfgCode = 0;
            SemanticTag tag;

            memset(keyBuf, 0, sizeof(char) * ESP32Config::kMaxConfigKeyNameLength);
            VerifyOrReturnValue(ESP32Config::KeyAllocator::SemanticTagValue(keyBuf, sizeof(keyBuf), endpointId, index, stIndex) ==
                                    CHIP_NO_ERROR,
                                ModeOptionsProvider(nullptr, nullptr));
            ESP32Config::Key stValueKey(ESP32Config::kConfigNamespace_ChipFactory, keyBuf);
            VerifyOrReturnValue(ESP32Config::ReadConfigValue(stValueKey, semanticTagValue) == CHIP_NO_ERROR,
                                ModeOptionsProvider(nullptr, nullptr));

            memset(keyBuf, 0, sizeof(char) * ESP32Config::kMaxConfigKeyNameLength);
            VerifyOrReturnValue(ESP32Config::KeyAllocator::SemanticTagMfgCode(keyBuf, sizeof(keyBuf), endpointId, index, stIndex) ==
                                    CHIP_NO_ERROR,
                                ModeOptionsProvider(nullptr, nullptr));
            ESP32Config::Key stMfgCodeKey(ESP32Config::kConfigNamespace_ChipFactory, keyBuf);
            VerifyOrReturnValue(ESP32Config::ReadConfigValue(stMfgCodeKey, semanticTagMfgCode) == CHIP_NO_ERROR,
                                ModeOptionsProvider(nullptr, nullptr));

            tag.value             = static_cast<uint16_t>(semanticTagValue);
            tag.mfgCode           = static_cast<chip::VendorId>(semanticTagMfgCode);
            semanticTags[stIndex] = static_cast<const SemanticTag>(tag);
        }

        option.label        = chip::CharSpan::fromCharString(modeLabelList[index].supportedModeLabel);
        option.mode         = static_cast<uint8_t>(supportedModeMode);
        option.semanticTags = DataModel::List<const SemanticTag>(semanticTags, semanticTagCount);

        modeOptionStruct[index] = option;
    }
	endpointArray[endpointId][0] = modeOptionStruct;
	endpointArray[endpointId][1] = (modeOptionStruct + supportedModeCount);

    return ModeOptionsProvider(modeOptionStruct, modeOptionStruct + supportedModeCount);
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

void StaticSupportedModesManager::FreeSupportedModes()
{
	for(int i=0; i<FIXED_ENDPOINT_COUNT; i++) {
		if(endpointArray[i][0] != nullptr) {
    		auto * begin = static_cast<ModeOptionStructType *>(endpointArray[i][0]);
    		auto * end   = static_cast<ModeOptionStructType *>(endpointArray[i][1]);
			for(auto *it = begin; it != end; ++it) {
        		auto & modeOption = *it;
				delete[] modeOption.label.data();
				delete[] modeOption.semanticTags.data();
			}
			delete[] begin;
		}
		endpointArray[i][0] = nullptr;
		endpointArray[i][1] = nullptr;
	}
	delete[] modeLabelList;
	delete[] modeOptionStruct;
	delete[] semanticTags;
}
