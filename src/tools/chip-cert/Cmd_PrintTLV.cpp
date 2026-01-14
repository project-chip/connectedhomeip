/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
 *    All rights reserved.
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

#include <cstring>
#include <memory>
#include <optional>
#include <utility>
#include <vector>

#include "chip-cert.h"

#include <lib/core/TLVDebug.h>
#include <lib/support/Base64.h>
#include <lib/support/BytesToHex.h>

namespace {

using namespace chip;
using namespace chip::ArgParser;
using namespace chip::Credentials;

#define CMD_NAME "chip-cert print-tlv"

bool HandleOption(const char * progName, OptionSet * optSet, int id, const char * name, const char * arg);
bool HandleNonOptionArgs(const char * progName, int argc, char * const argv[]);

// clang-format off
OptionDef gCmdOptionDefs[] =
{
    { "out",           kArgumentRequired, 'o' },
    { }
};

const char * const gCmdOptionHelp =
    "   -o, --out <file/stdout>\n"
    "\n"
    "       The output file name. If not specified or if specified as '-'\n"
    "       then output is written to stdout.\n"
    "\n"
    ;

OptionSet gCmdOptions =
{
    HandleOption,
    gCmdOptionDefs,
    "COMMAND OPTIONS",
    gCmdOptionHelp
};

HelpOptions gHelpOptions(
    CMD_NAME,
    "Usage: " CMD_NAME " [<options...>] <file/str>\n",
    CHIP_VERSION_STRING "\n" COPYRIGHT_STRING,
    "Print a Matter TLV structure in human-readable format.\n"
    "\n"
    "ARGUMENTS\n"
    "\n"
    "  <file/str>\n"
    "\n"
    "       File or string containing a Matter TLV structure. The tool will attempt\n"
    "       to auto-detect the encoding of the input string (filename, hex, or base64).\n"
    "\n"
);

OptionSet *gCmdOptionSets[] =
{
    &gCmdOptions,
    &gHelpOptions,
    nullptr
};
// clang-format on

const char * gInFileNameOrStr = nullptr;
const char * gOutFileName     = "-";
FILE * gOutFile               = nullptr;

bool HandleOption(const char * progName, OptionSet * optSet, int id, const char * name, const char * arg)
{
    switch (id)
    {
    case 'o':
        gOutFileName = arg;
        break;
    default:
        PrintArgError("%s: Unhandled option: %s\n", progName, name);
        return false;
    }

    return true;
}

bool HandleNonOptionArgs(const char * progName, int argc, char * const argv[])
{
    if (argc == 0)
    {
        PrintArgError("%s: Please specify the TLV data to be printed.\n", progName);
        return false;
    }

    if (argc > 1)
    {
        PrintArgError("%s: Unexpected argument: %s\n", progName, argv[1]);
        return false;
    }

    gInFileNameOrStr = argv[0];

    return true;
}

enum class TLVFormat
{
    kTLVFormat_Unknown = 0,
    kTLVFormat_Raw,
    kTLVFormat_Hex,
    kTLVFormat_Base64,
};

std::optional<std::vector<uint8_t>> ReadTLV(const char * fileNameOrStr)
{
    TLVFormat tlvFmt = TLVFormat::kTLVFormat_Unknown;
    size_t tlvLen    = 0;
    std::vector<uint8_t> tlvBuf;

    // If fileNameOrStr is a file name
    if (access(fileNameOrStr, R_OK) == 0)
    {
        uint32_t computedSize = 0;
        VerifyOrReturnError(ReadFileIntoMem(fileNameOrStr, nullptr, computedSize), std::nullopt);

        tlvLen = computedSize;
        tlvBuf.resize(tlvLen);

        VerifyOrReturnError(ReadFileIntoMem(fileNameOrStr, tlvBuf.data(), computedSize), std::nullopt);
        tlvFmt = TLVFormat::kTLVFormat_Raw;
    }
    // Otherwise, treat fileNameOrStr as a pointer to the TLV string (in hex or base64 encoded format)
    else
    {
        tlvLen = strlen(fileNameOrStr);
        tlvBuf.resize(tlvLen);
        memcpy(tlvBuf.data(), fileNameOrStr, tlvLen);

        // Check hex first since hex characters are a full subset of base64, and it's unlikely a
        // large base64 payload would only have the hex space in it.
        if (IsHexString(tlvBuf.data(), tlvLen))
        {
            tlvFmt = TLVFormat::kTLVFormat_Hex;
        }
        else if (IsBase64String(reinterpret_cast<const char *>(tlvBuf.data()), tlvLen))
        {
            tlvFmt = TLVFormat::kTLVFormat_Base64;
        }
        else
        {
            fprintf(stderr, "Invalid input string: neither hex nor base64 encoded!\n");
            return std::nullopt;
        }
    }

    if (tlvFmt == TLVFormat::kTLVFormat_Hex)
    {
        if (tlvLen % 2 != 0)
        {
            fprintf(stderr, "Invalid hex input string: length must be even, but got %u\n", static_cast<unsigned>(tlvLen));
            return std::nullopt;
        }
        size_t len = chip::Encoding::HexToBytes(Uint8::to_char(tlvBuf.data()), tlvLen, tlvBuf.data(), tlvLen);
        VerifyOrReturnError(CanCastTo<uint32_t>(2 * len), std::nullopt);
        VerifyOrReturnError(2 * len == tlvLen, std::nullopt);
        tlvLen = len;
    }
    else if (tlvFmt == TLVFormat::kTLVFormat_Base64)
    {
        VerifyOrReturnError(CanCastTo<uint16_t>(tlvLen), std::nullopt);
        uint16_t decodedLen = chip::Base64Decode(Uint8::to_char(tlvBuf.data()), static_cast<uint16_t>(tlvLen), tlvBuf.data());
        if (decodedLen == UINT16_MAX)
        {
            fprintf(stderr, "Invalid Base64 input string\n");
            return std::nullopt;
        }
        tlvLen = decodedLen;
    }

    tlvBuf.resize(tlvLen);

    return tlvBuf;
}

void ENFORCE_FORMAT(1, 2) SimpleDumpWriter(const char * aFormat, ...)
{
    va_list args;

    va_start(args, aFormat);

    vfprintf(gOutFile, aFormat, args);

    va_end(args);
}

bool PrintTLV(ByteSpan tlv)
{
    chip::TLV::TLVReader reader;

    VerifyOrReturnError(!tlv.empty(), false);

    reader.Init(tlv);
    reader.ImplicitProfileId = 0;

    CHIP_ERROR err = chip::TLV::Debug::Dump(reader, SimpleDumpWriter);

    if ((err != CHIP_NO_ERROR) && (err != CHIP_ERROR_END_OF_TLV))
    {
        fprintf(stderr, "Error during parsing: %" CHIP_ERROR_FORMAT "\n", err.Format());
        return false;
    }

    return true;
}

} // namespace

bool Cmd_PrintTLV(int argc, char * argv[])
{
    if (argc == 1)
    {
        gHelpOptions.PrintBriefUsage(stderr);
        return true;
    }

    VerifyOrReturnError(ParseArgs(CMD_NAME, argc, argv, gCmdOptionSets, HandleNonOptionArgs), false);

    std::optional<std::vector<uint8_t>> tlv = ReadTLV(gInFileNameOrStr);
    VerifyOrReturnError(tlv.has_value(), false);

    bool isSuccess = false;
    VerifyOrReturnError(OpenFile(gOutFileName, gOutFile, true), false);
    isSuccess = PrintTLV(ByteSpan{ tlv->data(), tlv->size() });
    CloseFile(gOutFile);

    return isSuccess;
}
