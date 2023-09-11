/*
 *
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include <app-common/zap-generated/attributes/Accessors.h>
#include <tcc-mode.h>

using namespace chip::app::Clusters;
using namespace chip::app::Clusters::RefrigeratorAndTemperatureControlledCabinetMode;
using chip::Protocols::InteractionModel::Status;
template <typename T>
using List              = chip::app::DataModel::List<T>;
using ModeTagStructType = chip::app::Clusters::detail::Structs::ModeTagStruct::Type;

static TccModeDelegate * gTccModeDelegate    = nullptr;
static ModeBase::Instance * gTccModeInstance = nullptr;

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
    if (modeIndex >= ArraySize(kModeOptions))
    {
        return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
    }
    return chip::CopyCharSpanToMutableCharSpan(kModeOptions[modeIndex].label, label);
}

CHIP_ERROR TccModeDelegate::GetModeValueByIndex(uint8_t modeIndex, uint8_t & value)
{
    if (modeIndex >= ArraySize(kModeOptions))
    {
        return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
    }
    value = kModeOptions[modeIndex].mode;
    return CHIP_NO_ERROR;
}

CHIP_ERROR TccModeDelegate::GetModeTagsByIndex(uint8_t modeIndex, List<ModeTagStructType> & tags)
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

void RefrigeratorAndTemperatureControlledCabinetMode::Shutdown()
{
    if (gTccModeInstance != nullptr)
    {
        delete gTccModeInstance;
        gTccModeInstance = nullptr;
    }
    if (gTccModeDelegate != nullptr)
    {
        delete gTccModeDelegate;
        gTccModeDelegate = nullptr;
    }
}

void emberAfRefrigeratorAndTemperatureControlledCabinetModeClusterInitCallback(chip::EndpointId endpointId)
{
    VerifyOrDie(endpointId == 1); // this cluster is only enabled for endpoint 1.
    VerifyOrDie(gTccModeDelegate == nullptr && gTccModeInstance == nullptr);
    gTccModeDelegate = new RefrigeratorAndTemperatureControlledCabinetMode::TccModeDelegate;
    gTccModeInstance = new ModeBase::Instance(gTccModeDelegate, 0x1, RefrigeratorAndTemperatureControlledCabinetMode::Id,
                                              chip::to_underlying(Feature::kOnOff));
    gTccModeInstance->Init();
}
