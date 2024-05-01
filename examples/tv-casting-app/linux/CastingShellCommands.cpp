/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

/**
 * @file Contains shell commands for a ContentApp relating to Content App platform of the Video Player.
 */

#include "CastingShellCommands.h"

#include "CastingServer.h"
#include "CastingUtils.h"
#include "app/clusters/bindings/BindingManager.h"
#include <CommissionableInit.h>
#include <inttypes.h>
#include <lib/core/CHIPCore.h>
#include <lib/shell/Commands.h>
#include <lib/shell/Engine.h>
#include <lib/shell/commands/Help.h>
#include <lib/support/CHIPArgParser.hpp>
#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>
#include <platform/CHIPDeviceLayer.h>

using namespace chip;
using namespace chip::DeviceLayer;

namespace {
LinuxCommissionableDataProvider gCommissionableDataProvider;
}

namespace chip {
namespace Shell {

static CHIP_ERROR PrintAllCommands()
{
    streamer_t * sout = streamer_get();
    streamer_printf(sout, "  help                 Usage: cast <subcommand>\r\n");
    streamer_printf(sout, "  print-bindings       Usage: cast print-bindings\r\n");
    streamer_printf(sout, "  print-fabrics        Usage: cast print-fabrics\r\n");
    streamer_printf(
        sout,
        "  delete-fabric <index>     Delete a fabric from the casting client's fabric store. Usage: cast delete-fabric 1\r\n");
    streamer_printf(
        sout,
        "  set-fabric <index>        Set current fabric from the casting client's fabric store. Usage: cast set-fabric 1\r\n");
    streamer_printf(sout,
                    "  init <nodeid> <fabric-index>  Initialize casting app using given nodeid and index from previous "
                    "commissioning. Usage: init 18446744004990074879 2\r\n");
    streamer_printf(sout, "  discover             Discover commissioners. Usage: cast discover\r\n");
    streamer_printf(
        sout, "  request <index>      Request commissioning from discovered commissioner with [index]. Usage: cast request 0\r\n");
    streamer_printf(sout, "  launch <url> <display>   Launch content. Usage: cast launch https://www.yahoo.com Hello\r\n");
    streamer_printf(
        sout,
        "  access <node>        Read and display clusters on each endpoint for <node>. Usage: cast access 0xFFFFFFEFFFFFFFFF\r\n");
    streamer_printf(sout, "  sendudc <address> <port> Send UDC message to address. Usage: cast sendudc ::1 5543\r\n");
    streamer_printf(sout, "  udccancel <address> <port> Send UDC cancel message to address. Usage: cast udccancel ::1 5543\r\n");
    streamer_printf(sout,
                    "  udccommissionerpasscode <address> <port> [CommissionerPasscodeReady] [PairingHint] [PairingInst] Send UDC "
                    "commissioner passcode message to address. Usage: udccommissionerpasscode ::1 5543 t 5 HelloWorld\r\n");
    streamer_printf(sout,
                    "  testudc <address> <port> [NoPasscode] [CdUponPasscodeDialog] [vid] [PairingHint] [PairingInst] Send UDC "
                    "message to address. Usage: cast testudc ::1 5543 t t 5 HelloWorld\r\n");
    streamer_printf(
        sout,
        "  cluster [clustercommand] Send cluster command. Usage: cast cluster keypadinput send-key 1 18446744004990074879 1\r\n");
    streamer_printf(sout, "\r\n");

    return CHIP_NO_ERROR;
}

void PrintBindings()
{
    for (const auto & binding : BindingTable::GetInstance())
    {
        ChipLogProgress(NotSpecified,
                        "Binding type=%d fab=%d nodeId=0x" ChipLogFormatX64
                        " groupId=%d local endpoint=%d remote endpoint=%d cluster=" ChipLogFormatMEI,
                        binding.type, binding.fabricIndex, ChipLogValueX64(binding.nodeId), binding.groupId, binding.local,
                        binding.remote, ChipLogValueMEI(binding.clusterId.value_or(0)));
    }
}

static CHIP_ERROR CastingHandler(int argc, char ** argv)
{
    if (argc == 0 || strcmp(argv[0], "help") == 0)
    {
        return PrintAllCommands();
    }
    if (strcmp(argv[0], "init") == 0)
    {
        ChipLogProgress(DeviceLayer, "init");

        char * eptr;
        chip::NodeId nodeId           = (chip::NodeId) strtoull(argv[1], &eptr, 10);
        chip::FabricIndex fabricIndex = (chip::FabricIndex) strtol(argv[2], &eptr, 10);
        return CastingServer::GetInstance()->TargetVideoPlayerInfoInit(nodeId, fabricIndex, OnConnectionSuccess,
                                                                       OnConnectionFailure, OnNewOrUpdatedEndpoint);
    }
    if (strcmp(argv[0], "discover") == 0)
    {
        ChipLogProgress(DeviceLayer, "discover");

        return DiscoverCommissioners();
    }
    if (strcmp(argv[0], "request") == 0)
    {
        ChipLogProgress(DeviceLayer, "request");
        if (argc < 2)
        {
            return PrintAllCommands();
        }
        char * eptr;
        int index = (int) strtol(argv[1], &eptr, 10);
        return RequestCommissioning(index);
    }
    if (strcmp(argv[0], "setusecommissionerpasscode") == 0)
    {
        ChipLogProgress(DeviceLayer, "setusecommissionerpasscode");
        if (argc < 2)
        {
            return PrintAllCommands();
        }
        char * eptr;
        int useCP = (int) strtol(argv[1], &eptr, 10);
        CastingServer::GetInstance()->SetCommissionerPasscodeEnabled(useCP == 1);
    }
    if (strcmp(argv[0], "launch") == 0)
    {
        ChipLogProgress(DeviceLayer, "launch");
        if (argc < 3)
        {
            return PrintAllCommands();
        }
        char * url     = argv[1];
        char * display = argv[2];
        return CastingServer::GetInstance()->ContentLauncherLaunchURL(
            CastingServer::GetInstance()->GetActiveTargetVideoPlayer()->GetEndpoint(4), url, display, LaunchURLResponseCallback);
    }
    if (strcmp(argv[0], "access") == 0)
    {
        ChipLogProgress(DeviceLayer, "access");
        if (argc < 2)
        {
            return PrintAllCommands();
        }
        char * eptr;
        chip::NodeId node = (chip::NodeId) strtoull(argv[1], &eptr, 0);
        CastingServer::GetInstance()->ReadServerClustersForNode(node);
        return CHIP_NO_ERROR;
    }
#if CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY_CLIENT
    if (strcmp(argv[0], "sendudc") == 0)
    {
        char * eptr;
        chip::Inet::IPAddress commissioner;
        chip::Inet::IPAddress::FromString(argv[1], commissioner);
        uint16_t port = (uint16_t) strtol(argv[2], &eptr, 10);
        PrepareForCommissioning();
        return CastingServer::GetInstance()->SendUserDirectedCommissioningRequest(
            chip::Transport::PeerAddress::UDP(commissioner, port));
    }
    if (strcmp(argv[0], "udccancel") == 0)
    {
        char * eptr;
        chip::Inet::IPAddress commissioner;
        chip::Inet::IPAddress::FromString(argv[1], commissioner);
        uint16_t port = (uint16_t) strtol(argv[2], &eptr, 10);

        Protocols::UserDirectedCommissioning::IdentificationDeclaration id;
        id.SetCancelPasscode(true);
        return Server::GetInstance().SendUserDirectedCommissioningRequest(chip::Transport::PeerAddress::UDP(commissioner, port),
                                                                          id);
    }
    if (strcmp(argv[0], "udccommissionerpasscode") == 0)
    {
        char * eptr;
        chip::Inet::IPAddress commissioner;
        chip::Inet::IPAddress::FromString(argv[1], commissioner);
        uint16_t port = (uint16_t) strtol(argv[2], &eptr, 10);

        // udccommissionerpasscode <address> <port> [CommissionerPasscodeReady] [PairingHint] [PairingInst]
        // ex. udccommissionerpasscode <address> <port> t 5 'hello world'

        Protocols::UserDirectedCommissioning::IdentificationDeclaration id;
        id.SetCommissionerPasscode(true);
        id.SetVendorId(1244); // set non-standard vid-pid to prevent dummy content apps from returning a passcode
        id.SetProductId(2234);
        if (argc > 3)
        {
            id.SetCommissionerPasscodeReady(strcmp(argv[3], "t") == 0);
        }
        if (argc > 4)
        {
            uint16_t hint = (uint16_t) strtol(argv[4], &eptr, 10);
            id.SetPairingHint(hint);
        }
        if (argc > 5)
        {
            id.SetPairingInst(argv[5]);
        }
        if (argc > 6)
        {
            uint16_t vid = (uint16_t) strtol(argv[6], &eptr, 10);
            Protocols::UserDirectedCommissioning::TargetAppInfo info;
            info.vendorId = vid;
            id.AddTargetAppInfo(info);
        }
        return Server::GetInstance().SendUserDirectedCommissioningRequest(chip::Transport::PeerAddress::UDP(commissioner, port),
                                                                          id);
    }
    if (strcmp(argv[0], "setcommissionerpasscode") == 0)
    {

        char * eptr;
        uint32_t passcode                                      = (uint32_t) strtol(argv[1], &eptr, 10);
        LinuxDeviceOptions::GetInstance().payload.setUpPINCode = passcode;

        VerifyOrDie(chip::examples::InitCommissionableDataProvider(gCommissionableDataProvider,
                                                                   LinuxDeviceOptions::GetInstance()) == CHIP_NO_ERROR);

        DeviceLayer::SetCommissionableDataProvider(&gCommissionableDataProvider);

        CastingServer::GetInstance()->SetCommissionerPasscodeReady();
    }
    if (strcmp(argv[0], "testudc") == 0)
    {
        char * eptr;
        chip::Inet::IPAddress commissioner;
        chip::Inet::IPAddress::FromString(argv[1], commissioner);
        uint16_t port = (uint16_t) strtol(argv[2], &eptr, 10);

        // sendudc <address> <port> [NoPasscode] [CdUponPasscodeDialog] [vid] [PairingHint] [PairingInst]
        // ex. sendudc <address> <port> t t 111 5 'hello world'

        Protocols::UserDirectedCommissioning::IdentificationDeclaration id;
        if (argc > 3)
        {
            id.SetNoPasscode(strcmp(argv[3], "t") == 0);
        }
        if (argc > 4)
        {
            id.SetCdUponPasscodeDialog(strcmp(argv[4], "t") == 0);
        }
        if (argc > 5)
        {
            uint16_t vid = (uint16_t) strtol(argv[5], &eptr, 10);
            Protocols::UserDirectedCommissioning::TargetAppInfo info;
            info.vendorId = vid;
            id.AddTargetAppInfo(info);
        }
        if (argc > 6)
        {
            uint16_t hint = (uint16_t) strtol(argv[6], &eptr, 10);
            id.SetPairingHint(hint);
        }
        if (argc > 7)
        {
            id.SetPairingInst(argv[7]);
        }
        return Server::GetInstance().SendUserDirectedCommissioningRequest(chip::Transport::PeerAddress::UDP(commissioner, port),
                                                                          id);
    }
#endif // CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY_CLIENT
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
    if (strcmp(argv[0], "set-fabric") == 0)
    {
        char * eptr;
        chip::FabricIndex fabricIndex = (chip::FabricIndex) strtol(argv[1], &eptr, 10);
        chip::NodeId nodeId           = CastingServer::GetInstance()->GetVideoPlayerNodeForFabricIndex(fabricIndex);
        if (nodeId == kUndefinedFabricIndex)
        {
            streamer_printf(streamer_get(), "ERROR - invalid fabric or video player nodeId not found\r\n");
            return CHIP_ERROR_INVALID_ARGUMENT;
        }
        return CastingServer::GetInstance()->TargetVideoPlayerInfoInit(nodeId, fabricIndex, OnConnectionSuccess,
                                                                       OnConnectionFailure, OnNewOrUpdatedEndpoint);
    }
    if (strcmp(argv[0], "cluster") == 0)
    {
        return ProcessClusterCommand(argc, argv);
    }
    return CHIP_ERROR_INVALID_ARGUMENT;
}

void RegisterCastingCommands()
{

    static const shell_command_t sDeviceComand = { &CastingHandler, "cast", "Casting commands. Usage: cast [command_name]" };

    // Register the root `device` command with the top-level shell.
    Engine::Root().RegisterCommands(&sDeviceComand, 1);
}

} // namespace Shell
} // namespace chip
