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

#include "barrier-control-server.h"
#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/CommandHandler.h>
#include <app/ConcreteCommandPath.h>
#include <app/util/af.h>
#include <app/util/config.h>
#include <lib/support/TypeTraits.h>

#include <assert.h>

// We need this for initializating default reporting configurations.
#include <app/reporting/reporting.h>

#include <platform/CHIPDeviceConfig.h>
#include <platform/CHIPDeviceLayer.h>

using namespace chip;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::BarrierControl;
using chip::Protocols::InteractionModel::Status;

// this is NOT in any spec (CHIP spec does not currently have BarrierControl)
// and XMLs do not attach these enums to clusters.
//
// This directly defines some constants. These could be replaced with real
// constants if we ever have some BarrierControl in the matter specification.
namespace chip {
namespace app {
namespace Clusters {
namespace BarrierControl {

namespace Position {
static constexpr uint8_t kClosed  = 0;
static constexpr uint8_t kOpen    = 100;
static constexpr uint8_t kUnknown = 255;
} // namespace Position

namespace MovingState {
static constexpr uint8_t kStopped = 0;
static constexpr uint8_t kClosing = 1;
static constexpr uint8_t kOpening = 2;
} // namespace MovingState

} // namespace BarrierControl
} // namespace Clusters
} // namespace app
} // namespace chip

typedef struct
{
    uint8_t currentPosition;
    uint8_t targetPosition;
    bool increasing;
    uint32_t delayMs;
} State;
static State state;

/**********************************************************
 * Matter timer scheduling glue logic
 *********************************************************/

void emberAfBarrierControlClusterServerTickCallback(EndpointId endpoint);

static void timerCallback(System::Layer *, void * callbackContext)
{
    emberAfBarrierControlClusterServerTickCallback(static_cast<EndpointId>(reinterpret_cast<uintptr_t>(callbackContext)));
}

static void scheduleTimerCallbackMs(EndpointId endpoint, uint32_t delayMs)
{
    DeviceLayer::SystemLayer().StartTimer(chip::System::Clock::Milliseconds32(delayMs), timerCallback,
                                          reinterpret_cast<void *>(static_cast<uintptr_t>(endpoint)));
}

static void cancelEndpointTimerCallback(EndpointId endpoint)
{
    DeviceLayer::SystemLayer().CancelTimer(timerCallback, reinterpret_cast<void *>(static_cast<uintptr_t>(endpoint)));
}

// -----------------------------------------------------------------------------
// Accessing attributes

uint8_t emAfPluginBarrierControlServerGetBarrierPosition(EndpointId endpoint)
{
    uint8_t position;
    Status status = Attributes::BarrierPosition::Get(endpoint, &position);
    assert(status == Status::Success);
    return position;
}

void emAfPluginBarrierControlServerSetBarrierPosition(EndpointId endpoint, uint8_t position)
{
    Status status = Attributes::BarrierPosition::Set(endpoint, position);
    assert(status == Status::Success);
}

bool emAfPluginBarrierControlServerIsPartialBarrierSupported(EndpointId endpoint)
{
    uint8_t bitmap;
    Status status = Attributes::BarrierCapabilities::Get(endpoint, &bitmap);
    assert(status == Status::Success);
    return (bitmap & to_underlying(BarrierControlCapabilities::kPartialBarrier)) != 0;
}

static uint16_t getOpenOrClosePeriod(EndpointId endpoint, bool open)
{
    uint16_t period = 0;
    Status status   = Status::Success;
#if defined(ZCL_USING_BARRIER_CONTROL_CLUSTER_BARRIER_OPEN_PERIOD_ATTRIBUTE)
    if (open)
    {
        status = Attributes::BarrierOpenPeriod::Get(endpoint, &period);
    }
#endif
#if defined(ZCL_USING_BARRIER_CONTROL_CLUSTER_BARRIER_CLOSE_PERIOD_ATTRIBUTE)
    if (!open)
    {
        status = Attributes::BarrierClosePeriod::Get(endpoint, &period);
    }
#endif
    assert(status == Status::Success);
    return period;
}

static void setMovingState(EndpointId endpoint, uint8_t newState)
{
    Status status = Attributes::BarrierMovingState::Set(endpoint, newState);
    assert(status == Status::Success);
}

uint16_t emAfPluginBarrierControlServerGetSafetyStatus(EndpointId endpoint)
{
    uint16_t safetyStatus;
    Status status = Attributes::BarrierSafetyStatus::Get(endpoint, &safetyStatus);
    assert(status == Status::Success);
    return safetyStatus;
}

static bool isRemoteLockoutOn(EndpointId endpoint)
{
    uint16_t safetyStatus = emAfPluginBarrierControlServerGetSafetyStatus(endpoint);
    return (safetyStatus & to_underlying(BarrierControlSafetyStatus::kRemoteLockout)) != 0;
}

void emAfPluginBarrierControlServerIncrementEvents(EndpointId endpoint, bool open, bool command)
{
    Status status   = Status::Success;
    uint16_t events = 0;

#if defined(ZCL_USING_BARRIER_CONTROL_CLUSTER_BARRIER_OPEN_EVENTS_ATTRIBUTE)
    if (open && !command)
    {
        status = Attributes::BarrierOpenEvents::Get(endpoint, &events);
    }
#endif
#if defined(ZCL_USING_BARRIER_CONTROL_CLUSTER_BARRIER_CLOSE_EVENTS_ATTRIBUTE)
    if (!open && !command)
    {
        status = Attributes::BarrierCloseEvents::Get(endpoint, &events);
    }
#endif
#if defined(ZCL_USING_BARRIER_CONTROL_CLUSTER_BARRIER_COMMAND_OPEN_EVENTS_ATTRIBUTE)
    if (open && command)
    {
        status = Attributes::BarrierCommandOpenEvents::Get(endpoint, &events);
    }
#endif
#if defined(ZCL_USING_BARRIER_CONTROL_CLUSTER_BARRIER_COMMAND_CLOSE_EVENTS_ATTRIBUTE)
    if (!open && command)
    {
        status = Attributes::BarrierCommandCloseEvents::Get(endpoint, &events);
    }
#endif
    assert(status == Status::Success);

    // Section 7.1.2.1.5-8 says that this events counter SHALL NOT roll over.
    // The maximum 16-bit unsigned integer in Zigbee is 0xFFFE, so we have this
    // check here.
    if (events == UINT16_MAX - 1)
    {
        return;
    }
    events++;

#if defined(ZCL_USING_BARRIER_CONTROL_CLUSTER_BARRIER_OPEN_EVENTS_ATTRIBUTE)
    if (open && !command)
    {
        status = Attributes::BarrierOpenEvents::Set(endpoint, events);
    }
#endif
#if defined(ZCL_USING_BARRIER_CONTROL_CLUSTER_BARRIER_CLOSE_EVENTS_ATTRIBUTE)
    if (!open && !command)
    {
        status = Attributes::BarrierCloseEvents::Set(endpoint, events);
    }
#endif
#if defined(ZCL_USING_BARRIER_CONTROL_CLUSTER_BARRIER_COMMAND_OPEN_EVENTS_ATTRIBUTE)
    if (open && command)
    {
        status = Attributes::BarrierCommandOpenEvents::Set(endpoint, events);
    }
#endif
#if defined(ZCL_USING_BARRIER_CONTROL_CLUSTER_BARRIER_COMMAND_CLOSE_EVENTS_ATTRIBUTE)
    if (!open && command)
    {
        status = Attributes::BarrierCommandCloseEvents::Set(endpoint, events);
    }
#endif
    assert(status == Status::Success);
}

// -----------------------------------------------------------------------------
// Opening/closing barrier

static uint8_t getCurrentPosition(EndpointId endpoint)
{
    // If the BarrierPosition attribute does not store the exact position of the
    // barrier, then it will be set to 0xFF. If this is the case, then we have no
    // way of knowing the position of the barrier. Let's guess that the barrier is
    // open so that we don't leave the barrier open when it should be closed.
    uint8_t currentPositionFromAttribute = emAfPluginBarrierControlServerGetBarrierPosition(endpoint);
    return ((currentPositionFromAttribute == BarrierControl::Position::kUnknown) ? BarrierControl::Position::kOpen
                                                                                 : currentPositionFromAttribute);
}

static uint32_t calculateDelayMs(EndpointId endpoint, uint8_t targetPosition, bool * opening)
{
    uint8_t currentPosition = emAfPluginBarrierControlServerGetBarrierPosition(endpoint);
    *opening                = targetPosition > currentPosition;
    uint8_t positionDelta   = static_cast<uint8_t>(*opening ? targetPosition - currentPosition : currentPosition - targetPosition);
    uint16_t openOrClosePeriodDs = getOpenOrClosePeriod(endpoint, *opening);
    uint32_t openOrClosePeriodMs = openOrClosePeriodDs * MILLISECOND_TICKS_PER_DECISECOND;

    // We use a minimum delay so that our barrier changes position in a realistic
    // amount of time.
    if (openOrClosePeriodDs == 0 || positionDelta == 0)
    {
        return MIN_POSITION_CHANGE_DELAY_MS;
    }

    uint32_t delayMs = openOrClosePeriodMs / positionDelta;
    return (delayMs < MIN_POSITION_CHANGE_DELAY_MS ? MIN_POSITION_CHANGE_DELAY_MS : delayMs);
}

void emberAfBarrierControlClusterServerTickCallback(EndpointId endpoint)
{
    if (state.currentPosition == state.targetPosition)
    {
        emAfPluginBarrierControlServerSetBarrierPosition(endpoint, state.currentPosition);
        setMovingState(endpoint, BarrierControl::MovingState::kStopped);
        cancelEndpointTimerCallback(endpoint);
    }
    else
    {
        if (state.increasing)
        {
            if (++state.currentPosition == 1)
            {
                // Zero -> nonzero: open event
                emAfPluginBarrierControlServerIncrementEvents(endpoint, true, false);
            }
        }
        else
        {
            if (--state.currentPosition == 0)
            {
                // Nonzero -> zero: close event
                emAfPluginBarrierControlServerIncrementEvents(endpoint, false, false);
            }
        }
        emAfPluginBarrierControlServerSetBarrierPosition(endpoint,
                                                         (emAfPluginBarrierControlServerIsPartialBarrierSupported(endpoint)
                                                              ? state.currentPosition
                                                              : BarrierControl::Position::kUnknown));
        setMovingState(endpoint,
                       (state.increasing ? BarrierControl::MovingState::kOpening : BarrierControl::MovingState::kClosing));

        scheduleTimerCallbackMs(endpoint, state.delayMs);
    }
}

// -----------------------------------------------------------------------------
// Handling commands

static void sendDefaultResponse(app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath, Status status)
{
    commandObj->AddStatus(commandPath, status);
}

bool emberAfBarrierControlClusterBarrierControlGoToPercentCallback(
    app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath,
    const Commands::BarrierControlGoToPercent::DecodableType & commandData)
{
    auto & percentOpen = commandData.percentOpen;

    EndpointId endpoint = commandPath.mEndpointId;
    Status status       = Status::Success;

    ChipLogProgress(Zcl, "RX: GoToPercentCallback p=%d", percentOpen);

    if (isRemoteLockoutOn(endpoint))
    {
        status = Status::Failure;
    }
    else if (percentOpen > 100 // "100" means "100%", so greater than that is invalid
             || (!emAfPluginBarrierControlServerIsPartialBarrierSupported(endpoint) &&
                 percentOpen != BarrierControl::Position::kClosed && percentOpen != BarrierControl::Position::kOpen))
    {
        status = Status::ConstraintError;
    }
    else
    {
        state.currentPosition = getCurrentPosition(endpoint);
        state.targetPosition  = percentOpen;
        state.delayMs         = calculateDelayMs(endpoint, state.targetPosition, &state.increasing);
        ChipLogProgress(Zcl, "Scheduling barrier move from %d to %d with %" PRIu32 "ms delay", state.currentPosition,
                        state.targetPosition, state.delayMs);
        scheduleTimerCallbackMs(endpoint, state.delayMs);

        if (state.currentPosition < state.targetPosition)
        {
            emAfPluginBarrierControlServerIncrementEvents(endpoint, true, true);
        }
        else if (state.currentPosition > state.targetPosition)
        {
            emAfPluginBarrierControlServerIncrementEvents(endpoint, false, true);
        }
    }

    sendDefaultResponse(commandObj, commandPath, status);

    return true;
}

bool emberAfBarrierControlClusterBarrierControlStopCallback(app::CommandHandler * commandObj,
                                                            const app::ConcreteCommandPath & commandPath,
                                                            const Commands::BarrierControlStop::DecodableType & commandData)
{
    EndpointId endpoint = commandPath.mEndpointId;
    cancelEndpointTimerCallback(endpoint);
    setMovingState(endpoint, BarrierControl::MovingState::kStopped);
    sendDefaultResponse(commandObj, commandPath, Status::Success);
    return true;
}

void MatterBarrierControlPluginServerInitCallback() {}

void MatterBarrierControlClusterServerShutdownCallback(EndpointId endpoint)
{
    ChipLogProgress(Zcl, "Shuting barrier control server cluster on endpoint %d", endpoint);
    cancelEndpointTimerCallback(endpoint);
}
