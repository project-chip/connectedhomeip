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
#include "group_command_translator.hpp"
#include "uic_mqtt.h"
#include <iostream>
#include <string>
#include <sstream>
#include "sl_log.h"
#include <nlohmann/json.hpp>
#include <app-common/zap-generated/cluster-objects.h>


using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters::Groups;
using namespace chip::DeviceLayer;
using namespace unify::matter_bridge;

#define LOG_TAG "group_cluster_server"


void GroupClusterCommandHandler::InvokeCommand(
  CommandHandlerInterface::HandlerContext &ctxt)
{
  
  std::string cmd;
  nlohmann::json payload;  
  
  // We copy the TLV reader here, because it is state full. and we need the
  // ember group handler to read the parameters as well
  TLV::TLVReader tlv;
  tlv.Init(ctxt.GetReader());

  switch (ctxt.mRequestPath.mCommandId) {
    case Commands::AddGroup::Id:
    {
      Commands::AddGroup::DecodableType addGroupData;
      CHIP_ERROR TLVError = DataModel::Decode(tlv, addGroupData);
      if(CHIP_ERROR::IsSuccess(TLVError)) {
        cmd = "AddGroup";
        payload["GroupId"] = addGroupData.groupId;
        payload["GroupName"] = std::string(addGroupData.groupName.begin(),addGroupData.groupName.end());

        group_translator::matter_group group = { addGroupData.groupId };
        group_translator::instance().add_matter_group( group );
      }
    }
      break;
    case Commands::RemoveGroup::Id:
    {
      Commands::RemoveGroup::DecodableType removeGroupData;
      CHIP_ERROR TLVError = DataModel::Decode(tlv, removeGroupData);
      if(CHIP_ERROR::IsSuccess(TLVError)) {
        cmd = "RemoveGroup";
        payload["GroupId"] = removeGroupData.groupId;
        group_translator::matter_group group = {removeGroupData.groupId };
        group_translator::instance().remove_matter_group( group );
      }
    }
      break;
    default:
      // As we are just "sniffing" on the commands we do not need to 
      // do the actual matter handling here. This is done by the ember code.
      break;
  }

  // We will not set the handle flag on the group cluster,
  // Because it need to be passed to the Matter App framework
  ctxt.SetCommandNotHandled();
  if(!cmd.empty()) {
    send_unify_mqtt_cmd(ctxt, cmd, payload);
  }

}



