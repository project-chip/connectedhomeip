#include "static-supported-modes-manager.h"
#include <app/util/debug-printing.h>
#include <app/util/ember-print.h>
#include "ESP32Config.h"

using namespace chip;
using namespace chip::app::Clusters;
using namespace chip::DeviceLayer::Internal;
using namespace chip::app::Clusters::ModeSelect;
using chip::Protocols::InteractionModel::Status;

using ModeOptionStructType = Structs::ModeOptionStruct::Type;
using SemanticTag          = Structs::SemanticTagStruct::Type;
template <typename T>
using List               = app::DataModel::List<T>;
char chip::app::Clusters::ModeSelect::StaticSupportedModesManager::supportedModeLabel[64] = "";

const StaticSupportedModesManager StaticSupportedModesManager::instance = StaticSupportedModesManager();

SupportedModesManager::ModeOptionsProvider StaticSupportedModesManager::getModeOptionsProvider(EndpointId endpointId) const
{
	char keyBuf[ESP32Config::kMaxConfigKeyNameLength];
    uint32_t supportedModeCount = 0;

    VerifyOrReturnValue(ESP32Config::KeyAllocator::SupportedModesCount(keyBuf, sizeof(keyBuf), endpointId) == CHIP_NO_ERROR, ModeOptionsProvider(nullptr, nullptr));
    ESP32Config::Key countKey(ESP32Config::kConfigNamespace_ChipFactory, keyBuf);
    VerifyOrReturnValue(ESP32Config::ReadConfigValue(countKey, supportedModeCount) == CHIP_NO_ERROR, ModeOptionsProvider(nullptr, nullptr));

	ModeOptionStructType *modeOptionStruct = new ModeOptionStructType[supportedModeCount + 1];

	for (int index = 0; index < supportedModeCount; index++) {
		Structs::ModeOptionStruct::Type option;
   	 	uint32_t supportedModeMode = 0;
    	uint32_t semanticTagCount = 0;
		size_t outLen   = 0;

		memset(supportedModeLabel, 0, sizeof(supportedModeLabel));
		memset(keyBuf, 0, sizeof(char)*ESP32Config::kMaxConfigKeyNameLength);
    	VerifyOrReturnValue(ESP32Config::KeyAllocator::SupportedModesLabel(keyBuf, sizeof(keyBuf), endpointId, index) == CHIP_NO_ERROR, ModeOptionsProvider(nullptr, nullptr));
    	ESP32Config::Key labelKey(ESP32Config::kConfigNamespace_ChipFactory, keyBuf);
    	VerifyOrReturnValue(ESP32Config::ReadConfigValueStr(labelKey, supportedModeLabel, sizeof(supportedModeLabel), outLen) == CHIP_NO_ERROR, ModeOptionsProvider(nullptr, nullptr));


		memset(keyBuf, 0, sizeof(char)*ESP32Config::kMaxConfigKeyNameLength);
    	VerifyOrReturnValue(ESP32Config::KeyAllocator::SupportedModesValue(keyBuf, sizeof(keyBuf), endpointId, index) == CHIP_NO_ERROR, ModeOptionsProvider(nullptr, nullptr));
    	ESP32Config::Key modeKey(ESP32Config::kConfigNamespace_ChipFactory, keyBuf);
    	VerifyOrReturnValue(ESP32Config::ReadConfigValue(labelKey, supportedModeMode) == CHIP_NO_ERROR, ModeOptionsProvider(nullptr, nullptr));

		memset(keyBuf, 0, sizeof(char)*ESP32Config::kMaxConfigKeyNameLength);
    	VerifyOrReturnValue(ESP32Config::KeyAllocator::SemanticTagsCount(keyBuf, sizeof(keyBuf), endpointId, index) == CHIP_NO_ERROR, ModeOptionsProvider(nullptr, nullptr));
    	ESP32Config::Key stCountKey(ESP32Config::kConfigNamespace_ChipFactory, keyBuf);
    	VerifyOrReturnValue(ESP32Config::ReadConfigValue(stCountKey, semanticTagCount) == CHIP_NO_ERROR, ModeOptionsProvider(nullptr, nullptr));

		SemanticTag *semanticTags = new SemanticTag[semanticTagCount];
		for (auto stIndex = 0; stIndex < semanticTagCount; stIndex++) {

			uint32_t semanticTagValue = 0;
			uint32_t semanticTagMfgCode = 0;
			SemanticTag tag;

			memset(keyBuf, 0, sizeof(char)*ESP32Config::kMaxConfigKeyNameLength);
    		VerifyOrReturnValue(ESP32Config::KeyAllocator::SemanticTagValue(keyBuf, sizeof(keyBuf), endpointId, index, stIndex) == CHIP_NO_ERROR, ModeOptionsProvider(nullptr, nullptr));
    		ESP32Config::Key stValueKey(ESP32Config::kConfigNamespace_ChipFactory, keyBuf);
    		VerifyOrReturnValue(ESP32Config::ReadConfigValue(stValueKey, semanticTagValue) == CHIP_NO_ERROR, ModeOptionsProvider(nullptr, nullptr));

			memset(keyBuf, 0, sizeof(char)*ESP32Config::kMaxConfigKeyNameLength);
    		VerifyOrReturnValue(ESP32Config::KeyAllocator::SemanticTagMfgCode(keyBuf, sizeof(keyBuf), endpointId, index, stIndex) == CHIP_NO_ERROR, ModeOptionsProvider(nullptr, nullptr));
    		ESP32Config::Key stMfgCodeKey(ESP32Config::kConfigNamespace_ChipFactory, keyBuf);
    		VerifyOrReturnValue(ESP32Config::ReadConfigValue(stMfgCodeKey, semanticTagMfgCode) == CHIP_NO_ERROR, ModeOptionsProvider(nullptr, nullptr));

			tag.value = static_cast<uint16_t>(semanticTagValue);
			tag.mfgCode = static_cast<chip::VendorId>(semanticTagMfgCode);
			semanticTags[stIndex] = static_cast<const SemanticTag>(tag);

		}

    	option.label =  chip::CharSpan::fromCharString(supportedModeLabel);
    	option.mode = static_cast<uint8_t>(supportedModeMode);
    	option.semanticTags = DataModel::List<const SemanticTag>(semanticTags, semanticTagCount);

		modeOptionStruct[index] = option;

	}

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
    auto * begin = this->getModeOptionsProvider(endpointId).begin();
    auto * end   = this->getModeOptionsProvider(endpointId).end();

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
