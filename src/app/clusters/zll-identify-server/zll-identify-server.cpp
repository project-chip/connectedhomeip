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

/**
 * @file
 * @brief Routines for the ZLL Identify Server plugin,
 * which contains additions to the Identify server
 * cluster.
 *******************************************************************************
 * # License
 * <b>Copyright 2018 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon
 * Laboratories Inc. Your use of this software is
 * governed by the terms of Silicon Labs Master
 * Software License Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement.
 * This software is distributed to you in Source Code
 * format and is governed by the sections of the MSLA
 * applicable to Source Code.
 *
 ******************************************************************************/

// this file contains all the common includes for clusters in the util
#include <app/CommandHandler.h>
#include <app/common/gen/attribute-id.h>
#include <app/common/gen/attribute-type.h>
#include <app/common/gen/cluster-id.h>
#include <app/common/gen/command-id.h>
#include <app/common/gen/enums.h>
#include <app/util/af.h>
#include <app/util/common.h>

#ifndef EZSP_HOST
//#include "hal/hal.h"
#endif

using namespace chip;

// Copied in from Z3LightSoc/Z3LightSoc.h
// Use this macro to check if ZLL Identify Server plugin is included
#define EMBER_AF_PLUGIN_ZLL_IDENTIFY_SERVER
// User options for plugin ZLL Identify Server
#define EMBER_AF_PLUGIN_ZLL_IDENTIFY_SERVER_EVENT_DELAY 1024
#define EMBER_AF_PLUGIN_ZLL_IDENTIFY_SERVER_BLINK_EVENTS 2
#define EMBER_AF_PLUGIN_ZLL_IDENTIFY_SERVER_BREATHE_EVENTS 4
#define EMBER_AF_PLUGIN_ZLL_IDENTIFY_SERVER_OKAY_EVENTS 6
#define EMBER_AF_PLUGIN_ZLL_IDENTIFY_SERVER_CHANNEL_CHANGE_EVENTS 8

typedef struct
{
    bool active;
    bool cancel;
    EmberAfIdentifyEffectIdentifier effectId;
    EmberAfIdentifyEffectVariant commandVariant;
    uint8_t eventsRemaining;
    uint16_t eventDelay;
} EmAfZllIdentifyState;

void emAfPluginZllIdentifyServerBlinkEffect(EndpointId endpoint);

void emAfPluginZllIdentifyServerBreatheEffect(EndpointId endpoint);

void emAfPluginZllIdentifyServerOkayEffect(EndpointId endpoint);

void emAfPluginZllIdentifyServerChannelChangeEffect(EndpointId endpoint);

extern EmberEventControl emberAfPluginZllIdentifyServerTriggerEffectEndpointEventControls[];

static EmAfZllIdentifyState stateTable[EMBER_AF_IDENTIFY_CLUSTER_SERVER_ENDPOINT_COUNT];

static EmAfZllIdentifyState * getZllIdentifyState(EndpointId endpoint);

static void deactivateZllIdentify(EmAfZllIdentifyState * state, EndpointId endpoint);

static EmAfZllIdentifyState * getZllIdentifyState(EndpointId endpoint)
{
    uint8_t index = emberAfFindClusterServerEndpointIndex(endpoint, ZCL_IDENTIFY_CLUSTER_ID);
    return (index == 0xFF ? NULL : &stateTable[index]);
}

static void deactivateZllIdentify(EmAfZllIdentifyState * state, EndpointId endpoint)
{
    if (state == NULL)
    {
        return;
    }

    state->active = false;
    state->cancel = false;

    emberAfEndpointEventControlSetInactive(emberAfPluginZllIdentifyServerTriggerEffectEndpointEventControls, endpoint);
}

void emberAfPluginZllIdentifyServerTriggerEffectEndpointEventHandler(EndpointId endpoint)
{
    EmAfZllIdentifyState * state = getZllIdentifyState(endpoint);

    if (state == NULL)
    {
        return;
    }

    switch (state->effectId)
    {
    case EMBER_ZCL_IDENTIFY_EFFECT_IDENTIFIER_BLINK:
        emAfPluginZllIdentifyServerBlinkEffect(endpoint);
        break;
    case EMBER_ZCL_IDENTIFY_EFFECT_IDENTIFIER_BREATHE:
        emAfPluginZllIdentifyServerBreatheEffect(endpoint);
        break;
    case EMBER_ZCL_IDENTIFY_EFFECT_IDENTIFIER_OKAY:
        emAfPluginZllIdentifyServerOkayEffect(endpoint);
        break;
    case EMBER_ZCL_IDENTIFY_EFFECT_IDENTIFIER_CHANNEL_CHANGE:
        emAfPluginZllIdentifyServerChannelChangeEffect(endpoint);
        break;
    case EMBER_ZCL_IDENTIFY_EFFECT_IDENTIFIER_FINISH_EFFECT: // At this point, these are functionally equivalent
    case EMBER_ZCL_IDENTIFY_EFFECT_IDENTIFIER_STOP_EFFECT:
    default:
        deactivateZllIdentify(state, endpoint);
        return;
    }
    if (state->cancel)
    {
        deactivateZllIdentify(state, endpoint);
        return;
    }

    if (state->active)
    {
        emberAfEndpointEventControlSetDelayMS(emberAfPluginZllIdentifyServerTriggerEffectEndpointEventControls, endpoint,
                                              state->eventDelay);
    }
}

bool emberAfIdentifyClusterTriggerEffectCallback(chip::app::CommandHandler * commandObj, uint8_t effectId, uint8_t effectVariant)
{
    EndpointId endpoint          = emberAfCurrentEndpoint();
    EmAfZllIdentifyState * state = getZllIdentifyState(endpoint);
    EmberAfStatus status;

    if (state == NULL)
    {
        status = EMBER_ZCL_STATUS_FAILURE;
        goto default_response;
    }

    emberAfIdentifyClusterPrintln("RX identify:trigger effect 0x%x variant 0x%x", effectId, effectVariant);

    if (state->active)
    {
        switch (state->effectId)
        {
        case EMBER_ZCL_IDENTIFY_EFFECT_IDENTIFIER_FINISH_EFFECT:
            state->cancel = true;
            status        = EMBER_ZCL_STATUS_SUCCESS;
            break;
        case EMBER_ZCL_IDENTIFY_EFFECT_IDENTIFIER_STOP_EFFECT:
            deactivateZllIdentify(state, endpoint);
            status = EMBER_ZCL_STATUS_SUCCESS;
            goto default_response;
        default:
            status = EMBER_ZCL_STATUS_FAILURE;
            break;
        }
    }
    else
    {
        switch (effectId)
        {
        case EMBER_ZCL_IDENTIFY_EFFECT_IDENTIFIER_BLINK:
            state->eventsRemaining = EMBER_AF_PLUGIN_ZLL_IDENTIFY_SERVER_BLINK_EVENTS;
            break;
        case EMBER_ZCL_IDENTIFY_EFFECT_IDENTIFIER_BREATHE:
            state->eventsRemaining = EMBER_AF_PLUGIN_ZLL_IDENTIFY_SERVER_BREATHE_EVENTS;
            break;
        case EMBER_ZCL_IDENTIFY_EFFECT_IDENTIFIER_OKAY:
            state->eventsRemaining = EMBER_AF_PLUGIN_ZLL_IDENTIFY_SERVER_OKAY_EVENTS;
            break;
        case EMBER_ZCL_IDENTIFY_EFFECT_IDENTIFIER_CHANNEL_CHANGE:
            state->eventsRemaining = EMBER_AF_PLUGIN_ZLL_IDENTIFY_SERVER_CHANNEL_CHANGE_EVENTS;
            break;
        case EMBER_ZCL_IDENTIFY_EFFECT_IDENTIFIER_FINISH_EFFECT: // At this point, these are functionally equivalent
        case EMBER_ZCL_IDENTIFY_EFFECT_IDENTIFIER_STOP_EFFECT:
            status = EMBER_ZCL_STATUS_SUCCESS;
            goto default_response;
        default:
            status = EMBER_ZCL_STATUS_FAILURE;
            goto default_response;
        }
        state->active         = true;
        state->cancel         = false;
        state->effectId       = (EmberAfIdentifyEffectIdentifier) effectId;
        state->commandVariant = (EmberAfIdentifyEffectVariant) effectVariant;
        state->eventDelay     = EMBER_AF_PLUGIN_ZLL_IDENTIFY_SERVER_EVENT_DELAY;
        emberAfEndpointEventControlSetDelayMS(emberAfPluginZllIdentifyServerTriggerEffectEndpointEventControls, endpoint,
                                              state->eventDelay);
        status = EMBER_ZCL_STATUS_SUCCESS;
    }

default_response:
    emberAfSendImmediateDefaultResponse(status);
    return true;
}

void emAfPluginZllIdentifyServerBlinkEffect(EndpointId endpoint)
{
    EmAfZllIdentifyState * state = getZllIdentifyState(endpoint);

    if (state == NULL || state->eventsRemaining == 0)
    {
        deactivateZllIdentify(state, endpoint);
        return;
    }

#ifndef EZSP_HOST
//    halToggleLed(BOARDLED0);
//    halToggleLed(BOARDLED1);
//    halToggleLed(BOARDLED2);
//    halToggleLed(BOARDLED3);
#endif

    state->eventsRemaining = state->eventsRemaining - 1;
}

void emAfPluginZllIdentifyServerBreatheEffect(EndpointId endpoint)
{
    emAfPluginZllIdentifyServerBlinkEffect(endpoint);
}

void emAfPluginZllIdentifyServerOkayEffect(EndpointId endpoint)
{
    emAfPluginZllIdentifyServerBlinkEffect(endpoint);
}

void emAfPluginZllIdentifyServerChannelChangeEffect(EndpointId endpoint)
{
    emAfPluginZllIdentifyServerBlinkEffect(endpoint);
}
