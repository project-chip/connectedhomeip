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

#include <inttypes.h>
#include <lib/core/CHIPCore.h>
#include <lib/shell/Commands.h>
#include <lib/shell/Engine.h>
#include <lib/shell/commands/Help.h>
#include <lib/support/CHIPArgParser.hpp>
#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>

#include <app/server/Server.h>
#include <app/util/attribute-storage.h>
#include <app/util/endpoint-config-api.h>
#include <credentials/examples/DeviceAttestationCredsExample.h>
#include <data-model-providers/codegen/Instance.h>

#include <transport/Session.h>

using namespace chip;
using namespace chip::Shell;
using namespace chip::Credentials;
using namespace chip::ArgParser;
using namespace chip::Transport;

// Anonymous namespace for file-scoped, static variables.
namespace {

chip::Shell::Engine sShellServerSubcommands;
uint16_t sServerPortOperational   = CHIP_PORT;
uint16_t sServerPortCommissioning = CHIP_UDC_PORT;
bool sServerEnabled               = false;

} // namespace

static CHIP_ERROR CmdAppServerHelp(int argc, char ** argv)
{
    sShellServerSubcommands.ForEachCommand(PrintCommandHelp, nullptr);
    return CHIP_NO_ERROR;
}

static CHIP_ERROR CmdAppServerStart(int argc, char ** argv)
{
    // Init ZCL Data Model and CHIP App Server
    static chip::CommonCaseDeviceServerInitParams initParams;
    (void) initParams.InitializeStaticResourcesBeforeServerInit();
    initParams.dataModelProvider             = app::CodegenDataModelProviderInstance(initParams.persistentStorageDelegate);
    initParams.operationalServicePort        = sServerPortOperational;
    initParams.userDirectedCommissioningPort = sServerPortCommissioning;

    chip::Server::GetInstance().Init(initParams);

    // Initialize device attestation config
    SetDeviceAttestationCredentialsProvider(Examples::GetExampleDACProvider());

    sServerEnabled = true;

    return CHIP_NO_ERROR;
}

static CHIP_ERROR CmdAppServerStop(int argc, char ** argv)
{
    if (sServerEnabled == false)
        return CHIP_NO_ERROR;
    chip::Server::GetInstance().Shutdown();
    sServerEnabled = false;
    return CHIP_NO_ERROR;
}

static CHIP_ERROR CmdAppServerPort(int argc, char ** argv)
{
    if (argc == 0)
    {
        streamer_printf(streamer_get(), "%d\r\n", sServerPortOperational);
    }
    else
    {
        bool success = ParseInt(argv[0], sServerPortOperational);
        if (!success)
            return CHIP_ERROR_INVALID_ARGUMENT;
    }

    return CHIP_NO_ERROR;
}

static CHIP_ERROR CmdAppServerUdcPort(int argc, char ** argv)
{
    if (argc == 0)
    {
        streamer_printf(streamer_get(), "%d\r\n", sServerPortCommissioning);
    }
    else
    {
        bool success = ParseInt(argv[0], sServerPortCommissioning);
        if (!success)
            return CHIP_ERROR_INVALID_ARGUMENT;
    }

    return CHIP_NO_ERROR;
}

static bool PrintServerSession(void * context, SessionHandle & session)
{
    switch (session->GetSessionType())
    {
    case Session::SessionType::kSecure: {
        SecureSession * secureSession         = session->AsSecureSession();
        SecureSession::Type secureSessionType = secureSession->GetSecureSessionType();
        streamer_printf(streamer_get(),
                        "session type=SECURE %s id=0x%04x peerSessionId=0x%04x peerNodeId=0x" ChipLogFormatX64 " fabricIdx=%d\r\n",
                        secureSessionType == SecureSession::Type::kCASE ? "CASE" : "PASE", secureSession->GetLocalSessionId(),
                        secureSession->AsSecureSession()->GetPeerSessionId(), ChipLogValueX64(secureSession->GetPeerNodeId()),
                        secureSession->GetFabricIndex());
        break;
    }

    case Session::SessionType::kUnauthenticated: {
        UnauthenticatedSession * unsecuredSession = session->AsUnauthenticatedSession();
        streamer_printf(streamer_get(), "session type=UNSECURED id=0x0000 peerNodeId=0x" ChipLogFormatX64 "\r\n",
                        ChipLogValueX64(unsecuredSession->GetPeerNodeId()));
        break;
    }

    case Session::SessionType::kGroupIncoming: {
        IncomingGroupSession * groupSession = session->AsIncomingGroupSession();
        streamer_printf(streamer_get(), "session type=GROUP INCOMING id=0x%04x fabricIdx=%d\r\n", groupSession->GetGroupId(),
                        groupSession->GetFabricIndex());
        break;
    }
    case Session::SessionType::kGroupOutgoing: {
        OutgoingGroupSession * groupSession = session->AsOutgoingGroupSession();
        streamer_printf(streamer_get(), "session type=GROUP OUTGOING id=0x%04x fabricIdx=%d\r\n", groupSession->GetGroupId(),
                        groupSession->GetFabricIndex());
        break;
    }

    default:
        streamer_printf(streamer_get(), "session type=UNDEFINED\r\n");
    }
    return true;
}

static CHIP_ERROR CmdAppServerSessions(int argc, char ** argv)
{
    Server::GetInstance().GetSecureSessionManager().ForEachSessionHandle(nullptr, PrintServerSession);

    return CHIP_NO_ERROR;
}

static CHIP_ERROR CmdAppServerExchanges(int argc, char ** argv)
{
    // Messaging::ExchangeManager * exchangeMgr = &Server::GetInstance().GetExchangeManager();

    return CHIP_NO_ERROR;
}

static CHIP_ERROR CmdAppServerClusters(int argc, char ** argv)
{
    bool server = true;

    for (uint16_t i = 0; i < emberAfEndpointCount(); i++)
    {
        EndpointId endpoint = emberAfEndpointFromIndex(i);

        uint8_t clusterCount = emberAfClusterCount(endpoint, server);

        streamer_printf(streamer_get(), "Endpoint %d:\r\n", endpoint);

        for (uint8_t clusterIndex = 0; clusterIndex < clusterCount; clusterIndex++)
        {
            const EmberAfCluster * cluster = emberAfGetNthCluster(endpoint, clusterIndex, server);
            streamer_printf(streamer_get(), "  - Cluster 0x%04X\r\n", cluster->clusterId);
        }
    }

    return CHIP_NO_ERROR;
}

static CHIP_ERROR CmdAppServerEndpoints(int argc, char ** argv)
{
    for (uint16_t i = 0; i < emberAfEndpointCount(); i++)
    {
        EndpointId endpoint = emberAfEndpointFromIndex(i);

        streamer_printf(streamer_get(), "Endpoint %d\r\n", endpoint);
    }

    return CHIP_NO_ERROR;
}

static CHIP_ERROR CmdAppServer(int argc, char ** argv)
{
    switch (argc)
    {
    case 0:
        return CmdAppServerHelp(argc, argv);
    case 1:
        if ((strcmp(argv[0], "help") == 0) || (strcmp(argv[0], "-h") == 0))
        {
            return CmdAppServerHelp(argc, argv);
        }
    }
    return sShellServerSubcommands.ExecCommand(argc, argv);
}

static void CmdAppServerAtExit()
{
    CmdAppServerStop(0, nullptr);
}

void cmd_app_server_init()
{
    static const shell_command_t sServerComand = { &CmdAppServer, "server",
                                                   "Manage the ZCL application server. Usage: server [help|start|stop]" };

    static const shell_command_t sServerSubCommands[] = {
        { &CmdAppServerHelp, "help", "Usage: server <subcommand>" },
        { &CmdAppServerStart, "start", "Start the ZCL application server." },
        { &CmdAppServerStop, "stop", "Stop the ZCL application server." },
        { &CmdAppServerPort, "port", "Get/Set operational port of server." },
        { &CmdAppServerUdcPort, "udcport", "Get/Set commissioning port of server." },
        { &CmdAppServerSessions, "sessions", "Manage active sessions on the server." },
        { &CmdAppServerExchanges, "exchanges", "Manage active exchanges on the server." },
        { &CmdAppServerClusters, "clusters", "Display clusters on the server." },
        { &CmdAppServerEndpoints, "endpoints", "Display endpoints on the server." },
    };

    std::atexit(CmdAppServerAtExit);

    // Register `server` subcommands with the local shell dispatcher.
    sShellServerSubcommands.RegisterCommands(sServerSubCommands, MATTER_ARRAY_SIZE(sServerSubCommands));

    // Register the root `server` command with the top-level shell.
    Engine::Root().RegisterCommands(&sServerComand, 1);
}
