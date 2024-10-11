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
 * @file Contains shell commands for a commissionee (eg. end device) related to commissioning.
 */

#include <CommissioneeShellCommands.h>
#include <app/server/Dnssd.h>
#include <app/server/Server.h>
#include <inttypes.h>
#include <lib/core/CHIPCore.h>
#include <lib/shell/Commands.h>
#include <lib/shell/Engine.h>
#include <lib/shell/commands/Help.h>
#include <lib/support/CHIPArgParser.hpp>
#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>
#include <platform/CHIPDeviceLayer.h>
#include <protocols/user_directed_commissioning/UserDirectedCommissioning.h>

namespace chip {
namespace Shell {

#if CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY_CLIENT
static CHIP_ERROR SendUDC(bool printHeader, chip::Transport::PeerAddress commissioner,
                          Protocols::UserDirectedCommissioning::IdentificationDeclaration id)
{
    streamer_t * sout = streamer_get();

    if (printHeader)
    {
        streamer_printf(sout, "SendUDC:        ");
    }

    Server::GetInstance().SendUserDirectedCommissioningRequest(commissioner, id);

    streamer_printf(sout, "done\r\n");

    return CHIP_NO_ERROR;
}
#endif // CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY_CLIENT

static CHIP_ERROR PrintAllCommands()
{
    streamer_t * sout = streamer_get();
    streamer_printf(sout, "  help                       Usage: commissionee <subcommand>\r\n");
#if CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY_CLIENT
    streamer_printf(sout, "  sendudc <address> <port>   Send UDC message to address. Usage: commissionee sendudc ::1 5543\r\n");
    streamer_printf(sout, "  udccancel <address> <port> Send UDC cancel message to address. Usage: cast udccancel ::1 5543\r\n");
    streamer_printf(sout,
                    "  udccommissionerpasscode <address> <port> [CommissionerPasscodeReady] [PairingHint] [PairingInst] Send UDC "
                    "commissioner passcode message to address. Usage: udccommissionerpasscode ::1 5543 t 5 HelloWorld\r\n");
    streamer_printf(sout,
                    "  testudc <address> <port> [NoPasscode] [CdUponPasscodeDialog] [vid] [PairingHint] [PairingInst] Send UDC "
                    "message to address. Usage: cast testudc ::1 5543 t t 5 HelloWorld\r\n");
#endif // CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY_CLIENT
    // TODO: Figure out whether setdiscoverytimeout is a reasonable thing to do
    // at all, and if so what semantics it should have.  Presumably it should
    // affect BLE discovery too, not just DNS-SD.  How should it interact with
    // explicit timeouts specified in OpenCommissioningWindow?
#if 0
    streamer_printf(
        sout, "  setdiscoverytimeout <timeout>   Set discovery timeout in seconds. Usage: commissionee setdiscoverytimeout 30\r\n");
#endif
#if CHIP_DEVICE_CONFIG_ENABLE_EXTENDED_DISCOVERY
    streamer_printf(sout,
                    "  setextendeddiscoverytimeout <timeout>   Set extendeddiscovery timeout in seconds. Usage: commissionee "
                    "setextendeddiscoverytimeout 30\r\n");
#endif // CHIP_DEVICE_CONFIG_ENABLE_EXTENDED_DISCOVERY
    streamer_printf(sout,
                    "  restartmdns <commissioningMode> (disabled|enabled_basic|enabled_enhanced)   Start Mdns with given "
                    "settings. Usage: commissionee "
                    "restartmdns enabled_basic\r\n");
    streamer_printf(sout, "  startbcm                   Start basic commissioning mode. Usage: commissionee startbcm\r\n");
    streamer_printf(sout, "\r\n");

    return CHIP_NO_ERROR;
}

static CHIP_ERROR CommissioneeHandler(int argc, char ** argv)
{
    if (argc == 0 || strcmp(argv[0], "help") == 0)
    {
        return PrintAllCommands();
    }
#if CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY_CLIENT
    if (strcmp(argv[0], "sendudc") == 0)
    {
        Protocols::UserDirectedCommissioning::IdentificationDeclaration id;

        char * eptr;
        chip::Inet::IPAddress commissioner;
        chip::Inet::IPAddress::FromString(argv[1], commissioner);
        uint16_t port = (uint16_t) strtol(argv[2], &eptr, 10);

        return SendUDC(true, chip::Transport::PeerAddress::UDP(commissioner, port), id);
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
        return Server::GetInstance().SendUserDirectedCommissioningRequest(chip::Transport::PeerAddress::UDP(commissioner, port),
                                                                          id);
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
    // TODO: Figure out whether setdiscoverytimeout is a reasonable thing to do
    // at all, and if so what semantics it should have.  Presumably it should
    // affect BLE discovery too, not just DNS-SD.  How should it interact with
    // explicit timeouts specified in OpenCommissioningWindow?
#if 0
    if (strcmp(argv[0], "setdiscoverytimeout") == 0)
    {
        char * eptr;
        int16_t timeout = (int16_t) strtol(argv[1], &eptr, 10);
        chip::app::DnssdServer::Instance().SetDiscoveryTimeoutSecs(timeout);
        return CHIP_NO_ERROR;
    }
#endif
#if CHIP_DEVICE_CONFIG_ENABLE_EXTENDED_DISCOVERY
    if (strcmp(argv[0], "setextendeddiscoverytimeout") == 0)
    {
        char * eptr;
        int16_t timeout = (int16_t) strtol(argv[1], &eptr, 10);
        chip::app::DnssdServer::Instance().SetExtendedDiscoveryTimeoutSecs(timeout);
        return CHIP_NO_ERROR;
    }
#endif // CHIP_DEVICE_CONFIG_ENABLE_EXTENDED_DISCOVERY
    if (strcmp(argv[0], "restartmdns") == 0)
    {
        if (argc < 2)
        {
            return PrintAllCommands();
        }
        if (strcmp(argv[1], "disabled") == 0)
        {
            chip::app::DnssdServer::Instance().StartServer(chip::Dnssd::CommissioningMode::kDisabled);
            return CHIP_NO_ERROR;
        }
        if (strcmp(argv[1], "enabled_basic") == 0)
        {
            chip::app::DnssdServer::Instance().StartServer(chip::Dnssd::CommissioningMode::kEnabledBasic);
            return CHIP_NO_ERROR;
        }
        if (strcmp(argv[1], "enabled_enhanced") == 0)
        {
            chip::app::DnssdServer::Instance().StartServer(chip::Dnssd::CommissioningMode::kEnabledEnhanced);
            return CHIP_NO_ERROR;
        }
        return PrintAllCommands();
    }
    if (strcmp(argv[0], "startbcm") == 0)
    {
        Server::GetInstance().GetCommissioningWindowManager().OpenBasicCommissioningWindow();
        return CHIP_NO_ERROR;
    }
    return CHIP_ERROR_INVALID_ARGUMENT;
}

void RegisterCommissioneeCommands()
{

    static const shell_command_t sDeviceComand = { &CommissioneeHandler, "commissionee",
                                                   "Commissionee commands. Usage: commissionee [command_name]" };

    // Register the root `device` command with the top-level shell.
    Engine::Root().RegisterCommands(&sDeviceComand, 1);
}

} // namespace Shell
} // namespace chip
