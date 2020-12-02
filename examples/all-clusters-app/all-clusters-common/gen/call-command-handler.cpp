
/*
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

#include <stdint.h>

#include "af-structs.h"
#include "call-command-handler.h"
#include "callback.h"
#include "command-id.h"
#include "util.h"

using namespace chip;

EmberAfStatus emberAfBarrierControlClusterClientCommandParse(EmberAfClusterCommand * cmd);
EmberAfStatus emberAfBarrierControlClusterServerCommandParse(EmberAfClusterCommand * cmd);
EmberAfStatus emberAfBasicClusterClientCommandParse(EmberAfClusterCommand * cmd);
EmberAfStatus emberAfBasicClusterServerCommandParse(EmberAfClusterCommand * cmd);
EmberAfStatus emberAfColorControlClusterClientCommandParse(EmberAfClusterCommand * cmd);
EmberAfStatus emberAfColorControlClusterServerCommandParse(EmberAfClusterCommand * cmd);
EmberAfStatus emberAfDoorLockClusterClientCommandParse(EmberAfClusterCommand * cmd);
EmberAfStatus emberAfDoorLockClusterServerCommandParse(EmberAfClusterCommand * cmd);
EmberAfStatus emberAfGroupsClusterClientCommandParse(EmberAfClusterCommand * cmd);
EmberAfStatus emberAfGroupsClusterServerCommandParse(EmberAfClusterCommand * cmd);
EmberAfStatus emberAfIasZoneClusterServerCommandParse(EmberAfClusterCommand * cmd);
EmberAfStatus emberAfIdentifyClusterClientCommandParse(EmberAfClusterCommand * cmd);
EmberAfStatus emberAfIdentifyClusterServerCommandParse(EmberAfClusterCommand * cmd);
EmberAfStatus emberAfLevelControlClusterClientCommandParse(EmberAfClusterCommand * cmd);
EmberAfStatus emberAfLevelControlClusterServerCommandParse(EmberAfClusterCommand * cmd);
EmberAfStatus emberAfOnOffClusterClientCommandParse(EmberAfClusterCommand * cmd);
EmberAfStatus emberAfOnOffClusterServerCommandParse(EmberAfClusterCommand * cmd);
EmberAfStatus emberAfScenesClusterClientCommandParse(EmberAfClusterCommand * cmd);
EmberAfStatus emberAfScenesClusterServerCommandParse(EmberAfClusterCommand * cmd);
EmberAfStatus emberAfTemperatureMeasurementClusterServerCommandParse(EmberAfClusterCommand * cmd);

static EmberAfStatus status(bool wasHandled, bool clusterExists, bool mfgSpecific)
{
    if (wasHandled)
    {
        return EMBER_ZCL_STATUS_SUCCESS;
    }
    else if (mfgSpecific)
    {
        return EMBER_ZCL_STATUS_UNSUP_MANUF_CLUSTER_COMMAND;
    }
    else if (clusterExists)
    {
        return EMBER_ZCL_STATUS_UNSUP_COMMAND;
    }
    else
    {
        return EMBER_ZCL_STATUS_UNSUPPORTED_CLUSTER;
    }
}

// Main command parsing controller.
EmberAfStatus emberAfClusterSpecificCommandParse(EmberAfClusterCommand * cmd)
{
    EmberAfStatus result = status(false, false, cmd->mfgSpecific);
    if (cmd->direction == (uint8_t) ZCL_DIRECTION_SERVER_TO_CLIENT &&
        emberAfContainsClientWithMfgCode(cmd->apsFrame->destinationEndpoint, cmd->apsFrame->clusterId, cmd->mfgCode))
    {
        switch (cmd->apsFrame->clusterId)
        {
        case ZCL_BARRIER_CONTROL_CLUSTER_ID:
            result = emberAfBarrierControlClusterClientCommandParse(cmd);
            break;
        case ZCL_BASIC_CLUSTER_ID:
            result = emberAfBasicClusterClientCommandParse(cmd);
            break;
        case ZCL_COLOR_CONTROL_CLUSTER_ID:
            result = emberAfColorControlClusterClientCommandParse(cmd);
            break;
        case ZCL_DOOR_LOCK_CLUSTER_ID:
            result = emberAfDoorLockClusterClientCommandParse(cmd);
            break;
        case ZCL_GROUPS_CLUSTER_ID:
            result = emberAfGroupsClusterClientCommandParse(cmd);
            break;
        case ZCL_IDENTIFY_CLUSTER_ID:
            result = emberAfIdentifyClusterClientCommandParse(cmd);
            break;
        case ZCL_LEVEL_CONTROL_CLUSTER_ID:
            result = emberAfLevelControlClusterClientCommandParse(cmd);
            break;
        case ZCL_ON_OFF_CLUSTER_ID:
            result = emberAfOnOffClusterClientCommandParse(cmd);
            break;
        case ZCL_SCENES_CLUSTER_ID:
            result = emberAfScenesClusterClientCommandParse(cmd);
            break;
        default:
            // Unrecognized cluster ID, error status will apply.
            break;
        }
    }
    else if (cmd->direction == (uint8_t) ZCL_DIRECTION_CLIENT_TO_SERVER &&
             emberAfContainsServerWithMfgCode(cmd->apsFrame->destinationEndpoint, cmd->apsFrame->clusterId, cmd->mfgCode))
    {
        switch (cmd->apsFrame->clusterId)
        {
        case ZCL_BARRIER_CONTROL_CLUSTER_ID:
            result = emberAfBarrierControlClusterServerCommandParse(cmd);
            break;
        case ZCL_BASIC_CLUSTER_ID:
            result = emberAfBasicClusterServerCommandParse(cmd);
            break;
        case ZCL_COLOR_CONTROL_CLUSTER_ID:
            result = emberAfColorControlClusterServerCommandParse(cmd);
            break;
        case ZCL_DOOR_LOCK_CLUSTER_ID:
            result = emberAfDoorLockClusterServerCommandParse(cmd);
            break;
        case ZCL_GROUPS_CLUSTER_ID:
            result = emberAfGroupsClusterServerCommandParse(cmd);
            break;
        case ZCL_IAS_ZONE_CLUSTER_ID:
            result = emberAfIasZoneClusterServerCommandParse(cmd);
            break;
        case ZCL_IDENTIFY_CLUSTER_ID:
            result = emberAfIdentifyClusterServerCommandParse(cmd);
            break;
        case ZCL_LEVEL_CONTROL_CLUSTER_ID:
            result = emberAfLevelControlClusterServerCommandParse(cmd);
            break;
        case ZCL_ON_OFF_CLUSTER_ID:
            result = emberAfOnOffClusterServerCommandParse(cmd);
            break;
        case ZCL_SCENES_CLUSTER_ID:
            result = emberAfScenesClusterServerCommandParse(cmd);
            break;
        case ZCL_TEMP_MEASUREMENT_CLUSTER_ID:
            result = emberAfTemperatureMeasurementClusterServerCommandParse(cmd);
            break;
        default:
            // Unrecognized cluster ID, error status will apply.
            break;
        }
    }
    return result;
}

// Cluster specific command parsing

EmberAfStatus emberAfBarrierControlClusterClientCommandParse(EmberAfClusterCommand * cmd)
{
    bool wasHandled = false;

    if (!cmd->mfgSpecific)
    {
        switch (cmd->commandId)
        {
        default: {
            // Unrecognized command ID, error status will apply.
            break;
        }
        }
    }
    return status(wasHandled, true, cmd->mfgSpecific);
}
EmberAfStatus emberAfBarrierControlClusterServerCommandParse(EmberAfClusterCommand * cmd)
{
    bool wasHandled = false;

    if (!cmd->mfgSpecific)
    {
        switch (cmd->commandId)
        {
        case ZCL_BARRIER_CONTROL_GO_TO_PERCENT_COMMAND_ID: {
            uint32_t payloadOffset = cmd->payloadStartIndex;
            uint8_t percentOpen;

            percentOpen = (*(uint8_t *) (cmd->buffer + payloadOffset));

            wasHandled = emberAfBarrierControlClusterBarrierControlGoToPercentCallback(percentOpen);
            break;
        }
        case ZCL_BARRIER_CONTROL_STOP_COMMAND_ID: {
            wasHandled = emberAfBarrierControlClusterBarrierControlStopCallback();
            break;
        }
        default: {
            // Unrecognized command ID, error status will apply.
            break;
        }
        }
    }
    return status(wasHandled, true, cmd->mfgSpecific);
}
EmberAfStatus emberAfBasicClusterClientCommandParse(EmberAfClusterCommand * cmd)
{
    bool wasHandled = false;

    if (!cmd->mfgSpecific)
    {
        switch (cmd->commandId)
        {
        default: {
            // Unrecognized command ID, error status will apply.
            break;
        }
        }
    }
    return status(wasHandled, true, cmd->mfgSpecific);
}
EmberAfStatus emberAfBasicClusterServerCommandParse(EmberAfClusterCommand * cmd)
{
    bool wasHandled = false;

    if (!cmd->mfgSpecific)
    {
        switch (cmd->commandId)
        {
        case ZCL_RESET_TO_FACTORY_DEFAULTS_COMMAND_ID: {
            wasHandled = emberAfBasicClusterResetToFactoryDefaultsCallback();
            break;
        }
        default: {
            // Unrecognized command ID, error status will apply.
            break;
        }
        }
    }
    return status(wasHandled, true, cmd->mfgSpecific);
}
EmberAfStatus emberAfColorControlClusterClientCommandParse(EmberAfClusterCommand * cmd)
{
    bool wasHandled = false;

    if (!cmd->mfgSpecific)
    {
        switch (cmd->commandId)
        {
        default: {
            // Unrecognized command ID, error status will apply.
            break;
        }
        }
    }
    return status(wasHandled, true, cmd->mfgSpecific);
}
EmberAfStatus emberAfColorControlClusterServerCommandParse(EmberAfClusterCommand * cmd)
{
    bool wasHandled = false;

    if (!cmd->mfgSpecific)
    {
        switch (cmd->commandId)
        {
        case ZCL_MOVE_COLOR_COMMAND_ID: {
            uint32_t payloadOffset = cmd->payloadStartIndex;
            int16_t rateX;
            int16_t rateY;
            uint8_t optionsMask;
            uint8_t optionsOverride;

            rateX = (*(int16_t *) (cmd->buffer + payloadOffset));
            payloadOffset += sizeof(int16_t);
            rateY = (*(int16_t *) (cmd->buffer + payloadOffset));
            payloadOffset += sizeof(int16_t);
            optionsMask = (*(uint8_t *) (cmd->buffer + payloadOffset));
            payloadOffset += sizeof(uint8_t);
            optionsOverride = (*(uint8_t *) (cmd->buffer + payloadOffset));

            wasHandled = emberAfColorControlClusterMoveColorCallback(rateX, rateY, optionsMask, optionsOverride);
            break;
        }
        case ZCL_MOVE_COLOR_TEMPERATURE_COMMAND_ID: {
            uint32_t payloadOffset = cmd->payloadStartIndex;
            uint8_t moveMode;
            uint16_t rate;
            uint16_t colorTemperatureMinimum;
            uint16_t colorTemperatureMaximum;
            uint8_t optionsMask;
            uint8_t optionsOverride;

            moveMode = (*(uint8_t *) (cmd->buffer + payloadOffset));
            payloadOffset += sizeof(uint8_t);
            rate = (*(uint16_t *) (cmd->buffer + payloadOffset));
            payloadOffset += sizeof(uint16_t);
            colorTemperatureMinimum = (*(uint16_t *) (cmd->buffer + payloadOffset));
            payloadOffset += sizeof(uint16_t);
            colorTemperatureMaximum = (*(uint16_t *) (cmd->buffer + payloadOffset));
            payloadOffset += sizeof(uint16_t);
            optionsMask = (*(uint8_t *) (cmd->buffer + payloadOffset));
            payloadOffset += sizeof(uint8_t);
            optionsOverride = (*(uint8_t *) (cmd->buffer + payloadOffset));

            wasHandled = emberAfColorControlClusterMoveColorTemperatureCallback(
                moveMode, rate, colorTemperatureMinimum, colorTemperatureMaximum, optionsMask, optionsOverride);
            break;
        }
        case ZCL_MOVE_HUE_COMMAND_ID: {
            uint32_t payloadOffset = cmd->payloadStartIndex;
            uint8_t moveMode;
            uint8_t rate;
            uint8_t optionsMask;
            uint8_t optionsOverride;

            moveMode = (*(uint8_t *) (cmd->buffer + payloadOffset));
            payloadOffset += sizeof(uint8_t);
            rate = (*(uint8_t *) (cmd->buffer + payloadOffset));
            payloadOffset += sizeof(uint8_t);
            optionsMask = (*(uint8_t *) (cmd->buffer + payloadOffset));
            payloadOffset += sizeof(uint8_t);
            optionsOverride = (*(uint8_t *) (cmd->buffer + payloadOffset));

            wasHandled = emberAfColorControlClusterMoveHueCallback(moveMode, rate, optionsMask, optionsOverride);
            break;
        }
        case ZCL_MOVE_SATURATION_COMMAND_ID: {
            uint32_t payloadOffset = cmd->payloadStartIndex;
            uint8_t moveMode;
            uint8_t rate;
            uint8_t optionsMask;
            uint8_t optionsOverride;

            moveMode = (*(uint8_t *) (cmd->buffer + payloadOffset));
            payloadOffset += sizeof(uint8_t);
            rate = (*(uint8_t *) (cmd->buffer + payloadOffset));
            payloadOffset += sizeof(uint8_t);
            optionsMask = (*(uint8_t *) (cmd->buffer + payloadOffset));
            payloadOffset += sizeof(uint8_t);
            optionsOverride = (*(uint8_t *) (cmd->buffer + payloadOffset));

            wasHandled = emberAfColorControlClusterMoveSaturationCallback(moveMode, rate, optionsMask, optionsOverride);
            break;
        }
        case ZCL_MOVE_TO_COLOR_COMMAND_ID: {
            uint32_t payloadOffset = cmd->payloadStartIndex;
            uint16_t colorX;
            uint16_t colorY;
            uint16_t transitionTime;
            uint8_t optionsMask;
            uint8_t optionsOverride;

            colorX = (*(uint16_t *) (cmd->buffer + payloadOffset));
            payloadOffset += sizeof(uint16_t);
            colorY = (*(uint16_t *) (cmd->buffer + payloadOffset));
            payloadOffset += sizeof(uint16_t);
            transitionTime = (*(uint16_t *) (cmd->buffer + payloadOffset));
            payloadOffset += sizeof(uint16_t);
            optionsMask = (*(uint8_t *) (cmd->buffer + payloadOffset));
            payloadOffset += sizeof(uint8_t);
            optionsOverride = (*(uint8_t *) (cmd->buffer + payloadOffset));

            wasHandled =
                emberAfColorControlClusterMoveToColorCallback(colorX, colorY, transitionTime, optionsMask, optionsOverride);
            break;
        }
        case ZCL_MOVE_TO_COLOR_TEMPERATURE_COMMAND_ID: {
            uint32_t payloadOffset = cmd->payloadStartIndex;
            uint16_t colorTemperature;
            uint16_t transitionTime;
            uint8_t optionsMask;
            uint8_t optionsOverride;

            colorTemperature = (*(uint16_t *) (cmd->buffer + payloadOffset));
            payloadOffset += sizeof(uint16_t);
            transitionTime = (*(uint16_t *) (cmd->buffer + payloadOffset));
            payloadOffset += sizeof(uint16_t);
            optionsMask = (*(uint8_t *) (cmd->buffer + payloadOffset));
            payloadOffset += sizeof(uint8_t);
            optionsOverride = (*(uint8_t *) (cmd->buffer + payloadOffset));

            wasHandled = emberAfColorControlClusterMoveToColorTemperatureCallback(colorTemperature, transitionTime, optionsMask,
                                                                                  optionsOverride);
            break;
        }
        case ZCL_MOVE_TO_HUE_COMMAND_ID: {
            uint32_t payloadOffset = cmd->payloadStartIndex;
            uint8_t hue;
            uint8_t direction;
            uint16_t transitionTime;
            uint8_t optionsMask;
            uint8_t optionsOverride;

            hue = (*(uint8_t *) (cmd->buffer + payloadOffset));
            payloadOffset += sizeof(uint8_t);
            direction = (*(uint8_t *) (cmd->buffer + payloadOffset));
            payloadOffset += sizeof(uint8_t);
            transitionTime = (*(uint16_t *) (cmd->buffer + payloadOffset));
            payloadOffset += sizeof(uint16_t);
            optionsMask = (*(uint8_t *) (cmd->buffer + payloadOffset));
            payloadOffset += sizeof(uint8_t);
            optionsOverride = (*(uint8_t *) (cmd->buffer + payloadOffset));

            wasHandled = emberAfColorControlClusterMoveToHueCallback(hue, direction, transitionTime, optionsMask, optionsOverride);
            break;
        }
        case ZCL_MOVE_TO_HUE_AND_SATURATION_COMMAND_ID: {
            uint32_t payloadOffset = cmd->payloadStartIndex;
            uint8_t hue;
            uint8_t saturation;
            uint16_t transitionTime;
            uint8_t optionsMask;
            uint8_t optionsOverride;

            hue = (*(uint8_t *) (cmd->buffer + payloadOffset));
            payloadOffset += sizeof(uint8_t);
            saturation = (*(uint8_t *) (cmd->buffer + payloadOffset));
            payloadOffset += sizeof(uint8_t);
            transitionTime = (*(uint16_t *) (cmd->buffer + payloadOffset));
            payloadOffset += sizeof(uint16_t);
            optionsMask = (*(uint8_t *) (cmd->buffer + payloadOffset));
            payloadOffset += sizeof(uint8_t);
            optionsOverride = (*(uint8_t *) (cmd->buffer + payloadOffset));

            wasHandled = emberAfColorControlClusterMoveToHueAndSaturationCallback(hue, saturation, transitionTime, optionsMask,
                                                                                  optionsOverride);
            break;
        }
        case ZCL_MOVE_TO_SATURATION_COMMAND_ID: {
            uint32_t payloadOffset = cmd->payloadStartIndex;
            uint8_t saturation;
            uint16_t transitionTime;
            uint8_t optionsMask;
            uint8_t optionsOverride;

            saturation = (*(uint8_t *) (cmd->buffer + payloadOffset));
            payloadOffset += sizeof(uint8_t);
            transitionTime = (*(uint16_t *) (cmd->buffer + payloadOffset));
            payloadOffset += sizeof(uint16_t);
            optionsMask = (*(uint8_t *) (cmd->buffer + payloadOffset));
            payloadOffset += sizeof(uint8_t);
            optionsOverride = (*(uint8_t *) (cmd->buffer + payloadOffset));

            wasHandled =
                emberAfColorControlClusterMoveToSaturationCallback(saturation, transitionTime, optionsMask, optionsOverride);
            break;
        }
        case ZCL_STEP_COLOR_COMMAND_ID: {
            uint32_t payloadOffset = cmd->payloadStartIndex;
            int16_t stepX;
            int16_t stepY;
            uint16_t transitionTime;
            uint8_t optionsMask;
            uint8_t optionsOverride;

            stepX = (*(int16_t *) (cmd->buffer + payloadOffset));
            payloadOffset += sizeof(int16_t);
            stepY = (*(int16_t *) (cmd->buffer + payloadOffset));
            payloadOffset += sizeof(int16_t);
            transitionTime = (*(uint16_t *) (cmd->buffer + payloadOffset));
            payloadOffset += sizeof(uint16_t);
            optionsMask = (*(uint8_t *) (cmd->buffer + payloadOffset));
            payloadOffset += sizeof(uint8_t);
            optionsOverride = (*(uint8_t *) (cmd->buffer + payloadOffset));

            wasHandled = emberAfColorControlClusterStepColorCallback(stepX, stepY, transitionTime, optionsMask, optionsOverride);
            break;
        }
        case ZCL_STEP_COLOR_TEMPERATURE_COMMAND_ID: {
            uint32_t payloadOffset = cmd->payloadStartIndex;
            uint8_t stepMode;
            uint16_t stepSize;
            uint16_t transitionTime;
            uint16_t colorTemperatureMinimum;
            uint16_t colorTemperatureMaximum;
            uint8_t optionsMask;
            uint8_t optionsOverride;

            stepMode = (*(uint8_t *) (cmd->buffer + payloadOffset));
            payloadOffset += sizeof(uint8_t);
            stepSize = (*(uint16_t *) (cmd->buffer + payloadOffset));
            payloadOffset += sizeof(uint16_t);
            transitionTime = (*(uint16_t *) (cmd->buffer + payloadOffset));
            payloadOffset += sizeof(uint16_t);
            colorTemperatureMinimum = (*(uint16_t *) (cmd->buffer + payloadOffset));
            payloadOffset += sizeof(uint16_t);
            colorTemperatureMaximum = (*(uint16_t *) (cmd->buffer + payloadOffset));
            payloadOffset += sizeof(uint16_t);
            optionsMask = (*(uint8_t *) (cmd->buffer + payloadOffset));
            payloadOffset += sizeof(uint8_t);
            optionsOverride = (*(uint8_t *) (cmd->buffer + payloadOffset));

            wasHandled = emberAfColorControlClusterStepColorTemperatureCallback(
                stepMode, stepSize, transitionTime, colorTemperatureMinimum, colorTemperatureMaximum, optionsMask, optionsOverride);
            break;
        }
        case ZCL_STEP_HUE_COMMAND_ID: {
            uint32_t payloadOffset = cmd->payloadStartIndex;
            uint8_t stepMode;
            uint8_t stepSize;
            uint8_t transitionTime;
            uint8_t optionsMask;
            uint8_t optionsOverride;

            stepMode = (*(uint8_t *) (cmd->buffer + payloadOffset));
            payloadOffset += sizeof(uint8_t);
            stepSize = (*(uint8_t *) (cmd->buffer + payloadOffset));
            payloadOffset += sizeof(uint8_t);
            transitionTime = (*(uint8_t *) (cmd->buffer + payloadOffset));
            payloadOffset += sizeof(uint8_t);
            optionsMask = (*(uint8_t *) (cmd->buffer + payloadOffset));
            payloadOffset += sizeof(uint8_t);
            optionsOverride = (*(uint8_t *) (cmd->buffer + payloadOffset));

            wasHandled =
                emberAfColorControlClusterStepHueCallback(stepMode, stepSize, transitionTime, optionsMask, optionsOverride);
            break;
        }
        case ZCL_STEP_SATURATION_COMMAND_ID: {
            uint32_t payloadOffset = cmd->payloadStartIndex;
            uint8_t stepMode;
            uint8_t stepSize;
            uint8_t transitionTime;
            uint8_t optionsMask;
            uint8_t optionsOverride;

            stepMode = (*(uint8_t *) (cmd->buffer + payloadOffset));
            payloadOffset += sizeof(uint8_t);
            stepSize = (*(uint8_t *) (cmd->buffer + payloadOffset));
            payloadOffset += sizeof(uint8_t);
            transitionTime = (*(uint8_t *) (cmd->buffer + payloadOffset));
            payloadOffset += sizeof(uint8_t);
            optionsMask = (*(uint8_t *) (cmd->buffer + payloadOffset));
            payloadOffset += sizeof(uint8_t);
            optionsOverride = (*(uint8_t *) (cmd->buffer + payloadOffset));

            wasHandled =
                emberAfColorControlClusterStepSaturationCallback(stepMode, stepSize, transitionTime, optionsMask, optionsOverride);
            break;
        }
        case ZCL_STOP_MOVE_STEP_COMMAND_ID: {
            uint32_t payloadOffset = cmd->payloadStartIndex;
            uint8_t optionsMask;
            uint8_t optionsOverride;

            optionsMask = (*(uint8_t *) (cmd->buffer + payloadOffset));
            payloadOffset += sizeof(uint8_t);
            optionsOverride = (*(uint8_t *) (cmd->buffer + payloadOffset));

            wasHandled = emberAfColorControlClusterStopMoveStepCallback(optionsMask, optionsOverride);
            break;
        }
        default: {
            // Unrecognized command ID, error status will apply.
            break;
        }
        }
    }
    return status(wasHandled, true, cmd->mfgSpecific);
}
EmberAfStatus emberAfDoorLockClusterClientCommandParse(EmberAfClusterCommand * cmd)
{
    bool wasHandled = false;

    if (!cmd->mfgSpecific)
    {
        switch (cmd->commandId)
        {
        case ZCL_LOCK_DOOR_RESPONSE_COMMAND_ID: {
            uint32_t payloadOffset = cmd->payloadStartIndex;
            uint8_t status;

            status = (*(uint8_t *) (cmd->buffer + payloadOffset));

            wasHandled = emberAfDoorLockClusterLockDoorResponseCallback(status);
            break;
        }
        case ZCL_UNLOCK_DOOR_RESPONSE_COMMAND_ID: {
            uint32_t payloadOffset = cmd->payloadStartIndex;
            uint8_t status;

            status = (*(uint8_t *) (cmd->buffer + payloadOffset));

            wasHandled = emberAfDoorLockClusterUnlockDoorResponseCallback(status);
            break;
        }
        default: {
            // Unrecognized command ID, error status will apply.
            break;
        }
        }
    }
    return status(wasHandled, true, cmd->mfgSpecific);
}
EmberAfStatus emberAfDoorLockClusterServerCommandParse(EmberAfClusterCommand * cmd)
{
    bool wasHandled = false;

    if (!cmd->mfgSpecific)
    {
        switch (cmd->commandId)
        {
        case ZCL_CLEAR_ALL_PINS_COMMAND_ID: {
            wasHandled = emberAfDoorLockClusterClearAllPinsCallback();
            break;
        }
        case ZCL_CLEAR_ALL_RFIDS_COMMAND_ID: {
            wasHandled = emberAfDoorLockClusterClearAllRfidsCallback();
            break;
        }
        case ZCL_CLEAR_HOLIDAY_SCHEDULE_COMMAND_ID: {
            uint32_t payloadOffset = cmd->payloadStartIndex;
            uint8_t scheduleId;

            scheduleId = (*(uint8_t *) (cmd->buffer + payloadOffset));

            wasHandled = emberAfDoorLockClusterClearHolidayScheduleCallback(scheduleId);
            break;
        }
        case ZCL_CLEAR_PIN_COMMAND_ID: {
            uint32_t payloadOffset = cmd->payloadStartIndex;
            uint16_t userId;

            userId = (*(uint16_t *) (cmd->buffer + payloadOffset));

            wasHandled = emberAfDoorLockClusterClearPinCallback(userId);
            break;
        }
        case ZCL_CLEAR_RFID_COMMAND_ID: {
            uint32_t payloadOffset = cmd->payloadStartIndex;
            uint16_t userId;

            userId = (*(uint16_t *) (cmd->buffer + payloadOffset));

            wasHandled = emberAfDoorLockClusterClearRfidCallback(userId);
            break;
        }
        case ZCL_CLEAR_WEEKDAY_SCHEDULE_COMMAND_ID: {
            uint32_t payloadOffset = cmd->payloadStartIndex;
            uint8_t scheduleId;
            uint16_t userId;

            scheduleId = (*(uint8_t *) (cmd->buffer + payloadOffset));
            payloadOffset += sizeof(uint8_t);
            userId = (*(uint16_t *) (cmd->buffer + payloadOffset));

            wasHandled = emberAfDoorLockClusterClearWeekdayScheduleCallback(scheduleId, userId);
            break;
        }
        case ZCL_CLEAR_YEARDAY_SCHEDULE_COMMAND_ID: {
            uint32_t payloadOffset = cmd->payloadStartIndex;
            uint8_t scheduleId;
            uint16_t userId;

            scheduleId = (*(uint8_t *) (cmd->buffer + payloadOffset));
            payloadOffset += sizeof(uint8_t);
            userId = (*(uint16_t *) (cmd->buffer + payloadOffset));

            wasHandled = emberAfDoorLockClusterClearYeardayScheduleCallback(scheduleId, userId);
            break;
        }
        case ZCL_GET_HOLIDAY_SCHEDULE_COMMAND_ID: {
            uint32_t payloadOffset = cmd->payloadStartIndex;
            uint8_t scheduleId;

            scheduleId = (*(uint8_t *) (cmd->buffer + payloadOffset));

            wasHandled = emberAfDoorLockClusterGetHolidayScheduleCallback(scheduleId);
            break;
        }
        case ZCL_GET_LOG_RECORD_COMMAND_ID: {
            uint32_t payloadOffset = cmd->payloadStartIndex;
            uint16_t logIndex;

            logIndex = (*(uint16_t *) (cmd->buffer + payloadOffset));

            wasHandled = emberAfDoorLockClusterGetLogRecordCallback(logIndex);
            break;
        }
        case ZCL_GET_PIN_COMMAND_ID: {
            uint32_t payloadOffset = cmd->payloadStartIndex;
            uint16_t userId;

            userId = (*(uint16_t *) (cmd->buffer + payloadOffset));

            wasHandled = emberAfDoorLockClusterGetPinCallback(userId);
            break;
        }
        case ZCL_GET_RFID_COMMAND_ID: {
            uint32_t payloadOffset = cmd->payloadStartIndex;
            uint16_t userId;

            userId = (*(uint16_t *) (cmd->buffer + payloadOffset));

            wasHandled = emberAfDoorLockClusterGetRfidCallback(userId);
            break;
        }
        case ZCL_GET_USER_TYPE_COMMAND_ID: {
            uint32_t payloadOffset = cmd->payloadStartIndex;
            uint16_t userId;

            userId = (*(uint16_t *) (cmd->buffer + payloadOffset));

            wasHandled = emberAfDoorLockClusterGetUserTypeCallback(userId);
            break;
        }
        case ZCL_GET_WEEKDAY_SCHEDULE_COMMAND_ID: {
            uint32_t payloadOffset = cmd->payloadStartIndex;
            uint8_t scheduleId;
            uint16_t userId;

            scheduleId = (*(uint8_t *) (cmd->buffer + payloadOffset));
            payloadOffset += sizeof(uint8_t);
            userId = (*(uint16_t *) (cmd->buffer + payloadOffset));

            wasHandled = emberAfDoorLockClusterGetWeekdayScheduleCallback(scheduleId, userId);
            break;
        }
        case ZCL_GET_YEARDAY_SCHEDULE_COMMAND_ID: {
            uint32_t payloadOffset = cmd->payloadStartIndex;
            uint8_t scheduleId;
            uint16_t userId;

            scheduleId = (*(uint8_t *) (cmd->buffer + payloadOffset));
            payloadOffset += sizeof(uint8_t);
            userId = (*(uint16_t *) (cmd->buffer + payloadOffset));

            wasHandled = emberAfDoorLockClusterGetYeardayScheduleCallback(scheduleId, userId);
            break;
        }
        case ZCL_LOCK_DOOR_COMMAND_ID: {
            uint32_t payloadOffset = cmd->payloadStartIndex;
            uint8_t * PIN;

            PIN = emberAfGetString(cmd->buffer, payloadOffset, cmd->bufLen);

            wasHandled = emberAfDoorLockClusterLockDoorCallback(PIN);
            break;
        }
        case ZCL_SET_HOLIDAY_SCHEDULE_COMMAND_ID: {
            uint32_t payloadOffset = cmd->payloadStartIndex;
            uint8_t scheduleId;
            uint32_t localStartTime;
            uint32_t localEndTime;
            uint8_t operatingModeDuringHoliday;

            scheduleId = (*(uint8_t *) (cmd->buffer + payloadOffset));
            payloadOffset += sizeof(uint8_t);
            localStartTime = (*(uint32_t *) (cmd->buffer + payloadOffset));
            payloadOffset += sizeof(uint32_t);
            localEndTime = (*(uint32_t *) (cmd->buffer + payloadOffset));
            payloadOffset += sizeof(uint32_t);
            operatingModeDuringHoliday = (*(uint8_t *) (cmd->buffer + payloadOffset));

            wasHandled = emberAfDoorLockClusterSetHolidayScheduleCallback(scheduleId, localStartTime, localEndTime,
                                                                          operatingModeDuringHoliday);
            break;
        }
        case ZCL_SET_PIN_COMMAND_ID: {
            uint32_t payloadOffset = cmd->payloadStartIndex;
            uint16_t userId;
            uint8_t userStatus;
            uint8_t userType;
            uint8_t * pin;

            userId = (*(uint16_t *) (cmd->buffer + payloadOffset));
            payloadOffset += sizeof(uint16_t);
            userStatus = (*(uint8_t *) (cmd->buffer + payloadOffset));
            payloadOffset += sizeof(uint8_t);
            userType = (*(uint8_t *) (cmd->buffer + payloadOffset));
            payloadOffset += sizeof(uint8_t);
            pin = emberAfGetString(cmd->buffer, payloadOffset, cmd->bufLen);

            wasHandled = emberAfDoorLockClusterSetPinCallback(userId, userStatus, userType, pin);
            break;
        }
        case ZCL_SET_RFID_COMMAND_ID: {
            uint32_t payloadOffset = cmd->payloadStartIndex;
            uint16_t userId;
            uint8_t userStatus;
            uint8_t userType;
            uint8_t * id;

            userId = (*(uint16_t *) (cmd->buffer + payloadOffset));
            payloadOffset += sizeof(uint16_t);
            userStatus = (*(uint8_t *) (cmd->buffer + payloadOffset));
            payloadOffset += sizeof(uint8_t);
            userType = (*(uint8_t *) (cmd->buffer + payloadOffset));
            payloadOffset += sizeof(uint8_t);
            id = emberAfGetString(cmd->buffer, payloadOffset, cmd->bufLen);

            wasHandled = emberAfDoorLockClusterSetRfidCallback(userId, userStatus, userType, id);
            break;
        }
        case ZCL_SET_USER_TYPE_COMMAND_ID: {
            uint32_t payloadOffset = cmd->payloadStartIndex;
            uint16_t userId;
            uint8_t userType;

            userId = (*(uint16_t *) (cmd->buffer + payloadOffset));
            payloadOffset += sizeof(uint16_t);
            userType = (*(uint8_t *) (cmd->buffer + payloadOffset));

            wasHandled = emberAfDoorLockClusterSetUserTypeCallback(userId, userType);
            break;
        }
        case ZCL_SET_WEEKDAY_SCHEDULE_COMMAND_ID: {
            uint32_t payloadOffset = cmd->payloadStartIndex;
            uint8_t scheduleId;
            uint16_t userId;
            uint8_t daysMask;
            uint8_t startHour;
            uint8_t startMinute;
            uint8_t endHour;
            uint8_t endMinute;

            scheduleId = (*(uint8_t *) (cmd->buffer + payloadOffset));
            payloadOffset += sizeof(uint8_t);
            userId = (*(uint16_t *) (cmd->buffer + payloadOffset));
            payloadOffset += sizeof(uint16_t);
            daysMask = (*(uint8_t *) (cmd->buffer + payloadOffset));
            payloadOffset += sizeof(uint8_t);
            startHour = (*(uint8_t *) (cmd->buffer + payloadOffset));
            payloadOffset += sizeof(uint8_t);
            startMinute = (*(uint8_t *) (cmd->buffer + payloadOffset));
            payloadOffset += sizeof(uint8_t);
            endHour = (*(uint8_t *) (cmd->buffer + payloadOffset));
            payloadOffset += sizeof(uint8_t);
            endMinute = (*(uint8_t *) (cmd->buffer + payloadOffset));

            wasHandled = emberAfDoorLockClusterSetWeekdayScheduleCallback(scheduleId, userId, daysMask, startHour, startMinute,
                                                                          endHour, endMinute);
            break;
        }
        case ZCL_SET_YEARDAY_SCHEDULE_COMMAND_ID: {
            uint32_t payloadOffset = cmd->payloadStartIndex;
            uint8_t scheduleId;
            uint16_t userId;
            uint32_t localStartTime;
            uint32_t localEndTime;

            scheduleId = (*(uint8_t *) (cmd->buffer + payloadOffset));
            payloadOffset += sizeof(uint8_t);
            userId = (*(uint16_t *) (cmd->buffer + payloadOffset));
            payloadOffset += sizeof(uint16_t);
            localStartTime = (*(uint32_t *) (cmd->buffer + payloadOffset));
            payloadOffset += sizeof(uint32_t);
            localEndTime = (*(uint32_t *) (cmd->buffer + payloadOffset));

            wasHandled = emberAfDoorLockClusterSetYeardayScheduleCallback(scheduleId, userId, localStartTime, localEndTime);
            break;
        }
        case ZCL_UNLOCK_DOOR_COMMAND_ID: {
            uint32_t payloadOffset = cmd->payloadStartIndex;
            uint8_t * PIN;

            PIN = emberAfGetString(cmd->buffer, payloadOffset, cmd->bufLen);

            wasHandled = emberAfDoorLockClusterUnlockDoorCallback(PIN);
            break;
        }
        case ZCL_UNLOCK_WITH_TIMEOUT_COMMAND_ID: {
            uint32_t payloadOffset = cmd->payloadStartIndex;
            uint16_t timeoutInSeconds;
            uint8_t * pin;

            timeoutInSeconds = (*(uint16_t *) (cmd->buffer + payloadOffset));
            payloadOffset += sizeof(uint16_t);
            pin = emberAfGetString(cmd->buffer, payloadOffset, cmd->bufLen);

            wasHandled = emberAfDoorLockClusterUnlockWithTimeoutCallback(timeoutInSeconds, pin);
            break;
        }
        default: {
            // Unrecognized command ID, error status will apply.
            break;
        }
        }
    }
    return status(wasHandled, true, cmd->mfgSpecific);
}
EmberAfStatus emberAfGroupsClusterClientCommandParse(EmberAfClusterCommand * cmd)
{
    bool wasHandled = false;

    if (!cmd->mfgSpecific)
    {
        switch (cmd->commandId)
        {
        case ZCL_ADD_GROUP_RESPONSE_COMMAND_ID: {
            uint32_t payloadOffset = cmd->payloadStartIndex;
            uint8_t status;
            GroupId groupId;

            status = (*(uint8_t *) (cmd->buffer + payloadOffset));
            payloadOffset += sizeof(uint8_t);
            groupId = (*(GroupId *) (cmd->buffer + payloadOffset));

            wasHandled = emberAfGroupsClusterAddGroupResponseCallback(status, groupId);
            break;
        }
        case ZCL_GET_GROUP_MEMBERSHIP_RESPONSE_COMMAND_ID: {
            uint32_t payloadOffset = cmd->payloadStartIndex;
            uint8_t capacity;
            uint8_t groupCount;
            uint8_t * groupList;

            capacity = (*(uint8_t *) (cmd->buffer + payloadOffset));
            payloadOffset += sizeof(uint8_t);
            groupCount = (*(uint8_t *) (cmd->buffer + payloadOffset));
            payloadOffset += sizeof(uint8_t);
            groupList = (uint8_t *) (cmd->buffer + payloadOffset);

            wasHandled = emberAfGroupsClusterGetGroupMembershipResponseCallback(capacity, groupCount, groupList);
            break;
        }
        case ZCL_REMOVE_GROUP_RESPONSE_COMMAND_ID: {
            uint32_t payloadOffset = cmd->payloadStartIndex;
            uint8_t status;
            GroupId groupId;

            status = (*(uint8_t *) (cmd->buffer + payloadOffset));
            payloadOffset += sizeof(uint8_t);
            groupId = (*(GroupId *) (cmd->buffer + payloadOffset));

            wasHandled = emberAfGroupsClusterRemoveGroupResponseCallback(status, groupId);
            break;
        }
        case ZCL_VIEW_GROUP_RESPONSE_COMMAND_ID: {
            uint32_t payloadOffset = cmd->payloadStartIndex;
            uint8_t status;
            GroupId groupId;
            uint8_t * groupName;

            status = (*(uint8_t *) (cmd->buffer + payloadOffset));
            payloadOffset += sizeof(uint8_t);
            groupId = (*(GroupId *) (cmd->buffer + payloadOffset));
            payloadOffset += sizeof(uint16_t);
            groupName = emberAfGetString(cmd->buffer, payloadOffset, cmd->bufLen);

            wasHandled = emberAfGroupsClusterViewGroupResponseCallback(status, groupId, groupName);
            break;
        }
        default: {
            // Unrecognized command ID, error status will apply.
            break;
        }
        }
    }
    return status(wasHandled, true, cmd->mfgSpecific);
}
EmberAfStatus emberAfGroupsClusterServerCommandParse(EmberAfClusterCommand * cmd)
{
    bool wasHandled = false;

    if (!cmd->mfgSpecific)
    {
        switch (cmd->commandId)
        {
        case ZCL_ADD_GROUP_COMMAND_ID: {
            uint32_t payloadOffset = cmd->payloadStartIndex;
            GroupId groupId;
            uint8_t * groupName;

            groupId = (*(GroupId *) (cmd->buffer + payloadOffset));
            payloadOffset += sizeof(uint16_t);
            groupName = emberAfGetString(cmd->buffer, payloadOffset, cmd->bufLen);

            wasHandled = emberAfGroupsClusterAddGroupCallback(groupId, groupName);
            break;
        }
        case ZCL_ADD_GROUP_IF_IDENTIFYING_COMMAND_ID: {
            uint32_t payloadOffset = cmd->payloadStartIndex;
            GroupId groupId;
            uint8_t * groupName;

            groupId = (*(GroupId *) (cmd->buffer + payloadOffset));
            payloadOffset += sizeof(uint16_t);
            groupName = emberAfGetString(cmd->buffer, payloadOffset, cmd->bufLen);

            wasHandled = emberAfGroupsClusterAddGroupIfIdentifyingCallback(groupId, groupName);
            break;
        }
        case ZCL_GET_GROUP_MEMBERSHIP_COMMAND_ID: {
            uint32_t payloadOffset = cmd->payloadStartIndex;
            uint8_t groupCount;
            uint8_t * groupList;

            groupCount = (*(uint8_t *) (cmd->buffer + payloadOffset));
            payloadOffset += sizeof(uint8_t);
            groupList = (uint8_t *) (cmd->buffer + payloadOffset);

            wasHandled = emberAfGroupsClusterGetGroupMembershipCallback(groupCount, groupList);
            break;
        }
        case ZCL_REMOVE_ALL_GROUPS_COMMAND_ID: {
            wasHandled = emberAfGroupsClusterRemoveAllGroupsCallback();
            break;
        }
        case ZCL_REMOVE_GROUP_COMMAND_ID: {
            uint32_t payloadOffset = cmd->payloadStartIndex;
            GroupId groupId;

            groupId = (*(GroupId *) (cmd->buffer + payloadOffset));

            wasHandled = emberAfGroupsClusterRemoveGroupCallback(groupId);
            break;
        }
        case ZCL_VIEW_GROUP_COMMAND_ID: {
            uint32_t payloadOffset = cmd->payloadStartIndex;
            GroupId groupId;

            groupId = (*(GroupId *) (cmd->buffer + payloadOffset));

            wasHandled = emberAfGroupsClusterViewGroupCallback(groupId);
            break;
        }
        default: {
            // Unrecognized command ID, error status will apply.
            break;
        }
        }
    }
    return status(wasHandled, true, cmd->mfgSpecific);
}
EmberAfStatus emberAfIasZoneClusterServerCommandParse(EmberAfClusterCommand * cmd)
{
    bool wasHandled = false;

    if (!cmd->mfgSpecific)
    {
        switch (cmd->commandId)
        {
        case ZCL_ZONE_ENROLL_RESPONSE_COMMAND_ID: {
            uint32_t payloadOffset = cmd->payloadStartIndex;
            uint8_t enrollResponseCode;
            uint8_t zoneId;

            enrollResponseCode = (*(uint8_t *) (cmd->buffer + payloadOffset));
            payloadOffset += sizeof(uint8_t);
            zoneId = (*(uint8_t *) (cmd->buffer + payloadOffset));

            wasHandled = emberAfIasZoneClusterZoneEnrollResponseCallback(enrollResponseCode, zoneId);
            break;
        }
        default: {
            // Unrecognized command ID, error status will apply.
            break;
        }
        }
    }
    return status(wasHandled, true, cmd->mfgSpecific);
}
EmberAfStatus emberAfIdentifyClusterClientCommandParse(EmberAfClusterCommand * cmd)
{
    bool wasHandled = false;

    if (!cmd->mfgSpecific)
    {
        switch (cmd->commandId)
        {
        case ZCL_IDENTIFY_QUERY_RESPONSE_COMMAND_ID: {
            uint32_t payloadOffset = cmd->payloadStartIndex;
            uint16_t timeout;

            timeout = (*(uint16_t *) (cmd->buffer + payloadOffset));

            wasHandled = emberAfIdentifyClusterIdentifyQueryResponseCallback(timeout);
            break;
        }
        default: {
            // Unrecognized command ID, error status will apply.
            break;
        }
        }
    }
    return status(wasHandled, true, cmd->mfgSpecific);
}
EmberAfStatus emberAfIdentifyClusterServerCommandParse(EmberAfClusterCommand * cmd)
{
    bool wasHandled = false;

    if (!cmd->mfgSpecific)
    {
        switch (cmd->commandId)
        {
        case ZCL_IDENTIFY_COMMAND_ID: {
            uint32_t payloadOffset = cmd->payloadStartIndex;
            uint16_t identifyTime;

            identifyTime = (*(uint16_t *) (cmd->buffer + payloadOffset));

            wasHandled = emberAfIdentifyClusterIdentifyCallback(identifyTime);
            break;
        }
        case ZCL_IDENTIFY_QUERY_COMMAND_ID: {
            wasHandled = emberAfIdentifyClusterIdentifyQueryCallback();
            break;
        }
        default: {
            // Unrecognized command ID, error status will apply.
            break;
        }
        }
    }
    return status(wasHandled, true, cmd->mfgSpecific);
}
EmberAfStatus emberAfLevelControlClusterClientCommandParse(EmberAfClusterCommand * cmd)
{
    bool wasHandled = false;

    if (!cmd->mfgSpecific)
    {
        switch (cmd->commandId)
        {
        default: {
            // Unrecognized command ID, error status will apply.
            break;
        }
        }
    }
    return status(wasHandled, true, cmd->mfgSpecific);
}
EmberAfStatus emberAfLevelControlClusterServerCommandParse(EmberAfClusterCommand * cmd)
{
    bool wasHandled = false;

    if (!cmd->mfgSpecific)
    {
        switch (cmd->commandId)
        {
        case ZCL_MOVE_COMMAND_ID: {
            uint32_t payloadOffset = cmd->payloadStartIndex;
            uint8_t moveMode;
            uint8_t rate;
            uint8_t optionMask;
            uint8_t optionOverride;

            moveMode = (*(uint8_t *) (cmd->buffer + payloadOffset));
            payloadOffset += sizeof(uint8_t);
            rate = (*(uint8_t *) (cmd->buffer + payloadOffset));
            payloadOffset += sizeof(uint8_t);
            optionMask = (*(uint8_t *) (cmd->buffer + payloadOffset));
            payloadOffset += sizeof(uint8_t);
            optionOverride = (*(uint8_t *) (cmd->buffer + payloadOffset));

            wasHandled = emberAfLevelControlClusterMoveCallback(moveMode, rate, optionMask, optionOverride);
            break;
        }
        case ZCL_MOVE_TO_LEVEL_COMMAND_ID: {
            uint32_t payloadOffset = cmd->payloadStartIndex;
            uint8_t level;
            uint16_t transitionTime;
            uint8_t optionMask;
            uint8_t optionOverride;

            level = (*(uint8_t *) (cmd->buffer + payloadOffset));
            payloadOffset += sizeof(uint8_t);
            transitionTime = (*(uint16_t *) (cmd->buffer + payloadOffset));
            payloadOffset += sizeof(uint16_t);
            optionMask = (*(uint8_t *) (cmd->buffer + payloadOffset));
            payloadOffset += sizeof(uint8_t);
            optionOverride = (*(uint8_t *) (cmd->buffer + payloadOffset));

            wasHandled = emberAfLevelControlClusterMoveToLevelCallback(level, transitionTime, optionMask, optionOverride);
            break;
        }
        case ZCL_MOVE_TO_LEVEL_WITH_ON_OFF_COMMAND_ID: {
            uint32_t payloadOffset = cmd->payloadStartIndex;
            uint8_t level;
            uint16_t transitionTime;

            level = (*(uint8_t *) (cmd->buffer + payloadOffset));
            payloadOffset += sizeof(uint8_t);
            transitionTime = (*(uint16_t *) (cmd->buffer + payloadOffset));

            wasHandled = emberAfLevelControlClusterMoveToLevelWithOnOffCallback(level, transitionTime);
            break;
        }
        case ZCL_MOVE_WITH_ON_OFF_COMMAND_ID: {
            uint32_t payloadOffset = cmd->payloadStartIndex;
            uint8_t moveMode;
            uint8_t rate;

            moveMode = (*(uint8_t *) (cmd->buffer + payloadOffset));
            payloadOffset += sizeof(uint8_t);
            rate = (*(uint8_t *) (cmd->buffer + payloadOffset));

            wasHandled = emberAfLevelControlClusterMoveWithOnOffCallback(moveMode, rate);
            break;
        }
        case ZCL_STEP_COMMAND_ID: {
            uint32_t payloadOffset = cmd->payloadStartIndex;
            uint8_t stepMode;
            uint8_t stepSize;
            uint16_t transitionTime;
            uint8_t optionMask;
            uint8_t optionOverride;

            stepMode = (*(uint8_t *) (cmd->buffer + payloadOffset));
            payloadOffset += sizeof(uint8_t);
            stepSize = (*(uint8_t *) (cmd->buffer + payloadOffset));
            payloadOffset += sizeof(uint8_t);
            transitionTime = (*(uint16_t *) (cmd->buffer + payloadOffset));
            payloadOffset += sizeof(uint16_t);
            optionMask = (*(uint8_t *) (cmd->buffer + payloadOffset));
            payloadOffset += sizeof(uint8_t);
            optionOverride = (*(uint8_t *) (cmd->buffer + payloadOffset));

            wasHandled = emberAfLevelControlClusterStepCallback(stepMode, stepSize, transitionTime, optionMask, optionOverride);
            break;
        }
        case ZCL_STEP_WITH_ON_OFF_COMMAND_ID: {
            uint32_t payloadOffset = cmd->payloadStartIndex;
            uint8_t stepMode;
            uint8_t stepSize;
            uint16_t transitionTime;

            stepMode = (*(uint8_t *) (cmd->buffer + payloadOffset));
            payloadOffset += sizeof(uint8_t);
            stepSize = (*(uint8_t *) (cmd->buffer + payloadOffset));
            payloadOffset += sizeof(uint8_t);
            transitionTime = (*(uint16_t *) (cmd->buffer + payloadOffset));

            wasHandled = emberAfLevelControlClusterStepWithOnOffCallback(stepMode, stepSize, transitionTime);
            break;
        }
        case ZCL_STOP_COMMAND_ID: {
            uint32_t payloadOffset = cmd->payloadStartIndex;
            uint8_t optionMask;
            uint8_t optionOverride;

            optionMask = (*(uint8_t *) (cmd->buffer + payloadOffset));
            payloadOffset += sizeof(uint8_t);
            optionOverride = (*(uint8_t *) (cmd->buffer + payloadOffset));

            wasHandled = emberAfLevelControlClusterStopCallback(optionMask, optionOverride);
            break;
        }
        case ZCL_STOP_WITH_ON_OFF_COMMAND_ID: {
            wasHandled = emberAfLevelControlClusterStopWithOnOffCallback();
            break;
        }
        default: {
            // Unrecognized command ID, error status will apply.
            break;
        }
        }
    }
    return status(wasHandled, true, cmd->mfgSpecific);
}
EmberAfStatus emberAfOnOffClusterClientCommandParse(EmberAfClusterCommand * cmd)
{
    bool wasHandled = false;

    if (!cmd->mfgSpecific)
    {
        switch (cmd->commandId)
        {
        default: {
            // Unrecognized command ID, error status will apply.
            break;
        }
        }
    }
    return status(wasHandled, true, cmd->mfgSpecific);
}
EmberAfStatus emberAfOnOffClusterServerCommandParse(EmberAfClusterCommand * cmd)
{
    bool wasHandled = false;

    if (!cmd->mfgSpecific)
    {
        switch (cmd->commandId)
        {
        case ZCL_OFF_COMMAND_ID: {
            wasHandled = emberAfOnOffClusterOffCallback();
            break;
        }
        case ZCL_ON_COMMAND_ID: {
            wasHandled = emberAfOnOffClusterOnCallback();
            break;
        }
        case ZCL_TOGGLE_COMMAND_ID: {
            wasHandled = emberAfOnOffClusterToggleCallback();
            break;
        }
        default: {
            // Unrecognized command ID, error status will apply.
            break;
        }
        }
    }
    return status(wasHandled, true, cmd->mfgSpecific);
}
EmberAfStatus emberAfScenesClusterClientCommandParse(EmberAfClusterCommand * cmd)
{
    bool wasHandled = false;

    if (!cmd->mfgSpecific)
    {
        switch (cmd->commandId)
        {
        case ZCL_ADD_SCENE_RESPONSE_COMMAND_ID: {
            uint32_t payloadOffset = cmd->payloadStartIndex;
            uint8_t status;
            GroupId groupId;
            uint8_t sceneId;

            status = (*(uint8_t *) (cmd->buffer + payloadOffset));
            payloadOffset += sizeof(uint8_t);
            groupId = (*(GroupId *) (cmd->buffer + payloadOffset));
            payloadOffset += sizeof(uint16_t);
            sceneId = (*(uint8_t *) (cmd->buffer + payloadOffset));

            wasHandled = emberAfScenesClusterAddSceneResponseCallback(status, groupId, sceneId);
            break;
        }
        case ZCL_GET_SCENE_MEMBERSHIP_RESPONSE_COMMAND_ID: {
            uint32_t payloadOffset = cmd->payloadStartIndex;
            uint8_t status;
            uint8_t capacity;
            GroupId groupId;
            uint8_t sceneCount;
            uint8_t * sceneList;

            status = (*(uint8_t *) (cmd->buffer + payloadOffset));
            payloadOffset += sizeof(uint8_t);
            capacity = (*(uint8_t *) (cmd->buffer + payloadOffset));
            payloadOffset += sizeof(uint8_t);
            groupId = (*(GroupId *) (cmd->buffer + payloadOffset));
            payloadOffset += sizeof(uint16_t);
            sceneCount = (*(uint8_t *) (cmd->buffer + payloadOffset));
            payloadOffset += sizeof(uint8_t);
            sceneList = (uint8_t *) (cmd->buffer + payloadOffset);

            wasHandled = emberAfScenesClusterGetSceneMembershipResponseCallback(status, capacity, groupId, sceneCount, sceneList);
            break;
        }
        case ZCL_REMOVE_ALL_SCENES_RESPONSE_COMMAND_ID: {
            uint32_t payloadOffset = cmd->payloadStartIndex;
            uint8_t status;
            GroupId groupId;

            status = (*(uint8_t *) (cmd->buffer + payloadOffset));
            payloadOffset += sizeof(uint8_t);
            groupId = (*(GroupId *) (cmd->buffer + payloadOffset));

            wasHandled = emberAfScenesClusterRemoveAllScenesResponseCallback(status, groupId);
            break;
        }
        case ZCL_REMOVE_SCENE_RESPONSE_COMMAND_ID: {
            uint32_t payloadOffset = cmd->payloadStartIndex;
            uint8_t status;
            GroupId groupId;
            uint8_t sceneId;

            status = (*(uint8_t *) (cmd->buffer + payloadOffset));
            payloadOffset += sizeof(uint8_t);
            groupId = (*(GroupId *) (cmd->buffer + payloadOffset));
            payloadOffset += sizeof(uint16_t);
            sceneId = (*(uint8_t *) (cmd->buffer + payloadOffset));

            wasHandled = emberAfScenesClusterRemoveSceneResponseCallback(status, groupId, sceneId);
            break;
        }
        case ZCL_STORE_SCENE_RESPONSE_COMMAND_ID: {
            uint32_t payloadOffset = cmd->payloadStartIndex;
            uint8_t status;
            GroupId groupId;
            uint8_t sceneId;

            status = (*(uint8_t *) (cmd->buffer + payloadOffset));
            payloadOffset += sizeof(uint8_t);
            groupId = (*(GroupId *) (cmd->buffer + payloadOffset));
            payloadOffset += sizeof(uint16_t);
            sceneId = (*(uint8_t *) (cmd->buffer + payloadOffset));

            wasHandled = emberAfScenesClusterStoreSceneResponseCallback(status, groupId, sceneId);
            break;
        }
        case ZCL_VIEW_SCENE_RESPONSE_COMMAND_ID: {
            uint32_t payloadOffset = cmd->payloadStartIndex;
            uint8_t status;
            GroupId groupId;
            uint8_t sceneId;
            uint16_t transitionTime;
            uint8_t * sceneName;
            uint8_t * extensionFieldSets;

            status = (*(uint8_t *) (cmd->buffer + payloadOffset));
            payloadOffset += sizeof(uint8_t);
            groupId = (*(GroupId *) (cmd->buffer + payloadOffset));
            payloadOffset += sizeof(uint16_t);
            sceneId = (*(uint8_t *) (cmd->buffer + payloadOffset));
            payloadOffset += sizeof(uint8_t);
            transitionTime = (*(uint16_t *) (cmd->buffer + payloadOffset));
            payloadOffset += sizeof(uint16_t);
            sceneName = emberAfGetString(cmd->buffer, payloadOffset, cmd->bufLen);
            payloadOffset += emberAfStringLength(sceneName) + 1u;
            extensionFieldSets = (uint8_t *) (cmd->buffer + payloadOffset);

            wasHandled = emberAfScenesClusterViewSceneResponseCallback(status, groupId, sceneId, transitionTime, sceneName,
                                                                       extensionFieldSets);
            break;
        }
        default: {
            // Unrecognized command ID, error status will apply.
            break;
        }
        }
    }
    return status(wasHandled, true, cmd->mfgSpecific);
}
EmberAfStatus emberAfScenesClusterServerCommandParse(EmberAfClusterCommand * cmd)
{
    bool wasHandled = false;

    if (!cmd->mfgSpecific)
    {
        switch (cmd->commandId)
        {
        case ZCL_ADD_SCENE_COMMAND_ID: {
            uint32_t payloadOffset = cmd->payloadStartIndex;
            GroupId groupId;
            uint8_t sceneId;
            uint16_t transitionTime;
            uint8_t * sceneName;
            uint8_t * extensionFieldSets;

            groupId = (*(GroupId *) (cmd->buffer + payloadOffset));
            payloadOffset += sizeof(uint16_t);
            sceneId = (*(uint8_t *) (cmd->buffer + payloadOffset));
            payloadOffset += sizeof(uint8_t);
            transitionTime = (*(uint16_t *) (cmd->buffer + payloadOffset));
            payloadOffset += sizeof(uint16_t);
            sceneName = emberAfGetString(cmd->buffer, payloadOffset, cmd->bufLen);
            payloadOffset += emberAfStringLength(sceneName) + 1u;
            extensionFieldSets = (uint8_t *) (cmd->buffer + payloadOffset);

            wasHandled = emberAfScenesClusterAddSceneCallback(groupId, sceneId, transitionTime, sceneName, extensionFieldSets);
            break;
        }
        case ZCL_GET_SCENE_MEMBERSHIP_COMMAND_ID: {
            uint32_t payloadOffset = cmd->payloadStartIndex;
            GroupId groupId;

            groupId = (*(GroupId *) (cmd->buffer + payloadOffset));

            wasHandled = emberAfScenesClusterGetSceneMembershipCallback(groupId);
            break;
        }
        case ZCL_RECALL_SCENE_COMMAND_ID: {
            uint32_t payloadOffset = cmd->payloadStartIndex;
            GroupId groupId;
            uint8_t sceneId;
            uint16_t transitionTime;

            groupId = (*(GroupId *) (cmd->buffer + payloadOffset));
            payloadOffset += sizeof(uint16_t);
            sceneId = (*(uint8_t *) (cmd->buffer + payloadOffset));
            payloadOffset += sizeof(uint8_t);
            transitionTime = (*(uint16_t *) (cmd->buffer + payloadOffset));

            wasHandled = emberAfScenesClusterRecallSceneCallback(groupId, sceneId, transitionTime);
            break;
        }
        case ZCL_REMOVE_ALL_SCENES_COMMAND_ID: {
            uint32_t payloadOffset = cmd->payloadStartIndex;
            GroupId groupId;

            groupId = (*(GroupId *) (cmd->buffer + payloadOffset));

            wasHandled = emberAfScenesClusterRemoveAllScenesCallback(groupId);
            break;
        }
        case ZCL_REMOVE_SCENE_COMMAND_ID: {
            uint32_t payloadOffset = cmd->payloadStartIndex;
            GroupId groupId;
            uint8_t sceneId;

            groupId = (*(GroupId *) (cmd->buffer + payloadOffset));
            payloadOffset += sizeof(uint16_t);
            sceneId = (*(uint8_t *) (cmd->buffer + payloadOffset));

            wasHandled = emberAfScenesClusterRemoveSceneCallback(groupId, sceneId);
            break;
        }
        case ZCL_STORE_SCENE_COMMAND_ID: {
            uint32_t payloadOffset = cmd->payloadStartIndex;
            GroupId groupId;
            uint8_t sceneId;

            groupId = (*(GroupId *) (cmd->buffer + payloadOffset));
            payloadOffset += sizeof(uint16_t);
            sceneId = (*(uint8_t *) (cmd->buffer + payloadOffset));

            wasHandled = emberAfScenesClusterStoreSceneCallback(groupId, sceneId);
            break;
        }
        case ZCL_VIEW_SCENE_COMMAND_ID: {
            uint32_t payloadOffset = cmd->payloadStartIndex;
            GroupId groupId;
            uint8_t sceneId;

            groupId = (*(GroupId *) (cmd->buffer + payloadOffset));
            payloadOffset += sizeof(uint16_t);
            sceneId = (*(uint8_t *) (cmd->buffer + payloadOffset));

            wasHandled = emberAfScenesClusterViewSceneCallback(groupId, sceneId);
            break;
        }
        default: {
            // Unrecognized command ID, error status will apply.
            break;
        }
        }
    }
    return status(wasHandled, true, cmd->mfgSpecific);
}
EmberAfStatus emberAfTemperatureMeasurementClusterServerCommandParse(EmberAfClusterCommand * cmd)
{
    bool wasHandled = false;

    if (!cmd->mfgSpecific)
    {
        switch (cmd->commandId)
        {
        default: {
            // Unrecognized command ID, error status will apply.
            break;
        }
        }
    }
    return status(wasHandled, true, cmd->mfgSpecific);
}
