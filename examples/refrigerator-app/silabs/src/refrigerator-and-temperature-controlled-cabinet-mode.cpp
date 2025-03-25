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
#include <refrigerator-and-temperature-controlled-cabinet-mode.h>

using namespace chip::app::Clusters;
using namespace chip::app::Clusters::RefrigeratorAndTemperatureControlledCabinetMode;
using chip::Protocols::InteractionModel::Status;
template <typename T>
using List              = chip::app::DataModel::List<T>;
using ModeTagStructType = chip::app::Clusters::detail::Structs::ModeTagStruct::Type;

static RefrigeratorAndTemperatureControlledCabinetModeDelegate * gRefrigeratorAndTemperatureControlledCabinetModeDelegate = nullptr;
static ModeBase::Instance * gRefrigeratorAndTemperatureControlledCabinetModeInstance                                      = nullptr;

CHIP_ERROR RefrigeratorAndTemperatureControlledCabinetModeDelegate::Init()
{
    return CHIP_NO_ERROR;
}

void RefrigeratorAndTemperatureControlledCabinetModeDelegate::HandleChangeToMode(
    uint8_t NewMode, ModeBase::Commands::ChangeToModeResponse::Type & response)
{
    response.status = to_underlying(ModeBase::StatusCode::kSuccess);
}

CHIP_ERROR RefrigeratorAndTemperatureControlledCabinetModeDelegate::GetModeLabelByIndex(uint8_t modeIndex,
                                                                                        chip::MutableCharSpan & label)
{
    if (modeIndex >= MATTER_ARRAY_SIZE(kModeOptions))
    {
        return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
    }
    return chip::CopyCharSpanToMutableCharSpan(kModeOptions[modeIndex].label, label);
}

CHIP_ERROR RefrigeratorAndTemperatureControlledCabinetModeDelegate::GetModeValueByIndex(uint8_t modeIndex, uint8_t & value)
{
    if (modeIndex >= MATTER_ARRAY_SIZE(kModeOptions))
    {
        return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
    }
    value = kModeOptions[modeIndex].mode;
    return CHIP_NO_ERROR;
}

CHIP_ERROR RefrigeratorAndTemperatureControlledCabinetModeDelegate::GetModeTagsByIndex(uint8_t modeIndex,
                                                                                       List<ModeTagStructType> & tags)
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

ModeBase::Instance * RefrigeratorAndTemperatureControlledCabinetMode::Instance()
{
    return gRefrigeratorAndTemperatureControlledCabinetModeInstance;
}

void RefrigeratorAndTemperatureControlledCabinetMode::Shutdown()
{
    if (gRefrigeratorAndTemperatureControlledCabinetModeInstance != nullptr)
    {
        delete gRefrigeratorAndTemperatureControlledCabinetModeInstance;
        gRefrigeratorAndTemperatureControlledCabinetModeInstance = nullptr;
    }
    if (gRefrigeratorAndTemperatureControlledCabinetModeDelegate != nullptr)
    {
        delete gRefrigeratorAndTemperatureControlledCabinetModeDelegate;
        gRefrigeratorAndTemperatureControlledCabinetModeDelegate = nullptr;
    }
}

void emberAfRefrigeratorAndTemperatureControlledCabinetModeClusterInitCallback(chip::EndpointId endpointId)
{
    VerifyOrDie(endpointId == 1); // this cluster is only enabled for endpoint 1.
    VerifyOrDie(gRefrigeratorAndTemperatureControlledCabinetModeDelegate == nullptr &&
                gRefrigeratorAndTemperatureControlledCabinetModeInstance == nullptr);
    gRefrigeratorAndTemperatureControlledCabinetModeDelegate =
        new RefrigeratorAndTemperatureControlledCabinetMode::RefrigeratorAndTemperatureControlledCabinetModeDelegate;
    gRefrigeratorAndTemperatureControlledCabinetModeInstance =
        new ModeBase::Instance(gRefrigeratorAndTemperatureControlledCabinetModeDelegate, 0x1,
                               RefrigeratorAndTemperatureControlledCabinetMode::Id, chip::to_underlying(Feature::kOnOff));
    gRefrigeratorAndTemperatureControlledCabinetModeInstance->Init();
}
