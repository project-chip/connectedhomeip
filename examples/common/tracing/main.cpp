/*
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 */

#include "TraceDecoder.h"
#include "TraceDecoderArgumentParser.h"

#include <lib/support/CHIPPlatformMemory.h>
#include <platform/CHIPDeviceLayer.h>

int main(int argc, char * argv[])
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    err = chip::Platform::MemoryInit();
    if (CHIP_NO_ERROR != err)
    {
        ChipLogError(NotSpecified, "Platform::MemoryInit() failure: %s", chip::ErrorStr(err));
        return EXIT_FAILURE;
    }

    VerifyOrReturnError(CHIP_NO_ERROR == TraceDecoderArgumentParser::GetInstance().ParseArguments(argc, argv), EXIT_FAILURE);

    auto decoder = chip::trace::TraceDecoder();
    decoder.SetOptions(TraceDecoderArgumentParser::GetInstance().options);

    err = decoder.ReadFile(TraceDecoderArgumentParser::GetInstance().logFile);
    if (CHIP_NO_ERROR != err)
    {
        ChipLogError(NotSpecified, "Can not decode file: %s. Error: %s", TraceDecoderArgumentParser::GetInstance().logFile,
                     chip::ErrorStr(err));
        return EXIT_FAILURE;
    }

    chip::Platform::MemoryShutdown();
    return EXIT_SUCCESS;
}
