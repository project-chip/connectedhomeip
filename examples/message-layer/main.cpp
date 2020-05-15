#include <iostream>
#include <thread>
#include <chrono>

#include <signal.h>

#include <CHIPVersion.h>

#include <core/CHIPBinding.h>
#include <core/CHIPExchangeMgr.h>
#include <core/CHIPMessageLayer.h>
#include <inet/InetLayer.h>
#include <support/CHIPArgParser.hpp>
#include <support/CodeUtils.h>
#include <system/SystemLayer.h>

namespace {

using namespace chip;
using namespace chip::ArgParser;

#define kToolName "message-layer-demo"

constexpr uint16_t kToolOptListen = 'l';
constexpr uint16_t kToolOptUDPIP  = 'u';

OptionDef sToolOptionDefs[] = { //
    { "listen", kNoArgument, kToolOptListen },
    { "udp", kNoArgument, kToolOptUDPIP },
    {}
};

constexpr const char * sToolOptionHelp = //
    "  -l, --listen\n      Act as a server (i.e., listen) for packets rather than send them.\n\n"
    "  -u, --udp\n         Use UDP over IP.\n\n";

bool HandleOption(const char * aProgram, OptionSet * aOptions, int aIdentifier, const char * aName, const char * aValue);
bool HandleNonOptionArgs(const char * aProgram, int argc, char * argv[]);

struct
{
    bool Listen = false;
    bool Udp    = false;
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

void SendClientRequest(ChipExchangeManager * exchangeMgr)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    Binding * binding = exchangeMgr->NewBinding();
    VerifyOrExit(binding != NULL, err = CHIP_ERROR_NO_MEMORY);

    std::cout << "Binding allocated" << std::endl;

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

        err = configuration                                                  //
                  .Target_NodeId(kAnyNodeId)                                 //
                  .TargetAddress_IP("::1", static_cast<uint16_t>(CHIP_PORT)) //
                  .PrepareBinding();
        SuccessOrExit(err);
    }

    std::cout << "Binding configured" << std::endl;

exit:
    if (err != CHIP_NO_ERROR)
    {
        std::cout << "Sent message error: " << ErrorStr(err) << std::endl;
    }
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
    System::Layer systemLayer;
    Inet::InetLayer inetLayer;

    ChipMessageLayer::InitContext initContext;

    err = systemLayer.Init(NULL);
    SuccessOrExit(err);

    err = inetLayer.Init(systemLayer, NULL);
    SuccessOrExit(err);

    err = fabricState.Init();
    SuccessOrExit(err);

    initContext.systemLayer = &systemLayer;
    initContext.inet        = &inetLayer;
    initContext.fabricState = &fabricState;
    initContext.listenTCP   = ProgramArguments.Listen;
    initContext.listenUDP   = ProgramArguments.Listen;

    err = messageLayer.Init(&initContext);
    SuccessOrExit(err);

    err = exchangeMgr.Init(&messageLayer);
    SuccessOrExit(err);

    if (ProgramArguments.Listen)
    {
        std::cout << "Acting as a SERVER ..." << std::endl;
    }
    else
    {
        std::cout << "Acting as a CLIENT ..." << std::endl;
        SendClientRequest(&exchangeMgr);
    }

    RegisterStopHandler();
    while (!programStopped)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }

    // TODO(Andrei): RAII would be nice here
    exchangeMgr.Shutdown();
    inetLayer.Shutdown();
    systemLayer.Shutdown();

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
