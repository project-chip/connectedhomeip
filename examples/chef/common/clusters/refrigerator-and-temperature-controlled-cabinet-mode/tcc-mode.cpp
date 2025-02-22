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
#include <app-common/zap-generated/attributes/Accessors.h>
#include <refrigerator-and-temperature-controlled-cabinet-mode/tcc-mode.h>

#ifdef MATTER_DM_PLUGIN_REFRIGERATOR_AND_TEMPERATURE_CONTROLLED_CABINET_MODE_SERVER

using namespace chip::app::Clusters;
using namespace chip::app::Clusters::RefrigeratorAndTemperatureControlledCabinetMode;
using chip::Protocols::InteractionModel::Status;
template <typename T>
using List              = chip::app::DataModel::List<T>;
using ModeTagStructType = chip::app::Clusters::detail::Structs::ModeTagStruct::Type;

static std::unique_ptr<TccModeDelegate> gTccModeDelegate;
static std::unique_ptr<ModeBase::Instance> gTccModeInstance;

CHIP_ERROR TccModeDelegate::Init()
{
    return CHIP_NO_ERROR;
}

void TccModeDelegate::HandleChangeToMode(uint8_t NewMode, ModeBase::Commands::ChangeToModeResponse::Type & response)
{
    response.status = to_underlying(ModeBase::StatusCode::kSuccess);
}

CHIP_ERROR TccModeDelegate::GetModeLabelByIndex(uint8_t modeIndex, chip::MutableCharSpan & label)
{
    if (modeIndex >= MATTER_ARRAY_SIZE(kModeOptions))
    {
        return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
    }
    return chip::CopyCharSpanToMutableCharSpan(kModeOptions[modeIndex].label, label);
}

CHIP_ERROR TccModeDelegate::GetModeValueByIndex(uint8_t modeIndex, uint8_t & value)
{
    if (modeIndex >= MATTER_ARRAY_SIZE(kModeOptions))
    {
        return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
    }
    value = kModeOptions[modeIndex].mode;
    return CHIP_NO_ERROR;
}

CHIP_ERROR TccModeDelegate::GetModeTagsByIndex(uint8_t modeIndex, List<ModeTagStructType> & tags)
{
    if (modeIndex >= MATTER_ARRAY_SIZE(kModeOptions))
    {
        return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
    }

    if (tags.size() < kModeOptions[modeIndex].modeTags.size())
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    std::copy(kModeOptions[modeIndex].modeTags.begin(), kModeOptions[modeIndex].modeTags.end(), tags.begin());
    tags.reduce_size(kModeOptions[modeIndex].modeTags.size());

    return CHIP_NO_ERROR;
}

void RefrigeratorAndTemperatureControlledCabinetMode::Shutdown()
{
    gTccModeInstance.reset();
    gTccModeDelegate.reset();
}

chip::Protocols::InteractionModel::Status
chefRefrigeratorAndTemperatureControlledCabinetModeExternalReadCallback(chip::EndpointId endpointId, chip::ClusterId clusterId,
                                                                        const EmberAfAttributeMetadata * attributeMetadata,
                                                                        uint8_t * buffer, uint16_t maxReadLength)
{
    chip::Protocols::InteractionModel::Status ret = chip::Protocols::InteractionModel::Status::Success;
    chip::AttributeId attributeId                 = attributeMetadata->attributeId;

    switch (attributeId)
    {
    case chip::app::Clusters::RefrigeratorAndTemperatureControlledCabinetMode::Attributes::CurrentMode::Id: {
        *buffer = gTccModeInstance->GetCurrentMode();
        ChipLogDetail(DeviceLayer, "Reading RunMode CurrentMode : %d", static_cast<int>(attributeId));
    }
    break;
    default:
        ret = chip::Protocols::InteractionModel::Status::UnsupportedRead;
        ChipLogDetail(DeviceLayer, "Unsupported attributeId %d from reading RefrigeratorAndTemperatureControlledCabinetMode",
                      static_cast<int>(attributeId));
        break;
    }

    return ret;
}

chip::Protocols::InteractionModel::Status chefRefrigeratorAndTemperatureControlledCabinetModeExternalWriteCallback(
    chip::EndpointId endpointId, chip::ClusterId clusterId, const EmberAfAttributeMetadata * attributeMetadata, uint8_t * buffer)
{
    VerifyOrDie(endpointId == 1); // this cluster is only enabled for endpoint 1
    VerifyOrDie(gTccModeInstance != nullptr);
    chip::Protocols::InteractionModel::Status ret;
    chip::AttributeId attributeId = attributeMetadata->attributeId;

    switch (attributeId)
    {
    case chip::app::Clusters::RefrigeratorAndTemperatureControlledCabinetMode::Attributes::CurrentMode::Id: {
        uint8_t m = static_cast<uint8_t>(buffer[0]);
        ret       = gTccModeInstance->UpdateCurrentMode(m);
        if (chip::Protocols::InteractionModel::Status::Success != ret)
        {
            ChipLogError(DeviceLayer, "Invalid Attribute Update status: %d", static_cast<int>(ret));
        }
    }
    break;
    default:
        ret = chip::Protocols::InteractionModel::Status::UnsupportedWrite;
        ChipLogError(DeviceLayer, "Unsupported Writng Attribute ID: %d", static_cast<int>(attributeId));
        break;
    }

    return ret;
}

void emberAfRefrigeratorAndTemperatureControlledCabinetModeClusterInitCallback(chip::EndpointId endpointId)
{
    VerifyOrDie(endpointId == 1); // this cluster is only enabled for endpoint 1.
    VerifyOrDie(gTccModeDelegate == nullptr && gTccModeInstance == nullptr);
    gTccModeDelegate = std::make_unique<RefrigeratorAndTemperatureControlledCabinetMode::TccModeDelegate>();
    gTccModeInstance =
        std::make_unique<ModeBase::Instance>(gTccModeDelegate.get(), 0x1, RefrigeratorAndTemperatureControlledCabinetMode::Id, 0);
    gTccModeInstance->Init();
}

#endif // MATTER_DM_PLUGIN_REFRIGERATOR_AND_TEMPERATURE_CONTROLLED_CABINET_MODE_SERVER
