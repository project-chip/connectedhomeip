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
#include <chef-laundry-washer-mode.h>

using namespace chip::app::Clusters;
using namespace chip::app::Clusters::LaundryWasherMode;
using chip::Protocols::InteractionModel::Status;
template <typename T>
using List              = chip::app::DataModel::List<T>;
using ModeTagStructType = chip::app::Clusters::detail::Structs::ModeTagStruct::Type;

#ifdef MATTER_DM_PLUGIN_LAUNDRY_WASHER_MODE_SERVER
static LaundryWasherModeDelegate * gLaundryWasherModeDelegate = nullptr;
static ModeBase::Instance * gLaundryWasherModeInstance        = nullptr;

CHIP_ERROR LaundryWasherModeDelegate::Init()
{
    return CHIP_NO_ERROR;
}

void LaundryWasherModeDelegate::HandleChangeToMode(uint8_t NewMode, ModeBase::Commands::ChangeToModeResponse::Type & response)
{
    response.status = to_underlying(ModeBase::StatusCode::kSuccess);
}

CHIP_ERROR LaundryWasherModeDelegate::GetModeLabelByIndex(uint8_t modeIndex, chip::MutableCharSpan & label)
{
    if (modeIndex >= MATTER_ARRAY_SIZE(kModeOptions))
    {
        return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
    }
    return chip::CopyCharSpanToMutableCharSpan(kModeOptions[modeIndex].label, label);
}

CHIP_ERROR LaundryWasherModeDelegate::GetModeValueByIndex(uint8_t modeIndex, uint8_t & value)
{
    if (modeIndex >= MATTER_ARRAY_SIZE(kModeOptions))
    {
        return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
    }
    value = kModeOptions[modeIndex].mode;
    return CHIP_NO_ERROR;
}

CHIP_ERROR LaundryWasherModeDelegate::GetModeTagsByIndex(uint8_t modeIndex, List<ModeTagStructType> & tags)
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

ModeBase::Instance * LaundryWasherMode::Instance()
{
    return gLaundryWasherModeInstance;
}

void LaundryWasherMode::Shutdown()
{
    if (gLaundryWasherModeInstance != nullptr)
    {
        delete gLaundryWasherModeInstance;
        gLaundryWasherModeInstance = nullptr;
    }
    if (gLaundryWasherModeDelegate != nullptr)
    {
        delete gLaundryWasherModeDelegate;
        gLaundryWasherModeDelegate = nullptr;
    }
}

chip::Protocols::InteractionModel::Status chefLaundryWasherModeWriteCallback(chip::EndpointId endpointId, chip::ClusterId clusterId,
                                                                             const EmberAfAttributeMetadata * attributeMetadata,
                                                                             uint8_t * buffer)
{
    VerifyOrReturnError(endpointId == 1 || gLaundryWasherModeInstance != nullptr,
                        chip::Protocols::InteractionModel::Status::Failure);

    chip::Protocols::InteractionModel::Status ret;
    chip::AttributeId attributeId = attributeMetadata->attributeId;

    switch (attributeId)
    {
    case chip::app::Clusters::LaundryWasherMode::Attributes::CurrentMode::Id: {
        uint8_t m = buffer[0];
        ret       = gLaundryWasherModeInstance->UpdateCurrentMode(m);
        if (chip::Protocols::InteractionModel::Status::Success != ret)
        {
            ChipLogError(DeviceLayer, "Invalid Attribute Update status: %d", static_cast<int>(ret));
        }
    }
    break;
    default:
        ret = chip::Protocols::InteractionModel::Status::UnsupportedWrite;
        ChipLogError(DeviceLayer, "Unsupported Attribute ID: %d", static_cast<int>(attributeId));
        break;
    }

    return ret;
}

chip::Protocols::InteractionModel::Status chefLaundryWasherModeReadCallback(chip::EndpointId endpointId, chip::ClusterId clusterId,
                                                                            const EmberAfAttributeMetadata * attributeMetadata,
                                                                            uint8_t * buffer, uint16_t maxReadLength)
{
    VerifyOrReturnValue(maxReadLength > 0, chip::Protocols::InteractionModel::Status::ResourceExhausted);

    chip::Protocols::InteractionModel::Status ret = chip::Protocols::InteractionModel::Status::Success;
    chip::AttributeId attributeId                 = attributeMetadata->attributeId;

    switch (attributeId)
    {
    case chip::app::Clusters::LaundryWasherMode::Attributes::CurrentMode::Id: {
        *buffer = gLaundryWasherModeInstance->GetCurrentMode();
        ChipLogDetail(DeviceLayer, "Reading LaundryWasherMode CurrentMode : %d", static_cast<int>(attributeId));
    }
    break;
    default:
        ret = chip::Protocols::InteractionModel::Status::UnsupportedRead;
        ChipLogDetail(DeviceLayer, "Unsupported attributeId %d from reading RvcCleanMode", static_cast<int>(attributeId));
        break;
    }

    return ret;
}

void emberAfLaundryWasherModeClusterInitCallback(chip::EndpointId endpointId)
{
    VerifyOrDie(endpointId == 1); // this cluster is only enabled for endpoint 1.
    VerifyOrDie(gLaundryWasherModeDelegate == nullptr && gLaundryWasherModeInstance == nullptr);
    gLaundryWasherModeDelegate = new LaundryWasherMode::LaundryWasherModeDelegate;
    gLaundryWasherModeInstance =
        new ModeBase::Instance(gLaundryWasherModeDelegate, 0x1, LaundryWasherMode::Id, chip::to_underlying(Feature::kOnOff));
    gLaundryWasherModeInstance->Init();
}
#endif // MATTER_DM_PLUGIN_LAUNDRY_WASHER_MODE_SERVER
