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

#include "attribute.hpp"
#include "dotdot_mqtt.h"
#include "mpc_attribute_store.h"
#include "mpc_sendable_command.hpp"
#include "sl_log.h"
#include "unify_dotdot_attribute_store.h"
#include <AppMain.h>
#include <iostream>
#include <sstream>
#include <string>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::OnOff;
using namespace chip::app::Clusters::LevelControl;
using namespace mpc;
using namespace attribute_store;

#define LOG_TAG "matter_cluster_command_server"

/**
 * @brief Checks if the cluster's command is supported by device with the given UNID under the given endpoint
 */
static sl_status_t check_cluster_support_for_node(const dotdot_unid_t unid, dotdot_endpoint_id_t endpoint, ClusterId clusID,
    CommandId cmdId)
{
    // get the attribute store node for the given endpoint ID
    attribute_store_node_t epNode = mpc_attribute_store_network_helper_get_endpoint_node(unid, endpoint);

    // get the cluster's AcceptedCommandList attribute entry from attribute store
    attribute_store_type_t type = ((clusID & 0xFFFF) << 16) | chip::app::Clusters::Globals::Attributes::AcceptedCommandList::Id;
    auto cmdList = attribute(epNode).child_by_type(type);
    if (!cmdList.is_valid())
        return SL_STATUS_FAIL;

    // check if the command ID is listed in the AcceptedCommandList entry
    if (cmdList.reported<std::string>().find(std::to_string(cmdId)) != std::string::npos)
        return SL_STATUS_OK;
    return SL_STATUS_FAIL;
}

// Callbacks used by the on_off cluster
static sl_status_t mpc_on_off_cluster_off_command(const dotdot_unid_t unid, dotdot_endpoint_id_t endpoint,
    uic_mqtt_dotdot_callback_call_type_t callback_type)
{
    chip::EndpointId endpointId;
    chip::NodeId nodeId;
    SendableCommand<chip::app::Clusters::OnOff::Commands::Off::Type> cmd;

    if (UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK == callback_type)
        return check_cluster_support_for_node(unid, endpoint, chip::app::Clusters::OnOff::Id,
            chip::app::Clusters::OnOff::Commands::Off::Id);

    if (mpc_attribute_store_get_endpoint_and_node_from_unid(unid, endpoint, nodeId, endpointId) == SL_STATUS_OK) {
        cmd.Send(chip::ScopedNodeId(nodeId, 1), endpointId);
        return SL_STATUS_OK;
    }
    return SL_STATUS_FAIL;
}

static sl_status_t mpc_on_off_cluster_on_command(const dotdot_unid_t unid, dotdot_endpoint_id_t endpoint,
    uic_mqtt_dotdot_callback_call_type_t callback_type)
{
    chip::EndpointId endpointId;
    chip::NodeId nodeId;
    SendableCommand<chip::app::Clusters::OnOff::Commands::On::Type> cmd;

    if (UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK == callback_type)
        return check_cluster_support_for_node(unid, endpoint, chip::app::Clusters::OnOff::Id,
            chip::app::Clusters::OnOff::Commands::On::Id);

    if (mpc_attribute_store_get_endpoint_and_node_from_unid(unid, endpoint, nodeId, endpointId) == SL_STATUS_OK) {
        cmd.Send(chip::ScopedNodeId(nodeId, 1), endpointId);
        return SL_STATUS_OK;
    }
    return SL_STATUS_FAIL;
}

static sl_status_t mpc_on_off_cluster_toggle_command(const dotdot_unid_t unid, dotdot_endpoint_id_t endpoint,
    uic_mqtt_dotdot_callback_call_type_t callback_type)
{
    chip::EndpointId endpointId;
    chip::NodeId nodeId;
    SendableCommand<chip::app::Clusters::OnOff::Commands::Toggle::Type> cmd;

    if (UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK == callback_type)
        return check_cluster_support_for_node(unid, endpoint, chip::app::Clusters::OnOff::Id,
            chip::app::Clusters::OnOff::Commands::Toggle::Id);

    if (mpc_attribute_store_get_endpoint_and_node_from_unid(unid, endpoint, nodeId, endpointId) == SL_STATUS_OK) {
        cmd.Send(chip::ScopedNodeId(nodeId, 1), endpointId);
        return SL_STATUS_OK;
    }
    return SL_STATUS_FAIL;
}

static sl_status_t mpc_on_off_cluster_off_with_effect_command(const dotdot_unid_t unid, dotdot_endpoint_id_t endpoint,
    uic_mqtt_dotdot_callback_call_type_t callback_type,
    OffWithEffectEffectIdentifier effect_identifier,

    uint8_t effect_variant)
{
    chip::EndpointId endpointId;
    chip::NodeId nodeId;
    SendableCommand<chip::app::Clusters::OnOff::Commands::OffWithEffect::Type> cmd;
    cmd.Data().effectIdentifier = static_cast<EffectIdentifierEnum>(effect_identifier);
    cmd.Data().effectVariant = static_cast<uint8_t>(effect_variant);

    if (UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK == callback_type)
        return check_cluster_support_for_node(unid, endpoint, chip::app::Clusters::OnOff::Id,
            chip::app::Clusters::OnOff::Commands::OffWithEffect::Id);

    if (mpc_attribute_store_get_endpoint_and_node_from_unid(unid, endpoint, nodeId, endpointId) == SL_STATUS_OK) {
        cmd.Send(chip::ScopedNodeId(nodeId, 1), endpointId);
        return SL_STATUS_OK;
    }
    return SL_STATUS_FAIL;
}

static sl_status_t mpc_on_off_cluster_on_with_recall_global_scene_command(const dotdot_unid_t unid, dotdot_endpoint_id_t endpoint,
    uic_mqtt_dotdot_callback_call_type_t callback_type)
{
    chip::EndpointId endpointId;
    chip::NodeId nodeId;
    SendableCommand<chip::app::Clusters::OnOff::Commands::OnWithRecallGlobalScene::Type> cmd;

    if (UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK == callback_type)
        return check_cluster_support_for_node(unid, endpoint, chip::app::Clusters::OnOff::Id,
            chip::app::Clusters::OnOff::Commands::OnWithRecallGlobalScene::Id);

    if (mpc_attribute_store_get_endpoint_and_node_from_unid(unid, endpoint, nodeId, endpointId) == SL_STATUS_OK) {
        cmd.Send(chip::ScopedNodeId(nodeId, 1), endpointId);
        return SL_STATUS_OK;
    }
    return SL_STATUS_FAIL;
}

static sl_status_t mpc_on_off_cluster_on_with_timed_off_command(const dotdot_unid_t unid, dotdot_endpoint_id_t endpoint,
    uic_mqtt_dotdot_callback_call_type_t callback_type,
    uint8_t on_off_control,

    uint16_t on_time,

    uint16_t off_wait_time)
{
    chip::EndpointId endpointId;
    chip::NodeId nodeId;
    SendableCommand<chip::app::Clusters::OnOff::Commands::OnWithTimedOff::Type> cmd;
    cmd.Data().onOffControl = static_cast<chip::BitMask<OnOffControlBitmap>>(on_off_control);
    cmd.Data().onTime = static_cast<uint16_t>(on_time);
    cmd.Data().offWaitTime = static_cast<uint16_t>(off_wait_time);

    if (UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK == callback_type)
        return check_cluster_support_for_node(unid, endpoint, chip::app::Clusters::OnOff::Id,
            chip::app::Clusters::OnOff::Commands::OnWithTimedOff::Id);

    if (mpc_attribute_store_get_endpoint_and_node_from_unid(unid, endpoint, nodeId, endpointId) == SL_STATUS_OK) {
        cmd.Send(chip::ScopedNodeId(nodeId, 1), endpointId);
        return SL_STATUS_OK;
    }
    return SL_STATUS_FAIL;
}

sl_status_t mpc_on_off_cluster_mapper_init()
{
    sl_log_debug(LOG_TAG, "on_off Cluster mapper initialization\n");
    uic_mqtt_dotdot_on_off_off_callback_set(&mpc_on_off_cluster_off_command);
    uic_mqtt_dotdot_on_off_on_callback_set(&mpc_on_off_cluster_on_command);
    uic_mqtt_dotdot_on_off_toggle_callback_set(&mpc_on_off_cluster_toggle_command);
    uic_mqtt_dotdot_on_off_off_with_effect_callback_set(&mpc_on_off_cluster_off_with_effect_command);
    uic_mqtt_dotdot_on_off_on_with_recall_global_scene_callback_set(&mpc_on_off_cluster_on_with_recall_global_scene_command);
    uic_mqtt_dotdot_on_off_on_with_timed_off_callback_set(&mpc_on_off_cluster_on_with_timed_off_command);

    return SL_STATUS_OK;
}
// Callbacks used by the level cluster
static sl_status_t mpc_level_cluster_move_to_level_command(const dotdot_unid_t unid, dotdot_endpoint_id_t endpoint,
    uic_mqtt_dotdot_callback_call_type_t callback_type, uint8_t level,

    uint16_t transition_time,

    uint8_t options_mask,

    uint8_t options_override)
{
    chip::EndpointId endpointId;
    chip::NodeId nodeId;
    SendableCommand<chip::app::Clusters::LevelControl::Commands::MoveToLevel::Type> cmd;
    cmd.Data().level = static_cast<uint8_t>(level);
    cmd.Data().transitionTime = static_cast<DataModel::Nullable<uint16_t>>(transition_time);
    cmd.Data().optionsMask = static_cast<chip::BitMask<LevelControlOptions>>(options_mask);
    cmd.Data().optionsOverride = static_cast<chip::BitMask<LevelControlOptions>>(options_override);

    if (UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK == callback_type)
        return check_cluster_support_for_node(unid, endpoint, chip::app::Clusters::LevelControl::Id,
            chip::app::Clusters::LevelControl::Commands::MoveToLevel::Id);

    if (mpc_attribute_store_get_endpoint_and_node_from_unid(unid, endpoint, nodeId, endpointId) == SL_STATUS_OK) {
        cmd.Send(chip::ScopedNodeId(nodeId, 1), endpointId);
        return SL_STATUS_OK;
    }
    return SL_STATUS_FAIL;
}

static sl_status_t mpc_level_cluster_move_command(const dotdot_unid_t unid, dotdot_endpoint_id_t endpoint,
    uic_mqtt_dotdot_callback_call_type_t callback_type, MoveStepMode move_mode,

    uint8_t rate,

    uint8_t options_mask,

    uint8_t options_override)
{
    chip::EndpointId endpointId;
    chip::NodeId nodeId;
    SendableCommand<chip::app::Clusters::LevelControl::Commands::Move::Type> cmd;
    cmd.Data().moveMode = static_cast<MoveMode>(move_mode);
    cmd.Data().rate = static_cast<DataModel::Nullable<uint8_t>>(rate);
    cmd.Data().optionsMask = static_cast<chip::BitMask<LevelControlOptions>>(options_mask);
    cmd.Data().optionsOverride = static_cast<chip::BitMask<LevelControlOptions>>(options_override);

    if (UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK == callback_type)
        return check_cluster_support_for_node(unid, endpoint, chip::app::Clusters::LevelControl::Id,
            chip::app::Clusters::LevelControl::Commands::Move::Id);

    if (mpc_attribute_store_get_endpoint_and_node_from_unid(unid, endpoint, nodeId, endpointId) == SL_STATUS_OK) {
        cmd.Send(chip::ScopedNodeId(nodeId, 1), endpointId);
        return SL_STATUS_OK;
    }
    return SL_STATUS_FAIL;
}

static sl_status_t mpc_level_cluster_step_command(const dotdot_unid_t unid, dotdot_endpoint_id_t endpoint,
    uic_mqtt_dotdot_callback_call_type_t callback_type, MoveStepMode step_mode,

    uint8_t step_size,

    uint16_t transition_time,

    uint8_t options_mask,

    uint8_t options_override)
{
    chip::EndpointId endpointId;
    chip::NodeId nodeId;
    SendableCommand<chip::app::Clusters::LevelControl::Commands::Step::Type> cmd;
    cmd.Data().stepMode = static_cast<StepMode>(step_mode);
    cmd.Data().stepSize = static_cast<uint8_t>(step_size);
    cmd.Data().transitionTime = static_cast<DataModel::Nullable<uint16_t>>(transition_time);
    cmd.Data().optionsMask = static_cast<chip::BitMask<LevelControlOptions>>(options_mask);
    cmd.Data().optionsOverride = static_cast<chip::BitMask<LevelControlOptions>>(options_override);

    if (UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK == callback_type)
        return check_cluster_support_for_node(unid, endpoint, chip::app::Clusters::LevelControl::Id,
            chip::app::Clusters::LevelControl::Commands::Step::Id);

    if (mpc_attribute_store_get_endpoint_and_node_from_unid(unid, endpoint, nodeId, endpointId) == SL_STATUS_OK) {
        cmd.Send(chip::ScopedNodeId(nodeId, 1), endpointId);
        return SL_STATUS_OK;
    }
    return SL_STATUS_FAIL;
}

static sl_status_t mpc_level_cluster_stop_command(const dotdot_unid_t unid, dotdot_endpoint_id_t endpoint,
    uic_mqtt_dotdot_callback_call_type_t callback_type, uint8_t options_mask,

    uint8_t options_override)
{
    chip::EndpointId endpointId;
    chip::NodeId nodeId;
    SendableCommand<chip::app::Clusters::LevelControl::Commands::Stop::Type> cmd;
    cmd.Data().optionsMask = static_cast<chip::BitMask<LevelControlOptions>>(options_mask);
    cmd.Data().optionsOverride = static_cast<chip::BitMask<LevelControlOptions>>(options_override);

    if (UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK == callback_type)
        return check_cluster_support_for_node(unid, endpoint, chip::app::Clusters::LevelControl::Id,
            chip::app::Clusters::LevelControl::Commands::Stop::Id);

    if (mpc_attribute_store_get_endpoint_and_node_from_unid(unid, endpoint, nodeId, endpointId) == SL_STATUS_OK) {
        cmd.Send(chip::ScopedNodeId(nodeId, 1), endpointId);
        return SL_STATUS_OK;
    }
    return SL_STATUS_FAIL;
}

static sl_status_t mpc_level_cluster_move_to_level_with_on_off_command(const dotdot_unid_t unid, dotdot_endpoint_id_t endpoint,
    uic_mqtt_dotdot_callback_call_type_t callback_type,
    uint8_t level,

    uint16_t transition_time,

    uint8_t options_mask,

    uint8_t options_override)
{
    chip::EndpointId endpointId;
    chip::NodeId nodeId;
    SendableCommand<chip::app::Clusters::LevelControl::Commands::MoveToLevelWithOnOff::Type> cmd;
    cmd.Data().level = static_cast<uint8_t>(level);
    cmd.Data().transitionTime = static_cast<DataModel::Nullable<uint16_t>>(transition_time);
    cmd.Data().optionsMask = static_cast<chip::BitMask<LevelControlOptions>>(options_mask);
    cmd.Data().optionsOverride = static_cast<chip::BitMask<LevelControlOptions>>(options_override);

    if (UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK == callback_type)
        return check_cluster_support_for_node(unid, endpoint, chip::app::Clusters::LevelControl::Id,
            chip::app::Clusters::LevelControl::Commands::MoveToLevelWithOnOff::Id);

    if (mpc_attribute_store_get_endpoint_and_node_from_unid(unid, endpoint, nodeId, endpointId) == SL_STATUS_OK) {
        cmd.Send(chip::ScopedNodeId(nodeId, 1), endpointId);
        return SL_STATUS_OK;
    }
    return SL_STATUS_FAIL;
}

static sl_status_t mpc_level_cluster_move_with_on_off_command(const dotdot_unid_t unid, dotdot_endpoint_id_t endpoint,
    uic_mqtt_dotdot_callback_call_type_t callback_type,
    MoveStepMode move_mode,

    uint8_t rate,

    uint8_t options_mask,

    uint8_t options_override)
{
    chip::EndpointId endpointId;
    chip::NodeId nodeId;
    SendableCommand<chip::app::Clusters::LevelControl::Commands::MoveWithOnOff::Type> cmd;
    cmd.Data().moveMode = static_cast<MoveMode>(move_mode);
    cmd.Data().rate = static_cast<DataModel::Nullable<uint8_t>>(rate);
    cmd.Data().optionsMask = static_cast<chip::BitMask<LevelControlOptions>>(options_mask);
    cmd.Data().optionsOverride = static_cast<chip::BitMask<LevelControlOptions>>(options_override);

    if (UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK == callback_type)
        return check_cluster_support_for_node(unid, endpoint, chip::app::Clusters::LevelControl::Id,
            chip::app::Clusters::LevelControl::Commands::MoveWithOnOff::Id);

    if (mpc_attribute_store_get_endpoint_and_node_from_unid(unid, endpoint, nodeId, endpointId) == SL_STATUS_OK) {
        cmd.Send(chip::ScopedNodeId(nodeId, 1), endpointId);
        return SL_STATUS_OK;
    }
    return SL_STATUS_FAIL;
}

static sl_status_t mpc_level_cluster_step_with_on_off_command(const dotdot_unid_t unid, dotdot_endpoint_id_t endpoint,
    uic_mqtt_dotdot_callback_call_type_t callback_type,
    MoveStepMode step_mode,

    uint8_t step_size,

    uint16_t transition_time,

    uint8_t options_mask,

    uint8_t options_override)
{
    chip::EndpointId endpointId;
    chip::NodeId nodeId;
    SendableCommand<chip::app::Clusters::LevelControl::Commands::StepWithOnOff::Type> cmd;
    cmd.Data().stepMode = static_cast<StepMode>(step_mode);
    cmd.Data().stepSize = static_cast<uint8_t>(step_size);
    cmd.Data().transitionTime = static_cast<DataModel::Nullable<uint16_t>>(transition_time);
    cmd.Data().optionsMask = static_cast<chip::BitMask<LevelControlOptions>>(options_mask);
    cmd.Data().optionsOverride = static_cast<chip::BitMask<LevelControlOptions>>(options_override);

    if (UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK == callback_type)
        return check_cluster_support_for_node(unid, endpoint, chip::app::Clusters::LevelControl::Id,
            chip::app::Clusters::LevelControl::Commands::StepWithOnOff::Id);

    if (mpc_attribute_store_get_endpoint_and_node_from_unid(unid, endpoint, nodeId, endpointId) == SL_STATUS_OK) {
        cmd.Send(chip::ScopedNodeId(nodeId, 1), endpointId);
        return SL_STATUS_OK;
    }
    return SL_STATUS_FAIL;
}

static sl_status_t mpc_level_cluster_stop_with_on_off_command(const dotdot_unid_t unid, dotdot_endpoint_id_t endpoint,
    uic_mqtt_dotdot_callback_call_type_t callback_type,
    uint8_t options_mask,

    uint8_t options_override)
{
    chip::EndpointId endpointId;
    chip::NodeId nodeId;
    SendableCommand<chip::app::Clusters::LevelControl::Commands::StopWithOnOff::Type> cmd;
    cmd.Data().optionsMask = static_cast<chip::BitMask<LevelControlOptions>>(options_mask);
    cmd.Data().optionsOverride = static_cast<chip::BitMask<LevelControlOptions>>(options_override);

    if (UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK == callback_type)
        return check_cluster_support_for_node(unid, endpoint, chip::app::Clusters::LevelControl::Id,
            chip::app::Clusters::LevelControl::Commands::StopWithOnOff::Id);

    if (mpc_attribute_store_get_endpoint_and_node_from_unid(unid, endpoint, nodeId, endpointId) == SL_STATUS_OK) {
        cmd.Send(chip::ScopedNodeId(nodeId, 1), endpointId);
        return SL_STATUS_OK;
    }
    return SL_STATUS_FAIL;
}

static sl_status_t mpc_level_cluster_move_to_closest_frequency_command(const dotdot_unid_t unid, dotdot_endpoint_id_t endpoint,
    uic_mqtt_dotdot_callback_call_type_t callback_type,
    uint16_t frequency)
{
    chip::EndpointId endpointId;
    chip::NodeId nodeId;
    SendableCommand<chip::app::Clusters::LevelControl::Commands::MoveToClosestFrequency::Type> cmd;
    cmd.Data().frequency = static_cast<uint16_t>(frequency);

    if (UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK == callback_type)
        return check_cluster_support_for_node(unid, endpoint, chip::app::Clusters::LevelControl::Id,
            chip::app::Clusters::LevelControl::Commands::MoveToClosestFrequency::Id);

    if (mpc_attribute_store_get_endpoint_and_node_from_unid(unid, endpoint, nodeId, endpointId) == SL_STATUS_OK) {
        cmd.Send(chip::ScopedNodeId(nodeId, 1), endpointId);
        return SL_STATUS_OK;
    }
    return SL_STATUS_FAIL;
}

sl_status_t mpc_level_cluster_mapper_init()
{
    sl_log_debug(LOG_TAG, "level Cluster mapper initialization\n");
    uic_mqtt_dotdot_level_move_to_level_callback_set(&mpc_level_cluster_move_to_level_command);
    uic_mqtt_dotdot_level_move_callback_set(&mpc_level_cluster_move_command);
    uic_mqtt_dotdot_level_step_callback_set(&mpc_level_cluster_step_command);
    uic_mqtt_dotdot_level_stop_callback_set(&mpc_level_cluster_stop_command);
    uic_mqtt_dotdot_level_move_to_level_with_on_off_callback_set(&mpc_level_cluster_move_to_level_with_on_off_command);
    uic_mqtt_dotdot_level_move_with_on_off_callback_set(&mpc_level_cluster_move_with_on_off_command);
    uic_mqtt_dotdot_level_step_with_on_off_callback_set(&mpc_level_cluster_step_with_on_off_command);
    uic_mqtt_dotdot_level_stop_with_on_off_callback_set(&mpc_level_cluster_stop_with_on_off_command);
    uic_mqtt_dotdot_level_move_to_closest_frequency_callback_set(&mpc_level_cluster_move_to_closest_frequency_command);

    return SL_STATUS_OK;
}
