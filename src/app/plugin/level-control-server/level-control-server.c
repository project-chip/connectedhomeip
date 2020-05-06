/***************************************************************************/ /**
                                                                               *
                                                                               *    <COPYRIGHT>
                                                                               *
                                                                               *    Licensed under the Apache License, Version 2.0
                                                                               *(the "License"); you may not use this file except in
                                                                               *compliance with the License. You may obtain a copy
                                                                               *of the License at
                                                                               *
                                                                               *        http://www.apache.org/licenses/LICENSE-2.0
                                                                               *
                                                                               *    Unless required by applicable law or agreed to
                                                                               *in writing, software distributed under the License
                                                                               *is distributed on an "AS IS" BASIS, WITHOUT
                                                                               *WARRANTIES OR CONDITIONS OF ANY KIND, either express
                                                                               *or implied. See the License for the specific
                                                                               *language governing permissions and limitations under
                                                                               *the License.
                                                                               *
                                                                               ******************************************************************************/

#ifdef ZAP_TEST
#include "utest.h"
#endif

#include PLATFORM_HEADER
#include CONFIGURATION_HEADER
#include ZAP_AF_API_STACK
#include ZAP_AF_API_BUFFER_MANAGEMENT
#include ZAP_AF_API_EVENT_QUEUE
#include ZAP_AF_API_HAL
#ifdef ZAP_AF_API_DEBUG_PRINT
#include ZAP_AF_API_DEBUG_PRINT
#endif
#include ZAP_AF_API_ZCL_CORE
#ifdef ZAP_AF_API_ZCL_SCENES_SERVER
#include ZAP_AF_API_ZCL_SCENES_SERVER
#endif
#include "level-control-server.h"

#define MIN_LEVEL ZAP_AF_PLUGIN_LEVEL_CONTROL_SERVER_MINIMUM_LEVEL
#define MAX_LEVEL ZAP_AF_PLUGIN_LEVEL_CONTROL_SERVER_MAXIMUM_LEVEL

#define BOUND_MIN(value) ((value) < MIN_LEVEL ? MIN_LEVEL : (value))
#define BOUND_MAX(value) ((value) < MAX_LEVEL ? (value) : MAX_LEVEL)
#define BOUND_MIN_MAX(value) BOUND_MIN(BOUND_MAX(value))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

#define MIN_DELAY_MS 1

#ifdef ZAP_AF_API_ZCL_SCENES_SERVER
#ifdef DEFINETOKENS
// Token based storage.
#define retrieveSceneSubTableEntry(entry, i) halCommonGetIndexedToken(&entry, TOKEN_ZCL_CORE_LEVEL_CONTROL_SCENE_SUBTABLE, i)
#define saveSceneSubTableEntry(entry, i) halCommonSetIndexedToken(TOKEN_ZCL_CORE_LEVEL_CONTROL_SCENE_SUBTABLE, i, &entry)
#else
// RAM based storage.
ZapLevelControlSceneSubTableEntry_t zapPluginLevelControlServerSceneSubTable[ZAP_AF_PLUGIN_SCENES_SERVER_TABLE_SIZE] = { { 0 } };
#define retrieveSceneSubTableEntry(entry, i) (entry = zapPluginLevelControlServerSceneSubTable[i])
#define saveSceneSubTableEntry(entry, i) (zapPluginLevelControlServerSceneSubTable[i] = entry)
#endif
#endif

int abs(int I);
static void moveToLevelHandler(const ZapCommandContext_t * context,
                               const ZapClusterLevelControlServerCommandMoveToLevelRequest_t * request, bool withOnOff);
static void moveHandler(const ZapCommandContext_t * context, const ZapClusterLevelControlServerCommandMoveRequest_t * request,
                        bool withOnOff);
static void stepHandler(const ZapCommandContext_t * context, const ZapClusterLevelControlServerCommandStepRequest_t * request,
                        bool withOnOff);
static void stopHandler(const ZapCommandContext_t * context, const ZapClusterLevelControlServerCommandStopRequest_t * request,
                        bool withOnOff);
static bool getOnOff(ZapEndpointId_t endpointId);
static void setOnOff(ZapEndpointId_t endpointId, bool onOff);
static uint8_t getCurrentLevel(ZapEndpointId_t endpointId);
static uint16_t getOnOffTransitionTime(ZapEndpointId_t endpointId);
static uint8_t getDefaultMoveRate(ZapEndpointId_t endpointId);
static bool shouldExecuteIfOff(uint8_t endpoint, uint8_t optionMask, uint8_t optionOverride);
static void setCurrentLevel(ZapEndpointId_t endpointId, uint8_t currentLevel);
static uint8_t getOnLevelOrCurrentLevel(ZapEndpointId_t endpointId);

typedef struct
{
    uint32_t delayMs;
    ZapEndpointId_t endpointId;
    uint8_t targetLevel;
    uint8_t postTransitionLevel;
    bool increasing;
    bool withOnOff;
    bool immediate; // If true level will be set in next tick
} State;

typedef struct
{
    Event event;
    State state;
} LevelControlEvent;

extern EventQueue emAppEventQueue;
static void eventHandler(LevelControlEvent * event);
static void eventMarker(LevelControlEvent * event);
static EventActions actions = { &emAppEventQueue, (void (*)(struct Event_s *)) eventHandler,
                                (void (*)(struct Event_s *)) eventMarker, "level control server" };

static LevelControlEvent * cancel(State * state);
static ZapStatus_t schedule(State * state);

void zapLevelControlServerSetOnOff(ZapEndpointId_t endpointId, bool value)
{
    State state = {
        .endpointId = endpointId,
        .increasing = value,
        .withOnOff  = true,
        .immediate  = false,
    };
    if (value)
    {
        // Light is being turned on
        state.targetLevel         = getOnLevelOrCurrentLevel(endpointId);
        state.postTransitionLevel = state.targetLevel;
        if (getOnOff(endpointId) == 0)
        {
            // Only smoothly transition from off to on if the light is off
            setCurrentLevel(endpointId, MIN_LEVEL);
            setOnOff(endpointId, true);
        }
    }
    else
    {
        // Light is being turned off
        setOnOff(endpointId, false);
        state.targetLevel         = MIN_LEVEL;
        state.postTransitionLevel = getOnLevelOrCurrentLevel(endpointId);
    }
    uint8_t currentLevel = getCurrentLevel(endpointId);
    if (currentLevel == state.targetLevel)
    {
        cancel(&state);
    }
    else
    {
        uint8_t stepSize             = abs(currentLevel - state.targetLevel);
        uint16_t onOffTransitionTime = getOnOffTransitionTime(endpointId);
        uint32_t delayMs =
            (onOffTransitionTime == 0x0 ? MIN_DELAY_MS : (onOffTransitionTime * MILLISECOND_TICKS_PER_DECISECOND / stepSize));
        state.delayMs = MAX(delayMs, MIN_DELAY_MS); // Make sure we didn't round down to zero
        schedule(&state);
    }
}

void zapClusterLevelControlServerCommandMoveToLevelRequestHandler(
    const ZapCommandContext_t * context, const ZapClusterLevelControlServerCommandMoveToLevelRequest_t * request)
{
    moveToLevelHandler(context, request, false); // without on/off
}

void zapClusterLevelControlServerCommandMoveRequestHandler(const ZapCommandContext_t * context,
                                                           const ZapClusterLevelControlServerCommandMoveRequest_t * request)
{
    moveHandler(context, request, false); // without on/off
}

void zapClusterLevelControlServerCommandStepRequestHandler(const ZapCommandContext_t * context,
                                                           const ZapClusterLevelControlServerCommandStepRequest_t * request)
{
    stepHandler(context, request, false); // without on/off
}

void zapClusterLevelControlServerCommandStopRequestHandler(const ZapCommandContext_t * context,
                                                           const ZapClusterLevelControlServerCommandStopRequest_t * request)
{
    stopHandler(context, request, false); // without on/off
}

void zapClusterLevelControlServerCommandMoveToLevelWithOnOffRequestHandler(
    const ZapCommandContext_t * context, const ZapClusterLevelControlServerCommandMoveToLevelWithOnOffRequest_t * request)
{
    moveToLevelHandler(context, (const ZapClusterLevelControlServerCommandMoveToLevelRequest_t *) request,
                       true); // with on/off
}

void zapClusterLevelControlServerCommandMoveWithOnOffRequestHandler(
    const ZapCommandContext_t * context, const ZapClusterLevelControlServerCommandMoveWithOnOffRequest_t * request)
{
    moveHandler(context, (const ZapClusterLevelControlServerCommandMoveRequest_t *) request,
                true); // with on/off
}

void zapClusterLevelControlServerCommandStepWithOnOffRequestHandler(
    const ZapCommandContext_t * context, const ZapClusterLevelControlServerCommandStepWithOnOffRequest_t * request)
{
    stepHandler(context, (const ZapClusterLevelControlServerCommandStepRequest_t *) request,
                true); // with on/off
}

void zapClusterLevelControlServerCommandStopWithOnOffRequestHandler(
    const ZapCommandContext_t * context, const ZapClusterLevelControlServerCommandStopWithOnOffRequest_t * request)
{
    stopHandler(context, (const ZapClusterLevelControlServerCommandStopRequest_t *) request,
                true); // with on/off
}

static void moveToLevelHandler(const ZapCommandContext_t * context,
                               const ZapClusterLevelControlServerCommandMoveToLevelRequest_t * request, bool withOnOff)
{
    zapCorePrintln("RX: MoveToLevel%s", (withOnOff ? "WithOnOff" : ""));

    // Use OptionsMask and OptionsOverride when implemented
    if (!withOnOff && (shouldExecuteIfOff(context->endpointId, 0, 0) != TRUE))
    {
        zapSendDefaultResponse(context, ZAP_STATUS_SUCCESS);
        return;
    }

    uint8_t currentLevel = getCurrentLevel(context->endpointId);
    uint8_t level        = BOUND_MIN_MAX(request->level);

    State state = {
        .endpointId          = context->endpointId,
        .targetLevel         = level,
        .postTransitionLevel = level,
        .increasing          = (currentLevel <= level),
        .withOnOff           = withOnOff,
        .immediate           = false,
    };

    if (state.increasing && state.withOnOff)
    {
        setOnOff(context->endpointId, (state.targetLevel != MIN_LEVEL));
    }

    ZapStatus_t status;
    if (currentLevel == state.targetLevel)
    {
        cancel(&state);
        status = ZAP_STATUS_SUCCESS;
    }
    else
    {
        uint8_t stepSize             = abs(currentLevel - request->level);
        uint16_t onOffTransitionTime = getOnOffTransitionTime(context->endpointId);
        uint32_t delayMs =
            ((request->transitionTime == 0xFFFF)
                 ? (onOffTransitionTime == 0x0 ? MIN_DELAY_MS : (onOffTransitionTime * MILLISECOND_TICKS_PER_DECISECOND / stepSize))
                 : (request->transitionTime * MILLISECOND_TICKS_PER_DECISECOND / stepSize));
        // If requested transition time is zero set level in next tick
        if (delayMs == 0)
        {
            state.immediate = true;
        }
        state.delayMs = MAX(delayMs, MIN_DELAY_MS); // Make sure we didn't round down to zero
        status        = schedule(&state);
    }
    zapSendDefaultResponse(context, status);
}

static void moveHandler(const ZapCommandContext_t * context, const ZapClusterLevelControlServerCommandMoveRequest_t * request,
                        bool withOnOff)
{
    zapCorePrintln("RX: Move%s", (withOnOff ? "WithOnOff" : ""));

    uint8_t level;
    switch (request->moveMode)
    {
    case 0: // up
        level = MAX_LEVEL;
        break;
    case 1: // down
        level = MIN_LEVEL;
        break;
    default:
        zapSendDefaultResponse(context, ZAP_STATUS_INVALID_FIELD);
        return;
    }

    // Use OptionsMask and OptionsOverride when implemented
    if (!withOnOff && (shouldExecuteIfOff(context->endpointId, 0, 0) != TRUE))
    {
        zapSendDefaultResponse(context, ZAP_STATUS_SUCCESS);
        return;
    }

    // Moving at zero rate means not moving at all
    if (request->rate == 0)
    {
        zapSendDefaultResponse(context, ZAP_STATUS_SUCCESS);
        return;
    }

    uint8_t currentLevel = getCurrentLevel(context->endpointId);
    State state          = {
        .endpointId          = context->endpointId,
        .targetLevel         = level,
        .postTransitionLevel = level,
        .increasing          = (currentLevel <= level),
        .withOnOff           = withOnOff,
        .immediate           = false,
    };

    if (state.increasing && state.withOnOff)
    {
        setOnOff(context->endpointId, (state.targetLevel != MIN_LEVEL));
    }

    ZapStatus_t status;
    if (currentLevel == state.targetLevel)
    {
        cancel(&state);
        status = ZAP_STATUS_SUCCESS;
    }
    else
    {
        uint8_t defaultMoveRate = getDefaultMoveRate(context->endpointId);
        uint32_t delayMs =
            (request->rate == 0xFF ? (defaultMoveRate == 0 ? MIN_DELAY_MS : MILLISECOND_TICKS_PER_SECOND / defaultMoveRate)
                                   : MILLISECOND_TICKS_PER_SECOND / request->rate);
        state.delayMs = MAX(delayMs, MIN_DELAY_MS); // Make sure we didn't round down to zero
        status        = schedule(&state);
    }
    zapSendDefaultResponse(context, status);
}

static void stepHandler(const ZapCommandContext_t * context, const ZapClusterLevelControlServerCommandStepRequest_t * request,
                        bool withOnOff)
{
    zapCorePrintln("RX: Step%s", (withOnOff ? "WithOnOff" : ""));

    uint8_t currentLevel = getCurrentLevel(context->endpointId);
    uint8_t level;
    switch (request->stepMode)
    {
    case 0: // up
        level = BOUND_MAX(currentLevel + request->stepSize);
        break;
    case 1: // down
        level = BOUND_MIN(currentLevel - request->stepSize);
        break;
    default:
        zapSendDefaultResponse(context, ZAP_STATUS_INVALID_FIELD);
        return;
    }

    // Use OptionsMask and OptionsOverride when implemented
    if (!withOnOff && (shouldExecuteIfOff(context->endpointId, 0, 0) != TRUE))
    {
        zapSendDefaultResponse(context, ZAP_STATUS_SUCCESS);
        return;
    }

    State state = {
        .endpointId          = context->endpointId,
        .targetLevel         = level,
        .postTransitionLevel = level,
        .increasing          = (currentLevel <= level),
        .withOnOff           = withOnOff,
        .immediate           = false,
    };

    if (state.increasing && state.withOnOff)
    {
        setOnOff(context->endpointId, (state.targetLevel != MIN_LEVEL));
    }

    ZapStatus_t status;
    if (currentLevel == state.targetLevel)
    {
        cancel(&state);
        status = ZAP_STATUS_SUCCESS;
    }
    else
    {
        state.delayMs =
            (request->transitionTime == 0xFFFF ? MIN_DELAY_MS
                                               : (request->transitionTime * MILLISECOND_TICKS_PER_DECISECOND / request->stepSize));
        status = schedule(&state);
    }
    zapSendDefaultResponse(context, status);
}

static void stopHandler(const ZapCommandContext_t * context, const ZapClusterLevelControlServerCommandStopRequest_t * request,
                        bool withOnOff)
{
    zapCorePrintln("RX: Stop%s", (withOnOff ? "WithOnOff" : ""));

    // Use OptionsMask and OptionsOverride when implemented
    if (!withOnOff && (shouldExecuteIfOff(context->endpointId, 0, 0) != TRUE))
    {
        zapSendDefaultResponse(context, ZAP_STATUS_SUCCESS);
        return;
    }

    State state = {
        .endpointId = context->endpointId,
        .immediate  = false,
    };
    cancel(&state);
    //}
    zapSendDefaultResponse(context, ZAP_STATUS_SUCCESS);
}

static bool getOnOff(ZapEndpointId_t endpointId)
{
    bool onOff;
    if (zapReadAttribute(endpointId, &zapClusterOnOffServerSpec, ZAP_CLUSTER_ON_OFF_SERVER_ATTRIBUTE_ON_OFF, &onOff,
                         sizeof(onOff)) == ZAP_STATUS_SUCCESS)
    {
        // Returns true if the light is on, false if the light is off
        return onOff;
    }
    else
    {
        // If zapReadAttribute fails to return a success status,
        // false is returned as a default to represent the off state
        return false;
    }
}

static void setOnOff(ZapEndpointId_t endpointId, bool onOff)
{
    zapWriteAttribute(endpointId, &zapClusterOnOffServerSpec, ZAP_CLUSTER_ON_OFF_SERVER_ATTRIBUTE_ON_OFF, &onOff, sizeof(onOff));
}

static uint8_t getCurrentLevel(ZapEndpointId_t endpointId)
{
    uint8_t currentLevel;
    if (zapReadAttribute(endpointId, &zapClusterLevelControlServerSpec, ZAP_CLUSTER_LEVEL_CONTROL_SERVER_ATTRIBUTE_CURRENT_LEVEL,
                         &currentLevel, sizeof(currentLevel)) == ZAP_STATUS_SUCCESS)
    {
        return currentLevel;
    }
    else
    {
        return 0;
    }
}

static uint16_t getOnOffTransitionTime(ZapEndpointId_t endpointId)
{
    uint16_t onOffTransitionTime;
    if (zapReadAttribute(endpointId, &zapClusterLevelControlServerSpec,
                         ZAP_CLUSTER_LEVEL_CONTROL_SERVER_ATTRIBUTE_ON_OFF_TRANSITION_TIME, &onOffTransitionTime,
                         sizeof(onOffTransitionTime)) == ZAP_STATUS_SUCCESS)
    {
        return onOffTransitionTime;
    }
    else
    {
        return 0; // Also default value for this attr
    }
}

// If attribute is not supported return 0
static uint8_t getDefaultMoveRate(ZapEndpointId_t endpointId)
{
    uint8_t defaultMoveRate = 0;
    if (zapReadAttribute(endpointId, &zapClusterLevelControlServerSpec,
                         ZAP_CLUSTER_LEVEL_CONTROL_SERVER_ATTRIBUTE_DEFAULT_MOVE_RATE, &defaultMoveRate,
                         sizeof(defaultMoveRate)) == ZAP_STATUS_SUCCESS)
    {
        return defaultMoveRate;
    }
    else
    {
        // There's no default for this attribute. Caller should treat 0 as "unsupported"
        return 0x0;
    }
}

// Determine whether the command should be executed based on the Options attribute and passed Mask & Override parameters
// Based on the corresponding function from Zigbee
static bool shouldExecuteIfOff(uint8_t endpoint, uint8_t optionMask, uint8_t optionOverride)
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

    uint8_t options;
    if (zapReadAttribute(endpoint, &zapClusterLevelControlServerSpec, ZAP_CLUSTER_LEVEL_CONTROL_SERVER_ATTRIBUTE_OPTIONS, &options,
                         sizeof(options)) != ZAP_STATUS_SUCCESS)
    {
        options = 0; // Default
    }

    // The device is on - hence ExecuteIfOff does not matter
    if (getOnOff(endpoint))
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
    // in section 3.10.2.2.3 (typo, should be 3.10.2.2.8.1).

    // ---------- The following order is important in decission making -------
    // -----------more readable ----------
    //
    if (optionMask == 0xFF && optionOverride == 0xFF)
    {
        // 0xFF are the default values passed to the command handler when
        // the payload is not present - in that case there is use of option
        // attribute to decide execution of the command
        return READBITS(options, ZAP_LEVEL_CONTROL_OPTIONS_EXECUTE_IF_OFF);
    }
    // ---------- The above is to distinguish if the payload is present or not

    if (READBITS(optionMask, ZAP_LEVEL_CONTROL_OPTIONS_EXECUTE_IF_OFF))
    {
        // Mask is present and set in the command payload, this indicates
        // use the over ride as temporary option
        return READBITS(optionOverride, ZAP_LEVEL_CONTROL_OPTIONS_EXECUTE_IF_OFF);
    }
    // if we are here - use the option bits
    return (READBITS(options, ZAP_LEVEL_CONTROL_OPTIONS_EXECUTE_IF_OFF));
}

static void setCurrentLevel(ZapEndpointId_t endpointId, uint8_t currentLevel)
{
    zapWriteAttribute(endpointId, &zapClusterLevelControlServerSpec, ZAP_CLUSTER_LEVEL_CONTROL_SERVER_ATTRIBUTE_CURRENT_LEVEL,
                      &currentLevel, sizeof(currentLevel));
}

static uint8_t getOnLevelOrCurrentLevel(ZapEndpointId_t endpointId)
{
    uint8_t onLevel;
    if ((zapReadAttribute(endpointId, &zapClusterLevelControlServerSpec, ZAP_CLUSTER_LEVEL_CONTROL_SERVER_ATTRIBUTE_ON_LEVEL,
                          &onLevel, sizeof(onLevel)) == ZAP_STATUS_SUCCESS) &&
        onLevel != 0xFF)
    {
        return onLevel;
    }
    else
    {
        return getCurrentLevel(endpointId);
    }
}

static void eventHandler(LevelControlEvent * event)
{
    uint8_t currentLevel;

    if (event->state.immediate)
    {
        currentLevel           = event->state.targetLevel;
        event->state.immediate = false;
    }
    else
    {
        currentLevel = (getCurrentLevel(event->state.endpointId) + (event->state.increasing ? +1 : -1));
    }

    setCurrentLevel(event->state.endpointId, currentLevel);
    if (currentLevel == event->state.targetLevel)
    {
        if (event->state.withOnOff)
        {
            setOnOff(event->state.endpointId, (currentLevel != MIN_LEVEL));
        }
        if (currentLevel != event->state.postTransitionLevel)
        {
            setCurrentLevel(event->state.endpointId, event->state.postTransitionLevel);
        }
    }
    else
    {
        zapEventSetDelayMs((Event *) event, event->state.delayMs);
    }
}

static void eventMarker(LevelControlEvent * event) {}

static bool predicate(LevelControlEvent * event, ZapEndpointId_t * endpointId)
{
    return (*endpointId == event->state.endpointId);
}

static LevelControlEvent * cancel(State * state)
{
    return (LevelControlEvent *) zapFindEvent(actions.queue, &actions, (EventPredicate) predicate, &state->endpointId);
}

static ZapStatus_t schedule(State * state)
{
    LevelControlEvent * event = cancel(state);
    if (event == NULL)
    {
        Buffer buffer = zapAllocateBuffer(sizeof(LevelControlEvent));
        if (buffer == NULL_BUFFER)
        {
            return ZAP_STATUS_FAILURE;
        }
        event = (LevelControlEvent *) zapGetBufferPointer(buffer);
    }
    event->event.actions = &actions;
    event->event.next    = NULL;
    event->state         = *state;
    zapEventSetDelayMs((Event *) event, event->state.delayMs);
    return ZAP_STATUS_SUCCESS;
}

#ifdef ZAP_AF_API_ZCL_SCENES_SERVER
// Scenes callback handlers...

void zapLevelControlServerEraseSceneHandler(uint8_t tableIdx)
{
    ZapLevelControlSceneSubTableEntry_t entry;

    entry.hasCurrentLevelValue = false;

    saveSceneSubTableEntry(entry, tableIdx);
}

bool zapLevelControlServerAddSceneHandler(ZapClusterId_t clusterId, uint8_t tableIdx, const uint8_t * sceneData, uint8_t length)
{
    if (clusterId == ZAP_CLUSTER_LEVEL_CONTROL)
    {
        if (length < 1)
        {
            return false; // ext field format error (CurrentLevelValue byte must be present).
        }

        // Extract bytes from input data block and update scene subtable fields.
        ZapLevelControlSceneSubTableEntry_t entry = { 0 };
        uint8_t * pData                           = (uint8_t *) sceneData;

        // We only know of one extension for the Level Control cluster and it is just
        // one byte, which means we can skip some logic for this cluster.
        // If other extensions are added in this cluster, more logic will be needed here.
        entry.hasCurrentLevelValue = true;
        entry.currentLevelValue    = zapPluginScenesServerGetUint8FromBuffer(&pData);

        saveSceneSubTableEntry(entry, tableIdx);

        return true;
    }

    return false;
}

void zapLevelControlServerRecallSceneHandler(ZapEndpointId_t endpointId, uint8_t tableIdx, uint32_t transitionTime100mS)
{
    // Handles the recallScene command for the level control cluster.
    // Note- this handler presently just updates (writes) the relevant cluster
    // attribute(s), in a production system this could be replaced by a call
    // to the relevant level control command handler to actually change the
    // hw state at the rate specified by the transition time.

    ZapLevelControlSceneSubTableEntry_t entry;
    retrieveSceneSubTableEntry(entry, tableIdx);

    if (entry.hasCurrentLevelValue)
    {
        zapWriteAttribute(endpointId, &zapClusterLevelControlServerSpec, ZAP_CLUSTER_LEVEL_CONTROL_SERVER_ATTRIBUTE_CURRENT_LEVEL,
                          (uint8_t *) &entry.currentLevelValue, sizeof(entry.currentLevelValue));
    }
}

void zapLevelControlServerStoreSceneHandler(ZapEndpointId_t endpointId, uint8_t tableIdx)
{
    ZapLevelControlSceneSubTableEntry_t entry;

    entry.hasCurrentLevelValue =
        (zapReadAttribute(endpointId, &zapClusterLevelControlServerSpec, ZAP_CLUSTER_LEVEL_CONTROL_SERVER_ATTRIBUTE_CURRENT_LEVEL,
                          (uint8_t *) &entry.currentLevelValue, sizeof(entry.currentLevelValue)) == ZAP_STATUS_SUCCESS);

    saveSceneSubTableEntry(entry, tableIdx);
}

void zapLevelControlServerCopySceneHandler(uint8_t srcTableIdx, uint8_t dstTableIdx)
{
    ZapLevelControlSceneSubTableEntry_t entry;
    retrieveSceneSubTableEntry(entry, srcTableIdx);

    saveSceneSubTableEntry(entry, dstTableIdx);
}

void zapLevelControlServerViewSceneHandler(uint8_t tableIdx, uint8_t ** ppExtFldData)
{
    ZapLevelControlSceneSubTableEntry_t entry;
    retrieveSceneSubTableEntry(entry, tableIdx);

    if (entry.hasCurrentLevelValue)
    {
        zapPluginScenesServerPutUint16InBuffer(ppExtFldData, ZAP_CLUSTER_LEVEL_CONTROL);
        zapPluginScenesServerPutUint8InBuffer(ppExtFldData, 1); // length=1
        zapPluginScenesServerPutUint8InBuffer(ppExtFldData, entry.currentLevelValue);
    }
}

void zapLevelControlServerPrintInfoSceneHandler(uint8_t tableIdx)
{
    ZapLevelControlSceneSubTableEntry_t entry;
    retrieveSceneSubTableEntry(entry, tableIdx);

    zapCorePrint(" lvl:%x", entry.currentLevelValue);
}
#endif
