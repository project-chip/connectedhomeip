/*
 *
 *    Copyright (c) 2023-2026 Project CHIP Authors
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
    uint8_t currentMode = GetInstance()->GetCurrentMode();

    // Our business logic states that we can only switch into the mapping state from the idle state.
    if (NewMode == RvcRunMode::ModeMapping && currentMode != RvcRunMode::ModeIdle)
    {
        response.status = to_underlying(ModeBase::StatusCode::kGenericFailure);
        response.statusText.SetValue("Change to the mapping mode is only allowed from idle"_span);
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

void MatterRvcRunModeClusterInitCallback(chip::EndpointId endpointId)
{
    VerifyOrDie(endpointId == 1); // this cluster is only enabled for endpoint 1.
    VerifyOrDie(!gRvcRunModeDelegate && !gRvcRunModeInstance);

    gRvcRunModeDelegate = std::make_unique<RvcRunModeDelegate>();
    gRvcRunModeInstance =
        std::make_unique<ModeBase::Instance>(gRvcRunModeDelegate.get(), endpointId, RvcRunMode::Id, 0 /* No feature bits */);
    TEMPORARY_RETURN_IGNORED gRvcRunModeInstance->Init();
}

void MatterRvcRunModeClusterShutdownCallback(chip::EndpointId endpointId, MatterClusterShutdownType)
{
    VerifyOrDie(endpointId == 1); // this cluster is only enabled for endpoint 1.
    if (gRvcRunModeInstance)
    {
        gRvcRunModeInstance->Shutdown();
    }
    RvcRunMode::Shutdown();
}

#endif // MATTER_DM_PLUGIN_RVC_RUN_MODE_SERVER

#ifdef MATTER_DM_PLUGIN_RVC_CLEAN_MODE_SERVER
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
        response.statusText.SetValue("Cannot change the cleaning mode during a clean"_span);
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

void MatterRvcCleanModeClusterInitCallback(chip::EndpointId endpointId)
{
    VerifyOrDie(endpointId == 1); // this cluster is only enabled for endpoint 1.
    VerifyOrDie(!gRvcCleanModeDelegate && !gRvcCleanModeInstance);

    gRvcCleanModeDelegate = std::make_unique<RvcCleanModeDelegate>();
    gRvcCleanModeInstance =
        std::make_unique<ModeBase::Instance>(gRvcCleanModeDelegate.get(), endpointId, RvcCleanMode::Id, 0 /* No feature bits */);
    TEMPORARY_RETURN_IGNORED gRvcCleanModeInstance->Init();
}

void MatterRvcCleanModeClusterShutdownCallback(chip::EndpointId endpointId, MatterClusterShutdownType)
{
    VerifyOrDie(endpointId == 1); // this cluster is only enabled for endpoint 1.
    if (gRvcCleanModeInstance)
    {
        gRvcCleanModeInstance->Shutdown();
    }
    RvcCleanMode::Shutdown();
}

#endif // MATTER_DM_PLUGIN_RVC_CLEAN_MODE_SERVER
