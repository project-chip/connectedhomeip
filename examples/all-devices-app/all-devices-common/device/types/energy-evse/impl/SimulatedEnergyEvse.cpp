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

#include <device/types/energy-evse/impl/SimulatedEnergyEvse.h>

#include <device/types/energy-evse/impl/SimulatedEnergyEvseTriggers.h>

#include <lib/support/BitMask.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>

namespace chip::app {

namespace {

using namespace Clusters::EnergyEvse;
using namespace Clusters::EnergyEvseMode;
using ModeTagStructType = Clusters::detail::Structs::ModeTagStruct::Type;

constexpr uint8_t kModeManual                    = 0;
constexpr uint8_t kModeTimeOfUse                 = 1;
constexpr uint8_t kModeSolarCharging             = 2;
constexpr uint8_t kModeTimeOfUseAndSolarCharging = 3;

const ModeTagStructType kModeTagsManual[]                    = { { .value = to_underlying(ModeTag::kManual) } };
const ModeTagStructType kModeTagsTimeOfUse[]                 = { { .value = to_underlying(ModeTag::kTimeOfUse) } };
const ModeTagStructType kModeTagsSolarCharging[]             = { { .value = to_underlying(ModeTag::kSolarCharging) } };
const ModeTagStructType kModeTagsTimeOfUseAndSolarCharging[] = {
    { .value = to_underlying(ModeTag::kTimeOfUse) },
    { .value = to_underlying(ModeTag::kSolarCharging) },
};

struct ModeOptionEntry
{
    CharSpan label;
    uint8_t mode;
    Span<const ModeTagStructType> modeTags;
};

const ModeOptionEntry kEvseModeOptions[] = {
    { "Manual"_span, kModeManual, Span(kModeTagsManual) },
    { "Auto-scheduled"_span, kModeTimeOfUse, Span(kModeTagsTimeOfUse) },
    { "Solar"_span, kModeSolarCharging, Span(kModeTagsSolarCharging) },
    { "Auto-scheduled with Solar charging"_span, kModeTimeOfUseAndSolarCharging, Span(kModeTagsTimeOfUseAndSolarCharging) },
};

constexpr uint64_t kEndpointMask = 0x0000FFFF00000000;

EndpointId GetEndpointIdFromEventTrigger(uint64_t eventTrigger)
{
    return static_cast<EndpointId>((eventTrigger & kEndpointMask) >> 32);
}

} // namespace

SimulatedEnergyEvse::SimulatedEnergyEvse(const Context & context) :
    EnergyEvse(EnergyEvse::Config{
        .timerDelegate          = context.timerDelegate,
        .diagnosticDataProvider = context.diagnosticDataProvider,
        .evseDelegate           = *static_cast<Clusters::EnergyEvse::Delegate *>(this),
        .evseModeDelegate       = *this,
        .evseFeatures           = BitMask<Clusters::EnergyEvse::Feature>(
            Clusters::EnergyEvse::Feature::kChargingPreferences, Clusters::EnergyEvse::Feature::kRfid,
            Clusters::EnergyEvse::Feature::kSoCReporting, Clusters::EnergyEvse::Feature::kPlugAndCharge,
            Clusters::EnergyEvse::Feature::kV2x),
        .evseOptionalCmds = Clusters::EnergyEvse::EnergyEvseCluster::OptionalCommandSet()
                                .Set<Clusters::EnergyEvse::Commands::StartDiagnostics::Id>(),
        .evseOptionalAttrs = BitMask<Clusters::EnergyEvse::OptionalAttributes>(
            Clusters::EnergyEvse::OptionalAttributes::kSupportsUserMaximumChargingCurrent,
            Clusters::EnergyEvse::OptionalAttributes::kSupportsRandomizationWindow,
            Clusters::EnergyEvse::OptionalAttributes::kSupportsApproximateEvEfficiency),
    }),
    Clusters::EnergyEvse::AllDevicesEnergyEvseDelegate(mEvseTargetsDelegate),
    mTestEventTriggerDelegate(context.testEventTriggerDelegate), mPersistentStorageDelegate(context.persistentStorageDelegate)
{}

SimulatedEnergyEvse::~SimulatedEnergyEvse()
{
    CancelActiveTimers();
    SetSimulatedEnergyEvseDelegateForTriggers(nullptr);
}

CHIP_ERROR SimulatedEnergyEvse::Register(EndpointId endpoint, CodeDrivenDataModelProvider & provider,
                                         EndpointComposition composition)
{
    ReturnErrorOnFailure(EnergyEvse::Register(endpoint, provider, composition));

    SetEndpointId(endpoint);
    SetCluster(&EnergyEvseCluster());
    ReturnErrorOnFailure(mEvseTargetsDelegate.InitTargetsStorage(&mPersistentStorageDelegate));

    ReturnErrorOnFailure(mTestEventTriggerDelegate.AddHandler(this));
    SetSimulatedEnergyEvseDelegateForTriggers(this);

    return CHIP_NO_ERROR;
}

void SimulatedEnergyEvse::Unregister(CodeDrivenDataModelProvider & provider)
{
    mTestEventTriggerDelegate.RemoveHandler(this);
    SetSimulatedEnergyEvseDelegateForTriggers(nullptr);
    CancelActiveTimers();
    mEvseTargetsDelegate.Shutdown();
    SetCluster(nullptr);
    EnergyEvse::Unregister(provider);
}

CHIP_ERROR SimulatedEnergyEvse::RegisterOptionalClusters(EndpointId endpoint, CodeDrivenDataModelProvider & provider)
{
    mIdentifyCluster.Create(Clusters::IdentifyCluster::Config(endpoint, GetTimerDelegate()).WithDelegate(this));
    return provider.AddCluster(mIdentifyCluster.Registration());
}

void SimulatedEnergyEvse::UnregisterOptionalClusters(CodeDrivenDataModelProvider & provider)
{
    if (mIdentifyCluster.IsConstructed())
    {
        LogErrorOnFailure(provider.RemoveCluster(&mIdentifyCluster.Cluster()));
        mIdentifyCluster.Destroy();
    }
}

CHIP_ERROR SimulatedEnergyEvse::HandleEventTrigger(uint64_t eventTrigger)
{
    EndpointId triggerEndpoint = GetEndpointIdFromEventTrigger(eventTrigger);
    VerifyOrReturnError(triggerEndpoint == 0 || triggerEndpoint == GetEndpointId(), CHIP_ERROR_INVALID_ARGUMENT);

    eventTrigger = clearEndpointInEventTrigger(eventTrigger);
    if (HandleEnergyEvseTestEventTrigger(eventTrigger))
    {
        return CHIP_NO_ERROR;
    }
    return CHIP_ERROR_INVALID_ARGUMENT;
}

CHIP_ERROR SimulatedEnergyEvse::Init()
{
    return CHIP_NO_ERROR;
}

CHIP_ERROR SimulatedEnergyEvse::GetModeLabelByIndex(uint8_t modeIndex, MutableCharSpan & label)
{
    VerifyOrReturnError(modeIndex < MATTER_ARRAY_SIZE(kEvseModeOptions), CHIP_ERROR_PROVIDER_LIST_EXHAUSTED);
    return CopyCharSpanToMutableCharSpan(kEvseModeOptions[modeIndex].label, label);
}

CHIP_ERROR SimulatedEnergyEvse::GetModeValueByIndex(uint8_t modeIndex, uint8_t & value)
{
    VerifyOrReturnError(modeIndex < MATTER_ARRAY_SIZE(kEvseModeOptions), CHIP_ERROR_PROVIDER_LIST_EXHAUSTED);
    value = kEvseModeOptions[modeIndex].mode;
    return CHIP_NO_ERROR;
}

CHIP_ERROR SimulatedEnergyEvse::GetModeTagsByIndex(uint8_t modeIndex,
                                                   DataModel::List<Clusters::detail::Structs::ModeTagStruct::Type> & tags)
{
    VerifyOrReturnError(modeIndex < MATTER_ARRAY_SIZE(kEvseModeOptions), CHIP_ERROR_PROVIDER_LIST_EXHAUSTED);
    const auto & modeTags = kEvseModeOptions[modeIndex].modeTags;
    VerifyOrReturnError(tags.size() >= modeTags.size(), CHIP_ERROR_BUFFER_TOO_SMALL);
    std::copy(modeTags.begin(), modeTags.end(), tags.begin());
    tags.reduce_size(modeTags.size());
    return CHIP_NO_ERROR;
}

void SimulatedEnergyEvse::HandleChangeToMode(uint8_t newMode, Clusters::ModeBase::Commands::ChangeToModeResponse::Type & response)
{
    ChipLogProgress(DeviceLayer, "EnergyEvseMode: ChangeToMode to %u", newMode);
    response.status = to_underlying(Clusters::ModeBase::StatusCode::kSuccess);
}

void SimulatedEnergyEvse::OnIdentifyStart(Clusters::IdentifyCluster & cluster)
{
    ChipLogProgress(DeviceLayer, "EnergyEvse: OnIdentifyStart");
}

void SimulatedEnergyEvse::OnIdentifyStop(Clusters::IdentifyCluster & cluster)
{
    ChipLogProgress(DeviceLayer, "EnergyEvse: OnIdentifyStop");
}

void SimulatedEnergyEvse::OnTriggerEffect(Clusters::IdentifyCluster & cluster)
{
    ChipLogProgress(DeviceLayer, "EnergyEvse: OnTriggerEffect");
}

} // namespace chip::app
