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
#include "on_off_command_translator.h"
#include <iostream>
#include <string>
#include <sstream>
#include "sl_log.h"

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::OnOff;
using namespace chip::DeviceLayer;
using namespace unify::matter_bridge;

#define LOG_TAG "on_off_cluster_server"

void OnOffClusterCommandHandler::InvokeCommand(
  CommandHandlerInterface::HandlerContext &ctxt)
{
  std::string cmd;
  
  switch (ctxt.mRequestPath.mCommandId) {
    case Commands::On::Id:
      cmd = "On";
      break;
    case Commands::Off::Id:
      cmd = "Off";
      break;
    case Commands::Toggle::Id:
      cmd = "Toggle";
      break;
  }

  if(!cmd.empty()) {
    ctxt.mCommandHandler.AddStatus(ctxt.mRequestPath, Protocols::InteractionModel::Status::Success);
    send_unify_mqtt_cmd(ctxt,cmd, nlohmann::json() );
  } else {
    ctxt.mCommandHandler.AddStatus(ctxt.mRequestPath, Protocols::InteractionModel::Status::UnsupportedCommand);
  }
  ctxt.SetCommandHandled();
}



