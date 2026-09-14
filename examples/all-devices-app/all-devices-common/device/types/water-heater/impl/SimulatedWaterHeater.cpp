/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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
 #include <clusters/WaterHeaterMode/Enums.h>
 #include <device/types/water-heater/impl/SimulatedWaterHeater.h>

 using chip::Protocols::InteractionModel::Status;
 using namespace chip::app::Clusters;
 using namespace chip::app::Clusters::WaterHeaterManagement;
 using namespace chip::app::Clusters::WaterHeaterManagement::Attributes;


namespace chip::app {

namespace {
    using ModeTagStructType = Clusters::detail::Structs::ModeTagStruct::Type;

    constexpr uint8_t kWaterHeaterModeOff = 0;
    constexpr uint8_t kWaterHeaterModeManual = 1;
    constexpr uint8_t kWaterHeaterModeTimed = 2;
    
    const ModeTagStructType kWaterHeaterModeOffTags[]     = { { .value = to_underlying(WaterHeaterMode::ModeTag::kOff) } };
    const ModeTagStructType kWaterHeaterModeManualTags[] = { { .value = to_underlying(WaterHeaterMode::ModeTag::kManual) } };
    const ModeTagStructType kWaterHeaterModeTimedTags[]  = { { .value = to_underlying(WaterHeaterMode::ModeTag::kTimed) } };
        
    struct WaterHeaterModeOption
    {
        CharSpan label;
        uint8_t value;
        Span<const ModeTagStructType> tags;
    };
    const WaterHeaterModeOption kWaterHeaterModeOptions[] = {
        { "Off"_span, kWaterHeaterModeOff, Span<const ModeTagStructType>(kWaterHeaterModeOffTags) },
        { "Manual"_span, kWaterHeaterModeManual, Span<const ModeTagStructType>(kWaterHeaterModeManualTags) },
        { "Timed"_span, kWaterHeaterModeTimed, Span<const ModeTagStructType>(kWaterHeaterModeTimedTags) },
    };
}

 SimulatedWaterHeater::SimulatedWaterHeater(const Config & config) : WaterHeater(config, *this, *this) {}

 SimulatedWaterHeater::~SimulatedWaterHeater() {
    mConfig.timerDelegate.CancelTimer(this);
 }

 void SimulatedWaterHeater::Unregister(CodeDrivenDataModelProvider & provider)
 {
    mConfig.timerDelegate.CancelTimer(this);
    WaterHeater::Unregister(provider);
 }

 void SimulatedWaterHeater::TimerFired()
{
    ChipLogProgress(AppServer, "WaterHeater: Boost duration elapsed");
    EndBoost();
}

Status SimulatedWaterHeater::HandleBoost(uint32_t duration, Optional<bool> oneShot, Optional<bool> emergencyBoost,
                                Optional<int16_t> temporarySetpoint, Optional<Percent> targetPercentage,
                                Optional<Percent> targetReheat)
{
    ChipLogProgress(AppServer, "WaterHeater: Boost duration=%" PRIu32 "s", duration);

    mConfig.timerDelegate.CancelTimer(this);

    mBoostState = Clusters::WaterHeaterManagement::BoostStateEnum::kActive;
    mHeatDemand = mHeaterTypes;

    CHIP_ERROR err =
        GenerateBoostStartedEvent(duration, oneShot, emergencyBoost, temporarySetpoint, targetPercentage, targetReheat);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "WaterHeater: Failed to generate BoostStarted event: %" CHIP_ERROR_FORMAT, err.Format());
    }

    SuccessOrDie(mConfig.timerDelegate.StartTimer(this, System::Clock::Seconds32(duration)));
    NotifyHeatDemandAndBoostStateChanged();
    return Status::Success;
}

Status SimulatedWaterHeater::HandleCancelBoost()
{
    if (mBoostState == BoostStateEnum::kInactive)
    {
        return Status::Success;
    }

    ChipLogProgress(AppServer, "WaterHeater: CancelBoost");
    EndBoost();
    return Status::Success;
}

BitMask<WaterHeaterHeatSourceBitmap> SimulatedWaterHeater::GetHeaterTypes()
{
    return mHeaterTypes;
}

BitMask<WaterHeaterHeatSourceBitmap> SimulatedWaterHeater::GetHeatDemand()
{
    return mHeatDemand;
}

uint16_t SimulatedWaterHeater::GetTankVolume()
{
    return 0;
}

Energy_mWh SimulatedWaterHeater::GetEstimatedHeatRequired()
{
    return 0;
}

Percent SimulatedWaterHeater::GetTankPercentage()
{
    return 0;
}

BoostStateEnum SimulatedWaterHeater::GetBoostState()
{
    return mBoostState;
}


void SimulatedWaterHeater::EndBoost()
{
    mConfig.timerDelegate.CancelTimer(this);
    mBoostState = BoostStateEnum::kInactive;
    mHeatDemand.ClearAll();

    CHIP_ERROR err = GenerateBoostEndedEvent();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "WaterHeater: Failed to generate BoostEnded event: %" CHIP_ERROR_FORMAT, err.Format());
    }

    NotifyHeatDemandAndBoostStateChanged();
}

void SimulatedWaterHeater::NotifyHeatDemandAndBoostStateChanged()
{
    VerifyOrReturn(mProvider != nullptr);
    mProvider->NotifyAttributeChanged({ SingleEndpoint::GetEndpointId(), Id, HeatDemand::Id },
                                      DataModel::AttributeChangeType::kReportable);
    mProvider->NotifyAttributeChanged({ SingleEndpoint::GetEndpointId(), Id, BoostState::Id },
                                      DataModel::AttributeChangeType::kReportable);
}

CHIP_ERROR SimulatedWaterHeater::Init()
{
    return CHIP_NO_ERROR;
}

CHIP_ERROR SimulatedWaterHeater::GetModeLabelByIndex(uint8_t modeIndex, MutableCharSpan & label)
{
    VerifyOrReturnError(modeIndex < MATTER_ARRAY_SIZE(kWaterHeaterModeOptions), CHIP_ERROR_PROVIDER_LIST_EXHAUSTED);
    return CopyCharSpanToMutableCharSpan(kWaterHeaterModeOptions[modeIndex].label, label);

}

CHIP_ERROR SimulatedWaterHeater::GetModeValueByIndex(uint8_t modeIndex, uint8_t & value)
{
    VerifyOrReturnError(modeIndex < MATTER_ARRAY_SIZE(kWaterHeaterModeOptions), CHIP_ERROR_PROVIDER_LIST_EXHAUSTED);
    value = kWaterHeaterModeOptions[modeIndex].value;
    return CHIP_NO_ERROR;
}

CHIP_ERROR SimulatedWaterHeater::GetModeTagsByIndex(uint8_t modeIndex, DataModel::List<Clusters::detail::Structs::ModeTagStruct::Type> & modeTags)
{
    VerifyOrReturnError(modeIndex < MATTER_ARRAY_SIZE(kWaterHeaterModeOptions), CHIP_ERROR_PROVIDER_LIST_EXHAUSTED);
    const auto & tags = kWaterHeaterModeOptions[modeIndex].tags;
    VerifyOrReturnError(modeTags.size() >= tags.size(), CHIP_ERROR_INVALID_ARGUMENT);
    std::copy(tags.begin(), tags.end(), modeTags.begin());
    modeTags.reduce_size(tags.size());
    return CHIP_NO_ERROR;
}

void SimulatedWaterHeater::HandleChangeToMode(uint8_t NewMode, Clusters::ModeBase::Commands::ChangeToModeResponse::Type & response)
{
    
}
 } // namespace chip::app