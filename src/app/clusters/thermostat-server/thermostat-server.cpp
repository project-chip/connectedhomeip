/**
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

#include "thermostat-server.h"
#include "PresetStructWithOwnedMembers.h"

#include <app/util/attribute-storage.h>

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/callback.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app/CommandHandler.h>
#include <app/ConcreteAttributePath.h>
#include <app/ConcreteCommandPath.h>
#include <lib/core/CHIPEncoding.h>
#include <platform/internal/CHIPDeviceLayerInternal.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::Thermostat;
using namespace chip::app::Clusters::Thermostat::Structs;
using namespace chip::app::Clusters::Thermostat::Attributes;

using imcode = Protocols::InteractionModel::Status;

constexpr int16_t kDefaultAbsMinHeatSetpointLimit = 700;  // 7C (44.5 F) is the default
constexpr int16_t kDefaultAbsMaxHeatSetpointLimit = 3000; // 30C (86 F) is the default
constexpr int16_t kDefaultMinHeatSetpointLimit    = 700;  // 7C (44.5 F) is the default
constexpr int16_t kDefaultMaxHeatSetpointLimit    = 3000; // 30C (86 F) is the default
constexpr int16_t kDefaultAbsMinCoolSetpointLimit = 1600; // 16C (61 F) is the default
constexpr int16_t kDefaultAbsMaxCoolSetpointLimit = 3200; // 32C (90 F) is the default
constexpr int16_t kDefaultMinCoolSetpointLimit    = 1600; // 16C (61 F) is the default
constexpr int16_t kDefaultMaxCoolSetpointLimit    = 3200; // 32C (90 F) is the default
constexpr int16_t kDefaultHeatingSetpoint         = 2000;
constexpr int16_t kDefaultCoolingSetpoint         = 2600;
constexpr int8_t kDefaultDeadBand                 = 25; // 2.5C is the default

// IMPORTANT NOTE:
// No Side effects are permitted in emberAfThermostatClusterServerPreAttributeChangedCallback
// If a setpoint changes is required as a result of setpoint limit change
// it does not happen here.  It is the responsibility of the device to adjust the setpoint(s)
// as required in emberAfThermostatClusterServerPostAttributeChangedCallback
// limit change validation assures that there is at least 1 setpoint that will be valid

#define FEATURE_MAP_HEAT 0x01
#define FEATURE_MAP_COOL 0x02
#define FEATURE_MAP_OCC 0x04
#define FEATURE_MAP_SCH 0x08
#define FEATURE_MAP_SB 0x10
#define FEATURE_MAP_AUTO 0x20

#define FEATURE_MAP_DEFAULT FEATURE_MAP_HEAT | FEATURE_MAP_COOL | FEATURE_MAP_AUTO

namespace {

ThermostatAttrAccess gThermostatAttrAccess;

static_assert(kThermostatEndpointCount <= kEmberInvalidEndpointIndex, "Thermostat Delegate table size error");

Delegate * gDelegateTable[kThermostatEndpointCount] = { nullptr };

Delegate * GetDelegate(EndpointId endpoint)
{
    uint16_t ep =
        emberAfGetClusterServerEndpointIndex(endpoint, Thermostat::Id, MATTER_DM_THERMOSTAT_CLUSTER_SERVER_ENDPOINT_COUNT);
    return (ep >= ArraySize(gDelegateTable) ? nullptr : gDelegateTable[ep]);
}

/**
 * @brief Check if a preset is valid.
 *
 * @param[in] preset The preset to check.
 *
 * @return true If the preset is valid i.e the PresetHandle (if not null) fits within size constraints and the presetScenario enum
 *         value is valid. Otherwise, return false.
 */
bool IsValidPresetEntry(const PresetStruct::Type & preset)
{
    // Check that the preset handle is not too long.
    if (!preset.presetHandle.IsNull() && preset.presetHandle.Value().size() > kPresetHandleSize)
    {
        return false;
    }

    // Ensure we have a valid PresetScenario.
    return (preset.presetScenario != PresetScenarioEnum::kUnknownEnumValue);
}

/**
 * @brief Callback that is called when the timeout for editing the presets expires.
 *
 * @param[in] systemLayer The system layer.
 * @param[in] callbackContext The context passed to the timer callback.
 */
void TimerExpiredCallback(System::Layer * systemLayer, void * callbackContext)
{
    EndpointId endpoint = static_cast<EndpointId>(reinterpret_cast<uintptr_t>(callbackContext));

    Delegate * delegate = GetDelegate(endpoint);
    VerifyOrReturn(delegate != nullptr, ChipLogError(Zcl, "Delegate is null. Unable to handle timer expired"));

    delegate->ClearPendingPresetList();
    gThermostatAttrAccess.SetPresetsEditable(endpoint, false);
}

/**
 * @brief Schedules a timer for the given timeout in seconds.
 *
 * @param[in] endpoint The endpoint to use.
 * @param[in] timeoutSeconds The timeout in seconds.
 */
void ScheduleTimer(EndpointId endpoint, uint16_t timeoutSeconds)
{
    DeviceLayer::SystemLayer().StartTimer(System::Clock::Seconds16(timeoutSeconds), TimerExpiredCallback,
                                          reinterpret_cast<void *>(static_cast<uintptr_t>(endpoint)));
}

/**
 * @brief Clears the currently scheduled timer.
 *
 * @param[in] endpoint The endpoint to use.
 */
void ClearTimer(EndpointId endpoint)
{
    DeviceLayer::SystemLayer().CancelTimer(TimerExpiredCallback, reinterpret_cast<void *>(static_cast<uintptr_t>(endpoint)));
}

/**
 * @brief Extends the currently scheduled timer to a new timeout value in seconds
 *
 * @param[in] endpoint The endpoint to use.
 * @param[in] timeoutSeconds The timeout in seconds to extend the timer to.
 */
void ExtendTimer(EndpointId endpoint, uint16_t timeoutSeconds)
{
    DeviceLayer::SystemLayer().ExtendTimerTo(System::Clock::Seconds16(timeoutSeconds), TimerExpiredCallback,
                                             reinterpret_cast<void *>(static_cast<uintptr_t>(endpoint)));
}

/**
 * @brief Checks if the preset is built-in
 *
 * @param[in] preset The preset to check.
 *
 * @return true If the preset is built-in, false otherwise.
 */
bool IsBuiltIn(const PresetStructWithOwnedMembers & preset)
{
    return preset.GetBuiltIn().ValueOr(false);
}

/**
 * @brief Checks if the presets are matching i.e the presetHandles are the same.
 *
 * @param[in] preset The preset to check.
 * @param[in] presetToMatch The preset to match with.
 *
 * @return true If the presets match, false otherwise. If both preset handles are null, returns false
 */
bool PresetHandlesExistAndMatch(const PresetStructWithOwnedMembers & preset, const PresetStructWithOwnedMembers & presetToMatch)
{
    return !preset.GetPresetHandle().IsNull() && !presetToMatch.GetPresetHandle().IsNull() &&
        preset.GetPresetHandle().Value().data_equal(presetToMatch.GetPresetHandle().Value());
}

/**
 * @brief Get the source scoped node id.
 *
 * @param[in] commandObj The command handler object.
 *
 * @return The scoped node id of the source node. If the scoped node id is not retreived, return ScopedNodeId().
 */
ScopedNodeId GetSourceScopedNodeId(CommandHandler * commandObj)
{
    ScopedNodeId sourceNodeId = ScopedNodeId();
    auto sessionHandle        = commandObj->GetExchangeContext()->GetSessionHandle();

    if (sessionHandle->IsSecureSession())
    {
        sourceNodeId = sessionHandle->AsSecureSession()->GetPeer();
    }
    else if (sessionHandle->IsGroupSession())
    {
        sourceNodeId = sessionHandle->AsIncomingGroupSession()->GetPeer();
    }
    return sourceNodeId;
}

/**
 * @brief Utility to clean up state by clearing the pending presets list, canceling the timer
 *        and setting PresetsEditable to false and clear the originator scoped node id.
 *
 * @param[in] delegate The delegate to use.
 * @param[in] endpoint The endpoint to use.
 */
void CleanUp(Delegate * delegate, EndpointId endpoint)
{
    if (delegate != nullptr)
    {
        delegate->ClearPendingPresetList();
    }
    ClearTimer(endpoint);
    gThermostatAttrAccess.SetPresetsEditable(endpoint, false);
    gThermostatAttrAccess.SetOriginatorScopedNodeId(endpoint, ScopedNodeId());
}

/**
 * @brief Sends a response for the command and cleans up state by calling CleanUp()
 *
 * @param[in] delegate The delegate to use.
 * @param[in] endpoint The endpoint to use.
 * @param[in] commandObj The command handler to use to add the status response.
 * @param[in] commandPath The command path.
 * @param[in] status The status code to send as the response.
 *
 * @return true to indicate the response has been sent and command has been handled.
 */
bool SendResponseAndCleanUp(Delegate * delegate, EndpointId endpoint, CommandHandler * commandObj,
                            const ConcreteCommandPath & commandPath, imcode status)
{
    commandObj->AddStatus(commandPath, status);
    CleanUp(delegate, endpoint);
    return true;
}

/**
 * @brief Finds an entry in the pending presets list that matches a preset.
 *        The presetHandle of the two presets must match.
 *
 * @param[in] delegate The delegate to use.
 * @param[in] presetToMatch The preset to match with.
 *
 * @return true if a matching entry was found in the pending presets list, false otherwise.
 */
bool MatchingPendingPresetExists(Delegate * delegate, const PresetStructWithOwnedMembers & presetToMatch)
{
    VerifyOrReturnValue(delegate != nullptr, false);

    for (uint8_t i = 0; true; i++)
    {
        PresetStructWithOwnedMembers preset;
        CHIP_ERROR err = delegate->GetPendingPresetAtIndex(i, preset);

        if (err == CHIP_ERROR_PROVIDER_LIST_EXHAUSTED)
        {
            break;
        }
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(Zcl, "MatchingPendingPresetExists: GetPendingPresetAtIndex failed with error %" CHIP_ERROR_FORMAT,
                         err.Format());
            return false;
        }

        if (PresetHandlesExistAndMatch(preset, presetToMatch))
        {
            return true;
        }
    }
    return false;
}

/**
 * @brief Finds and returns an entry in the Presets attribute list that matches a preset.
 *        The presetHandle of the two presets must match.
 *
 * @param[in] delegate The delegate to use.
 * @param[in] presetToMatch The preset to match with.
 * @param[out] matchingPreset The preset in the Presets attribute list that has the same PresetHandle as the presetToMatch.
 *
 * @return true if a matching entry was found in the  presets attribute list, false otherwise.
 */
bool GetMatchingPresetInPresets(Delegate * delegate, const PresetStructWithOwnedMembers & presetToMatch,
                                PresetStructWithOwnedMembers & matchingPreset)
{
    VerifyOrReturnValue(delegate != nullptr, false);

    for (uint8_t i = 0; true; i++)
    {
        CHIP_ERROR err = delegate->GetPresetAtIndex(i, matchingPreset);

        if (err == CHIP_ERROR_PROVIDER_LIST_EXHAUSTED)
        {
            break;
        }
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(Zcl, "GetMatchingPresetInPresets: GetPresetAtIndex failed with error %" CHIP_ERROR_FORMAT, err.Format());
            return false;
        }

        if (PresetHandlesExistAndMatch(matchingPreset, presetToMatch))
        {
            return true;
        }
    }
    return false;
}

/**
 * @brief Checks if the given preset handle is present in the  presets attribute
 * @param[in] delegate The delegate to use.
 * @param[in] presetHandleToMatch The preset handle to match with.
 *
 * @return true if the given preset handle is present in the  presets attribute list, false otherwise.
 */
bool IsPresetHandlePresentInPresets(Delegate * delegate, const ByteSpan & presetHandleToMatch)
{
    VerifyOrReturnValue(delegate != nullptr, false);

    PresetStructWithOwnedMembers matchingPreset;
    for (uint8_t i = 0; true; i++)
    {
        CHIP_ERROR err = delegate->GetPresetAtIndex(i, matchingPreset);

        if (err == CHIP_ERROR_PROVIDER_LIST_EXHAUSTED)
        {
            return false;
        }

        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(Zcl, "IsPresetHandlePresentInPresets: GetPresetAtIndex failed with error %" CHIP_ERROR_FORMAT,
                         err.Format());
            return false;
        }

        if (!matchingPreset.GetPresetHandle().IsNull() && matchingPreset.GetPresetHandle().Value().data_equal(presetHandleToMatch))
        {
            return true;
        }
    }
    return false;
}

/**
 * @brief Returns the length of the list of presets if the pending presets were to be applied. The calculation is done by
 *        adding the number of presets in Presets attribute list to the number of pending presets in the pending
 *        presets list and subtracting the number of duplicate presets. This is called before changes are actually applied.
 *
 * @param[in] delegate The delegate to use.
 *
 * @return count of the updated Presets attribute if the pending presets were applied to it. Return 0 for error cases.
 */
uint8_t CountUpdatedPresetsAfterApplyingPendingPresets(Delegate * delegate)
{
    uint8_t numberOfPresets        = 0;
    uint8_t numberOfMatches        = 0;
    uint8_t numberOfPendingPresets = 0;

    VerifyOrReturnValue(delegate != nullptr, 0);

    for (uint8_t i = 0; true; i++)
    {
        PresetStructWithOwnedMembers preset;
        CHIP_ERROR err = delegate->GetPresetAtIndex(i, preset);

        if (err == CHIP_ERROR_PROVIDER_LIST_EXHAUSTED)
        {
            break;
        }
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(Zcl, "GetUpdatedPresetsCount: GetPresetAtIndex failed with error %" CHIP_ERROR_FORMAT, err.Format());
            return 0;
        }
        numberOfPresets++;

        bool found = MatchingPendingPresetExists(delegate, preset);

        if (found)
        {
            numberOfMatches++;
        }
    }

    for (uint8_t i = 0; true; i++)
    {
        PresetStructWithOwnedMembers pendingPreset;
        CHIP_ERROR err = delegate->GetPendingPresetAtIndex(i, pendingPreset);

        if (err == CHIP_ERROR_PROVIDER_LIST_EXHAUSTED)
        {
            break;
        }
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(Zcl, "GetUpdatedPresetsCount: GetPendingPresetAtIndex failed with error %" CHIP_ERROR_FORMAT,
                         err.Format());
            return 0;
        }
        numberOfPendingPresets++;
    }

    // TODO: #34546 - Need to support deletion of presets that are removed from Presets.
    // This API needs to modify its logic for the deletion case.
    return static_cast<uint8_t>(numberOfPresets + numberOfPendingPresets - numberOfMatches);
}

/**
 * @brief Checks if the presetScenario is present in the PresetTypes attribute.
 *
 * @param[in] delegate The delegate to use.
 * @param[in] presetScenario The presetScenario to match with.
 *
 * @return true if the presetScenario is found, false otherwise.
 */
bool PresetScenarioExistsInPresetTypes(Delegate * delegate, PresetScenarioEnum presetScenario)
{
    VerifyOrReturnValue(delegate != nullptr, false);

    for (uint8_t i = 0; true; i++)
    {
        PresetTypeStruct::Type presetType;
        auto err = delegate->GetPresetTypeAtIndex(i, presetType);
        if (err != CHIP_NO_ERROR)
        {
            return false;
        }

        if (presetType.presetScenario == presetScenario)
        {
            return true;
        }
    }
    return false;
}

/**
 * @brief Returns the count of preset entries in the pending presets list that have the matching presetHandle.
 * @param[in] delegate The delegate to use.
 * @param[in] presetHandleToMatch The preset handle to match.
 *
 * @return count of the number of presets found with the matching presetHandle. Returns 0 if no matching presets were found.
 */
uint8_t CountPresetsInPendingListWithPresetHandle(Delegate * delegate, const ByteSpan & presetHandleToMatch)
{
    uint8_t count = 0;
    VerifyOrReturnValue(delegate != nullptr, count);

    for (uint8_t i = 0; true; i++)
    {
        PresetStructWithOwnedMembers preset;
        auto err = delegate->GetPendingPresetAtIndex(i, preset);
        if (err != CHIP_NO_ERROR)
        {
            return count;
        }

        DataModel::Nullable<ByteSpan> presetHandle = preset.GetPresetHandle();
        if (!presetHandle.IsNull() && presetHandle.Value().data_equal(presetHandleToMatch))
        {
            count++;
        }
    }
    return count;
}

/**
 * @brief Checks if the presetType for the given preset scenario supports name in the presetTypeFeatures bitmap.
 *
 * @param[in] delegate The delegate to use.
 * @param[in] presetScenario The presetScenario to match with.
 *
 * @return true if the presetType for the given preset scenario supports name, false otherwise.
 */
bool PresetTypeSupportsNames(Delegate * delegate, PresetScenarioEnum scenario)
{
    VerifyOrReturnValue(delegate != nullptr, false);

    for (uint8_t i = 0; true; i++)
    {
        PresetTypeStruct::Type presetType;
        auto err = delegate->GetPresetTypeAtIndex(i, presetType);
        if (err != CHIP_NO_ERROR)
        {
            return false;
        }

        if (presetType.presetScenario == scenario)
        {
            return (presetType.presetTypeFeatures.Has(PresetTypeFeaturesBitmap::kSupportsNames));
        }
    }
    return false;
}

int16_t EnforceHeatingSetpointLimits(int16_t HeatingSetpoint, EndpointId endpoint)
{
    // Optional Mfg supplied limits
    int16_t AbsMinHeatSetpointLimit = kDefaultAbsMinHeatSetpointLimit;
    int16_t AbsMaxHeatSetpointLimit = kDefaultAbsMaxHeatSetpointLimit;

    // Optional User supplied limits
    int16_t MinHeatSetpointLimit = kDefaultMinHeatSetpointLimit;
    int16_t MaxHeatSetpointLimit = kDefaultMaxHeatSetpointLimit;

    // Attempt to read the setpoint limits
    // Absmin/max are manufacturer limits
    // min/max are user imposed min/max

    // Note that the limits are initialized above per the spec limits
    // if they are not present Get() will not update the value so the defaults are used
    imcode status;

    // https://github.com/CHIP-Specifications/connectedhomeip-spec/issues/3724
    // behavior is not specified when Abs * values are not present and user values are present
    // implemented behavior accepts the user values without regard to default Abs values.

    // Per global matter data model policy
    // if a attribute is not present then it's default shall be used.

    status = AbsMinHeatSetpointLimit::Get(endpoint, &AbsMinHeatSetpointLimit);
    if (status != imcode::Success)
    {
        ChipLogError(Zcl, "Warning: AbsMinHeatSetpointLimit missing using default");
    }

    status = AbsMaxHeatSetpointLimit::Get(endpoint, &AbsMaxHeatSetpointLimit);
    if (status != imcode::Success)
    {
        ChipLogError(Zcl, "Warning: AbsMaxHeatSetpointLimit missing using default");
    }
    status = MinHeatSetpointLimit::Get(endpoint, &MinHeatSetpointLimit);
    if (status != imcode::Success)
    {
        MinHeatSetpointLimit = AbsMinHeatSetpointLimit;
    }

    status = MaxHeatSetpointLimit::Get(endpoint, &MaxHeatSetpointLimit);
    if (status != imcode::Success)
    {
        MaxHeatSetpointLimit = AbsMaxHeatSetpointLimit;
    }

    // Make sure the user imposed limits are within the manufacturer imposed limits

    // https://github.com/CHIP-Specifications/connectedhomeip-spec/issues/3725
    // Spec does not specify the behavior is the requested setpoint exceeds the limit allowed
    // This implementation clamps at the limit.

    // resolution of 3725 is to clamp.

    if (MinHeatSetpointLimit < AbsMinHeatSetpointLimit)
        MinHeatSetpointLimit = AbsMinHeatSetpointLimit;

    if (MaxHeatSetpointLimit > AbsMaxHeatSetpointLimit)
        MaxHeatSetpointLimit = AbsMaxHeatSetpointLimit;

    if (HeatingSetpoint < MinHeatSetpointLimit)
        HeatingSetpoint = MinHeatSetpointLimit;

    if (HeatingSetpoint > MaxHeatSetpointLimit)
        HeatingSetpoint = MaxHeatSetpointLimit;

    return HeatingSetpoint;
}

int16_t EnforceCoolingSetpointLimits(int16_t CoolingSetpoint, EndpointId endpoint)
{
    // Optional Mfg supplied limits
    int16_t AbsMinCoolSetpointLimit = kDefaultAbsMinCoolSetpointLimit;
    int16_t AbsMaxCoolSetpointLimit = kDefaultAbsMaxCoolSetpointLimit;

    // Optional User supplied limits
    int16_t MinCoolSetpointLimit = kDefaultMinCoolSetpointLimit;
    int16_t MaxCoolSetpointLimit = kDefaultMaxCoolSetpointLimit;

    // Attempt to read the setpoint limits
    // Absmin/max are manufacturer limits
    // min/max are user imposed min/max

    // Note that the limits are initialized above per the spec limits
    // if they are not present Get() will not update the value so the defaults are used
    imcode status;

    // https://github.com/CHIP-Specifications/connectedhomeip-spec/issues/3724
    // behavior is not specified when Abs * values are not present and user values are present
    // implemented behavior accepts the user values without regard to default Abs values.

    // Per global matter data model policy
    // if a attribute is not present then it's default shall be used.

    status = AbsMinCoolSetpointLimit::Get(endpoint, &AbsMinCoolSetpointLimit);
    if (status != imcode::Success)
    {
        ChipLogError(Zcl, "Warning: AbsMinCoolSetpointLimit missing using default");
    }

    status = AbsMaxCoolSetpointLimit::Get(endpoint, &AbsMaxCoolSetpointLimit);
    if (status != imcode::Success)
    {
        ChipLogError(Zcl, "Warning: AbsMaxCoolSetpointLimit missing using default");
    }

    status = MinCoolSetpointLimit::Get(endpoint, &MinCoolSetpointLimit);
    if (status != imcode::Success)
    {
        MinCoolSetpointLimit = AbsMinCoolSetpointLimit;
    }

    status = MaxCoolSetpointLimit::Get(endpoint, &MaxCoolSetpointLimit);
    if (status != imcode::Success)
    {
        MaxCoolSetpointLimit = AbsMaxCoolSetpointLimit;
    }

    // Make sure the user imposed limits are within the manufacture imposed limits
    // https://github.com/CHIP-Specifications/connectedhomeip-spec/issues/3725
    // Spec does not specify the behavior is the requested setpoint exceeds the limit allowed
    // This implementation clamps at the limit.

    // resolution of 3725 is to clamp.

    if (MinCoolSetpointLimit < AbsMinCoolSetpointLimit)
        MinCoolSetpointLimit = AbsMinCoolSetpointLimit;

    if (MaxCoolSetpointLimit > AbsMaxCoolSetpointLimit)
        MaxCoolSetpointLimit = AbsMaxCoolSetpointLimit;

    if (CoolingSetpoint < MinCoolSetpointLimit)
        CoolingSetpoint = MinCoolSetpointLimit;

    if (CoolingSetpoint > MaxCoolSetpointLimit)
        CoolingSetpoint = MaxCoolSetpointLimit;

    return CoolingSetpoint;
}

} // anonymous namespace

namespace chip {
namespace app {
namespace Clusters {
namespace Thermostat {

void SetDefaultDelegate(EndpointId endpoint, Delegate * delegate)
{
    uint16_t ep =
        emberAfGetClusterServerEndpointIndex(endpoint, Thermostat::Id, MATTER_DM_THERMOSTAT_CLUSTER_SERVER_ENDPOINT_COUNT);
    // if endpoint is found, add the delegate in the delegate table
    if (ep < ArraySize(gDelegateTable))
    {
        gDelegateTable[ep] = delegate;
    }
}

void ThermostatAttrAccess::SetPresetsEditable(EndpointId endpoint, bool presetEditable)
{
    uint16_t ep =
        emberAfGetClusterServerEndpointIndex(endpoint, Thermostat::Id, MATTER_DM_THERMOSTAT_CLUSTER_SERVER_ENDPOINT_COUNT);

    if (ep < ArraySize(mPresetsEditables))
    {
        mPresetsEditables[ep] = presetEditable;
    }
}

bool ThermostatAttrAccess::GetPresetsEditable(EndpointId endpoint)
{
    bool presetEditable = false;
    uint16_t ep =
        emberAfGetClusterServerEndpointIndex(endpoint, Thermostat::Id, MATTER_DM_THERMOSTAT_CLUSTER_SERVER_ENDPOINT_COUNT);

    if (ep < ArraySize(mPresetsEditables))
    {
        presetEditable = mPresetsEditables[ep];
    }
    return presetEditable;
}

void ThermostatAttrAccess::SetOriginatorScopedNodeId(EndpointId endpoint, ScopedNodeId originatorNodeId)
{
    uint16_t ep =
        emberAfGetClusterServerEndpointIndex(endpoint, Thermostat::Id, MATTER_DM_THERMOSTAT_CLUSTER_SERVER_ENDPOINT_COUNT);

    if (ep < ArraySize(mPresetEditRequestOriginatorNodeIds))
    {
        mPresetEditRequestOriginatorNodeIds[ep] = originatorNodeId;
    }
}

ScopedNodeId ThermostatAttrAccess::GetOriginatorScopedNodeId(EndpointId endpoint)
{
    ScopedNodeId originatorNodeId = ScopedNodeId();
    uint16_t ep =
        emberAfGetClusterServerEndpointIndex(endpoint, Thermostat::Id, MATTER_DM_THERMOSTAT_CLUSTER_SERVER_ENDPOINT_COUNT);

    if (ep < ArraySize(mPresetEditRequestOriginatorNodeIds))
    {
        originatorNodeId = mPresetEditRequestOriginatorNodeIds[ep];
    }
    return originatorNodeId;
}

CHIP_ERROR ThermostatAttrAccess::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    VerifyOrDie(aPath.mClusterId == Thermostat::Id);

    uint32_t ourFeatureMap;
    bool localTemperatureNotExposedSupported = (FeatureMap::Get(aPath.mEndpointId, &ourFeatureMap) == imcode::Success) &&
        ((ourFeatureMap & to_underlying(Feature::kLocalTemperatureNotExposed)) != 0);

    switch (aPath.mAttributeId)
    {
    case LocalTemperature::Id:
        if (localTemperatureNotExposedSupported)
        {
            return aEncoder.EncodeNull();
        }
        break;
    case RemoteSensing::Id:
        if (localTemperatureNotExposedSupported)
        {
            BitMask<RemoteSensingBitmap> valueRemoteSensing;
            imcode status = RemoteSensing::Get(aPath.mEndpointId, &valueRemoteSensing);
            if (status != imcode::Success)
            {
                StatusIB statusIB(status);
                return statusIB.ToChipError();
            }
            valueRemoteSensing.Clear(RemoteSensingBitmap::kLocalTemperature);
            return aEncoder.Encode(valueRemoteSensing);
        }
        break;
    case PresetTypes::Id: {
        Delegate * delegate = GetDelegate(aPath.mEndpointId);
        VerifyOrReturnError(delegate != nullptr, CHIP_ERROR_INCORRECT_STATE, ChipLogError(Zcl, "Delegate is null"));

        return aEncoder.EncodeList([delegate](const auto & encoder) -> CHIP_ERROR {
            for (uint8_t i = 0; true; i++)
            {
                PresetTypeStruct::Type presetType;
                auto err = delegate->GetPresetTypeAtIndex(i, presetType);
                if (err == CHIP_ERROR_PROVIDER_LIST_EXHAUSTED)
                {
                    return CHIP_NO_ERROR;
                }
                ReturnErrorOnFailure(err);
                ReturnErrorOnFailure(encoder.Encode(presetType));
            }
        });
    }
    break;
    case NumberOfPresets::Id: {
        Delegate * delegate = GetDelegate(aPath.mEndpointId);
        VerifyOrReturnError(delegate != nullptr, CHIP_ERROR_INCORRECT_STATE, ChipLogError(Zcl, "Delegate is null"));

        ReturnErrorOnFailure(aEncoder.Encode(delegate->GetNumberOfPresets()));
    }
    break;
    case Presets::Id: {
        Delegate * delegate = GetDelegate(aPath.mEndpointId);
        VerifyOrReturnError(delegate != nullptr, CHIP_ERROR_INCORRECT_STATE, ChipLogError(Zcl, "Delegate is null"));

        return aEncoder.EncodeList([delegate](const auto & encoder) -> CHIP_ERROR {
            for (uint8_t i = 0; true; i++)
            {
                PresetStructWithOwnedMembers preset;
                auto err = delegate->GetPresetAtIndex(i, preset);
                if (err == CHIP_ERROR_PROVIDER_LIST_EXHAUSTED)
                {
                    return CHIP_NO_ERROR;
                }
                ReturnErrorOnFailure(err);
                ReturnErrorOnFailure(encoder.Encode(preset));
            }
        });
    }
    break;
    case PresetsSchedulesEditable::Id: {
        ReturnErrorOnFailure(aEncoder.Encode(GetPresetsEditable(aPath.mEndpointId)));
    }
    break;
    case ActivePresetHandle::Id: {
        Delegate * delegate = GetDelegate(aPath.mEndpointId);
        VerifyOrReturnError(delegate != nullptr, CHIP_ERROR_INCORRECT_STATE, ChipLogError(Zcl, "Delegate is null"));

        uint8_t buffer[kPresetHandleSize];
        MutableByteSpan activePresetHandle(buffer);

        CHIP_ERROR err = delegate->GetActivePresetHandle(activePresetHandle);
        ReturnErrorOnFailure(err);

        if (activePresetHandle.empty())
        {
            ReturnErrorOnFailure(aEncoder.EncodeNull());
        }
        else
        {
            ReturnErrorOnFailure(aEncoder.Encode(activePresetHandle));
        }
    }
    break;
    case ScheduleTypes::Id: {
        return aEncoder.EncodeList([](const auto & encoder) -> CHIP_ERROR { return CHIP_NO_ERROR; });
    }
    break;
    case Schedules::Id: {
        return aEncoder.EncodeList([](const auto & encoder) -> CHIP_ERROR { return CHIP_NO_ERROR; });
    }
    break;
    default: // return CHIP_NO_ERROR and just read from the attribute store in default
        break;
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR ThermostatAttrAccess::Write(const ConcreteDataAttributePath & aPath, AttributeValueDecoder & aDecoder)
{
    VerifyOrDie(aPath.mClusterId == Thermostat::Id);

    uint32_t ourFeatureMap;
    bool localTemperatureNotExposedSupported = (FeatureMap::Get(aPath.mEndpointId, &ourFeatureMap) == imcode::Success) &&
        ((ourFeatureMap & to_underlying(Feature::kLocalTemperatureNotExposed)) != 0);

    switch (aPath.mAttributeId)
    {
    case RemoteSensing::Id:
        if (localTemperatureNotExposedSupported)
        {
            uint8_t valueRemoteSensing;
            ReturnErrorOnFailure(aDecoder.Decode(valueRemoteSensing));
            if (valueRemoteSensing & 0x01) // If setting bit 1 (LocalTemperature RemoteSensing bit)
            {
                return CHIP_IM_GLOBAL_STATUS(ConstraintError);
            }
            imcode status = RemoteSensing::Set(aPath.mEndpointId, valueRemoteSensing);
            StatusIB statusIB(status);
            return statusIB.ToChipError();
        }
        break;
    case Presets::Id: {

        EndpointId endpoint = aPath.mEndpointId;
        Delegate * delegate = GetDelegate(endpoint);
        VerifyOrReturnError(delegate != nullptr, CHIP_ERROR_INCORRECT_STATE, ChipLogError(Zcl, "Delegate is null"));

        // Presets are not editable, return INVALID_IN_STATE.
        VerifyOrReturnError(GetPresetsEditable(endpoint), CHIP_IM_GLOBAL_STATUS(InvalidInState),
                            ChipLogError(Zcl, "Presets are not editable"));

        // Check if the OriginatorScopedNodeId at the endpoint is the same as the node editing the presets,
        // otherwise return BUSY.
        const Access::SubjectDescriptor subjectDescriptor = aDecoder.GetSubjectDescriptor();
        ScopedNodeId scopedNodeId                         = ScopedNodeId();

        // Get the node id if the authentication mode is CASE.
        if (subjectDescriptor.authMode == Access::AuthMode::kCase)
        {
            scopedNodeId = ScopedNodeId(subjectDescriptor.subject, subjectDescriptor.fabricIndex);
        }

        if (GetOriginatorScopedNodeId(endpoint) != scopedNodeId)
        {
            ChipLogError(Zcl, "Another node is editing presets. Server is busy. Try again later");
            return CHIP_IM_GLOBAL_STATUS(Busy);
        }

        // If the list operation is replace all, clear the existing pending list, iterate over the new presets list
        // and add to the pending presets list.
        if (!aPath.IsListOperation() || aPath.mListOp == ConcreteDataAttributePath::ListOperation::ReplaceAll)
        {
            // Clear the pending presets list
            delegate->ClearPendingPresetList();

            Presets::TypeInfo::DecodableType newPresetsList;
            ReturnErrorOnFailure(aDecoder.Decode(newPresetsList));

            // Iterate over the presets and call the delegate to append to the list of pending presets.
            auto iter = newPresetsList.begin();
            while (iter.Next())
            {
                const PresetStruct::Type & preset = iter.GetValue();
                if (IsValidPresetEntry(preset))
                {
                    ReturnErrorOnFailure(delegate->AppendToPendingPresetList(preset));
                }
                else
                {
                    return CHIP_IM_GLOBAL_STATUS(ConstraintError);
                }
            }
            return iter.GetStatus();
        }

        // If the list operation is AppendItem, call the delegate to append the item to the list of pending presets.
        if (aPath.mListOp == ConcreteDataAttributePath::ListOperation::AppendItem)
        {
            PresetStruct::Type preset;
            ReturnErrorOnFailure(aDecoder.Decode(preset));
            if (IsValidPresetEntry(preset))
            {
                return delegate->AppendToPendingPresetList(preset);
            }
            return CHIP_IM_GLOBAL_STATUS(ConstraintError);
        }
    }
    break;
    case Schedules::Id: {
        return CHIP_ERROR_NOT_IMPLEMENTED;
    }
    break;
    default: // return CHIP_NO_ERROR and just write to the attribute store in default
        break;
    }

    return CHIP_NO_ERROR;
}

} // namespace Thermostat
} // namespace Clusters
} // namespace app
} // namespace chip

void emberAfThermostatClusterServerInitCallback(chip::EndpointId endpoint)
{
    // TODO
    // Get from the "real thermostat"
    // current mode
    // current occupied heating setpoint
    // current unoccupied heating setpoint
    // current occupied cooling setpoint
    // current unoccupied cooling setpoint
    // and update the zcl cluster values
    // This should be a callback defined function
    // with weak binding so that real thermostat
    // can get the values.
    // or should this just be the responsibility of the thermostat application?
}

Protocols::InteractionModel::Status
MatterThermostatClusterServerPreAttributeChangedCallback(const app::ConcreteAttributePath & attributePath,
                                                         EmberAfAttributeType attributeType, uint16_t size, uint8_t * value)
{
    EndpointId endpoint = attributePath.mEndpointId;
    int16_t requested;

    // Limits will be needed for all checks
    // so we just get them all now
    int16_t AbsMinHeatSetpointLimit;
    int16_t AbsMaxHeatSetpointLimit;
    int16_t MinHeatSetpointLimit;
    int16_t MaxHeatSetpointLimit;
    int16_t AbsMinCoolSetpointLimit;
    int16_t AbsMaxCoolSetpointLimit;
    int16_t MinCoolSetpointLimit;
    int16_t MaxCoolSetpointLimit;
    int8_t DeadBand      = 0;
    int16_t DeadBandTemp = 0;
    int16_t OccupiedCoolingSetpoint;
    int16_t OccupiedHeatingSetpoint;
    int16_t UnoccupiedCoolingSetpoint;
    int16_t UnoccupiedHeatingSetpoint;
    uint32_t OurFeatureMap;
    bool AutoSupported      = false;
    bool HeatSupported      = false;
    bool CoolSupported      = false;
    bool OccupancySupported = false;

    if (FeatureMap::Get(endpoint, &OurFeatureMap) != imcode::Success)
        OurFeatureMap = FEATURE_MAP_DEFAULT;

    if (OurFeatureMap & 1 << 5) // Bit 5 is Auto Mode supported
        AutoSupported = true;

    if (OurFeatureMap & 1 << 0)
        HeatSupported = true;

    if (OurFeatureMap & 1 << 1)
        CoolSupported = true;

    if (OurFeatureMap & 1 << 2)
        OccupancySupported = true;

    if (AutoSupported)
    {
        if (MinSetpointDeadBand::Get(endpoint, &DeadBand) != imcode::Success)
        {
            DeadBand = kDefaultDeadBand;
        }
        DeadBandTemp = static_cast<int16_t>(DeadBand * 10);
    }

    if (AbsMinCoolSetpointLimit::Get(endpoint, &AbsMinCoolSetpointLimit) != imcode::Success)
        AbsMinCoolSetpointLimit = kDefaultAbsMinCoolSetpointLimit;

    if (AbsMaxCoolSetpointLimit::Get(endpoint, &AbsMaxCoolSetpointLimit) != imcode::Success)
        AbsMaxCoolSetpointLimit = kDefaultAbsMaxCoolSetpointLimit;

    if (MinCoolSetpointLimit::Get(endpoint, &MinCoolSetpointLimit) != imcode::Success)
        MinCoolSetpointLimit = AbsMinCoolSetpointLimit;

    if (MaxCoolSetpointLimit::Get(endpoint, &MaxCoolSetpointLimit) != imcode::Success)
        MaxCoolSetpointLimit = AbsMaxCoolSetpointLimit;

    if (AbsMinHeatSetpointLimit::Get(endpoint, &AbsMinHeatSetpointLimit) != imcode::Success)
        AbsMinHeatSetpointLimit = kDefaultAbsMinHeatSetpointLimit;

    if (AbsMaxHeatSetpointLimit::Get(endpoint, &AbsMaxHeatSetpointLimit) != imcode::Success)
        AbsMaxHeatSetpointLimit = kDefaultAbsMaxHeatSetpointLimit;

    if (MinHeatSetpointLimit::Get(endpoint, &MinHeatSetpointLimit) != imcode::Success)
        MinHeatSetpointLimit = AbsMinHeatSetpointLimit;

    if (MaxHeatSetpointLimit::Get(endpoint, &MaxHeatSetpointLimit) != imcode::Success)
        MaxHeatSetpointLimit = AbsMaxHeatSetpointLimit;

    if (CoolSupported)
        if (OccupiedCoolingSetpoint::Get(endpoint, &OccupiedCoolingSetpoint) != imcode::Success)
        {
            ChipLogError(Zcl, "Error: Can not read Occupied Cooling Setpoint");
            return imcode::Failure;
        }

    if (HeatSupported)
        if (OccupiedHeatingSetpoint::Get(endpoint, &OccupiedHeatingSetpoint) != imcode::Success)
        {
            ChipLogError(Zcl, "Error: Can not read Occupied Heating Setpoint");
            return imcode::Failure;
        }

    if (CoolSupported && OccupancySupported)
        if (UnoccupiedCoolingSetpoint::Get(endpoint, &UnoccupiedCoolingSetpoint) != imcode::Success)
        {
            ChipLogError(Zcl, "Error: Can not read Unoccupied Cooling Setpoint");
            return imcode::Failure;
        }

    if (HeatSupported && OccupancySupported)
        if (UnoccupiedHeatingSetpoint::Get(endpoint, &UnoccupiedHeatingSetpoint) != imcode::Success)
        {
            ChipLogError(Zcl, "Error: Can not read Unoccupied Heating Setpoint");
            return imcode::Failure;
        }

    switch (attributePath.mAttributeId)
    {
    case OccupiedHeatingSetpoint::Id: {
        requested = static_cast<int16_t>(chip::Encoding::LittleEndian::Get16(value));
        if (!HeatSupported)
            return imcode::UnsupportedAttribute;
        if (requested < AbsMinHeatSetpointLimit || requested < MinHeatSetpointLimit || requested > AbsMaxHeatSetpointLimit ||
            requested > MaxHeatSetpointLimit)
            return imcode::InvalidValue;
        if (AutoSupported)
        {
            if (requested > OccupiedCoolingSetpoint - DeadBandTemp)
                return imcode::InvalidValue;
        }
        return imcode::Success;
    }

    case OccupiedCoolingSetpoint::Id: {
        requested = static_cast<int16_t>(chip::Encoding::LittleEndian::Get16(value));
        if (!CoolSupported)
            return imcode::UnsupportedAttribute;
        if (requested < AbsMinCoolSetpointLimit || requested < MinCoolSetpointLimit || requested > AbsMaxCoolSetpointLimit ||
            requested > MaxCoolSetpointLimit)
            return imcode::InvalidValue;
        if (AutoSupported)
        {
            if (requested < OccupiedHeatingSetpoint + DeadBandTemp)
                return imcode::InvalidValue;
        }
        return imcode::Success;
    }

    case UnoccupiedHeatingSetpoint::Id: {
        requested = static_cast<int16_t>(chip::Encoding::LittleEndian::Get16(value));
        if (!(HeatSupported && OccupancySupported))
            return imcode::UnsupportedAttribute;
        if (requested < AbsMinHeatSetpointLimit || requested < MinHeatSetpointLimit || requested > AbsMaxHeatSetpointLimit ||
            requested > MaxHeatSetpointLimit)
            return imcode::InvalidValue;
        if (AutoSupported)
        {
            if (requested > UnoccupiedCoolingSetpoint - DeadBandTemp)
                return imcode::InvalidValue;
        }
        return imcode::Success;
    }
    case UnoccupiedCoolingSetpoint::Id: {
        requested = static_cast<int16_t>(chip::Encoding::LittleEndian::Get16(value));
        if (!(CoolSupported && OccupancySupported))
            return imcode::UnsupportedAttribute;
        if (requested < AbsMinCoolSetpointLimit || requested < MinCoolSetpointLimit || requested > AbsMaxCoolSetpointLimit ||
            requested > MaxCoolSetpointLimit)
            return imcode::InvalidValue;
        if (AutoSupported)
        {
            if (requested < UnoccupiedHeatingSetpoint + DeadBandTemp)
                return imcode::InvalidValue;
        }
        return imcode::Success;
    }

    case MinHeatSetpointLimit::Id: {
        requested = static_cast<int16_t>(chip::Encoding::LittleEndian::Get16(value));
        if (!HeatSupported)
            return imcode::UnsupportedAttribute;
        if (requested < AbsMinHeatSetpointLimit || requested > MaxHeatSetpointLimit || requested > AbsMaxHeatSetpointLimit)
            return imcode::InvalidValue;
        if (AutoSupported)
        {
            if (requested > MinCoolSetpointLimit - DeadBandTemp)
                return imcode::InvalidValue;
        }
        return imcode::Success;
    }
    case MaxHeatSetpointLimit::Id: {
        requested = static_cast<int16_t>(chip::Encoding::LittleEndian::Get16(value));
        if (!HeatSupported)
            return imcode::UnsupportedAttribute;
        if (requested < AbsMinHeatSetpointLimit || requested < MinHeatSetpointLimit || requested > AbsMaxHeatSetpointLimit)
            return imcode::InvalidValue;
        if (AutoSupported)
        {
            if (requested > MaxCoolSetpointLimit - DeadBandTemp)
                return imcode::InvalidValue;
        }
        return imcode::Success;
    }
    case MinCoolSetpointLimit::Id: {
        requested = static_cast<int16_t>(chip::Encoding::LittleEndian::Get16(value));
        if (!CoolSupported)
            return imcode::UnsupportedAttribute;
        if (requested < AbsMinCoolSetpointLimit || requested > MaxCoolSetpointLimit || requested > AbsMaxCoolSetpointLimit)
            return imcode::InvalidValue;
        if (AutoSupported)
        {
            if (requested < MinHeatSetpointLimit + DeadBandTemp)
                return imcode::InvalidValue;
        }
        return imcode::Success;
    }
    case MaxCoolSetpointLimit::Id: {
        requested = static_cast<int16_t>(chip::Encoding::LittleEndian::Get16(value));
        if (!CoolSupported)
            return imcode::UnsupportedAttribute;
        if (requested < AbsMinCoolSetpointLimit || requested < MinCoolSetpointLimit || requested > AbsMaxCoolSetpointLimit)
            return imcode::InvalidValue;
        if (AutoSupported)
        {
            if (requested < MaxHeatSetpointLimit + DeadBandTemp)
                return imcode::InvalidValue;
        }
        return imcode::Success;
    }
    case MinSetpointDeadBand::Id: {
        requested = *value;
        if (!AutoSupported)
            return imcode::UnsupportedAttribute;
        if (requested < 0 || requested > 25)
            return imcode::InvalidValue;
        return imcode::Success;
    }

    case ControlSequenceOfOperation::Id: {
        uint8_t requestedCSO;
        requestedCSO = *value;
        if (requestedCSO > to_underlying(ControlSequenceOfOperationEnum::kCoolingAndHeatingWithReheat))
            return imcode::InvalidValue;
        return imcode::Success;
    }

    case SystemMode::Id: {
        ControlSequenceOfOperationEnum ControlSequenceOfOperation;
        imcode status = ControlSequenceOfOperation::Get(endpoint, &ControlSequenceOfOperation);
        if (status != imcode::Success)
        {
            return imcode::InvalidValue;
        }
        auto RequestedSystemMode = static_cast<SystemModeEnum>(*value);
        if (ControlSequenceOfOperation > ControlSequenceOfOperationEnum::kCoolingAndHeatingWithReheat ||
            RequestedSystemMode > SystemModeEnum::kFanOnly)
        {
            return imcode::InvalidValue;
        }

        switch (ControlSequenceOfOperation)
        {
        case ControlSequenceOfOperationEnum::kCoolingOnly:
        case ControlSequenceOfOperationEnum::kCoolingWithReheat:
            if (RequestedSystemMode == SystemModeEnum::kHeat || RequestedSystemMode == SystemModeEnum::kEmergencyHeat)
                return imcode::InvalidValue;
            else
                return imcode::Success;

        case ControlSequenceOfOperationEnum::kHeatingOnly:
        case ControlSequenceOfOperationEnum::kHeatingWithReheat:
            if (RequestedSystemMode == SystemModeEnum::kCool || RequestedSystemMode == SystemModeEnum::kPrecooling)
                return imcode::InvalidValue;
            else
                return imcode::Success;
        default:
            return imcode::Success;
        }
    }
    default:
        return imcode::Success;
    }
}

bool emberAfThermostatClusterClearWeeklyScheduleCallback(app::CommandHandler * commandObj,
                                                         const app::ConcreteCommandPath & commandPath,
                                                         const Commands::ClearWeeklySchedule::DecodableType & commandData)
{
    // TODO
    return false;
}

bool emberAfThermostatClusterGetWeeklyScheduleCallback(app::CommandHandler * commandObj,
                                                       const app::ConcreteCommandPath & commandPath,
                                                       const Commands::GetWeeklySchedule::DecodableType & commandData)
{
    // TODO
    return false;
}

bool emberAfThermostatClusterSetWeeklyScheduleCallback(app::CommandHandler * commandObj,
                                                       const app::ConcreteCommandPath & commandPath,
                                                       const Commands::SetWeeklySchedule::DecodableType & commandData)
{
    // TODO
    return false;
}

bool emberAfThermostatClusterSetActiveScheduleRequestCallback(
    chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
    const chip::app::Clusters::Thermostat::Commands::SetActiveScheduleRequest::DecodableType & commandData)
{
    // TODO
    return false;
}

bool emberAfThermostatClusterSetActivePresetRequestCallback(
    chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
    const chip::app::Clusters::Thermostat::Commands::SetActivePresetRequest::DecodableType & commandData)
{
    EndpointId endpoint = commandPath.mEndpointId;
    Delegate * delegate = GetDelegate(endpoint);

    if (delegate == nullptr)
    {
        ChipLogError(Zcl, "Delegate is null");
        commandObj->AddStatus(commandPath, imcode::InvalidInState);
        return true;
    }

    ByteSpan newPresetHandle = commandData.presetHandle;

    // If the preset handle passed in the command is not present in the Presets attribute, return INVALID_COMMAND.
    if (!IsPresetHandlePresentInPresets(delegate, newPresetHandle))
    {
        commandObj->AddStatus(commandPath, imcode::InvalidCommand);
        return true;
    }

    CHIP_ERROR err = delegate->SetActivePresetHandle(DataModel::MakeNullable(newPresetHandle));

    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "Failed to set ActivePresetHandle with error %" CHIP_ERROR_FORMAT, err.Format());
        commandObj->AddStatus(commandPath, StatusIB(err).mStatus);
        return true;
    }

    commandObj->AddStatus(commandPath, imcode::Success);
    return true;
}

bool emberAfThermostatClusterStartPresetsSchedulesEditRequestCallback(
    chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
    const chip::app::Clusters::Thermostat::Commands::StartPresetsSchedulesEditRequest::DecodableType & commandData)
{
    ScopedNodeId sourceNodeId = GetSourceScopedNodeId(commandObj);

    EndpointId endpoint = commandPath.mEndpointId;

    // If the presets are editable and the scoped node id of the client sending StartPresetsSchedulesEditRequest command
    // is not the same as the one that previously originated a StartPresetsSchedulesEditRequest command, return BUSY.
    if (gThermostatAttrAccess.GetPresetsEditable(endpoint) &&
        (gThermostatAttrAccess.GetOriginatorScopedNodeId(endpoint) != sourceNodeId))
    {
        commandObj->AddStatus(commandPath, imcode::Busy);
        return true;
    }

    // If presets are editable and the scoped node id of the client sending StartPresetsSchedulesEditRequest command
    // is the same as the one that previously originated a StartPresetsSchedulesEditRequest command, extend the timer.
    if (gThermostatAttrAccess.GetPresetsEditable(endpoint))
    {
        ExtendTimer(endpoint, commandData.timeoutSeconds);
        commandObj->AddStatus(commandPath, imcode::Success);
        return true;
    }

    // Set presets editable to true and the scoped originator node id to the source scoped node id, and start a timer with the
    // timeout in seconds passed in the command args. Return success.
    gThermostatAttrAccess.SetPresetsEditable(endpoint, true);
    gThermostatAttrAccess.SetOriginatorScopedNodeId(endpoint, sourceNodeId);
    ScheduleTimer(endpoint, commandData.timeoutSeconds);
    commandObj->AddStatus(commandPath, imcode::Success);
    return true;
}

bool emberAfThermostatClusterCancelPresetsSchedulesEditRequestCallback(
    chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
    const chip::app::Clusters::Thermostat::Commands::CancelPresetsSchedulesEditRequest::DecodableType & commandData)
{
    EndpointId endpoint = commandPath.mEndpointId;

    // If presets are not editable, return INVALID_IN_STATE.
    if (!gThermostatAttrAccess.GetPresetsEditable(endpoint))
    {
        commandObj->AddStatus(commandPath, imcode::InvalidInState);
        return true;
    }

    ScopedNodeId sourceNodeId = GetSourceScopedNodeId(commandObj);

    // If the node id sending the CancelPresetsSchedulesRequest command is not the same as the one which send the
    // previous StartPresetsSchedulesEditRequest, return UNSUPPORTED_ACCESS.
    if (gThermostatAttrAccess.GetOriginatorScopedNodeId(endpoint) != sourceNodeId)
    {
        commandObj->AddStatus(commandPath, imcode::UnsupportedAccess);
        return true;
    }

    Delegate * delegate = GetDelegate(endpoint);

    if (delegate == nullptr)
    {
        ChipLogError(Zcl, "Delegate is null");
        return SendResponseAndCleanUp(delegate, endpoint, commandObj, commandPath, imcode::InvalidInState);
    }

    // Clear the timer, discard the changes and set PresetsEditable to false.
    return SendResponseAndCleanUp(delegate, endpoint, commandObj, commandPath, imcode::Success);
}

bool emberAfThermostatClusterCommitPresetsSchedulesRequestCallback(
    chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
    const chip::app::Clusters::Thermostat::Commands::CommitPresetsSchedulesRequest::DecodableType & commandData)
{
    EndpointId endpoint = commandPath.mEndpointId;
    Delegate * delegate = GetDelegate(endpoint);

    if (delegate == nullptr)
    {
        ChipLogError(Zcl, "Delegate is null");
        return SendResponseAndCleanUp(delegate, endpoint, commandObj, commandPath, imcode::InvalidInState);
    }

    // If presets are not editable, return INVALID_IN_STATE.
    if (!gThermostatAttrAccess.GetPresetsEditable(endpoint))
    {
        return SendResponseAndCleanUp(delegate, endpoint, commandObj, commandPath, imcode::InvalidInState);
    }

    ScopedNodeId sourceNodeId = GetSourceScopedNodeId(commandObj);

    // If the node id sending the CommitPresetsSchedulesRequest command is not the same as the one which send the
    // StartPresetsSchedulesEditRequest, return UNSUPPORTED_ACCESS.
    if (gThermostatAttrAccess.GetOriginatorScopedNodeId(endpoint) != sourceNodeId)
    {
        commandObj->AddStatus(commandPath, imcode::UnsupportedAccess);
        return true;
    }

    PresetStructWithOwnedMembers preset;
    CHIP_ERROR err = CHIP_NO_ERROR;

    // For each preset in the presets attribute, check that the matching preset in the pending presets list does not
    // violate any spec constraints.
    for (uint8_t i = 0; true; i++)
    {
        err = delegate->GetPresetAtIndex(i, preset);

        if (err == CHIP_ERROR_PROVIDER_LIST_EXHAUSTED)
        {
            break;
        }
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(Zcl,
                         "emberAfThermostatClusterCommitPresetsSchedulesRequestCallback: GetPresetAtIndex failed with error "
                         "%" CHIP_ERROR_FORMAT,
                         err.Format());
            return SendResponseAndCleanUp(delegate, endpoint, commandObj, commandPath, imcode::InvalidInState);
        }

        bool found = MatchingPendingPresetExists(delegate, preset);

        // If a built in preset in the Presets attribute list is removed and not found in the pending presets list, return
        // CONSTRAINT_ERROR.
        if (IsBuiltIn(preset) && !found)
        {
            return SendResponseAndCleanUp(delegate, endpoint, commandObj, commandPath, imcode::ConstraintError);
        }
    }

    // If there is an ActivePresetHandle set, find the preset in the pending presets list that matches the ActivePresetHandle
    // attribute. If a preset is not found with the same presetHandle, return INVALID_IN_STATE. If there is no ActivePresetHandle
    // attribute set, continue with other checks.
    uint8_t buffer[kPresetHandleSize];
    MutableByteSpan activePresetHandle(buffer);

    err = delegate->GetActivePresetHandle(activePresetHandle);

    if (err != CHIP_NO_ERROR)
    {
        return SendResponseAndCleanUp(delegate, endpoint, commandObj, commandPath, imcode::InvalidInState);
    }

    if (!activePresetHandle.empty())
    {
        uint8_t count = CountPresetsInPendingListWithPresetHandle(delegate, activePresetHandle);
        if (count == 0)
        {
            return SendResponseAndCleanUp(delegate, endpoint, commandObj, commandPath, imcode::InvalidInState);
        }
    }

    // For each preset in the pending presets list, check that the preset does not violate any spec constraints.
    for (uint8_t i = 0; true; i++)
    {
        PresetStructWithOwnedMembers pendingPreset;
        err = delegate->GetPendingPresetAtIndex(i, pendingPreset);

        if (err == CHIP_ERROR_PROVIDER_LIST_EXHAUSTED)
        {
            break;
        }
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(Zcl,
                         "emberAfThermostatClusterCommitPresetsSchedulesRequestCallback: GetPendingPresetAtIndex failed with error "
                         "%" CHIP_ERROR_FORMAT,
                         err.Format());
            return SendResponseAndCleanUp(delegate, endpoint, commandObj, commandPath, imcode::InvalidInState);
        }

        bool isPendingPresetWithNullPresetHandle = pendingPreset.GetPresetHandle().IsNull();

        // If the preset handle is null and the built in field is set to true, return CONSTRAINT_ERROR.
        if (isPendingPresetWithNullPresetHandle && IsBuiltIn(pendingPreset))
        {
            return SendResponseAndCleanUp(delegate, endpoint, commandObj, commandPath, imcode::ConstraintError);
        }

        bool foundMatchingPresetInPresets = false;
        PresetStructWithOwnedMembers matchingPreset;
        if (!isPendingPresetWithNullPresetHandle)
        {
            foundMatchingPresetInPresets = GetMatchingPresetInPresets(delegate, pendingPreset, matchingPreset);

            // If the presetHandle for the pending preset is not null and a matching preset is not found in the
            // presets attribute list, return NOT_FOUND.
            if (!foundMatchingPresetInPresets)
            {
                return SendResponseAndCleanUp(delegate, endpoint, commandObj, commandPath, imcode::NotFound);
            }

            // Find the number of presets in the pending preset list that match the preset handle. If there are duplicate
            // entries, return CONSTRAINT_ERROR.
            uint8_t count = CountPresetsInPendingListWithPresetHandle(delegate, pendingPreset.GetPresetHandle().Value());
            if (count > 1)
            {
                return SendResponseAndCleanUp(delegate, endpoint, commandObj, commandPath, imcode::ConstraintError);
            }
        }

        // If the preset is found in the  presets attribute list and the preset is builtIn in the pending presets list
        //  but not in the presets attribute list, return UNSUPPORTED_ACCESS.
        if (foundMatchingPresetInPresets && (IsBuiltIn(pendingPreset) && !IsBuiltIn(matchingPreset)))
        {
            return SendResponseAndCleanUp(delegate, endpoint, commandObj, commandPath, imcode::UnsupportedAccess);
        }

        // If the preset is found in the  presets attribute list and the preset is builtIn in the presets attribute
        //  but not in the pending presets list, return UNSUPPORTED_ACCESS.
        if (foundMatchingPresetInPresets && (!IsBuiltIn(pendingPreset) && IsBuiltIn(matchingPreset)))
        {
            return SendResponseAndCleanUp(delegate, endpoint, commandObj, commandPath, imcode::UnsupportedAccess);
        }

        // If the presetScenario is not found in the preset types, return CONSTRAINT_ERROR.
        PresetScenarioEnum presetScenario = pendingPreset.GetPresetScenario();
        if (!PresetScenarioExistsInPresetTypes(delegate, presetScenario))
        {
            return SendResponseAndCleanUp(delegate, endpoint, commandObj, commandPath, imcode::ConstraintError);
        }

        // If the preset type for the preset scenario does not support names and a name is specified, return CONSTRAINT_ERROR.
        if (!PresetTypeSupportsNames(delegate, presetScenario) && pendingPreset.GetName().HasValue())
        {
            return SendResponseAndCleanUp(delegate, endpoint, commandObj, commandPath, imcode::ConstraintError);
        }

        // Enforce the Setpoint Limits for both the cooling and heating setpoints in the pending preset.
        Optional<int16_t> coolingSetpointValue = pendingPreset.GetCoolingSetpoint();
        if (coolingSetpointValue.HasValue())
        {
            pendingPreset.SetCoolingSetpoint(MakeOptional(EnforceCoolingSetpointLimits(coolingSetpointValue.Value(), endpoint)));
        }

        Optional<int16_t> heatingSetpointValue = pendingPreset.GetHeatingSetpoint();
        if (heatingSetpointValue.HasValue())
        {
            pendingPreset.SetHeatingSetpoint(MakeOptional(EnforceHeatingSetpointLimits(heatingSetpointValue.Value(), endpoint)));
        }
    }

    uint8_t totalCount = CountUpdatedPresetsAfterApplyingPendingPresets(delegate);

    uint8_t numberOfPresetsSupported = delegate->GetNumberOfPresets();

    if (numberOfPresetsSupported == 0)
    {
        ChipLogError(Zcl, "emberAfThermostatClusterCommitPresetsSchedulesRequestCallback: Failed to get NumberOfPresets");
        return SendResponseAndCleanUp(delegate, endpoint, commandObj, commandPath, imcode::InvalidInState);
    }

    // If the expected length of the presets attribute with the applied changes exceeds the total number of presets supported,
    // return RESOURCE_EXHAUSTED. Note that the changes are not yet applied.
    if (numberOfPresetsSupported > 0 && totalCount > numberOfPresetsSupported)
    {
        return SendResponseAndCleanUp(delegate, endpoint, commandObj, commandPath, imcode::ResourceExhausted);
    }

    // TODO: Check if the number of presets for each presetScenario exceeds the max number of presets supported for that
    // scenario. We plan to support only one preset for each presetScenario for our use cases so defer this for re-evaluation.

    // Call the delegate API to apply the pending presets to the presets attribute and update it.
    err = delegate->ApplyPendingPresets();

    if (err != CHIP_NO_ERROR)
    {
        return SendResponseAndCleanUp(delegate, endpoint, commandObj, commandPath, imcode::InvalidInState);
    }

    return SendResponseAndCleanUp(delegate, endpoint, commandObj, commandPath, imcode::Success);
}

bool emberAfThermostatClusterSetpointRaiseLowerCallback(app::CommandHandler * commandObj,
                                                        const app::ConcreteCommandPath & commandPath,
                                                        const Commands::SetpointRaiseLower::DecodableType & commandData)
{
    auto & mode   = commandData.mode;
    auto & amount = commandData.amount;

    EndpointId aEndpointId = commandPath.mEndpointId;

    int16_t HeatingSetpoint = kDefaultHeatingSetpoint, CoolingSetpoint = kDefaultCoolingSetpoint; // Set to defaults to be safe
    imcode status                     = imcode::Failure;
    imcode WriteCoolingSetpointStatus = imcode::Failure;
    imcode WriteHeatingSetpointStatus = imcode::Failure;
    int16_t DeadBandTemp              = 0;
    int8_t DeadBand                   = 0;
    uint32_t OurFeatureMap;
    bool AutoSupported = false;
    bool HeatSupported = false;
    bool CoolSupported = false;

    if (FeatureMap::Get(aEndpointId, &OurFeatureMap) != imcode::Success)
        OurFeatureMap = FEATURE_MAP_DEFAULT;

    if (OurFeatureMap & 1 << 5) // Bit 5 is Auto Mode supported
        AutoSupported = true;

    if (OurFeatureMap & 1 << 0)
        HeatSupported = true;

    if (OurFeatureMap & 1 << 1)
        CoolSupported = true;

    if (AutoSupported)
    {
        if (MinSetpointDeadBand::Get(aEndpointId, &DeadBand) != imcode::Success)
            DeadBand = kDefaultDeadBand;
        DeadBandTemp = static_cast<int16_t>(DeadBand * 10);
    }

    switch (mode)
    {
    case SetpointRaiseLowerModeEnum::kBoth:
        if (HeatSupported && CoolSupported)
        {
            int16_t DesiredCoolingSetpoint, CoolLimit, DesiredHeatingSetpoint, HeatLimit;
            if (OccupiedCoolingSetpoint::Get(aEndpointId, &CoolingSetpoint) == imcode::Success)
            {
                DesiredCoolingSetpoint = static_cast<int16_t>(CoolingSetpoint + amount * 10);
                CoolLimit              = static_cast<int16_t>(DesiredCoolingSetpoint -
                                                 EnforceCoolingSetpointLimits(DesiredCoolingSetpoint, aEndpointId));
                {
                    if (OccupiedHeatingSetpoint::Get(aEndpointId, &HeatingSetpoint) == imcode::Success)
                    {
                        DesiredHeatingSetpoint = static_cast<int16_t>(HeatingSetpoint + amount * 10);
                        HeatLimit              = static_cast<int16_t>(DesiredHeatingSetpoint -
                                                         EnforceHeatingSetpointLimits(DesiredHeatingSetpoint, aEndpointId));
                        {
                            if (CoolLimit != 0 || HeatLimit != 0)
                            {
                                if (abs(CoolLimit) <= abs(HeatLimit))
                                {
                                    // We are limited by the Heating Limit
                                    DesiredHeatingSetpoint = static_cast<int16_t>(DesiredHeatingSetpoint - HeatLimit);
                                    DesiredCoolingSetpoint = static_cast<int16_t>(DesiredCoolingSetpoint - HeatLimit);
                                }
                                else
                                {
                                    // We are limited by Cooling Limit
                                    DesiredHeatingSetpoint = static_cast<int16_t>(DesiredHeatingSetpoint - CoolLimit);
                                    DesiredCoolingSetpoint = static_cast<int16_t>(DesiredCoolingSetpoint - CoolLimit);
                                }
                            }
                            WriteCoolingSetpointStatus = OccupiedCoolingSetpoint::Set(aEndpointId, DesiredCoolingSetpoint);
                            if (WriteCoolingSetpointStatus != imcode::Success)
                            {
                                ChipLogError(Zcl, "Error: SetOccupiedCoolingSetpoint failed!");
                            }
                            WriteHeatingSetpointStatus = OccupiedHeatingSetpoint::Set(aEndpointId, DesiredHeatingSetpoint);
                            if (WriteHeatingSetpointStatus != imcode::Success)
                            {
                                ChipLogError(Zcl, "Error: SetOccupiedHeatingSetpoint failed!");
                            }
                        }
                    }
                }
            }
        }

        if (CoolSupported && !HeatSupported)
        {
            if (OccupiedCoolingSetpoint::Get(aEndpointId, &CoolingSetpoint) == imcode::Success)
            {
                CoolingSetpoint            = static_cast<int16_t>(CoolingSetpoint + amount * 10);
                CoolingSetpoint            = EnforceCoolingSetpointLimits(CoolingSetpoint, aEndpointId);
                WriteCoolingSetpointStatus = OccupiedCoolingSetpoint::Set(aEndpointId, CoolingSetpoint);
                if (WriteCoolingSetpointStatus != imcode::Success)
                {
                    ChipLogError(Zcl, "Error: SetOccupiedCoolingSetpoint failed!");
                }
            }
        }

        if (HeatSupported && !CoolSupported)
        {
            if (OccupiedHeatingSetpoint::Get(aEndpointId, &HeatingSetpoint) == imcode::Success)
            {
                HeatingSetpoint            = static_cast<int16_t>(HeatingSetpoint + amount * 10);
                HeatingSetpoint            = EnforceHeatingSetpointLimits(HeatingSetpoint, aEndpointId);
                WriteHeatingSetpointStatus = OccupiedHeatingSetpoint::Set(aEndpointId, HeatingSetpoint);
                if (WriteHeatingSetpointStatus != imcode::Success)
                {
                    ChipLogError(Zcl, "Error: SetOccupiedHeatingSetpoint failed!");
                }
            }
        }

        if ((!HeatSupported || WriteHeatingSetpointStatus == imcode::Success) &&
            (!CoolSupported || WriteCoolingSetpointStatus == imcode::Success))
            status = imcode::Success;
        break;

    case SetpointRaiseLowerModeEnum::kCool:
        if (CoolSupported)
        {
            if (OccupiedCoolingSetpoint::Get(aEndpointId, &CoolingSetpoint) == imcode::Success)
            {
                CoolingSetpoint = static_cast<int16_t>(CoolingSetpoint + amount * 10);
                CoolingSetpoint = EnforceCoolingSetpointLimits(CoolingSetpoint, aEndpointId);
                if (AutoSupported)
                {
                    // Need to check if we can move the cooling setpoint while maintaining the dead band
                    if (OccupiedHeatingSetpoint::Get(aEndpointId, &HeatingSetpoint) == imcode::Success)
                    {
                        if (CoolingSetpoint - HeatingSetpoint < DeadBandTemp)
                        {
                            // Dead Band Violation
                            // Try to adjust it
                            HeatingSetpoint = static_cast<int16_t>(CoolingSetpoint - DeadBandTemp);
                            if (HeatingSetpoint == EnforceHeatingSetpointLimits(HeatingSetpoint, aEndpointId))
                            {
                                // Desired cooling setpoint is enforcable
                                // Set the new cooling and heating setpoints
                                if (OccupiedHeatingSetpoint::Set(aEndpointId, HeatingSetpoint) == imcode::Success)
                                {
                                    if (OccupiedCoolingSetpoint::Set(aEndpointId, CoolingSetpoint) == imcode::Success)
                                        status = imcode::Success;
                                }
                                else
                                    ChipLogError(Zcl, "Error: SetOccupiedHeatingSetpoint failed!");
                            }
                            else
                            {
                                ChipLogError(Zcl, "Error: Could Not adjust heating setpoint to maintain dead band!");
                                status = imcode::InvalidCommand;
                            }
                        }
                        else
                            status = OccupiedCoolingSetpoint::Set(aEndpointId, CoolingSetpoint);
                    }
                    else
                        ChipLogError(Zcl, "Error: GetOccupiedHeatingSetpoint failed!");
                }
                else
                {
                    status = OccupiedCoolingSetpoint::Set(aEndpointId, CoolingSetpoint);
                }
            }
            else
                ChipLogError(Zcl, "Error: GetOccupiedCoolingSetpoint failed!");
        }
        else
            status = imcode::InvalidCommand;
        break;

    case SetpointRaiseLowerModeEnum::kHeat:
        if (HeatSupported)
        {
            if (OccupiedHeatingSetpoint::Get(aEndpointId, &HeatingSetpoint) == imcode::Success)
            {
                HeatingSetpoint = static_cast<int16_t>(HeatingSetpoint + amount * 10);
                HeatingSetpoint = EnforceHeatingSetpointLimits(HeatingSetpoint, aEndpointId);
                if (AutoSupported)
                {
                    // Need to check if we can move the cooling setpoint while maintaining the dead band
                    if (OccupiedCoolingSetpoint::Get(aEndpointId, &CoolingSetpoint) == imcode::Success)
                    {
                        if (CoolingSetpoint - HeatingSetpoint < DeadBandTemp)
                        {
                            // Dead Band Violation
                            // Try to adjust it
                            CoolingSetpoint = static_cast<int16_t>(HeatingSetpoint + DeadBandTemp);
                            if (CoolingSetpoint == EnforceCoolingSetpointLimits(CoolingSetpoint, aEndpointId))
                            {
                                // Desired cooling setpoint is enforcable
                                // Set the new cooling and heating setpoints
                                if (OccupiedCoolingSetpoint::Set(aEndpointId, CoolingSetpoint) == imcode::Success)
                                {
                                    if (OccupiedHeatingSetpoint::Set(aEndpointId, HeatingSetpoint) == imcode::Success)
                                        status = imcode::Success;
                                }
                                else
                                    ChipLogError(Zcl, "Error: SetOccupiedCoolingSetpoint failed!");
                            }
                            else
                            {
                                ChipLogError(Zcl, "Error: Could Not adjust cooling setpoint to maintain dead band!");
                                status = imcode::InvalidCommand;
                            }
                        }
                        else
                            status = OccupiedHeatingSetpoint::Set(aEndpointId, HeatingSetpoint);
                    }
                    else
                        ChipLogError(Zcl, "Error: GetOccupiedCoolingSetpoint failed!");
                }
                else
                {
                    status = OccupiedHeatingSetpoint::Set(aEndpointId, HeatingSetpoint);
                }
            }
            else
                ChipLogError(Zcl, "Error: GetOccupiedHeatingSetpoint failed!");
        }
        else
            status = imcode::InvalidCommand;
        break;

    default:
        status = imcode::InvalidCommand;
        break;
    }

    commandObj->AddStatus(commandPath, status);
    return true;
}

void MatterThermostatPluginServerInitCallback()
{
    registerAttributeAccessOverride(&gThermostatAttrAccess);
}
