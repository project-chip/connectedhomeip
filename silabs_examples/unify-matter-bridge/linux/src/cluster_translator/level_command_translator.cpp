/******************************************************************************
 * # License
 * <b>Copyright 2022 Silicon Laboratories Inc. www.silabs.com</b>
 ******************************************************************************
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 *****************************************************************************/
#include "level_command_translator.hpp"

// Standard library
#include <iostream>
#include <string>
#include <sstream>

// Unify library
#include "uic_mqtt.h"
#include "sl_log.h"

namespace Unify
{
#include "dotdot_mqtt_send_commands.h"
}

// Third party
#include <nlohmann/json.hpp>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::LevelControl;
using namespace chip::DeviceLayer;
using namespace unify::matter_bridge;

#define LOG_TAG "level_command_cluster_server"

void set_command_handled_success(CommandHandlerInterface::HandlerContext &ctxt)
{
  ctxt.mCommandHandler.AddStatus(ctxt.mRequestPath,
                                 Protocols::InteractionModel::Status::Success);
  ctxt.SetCommandHandled();
}

void set_command_handled_failed(CommandHandlerInterface::HandlerContext &ctxt)
{
  ctxt.mCommandHandler.AddStatus(
    ctxt.mRequestPath,
    Protocols::InteractionModel::Status::InvalidCommand);
  ctxt.SetCommandHandled();
}

void LevelClusterCommandHandler::InvokeCommand(
  CommandHandlerInterface::HandlerContext &ctxt)
{
  auto unify_node
    = m_node_state_monitor.bridged_endpoint(ctxt.mRequestPath.mEndpointId);

  if (!unify_node) {
    return;
  }

  switch (ctxt.mRequestPath.mCommandId) {
    case Commands::MoveToLevel::Id: {
      Commands::MoveToLevel::DecodableType move_to_level_command_data;
      CHIP_ERROR TLVError
        = DataModel::Decode(ctxt.GetReader(), move_to_level_command_data);

      if (!CHIP_ERROR::IsSuccess(TLVError)) {
        set_command_handled_failed(ctxt);
        sl_log_error(LOG_TAG,
                     "Failed processing MoveToLevel command on Level Cluster");
        return;
      }

      Unify::uic_mqtt_dotdot_level_command_move_to_level_fields_t
        move_to_level_fields;
      move_to_level_fields.level = move_to_level_command_data.level;
      move_to_level_fields.transition_time
        = move_to_level_command_data.transitionTime;
      move_to_level_fields.options_mask = move_to_level_command_data.optionMask;
      move_to_level_fields.options_override
        = move_to_level_command_data.optionOverride;

      send_unify_mqtt_command_with_callbacks<
        const Unify::uic_mqtt_dotdot_level_command_move_to_level_fields_t *>(
        ctxt,
        &move_to_level_fields,
        Unify::uic_mqtt_dotdot_level_publish_move_to_level_command,
        Unify::uic_mqtt_dotdot_level_publish_move_to_level_command_to_group);

      set_command_handled_success(ctxt);
    } break;
    case Commands::Move::Id: {
      Commands::Move::DecodableType move_command_data;
      CHIP_ERROR TLVError
        = DataModel::Decode(ctxt.GetReader(), move_command_data);

      if (!CHIP_ERROR::IsSuccess(TLVError)) {
        set_command_handled_failed(ctxt);
        sl_log_error(LOG_TAG,
                     "Failed processing Move command on Level Cluster");
        return;
      }

      Unify::uic_mqtt_dotdot_level_command_move_fields_t move_fields;
      move_fields.move_mode
        = static_cast<Unify::MoveStepMode>(move_command_data.moveMode);
      move_fields.rate             = move_command_data.rate;
      move_fields.options_mask     = move_command_data.optionMask;
      move_fields.options_override = move_command_data.optionOverride;

      send_unify_mqtt_command_with_callbacks<
        const Unify::uic_mqtt_dotdot_level_command_move_fields_t *>(
        ctxt,
        &move_fields,
        Unify::uic_mqtt_dotdot_level_publish_move_command,
        Unify::uic_mqtt_dotdot_level_publish_move_command_to_group);
      set_command_handled_success(ctxt);
    } break;
    case Commands::Step::Id: {
      Commands::Step::DecodableType step_command_data;

      CHIP_ERROR TLVError
        = DataModel::Decode(ctxt.GetReader(), step_command_data);

      if (!CHIP_ERROR::IsSuccess(TLVError)) {
        set_command_handled_failed(ctxt);
        sl_log_error(LOG_TAG,
                     "Failed processing Step command on Level Cluster");
        return;
      }

      Unify::uic_mqtt_dotdot_level_command_step_fields_t step_fields;
      step_fields.step_mode
        = static_cast<Unify::MoveStepMode>(step_command_data.stepMode);
      step_fields.step_size        = step_command_data.stepSize;
      step_fields.transition_time  = step_command_data.transitionTime;
      step_fields.options_mask     = step_command_data.optionMask;
      step_fields.options_override = step_command_data.optionOverride;
      send_unify_mqtt_command_with_callbacks<
        const Unify::uic_mqtt_dotdot_level_command_step_fields_t *>(
        ctxt,
        &step_fields,
        Unify::uic_mqtt_dotdot_level_publish_step_command,
        Unify::uic_mqtt_dotdot_level_publish_step_command_to_group);
      set_command_handled_success(ctxt);
    } break;
    case Commands::Stop::Id: {
      Commands::Stop::DecodableType stop_command_data;
      CHIP_ERROR TLVError
        = DataModel::Decode(ctxt.GetReader(), stop_command_data);

      if (!CHIP_ERROR::IsSuccess(TLVError)) {
        set_command_handled_failed(ctxt);
        sl_log_error(LOG_TAG,
                     "Failed processing Stop command on Level Cluster");
        return;
      }

      Unify::uic_mqtt_dotdot_level_command_stop_fields_t stop_fields;
      stop_fields.options_mask     = stop_command_data.optionMask;
      stop_fields.options_override = stop_command_data.optionOverride;
      send_unify_mqtt_command_with_callbacks<
        const Unify::uic_mqtt_dotdot_level_command_stop_fields_t *>(
        ctxt,
        &stop_fields,
        Unify::uic_mqtt_dotdot_level_publish_stop_command,
        Unify::uic_mqtt_dotdot_level_publish_stop_command_to_group);
      set_command_handled_success(ctxt);
    } break;
    case Commands::MoveToLevelWithOnOff::Id: {
      Commands::MoveToLevelWithOnOff::DecodableType
        move_to_level_with_on_off_command_data;
      CHIP_ERROR TLVError
        = DataModel::Decode(ctxt.GetReader(),
                            move_to_level_with_on_off_command_data);

      if (!CHIP_ERROR::IsSuccess(TLVError)) {
        set_command_handled_failed(ctxt);
        sl_log_error(
          LOG_TAG,
          "Failed processing MoveToLevelWithOnOff command on Level Cluster");
        return;
      }

      Unify::uic_mqtt_dotdot_level_command_move_to_level_with_on_off_fields_t
        move_to_level_with_on_off_fields;
      move_to_level_with_on_off_fields.level
        = move_to_level_with_on_off_command_data.level;
      move_to_level_with_on_off_fields.transition_time
        = move_to_level_with_on_off_command_data.transitionTime;
      move_to_level_with_on_off_fields.options_mask     = 0;
      move_to_level_with_on_off_fields.options_override = 0;
      send_unify_mqtt_command_with_callbacks<
        const Unify::
          uic_mqtt_dotdot_level_command_move_to_level_with_on_off_fields_t *>(
        ctxt,
        &move_to_level_with_on_off_fields,
        Unify::uic_mqtt_dotdot_level_publish_move_to_level_with_on_off_command,
        Unify::
          uic_mqtt_dotdot_level_publish_move_to_level_with_on_off_command_to_group);
      set_command_handled_success(ctxt);
    } break;
    case Commands::MoveWithOnOff::Id: {
      Commands::MoveWithOnOff::DecodableType move_with_on_off_command_data;
      CHIP_ERROR TLVError
        = DataModel::Decode(ctxt.GetReader(), move_with_on_off_command_data);

      if (!CHIP_ERROR::IsSuccess(TLVError)) {
        set_command_handled_failed(ctxt);
        sl_log_error(
          LOG_TAG,
          "Failed processing MoveWithOnOff command on Level Cluster");
        return;
      }

      Unify::uic_mqtt_dotdot_level_command_move_with_on_off_fields_t
        move_with_on_off_fields;
      move_with_on_off_fields.move_mode = static_cast<Unify::MoveStepMode>(
        move_with_on_off_command_data.moveMode);
      move_with_on_off_fields.rate         = move_with_on_off_command_data.rate;
      move_with_on_off_fields.options_mask = 0;
      move_with_on_off_fields.options_override = 0;
      send_unify_mqtt_command_with_callbacks<
        const Unify::uic_mqtt_dotdot_level_command_move_with_on_off_fields_t *>(
        ctxt,
        &move_with_on_off_fields,
        Unify::uic_mqtt_dotdot_level_publish_move_with_on_off_command,
        Unify::uic_mqtt_dotdot_level_publish_move_with_on_off_command_to_group);
      set_command_handled_success(ctxt);
    } break;
    case Commands::StepWithOnOff::Id: {
      Commands::StepWithOnOff::DecodableType step_with_on_off_command_data;
      CHIP_ERROR TLVError
        = DataModel::Decode(ctxt.GetReader(), step_with_on_off_command_data);

      if (!CHIP_ERROR::IsSuccess(TLVError)) {
        set_command_handled_failed(ctxt);
        sl_log_error(
          LOG_TAG,
          "Failed processing StepWithOnOff command on Level Cluster");
        return;
      }

      Unify::uic_mqtt_dotdot_level_command_step_with_on_off_fields_t
        step_with_on_off_fields;
      step_with_on_off_fields.step_mode = static_cast<Unify::MoveStepMode>(
        step_with_on_off_command_data.stepMode);
      step_with_on_off_fields.step_size
        = step_with_on_off_command_data.stepSize;
      step_with_on_off_fields.transition_time
        = step_with_on_off_command_data.transitionTime;

      send_unify_mqtt_command_with_callbacks<
        const Unify::uic_mqtt_dotdot_level_command_step_with_on_off_fields_t *>(
        ctxt,
        &step_with_on_off_fields,
        Unify::uic_mqtt_dotdot_level_publish_step_with_on_off_command,
        Unify::uic_mqtt_dotdot_level_publish_step_with_on_off_command_to_group);

      set_command_handled_success(ctxt);
    } break;
    case Commands::StopWithOnOff::Id: {
      Commands::StopWithOnOff::DecodableType stop_with_on_off_command_data;
      CHIP_ERROR TLVError
        = DataModel::Decode(ctxt.GetReader(), stop_with_on_off_command_data);

      if (!CHIP_ERROR::IsSuccess(TLVError)) {
        set_command_handled_failed(ctxt);
        sl_log_error(
          LOG_TAG,
          "Failed processing StopWithOnOff command on Level Cluster");
        return;
      }

      Unify::uic_mqtt_dotdot_level_command_stop_with_on_off_fields_t
        stop_with_on_off_fields;
      stop_with_on_off_fields.options_mask     = 0;
      stop_with_on_off_fields.options_override = 0;

      send_unify_mqtt_command_with_callbacks<
        const Unify::uic_mqtt_dotdot_level_command_stop_with_on_off_fields_t *>(
        ctxt,
        &stop_with_on_off_fields,
        Unify::uic_mqtt_dotdot_level_publish_stop_with_on_off_command,
        Unify::uic_mqtt_dotdot_level_publish_stop_with_on_off_command_to_group);
      set_command_handled_success(ctxt);
    } break;
    default: {
      sl_log_error(LOG_TAG,
                   "Unsupported command on Level Cluster: %d",
                   ctxt.mRequestPath.mCommandId);
      return;
    }
  }
}
