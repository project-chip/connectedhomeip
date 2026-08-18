/***************************************************************************
 * @file IMShellCommands.cpp
 * @brief Shell commands for the Interaction Model on the Silicon Labs platform.
 *******************************************************************************
 * # License
 * <b>Copyright 2025 Silicon Laboratories Inc. www.silabs.com</b>
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

#include "IMShellCommands.h"

#include "../../subscription/SubscriptionManager.h"

#include <app/server/Server.h>
#include <clusters/OccupancySensing/AttributeIds.h>
#include <clusters/OccupancySensing/ClusterId.h>
#include <clusters/OnOff/AttributeIds.h>
#include <clusters/OnOff/ClusterId.h>
#include <credentials/FabricTable.h>
#include <lib/shell/Command.h>
#include <lib/shell/Engine.h>
#include <lib/shell/commands/Help.h>
#include <lib/support/CHIPArgParser.hpp>
#include <lib/support/CodeUtils.h>

using namespace chip;
using namespace chip::app;
using chip::Silabs::SubscriptionManager;
using Shell::Engine;
using Shell::shell_command_t;
using Shell::streamer_get;
using Shell::streamer_printf;

namespace {

Engine sShellIMSubCommands;

// ----------------------------------------------------------------------------
// Shell-side subscription delegate (formats events to the shell stream)
// ----------------------------------------------------------------------------

class ShellSubscriptionDelegate : public SubscriptionManager::Delegate
{
public:
    void OnAttributeData(SubscriptionManager::Handle /*handle*/, const SubscriptionManager::Info & /*info*/,
                         const ConcreteDataAttributePath & aPath, TLV::TLVReader * apData, const StatusIB & aStatus) override
    {
        if (!aStatus.IsSuccess())
        {
            streamer_printf(streamer_get(),
                            "IM: Attribute error ep=%u cluster=0x%08" PRIX32 " attr=0x%08" PRIX32 " status=0x%02x\r\n",
                            aPath.mEndpointId, aPath.mClusterId, aPath.mAttributeId, to_underlying(aStatus.mStatus));
            return;
        }

        switch (aPath.mClusterId)
        {
        case Clusters::OnOff::Id:
            if (aPath.mAttributeId == Clusters::OnOff::Attributes::OnOff::Id)
            {
                bool value = false;
                if (apData != nullptr && apData->Get(value) == CHIP_NO_ERROR)
                {
                    streamer_printf(streamer_get(), "Attribute Report for Cluster OnOff attribute OnOff Value %s\r\n",
                                    value ? "ON" : "OFF");
                }
                else
                {
                    streamer_printf(streamer_get(), "Attribute Report for Cluster OnOff attribute OnOff Value <invalid>\r\n");
                }
            }
            else
            {
                streamer_printf(streamer_get(), "Attribute Report for Cluster OnOff attribute 0x%08" PRIX32 "\r\n",
                                aPath.mAttributeId);
            }
            break;

        case Clusters::OccupancySensing::Id:
            if (aPath.mAttributeId == Clusters::OccupancySensing::Attributes::Occupancy::Id)
            {
                uint8_t value = 0;
                if (apData != nullptr && apData->Get(value) == CHIP_NO_ERROR)
                {
                    streamer_printf(streamer_get(),
                                    "Attribute Report for Cluster OccupancySensing attribute Occupancy Value %u\r\n", value);
                }
                else
                {
                    streamer_printf(streamer_get(),
                                    "Attribute Report for Cluster OccupancySensing attribute Occupancy Value <invalid>\r\n");
                }
            }
            else
            {
                streamer_printf(streamer_get(), "Attribute Report for Cluster OccupancySensing attribute 0x%08" PRIX32 "\r\n",
                                aPath.mAttributeId);
            }
            break;

        default:
            streamer_printf(streamer_get(), "Attribute Report for Cluster 0x%08" PRIX32 " attribute 0x%08" PRIX32 "\r\n",
                            aPath.mClusterId, aPath.mAttributeId);
            break;
        }
    }

    void OnSubscriptionEstablished(SubscriptionManager::Handle /*handle*/, const SubscriptionManager::Info & /*info*/,
                                   SubscriptionId aSubscriptionId) override
    {
        streamer_printf(streamer_get(), "IM: Subscription established (id=0x%" PRIX32 ")\r\n", aSubscriptionId);
    }

    void OnError(SubscriptionManager::Handle /*handle*/, const SubscriptionManager::Info & /*info*/, CHIP_ERROR aError) override
    {
        streamer_printf(streamer_get(), "IM: Subscription error: %s\r\n", ErrorStr(aError));
    }

    void OnSubscriptionTerminated(SubscriptionManager::Handle /*handle*/,
                                  const SubscriptionManager::Info & /*info*/) override
    {
        streamer_printf(streamer_get(), "IM: Subscription terminated\r\n");
    }
};

ShellSubscriptionDelegate sShellSubscriptionDelegate;

// ----------------------------------------------------------------------------
// Shell handlers
// ----------------------------------------------------------------------------

CHIP_ERROR HelpHandler(int argc, char ** argv)
{
    sShellIMSubCommands.ForEachCommand(Shell::PrintCommandHelp, nullptr);
    return CHIP_NO_ERROR;
}

CHIP_ERROR IMCommandHandler(int argc, char ** argv)
{
    if (argc == 0)
    {
        return HelpHandler(argc, argv);
    }

    return sShellIMSubCommands.ExecCommand(argc, argv);
}

CHIP_ERROR ShowFabricHandler(int argc, char ** argv)
{
    const FabricTable & fabricTable = Server::GetInstance().GetFabricTable();
    uint8_t fabricCount             = fabricTable.FabricCount();

    streamer_printf(streamer_get(), "Fabric count: %u\r\n", fabricCount);

    for (const FabricInfo & fabric : fabricTable)
    {
        streamer_printf(streamer_get(), "  [%u] FabricId=0x" ChipLogFormatX64 " NodeId=0x" ChipLogFormatX64 "\r\n",
                        fabric.GetFabricIndex(), ChipLogValueX64(fabric.GetFabricId()), ChipLogValueX64(fabric.GetNodeId()));
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR SubscribeHandler(int argc, char ** argv)
{
    // Usage: im subscribe <fabricIndex> <nodeId> <endpointId> <clusterId> <attributeId>
    VerifyOrReturnError(
        argc == 5, CHIP_ERROR_INVALID_ARGUMENT,
        streamer_printf(streamer_get(), "Usage: im subscribe <fabricIndex> <nodeId> <endpointId> <clusterId> <attributeId>\r\n"););

    SubscriptionManager::Info info;

    VerifyOrReturnError(ArgParser::ParseInt(argv[0], info.fabricIndex), CHIP_ERROR_INVALID_ARGUMENT,
                        streamer_printf(streamer_get(), "Invalid fabricIndex\r\n"););
    VerifyOrReturnError(ArgParser::ParseInt(argv[1], info.nodeId), CHIP_ERROR_INVALID_ARGUMENT,
                        streamer_printf(streamer_get(), "Invalid nodeId\r\n"););
    VerifyOrReturnError(ArgParser::ParseInt(argv[2], info.endpointId), CHIP_ERROR_INVALID_ARGUMENT,
                        streamer_printf(streamer_get(), "Invalid endpointId\r\n"););
    VerifyOrReturnError(ArgParser::ParseInt(argv[3], info.clusterId), CHIP_ERROR_INVALID_ARGUMENT,
                        streamer_printf(streamer_get(), "Invalid clusterId\r\n"););
    VerifyOrReturnError(ArgParser::ParseInt(argv[4], info.attributeId), CHIP_ERROR_INVALID_ARGUMENT,
                        streamer_printf(streamer_get(), "Invalid attributeId\r\n"););

    SubscriptionManager::Handle handle = SubscriptionManager::kInvalidHandle;
    CHIP_ERROR err                     = SubscriptionManager::Instance().Subscribe(info, &handle);
    if (err == CHIP_ERROR_INVALID_FABRIC_INDEX)
    {
        streamer_printf(streamer_get(), "Fabric index %u not found\r\n", info.fabricIndex);
        return err;
    }
    if (err == CHIP_ERROR_NO_MEMORY)
    {
        streamer_printf(streamer_get(), "No free subscription slots (max %u) or allocation failed\r\n",
                        SubscriptionManager::kMaxSubscriptions);
        return err;
    }
    if (err != CHIP_NO_ERROR)
    {
        streamer_printf(streamer_get(), "Failed to send subscribe request: %s\r\n", ErrorStr(err));
        return err;
    }

    streamer_printf(streamer_get(),
                    "Subscribing to node 0x" ChipLogFormatX64 " on fabric %u ep=%u cluster=0x%08" PRIX32 " attr=0x%08" PRIX32
                    " (slot %u)\r\n",
                    ChipLogValueX64(info.nodeId), info.fabricIndex, info.endpointId, info.clusterId, info.attributeId, handle);

    return CHIP_NO_ERROR;
}

CHIP_ERROR SubscriptionsHandler(int argc, char ** argv)
{
    SubscriptionManager & manager = SubscriptionManager::Instance();

    streamer_printf(streamer_get(), "Active subscriptions: %u/%u\r\n", manager.ActiveCount(),
                    SubscriptionManager::kMaxSubscriptions);

    for (SubscriptionManager::Handle i = 0; i < SubscriptionManager::kMaxSubscriptions; i++)
    {
        const SubscriptionManager::Info * info = manager.GetInfo(i);
        if (info != nullptr)
        {
            streamer_printf(streamer_get(),
                            "  [%u] NodeId=0x" ChipLogFormatX64 " Cluster=0x%08" PRIX32 " Attribute=0x%08" PRIX32 "\r\n", i,
                            ChipLogValueX64(info->nodeId), info->clusterId, info->attributeId);
        }
    }

    return CHIP_NO_ERROR;
}

} // namespace

namespace IMShellCommands {

void RegisterCommands()
{
    static const Shell::Command sIMSubCommands[] = {
        { &HelpHandler, "help", "Usage: im <subcommand>" },
        { &ShowFabricHandler, "fabrics", "Show commissioned fabrics" },
        { &SubscribeHandler, "subscribe", "Subscribe to an attribute." },
        { &SubscriptionsHandler, "subscriptions", "Show active subscriptions." },
    };
    static const Shell::Command sIMCmd = { &IMCommandHandler, "im", "Interaction Model commands." };

    sShellIMSubCommands.RegisterCommands(sIMSubCommands, MATTER_ARRAY_SIZE(sIMSubCommands));
    Engine::Root().RegisterCommands(&sIMCmd, 1);

    SubscriptionManager::Instance().SetDelegate(&sShellSubscriptionDelegate);
}

} // namespace IMShellCommands
