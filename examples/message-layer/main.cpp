#include <iostream>

#include <CHIPVersion.h>
#include <support/CHIPArgParser.hpp>

namespace {
using namespace chip::ArgParser;

#define kToolName "message-layer-demo"

constexpr uint16_t kToolOptListen = 'l';
constexpr uint16_t kToolOptUDPIP  = 'u';

OptionDef sToolOptionDefs[] = { //
    { "listen", kNoArgument, kToolOptListen },
    { "udp", kNoArgument, kToolOptUDPIP },
    { "help", kNoArgument, 'h' },
    { "version", kNoArgument, 'v' },
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

static OptionSet * sToolOptionSets[] = { &sToolOptions, NULL };

bool HandleOption(const char * aProgram, OptionSet * aOptions, int aIdentifier, const char * aName, const char * aValue)
{
    // FIXME: implement
    std::cout << aProgram << std::endl;
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

    std::cout << "HELLO WORLD" << std::endl;

    return 0;
}
