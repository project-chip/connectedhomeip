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

#include "scenes.h"
#include "app/util/common.h"
#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app-common/zap-generated/ids/Commands.h>
#include <app/CommandHandler.h>
#include <app/ConcreteCommandPath.h>
#include <app/util/af.h>
#include <app/util/attribute-storage-null-handling.h>
#include <app/util/config.h>
#include <app/util/error-mapping.h>

#ifdef EMBER_AF_PLUGIN_GROUPS_SERVER
#include <app/clusters/groups-server/groups-server.h>
#endif

#ifdef EMBER_AF_PLUGIN_ZLL_SCENES_SERVER
#include "../zll-scenes-server/zll-scenes-server.h"
#endif

using namespace chip;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::Scenes;
using namespace chip::app::Clusters::Scenes::Commands;

uint8_t emberAfPluginScenesServerEntriesInUse = 0;
#if !defined(EMBER_AF_PLUGIN_SCENES_USE_TOKENS) || defined(EZSP_HOST)
EmberAfSceneTableEntry emberAfPluginScenesServerSceneTable[MATTER_SCENES_TABLE_SIZE];
#endif

static bool logReadError(EmberAfStatus status, const char * attributeName)
{
    // Don't log errors for the "this cluster is not even supported" cases.
    if (status != EMBER_ZCL_STATUS_SUCCESS && status != EMBER_ZCL_STATUS_UNSUPPORTED_ENDPOINT &&
        status != EMBER_ZCL_STATUS_UNSUPPORTED_CLUSTER)
    {
        emberAfScenesClusterPrintln("ERR: %sing %s 0x%x", "read", attributeName, status);
    }
    return status == EMBER_ZCL_STATUS_SUCCESS;
}

static EmberAfStatus logWriteError(EmberAfStatus status, const char * attributeName)
{
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        emberAfScenesClusterPrintln("ERR: %sing %s 0x%x", "writ", attributeName, status);
    }
    return status;
}

bool isEndpointInGroup(chip::FabricIndex fabricIndex, EndpointId endpoint, GroupId groupId)
{
#ifdef EMBER_AF_PLUGIN_GROUPS_SERVER
    return (groupId == ZCL_SCENES_GLOBAL_SCENE_GROUP_ID ||
            emberAfGroupsClusterEndpointInGroupCallback(fabricIndex, endpoint, groupId));
#else
    return (groupId == ZCL_SCENES_GLOBAL_SCENE_GROUP_ID);
#endif // EMBER_AF_PLUGIN_GROUPS_SERVER
}

void emberAfScenesClusterServerInitCallback(EndpointId endpoint)
{
#if defined(MATTER_CLUSTER_SCENE_NAME_SUPPORT) && MATTER_CLUSTER_SCENE_NAME_SUPPORT
    {
        // The high bit of Name Support indicates whether scene names are supported.
        uint8_t nameSupport = EMBER_BIT(7);
        logWriteError(Attributes::NameSupport::Set(endpoint, nameSupport), "NameSupport");
    }
#endif
#if !defined(EMBER_AF_PLUGIN_SCENES_USE_TOKENS) || defined(EZSP_HOST)
    {
        uint8_t i;
        for (i = 0; i < MATTER_SCENES_TABLE_SIZE; i++)
        {
            EmberAfSceneTableEntry entry;
            emberAfPluginScenesServerRetrieveSceneEntry(entry, i);
            entry.endpoint = EMBER_AF_SCENE_TABLE_UNUSED_ENDPOINT_ID;
            emberAfPluginScenesServerSaveSceneEntry(entry, i);
        }
        emberAfPluginScenesServerSetNumSceneEntriesInUse(0);
    }
#endif
    emberAfScenesSetSceneCountAttribute(endpoint, emberAfPluginScenesServerNumSceneEntriesInUse());
}

EmberAfStatus emberAfScenesSetSceneCountAttribute(EndpointId endpoint, uint8_t newCount)
{
    return logWriteError(Attributes::SceneCount::Set(endpoint, newCount), "SceneCount");
}

EmberAfStatus emberAfScenesMakeValid(EndpointId endpoint, uint8_t sceneId, GroupId groupId)
{
    EmberAfStatus status;
    bool valid = true;

    // scene ID
    status = logWriteError(Attributes::CurrentScene::Set(endpoint, sceneId), "CurrentScene");
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        return status;
    }

    // group ID
    status = logWriteError(Attributes::CurrentGroup::Set(endpoint, groupId), "CurrentGroup");
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        return status;
    }

    status = logWriteError(Attributes::SceneValid::Set(endpoint, valid), "SceneValid");
    return status;
}

EmberAfStatus emberAfScenesClusterMakeInvalidCallback(EndpointId endpoint)
{
    bool valid = false;
    return logWriteError(Attributes::SceneValid::Set(endpoint, valid), "SceneValid");
}

void emAfPluginScenesServerPrintInfo()
{
    uint8_t i;
    EmberAfSceneTableEntry entry;
    emberAfCorePrintln("using 0x%x out of 0x%x table slots", emberAfPluginScenesServerNumSceneEntriesInUse(),
                       MATTER_SCENES_TABLE_SIZE);
    for (i = 0; i < MATTER_SCENES_TABLE_SIZE; i++)
    {
        emberAfPluginScenesServerRetrieveSceneEntry(entry, i);
        emberAfCorePrint("%x: ", i);
        if (entry.endpoint != EMBER_AF_SCENE_TABLE_UNUSED_ENDPOINT_ID)
        {
            emberAfCorePrint("ep %x grp %2x scene %x tt %d", entry.endpoint, entry.groupId, entry.sceneId, entry.transitionTime);
            emberAfCorePrint(".%d", entry.transitionTime100ms);
#if defined(MATTER_CLUSTER_SCENE_NAME_SUPPORT) && MATTER_CLUSTER_SCENE_NAME_SUPPORT
            emberAfCorePrint(" name(%x)\"", emberAfStringLength(entry.name));
            emberAfCorePrintString(entry.name);
            emberAfCorePrint("\"");
#endif
#ifdef ZCL_USING_ON_OFF_CLUSTER_SERVER
            emberAfCorePrint(" on/off %x", entry.onOffValue);
#endif
#ifdef ZCL_USING_LEVEL_CONTROL_CLUSTER_SERVER
            if (entry.currentLevelValue.IsNull())
            {
                emberAfCorePrint(" lvl null");
            }
            else
            {
                emberAfCorePrint(" lvl %x", entry.currentLevelValue.Value());
            }
#endif
#ifdef ZCL_USING_THERMOSTAT_CLUSTER_SERVER
            emberAfCorePrint(" therm %2x %2x %x", entry.occupiedCoolingSetpointValue, entry.occupiedHeatingSetpointValue,
                             entry.systemModeValue);
#endif
#ifdef ZCL_USING_COLOR_CONTROL_CLUSTER_SERVER
            emberAfCorePrint(" color %2x %2x", entry.currentXValue, entry.currentYValue);
            emberAfCorePrint(" %2x %x %x %x %2x %2x", entry.enhancedCurrentHueValue, entry.currentSaturationValue,
                             entry.colorLoopActiveValue, entry.colorLoopDirectionValue, entry.colorLoopTimeValue,
                             entry.colorTemperatureMiredsValue);
            emberAfCoreFlush();
#endif // ZCL_USING_COLOR_CONTROL_CLUSTER_SERVER
#ifdef ZCL_USING_DOOR_LOCK_CLUSTER_SERVER
            if (entry.lockStateValue.IsNull())
            {
                emberAfCorePrint(" door null");
            }
            else
            {
                emberAfCorePrint(" door %x", to_underlying(entry.lockStateValue.Value()));
            }
#endif
#ifdef ZCL_USING_WINDOW_COVERING_CLUSTER_SERVER
            if (!entry.currentPositionLiftPercentageValue.IsNull() && !entry.currentPositionTiltPercentageValue.IsNull())
            {
                emberAfCorePrint(" Window current percentage Lift %3u, Tilt %3u", entry.currentPositionLiftPercentageValue.Value(),
                                 entry.currentPositionTiltPercentageValue.Value());
            }
            else if (!entry.currentPositionLiftPercentageValue.IsNull())
            {
                emberAfCorePrint(" Window current percentage Lift %3u", entry.currentPositionLiftPercentageValue.Value());
            }
            else if (!entry.currentPositionTiltPercentageValue.IsNull())
            {
                emberAfCorePrint(" Window current percentage Tilt %3u", entry.currentPositionTiltPercentageValue.Value());
            }

            if (!entry.targetPositionLiftPercent100thsValue.IsNull() && !entry.targetPositionTiltPercent100thsValue.IsNull())
            {
                emberAfCorePrint(" Window target percent100ths Lift %5u, Tilt %5u",
                                 entry.targetPositionLiftPercent100thsValue.Value(),
                                 entry.targetPositionTiltPercent100thsValue.Value());
            }
            else if (!entry.targetPositionLiftPercent100thsValue.IsNull())
            {
                emberAfCorePrint(" Window target percent100ths Lift %5u", entry.targetPositionLiftPercent100thsValue.Value());
            }
            else if (!entry.targetPositionTiltPercent100thsValue.IsNull())
            {
                emberAfCorePrint(" Window target percent100ths Tilt %5u", entry.targetPositionTiltPercent100thsValue.Value());
            }
#endif
        }
        emberAfCorePrintln("%s", "");
    }
}

bool emberAfScenesClusterAddSceneCallback(app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath,
                                          const Commands::AddScene::DecodableType & commandData)
{
    auto & groupId            = commandData.groupID;
    auto & sceneId            = commandData.sceneID;
    auto & transitionTime     = commandData.transitionTime;
    auto & sceneName          = commandData.sceneName;
    auto & extensionFieldSets = commandData.extensionFieldSets;

    return emberAfPluginScenesServerParseAddScene(commandObj, commandPath, groupId, sceneId, transitionTime, sceneName,
                                                  extensionFieldSets);
}

bool emberAfScenesClusterViewSceneCallback(app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath,
                                           const Commands::ViewScene::DecodableType & commandData)
{
    auto & groupId = commandData.groupID;
    auto & sceneId = commandData.sceneID;

    return emberAfPluginScenesServerParseViewScene(commandObj, commandPath, groupId, sceneId);
}

bool emberAfScenesClusterRemoveSceneCallback(app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath,
                                             const Commands::RemoveScene::DecodableType & commandData)
{
    auto fabricIndex = commandObj->GetAccessingFabricIndex();
    auto & groupId   = commandData.groupID;
    auto & sceneId   = commandData.sceneID;

    EmberAfStatus status = EMBER_ZCL_STATUS_NOT_FOUND;
    CHIP_ERROR err       = CHIP_NO_ERROR;

    emberAfScenesClusterPrintln("RX: RemoveScene 0x%2x, 0x%x", groupId, sceneId);

    if (!isEndpointInGroup(fabricIndex, commandPath.mEndpointId, groupId))
    {
        status = EMBER_ZCL_STATUS_INVALID_COMMAND;
    }
    else
    {
        uint8_t i;
        for (i = 0; i < MATTER_SCENES_TABLE_SIZE; i++)
        {
            EmberAfSceneTableEntry entry;
            emberAfPluginScenesServerRetrieveSceneEntry(entry, i);
            if (entry.endpoint == commandPath.mEndpointId && entry.groupId == groupId && entry.sceneId == sceneId)
            {
                entry.endpoint = EMBER_AF_SCENE_TABLE_UNUSED_ENDPOINT_ID;
                emberAfPluginScenesServerSaveSceneEntry(entry, i);
                emberAfPluginScenesServerDecrNumSceneEntriesInUse();
                emberAfScenesSetSceneCountAttribute(commandPath.mEndpointId, emberAfPluginScenesServerNumSceneEntriesInUse());
                status = EMBER_ZCL_STATUS_SUCCESS;
                break;
            }
        }
    }

    app::ConcreteCommandPath path = { commandPath.mEndpointId, Scenes::Id, RemoveSceneResponse::Id };
    TLV::TLVWriter * writer       = nullptr;
    SuccessOrExit(err = commandObj->PrepareCommand(path));
    VerifyOrExit((writer = commandObj->GetCommandDataIBTLVWriter()) != nullptr, err = CHIP_ERROR_INCORRECT_STATE);
    SuccessOrExit(err = writer->Put(TLV::ContextTag(0), status));
    SuccessOrExit(err = writer->Put(TLV::ContextTag(1), groupId));
    SuccessOrExit(err = writer->Put(TLV::ContextTag(2), sceneId));
    SuccessOrExit(err = commandObj->FinishCommand());

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "Failed to encode response command.");
    }
    return true;
}

bool emberAfScenesClusterRemoveAllScenesCallback(app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath,
                                                 const Commands::RemoveAllScenes::DecodableType & commandData)
{
    auto fabricIndex = commandObj->GetAccessingFabricIndex();
    auto & groupId   = commandData.groupID;

    EmberAfStatus status = EMBER_ZCL_STATUS_INVALID_COMMAND;
    CHIP_ERROR err       = CHIP_NO_ERROR;

    emberAfScenesClusterPrintln("RX: RemoveAllScenes 0x%2x", groupId);

    if (isEndpointInGroup(fabricIndex, commandPath.mEndpointId, groupId))
    {
        uint8_t i;
        status = EMBER_ZCL_STATUS_SUCCESS;
        for (i = 0; i < MATTER_SCENES_TABLE_SIZE; i++)
        {
            EmberAfSceneTableEntry entry;
            emberAfPluginScenesServerRetrieveSceneEntry(entry, i);
            if (entry.endpoint == commandPath.mEndpointId && entry.groupId == groupId)
            {
                entry.endpoint = EMBER_AF_SCENE_TABLE_UNUSED_ENDPOINT_ID;
                emberAfPluginScenesServerSaveSceneEntry(entry, i);
                emberAfPluginScenesServerDecrNumSceneEntriesInUse();
            }
        }
        emberAfScenesSetSceneCountAttribute(commandPath.mEndpointId, emberAfPluginScenesServerNumSceneEntriesInUse());
    }

    app::ConcreteCommandPath path = { commandPath.mEndpointId, Scenes::Id, RemoveAllScenesResponse::Id };
    TLV::TLVWriter * writer       = nullptr;
    SuccessOrExit(err = commandObj->PrepareCommand(path));
    VerifyOrExit((writer = commandObj->GetCommandDataIBTLVWriter()) != nullptr, err = CHIP_ERROR_INCORRECT_STATE);
    SuccessOrExit(err = writer->Put(TLV::ContextTag(0), status));
    SuccessOrExit(err = writer->Put(TLV::ContextTag(1), groupId));
    SuccessOrExit(err = commandObj->FinishCommand());
exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "Failed to encode response command.");
    }
    return true;
}

bool emberAfScenesClusterStoreSceneCallback(app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath,
                                            const Commands::StoreScene::DecodableType & commandData)
{
    auto fabricIndex = commandObj->GetAccessingFabricIndex();
    auto & groupId   = commandData.groupID;
    auto & sceneId   = commandData.sceneID;

    EmberAfStatus status;
    CHIP_ERROR err = CHIP_NO_ERROR;
    emberAfScenesClusterPrintln("RX: StoreScene 0x%2x, 0x%x", groupId, sceneId);
    status = emberAfScenesClusterStoreCurrentSceneCallback(fabricIndex, commandPath.mEndpointId, groupId, sceneId);

    app::ConcreteCommandPath path = { commandPath.mEndpointId, Scenes::Id, StoreSceneResponse::Id };
    TLV::TLVWriter * writer       = nullptr;
    SuccessOrExit(err = commandObj->PrepareCommand(path));
    VerifyOrExit((writer = commandObj->GetCommandDataIBTLVWriter()) != nullptr, err = CHIP_ERROR_INCORRECT_STATE);
    SuccessOrExit(err = writer->Put(TLV::ContextTag(0), status));
    SuccessOrExit(err = writer->Put(TLV::ContextTag(1), groupId));
    SuccessOrExit(err = writer->Put(TLV::ContextTag(2), sceneId));
    SuccessOrExit(err = commandObj->FinishCommand());
exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "Failed to encode response command.");
    }
    return true;
}

bool emberAfScenesClusterRecallSceneCallback(app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath,
                                             const Commands::RecallScene::DecodableType & commandData)
{
    auto fabricIndex = commandObj->GetAccessingFabricIndex();
    auto & groupId   = commandData.groupID;
    auto & sceneId   = commandData.sceneID;

    // NOTE: TransitionTime field in the RecallScene command is currently
    // ignored. Per Zigbee Alliance ZCL 7 (07-5123-07):
    //
    // "The transition time determines how long the tranition takes from the
    // old cluster state to the new cluster state. It is recommended that, where
    // possible (e.g., it is not possible for attributes with Boolean type),
    // a gradual transition SHOULD take place from the old to the new state
    // over this time. However, the exact transition is manufacturer dependent."
    //
    // The manufacturer-dependent implementation here is to immediately set
    // all attributes to their scene-specified values, without regard to the
    // value of TransitionTime.

    EmberAfStatus status;
    emberAfScenesClusterPrintln("RX: RecallScene 0x%2x, 0x%x", groupId, sceneId);
    status = emberAfScenesClusterRecallSavedSceneCallback(fabricIndex, commandPath.mEndpointId, groupId, sceneId);
#ifdef EMBER_AF_PLUGIN_ZLL_SCENES_SERVER
    if (status == EMBER_ZCL_STATUS_SUCCESS)
    {
        emberAfPluginZllScenesServerRecallSceneZllExtensions(commandPath.mEndpointId);
    }
#endif
    CHIP_ERROR sendErr = commandObj->AddStatus(commandPath, app::ToInteractionModelStatus(status));
    if (CHIP_NO_ERROR != sendErr)
    {
        emberAfScenesClusterPrintln("Scenes: failed to send %s: %" CHIP_ERROR_FORMAT, "status_response", sendErr.Format());
    }
    return true;
}

bool emberAfScenesClusterGetSceneMembershipCallback(app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath,
                                                    const Commands::GetSceneMembership::DecodableType & commandData)
{
    auto fabricIndex = commandObj->GetAccessingFabricIndex();
    auto & groupId   = commandData.groupID;

    CHIP_ERROR err       = CHIP_NO_ERROR;
    EmberAfStatus status = EMBER_ZCL_STATUS_SUCCESS;
    uint8_t sceneCount   = 0;
    uint8_t sceneList[MATTER_SCENES_TABLE_SIZE];

    emberAfScenesClusterPrintln("RX: GetSceneMembership 0x%2x", groupId);

    if (!isEndpointInGroup(fabricIndex, commandPath.mEndpointId, groupId))
    {
        status = EMBER_ZCL_STATUS_INVALID_COMMAND;
    }

    if (status == EMBER_ZCL_STATUS_SUCCESS)
    {
        uint8_t i;
        for (i = 0; i < MATTER_SCENES_TABLE_SIZE; i++)
        {
            EmberAfSceneTableEntry entry;
            emberAfPluginScenesServerRetrieveSceneEntry(entry, i);
            if (entry.endpoint == commandPath.mEndpointId && entry.groupId == groupId)
            {
                sceneList[sceneCount] = entry.sceneId;
                sceneCount++;
            }
        }
        emberAfPutInt8uInResp(sceneCount);
        for (i = 0; i < sceneCount; i++)
        {
            emberAfPutInt8uInResp(sceneList[i]);
        }
    }

    {
        app::ConcreteCommandPath path = { commandPath.mEndpointId, Scenes::Id, GetSceneMembershipResponse::Id };
        TLV::TLVWriter * writer       = nullptr;
        SuccessOrExit(err = commandObj->PrepareCommand(path));
        VerifyOrExit((writer = commandObj->GetCommandDataIBTLVWriter()) != nullptr, err = CHIP_ERROR_INCORRECT_STATE);
        SuccessOrExit(err = writer->Put(TLV::ContextTag(0), status));
        SuccessOrExit(
            err = writer->Put(TLV::ContextTag(1),
                              static_cast<uint8_t>(MATTER_SCENES_TABLE_SIZE - emberAfPluginScenesServerNumSceneEntriesInUse())));
        SuccessOrExit(err = writer->Put(TLV::ContextTag(2), groupId));
        SuccessOrExit(err = writer->Put(TLV::ContextTag(3), sceneCount));
        SuccessOrExit(err = writer->Put(TLV::ContextTag(4), ByteSpan(sceneList, sceneCount)));
        SuccessOrExit(err = commandObj->FinishCommand());
    }

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "Failed to encode response command.");
    }
    return true;
}

EmberAfStatus emberAfScenesClusterStoreCurrentSceneCallback(chip::FabricIndex fabricIndex, EndpointId endpoint, GroupId groupId,
                                                            uint8_t sceneId)
{
    EmberAfSceneTableEntry entry;
    uint8_t i, index = EMBER_AF_SCENE_TABLE_NULL_INDEX;

    if (!isEndpointInGroup(fabricIndex, endpoint, groupId))
    {
        return EMBER_ZCL_STATUS_INVALID_COMMAND;
    }

    for (i = 0; i < MATTER_SCENES_TABLE_SIZE; i++)
    {
        emberAfPluginScenesServerRetrieveSceneEntry(entry, i);
        if (entry.endpoint == endpoint && entry.groupId == groupId && entry.sceneId == sceneId)
        {
            index = i;
            break;
        }
        if (index == EMBER_AF_SCENE_TABLE_NULL_INDEX && entry.endpoint == EMBER_AF_SCENE_TABLE_UNUSED_ENDPOINT_ID)
        {
            index = i;
        }
    }

    // If the target index is still zero, the table is full.
    if (index == EMBER_AF_SCENE_TABLE_NULL_INDEX)
    {
        return EMBER_ZCL_STATUS_RESOURCE_EXHAUSTED;
    }

    emberAfPluginScenesServerRetrieveSceneEntry(entry, index);

    // When creating a new entry or refreshing an existing one, the extension
    // fields are updated with the current state of other clusters on the device.
#ifdef ZCL_USING_ON_OFF_CLUSTER_SERVER
    entry.hasOnOffValue = logReadError(OnOff::Attributes::OnOff::Get(endpoint, &entry.onOffValue), "OnOff");
#endif
#ifdef ZCL_USING_LEVEL_CONTROL_CLUSTER_SERVER
    entry.hasCurrentLevelValue =
        logReadError(LevelControl::Attributes::CurrentLevel::Get(endpoint, entry.currentLevelValue), "CurrentLevel");
#endif
#ifdef ZCL_USING_THERMOSTAT_CLUSTER_SERVER
    {
        using namespace Thermostat::Attributes;
        entry.hasOccupiedCoolingSetpointValue =
            logReadError(OccupiedCoolingSetpoint::Get(endpoint, &entry.occupiedCoolingSetpointValue), "OccupiedCoolingSetpoint");
        entry.hasOccupiedHeatingSetpointValue =
            logReadError(OccupiedHeatingSetpoint::Get(endpoint, &entry.occupiedHeatingSetpointValue), "OccupiedHeatingSetpoint");
        entry.hasSystemModeValue = logReadError(SystemMode::Get(endpoint, &entry.systemModeValue), "SystemMode");
    }
#endif
#ifdef ZCL_USING_COLOR_CONTROL_CLUSTER_SERVER
    {
        using namespace ColorControl::Attributes;
        entry.hasCurrentXValue = logReadError(CurrentX::Get(endpoint, &entry.currentXValue), "CurrentX");
        entry.hasCurrentYValue = logReadError(CurrentY::Get(endpoint, &entry.currentYValue), "CurrentY");
        entry.hasEnhancedCurrentHueValue =
            logReadError(EnhancedCurrentHue::Get(endpoint, &entry.enhancedCurrentHueValue), "EnhancedCurrentHue");
        entry.hasCurrentSaturationValue =
            logReadError(CurrentSaturation::Get(endpoint, &entry.currentSaturationValue), "CurrentSaturation");
        entry.hasColorLoopActiveValue =
            logReadError(ColorLoopActive::Get(endpoint, &entry.colorLoopActiveValue), "ColorLoopActive");
        entry.hasColorLoopDirectionValue =
            logReadError(ColorLoopDirection::Get(endpoint, &entry.colorLoopDirectionValue), "ColorLoopDirection");
        entry.hasColorLoopTimeValue = logReadError(ColorLoopTime::Get(endpoint, &entry.colorLoopTimeValue), "ColorLoopTime");
        entry.hasColorTemperatureMiredsValue =
            logReadError(ColorTemperatureMireds::Get(endpoint, &entry.colorTemperatureMiredsValue), "ColorTemperatureMireds");
    }
#endif // ZCL_USING_COLOR_CONTROL_CLUSTER_SERVER
#ifdef ZCL_USING_DOOR_LOCK_CLUSTER_SERVER
    entry.hasLockStateValue = logReadError(DoorLock::Attributes::LockState::Get(endpoint, entry.lockStateValue), "LockState");
#endif
#ifdef ZCL_USING_WINDOW_COVERING_CLUSTER_SERVER
    {
        using namespace WindowCovering::Attributes;
        entry.hasCurrentPositionLiftPercentageValue =
            logReadError(CurrentPositionLiftPercentage::Get(endpoint, entry.currentPositionLiftPercentageValue),
                         "CurrentPositionLiftPercentage");
        entry.hasCurrentPositionTiltPercentageValue =
            logReadError(CurrentPositionTiltPercentage::Get(endpoint, entry.currentPositionTiltPercentageValue),
                         "CurrentPositionTiltPercentage");
        entry.hasTargetPositionLiftPercent100thsValue =
            logReadError(TargetPositionLiftPercent100ths::Get(endpoint, entry.targetPositionLiftPercent100thsValue),
                         "TragetPositionLiftPercent100ths");
        entry.hasTargetPositionTiltPercent100thsValue =
            logReadError(TargetPositionTiltPercent100ths::Get(endpoint, entry.targetPositionTiltPercent100thsValue),
                         "TragetPositionTiltPercent100ths");
    }
#endif

    // When creating a new entry, the name is set to the null string (i.e., the
    // length is set to zero) and the transition time is set to zero.  The scene
    // count must be increased and written to the attribute table when adding a
    // new scene.  Otherwise, these fields and the count are left alone.
    if (i != index)
    {
        entry.endpoint = endpoint;
        entry.groupId  = groupId;
        entry.sceneId  = sceneId;
#if defined(MATTER_CLUSTER_SCENE_NAME_SUPPORT) && MATTER_CLUSTER_SCENE_NAME_SUPPORT
        entry.name[0] = 0;
#endif
        entry.transitionTime      = 0;
        entry.transitionTime100ms = 0;
        emberAfPluginScenesServerIncrNumSceneEntriesInUse();
        emberAfScenesSetSceneCountAttribute(endpoint, emberAfPluginScenesServerNumSceneEntriesInUse());
    }

    // Save the scene entry and mark is as valid by storing its scene and group
    // ids in the attribute table and setting valid to true.
    emberAfPluginScenesServerSaveSceneEntry(entry, index);
    emberAfScenesMakeValid(endpoint, sceneId, groupId);
    return EMBER_ZCL_STATUS_SUCCESS;
}

EmberAfStatus emberAfScenesClusterRecallSavedSceneCallback(chip::FabricIndex fabricIndex, EndpointId endpoint, GroupId groupId,
                                                           uint8_t sceneId)
{
    if (!isEndpointInGroup(fabricIndex, endpoint, groupId))
    {
        return EMBER_ZCL_STATUS_INVALID_COMMAND;
    }

    uint8_t i;
    for (i = 0; i < MATTER_SCENES_TABLE_SIZE; i++)
    {
        EmberAfSceneTableEntry entry;
        emberAfPluginScenesServerRetrieveSceneEntry(entry, i);
        if (entry.endpoint == endpoint && entry.groupId == groupId && entry.sceneId == sceneId)
        {
#ifdef ZCL_USING_ON_OFF_CLUSTER_SERVER
            if (entry.hasOnOffValue)
            {
                logWriteError(OnOff::Attributes::OnOff::Set(endpoint, entry.onOffValue), "OnOff");
            }
#endif
#ifdef ZCL_USING_LEVEL_CONTROL_CLUSTER_SERVER
            if (entry.hasCurrentLevelValue)
            {
                logWriteError(LevelControl::Attributes::CurrentLevel::Set(endpoint, entry.currentLevelValue), "CurrentLevel");
            }
#endif
#ifdef ZCL_USING_THERMOSTAT_CLUSTER_SERVER
            {
                using namespace Thermostat::Attributes;
                if (entry.hasOccupiedCoolingSetpointValue)
                {
                    logWriteError(OccupiedCoolingSetpoint::Set(endpoint, entry.occupiedCoolingSetpointValue),
                                  "OccupiedCoolingSetpoint");
                }
                if (entry.hasOccupiedHeatingSetpointValue)
                {
                    logWriteError(OccupiedHeatingSetpoint::Set(endpoint, entry.occupiedHeatingSetpointValue),
                                  "OccupiedHeatingSetpoint");
                }
                if (entry.hasSystemModeValue)
                {
                    logWriteError(SystemMode::Set(endpoint, entry.systemModeValue), "SystemMode");
                }
            }
#endif
#ifdef ZCL_USING_COLOR_CONTROL_CLUSTER_SERVER
            {
                using namespace ColorControl::Attributes;
                if (entry.hasCurrentXValue)
                {
                    logWriteError(CurrentX::Set(endpoint, entry.currentXValue), "CurrentX");
                }
                if (entry.hasCurrentYValue)
                {
                    logWriteError(CurrentY::Set(endpoint, entry.currentXValue), "CurrentY");
                }

                if (entry.hasEnhancedCurrentHueValue)
                {
                    logWriteError(EnhancedCurrentHue::Set(endpoint, entry.enhancedCurrentHueValue), "EnhancedCurrentHue");
                }
                if (entry.hasCurrentSaturationValue)
                {
                    logWriteError(CurrentSaturation::Set(endpoint, entry.currentSaturationValue), "CurrentSaturation");
                }
                if (entry.hasColorLoopActiveValue)
                {
                    logWriteError(ColorLoopActive::Set(endpoint, entry.colorLoopActiveValue), "ColorLoopActive");
                }
                if (entry.hasColorLoopDirectionValue)
                {
                    logWriteError(ColorLoopDirection::Set(endpoint, entry.colorLoopDirectionValue), "ColorLoopDirection");
                }
                if (entry.hasColorLoopTimeValue)
                {
                    logWriteError(ColorLoopTime::Set(endpoint, entry.colorLoopTimeValue), "ColorLoopTime");
                }
                if (entry.hasColorTemperatureMiredsValue)
                {
                    logWriteError(ColorTemperatureMireds::Set(endpoint, entry.colorTemperatureMiredsValue),
                                  "ColorTemperatureMireds");
                }
            }
#endif // ZCL_USING_COLOR_CONTROL_CLUSTER_SERVER
#ifdef ZCL_USING_DOOR_LOCK_CLUSTER_SERVER
            if (entry.hasLockStateValue)
            {
                logWriteError(DoorLock::Attributes::LockState::Set(endpoint, entry.lockStateValue), "LockState");
            }
#endif
#ifdef ZCL_USING_WINDOW_COVERING_CLUSTER_SERVER
            {
                using namespace WindowCovering::Attributes;
                if (entry.hasCurrentPositionLiftPercentageValue)
                {
                    logWriteError(CurrentPositionLiftPercentage::Set(endpoint, entry.currentPositionLiftPercentageValue),
                                  "CurrentPositionLiftPercentage");
                }
                if (entry.hasCurrentPositionTiltPercentageValue)
                {
                    logWriteError(CurrentPositionTiltPercentage::Set(endpoint, entry.currentPositionTiltPercentageValue),
                                  "CurrentPositionTiltPercentage");
                }
                if (entry.hasTargetPositionLiftPercent100thsValue)
                {
                    logWriteError(TargetPositionLiftPercent100ths::Set(endpoint, entry.targetPositionLiftPercent100thsValue),
                                  "TargetPositionLiftPercent100ths");
                }
                if (entry.hasTargetPositionTiltPercent100thsValue)
                {
                    logWriteError(TargetPositionTiltPercent100ths::Set(endpoint, entry.targetPositionTiltPercent100thsValue),
                                  "TargetPositionTiltPercent100ths");
                }
            }
#endif
            emberAfScenesMakeValid(endpoint, sceneId, groupId);
            return EMBER_ZCL_STATUS_SUCCESS;
        }
    }

    return EMBER_ZCL_STATUS_NOT_FOUND;
}

template <typename T>
struct NullableUnderlyingType
{
};

template <typename T>
struct NullableUnderlyingType<app::DataModel::Nullable<T>>
{
    using Type = T;
};

bool emberAfPluginScenesServerParseAddScene(
    app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath, GroupId groupId, uint8_t sceneId,
    uint16_t transitionTime, const CharSpan & sceneName,
    const app::DataModel::DecodableList<Structs::ExtensionFieldSet::DecodableType> & extensionFieldSets)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    EmberAfSceneTableEntry entry;
    EmberAfStatus status;
    bool enhanced       = (commandPath.mCommandId == EnhancedAddScene::Id);
    auto fabricIndex    = commandObj->GetAccessingFabricIndex();
    EndpointId endpoint = commandPath.mEndpointId;
    uint8_t i, index = EMBER_AF_SCENE_TABLE_NULL_INDEX;

    emberAfScenesClusterPrintln("RX: %pAddScene 0x%2x, 0x%x, 0x%2x, \"%.*s\"", (enhanced ? "Enhanced" : ""), groupId, sceneId,
                                transitionTime, static_cast<int>(sceneName.size()), sceneName.data());

    auto fieldSetIter = extensionFieldSets.begin();

    // Add Scene commands can only reference groups to which we belong.
    if (!isEndpointInGroup(fabricIndex, endpoint, groupId))
    {
        status = EMBER_ZCL_STATUS_INVALID_COMMAND;
        goto kickout;
    }

    for (i = 0; i < MATTER_SCENES_TABLE_SIZE; i++)
    {
        emberAfPluginScenesServerRetrieveSceneEntry(entry, i);
        if (entry.endpoint == endpoint && entry.groupId == groupId && entry.sceneId == sceneId)
        {
            index = i;
            break;
        }
        if (index == EMBER_AF_SCENE_TABLE_NULL_INDEX && entry.endpoint == EMBER_AF_SCENE_TABLE_UNUSED_ENDPOINT_ID)
        {
            index = i;
        }
    }

    // If the target index is still zero, the table is full.
    if (index == EMBER_AF_SCENE_TABLE_NULL_INDEX)
    {
        status = EMBER_ZCL_STATUS_RESOURCE_EXHAUSTED;
        goto kickout;
    }

    emberAfPluginScenesServerRetrieveSceneEntry(entry, index);

    // The transition time is specified in seconds in the regular version of the
    // command and tenths of a second in the enhanced version.
    if (enhanced)
    {
        entry.transitionTime      = transitionTime / 10;
        entry.transitionTime100ms = (uint8_t)(transitionTime - entry.transitionTime * 10);
    }
    else
    {
        entry.transitionTime      = transitionTime;
        entry.transitionTime100ms = 0;
    }

#if defined(MATTER_CLUSTER_SCENE_NAME_SUPPORT) && MATTER_CLUSTER_SCENE_NAME_SUPPORT
    emberAfCopyString(entry.name, Uint8::from_const_char(sceneName.data()), ZCL_SCENES_CLUSTER_MAXIMUM_NAME_LENGTH);
#endif

    // When adding a new scene, wipe out all of the extensions before parsing the
    // extension field sets data.
    if (i != index)
    {
#ifdef ZCL_USING_ON_OFF_CLUSTER_SERVER
        entry.hasOnOffValue = false;
#endif
#ifdef ZCL_USING_LEVEL_CONTROL_CLUSTER_SERVER
        entry.hasCurrentLevelValue = false;
#endif
#ifdef ZCL_USING_THERMOSTAT_CLUSTER_SERVER
        entry.hasOccupiedCoolingSetpointValue = false;
        entry.hasOccupiedHeatingSetpointValue = false;
        entry.hasSystemModeValue              = false;
#endif
#ifdef ZCL_USING_COLOR_CONTROL_CLUSTER_SERVER
        entry.hasCurrentXValue               = false;
        entry.hasCurrentYValue               = false;
        entry.hasEnhancedCurrentHueValue     = false;
        entry.hasCurrentSaturationValue      = false;
        entry.hasColorLoopActiveValue        = false;
        entry.hasColorLoopDirectionValue     = false;
        entry.hasColorLoopTimeValue          = false;
        entry.hasColorTemperatureMiredsValue = false;
#endif // ZCL_USING_COLOR_CONTROL_CLUSTER_SERVER
#ifdef ZCL_USING_DOOR_LOCK_CLUSTER_SERVER
        entry.hasLockStateValue = false;
#endif
#ifdef ZCL_USING_WINDOW_COVERING_CLUSTER_SERVER
        entry.hasCurrentPositionLiftPercentageValue   = false;
        entry.hasCurrentPositionTiltPercentageValue   = false;
        entry.hasTargetPositionLiftPercent100thsValue = false;
        entry.hasTargetPositionTiltPercent100thsValue = false;
#endif
    }

    while (fieldSetIter.Next())
    {
        auto & fieldSet = fieldSetIter.GetValue();

        ClusterId clusterId = fieldSet.clusterID;

        // TODO: We need to encode scene field sets in TLV.
        // https://github.com/project-chip/connectedhomeip/issues/10334
        switch (clusterId)
        {
#if 0
#ifdef ZCL_USING_ON_OFF_CLUSTER_SERVER
        case OnOff::Id:
            // We only know of one extension for the On/Off cluster and it is just one
            // byte, which means we can skip some logic for this cluster.  If other
            // extensions are added in this cluster, more logic will be needed here.
            entry.hasOnOffValue = true;
            entry.onOffValue    = emberAfGetInt8u(extensionFieldSets, extensionFieldSetsIndex, extensionFieldSetsLen);
            break;
#endif
#ifdef ZCL_USING_LEVEL_CONTROL_CLUSTER_SERVER
        case LevelControl::Id: {
            // We only know of one extension for the Level Control cluster and it is
            // just one byte, which means we can skip some logic for this cluster.  If
            // other extensions are added in this cluster, more logic will be needed
            // here.
            entry.hasCurrentLevelValue = true;
            using Traits = NumericAttributeTraits<NullableUnderlyingType<decltype(entry.currentLevelValue)>::Type>;
            // TODO: This is not right; we should be getting TLV values here or something!
            Traits::StorageType storedValue = emberAfGetInt8u(extensionFieldSets, extensionFieldSetsIndex, extensionFieldSetsLen);
            if (Traits::IsNullValue(storedValue)) {
                entry.currentLevelValue.SetNull();
            } else {
                entry.currentLevelValue.SetValue(storedValue);
            }
            break;
        }
#endif
#ifdef ZCL_USING_THERMOSTAT_CLUSTER_SERVER
        case Thermostat::Id:
            if (length < 2)
            {
                break;
            }
            entry.hasOccupiedCoolingSetpointValue = true;
            entry.occupiedCoolingSetpointValue =
                (int16_t) emberAfGetInt16u(extensionFieldSets, extensionFieldSetsIndex, extensionFieldSetsLen);
            extensionFieldSetsIndex = static_cast<uint16_t>(extensionFieldSetsIndex + 2);
            length                  = static_cast<uint8_t>(length - 2);
            if (length < 2)
            {
                break;
            }
            entry.hasOccupiedHeatingSetpointValue = true;
            entry.occupiedHeatingSetpointValue =
                (int16_t) emberAfGetInt16u(extensionFieldSets, extensionFieldSetsIndex, extensionFieldSetsLen);
            extensionFieldSetsIndex = static_cast<uint16_t>(extensionFieldSetsIndex + 2);
            length                  = static_cast<uint8_t>(length - 2);
            if (length < 1)
            {
                break;
            }
            entry.hasSystemModeValue = true;
            entry.systemModeValue    = emberAfGetInt8u(extensionFieldSets, extensionFieldSetsIndex, extensionFieldSetsLen);
            // If additional Thermostat extensions are added, adjust the index and
            // length variables here.
            break;
#endif
#ifdef ZCL_USING_COLOR_CONTROL_CLUSTER_SERVER
        case ColorControl::Id:
            if (length < 2)
            {
                break;
            }
            entry.hasCurrentXValue  = true;
            entry.currentXValue     = emberAfGetInt16u(extensionFieldSets, extensionFieldSetsIndex, extensionFieldSetsLen);
            extensionFieldSetsIndex = static_cast<uint16_t>(extensionFieldSetsIndex + 2);
            length                  = static_cast<uint8_t>(length - 2);
            if (length < 2)
            {
                break;
            }
            entry.hasCurrentYValue = true;
            entry.currentYValue    = emberAfGetInt16u(extensionFieldSets, extensionFieldSetsIndex, extensionFieldSetsLen);
            if (enhanced)
            {
                extensionFieldSetsIndex = static_cast<uint16_t>(extensionFieldSetsIndex + 2);
                length                  = static_cast<uint8_t>(length - 2);
                ;
                if (length < 2)
                {
                    break;
                }
                entry.hasEnhancedCurrentHueValue = true;
                entry.enhancedCurrentHueValue =
                    emberAfGetInt16u(extensionFieldSets, extensionFieldSetsIndex, extensionFieldSetsLen);
                extensionFieldSetsIndex = static_cast<uint16_t>(extensionFieldSetsIndex + 2);
                length                  = static_cast<uint8_t>(length - 2);
                if (length < 1)
                {
                    break;
                }
                entry.hasCurrentSaturationValue = true;
                entry.currentSaturationValue = emberAfGetInt8u(extensionFieldSets, extensionFieldSetsIndex, extensionFieldSetsLen);
                extensionFieldSetsIndex++;
                length--;
                if (length < 1)
                {
                    break;
                }
                entry.hasColorLoopActiveValue = true;
                entry.colorLoopActiveValue    = emberAfGetInt8u(extensionFieldSets, extensionFieldSetsIndex, extensionFieldSetsLen);
                extensionFieldSetsIndex++;
                length--;
                if (length < 1)
                {
                    break;
                }
                entry.hasColorLoopDirectionValue = true;
                entry.colorLoopDirectionValue = emberAfGetInt8u(extensionFieldSets, extensionFieldSetsIndex, extensionFieldSetsLen);
                extensionFieldSetsIndex++;
                length--;
                if (length < 2)
                {
                    break;
                }
                entry.hasColorLoopTimeValue = true;
                entry.colorLoopTimeValue    = emberAfGetInt16u(extensionFieldSets, extensionFieldSetsIndex, extensionFieldSetsLen);
                extensionFieldSetsIndex     = static_cast<uint16_t>(extensionFieldSetsIndex + 2);
                length                      = static_cast<uint8_t>(length - 2);
                if (length < 2)
                {
                    break;
                }
                entry.hasColorTemperatureMiredsValue = true;
                entry.colorTemperatureMiredsValue =
                    emberAfGetInt16u(extensionFieldSets, extensionFieldSetsIndex, extensionFieldSetsLen);
            }
            // If additional Color Control extensions are added, adjust the index and
            // length variables here.
            break;
#endif // ZCL_USING_COLOR_CONTROL_CLUSTER_SERVER
#ifdef ZCL_USING_DOOR_LOCK_CLUSTER_SERVER
        case DoorLock::Id: {
            // We only know of one extension for the Door Lock cluster and it is just
            // one byte, which means we can skip some logic for this cluster.  If
            // other extensions are added in this cluster, more logic will be needed
            // here.
            entry.hasLockStateValue = true;
            using Traits = NumericAttributeTraits<NullableUnderlyingType<decltype(entry.lockStateValue)>::Type>;
            // TODO: This is not right; we should be getting TLV values here or something!
            Traits::StorageType storedValue = emberAfGetInt8u(extensionFieldSets, extensionFieldSetsIndex, extensionFieldSetsLen);
            if (Traits::IsNullValue(storedValue)) {
                entry.lockStateValue.SetNull();
            } else {
                entry.lockStateValue.SetValue(storedValue);
            }
            break;
        }
#endif
#ifdef ZCL_USING_WINDOW_COVERING_CLUSTER_SERVER
        case WindowCovering::Id:
            // If we're here, we know we have at least one byte, so we can skip the
            // length check for the first field.
            {
                entry.hasCurrentPositionLiftPercentageValue = true;
                using Traits = NumericAttributeTraits<NullableUnderlyingType<decltype(entry.currentPositionLiftPercentageValue)>::Type>;
                // TODO: This is not right; we should be getting TLV values here or something!
                Traits::StorageType storedValue =
                    emberAfGetInt8u(extensionFieldSets, extensionFieldSetsIndex, extensionFieldSetsLen);
                if (Traits::IsNullValue(storedValue)){
                    entry.currentPositionLiftPercentageValue.SetNull();
                } else {
                    entry.currentPositionLiftPercentageValue.SetValue(storedValue);
                }
            }

            extensionFieldSetsIndex++;
            length--;
            if (length < 1)
            {
                break;
            }

            {
                entry.hasCurrentPositionTiltPercentageValue = true;
                using Traits = NumericAttributeTraits<NullableUnderlyingType<decltype(entry.currentPositionTiltPercentageValue)>::Type>;
                // TODO: This is not right; we should be getting TLV values here or something!
                Traits::StorageType storedValue =
                    emberAfGetInt8u(extensionFieldSets, extensionFieldSetsIndex, extensionFieldSetsLen);
                if (Traits::IsNullValue(storedValue)){
                    entry.currentPositionTiltPercentageValue.SetNull();
                } else {
                    entry.currentPositionTiltPercentageValue.SetValue(storedValue);
                }
            }

            extensionFieldSetsIndex++;
            length--;
            if (length < 2)
            {
                break;
            }

            {
                entry.hasTargetPositionLiftPercent100thsValue = true;
                using Traits = NumericAttributeTraits<NullableUnderlyingType<decltype(entry.targetPositionLiftPercent100thsValue)>::Type>;
                // TODO: This is not right; we should be getting TLV values here or something!
                Traits::StorageType storedValue =
                    emberAfGetInt8u(extensionFieldSets, extensionFieldSetsIndex, extensionFieldSetsLen);
                if (Traits::IsNullValue(storedValue)){
                    entry.targetPositionLiftPercent100thsValue.SetNull();
                } else {
                    entry.targetPositionLiftPercent100thsValue.SetValue(storedValue);
                }
            }

            extensionFieldSetsIndex = static_cast<uint16_t>(extensionFieldSetsIndex + 2);
            length                  = static_cast<uint8_t>(length - 2);
            if (length < 2)
            {
                break;
            }

            {
                entry.hasTargetPositionTiltPercent100thsValue = true;
                using Traits = NumericAttributeTraits<NullableUnderlyingType<decltype(entry.targetPositionTiltPercent100thsValue)>::Type>;
                // TODO: This is not right; we should be getting TLV values here or something!
                Traits::StorageType storedValue =
                    emberAfGetInt8u(extensionFieldSets, extensionFieldSetsIndex, extensionFieldSetsLen);
                if (Traits::IsNullValue(storedValue)){
                    entry.targetPositionTiltPercent100thsValue.SetNull();
                } else {
                    entry.targetPositionTiltPercent100thsValue.SetValue(storedValue);
                }
            }
            // If additional Window Covering extensions are added, adjust the index
            // and length variables here.
            break;
#endif
#endif // if 0 disabling all the code.
        default:
            break;
        }
    }

    if (fieldSetIter.GetStatus() != CHIP_NO_ERROR)
    {
        status = EMBER_ZCL_STATUS_MALFORMED_COMMAND;
        goto kickout;
    }

    // If we got this far, we either added a new entry or updated an existing one.
    // If we added, store the basic data and increment the scene count.  In either
    // case, save the entry.
    if (i != index)
    {
        entry.endpoint = endpoint;
        entry.groupId  = groupId;
        entry.sceneId  = sceneId;
        emberAfPluginScenesServerIncrNumSceneEntriesInUse();
        emberAfScenesSetSceneCountAttribute(endpoint, emberAfPluginScenesServerNumSceneEntriesInUse());
    }
    emberAfPluginScenesServerSaveSceneEntry(entry, index);
    status = EMBER_ZCL_STATUS_SUCCESS;

kickout:

    app::ConcreteCommandPath path = { commandPath.mEndpointId, Scenes::Id, AddSceneResponse::Id };
    if (enhanced)
    {
        path = { commandPath.mEndpointId, Scenes::Id, EnhancedAddSceneResponse::Id };
    }
    TLV::TLVWriter * writer = nullptr;
    SuccessOrExit(err = commandObj->PrepareCommand(path));
    VerifyOrExit((writer = commandObj->GetCommandDataIBTLVWriter()) != nullptr, err = CHIP_ERROR_INCORRECT_STATE);
    SuccessOrExit(err = writer->Put(TLV::ContextTag(0), status));
    SuccessOrExit(err = writer->Put(TLV::ContextTag(1), groupId));
    SuccessOrExit(err = writer->Put(TLV::ContextTag(2), sceneId));
    SuccessOrExit(err = commandObj->FinishCommand());

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "Failed to encode response command.");
    }
    return true;
}

bool emberAfPluginScenesServerParseViewScene(app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath,
                                             GroupId groupId, uint8_t sceneId)
{
    CHIP_ERROR err               = CHIP_NO_ERROR;
    EmberAfSceneTableEntry entry = {};
    EmberAfStatus status         = EMBER_ZCL_STATUS_NOT_FOUND;
    bool enhanced                = (commandPath.mCommandId == EnhancedViewScene::Id);
    FabricIndex fabricIndex      = commandObj->GetAccessingFabricIndex();
    EndpointId endpoint          = commandPath.mEndpointId;

    emberAfScenesClusterPrintln("RX: %pViewScene 0x%2x, 0x%x", (enhanced ? "Enhanced" : ""), groupId, sceneId);

    // View Scene commands can only reference groups which we belong to.
    if (!isEndpointInGroup(fabricIndex, endpoint, groupId))
    {
        status = EMBER_ZCL_STATUS_INVALID_COMMAND;
    }
    else
    {
        uint8_t i;
        for (i = 0; i < MATTER_SCENES_TABLE_SIZE; i++)
        {
            emberAfPluginScenesServerRetrieveSceneEntry(entry, i);
            if (entry.endpoint == endpoint && entry.groupId == groupId && entry.sceneId == sceneId)
            {
                status = EMBER_ZCL_STATUS_SUCCESS;
                break;
            }
        }
    }

    // The status, group id, and scene id are always included in the response, but
    // the transition time, name, and extension fields are only included if the
    // scene was found.
    app::ConcreteCommandPath path = { commandPath.mEndpointId, Scenes::Id, ViewSceneResponse::Id };
    if (enhanced)
    {
        path = { commandPath.mEndpointId, Scenes::Id, EnhancedViewSceneResponse::Id };
    }
    TLV::TLVWriter * writer = nullptr;
    SuccessOrExit(err = commandObj->PrepareCommand(path));
    VerifyOrExit((writer = commandObj->GetCommandDataIBTLVWriter()) != nullptr, err = CHIP_ERROR_INCORRECT_STATE);
    SuccessOrExit(err = writer->Put(TLV::ContextTag(0), status));
    SuccessOrExit(err = writer->Put(TLV::ContextTag(1), groupId));
    SuccessOrExit(err = writer->Put(TLV::ContextTag(2), sceneId));
    SuccessOrExit(err = writer->Put(TLV::ContextTag(3),
                                    static_cast<uint16_t>(enhanced ? entry.transitionTime * 10 + entry.transitionTime100ms
                                                                   : entry.transitionTime)));
#if defined(MATTER_CLUSTER_SCENE_NAME_SUPPORT) && MATTER_CLUSTER_SCENE_NAME_SUPPORT
    SuccessOrExit(err = writer->Put(TLV::ContextTag(4), entry.name));
#else
    SuccessOrExit(err = writer->PutString(TLV::ContextTag(4), ""));
#endif
    // #6620: Need the build the array for response.
    SuccessOrExit(err = writer->Put(TLV::ContextTag(5), ByteSpan(nullptr, 0)));
    SuccessOrExit(err = commandObj->FinishCommand());

    /*
        if (status == EMBER_ZCL_STATUS_SUCCESS)
        {
            // The transition time is returned in seconds in the regular version of the
            // command and tenths of a second in the enhanced version.
            emberAfPutInt16uInResp(
                static_cast<uint16_t>(enhanced ? entry.transitionTime * 10 + entry.transitionTime100ms : entry.transitionTime));
    #if defined(MATTER_CLUSTER_SCENE_NAME_SUPPORT) && MATTER_CLUSTER_SCENE_NAME_SUPPORT
            emberAfPutStringInResp(entry.name);
    #else
            emberAfPutInt8uInResp(0); // name length
    #endif
    #ifdef ZCL_USING_ON_OFF_CLUSTER_SERVER
            if (entry.hasOnOffValue)
            {
                emberAfPutInt16uInResp(OnOff::Id);
                emberAfPutInt8uInResp(1); // length
                emberAfPutInt8uInResp(entry.onOffValue);
            }
    #endif
    #ifdef ZCL_USING_LEVEL_CONTROL_CLUSTER_SERVER
            if (entry.hasCurrentLevelValue)
            {
                emberAfPutInt16uInResp(LevelControl::Id);
                emberAfPutInt8uInResp(1); // length
                emberAfPutInt8uInResp(entry.currentLevelValue);
            }
    #endif
    #ifdef ZCL_USING_THERMOSTAT_CLUSTER_SERVER
            if (entry.hasOccupiedCoolingSetpointValue)
            {
                uint8_t * length;
                emberAfPutInt16uInResp(Thermostat::Id);
                length = &appResponseData[appResponseLength];
                emberAfPutInt8uInResp(0); // temporary length
                emberAfPutInt16sInResp(entry.occupiedCoolingSetpointValue);
                *length += 2;
                if (entry.hasOccupiedHeatingSetpointValue)
                {
                    emberAfPutInt16sInResp(entry.occupiedHeatingSetpointValue);
                    *length += 2;
                    if (entry.hasSystemModeValue)
                    {
                        emberAfPutInt8uInResp(entry.systemModeValue);
                        (*length)++;
                    }
                }
            }
    #endif
    #ifdef ZCL_USING_COLOR_CONTROL_CLUSTER_SERVER
            if (entry.hasCurrentXValue)
            {
                uint8_t * length;
                emberAfPutInt16uInResp(ColorControl::Id);
                length = &appResponseData[appResponseLength];
                emberAfPutInt8uInResp(0); // temporary length
                emberAfPutInt16uInResp(entry.currentXValue);
                *length = static_cast<uint8_t>(*length + 2);
                if (entry.hasCurrentYValue)
                {
                    emberAfPutInt16uInResp(entry.currentYValue);
                    *length = static_cast<uint8_t>(*length + 2);
                    if (enhanced)
                    {
                        if (entry.hasEnhancedCurrentHueValue)
                        {
                            emberAfPutInt16uInResp(entry.enhancedCurrentHueValue);
                            *length = static_cast<uint8_t>(*length + 2);
                            if (entry.hasCurrentSaturationValue)
                            {
                                emberAfPutInt8uInResp(entry.currentSaturationValue);
                                (*length)++;
                                if (entry.hasColorLoopActiveValue)
                                {
                                    emberAfPutInt8uInResp(entry.colorLoopActiveValue);
                                    (*length)++;
                                    if (entry.hasColorLoopDirectionValue)
                                    {
                                        emberAfPutInt8uInResp(entry.colorLoopDirectionValue);
                                        (*length)++;
                                        if (entry.hasColorLoopTimeValue)
                                        {
                                            emberAfPutInt16uInResp(entry.colorLoopTimeValue);
                                            *length = static_cast<uint8_t>(*length + 2);
                                            if (entry.hasColorTemperatureMiredsValue)
                                            {
                                                emberAfPutInt16uInResp(entry.colorTemperatureMiredsValue);
                                                *length = static_cast<uint8_t>(*length + 2);
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
    #endif // ZCL_USING_COLOR_CONTROL_CLUSTER_SERVER
    #ifdef ZCL_USING_DOOR_LOCK_CLUSTER_SERVER
            if (entry.hasLockStateValue)
            {
                emberAfPutInt16uInResp(DoorLock::Id);
                emberAfPutInt8uInResp(1); // length
                emberAfPutInt8uInResp(entry.lockStateValue);
            }
    #endif
    #ifdef ZCL_USING_WINDOW_COVERING_CLUSTER_SERVER
            if (entry.hasCurrentPositionLiftPercentageValue)
            {
                uint8_t * length;
                emberAfPutInt16uInResp(WindowCovering::Id);
                length = &appResponseData[appResponseLength];
                emberAfPutInt8uInResp(0); // temporary length
                emberAfPutInt8uInResp(entry.currentPositionLiftPercentageValue);
                (*length)++;
                if (entry.hasCurrentPositionTiltPercentageValue)
                {
                    emberAfPutInt8uInResp(entry.currentPositionTiltPercentageValue);
                    (*length)++;
                    if (entry.hasTargetPositionLiftPercent100thsValue)
                    {
                        emberAfPutInt16uInResp(entry.targetPositionLiftPercent100thsValue);
                        *length = static_cast<uint8_t>(*length + 2);
                        if (entry.hasTargetPositionTiltPercent100thsValue)
                        {
                            emberAfPutInt16uInResp(entry.targetPositionTiltPercent100thsValue);
                            *length = static_cast<uint8_t>(*length + 2);
                        }
                    }
                }
            }
    #endif
        }

    sendStatus = emberAfSendResponse();
    if (EMBER_SUCCESS != sendStatus)
    {
        emberAfScenesClusterPrintln("Scenes: failed to send %s response: 0x%x", "view_scene", sendStatus);
    }
    */

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "Failed to encode response command.");
    }
    return true;
}

void emberAfScenesClusterRemoveScenesInGroupCallback(EndpointId endpoint, GroupId groupId)
{
    uint8_t i;
    for (i = 0; i < MATTER_SCENES_TABLE_SIZE; i++)
    {
        EmberAfSceneTableEntry entry;
        emberAfPluginScenesServerRetrieveSceneEntry(entry, i);
        if (entry.endpoint == endpoint && entry.groupId == groupId)
        {
            entry.groupId  = ZCL_SCENES_GLOBAL_SCENE_GROUP_ID;
            entry.endpoint = EMBER_AF_SCENE_TABLE_UNUSED_ENDPOINT_ID;
            emberAfPluginScenesServerSaveSceneEntry(entry, i);
            emberAfPluginScenesServerDecrNumSceneEntriesInUse();
            emberAfScenesSetSceneCountAttribute(endpoint, emberAfPluginScenesServerNumSceneEntriesInUse());
        }
    }
}

void MatterScenesPluginServerInitCallback() {}
