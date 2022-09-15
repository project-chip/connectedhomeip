/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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

/**
 *    @file
 *      This file implements the command handler for the 'chip-cert' tool
 *      that prints the contents of a CHIP certificate.
 *
 */

#include "chip-cert.h"

#include <credentials/CertificationDeclaration.h>
#include <lib/core/CHIPTLVDebug.hpp>
#include <lib/support/BytesToHex.h>

namespace {

using namespace chip;
using namespace chip::ArgParser;
using namespace chip::Credentials;

#define CMD_NAME "chip-cert print-cd"

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
    "       The output printed CD content file name. If not specified\n"
    "       or if specified '-' then output is written to stdout.\n"
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
    "Print a CHIP certification declaration (CD) content.\n"
    "\n"
    "ARGUMENTS\n"
    "\n"
    "  <file/str>\n"
    "\n"
    "       File or string containing a CHIP CMS Signed CD message.\n"
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
        PrintArgError("%s: Please specify the CD to be printed.\n", progName);
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

enum CDFormat
{
    kCDFormat_Unknown = 0,
    kCDFormat_Raw,
    kCDFormat_Hex,
    kCDFormat_Base64,
};

CDFormat DetectCDFormat(const uint8_t * cd, uint32_t cdLen)
{
    static const uint8_t cdRawPrefix1[] = { 0x30, 0x81 };
    static const uint8_t cdRawPrefix2[] = { 0x30, 0x82 };
    static const char * cdHexPrefix     = "308";
    static const char * cdB64Prefix     = "MI";

    VerifyOrReturnError(cd != nullptr, kCDFormat_Unknown);

    if ((cdLen > sizeof(cdRawPrefix1)) &&
        (memcmp(cd, cdRawPrefix1, sizeof(cdRawPrefix1)) == 0 || memcmp(cd, cdRawPrefix2, sizeof(cdRawPrefix2)) == 0))
    {
        return kCDFormat_Raw;
    }

    if ((cdLen > strlen(cdHexPrefix)) && (memcmp(cd, cdHexPrefix, strlen(cdHexPrefix)) == 0))
    {
        return kCDFormat_Hex;
    }

    if ((cdLen > strlen(cdB64Prefix)) && (memcmp(cd, cdB64Prefix, strlen(cdB64Prefix)) == 0))
    {
        return kCDFormat_Base64;
    }

    return kCDFormat_Unknown;
}

bool ReadCD(const char * fileNameOrStr, MutableByteSpan cd)
{
    CDFormat cdFmt = kCDFormat_Unknown;
    uint32_t cdLen = 0;
    std::unique_ptr<uint8_t[]> cdBuf;

    // If fileNameOrStr is a file name
    if (access(fileNameOrStr, R_OK) == 0)
    {
        VerifyOrReturnError(ReadFileIntoMem(fileNameOrStr, nullptr, cdLen), false);

        cdBuf = std::unique_ptr<uint8_t[]>(new uint8_t[cdLen]);

        VerifyOrReturnError(ReadFileIntoMem(fileNameOrStr, cdBuf.get(), cdLen), false);

        cdFmt = DetectCDFormat(cdBuf.get(), cdLen);
        if (cdFmt == kCDFormat_Unknown)
        {
            fprintf(stderr, "Unrecognized CD Format in File: %s\n", fileNameOrStr);
            return false;
        }
    }
    // Otherwise, treat fileNameOrStr as a pointer to the CD string (in hex or base64 encoded format)
    else
    {
        cdLen = static_cast<uint32_t>(strlen(fileNameOrStr));

        cdFmt = DetectCDFormat(reinterpret_cast<const uint8_t *>(fileNameOrStr), cdLen);
        if (cdFmt == kCDFormat_Unknown)
        {
            fprintf(stderr, "Unrecognized CD Format in the Input Argument: %s\n", fileNameOrStr);
            return false;
        }

        cdBuf = std::unique_ptr<uint8_t[]>(new uint8_t[cdLen]);
        memcpy(cdBuf.get(), fileNameOrStr, cdLen);
    }

    if (cdFmt == kCDFormat_Hex)
    {
        size_t len = chip::Encoding::HexToBytes(Uint8::to_char(cdBuf.get()), cdLen, cdBuf.get(), cdLen);
        VerifyOrReturnError(CanCastTo<uint32_t>(2 * len), false);
        VerifyOrReturnError(2 * len == cdLen, false);
        cdLen = static_cast<uint32_t>(len);
    }
    else if (cdFmt == kCDFormat_Base64)
    {
        VerifyOrReturnError(Base64Decode(cdBuf.get(), cdLen, cdBuf.get(), cdLen, cdLen), false);
    }

    VerifyOrReturnError(cdLen <= cd.size(), false);
    memcpy(cd.data(), cdBuf.get(), cdLen);

    cd.reduce_size(cdLen);

    return true;
}

void ENFORCE_FORMAT(1, 2) SimpleDumpWriter(const char * aFormat, ...)
{
    va_list args;

    va_start(args, aFormat);

    vfprintf(gOutFile, aFormat, args);

    va_end(args);
}

bool PrintCD(ByteSpan cd)
{
    chip::TLV::TLVReader reader;
    ByteSpan signerKeyId;
    ByteSpan cdContent;
    std::unique_ptr<char[]> signerKeyIdHex;
    uint32_t signerKeyIdHexLen = 0;

    VerifyOrReturnError(!cd.empty(), false);
    VerifyOrReturnError(OpenFile(gOutFileName, gOutFile, true), false);
    VerifyOrReturnError(CMS_ExtractKeyId(cd, signerKeyId) == CHIP_NO_ERROR, false);
    VerifyOrReturnError(CMS_ExtractCDContent(cd, cdContent) == CHIP_NO_ERROR, false);

    signerKeyIdHexLen = 2 * static_cast<uint32_t>(signerKeyId.size()) + 1;
    signerKeyIdHex    = std::unique_ptr<char[]>(new char[signerKeyIdHexLen]);
    VerifyOrReturnError(Encoding::BytesToUppercaseHexString(signerKeyId.data(), signerKeyId.size(), signerKeyIdHex.get(),
                                                            signerKeyIdHexLen) == CHIP_NO_ERROR,
                        false);

    fprintf(gOutFile, "SignerKeyId value: hex:%s\n", signerKeyIdHex.get());

    reader.Init(cdContent);

    VerifyOrReturnError(chip::TLV::Debug::Dump(reader, SimpleDumpWriter) == CHIP_NO_ERROR, false);

    return true;
}

} // namespace

bool Cmd_PrintCD(int argc, char * argv[])
{
    uint8_t cdBuf[kCertificationElements_TLVEncodedMaxLength] = { 0 };
    MutableByteSpan cd(cdBuf);

    if (argc == 1)
    {
        gHelpOptions.PrintBriefUsage(stderr);
        return true;
    }

    VerifyOrReturnError(ParseArgs(CMD_NAME, argc, argv, gCmdOptionSets, HandleNonOptionArgs), false);

    VerifyOrReturnError(ReadCD(gInFileNameOrStr, cd), false);

    return PrintCD(cd);
}
