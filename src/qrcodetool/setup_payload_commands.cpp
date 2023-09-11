/*
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
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
