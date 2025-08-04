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
#include <chef-rvc-mode-delegate.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using chip::Protocols::InteractionModel::Status;
template <typename T>
using List              = chip::app::DataModel::List<T>;
using ModeTagStructType = chip::app::Clusters::detail::Structs::ModeTagStruct::Type;

#ifdef MATTER_DM_PLUGIN_RVC_RUN_MODE_SERVER
#include <chef-rvc-mode-delegate.h>

#ifdef MATTER_DM_PLUGIN_RVC_OPERATIONAL_STATE_SERVER
#include <chef-rvc-operational-state-delegate.h>
#endif // MATTER_DM_PLUGIN_RVC_OPERATIONAL_STATE_SERVER

using namespace chip::app::Clusters::RvcRunMode;

static std::unique_ptr<RvcRunModeDelegate> gRvcRunModeDelegate;
static std::unique_ptr<ModeBase::Instance> gRvcRunModeInstance;

chip::app::Clusters::ModeBase::Instance * getRvcRunModeInstance()
{
    return gRvcRunModeInstance.get();
}

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

#ifdef MATTER_DM_PLUGIN_RVC_OPERATIONAL_STATE_SERVER
    OperationalState::GenericOperationalError err(to_underlying(OperationalState::ErrorStateEnum::kNoError));
    if (NewMode == RvcRunMode::ModeIdle)
    {
        if (currentMode != RvcRunMode::ModeIdle)
        { // Stop existing cycle when going from cleaning/mapping to idle.
            ChipLogProgress(DeviceLayer, "Stopping RVC cycle: %d", currentMode);
            getRvcOperationalStateDelegate()->HandleStopStateCallback(err);
        }
    }
    else
    {
        if (currentMode == RvcRunMode::ModeIdle)
        { // Start a new cycle when going from idle to clening/mapping.
            ChipLogProgress(DeviceLayer, "Starting new RVC cycle: %d", NewMode);
            getRvcOperationalStateDelegate()->HandleStartStateCallback(err);
        }
    }
    if (err.IsEqual(OperationalState::GenericOperationalError(to_underlying(OperationalState::ErrorStateEnum::kNoError))))
    {
        response.status = to_underlying(ModeBase::StatusCode::kSuccess);
    }
    else
    {
        response.status = to_underlying(ModeBase::StatusCode::kGenericFailure);
    }
#else
    response.status = to_underlying(ModeBase::StatusCode::kSuccess);
#endif // MATTER_DM_PLUGIN_RVC_OPERATIONAL_STATE_SERVER
}

CHIP_ERROR RvcRunModeDelegate::GetModeLabelByIndex(uint8_t modeIndex, chip::MutableCharSpan & label)
{
    if (modeIndex >= MATTER_ARRAY_SIZE(kModeOptions))
    {
        return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
    }
    return chip::CopyCharSpanToMutableCharSpan(kModeOptions[modeIndex].label, label);
}

CHIP_ERROR RvcRunModeDelegate::GetModeValueByIndex(uint8_t modeIndex, uint8_t & value)
{
    if (modeIndex >= MATTER_ARRAY_SIZE(kModeOptions))
    {
        return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
    }
    value = kModeOptions[modeIndex].mode;
    return CHIP_NO_ERROR;
}

CHIP_ERROR RvcRunModeDelegate::GetModeTagsByIndex(uint8_t modeIndex, List<ModeTagStructType> & tags)
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

void RvcRunMode::Shutdown()
{
    gRvcRunModeInstance.reset();
    gRvcRunModeDelegate.reset();
}

chip::Protocols::InteractionModel::Status chefRvcRunModeWriteCallback(chip::EndpointId endpointId, chip::ClusterId clusterId,
                                                                      const EmberAfAttributeMetadata * attributeMetadata,
                                                                      uint8_t * buffer)
{
    VerifyOrDie(endpointId == 1); // this cluster is only enabled for endpoint 1
    VerifyOrDie(gRvcRunModeInstance != nullptr);
    chip::Protocols::InteractionModel::Status ret;
    chip::AttributeId attributeId = attributeMetadata->attributeId;

    switch (attributeId)
    {
    case chip::app::Clusters::RvcRunMode::Attributes::CurrentMode::Id: {
        uint8_t m = static_cast<uint8_t>(buffer[0]);
        ret       = gRvcRunModeInstance->UpdateCurrentMode(m);
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

chip::Protocols::InteractionModel::Status chefRvcRunModeReadCallback(chip::EndpointId endpointId, chip::ClusterId clusterId,
                                                                     const EmberAfAttributeMetadata * attributeMetadata,
                                                                     uint8_t * buffer, uint16_t maxReadLength)
{
    chip::Protocols::InteractionModel::Status ret = chip::Protocols::InteractionModel::Status::Success;
    chip::AttributeId attributeId                 = attributeMetadata->attributeId;

    switch (attributeId)
    {
    case chip::app::Clusters::RvcRunMode::Attributes::CurrentMode::Id: {
        *buffer = gRvcRunModeInstance->GetCurrentMode();
        ChipLogDetail(DeviceLayer, "Reading RunMode CurrentMode : %d", static_cast<int>(attributeId));
    }
    break;
    default:
        ret = chip::Protocols::InteractionModel::Status::UnsupportedRead;
        ChipLogDetail(DeviceLayer, "Unsupported attributeId %d from reading RvcRunMode", static_cast<int>(attributeId));
        break;
    }

    return ret;
}

void emberAfRvcRunModeClusterInitCallback(chip::EndpointId endpointId)
{
    VerifyOrDie(endpointId == 1); // this cluster is only enabled for endpoint 1.
    VerifyOrDie(!gRvcRunModeDelegate && !gRvcRunModeInstance);

    gRvcRunModeDelegate = std::make_unique<RvcRunModeDelegate>();
    gRvcRunModeInstance =
        std::make_unique<ModeBase::Instance>(gRvcRunModeDelegate.get(), endpointId, RvcRunMode::Id, 0 /* No feature bits */);
    gRvcRunModeInstance->Init();
}

#endif // MATTER_DM_PLUGIN_RVC_RUN_MODE_SERVER

#ifdef MATTER_DM_PLUGIN_RVC_CLEAN_MODE_SERVER
#include <chef-rvc-mode-delegate.h>
using namespace chip::app::Clusters::RvcCleanMode;
static std::unique_ptr<RvcCleanModeDelegate> gRvcCleanModeDelegate;
static std::unique_ptr<ModeBase::Instance> gRvcCleanModeInstance;

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
    if (modeIndex >= MATTER_ARRAY_SIZE(kModeOptions))
    {
        return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
    }
    return chip::CopyCharSpanToMutableCharSpan(kModeOptions[modeIndex].label, label);
}

CHIP_ERROR RvcCleanModeDelegate::GetModeValueByIndex(uint8_t modeIndex, uint8_t & value)
{
    if (modeIndex >= MATTER_ARRAY_SIZE(kModeOptions))
    {
        return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
    }
    value = kModeOptions[modeIndex].mode;
    return CHIP_NO_ERROR;
}

CHIP_ERROR RvcCleanModeDelegate::GetModeTagsByIndex(uint8_t modeIndex, List<ModeTagStructType> & tags)
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

void RvcCleanMode::Shutdown()
{
    gRvcCleanModeInstance.reset();
    gRvcCleanModeDelegate.reset();
}

chip::Protocols::InteractionModel::Status chefRvcCleanModeWriteCallback(chip::EndpointId endpointId, chip::ClusterId clusterId,
                                                                        const EmberAfAttributeMetadata * attributeMetadata,
                                                                        uint8_t * buffer)
{
    VerifyOrDie(endpointId == 1); // this cluster is only enabled for endpoint 1
    VerifyOrDie(gRvcCleanModeInstance != nullptr);
    chip::Protocols::InteractionModel::Status ret;
    chip::AttributeId attributeId = attributeMetadata->attributeId;

    switch (attributeId)
    {
    case chip::app::Clusters::RvcCleanMode::Attributes::CurrentMode::Id: {
        uint8_t m = static_cast<uint8_t>(buffer[0]);
        ret       = gRvcCleanModeInstance->UpdateCurrentMode(m);
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

chip::Protocols::InteractionModel::Status chefRvcCleanModeReadCallback(chip::EndpointId endpointId, chip::ClusterId clusterId,
                                                                       const EmberAfAttributeMetadata * attributeMetadata,
                                                                       uint8_t * buffer, uint16_t maxReadLength)
{
    VerifyOrReturnValue(maxReadLength > 0, chip::Protocols::InteractionModel::Status::ResourceExhausted);
    buffer[0] = gRvcCleanModeInstance->GetCurrentMode();

    chip::Protocols::InteractionModel::Status ret = chip::Protocols::InteractionModel::Status::Success;
    chip::AttributeId attributeId                 = attributeMetadata->attributeId;

    switch (attributeId)
    {
    case chip::app::Clusters::RvcCleanMode::Attributes::CurrentMode::Id: {
        *buffer = gRvcCleanModeInstance->GetCurrentMode();
        ChipLogDetail(DeviceLayer, "Reading CleanMode CurrentMode : %d", static_cast<int>(attributeId));
    }
    break;
    default:
        ret = chip::Protocols::InteractionModel::Status::UnsupportedRead;
        ChipLogDetail(DeviceLayer, "Unsupported attributeId %d from reading RvcCleanMode", static_cast<int>(attributeId));
        break;
    }

    return ret;
}

void emberAfRvcCleanModeClusterInitCallback(chip::EndpointId endpointId)
{
    VerifyOrDie(endpointId == 1); // this cluster is only enabled for endpoint 1.
    VerifyOrDie(!gRvcCleanModeDelegate && !gRvcCleanModeInstance);

    gRvcCleanModeDelegate = std::make_unique<RvcCleanModeDelegate>();
    gRvcCleanModeInstance =
        std::make_unique<ModeBase::Instance>(gRvcCleanModeDelegate.get(), endpointId, RvcCleanMode::Id, 0 /* No feature bits */);
    gRvcCleanModeInstance->Init();
}
#endif // MATTER_DM_PLUGIN_RVC_CLEAN_MODE_SERVER
