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
#include "JsonTLV.h"
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
                        binding.remote, ChipLogValueMEI(binding.clusterId.ValueOr(0)));
    }
}

class MockCommandSenderCallback : public chip::app::CommandSender::Callback
{
public:
    void OnResponse(chip::app::CommandSender * apCommandSender, const chip::app::ConcreteCommandPath & aPath,
                    const chip::app::StatusIB & aStatus, chip::TLV::TLVReader * aData) override
    {
        ChipLogDetail(Controller, "OnResponse Cluster Command: Cluster=%" PRIx32 " Command=%" PRIx32 " Endpoint=%x",
                      aPath.mClusterId, aPath.mCommandId, aPath.mEndpointId);

        uint8_t out_buf[1024];
        Encoding::BufferWriter buf_writer(out_buf, sizeof(out_buf));

        TLV::Debug::Dump(*aData, TLV::Json::TLVDumpWriter);
        TLV::Json::WriteJSON(*aData, buf_writer);

        ChipLogDetail(Controller, "OnResponse %s", out_buf);

        ChipLogDetail(Controller, "OnResponse DumpDone");
    }
    void OnError(const chip::app::CommandSender * apCommandSender, CHIP_ERROR aError) override
    {
        ChipLogError(Controller, "OnError happens with %" CHIP_ERROR_FORMAT, aError.Format());
    }
    void OnDone(chip::app::CommandSender * apCommandSender) override
    {
        ChipLogDetail(Controller, "OnDone CommandSender");
        if (apCommandSender != nullptr)
        {
            chip::Platform::Delete(apCommandSender);
            ChipLogDetail(Controller, "Cleaned up");
        }
    }

} mockCommandSenderDelegate;

CHIP_ERROR SendClusterCommand(chip::NodeId node, chip::EndpointId endpointId, chip::ClusterId clusterId, chip::CommandId commandId,
                              uint16_t timedRequestTimeoutMs, uint16_t imTimeoutMs, char * json)
{
    chip::TLV::TLVWriter writer;
    chip::TLV::TLVReader reader;

    uint8_t buf[74];
    uint8_t out_buf[1024];

    Encoding::BufferWriter buf_writer(out_buf, sizeof(out_buf));

    writer.Init(buf);
    writer.ImplicitProfileId = chip::TLV::kCommonProfileId;

    JsonTLV customJson;
    ReturnErrorOnFailure(customJson.Parse("SendClusterCommand", json));

    ReturnErrorOnFailure(customJson.Encode(writer, chip::TLV::CommonTag(77)));

    reader.Init(buf, writer.GetLengthWritten());
    reader.ImplicitProfileId = chip::TLV::kCommonProfileId;
    reader.Next();

    CastingServer::GetInstance()->SendCommand(mockCommandSenderDelegate, endpointId, clusterId, commandId, reader,
                                              timedRequestTimeoutMs, imTimeoutMs);
    return CHIP_NO_ERROR;
}

CHIP_ERROR RunJsonTest(const char * json, const char * testName)
{
    chip::TLV::TLVWriter writer;
    chip::TLV::TLVReader reader;

    uint8_t buf[74];
    uint8_t out_buf[1024];
    Encoding::BufferWriter buf_writer(out_buf, sizeof(out_buf));

    writer.Init(buf);
    writer.ImplicitProfileId = chip::TLV::kCommonProfileId;

    JsonTLV customJson;
    ReturnLogErrorOnFailure(customJson.Parse(testName, json));
    ReturnLogErrorOnFailure(customJson.Encode(writer, chip::TLV::CommonTag(77)));

    ChipLogProgress(AppServer, "--------Start Raw Bytes");
    for (uint32_t i = 0; i < writer.GetLengthWritten(); i++)
    {
        if (i != 0 && i % 16 == 0)
            printf("\n");
        printf("0x%02X, ", buf[i]);
    }
    printf("\n");
    ChipLogProgress(AppServer, "--------End Raw Bytes");

    reader.Init(buf, writer.GetLengthWritten());
    reader.ImplicitProfileId = chip::TLV::kCommonProfileId;
    reader.Next();

    TLV::Debug::Dump(reader, TLV::Json::TLVDumpWriter);

    ChipLogProgress(AppServer, "%s", json);
    TLV::Json::WriteJSON(reader, buf_writer);

    ChipLogProgress(AppServer, "%s", out_buf);

    Json::Value inputValue;
    VerifyOrReturnLogError(JsonParser2::ParseCustomArgument(testName, json, inputValue), CHIP_ERROR_INVALID_ARGUMENT);

    Json::Value outputValue;
    VerifyOrReturnLogError(JsonParser2::ParseCustomArgument(testName, (char *) out_buf, outputValue), CHIP_ERROR_INVALID_ARGUMENT);

    VerifyOrReturnLogError(inputValue == outputValue, CHIP_ERROR_DECODE_FAILED);

    return CHIP_NO_ERROR;
}

CHIP_ERROR RunJsonTests()
{
    char json[] = "{"
                  "\"0x0\" : { \"tlvType\":\"0x00\", \"value\": 7 },"
                  "\"0x1\" : { \"tlvType\":\"0x0c\", \"value\": \"foo\" },"
                  "\"0x2\" : {"
                  "\"0x0\" : { \"tlvType\":\"0x0a\", \"value\": 8.000000 }"
                  "},"
                  "\"0x3\" : ["
                  "{ \"tlvType\":\"0x0c\", \"value\": \"bar\" },"
                  "{ \"tlvType\":\"0x0c\", \"value\": \"bat\" },"
                  "{"
                  "\"0x0\" : { \"tlvType\":\"0x04\", \"value\": 7 },"
                  "\"0x1\" : { \"tlvType\":\"0x04\", \"value\": 8 }"
                  "},"
                  "{ \"tlvType\":\"0x08\", \"value\": true },"
                  "{ \"tlvType\":\"0x04\", \"value\": 5463 }"
                  "],"
                  "\"0x4\" : { \"tlvType\":\"0x04\", \"value\": 3 }"
                  "}";
    ReturnErrorOnFailure(RunJsonTest(json, "test 1"));

    ReturnErrorOnFailure(RunJsonTest(
        "{\"0x0\" : { \"tlvType\":\"0x00\", \"value\": 7 },\"0x1\" : { \"tlvType\":\"0x0c\", \"value\": \"foo\" },\"0x2\" "
        ": {\"0x0\" : { \"tlvType\":\"0x0a\", \"value\": 8.000000 }}}",
        "test 2"));

    ReturnErrorOnFailure(RunJsonTest("{\"0x0\" : { \"tlvType\":\"0x04\", \"value\": 1 }}", "test 3"));

    return CHIP_NO_ERROR;
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
    if (strcmp(argv[0], "invoke") == 0)
    {
        ChipLogProgress(DeviceLayer, "invoke");
        if (argc < 8)
        {
            return PrintAllCommands();
        }

        // Keypad Input: send key
        // cast invoke 0 1 1289 0 0 5000 {"0x0":{"tlvType":"0x04","value":7}}

        // MediaPlayback: play
        // cast invoke 0 1 1286 0 0 5000 {}

        char * eptr;
        chip::NodeId node              = (chip::NodeId) strtoull(argv[1], &eptr, 0);
        chip::EndpointId endpointId    = (chip::EndpointId) strtoull(argv[2], &eptr, 0);
        chip::ClusterId clusterId      = (chip::ClusterId) strtoull(argv[3], &eptr, 0);
        chip::CommandId commandId      = (chip::CommandId) strtoull(argv[4], &eptr, 0);
        uint16_t timedRequestTimeoutMs = (uint16_t) strtoull(argv[5], &eptr, 0);
        uint16_t imTimeoutMs           = (uint16_t) strtoull(argv[6], &eptr, 0);
        char * json                    = argv[7];

        ChipLogProgress(DeviceLayer, "invoke json=%s", json);

        SendClusterCommand(node, endpointId, clusterId, commandId, timedRequestTimeoutMs, imTimeoutMs, json);
        return CHIP_NO_ERROR;
    }
    if (strcmp(argv[0], "run-json-tests") == 0)
    {
        ChipLogProgress(DeviceLayer, "run-json-tests");
        return RunJsonTests();
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
