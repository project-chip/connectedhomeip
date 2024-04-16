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

#include <algorithm>

// this file contains all the common includes for clusters in the util
#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <app/CommandHandler.h>
#include <app/ConcreteCommandPath.h>
#include <app/util/af.h>
#include <app/util/config.h>
#include <app/util/util.h>

#include <app/reporting/reporting.h>
#include <lib/core/Optional.h>
#include <platform/CHIPDeviceConfig.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/PlatformManager.h>
#include <tracing/macros.h>

#ifdef MATTER_DM_PLUGIN_SCENES_MANAGEMENT
#include <app/clusters/scenes-server/scenes-server.h>
#endif // MATTER_DM_PLUGIN_SCENES_MANAGEMENT

#ifdef MATTER_DM_PLUGIN_ON_OFF
#include <app/clusters/on-off-server/on-off-server.h>
#endif // MATTER_DM_PLUGIN_ON_OFF

#ifdef MATTER_DM_PLUGIN_COLOR_CONTROL_SERVER_TEMP
#include <app/clusters/color-control-server/color-control-server.h>
#endif // MATTER_DM_PLUGIN_COLOR_CONTROL_SERVER_TEMP

#include <assert.h>

using namespace chip;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::LevelControl;
using chip::Protocols::InteractionModel::Status;

#ifndef IGNORE_LEVEL_CONTROL_CLUSTER_START_UP_CURRENT_LEVEL
static bool areStartUpLevelControlServerAttributesNonVolatile(EndpointId endpoint);
#endif // IGNORE_LEVEL_CONTROL_CLUSTER_START_UP_CURRENT_LEVEL

#if (MATTER_DM_PLUGIN_LEVEL_CONTROL_RATE == 0)
#define FASTEST_TRANSITION_TIME_MS 0
#else
#define FASTEST_TRANSITION_TIME_MS (MILLISECOND_TICKS_PER_SECOND / MATTER_DM_PLUGIN_LEVEL_CONTROL_RATE)
#endif // MATTER_DM_PLUGIN_LEVEL_CONTROL_RATE

#define LEVEL_CONTROL_LIGHTING_MIN_LEVEL 0x01
#define LEVEL_CONTROL_LIGHTING_MAX_LEVEL 0xFE

#define INVALID_STORED_LEVEL 0xFFFF

#define STARTUP_CURRENT_LEVEL_USE_DEVICE_MINIMUM 0x00
#define STARTUP_CURRENT_LEVEL_USE_PREVIOUS_LEVEL 0xFF

static constexpr size_t kLevelControlStateTableSize =
    MATTER_DM_LEVEL_CONTROL_CLUSTER_SERVER_ENDPOINT_COUNT + CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT;
static_assert(kLevelControlStateTableSize <= kEmberInvalidEndpointIndex, "LevelControl state table size error");

struct CallbackScheduleState
{
    System::Clock::Timestamp idealTimestamp; // The ideal time-stamp for the next callback to be scheduled.
    System::Clock::Milliseconds32 runTime;   // The duration of the previous scheduled callback function.
                                             // e.g. running time of emberAfLevelControlClusterServerTickCallback
                                             // when called consecutively
};

typedef struct
{
    CommandId commandId;
    uint8_t moveToLevel;
    bool increasing;
    uint8_t onLevel;
    uint8_t minLevel;
    uint8_t maxLevel;
    uint16_t storedLevel;
    uint32_t eventDurationMs;
    uint32_t transitionTimeMs;
    uint32_t elapsedTimeMs;
    CallbackScheduleState callbackSchedule;
} EmberAfLevelControlState;

static EmberAfLevelControlState stateTable[kLevelControlStateTableSize];

static EmberAfLevelControlState * getState(EndpointId endpoint);

static Status moveToLevelHandler(EndpointId endpoint, CommandId commandId, uint8_t level,
                                 app::DataModel::Nullable<uint16_t> transitionTimeDs,
                                 chip::Optional<BitMask<OptionsBitmap>> optionsMask,
                                 chip::Optional<BitMask<OptionsBitmap>> optionsOverride, uint16_t storedLevel);
static void moveHandler(app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath, MoveModeEnum moveMode,
                        app::DataModel::Nullable<uint8_t> rate, chip::Optional<BitMask<OptionsBitmap>> optionsMask,
                        chip::Optional<BitMask<OptionsBitmap>> optionsOverride);
static void stepHandler(app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath, StepModeEnum stepMode,
                        uint8_t stepSize, app::DataModel::Nullable<uint16_t> transitionTimeDs,
                        chip::Optional<BitMask<OptionsBitmap>> optionsMask, chip::Optional<BitMask<OptionsBitmap>> optionsOverride);
static void stopHandler(app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath,
                        chip::Optional<BitMask<OptionsBitmap>> optionsMask, chip::Optional<BitMask<OptionsBitmap>> optionsOverride);

static void setOnOffValue(EndpointId endpoint, bool onOff);
static void writeRemainingTime(EndpointId endpoint, uint16_t remainingTimeMs);
static bool shouldExecuteIfOff(EndpointId endpoint, CommandId commandId, chip::Optional<chip::BitMask<OptionsBitmap>> optionsMask,
                               chip::Optional<chip::BitMask<OptionsBitmap>> optionsOverride);

#if defined(MATTER_DM_PLUGIN_SCENES_MANAGEMENT) && CHIP_CONFIG_SCENES_USE_DEFAULT_HANDLERS
class DefaultLevelControlSceneHandler : public scenes::DefaultSceneHandlerImpl
{
public:
    // As per spec, 2 attributes are scenable in the level control cluster
    static constexpr uint8_t kLevelMaxScenableAttributes = 2;

    DefaultLevelControlSceneHandler() = default;
    ~DefaultLevelControlSceneHandler() override {}

    // Default function for LevelControl cluster, only puts the LevelControl cluster ID in the span if supported on the caller
    // endpoint
    virtual void GetSupportedClusters(EndpointId endpoint, Span<ClusterId> & clusterBuffer) override
    {
        if (emberAfContainsServer(endpoint, LevelControl::Id) && clusterBuffer.size() >= 1)
        {
            clusterBuffer[0] = LevelControl::Id;
            clusterBuffer.reduce_size(1);
        }
        else
        {
            clusterBuffer.reduce_size(0);
        }
    }

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
        using AttributeValuePair = ScenesManagement::Structs::AttributeValuePair::Type;

        app::DataModel::Nullable<uint8_t> level;
        VerifyOrReturnError(Status::Success == Attributes::CurrentLevel::Get(endpoint, level), CHIP_ERROR_READ_FAILED);

        AttributeValuePair pairs[kLevelMaxScenableAttributes];

        uint8_t maxLevel;
        VerifyOrReturnError(Status::Success == Attributes::MaxLevel::Get(endpoint, &maxLevel), CHIP_ERROR_READ_FAILED);

        pairs[0].attributeID = Attributes::CurrentLevel::Id;
        if (!level.IsNull())
        {
            pairs[0].attributeValue = level.Value();
        }
        else
        {
            chip::app::NumericAttributeTraits<uint32_t>::SetNull(pairs[0].attributeValue);
        }
        size_t attributeCount = 1;
        if (LevelControlHasFeature(endpoint, LevelControl::Feature::kFrequency))
        {
            uint16_t frequency;
            VerifyOrReturnError(Status::Success == Attributes::CurrentFrequency::Get(endpoint, &frequency), CHIP_ERROR_READ_FAILED);
            pairs[attributeCount].attributeID    = Attributes::CurrentFrequency::Id;
            pairs[attributeCount].attributeValue = frequency;
            attributeCount++;
        }

        app::DataModel::List<AttributeValuePair> attributeValueList(pairs, attributeCount);

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
        app::DataModel::DecodableList<ScenesManagement::Structs::AttributeValuePair::DecodableType> attributeValueList;

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
                level = static_cast<uint8_t>(decodePair.attributeValue);
                break;
            case Attributes::CurrentFrequency::Id:
                // TODO : Uncomment when frequency is supported by the level control cluster
                // frequency = static_cast<uint16_t>(decodePair.attributeValue);
                break;
            default:
                return CHIP_ERROR_INVALID_ARGUMENT;
            }
        }
        ReturnErrorOnFailure(pair_iterator.GetStatus());

        // TODO : Implement action on frequency when frequency not provisional anymore
        // if(LevelControlHasFeature(endpoint, LevelControl::Feature::kFrequency)){}

        if (!chip::app::NumericAttributeTraits<uint8_t>::IsNullValue(level))
        {
            CommandId command = LevelControlHasFeature(endpoint, LevelControl::Feature::kOnOff) ? Commands::MoveToLevelWithOnOff::Id
                                                                                                : Commands::MoveToLevel::Id;

            moveToLevelHandler(endpoint, command, level, app::DataModel::MakeNullable(static_cast<uint16_t>(timeMs / 100)),
                               chip::Optional<BitMask<OptionsBitmap>>(), chip::Optional<BitMask<OptionsBitmap>>(),
                               INVALID_STORED_LEVEL);
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

static EmberAfLevelControlState * getState(EndpointId endpoint)
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

void emberAfLevelControlClusterServerTickCallback(EndpointId endpoint)
{
    EmberAfLevelControlState * state = getState(endpoint);
    Status status;
    app::DataModel::Nullable<uint8_t> currentLevel;
    const auto callbackStartTimestamp = System::SystemClock().GetMonotonicTimestamp();

    if (state == nullptr)
    {
        return;
    }

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

    status = Attributes::CurrentLevel::Set(endpoint, currentLevel);
    if (status != Status::Success)
    {
        ChipLogProgress(Zcl, "ERR: writing current level %x", to_underlying(status));
        state->callbackSchedule.runTime = System::Clock::Milliseconds32(0);
        writeRemainingTime(endpoint, 0);
        return;
    }

    updateCoupledColorTemp(endpoint);

    // Are we at the requested level?
    if (currentLevel.Value() == state->moveToLevel)
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
        writeRemainingTime(endpoint, static_cast<uint16_t>(state->transitionTimeMs - state->elapsedTimeMs));
        scheduleTimerCallbackMs(endpoint, computeCallbackWaitTimeMs(state->callbackSchedule, state->eventDurationMs));
    }
}

static void writeRemainingTime(EndpointId endpoint, uint16_t remainingTimeMs)
{
#ifndef IGNORE_LEVEL_CONTROL_CLUSTER_LEVEL_CONTROL_REMAINING_TIME
    if (emberAfContainsAttribute(endpoint, LevelControl::Id, LevelControl::Attributes::RemainingTime::Id))
    {
        // Convert milliseconds to tenths of a second, rounding any fractional value
        // up to the nearest whole value.  This means:
        //
        //   0 ms = 0.00 ds = 0 ds
        //   1 ms = 0.01 ds = 1 ds
        //   ...
        //   100 ms = 1.00 ds = 1 ds
        //   101 ms = 1.01 ds = 2 ds
        //   ...
        //   200 ms = 2.00 ds = 2 ds
        //   201 ms = 2.01 ds = 3 ds
        //   ...
        //
        // This is done to ensure that the attribute, in tenths of a second, only
        // goes to zero when the remaining time in milliseconds is actually zero.
        uint16_t remainingTimeDs = static_cast<uint16_t>((remainingTimeMs + 99) / 100);
        Status status            = LevelControl::Attributes::RemainingTime::Set(endpoint, remainingTimeDs);
        if (status != Status::Success)
        {
            ChipLogProgress(Zcl, "ERR: writing remaining time %x", to_underlying(status));
        }
    }
#endif // IGNORE_LEVEL_CONTROL_CLUSTER_LEVEL_CONTROL_REMAINING_TIME
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

bool emberAfLevelControlClusterMoveToLevelCallback(app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath,
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

chip::scenes::SceneHandler * GetSceneHandler()
{
#if defined(MATTER_DM_PLUGIN_SCENES_MANAGEMENT) && CHIP_CONFIG_SCENES_USE_DEFAULT_HANDLERS
    return &sLevelControlSceneHandler;
#else
    return nullptr;
#endif // defined(MATTER_DM_PLUGIN_SCENES_MANAGEMENT) && CHIP_CONFIG_SCENES_USE_DEFAULT_HANDLERS
}

} // namespace LevelControlServer

bool emberAfLevelControlClusterMoveToLevelWithOnOffCallback(app::CommandHandler * commandObj,
                                                            const app::ConcreteCommandPath & commandPath,
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

bool emberAfLevelControlClusterMoveCallback(app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath,
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

bool emberAfLevelControlClusterMoveWithOnOffCallback(app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath,
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

bool emberAfLevelControlClusterStepCallback(app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath,
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

bool emberAfLevelControlClusterStepWithOnOffCallback(app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath,
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

bool emberAfLevelControlClusterStopCallback(app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath,
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

bool emberAfLevelControlClusterStopWithOnOffCallback(app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath,
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
                                 app::DataModel::Nullable<uint16_t> transitionTimeDs,
                                 chip::Optional<BitMask<OptionsBitmap>> optionsMask,
                                 chip::Optional<BitMask<OptionsBitmap>> optionsOverride, uint16_t storedLevel)
{
    EmberAfLevelControlState * state = getState(endpoint);
    app::DataModel::Nullable<uint8_t> currentLevel;
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

#ifndef IGNORE_LEVEL_CONTROL_CLUSTER_TRANSITION
    // If the Transition time field takes the value null, then the time taken
    // to move to the new level shall instead be determined by the On/Off
    // Transition Time attribute.  If On/Off Transition Time, which is an
    // optional attribute, is not present, the device shall move to its new level
    // as fast as it is able.
    if (transitionTimeDs.IsNull())
    {
#ifndef IGNORE_LEVEL_CONTROL_CLUSTER_ON_OFF_TRANSITION_TIME
        if (emberAfContainsAttribute(endpoint, LevelControl::Id, Attributes::OnOffTransitionTime::Id))
        {
            uint16_t onOffTransitionTime = 0;
            status                       = Attributes::OnOffTransitionTime::Get(endpoint, &onOffTransitionTime);
            if (status != Status::Success)
            {
                ChipLogProgress(Zcl, "ERR: reading on/off transition time %x", to_underlying(status));
                return status;
            }

            // Transition time comes in (or is stored, in the case of On/Off Transition
            // Time) as tenths of a second, but we work in milliseconds.
            state->transitionTimeMs = (onOffTransitionTime * MILLISECOND_TICKS_PER_SECOND / 10);
        }
        else
        {
            state->transitionTimeMs = FASTEST_TRANSITION_TIME_MS;
        }
#else
        // If the Transition Time field is 0xFFFF and On/Off Transition Time,
        // which is an optional attribute, is not present, the device shall move to
        // its new level as fast as it is able.
        state->transitionTimeMs = FASTEST_TRANSITION_TIME_MS;
#endif // IGNORE_LEVEL_CONTROL_CLUSTER_ON_OFF_TRANSITION_TIME
    }
    else
    {
        // Transition time comes in (or is stored, in the case of On/Off Transition
        // Time) as tenths of a second, but we work in milliseconds.
        state->transitionTimeMs = (transitionTimeDs.Value() * MILLISECOND_TICKS_PER_SECOND / 10);
    }
#else
    // Transition is not supported so always use fastest transition time and ignore
    // both the provided transition time as well as OnOffTransitionTime.
    ChipLogProgress(Zcl, "Device does not support transition, ignoring transition time");
    state->transitionTimeMs = FASTEST_TRANSITION_TIME_MS;
#endif // IGNORE_LEVEL_CONTROL_CLUSTER_TRANSITION

    // The duration between events will be the transition time divided by the
    // distance we must move.
    state->eventDurationMs = state->transitionTimeMs / std::max(static_cast<uint8_t>(1u), actualStepSize);
    state->elapsedTimeMs   = 0;

    state->storedLevel = storedLevel;

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

static void moveHandler(app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath, MoveModeEnum moveMode,
                        app::DataModel::Nullable<uint8_t> rate, chip::Optional<BitMask<OptionsBitmap>> optionsMask,
                        chip::Optional<BitMask<OptionsBitmap>> optionsOverride)
{
    EndpointId endpoint = commandPath.mEndpointId;
    CommandId commandId = commandPath.mCommandId;

    EmberAfLevelControlState * state = getState(endpoint);
    Status status;
    app::DataModel::Nullable<uint8_t> currentLevel;
    uint8_t difference;

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

    if (!rate.IsNull() && (rate.Value() == 0))
    {
        // Move at a rate of zero is no move at all. Immediately succeed without touching anything.
        ChipLogProgress(Zcl, "Immediate success due to move rate of 0 (would move at no rate).");
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

#ifndef IGNORE_LEVEL_CONTROL_CLUSTER_TRANSITION
    // If the Rate field is null, the device should move at the default move rate, if available,
    // Otherwise, move as fast as possible
    if (rate.IsNull())
    {
        app::DataModel::Nullable<uint8_t> defaultMoveRate;
        status = Attributes::DefaultMoveRate::Get(endpoint, defaultMoveRate);
        if (status != Status::Success || defaultMoveRate.IsNull())
        {
            ChipLogProgress(Zcl, "ERR: reading default move rate %x", to_underlying(status));
            state->eventDurationMs = FASTEST_TRANSITION_TIME_MS;
        }
        else
        {
            // nonsensical case, means "don't move", so we're done
            if (defaultMoveRate.Value() == 0)
            {
                status = Status::Success;
                goto send_default_response;
            }
            // Already checked that defaultMoveRate.Value() != 0.
            state->eventDurationMs = MILLISECOND_TICKS_PER_SECOND / defaultMoveRate.Value();
        }
    }
    else
    {
        state->eventDurationMs = MILLISECOND_TICKS_PER_SECOND / std::max(static_cast<uint8_t>(1u), rate.Value());
    }
#else
    // Transition/rate is not supported so always use fastest transition time and ignore
    // both the provided transition time as well as OnOffTransitionTime.
    ChipLogProgress(Zcl, "Device does not support transition, ignoring rate");
    state->eventDurationMs = FASTEST_TRANSITION_TIME_MS;
#endif // IGNORE_LEVEL_CONTROL_CLUSTER_TRANSITION

    state->transitionTimeMs = difference * state->eventDurationMs;
    state->elapsedTimeMs    = 0;

    // storedLevel is not used for Move commands.
    state->storedLevel = INVALID_STORED_LEVEL;

    state->callbackSchedule.runTime = System::Clock::Milliseconds32(0);

    // The setup was successful, so mark the new state as active and return.
    scheduleTimerCallbackMs(endpoint, computeCallbackWaitTimeMs(state->callbackSchedule, state->eventDurationMs));
    status = Status::Success;

send_default_response:
    commandObj->AddStatus(commandPath, status);
}

static void stepHandler(app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath, StepModeEnum stepMode,
                        uint8_t stepSize, app::DataModel::Nullable<uint16_t> transitionTimeDs,
                        chip::Optional<BitMask<OptionsBitmap>> optionsMask, chip::Optional<BitMask<OptionsBitmap>> optionsOverride)
{
    EndpointId endpoint = commandPath.mEndpointId;
    CommandId commandId = commandPath.mCommandId;

    EmberAfLevelControlState * state = getState(endpoint);
    Status status;
    app::DataModel::Nullable<uint8_t> currentLevel;
    uint8_t actualStepSize = stepSize;

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

#ifndef IGNORE_LEVEL_CONTROL_CLUSTER_TRANSITION
    // If the Transition Time field is null, the device should move as fast as
    // it is able.
    if (transitionTimeDs.IsNull())
    {
        state->transitionTimeMs = FASTEST_TRANSITION_TIME_MS;
    }
    else
    {
        // Transition time comes in as tenths of a second, but we work in
        // milliseconds.
        state->transitionTimeMs = (transitionTimeDs.Value() * MILLISECOND_TICKS_PER_SECOND / 10);
        // If the new level was pegged at the minimum level, the transition time
        // shall be proportionally reduced.  This is done after the conversion to
        // milliseconds to reduce rounding errors in integer division.
        if (stepSize != actualStepSize)
        {
            state->transitionTimeMs = (state->transitionTimeMs * actualStepSize / std::max(static_cast<uint8_t>(1u), stepSize));
        }
    }
#else
    // Transition is not supported so always use fastest transition time and ignore
    // both the provided transition time as well as OnOffTransitionTime.
    ChipLogProgress(Zcl, "Device does not support transition, ignoring transition time");
    state->transitionTimeMs = FASTEST_TRANSITION_TIME_MS;
#endif // IGNORE_LEVEL_CONTROL_CLUSTER_TRANSITION

    // The duration between events will be the transition time divided by the
    // distance we must move.
    state->eventDurationMs = state->transitionTimeMs / std::max(static_cast<uint8_t>(1u), actualStepSize);
    state->elapsedTimeMs   = 0;

    // storedLevel is not used for Step commands
    state->storedLevel = INVALID_STORED_LEVEL;

    state->callbackSchedule.runTime = System::Clock::Milliseconds32(0);

    // The setup was successful, so mark the new state as active and return.
    scheduleTimerCallbackMs(endpoint, computeCallbackWaitTimeMs(state->callbackSchedule, state->eventDurationMs));
    status = Status::Success;

send_default_response:
    commandObj->AddStatus(commandPath, status);
}

static void stopHandler(app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath,
                        chip::Optional<BitMask<OptionsBitmap>> optionsMask, chip::Optional<BitMask<OptionsBitmap>> optionsOverride)
{
    EndpointId endpoint = commandPath.mEndpointId;
    CommandId commandId = commandPath.mCommandId;

    EmberAfLevelControlState * state = getState(endpoint);
    Status status;

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

    // Cancel any currently active command.
    cancelEndpointTimerCallback(endpoint);
    writeRemainingTime(endpoint, 0);
    status = Status::Success;

send_default_response:
    commandObj->AddStatus(commandPath, status);
}

// Follows 07-5123-04 (ZigBee Cluster Library doc), section 3.10.2.1.1.
// Quotes are from table 3.46.
void emberAfOnOffClusterLevelControlEffectCallback(EndpointId endpoint, bool newValue)
{
    app::DataModel::Nullable<uint8_t> resolvedLevel;
    app::DataModel::Nullable<uint8_t> temporaryCurrentLevelCache;
    app::DataModel::Nullable<uint16_t> transitionTime;

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

#ifndef IGNORE_LEVEL_CONTROL_CLUSTER_ON_LEVEL_ATTRIBUTE
    if (emberAfContainsAttribute(endpoint, LevelControl::Id, Attributes::OnLevel::Id))
    {
        status = Attributes::OnLevel::Get(endpoint, resolvedLevel);
        if (status != Status::Success)
        {
            ChipLogProgress(Zcl, "ERR: reading on level %x", to_underlying(status));
            return;
        }

        if (resolvedLevel.IsNull())
        {
            // OnLevel has undefined value; fall back to CurrentLevel.
            resolvedLevel.SetNonNull(temporaryCurrentLevelCache.Value());
        }
        else
        {
            useOnLevel = true;
        }
    }
    else
    {
        resolvedLevel.SetNonNull(temporaryCurrentLevelCache.Value());
    }
#else
    resolvedLevel.SetNonNull(temporaryCurrentLevelCache.Value());
#endif // IGNORE_LEVEL_CONTROL_CLUSTER_ON_LEVEL_ATTRIBUTE

    // Read the OnOffTransitionTime attribute.
#ifndef IGNORE_LEVEL_CONTROL_CLUSTER_ON_OFF_TRANSITION_TIME
    if (emberAfContainsAttribute(endpoint, LevelControl::Id, Attributes::OnOffTransitionTime::Id))
    {
        status = Attributes::OnOffTransitionTime::Get(endpoint, &currentOnOffTransitionTime);
        if (status != Status::Success)
        {
            ChipLogProgress(Zcl, "ERR: reading current level %x", to_underlying(status));
            return;
        }
        transitionTime.SetNonNull(currentOnOffTransitionTime);
    }
    else
    {
        transitionTime.SetNull();
    }
#else
    transitionTime.SetNull();
#endif // IGNORE_LEVEL_CONTROL_CLUSTER_ON_OFF_TRANSITION_TIME

    if (newValue)
    {
        // If newValue is OnOff::Commands::On::Id...
        // "Set CurrentLevel to minimum level allowed for the device."
        status = Attributes::CurrentLevel::Set(endpoint, minimumLevelAllowedForTheDevice);
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

    app::DataModel::Nullable<uint8_t> currentLevel;
    Status status = Attributes::CurrentLevel::Get(endpoint, currentLevel);
    if (status == Status::Success)
    {
#ifndef IGNORE_LEVEL_CONTROL_CLUSTER_START_UP_CURRENT_LEVEL
        // StartUp behavior relies StartUpCurrentLevel attributes being Non Volatile.
        if (areStartUpLevelControlServerAttributesNonVolatile(endpoint))
        {
            // 1.5.14. StartUpCurrentLevel Attribute
            // The StartUpCurrentLevel attribute SHALL define the desired startup level
            // for a device when it is supplied with power and this level SHALL be
            // reflected in the CurrentLevel attribute. The values of the StartUpCurrentLevel
            // attribute are listed below:
            // Table 4. Values of the StartUpCurrentLevel attribute
            // Value      Action on power up
            // 0x00       Set the CurrentLevel attribute to the minimum value permitted on the device.
            // 0x01-0xfe  Set the CurrentLevel attribute to this value.
            // NULL       Set the CurrentLevel attribute to its previous value.
            // 0xFF       Work Around ZAP Can't set default value to NULL
            // https://github.com/project-chip/zap/issues/354

            app::DataModel::Nullable<uint8_t> startUpCurrentLevel;
            status = Attributes::StartUpCurrentLevel::Get(endpoint, startUpCurrentLevel);
            if (status == Status::Success)
            {
                if (!startUpCurrentLevel.IsNull())
                {
                    if (startUpCurrentLevel.Value() == STARTUP_CURRENT_LEVEL_USE_DEVICE_MINIMUM)
                    {
                        currentLevel.SetNonNull(state->minLevel);
                    }
                    else
                    {
                        // Otherwise set to specified value 0x01-0xFE.
                        // But, need to enforce currentLevel's min/max, right?
                        // Spec doesn't mention this.
                        if (startUpCurrentLevel.Value() < state->minLevel)
                        {
                            currentLevel.SetNonNull(state->minLevel);
                        }
                        else if (startUpCurrentLevel.Value() > state->maxLevel)
                        {
                            currentLevel.SetNonNull(state->maxLevel);
                        }
                        else
                        {
                            currentLevel.SetNonNull(startUpCurrentLevel.Value());
                        }
                    }
                }
                // Otherwise Set the CurrentLevel attribute to its previous value which was already fetch above

                Attributes::CurrentLevel::Set(endpoint, currentLevel);
            }
        }
#endif // IGNORE_LEVEL_CONTROL_CLUSTER_START_UP_CURRENT_LEVEL
       // In any case, we make sure that the respects min/max
        if (currentLevel.IsNull() || currentLevel.Value() < state->minLevel)
        {
            Attributes::CurrentLevel::Set(endpoint, state->minLevel);
        }
        else if (currentLevel.Value() > state->maxLevel)
        {
            Attributes::CurrentLevel::Set(endpoint, state->maxLevel);
        }
    }

#if defined(MATTER_DM_PLUGIN_SCENES_MANAGEMENT) && CHIP_CONFIG_SCENES_USE_DEFAULT_HANDLERS
    // Registers Scene handlers for the level control cluster on the server
    app::Clusters::ScenesManagement::ScenesServer::Instance().RegisterSceneHandler(endpoint, LevelControlServer::GetSceneHandler());
#endif // defined(MATTER_DM_PLUGIN_SCENES_MANAGEMENT) && CHIP_CONFIG_SCENES_USE_DEFAULT_HANDLERS

    emberAfPluginLevelControlClusterServerPostInitCallback(endpoint);
}

void MatterLevelControlClusterServerShutdownCallback(EndpointId endpoint)
{
    ChipLogProgress(Zcl, "Shuting down level control server cluster on endpoint %d", endpoint);
    cancelEndpointTimerCallback(endpoint);
}

#ifndef IGNORE_LEVEL_CONTROL_CLUSTER_START_UP_CURRENT_LEVEL
static bool areStartUpLevelControlServerAttributesNonVolatile(EndpointId endpoint)
{
    return !emberAfIsKnownVolatileAttribute(endpoint, LevelControl::Id, Attributes::CurrentLevel::Id) &&
        !emberAfIsKnownVolatileAttribute(endpoint, LevelControl::Id, Attributes::StartUpCurrentLevel::Id);
}
#endif // IGNORE_LEVEL_CONTROL_CLUSTER_START_UP_CURRENT_LEVEL

void emberAfPluginLevelControlClusterServerPostInitCallback(EndpointId endpoint) {}

bool LevelControlHasFeature(EndpointId endpoint, Feature feature)
{
    bool success;
    uint32_t featureMap;
    success = (Attributes::FeatureMap::Get(endpoint, &featureMap) == Status::Success);

    return success ? ((featureMap & to_underlying(feature)) != 0) : false;
}

void MatterLevelControlPluginServerInitCallback() {}
