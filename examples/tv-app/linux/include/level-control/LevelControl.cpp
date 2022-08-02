/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
 *    All rights reserved.
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

#include <app/util/af.h>

#include <app-common/zap-generated/af-structs.h>
#include <app-common/zap-generated/attribute-id.h>
#include <app-common/zap-generated/attribute-type.h>
#include <app-common/zap-generated/cluster-id.h>
#include <app-common/zap-generated/command-id.h>

using namespace chip;

#define MAX_LEVEL 99
#define MIN_LEVEL 1

typedef struct
{
    CommandId commandId;
    uint16_t storedLevel;
    bool increasing;
} EmberAfLevelControlState;

static EmberAfLevelControlState stateTable[EMBER_AF_LEVEL_CONTROL_CLUSTER_SERVER_ENDPOINT_COUNT];

static EmberAfLevelControlState * getState(EndpointId endpoint)
{
    uint8_t ep = emberAfFindClusterServerEndpointIndex(endpoint, ZCL_LEVEL_CONTROL_CLUSTER_ID);
    return (ep == 0xFF ? NULL : &stateTable[ep]);
}

static void stepHandler(CommandId commandId, uint8_t stepMode, uint8_t stepSize, pp::DataModel::Nullable<uint16_t> transitionTimeDs,
                        uint8_t optionMask, uint8_t optionOverride)
{

    EndpointId endpoint              = emberAfCurrentEndpoint();
    EmberAfLevelControlState * state = getState(endpoint);
    EmberAfStatus status;
    app::DataModel::Nullable<uint8_t> nullableCurrentLevel;
    uint8_t currentLevel;

    status = Attributes::CurrentLevel::Get(endpoint, nullableCurrentLevel);
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        emberAfLevelControlClusterPrintln("ERR: reading current level %x", status);
        goto send_default_response;
    }

    if (nullableCurrentLevel.IsNull())
    {
        emberAfLevelControlClusterPrintln("ERR: Current Level is null");
        status = EMBER_ZCL_STATUS_FAILURE;

        goto send_default_response;
    }

    currentLevel = nullableCurrentLevel.Value();

    switch (stepMode)
    {
    case EMBER_ZCL_STEP_MODE_UP:
        state->increasing = true;
        if (MAX_LEVEL >= currentLevel + stepSize)
        {
            currentLevel = currentLevel + stepSize;
        }
        break;
    case EMBER_ZCL_STEP_MODE_DOWN:
        state->increasing = false;
        if (MIN_LEVEL <= currentLevel - stepSize)
        {
            currentLevel = currentLevel - stepSize;
        }
        break;
    default:
        status = EMBER_ZCL_STATUS_INVALID_FIELD;
        goto send_default_response;
    }

    if (currentLevel != state->storedLevel)
    {
        int volumeIncrementCount = abs(currentLevel - state->storedLevel);
        for (int i = 0; i < volumeIncrementCount; ++i)
        {
            if (state->increasing)
            {
                ChipLogProgress(Zcl, "Volume UP");
                // TODO: Insert your code here to send volume up command
            }
            else
            {
                ChipLogProgress(Zcl, "Volume DOWN");
                // TODO: Insert your code here to send volume down command
            }
        }

        nullableCurrentLevel.SetNonNull(currentLevel);
        status = Attributes::CurrentLevel::Set(endpoint, nullableCurrentLevel);

        state->storedLevel = currentLevel;
        ChipLogProgress(Zcl, "Setting volume to new level %d", state->storedLevel);
    }

send_default_response:
    if (emberAfCurrentCommand()->apsFrame->clusterId == ZCL_LEVEL_CONTROL_CLUSTER_ID)
    {
        emberAfSendImmediateDefaultResponse(status);
    }
}

bool emberAfLevelControlClusterStepCallback(uint8_t stepMode, uint8_t stepSize, pp::DataModel::Nullable<uint8_t> transitionTime,
                                            uint8_t optionMask, uint8_t optionOverride)
{
    stepHandler(ZCL_STEP_COMMAND_ID, stepMode, stepSize, transitionTime, optionMask, optionOverride);
    return true;
}

bool emberAfLevelControlClusterMoveCallback(unsigned char, unsigned char, unsigned char, unsigned char)
{
    ChipLogProgress(Zcl, "Not supported");
    return true;
}

bool emberAfLevelControlClusterMoveToLevelCallback(unsigned char, unsigned short, unsigned char, unsigned char)
{
    ChipLogProgress(Zcl, "Not supported");
    return true;
}

bool emberAfLevelControlClusterMoveToLevelWithOnOffCallback(unsigned char, unsigned short)
{
    ChipLogProgress(Zcl, "Not supported");
    return true;
}

bool emberAfLevelControlClusterMoveWithOnOffCallback(unsigned char, unsigned char)
{
    ChipLogProgress(Zcl, "Not supported");
    return true;
}

bool emberAfLevelControlClusterStopCallback(unsigned char, unsigned char)
{
    ChipLogProgress(Zcl, "Not supported");
    return true;
}

bool emberAfLevelControlClusterStopWithOnOffCallback()
{
    ChipLogProgress(Zcl, "Not supported");
    return true;
}

bool emberAfOnOffClusterLevelControlEffectCallback(unsigned char, bool)
{
    ChipLogProgress(Zcl, "Not supported");
    return true;
}

bool emberAfLevelControlClusterServerInitCallback(unsigned char)
{
    ChipLogProgress(Zcl, "Not supported");
    return true;
}

bool emberAfLevelControlClusterStepWithOnOffCallback(unsigned char, unsigned char, unsigned short)
{
    ChipLogProgress(Zcl, "Not supported");
    return true;
}

bool emberAfLevelControlClusterServerTickCallback(unsigned char)
{
    ChipLogProgress(Zcl, "Not supported");
    return true;
}
