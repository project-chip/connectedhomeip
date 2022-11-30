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
#include "sl_log.h"
#include "uic_mqtt.h"
#include <app-common/zap-generated/cluster-objects.h>
#include <credentials/GroupDataProvider.h>
#include <iostream>
#include <nlohmann/json.hpp>
#include <optional>
#include <sstream>
#include <string>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters::Groups;
using namespace chip::DeviceLayer;
using namespace unify::matter_bridge;

using namespace chip::Credentials;

#define LOG_TAG "group_cluster_server"

void GroupClusterCommandHandler::RemoveAllGroups(chip::FabricIndex fabric_index)
{
    GroupDataProvider * provider = GetGroupDataProvider();
    auto group_info_iter         = provider->IterateGroupInfo(fabric_index);
    group_translator::matter_group removed_matter_group;
    removed_matter_group.fabric_index = fabric_index;
    GroupDataProvider::GroupInfo group;
    if (group_info_iter != nullptr)
    {
        while (group_info_iter->Next(group))
        {
            removed_matter_group.group = group.group_id;
            m_group_translator.remove_matter_group(removed_matter_group);
        }
        group_info_iter->Release();
    }
}

void GroupClusterCommandHandler::InvokeCommand(CommandHandlerInterface::HandlerContext & ctxt)
{

    std::string cmd;
    nlohmann::json payload;

    // We copy the TLV reader here, because it is state full. and we need the
    // ember group handler to read the parameters as well
    TLV::TLVReader tlv;
    tlv.Init(ctxt.GetReader());
    chip::FabricIndex fabric_index = ctxt.mCommandHandler.GetExchangeContext()->GetSessionHandle()->GetFabricIndex();
    switch (ctxt.mRequestPath.mCommandId)
    {
    case Commands::AddGroup::Id: {
        Commands::AddGroup::DecodableType addGroupData;
        CHIP_ERROR TLVError = DataModel::Decode(tlv, addGroupData);
        if (CHIP_ERROR::IsSuccess(TLVError))
        {
            // Checking if the Endpoint is already a part of the GroupID
            GroupDataProvider * provider = GetGroupDataProvider();
            if (provider->HasEndpoint(fabric_index, addGroupData.groupId, ctxt.mRequestPath.mEndpointId))
            {
                break;
            }
            // Checking if the GroupId has already entry
            GroupDataProvider::GroupInfo group_info;
            group_translator::matter_group group = { addGroupData.groupId, fabric_index };
            if (CHIP_NO_ERROR != provider->GetGroupInfo(fabric_index, addGroupData.groupId, group_info))
            {
                if (!m_group_translator.add_matter_group(group))
                {
                    sl_log_warning(LOG_TAG, "Failed to assign request matter group a corresponding unify group id");
                    break;
                }
            }
            std::optional<unify_group_t> unify_group = m_group_translator.get_unify_group(group);
            if (unify_group.has_value())
            {
                cmd                  = "AddGroup";
                payload["GroupName"] = std::string(addGroupData.groupName.begin(), addGroupData.groupName.end());
                payload["GroupId"]   = unify_group.value();
            }
        }
    }
    break;
    case Commands::RemoveGroup::Id: {
        Commands::RemoveGroup::DecodableType removeGroupData;
        CHIP_ERROR TLVError = DataModel::Decode(tlv, removeGroupData);
        if (CHIP_ERROR::IsSuccess(TLVError))
        {
            group_translator::matter_group group     = { removeGroupData.groupId, fabric_index };
            std::optional<unify_group_t> unify_group = m_group_translator.get_unify_group(group);
            if (unify_group.has_value())
            {
                m_group_translator.remove_matter_group(group);
                cmd                = "RemoveGroup";
                payload["GroupId"] = unify_group.value();
            }
        }
    }
    break;
    case Commands::RemoveAllGroups::Id: {
        Commands::RemoveAllGroups::DecodableType removingAllGroupData;
        CHIP_ERROR TLVError = DataModel::Decode(tlv, removingAllGroupData);
        if (CHIP_ERROR::IsSuccess(TLVError))
        {
            GroupClusterCommandHandler::RemoveAllGroups(fabric_index);
            cmd = "RemoveAllGroups";
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
    if (!cmd.empty())
    {
        send_unify_mqtt_cmd(ctxt, cmd, payload);
    }
}
