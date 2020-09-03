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

/**
 *
 *    Copyright (c) 2020 Silicon Labs
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
/****************************************************************************
 * @file
 * @brief Implementation for the Barrier Control
 *Server plugin.
 *******************************************************************************
 ******************************************************************************/

#include "barrier-control-server.h"
#include <app/util/af.h>

#include <assert.h>

// We need this for initializating default reporting configurations.
#include <app/reporting/reporting.h>

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
// Framework initialization

void emberAfPluginBarrierControlServerInitCallback(void) {}

// -----------------------------------------------------------------------------
// Accessing attributes

uint8_t emAfPluginBarrierControlServerGetBarrierPosition(uint8_t endpoint)
{
    uint8_t position;
    EmberAfStatus status = emberAfReadServerAttribute(endpoint, ZCL_BARRIER_CONTROL_CLUSTER_ID, ZCL_BARRIER_POSITION_ATTRIBUTE_ID,
                                                      &position, sizeof(position));
    assert(status == EMBER_ZCL_STATUS_SUCCESS);
    return position;
}

void emAfPluginBarrierControlServerSetBarrierPosition(uint8_t endpoint, uint8_t position)
{
    EmberAfStatus status = emberAfWriteServerAttribute(endpoint, ZCL_BARRIER_CONTROL_CLUSTER_ID, ZCL_BARRIER_POSITION_ATTRIBUTE_ID,
                                                       &position, ZCL_INT8U_ATTRIBUTE_TYPE);
    assert(status == EMBER_ZCL_STATUS_SUCCESS);
}

bool emAfPluginBarrierControlServerIsPartialBarrierSupported(uint8_t endpoint)
{
    uint8_t bitmap;
    EmberAfStatus status = emberAfReadServerAttribute(endpoint, ZCL_BARRIER_CONTROL_CLUSTER_ID,
                                                      ZCL_BARRIER_CAPABILITIES_ATTRIBUTE_ID, &bitmap, sizeof(bitmap));
    assert(status == EMBER_ZCL_STATUS_SUCCESS);
    return READBITS(bitmap, EMBER_AF_BARRIER_CONTROL_CAPABILITIES_PARTIAL_BARRIER);
}

static uint16_t getOpenOrClosePeriod(uint8_t endpoint, bool open)
{
    uint16_t period                = 0;
    EmberAfAttributeId attributeId = 0xFFFF;
#if defined(ZCL_USING_BARRIER_CONTROL_CLUSTER_BARRIER_OPEN_PERIOD_ATTRIBUTE)
    if (open)
    {
        attributeId = ZCL_BARRIER_OPEN_PERIOD_ATTRIBUTE_ID;
    }
#endif
#if defined(ZCL_USING_BARRIER_CONTROL_CLUSTER_BARRIER_CLOSE_PERIOD_ATTRIBUTE)
    if (!open)
    {
        attributeId = ZCL_BARRIER_CLOSE_PERIOD_ATTRIBUTE_ID;
    }
#endif

    if (attributeId != 0xFFFF)
    {
        EmberAfStatus status =
            emberAfReadServerAttribute(endpoint, ZCL_BARRIER_CONTROL_CLUSTER_ID, attributeId, (uint8_t *) &period, sizeof(period));
        assert(status == EMBER_ZCL_STATUS_SUCCESS);
    }

    return period;
}

static void setMovingState(uint8_t endpoint, uint8_t state)
{
    EmberAfStatus status = emberAfWriteServerAttribute(endpoint, ZCL_BARRIER_CONTROL_CLUSTER_ID,
                                                       ZCL_BARRIER_MOVING_STATE_ATTRIBUTE_ID, &state, ZCL_ENUM8_ATTRIBUTE_TYPE);
    assert(status == EMBER_ZCL_STATUS_SUCCESS);
}

uint16_t emAfPluginBarrierControlServerGetSafetyStatus(uint8_t endpoint)
{
    uint16_t safetyStatus;
    EmberAfStatus status =
        emberAfReadServerAttribute(endpoint, ZCL_BARRIER_CONTROL_CLUSTER_ID, ZCL_BARRIER_SAFETY_STATUS_ATTRIBUTE_ID,
                                   (uint8_t *) &safetyStatus, sizeof(safetyStatus));
    assert(status == EMBER_ZCL_STATUS_SUCCESS);
    return safetyStatus;
}

static bool isRemoteLockoutOn(uint8_t endpoint)
{
    uint16_t safetyStatus = emAfPluginBarrierControlServerGetSafetyStatus(endpoint);
    return READBITS(safetyStatus, EMBER_AF_BARRIER_CONTROL_SAFETY_STATUS_REMOTE_LOCKOUT);
}

void emAfPluginBarrierControlServerIncrementEvents(uint8_t endpoint, bool open, bool command)
{
    uint8_t mask = (0
#if defined(ZCL_USING_BARRIER_CONTROL_CLUSTER_BARRIER_OPEN_EVENTS_ATTRIBUTE)
                    | (open && !command ? BIT(0) : 0)
#endif
#if defined(ZCL_USING_BARRIER_CONTROL_CLUSTER_BARRIER_CLOSE_EVENTS_ATTRIBUTE)
                    | (!open && !command ? BIT(1) : 0)
#endif
#if defined(ZCL_USING_BARRIER_CONTROL_CLUSTER_BARRIER_COMMAND_OPEN_EVENTS_ATTRIBUTE)
                    | (open && command ? BIT(2) : 0)
#endif
#if defined(ZCL_USING_BARRIER_CONTROL_CLUSTER_BARRIER_COMMAND_CLOSE_EVENTS_ATTRIBUTE)
                    | (!open && command ? BIT(3) : 0)
#endif
    );

    EmberAfAttributeId baseEventAttributeId = ZCL_BARRIER_OPEN_EVENTS_ATTRIBUTE_ID;
    for (size_t bit = 0; bit < 4; bit++)
    {
        if (READBIT(mask, bit))
        {
            EmberAfAttributeId attributeId = baseEventAttributeId + bit;
            uint16_t events;
            EmberAfStatus status = emberAfReadServerAttribute(endpoint, ZCL_BARRIER_CONTROL_CLUSTER_ID, attributeId,
                                                              (uint8_t *) &events, sizeof(events));
            assert(status == EMBER_ZCL_STATUS_SUCCESS);

            // Section 7.1.2.1.5-8 says that this events counter SHALL NOT roll over.
            // The maximum 16-bit unsigned integer in Zigbee is 0xFFFE, so we have this
            // check here.
            if (events != UINT16_MAX - 1)
            {
                events++;
                status = emberAfWriteServerAttribute(endpoint, ZCL_BARRIER_CONTROL_CLUSTER_ID, attributeId, (uint8_t *) &events,
                                                     ZCL_INT16U_ATTRIBUTE_TYPE);
                assert(status == EMBER_ZCL_STATUS_SUCCESS);
            }
        }
    }
}

// -----------------------------------------------------------------------------
// Opening/closing barrier

static uint8_t getCurrentPosition(uint8_t endpoint)
{
    // If the BarrierPosition attribute does not store the exact position of the
    // barrier, then it will be set to 0xFF. If this is the case, then we have no
    // way of knowing the position of the barrier. Let's guess that the barrier is
    // open so that we don't leave the barrier open when it should be closed.
    uint8_t currentPositionFromAttribute = emAfPluginBarrierControlServerGetBarrierPosition(endpoint);
    return ((currentPositionFromAttribute == EMBER_ZCL_BARRIER_CONTROL_BARRIER_POSITION_UNKNOWN)
                ? EMBER_ZCL_BARRIER_CONTROL_BARRIER_POSITION_OPEN
                : currentPositionFromAttribute);
}

static uint32_t calculateDelayMs(uint8_t endpoint, uint8_t targetPosition, bool * opening)
{
    uint8_t currentPosition      = emAfPluginBarrierControlServerGetBarrierPosition(endpoint);
    *opening                     = targetPosition > currentPosition;
    uint8_t positionDelta        = (*opening ? targetPosition - currentPosition : currentPosition - targetPosition);
    uint16_t openOrClosePeriodDs = getOpenOrClosePeriod(endpoint, *opening);
    uint32_t openOrClosePeriodMs = openOrClosePeriodDs * MILLISECOND_TICKS_PER_DECISECOND;

    // We use a minimum delay so that our barrier changes position in a realistic
    // amount of time.
    if (openOrClosePeriodDs == 0 || positionDelta == 0)
    {
        return MIN_POSITION_CHANGE_DELAY_MS;
    }
    else
    {
        uint32_t delayMs = openOrClosePeriodMs / positionDelta;
        return (delayMs < MIN_POSITION_CHANGE_DELAY_MS ? MIN_POSITION_CHANGE_DELAY_MS : delayMs);
    }
}

void emberAfBarrierControlClusterServerTickCallback(uint8_t endpoint)
{
    if (state.currentPosition == state.targetPosition)
    {
        emAfPluginBarrierControlServerSetBarrierPosition(endpoint, state.currentPosition);
        setMovingState(endpoint, EMBER_ZCL_BARRIER_CONTROL_MOVING_STATE_STOPPED);
        emberAfDeactivateServerTick(endpoint, ZCL_BARRIER_CONTROL_CLUSTER_ID);
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
                                                              : EMBER_ZCL_BARRIER_CONTROL_BARRIER_POSITION_UNKNOWN));
        setMovingState(
            endpoint,
            (state.increasing ? EMBER_ZCL_BARRIER_CONTROL_MOVING_STATE_OPENING : EMBER_ZCL_BARRIER_CONTROL_MOVING_STATE_CLOSING));
        emberAfScheduleServerTick(endpoint, ZCL_BARRIER_CONTROL_CLUSTER_ID, state.delayMs);
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

bool emberAfBarrierControlClusterBarrierControlGoToPercentCallback(uint8_t percentOpen)
{
    uint8_t endpoint     = emberAfCurrentCommand()->apsFrame->destinationEndpoint;
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
        status = EMBER_ZCL_STATUS_INVALID_VALUE;
    }
    else
    {
        state.currentPosition = getCurrentPosition(endpoint);
        state.targetPosition  = percentOpen;
        state.delayMs         = calculateDelayMs(endpoint, state.targetPosition, &state.increasing);
        emberAfBarrierControlClusterPrintln("Scheduling barrier move from %d to %d with %dms delay", state.currentPosition,
                                            state.targetPosition, state.delayMs);
        emberAfScheduleServerTick(endpoint, ZCL_BARRIER_CONTROL_CLUSTER_ID, state.delayMs);

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

bool emberAfBarrierControlClusterBarrierControlStopCallback(void)
{
    uint8_t endpoint = emberAfCurrentCommand()->apsFrame->destinationEndpoint;
    emberAfDeactivateServerTick(endpoint, ZCL_BARRIER_CONTROL_CLUSTER_ID);
    setMovingState(endpoint, EMBER_ZCL_BARRIER_CONTROL_MOVING_STATE_STOPPED);
    sendDefaultResponse(EMBER_ZCL_STATUS_SUCCESS);
    return true;
}
