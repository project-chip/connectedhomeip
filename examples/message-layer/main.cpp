#include <iostream>
#include <thread>
#include <chrono>
#include <string>

#include <signal.h>

#include <CHIPVersion.h>

#include <core/CHIPBinding.h>
#include <core/CHIPExchangeMgr.h>
#include <core/CHIPMessageLayer.h>
#include <inet/InetLayer.h>
#include <support/CHIPArgParser.hpp>
#include <support/CodeUtils.h>
#include <system/SystemLayer.h>
#include <controller/CHIPDeviceController.h>

namespace {

using namespace chip;
using namespace chip::ArgParser;

constexpr uint32_t kProfileId  = 100;
constexpr uint8_t kMessageType = 123;

#define kToolName "message-layer-demo"

constexpr uint16_t kToolOptListen  = 'l';
constexpr uint16_t kToolOptUDPIP   = 'u';
constexpr uint16_t kToolOptNodeId  = 'n';
constexpr uint16_t kToolOptMessage = 'm';

OptionDef sToolOptionDefs[] = { //
    { "listen", kNoArgument, kToolOptListen },
    { "udp", kNoArgument, kToolOptUDPIP },
    { "node", kArgumentRequired, kToolOptNodeId },
    { "message", kArgumentRequired, kToolOptMessage },
    {}
};

constexpr const char * sToolOptionHelp = //
    "  -l, --listen\n         Act as a server (i.e., listen) for packets rather than send them.\n\n"
    "  -u, --udp\n         Use UDP over IP.\n\n"
    "  -n, --node <Id>\n           Use UDP over IP.\n\n"
    "  -m, --message <Id>\n           Message to send over the wire.\n\n";

bool HandleOption(const char * aProgram, OptionSet * aOptions, int aIdentifier, const char * aName, const char * aValue);
bool HandleNonOptionArgs(const char * aProgram, int argc, char * argv[]);

struct
{
    bool Listen         = false;
    bool Udp            = false;
    int64_t NodeId      = 1;
    std::string Message = "This is a message";
} ProgramArguments;

OptionSet sToolOptions = { //
    HandleOption,          //
    sToolOptionDefs,       //
    "GENERAL OPTIONS",     //
    sToolOptionHelp
};

HelpOptions sHelpOptions(                                                         //
    kToolName,                                                                    //
    "Usage: " kToolName " [ <options> ] [ -g <group> [ ... ] -I <interface> ]\n", //
    CHIP_VERSION_STRING "\nCopyright (c) 2020 Project CHIP Authors\nAll rights reserved.\n");

static OptionSet * sToolOptionSets[] = { &sToolOptions, &sHelpOptions, NULL };

bool HandleOption(const char * aProgram, OptionSet * aOptions, int aIdentifier, const char * aName, const char * aValue)
{
    switch (aIdentifier)
    {
    case kToolOptListen:
        ProgramArguments.Listen = true;
        break;
    case kToolOptUDPIP:
        ProgramArguments.Udp = true;
        break;
    case kToolOptNodeId:
        ProgramArguments.NodeId = atoll(aValue);
        break;
    case kToolOptMessage:
        ProgramArguments.Message = aValue;
        break;
    default:
        return false;
    }
    return true;
}

bool HandleNonOptionArgs(const char * aProgram, int argc, char * argv[])
{
    if (argc > 0)
    {
        PrintArgError("%s: unexpected argument: %s\n", aProgram, argv[0]);
        return false;
    }

    return true;
}

volatile bool programStopped = false;
void StopHandler(int s)
{
    programStopped = true;
}

void RegisterStopHandler()
{
    signal(SIGHUP, StopHandler);
    signal(SIGQUIT, StopHandler);
    signal(SIGSTOP, StopHandler);
    signal(SIGINT, StopHandler);
}
void HandleBindingEvent(void * appState, chip::Binding::EventType event, const chip::Binding::InEventParam & in,
                        chip::Binding::OutEventParam & out)
{

    if (event != chip::Binding::EventType::kEvent_BindingReady)
    {
        return;
    }
    std::cout << "Binding ready. Can sent message." << std::endl;

    CHIP_ERROR err;
    chip::ExchangeContext * exchangeContext = NULL;

    err = in.Source->NewExchangeContext(exchangeContext);
    SuccessOrExit(err);

    {
        PacketBuffer * packet = PacketBuffer::NewWithAvailableSize(ProgramArguments.Message.size());
        VerifyOrExit(packet != NULL, err = CHIP_ERROR_NO_MEMORY);

        packet->SetDataLength(ProgramArguments.Message.size());
        memcpy(packet->Start(), ProgramArguments.Message.data(), ProgramArguments.Message.size());
        err = exchangeContext->SendMessage(kProfileId, kMessageType, packet);
    }

    std::cout << "Message has been sent." << std::endl;
exit:
    if (err != CHIP_NO_ERROR)
    {
        std::cout << "Sent message error: " << ErrorStr(err) << std::endl;
    }
}

void ConnectToServer(ChipExchangeManager * exchangeMgr)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    Binding * binding = exchangeMgr->NewBinding();
    VerifyOrExit(binding != NULL, err = CHIP_ERROR_NO_MEMORY);

    binding->SetEventCallback(HandleBindingEvent);

    {

        Binding::Configuration configuration = binding->BeginConfiguration();

        if (ProgramArguments.Udp)
        {
            configuration.Transport_UDP();
        }
        else
        {
            configuration.Transport_TCP();
        }

        Inet::IPAddress address;
        VerifyOrExit(Inet::IPAddress::FromString("::1", address), err = CHIP_ERROR_INVALID_ARGUMENT);

        err = configuration                               //
                  .Target_NodeId(ProgramArguments.NodeId) //
                  .TargetAddress_IP(address, CHIP_PORT)   //
                  .PrepareBinding();
        SuccessOrExit(err);
    }

exit:
    if (err != CHIP_NO_ERROR)
    {
        std::cout << "Sent message error: " << ErrorStr(err) << std::endl;
    }
}

void ReceiveMessageHandler(ExchangeContext * ec, const IPPacketInfo * pktInfo, const ChipMessageInfo * msgInfo, uint32_t profileId,
                           uint8_t msgType, PacketBuffer * payload)
{
    std::cout << "Received a message type " << static_cast<int>(msgType) << std::endl;

    std::string data(reinterpret_cast<const char *>(payload->Start()), payload->DataLength());

    std::cout << "Payload of size " << payload->DataLength() << ": '" << data << "'" << std::endl;
}

} // namespace

int main(int argc, char * argv[])
{

    if (!ParseArgs(kToolName, argc, argv, sToolOptionSets, HandleNonOptionArgs))
    {
        std::cout << "Failed to parse arguments" << std::endl;
        return 1;
    }

    CHIP_ERROR err = CHIP_NO_ERROR;

    ChipExchangeManager exchangeMgr;
    ChipMessageLayer messageLayer;
    ChipFabricState fabricState;
    chip::DeviceController::ChipDeviceController controller;

    ChipMessageLayer::InitContext initContext;

    err = fabricState.Init();
    SuccessOrExit(err);

    err = controller.Init();
    SuccessOrExit(err);

    initContext.systemLayer = controller.SystemLayer();
    initContext.inet        = controller.InetLayer();
    initContext.fabricState = &fabricState;
    initContext.listenTCP   = ProgramArguments.Listen;
    initContext.listenUDP   = ProgramArguments.Listen || ProgramArguments.Udp;

    err = messageLayer.Init(&initContext);
    SuccessOrExit(err);

    err = exchangeMgr.Init(&messageLayer);
    SuccessOrExit(err);

    err = exchangeMgr.RegisterUnsolicitedMessageHandler(kProfileId, ReceiveMessageHandler, nullptr);
    SuccessOrExit(err);

    if (ProgramArguments.Listen)
    {
        std::cout << "Acting as a SERVER ..." << std::endl;
        std::cout << "Node ID: " << fabricState.LocalNodeId << std::endl;
    }
    else
    {
        std::cout << "Acting as a CLIENT ..." << std::endl;
        ConnectToServer(&exchangeMgr);
    }

    err = messageLayer.RefreshEndpoints();
    SuccessOrExit(err);

    RegisterStopHandler();
    while (!programStopped)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        controller.ServiceEvents();
    }

    // TODO(Andrei): RAII would be nice here
    exchangeMgr.Shutdown();
    controller.Shutdown();

exit:
    if (err != CHIP_NO_ERROR)
    {
        std::cout << "FAILED: " << ErrorStr(err) << std::endl;
    }
    else
    {
        std::cout << "DONE!" << std::endl;
    }

    return 0;
}
