/*
 *   Copyright (c) 2022 Project CHIP Authors
 *   All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */

#include "TraceDecoderArgumentParser.h"

using namespace chip::ArgParser;

namespace {

TraceDecoderArgumentParser gTraceDecoderArgumentParser;

constexpr unsigned kAppUsageLength = 64;

// Follow the code style of command line arguments in case we need to add more options in the future.
enum
{
    kProgramOption_LogFile                                  = 0x1000,
    kProgramOption_DisableProtocolSecureChannel             = 0x1001,
    kProgramOption_DisableProtocolInteractionModel          = 0x1002,
    kProgramOption_DisableProtocolBDX                       = 0x1003,
    kProgramOption_DisableProtocolUserDirectedCommissioning = 0x1004,
    kProgramOption_DisableProtocolEcho                      = 0x1005,
    kProgramOption_DisableMessageInitiator                  = 0x1006,
    kProgramOption_DisableMessageResponder                  = 0x1007,
    kProgramOption_DisableDataEncryptedPayload              = 0x1008,
};

OptionDef sProgramOptionDefs[] = {
    { "source", kArgumentRequired, kProgramOption_LogFile },
    // Protocol
    { "disable-protocol-secure-channel", kNoArgument, kProgramOption_DisableProtocolSecureChannel },
    { "disable-protocol-interaction-model", kNoArgument, kProgramOption_DisableProtocolInteractionModel },
    { "disable-protocol-bdx", kNoArgument, kProgramOption_DisableProtocolBDX },
    { "disable-protocol-user-directed-commissioning", kNoArgument, kProgramOption_DisableProtocolUserDirectedCommissioning },
    { "disable-protocol-echo", kNoArgument, kProgramOption_DisableProtocolEcho },
    // Message
    { "disable-message-initiator", kNoArgument, kProgramOption_DisableMessageInitiator },
    { "disable-message-responder", kNoArgument, kProgramOption_DisableMessageResponder },
    // Data
    { "disable-data-encrypted-payload", kNoArgument, kProgramOption_DisableDataEncryptedPayload },
    {}
};

const char sProgramOptionHelp[] = "  --source <filepath>\n"
                                  "       The log file to decode.\n"
                                  // Protocol
                                  "  --disable-protocol-secure-channel\n"
                                  "       Disable Secure Channel protocol logging\n"
                                  "  --disable-protocol-interaction-model\n"
                                  "       Disable Interaction Model protocol logging\n"
                                  "  --disable-protocol-bdx\n"
                                  "       Disable BDX protocol logging\n"
                                  "  --disable-protocol-user-directed-commissioning\n"
                                  "       Disable User Directed Commissioning protocol logging\n"
                                  "  --disable-protocol-echo\n"
                                  "       Disable Echo protocol logging\n"
                                  // Message
                                  "  --disable-message-initiator\n"
                                  "       Disable Initiator message logging\n"
                                  "  --disable-message-responder\n"
                                  "       Disable Responder message logging\n"
                                  // Data
                                  "  --disable-data-encrypted-payload\n"
                                  "       Disable encrypted payload logging\n"
                                  "\n";

bool HandleOption(const char * aProgram, OptionSet * aOptions, int aIdentifier, const char * aName, const char * aValue)
{
    bool retval = true;

    switch (aIdentifier)
    {
    // Protocol
    case kProgramOption_LogFile:
        gTraceDecoderArgumentParser.logFile = aValue;
        break;
    case kProgramOption_DisableProtocolSecureChannel:
        gTraceDecoderArgumentParser.options.mEnableProtocolSecureChannel = false;
        break;
    case kProgramOption_DisableProtocolInteractionModel:
        gTraceDecoderArgumentParser.options.mEnableProtocolInteractionModel = false;
        break;
    case kProgramOption_DisableProtocolBDX:
        gTraceDecoderArgumentParser.options.mEnableProtocolBDX = false;
        break;
    case kProgramOption_DisableProtocolUserDirectedCommissioning:
        gTraceDecoderArgumentParser.options.mEnableProtocolUserDirectedCommissioning = false;
        break;
    case kProgramOption_DisableProtocolEcho:
        gTraceDecoderArgumentParser.options.mEnableProtocolEcho = false;
        break;
    // Message
    case kProgramOption_DisableMessageInitiator:
        gTraceDecoderArgumentParser.options.mEnableMessageInitiator = false;
        break;
    case kProgramOption_DisableMessageResponder:
        gTraceDecoderArgumentParser.options.mEnableMessageResponder = false;
        break;
    // Data
    case kProgramOption_DisableDataEncryptedPayload:
        gTraceDecoderArgumentParser.options.mEnableDataEncryptedPayload = false;
        break;
    default:
        chip::ArgParser::PrintArgError("%s: INTERNAL ERROR: Unhandled option: %s\n", aProgram, aName);
        retval = false;
        break;
    }

    return (retval);
}

OptionSet sProgramOptions = { HandleOption, sProgramOptionDefs, "GENERAL OPTIONS", sProgramOptionHelp };

OptionSet * sProgramOptionSets[] = { &sProgramOptions, nullptr, nullptr };
} // namespace

TraceDecoderArgumentParser & TraceDecoderArgumentParser::GetInstance()
{
    return gTraceDecoderArgumentParser;
}

CHIP_ERROR TraceDecoderArgumentParser::ParseArguments(int argc, char * const argv[])
{
    char usage[kAppUsageLength];
    snprintf(usage, kAppUsageLength, "Usage: %s [options]", argv[0]);

    HelpOptions helpOptions(argv[0], usage, "1.0");
    sProgramOptionSets[1] = &helpOptions;

    if (!ParseArgs(argv[0], argc, argv, sProgramOptionSets))
    {
        return CHIP_ERROR_INTERNAL;
    }
    return CHIP_NO_ERROR;
}
