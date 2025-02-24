/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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
#include <energy-evse-modes.h>

using namespace chip::app::Clusters;
using namespace chip::app::Clusters::EnergyEvseMode;
using chip::Protocols::InteractionModel::Status;
template <typename T>
using List              = chip::app::DataModel::List<T>;
using ModeTagStructType = chip::app::Clusters::detail::Structs::ModeTagStruct::Type;

static std::unique_ptr<EnergyEvseModeDelegate> gEnergyEvseModeDelegate;
static std::unique_ptr<ModeBase::Instance> gEnergyEvseModeInstance;

CHIP_ERROR EnergyEvseModeDelegate::Init()
{
    return CHIP_NO_ERROR;
}

void EnergyEvseModeDelegate::HandleChangeToMode(uint8_t NewMode, ModeBase::Commands::ChangeToModeResponse::Type & response)
{
    response.status = to_underlying(ModeBase::StatusCode::kSuccess);
}

CHIP_ERROR EnergyEvseModeDelegate::GetModeLabelByIndex(uint8_t modeIndex, chip::MutableCharSpan & label)
{
    if (modeIndex >= MATTER_ARRAY_SIZE(kModeOptions))
    {
        return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
    }
    return chip::CopyCharSpanToMutableCharSpan(kModeOptions[modeIndex].label, label);
}

CHIP_ERROR EnergyEvseModeDelegate::GetModeValueByIndex(uint8_t modeIndex, uint8_t & value)
{
    if (modeIndex >= MATTER_ARRAY_SIZE(kModeOptions))
    {
        return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
    }
    value = kModeOptions[modeIndex].mode;
    return CHIP_NO_ERROR;
}

CHIP_ERROR EnergyEvseModeDelegate::GetModeTagsByIndex(uint8_t modeIndex, List<ModeTagStructType> & tags)
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

ModeBase::Instance * EnergyEvseMode::Instance()
{
    return gEnergyEvseModeInstance.get();
}

void EnergyEvseMode::Shutdown()
{
    gEnergyEvseModeInstance.reset();
    gEnergyEvseModeDelegate.reset();
}

void emberAfEnergyEvseModeClusterInitCallback(chip::EndpointId endpointId)
{
    VerifyOrDie(!gEnergyEvseModeDelegate && !gEnergyEvseModeInstance);
    gEnergyEvseModeDelegate = std::make_unique<EnergyEvseMode::EnergyEvseModeDelegate>();
    gEnergyEvseModeInstance = std::make_unique<ModeBase::Instance>(gEnergyEvseModeDelegate.get(), 0x1, EnergyEvseMode::Id, 0);
    gEnergyEvseModeInstance->Init();
}
