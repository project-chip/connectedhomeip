/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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

#include <thermostat-mode-delegate-impl.h>

using namespace chip::app::Clusters;
using namespace chip::app::Clusters::ThermostatMode;
using chip::to_underlying;
using chip::Protocols::InteractionModel::Status;

template <typename T>
using List              = chip::app::DataModel::List<T>;
using ModeTagStructType = chip::app::Clusters::detail::Structs::ModeTagStruct::Type;

namespace {

ThermostatModeDelegate * gThermostatModeDelegate = nullptr;
ModeBase::Instance * gThermostatModeInstance     = nullptr;

} // namespace

CHIP_ERROR ThermostatModeDelegate::Init()
{
    return CHIP_NO_ERROR;
}

void ThermostatModeDelegate::HandleChangeToMode(uint8_t mode, ModeBase::Commands::ChangeToModeResponse::Type & response)
{
    if (GetInstance() != nullptr && GetInstance()->GetFailTransition())
    {
        response.status = to_underlying(ModeBase::StatusCode::kInvalidInMode);
        response.statusText.SetValue("Mode change not allowed due to device state"_span);
        return;
    }

    response.status = to_underlying(ModeBase::StatusCode::kSuccess);
}

void ThermostatModeDelegate::HandleChangeToModeByCoreTag(uint16_t newModeTag, uint8_t & newMode,
                                                         ModeBase::Commands::ChangeToModeResponse::Type & response)
{
    HandleChangeToMode(newMode, response);
}

CHIP_ERROR ThermostatModeDelegate::GetModeLabelByIndex(uint8_t modeIndex, chip::MutableCharSpan & label)
{
    if (modeIndex >= MATTER_ARRAY_SIZE(kModeOptions))
    {
        return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
    }
    return chip::CopyCharSpanToMutableCharSpan(kModeOptions[modeIndex].label, label);
}

CHIP_ERROR ThermostatModeDelegate::GetModeValueByIndex(uint8_t modeIndex, uint8_t & value)
{
    if (modeIndex >= MATTER_ARRAY_SIZE(kModeOptions))
    {
        return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
    }
    value = kModeOptions[modeIndex].mode;
    return CHIP_NO_ERROR;
}

CHIP_ERROR ThermostatModeDelegate::GetModeTagsByIndex(uint8_t modeIndex, List<ModeTagStructType> & tags)
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

CHIP_ERROR ThermostatModeDelegate::GetCoreModeTagByIndex(uint8_t tagIndex, uint16_t & tag)
{
    static constexpr uint16_t kCoreModeTags[] = {
        to_underlying(ModeTag::kAuto),
        to_underlying(ModeTag::kOff),
        to_underlying(ModeTag::kCool),
        to_underlying(ModeTag::kHeat),
    };

    if (tagIndex >= MATTER_ARRAY_SIZE(kCoreModeTags))
    {
        return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
    }

    tag = kCoreModeTags[tagIndex];
    return CHIP_NO_ERROR;
}

ModeBase::Instance * ThermostatMode::Instance()
{
    return gThermostatModeInstance;
}

void ThermostatMode::Shutdown()
{
    if (gThermostatModeInstance != nullptr)
    {
        delete gThermostatModeInstance;
        gThermostatModeInstance = nullptr;
    }
    if (gThermostatModeDelegate != nullptr)
    {
        delete gThermostatModeDelegate;
        gThermostatModeDelegate = nullptr;
    }
}

CHIP_ERROR ThermostatMode::Init(EndpointId endpointId)
{
    if (gThermostatModeDelegate == nullptr && gThermostatModeInstance == nullptr)
    {
        MatterThermostatModeClusterInitCallback(endpointId);
    }
    return CHIP_NO_ERROR;
}

void MatterThermostatModeClusterInitCallback(chip::EndpointId endpointId)
{
    VerifyOrDie(endpointId == 1); // this cluster is only enabled for endpoint 1.
    VerifyOrDie(gThermostatModeDelegate == nullptr && gThermostatModeInstance == nullptr);
    gThermostatModeDelegate = new ThermostatMode::ThermostatModeDelegate;
    gThermostatModeInstance = new ModeBase::Instance(gThermostatModeDelegate, endpointId, ThermostatMode::Id,
                                                     to_underlying(ThermostatMode::Feature::kCoreModes));
    TEMPORARY_RETURN_IGNORED gThermostatModeInstance->Init();
}

void MatterThermostatModeClusterShutdownCallback(chip::EndpointId endpointId, MatterClusterShutdownType)
{
    VerifyOrDie(endpointId == 1);
    if (gThermostatModeInstance != nullptr)
    {
        gThermostatModeInstance->Shutdown();
    }
    ThermostatMode::Shutdown();
}
