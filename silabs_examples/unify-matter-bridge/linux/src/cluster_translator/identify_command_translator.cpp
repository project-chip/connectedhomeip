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
#include "identify_command_translator.hpp"
#include <attribute_state_cache.hpp>

// Standard library
#include <iostream>
#include <string>
#include <sstream>

// Unify library
#include "uic_mqtt.h"
#include "sl_log.h"

// Third party
#include <nlohmann/json.hpp>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::Identify;
using namespace unify::matter_bridge;

#define LOG_TAG "identify_cluster_command_server"

void IdentifyClusterCommandHandler::InvokeCommand(
  CommandHandlerInterface::HandlerContext &ctxt)
{
  auto unify_node
    = m_node_state_monitor.bridged_endpoint(ctxt.mRequestPath.mEndpointId);

  if (!unify_node) {
    return;
  }
  nlohmann::json payload     = {};

  std::string cmd;  
  switch (ctxt.mRequestPath.mCommandId) {
    case Commands::Identify::Id: {
      Commands::Identify::DecodableType data;
      CHIP_ERROR TLVError = DataModel::Decode(ctxt.GetReader(), data);
      if (TLVError != CHIP_NO_ERROR) {
        ctxt.mCommandHandler.AddStatus(
          ctxt.mRequestPath,
          Protocols::InteractionModel::Status::InvalidCommand);
        ctxt.SetCommandHandled();
        sl_log_error(LOG_TAG, "Unable to read Identify command payload");
        return;
      }
      
      cmd = "Identify";
      payload["IdentifyTime"] = data.identifyTime;
      ctxt.mCommandHandler.AddStatus(
        ctxt.mRequestPath,
        Protocols::InteractionModel::Status::Success);
      ctxt.SetCommandHandled();
      break;
    }
    /**case Commands::IdentifyQuery::Id: {
      cmd = "IdentifyQuery";
      //read the remaining time in second for identify the endpoint
      ConcreteAttributePath attr_path
        = ConcreteAttributePath(ctxt.mRequestPath.mEndpointId,
                                Identify::Id,
                                Attributes::IdentifyTime::Id);
      Attributes::IdentifyTime::TypeInfo::Type remaining_time = 0;
      remaining_time
        = attribute_state_cache::get_instance()
            .get<Attributes::IdentifyTime::TypeInfo::Type>(attr_path);
      // Preparing the response
      Commands::IdentifyQueryResponse::Type data_response;
      data_response.timeout = remaining_time;
      ctxt.mCommandHandler.AddResponseData(ctxt.mRequestPath, data_response);
      break;
    }*/
    case Commands::TriggerEffect::Id: {
      Commands::TriggerEffect::DecodableType data;
      CHIP_ERROR TLVError = DataModel::Decode(ctxt.GetReader(), data);
      if (TLVError != CHIP_NO_ERROR) {
        ctxt.mCommandHandler.AddStatus(
          ctxt.mRequestPath,
          Protocols::InteractionModel::Status::InvalidCommand);
        ctxt.SetCommandHandled();
        sl_log_error(LOG_TAG, "Unable to read TriggerEffect command payload");
        return;
      }
      cmd = "TriggerEffect";
      payload["EffectIdentifier"] = static_cast<uint8_t>(data.effectIdentifier);
      payload["EffectVariant"]    = static_cast<uint8_t>(data.effectVariant);
      ctxt.mCommandHandler.AddStatus(
        ctxt.mRequestPath,
        Protocols::InteractionModel::Status::Success);
      ctxt.SetCommandHandled();
      break;
    }
    default: {
      ctxt.mCommandHandler.AddStatus(
        ctxt.mRequestPath,
        Protocols::InteractionModel::Status::UnsupportedCommand);
      ctxt.SetCommandHandled();
      sl_log_info(LOG_TAG, "Unknown identify cluster command is received");
      return;
    }
  }
  send_unify_mqtt_cmd( ctxt, cmd, payload);
}