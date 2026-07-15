/*
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

#include <device/types/humidity-conditioner/impl/LoggingHumidityConditioner.h>

#include <lib/support/logging/CHIPLogging.h>

using namespace chip::app::Clusters;

namespace chip {
namespace app {
namespace {

constexpr uint16_t kDefaultMeasuredHumidity = 5000; // 50.00%

const RelativeHumidityMeasurementCluster::Config kDefaultHumidityConfig = []() {
    RelativeHumidityMeasurementCluster::Config config;
    config.minMeasuredValue = DataModel::MakeNullable<uint16_t>(0);
    config.maxMeasuredValue = DataModel::MakeNullable<uint16_t>(10000);
    config.WithTolerance(100);
    return config;
}();

const HumidistatCluster::StartupConfiguration kDefaultHumidistatConfig = []() {
    HumidistatCluster::StartupConfiguration config;
    config.mode           = Humidistat::ModeEnum::kAuto;
    config.systemState    = Humidistat::SystemStateEnum::kIdle;
    config.userSetpoint   = 45;
    config.minSetpoint    = 20;
    config.maxSetpoint    = 80;
    config.step           = 5;
    config.targetSetpoint = 45;
    return config;
}();

} // namespace

LoggingHumidityConditioner::LoggingHumidityConditioner(TimerDelegate & timerDelegate) :
    HumidityConditioner(timerDelegate,
                        BitFlags<Humidistat::Feature>(Humidistat::Feature::kHumidifier, Humidistat::Feature::kDehumidifier,
                                                      Humidistat::Feature::kSensor, Humidistat::Feature::kContinuous,
                                                      Humidistat::Feature::kOptimal, Humidistat::Feature::kAuto,
                                                      Humidistat::Feature::kColdMist, Humidistat::Feature::kCondPump),
                        HumidistatCluster::OptionalAttributeSet().Set<Humidistat::Attributes::Sleep::Id>().Set<
                            Humidistat::Attributes::TargetSetpoint::Id>(),
                        kDefaultHumidistatConfig, kDefaultHumidityConfig)
{}

CHIP_ERROR LoggingHumidityConditioner::Register(EndpointId endpoint, CodeDrivenDataModelProvider & provider,
                                                EndpointComposition composition)
{
    ReturnErrorOnFailure(HumidityConditioner::Register(endpoint, provider, composition));

    HumidistatCluster().SetDelegate(this);
    ReturnErrorOnFailure(RelativeHumidityMeasurementCluster().SetMeasuredValue(DataModel::MakeNullable(kDefaultMeasuredHumidity)));
    return CHIP_NO_ERROR;
}

void LoggingHumidityConditioner::OnModeChanged(Humidistat::ModeEnum newMode)
{
    ChipLogProgress(DeviceLayer, "LoggingHumidityConditioner::OnModeChanged() -> %u",
                    static_cast<unsigned>(to_underlying(newMode)));
}

void LoggingHumidityConditioner::OnSystemStateChanged(Humidistat::SystemStateEnum newSystemState)
{
    ChipLogProgress(DeviceLayer, "LoggingHumidityConditioner::OnSystemStateChanged() -> %u",
                    static_cast<unsigned>(to_underlying(newSystemState)));
}

void LoggingHumidityConditioner::OnUserSetpointChanged(chip::Percent newUserSetpoint)
{
    ChipLogProgress(DeviceLayer, "LoggingHumidityConditioner::OnUserSetpointChanged() -> %u",
                    static_cast<unsigned>(newUserSetpoint));
}

void LoggingHumidityConditioner::OnTargetSetpointChanged(chip::Percent newTargetSetpoint)
{
    ChipLogProgress(DeviceLayer, "LoggingHumidityConditioner::OnTargetSetpointChanged() -> %u",
                    static_cast<unsigned>(newTargetSetpoint));
}

void LoggingHumidityConditioner::OnMistTypeChanged(BitMask<Humidistat::MistTypeBitmap> newMistType)
{
    ChipLogProgress(DeviceLayer, "LoggingHumidityConditioner::OnMistTypeChanged() -> 0x%02X", newMistType.Raw());
}

void LoggingHumidityConditioner::OnContinuousChanged(bool newContinuous)
{
    ChipLogProgress(DeviceLayer, "LoggingHumidityConditioner::OnContinuousChanged() -> %s", newContinuous ? "true" : "false");
}

void LoggingHumidityConditioner::OnSleepChanged(bool newSleep)
{
    ChipLogProgress(DeviceLayer, "LoggingHumidityConditioner::OnSleepChanged() -> %s", newSleep ? "true" : "false");
}

void LoggingHumidityConditioner::OnOptimalChanged(bool newOptimal)
{
    ChipLogProgress(DeviceLayer, "LoggingHumidityConditioner::OnOptimalChanged() -> %s", newOptimal ? "true" : "false");
}

void LoggingHumidityConditioner::OnCondPumpEnabledChanged(bool newCondPumpEnabled)
{
    ChipLogProgress(DeviceLayer, "LoggingHumidityConditioner::OnCondPumpEnabledChanged() -> %s",
                    newCondPumpEnabled ? "true" : "false");
}

void LoggingHumidityConditioner::OnCondRunCountChanged(uint16_t newCondRunCount)
{
    ChipLogProgress(DeviceLayer, "LoggingHumidityConditioner::OnCondRunCountChanged() -> %u",
                    static_cast<unsigned>(newCondRunCount));
}

} // namespace app
} // namespace chip