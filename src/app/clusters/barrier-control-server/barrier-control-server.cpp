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
#include <app-common/zap-generated/af-structs.h>
#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/CommandHandler.h>
#include <app/ConcreteCommandPath.h>
#include <app/util/af.h>

#include <assert.h>

// We need this for initializating default reporting configurations.
#include <app/reporting/reporting.h>

using namespace chip;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::BarrierControl;

typedef struct
{
    uint8_t currentPosition;
    uint8_t targetPosition;
    bool increasing;
    uint32_t delayMs;
} State;
static State state;

#ifdef EMBER_SCRIPTED_TEST
#define ZCL_USING_BARRIER_CONTROL_CLUSTER_OPEN_PERIOD_ATTRIBUTE
#define ZCL_USING_BARRIER_CONTROL_CLUSTER_CLOSE_PERIOD_ATTRIBUTE
#define ZCL_USING_BARRIER_CONTROL_CLUSTER_BARRIER_OPEN_EVENTS_ATTRIBUTE
#define ZCL_USING_BARRIER_CONTROL_CLUSTER_BARRIER_CLOSE_EVENTS_ATTRIBUTE
#define ZCL_USING_BARRIER_CONTROL_CLUSTER_BARRIER_COMMAND_OPEN_EVENTS_ATTRIBUTE
#define ZCL_USING_BARRIER_CONTROL_CLUSTER_BARRIER_COMMAND_CLOSE_EVENTS_ATTRIBUTE
#endif

// -----------------------------------------------------------------------------
// Accessing attributes

uint8_t emAfPluginBarrierControlServerGetBarrierPosition(EndpointId endpoint)
{
    uint8_t position;
    EmberAfStatus status = Attributes::BarrierPosition::Get(endpoint, &position);
    assert(status == EMBER_ZCL_STATUS_SUCCESS);
    return position;
}

void emAfPluginBarrierControlServerSetBarrierPosition(EndpointId endpoint, uint8_t position)
{
    EmberAfStatus status = Attributes::BarrierPosition::Set(endpoint, position);
    assert(status == EMBER_ZCL_STATUS_SUCCESS);
}

bool emAfPluginBarrierControlServerIsPartialBarrierSupported(EndpointId endpoint)
{
    uint8_t bitmap;
    EmberAfStatus status = Attributes::BarrierCapabilities::Get(endpoint, &bitmap);
    assert(status == EMBER_ZCL_STATUS_SUCCESS);
    return READBITS(bitmap, EMBER_AF_BARRIER_CONTROL_CAPABILITIES_PARTIAL_BARRIER);
}

static uint16_t getOpenOrClosePeriod(EndpointId endpoint, bool open)
{
    uint16_t period      = 0;
    EmberAfStatus status = EMBER_ZCL_STATUS_SUCCESS;
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
    assert(status == EMBER_ZCL_STATUS_SUCCESS);
    return period;
}

static void setMovingState(EndpointId endpoint, uint8_t newState)
{
    EmberAfStatus status = Attributes::BarrierMovingState::Set(endpoint, newState);
    assert(status == EMBER_ZCL_STATUS_SUCCESS);
}

uint16_t emAfPluginBarrierControlServerGetSafetyStatus(EndpointId endpoint)
{
    uint16_t safetyStatus;
    EmberAfStatus status = Attributes::BarrierSafetyStatus::Get(endpoint, &safetyStatus);
    assert(status == EMBER_ZCL_STATUS_SUCCESS);
    return safetyStatus;
}

static bool isRemoteLockoutOn(EndpointId endpoint)
{
    uint16_t safetyStatus = emAfPluginBarrierControlServerGetSafetyStatus(endpoint);
    return READBITS(safetyStatus, EMBER_AF_BARRIER_CONTROL_SAFETY_STATUS_REMOTE_LOCKOUT);
}

void emAfPluginBarrierControlServerIncrementEvents(EndpointId endpoint, bool open, bool command)
{
    EmberAfStatus status = EMBER_ZCL_STATUS_SUCCESS;
    uint16_t events      = 0;

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
    assert(status == EMBER_ZCL_STATUS_SUCCESS);

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
    assert(status == EMBER_ZCL_STATUS_SUCCESS);
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
    return ((currentPositionFromAttribute == EMBER_ZCL_BARRIER_CONTROL_BARRIER_POSITION_UNKNOWN)
                ? static_cast<uint8_t>(EMBER_ZCL_BARRIER_CONTROL_BARRIER_POSITION_OPEN)
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
        setMovingState(endpoint, EMBER_ZCL_BARRIER_CONTROL_MOVING_STATE_STOPPED);
        emberAfDeactivateServerTick(endpoint, BarrierControl::Id);
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
        emAfPluginBarrierControlServerSetBarrierPosition(
            endpoint,
            (emAfPluginBarrierControlServerIsPartialBarrierSupported(endpoint)
                 ? state.currentPosition
                 : static_cast<uint8_t>(EMBER_ZCL_BARRIER_CONTROL_BARRIER_POSITION_UNKNOWN)));
        setMovingState(
            endpoint,
            (state.increasing ? EMBER_ZCL_BARRIER_CONTROL_MOVING_STATE_OPENING : EMBER_ZCL_BARRIER_CONTROL_MOVING_STATE_CLOSING));
        emberAfScheduleServerTick(endpoint, BarrierControl::Id, state.delayMs);
    }
}

// -----------------------------------------------------------------------------
// Handling commands

static void sendDefaultResponse(EmberAfStatus status)
{
    if (emberAfSendImmediateDefaultResponse(status) != EMBER_SUCCESS)
    {
        emberAfBarrierControlClusterPrintln("Failed to send default response");
    }
}

bool emberAfBarrierControlClusterBarrierControlGoToPercentCallback(
    app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath,
    const Commands::BarrierControlGoToPercent::DecodableType & commandData)
{
    auto & percentOpen = commandData.percentOpen;

    EndpointId endpoint  = commandPath.mEndpointId;
    EmberAfStatus status = EMBER_ZCL_STATUS_SUCCESS;

    emberAfBarrierControlClusterPrintln("RX: GoToPercentCallback p=%d", percentOpen);

    if (isRemoteLockoutOn(endpoint))
    {
        status = EMBER_ZCL_STATUS_FAILURE;
    }
    else if (percentOpen > 100 // "100" means "100%", so greater than that is invalid
             || (!emAfPluginBarrierControlServerIsPartialBarrierSupported(endpoint) &&
                 percentOpen != EMBER_ZCL_BARRIER_CONTROL_BARRIER_POSITION_CLOSED &&
                 percentOpen != EMBER_ZCL_BARRIER_CONTROL_BARRIER_POSITION_OPEN))
    {
        status = EMBER_ZCL_STATUS_CONSTRAINT_ERROR;
    }
    else
    {
        state.currentPosition = getCurrentPosition(endpoint);
        state.targetPosition  = percentOpen;
        state.delayMs         = calculateDelayMs(endpoint, state.targetPosition, &state.increasing);
        emberAfBarrierControlClusterPrintln("Scheduling barrier move from %d to %d with %" PRIu32 "ms delay", state.currentPosition,
                                            state.targetPosition, state.delayMs);
        emberAfScheduleServerTick(endpoint, BarrierControl::Id, state.delayMs);

        if (state.currentPosition < state.targetPosition)
        {
            emAfPluginBarrierControlServerIncrementEvents(endpoint, true, true);
        }
        else if (state.currentPosition > state.targetPosition)
        {
            emAfPluginBarrierControlServerIncrementEvents(endpoint, false, true);
        }
    }

    sendDefaultResponse(status);

    return true;
}

bool emberAfBarrierControlClusterBarrierControlStopCallback(app::CommandHandler * commandObj,
                                                            const app::ConcreteCommandPath & commandPath,
                                                            const Commands::BarrierControlStop::DecodableType & commandData)
{
    EndpointId endpoint = commandPath.mEndpointId;
    emberAfDeactivateServerTick(endpoint, BarrierControl::Id);
    setMovingState(endpoint, EMBER_ZCL_BARRIER_CONTROL_MOVING_STATE_STOPPED);
    sendDefaultResponse(EMBER_ZCL_STATUS_SUCCESS);
    return true;
}

void MatterBarrierControlPluginServerInitCallback() {}
