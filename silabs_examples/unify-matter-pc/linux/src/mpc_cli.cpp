/******************************************************************************
 * # License
 * <b>Copyright 2023 Silicon Laboratories Inc. www.silabs.com</b>
 ******************************************************************************
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 *****************************************************************************/

#include "mpc_command_sender.hpp"
#include "mpc_sendable_command.hpp"
#include "sl_log.h"
#include "sl_status.h"
#include "uic_stdin.hpp"
#include "uic_stdin_process.h"

#include "Options.h"
#include "app/server/OnboardingCodesUtil.h"
#include "app/server/Server.h"
#include <iostream>

using namespace chip;
using namespace chip::app;
using namespace mpc;

#define LOG_TAG "mpc_cli"

// CLI handling
static sl_status_t open_commission_cli_func(const handle_args_t & arg)
{
    LinuxDeviceOptions::GetInstance().payload.commissioningFlow = chip::CommissioningFlow::kStandard;
    PrintOnboardingCodes(LinuxDeviceOptions::GetInstance().payload);
    Server::GetInstance().GetCommissioningWindowManager().OpenBasicCommissioningWindow();
    return SL_STATUS_OK;
}

static sl_status_t close_commission_cli_func(const handle_args_t & arg)
{
    Server::GetInstance().GetCommissioningWindowManager().CloseCommissioningWindow();
    return SL_STATUS_OK;
}

template <typename T>
class MPCCliAttributeReader : public ReadClient::Callback
{
public:
    MPCCliAttributeReader() {}
    void SetCtxt(T * ctxt) { mCtxt = ctxt; }
    void OnDone(ReadClient * apReadClient) override
    {
        sl_log_info(LOG_TAG, "Something is done");
        chip::Platform::Delete<T>(mCtxt);
    }

    void OnError(CHIP_ERROR err) override { sl_log_error(LOG_TAG, "Error reading attribute : %s", chip::ErrorStr(err)); }
    void OnReportBegin() override { sl_log_error(LOG_TAG, "Report about to start"); }

    void OnReportEnd() override { sl_log_error(LOG_TAG, "Report ends here"); }

    void OnAttributeData(const chip::app::ConcreteDataAttributePath & path, chip::TLV::TLVReader * data,
                         const chip::app::StatusIB & status) override
    {
        CHIP_ERROR error = status.ToChipError();
        sl_log_error(LOG_TAG, "Response : %s", chip::ErrorStr(error));
        if (path.mClusterId == chip::app::Clusters::Descriptor::Id)
        {
            chip::app::Clusters::Descriptor::Attributes::TypeInfo::DecodableType info;
            info.Decode(*data, path);
            size_t count = 0;
            switch (path.mAttributeId)
            {
            case chip::app::Clusters::Descriptor::Attributes::DeviceTypeList::Id: {
                info.deviceTypeList.ComputeSize(&count);
                sl_log_info(LOG_TAG, "%u entries on read to %u", count, path.mAttributeId);
                int i = 0;
                for (auto ent = info.deviceTypeList.begin(); ent.Next(); i++)
                {
                    sl_log_info(LOG_TAG, "[%u] %x", i, ent.GetValue());
                }
            }
            break;
            case chip::app::Clusters::Descriptor::Attributes::ServerList::Id: {
                info.serverList.ComputeSize(&count);
                sl_log_info(LOG_TAG, "%u entries on read to %u", count, path.mAttributeId);
                int i = 0;
                for (auto ent = info.serverList.begin(); ent.Next(); i++)
                {
                    sl_log_info(LOG_TAG, "[%u] %x", i, ent.GetValue());
                }
            }
            break;
            case chip::app::Clusters::Descriptor::Attributes::ClientList::Id: {
                info.clientList.ComputeSize(&count);
                sl_log_info(LOG_TAG, "%u entries on read to %u", count, path.mAttributeId);
                int i = 0;
                for (auto ent = info.clientList.begin(); ent.Next(); i++)
                {
                    sl_log_info(LOG_TAG, "[%u] %x", i, ent.GetValue());
                }
            }
            break;
            case chip::app::Clusters::Descriptor::Attributes::PartsList::Id: {
                info.partsList.ComputeSize(&count);
                sl_log_info(LOG_TAG, "%u entries on read to %u", count, path.mAttributeId);
                int i = 0;
                for (auto ent = info.partsList.begin(); ent.Next(); i++)
                {
                    sl_log_info(LOG_TAG, "[%u] %x", i, ent.GetValue());
                }
            }
            break;
            default:
                break;
            }
        }
    }

private:
    T * mCtxt;
};

static sl_status_t parrse_node_and_endpoint(const handle_args_t & arg, NodeId & dest, EndpointId & epID)
{
    unsigned long long value = 0;
    value                    = stoull(arg[1]);
    if (value < 0x1 || value > 0xFFFFFFEFFFFFFFFF) // not within Operational Node ID range
    {
        sl_log_error(LOG_TAG, "send: invalid dest nodeID");
        return SL_STATUS_FAIL;
    }
    dest = value;

    value = stoull(arg[2]);
    if (value > 0xFFFF) // not within endpoint ID range
    {
        sl_log_error(LOG_TAG, "send: invalid epID");
        return SL_STATUS_FAIL;
    }
    epID = (EndpointId)(value & 0xFFFF);
    return SL_STATUS_OK;
}

static sl_status_t read_attribute_cli_func(const handle_args_t & arg)
{
    static MPCCliAttributeReader<AttributeReadRequest> callbacks;
    if (arg.size() != 5)
    {
        dprintf(uic_stdin_get_output_fd(),
                "Invalid number of arguments (%zu), expected args:"
                "read_attribute <destID>,<epID>,<clusterID>,<attrID> \n",
                arg.size());
        return SL_STATUS_FAIL;
    }

    NodeId dest;
    EndpointId epID;
    ClusterId clusterID;
    AttributeId attrID;

    unsigned long long value = 0;

    if (SL_STATUS_OK != parrse_node_and_endpoint(arg, dest, epID))
    {
        return SL_STATUS_FAIL;
    };

    value = stoull(arg[3]);
    if (value > 0xFFFFFFFF) // not within cluster ID range
    {
        sl_log_error(LOG_TAG, "send: invalid clusterID");
        return SL_STATUS_FAIL;
    }
    clusterID = (ClusterId)(value & 0xFFFFFFFF);

    value = stoull(arg[4]);
    if (value > 0xFFFFFFFF) // not within attribute ID range
    {
        sl_log_error(LOG_TAG, "send: invalid attrID");
        return SL_STATUS_FAIL;
    }
    attrID = (AttributeId)(value & 0xFFFFFFFF);

    auto request = chip::Platform::New<AttributeReadRequest>(dest, epID, clusterID, attrID);
    callbacks.SetCtxt(request);
    request->SetCallbacks(&callbacks);
    request->SendCommand();

    return SL_STATUS_OK;
}

static sl_status_t send_move_to_level_cli_func(const handle_args_t & arg)
{
    NodeId node_id;
    EndpointId endpoint_id;

    if (arg.size() != 3)
    {
        dprintf(uic_stdin_get_output_fd(),
                "Invalid number of arguments (%zu), expected args:"
                "send_move_to_lenl <destID>,<epID>,<level> \n",
                arg.size());
        return SL_STATUS_FAIL;
    }

    if (SL_STATUS_OK != parrse_node_and_endpoint(arg, node_id, endpoint_id))
    {
        return SL_STATUS_FAIL;
    };

    SendableCommand<Clusters::LevelControl::Commands::MoveToLevel::Type> move_to_level;
    move_to_level.Data().level = stoull(arg[2]);
    move_to_level.Send(chip::ScopedNodeId(node_id, 1), endpoint_id);

    return SL_STATUS_OK;
}

static sl_status_t subs_cli_func(const handle_args_t & arg)
{
    static MPCCliAttributeReader<SubscribeRequest> callbacks;
    if (arg.size() < 8)
    {
        dprintf(uic_stdin_get_output_fd(),
                "Invalid number of arguments (%zu), expected args:"
                "send <minInterval>,<maxInterval>,<keepSubs>,<destID>,"
                "<epID1>,<clusterID1>,<attrID1>[<epID2>,<clusterID2>,<attrID2>,...] \n",
                arg.size());
        return SL_STATUS_FAIL;
    }

    NodeId dest;
    EndpointId epID;
    ClusterId clusterID;
    AttributeId attrID;
    SubscribeRequestParams params;
    std::vector<AttributePathParams> path;

    unsigned long long value = 0;
    value                    = stoull(arg[1]);
    if (value > 0xFFFF)
    {
        sl_log_error(LOG_TAG, "send: invalid min interval");
        return SL_STATUS_FAIL;
    }
    params.minInterval = (uint16_t)(value & 0xFFFF);
    value              = stoull(arg[2]);
    if (value > 0xFFFF)
    {
        sl_log_error(LOG_TAG, "send: invalid max interval");
        return SL_STATUS_FAIL;
    }
    params.maxInterval = (uint16_t)(value & 0xFFFF);
    value              = stoull(arg[3]);
    if (value > 0x2)
    {
        sl_log_error(LOG_TAG, "send: invalid flag for keep subscription");
        return SL_STATUS_FAIL;
    }
    params.keepSubscription = (bool) value;
    value                   = stoull(arg[4]);
    if (value < 0x1 || value > 0xFFFFFFEFFFFFFFFF) // not within Operational Node ID range
    {
        sl_log_error(LOG_TAG, "send: invalid dest nodeID");
        return SL_STATUS_FAIL;
    }
    dest = value;

    for (size_t i = 5; i < arg.size(); i += 3)
    {
        value = stoull(arg[i]);
        if (value > 0xFFFF) // not within endpoint ID range
        {
            sl_log_error(LOG_TAG, "send: invalid epID");
            return SL_STATUS_FAIL;
        }
        epID = (EndpointId)(value & 0xFFFF);

        value = stoull(arg[i + 1]);
        if (value > 0xFFFFFFFF) // not within cluster ID range
        {
            sl_log_error(LOG_TAG, "send: invalid clusterID");
            return SL_STATUS_FAIL;
        }
        clusterID = (ClusterId)(value & 0xFFFFFFFF);
        value     = stoull(arg[i + 2]);
        if (value >= 0xFFFFFFFF) // not within attribute ID range
        {
            sl_log_error(LOG_TAG, "send: invalid attrID");
            return SL_STATUS_FAIL;
        }
        attrID = (AttributeId)(value & 0xFFFFFFFF);
        path.push_back(AttributePathParams(epID, clusterID, attrID));
    }

    auto subsReq = chip::Platform::New<SubscribeRequest>(dest, path, params);
    callbacks.SetCtxt(subsReq);
    subsReq->SetCallbacks(&callbacks);
    subsReq->SendCommand();

    return SL_STATUS_OK;
}

command_map_t unify_cli_commands = {
    { "commission", { "Open commissioning window", open_commission_cli_func } },
    { "closecommission", { "Close the commissioning window", close_commission_cli_func } },
    { "read_attribute", { "Read attribute", read_attribute_cli_func } },
    { "send_move_to_lenl", { "Send an MoveToLevel command", send_move_to_level_cli_func } },
    { "subscribe", { "send subscribe request", subs_cli_func } },
};

sl_status_t mpc_cli_init()
{
    uic_stdin_add_commands(unify_cli_commands);
    return SL_STATUS_OK;
}
