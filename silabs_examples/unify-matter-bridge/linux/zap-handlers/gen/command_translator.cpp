/*******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 ******************************************************************************/
#define CHIP_USE_ENUM_CLASS_FOR_IM_ENUM

#include "command_translator.hpp"
#include "chip_types_to_json.hpp"
#include "sl_log.h"
#include <iostream>
#include <sstream>
#include <string>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace unify::matter_bridge;

#define LOG_TAG "matter_cluster_command_server"

void IdentifyClusterCommandHandler::InvokeCommand(CommandHandlerInterface::HandlerContext & ctxt)
{
    using namespace chip::app::Clusters::Identify;

    auto unify_node = m_node_state_monitor.bridged_endpoint(ctxt.mRequestPath.mEndpointId);
    if (!unify_node)
    {
        sl_log_info(LOG_TAG, "The endpoint [%i] is not a part of unify matter bridge node", ctxt.mRequestPath.mEndpointId);
        return;
    }

    std::string cmd;
    nlohmann::json payload = {};

    switch (ctxt.mRequestPath.mCommandId)
    {
    case Commands::Identify::Id: {
        cmd = "Identify";
        Commands::Identify::DecodableType data;
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR)
        {
            try
            {
                payload["IdentifyTime"] = to_json(data.identifyTime);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
        }
    }
    break;
    case Commands::TriggerEffect::Id: {
        cmd = "TriggerEffect";
        Commands::TriggerEffect::DecodableType data;
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR)
        {
            try
            {
                payload["EffectIdentifier"] = to_json(data.effectIdentifier);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try
            {
                payload["EffectVariant"] = to_json(data.effectVariant);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
        }
    }
    break;
    }

    if (!cmd.empty())
    {
        ctxt.mCommandHandler.AddStatus(ctxt.mRequestPath, Protocols::InteractionModel::Status::Success);
        send_unify_mqtt_cmd(ctxt, cmd, payload);
        sl_log_debug(LOG_TAG, "Mapped [%] command to unify dotdot data model", cmd.c_str());
    }
    else
    {
        ctxt.mCommandHandler.AddStatus(ctxt.mRequestPath, Protocols::InteractionModel::Status::UnsupportedCommand);
    }
    ctxt.SetCommandHandled();
}

void ScenesClusterCommandHandler::InvokeCommand(CommandHandlerInterface::HandlerContext & ctxt)
{
    using namespace chip::app::Clusters::Scenes;

    auto unify_node = m_node_state_monitor.bridged_endpoint(ctxt.mRequestPath.mEndpointId);
    if (!unify_node)
    {
        sl_log_info(LOG_TAG, "The endpoint [%i] is not a part of unify matter bridge node", ctxt.mRequestPath.mEndpointId);
        return;
    }

    std::string cmd;
    nlohmann::json payload = {};

    switch (ctxt.mRequestPath.mCommandId)
    {
    case Commands::AddScene::Id: {
        cmd = "AddScene";
        Commands::AddScene::DecodableType data;
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR)
        {
            try
            {
                payload["GroupId"] = to_json(data.groupId);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try
            {
                payload["SceneId"] = to_json(data.sceneId);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try
            {
                payload["TransitionTime"] = to_json(data.transitionTime);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try
            {
                payload["SceneName"] = to_json(data.sceneName);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try
            {
                payload["ExtensionFieldSets"] = to_json(data.extensionFieldSets);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
        }
    }
    break;
    case Commands::ViewScene::Id: {
        cmd = "ViewScene";
        Commands::ViewScene::DecodableType data;
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR)
        {
            try
            {
                payload["GroupId"] = to_json(data.groupId);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try
            {
                payload["SceneId"] = to_json(data.sceneId);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
        }
    }
    break;
    case Commands::RemoveScene::Id: {
        cmd = "RemoveScene";
        Commands::RemoveScene::DecodableType data;
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR)
        {
            try
            {
                payload["GroupId"] = to_json(data.groupId);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try
            {
                payload["SceneId"] = to_json(data.sceneId);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
        }
    }
    break;
    case Commands::RemoveAllScenes::Id: {
        cmd = "RemoveAllScenes";
        Commands::RemoveAllScenes::DecodableType data;
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR)
        {
            try
            {
                payload["GroupId"] = to_json(data.groupId);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
        }
    }
    break;
    case Commands::StoreScene::Id: {
        cmd = "StoreScene";
        Commands::StoreScene::DecodableType data;
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR)
        {
            try
            {
                payload["GroupId"] = to_json(data.groupId);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try
            {
                payload["SceneId"] = to_json(data.sceneId);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
        }
    }
    break;
    case Commands::RecallScene::Id: {
        cmd = "RecallScene";
        Commands::RecallScene::DecodableType data;
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR)
        {
            try
            {
                payload["GroupId"] = to_json(data.groupId);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try
            {
                payload["SceneId"] = to_json(data.sceneId);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            if (data.transitionTime.HasValue())
            {
                try
                {
                    payload["TransitionTime"] = to_json(data.transitionTime.Value());
                } catch (std::exception & ex)
                {
                    sl_log_warning(LOG_TAG, "Failed to add the command arguments value to json format: %s", ex.what());
                }
            }
        }
    }
    break;
    case Commands::GetSceneMembership::Id: {
        cmd = "GetSceneMembership";
        Commands::GetSceneMembership::DecodableType data;
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR)
        {
            try
            {
                payload["GroupId"] = to_json(data.groupId);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
        }
    }
    break;
    case Commands::EnhancedAddScene::Id: {
        cmd = "EnhancedAddScene";
        Commands::EnhancedAddScene::DecodableType data;
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR)
        {
            try
            {
                payload["GroupId"] = to_json(data.groupId);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try
            {
                payload["SceneId"] = to_json(data.sceneId);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try
            {
                payload["TransitionTime"] = to_json(data.transitionTime);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try
            {
                payload["SceneName"] = to_json(data.sceneName);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try
            {
                payload["ExtensionFieldSets"] = to_json(data.extensionFieldSets);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
        }
    }
    break;
    case Commands::EnhancedViewScene::Id: {
        cmd = "EnhancedViewScene";
        Commands::EnhancedViewScene::DecodableType data;
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR)
        {
            try
            {
                payload["GroupId"] = to_json(data.groupId);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try
            {
                payload["SceneId"] = to_json(data.sceneId);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
        }
    }
    break;
    case Commands::CopyScene::Id: {
        cmd = "CopyScene";
        Commands::CopyScene::DecodableType data;
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR)
        {
            try
            {
                payload["Mode"] = to_json(data.mode);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try
            {
                payload["GroupIdFrom"] = to_json(data.groupIdFrom);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try
            {
                payload["SceneIdFrom"] = to_json(data.sceneIdFrom);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try
            {
                payload["GroupIdTo"] = to_json(data.groupIdTo);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try
            {
                payload["SceneIdTo"] = to_json(data.sceneIdTo);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
        }
    }
    break;
    }

    if (!cmd.empty())
    {
        ctxt.mCommandHandler.AddStatus(ctxt.mRequestPath, Protocols::InteractionModel::Status::Success);
        send_unify_mqtt_cmd(ctxt, cmd, payload);
        sl_log_debug(LOG_TAG, "Mapped [%] command to unify dotdot data model", cmd.c_str());
    }
    else
    {
        ctxt.mCommandHandler.AddStatus(ctxt.mRequestPath, Protocols::InteractionModel::Status::UnsupportedCommand);
    }
    ctxt.SetCommandHandled();
}

void OnOffClusterCommandHandler::InvokeCommand(CommandHandlerInterface::HandlerContext & ctxt)
{
    using namespace chip::app::Clusters::OnOff;

    auto unify_node = m_node_state_monitor.bridged_endpoint(ctxt.mRequestPath.mEndpointId);
    if (!unify_node)
    {
        sl_log_info(LOG_TAG, "The endpoint [%i] is not a part of unify matter bridge node", ctxt.mRequestPath.mEndpointId);
        return;
    }

    std::string cmd;
    nlohmann::json payload = {};

    switch (ctxt.mRequestPath.mCommandId)
    {
    case Commands::Off::Id: {
        cmd = "Off";
        Commands::Off::DecodableType data;
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR)
        {
        }
    }
    break;
    case Commands::On::Id: {
        cmd = "On";
        Commands::On::DecodableType data;
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR)
        {
        }
    }
    break;
    case Commands::Toggle::Id: {
        cmd = "Toggle";
        Commands::Toggle::DecodableType data;
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR)
        {
        }
    }
    break;
    case Commands::OffWithEffect::Id: {
        cmd = "OffWithEffect";
        Commands::OffWithEffect::DecodableType data;
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR)
        {
            try
            {
                payload["EffectId"] = to_json(data.effectId);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try
            {
                payload["EffectVariant"] = to_json(data.effectVariant);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
        }
    }
    break;
    case Commands::OnWithRecallGlobalScene::Id: {
        cmd = "OnWithRecallGlobalScene";
        Commands::OnWithRecallGlobalScene::DecodableType data;
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR)
        {
        }
    }
    break;
    case Commands::OnWithTimedOff::Id: {
        cmd = "OnWithTimedOff";
        Commands::OnWithTimedOff::DecodableType data;
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR)
        {
            try
            {
                payload["OnOffControl"] = to_json(data.onOffControl);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try
            {
                payload["OnTime"] = to_json(data.onTime);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try
            {
                payload["OffWaitTime"] = to_json(data.offWaitTime);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
        }
    }
    break;
    }

    if (!cmd.empty())
    {
        ctxt.mCommandHandler.AddStatus(ctxt.mRequestPath, Protocols::InteractionModel::Status::Success);
        send_unify_mqtt_cmd(ctxt, cmd, payload);
        sl_log_debug(LOG_TAG, "Mapped [%] command to unify dotdot data model", cmd.c_str());
    }
    else
    {
        ctxt.mCommandHandler.AddStatus(ctxt.mRequestPath, Protocols::InteractionModel::Status::UnsupportedCommand);
    }
    ctxt.SetCommandHandled();
}

void OnOffSwitchConfigurationClusterCommandHandler::InvokeCommand(CommandHandlerInterface::HandlerContext & ctxt)
{
    using namespace chip::app::Clusters::OnOffSwitchConfiguration;

    auto unify_node = m_node_state_monitor.bridged_endpoint(ctxt.mRequestPath.mEndpointId);
    if (!unify_node)
    {
        sl_log_info(LOG_TAG, "The endpoint [%i] is not a part of unify matter bridge node", ctxt.mRequestPath.mEndpointId);
        return;
    }

    std::string cmd;
    nlohmann::json payload = {};

    switch (ctxt.mRequestPath.mCommandId)
    {
    }

    if (!cmd.empty())
    {
        ctxt.mCommandHandler.AddStatus(ctxt.mRequestPath, Protocols::InteractionModel::Status::Success);
        send_unify_mqtt_cmd(ctxt, cmd, payload);
        sl_log_debug(LOG_TAG, "Mapped [%] command to unify dotdot data model", cmd.c_str());
    }
    else
    {
        ctxt.mCommandHandler.AddStatus(ctxt.mRequestPath, Protocols::InteractionModel::Status::UnsupportedCommand);
    }
    ctxt.SetCommandHandled();
}

void LevelControlClusterCommandHandler::InvokeCommand(CommandHandlerInterface::HandlerContext & ctxt)
{
    using namespace chip::app::Clusters::LevelControl;

    auto unify_node = m_node_state_monitor.bridged_endpoint(ctxt.mRequestPath.mEndpointId);
    if (!unify_node)
    {
        sl_log_info(LOG_TAG, "The endpoint [%i] is not a part of unify matter bridge node", ctxt.mRequestPath.mEndpointId);
        return;
    }

    std::string cmd;
    nlohmann::json payload = {};

    switch (ctxt.mRequestPath.mCommandId)
    {
    case Commands::MoveToLevel::Id: {
        cmd = "MoveToLevel";
        Commands::MoveToLevel::DecodableType data;
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR)
        {
            try
            {
                payload["Level"] = to_json(data.level);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try
            {
                payload["TransitionTime"] = to_json(data.transitionTime);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try
            {
                payload["OptionsMask"] = to_json(data.optionsMask);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try
            {
                payload["OptionsOverride"] = to_json(data.optionsOverride);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
        }
    }
    break;
    case Commands::Move::Id: {
        cmd = "Move";
        Commands::Move::DecodableType data;
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR)
        {
            try
            {
                payload["MoveMode"] = to_json(data.moveMode);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try
            {
                payload["Rate"] = to_json(data.rate);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try
            {
                payload["OptionsMask"] = to_json(data.optionsMask);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try
            {
                payload["OptionsOverride"] = to_json(data.optionsOverride);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
        }
    }
    break;
    case Commands::Step::Id: {
        cmd = "Step";
        Commands::Step::DecodableType data;
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR)
        {
            try
            {
                payload["StepMode"] = to_json(data.stepMode);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try
            {
                payload["StepSize"] = to_json(data.stepSize);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try
            {
                payload["TransitionTime"] = to_json(data.transitionTime);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try
            {
                payload["OptionsMask"] = to_json(data.optionsMask);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try
            {
                payload["OptionsOverride"] = to_json(data.optionsOverride);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
        }
    }
    break;
    case Commands::Stop::Id: {
        cmd = "Stop";
        Commands::Stop::DecodableType data;
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR)
        {
            try
            {
                payload["OptionsMask"] = to_json(data.optionsMask);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try
            {
                payload["OptionsOverride"] = to_json(data.optionsOverride);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
        }
    }
    break;
    case Commands::MoveToLevelWithOnOff::Id: {
        cmd = "MoveToLevelWithOnOff";
        Commands::MoveToLevelWithOnOff::DecodableType data;
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR)
        {
            try
            {
                payload["Level"] = to_json(data.level);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try
            {
                payload["TransitionTime"] = to_json(data.transitionTime);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try
            {
                payload["OptionsMask"] = to_json(data.optionsMask);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try
            {
                payload["OptionsOverride"] = to_json(data.optionsOverride);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
        }
    }
    break;
    case Commands::MoveWithOnOff::Id: {
        cmd = "MoveWithOnOff";
        Commands::MoveWithOnOff::DecodableType data;
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR)
        {
            try
            {
                payload["MoveMode"] = to_json(data.moveMode);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try
            {
                payload["Rate"] = to_json(data.rate);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try
            {
                payload["OptionsMask"] = to_json(data.optionsMask);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try
            {
                payload["OptionsOverride"] = to_json(data.optionsOverride);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
        }
    }
    break;
    case Commands::StepWithOnOff::Id: {
        cmd = "StepWithOnOff";
        Commands::StepWithOnOff::DecodableType data;
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR)
        {
            try
            {
                payload["StepMode"] = to_json(data.stepMode);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try
            {
                payload["StepSize"] = to_json(data.stepSize);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try
            {
                payload["TransitionTime"] = to_json(data.transitionTime);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try
            {
                payload["OptionsMask"] = to_json(data.optionsMask);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try
            {
                payload["OptionsOverride"] = to_json(data.optionsOverride);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
        }
    }
    break;
    case Commands::StopWithOnOff::Id: {
        cmd = "StopWithOnOff";
        Commands::StopWithOnOff::DecodableType data;
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR)
        {
            try
            {
                payload["OptionsMask"] = to_json(data.optionsMask);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try
            {
                payload["OptionsOverride"] = to_json(data.optionsOverride);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
        }
    }
    break;
    case Commands::MoveToClosestFrequency::Id: {
        cmd = "MoveToClosestFrequency";
        Commands::MoveToClosestFrequency::DecodableType data;
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR)
        {
            try
            {
                payload["Frequency"] = to_json(data.frequency);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
        }
    }
    break;
    }

    if (!cmd.empty())
    {
        ctxt.mCommandHandler.AddStatus(ctxt.mRequestPath, Protocols::InteractionModel::Status::Success);
        send_unify_mqtt_cmd(ctxt, cmd, payload);
        sl_log_debug(LOG_TAG, "Mapped [%] command to unify dotdot data model", cmd.c_str());
    }
    else
    {
        ctxt.mCommandHandler.AddStatus(ctxt.mRequestPath, Protocols::InteractionModel::Status::UnsupportedCommand);
    }
    ctxt.SetCommandHandled();
}

void BinaryInputBasicClusterCommandHandler::InvokeCommand(CommandHandlerInterface::HandlerContext & ctxt)
{
    using namespace chip::app::Clusters::BinaryInputBasic;

    auto unify_node = m_node_state_monitor.bridged_endpoint(ctxt.mRequestPath.mEndpointId);
    if (!unify_node)
    {
        sl_log_info(LOG_TAG, "The endpoint [%i] is not a part of unify matter bridge node", ctxt.mRequestPath.mEndpointId);
        return;
    }

    std::string cmd;
    nlohmann::json payload = {};

    switch (ctxt.mRequestPath.mCommandId)
    {
    }

    if (!cmd.empty())
    {
        ctxt.mCommandHandler.AddStatus(ctxt.mRequestPath, Protocols::InteractionModel::Status::Success);
        send_unify_mqtt_cmd(ctxt, cmd, payload);
        sl_log_debug(LOG_TAG, "Mapped [%] command to unify dotdot data model", cmd.c_str());
    }
    else
    {
        ctxt.mCommandHandler.AddStatus(ctxt.mRequestPath, Protocols::InteractionModel::Status::UnsupportedCommand);
    }
    ctxt.SetCommandHandled();
}

void PulseWidthModulationClusterCommandHandler::InvokeCommand(CommandHandlerInterface::HandlerContext & ctxt)
{
    using namespace chip::app::Clusters::PulseWidthModulation;

    auto unify_node = m_node_state_monitor.bridged_endpoint(ctxt.mRequestPath.mEndpointId);
    if (!unify_node)
    {
        sl_log_info(LOG_TAG, "The endpoint [%i] is not a part of unify matter bridge node", ctxt.mRequestPath.mEndpointId);
        return;
    }

    std::string cmd;
    nlohmann::json payload = {};

    switch (ctxt.mRequestPath.mCommandId)
    {
    }

    if (!cmd.empty())
    {
        ctxt.mCommandHandler.AddStatus(ctxt.mRequestPath, Protocols::InteractionModel::Status::Success);
        send_unify_mqtt_cmd(ctxt, cmd, payload);
        sl_log_debug(LOG_TAG, "Mapped [%] command to unify dotdot data model", cmd.c_str());
    }
    else
    {
        ctxt.mCommandHandler.AddStatus(ctxt.mRequestPath, Protocols::InteractionModel::Status::UnsupportedCommand);
    }
    ctxt.SetCommandHandled();
}

void ActionsClusterCommandHandler::InvokeCommand(CommandHandlerInterface::HandlerContext & ctxt)
{
    using namespace chip::app::Clusters::Actions;

    auto unify_node = m_node_state_monitor.bridged_endpoint(ctxt.mRequestPath.mEndpointId);
    if (!unify_node)
    {
        sl_log_info(LOG_TAG, "The endpoint [%i] is not a part of unify matter bridge node", ctxt.mRequestPath.mEndpointId);
        return;
    }

    std::string cmd;
    nlohmann::json payload = {};

    switch (ctxt.mRequestPath.mCommandId)
    {
    case Commands::InstantAction::Id: {
        cmd = "InstantAction";
        Commands::InstantAction::DecodableType data;
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR)
        {
            try
            {
                payload["ActionID"] = to_json(data.actionID);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            if (data.invokeID.HasValue())
            {
                try
                {
                    payload["InvokeID"] = to_json(data.invokeID.Value());
                } catch (std::exception & ex)
                {
                    sl_log_warning(LOG_TAG, "Failed to add the command arguments value to json format: %s", ex.what());
                }
            }
        }
    }
    break;
    case Commands::InstantActionWithTransition::Id: {
        cmd = "InstantActionWithTransition";
        Commands::InstantActionWithTransition::DecodableType data;
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR)
        {
            try
            {
                payload["ActionID"] = to_json(data.actionID);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            if (data.invokeID.HasValue())
            {
                try
                {
                    payload["InvokeID"] = to_json(data.invokeID.Value());
                } catch (std::exception & ex)
                {
                    sl_log_warning(LOG_TAG, "Failed to add the command arguments value to json format: %s", ex.what());
                }
            }
            try
            {
                payload["TransitionTime"] = to_json(data.transitionTime);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
        }
    }
    break;
    case Commands::StartAction::Id: {
        cmd = "StartAction";
        Commands::StartAction::DecodableType data;
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR)
        {
            try
            {
                payload["ActionID"] = to_json(data.actionID);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            if (data.invokeID.HasValue())
            {
                try
                {
                    payload["InvokeID"] = to_json(data.invokeID.Value());
                } catch (std::exception & ex)
                {
                    sl_log_warning(LOG_TAG, "Failed to add the command arguments value to json format: %s", ex.what());
                }
            }
        }
    }
    break;
    case Commands::StartActionWithDuration::Id: {
        cmd = "StartActionWithDuration";
        Commands::StartActionWithDuration::DecodableType data;
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR)
        {
            try
            {
                payload["ActionID"] = to_json(data.actionID);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            if (data.invokeID.HasValue())
            {
                try
                {
                    payload["InvokeID"] = to_json(data.invokeID.Value());
                } catch (std::exception & ex)
                {
                    sl_log_warning(LOG_TAG, "Failed to add the command arguments value to json format: %s", ex.what());
                }
            }
            try
            {
                payload["Duration"] = to_json(data.duration);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
        }
    }
    break;
    case Commands::StopAction::Id: {
        cmd = "StopAction";
        Commands::StopAction::DecodableType data;
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR)
        {
            try
            {
                payload["ActionID"] = to_json(data.actionID);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            if (data.invokeID.HasValue())
            {
                try
                {
                    payload["InvokeID"] = to_json(data.invokeID.Value());
                } catch (std::exception & ex)
                {
                    sl_log_warning(LOG_TAG, "Failed to add the command arguments value to json format: %s", ex.what());
                }
            }
        }
    }
    break;
    case Commands::PauseAction::Id: {
        cmd = "PauseAction";
        Commands::PauseAction::DecodableType data;
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR)
        {
            try
            {
                payload["ActionID"] = to_json(data.actionID);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            if (data.invokeID.HasValue())
            {
                try
                {
                    payload["InvokeID"] = to_json(data.invokeID.Value());
                } catch (std::exception & ex)
                {
                    sl_log_warning(LOG_TAG, "Failed to add the command arguments value to json format: %s", ex.what());
                }
            }
        }
    }
    break;
    case Commands::PauseActionWithDuration::Id: {
        cmd = "PauseActionWithDuration";
        Commands::PauseActionWithDuration::DecodableType data;
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR)
        {
            try
            {
                payload["ActionID"] = to_json(data.actionID);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            if (data.invokeID.HasValue())
            {
                try
                {
                    payload["InvokeID"] = to_json(data.invokeID.Value());
                } catch (std::exception & ex)
                {
                    sl_log_warning(LOG_TAG, "Failed to add the command arguments value to json format: %s", ex.what());
                }
            }
            try
            {
                payload["Duration"] = to_json(data.duration);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
        }
    }
    break;
    case Commands::ResumeAction::Id: {
        cmd = "ResumeAction";
        Commands::ResumeAction::DecodableType data;
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR)
        {
            try
            {
                payload["ActionID"] = to_json(data.actionID);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            if (data.invokeID.HasValue())
            {
                try
                {
                    payload["InvokeID"] = to_json(data.invokeID.Value());
                } catch (std::exception & ex)
                {
                    sl_log_warning(LOG_TAG, "Failed to add the command arguments value to json format: %s", ex.what());
                }
            }
        }
    }
    break;
    case Commands::EnableAction::Id: {
        cmd = "EnableAction";
        Commands::EnableAction::DecodableType data;
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR)
        {
            try
            {
                payload["ActionID"] = to_json(data.actionID);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            if (data.invokeID.HasValue())
            {
                try
                {
                    payload["InvokeID"] = to_json(data.invokeID.Value());
                } catch (std::exception & ex)
                {
                    sl_log_warning(LOG_TAG, "Failed to add the command arguments value to json format: %s", ex.what());
                }
            }
        }
    }
    break;
    case Commands::EnableActionWithDuration::Id: {
        cmd = "EnableActionWithDuration";
        Commands::EnableActionWithDuration::DecodableType data;
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR)
        {
            try
            {
                payload["ActionID"] = to_json(data.actionID);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            if (data.invokeID.HasValue())
            {
                try
                {
                    payload["InvokeID"] = to_json(data.invokeID.Value());
                } catch (std::exception & ex)
                {
                    sl_log_warning(LOG_TAG, "Failed to add the command arguments value to json format: %s", ex.what());
                }
            }
            try
            {
                payload["Duration"] = to_json(data.duration);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
        }
    }
    break;
    case Commands::DisableAction::Id: {
        cmd = "DisableAction";
        Commands::DisableAction::DecodableType data;
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR)
        {
            try
            {
                payload["ActionID"] = to_json(data.actionID);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            if (data.invokeID.HasValue())
            {
                try
                {
                    payload["InvokeID"] = to_json(data.invokeID.Value());
                } catch (std::exception & ex)
                {
                    sl_log_warning(LOG_TAG, "Failed to add the command arguments value to json format: %s", ex.what());
                }
            }
        }
    }
    break;
    case Commands::DisableActionWithDuration::Id: {
        cmd = "DisableActionWithDuration";
        Commands::DisableActionWithDuration::DecodableType data;
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR)
        {
            try
            {
                payload["ActionID"] = to_json(data.actionID);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            if (data.invokeID.HasValue())
            {
                try
                {
                    payload["InvokeID"] = to_json(data.invokeID.Value());
                } catch (std::exception & ex)
                {
                    sl_log_warning(LOG_TAG, "Failed to add the command arguments value to json format: %s", ex.what());
                }
            }
            try
            {
                payload["Duration"] = to_json(data.duration);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
        }
    }
    break;
    }

    if (!cmd.empty())
    {
        ctxt.mCommandHandler.AddStatus(ctxt.mRequestPath, Protocols::InteractionModel::Status::Success);
        send_unify_mqtt_cmd(ctxt, cmd, payload);
        sl_log_debug(LOG_TAG, "Mapped [%] command to unify dotdot data model", cmd.c_str());
    }
    else
    {
        ctxt.mCommandHandler.AddStatus(ctxt.mRequestPath, Protocols::InteractionModel::Status::UnsupportedCommand);
    }
    ctxt.SetCommandHandled();
}

void BasicClusterCommandHandler::InvokeCommand(CommandHandlerInterface::HandlerContext & ctxt)
{
    using namespace chip::app::Clusters::Basic;

    auto unify_node = m_node_state_monitor.bridged_endpoint(ctxt.mRequestPath.mEndpointId);
    if (!unify_node)
    {
        sl_log_info(LOG_TAG, "The endpoint [%i] is not a part of unify matter bridge node", ctxt.mRequestPath.mEndpointId);
        return;
    }

    std::string cmd;
    nlohmann::json payload = {};

    switch (ctxt.mRequestPath.mCommandId)
    {
    case Commands::MfgSpecificPing::Id: {
        cmd = "MfgSpecificPing";
        Commands::MfgSpecificPing::DecodableType data;
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR)
        {
        }
    }
    break;
    }

    if (!cmd.empty())
    {
        ctxt.mCommandHandler.AddStatus(ctxt.mRequestPath, Protocols::InteractionModel::Status::Success);
        send_unify_mqtt_cmd(ctxt, cmd, payload);
        sl_log_debug(LOG_TAG, "Mapped [%] command to unify dotdot data model", cmd.c_str());
    }
    else
    {
        ctxt.mCommandHandler.AddStatus(ctxt.mRequestPath, Protocols::InteractionModel::Status::UnsupportedCommand);
    }
    ctxt.SetCommandHandled();
}

void OtaSoftwareUpdateProviderClusterCommandHandler::InvokeCommand(CommandHandlerInterface::HandlerContext & ctxt)
{
    using namespace chip::app::Clusters::OtaSoftwareUpdateProvider;

    auto unify_node = m_node_state_monitor.bridged_endpoint(ctxt.mRequestPath.mEndpointId);
    if (!unify_node)
    {
        sl_log_info(LOG_TAG, "The endpoint [%i] is not a part of unify matter bridge node", ctxt.mRequestPath.mEndpointId);
        return;
    }

    std::string cmd;
    nlohmann::json payload = {};

    switch (ctxt.mRequestPath.mCommandId)
    {
    case Commands::QueryImage::Id: {
        cmd = "QueryImage";
        Commands::QueryImage::DecodableType data;
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR)
        {
            try
            {
                payload["VendorId"] = to_json(data.vendorId);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try
            {
                payload["ProductId"] = to_json(data.productId);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try
            {
                payload["SoftwareVersion"] = to_json(data.softwareVersion);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try
            {
                payload["ProtocolsSupported"] = to_json(data.protocolsSupported);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            if (data.hardwareVersion.HasValue())
            {
                try
                {
                    payload["HardwareVersion"] = to_json(data.hardwareVersion.Value());
                } catch (std::exception & ex)
                {
                    sl_log_warning(LOG_TAG, "Failed to add the command arguments value to json format: %s", ex.what());
                }
            }
            if (data.location.HasValue())
            {
                try
                {
                    payload["Location"] = to_json(data.location.Value());
                } catch (std::exception & ex)
                {
                    sl_log_warning(LOG_TAG, "Failed to add the command arguments value to json format: %s", ex.what());
                }
            }
            if (data.requestorCanConsent.HasValue())
            {
                try
                {
                    payload["RequestorCanConsent"] = to_json(data.requestorCanConsent.Value());
                } catch (std::exception & ex)
                {
                    sl_log_warning(LOG_TAG, "Failed to add the command arguments value to json format: %s", ex.what());
                }
            }
            if (data.metadataForProvider.HasValue())
            {
                try
                {
                    payload["MetadataForProvider"] = to_json(data.metadataForProvider.Value());
                } catch (std::exception & ex)
                {
                    sl_log_warning(LOG_TAG, "Failed to add the command arguments value to json format: %s", ex.what());
                }
            }
        }
    }
    break;
    case Commands::ApplyUpdateRequest::Id: {
        cmd = "ApplyUpdateRequest";
        Commands::ApplyUpdateRequest::DecodableType data;
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR)
        {
            try
            {
                payload["UpdateToken"] = to_json(data.updateToken);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try
            {
                payload["NewVersion"] = to_json(data.newVersion);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
        }
    }
    break;
    case Commands::NotifyUpdateApplied::Id: {
        cmd = "NotifyUpdateApplied";
        Commands::NotifyUpdateApplied::DecodableType data;
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR)
        {
            try
            {
                payload["UpdateToken"] = to_json(data.updateToken);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try
            {
                payload["SoftwareVersion"] = to_json(data.softwareVersion);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
        }
    }
    break;
    }

    if (!cmd.empty())
    {
        ctxt.mCommandHandler.AddStatus(ctxt.mRequestPath, Protocols::InteractionModel::Status::Success);
        send_unify_mqtt_cmd(ctxt, cmd, payload);
        sl_log_debug(LOG_TAG, "Mapped [%] command to unify dotdot data model", cmd.c_str());
    }
    else
    {
        ctxt.mCommandHandler.AddStatus(ctxt.mRequestPath, Protocols::InteractionModel::Status::UnsupportedCommand);
    }
    ctxt.SetCommandHandled();
}

void OtaSoftwareUpdateRequestorClusterCommandHandler::InvokeCommand(CommandHandlerInterface::HandlerContext & ctxt)
{
    using namespace chip::app::Clusters::OtaSoftwareUpdateRequestor;

    auto unify_node = m_node_state_monitor.bridged_endpoint(ctxt.mRequestPath.mEndpointId);
    if (!unify_node)
    {
        sl_log_info(LOG_TAG, "The endpoint [%i] is not a part of unify matter bridge node", ctxt.mRequestPath.mEndpointId);
        return;
    }

    std::string cmd;
    nlohmann::json payload = {};

    switch (ctxt.mRequestPath.mCommandId)
    {
    case Commands::AnnounceOtaProvider::Id: {
        cmd = "AnnounceOtaProvider";
        Commands::AnnounceOtaProvider::DecodableType data;
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR)
        {
            try
            {
                payload["ProviderNodeId"] = to_json(data.providerNodeId);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try
            {
                payload["VendorId"] = to_json(data.vendorId);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try
            {
                payload["AnnouncementReason"] = to_json(data.announcementReason);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            if (data.metadataForNode.HasValue())
            {
                try
                {
                    payload["MetadataForNode"] = to_json(data.metadataForNode.Value());
                } catch (std::exception & ex)
                {
                    sl_log_warning(LOG_TAG, "Failed to add the command arguments value to json format: %s", ex.what());
                }
            }
            try
            {
                payload["Endpoint"] = to_json(data.endpoint);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
        }
    }
    break;
    }

    if (!cmd.empty())
    {
        ctxt.mCommandHandler.AddStatus(ctxt.mRequestPath, Protocols::InteractionModel::Status::Success);
        send_unify_mqtt_cmd(ctxt, cmd, payload);
        sl_log_debug(LOG_TAG, "Mapped [%] command to unify dotdot data model", cmd.c_str());
    }
    else
    {
        ctxt.mCommandHandler.AddStatus(ctxt.mRequestPath, Protocols::InteractionModel::Status::UnsupportedCommand);
    }
    ctxt.SetCommandHandled();
}

void LocalizationConfigurationClusterCommandHandler::InvokeCommand(CommandHandlerInterface::HandlerContext & ctxt)
{
    using namespace chip::app::Clusters::LocalizationConfiguration;

    auto unify_node = m_node_state_monitor.bridged_endpoint(ctxt.mRequestPath.mEndpointId);
    if (!unify_node)
    {
        sl_log_info(LOG_TAG, "The endpoint [%i] is not a part of unify matter bridge node", ctxt.mRequestPath.mEndpointId);
        return;
    }

    std::string cmd;
    nlohmann::json payload = {};

    switch (ctxt.mRequestPath.mCommandId)
    {
    }

    if (!cmd.empty())
    {
        ctxt.mCommandHandler.AddStatus(ctxt.mRequestPath, Protocols::InteractionModel::Status::Success);
        send_unify_mqtt_cmd(ctxt, cmd, payload);
        sl_log_debug(LOG_TAG, "Mapped [%] command to unify dotdot data model", cmd.c_str());
    }
    else
    {
        ctxt.mCommandHandler.AddStatus(ctxt.mRequestPath, Protocols::InteractionModel::Status::UnsupportedCommand);
    }
    ctxt.SetCommandHandled();
}

void TimeFormatLocalizationClusterCommandHandler::InvokeCommand(CommandHandlerInterface::HandlerContext & ctxt)
{
    using namespace chip::app::Clusters::TimeFormatLocalization;

    auto unify_node = m_node_state_monitor.bridged_endpoint(ctxt.mRequestPath.mEndpointId);
    if (!unify_node)
    {
        sl_log_info(LOG_TAG, "The endpoint [%i] is not a part of unify matter bridge node", ctxt.mRequestPath.mEndpointId);
        return;
    }

    std::string cmd;
    nlohmann::json payload = {};

    switch (ctxt.mRequestPath.mCommandId)
    {
    }

    if (!cmd.empty())
    {
        ctxt.mCommandHandler.AddStatus(ctxt.mRequestPath, Protocols::InteractionModel::Status::Success);
        send_unify_mqtt_cmd(ctxt, cmd, payload);
        sl_log_debug(LOG_TAG, "Mapped [%] command to unify dotdot data model", cmd.c_str());
    }
    else
    {
        ctxt.mCommandHandler.AddStatus(ctxt.mRequestPath, Protocols::InteractionModel::Status::UnsupportedCommand);
    }
    ctxt.SetCommandHandled();
}

void UnitLocalizationClusterCommandHandler::InvokeCommand(CommandHandlerInterface::HandlerContext & ctxt)
{
    using namespace chip::app::Clusters::UnitLocalization;

    auto unify_node = m_node_state_monitor.bridged_endpoint(ctxt.mRequestPath.mEndpointId);
    if (!unify_node)
    {
        sl_log_info(LOG_TAG, "The endpoint [%i] is not a part of unify matter bridge node", ctxt.mRequestPath.mEndpointId);
        return;
    }

    std::string cmd;
    nlohmann::json payload = {};

    switch (ctxt.mRequestPath.mCommandId)
    {
    }

    if (!cmd.empty())
    {
        ctxt.mCommandHandler.AddStatus(ctxt.mRequestPath, Protocols::InteractionModel::Status::Success);
        send_unify_mqtt_cmd(ctxt, cmd, payload);
        sl_log_debug(LOG_TAG, "Mapped [%] command to unify dotdot data model", cmd.c_str());
    }
    else
    {
        ctxt.mCommandHandler.AddStatus(ctxt.mRequestPath, Protocols::InteractionModel::Status::UnsupportedCommand);
    }
    ctxt.SetCommandHandled();
}

void PowerSourceConfigurationClusterCommandHandler::InvokeCommand(CommandHandlerInterface::HandlerContext & ctxt)
{
    using namespace chip::app::Clusters::PowerSourceConfiguration;

    auto unify_node = m_node_state_monitor.bridged_endpoint(ctxt.mRequestPath.mEndpointId);
    if (!unify_node)
    {
        sl_log_info(LOG_TAG, "The endpoint [%i] is not a part of unify matter bridge node", ctxt.mRequestPath.mEndpointId);
        return;
    }

    std::string cmd;
    nlohmann::json payload = {};

    switch (ctxt.mRequestPath.mCommandId)
    {
    }

    if (!cmd.empty())
    {
        ctxt.mCommandHandler.AddStatus(ctxt.mRequestPath, Protocols::InteractionModel::Status::Success);
        send_unify_mqtt_cmd(ctxt, cmd, payload);
        sl_log_debug(LOG_TAG, "Mapped [%] command to unify dotdot data model", cmd.c_str());
    }
    else
    {
        ctxt.mCommandHandler.AddStatus(ctxt.mRequestPath, Protocols::InteractionModel::Status::UnsupportedCommand);
    }
    ctxt.SetCommandHandled();
}

void PowerSourceClusterCommandHandler::InvokeCommand(CommandHandlerInterface::HandlerContext & ctxt)
{
    using namespace chip::app::Clusters::PowerSource;

    auto unify_node = m_node_state_monitor.bridged_endpoint(ctxt.mRequestPath.mEndpointId);
    if (!unify_node)
    {
        sl_log_info(LOG_TAG, "The endpoint [%i] is not a part of unify matter bridge node", ctxt.mRequestPath.mEndpointId);
        return;
    }

    std::string cmd;
    nlohmann::json payload = {};

    switch (ctxt.mRequestPath.mCommandId)
    {
    }

    if (!cmd.empty())
    {
        ctxt.mCommandHandler.AddStatus(ctxt.mRequestPath, Protocols::InteractionModel::Status::Success);
        send_unify_mqtt_cmd(ctxt, cmd, payload);
        sl_log_debug(LOG_TAG, "Mapped [%] command to unify dotdot data model", cmd.c_str());
    }
    else
    {
        ctxt.mCommandHandler.AddStatus(ctxt.mRequestPath, Protocols::InteractionModel::Status::UnsupportedCommand);
    }
    ctxt.SetCommandHandled();
}

void GeneralCommissioningClusterCommandHandler::InvokeCommand(CommandHandlerInterface::HandlerContext & ctxt)
{
    using namespace chip::app::Clusters::GeneralCommissioning;

    auto unify_node = m_node_state_monitor.bridged_endpoint(ctxt.mRequestPath.mEndpointId);
    if (!unify_node)
    {
        sl_log_info(LOG_TAG, "The endpoint [%i] is not a part of unify matter bridge node", ctxt.mRequestPath.mEndpointId);
        return;
    }

    std::string cmd;
    nlohmann::json payload = {};

    switch (ctxt.mRequestPath.mCommandId)
    {
    case Commands::ArmFailSafe::Id: {
        cmd = "ArmFailSafe";
        Commands::ArmFailSafe::DecodableType data;
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR)
        {
            try
            {
                payload["ExpiryLengthSeconds"] = to_json(data.expiryLengthSeconds);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try
            {
                payload["Breadcrumb"] = to_json(data.breadcrumb);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
        }
    }
    break;
    case Commands::SetRegulatoryConfig::Id: {
        cmd = "SetRegulatoryConfig";
        Commands::SetRegulatoryConfig::DecodableType data;
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR)
        {
            try
            {
                payload["NewRegulatoryConfig"] = to_json(data.newRegulatoryConfig);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try
            {
                payload["CountryCode"] = to_json(data.countryCode);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try
            {
                payload["Breadcrumb"] = to_json(data.breadcrumb);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
        }
    }
    break;
    case Commands::CommissioningComplete::Id: {
        cmd = "CommissioningComplete";
        Commands::CommissioningComplete::DecodableType data;
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR)
        {
        }
    }
    break;
    }

    if (!cmd.empty())
    {
        ctxt.mCommandHandler.AddStatus(ctxt.mRequestPath, Protocols::InteractionModel::Status::Success);
        send_unify_mqtt_cmd(ctxt, cmd, payload);
        sl_log_debug(LOG_TAG, "Mapped [%] command to unify dotdot data model", cmd.c_str());
    }
    else
    {
        ctxt.mCommandHandler.AddStatus(ctxt.mRequestPath, Protocols::InteractionModel::Status::UnsupportedCommand);
    }
    ctxt.SetCommandHandled();
}

void DiagnosticLogsClusterCommandHandler::InvokeCommand(CommandHandlerInterface::HandlerContext & ctxt)
{
    using namespace chip::app::Clusters::DiagnosticLogs;

    auto unify_node = m_node_state_monitor.bridged_endpoint(ctxt.mRequestPath.mEndpointId);
    if (!unify_node)
    {
        sl_log_info(LOG_TAG, "The endpoint [%i] is not a part of unify matter bridge node", ctxt.mRequestPath.mEndpointId);
        return;
    }

    std::string cmd;
    nlohmann::json payload = {};

    switch (ctxt.mRequestPath.mCommandId)
    {
    case Commands::RetrieveLogsRequest::Id: {
        cmd = "RetrieveLogsRequest";
        Commands::RetrieveLogsRequest::DecodableType data;
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR)
        {
            try
            {
                payload["Intent"] = to_json(data.intent);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try
            {
                payload["RequestedProtocol"] = to_json(data.requestedProtocol);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try
            {
                payload["TransferFileDesignator"] = to_json(data.transferFileDesignator);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
        }
    }
    break;
    }

    if (!cmd.empty())
    {
        ctxt.mCommandHandler.AddStatus(ctxt.mRequestPath, Protocols::InteractionModel::Status::Success);
        send_unify_mqtt_cmd(ctxt, cmd, payload);
        sl_log_debug(LOG_TAG, "Mapped [%] command to unify dotdot data model", cmd.c_str());
    }
    else
    {
        ctxt.mCommandHandler.AddStatus(ctxt.mRequestPath, Protocols::InteractionModel::Status::UnsupportedCommand);
    }
    ctxt.SetCommandHandled();
}

void GeneralDiagnosticsClusterCommandHandler::InvokeCommand(CommandHandlerInterface::HandlerContext & ctxt)
{
    using namespace chip::app::Clusters::GeneralDiagnostics;

    auto unify_node = m_node_state_monitor.bridged_endpoint(ctxt.mRequestPath.mEndpointId);
    if (!unify_node)
    {
        sl_log_info(LOG_TAG, "The endpoint [%i] is not a part of unify matter bridge node", ctxt.mRequestPath.mEndpointId);
        return;
    }

    std::string cmd;
    nlohmann::json payload = {};

    switch (ctxt.mRequestPath.mCommandId)
    {
    case Commands::TestEventTrigger::Id: {
        cmd = "TestEventTrigger";
        Commands::TestEventTrigger::DecodableType data;
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR)
        {
            try
            {
                payload["EnableKey"] = to_json(data.enableKey);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try
            {
                payload["EventTrigger"] = to_json(data.eventTrigger);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
        }
    }
    break;
    }

    if (!cmd.empty())
    {
        ctxt.mCommandHandler.AddStatus(ctxt.mRequestPath, Protocols::InteractionModel::Status::Success);
        send_unify_mqtt_cmd(ctxt, cmd, payload);
        sl_log_debug(LOG_TAG, "Mapped [%] command to unify dotdot data model", cmd.c_str());
    }
    else
    {
        ctxt.mCommandHandler.AddStatus(ctxt.mRequestPath, Protocols::InteractionModel::Status::UnsupportedCommand);
    }
    ctxt.SetCommandHandled();
}

void SoftwareDiagnosticsClusterCommandHandler::InvokeCommand(CommandHandlerInterface::HandlerContext & ctxt)
{
    using namespace chip::app::Clusters::SoftwareDiagnostics;

    auto unify_node = m_node_state_monitor.bridged_endpoint(ctxt.mRequestPath.mEndpointId);
    if (!unify_node)
    {
        sl_log_info(LOG_TAG, "The endpoint [%i] is not a part of unify matter bridge node", ctxt.mRequestPath.mEndpointId);
        return;
    }

    std::string cmd;
    nlohmann::json payload = {};

    switch (ctxt.mRequestPath.mCommandId)
    {
    case Commands::ResetWatermarks::Id: {
        cmd = "ResetWatermarks";
        Commands::ResetWatermarks::DecodableType data;
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR)
        {
        }
    }
    break;
    }

    if (!cmd.empty())
    {
        ctxt.mCommandHandler.AddStatus(ctxt.mRequestPath, Protocols::InteractionModel::Status::Success);
        send_unify_mqtt_cmd(ctxt, cmd, payload);
        sl_log_debug(LOG_TAG, "Mapped [%] command to unify dotdot data model", cmd.c_str());
    }
    else
    {
        ctxt.mCommandHandler.AddStatus(ctxt.mRequestPath, Protocols::InteractionModel::Status::UnsupportedCommand);
    }
    ctxt.SetCommandHandled();
}

void ThreadNetworkDiagnosticsClusterCommandHandler::InvokeCommand(CommandHandlerInterface::HandlerContext & ctxt)
{
    using namespace chip::app::Clusters::ThreadNetworkDiagnostics;

    auto unify_node = m_node_state_monitor.bridged_endpoint(ctxt.mRequestPath.mEndpointId);
    if (!unify_node)
    {
        sl_log_info(LOG_TAG, "The endpoint [%i] is not a part of unify matter bridge node", ctxt.mRequestPath.mEndpointId);
        return;
    }

    std::string cmd;
    nlohmann::json payload = {};

    switch (ctxt.mRequestPath.mCommandId)
    {
    case Commands::ResetCounts::Id: {
        cmd = "ResetCounts";
        Commands::ResetCounts::DecodableType data;
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR)
        {
        }
    }
    break;
    }

    if (!cmd.empty())
    {
        ctxt.mCommandHandler.AddStatus(ctxt.mRequestPath, Protocols::InteractionModel::Status::Success);
        send_unify_mqtt_cmd(ctxt, cmd, payload);
        sl_log_debug(LOG_TAG, "Mapped [%] command to unify dotdot data model", cmd.c_str());
    }
    else
    {
        ctxt.mCommandHandler.AddStatus(ctxt.mRequestPath, Protocols::InteractionModel::Status::UnsupportedCommand);
    }
    ctxt.SetCommandHandled();
}

void WiFiNetworkDiagnosticsClusterCommandHandler::InvokeCommand(CommandHandlerInterface::HandlerContext & ctxt)
{
    using namespace chip::app::Clusters::WiFiNetworkDiagnostics;

    auto unify_node = m_node_state_monitor.bridged_endpoint(ctxt.mRequestPath.mEndpointId);
    if (!unify_node)
    {
        sl_log_info(LOG_TAG, "The endpoint [%i] is not a part of unify matter bridge node", ctxt.mRequestPath.mEndpointId);
        return;
    }

    std::string cmd;
    nlohmann::json payload = {};

    switch (ctxt.mRequestPath.mCommandId)
    {
    case Commands::ResetCounts::Id: {
        cmd = "ResetCounts";
        Commands::ResetCounts::DecodableType data;
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR)
        {
        }
    }
    break;
    }

    if (!cmd.empty())
    {
        ctxt.mCommandHandler.AddStatus(ctxt.mRequestPath, Protocols::InteractionModel::Status::Success);
        send_unify_mqtt_cmd(ctxt, cmd, payload);
        sl_log_debug(LOG_TAG, "Mapped [%] command to unify dotdot data model", cmd.c_str());
    }
    else
    {
        ctxt.mCommandHandler.AddStatus(ctxt.mRequestPath, Protocols::InteractionModel::Status::UnsupportedCommand);
    }
    ctxt.SetCommandHandled();
}

void EthernetNetworkDiagnosticsClusterCommandHandler::InvokeCommand(CommandHandlerInterface::HandlerContext & ctxt)
{
    using namespace chip::app::Clusters::EthernetNetworkDiagnostics;

    auto unify_node = m_node_state_monitor.bridged_endpoint(ctxt.mRequestPath.mEndpointId);
    if (!unify_node)
    {
        sl_log_info(LOG_TAG, "The endpoint [%i] is not a part of unify matter bridge node", ctxt.mRequestPath.mEndpointId);
        return;
    }

    std::string cmd;
    nlohmann::json payload = {};

    switch (ctxt.mRequestPath.mCommandId)
    {
    case Commands::ResetCounts::Id: {
        cmd = "ResetCounts";
        Commands::ResetCounts::DecodableType data;
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR)
        {
        }
    }
    break;
    }

    if (!cmd.empty())
    {
        ctxt.mCommandHandler.AddStatus(ctxt.mRequestPath, Protocols::InteractionModel::Status::Success);
        send_unify_mqtt_cmd(ctxt, cmd, payload);
        sl_log_debug(LOG_TAG, "Mapped [%] command to unify dotdot data model", cmd.c_str());
    }
    else
    {
        ctxt.mCommandHandler.AddStatus(ctxt.mRequestPath, Protocols::InteractionModel::Status::UnsupportedCommand);
    }
    ctxt.SetCommandHandled();
}

void TimeSynchronizationClusterCommandHandler::InvokeCommand(CommandHandlerInterface::HandlerContext & ctxt)
{
    using namespace chip::app::Clusters::TimeSynchronization;

    auto unify_node = m_node_state_monitor.bridged_endpoint(ctxt.mRequestPath.mEndpointId);
    if (!unify_node)
    {
        sl_log_info(LOG_TAG, "The endpoint [%i] is not a part of unify matter bridge node", ctxt.mRequestPath.mEndpointId);
        return;
    }

    std::string cmd;
    nlohmann::json payload = {};

    switch (ctxt.mRequestPath.mCommandId)
    {
    case Commands::SetUtcTime::Id: {
        cmd = "SetUtcTime";
        Commands::SetUtcTime::DecodableType data;
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR)
        {
            try
            {
                payload["UtcTime"] = to_json(data.utcTime);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try
            {
                payload["Granularity"] = to_json(data.granularity);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            if (data.timeSource.HasValue())
            {
                try
                {
                    payload["TimeSource"] = to_json(data.timeSource.Value());
                } catch (std::exception & ex)
                {
                    sl_log_warning(LOG_TAG, "Failed to add the command arguments value to json format: %s", ex.what());
                }
            }
        }
    }
    break;
    }

    if (!cmd.empty())
    {
        ctxt.mCommandHandler.AddStatus(ctxt.mRequestPath, Protocols::InteractionModel::Status::Success);
        send_unify_mqtt_cmd(ctxt, cmd, payload);
        sl_log_debug(LOG_TAG, "Mapped [%] command to unify dotdot data model", cmd.c_str());
    }
    else
    {
        ctxt.mCommandHandler.AddStatus(ctxt.mRequestPath, Protocols::InteractionModel::Status::UnsupportedCommand);
    }
    ctxt.SetCommandHandled();
}

void BridgedDeviceBasicClusterCommandHandler::InvokeCommand(CommandHandlerInterface::HandlerContext & ctxt)
{
    using namespace chip::app::Clusters::BridgedDeviceBasic;

    auto unify_node = m_node_state_monitor.bridged_endpoint(ctxt.mRequestPath.mEndpointId);
    if (!unify_node)
    {
        sl_log_info(LOG_TAG, "The endpoint [%i] is not a part of unify matter bridge node", ctxt.mRequestPath.mEndpointId);
        return;
    }

    std::string cmd;
    nlohmann::json payload = {};

    switch (ctxt.mRequestPath.mCommandId)
    {
    }

    if (!cmd.empty())
    {
        ctxt.mCommandHandler.AddStatus(ctxt.mRequestPath, Protocols::InteractionModel::Status::Success);
        send_unify_mqtt_cmd(ctxt, cmd, payload);
        sl_log_debug(LOG_TAG, "Mapped [%] command to unify dotdot data model", cmd.c_str());
    }
    else
    {
        ctxt.mCommandHandler.AddStatus(ctxt.mRequestPath, Protocols::InteractionModel::Status::UnsupportedCommand);
    }
    ctxt.SetCommandHandled();
}

void SwitchClusterCommandHandler::InvokeCommand(CommandHandlerInterface::HandlerContext & ctxt)
{
    using namespace chip::app::Clusters::Switch;

    auto unify_node = m_node_state_monitor.bridged_endpoint(ctxt.mRequestPath.mEndpointId);
    if (!unify_node)
    {
        sl_log_info(LOG_TAG, "The endpoint [%i] is not a part of unify matter bridge node", ctxt.mRequestPath.mEndpointId);
        return;
    }

    std::string cmd;
    nlohmann::json payload = {};

    switch (ctxt.mRequestPath.mCommandId)
    {
    }

    if (!cmd.empty())
    {
        ctxt.mCommandHandler.AddStatus(ctxt.mRequestPath, Protocols::InteractionModel::Status::Success);
        send_unify_mqtt_cmd(ctxt, cmd, payload);
        sl_log_debug(LOG_TAG, "Mapped [%] command to unify dotdot data model", cmd.c_str());
    }
    else
    {
        ctxt.mCommandHandler.AddStatus(ctxt.mRequestPath, Protocols::InteractionModel::Status::UnsupportedCommand);
    }
    ctxt.SetCommandHandled();
}

void AdministratorCommissioningClusterCommandHandler::InvokeCommand(CommandHandlerInterface::HandlerContext & ctxt)
{
    using namespace chip::app::Clusters::AdministratorCommissioning;

    auto unify_node = m_node_state_monitor.bridged_endpoint(ctxt.mRequestPath.mEndpointId);
    if (!unify_node)
    {
        sl_log_info(LOG_TAG, "The endpoint [%i] is not a part of unify matter bridge node", ctxt.mRequestPath.mEndpointId);
        return;
    }

    std::string cmd;
    nlohmann::json payload = {};

    switch (ctxt.mRequestPath.mCommandId)
    {
    case Commands::OpenCommissioningWindow::Id: {
        cmd = "OpenCommissioningWindow";
        Commands::OpenCommissioningWindow::DecodableType data;
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR)
        {
            try
            {
                payload["CommissioningTimeout"] = to_json(data.commissioningTimeout);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try
            {
                payload["PAKEVerifier"] = to_json(data.PAKEVerifier);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try
            {
                payload["Discriminator"] = to_json(data.discriminator);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try
            {
                payload["Iterations"] = to_json(data.iterations);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try
            {
                payload["Salt"] = to_json(data.salt);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
        }
    }
    break;
    case Commands::OpenBasicCommissioningWindow::Id: {
        cmd = "OpenBasicCommissioningWindow";
        Commands::OpenBasicCommissioningWindow::DecodableType data;
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR)
        {
            try
            {
                payload["CommissioningTimeout"] = to_json(data.commissioningTimeout);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
        }
    }
    break;
    case Commands::RevokeCommissioning::Id: {
        cmd = "RevokeCommissioning";
        Commands::RevokeCommissioning::DecodableType data;
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR)
        {
        }
    }
    break;
    }

    if (!cmd.empty())
    {
        ctxt.mCommandHandler.AddStatus(ctxt.mRequestPath, Protocols::InteractionModel::Status::Success);
        send_unify_mqtt_cmd(ctxt, cmd, payload);
        sl_log_debug(LOG_TAG, "Mapped [%] command to unify dotdot data model", cmd.c_str());
    }
    else
    {
        ctxt.mCommandHandler.AddStatus(ctxt.mRequestPath, Protocols::InteractionModel::Status::UnsupportedCommand);
    }
    ctxt.SetCommandHandled();
}

void OperationalCredentialsClusterCommandHandler::InvokeCommand(CommandHandlerInterface::HandlerContext & ctxt)
{
    using namespace chip::app::Clusters::OperationalCredentials;

    auto unify_node = m_node_state_monitor.bridged_endpoint(ctxt.mRequestPath.mEndpointId);
    if (!unify_node)
    {
        sl_log_info(LOG_TAG, "The endpoint [%i] is not a part of unify matter bridge node", ctxt.mRequestPath.mEndpointId);
        return;
    }

    std::string cmd;
    nlohmann::json payload = {};

    switch (ctxt.mRequestPath.mCommandId)
    {
    case Commands::AttestationRequest::Id: {
        cmd = "AttestationRequest";
        Commands::AttestationRequest::DecodableType data;
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR)
        {
            try
            {
                payload["AttestationNonce"] = to_json(data.attestationNonce);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
        }
    }
    break;
    case Commands::CertificateChainRequest::Id: {
        cmd = "CertificateChainRequest";
        Commands::CertificateChainRequest::DecodableType data;
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR)
        {
            try
            {
                payload["CertificateType"] = to_json(data.certificateType);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
        }
    }
    break;
    case Commands::CSRRequest::Id: {
        cmd = "CSRRequest";
        Commands::CSRRequest::DecodableType data;
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR)
        {
            try
            {
                payload["CSRNonce"] = to_json(data.CSRNonce);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            if (data.isForUpdateNOC.HasValue())
            {
                try
                {
                    payload["IsForUpdateNOC"] = to_json(data.isForUpdateNOC.Value());
                } catch (std::exception & ex)
                {
                    sl_log_warning(LOG_TAG, "Failed to add the command arguments value to json format: %s", ex.what());
                }
            }
        }
    }
    break;
    case Commands::AddNOC::Id: {
        cmd = "AddNOC";
        Commands::AddNOC::DecodableType data;
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR)
        {
            try
            {
                payload["NOCValue"] = to_json(data.NOCValue);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            if (data.ICACValue.HasValue())
            {
                try
                {
                    payload["ICACValue"] = to_json(data.ICACValue.Value());
                } catch (std::exception & ex)
                {
                    sl_log_warning(LOG_TAG, "Failed to add the command arguments value to json format: %s", ex.what());
                }
            }
            try
            {
                payload["IPKValue"] = to_json(data.IPKValue);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try
            {
                payload["CaseAdminSubject"] = to_json(data.caseAdminSubject);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try
            {
                payload["AdminVendorId"] = to_json(data.adminVendorId);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
        }
    }
    break;
    case Commands::UpdateNOC::Id: {
        cmd = "UpdateNOC";
        Commands::UpdateNOC::DecodableType data;
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR)
        {
            try
            {
                payload["NOCValue"] = to_json(data.NOCValue);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            if (data.ICACValue.HasValue())
            {
                try
                {
                    payload["ICACValue"] = to_json(data.ICACValue.Value());
                } catch (std::exception & ex)
                {
                    sl_log_warning(LOG_TAG, "Failed to add the command arguments value to json format: %s", ex.what());
                }
            }
        }
    }
    break;
    case Commands::UpdateFabricLabel::Id: {
        cmd = "UpdateFabricLabel";
        Commands::UpdateFabricLabel::DecodableType data;
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR)
        {
            try
            {
                payload["Label"] = to_json(data.label);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
        }
    }
    break;
    case Commands::RemoveFabric::Id: {
        cmd = "RemoveFabric";
        Commands::RemoveFabric::DecodableType data;
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR)
        {
            try
            {
                payload["FabricIndex"] = to_json(data.fabricIndex);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
        }
    }
    break;
    case Commands::AddTrustedRootCertificate::Id: {
        cmd = "AddTrustedRootCertificate";
        Commands::AddTrustedRootCertificate::DecodableType data;
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR)
        {
            try
            {
                payload["RootCertificate"] = to_json(data.rootCertificate);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
        }
    }
    break;
    }

    if (!cmd.empty())
    {
        ctxt.mCommandHandler.AddStatus(ctxt.mRequestPath, Protocols::InteractionModel::Status::Success);
        send_unify_mqtt_cmd(ctxt, cmd, payload);
        sl_log_debug(LOG_TAG, "Mapped [%] command to unify dotdot data model", cmd.c_str());
    }
    else
    {
        ctxt.mCommandHandler.AddStatus(ctxt.mRequestPath, Protocols::InteractionModel::Status::UnsupportedCommand);
    }
    ctxt.SetCommandHandled();
}

void GroupKeyManagementClusterCommandHandler::InvokeCommand(CommandHandlerInterface::HandlerContext & ctxt)
{
    using namespace chip::app::Clusters::GroupKeyManagement;

    auto unify_node = m_node_state_monitor.bridged_endpoint(ctxt.mRequestPath.mEndpointId);
    if (!unify_node)
    {
        sl_log_info(LOG_TAG, "The endpoint [%i] is not a part of unify matter bridge node", ctxt.mRequestPath.mEndpointId);
        return;
    }

    std::string cmd;
    nlohmann::json payload = {};

    switch (ctxt.mRequestPath.mCommandId)
    {
    case Commands::KeySetWrite::Id: {
        cmd = "KeySetWrite";
        Commands::KeySetWrite::DecodableType data;
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR)
        {
            try
            {
                payload["GroupKeySet"] = to_json(data.groupKeySet);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
        }
    }
    break;
    case Commands::KeySetRead::Id: {
        cmd = "KeySetRead";
        Commands::KeySetRead::DecodableType data;
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR)
        {
            try
            {
                payload["GroupKeySetID"] = to_json(data.groupKeySetID);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
        }
    }
    break;
    case Commands::KeySetRemove::Id: {
        cmd = "KeySetRemove";
        Commands::KeySetRemove::DecodableType data;
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR)
        {
            try
            {
                payload["GroupKeySetID"] = to_json(data.groupKeySetID);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
        }
    }
    break;
    case Commands::KeySetReadAllIndices::Id: {
        cmd = "KeySetReadAllIndices";
        Commands::KeySetReadAllIndices::DecodableType data;
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR)
        {
            try
            {
                payload["GroupKeySetIDs"] = to_json(data.groupKeySetIDs);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
        }
    }
    break;
    }

    if (!cmd.empty())
    {
        ctxt.mCommandHandler.AddStatus(ctxt.mRequestPath, Protocols::InteractionModel::Status::Success);
        send_unify_mqtt_cmd(ctxt, cmd, payload);
        sl_log_debug(LOG_TAG, "Mapped [%] command to unify dotdot data model", cmd.c_str());
    }
    else
    {
        ctxt.mCommandHandler.AddStatus(ctxt.mRequestPath, Protocols::InteractionModel::Status::UnsupportedCommand);
    }
    ctxt.SetCommandHandled();
}

void FixedLabelClusterCommandHandler::InvokeCommand(CommandHandlerInterface::HandlerContext & ctxt)
{
    using namespace chip::app::Clusters::FixedLabel;

    auto unify_node = m_node_state_monitor.bridged_endpoint(ctxt.mRequestPath.mEndpointId);
    if (!unify_node)
    {
        sl_log_info(LOG_TAG, "The endpoint [%i] is not a part of unify matter bridge node", ctxt.mRequestPath.mEndpointId);
        return;
    }

    std::string cmd;
    nlohmann::json payload = {};

    switch (ctxt.mRequestPath.mCommandId)
    {
    }

    if (!cmd.empty())
    {
        ctxt.mCommandHandler.AddStatus(ctxt.mRequestPath, Protocols::InteractionModel::Status::Success);
        send_unify_mqtt_cmd(ctxt, cmd, payload);
        sl_log_debug(LOG_TAG, "Mapped [%] command to unify dotdot data model", cmd.c_str());
    }
    else
    {
        ctxt.mCommandHandler.AddStatus(ctxt.mRequestPath, Protocols::InteractionModel::Status::UnsupportedCommand);
    }
    ctxt.SetCommandHandled();
}

void UserLabelClusterCommandHandler::InvokeCommand(CommandHandlerInterface::HandlerContext & ctxt)
{
    using namespace chip::app::Clusters::UserLabel;

    auto unify_node = m_node_state_monitor.bridged_endpoint(ctxt.mRequestPath.mEndpointId);
    if (!unify_node)
    {
        sl_log_info(LOG_TAG, "The endpoint [%i] is not a part of unify matter bridge node", ctxt.mRequestPath.mEndpointId);
        return;
    }

    std::string cmd;
    nlohmann::json payload = {};

    switch (ctxt.mRequestPath.mCommandId)
    {
    }

    if (!cmd.empty())
    {
        ctxt.mCommandHandler.AddStatus(ctxt.mRequestPath, Protocols::InteractionModel::Status::Success);
        send_unify_mqtt_cmd(ctxt, cmd, payload);
        sl_log_debug(LOG_TAG, "Mapped [%] command to unify dotdot data model", cmd.c_str());
    }
    else
    {
        ctxt.mCommandHandler.AddStatus(ctxt.mRequestPath, Protocols::InteractionModel::Status::UnsupportedCommand);
    }
    ctxt.SetCommandHandled();
}

void ProxyConfigurationClusterCommandHandler::InvokeCommand(CommandHandlerInterface::HandlerContext & ctxt)
{
    using namespace chip::app::Clusters::ProxyConfiguration;

    auto unify_node = m_node_state_monitor.bridged_endpoint(ctxt.mRequestPath.mEndpointId);
    if (!unify_node)
    {
        sl_log_info(LOG_TAG, "The endpoint [%i] is not a part of unify matter bridge node", ctxt.mRequestPath.mEndpointId);
        return;
    }

    std::string cmd;
    nlohmann::json payload = {};

    switch (ctxt.mRequestPath.mCommandId)
    {
    }

    if (!cmd.empty())
    {
        ctxt.mCommandHandler.AddStatus(ctxt.mRequestPath, Protocols::InteractionModel::Status::Success);
        send_unify_mqtt_cmd(ctxt, cmd, payload);
        sl_log_debug(LOG_TAG, "Mapped [%] command to unify dotdot data model", cmd.c_str());
    }
    else
    {
        ctxt.mCommandHandler.AddStatus(ctxt.mRequestPath, Protocols::InteractionModel::Status::UnsupportedCommand);
    }
    ctxt.SetCommandHandled();
}

void ProxyDiscoveryClusterCommandHandler::InvokeCommand(CommandHandlerInterface::HandlerContext & ctxt)
{
    using namespace chip::app::Clusters::ProxyDiscovery;

    auto unify_node = m_node_state_monitor.bridged_endpoint(ctxt.mRequestPath.mEndpointId);
    if (!unify_node)
    {
        sl_log_info(LOG_TAG, "The endpoint [%i] is not a part of unify matter bridge node", ctxt.mRequestPath.mEndpointId);
        return;
    }

    std::string cmd;
    nlohmann::json payload = {};

    switch (ctxt.mRequestPath.mCommandId)
    {
    }

    if (!cmd.empty())
    {
        ctxt.mCommandHandler.AddStatus(ctxt.mRequestPath, Protocols::InteractionModel::Status::Success);
        send_unify_mqtt_cmd(ctxt, cmd, payload);
        sl_log_debug(LOG_TAG, "Mapped [%] command to unify dotdot data model", cmd.c_str());
    }
    else
    {
        ctxt.mCommandHandler.AddStatus(ctxt.mRequestPath, Protocols::InteractionModel::Status::UnsupportedCommand);
    }
    ctxt.SetCommandHandled();
}

void ProxyValidClusterCommandHandler::InvokeCommand(CommandHandlerInterface::HandlerContext & ctxt)
{
    using namespace chip::app::Clusters::ProxyValid;

    auto unify_node = m_node_state_monitor.bridged_endpoint(ctxt.mRequestPath.mEndpointId);
    if (!unify_node)
    {
        sl_log_info(LOG_TAG, "The endpoint [%i] is not a part of unify matter bridge node", ctxt.mRequestPath.mEndpointId);
        return;
    }

    std::string cmd;
    nlohmann::json payload = {};

    switch (ctxt.mRequestPath.mCommandId)
    {
    }

    if (!cmd.empty())
    {
        ctxt.mCommandHandler.AddStatus(ctxt.mRequestPath, Protocols::InteractionModel::Status::Success);
        send_unify_mqtt_cmd(ctxt, cmd, payload);
        sl_log_debug(LOG_TAG, "Mapped [%] command to unify dotdot data model", cmd.c_str());
    }
    else
    {
        ctxt.mCommandHandler.AddStatus(ctxt.mRequestPath, Protocols::InteractionModel::Status::UnsupportedCommand);
    }
    ctxt.SetCommandHandled();
}

void BooleanStateClusterCommandHandler::InvokeCommand(CommandHandlerInterface::HandlerContext & ctxt)
{
    using namespace chip::app::Clusters::BooleanState;

    auto unify_node = m_node_state_monitor.bridged_endpoint(ctxt.mRequestPath.mEndpointId);
    if (!unify_node)
    {
        sl_log_info(LOG_TAG, "The endpoint [%i] is not a part of unify matter bridge node", ctxt.mRequestPath.mEndpointId);
        return;
    }

    std::string cmd;
    nlohmann::json payload = {};

    switch (ctxt.mRequestPath.mCommandId)
    {
    }

    if (!cmd.empty())
    {
        ctxt.mCommandHandler.AddStatus(ctxt.mRequestPath, Protocols::InteractionModel::Status::Success);
        send_unify_mqtt_cmd(ctxt, cmd, payload);
        sl_log_debug(LOG_TAG, "Mapped [%] command to unify dotdot data model", cmd.c_str());
    }
    else
    {
        ctxt.mCommandHandler.AddStatus(ctxt.mRequestPath, Protocols::InteractionModel::Status::UnsupportedCommand);
    }
    ctxt.SetCommandHandled();
}

void ModeSelectClusterCommandHandler::InvokeCommand(CommandHandlerInterface::HandlerContext & ctxt)
{
    using namespace chip::app::Clusters::ModeSelect;

    auto unify_node = m_node_state_monitor.bridged_endpoint(ctxt.mRequestPath.mEndpointId);
    if (!unify_node)
    {
        sl_log_info(LOG_TAG, "The endpoint [%i] is not a part of unify matter bridge node", ctxt.mRequestPath.mEndpointId);
        return;
    }

    std::string cmd;
    nlohmann::json payload = {};

    switch (ctxt.mRequestPath.mCommandId)
    {
    case Commands::ChangeToMode::Id: {
        cmd = "ChangeToMode";
        Commands::ChangeToMode::DecodableType data;
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR)
        {
            try
            {
                payload["NewMode"] = to_json(data.newMode);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
        }
    }
    break;
    }

    if (!cmd.empty())
    {
        ctxt.mCommandHandler.AddStatus(ctxt.mRequestPath, Protocols::InteractionModel::Status::Success);
        send_unify_mqtt_cmd(ctxt, cmd, payload);
        sl_log_debug(LOG_TAG, "Mapped [%] command to unify dotdot data model", cmd.c_str());
    }
    else
    {
        ctxt.mCommandHandler.AddStatus(ctxt.mRequestPath, Protocols::InteractionModel::Status::UnsupportedCommand);
    }
    ctxt.SetCommandHandled();
}

void DoorLockClusterCommandHandler::InvokeCommand(CommandHandlerInterface::HandlerContext & ctxt)
{
    using namespace chip::app::Clusters::DoorLock;

    auto unify_node = m_node_state_monitor.bridged_endpoint(ctxt.mRequestPath.mEndpointId);
    if (!unify_node)
    {
        sl_log_info(LOG_TAG, "The endpoint [%i] is not a part of unify matter bridge node", ctxt.mRequestPath.mEndpointId);
        return;
    }

    std::string cmd;
    nlohmann::json payload = {};

    switch (ctxt.mRequestPath.mCommandId)
    {
    case Commands::LockDoor::Id: {
        cmd = "LockDoor";
        Commands::LockDoor::DecodableType data;
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR)
        {
            if (data.pinCode.HasValue())
            {
                try
                {
                    payload["PinCode"] = to_json(data.pinCode.Value());
                } catch (std::exception & ex)
                {
                    sl_log_warning(LOG_TAG, "Failed to add the command arguments value to json format: %s", ex.what());
                }
            }
        }
    }
    break;
    case Commands::UnlockDoor::Id: {
        cmd = "UnlockDoor";
        Commands::UnlockDoor::DecodableType data;
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR)
        {
            if (data.pinCode.HasValue())
            {
                try
                {
                    payload["PinCode"] = to_json(data.pinCode.Value());
                } catch (std::exception & ex)
                {
                    sl_log_warning(LOG_TAG, "Failed to add the command arguments value to json format: %s", ex.what());
                }
            }
        }
    }
    break;
    case Commands::UnlockWithTimeout::Id: {
        cmd = "UnlockWithTimeout";
        Commands::UnlockWithTimeout::DecodableType data;
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR)
        {
            try
            {
                payload["Timeout"] = to_json(data.timeout);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            if (data.pinCode.HasValue())
            {
                try
                {
                    payload["PinCode"] = to_json(data.pinCode.Value());
                } catch (std::exception & ex)
                {
                    sl_log_warning(LOG_TAG, "Failed to add the command arguments value to json format: %s", ex.what());
                }
            }
        }
    }
    break;
    case Commands::SetWeekDaySchedule::Id: {
        cmd = "SetWeekDaySchedule";
        Commands::SetWeekDaySchedule::DecodableType data;
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR)
        {
            try
            {
                payload["WeekDayIndex"] = to_json(data.weekDayIndex);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try
            {
                payload["UserIndex"] = to_json(data.userIndex);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try
            {
                payload["DaysMask"] = to_json(data.daysMask);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try
            {
                payload["StartHour"] = to_json(data.startHour);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try
            {
                payload["StartMinute"] = to_json(data.startMinute);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try
            {
                payload["EndHour"] = to_json(data.endHour);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try
            {
                payload["EndMinute"] = to_json(data.endMinute);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
        }
    }
    break;
    case Commands::GetWeekDaySchedule::Id: {
        cmd = "GetWeekDaySchedule";
        Commands::GetWeekDaySchedule::DecodableType data;
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR)
        {
            try
            {
                payload["WeekDayIndex"] = to_json(data.weekDayIndex);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try
            {
                payload["UserIndex"] = to_json(data.userIndex);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
        }
    }
    break;
    case Commands::ClearWeekDaySchedule::Id: {
        cmd = "ClearWeekDaySchedule";
        Commands::ClearWeekDaySchedule::DecodableType data;
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR)
        {
            try
            {
                payload["WeekDayIndex"] = to_json(data.weekDayIndex);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try
            {
                payload["UserIndex"] = to_json(data.userIndex);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
        }
    }
    break;
    case Commands::SetYearDaySchedule::Id: {
        cmd = "SetYearDaySchedule";
        Commands::SetYearDaySchedule::DecodableType data;
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR)
        {
            try
            {
                payload["YearDayIndex"] = to_json(data.yearDayIndex);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try
            {
                payload["UserIndex"] = to_json(data.userIndex);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try
            {
                payload["LocalStartTime"] = to_json(data.localStartTime);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try
            {
                payload["LocalEndTime"] = to_json(data.localEndTime);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
        }
    }
    break;
    case Commands::GetYearDaySchedule::Id: {
        cmd = "GetYearDaySchedule";
        Commands::GetYearDaySchedule::DecodableType data;
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR)
        {
            try
            {
                payload["YearDayIndex"] = to_json(data.yearDayIndex);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try
            {
                payload["UserIndex"] = to_json(data.userIndex);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
        }
    }
    break;
    case Commands::ClearYearDaySchedule::Id: {
        cmd = "ClearYearDaySchedule";
        Commands::ClearYearDaySchedule::DecodableType data;
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR)
        {
            try
            {
                payload["YearDayIndex"] = to_json(data.yearDayIndex);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try
            {
                payload["UserIndex"] = to_json(data.userIndex);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
        }
    }
    break;
    case Commands::SetHolidaySchedule::Id: {
        cmd = "SetHolidaySchedule";
        Commands::SetHolidaySchedule::DecodableType data;
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR)
        {
            try
            {
                payload["HolidayIndex"] = to_json(data.holidayIndex);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try
            {
                payload["LocalStartTime"] = to_json(data.localStartTime);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try
            {
                payload["LocalEndTime"] = to_json(data.localEndTime);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try
            {
                payload["OperatingMode"] = to_json(data.operatingMode);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
        }
    }
    break;
    case Commands::GetHolidaySchedule::Id: {
        cmd = "GetHolidaySchedule";
        Commands::GetHolidaySchedule::DecodableType data;
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR)
        {
            try
            {
                payload["HolidayIndex"] = to_json(data.holidayIndex);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
        }
    }
    break;
    case Commands::ClearHolidaySchedule::Id: {
        cmd = "ClearHolidaySchedule";
        Commands::ClearHolidaySchedule::DecodableType data;
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR)
        {
            try
            {
                payload["HolidayIndex"] = to_json(data.holidayIndex);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
        }
    }
    break;
    case Commands::SetUser::Id: {
        cmd = "SetUser";
        Commands::SetUser::DecodableType data;
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR)
        {
            try
            {
                payload["OperationType"] = to_json(data.operationType);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try
            {
                payload["UserIndex"] = to_json(data.userIndex);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try
            {
                payload["UserName"] = to_json(data.userName);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try
            {
                payload["UserUniqueId"] = to_json(data.userUniqueId);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try
            {
                payload["UserStatus"] = to_json(data.userStatus);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try
            {
                payload["UserType"] = to_json(data.userType);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try
            {
                payload["CredentialRule"] = to_json(data.credentialRule);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
        }
    }
    break;
    case Commands::GetUser::Id: {
        cmd = "GetUser";
        Commands::GetUser::DecodableType data;
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR)
        {
            try
            {
                payload["UserIndex"] = to_json(data.userIndex);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
        }
    }
    break;
    case Commands::ClearUser::Id: {
        cmd = "ClearUser";
        Commands::ClearUser::DecodableType data;
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR)
        {
            try
            {
                payload["UserIndex"] = to_json(data.userIndex);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
        }
    }
    break;
    case Commands::SetCredential::Id: {
        cmd = "SetCredential";
        Commands::SetCredential::DecodableType data;
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR)
        {
            try
            {
                payload["OperationType"] = to_json(data.operationType);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try
            {
                payload["Credential"] = to_json(data.credential);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try
            {
                payload["CredentialData"] = to_json(data.credentialData);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try
            {
                payload["UserIndex"] = to_json(data.userIndex);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try
            {
                payload["UserStatus"] = to_json(data.userStatus);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try
            {
                payload["UserType"] = to_json(data.userType);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
        }
    }
    break;
    case Commands::GetCredentialStatus::Id: {
        cmd = "GetCredentialStatus";
        Commands::GetCredentialStatus::DecodableType data;
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR)
        {
            try
            {
                payload["Credential"] = to_json(data.credential);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
        }
    }
    break;
    case Commands::ClearCredential::Id: {
        cmd = "ClearCredential";
        Commands::ClearCredential::DecodableType data;
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR)
        {
            try
            {
                payload["Credential"] = to_json(data.credential);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
        }
    }
    break;
    }

    if (!cmd.empty())
    {
        ctxt.mCommandHandler.AddStatus(ctxt.mRequestPath, Protocols::InteractionModel::Status::Success);
        send_unify_mqtt_cmd(ctxt, cmd, payload);
        sl_log_debug(LOG_TAG, "Mapped [%] command to unify dotdot data model", cmd.c_str());
    }
    else
    {
        ctxt.mCommandHandler.AddStatus(ctxt.mRequestPath, Protocols::InteractionModel::Status::UnsupportedCommand);
    }
    ctxt.SetCommandHandled();
}

void WindowCoveringClusterCommandHandler::InvokeCommand(CommandHandlerInterface::HandlerContext & ctxt)
{
    using namespace chip::app::Clusters::WindowCovering;

    auto unify_node = m_node_state_monitor.bridged_endpoint(ctxt.mRequestPath.mEndpointId);
    if (!unify_node)
    {
        sl_log_info(LOG_TAG, "The endpoint [%i] is not a part of unify matter bridge node", ctxt.mRequestPath.mEndpointId);
        return;
    }

    std::string cmd;
    nlohmann::json payload = {};

    switch (ctxt.mRequestPath.mCommandId)
    {
    case Commands::UpOrOpen::Id: {
        cmd = "UpOrOpen";
        Commands::UpOrOpen::DecodableType data;
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR)
        {
        }
    }
    break;
    case Commands::DownOrClose::Id: {
        cmd = "DownOrClose";
        Commands::DownOrClose::DecodableType data;
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR)
        {
        }
    }
    break;
    case Commands::StopMotion::Id: {
        cmd = "StopMotion";
        Commands::StopMotion::DecodableType data;
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR)
        {
        }
    }
    break;
    case Commands::GoToLiftValue::Id: {
        cmd = "GoToLiftValue";
        Commands::GoToLiftValue::DecodableType data;
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR)
        {
            try
            {
                payload["LiftValue"] = to_json(data.liftValue);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
        }
    }
    break;
    case Commands::GoToLiftPercentage::Id: {
        cmd = "GoToLiftPercentage";
        Commands::GoToLiftPercentage::DecodableType data;
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR)
        {
            try
            {
                payload["LiftPercent100thsValue"] = to_json(data.liftPercent100thsValue);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
        }
    }
    break;
    case Commands::GoToTiltValue::Id: {
        cmd = "GoToTiltValue";
        Commands::GoToTiltValue::DecodableType data;
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR)
        {
            try
            {
                payload["TiltValue"] = to_json(data.tiltValue);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
        }
    }
    break;
    case Commands::GoToTiltPercentage::Id: {
        cmd = "GoToTiltPercentage";
        Commands::GoToTiltPercentage::DecodableType data;
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR)
        {
            try
            {
                payload["TiltPercent100thsValue"] = to_json(data.tiltPercent100thsValue);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
        }
    }
    break;
    }

    if (!cmd.empty())
    {
        ctxt.mCommandHandler.AddStatus(ctxt.mRequestPath, Protocols::InteractionModel::Status::Success);
        send_unify_mqtt_cmd(ctxt, cmd, payload);
        sl_log_debug(LOG_TAG, "Mapped [%] command to unify dotdot data model", cmd.c_str());
    }
    else
    {
        ctxt.mCommandHandler.AddStatus(ctxt.mRequestPath, Protocols::InteractionModel::Status::UnsupportedCommand);
    }
    ctxt.SetCommandHandled();
}

void BarrierControlClusterCommandHandler::InvokeCommand(CommandHandlerInterface::HandlerContext & ctxt)
{
    using namespace chip::app::Clusters::BarrierControl;

    auto unify_node = m_node_state_monitor.bridged_endpoint(ctxt.mRequestPath.mEndpointId);
    if (!unify_node)
    {
        sl_log_info(LOG_TAG, "The endpoint [%i] is not a part of unify matter bridge node", ctxt.mRequestPath.mEndpointId);
        return;
    }

    std::string cmd;
    nlohmann::json payload = {};

    switch (ctxt.mRequestPath.mCommandId)
    {
    case Commands::BarrierControlGoToPercent::Id: {
        cmd = "BarrierControlGoToPercent";
        Commands::BarrierControlGoToPercent::DecodableType data;
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR)
        {
            try
            {
                payload["PercentOpen"] = to_json(data.percentOpen);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
        }
    }
    break;
    case Commands::BarrierControlStop::Id: {
        cmd = "BarrierControlStop";
        Commands::BarrierControlStop::DecodableType data;
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR)
        {
        }
    }
    break;
    }

    if (!cmd.empty())
    {
        ctxt.mCommandHandler.AddStatus(ctxt.mRequestPath, Protocols::InteractionModel::Status::Success);
        send_unify_mqtt_cmd(ctxt, cmd, payload);
        sl_log_debug(LOG_TAG, "Mapped [%] command to unify dotdot data model", cmd.c_str());
    }
    else
    {
        ctxt.mCommandHandler.AddStatus(ctxt.mRequestPath, Protocols::InteractionModel::Status::UnsupportedCommand);
    }
    ctxt.SetCommandHandled();
}

void PumpConfigurationAndControlClusterCommandHandler::InvokeCommand(CommandHandlerInterface::HandlerContext & ctxt)
{
    using namespace chip::app::Clusters::PumpConfigurationAndControl;

    auto unify_node = m_node_state_monitor.bridged_endpoint(ctxt.mRequestPath.mEndpointId);
    if (!unify_node)
    {
        sl_log_info(LOG_TAG, "The endpoint [%i] is not a part of unify matter bridge node", ctxt.mRequestPath.mEndpointId);
        return;
    }

    std::string cmd;
    nlohmann::json payload = {};

    switch (ctxt.mRequestPath.mCommandId)
    {
    }

    if (!cmd.empty())
    {
        ctxt.mCommandHandler.AddStatus(ctxt.mRequestPath, Protocols::InteractionModel::Status::Success);
        send_unify_mqtt_cmd(ctxt, cmd, payload);
        sl_log_debug(LOG_TAG, "Mapped [%] command to unify dotdot data model", cmd.c_str());
    }
    else
    {
        ctxt.mCommandHandler.AddStatus(ctxt.mRequestPath, Protocols::InteractionModel::Status::UnsupportedCommand);
    }
    ctxt.SetCommandHandled();
}

void ThermostatClusterCommandHandler::InvokeCommand(CommandHandlerInterface::HandlerContext & ctxt)
{
    using namespace chip::app::Clusters::Thermostat;

    auto unify_node = m_node_state_monitor.bridged_endpoint(ctxt.mRequestPath.mEndpointId);
    if (!unify_node)
    {
        sl_log_info(LOG_TAG, "The endpoint [%i] is not a part of unify matter bridge node", ctxt.mRequestPath.mEndpointId);
        return;
    }

    std::string cmd;
    nlohmann::json payload = {};

    switch (ctxt.mRequestPath.mCommandId)
    {
    case Commands::SetpointRaiseLower::Id: {
        cmd = "SetpointRaiseLower";
        Commands::SetpointRaiseLower::DecodableType data;
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR)
        {
            try
            {
                payload["Mode"] = to_json(data.mode);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try
            {
                payload["Amount"] = to_json(data.amount);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
        }
    }
    break;
    case Commands::SetWeeklySchedule::Id: {
        cmd = "SetWeeklySchedule";
        Commands::SetWeeklySchedule::DecodableType data;
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR)
        {
            try
            {
                payload["NumberOfTransitionsForSequence"] = to_json(data.numberOfTransitionsForSequence);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try
            {
                payload["DayOfWeekForSequence"] = to_json(data.dayOfWeekForSequence);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try
            {
                payload["ModeForSequence"] = to_json(data.modeForSequence);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try
            {
                payload["Transitions"] = to_json(data.transitions);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
        }
    }
    break;
    case Commands::GetWeeklySchedule::Id: {
        cmd = "GetWeeklySchedule";
        Commands::GetWeeklySchedule::DecodableType data;
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR)
        {
            try
            {
                payload["DaysToReturn"] = to_json(data.daysToReturn);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try
            {
                payload["ModeToReturn"] = to_json(data.modeToReturn);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
        }
    }
    break;
    case Commands::ClearWeeklySchedule::Id: {
        cmd = "ClearWeeklySchedule";
        Commands::ClearWeeklySchedule::DecodableType data;
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR)
        {
        }
    }
    break;
    }

    if (!cmd.empty())
    {
        ctxt.mCommandHandler.AddStatus(ctxt.mRequestPath, Protocols::InteractionModel::Status::Success);
        send_unify_mqtt_cmd(ctxt, cmd, payload);
        sl_log_debug(LOG_TAG, "Mapped [%] command to unify dotdot data model", cmd.c_str());
    }
    else
    {
        ctxt.mCommandHandler.AddStatus(ctxt.mRequestPath, Protocols::InteractionModel::Status::UnsupportedCommand);
    }
    ctxt.SetCommandHandled();
}

void FanControlClusterCommandHandler::InvokeCommand(CommandHandlerInterface::HandlerContext & ctxt)
{
    using namespace chip::app::Clusters::FanControl;

    auto unify_node = m_node_state_monitor.bridged_endpoint(ctxt.mRequestPath.mEndpointId);
    if (!unify_node)
    {
        sl_log_info(LOG_TAG, "The endpoint [%i] is not a part of unify matter bridge node", ctxt.mRequestPath.mEndpointId);
        return;
    }

    std::string cmd;
    nlohmann::json payload = {};

    switch (ctxt.mRequestPath.mCommandId)
    {
    }

    if (!cmd.empty())
    {
        ctxt.mCommandHandler.AddStatus(ctxt.mRequestPath, Protocols::InteractionModel::Status::Success);
        send_unify_mqtt_cmd(ctxt, cmd, payload);
        sl_log_debug(LOG_TAG, "Mapped [%] command to unify dotdot data model", cmd.c_str());
    }
    else
    {
        ctxt.mCommandHandler.AddStatus(ctxt.mRequestPath, Protocols::InteractionModel::Status::UnsupportedCommand);
    }
    ctxt.SetCommandHandled();
}

void ThermostatUserInterfaceConfigurationClusterCommandHandler::InvokeCommand(CommandHandlerInterface::HandlerContext & ctxt)
{
    using namespace chip::app::Clusters::ThermostatUserInterfaceConfiguration;

    auto unify_node = m_node_state_monitor.bridged_endpoint(ctxt.mRequestPath.mEndpointId);
    if (!unify_node)
    {
        sl_log_info(LOG_TAG, "The endpoint [%i] is not a part of unify matter bridge node", ctxt.mRequestPath.mEndpointId);
        return;
    }

    std::string cmd;
    nlohmann::json payload = {};

    switch (ctxt.mRequestPath.mCommandId)
    {
    }

    if (!cmd.empty())
    {
        ctxt.mCommandHandler.AddStatus(ctxt.mRequestPath, Protocols::InteractionModel::Status::Success);
        send_unify_mqtt_cmd(ctxt, cmd, payload);
        sl_log_debug(LOG_TAG, "Mapped [%] command to unify dotdot data model", cmd.c_str());
    }
    else
    {
        ctxt.mCommandHandler.AddStatus(ctxt.mRequestPath, Protocols::InteractionModel::Status::UnsupportedCommand);
    }
    ctxt.SetCommandHandled();
}

void ColorControlClusterCommandHandler::InvokeCommand(CommandHandlerInterface::HandlerContext & ctxt)
{
    using namespace chip::app::Clusters::ColorControl;

    auto unify_node = m_node_state_monitor.bridged_endpoint(ctxt.mRequestPath.mEndpointId);
    if (!unify_node)
    {
        sl_log_info(LOG_TAG, "The endpoint [%i] is not a part of unify matter bridge node", ctxt.mRequestPath.mEndpointId);
        return;
    }

    std::string cmd;
    nlohmann::json payload = {};

    switch (ctxt.mRequestPath.mCommandId)
    {
    case Commands::MoveToHue::Id: {
        cmd = "MoveToHue";
        Commands::MoveToHue::DecodableType data;
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR)
        {
            try
            {
                payload["Hue"] = to_json(data.hue);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try
            {
                payload["Direction"] = to_json(data.direction);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try
            {
                payload["TransitionTime"] = to_json(data.transitionTime);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try
            {
                payload["OptionsMask"] = to_json(data.optionsMask);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try
            {
                payload["OptionsOverride"] = to_json(data.optionsOverride);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
        }
    }
    break;
    case Commands::MoveHue::Id: {
        cmd = "MoveHue";
        Commands::MoveHue::DecodableType data;
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR)
        {
            try
            {
                payload["MoveMode"] = to_json(data.moveMode);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try
            {
                payload["Rate"] = to_json(data.rate);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try
            {
                payload["OptionsMask"] = to_json(data.optionsMask);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try
            {
                payload["OptionsOverride"] = to_json(data.optionsOverride);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
        }
    }
    break;
    case Commands::StepHue::Id: {
        cmd = "StepHue";
        Commands::StepHue::DecodableType data;
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR)
        {
            try
            {
                payload["StepMode"] = to_json(data.stepMode);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try
            {
                payload["StepSize"] = to_json(data.stepSize);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try
            {
                payload["TransitionTime"] = to_json(data.transitionTime);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try
            {
                payload["OptionsMask"] = to_json(data.optionsMask);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try
            {
                payload["OptionsOverride"] = to_json(data.optionsOverride);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
        }
    }
    break;
    case Commands::MoveToSaturation::Id: {
        cmd = "MoveToSaturation";
        Commands::MoveToSaturation::DecodableType data;
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR)
        {
            try
            {
                payload["Saturation"] = to_json(data.saturation);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try
            {
                payload["TransitionTime"] = to_json(data.transitionTime);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try
            {
                payload["OptionsMask"] = to_json(data.optionsMask);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try
            {
                payload["OptionsOverride"] = to_json(data.optionsOverride);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
        }
    }
    break;
    case Commands::MoveSaturation::Id: {
        cmd = "MoveSaturation";
        Commands::MoveSaturation::DecodableType data;
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR)
        {
            try
            {
                payload["MoveMode"] = to_json(data.moveMode);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try
            {
                payload["Rate"] = to_json(data.rate);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try
            {
                payload["OptionsMask"] = to_json(data.optionsMask);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try
            {
                payload["OptionsOverride"] = to_json(data.optionsOverride);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
        }
    }
    break;
    case Commands::StepSaturation::Id: {
        cmd = "StepSaturation";
        Commands::StepSaturation::DecodableType data;
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR)
        {
            try
            {
                payload["StepMode"] = to_json(data.stepMode);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try
            {
                payload["StepSize"] = to_json(data.stepSize);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try
            {
                payload["TransitionTime"] = to_json(data.transitionTime);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try
            {
                payload["OptionsMask"] = to_json(data.optionsMask);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try
            {
                payload["OptionsOverride"] = to_json(data.optionsOverride);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
        }
    }
    break;
    case Commands::MoveToHueAndSaturation::Id: {
        cmd = "MoveToHueAndSaturation";
        Commands::MoveToHueAndSaturation::DecodableType data;
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR)
        {
            try
            {
                payload["Hue"] = to_json(data.hue);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try
            {
                payload["Saturation"] = to_json(data.saturation);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try
            {
                payload["TransitionTime"] = to_json(data.transitionTime);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try
            {
                payload["OptionsMask"] = to_json(data.optionsMask);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try
            {
                payload["OptionsOverride"] = to_json(data.optionsOverride);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
        }
    }
    break;
    case Commands::MoveToColor::Id: {
        cmd = "MoveToColor";
        Commands::MoveToColor::DecodableType data;
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR)
        {
            try
            {
                payload["ColorX"] = to_json(data.colorX);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try
            {
                payload["ColorY"] = to_json(data.colorY);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try
            {
                payload["TransitionTime"] = to_json(data.transitionTime);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try
            {
                payload["OptionsMask"] = to_json(data.optionsMask);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try
            {
                payload["OptionsOverride"] = to_json(data.optionsOverride);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
        }
    }
    break;
    case Commands::MoveColor::Id: {
        cmd = "MoveColor";
        Commands::MoveColor::DecodableType data;
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR)
        {
            try
            {
                payload["RateX"] = to_json(data.rateX);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try
            {
                payload["RateY"] = to_json(data.rateY);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try
            {
                payload["OptionsMask"] = to_json(data.optionsMask);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try
            {
                payload["OptionsOverride"] = to_json(data.optionsOverride);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
        }
    }
    break;
    case Commands::StepColor::Id: {
        cmd = "StepColor";
        Commands::StepColor::DecodableType data;
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR)
        {
            try
            {
                payload["StepX"] = to_json(data.stepX);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try
            {
                payload["StepY"] = to_json(data.stepY);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try
            {
                payload["TransitionTime"] = to_json(data.transitionTime);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try
            {
                payload["OptionsMask"] = to_json(data.optionsMask);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try
            {
                payload["OptionsOverride"] = to_json(data.optionsOverride);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
        }
    }
    break;
    case Commands::MoveToColorTemperature::Id: {
        cmd = "MoveToColorTemperature";
        Commands::MoveToColorTemperature::DecodableType data;
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR)
        {
            try
            {
                payload["ColorTemperature"] = to_json(data.colorTemperature);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try
            {
                payload["TransitionTime"] = to_json(data.transitionTime);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try
            {
                payload["OptionsMask"] = to_json(data.optionsMask);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try
            {
                payload["OptionsOverride"] = to_json(data.optionsOverride);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
        }
    }
    break;
    case Commands::EnhancedMoveToHue::Id: {
        cmd = "EnhancedMoveToHue";
        Commands::EnhancedMoveToHue::DecodableType data;
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR)
        {
            try
            {
                payload["EnhancedHue"] = to_json(data.enhancedHue);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try
            {
                payload["Direction"] = to_json(data.direction);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try
            {
                payload["TransitionTime"] = to_json(data.transitionTime);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try
            {
                payload["OptionsMask"] = to_json(data.optionsMask);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try
            {
                payload["OptionsOverride"] = to_json(data.optionsOverride);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
        }
    }
    break;
    case Commands::EnhancedMoveHue::Id: {
        cmd = "EnhancedMoveHue";
        Commands::EnhancedMoveHue::DecodableType data;
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR)
        {
            try
            {
                payload["MoveMode"] = to_json(data.moveMode);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try
            {
                payload["Rate"] = to_json(data.rate);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try
            {
                payload["OptionsMask"] = to_json(data.optionsMask);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try
            {
                payload["OptionsOverride"] = to_json(data.optionsOverride);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
        }
    }
    break;
    case Commands::EnhancedStepHue::Id: {
        cmd = "EnhancedStepHue";
        Commands::EnhancedStepHue::DecodableType data;
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR)
        {
            try
            {
                payload["StepMode"] = to_json(data.stepMode);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try
            {
                payload["StepSize"] = to_json(data.stepSize);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try
            {
                payload["TransitionTime"] = to_json(data.transitionTime);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try
            {
                payload["OptionsMask"] = to_json(data.optionsMask);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try
            {
                payload["OptionsOverride"] = to_json(data.optionsOverride);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
        }
    }
    break;
    case Commands::EnhancedMoveToHueAndSaturation::Id: {
        cmd = "EnhancedMoveToHueAndSaturation";
        Commands::EnhancedMoveToHueAndSaturation::DecodableType data;
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR)
        {
            try
            {
                payload["EnhancedHue"] = to_json(data.enhancedHue);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try
            {
                payload["Saturation"] = to_json(data.saturation);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try
            {
                payload["TransitionTime"] = to_json(data.transitionTime);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try
            {
                payload["OptionsMask"] = to_json(data.optionsMask);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try
            {
                payload["OptionsOverride"] = to_json(data.optionsOverride);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
        }
    }
    break;
    case Commands::ColorLoopSet::Id: {
        cmd = "ColorLoopSet";
        Commands::ColorLoopSet::DecodableType data;
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR)
        {
            try
            {
                payload["UpdateFlags"] = to_json(data.updateFlags);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try
            {
                payload["Action"] = to_json(data.action);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try
            {
                payload["Direction"] = to_json(data.direction);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try
            {
                payload["Time"] = to_json(data.time);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try
            {
                payload["StartHue"] = to_json(data.startHue);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try
            {
                payload["OptionsMask"] = to_json(data.optionsMask);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try
            {
                payload["OptionsOverride"] = to_json(data.optionsOverride);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
        }
    }
    break;
    case Commands::StopMoveStep::Id: {
        cmd = "StopMoveStep";
        Commands::StopMoveStep::DecodableType data;
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR)
        {
            try
            {
                payload["OptionsMask"] = to_json(data.optionsMask);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try
            {
                payload["OptionsOverride"] = to_json(data.optionsOverride);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
        }
    }
    break;
    case Commands::MoveColorTemperature::Id: {
        cmd = "MoveColorTemperature";
        Commands::MoveColorTemperature::DecodableType data;
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR)
        {
            try
            {
                payload["MoveMode"] = to_json(data.moveMode);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try
            {
                payload["Rate"] = to_json(data.rate);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try
            {
                payload["ColorTemperatureMinimumMireds"] = to_json(data.colorTemperatureMinimumMireds);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try
            {
                payload["ColorTemperatureMaximumMireds"] = to_json(data.colorTemperatureMaximumMireds);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try
            {
                payload["OptionsMask"] = to_json(data.optionsMask);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try
            {
                payload["OptionsOverride"] = to_json(data.optionsOverride);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
        }
    }
    break;
    case Commands::StepColorTemperature::Id: {
        cmd = "StepColorTemperature";
        Commands::StepColorTemperature::DecodableType data;
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR)
        {
            try
            {
                payload["StepMode"] = to_json(data.stepMode);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try
            {
                payload["StepSize"] = to_json(data.stepSize);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try
            {
                payload["TransitionTime"] = to_json(data.transitionTime);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try
            {
                payload["ColorTemperatureMinimumMireds"] = to_json(data.colorTemperatureMinimumMireds);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try
            {
                payload["ColorTemperatureMaximumMireds"] = to_json(data.colorTemperatureMaximumMireds);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try
            {
                payload["OptionsMask"] = to_json(data.optionsMask);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try
            {
                payload["OptionsOverride"] = to_json(data.optionsOverride);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
        }
    }
    break;
    }

    if (!cmd.empty())
    {
        ctxt.mCommandHandler.AddStatus(ctxt.mRequestPath, Protocols::InteractionModel::Status::Success);
        send_unify_mqtt_cmd(ctxt, cmd, payload);
        sl_log_debug(LOG_TAG, "Mapped [%] command to unify dotdot data model", cmd.c_str());
    }
    else
    {
        ctxt.mCommandHandler.AddStatus(ctxt.mRequestPath, Protocols::InteractionModel::Status::UnsupportedCommand);
    }
    ctxt.SetCommandHandled();
}

void BallastConfigurationClusterCommandHandler::InvokeCommand(CommandHandlerInterface::HandlerContext & ctxt)
{
    using namespace chip::app::Clusters::BallastConfiguration;

    auto unify_node = m_node_state_monitor.bridged_endpoint(ctxt.mRequestPath.mEndpointId);
    if (!unify_node)
    {
        sl_log_info(LOG_TAG, "The endpoint [%i] is not a part of unify matter bridge node", ctxt.mRequestPath.mEndpointId);
        return;
    }

    std::string cmd;
    nlohmann::json payload = {};

    switch (ctxt.mRequestPath.mCommandId)
    {
    }

    if (!cmd.empty())
    {
        ctxt.mCommandHandler.AddStatus(ctxt.mRequestPath, Protocols::InteractionModel::Status::Success);
        send_unify_mqtt_cmd(ctxt, cmd, payload);
        sl_log_debug(LOG_TAG, "Mapped [%] command to unify dotdot data model", cmd.c_str());
    }
    else
    {
        ctxt.mCommandHandler.AddStatus(ctxt.mRequestPath, Protocols::InteractionModel::Status::UnsupportedCommand);
    }
    ctxt.SetCommandHandled();
}

void IlluminanceMeasurementClusterCommandHandler::InvokeCommand(CommandHandlerInterface::HandlerContext & ctxt)
{
    using namespace chip::app::Clusters::IlluminanceMeasurement;

    auto unify_node = m_node_state_monitor.bridged_endpoint(ctxt.mRequestPath.mEndpointId);
    if (!unify_node)
    {
        sl_log_info(LOG_TAG, "The endpoint [%i] is not a part of unify matter bridge node", ctxt.mRequestPath.mEndpointId);
        return;
    }

    std::string cmd;
    nlohmann::json payload = {};

    switch (ctxt.mRequestPath.mCommandId)
    {
    }

    if (!cmd.empty())
    {
        ctxt.mCommandHandler.AddStatus(ctxt.mRequestPath, Protocols::InteractionModel::Status::Success);
        send_unify_mqtt_cmd(ctxt, cmd, payload);
        sl_log_debug(LOG_TAG, "Mapped [%] command to unify dotdot data model", cmd.c_str());
    }
    else
    {
        ctxt.mCommandHandler.AddStatus(ctxt.mRequestPath, Protocols::InteractionModel::Status::UnsupportedCommand);
    }
    ctxt.SetCommandHandled();
}

void TemperatureMeasurementClusterCommandHandler::InvokeCommand(CommandHandlerInterface::HandlerContext & ctxt)
{
    using namespace chip::app::Clusters::TemperatureMeasurement;

    auto unify_node = m_node_state_monitor.bridged_endpoint(ctxt.mRequestPath.mEndpointId);
    if (!unify_node)
    {
        sl_log_info(LOG_TAG, "The endpoint [%i] is not a part of unify matter bridge node", ctxt.mRequestPath.mEndpointId);
        return;
    }

    std::string cmd;
    nlohmann::json payload = {};

    switch (ctxt.mRequestPath.mCommandId)
    {
    }

    if (!cmd.empty())
    {
        ctxt.mCommandHandler.AddStatus(ctxt.mRequestPath, Protocols::InteractionModel::Status::Success);
        send_unify_mqtt_cmd(ctxt, cmd, payload);
        sl_log_debug(LOG_TAG, "Mapped [%] command to unify dotdot data model", cmd.c_str());
    }
    else
    {
        ctxt.mCommandHandler.AddStatus(ctxt.mRequestPath, Protocols::InteractionModel::Status::UnsupportedCommand);
    }
    ctxt.SetCommandHandled();
}

void PressureMeasurementClusterCommandHandler::InvokeCommand(CommandHandlerInterface::HandlerContext & ctxt)
{
    using namespace chip::app::Clusters::PressureMeasurement;

    auto unify_node = m_node_state_monitor.bridged_endpoint(ctxt.mRequestPath.mEndpointId);
    if (!unify_node)
    {
        sl_log_info(LOG_TAG, "The endpoint [%i] is not a part of unify matter bridge node", ctxt.mRequestPath.mEndpointId);
        return;
    }

    std::string cmd;
    nlohmann::json payload = {};

    switch (ctxt.mRequestPath.mCommandId)
    {
    }

    if (!cmd.empty())
    {
        ctxt.mCommandHandler.AddStatus(ctxt.mRequestPath, Protocols::InteractionModel::Status::Success);
        send_unify_mqtt_cmd(ctxt, cmd, payload);
        sl_log_debug(LOG_TAG, "Mapped [%] command to unify dotdot data model", cmd.c_str());
    }
    else
    {
        ctxt.mCommandHandler.AddStatus(ctxt.mRequestPath, Protocols::InteractionModel::Status::UnsupportedCommand);
    }
    ctxt.SetCommandHandled();
}

void FlowMeasurementClusterCommandHandler::InvokeCommand(CommandHandlerInterface::HandlerContext & ctxt)
{
    using namespace chip::app::Clusters::FlowMeasurement;

    auto unify_node = m_node_state_monitor.bridged_endpoint(ctxt.mRequestPath.mEndpointId);
    if (!unify_node)
    {
        sl_log_info(LOG_TAG, "The endpoint [%i] is not a part of unify matter bridge node", ctxt.mRequestPath.mEndpointId);
        return;
    }

    std::string cmd;
    nlohmann::json payload = {};

    switch (ctxt.mRequestPath.mCommandId)
    {
    }

    if (!cmd.empty())
    {
        ctxt.mCommandHandler.AddStatus(ctxt.mRequestPath, Protocols::InteractionModel::Status::Success);
        send_unify_mqtt_cmd(ctxt, cmd, payload);
        sl_log_debug(LOG_TAG, "Mapped [%] command to unify dotdot data model", cmd.c_str());
    }
    else
    {
        ctxt.mCommandHandler.AddStatus(ctxt.mRequestPath, Protocols::InteractionModel::Status::UnsupportedCommand);
    }
    ctxt.SetCommandHandled();
}

void RelativeHumidityMeasurementClusterCommandHandler::InvokeCommand(CommandHandlerInterface::HandlerContext & ctxt)
{
    using namespace chip::app::Clusters::RelativeHumidityMeasurement;

    auto unify_node = m_node_state_monitor.bridged_endpoint(ctxt.mRequestPath.mEndpointId);
    if (!unify_node)
    {
        sl_log_info(LOG_TAG, "The endpoint [%i] is not a part of unify matter bridge node", ctxt.mRequestPath.mEndpointId);
        return;
    }

    std::string cmd;
    nlohmann::json payload = {};

    switch (ctxt.mRequestPath.mCommandId)
    {
    }

    if (!cmd.empty())
    {
        ctxt.mCommandHandler.AddStatus(ctxt.mRequestPath, Protocols::InteractionModel::Status::Success);
        send_unify_mqtt_cmd(ctxt, cmd, payload);
        sl_log_debug(LOG_TAG, "Mapped [%] command to unify dotdot data model", cmd.c_str());
    }
    else
    {
        ctxt.mCommandHandler.AddStatus(ctxt.mRequestPath, Protocols::InteractionModel::Status::UnsupportedCommand);
    }
    ctxt.SetCommandHandled();
}

void OccupancySensingClusterCommandHandler::InvokeCommand(CommandHandlerInterface::HandlerContext & ctxt)
{
    using namespace chip::app::Clusters::OccupancySensing;

    auto unify_node = m_node_state_monitor.bridged_endpoint(ctxt.mRequestPath.mEndpointId);
    if (!unify_node)
    {
        sl_log_info(LOG_TAG, "The endpoint [%i] is not a part of unify matter bridge node", ctxt.mRequestPath.mEndpointId);
        return;
    }

    std::string cmd;
    nlohmann::json payload = {};

    switch (ctxt.mRequestPath.mCommandId)
    {
    }

    if (!cmd.empty())
    {
        ctxt.mCommandHandler.AddStatus(ctxt.mRequestPath, Protocols::InteractionModel::Status::Success);
        send_unify_mqtt_cmd(ctxt, cmd, payload);
        sl_log_debug(LOG_TAG, "Mapped [%] command to unify dotdot data model", cmd.c_str());
    }
    else
    {
        ctxt.mCommandHandler.AddStatus(ctxt.mRequestPath, Protocols::InteractionModel::Status::UnsupportedCommand);
    }
    ctxt.SetCommandHandled();
}

void WakeOnLanClusterCommandHandler::InvokeCommand(CommandHandlerInterface::HandlerContext & ctxt)
{
    using namespace chip::app::Clusters::WakeOnLan;

    auto unify_node = m_node_state_monitor.bridged_endpoint(ctxt.mRequestPath.mEndpointId);
    if (!unify_node)
    {
        sl_log_info(LOG_TAG, "The endpoint [%i] is not a part of unify matter bridge node", ctxt.mRequestPath.mEndpointId);
        return;
    }

    std::string cmd;
    nlohmann::json payload = {};

    switch (ctxt.mRequestPath.mCommandId)
    {
    }

    if (!cmd.empty())
    {
        ctxt.mCommandHandler.AddStatus(ctxt.mRequestPath, Protocols::InteractionModel::Status::Success);
        send_unify_mqtt_cmd(ctxt, cmd, payload);
        sl_log_debug(LOG_TAG, "Mapped [%] command to unify dotdot data model", cmd.c_str());
    }
    else
    {
        ctxt.mCommandHandler.AddStatus(ctxt.mRequestPath, Protocols::InteractionModel::Status::UnsupportedCommand);
    }
    ctxt.SetCommandHandled();
}

void ChannelClusterCommandHandler::InvokeCommand(CommandHandlerInterface::HandlerContext & ctxt)
{
    using namespace chip::app::Clusters::Channel;

    auto unify_node = m_node_state_monitor.bridged_endpoint(ctxt.mRequestPath.mEndpointId);
    if (!unify_node)
    {
        sl_log_info(LOG_TAG, "The endpoint [%i] is not a part of unify matter bridge node", ctxt.mRequestPath.mEndpointId);
        return;
    }

    std::string cmd;
    nlohmann::json payload = {};

    switch (ctxt.mRequestPath.mCommandId)
    {
    case Commands::ChangeChannel::Id: {
        cmd = "ChangeChannel";
        Commands::ChangeChannel::DecodableType data;
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR)
        {
            try
            {
                payload["Match"] = to_json(data.match);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
        }
    }
    break;
    case Commands::ChangeChannelByNumber::Id: {
        cmd = "ChangeChannelByNumber";
        Commands::ChangeChannelByNumber::DecodableType data;
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR)
        {
            try
            {
                payload["MajorNumber"] = to_json(data.majorNumber);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try
            {
                payload["MinorNumber"] = to_json(data.minorNumber);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
        }
    }
    break;
    case Commands::SkipChannel::Id: {
        cmd = "SkipChannel";
        Commands::SkipChannel::DecodableType data;
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR)
        {
            try
            {
                payload["Count"] = to_json(data.count);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
        }
    }
    break;
    }

    if (!cmd.empty())
    {
        ctxt.mCommandHandler.AddStatus(ctxt.mRequestPath, Protocols::InteractionModel::Status::Success);
        send_unify_mqtt_cmd(ctxt, cmd, payload);
        sl_log_debug(LOG_TAG, "Mapped [%] command to unify dotdot data model", cmd.c_str());
    }
    else
    {
        ctxt.mCommandHandler.AddStatus(ctxt.mRequestPath, Protocols::InteractionModel::Status::UnsupportedCommand);
    }
    ctxt.SetCommandHandled();
}

void TargetNavigatorClusterCommandHandler::InvokeCommand(CommandHandlerInterface::HandlerContext & ctxt)
{
    using namespace chip::app::Clusters::TargetNavigator;

    auto unify_node = m_node_state_monitor.bridged_endpoint(ctxt.mRequestPath.mEndpointId);
    if (!unify_node)
    {
        sl_log_info(LOG_TAG, "The endpoint [%i] is not a part of unify matter bridge node", ctxt.mRequestPath.mEndpointId);
        return;
    }

    std::string cmd;
    nlohmann::json payload = {};

    switch (ctxt.mRequestPath.mCommandId)
    {
    case Commands::NavigateTarget::Id: {
        cmd = "NavigateTarget";
        Commands::NavigateTarget::DecodableType data;
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR)
        {
            try
            {
                payload["Target"] = to_json(data.target);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            if (data.data.HasValue())
            {
                try
                {
                    payload["Data"] = to_json(data.data.Value());
                } catch (std::exception & ex)
                {
                    sl_log_warning(LOG_TAG, "Failed to add the command arguments value to json format: %s", ex.what());
                }
            }
        }
    }
    break;
    }

    if (!cmd.empty())
    {
        ctxt.mCommandHandler.AddStatus(ctxt.mRequestPath, Protocols::InteractionModel::Status::Success);
        send_unify_mqtt_cmd(ctxt, cmd, payload);
        sl_log_debug(LOG_TAG, "Mapped [%] command to unify dotdot data model", cmd.c_str());
    }
    else
    {
        ctxt.mCommandHandler.AddStatus(ctxt.mRequestPath, Protocols::InteractionModel::Status::UnsupportedCommand);
    }
    ctxt.SetCommandHandled();
}

void MediaPlaybackClusterCommandHandler::InvokeCommand(CommandHandlerInterface::HandlerContext & ctxt)
{
    using namespace chip::app::Clusters::MediaPlayback;

    auto unify_node = m_node_state_monitor.bridged_endpoint(ctxt.mRequestPath.mEndpointId);
    if (!unify_node)
    {
        sl_log_info(LOG_TAG, "The endpoint [%i] is not a part of unify matter bridge node", ctxt.mRequestPath.mEndpointId);
        return;
    }

    std::string cmd;
    nlohmann::json payload = {};

    switch (ctxt.mRequestPath.mCommandId)
    {
    case Commands::Play::Id: {
        cmd = "Play";
        Commands::Play::DecodableType data;
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR)
        {
        }
    }
    break;
    case Commands::Pause::Id: {
        cmd = "Pause";
        Commands::Pause::DecodableType data;
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR)
        {
        }
    }
    break;
    case Commands::StopPlayback::Id: {
        cmd = "StopPlayback";
        Commands::StopPlayback::DecodableType data;
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR)
        {
        }
    }
    break;
    case Commands::StartOver::Id: {
        cmd = "StartOver";
        Commands::StartOver::DecodableType data;
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR)
        {
        }
    }
    break;
    case Commands::Previous::Id: {
        cmd = "Previous";
        Commands::Previous::DecodableType data;
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR)
        {
        }
    }
    break;
    case Commands::Next::Id: {
        cmd = "Next";
        Commands::Next::DecodableType data;
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR)
        {
        }
    }
    break;
    case Commands::Rewind::Id: {
        cmd = "Rewind";
        Commands::Rewind::DecodableType data;
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR)
        {
        }
    }
    break;
    case Commands::FastForward::Id: {
        cmd = "FastForward";
        Commands::FastForward::DecodableType data;
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR)
        {
        }
    }
    break;
    case Commands::SkipForward::Id: {
        cmd = "SkipForward";
        Commands::SkipForward::DecodableType data;
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR)
        {
            try
            {
                payload["DeltaPositionMilliseconds"] = to_json(data.deltaPositionMilliseconds);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
        }
    }
    break;
    case Commands::SkipBackward::Id: {
        cmd = "SkipBackward";
        Commands::SkipBackward::DecodableType data;
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR)
        {
            try
            {
                payload["DeltaPositionMilliseconds"] = to_json(data.deltaPositionMilliseconds);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
        }
    }
    break;
    case Commands::Seek::Id: {
        cmd = "Seek";
        Commands::Seek::DecodableType data;
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR)
        {
            try
            {
                payload["Position"] = to_json(data.position);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
        }
    }
    break;
    }

    if (!cmd.empty())
    {
        ctxt.mCommandHandler.AddStatus(ctxt.mRequestPath, Protocols::InteractionModel::Status::Success);
        send_unify_mqtt_cmd(ctxt, cmd, payload);
        sl_log_debug(LOG_TAG, "Mapped [%] command to unify dotdot data model", cmd.c_str());
    }
    else
    {
        ctxt.mCommandHandler.AddStatus(ctxt.mRequestPath, Protocols::InteractionModel::Status::UnsupportedCommand);
    }
    ctxt.SetCommandHandled();
}

void MediaInputClusterCommandHandler::InvokeCommand(CommandHandlerInterface::HandlerContext & ctxt)
{
    using namespace chip::app::Clusters::MediaInput;

    auto unify_node = m_node_state_monitor.bridged_endpoint(ctxt.mRequestPath.mEndpointId);
    if (!unify_node)
    {
        sl_log_info(LOG_TAG, "The endpoint [%i] is not a part of unify matter bridge node", ctxt.mRequestPath.mEndpointId);
        return;
    }

    std::string cmd;
    nlohmann::json payload = {};

    switch (ctxt.mRequestPath.mCommandId)
    {
    case Commands::SelectInput::Id: {
        cmd = "SelectInput";
        Commands::SelectInput::DecodableType data;
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR)
        {
            try
            {
                payload["Index"] = to_json(data.index);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
        }
    }
    break;
    case Commands::ShowInputStatus::Id: {
        cmd = "ShowInputStatus";
        Commands::ShowInputStatus::DecodableType data;
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR)
        {
        }
    }
    break;
    case Commands::HideInputStatus::Id: {
        cmd = "HideInputStatus";
        Commands::HideInputStatus::DecodableType data;
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR)
        {
        }
    }
    break;
    case Commands::RenameInput::Id: {
        cmd = "RenameInput";
        Commands::RenameInput::DecodableType data;
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR)
        {
            try
            {
                payload["Index"] = to_json(data.index);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try
            {
                payload["Name"] = to_json(data.name);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
        }
    }
    break;
    }

    if (!cmd.empty())
    {
        ctxt.mCommandHandler.AddStatus(ctxt.mRequestPath, Protocols::InteractionModel::Status::Success);
        send_unify_mqtt_cmd(ctxt, cmd, payload);
        sl_log_debug(LOG_TAG, "Mapped [%] command to unify dotdot data model", cmd.c_str());
    }
    else
    {
        ctxt.mCommandHandler.AddStatus(ctxt.mRequestPath, Protocols::InteractionModel::Status::UnsupportedCommand);
    }
    ctxt.SetCommandHandled();
}

void LowPowerClusterCommandHandler::InvokeCommand(CommandHandlerInterface::HandlerContext & ctxt)
{
    using namespace chip::app::Clusters::LowPower;

    auto unify_node = m_node_state_monitor.bridged_endpoint(ctxt.mRequestPath.mEndpointId);
    if (!unify_node)
    {
        sl_log_info(LOG_TAG, "The endpoint [%i] is not a part of unify matter bridge node", ctxt.mRequestPath.mEndpointId);
        return;
    }

    std::string cmd;
    nlohmann::json payload = {};

    switch (ctxt.mRequestPath.mCommandId)
    {
    case Commands::Sleep::Id: {
        cmd = "Sleep";
        Commands::Sleep::DecodableType data;
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR)
        {
        }
    }
    break;
    }

    if (!cmd.empty())
    {
        ctxt.mCommandHandler.AddStatus(ctxt.mRequestPath, Protocols::InteractionModel::Status::Success);
        send_unify_mqtt_cmd(ctxt, cmd, payload);
        sl_log_debug(LOG_TAG, "Mapped [%] command to unify dotdot data model", cmd.c_str());
    }
    else
    {
        ctxt.mCommandHandler.AddStatus(ctxt.mRequestPath, Protocols::InteractionModel::Status::UnsupportedCommand);
    }
    ctxt.SetCommandHandled();
}

void KeypadInputClusterCommandHandler::InvokeCommand(CommandHandlerInterface::HandlerContext & ctxt)
{
    using namespace chip::app::Clusters::KeypadInput;

    auto unify_node = m_node_state_monitor.bridged_endpoint(ctxt.mRequestPath.mEndpointId);
    if (!unify_node)
    {
        sl_log_info(LOG_TAG, "The endpoint [%i] is not a part of unify matter bridge node", ctxt.mRequestPath.mEndpointId);
        return;
    }

    std::string cmd;
    nlohmann::json payload = {};

    switch (ctxt.mRequestPath.mCommandId)
    {
    case Commands::SendKey::Id: {
        cmd = "SendKey";
        Commands::SendKey::DecodableType data;
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR)
        {
            try
            {
                payload["KeyCode"] = to_json(data.keyCode);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
        }
    }
    break;
    }

    if (!cmd.empty())
    {
        ctxt.mCommandHandler.AddStatus(ctxt.mRequestPath, Protocols::InteractionModel::Status::Success);
        send_unify_mqtt_cmd(ctxt, cmd, payload);
        sl_log_debug(LOG_TAG, "Mapped [%] command to unify dotdot data model", cmd.c_str());
    }
    else
    {
        ctxt.mCommandHandler.AddStatus(ctxt.mRequestPath, Protocols::InteractionModel::Status::UnsupportedCommand);
    }
    ctxt.SetCommandHandled();
}

void ContentLauncherClusterCommandHandler::InvokeCommand(CommandHandlerInterface::HandlerContext & ctxt)
{
    using namespace chip::app::Clusters::ContentLauncher;

    auto unify_node = m_node_state_monitor.bridged_endpoint(ctxt.mRequestPath.mEndpointId);
    if (!unify_node)
    {
        sl_log_info(LOG_TAG, "The endpoint [%i] is not a part of unify matter bridge node", ctxt.mRequestPath.mEndpointId);
        return;
    }

    std::string cmd;
    nlohmann::json payload = {};

    switch (ctxt.mRequestPath.mCommandId)
    {
    case Commands::LaunchContent::Id: {
        cmd = "LaunchContent";
        Commands::LaunchContent::DecodableType data;
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR)
        {
            try
            {
                payload["Search"] = to_json(data.search);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try
            {
                payload["AutoPlay"] = to_json(data.autoPlay);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            if (data.data.HasValue())
            {
                try
                {
                    payload["Data"] = to_json(data.data.Value());
                } catch (std::exception & ex)
                {
                    sl_log_warning(LOG_TAG, "Failed to add the command arguments value to json format: %s", ex.what());
                }
            }
        }
    }
    break;
    case Commands::LaunchURL::Id: {
        cmd = "LaunchURL";
        Commands::LaunchURL::DecodableType data;
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR)
        {
            try
            {
                payload["ContentURL"] = to_json(data.contentURL);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            if (data.displayString.HasValue())
            {
                try
                {
                    payload["DisplayString"] = to_json(data.displayString.Value());
                } catch (std::exception & ex)
                {
                    sl_log_warning(LOG_TAG, "Failed to add the command arguments value to json format: %s", ex.what());
                }
            }
            if (data.brandingInformation.HasValue())
            {
                try
                {
                    payload["BrandingInformation"] = to_json(data.brandingInformation.Value());
                } catch (std::exception & ex)
                {
                    sl_log_warning(LOG_TAG, "Failed to add the command arguments value to json format: %s", ex.what());
                }
            }
        }
    }
    break;
    }

    if (!cmd.empty())
    {
        ctxt.mCommandHandler.AddStatus(ctxt.mRequestPath, Protocols::InteractionModel::Status::Success);
        send_unify_mqtt_cmd(ctxt, cmd, payload);
        sl_log_debug(LOG_TAG, "Mapped [%] command to unify dotdot data model", cmd.c_str());
    }
    else
    {
        ctxt.mCommandHandler.AddStatus(ctxt.mRequestPath, Protocols::InteractionModel::Status::UnsupportedCommand);
    }
    ctxt.SetCommandHandled();
}

void AudioOutputClusterCommandHandler::InvokeCommand(CommandHandlerInterface::HandlerContext & ctxt)
{
    using namespace chip::app::Clusters::AudioOutput;

    auto unify_node = m_node_state_monitor.bridged_endpoint(ctxt.mRequestPath.mEndpointId);
    if (!unify_node)
    {
        sl_log_info(LOG_TAG, "The endpoint [%i] is not a part of unify matter bridge node", ctxt.mRequestPath.mEndpointId);
        return;
    }

    std::string cmd;
    nlohmann::json payload = {};

    switch (ctxt.mRequestPath.mCommandId)
    {
    case Commands::SelectOutput::Id: {
        cmd = "SelectOutput";
        Commands::SelectOutput::DecodableType data;
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR)
        {
            try
            {
                payload["Index"] = to_json(data.index);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
        }
    }
    break;
    case Commands::RenameOutput::Id: {
        cmd = "RenameOutput";
        Commands::RenameOutput::DecodableType data;
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR)
        {
            try
            {
                payload["Index"] = to_json(data.index);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try
            {
                payload["Name"] = to_json(data.name);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
        }
    }
    break;
    }

    if (!cmd.empty())
    {
        ctxt.mCommandHandler.AddStatus(ctxt.mRequestPath, Protocols::InteractionModel::Status::Success);
        send_unify_mqtt_cmd(ctxt, cmd, payload);
        sl_log_debug(LOG_TAG, "Mapped [%] command to unify dotdot data model", cmd.c_str());
    }
    else
    {
        ctxt.mCommandHandler.AddStatus(ctxt.mRequestPath, Protocols::InteractionModel::Status::UnsupportedCommand);
    }
    ctxt.SetCommandHandled();
}

void ApplicationLauncherClusterCommandHandler::InvokeCommand(CommandHandlerInterface::HandlerContext & ctxt)
{
    using namespace chip::app::Clusters::ApplicationLauncher;

    auto unify_node = m_node_state_monitor.bridged_endpoint(ctxt.mRequestPath.mEndpointId);
    if (!unify_node)
    {
        sl_log_info(LOG_TAG, "The endpoint [%i] is not a part of unify matter bridge node", ctxt.mRequestPath.mEndpointId);
        return;
    }

    std::string cmd;
    nlohmann::json payload = {};

    switch (ctxt.mRequestPath.mCommandId)
    {
    case Commands::LaunchApp::Id: {
        cmd = "LaunchApp";
        Commands::LaunchApp::DecodableType data;
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR)
        {
            try
            {
                payload["Application"] = to_json(data.application);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            if (data.data.HasValue())
            {
                try
                {
                    payload["Data"] = to_json(data.data.Value());
                } catch (std::exception & ex)
                {
                    sl_log_warning(LOG_TAG, "Failed to add the command arguments value to json format: %s", ex.what());
                }
            }
        }
    }
    break;
    case Commands::StopApp::Id: {
        cmd = "StopApp";
        Commands::StopApp::DecodableType data;
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR)
        {
            try
            {
                payload["Application"] = to_json(data.application);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
        }
    }
    break;
    case Commands::HideApp::Id: {
        cmd = "HideApp";
        Commands::HideApp::DecodableType data;
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR)
        {
            try
            {
                payload["Application"] = to_json(data.application);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
        }
    }
    break;
    }

    if (!cmd.empty())
    {
        ctxt.mCommandHandler.AddStatus(ctxt.mRequestPath, Protocols::InteractionModel::Status::Success);
        send_unify_mqtt_cmd(ctxt, cmd, payload);
        sl_log_debug(LOG_TAG, "Mapped [%] command to unify dotdot data model", cmd.c_str());
    }
    else
    {
        ctxt.mCommandHandler.AddStatus(ctxt.mRequestPath, Protocols::InteractionModel::Status::UnsupportedCommand);
    }
    ctxt.SetCommandHandled();
}

void ApplicationBasicClusterCommandHandler::InvokeCommand(CommandHandlerInterface::HandlerContext & ctxt)
{
    using namespace chip::app::Clusters::ApplicationBasic;

    auto unify_node = m_node_state_monitor.bridged_endpoint(ctxt.mRequestPath.mEndpointId);
    if (!unify_node)
    {
        sl_log_info(LOG_TAG, "The endpoint [%i] is not a part of unify matter bridge node", ctxt.mRequestPath.mEndpointId);
        return;
    }

    std::string cmd;
    nlohmann::json payload = {};

    switch (ctxt.mRequestPath.mCommandId)
    {
    }

    if (!cmd.empty())
    {
        ctxt.mCommandHandler.AddStatus(ctxt.mRequestPath, Protocols::InteractionModel::Status::Success);
        send_unify_mqtt_cmd(ctxt, cmd, payload);
        sl_log_debug(LOG_TAG, "Mapped [%] command to unify dotdot data model", cmd.c_str());
    }
    else
    {
        ctxt.mCommandHandler.AddStatus(ctxt.mRequestPath, Protocols::InteractionModel::Status::UnsupportedCommand);
    }
    ctxt.SetCommandHandled();
}

void AccountLoginClusterCommandHandler::InvokeCommand(CommandHandlerInterface::HandlerContext & ctxt)
{
    using namespace chip::app::Clusters::AccountLogin;

    auto unify_node = m_node_state_monitor.bridged_endpoint(ctxt.mRequestPath.mEndpointId);
    if (!unify_node)
    {
        sl_log_info(LOG_TAG, "The endpoint [%i] is not a part of unify matter bridge node", ctxt.mRequestPath.mEndpointId);
        return;
    }

    std::string cmd;
    nlohmann::json payload = {};

    switch (ctxt.mRequestPath.mCommandId)
    {
    case Commands::GetSetupPIN::Id: {
        cmd = "GetSetupPIN";
        Commands::GetSetupPIN::DecodableType data;
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR)
        {
            try
            {
                payload["TempAccountIdentifier"] = to_json(data.tempAccountIdentifier);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
        }
    }
    break;
    case Commands::Login::Id: {
        cmd = "Login";
        Commands::Login::DecodableType data;
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR)
        {
            try
            {
                payload["TempAccountIdentifier"] = to_json(data.tempAccountIdentifier);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try
            {
                payload["SetupPIN"] = to_json(data.setupPIN);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
        }
    }
    break;
    case Commands::Logout::Id: {
        cmd = "Logout";
        Commands::Logout::DecodableType data;
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR)
        {
        }
    }
    break;
    }

    if (!cmd.empty())
    {
        ctxt.mCommandHandler.AddStatus(ctxt.mRequestPath, Protocols::InteractionModel::Status::Success);
        send_unify_mqtt_cmd(ctxt, cmd, payload);
        sl_log_debug(LOG_TAG, "Mapped [%] command to unify dotdot data model", cmd.c_str());
    }
    else
    {
        ctxt.mCommandHandler.AddStatus(ctxt.mRequestPath, Protocols::InteractionModel::Status::UnsupportedCommand);
    }
    ctxt.SetCommandHandled();
}

void ElectricalMeasurementClusterCommandHandler::InvokeCommand(CommandHandlerInterface::HandlerContext & ctxt)
{
    using namespace chip::app::Clusters::ElectricalMeasurement;

    auto unify_node = m_node_state_monitor.bridged_endpoint(ctxt.mRequestPath.mEndpointId);
    if (!unify_node)
    {
        sl_log_info(LOG_TAG, "The endpoint [%i] is not a part of unify matter bridge node", ctxt.mRequestPath.mEndpointId);
        return;
    }

    std::string cmd;
    nlohmann::json payload = {};

    switch (ctxt.mRequestPath.mCommandId)
    {
    case Commands::GetProfileInfoCommand::Id: {
        cmd = "GetProfileInfoCommand";
        Commands::GetProfileInfoCommand::DecodableType data;
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR)
        {
        }
    }
    break;
    case Commands::GetMeasurementProfileCommand::Id: {
        cmd = "GetMeasurementProfileCommand";
        Commands::GetMeasurementProfileCommand::DecodableType data;
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR)
        {
            try
            {
                payload["AttributeId"] = to_json(data.attributeId);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try
            {
                payload["StartTime"] = to_json(data.startTime);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try
            {
                payload["NumberOfIntervals"] = to_json(data.numberOfIntervals);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
        }
    }
    break;
    }

    if (!cmd.empty())
    {
        ctxt.mCommandHandler.AddStatus(ctxt.mRequestPath, Protocols::InteractionModel::Status::Success);
        send_unify_mqtt_cmd(ctxt, cmd, payload);
        sl_log_debug(LOG_TAG, "Mapped [%] command to unify dotdot data model", cmd.c_str());
    }
    else
    {
        ctxt.mCommandHandler.AddStatus(ctxt.mRequestPath, Protocols::InteractionModel::Status::UnsupportedCommand);
    }
    ctxt.SetCommandHandled();
}

void FaultInjectionClusterCommandHandler::InvokeCommand(CommandHandlerInterface::HandlerContext & ctxt)
{
    using namespace chip::app::Clusters::FaultInjection;

    auto unify_node = m_node_state_monitor.bridged_endpoint(ctxt.mRequestPath.mEndpointId);
    if (!unify_node)
    {
        sl_log_info(LOG_TAG, "The endpoint [%i] is not a part of unify matter bridge node", ctxt.mRequestPath.mEndpointId);
        return;
    }

    std::string cmd;
    nlohmann::json payload = {};

    switch (ctxt.mRequestPath.mCommandId)
    {
    case Commands::FailAtFault::Id: {
        cmd = "FailAtFault";
        Commands::FailAtFault::DecodableType data;
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR)
        {
            try
            {
                payload["Type"] = to_json(data.type);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try
            {
                payload["Id"] = to_json(data.id);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try
            {
                payload["NumCallsToSkip"] = to_json(data.numCallsToSkip);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try
            {
                payload["NumCallsToFail"] = to_json(data.numCallsToFail);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try
            {
                payload["TakeMutex"] = to_json(data.takeMutex);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
        }
    }
    break;
    case Commands::FailRandomlyAtFault::Id: {
        cmd = "FailRandomlyAtFault";
        Commands::FailRandomlyAtFault::DecodableType data;
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR)
        {
            try
            {
                payload["Type"] = to_json(data.type);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try
            {
                payload["Id"] = to_json(data.id);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try
            {
                payload["Percentage"] = to_json(data.percentage);
            } catch (std::exception & ex)
            {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
        }
    }
    break;
    }

    if (!cmd.empty())
    {
        ctxt.mCommandHandler.AddStatus(ctxt.mRequestPath, Protocols::InteractionModel::Status::Success);
        send_unify_mqtt_cmd(ctxt, cmd, payload);
        sl_log_debug(LOG_TAG, "Mapped [%] command to unify dotdot data model", cmd.c_str());
    }
    else
    {
        ctxt.mCommandHandler.AddStatus(ctxt.mRequestPath, Protocols::InteractionModel::Status::UnsupportedCommand);
    }
    ctxt.SetCommandHandled();
}
