/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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
#include "simple-app-helper.h"

#include "clusters/ContentLauncherCluster.h"

#include "app/clusters/bindings/BindingManager.h"
#include <inttypes.h>
#include <lib/core/CHIPCore.h>
#include <lib/shell/Commands.h>
#include <lib/shell/Engine.h>
#include <lib/shell/commands/Help.h>
#include <lib/support/CHIPArgParser.hpp>
#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>
#include <platform/CHIPDeviceLayer.h>

// VendorId of the Endpoint on the CastingPlayer that the CastingApp desires to interact with after connection
const uint16_t kDesiredEndpointVendorId = 65521;

DiscoveryDelegateImpl * DiscoveryDelegateImpl::_discoveryDelegateImpl = nullptr;

DiscoveryDelegateImpl * DiscoveryDelegateImpl::GetInstance()
{
    if (_discoveryDelegateImpl == nullptr)
    {
        _discoveryDelegateImpl = new DiscoveryDelegateImpl();
    }
    return _discoveryDelegateImpl;
}

void DiscoveryDelegateImpl::HandleOnAdded(matter::casting::memory::Strong<matter::casting::core::CastingPlayer> player)
{
    if (commissionersCount == 0)
    {
        ChipLogProgress(AppServer, "Select discovered Casting Player (start index = 0) to request commissioning");

        ChipLogProgress(AppServer, "Example: cast request 0");
    }
    ChipLogProgress(AppServer, "Discovered CastingPlayer #%d", commissionersCount);
    ++commissionersCount;
    player->LogDetail();
}

void DiscoveryDelegateImpl::HandleOnUpdated(matter::casting::memory::Strong<matter::casting::core::CastingPlayer> player)
{
    ChipLogProgress(AppServer, "Updated CastingPlayer with ID: %s", player->GetId());
}

void ConnectionHandler(CHIP_ERROR err, matter::casting::core::CastingPlayer * castingPlayer)
{
    ChipLogProgress(AppServer, "ConnectionHandler called with %" CHIP_ERROR_FORMAT, err.Format());
}

#if defined(ENABLE_CHIP_SHELL)
void RegisterCommands()
{
    static const chip::Shell::shell_command_t sDeviceComand = { &CommandHandler, "cast",
                                                                "Casting commands. Usage: cast [command_name]" };

    // Register the root `device` command with the top-level shell.
    chip::Shell::Engine::Root().RegisterCommands(&sDeviceComand, 1);
}

CHIP_ERROR CommandHandler(int argc, char ** argv)
{
    if (argc == 0 || strcmp(argv[0], "help") == 0)
    {
        return PrintAllCommands();
    }
    if (strcmp(argv[0], "discover") == 0)
    {
        ChipLogProgress(AppServer, "discover");

        return matter::casting::core::CastingPlayerDiscovery::GetInstance()->StartDiscovery(kTargetPlayerDeviceType);
    }
    if (strcmp(argv[0], "stop-discovery") == 0)
    {
        ChipLogProgress(AppServer, "stop-discovery");
        return matter::casting::core::CastingPlayerDiscovery::GetInstance()->StopDiscovery();
    }
    if (strcmp(argv[0], "request") == 0)
    {
        ChipLogProgress(AppServer, "request");
        if (argc < 2)
        {
            return PrintAllCommands();
        }
        char * eptr;
        unsigned long index = static_cast<unsigned long>(strtol(argv[1], &eptr, 10));
        std::vector<matter::casting::memory::Strong<matter::casting::core::CastingPlayer>> castingPlayers =
            matter::casting::core::CastingPlayerDiscovery::GetInstance()->GetCastingPlayers();
        VerifyOrReturnValue(0 <= index && index < castingPlayers.size(), CHIP_ERROR_INVALID_ARGUMENT,
                            ChipLogError(AppServer, "Invalid casting player index provided: %lu", index));
        std::shared_ptr<matter::casting::core::CastingPlayer> targetCastingPlayer = castingPlayers.at(index);

        matter::casting::core::EndpointFilter desiredEndpointFilter;
        desiredEndpointFilter.vendorId = kDesiredEndpointVendorId;
        targetCastingPlayer->VerifyOrEstablishConnection(ConnectionHandler, matter::casting::core::kCommissioningWindowTimeoutSec,
                                                         desiredEndpointFilter);
        return CHIP_NO_ERROR;
    }
    if (strcmp(argv[0], "print-bindings") == 0)
    {
        PrintBindings();
        return CHIP_NO_ERROR;
    }
    if (strcmp(argv[0], "print-fabrics") == 0)
    {
        PrintFabrics();
        return CHIP_NO_ERROR;
    }
    if (strcmp(argv[0], "delete-fabric") == 0)
    {
        char * eptr;
        chip::FabricIndex fabricIndex = (chip::FabricIndex) strtol(argv[1], &eptr, 10);
        chip::Server::GetInstance().GetFabricTable().Delete(fabricIndex);
        return CHIP_NO_ERROR;
    }
    return CHIP_ERROR_INVALID_ARGUMENT;
}

CHIP_ERROR PrintAllCommands()
{
    chip::Shell::streamer_t * sout = chip::Shell::streamer_get();
    streamer_printf(sout, "  help                 Usage: cast <subcommand>\r\n");
    streamer_printf(sout, "  print-bindings       Usage: cast print-bindings\r\n");
    streamer_printf(sout, "  print-fabrics        Usage: cast print-fabrics\r\n");
    streamer_printf(
        sout,
        "  delete-fabric <index>     Delete a fabric from the casting client's fabric store. Usage: cast delete-fabric 1\r\n");
    streamer_printf(sout, "  discover             Discover Casting Players. Usage: cast discover\r\n");
    streamer_printf(sout, "  stop-discovery       Stop Discovery of Casting Players. Usage: cast stop-discovery\r\n");
    streamer_printf(
        sout, "  request <index>      Request connecting to discovered Casting Player with [index]. Usage: cast request 0\r\n");
    streamer_printf(sout, "\r\n");

    return CHIP_NO_ERROR;
}

void PrintBindings()
{
    for (const auto & binding : chip::BindingTable::GetInstance())
    {
        ChipLogProgress(AppServer,
                        "Binding type=%d fab=%d nodeId=0x" ChipLogFormatX64
                        " groupId=%d local endpoint=%d remote endpoint=%d cluster=" ChipLogFormatMEI,
                        binding.type, binding.fabricIndex, ChipLogValueX64(binding.nodeId), binding.groupId, binding.local,
                        binding.remote, ChipLogValueMEI(binding.clusterId.ValueOr(0)));
    }
}

void PrintFabrics()
{
    // set fabric to be the first in the list
    for (const auto & fb : chip::Server::GetInstance().GetFabricTable())
    {
        chip::FabricIndex fabricIndex = fb.GetFabricIndex();
        ChipLogError(AppServer, "Next Fabric index=%d", fabricIndex);
        if (!fb.IsInitialized())
        {
            ChipLogError(AppServer, " -- Not initialized");
            continue;
        }
        chip::NodeId myNodeId = fb.GetNodeId();
        ChipLogProgress(AppServer,
                        "---- Current Fabric nodeId=0x" ChipLogFormatX64 " fabricId=0x" ChipLogFormatX64 " fabricIndex=%d",
                        ChipLogValueX64(myNodeId), ChipLogValueX64(fb.GetFabricId()), fabricIndex);
    }
}

#endif // ENABLE_CHIP_SHELL
