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
#include <app/util/config.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using chip::Protocols::InteractionModel::Status;
template <typename T>
using List              = chip::app::DataModel::List<T>;
using ModeTagStructType = chip::app::Clusters::detail::Structs::ModeTagStruct::Type;

#ifdef MATTER_DM_PLUGIN_RVC_RUN_MODE_SERVER
#include <chef-rvc-mode-delegate.h>
using namespace chip::app::Clusters::RvcRunMode;
static RvcRunModeDelegate * gRvcRunModeDelegate = nullptr;
static ModeBase::Instance * gRvcRunModeInstance = nullptr;

CHIP_ERROR RvcRunModeDelegate::Init()
{
    return CHIP_NO_ERROR;
}

void RvcRunModeDelegate::HandleChangeToMode(uint8_t NewMode, ModeBase::Commands::ChangeToModeResponse::Type & response)
{
    uint8_t currentMode = mInstance->GetCurrentMode();

    // Our business logic states that we can only switch into the mapping state from the idle state.
    if (NewMode == RvcRunMode::ModeMapping && currentMode != RvcRunMode::ModeIdle)
    {
        response.status = to_underlying(ModeBase::StatusCode::kGenericFailure);
        response.statusText.SetValue(chip::CharSpan::fromCharString("Change to the mapping mode is only allowed from idle"));
        return;
    }

    response.status = to_underlying(ModeBase::StatusCode::kSuccess);
}

CHIP_ERROR RvcRunModeDelegate::GetModeLabelByIndex(uint8_t modeIndex, chip::MutableCharSpan & label)
{
    if (modeIndex >= ArraySize(kModeOptions))
    {
        return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
    }
    return chip::CopyCharSpanToMutableCharSpan(kModeOptions[modeIndex].label, label);
}

CHIP_ERROR RvcRunModeDelegate::GetModeValueByIndex(uint8_t modeIndex, uint8_t & value)
{
    if (modeIndex >= ArraySize(kModeOptions))
    {
        return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
    }
    value = kModeOptions[modeIndex].mode;
    return CHIP_NO_ERROR;
}

CHIP_ERROR RvcRunModeDelegate::GetModeTagsByIndex(uint8_t modeIndex, List<ModeTagStructType> & tags)
{
    if (modeIndex >= ArraySize(kModeOptions))
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

ModeBase::Instance * RvcRunMode::Instance()
{
    return gRvcRunModeInstance;
}

void RvcRunMode::Shutdown()
{
    if (gRvcRunModeInstance != nullptr)
    {
        delete gRvcRunModeInstance;
        gRvcRunModeInstance = nullptr;
    }
    if (gRvcRunModeDelegate != nullptr)
    {
        delete gRvcRunModeDelegate;
        gRvcRunModeDelegate = nullptr;
    }
}

chip::Protocols::InteractionModel::Status chefRvcRunModeWriteCallback(chip::EndpointId endpointId, chip::ClusterId clusterId,
                                                                      const EmberAfAttributeMetadata * attributeMetadata,
                                                                      uint8_t * buffer)
{
    // this cluster is only enabled for endpoint 1
    if (endpointId != 1)
    {
        return chip::Protocols::InteractionModel::Status::UnsupportedEndpoint;
    }

    chip::Protocols::InteractionModel::Status ret;
    VerifyOrDie(gRvcRunModeInstance != nullptr);

    ModeBase::Instance * clusterInstance = gRvcRunModeInstance;
    chip::AttributeId attributeId        = attributeMetadata->attributeId;

    switch (attributeId)
    {
    case chip::app::Clusters::RvcRunMode::Attributes::CurrentMode::Id: {
        uint8_t m = static_cast<uint8_t>(buffer[0]);
        ret       = clusterInstance->UpdateCurrentMode(m);
        if (chip::Protocols::InteractionModel::Status::Success != ret)
        {
            ChipLogError(DeviceLayer, "Invalid Attribute Update status: %d", static_cast<int>(ret));
            break;
        }
    }
    break;
    default:
        ret = chip::Protocols::InteractionModel::Status::UnsupportedAttribute;
        ChipLogError(DeviceLayer, "Unsupported Attribute ID: %d", static_cast<int>(attributeId));
        break;
    }

    return ret;
}

chip::Protocols::InteractionModel::Status chefRvcRunModeReadCallback(chip::EndpointId endpointId, chip::ClusterId clusterId,
                                                                     const EmberAfAttributeMetadata * attributeMetadata,
                                                                     uint8_t * buffer, uint16_t maxReadLength)
{
    if (sizeof(*attributeMetadata) <= sizeof(buffer))
    {
        memcpy(buffer, &attributeMetadata, sizeof(attributeMetadata));
        return chip::Protocols::InteractionModel::Status::Success;
    }
    return chip::Protocols::InteractionModel::Status::ResourceExhausted;
}

void emberAfRvcRunModeClusterInitCallback(chip::EndpointId endpointId)
{
    VerifyOrDie(endpointId == 1); // this cluster is only enabled for endpoint 1.
    VerifyOrDie(gRvcRunModeDelegate == nullptr && gRvcRunModeInstance == nullptr);
    gRvcRunModeDelegate = new RvcRunMode::RvcRunModeDelegate;
    gRvcRunModeInstance = new ModeBase::Instance(gRvcRunModeDelegate, endpointId, RvcRunMode::Id,
                                                 chip::to_underlying(RvcRunMode::Feature::kNoFeatures));
    gRvcRunModeInstance->Init();
}

#endif // MATTER_DM_PLUGIN_RVC_RUN_MODE_SERVER

#ifdef MATTER_DM_PLUGIN_RVC_CLEAN_MODE_SERVER
#include <chef-rvc-mode-delegate.h>
using namespace chip::app::Clusters::RvcCleanMode;
static RvcCleanModeDelegate * gRvcCleanModeDelegate = nullptr;
static ModeBase::Instance * gRvcCleanModeInstance   = nullptr;

CHIP_ERROR RvcCleanModeDelegate::Init()
{
    return CHIP_NO_ERROR;
}

void RvcCleanModeDelegate::HandleChangeToMode(uint8_t NewMode, ModeBase::Commands::ChangeToModeResponse::Type & response)
{
    uint8_t rvcRunCurrentMode = gRvcRunModeInstance->GetCurrentMode();

    if (rvcRunCurrentMode == RvcRunMode::ModeCleaning)
    {
        response.status = to_underlying(RvcCleanMode::StatusCode::kCleaningInProgress);
        response.statusText.SetValue(chip::CharSpan::fromCharString("Cannot change the cleaning mode during a clean"));
        return;
    }

    response.status = to_underlying(ModeBase::StatusCode::kSuccess);
}

CHIP_ERROR RvcCleanModeDelegate::GetModeLabelByIndex(uint8_t modeIndex, chip::MutableCharSpan & label)
{
    if (modeIndex >= ArraySize(kModeOptions))
    {
        return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
    }
    return chip::CopyCharSpanToMutableCharSpan(kModeOptions[modeIndex].label, label);
}

CHIP_ERROR RvcCleanModeDelegate::GetModeValueByIndex(uint8_t modeIndex, uint8_t & value)
{
    if (modeIndex >= ArraySize(kModeOptions))
    {
        return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
    }
    value = kModeOptions[modeIndex].mode;
    return CHIP_NO_ERROR;
}

CHIP_ERROR RvcCleanModeDelegate::GetModeTagsByIndex(uint8_t modeIndex, List<ModeTagStructType> & tags)
{
    if (modeIndex >= ArraySize(kModeOptions))
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

ModeBase::Instance * RvcCleanMode::Instance()
{
    return gRvcCleanModeInstance;
}

void RvcCleanMode::Shutdown()
{
    if (gRvcCleanModeInstance != nullptr)
    {
        delete gRvcCleanModeInstance;
        gRvcCleanModeInstance = nullptr;
    }
    if (gRvcCleanModeDelegate != nullptr)
    {
        delete gRvcCleanModeDelegate;
        gRvcCleanModeDelegate = nullptr;
    }
}

chip::Protocols::InteractionModel::Status chefRvcCleanModeWriteCallback(chip::EndpointId endpointId, chip::ClusterId clusterId,
                                                                        const EmberAfAttributeMetadata * attributeMetadata,
                                                                        uint8_t * buffer)
{
    // this cluster is only enabled for endpoint 1
    if (endpointId != 1)
    {
        return chip::Protocols::InteractionModel::Status::UnsupportedEndpoint;
    }

    VerifyOrDie(gRvcCleanModeInstance != nullptr);

    chip::Protocols::InteractionModel::Status ret;
    ModeBase::Instance * clusterInstance = gRvcCleanModeInstance;
    chip::AttributeId attributeId        = attributeMetadata->attributeId;

    switch (attributeId)
    {
    case chip::app::Clusters::RvcCleanMode::Attributes::CurrentMode::Id: {
        uint8_t m                                  = static_cast<uint8_t>(buffer[0]);
        Protocols::InteractionModel::Status status = clusterInstance->UpdateCurrentMode(m);
        if (Protocols::InteractionModel::Status::Success == status)
        {
            break;
        }
        ret = chip::Protocols::InteractionModel::Status::UnsupportedWrite;
        ChipLogError(DeviceLayer, "Invalid Attribute Update status: %d", static_cast<int>(status));
    }
    break;
    default:
        ret = chip::Protocols::InteractionModel::Status::UnsupportedAttribute;
        ChipLogError(DeviceLayer, "Unsupported Attribute ID: %d", static_cast<int>(attributeId));
        break;
    }

    return ret;
}

chip::Protocols::InteractionModel::Status chefRvcCleanModeReadCallback(chip::EndpointId endpointId, chip::ClusterId clusterId,
                                                                       const EmberAfAttributeMetadata * attributeMetadata,
                                                                       uint8_t * buffer, uint16_t maxReadLength)
{
    if (sizeof(*attributeMetadata) <= sizeof(buffer))
    {
        memcpy(buffer, &attributeMetadata, sizeof(attributeMetadata));
        return chip::Protocols::InteractionModel::Status::Success;
    }
    return chip::Protocols::InteractionModel::Status::ResourceExhausted;
}

void emberAfRvcCleanModeClusterInitCallback(chip::EndpointId endpointId)
{
    VerifyOrDie(endpointId == 1); // this cluster is only enabled for endpoint 1.
    VerifyOrDie(gRvcCleanModeDelegate == nullptr && gRvcCleanModeInstance == nullptr);
    gRvcCleanModeDelegate = new RvcCleanMode::RvcCleanModeDelegate;
    gRvcCleanModeInstance = new ModeBase::Instance(gRvcCleanModeDelegate, 0x1, RvcCleanMode::Id,
                                                   chip::to_underlying(RvcCleanMode::Feature::kNoFeatures));
    gRvcCleanModeInstance->Init();
}
#endif // MATTER_DM_PLUGIN_RVC_CLEAN_MODE_SERVER
