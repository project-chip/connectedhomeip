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

void MatterRefrigeratorAndTemperatureControlledCabinetModeClusterInitCallback(chip::EndpointId endpointId)
{
    VerifyOrDie(endpointId == 1); // this cluster is only enabled for endpoint 1.
    VerifyOrDie(gTccModeDelegate == nullptr && gTccModeInstance == nullptr);
    gTccModeDelegate = std::make_unique<RefrigeratorAndTemperatureControlledCabinetMode::TccModeDelegate>();
    gTccModeInstance =
        std::make_unique<ModeBase::Instance>(gTccModeDelegate.get(), 0x1, RefrigeratorAndTemperatureControlledCabinetMode::Id, 0);
    TEMPORARY_RETURN_IGNORED gTccModeInstance->Init();
}

void MatterRefrigeratorAndTemperatureControlledCabinetModeClusterShutdownCallback(chip::EndpointId endpointId,
                                                                                  MatterClusterShutdownType)
{
    VerifyOrDie(endpointId == 1); // this cluster is only enabled for endpoint 1.
    if (gTccModeInstance)
    {
        gTccModeInstance->Shutdown();
    }
    RefrigeratorAndTemperatureControlledCabinetMode::Shutdown();
}

#endif // MATTER_DM_PLUGIN_REFRIGERATOR_AND_TEMPERATURE_CONTROLLED_CABINET_MODE_SERVER
