#include <iostream>

#include <CHIPVersion.h>

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

    err = messageLayer.Init(&initContext);
    SuccessOrExit(err);

    err = exchangeMgr.Init(&messageLayer);
    SuccessOrExit(err);

    // FIXME: what now?

exit:
    if (err != CHIP_NO_ERROR)
    {
        std::cout << "FAILED: " << ErrorStr(err) << std::endl;
    }

    return 0;
}
