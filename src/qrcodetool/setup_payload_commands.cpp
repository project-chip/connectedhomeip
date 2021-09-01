/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

#include "setup_payload_commands.h"

#include <lib/support/logging/CHIPLogging.h>
#include <setup_payload/SetupPayloadHelper.h>
#include <stdio.h>
#include <unistd.h>

using namespace chip;

enum class SetupPayloadCodeType
{
    SetupPayloadCodeTypeQR,
    SetupPayloadCodeTypeManual
};

static std::string _extractFilePath(int argc, char * const * argv)
{
    std::string path;
    if (argc == 0)
    {
        return path;
    }
    int ch;
    const char * filePath = nullptr;

    while ((ch = getopt(argc, argv, "f:")) != -1)
    {
        switch (ch)
        {
        case 'f':
            filePath = optarg;
            break;

        case '?':
        default:
            return path; /* @@@ Return 2 triggers usage message. */
        }
    }
    return std::string(filePath);
}

extern int setup_payload_operation_generate_qr_code(int argc, char * const * argv)
{
    ChipLogDetail(chipTool, "setup_payload_operation_generate_qr_code\n");
    std::string path = _extractFilePath(argc, argv);
    if (path.length() == 0)
    {
        return 2;
    }
    std::string code;
    CHIP_ERROR err = generateQRCodeFromFilePath(path, code);
    if (err == CHIP_NO_ERROR)
    {
        ChipLogDetail(chipTool, "QR Code: %s", code.c_str());
        return 0;
    }

    return 2;
}

extern int setup_payload_operation_generate_manual_code(int argc, char * const * argv)
{
    ChipLogDetail(chipTool, "setup_payload_operation_generate_qr_code\n");
    std::string path = _extractFilePath(argc, argv);
    if (path.length() == 0)
    {
        return 2;
    }
    std::string code;
    CHIP_ERROR err = generateManualCodeFromFilePath(path, code);
    if (err == CHIP_NO_ERROR)
    {
        ChipLogDetail(chipTool, "Manual Code: %s", code.c_str());
        return 0;
    }

    return 2;
}
