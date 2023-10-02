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

using namespace chip::app::Clusters;
using chip::Protocols::InteractionModel::Status;
template <typename T>
using List              = chip::app::DataModel::List<T>;
using ModeTagStructType = chip::app::Clusters::detail::Structs::ModeTagStruct::Type;

#ifdef ZCL_USING_RVC_RUN_MODE_CLUSTER_SERVER
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

void emberAfRvcRunModeClusterInitCallback(chip::EndpointId endpointId)
{
    VerifyOrDie(endpointId == 1); // this cluster is only enabled for endpoint 1.
    VerifyOrDie(gRvcRunModeDelegate == nullptr && gRvcRunModeInstance == nullptr);
    gRvcRunModeDelegate = new RvcRunMode::RvcRunModeDelegate;
    gRvcRunModeInstance =
        new ModeBase::Instance(gRvcRunModeDelegate, 0x1, RvcRunMode::Id, chip::to_underlying(RvcRunMode::Feature::kOnOff));
    gRvcRunModeInstance->Init();
}

#ifdef ZCL_USING_RVC_CLEAN_MODE_CLUSTER_SERVER
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

void emberAfRvcCleanModeClusterInitCallback(chip::EndpointId endpointId)
{
    VerifyOrDie(endpointId == 1); // this cluster is only enabled for endpoint 1.
    VerifyOrDie(gRvcCleanModeDelegate == nullptr && gRvcCleanModeInstance == nullptr);
    gRvcCleanModeDelegate = new RvcCleanMode::RvcCleanModeDelegate;
    gRvcCleanModeInstance =
        new ModeBase::Instance(gRvcCleanModeDelegate, 0x1, RvcCleanMode::Id, chip::to_underlying(RvcCleanMode::Feature::kOnOff));
    gRvcCleanModeInstance->Init();
}
#endif // ZCL_USING_RVC_CLEAN_MODE_CLUSTER_SERVER
#endif // ZCL_USING_RVC_RUN_MODE_CLUSTER_SERVER
