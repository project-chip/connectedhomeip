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

// clusters specific header
#include "level-control.h"
#include "impl/level-control-internals.h"
// #if defined(IGNORE_LEVEL_CONTROL_CLUSTER_LEVEL_CONTROL_REMAINING_TIME)
// #include "impl/level-control-ignore-remaining-time.cpp"
// #else
// #include "impl/level-control-default.cpp"
// #endif

#include <algorithm>

// this file contains all the common includes for clusters in the util
#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <app/CommandHandler.h>
#include <app/ConcreteCommandPath.h>
#include <app/cluster-building-blocks/QuieterReporting.h>
#include <app/util/attribute-storage.h>
#include <app/util/config.h>
#include <app/util/util.h>

#include <app/reporting/reporting.h>
#include <lib/core/Optional.h>
#include <platform/CHIPDeviceConfig.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/PlatformManager.h>
#include <tracing/macros.h>

#ifdef MATTER_DM_PLUGIN_SCENES_MANAGEMENT
#include <app/clusters/scenes-server/CodegenAttributeValuePairValidator.h> // nogncheck
#include <app/clusters/scenes-server/scenes-server.h>                      // nogncheck
#endif // MATTER_DM_PLUGIN_SCENES_MANAGEMENT

#ifdef MATTER_DM_PLUGIN_ON_OFF
#include <app/clusters/on-off-server/on-off-server.h>
#endif // MATTER_DM_PLUGIN_ON_OFF

#ifdef MATTER_DM_PLUGIN_COLOR_CONTROL_SERVER_TEMP
#include <app/clusters/color-control-server/color-control-server.h>
#endif // MATTER_DM_PLUGIN_COLOR_CONTROL_SERVER_TEMP

#include <assert.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::LevelControl;
using chip::Protocols::InteractionModel::Status;

#if (MATTER_DM_PLUGIN_LEVEL_CONTROL_RATE == 0)
#define FASTEST_TRANSITION_TIME_MS 0
#else
#define FASTEST_TRANSITION_TIME_MS (MILLISECOND_TICKS_PER_SECOND / MATTER_DM_PLUGIN_LEVEL_CONTROL_RATE)
#endif // MATTER_DM_PLUGIN_LEVEL_CONTROL_RATE

#define LEVEL_CONTROL_LIGHTING_MIN_LEVEL 0x01
#define LEVEL_CONTROL_LIGHTING_MAX_LEVEL 0xFE

#define INVALID_STORED_LEVEL 0xFFFF

static constexpr size_t kLevelControlStateTableSize =
    MATTER_DM_LEVEL_CONTROL_CLUSTER_SERVER_ENDPOINT_COUNT + CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT;
static_assert(kLevelControlStateTableSize <= kEmberInvalidEndpointIndex, "LevelControl state table size error");

static EmberAfLevelControlState stateTable[kLevelControlStateTableSize];

EmberAfLevelControlState * getState(EndpointId endpoint);

static Status moveToLevelHandler(EndpointId endpoint, CommandId commandId, uint8_t level,
                                 DataModel::Nullable<uint16_t> transitionTimeDs, chip::Optional<BitMask<OptionsBitmap>> optionsMask,
                                 chip::Optional<BitMask<OptionsBitmap>> optionsOverride, uint16_t storedLevel);
static void moveHandler(CommandHandler * commandObj, const ConcreteCommandPath & commandPath, MoveModeEnum moveMode,
                        DataModel::Nullable<uint8_t> rate, chip::Optional<BitMask<OptionsBitmap>> optionsMask,
                        chip::Optional<BitMask<OptionsBitmap>> optionsOverride);
static void stepHandler(CommandHandler * commandObj, const ConcreteCommandPath & commandPath, StepModeEnum stepMode,
                        uint8_t stepSize, DataModel::Nullable<uint16_t> transitionTimeDs,
                        chip::Optional<BitMask<OptionsBitmap>> optionsMask, chip::Optional<BitMask<OptionsBitmap>> optionsOverride);
static void stopHandler(CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
                        chip::Optional<BitMask<OptionsBitmap>> optionsMask, chip::Optional<BitMask<OptionsBitmap>> optionsOverride);

static void setOnOffValue(EndpointId endpoint, bool onOff);
static bool shouldExecuteIfOff(EndpointId endpoint, CommandId commandId, chip::Optional<chip::BitMask<OptionsBitmap>> optionsMask,
                               chip::Optional<chip::BitMask<OptionsBitmap>> optionsOverride);

#if defined(MATTER_DM_PLUGIN_SCENES_MANAGEMENT) && CHIP_CONFIG_SCENES_USE_DEFAULT_HANDLERS
class DefaultLevelControlSceneHandler : public scenes::DefaultSceneHandlerImpl
{
public:
    // As per spec, 2 attributes are scenable in the level control cluster
    static constexpr uint8_t kLevelMaxScenableAttributes = 2;

    DefaultLevelControlSceneHandler() : scenes::DefaultSceneHandlerImpl(scenes::CodegenAttributeValuePairValidator::Instance()) {}
    ~DefaultLevelControlSceneHandler() override {}

    // Default function for LevelControl cluster, only checks if LevelControl is enabled on the endpoint
    bool SupportsCluster(EndpointId endpoint, ClusterId cluster) override
    {
        return (cluster == LevelControl::Id) && (emberAfContainsServer(endpoint, LevelControl::Id));
    }

    /// @brief Serialize the Cluster's EFS value
    /// @param endpoint target endpoint
    /// @param cluster  target cluster
    /// @param serializedBytes data to serialize into EFS
    /// @return CHIP_NO_ERROR if successfully serialized the data, CHIP_ERROR_INVALID_ARGUMENT otherwise
    CHIP_ERROR SerializeSave(EndpointId endpoint, ClusterId cluster, MutableByteSpan & serializedBytes) override
    {
        using AttributeValuePair = ScenesManagement::Structs::AttributeValuePairStruct::Type;

        DataModel::Nullable<uint8_t> level;
        VerifyOrReturnError(Status::Success == Attributes::CurrentLevel::Get(endpoint, level), CHIP_ERROR_READ_FAILED);

        AttributeValuePair pairs[kLevelMaxScenableAttributes];

        uint8_t maxLevel;
        VerifyOrReturnError(Status::Success == Attributes::MaxLevel::Get(endpoint, &maxLevel), CHIP_ERROR_READ_FAILED);

        pairs[0].attributeID = Attributes::CurrentLevel::Id;
        if (!level.IsNull())
        {
            pairs[0].valueUnsigned8.SetValue(level.Value());
        }
        else
        {
            pairs[0].valueUnsigned8.SetValue(NumericAttributeTraits<uint8_t>::kNullValue);
        }
        size_t attributeCount = 1;
        if (LevelControlHasFeature(endpoint, LevelControl::Feature::kFrequency))
        {
            uint16_t frequency;
            VerifyOrReturnError(Status::Success == Attributes::CurrentFrequency::Get(endpoint, &frequency), CHIP_ERROR_READ_FAILED);
            pairs[attributeCount].attributeID = Attributes::CurrentFrequency::Id;
            pairs[attributeCount].valueUnsigned16.SetValue(frequency);
            attributeCount++;
        }

        DataModel::List<AttributeValuePair> attributeValueList(pairs, attributeCount);

        return EncodeAttributeValueList(attributeValueList, serializedBytes);
    }

    /// @brief Default EFS interaction when applying scene to the OnOff Cluster
    /// @param endpoint target endpoint
    /// @param cluster  target cluster
    /// @param serializedBytes Data from nvm
    /// @param timeMs transition time in ms
    /// @return CHIP_NO_ERROR if value as expected, CHIP_ERROR_INVALID_ARGUMENT otherwise
    CHIP_ERROR ApplyScene(EndpointId endpoint, ClusterId cluster, const ByteSpan & serializedBytes,
                          scenes::TransitionTimeMs timeMs) override
    {
        DataModel::DecodableList<ScenesManagement::Structs::AttributeValuePairStruct::DecodableType> attributeValueList;

        ReturnErrorOnFailure(DecodeAttributeValueList(serializedBytes, attributeValueList));

        size_t attributeCount = 0;
        ReturnErrorOnFailure(attributeValueList.ComputeSize(&attributeCount));
        VerifyOrReturnError(attributeCount <= kLevelMaxScenableAttributes, CHIP_ERROR_BUFFER_TOO_SMALL);

        auto pair_iterator = attributeValueList.begin();

        // The level control cluster should have a maximum of 2 attributes
        uint8_t level = 0;
        // TODO : Uncomment when frequency is supported by the level control cluster
        // uint16_t frequency;
        while (pair_iterator.Next())
        {
            auto & decodePair = pair_iterator.GetValue();

            // If attribute ID was encoded, checks which attribute from LC cluster is there
            switch (decodePair.attributeID)
            {
            case Attributes::CurrentLevel::Id:
                VerifyOrReturnError(decodePair.valueUnsigned8.HasValue(), CHIP_ERROR_INVALID_ARGUMENT);
                level = decodePair.valueUnsigned8.Value();
                break;
            case Attributes::CurrentFrequency::Id:
                // TODO : Uncomment when frequency is supported by the level control cluster
                // VerifyOrReturnError(decodePair.valueUnsigned16.HasValue(), CHIP_ERROR_INVALID_ARGUMENT);
                // frequency = decodePair.valueUnsigned16.Value();
                break;
            default:
                return CHIP_ERROR_INVALID_ARGUMENT;
            }
        }
        ReturnErrorOnFailure(pair_iterator.GetStatus());

        // TODO : Implement action on frequency when frequency not provisional anymore
        // if(LevelControlHasFeature(endpoint, LevelControl::Feature::kFrequency)){}

        EmberAfLevelControlState * state = getState(endpoint);
        if (level < state->minLevel || level > state->maxLevel)
        {
            NumericAttributeTraits<uint8_t>::SetNull(level);
        }

        if (!NumericAttributeTraits<uint8_t>::IsNullValue(level))
        {
            moveToLevelHandler(
                endpoint, Commands::MoveToLevel::Id, level, DataModel::MakeNullable(static_cast<uint16_t>(timeMs / 100)),
                chip::Optional<BitMask<OptionsBitmap>>(1), chip::Optional<BitMask<OptionsBitmap>>(1), INVALID_STORED_LEVEL);
        }

        return CHIP_NO_ERROR;
    }
};
static DefaultLevelControlSceneHandler sLevelControlSceneHandler;

#endif // defined(MATTER_DM_PLUGIN_SCENES_MANAGEMENT) && CHIP_CONFIG_SCENES_USE_DEFAULT_HANDLERS

#if !defined(IGNORE_LEVEL_CONTROL_CLUSTER_OPTIONS) && defined(MATTER_DM_PLUGIN_COLOR_CONTROL_SERVER_TEMP)
static void reallyUpdateCoupledColorTemp(EndpointId endpoint);
#define updateCoupledColorTemp(endpoint) reallyUpdateCoupledColorTemp(endpoint)
#else
#define updateCoupledColorTemp(endpoint)
#endif // IGNORE_LEVEL_CONTROL_CLUSTER_OPTIONS && MATTER_DM_PLUGIN_COLOR_CONTROL_SERVER_TEMP

void emberAfLevelControlClusterServerTickCallback(EndpointId endpoint);

static void timerCallback(System::Layer *, void * callbackContext)
{
    emberAfLevelControlClusterServerTickCallback(static_cast<EndpointId>(reinterpret_cast<uintptr_t>(callbackContext)));
}

static uint32_t computeCallbackWaitTimeMs(CallbackScheduleState & callbackSchedule, uint32_t delayMs)
{
    auto delay             = System::Clock::Milliseconds32(delayMs);
    auto waitTime          = delay;
    const auto currentTime = System::SystemClock().GetMonotonicTimestamp();

    // Subsequent call
    if (callbackSchedule.runTime.count())
    {
        // Check whether the previous scheduled callback was late and whether its running time
        // is smaller than the desired delay
        // If the running time of the scheduled callback is greater than the desired delay
        // then do nothing; do not flood the event loop if the device is not fast enough
        if ((currentTime > callbackSchedule.idealTimestamp) && (callbackSchedule.runTime < delay))
        {
            System::Clock::Timestamp latency = currentTime - callbackSchedule.idealTimestamp;

            if (latency >= delay)
            {
                waitTime = System::Clock::Milliseconds32(0);
            }
            else
            {
                waitTime -= latency;
            }
        }
    }
    // First-time call
    else
    {
        // initialize idealTimestamp
        callbackSchedule.idealTimestamp = currentTime;
    }

    callbackSchedule.idealTimestamp += System::Clock::Milliseconds32(delayMs);
    callbackSchedule.runTime = System::Clock::Milliseconds32(0);

    return waitTime.count();
}

static void scheduleTimerCallbackMs(EndpointId endpoint, uint32_t delayMs)
{
    CHIP_ERROR err = DeviceLayer::SystemLayer().StartTimer(chip::System::Clock::Milliseconds32(delayMs), timerCallback,
                                                           reinterpret_cast<void *>(static_cast<uintptr_t>(endpoint)));

    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "Level Control Server failed to schedule event: %" CHIP_ERROR_FORMAT, err.Format());
    }
}

static void cancelEndpointTimerCallback(EndpointId endpoint)
{
    DeviceLayer::SystemLayer().CancelTimer(timerCallback, reinterpret_cast<void *>(static_cast<uintptr_t>(endpoint)));
}

EmberAfLevelControlState * getState(EndpointId endpoint)
{
    uint16_t ep =
        emberAfGetClusterServerEndpointIndex(endpoint, LevelControl::Id, MATTER_DM_LEVEL_CONTROL_CLUSTER_SERVER_ENDPOINT_COUNT);
    return (ep >= kLevelControlStateTableSize ? nullptr : &stateTable[ep]);
}

#if !defined(IGNORE_LEVEL_CONTROL_CLUSTER_OPTIONS) && defined(MATTER_DM_PLUGIN_COLOR_CONTROL_SERVER_TEMP)
static void reallyUpdateCoupledColorTemp(EndpointId endpoint)
{
    LevelControl::Attributes::Options::TypeInfo::Type options;
    Status status = Attributes::Options::Get(endpoint, &options);
    if (status != Status::Success)
    {
        ChipLogProgress(Zcl, "Unable to read Options attribute: 0x%X", to_underlying(status));
        return;
    }

    if (emberAfContainsAttribute(endpoint, ColorControl::Id, ColorControl::Attributes::ColorTemperatureMireds::Id))
    {
        if (options.Has(OptionsBitmap::kCoupleColorTempToLevel))
        {
            emberAfPluginLevelControlCoupledColorTempChangeCallback(endpoint);
        }
    }
}
#endif // IGNORE_LEVEL_CONTROL_CLUSTER_OPTIONS && MATTER_DM_PLUGIN_COLOR_CONTROL_SERVER_TEMP

/*
 * @brief
 * This function is used to update the current level attribute
 * while respecting its defined quiet reporting quality:
 * The attribute will be reported:
 * - At most once per second, or
 * - At the end of the movement/transition, or
 * - When it changes from null to any other value and vice versa.
 *
 * @param endpoint: endpoint on which the currentLevel attribute must be updated.
 * @param state: LevelControlState struct of this given endpoint.
 * @param newValue: Value to update the attribute with
 * @param isEndOfTransition: Boolean that indicate whether the update is occuring at the end of a level transition
 * @return Success in setting the attribute value or the IM error code for the failure.
 */
Status SetCurrentLevelQuietReport(EndpointId endpoint, EmberAfLevelControlState * state, DataModel::Nullable<uint8_t> newValue,
                                  bool isEndOfTransition)
{
    AttributeDirtyState dirtyState;
    auto now = System::SystemClock().GetMonotonicTimestamp();

    if (isEndOfTransition)
    {
        // At the start or end of the movement/transition we must report
        auto predicate = [](const decltype(state->quietCurrentLevel)::SufficientChangePredicateCandidate &) -> bool {
            return true;
        };
        dirtyState = state->quietCurrentLevel.SetValue(newValue, now, predicate);
    }
    else
    {
        // During transtions, reports should be at most once per second
        System::Clock::Milliseconds64 reportInterval =
            std::max(System::Clock::Milliseconds64(1000), System::Clock::Milliseconds64(state->transitionTimeMs / 4));
        auto predicate = state->quietCurrentLevel.GetPredicateForSufficientTimeSinceLastDirty(reportInterval);
        dirtyState     = state->quietCurrentLevel.SetValue(newValue, now, predicate);
    }

    MarkAttributeDirty markDirty = MarkAttributeDirty::kNo;
    if (dirtyState == AttributeDirtyState::kMustReport)
    {
        markDirty = MarkAttributeDirty::kYes;
    }
    return Attributes::CurrentLevel::Set(endpoint, state->quietCurrentLevel.value(), markDirty);
}

void emberAfLevelControlClusterServerTickCallback(EndpointId endpoint)
{
    EmberAfLevelControlState * state = getState(endpoint);
    Status status;
    DataModel::Nullable<uint8_t> currentLevel;
    const auto callbackStartTimestamp = System::SystemClock().GetMonotonicTimestamp();
    bool isTransitionStart            = false;
    bool isTransitionEnd              = false;

    if (state == nullptr)
    {
        return;
    }

    isTransitionStart = (state->elapsedTimeMs == 0);
    state->elapsedTimeMs += state->eventDurationMs;

    // Read the attribute; print error message and return if it can't be read
    status = LevelControl::Attributes::CurrentLevel::Get(endpoint, currentLevel);

    if (status != Status::Success || currentLevel.IsNull())
    {
        ChipLogProgress(Zcl, "ERR: reading current level %x", to_underlying(status));
        state->callbackSchedule.runTime = System::Clock::Milliseconds32(0);
        writeRemainingTime(endpoint, 0);
        return;
    }

    ChipLogDetail(Zcl, "Event: move from %d", currentLevel.Value());

    // adjust by the proper amount, either up or down
    if (state->transitionTimeMs == 0)
    {
        // Immediate, not over a time interval.
        currentLevel.SetNonNull(state->moveToLevel);
    }
    else if (state->increasing)
    {
        assert(currentLevel.Value() < state->maxLevel);
        assert(currentLevel.Value() < state->moveToLevel);
        currentLevel.SetNonNull(static_cast<uint8_t>(currentLevel.Value() + 1));
    }
    else
    {
        assert(state->minLevel < currentLevel.Value());
        assert(state->moveToLevel < currentLevel.Value());
        currentLevel.SetNonNull(static_cast<uint8_t>(currentLevel.Value() - 1));
    }

    ChipLogDetail(Zcl, " to %d ", currentLevel.Value());
    ChipLogDetail(Zcl, "(diff %c1)", state->increasing ? '+' : '-');

    // Are we at the requested level?
    isTransitionEnd = (currentLevel.Value() == state->moveToLevel);
    status          = SetCurrentLevelQuietReport(endpoint, state, currentLevel, isTransitionEnd);
    if (status != Status::Success)
    {
        ChipLogProgress(Zcl, "ERR: writing current level %x", to_underlying(status));
        state->callbackSchedule.runTime = System::Clock::Milliseconds32(0);
        writeRemainingTime(endpoint, 0);
        return;
    }

    updateCoupledColorTemp(endpoint);

    if (isTransitionEnd)
    {
        if (state->commandId == Commands::MoveToLevelWithOnOff::Id || state->commandId == Commands::MoveWithOnOff::Id ||
            state->commandId == Commands::StepWithOnOff::Id)
        {
            setOnOffValue(endpoint, (currentLevel.Value() != state->minLevel));
        }

        if (state->storedLevel != INVALID_STORED_LEVEL)
        {
            uint8_t storedLevel8u = (uint8_t) state->storedLevel;
            status                = Attributes::CurrentLevel::Set(endpoint, storedLevel8u);
            if (status != Status::Success)
            {
                ChipLogProgress(Zcl, "ERR: writing current level %x", to_underlying(status));
            }
            else
            {
                updateCoupledColorTemp(endpoint);
            }
        }

        state->callbackSchedule.runTime = System::Clock::Milliseconds32(0);
        writeRemainingTime(endpoint, 0);
    }
    else
    {
        state->callbackSchedule.runTime = System::SystemClock().GetMonotonicTimestamp() - callbackStartTimestamp;
        writeRemainingTime(endpoint, static_cast<uint16_t>(state->transitionTimeMs - state->elapsedTimeMs), isTransitionStart);
        scheduleTimerCallbackMs(endpoint, computeCallbackWaitTimeMs(state->callbackSchedule, state->eventDurationMs));
    }
}

static void setOnOffValue(EndpointId endpoint, bool onOff)
{
#ifdef MATTER_DM_PLUGIN_ON_OFF
    if (emberAfContainsServer(endpoint, OnOff::Id))
    {
        ChipLogProgress(Zcl, "Setting on/off to %s due to level change", onOff ? "ON" : "OFF");
        OnOffServer::Instance().setOnOffValue(endpoint, (onOff ? OnOff::Commands::On::Id : OnOff::Commands::Off::Id), true);
    }
#endif // MATTER_DM_PLUGIN_ON_OFF
}

static bool shouldExecuteIfOff(EndpointId endpoint, CommandId commandId, chip::Optional<chip::BitMask<OptionsBitmap>> optionsMask,
                               chip::Optional<chip::BitMask<OptionsBitmap>> optionsOverride)
{
#ifndef IGNORE_LEVEL_CONTROL_CLUSTER_OPTIONS
    if (emberAfContainsAttribute(endpoint, LevelControl::Id, Attributes::Options::Id))
    {
        // From 3.10.2.2.8.1 of ZCL7 document 14-0127-20j-zcl-ch-3-general.docx:
        //   "Command execution SHALL NOT continue beyond the Options processing if
        //    all of these criteria are true:
        //      - The command is one of the ‘without On/Off’ commands: Move, Move to
        //        Level, Stop, or Step.
        //      - The On/Off cluster exists on the same endpoint as this cluster.
        //      - The OnOff attribute of the On/Off cluster, on this endpoint, is 0x00
        //        (FALSE).
        //      - The value of the ExecuteIfOff bit is 0."
        if (commandId > Commands::Stop::Id)
        {
            return true;
        }

        if (!emberAfContainsServer(endpoint, OnOff::Id))
        {
            return true;
        }

        LevelControl::Attributes::Options::TypeInfo::Type options;
        Status status = Attributes::Options::Get(endpoint, &options);
        if (status != Status::Success)
        {
            ChipLogProgress(Zcl, "Unable to read Options attribute: 0x%X", to_underlying(status));
            // If we can't read the attribute, then we should just assume that it has its
            // default value.
        }

        bool on;
        status = OnOff::Attributes::OnOff::Get(endpoint, &on);
        if (status != Status::Success)
        {
            ChipLogProgress(Zcl, "Unable to read OnOff attribute: 0x%X", to_underlying(status));
            return true;
        }
        // The device is on - hence ExecuteIfOff does not matter
        if (on)
        {
            return true;
        }
        // The OptionsMask & OptionsOverride fields SHALL both be present or both
        // omitted in the command. A temporary Options bitmap SHALL be created from
        // the Options attribute, using the OptionsMask & OptionsOverride fields, if
        // present. Each bit of the temporary Options bitmap SHALL be determined as
        // follows:
        // Each bit in the Options attribute SHALL determine the corresponding bit in
        // the temporary Options bitmap, unless the OptionsMask field is present and
        // has the corresponding bit set to 1, in which case the corresponding bit in
        // the OptionsOverride field SHALL determine the corresponding bit in the
        // temporary Options bitmap.
        // The resulting temporary Options bitmap SHALL then be processed as defined
        // in section 3.10.2.2.3.

        // ---------- The following order is important in decision making -------
        // -----------more readable ----------
        //
        if (!optionsMask.HasValue() || !optionsOverride.HasValue())
        {
            // in case optionMask or optionOverride is not set, use of option
            // attribute to decide execution of the command
            return options.Has(OptionsBitmap::kExecuteIfOff);
        }
        // ---------- The above is to distinguish if the payload is present or not

        if (optionsMask.Value().Has(OptionsBitmap::kExecuteIfOff))
        {
            // Mask is present and set in the command payload, this indicates
            // use the over ride as temporary option
            return optionsOverride.Value().Has(OptionsBitmap::kExecuteIfOff);
        }
        // if we are here - use the option bits
        return options.Has(OptionsBitmap::kExecuteIfOff);
    }

#endif // IGNORE_LEVEL_CONTROL_CLUSTER_OPTIONS
       // By default, we return true to continue supporting backwards compatibility.
    return true;
}

bool emberAfLevelControlClusterMoveToLevelCallback(CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
                                                   const Commands::MoveToLevel::DecodableType & commandData)
{
    MATTER_TRACE_SCOPE("MoveToLevel", "LevelControl");
    commandObj->AddStatus(commandPath, LevelControlServer::MoveToLevel(commandPath.mEndpointId, commandData));
    return true;
}

namespace LevelControlServer {

Status MoveToLevel(EndpointId endpointId, const Commands::MoveToLevel::DecodableType & commandData)
{
    auto & level           = commandData.level;
    auto & transitionTime  = commandData.transitionTime;
    auto & optionsMask     = commandData.optionsMask;
    auto & optionsOverride = commandData.optionsOverride;

    if (transitionTime.IsNull())
    {
        ChipLogProgress(Zcl, "%s MOVE_TO_LEVEL %x null %x %x", "RX level-control:", level, optionsMask.Raw(),
                        optionsOverride.Raw());
    }
    else
    {
        ChipLogProgress(Zcl, "%s MOVE_TO_LEVEL %x %2x %x %x", "RX level-control:", level, transitionTime.Value(), optionsMask.Raw(),
                        optionsOverride.Raw());
    }

    return moveToLevelHandler(endpointId, Commands::MoveToLevel::Id, level, transitionTime,
                              Optional<BitMask<OptionsBitmap>>(optionsMask), Optional<BitMask<OptionsBitmap>>(optionsOverride),
                              INVALID_STORED_LEVEL); // Don't revert to the stored level
}

#ifdef MATTER_DM_PLUGIN_SCENES_MANAGEMENT
chip::scenes::SceneHandler * GetSceneHandler()
{
#if CHIP_CONFIG_SCENES_USE_DEFAULT_HANDLERS
    return &sLevelControlSceneHandler;
#else
    return nullptr;
#endif // CHIP_CONFIG_SCENES_USE_DEFAULT_HANDLERS
}
#endif // ifdef MATTER_DM_PLUGIN_SCENES_MANAGEMENT

} // namespace LevelControlServer

bool emberAfLevelControlClusterMoveToLevelWithOnOffCallback(CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
                                                            const Commands::MoveToLevelWithOnOff::DecodableType & commandData)
{
    MATTER_TRACE_SCOPE("MoveToLevelWithOnOff", "LevelControl");
    auto & level           = commandData.level;
    auto & transitionTime  = commandData.transitionTime;
    auto & optionsMask     = commandData.optionsMask;
    auto & optionsOverride = commandData.optionsOverride;

    if (transitionTime.IsNull())
    {
        ChipLogProgress(Zcl, "%s MOVE_TO_LEVEL_WITH_ON_OFF %x null %x %x", "RX level-control:", level, optionsMask.Raw(),
                        optionsOverride.Raw());
    }
    else
    {
        ChipLogProgress(Zcl, "%s MOVE_TO_LEVEL_WITH_ON_OFF %x %2x %x %x", "RX level-control:", level, transitionTime.Value(),
                        optionsMask.Raw(), optionsOverride.Raw());
    }

    Status status =
        moveToLevelHandler(commandPath.mEndpointId, Commands::MoveToLevelWithOnOff::Id, level, transitionTime,
                           Optional<BitMask<OptionsBitmap>>(optionsMask), Optional<BitMask<OptionsBitmap>>(optionsOverride),
                           INVALID_STORED_LEVEL); // Don't revert to the stored level

    commandObj->AddStatus(commandPath, status);

    return true;
}

bool emberAfLevelControlClusterMoveCallback(CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
                                            const Commands::Move::DecodableType & commandData)
{
    MATTER_TRACE_SCOPE("Move", "LevelControl");
    auto & moveMode        = commandData.moveMode;
    auto & rate            = commandData.rate;
    auto & optionsMask     = commandData.optionsMask;
    auto & optionsOverride = commandData.optionsOverride;

    if (rate.IsNull())
    {
        ChipLogProgress(Zcl, "%s MOVE %x null %x %x", "RX level-control:", to_underlying(moveMode), optionsMask.Raw(),
                        optionsOverride.Raw());
    }
    else
    {
        ChipLogProgress(Zcl, "%s MOVE %x %u %x %x", "RX level-control:", to_underlying(moveMode), rate.Value(), optionsMask.Raw(),
                        optionsOverride.Raw());
    }

    moveHandler(commandObj, commandPath, moveMode, rate, Optional<BitMask<OptionsBitmap>>(optionsMask),
                Optional<BitMask<OptionsBitmap>>(optionsOverride));
    return true;
}

bool emberAfLevelControlClusterMoveWithOnOffCallback(CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
                                                     const Commands::MoveWithOnOff::DecodableType & commandData)
{
    MATTER_TRACE_SCOPE("MoveWithOnOff", "LevelControl");
    auto & moveMode        = commandData.moveMode;
    auto & rate            = commandData.rate;
    auto & optionsMask     = commandData.optionsMask;
    auto & optionsOverride = commandData.optionsOverride;

    if (rate.IsNull())
    {
        ChipLogProgress(Zcl, "%s MOVE_WITH_ON_OFF %x null %x %x", "RX level-control:", to_underlying(moveMode), optionsMask.Raw(),
                        optionsOverride.Raw());
    }
    else
    {
        ChipLogProgress(Zcl, "%s MOVE_WITH_ON_OFF %u %2x %x %x", "RX level-control:", to_underlying(moveMode), rate.Value(),
                        optionsMask.Raw(), optionsOverride.Raw());
    }

    moveHandler(commandObj, commandPath, moveMode, rate, Optional<BitMask<OptionsBitmap>>(optionsMask),
                Optional<BitMask<OptionsBitmap>>(optionsOverride));
    return true;
}

bool emberAfLevelControlClusterStepCallback(CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
                                            const Commands::Step::DecodableType & commandData)
{
    MATTER_TRACE_SCOPE("Step", "LevelControl");
    auto & stepMode        = commandData.stepMode;
    auto & stepSize        = commandData.stepSize;
    auto & transitionTime  = commandData.transitionTime;
    auto & optionsMask     = commandData.optionsMask;
    auto & optionsOverride = commandData.optionsOverride;

    if (transitionTime.IsNull())
    {
        ChipLogProgress(Zcl, "%s STEP %x %x null %x %x", "RX level-control:", to_underlying(stepMode), stepSize, optionsMask.Raw(),
                        optionsOverride.Raw());
    }
    else
    {
        ChipLogProgress(Zcl, "%s STEP %x %x %2x %x %x", "RX level-control:", to_underlying(stepMode), stepSize,
                        transitionTime.Value(), optionsMask.Raw(), optionsOverride.Raw());
    }

    stepHandler(commandObj, commandPath, stepMode, stepSize, transitionTime, Optional<BitMask<OptionsBitmap>>(optionsMask),
                Optional<BitMask<OptionsBitmap>>(optionsOverride));
    return true;
}

bool emberAfLevelControlClusterStepWithOnOffCallback(CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
                                                     const Commands::StepWithOnOff::DecodableType & commandData)
{
    MATTER_TRACE_SCOPE("StepWithOnOff", "LevelControl");
    auto & stepMode        = commandData.stepMode;
    auto & stepSize        = commandData.stepSize;
    auto & transitionTime  = commandData.transitionTime;
    auto & optionsMask     = commandData.optionsMask;
    auto & optionsOverride = commandData.optionsOverride;

    if (transitionTime.IsNull())
    {
        ChipLogProgress(Zcl, "%s STEP_WITH_ON_OFF %x %x null %x %x", "RX level-control:", to_underlying(stepMode), stepSize,
                        optionsMask.Raw(), optionsOverride.Raw());
    }
    else
    {
        ChipLogProgress(Zcl, "%s STEP_WITH_ON_OFF %x %x %2x %x %x", "RX level-control:", to_underlying(stepMode), stepSize,
                        transitionTime.Value(), optionsMask.Raw(), optionsOverride.Raw());
    }

    stepHandler(commandObj, commandPath, stepMode, stepSize, transitionTime, Optional<BitMask<OptionsBitmap>>(optionsMask),
                Optional<BitMask<OptionsBitmap>>(optionsOverride));
    return true;
}

bool emberAfLevelControlClusterStopCallback(CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
                                            const Commands::Stop::DecodableType & commandData)
{
    MATTER_TRACE_SCOPE("Stop", "LevelControl");
    auto & optionsMask     = commandData.optionsMask;
    auto & optionsOverride = commandData.optionsOverride;

    ChipLogProgress(Zcl, "%s STOP", "RX level-control:");
    stopHandler(commandObj, commandPath, Optional<BitMask<OptionsBitmap>>(optionsMask),
                Optional<BitMask<OptionsBitmap>>(optionsOverride));
    return true;
}

bool emberAfLevelControlClusterStopWithOnOffCallback(CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
                                                     const Commands::StopWithOnOff::DecodableType & commandData)
{
    MATTER_TRACE_SCOPE("StopWithOnOff", "LevelControl");
    auto & optionsMask     = commandData.optionsMask;
    auto & optionsOverride = commandData.optionsOverride;
    ChipLogProgress(Zcl, "%s STOP_WITH_ON_OFF", "RX level-control:");
    stopHandler(commandObj, commandPath, Optional<BitMask<OptionsBitmap>>(optionsMask),
                Optional<BitMask<OptionsBitmap>>(optionsOverride));
    return true;
}

static Status moveToLevelHandler(EndpointId endpoint, CommandId commandId, uint8_t level,
                                 DataModel::Nullable<uint16_t> transitionTimeDs, chip::Optional<BitMask<OptionsBitmap>> optionsMask,
                                 chip::Optional<BitMask<OptionsBitmap>> optionsOverride, uint16_t storedLevel)
{
    EmberAfLevelControlState * state = getState(endpoint);
    DataModel::Nullable<uint8_t> currentLevel;
    uint8_t actualStepSize;

    if (state == nullptr)
    {
        return Status::Failure;
    }

    if (level > MATTER_DM_PLUGIN_LEVEL_CONTROL_MAXIMUM_LEVEL)
    {
        return Status::InvalidCommand;
    }

    if (!shouldExecuteIfOff(endpoint, commandId, optionsMask, optionsOverride))
    {
        return Status::Success;
    }

    // Cancel any currently active command before fiddling with the state.
    cancelEndpointTimerCallback(endpoint);

    Status status = Attributes::CurrentLevel::Get(endpoint, currentLevel);
    if (status != Status::Success)
    {
        ChipLogProgress(Zcl, "ERR: reading current level %x", to_underlying(status));
        return status;
    }

    if (currentLevel.IsNull())
    {
        ChipLogProgress(Zcl, "ERR: Current Level is null");
        return Status::Failure;
    }

    state->commandId = commandId;

    // Move To Level commands cause the device to move from its current level to
    // the specified level at the specified rate.
    if (state->maxLevel <= level)
    {
        state->moveToLevel = state->maxLevel;
    }
    else if (level <= state->minLevel)
    {
        state->moveToLevel = state->minLevel;
    }
    else
    {
        state->moveToLevel = level;
    }

    // If the level is decreasing, the On/Off attribute is left unchanged.  This
    // logic is to prevent a light from transitioning from off to bright to dim.
    // Instead, a light that is off will stay off until the target level is
    // reached.
    if (currentLevel.Value() <= state->moveToLevel)
    {
        if (commandId == Commands::MoveToLevelWithOnOff::Id)
        {
            setOnOffValue(endpoint, (state->moveToLevel != state->minLevel));
        }
        if (currentLevel.Value() == state->moveToLevel)
        {
            return Status::Success;
        }
        state->increasing = true;
        actualStepSize    = static_cast<uint8_t>(state->moveToLevel - currentLevel.Value());
    }
    else
    {
        state->increasing = false;
        actualStepSize    = static_cast<uint8_t>(currentLevel.Value() - state->moveToLevel);
    }

    uint32_t transitionTimeMs = 0;
    status = ComputeTransitionTimeMsForMoveToLevel(endpoint, transitionTimeDs, actualStepSize, FASTEST_TRANSITION_TIME_MS,
                                                   transitionTimeMs);
    if (status != Status::Success)
    {
        return status;
    }
    state->transitionTimeMs = transitionTimeMs;

    // The duration between events will be the transition time divided by the
    // distance we must move.
    state->eventDurationMs          = state->transitionTimeMs / std::max(static_cast<uint8_t>(1u), actualStepSize);
    state->elapsedTimeMs            = 0;
    state->storedLevel              = storedLevel;
    state->callbackSchedule.runTime = System::Clock::Milliseconds32(0);

#ifdef MATTER_DM_PLUGIN_SCENES_MANAGEMENT
    // The level has changed, the scene is no longer valid.
    if (emberAfContainsServer(endpoint, ScenesManagement::Id))
    {
        ScenesManagement::ScenesServer::Instance().MakeSceneInvalidForAllFabrics(endpoint);
    }
#endif // MATTER_DM_PLUGIN_SCENES_MANAGEMENT

    // The setup was successful, so mark the new state as active and return.
    scheduleTimerCallbackMs(endpoint, computeCallbackWaitTimeMs(state->callbackSchedule, state->eventDurationMs));

#ifdef MATTER_DM_PLUGIN_ON_OFF
    // Check that the received MoveToLevelWithOnOff produces a On action and that the onoff support the lighting featuremap
    if (commandId == Commands::MoveToLevelWithOnOff::Id && state->moveToLevel != state->minLevel &&
        OnOffServer::Instance().SupportsLightingApplications(endpoint))
    {
        OnOff::Attributes::GlobalSceneControl::Set(endpoint, true);
    }
#endif // MATTER_DM_PLUGIN_ON_OFF

    return Status::Success;
}

static void moveHandler(CommandHandler * commandObj, const ConcreteCommandPath & commandPath, MoveModeEnum moveMode,
                        DataModel::Nullable<uint8_t> rate, chip::Optional<BitMask<OptionsBitmap>> optionsMask,
                        chip::Optional<BitMask<OptionsBitmap>> optionsOverride)
{
    Status status;
    uint8_t difference;
    EmberAfLevelControlState * state;
    DataModel::Nullable<uint8_t> currentLevel;

    EndpointId endpoint = commandPath.mEndpointId;
    CommandId commandId = commandPath.mCommandId;
    // Validate the received rate and moveMode first.
    if (rate == static_cast<uint8_t>(0) || moveMode == MoveModeEnum::kUnknownEnumValue)
    {
        status = Status::InvalidCommand;
        goto send_default_response;
    }

    state = getState(endpoint);
    if (state == nullptr)
    {
        status = Status::Failure;
        goto send_default_response;
    }

    if (!shouldExecuteIfOff(endpoint, commandId, optionsMask, optionsOverride))
    {
        status = Status::Success;
        goto send_default_response;
    }

    uint8_t eventDuration; // use this local var so state->eventDurationMs is only set once the command is validated.
    status = ComputeEventDurationMsForMove(endpoint, rate, FASTEST_TRANSITION_TIME_MS, eventDuration);
    if (status != Status::Success)
    {
        goto send_default_response;
    }

    // Cancel any currently active command before fiddling with the state.
    cancelEndpointTimerCallback(endpoint);

    state->eventDurationMs = eventDuration;
    status                 = Attributes::CurrentLevel::Get(endpoint, currentLevel);
    if (status != Status::Success)
    {
        ChipLogProgress(Zcl, "ERR: reading current level %x", to_underlying(status));
        goto send_default_response;
    }

    if (currentLevel.IsNull())
    {
        ChipLogProgress(Zcl, "ERR: Current Level is null");
        status = Status::Failure;

        goto send_default_response;
    }

    state->commandId = commandId;

    // Move commands cause the device to move from its current level to either
    // the maximum or minimum level at the specified rate.
    switch (moveMode)
    {
    case MoveModeEnum::kUp:
        state->increasing  = true;
        state->moveToLevel = state->maxLevel;
        difference         = static_cast<uint8_t>(state->maxLevel - currentLevel.Value());
        break;
    case MoveModeEnum::kDown:
        state->increasing  = false;
        state->moveToLevel = state->minLevel;
        difference         = static_cast<uint8_t>(currentLevel.Value() - state->minLevel);
        break;
    default:
        status = Status::InvalidCommand;
        goto send_default_response;
    }

    // If the level is decreasing, the On/Off attribute is left unchanged.  This
    // logic is to prevent a light from transitioning from off to bright to dim.
    // Instead, a light that is off will stay off until the target level is
    // reached.
    if (currentLevel.Value() <= state->moveToLevel)
    {
        if (commandId == Commands::MoveWithOnOff::Id)
        {
            setOnOffValue(endpoint, (state->moveToLevel != state->minLevel));
        }
        if (currentLevel.Value() == state->moveToLevel)
        {
            status = Status::Success;
            goto send_default_response;
        }
    }

    state->transitionTimeMs = difference * state->eventDurationMs;
    state->elapsedTimeMs    = 0;

    // storedLevel is not used for Move commands.
    state->storedLevel              = INVALID_STORED_LEVEL;
    state->callbackSchedule.runTime = System::Clock::Milliseconds32(0);

    // The setup was successful, so mark the new state as active and return.
    scheduleTimerCallbackMs(endpoint, computeCallbackWaitTimeMs(state->callbackSchedule, state->eventDurationMs));
    status = Status::Success;

send_default_response:
    commandObj->AddStatus(commandPath, status);
}

static void stepHandler(CommandHandler * commandObj, const ConcreteCommandPath & commandPath, StepModeEnum stepMode,
                        uint8_t stepSize, DataModel::Nullable<uint16_t> transitionTimeDs,
                        chip::Optional<BitMask<OptionsBitmap>> optionsMask, chip::Optional<BitMask<OptionsBitmap>> optionsOverride)
{
    Status status;
    EmberAfLevelControlState * state;
    DataModel::Nullable<uint8_t> currentLevel;

    EndpointId endpoint    = commandPath.mEndpointId;
    CommandId commandId    = commandPath.mCommandId;
    uint8_t actualStepSize = stepSize;

    // Validate the received stepSize and stepMode first.
    if (stepSize == 0 || stepMode == StepModeEnum::kUnknownEnumValue)
    {
        status = Status::InvalidCommand;
        goto send_default_response;
    }

    state = getState(endpoint);
    if (state == nullptr)
    {
        status = Status::Failure;
        goto send_default_response;
    }

    if (!shouldExecuteIfOff(endpoint, commandId, optionsMask, optionsOverride))
    {
        status = Status::Success;
        goto send_default_response;
    }

    // Cancel any currently active command before fiddling with the state.
    cancelEndpointTimerCallback(endpoint);

    status = Attributes::CurrentLevel::Get(endpoint, currentLevel);
    if (status != Status::Success)
    {
        ChipLogProgress(Zcl, "ERR: reading current level %x", to_underlying(status));
        goto send_default_response;
    }

    if (currentLevel.IsNull())
    {
        ChipLogProgress(Zcl, "ERR: Current Level is null");
        status = Status::Failure;

        goto send_default_response;
    }

    state->commandId = commandId;

    // Step commands cause the device to move from its current level to a new
    // level over the specified transition time.
    switch (stepMode)
    {
    case StepModeEnum::kUp:
        state->increasing = true;
        if (state->maxLevel - currentLevel.Value() < stepSize)
        {
            state->moveToLevel = state->maxLevel;
            actualStepSize     = static_cast<uint8_t>(state->maxLevel - currentLevel.Value());
        }
        else
        {
            state->moveToLevel = static_cast<uint8_t>(currentLevel.Value() + stepSize);
        }
        break;
    case StepModeEnum::kDown:
        state->increasing = false;
        if (currentLevel.Value() - state->minLevel < stepSize)
        {
            state->moveToLevel = state->minLevel;
            actualStepSize     = static_cast<uint8_t>(currentLevel.Value() - state->minLevel);
        }
        else
        {
            state->moveToLevel = static_cast<uint8_t>(currentLevel.Value() - stepSize);
        }
        break;
    default:
        // Should never happen as it is verified at function entry.
        status = Status::InvalidCommand;
        goto send_default_response;
    }

    // If the level is decreasing, the On/Off attribute is left unchanged.  This
    // logic is to prevent a light from transitioning from off to bright to dim.
    // Instead, a light that is off will stay off until the target level is
    // reached.
    if (currentLevel.Value() <= state->moveToLevel)
    {
        if (commandId == Commands::StepWithOnOff::Id)
        {
            setOnOffValue(endpoint, (state->moveToLevel != state->minLevel));
        }
        if (currentLevel.Value() == state->moveToLevel)
        {
            status = Status::Success;
            goto send_default_response;
        }
    }

    state->transitionTimeMs =
        ComputeTransitionTimeMsForStep(endpoint, transitionTimeDs, stepSize, actualStepSize, FASTEST_TRANSITION_TIME_MS);

    // The duration between events will be the transition time divided by the
    // distance we must move.
    state->eventDurationMs = state->transitionTimeMs / std::max(static_cast<uint8_t>(1u), actualStepSize);
    state->elapsedTimeMs   = 0;

    // storedLevel is not used for Step commands
    state->storedLevel              = INVALID_STORED_LEVEL;
    state->callbackSchedule.runTime = System::Clock::Milliseconds32(0);

    // The setup was successful, so mark the new state as active and return.
    scheduleTimerCallbackMs(endpoint, computeCallbackWaitTimeMs(state->callbackSchedule, state->eventDurationMs));
    status = Status::Success;

send_default_response:
    commandObj->AddStatus(commandPath, status);
}

static void stopHandler(CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
                        chip::Optional<BitMask<OptionsBitmap>> optionsMask, chip::Optional<BitMask<OptionsBitmap>> optionsOverride)
{
    EndpointId endpoint              = commandPath.mEndpointId;
    CommandId commandId              = commandPath.mCommandId;
    EmberAfLevelControlState * state = getState(endpoint);
    Status status                    = Status::Success;

    if (state == nullptr)
    {
        status = Status::Failure;
        goto send_default_response;
    }

    if (!shouldExecuteIfOff(endpoint, commandId, optionsMask, optionsOverride))
    {
        goto send_default_response;
    }

    // Cancel any currently active command.
    cancelEndpointTimerCallback(endpoint);
    SetCurrentLevelQuietReport(endpoint, state, state->quietCurrentLevel.value(), true /*isEndOfTransition*/);
    writeRemainingTime(endpoint, 0);

send_default_response:
    commandObj->AddStatus(commandPath, status);
}

// Follows 07-5123-04 (ZigBee Cluster Library doc), section 3.10.2.1.1.
// Quotes are from table 3.46.
void emberAfOnOffClusterLevelControlEffectCallback(EndpointId endpoint, bool newValue)
{
    DataModel::Nullable<uint8_t> resolvedLevel;
    DataModel::Nullable<uint8_t> temporaryCurrentLevelCache;
    DataModel::Nullable<uint16_t> transitionTime;

    uint16_t currentOnOffTransitionTime;
    Status status;
    bool useOnLevel = false;

    EmberAfLevelControlState * state = getState(endpoint);
    if (state == nullptr)
    {
        ChipLogProgress(Zcl, "ERR: Level control cluster not available on ep%d", endpoint);
        return;
    }

    uint8_t minimumLevelAllowedForTheDevice = state->minLevel;

    // "Temporarily store CurrentLevel."
    status = Attributes::CurrentLevel::Get(endpoint, temporaryCurrentLevelCache);
    if (status != Status::Success)
    {
        ChipLogProgress(Zcl, "ERR: reading current level %x", to_underlying(status));
        return;
    }

    if (temporaryCurrentLevelCache.IsNull())
    {
        ChipLogProgress(Zcl, "ERR: Current Level is null");
        return;
    }

    ResolveOnLevel(endpoint, temporaryCurrentLevelCache, resolvedLevel, useOnLevel);

    // Read the OnOffTransitionTime attribute.
    if (TryGetOnOffTransitionTimeDs(endpoint, currentOnOffTransitionTime))
    {
        transitionTime.SetNonNull(currentOnOffTransitionTime);
    }
    else
    {
        transitionTime.SetNull();
    }

    if (newValue)
    {
        // If newValue is OnOff::Commands::On::Id...
        // "Set CurrentLevel to minimum level allowed for the device."
        status = SetCurrentLevelQuietReport(endpoint, state, minimumLevelAllowedForTheDevice, false /*isEndOfTransition*/);
        if (status != Status::Success)
        {
            ChipLogProgress(Zcl, "ERR: reading current level %x", to_underlying(status));
            return;
        }

        // "Move CurrentLevel to OnLevel, or to the stored level if OnLevel is not
        // defined, over the time period OnOffTransitionTime."
        moveToLevelHandler(endpoint, Commands::MoveToLevel::Id, resolvedLevel.Value(), transitionTime, chip::NullOptional,
                           chip::NullOptional,
                           INVALID_STORED_LEVEL); // Don't revert to stored level
    }
    else
    {
        // ...else if newValue is OnOff::Commands::Off::Id...
        // "Move CurrentLevel to the minimum level allowed for the device over the
        // time period OnOffTransitionTime."
        if (useOnLevel)
        {
            // If OnLevel is defined, don't revert to stored level.
            moveToLevelHandler(endpoint, Commands::MoveToLevelWithOnOff::Id, minimumLevelAllowedForTheDevice, transitionTime,
                               chip::NullOptional, chip::NullOptional, INVALID_STORED_LEVEL);
        }
        else
        {
            // If OnLevel is not defined, set the CurrentLevel to the stored level.
            moveToLevelHandler(endpoint, Commands::MoveToLevelWithOnOff::Id, minimumLevelAllowedForTheDevice, transitionTime,
                               chip::NullOptional, chip::NullOptional, temporaryCurrentLevelCache.Value());
        }
    }
}

void emberAfLevelControlClusterServerInitCallback(EndpointId endpoint)
{
    EmberAfLevelControlState * state = getState(endpoint);

    if (state == nullptr)
    {
        ChipLogProgress(Zcl, "ERR: Level control cluster not available on ep%d", endpoint);
        return;
    }

    state->minLevel = MATTER_DM_PLUGIN_LEVEL_CONTROL_MINIMUM_LEVEL;
    state->maxLevel = MATTER_DM_PLUGIN_LEVEL_CONTROL_MAXIMUM_LEVEL;

    // If these read only attributes are enabled we use those values as our set minLevel and maxLevel
    // if get isn't possible, value stays at default
    Attributes::MinLevel::Get(endpoint, &state->minLevel);
    Attributes::MaxLevel::Get(endpoint, &state->maxLevel);

    if (LevelControlHasFeature(endpoint, Feature::kLighting))
    {
        if (state->minLevel < LEVEL_CONTROL_LIGHTING_MIN_LEVEL)
        {
            state->minLevel = LEVEL_CONTROL_LIGHTING_MIN_LEVEL;
        }

        if (state->maxLevel > LEVEL_CONTROL_LIGHTING_MAX_LEVEL)
        {
            state->maxLevel = LEVEL_CONTROL_LIGHTING_MAX_LEVEL;
        }
    }

    DataModel::Nullable<uint8_t> currentLevel;
    Status status = Attributes::CurrentLevel::Get(endpoint, currentLevel);
    if (status == Status::Success)
    {
        HandleStartUpCurrentLevel(endpoint, state, currentLevel);
        // In any case, we make sure that the respects min/max
        if (currentLevel.IsNull() || currentLevel.Value() < state->minLevel)
        {
            SetCurrentLevelQuietReport(endpoint, state, state->minLevel, false /*isEndOfTransition*/);
        }
        else if (currentLevel.Value() > state->maxLevel)
        {
            SetCurrentLevelQuietReport(endpoint, state, state->maxLevel, false /*isEndOfTransition*/);
        }
    }

#if defined(MATTER_DM_PLUGIN_SCENES_MANAGEMENT) && CHIP_CONFIG_SCENES_USE_DEFAULT_HANDLERS
    // Registers Scene handlers for the level control cluster on the server
    Clusters::ScenesManagement::ScenesServer::Instance().RegisterSceneHandler(endpoint, LevelControlServer::GetSceneHandler());
#endif // defined(MATTER_DM_PLUGIN_SCENES_MANAGEMENT) && CHIP_CONFIG_SCENES_USE_DEFAULT_HANDLERS

    emberAfPluginLevelControlClusterServerPostInitCallback(endpoint);
}

void MatterLevelControlClusterServerShutdownCallback(EndpointId endpoint)
{
    ChipLogProgress(Zcl, "Shuting down level control server cluster on endpoint %d", endpoint);
    cancelEndpointTimerCallback(endpoint);
}

void emberAfPluginLevelControlClusterServerPostInitCallback(EndpointId endpoint) {}

bool LevelControlHasFeature(EndpointId endpoint, Feature feature)
{
    bool success;
    uint32_t featureMap;
    success = (Attributes::FeatureMap::Get(endpoint, &featureMap) == Status::Success);

    return success ? ((featureMap & to_underlying(feature)) != 0) : false;
}

void MatterLevelControlPluginServerInitCallback() {}
void MatterLevelControlPluginServerShutdownCallback() {}
